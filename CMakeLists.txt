project(easy_paint)

cmake_minimum_required(VERSION 2.8)

add_definitions(-g -Wall -DGLEW_STATIC)

aux_source_directory(src EASY_PAINT_COMMON_SRC)
    aux_source_directory(src/windows EASY_PATIN_PLATFORM_SRC)

add_executable(easy_paint ${EASY_PAINT_COMMON_SRC} ${EASY_PATIN_PLATFORM_SRC})
set_target_properties(easy_paint PROPERTIES RUNTIME_OUTPUT_DIRECTORY
                      ${PROJECT_SOURCE_DIR}/bin)

if (WIN32)
    set(LIBS sf glfw3 glew32s glu32 opengl32 DevIL png zip z)
else (WIN32)
    set(LIBS sf m glfw3 GLEW GL png zip z X11 Xrandr Xi Xxf86vm pthread)
endif (WIN32)


target_link_libraries(easy_paint ${LIBS})
