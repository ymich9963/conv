#include "conv.h"

// TODO: Do one with padding and one without. Maybe do different types e.g. fast convolution
// TODO: Check normalise_data function in DFTT because it might act weird with neg nums
// TODO: Multiple channels? Choose the sf_info with the most channels
// FIX: --info output

int main(int argc, char** argv)
{
    SF_INFO sf_info_x = {0};
    SF_INFO sf_info_h = {0};
    SF_INFO sf_info_y = {0};
    double* x = NULL;
    double* h = NULL;
    double* y = NULL;
    conv_config_t conv_conf = {0};

    set_defaults(&conv_conf);

    CHECK_ERR(get_options(argc, argv, &conv_conf));

    /* Read both the inputs */
    conv_conf.input_info[X_INDEX].inp(&conv_conf.input_info[X_INDEX], &sf_info_x, &x);
    conv_conf.input_info[H_INDEX].inp(&conv_conf.input_info[H_INDEX], &sf_info_h, &h);

    if (!conv_conf.quiet_flag) {
       fprintf(stdout, "\n--INFO--");
       fprintf(stdout, "\n\t=X INPUT=\n");
       show_input_info(&conv_conf.input_info[X_INDEX], &sf_info_x, conv_conf.info_flag, conv_conf.quiet_flag); 
       fprintf(stdout, "\n\t=H INPUT=\n");
       show_input_info(&conv_conf.input_info[H_INDEX], &sf_info_h, conv_conf.info_flag, conv_conf.quiet_flag); 
       fprintf(stdout, "---\n\n");
    }

    /* Get the largest total samples from the inputs */
    size_t size_x = conv_conf.input_info[X_INDEX].data_samples;
    size_t size_h = conv_conf.input_info[H_INDEX].data_samples;
    size_t size_y = size_x + size_h - 1;

    conv_conf.total_samples = size_y;
    conv_conf.channels = conv_conf.input_info[X_INDEX].channels > conv_conf.input_info[H_INDEX].channels ? conv_conf.input_info[X_INDEX].channels : conv_conf.input_info[H_INDEX].channels;

    /* Allocate output array */
    y = calloc(sizeof(double), size_y);

    fprintf(stdout, "Executing convolution...\n");

    /* Start timer */
    check_timer_start(&conv_conf);

    /* Execute convolution sum */
    conv(x, size_x, h, size_h, y, size_y);

    /* Stop timer and output */
    check_timer_end_output(&conv_conf);

    fprintf(stdout, "Convolution finished.\n");

    /* If no output type is specified, set it based on inputs */
    if (conv_conf.outp == NULL) {
        conv_conf.outp = autoset_output_format(conv_conf.input_info[X_INDEX].input_type, conv_conf.input_info[H_INDEX].input_type);
    }

    /* Normalise data */
    if (conv_conf.norm_flag) {
        normalise_data(y, size_y);
    }

    /* Generate the output file name */
    generate_file_name(conv_conf.ofile, conv_conf.input_info, conv_conf.input_flag);

    if (sf_info_x.format != 0) {
        sf_info_y = sf_info_x;
    } else if (sf_info_h.format != 0) {
        sf_info_y = sf_info_h;
    }

    /* Output to specified buffer */
    conv_conf.outp(&conv_conf, &sf_info_y, y);

    return 0;
}
