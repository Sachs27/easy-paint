#include <stdlib.h>

#include <sf/log.h>

#include "input_manager.h"

struct input_manager *input_manager = NULL;

void input_manager_touch_down(int x, int y)
{
    struct input_manager *im = input_manager;

    if (im->keys[KEY_MB_LEFT] == KEY_RELEASE) {
        im->keys[KEY_MB_LEFT] = KEY_PRESS;
        im->is_mb_move = 0;
        im->xpressed = x;
        im->ypressed = y;
    } else if (im->keys[KEY_MB_LEFT] == KEY_PRESS
               || im->keys[KEY_MB_LEFT] == KEY_LONG_PRESS) {
        struct ivec2 pos;
        pos.x = x;
        pos.y = y;
        sf_list_push(&im->mb_left_buffer, &pos);

        if (abs(x - im->xpressed) > IM_KEY_MOVE_DELTA_PIXEL
            || abs(y - im->ypressed) > IM_KEY_MOVE_DELTA_PIXEL) {
            im->is_mb_move = 1;
        }
    }

    im->mouse.x = x;
    im->mouse.y = y;
}

void input_manager_touch_up(void)
{
    struct input_manager *im = input_manager;

    if (im->keys[KEY_MB_LEFT] == KEY_LONG_PRESS || im->is_mb_move) {
        im->keys[KEY_MB_LEFT] = KEY_RELEASE;
    } else {
        im->keys[KEY_MB_LEFT] = KEY_TAP;
    }

    im->mb_left_time = 0.0f;
}

void input_manager_update(double dt)
{
    struct input_manager *im = input_manager;

    if (im->keys[KEY_MB_LEFT] == KEY_PRESS) {
        if (!im->is_mb_move) {
            im->mb_left_time += dt;
            if (im->mb_left_time >= IM_KEY_LONG_PRESS_TIME) {
                im->keys[KEY_MB_LEFT] = KEY_LONG_PRESS;
            }
        }
    } else if (im->keys[KEY_MB_LEFT] == KEY_TAP) {
        im->keys[KEY_MB_LEFT] = KEY_RELEASE;
    }

    sf_list_clear(&im->mb_left_buffer);
}
