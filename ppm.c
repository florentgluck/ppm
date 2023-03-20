/**
 * @file ppm.c
 * @author Florent Gluck
 * @date 4 Mar 2021
 * @brief Routines to read and write PPM files.
 *
 * Both binary (P6 type) and plain ASCII (P3 type) PPM file types are supported.
 * The PPM file format is described here: http://netpbm.sourceforge.net/doc/ppm.html
 *
 * To convert a JPG image into a binary PPM file (P6) with ImageMagick:
 * convert image.jpg output.ppm
 *
 * To convert a JPG image into a plain ASCII PPM file (P3) with ImageMagick:
 * convert -compress none image.jpg output.ppm
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ppm.h"

typedef struct {
    enum PPM_TYPE type;
    unsigned int width;
    unsigned int height;
    unsigned int maxval;
    long data_offset;   // offset in the file of the image data (pixels)
} ppm_header_t;

static void readline(FILE *f, char *line, int max_line_length);
static ppm_header_t *load_header(char *filename);

/**
 * Allocate the memory for an image of size width*height
 * @param width the width of the image to allocate
 * @param height the height of the image to allocate
 * @return a pointer to the allocated image or NULL if the allocation failed
 */
img_t *alloc_img(int width, int height) {
    img_t *img = malloc(sizeof(img_t));

    if (!img) return NULL;

    img->width = width;
    img->height = height;
    img->pix1d = malloc(sizeof(pixel_t) * width * height);
    if (!img->pix1d) {
        free(img);
        return NULL;
    }

    img->pix2d = malloc(sizeof(pixel_t*) * height);
    if (!img->pix2d) {
        free(img->pix1d);
        free(img);
        return NULL;
    }

    for (int i = 0; i < height; i++)
        img->pix2d[i] = img->pix1d + width*i;

    return img;
}

/**
 * Free an allocated image.
 * @param img a pointer to the image to free
 */
void free_img(img_t *img) {
    free(img->pix1d);
    free(img->pix2d);
    free(img);
}

/**
 * Write a 24-bit RGB PPM file (either ASCII P3 type or binary P6 type).
 * @param filename (absolute or relative path) of the image to write
 * @param img a pointer to the image to write
 * @param PPM_TYPE the type of the image to write (binary or ASCII)
 * @return boolean value indicating whether the write succeeded or not
 */
bool write_ppm(char *filename, img_t *img, enum PPM_TYPE type) {
    FILE *f = fopen(filename, "w");
    if (!f) return false;

    if (type == PPM_RAW) {
        fprintf(f, "%s\n%d %d\n255\n", "P6", img->width, img->height);
        // Write image content
        for (int i = 0; i < img->width * img->height; i++) {
            pixel_t *p = &img->pix1d[i];
            fwrite(&p->r, sizeof(p->r), 1, f);
            fwrite(&p->g, sizeof(p->g), 1, f);
            fwrite(&p->b, sizeof(p->b), 1, f);
        }
    }
    else {
        fprintf(f, "%s\n%d %d\n255\n", "P3", img->width, img->height);
        // Write image content
        int count = 0;
        for (int i = 0; i < img->width * img->height; i++) {
            pixel_t *p = &img->pix1d[i];
            fprintf(f, "%d %d %d ", p->r, p->g, p->b);
            if (++count % 5 == 0)  // New line every 5 pixels (max 70 characters/line)
                fprintf(f, "\n");
        }
    }

    fclose(f);
    return true;
}

/**
 * Load a 24-bit RGB PPM file (either ASCII P3 type or binary P6 type).
 * The routine takes care of allocating the memory for the image.
 * @param filename (absolute or relative path) of the image to load
 * @return a pointer to the loaded image or NULL if an error occured
 */
img_t *load_ppm(char *filename) {
    ppm_header_t *header = load_header(filename);
    if (!header) goto error1;

    // Allocate memory for image structure and image data
    img_t *img = alloc_img(header->width, header->height);
    if (!img) goto error1;

    FILE *f = fopen(filename, "r");
    if (f == NULL) goto error1;

    fseek(f, header->data_offset, SEEK_SET);

    if (header->type == PPM_ASCII) {
        // Image data in RGB order, ASCII encoded 
        for (unsigned int i = 0; i < header->width * header->height; i++) {
            unsigned int r, g, b;
            int matches = fscanf(f, "%u %u %u", &r, &g, &b);
            if (matches != 3) goto error2;
            if (r > header->maxval || g > header->maxval || b > header->maxval) goto error2;
            pixel_t p = { r, g, b };
            img->pix1d[i] = p;
        }
    }
    else {
        // Image data in RGB order, binary encoded 
        for (unsigned int i = 0; i < header->width * header->height; i++) {
            fread(&img->pix1d[i].r, 1, 1, f);
            fread(&img->pix1d[i].g, 1, 1, f);
            fread(&img->pix1d[i].b, 1, 1, f);
        }
    }

    free(header);
    fclose(f);
    return img;

error2:
    fclose(f);

error1:
    free(header);
    return NULL;
}

// ====================================================================================================
// Private functions
// ====================================================================================================

// Read a line (reads up to max_line_length chars) while skipping comments (lines starting with '#').
static void readline(FILE *f, char *line, int max_line_length) {
    static int line_nb = 0;
    while (1) {
        line_nb++;
        char fmt[16];
        sprintf(fmt, "%%%d[^\n]", max_line_length);
        fscanf(f, fmt, line);
        fscanf(f, "\n");
        if (line[0] != '#') break;
    }
}

// Parse a PPM header.
static ppm_header_t *load_header(char *filename) {
    FILE *f = fopen(filename, "r");
    if (f == NULL) return NULL;

    ppm_header_t *header = calloc(1, sizeof(ppm_header_t));

    const int MAX_LENGTH = 1024;
    char line[MAX_LENGTH+1];

    // PPM file type: either P3 or P6
    readline(f, line, MAX_LENGTH);
    if (strcmp("P3", line) == 0) {
        header->type = PPM_ASCII;
    }
    else if (strcmp("P6", line) == 0) {
        header->type = PPM_RAW;
    }
    else {
        fprintf(stderr, "PPM reader: unsupported format!\n");
        goto error;
    }

    // Image width and height
    readline(f, line, MAX_LENGTH);
    int matches = sscanf(line, "%u %u", &header->width, &header->height);
    if (matches != 2) goto error;

    // Maximum value per component
    readline(f, line, MAX_LENGTH);
    matches = sscanf(line, "%u ", &header->maxval);
    if (matches != 1) goto error;
    if (header->maxval > 255) {
        fprintf(stderr, "PPM reader: doesn't support more than 1 byte per component!\n");
        goto error;
    }

    header->data_offset = ftell(f);
    fclose(f);
    return header;

error:
    free(header);
    fclose(f);
    return NULL;
}
