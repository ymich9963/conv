#include "conv.h"

void set_defaults(conv_config_t* restrict conv_conf)
{
    memset(conv_conf->ofile, '\0', MAX_STR);
    memset(conv_conf->input_info, '\0', sizeof(input_info_t) * MAXMIN_INPUT_COUNT);

    conv_conf->total_samples    = 0;
    conv_conf->precision        = 6;

    conv_conf->info_flag    = 0;
    conv_conf->input_flag   = 0;
    conv_conf->quiet_flag   = 0;
    conv_conf->timer_flag   = 0;
    conv_conf->norm_flag    = 0;

    conv_conf->outp    = NULL;
}

int get_options(int argc, char** restrict argv, conv_config_t* restrict conv_conf)
{
    int dval = 0;
    int input_count = 0;

    if (argc == 1) {
        fprintf(stdout, WELCOME_STR);

        return 1;
    }

    if (argc == 2) {
        if (!(strcmp("--version", argv[1]))) {
            fprintf(stdout, VERSION_STR);

            return 1;
        }

        if (!(strcmp("--help", argv[1]))) {
            output_help();

            return 1;
        }
    }

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] != '-' && argv[i - 1][0] != '-') {
            CHECK_STR_LEN(argv[i]);
            strcpy(conv_conf->input_info[input_count].ibuff, argv[i]);
            CHECK_INPUT_COUNT_MAXMIN(input_count);
            CHECK_RET(get_input_type(&conv_conf->input_info[input_count]));
            input_count++;
            continue;
        }

        if (!(strcmp("-i", argv[i]))) {
            CHECK_STR_LEN(argv[i + 1]);
            strcpy(conv_conf->input_info[input_count].ibuff, argv[i + 1]);
            CHECK_INPUT_COUNT_MAXMIN(input_count);
            CHECK_RET(get_input_type(&conv_conf->input_info[input_count]));
            input_count++;
            i++;
            continue;
        }

        if (!(strcmp("-o", argv[i])) || !(strcmp("--output", argv[i]))) {
            CHECK_STR_LEN(argv[i + 1]);
            strcpy(conv_conf->ofile, argv[i + 1]);
            i++;
            continue;
        }

        if (!(strcmp("-f", argv[i])) || !(strcmp("--output-format", argv[i]))) {
            CHECK_STR_LEN(argv[i + 1]);
            CHECK_RET(select_output_format(conv_conf, argv[i + 1]));
            i++;
            continue;
        }

        if (!(strcmp("-p", argv[i])) || !(strcmp("--precision", argv[i]))) {
            CHECK_RES(sscanf(argv[i + 1], "%d", &dval));
            conv_conf->precision = dval;
            i++;
            continue;
        }

        if (!(strcmp("--info", argv[i]))) {
            conv_conf->info_flag = 1;
            continue;
        }

        if (!(strcmp("--timer", argv[i]))) {
            conv_conf->timer_flag = 1;
            continue;
        }

        if (!(strcmp("--norm", argv[i])) || !(strcmp("--normalise", argv[i]))) {
            conv_conf->norm_flag = 1; 
            continue;
        }

        if (!(strcmp("-q", argv[i])) || !(strcmp("--quiet", argv[i]))) {
            conv_conf->quiet_flag = 1; 
            continue;
        }

        fprintf(stderr, "\nNo such option '%s'. Please check inputs.\n\n", argv[i]);

        return 1;
    }

    return 0;
}

void check_timer_start(conv_config_t* restrict conv_conf)
{
    if (conv_conf->timer_flag && !conv_conf->quiet_flag) {
        timespec_get(&conv_conf->start_time, TIME_UTC);
        printf("Started timer.\n");
    }
}

int (*autoset_output_format(char type_x, char type_h)) (conv_config_t*, SF_INFO*, double*) {
    if (type_x == 'a' || type_h == 'a') {
        return &output_file_audio;
    } else if (type_x == 'c' || type_h == 'c') {
        return &output_file_csv;
    } else {
        return &output_stdout;
    }
}

