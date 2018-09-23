#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <cuda_runtime.h>

extern "C" { 
    #include "image_io.h" 
}

#include "segmentation.h"

#define DEFAULT_N_CLUS 4
#define DEFAULT_MAX_ITERS 150
#define DEFAULT_OUT_PATH "result.jpg"

double get_time();
void print_devices();
void print_usage(char *pgr_name);
void print_exec(int width, int height, int n_ch, int n_clus, int n_iters, double sse, double exec_time);

int main(int argc, char **argv)
{
    char *in_path = NULL;
    char *out_path = DEFAULT_OUT_PATH;
    byte_t *data;
    int width, height, n_ch;
    int n_clus = DEFAULT_N_CLUS;
    int n_iters = DEFAULT_MAX_ITERS;
    int seed = time(NULL);
    double sse, start_time, exec_time;

    // PARSING ARGUMENTS AND OPTIONAL PARAMETERS

    char optchar;
    while ((optchar = getopt(argc, argv, "k:m:o:s:h")) != -1) {
        switch (optchar) {
            case 'k':
                n_clus = strtol(optarg, NULL, 10);
                break;
            case 'm':
                n_iters = strtol(optarg, NULL, 10);
                break;
            case 'o':
                out_path = optarg;
                break;
            case 's':
                seed = strtol(optarg, NULL, 10);
                break;
            case 'h':
            default:
                print_usage(argv[0]);
                exit(EXIT_FAILURE);
                break;
        }
    }

    in_path = argv[optind];

    // VALIDATING INPUT PARAMETERS

    if (in_path == NULL) {
        print_usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    if (n_clus < 2) {
        fprintf(stderr, "INPUT ERROR: << Invalid number of clusters >> \n");
        exit(EXIT_FAILURE);
    }

    if (n_iters < 1) {
        fprintf(stderr, "INPUT ERROR: << Invalid maximum number of iterations >> \n");
        exit(EXIT_FAILURE);
    }

    srand(seed);

    print_devices();

    // SCANNING INPUT IMAGE

    data = img_load(in_path, &width, &height, &n_ch);

    // EXECUTING KMEANS SEGMENTATION

    start_time = get_time();
    kmeans_segm_cuda(data, width, height, n_ch, n_clus, &n_iters, &sse);
    exec_time = get_time() - start_time;

    // SAVING AND PRINTING RESULTS

    img_save(out_path, data, width, height, n_ch);
    print_exec(width, height, n_ch, n_clus, n_iters, sse, exec_time);

    free(data);

    return EXIT_SUCCESS;
}

double get_time()
{
    struct timeval timecheck;

    gettimeofday(&timecheck, NULL);

    return timecheck.tv_sec + timecheck.tv_usec / 1000000.0;
}

void print_usage(char *pgr_name)
{
    const char *usage = "\nPROGRAM USAGE \n\n"
        "   %s [-h] [-k num_clusters] [-m max_iters] [-o output_img] \n"
        "                [-o output_img] [-s seed] input_image \n\n"
        "   The input image filepath is the only mandatory argument and \n"
        "   must be specified last, after all the optional parameters. \n"
        "   Valid input image formats are JPEG, PNG, BMP, GIF, TGA, PSD, \n"
        "   PIC, HDR and PNM. The program performs a color-based segmentation\n"
        "   of the input image using a parallel version the k-means \n"
        "   clustering algorithm implemented via CUDA.\n\n"
        "OPTIONAL PARAMETERS \n\n"
        "   -k num_clusters : number of clusters to use for the segmentation of \n"
        "                     the image. Must be bigger than 1. Default is %d. \n"
        "   -m max_iters    : maximum number of iterations that the clustering \n"
        "                     algorithm can perform before being forced to stop. \n"
        "                     Must be bigger that 0. Default is %d. \n"
        "   -o output_image : filepath of the output image. Valid output image \n"
        "                     formats are JPEG, PNG, BMP and TGA. If not specified, \n"
        "                     the resulting image will be saved in the current \n"
        "                     directory using JPEG format. \n"
        "   -s seed         : seed to use for the random selection of the initial \n"
        "                     centers. The clustering algorithm will always use \n"
        "                     the same set of initial centers if the same \n"
        "                     seed is specified. \n"
        "   -h              : print usage information. \n\n";

    fprintf(stderr, usage, pgr_name, DEFAULT_N_CLUS, DEFAULT_MAX_ITERS);
}

void print_devices() 
{
    int i, n_devices;
    cudaDeviceProp prop;

    cudaGetDeviceCount(&n_devices);

    printf("\nCUDA DEVICES AVAILABLE\n\n");
    
    for (i = 0; i < n_devices; i++) {
        cudaGetDeviceProperties(&prop, i);
        printf("  Device number       : %d\n", i);
        printf("  Device name         : %s\n", prop.name);
        printf("  Compute capability  : %d.%d\n\n", prop.major, prop.minor);
    }
}

void print_exec(int width, int height, int n_ch, int n_clus, int n_iters, double sse, double exec_time)
{
    const char *details = "EXECUTION DETAILS\n\n"
        "  Image size             : %d x %d\n"
        "  Color channels         : %d\n"
        "  Number of clusters     : %d\n"
        "  Number of iterations   : %d\n"
        "  Sum of squared errors  : %f\n"
        "  Execution time         : %f\n\n";

    fprintf(stdout, details, width, height, n_ch, n_clus, n_iters, sse, exec_time);
}
 