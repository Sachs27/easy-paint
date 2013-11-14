#include <GLFW/glfw3.h>

#include <sf/utils.h>

#include "../input_manager.h"


static struct input_manager *input_manager = NULL;

static void handle_mouse_pos(struct input_manager *im,
                             double xpos, double ypos) {
    im->mouse.x = (int) xpos;
    im->mouse.y = (int) ypos;
}

static void on_mouse_pos(GLFWwindow *handle, double xpos, double ypos) {
    handle_mouse_pos(input_manager, xpos, ypos);
}

static void handle_mouse_button(struct input_manager *im,
                                int button, int action, int mods) {
    if (action == GLFW_REPEAT) {
        return;
    }

    switch (button) {
    case GLFW_MOUSE_BUTTON_LEFT:
        im->keys[KEY_MB_LEFT] = action;
        break;
    case GLFW_MOUSE_BUTTON_RIGHT:
        im->keys[KEY_MB_RIGHT] = action;
        break;
    case GLFW_MOUSE_BUTTON_MIDDLE:
        im->keys[KEY_MB_MIDDLE] = action;
        break;
    }
}

static void on_mouse_button(GLFWwindow *handle,
                            int button, int action, int mods) {
    handle_mouse_button(input_manager, button, action, mods);
}

static void handle_key(struct input_manager *im, int key, int scancode,
                       int action, int mods) {
    if (action == GLFW_REPEAT) {
        return;
    }

    switch (key) {
    case GLFW_KEY_H:
        im->keys[KEY_H] = action;
        break;
    case GLFW_KEY_UP:
        im->keys[KEY_UP] = action;
        break;
    case GLFW_KEY_DOWN:
        im->keys[KEY_DOWN] = action;
        break;
    case GLFW_KEY_LEFT:
        im->keys[KEY_LEFT] = action;
        break;
    case GLFW_KEY_RIGHT:
        im->keys[KEY_RIGHT] = action;
        break;
    case GLFW_KEY_ESCAPE:
        im->keys[KEY_ESC] = action;
        break;
    case GLFW_KEY_0:
        im->keys[KEY_0] = action;
        break;
    case GLFW_KEY_1:
        im->keys[KEY_1] = action;
        break;
    case GLFW_KEY_2:
        im->keys[KEY_2] = action;
        break;
    case GLFW_KEY_3:
        im->keys[KEY_3] = action;
        break;
    }
}

static void on_key(GLFWwindow *handle, int key, int scancode,
                   int action, int mods) {
        handle_key(input_manager, key, scancode, action, mods);
}

struct input_manager *input_manager_create(struct window *win) {
    if (input_manager) {
        sf_free(input_manager);
    }

    input_manager = sf_calloc(sizeof(*input_manager));
    input_manager->win = win;

    glfwSetMouseButtonCallback(win->handle, on_mouse_button);
    glfwSetCursorPosCallback(win->handle, on_mouse_pos);
    glfwSetKeyCallback(win->handle, on_key);

    return input_manager;
}

void input_manager_update(void) {
    struct input_manager *im = input_manager;
    int k;

    for (k = 0; k < NKEYS; ++k) {
        if (im->last_keys[k] == KEY_PRESS) {
            im->keys[k] = KEY_REPEAT;
        }
        im->last_keys[k] = im->keys[k];
    }
}