int get_input_type(input_info_t* restrict input_info)
{
    SNDFILE* file = NULL;   // Pointer to the input audio file
    SF_INFO sf_info;        // Input audio file info

    /* Initialise the struct */
    memset(&sf_info, 0, sizeof(SF_INFO));

    /* Check if input is valid and assign the type */
    file = sf_open(input_info->ibuff, SFM_READ, &sf_info);
    if (file) {
        input_info->input_type = AUDIO_TYPE_CHAR;
        input_info->inp = &read_audio_file_input;
    } else if (!(check_csv_extension(get_extension(input_info->ibuff)))) {
        input_info->input_type = CSV_TYPE_CHAR;
        input_info->inp = &read_csv_string_file_input;
    } else if (!(check_csv_string(input_info->ibuff))) {
        input_info->input_type = STR_TYPE_CHAR;
        input_info->inp = &read_csv_string_file_input;
    } else {
        fprintf(stderr, "Input '%s' is not an audio file or a CSV file/string.\n", input_info->ibuff);

        return 1;
    }

    sf_close(file);

    return 0;
}

void conv(double* x, size_t size_x, double* h, size_t size_h, double* y, size_t size_y)
{
    for(size_t n = 0; n < size_y; n++) {

        /* Limits */
        size_t k_min = (0 > (int64_t)(n - (size_h - 1))) ? 0 : n - (size_h - 1);
        size_t k_max = (n < size_x - 1) ? n : size_x - 1;

        for(size_t k = k_min; k <= k_max; k++) {
            y[n] += x[k] * h[n - k];
        }
    }
}

char* get_datetime_string()
{
    time_t time_since_epoch = time(NULL);
    struct tm* tm = localtime(&time_since_epoch);
    static char s[13];
    strftime(s, sizeof(s), "%d%m%y%H%M%S", tm);

    return s;
}

char* get_extension(char* restrict ifile_name)
{
    uint8_t extension_len = 0;
    uint8_t dot_index = 0;
    static char* extension = NULL;

    /* Remove the path specifier */
    if (ifile_name[0] == '.' && ifile_name[1] == '\\') {
        memmove(ifile_name, ifile_name + 2, MAX_STR - 2);
    }

    const uint16_t ifile_name_length = strlen(ifile_name);

    for (uint16_t i = ifile_name_length; i > 0; i--) {
        if (ifile_name[i] != '\0') {
            extension_len++;
            if (ifile_name[i] == '.') {
                dot_index = i;
                break;
            }
        }
    }

    extension = calloc(sizeof(char), extension_len);
    for (uint16_t i = dot_index; i < ifile_name_length; i++) {
        if (ifile_name[i] != '\0') {
            extension[i - dot_index] = ifile_name[i];
        }
    }

    if (strlen(extension) < 1) {
        free(extension);

        return "\0";
    }

    return extension;
}

void generate_file_name(char* restrict ofile, input_info_t* restrict input_info, uint8_t input_flag)
{
    if (ofile[0] != '\0' ) {

        return;
    }

    char* ifile_no_extension_x = calloc(sizeof(char), MIN_STR);
    char* ifile_no_extension_h = calloc(sizeof(char), MIN_STR);
    char* extension_x = calloc(sizeof(char), MAX_EXT_STR);
    char* extension_h = calloc(sizeof(char), MAX_EXT_STR);


    switch (input_info[X_INDEX].input_type) {
        case AUDIO_TYPE_CHAR:
        case CSV_TYPE_CHAR:
            extension_x = get_extension(input_info[X_INDEX].ibuff);
            strncpy(ifile_no_extension_x, input_info[X_INDEX].ibuff, strlen(input_info[X_INDEX].ibuff) - strlen(extension_x));
            break;
        case STR_TYPE_CHAR:
            strcpy(ifile_no_extension_x, "stringcsv");
            break;
        default:
            fprintf(stderr, "Type char not supported.\n");
            return;
    }

    switch (input_info[H_INDEX].input_type) {
        case AUDIO_TYPE_CHAR:
        case CSV_TYPE_CHAR:
            extension_h = get_extension(input_info[H_INDEX].ibuff);
            strncpy(ifile_no_extension_h, input_info[H_INDEX].ibuff, strlen(input_info[H_INDEX].ibuff) - strlen(extension_h));
            break;
        case STR_TYPE_CHAR:
            strcpy(ifile_no_extension_h, "stringcsv");
            break;
        default:
            fprintf(stderr, "Type char not supported.\n");
            return;
    }

    /* Use the h[n] input to decide the output extension */
    sprintf(ofile, "conv-%s-%s-%s%s", ifile_no_extension_x, ifile_no_extension_h, get_datetime_string(), extension_h);
}

