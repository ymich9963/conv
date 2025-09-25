#include <sndfile.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
#include <time.h>

#define MAX_STR 500
#define MIN_STR 200
#define MAXMIN_INPUT_COUNT 2
#define MAX_EXT_STR 6
#define X_INDEX 0
#define H_INDEX 1
#define AUDIO_TYPE_CHAR 'a'
#define CSV_TYPE_CHAR   'c'
#define STR_TYPE_CHAR   's'
#define WELCOME_STR "\nConvolution tool (conv). Created by Yiannis Michael (ymich9963), 2025.\n\nUse '--version' for version information, or '--help' for the list of options.\n\nBasic usage 'Conv <Input audio file or CSV file or CSV string> [options]. For list of options use '--help'.\n"
#define VERSION_STR "\nconv v0.1.0.\n\n"
#define SND_MAJOR_FORMAT_NUM 27
#define SND_SUBTYPE_NUM 36

/* Check macros */
/* Check response from sscanf */
#define	CHECK_RES(x) ({ if (!(x)) { \
							fprintf(stderr, "Argument entered was wrong...\n"); \
							return 1; \
						} \
					  })

/* Check if an error occured to exit program */
#define	CHECK_ERR(x) ({ if ((x)) { \
							exit(EXIT_FAILURE); \
						} \
					  })

/* Check if a function returns failure */
#define	CHECK_RET(x) ({ if ((x)) { \
							return 1; \
						} \
					  })

/* Check the quiet flag and output a simple sting */
#define	STATUS(x, string) ({ if ((!x)) { \
        printf(string); \
        } \
        })

/* Check string length */
#define	CHECK_STR_LEN(x) ({ if (strlen((x)) > MAX_STR) { \
							fprintf(stderr, "Argument string length was too large. Max is %d.\n", MAX_STR); \
							return 1; \
						} \
					  })

#define	CHECK_INPUT_COUNT_MAXMIN(x) ({ if ((x) > MAXMIN_INPUT_COUNT) { \
							fprintf(stderr, "A maximum and minimum of %d inputs are needed.\n", MAXMIN_INPUT_COUNT); \
							return 1; \
						} \
					  })

typedef struct Conv_Config conv_config_t;

typedef struct InputInfo input_info_t;

typedef struct InputInfo {
    char input_type;
    char ibuff[MAX_STR];
    size_t data_samples; 
    uint8_t channels;

    int (*inp)(input_info_t* input_data, SF_INFO* sf_info, double** x);
}input_info_t;

typedef struct Conv_Config {
    /* Buffers */
    char ofile[MAX_STR];

    input_info_t input_info[MAXMIN_INPUT_COUNT];
    size_t total_samples; 
    uint8_t channels;

    /* Format specifier vars */
    char format[9];         // Format string for the output precision
    uint8_t precision;

    /* Timers */
    struct timespec start_time;
    struct timespec end_time;

    /* Flags */
    uint8_t info_flag;
    uint8_t input_flag;
    uint8_t quiet_flag;
    uint8_t timer_flag;
    uint8_t norm_flag;

    /* Function pointers */
    int (*outp)(conv_config_t* conv_conf, SF_INFO* sf_info, double* x);
} conv_config_t;

/**
 * @brief Set default values to make sure Conv runs correctly.
 *
 * @param conv_conf Conv Config struct.
 */
void set_defaults(conv_config_t* conv_conf);

/**
 * @brief Get the options specified from the CLI.
 *
 * @param argc Option count.
 * @param argv Option string array.
 * @param conv_conf Conv Config struct.
 * @return Success or failure.
 */
int get_options(int argc, char** argv, conv_config_t* conv_conf);

int get_input_type(input_info_t* input_info);

/**
 * @brief Check if the timer should be started.
 *
 * @param conv_conf Conv Config struct.
 */
void check_timer_start(conv_config_t* conv_conf);

void conv(double* x1, size_t size_x1, double* x2, size_t size_x2, double* y, size_t size_y);

/**
 * @brief Get a date and time string in HHMMSSddmmyy format.
 *
 * @return Date/time string.
 */
char* get_datetime_string();

/**
 * @brief Get the extension from the input file name.
 *
 * @param ifile_name Input file name to the function. 
 * @return Blank string or extension.
 */
char* get_extension(char* ifile_name);

/**
 * @brief Generate the output file name based on the input flag and the input file name.
 *
 * @param ofile Output file name.
 * @param ifile Input file name.
 * @param input_flag Input flag to distinguish between CSV file and string.
 */
void generate_file_name(char* ofile, input_info_t* input_info, uint8_t input_flag);

/**
 * @brief Select the output format.
 *
 * @param conv_conf Conv Config struct.
 * @param strval Option value.
 * @return Success or failure.
 */
int select_output_format(conv_config_t* conv_conf, char* strval);

int read_audio_file_input(input_info_t* conv_conf, SF_INFO* sf_info, double** x);

