/**
 * @file ppm_example.c
 * @author Florent Gluck
 * @date 4 March 2021
 * @brief Simple example program showing how to use the PPM routines.
 */

#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <string.h>
#include "ppm.h"

/**
 * Display the program's syntaxe.
 * @param argv program's command line arguments
 */
void usage(char **argv) {
    fprintf(stderr, "usage: %s [-ascii] input output\n"\
        "Where input and output are PPM files and the optional argument\n"\
        "-ascii specifies to write a plain text PPM file.\n", basename(argv[0]));
    exit(EXIT_FAILURE);
}

/**
 * Program entry point.
 * @param argc command line argument count
 * @param argv program's command line arguments
 */
int main(int argc, char **argv) {
    char *input = NULL, *output = NULL;
    enum PPM_TYPE type;

    // Parse command line
    if (argc == 3) {
        type = PPM_RAW;
        input = argv[1];
        output = argv[2];
    }
    else if (argc ==  4) {
        if (strcmp("-ascii", argv[1]) != 0) usage(argv);
        type = PPM_ASCII;
        input = argv[2];
        output = argv[3];
    }
    else {
        usage(argv);
    }

    // Load image
    img_t *img = load_ppm(input);
    if (!img) {
        fprintf(stderr, "Failed loading \"%s\"!\n", input);
        return EXIT_FAILURE;
    }

    // Reduce image's first quadrant's brightness intensity
    for (int j = 0; j < img->height/2; j++) {
        for (int i = 0; i < img->width/2; i++) {
            pixel_t *p = &img->pix2d[j][i];
            p->r /= 2;
            p->g /= 2;
            p->b /= 2;
        }
    }

    // Write image
    if (!write_ppm(output, img, type)) {
        fprintf(stderr, "Failed writing \"%s\"!\n", output);
        free_img(img);
        return EXIT_FAILURE;
    }

    free_img(img);
    return EXIT_SUCCESS;
}