int select_output_format(conv_config_t* restrict conv_conf, char* restrict strval)
{
    conv_conf->outp = NULL;

    if(!(strcmp("audio", strval))) {
        conv_conf->outp = &output_file_audio; 
    }
    if(!(strcmp("stdout", strval))) {
        conv_conf->outp = &output_stdout; 
    }
    if(!(strcmp("stdout-csv", strval))) {
        conv_conf->outp = &output_stdout_csv; 
    }
    if(!(strcmp("columns", strval))) {
        conv_conf->outp = &output_file_columns; 
    }
    if(!(strcmp("csv", strval))) {
        conv_conf->outp = &output_file_csv; 
    }

    if (!conv_conf->outp){
        fprintf(stderr, "\nOutput format '%s' not available.\n", strval);

        return 1;
    }

    return 0;
}

int read_audio_file_input(input_info_t* restrict input_data, SF_INFO* restrict sf_info, double** restrict x)
{
    SNDFILE* file = NULL;          // Pointer to the input audio file

    /* Open the input file */
    CHECK_ERR(open_audio_file(&file, sf_info, input_data->ibuff));

    /* Read the input audio file */
    CHECK_ERR(get_audio_file_data(file, sf_info, x));

    input_data->data_samples = sf_info->frames;
    input_data->channels = sf_info->channels;

    sf_close(file);
    return 0;
}

int open_audio_file(SNDFILE** restrict file, SF_INFO* restrict sf_info, char* restrict ibuff)
{
    *file = sf_open(ibuff, SFM_READ, sf_info);
    if(!(*file)) {
        fprintf(stderr, "%s\n", sf_strerror(*file));

        return 1;
    }

    return 0;
}

int get_audio_file_data(SNDFILE* restrict file, SF_INFO* restrict sf_info, double** restrict x)
{
    /* Get audio file data size */
    *x = calloc(sf_info->frames * sf_info->channels, sizeof(double));

    /* Read data and place into buffer */
    sf_count_t sf_count = sf_readf_double(file, *x, sf_info->frames);

    /* Check */
    if (sf_count != sf_info->frames) {
        fprintf(stderr, "\nRead count not equal to requested frames, %lld != %lld.\n", sf_count, sf_info->frames);

        return 1;
    }

    return 0;
}

void show_input_info(input_info_t* restrict input_info, SF_INFO* restrict sf_info)
{
        if (input_info->input_type == 'a') {
            fprintf(stdout, "File Name: %s\n", input_info->ibuff);
            fprintf(stdout, "Sample Rate: %d\n", sf_info->samplerate);
            fprintf(stdout, "Samples: %lld\n", sf_info->frames);
            fprintf(stdout, "Channels: %d\n", sf_info->channels);
            fprintf(stdout, "Format: %s\n", get_sndfile_major_format(sf_info));
            fprintf(stdout, "Subtype: %s\n", get_sndfile_subtype(sf_info));
        } else {
            fprintf(stdout, input_info->input_type == 'c' ? "File Name: %s\n" : "Input String: %s\n", input_info->ibuff);
            fprintf(stdout, "Samples: %lld\n", input_info->data_samples);
            fprintf(stdout, input_info->input_type == 'c' ? "Format: CSV File\n" : "Format: CSV String\n");
        }

}

const char* get_sndfile_major_format(SF_INFO* restrict sf_info)
{
    SF_FORMAT_INFO format_info ;
    int k, count;
    const uint32_t format_mask = 0x00FF0000;
    const uint32_t major_format = sf_info->format & format_mask;

    sf_command(NULL, SFC_GET_FORMAT_MAJOR_COUNT, &count, sizeof (int));

    for (k = 0 ; k < count ; k++) {
        format_info.format = k;
        sf_command(NULL, SFC_GET_FORMAT_MAJOR, &format_info, sizeof(format_info));
        if (major_format == format_info.format) {
            return format_info.name;
        }
    }

    return "N/A";
}

const char* get_sndfile_subtype(SF_INFO* restrict sf_info)
{
    SF_FORMAT_INFO format_info ;
    int k, count;
    const uint16_t subtype_mask = 0x00FF;
    const uint16_t subtype = sf_info->format & subtype_mask;

    sf_command(NULL, SFC_GET_FORMAT_SUBTYPE_COUNT, &count, sizeof (int));

    for (k = 0 ; k < count ; k++) {
        format_info.format = k;
        sf_command(NULL, SFC_GET_FORMAT_SUBTYPE, &format_info, sizeof(format_info));
        if (subtype == format_info.format) {
            return format_info.name;
        }
    }

    return "N/A";
}

