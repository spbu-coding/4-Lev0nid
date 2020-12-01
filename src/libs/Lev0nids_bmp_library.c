#include "Lev0nids_bmp_library.h"

enum BMP_ERROR LAST_ERROR = NO_ERROR;

//Reads a little-endian unsigned int
static int read_uint(unsigned int* x, FILE* file) {
    unsigned char input[4];
    if(fread(input, 4, 1, file) != 1) {
        if(feof(file) != 0) {
            LAST_ERROR = BROKEN_FILE;
        } else {
            LAST_ERROR = READING_ERROR;
        }
        return 0;
    }
    *x = (input[3] << 24 | input[2] << 16 | input[1] << 8 | input[0]);
    return 1;
}

//Reads a little-endian unsigned short
static int read_ushort(unsigned short* x, FILE* file) {
    unsigned char input[2];
    if(fread(input, 2, 1, file) != 1) {
        if(feof(file) != 0) {
            LAST_ERROR = BROKEN_FILE;
        } else {
            LAST_ERROR = READING_ERROR;
        }
        return 0;
    }
    *x = (input[1] << 8 | input[0]);
    return 1;
}

//Reads a little-endian int
static int read_int(int* x, FILE* file) {
    char input[4];
    if(fread(input, 4, 1, file) != 1) {
        if(feof(file) != 0) {
            LAST_ERROR = BROKEN_FILE;
        } else {
            LAST_ERROR = READING_ERROR;
        }
        return 0;
    }
    *x = (input[3] << 24 | input[2] << 16 | input[1] << 8 | input[0]);
    return 1;
}
//Writes a little-endian unsigned int
static int write_uint(unsigned int x, FILE* file) {
    unsigned char output[4];
    output[3] = (unsigned char)((x & 0xff000000) >> 24);
    output[2] = (unsigned char)((x & 0x00ff0000) >> 16);
    output[1] = (unsigned char)((x & 0x0000ff00) >> 8);
    output[0] = (unsigned char)((x & 0x000000ff) >> 0);

    return (file && fwrite(output, 4, 1, file) == 1);
}

//Reads a little-endian int
static int write_int(int x, FILE* file) {
    char output[4];
    output[3] = (char)((x & 0xff000000) >> 24);
    output[2] = (char)((x & 0x00ff0000) >> 16);
    output[1] = (char)((x & 0x0000ff00) >> 8);
    output[0] = (char)((x & 0x000000ff) >> 0);

    return (file && fwrite(output, 4, 1, file) == 1);
}

//Reads a little-endian unsigned short
static int write_ushort(unsigned short x, FILE* file) {
    char output[2];
    output[1] = (unsigned char)((x & 0xff00) >> 8);
    output[0] = (unsigned char)((x & 0x00ff) >> 0);
    return (file && fwrite(output, 2, 1, file) == 1);
}

static bool check_type(unsigned short type) {
    char possible_values[6][2] = {"BM", "BA", "CI", "CP", "IC", "PT"};
    for(unsigned int i = 0; i < 6; i++) {
        if(memcmp(&type, possible_values, 2) == 0) return true;
    }
    return false;
}

