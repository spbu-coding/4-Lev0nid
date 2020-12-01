#include <stdio.h>
#include <string.h>
#include "libs/Lev0nids_bmp_library.h"
#include "libs/qdbmp.h"

int convert_using_mine_library(char** argv) {
    MY_BMP* bmp;
    bmp = read_BMP(argv[2]);
    if(bmp == NULL) {
        if (LAST_ERROR == FILE_NOT_FOUND) {
            printf("File was not found.");
            return -1;
        } else if (LAST_ERROR == INCORRECT_FORMAT) {
            printf("File's format is incorrect.");
            return -2;
        } else if (LAST_ERROR == INCORRECT_SIZE) {
            printf("File's size is incorrect.");
            return -2;
        } else if (LAST_ERROR == BROKEN_FILE) {
            printf("File is broken.");
            return -1;
        } else if (LAST_ERROR == READING_ERROR) {
            printf("Cannot read the file.");
            return -1;
        }
    }
    make_BMP_negative(bmp);
    save_BMP(bmp, argv[3]);
    free_BMP(bmp);
    return 0;
}

int convert_using_their_library(char** argv) {
    BMP* bmp;
    bmp = BMP_ReadFile(argv[2]);
    BMP_CHECK_ERROR(stdout, -3);
    unsigned char r, g, b;
    if(BMP_GetDepth(bmp) == 8) {
        for(int i = 0; i < 256; i++) {
            BMP_GetPaletteColor(bmp, i, &r, &g, &b);
            BMP_SetPaletteColor(bmp, i, ~r, ~g, ~b);
        }
    } else if(BMP_GetDepth(bmp) == 24){
        for(int i = 0; i < BMP_GetWidth(bmp); i++) {
            for(int j = 0; j < BMP_GetHeight(bmp); j++) {
                BMP_GetPixelRGB(bmp, i, j, &r, &g, &b);
                BMP_SetPixelRGB(bmp, i, j, ~r, ~g, ~b);
                BMP_CHECK_ERROR(stdout, -3);
            }
        }
    } else {
        printf("File's format is incorrect.");
        return -3;
    }
    BMP_WriteFile(bmp, argv[3]);
    BMP_CHECK_ERROR(stdout, -3);
    BMP_Free(bmp);
    return 0;
}

int main(int argc, char** argv) {
    if(argc != 4) {
        printf("Incorrect amount of arguments.");
        return -1;
    }
    static const char* mine = "--mine";
    static const char* theirs = "--theirs";
    if(strcmp(argv[1], mine) == 0) {
         return convert_using_mine_library(argv);
    } else if(strcmp(argv[1], theirs) == 0) {
        return convert_using_their_library(argv);
    } else {
        printf("Incorrect command line arguments.");
        return -1;
    }
}
