#include <string.h>
#include <unistd.h>

#include <GLFW/glfw3.h>
#include <IL/il.h>

#include "sf.h"


enum il_init_status {
    IL_INIT_OK = 0,
    IL_INIT_FAILED,
    IL_INIT_NOT_INITED,
};

enum glfw_init_status {
    GLFW_INIT_OK = 0,
    GLFW_INIT_FAILED,
};

static enum il_init_status il_init(void) {
    ilInit();
    ilClearColor(255, 0, 0, 255);

    ILenum ilerr = ilGetError();
    if (ilerr != IL_NO_ERROR) {
        return IL_INIT_FAILED;
    }

    return IL_INIT_OK;
}

static void on_glfw_error(int error, const char *desc) {
    return (void) fputs(desc, stderr);
}

static enum glfw_init_status glfw_init(void) {
    glfwSetErrorCallback(on_glfw_error);

    if (!glfwInit()) {
        return GLFW_INIT_FAILED;
    }

    return GLFW_INIT_OK;
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
    return SF_OK;
}

int sf_init(int argc, char *argv[]) {
    if (change_working_directory(argv[0]) != SF_OK) {
        fprintf(stderr, "Failed to change working directoyr.\n");
        return -1;
    }

    if (il_init() != IL_INIT_OK) {
        fprintf(stderr, "Failed to initialize DevIL.\n");
        return -1;
    }

    if (glfw_init() != GLFW_INIT_OK) {
        return -1;
    }

    return SF_OK;
}
