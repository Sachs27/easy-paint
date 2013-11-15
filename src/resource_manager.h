#ifndef RESOURCE_MANAGER
#define RESOURCE_MANAGER


#include <zip.h>

#include "record.h"
#include "texture.h"


enum RESOURCE_TYPE {
    RESOURCE_TEXTURE,
    RESOURCE_RECORD,
};

enum RESOURCE_TEXTURE_ID {
    RESOURCE_TEXTURE_ICON_PEN,
    RESOURCE_TEXTURE_ICON_PENCIL,
    RESOURCE_TEXTURE_ICON_ERASER,
    RESOURCE_TEXTURE_ICON_UNDO,
    RESOURCE_TEXTURE_ICON_REDO,
    RESOURCE_TEXTURE_ICON_STOP,
    RESOURCE_TEXTURE_ICON_PAUSE,
    RESOURCE_TEXTURE_ICON_PLAY,
    RESOURCE_TEXTURE_ICON_FASTFORWARD,
    RESOURCE_TEXTURE_ICON_REWIND,
    RESOURCE_TEXTURE_ICON_PARENT,
    RESOURCE_TEXTURE_ICON_LOGO,
    RESOURCE_TEXTURE_ICON_LABEL1,
    RESOURCE_TEXTURE_ICON_LABEL2,
    RESOURCE_TEXTURE_ICON_LABEL3,
    RESOURCE_NTEXTURES,
};

enum RESOURCE_RECORD_ID {
    RESOURCE_RECORD_0,
    RESOURCE_RECORD_1,
    RESOURCE_RECORD_2,
    RESOURCE_RECORD_3,
    RESOURCE_RECORD_4,
    RESOURCE_NRECORDS,
};

struct resource_manager {
    const char     *root;

    int             iszip;
    struct zip     *archive;

    char            istexture_loaded[RESOURCE_NTEXTURES];
    struct texture  textures[RESOURCE_NTEXTURES];

    char            isrecord_loaded[RESOURCE_NRECORDS];
    struct record   records[RESOURCE_NRECORDS];
};


int resource_manager_init(struct resource_manager *rm, const char *pathname);

void resource_manager_destroy(struct resource_manager *rm);

void *resource_manager_load(struct resource_manager *rm, int type, int id);

void *resource_manager_get(struct resource_manager *rm, int type, int id);


#endif /* RESOURCE_MANAGER */
