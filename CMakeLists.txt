project(easy_paint)

cmake_minimum_required(VERSION 2.8)

add_definitions(-g -Wall -DGLEW_STATIC)

aux_source_directory(src EASY_PAINT_SRC)

add_executable(easy_paint ${EASY_PAINT_SRC})
set_target_properties(easy_paint PROPERTIES RUNTIME_OUTPUT_DIRECTORY
                      ${PROJECT_SOURCE_DIR}/bin)
target_link_libraries(easy_paint sf glfw3 glew32s glu32 opengl32 DevIL)
