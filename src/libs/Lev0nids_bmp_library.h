//
// Created by Lev0nid on 27.10.2020.
//
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>

#ifndef BMP_INVERTER_BMP_CONVERTER_H
#define BMP_INVERTER_BMP_CONVERTER_H

#define SIZE_OF_PALETTE_8bpp 256 * 4

enum BMP_ERROR {
    NO_ERROR,
    FILE_NOT_FOUND,
    INCORRECT_FORMAT,
    INCORRECT_SIZE,
    BROKEN_FILE,
    NULL_BMP,
    READING_ERROR
};

enum BMP_ERROR LAST_ERROR;

struct MY_BMP_HEADER {
    unsigned short type;
    unsigned int size;
    unsigned int pixel_array_offset;
    int width;
    int height;
    unsigned short bits_per_pixel;
    unsigned int image_size;
    unsigned int horizontal_resolution;
    unsigned int vertical_resolution;
    unsigned int number_of_colors;
    unsigned int number_of_important_colors;
};

struct MY_BMP {
    struct MY_BMP_HEADER *header;
    unsigned char* color_palette;
    unsigned char* pixel_array;
};

typedef struct MY_BMP MY_BMP;
typedef struct MY_BMP_HEADER MY_BMP_HEADER;

MY_BMP* read_BMP(char* name_of_file);
int compare_BMP(char* first_bmp_name, char* second_bmp_name);
void save_BMP(MY_BMP* bmp, char* name_of_file);
void make_BMP_negative(MY_BMP* bmp);
void free_BMP(MY_BMP* bmp);

#endif //BMP_INVERTER_BMP_CONVERTER_H