static MY_BMP_HEADER* get_header(FILE* file) {
    MY_BMP_HEADER* bmp_header = (MY_BMP_HEADER*)malloc(sizeof(MY_BMP_HEADER));
    if(bmp_header == NULL) {
        return NULL;
    }

    unsigned int calculated_size;
    fseek(file, 0, SEEK_END);
    calculated_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if(read_ushort(&bmp_header->type, file) == 0) return NULL;
    if(!check_type(bmp_header->type)) {
        LAST_ERROR = INCORRECT_FORMAT;
        return NULL;
    }

    if(read_uint(&bmp_header->size, file) == 0) return NULL;
    if(bmp_header->size != calculated_size) {
        LAST_ERROR = INCORRECT_SIZE;
        return NULL;
    }

    unsigned int reserved;
    if(read_uint(&reserved, file) == 0) return NULL;
    if(reserved != 0) {
        LAST_ERROR = BROKEN_FILE;
        return NULL;
    }

    if(read_uint(&bmp_header->pixel_array_offset, file) == 0) return NULL;

    unsigned bmp_version;
    if(read_uint(&bmp_version, file) == 0) return NULL;
    if(bmp_version != 40) {
        LAST_ERROR = INCORRECT_FORMAT;
        return NULL;
    }

    if(read_int(&bmp_header->width, file) == 0) return NULL;
    if(read_int(&bmp_header->height, file) == 0) return NULL;

    unsigned short number_of_color_planes;
    if(read_ushort(&number_of_color_planes, file) == 0) return NULL;
    if(number_of_color_planes != 1) {
        LAST_ERROR = BROKEN_FILE;
        return NULL;
    }

    if(read_ushort(&bmp_header->bits_per_pixel, file) == 0) return NULL;
    if(bmp_header->bits_per_pixel != 8 && bmp_header->bits_per_pixel != 24) {
        LAST_ERROR = INCORRECT_FORMAT;
        return NULL;
    }

    unsigned int compression_method;
    if(read_uint(&compression_method, file) == 0) return NULL;
    if(compression_method != 0) {
        LAST_ERROR = INCORRECT_FORMAT;
        return NULL;
    }

    if(read_uint(&bmp_header->image_size, file) == 0) return NULL;
    if(bmp_header->image_size != 0) {
        unsigned int calculated_image_size = bmp_header->size - bmp_header->pixel_array_offset;
        if(calculated_image_size != bmp_header->image_size) {
            LAST_ERROR = INCORRECT_SIZE;
            return NULL;
        }
    }

    if(read_uint(&bmp_header->horizontal_resolution, file) == 0) return NULL;
    if(read_uint(&bmp_header->vertical_resolution, file) == 0) return NULL;
    if(read_uint(&bmp_header->number_of_colors, file) == 0) return NULL;
    if(read_uint(&bmp_header->number_of_important_colors, file) == 0) return NULL;
    return bmp_header;
}

static void get_pixel_color(MY_BMP* bmp, int x, int y, unsigned char* r, unsigned char* g, unsigned char* b) {
    unsigned char* pixel;
    unsigned int bytes_per_row = bmp->header->image_size / abs(bmp->header->height);
    unsigned int bytes_per_pixel;
    if(bmp->header->bits_per_pixel == 8) {
        bytes_per_pixel = 1;
    } else {
        bytes_per_pixel = 3;
    }
    pixel = bmp->pixel_array + ((abs(bmp->header->height) - y - 1) * bytes_per_row + x * bytes_per_pixel);
    if(bmp->color_palette != NULL) { //Если bmp 8 бит, то надо в палитру посмотреть
        pixel = bmp->color_palette + *pixel * 4;
    }
    *r = pixel[2];
    *g = pixel[1];
    *b = pixel[0];
}

static void set_pixel_color(MY_BMP* bmp, int x, int y, unsigned char r, unsigned char g, unsigned char b) {
    unsigned char* pixel;
    unsigned int bytes_per_row = bmp->header->image_size / abs(bmp->header->height);
    unsigned int bytes_per_pixel = 3;
    pixel = bmp->pixel_array + ((abs(bmp->header->height) - y - 1) * bytes_per_row + x * bytes_per_pixel);
    pixel[2] = r;
    pixel[1] = g;
    pixel[0] = b;
}

void free_BMP(MY_BMP* bmp) {
    if(bmp->color_palette != NULL)
        free(bmp->color_palette);
    if(bmp->pixel_array != NULL)
        free(bmp->pixel_array);
    free(bmp);
}

void make_BMP_negative(MY_BMP* bmp) {
    if(bmp == NULL) {
        LAST_ERROR = NULL_BMP;
        return;
    }

    if(bmp->header->bits_per_pixel == 8) {
        for(unsigned int i = 0; i < 256; i++) {
            bmp->color_palette[4 * i] = (~bmp->color_palette[4 * i]);
            bmp->color_palette[4 * i + 1] = (~bmp->color_palette[4 * i + 1]);
            bmp->color_palette[4 * i + 2] = (~bmp->color_palette[4 * i + 2]);
        }
    } else if(bmp->header->bits_per_pixel == 24) {
        unsigned char r, g, b;
        for (int x = 0; x < abs(bmp->header->width); x++) {
            for (int y = 0; y < abs(bmp->header->height); y++) {
                get_pixel_color(bmp, x, y, &r, &g, &b);
                set_pixel_color(bmp, x, y, ~r, ~g, ~b);
            }
        }
    } else {
        LAST_ERROR = INCORRECT_FORMAT;
    }
}