int check_csv_string(char* restrict ibuff)
{
    /* Make a copy of the string to get the number of data points  */
    char* data_string_copy = calloc(strlen(ibuff), sizeof(char)); 
    strcpy(data_string_copy, ibuff);

    /* Get the number of data points */
    size_t samples = 0;
    char* token = strtok(data_string_copy, ",");
    while (token != NULL) {
        samples++;
        token = strtok(NULL, ",");
    }

    free(data_string_copy);

    if (samples > 1) {
        return 0;
    } else {
        return 1;
    }
}

int check_csv_extension(char* restrict extension)
{
    const int supported_csv_ext_num = 2;
    char* supported_csv_ext[] = {".csv", ".txt"};
    uint8_t supported_flag = 0;
    
    for (int i = 0; i < supported_csv_ext_num; i++) {
        if (!strcmp(supported_csv_ext[i], extension)) {
            supported_flag = 1;
        }
    }

    if (!supported_flag) {
        return 1;
    }

    return 0;
}

int read_csv_string_file_input(input_info_t* restrict input_data, SF_INFO* restrict sf_info, double** restrict x)
{
    FILE* file = NULL;          // Pointer to the input audio file
    char* data_string = NULL;   // Input data from file

    /* Try to open and read input file, if not then it's considered a data string */
    if (!(open_csv_file(&file, input_data->ibuff))) {
        CHECK_ERR(read_csv_file_data(file, &data_string));
        // input_data->input_flag = 0;
    } else {
        data_string = calloc(strlen(input_data->ibuff) + 1, sizeof(char)); 
        strcpy(data_string, input_data->ibuff);
        // input_data->input_flag = 1;
    }

    get_data_from_string(data_string, x, &input_data->data_samples);

    /* Output info on the inputted file */
    // show_input_csv_info(input_data);

    free(data_string);
    fclose(file);
    return 0;
}

int open_csv_file(FILE** restrict file, char* restrict ibuff)
{
    *file = fopen(ibuff, "r");
    if(!(*file)) {

        return 1;
    }

    return 0;
}

int read_csv_file_data(FILE* restrict file, char** restrict data_string)
{
    /* Go to the end of the file to find the size needed to store the data */
    fseek(file, 0, SEEK_END);

    long pos = ftell(file);

    /* Put the file position indicator back to the start */
    long ret = fseek(file, 0, SEEK_SET);
    if (ret == -1L || pos < 0) {
        fprintf(stderr, "Error when trying to read CSV file data.\n");

        return 1;
    }

    /* Allocate the file size */
    *data_string = calloc(pos + 1, sizeof(char));

    /* Read the file data and place into string variable */
    fgets(*data_string, pos + 1, file);

    return 0;
}

int get_data_from_string(char* restrict data_string, double** restrict x, size_t* restrict detected_samples)
{
    /* Make a copy of the string to get the number of data points  */
    char* data_string_copy = malloc(sizeof(char) * strlen(data_string)); 
    strcpy(data_string_copy, data_string);

    /* Get the number of data points */
    size_t samples = 0;
    char* token = strtok(data_string_copy, ",");
    while (token != NULL) {
        samples++;
        token = strtok(NULL, ",");
    }

    /* Allocate the size of the array based on the data points */
    *x = calloc(samples, sizeof(double));

    /* Retrieve the data */
    size_t i = 0;
    double lfval = 0.0f;
    token = strtok(data_string, ",");
    while (token != NULL && i < samples) {
        sscanf(token, "%lf", &lfval);
        (*x)[i] = lfval;
        token = strtok(NULL, ",");
        i++;
    }

    *detected_samples = samples;
    free(data_string_copy);

    return 0;
}

void normalise_data(double* restrict x, size_t size)
{
    double max_abs_val = 0;
    double val = 0;
    for (size_t i = 0; i < size; i++) {
        if (x[i] > 0.0f) {
            val = x[i];
        } else {
            val = -x[i];
        }
        if (val > max_abs_val) {
            max_abs_val = val;
        }
    }

    for (size_t i = 0; i < size; i++) {
        x[i] /= max_abs_val;
    }
}

int output_file_audio(conv_config_t* restrict conv_conf, SF_INFO* restrict sf_info, double* restrict x)
{
    SNDFILE* sndfile = sf_open(conv_conf->ofile, SFM_WRITE, sf_info);
    if(!(sndfile)) {
        fprintf(stderr, "%s\n", sf_strerror(sndfile));

        return 1;
    }

    sf_write_double(sndfile, x, conv_conf->total_samples);

    sf_close(sndfile);
    printf("Saved result to '%s'.\n", conv_conf->ofile);
    return 0;
}

