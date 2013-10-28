#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "resource_manager.h"

#ifndef PATH_MAX
#define PATH_MAX 1024
#endif


static const char *texture_filenames[] = {
    [RESOURCE_TEXTURE_ICON_PEN] = "assets/icons/pen.png",
    [RESOURCE_TEXTURE_ICON_PENCIL] = "assets/icons/pencil.png",
    [RESOURCE_TEXTURE_ICON_ERASER] = "assets/icons/eraser.png",
    [RESOURCE_TEXTURE_ICON_UNDO] = "assets/icons/undo.png",
    [RESOURCE_TEXTURE_ICON_REDO] = "assets/icons/redo.png",
    [RESOURCE_TEXTURE_ICON_STOP] = "assets/icons/stop.png",
    [RESOURCE_TEXTURE_ICON_PAUSE] = "assets/icons/pause.png",
    [RESOURCE_TEXTURE_ICON_PLAY] = "assets/icons/play.png",
    [RESOURCE_TEXTURE_ICON_FASTFORWARD] = "assets/icons/fastforward.png",
    [RESOURCE_TEXTURE_ICON_REWIND] = "assets/icons/rewind.png",
    [RESOURCE_TEXTURE_ICON_PARENT] = "assets/icons/parent.png",
    [RESOURCE_TEXTURE_ICON_LOGO] = "assets/icons/logo.png",
    [RESOURCE_TEXTURE_ICON_LABEL1] = "assets/icons/label1.png",
    [RESOURCE_TEXTURE_ICON_LABEL2] = "assets/icons/label2.png",
    [RESOURCE_TEXTURE_ICON_LABEL3] = "assets/icons/label3.png",
};

static const char *record_filenames[] = {
    "assets/record/bird.rec",
    "assets/record/cake.rec",
    "assets/record/mushroom.rec",
    "assets/record/uncle-sun.rec",
    "assets/record/whale.rec",
};

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
    return 0;
}


struct resource_manager *resource_manager_create(const char *pathname) {
    struct resource_manager *rm;
    char *ptr;

    rm = calloc(1, sizeof(*rm));
    rm->root = strdup(pathname);

    ptr = strrchr(pathname, '.');
    if (ptr && (strcmp(ptr, ".zip") == 0 || strcmp(ptr, ".apk") == 0)) {
        rm->archive = zip_open(pathname, 0, NULL);
        rm->iszip = 1;
    } else {
        rm->iszip = 0;
        assert(change_working_directory(pathname) == 0);
    }

    return rm;
}

void *resource_manager_load(struct resource_manager *rm, int type, int id) {
    if (!rm->iszip) {
        change_working_directory(rm->root);
    }

    switch (type) {
    case RESOURCE_TEXTURE:
        assert(id < RESOURCE_NTEXTURES);
        if (rm->iszip) {
            if (texture_load_2d_zip(rm->textures + id,
                                rm->archive, texture_filenames[id]) == 0) {
                rm->istexture_loaded[id] = 1;
            }
        } else {
            if (texture_load_2d(rm->textures + id,
                                texture_filenames[id]) == 0) {
                rm->istexture_loaded[id] = 1;
            }
        }
        assert(rm->istexture_loaded[id]);
        return rm->textures + id;
    case RESOURCE_RECORD:
        if (rm->iszip) {
            if (record_load_zip(rm->records + id, rm->archive,
                                record_filenames[id]) == 0) {
                rm->isrecord_loaded[id] = 1;
            }
        } else {
            if (record_load(rm->records + id, record_filenames[id]) == 0) {
                rm->isrecord_loaded[id] = 1;
            }
        }
        assert(rm->isrecord_loaded[id]);
        return rm->records + id;
    }

    return NULL;
}

void *resource_manager_get(struct resource_manager *rm, int type, int id) {
    switch (type) {
    case RESOURCE_TEXTURE:
        assert(id < RESOURCE_NTEXTURES);
        if (!rm->istexture_loaded[id]) {
            resource_manager_load(rm, type, id);
        }
        return rm->textures + id;
    case RESOURCE_RECORD:
        assert(id < RESOURCE_NRECORDS);
        if (!rm->isrecord_loaded[id]) {
            resource_manager_load(rm, type, id);
        }
        return rm->records + id;
    }

    return NULL;
}
