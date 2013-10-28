project(libzip)

cmake_minimum_required(VERSION 2.8)

aux_source_directory(./ SRC)
add_library(zip ${SRC})
target_link_libraries(zip z)