MY_BMP* read_BMP(char* name_of_file) {
    MY_BMP* bmp;
    FILE* file;

    if(name_of_file == NULL) {
        LAST_ERROR = FILE_NOT_FOUND;
        return NULL;
    }

    bmp = (MY_BMP*)malloc(sizeof(MY_BMP));
    if(bmp == NULL) {
        free_BMP(bmp);
        return NULL;
    }
    bmp->pixel_array = NULL;
    bmp->color_palette = NULL;

    file = fopen(name_of_file, "rb");
    if(file == NULL) {
        LAST_ERROR = FILE_NOT_FOUND;
        return NULL;
    }

    bmp->header = get_header(file);
    if(LAST_ERROR != NO_ERROR) {
        free_BMP(bmp);
        return NULL;
    }

    if(bmp->header->bits_per_pixel == 8) {
        unsigned int size_of_palette = SIZE_OF_PALETTE_8bpp;
        bmp->color_palette = (unsigned char*)malloc(size_of_palette * sizeof(unsigned char));
        if(bmp->color_palette == NULL) {
            free_BMP(bmp);
            return NULL;
        }
        if(fread(bmp->color_palette, sizeof(unsigned char), size_of_palette, file) != size_of_palette) {
            if(feof(file) != 0) {
                LAST_ERROR = BROKEN_FILE;
            } else {
                LAST_ERROR = READING_ERROR;
            }
            free_BMP(bmp);
            return NULL;
        }
    } else {
        bmp->color_palette = NULL;
    }

    bmp->pixel_array = (unsigned char*)malloc(bmp->header->image_size);
    if(bmp->pixel_array == NULL) {
        free_BMP(bmp);
        return NULL;
    }
    if(fread(bmp->pixel_array, sizeof(unsigned char), bmp->header->image_size, file) != bmp->header->image_size) {
        if(feof(file) != 0) {
            LAST_ERROR = BROKEN_FILE;
        } else {
            LAST_ERROR = READING_ERROR;
        }
        free_BMP(bmp);
        return NULL;
    }

    fclose(file);
    return bmp;
}

int compare_BMP(char* first_bmp_name, char* second_bmp_name) {
    MY_BMP* first_bmp = read_BMP(first_bmp_name);
    if(first_bmp == NULL) {
        return -1;
    }

    MY_BMP* second_bmp = read_BMP(second_bmp_name);
    if(second_bmp == NULL) {
        return -2;
    }

    if(first_bmp->header->width != second_bmp->header->width|| //bmp с разными знакаами высоты или ширины несравнимы
    first_bmp->header->height != second_bmp->header->height ||
    first_bmp->header->bits_per_pixel != second_bmp->header->bits_per_pixel) {
        free_BMP(first_bmp);
        free_BMP(second_bmp);
        return -3;
    }

    int amount_of_different_points = 0;
    for(int x = 0; x < abs(first_bmp->header->width) && amount_of_different_points < 100; x++) {
        for(int y = 0; y < abs(second_bmp->header->height) && amount_of_different_points < 100; y++) {
            unsigned char first_r, first_g, first_b, second_r, second_g, second_b;
            get_pixel_color(first_bmp, x, y, &first_r, &first_g, &first_b);
            get_pixel_color(second_bmp, x, y, &second_r, &second_g, &second_b);
            if(first_r != second_r || first_g != second_g || first_b != second_b) {
                amount_of_different_points++;
                fprintf(stderr, "%d %d\n", x, y);
            }
        }
    }
    free_BMP(first_bmp);
    free_BMP(second_bmp);
    return amount_of_different_points;
}

void save_BMP(MY_BMP* bmp, char* name_of_file) {
    FILE* file = fopen(name_of_file, "wb");
    write_ushort(bmp->header->type, file);
    write_uint(bmp->header->size, file);
    unsigned int reserved = 0;
    write_uint(reserved, file);
    write_uint(bmp->header->pixel_array_offset, file);
    unsigned int bmp_version = 40;
    write_uint(bmp_version, file);
    write_int(bmp->header->width, file);
    write_int(bmp->header->height, file);
    unsigned short number_of_color_planes = 1;
    write_ushort(number_of_color_planes, file);
    write_ushort(bmp->header->bits_per_pixel, file);
    unsigned int compression_method = 0;
    write_uint(compression_method, file);
    write_uint(bmp->header->image_size, file);
    write_uint(bmp->header->horizontal_resolution, file);
    write_uint(bmp->header->vertical_resolution, file);
    write_uint(bmp->header->number_of_colors, file);
    write_uint(bmp->header->number_of_important_colors, file);

    if(bmp->header->bits_per_pixel == 8)
        fwrite(bmp->color_palette, sizeof(unsigned char), SIZE_OF_PALETTE_8bpp, file);

    fwrite(bmp->pixel_array, sizeof(unsigned char), bmp->header->image_size, file);
    fclose(file);
}