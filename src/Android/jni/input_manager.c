#include <stdlib.h>

#include <sf/utils.h>
#include <sf/log.h>

#include "../../input_manager.h"


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

    return input_manager;
}

void input_manager_destroy(void) {
    if (input_manager == NULL) {
        return;
    }

    sf_list_destroy(&input_manager->mb_left_buffer);
    sf_free(input_manager);
    input_manager = NULL;
}
