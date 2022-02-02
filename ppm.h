/**
 * @file ppm.h
 * @author Florent Gluck
 * @date 4 Mar 2021
 * @brief Routines to read and write PPM files.
 */

#include <stdint.h>
#include <stdbool.h>

/**
 * Store a 24-bit pixel (8-bit per component).
 * @param r the red component
 * @param g the green component
 * @param b the blue component
 */
typedef struct pixel_st {
    uint8_t r, g, b;
} pixel_t;

/**
 * Structure holding a 24-bit per pixel image.
 * @param width the width of the image
 * @param height the height of the image
 * @param pix1d accessor to the image pixel data as a 1D array
 * @param pix2d accessor to the image pixel data as a 2D array [height][width]
 */
typedef struct img_st {
    int width;
    int height;
    pixel_t *pix1d;
    pixel_t **pix2d;
} img_t;

/**
 * Supported PPM types, either RAW or ASCII.
 */
enum PPM_TYPE {
    PPM_RAW,
    PPM_ASCII
};

extern img_t *alloc_img(int width, int height);
extern void free_img(img_t *img);
extern img_t *load_ppm(char *filename);
extern bool write_ppm(char *filename, img_t *img, enum PPM_TYPE);

