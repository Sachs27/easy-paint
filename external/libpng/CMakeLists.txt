project(libpng)

cmake_minimum_required(VERSION 2.8)

aux_source_directory(./ SRC)
add_library(png ${SRC})