/**
 * @brief Open the audio file.
 *
 * @param file Pointer to SNDFILE pointer.
 * @param sf_info SF_INFO type from libsndfile.
 * @param ibuff Input buffer of size MAX_STR 
 * @return Success or failure.
 */
int open_audio_file(SNDFILE** file, SF_INFO* sf_info, char* ibuff);

/**
 * @brief Get the data from the audio file.
 *
 * @param file SNDFILE pointer.
 * @param sf_info SF_INFO type from libsndfile.
 * @param x Pointer to data buffer.
 * @return Success or failure.
 */
int get_audio_file_data(SNDFILE* file, SF_INFO* sf_info, double** x);


/**
 * @brief Get the SNDFILE major format string. Same as descriptions given in the documentation.
 *
 * @param sf_info Pointer to SF_INFO variable containing file information.
 * @return Major format string
 */
const char* get_sndfile_major_format(SF_INFO* sf_info);

/**
 * @brief Get the SNDFILE subtype string. Same as subtypes given in the documentation.
 *
 * @param sf_info Pointer to SF_INFO variable containing file information.
 * @return Subtype string.
 */
const char* get_sndfile_subtype(SF_INFO* sf_info);

/**
 * @brief Check the string if is in CSV format.
 *
 * @param ibuff Input buffer.
 * @return Success or failure.
 */
int check_csv_string(char* ibuff);

/**
 * @brief Check the extension in the input buffer if it can be read like a CSV.
 *
 * @param ibuff Input buffer.
 * @return Success or failure.
 */
int check_csv_extension(char* ibuff);

/**
 * @brief Read the input as a CSV file or CSV string.
 *
 * @param conv_conf Conv Config struct.
 * @param x Pointer to data buffer.
 * @return Success or failure.
 */
int read_csv_string_file_input(input_info_t* input_data, SF_INFO* sf_info, double** x);

/**
 * @brief Open the input file as a CSV file. If the function fails it means it's a CSV string.
 *
 * @param file Pointer to FILE pointer.
 * @param ibuff Input buffer.
 * @return Success or failure.
 */
int open_csv_file(FILE** file, char* ibuff);

/**
 * @brief Read the data from the CSV file.
 *
 * @param file FILE pointer.
 * @param data_string Pointer to a string buffer.
 * @return Success or failure.
 */
int read_csv_file_data(FILE* file, char** data_string);

/**
 * @brief Get the data from a CSV string and store it in a buffer.
 *
 * @param data_string String containing the data.
 * @param x Pointer to buffer to store the data.
 * @param detected_samples Variable to store the detected samples in the string.
 * @return Success or failure.
 */
int get_data_from_string(char* data_string, double** x, size_t* detected_samples);

void show_input_info(input_info_t* input_info, SF_INFO* sf_info);

/**
 * @brief Set the precision to output values in.
 *
 * @param format Format string to be used in printf().
 * @param precision Precision amount.
 */
void set_precision_format(char format[9], uint8_t precision);

int (*autoset_output_format(char type_x, char type_h)) (conv_config_t*, SF_INFO*, double*);

/**
 * @brief Check if the timer should be stopped and outputed.
 *
 * @param conv_conf Conv Config struct.
 */
void check_timer_end_output(conv_config_t* conv_conf);

/**
 * @brief Output result to stdout.
 *
 * @param conv_conf WindFcn config struct.
 * @param sf_info Input file SF_INFO struct. Unused in this function.
 * @param x Data buffer.
 * @return Success or failure.
 */
int output_stdout(conv_config_t* conv_conf, SF_INFO* sf_info, double* x);

/**
 * @brief Output result to stdout as CSVs.
 *
 * @param conv_conf WindFcn config struct.
 * @param sf_info Input file SF_INFO struct. Unused in this function.
 * @param x Data buffer.
 * @return Success or failure.
 */
int output_stdout_csv(conv_config_t* conv_conf, SF_INFO* sf_info, double* x);

/**
 * @brief Output result to a file as a column.
 *
 * @param conv_conf WindFcn config struct.
 * @param sf_info Input file SF_INFO struct. Unused in this function.
 * @param x Data buffer.
 * @return Success or failure.
 */
int output_file_columns(conv_config_t* conv_conf, SF_INFO* sf_info, double* x);

/**
 * @brief Output result to a CSV file.
 *
 * @param conv_conf WindFcn config struct.
 * @param sf_info Input file SF_INFO struct. Unused in this function.
 * @param x Data buffer.
 * @return Success or failure.
 */
int output_file_csv(conv_config_t* conv_conf, SF_INFO* sf_info, double* x);

/**
 * @brief Output result as an audio file.
 *
 * @param conv_conf WindFcn config struct.
 * @param sf_info Input file SF_INFO struct.
 * @param x Data buffer.
 * @return Success or failure.
 */
int output_file_audio(conv_config_t* conv_conf, SF_INFO* sf_info, double* x);

void normalise_data(double* x, size_t size);

/**
 * @brief Output the '--help' option.
 * @return Success or failure.
 */
int output_help();
