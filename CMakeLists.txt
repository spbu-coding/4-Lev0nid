cmake_minimum_required(VERSION 3.7)
project(tests LANGUAGES C)

set(CMAKE_C_STANDARD 99)

add_executable(converter src/converter.c src/libs/Lev0nids_bmp_library.c src/lib/qdbmp.c)
add_executable(comparer src/converter.c src/libs/Lev0nids_bmp_library.c)