#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <sf_utils.h>

#ifndef PATH_MAX
#define PATH_MAX 1024
#endif

#include "../app.h"
#include "../window.h"
#include "../input_manager.h"
#include "../renderer2d.h"
#include "../ui.h"

static void on_glfw_error(int error, const char *desc) {
    return (void) fputs(desc, stderr);
}

static int glfw_init(void) {
    glfwSetErrorCallback(on_glfw_error);

    if (!glfwInit()) {
        return -1;
    }

    return 0;
}

static int change_working_directory(const char *pathname) {
#if defined(__WIN32__)
    static char path_seperator = '\\';
#else
    static char path_seperator = '/';
#endif
    char path[PATH_MAX];
    char *ptr;

    strncpy(path, pathname, PATH_MAX);
    ptr = strrchr(path, path_seperator);
    if (ptr != NULL) {
        *ptr = '\0';
        if (chdir(path) < 0) {
            return -1;
        }
    }
    fprintf(stdout, "Current working directory: %s\n", path);
    return 0;
}

static int init(int argc, char *argv[]) {
    GLenum err;

    if (change_working_directory(argv[0]) != 0) {
        fprintf(stderr, "Failed to change working directoyr.\n");
        return -1;
    }

    if (glfw_init() != 0) {
        return -1;
    }

    g_app.window = window_create(WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT);
    if (!g_app.window) {
        glfwTerminate();
        return -1;
    }

    window_on_resize(app_on_resize);

    fprintf(stdout, "OpenGL Version: %s\n", glGetString(GL_VERSION));

    /*
     * GLEW has a problem with core contexts.
     * It calls glGetString(GL_EXTENSIONS), which causes GL_INVALID_ENUM
     * on GL 3.2+ core context as soon as glewInit() is called.
     * It also doesn't fetch the function pointers.
     */
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        return -1;
    }
    while ((err = glGetError()) != GL_NO_ERROR);

    g_app.im = input_manager_create(g_app.window);
    return 0;
}

int main(int argc, char *argv[]) {
    uint64_t cur_tick, last_tick;

    if (init(argc, argv) != 0) {
        return -1;
    }

    app_init();

    cur_tick = sf_get_ticks();
    while (window_isopen()) {
        last_tick = cur_tick;
        cur_tick = sf_get_ticks();

        input_manager_update();

        app_on_update((cur_tick - last_tick) * 1.0 / 1E9);

        app_on_render();

        glfwSwapBuffers(g_app.window->handle);
        glfwPollEvents();
    }

    glfwTerminate();

    return 0;
}
