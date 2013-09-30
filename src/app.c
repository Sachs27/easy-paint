#include <stdio.h>
#include <unistd.h>

#include "app.h"


struct app g_app;

static char *WINDOW_TITLE = "Easy Paint";
static int WINDOW_WIDTH = 800;
static int WINDOW_HEIGHT = 480;

void init(void);
void update(double dt);
void render(void);


static int app_init(int argc, char *argv[]) {
    GLenum err;

    g_app.init_cb = init;
    g_app.update_cb = update;
    g_app.render_cb = render;

    if (sf_init(argc, argv)) {
        return -1;
    }
#if 0
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif
    g_app.window = window_create(WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT);
    if (!g_app.window) {
        glfwTerminate();
        return -1;
    }
    fprintf(stdout, "OpenGL Version: %s\n", glGetString(GL_VERSION));
    /* GLEW has a problem with core contexts.
     * It calls glGetString(GL_EXTENSIONS), which causes GL_INVALID_ENUM
     * on GL 3.2+ core context as soon as glewInit() is called.
     * It also doesn't fetch the function pointers.  */
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        return -1;
    }
    while ((err = glGetError()) != GL_NO_ERROR);

    g_app.im = input_manager_create(g_app.window);

    g_app.init_cb();

    return 0;
}

int main(int argc, char *argv[]) {
    if (app_init(argc, argv) != 0) {
        return -1;
    }

    while (window_isopen(g_app.window)) {
        input_manager_update(g_app.im);
        g_app.update_cb(0.0f);
        g_app.render_cb();
        glfwSwapBuffers(g_app.window->handle);
        glfwPollEvents();
    }

    glfwTerminate();

    return 0;
}
