#include <GLFW/glfw3.h>

#include <sf/utils.h>
#include <sf/log.h>

#include "../input_manager.h"


static void handle_mouse_pos(struct input_manager *im,
                             double xpos, double ypos) {
    int x = xpos;
    int y = ypos;

    if (im->keys[KEY_MB_LEFT] == KEY_PRESS
        && (x != im->mouse.x || y != im->mouse.y)) {
        input_manager_touch_down(x, y);
    }

    im->mouse.x = x;
    im->mouse.y = y;
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
        if (action == GLFW_PRESS) {
            input_manager_touch_down(im->mouse.x, im->mouse.y);
        } else if (action == GLFW_RELEASE) {
            input_manager_touch_up();
        }
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
    sf_list_def_t def;

    if (input_manager) {
        sf_free(input_manager);
        input_manager = NULL;
    }

    input_manager = sf_calloc(sizeof(*input_manager));
    sf_memzero(&def, sizeof(def));
    def.size = sizeof(struct ivec2);
    sf_list_init(&input_manager->mb_left_buffer, &def);
    input_manager->win            = win;

    glfwSetMouseButtonCallback(win->handle, on_mouse_button);
    glfwSetCursorPosCallback(win->handle, on_mouse_pos);
    glfwSetKeyCallback(win->handle, on_key);

    return input_manager;
}

void input_manager_destroy(void) {
    struct window *win;

    if (input_manager == NULL) {
        return;
    }

    sf_list_destroy(&input_manager->mb_left_buffer);

    win = input_manager->win;
    glfwSetMouseButtonCallback(win->handle, 0);
    glfwSetCursorPosCallback(win->handle, 0);
    glfwSetKeyCallback(win->handle, 0);
    sf_free(input_manager);
    input_manager = NULL;
}
