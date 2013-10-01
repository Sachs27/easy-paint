#include <string.h>
#include <unistd.h>

#include <GLFW/glfw3.h>
#include <IL/il.h>

#include "sf.h"


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

int sf_init(int argc, char *argv[]) {
    if (change_working_directory(argv[0]) != 0) {
        fprintf(stderr, "Failed to change working directoyr.\n");
        return -1;
    }

    if (glfw_init() != 0) {
        return -1;
    }

    return 0;
}