void set_precision_format(char format[9], uint8_t precision)
{
    sprintf(format, "%%.%dlf", precision);
}

int output_stdout(conv_config_t* restrict conv_conf, SF_INFO* restrict sf_info, double* restrict x)
{
    FILE * file = stdout;
    if (!conv_conf->quiet_flag) {
        fprintf(file, "\n");
    }

    set_precision_format(conv_conf->format, conv_conf->precision);
    for (size_t i = 0; i < conv_conf->total_samples; i++) {
        fprintf(file, "%lf", x[i]);
        fprintf(file, "\n");
    }

    return 0;
}

int output_stdout_csv(conv_config_t* restrict conv_conf, SF_INFO* restrict sf_info, double* restrict x)
{
    FILE * file = stdout;
    if (!conv_conf->quiet_flag) {
        fprintf(file, "\n");
    }

    set_precision_format(conv_conf->format, conv_conf->precision);
    for (size_t i = 0; i < conv_conf->total_samples - 1; i++) {
        fprintf(file, conv_conf->format, x[i]);
        fprintf(file, ",");
    }
    fprintf(file, conv_conf->format, x[conv_conf->total_samples]);
    fprintf(file, "\n");

    return 0;
}

int output_file_columns(conv_config_t* restrict conv_conf, SF_INFO* restrict sf_info, double* restrict x)
{
    FILE* file = fopen(conv_conf->ofile, "w");
    if(!(file)) {
        fprintf(stderr, "\nError, unable to open output file.\n\n");

        return 1;
    };

    set_precision_format(conv_conf->format, conv_conf->precision);
    for (size_t i = 0; i < conv_conf->total_samples; i++) {
        fprintf(file, conv_conf->format, x[i]);
        fprintf(file, "\n");
    }

    if (!conv_conf->quiet_flag) {
        printf("Outputted data to '%s'.\n", conv_conf->ofile);
    }

    fclose(file);
    return 0;
}

int output_file_csv(conv_config_t* restrict conv_conf, SF_INFO* restrict sf_info, double* restrict x)
{
    FILE* file = fopen(conv_conf->ofile, "w");
    if(!(file)) {
        fprintf(stderr, "\nError, unable to open output file.\n\n");

        return 1;
    };

    set_precision_format(conv_conf->format, conv_conf->precision);
    for (size_t i = 0; i < conv_conf->total_samples - 1; i++) {
        fprintf(file, conv_conf->format, x[i]);
        fprintf(file, ",");
    }
    fprintf(file, conv_conf->format, x[conv_conf->total_samples]);

    if (!conv_conf->quiet_flag) {
        printf("Outputted data to '%s'.\n", conv_conf->ofile);
    }

    fclose(file);
    return 0;
}

void check_timer_end_output(conv_config_t* conv_conf)
{
    if (conv_conf->timer_flag && !conv_conf->quiet_flag) {
        double time_taken;
        timespec_get(&conv_conf->end_time, TIME_UTC);
        time_taken = (conv_conf->end_time.tv_sec - conv_conf->start_time.tv_sec) + ((conv_conf->end_time.tv_nsec - conv_conf->start_time.tv_nsec) / 1e9);
        printf("Time taken: %.9lf seconds\n", time_taken);
    }
}

int output_help()
{
    printf( "\n"
            "Convolution tool (conv) help page.\n\n"
            "Basic usage 'conv <Input audio file or CSV file or CSV string> <Input audio file or CSV file or CSV string> [options]. For list of options see below.\n\n"
            "\t\t--info\t\t\t\t= Output to stdout some info about the input file.\n"
            "\t-i,\t--input <File/String>\t\t= Accepts audio files and CSV files or strings. Make sure to separate string with commas, e.g. 1,0,0,1. Use the options below if you want to specify but DFTT implements auto-detection.\n"
            "\t-o,\t--output <File Name>\t\t= Path or name of the output file.\n"
            "\t-f,\t--output-format <Format>\t= Format of the output file. Select between: 'audio', 'stdout', 'stdout-csv', 'columns', and 'csv'.\n"
            "\t-p,\t--precision <Number>\t\t= Decimal number to define how many decimal places to output.\n"
            "\t--norm,\t--normalise\t\t\t= Normalise the data. Only works wit --pow.\n"
            "\t\t--timer\t\t\t\t= Start a timer to see how long the calculation takes.\n"
            "\t-q,\t--quiet\t\t\t\t= Silence all status messages to stdout. Overwrites '--info'.\n"
            "\n"
            );

    return 0;
}
