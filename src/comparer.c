#include <stdio.h>
#include "libs/Lev0nids_bmp_library.h"

//Название bmp для сравнения вводятся через аргументы командной строки
int main(int argc, char** argv) {
    if(argc != 3) {
        printf("Incorrect amount of params");
        return -1;
    }

    int comparison_result = compare_BMP(argv[1], argv[2]);

    if(comparison_result == -1) {
        if(LAST_ERROR == FILE_NOT_FOUND) {
            printf("First file was not found.");
        } else if(LAST_ERROR == INCORRECT_FORMAT) {
            printf("First file's format is incorrect.");
        } else if(LAST_ERROR == INCORRECT_SIZE) {
            printf("First file's size is incorrect.");
        } else if(LAST_ERROR == BROKEN_FILE) {
            printf("First file is broken.");
        } else if(LAST_ERROR == READING_ERROR) {
            printf("Cannot read first file.");
        }
        return -1;
    } else if(comparison_result == -2) {
        if(LAST_ERROR == FILE_NOT_FOUND) {
            printf("Second file was not found.");
        } else if(LAST_ERROR == INCORRECT_FORMAT) {
            printf("Second file's format is incorrect.");
        } else if(LAST_ERROR == INCORRECT_SIZE) {
            printf("Second file's size is incorrect.");
        } else if(LAST_ERROR == BROKEN_FILE) {
            printf("Second file is broken.");
        } else if(LAST_ERROR == READING_ERROR) {
            printf("Cannot read second file.");
        }
        return -1;
    } else if(comparison_result == -3) {
        printf("It is not possible to compare files.");
        return -1;
    } else {
        return comparison_result;
    }
}
