#include <assert.h>
#include <stdlib.h>

#include <sf/utils.h>

#include "resmgr.h"

#include "texture.h"
#include "record.h"


struct texture_slot {
    char *name;
    struct texture texture;
};

static void texture_slot_free(void *elt)
{
    struct texture_slot *ts = elt;

    texture_destroy(&ts->texture);
}

struct record_slot {
    char *name;
    struct record record;
};

static void record_slot_free(void *elt)
{
    struct record_slot *rs = elt;

    record_destroy(&rs->record);
}

static struct resmgr {
    char *res_path;
    char *save_path;

    sf_list_t textures;     /* elt: struct texture_slot */
    sf_list_t records;      /* elt: struct record_slot */

    sf_list_t user_define_records;

    sf_pool_t str_pool;
} rm;

int rm_init(const char *res_path, const char *save_path)
{
    char buf[PATH_MAX];
    int ret = SF_OK;
    sf_list_def_t def;

    if ((ret = fs_init(1, (char **) &res_path)) != SF_OK) {
        return ret;
    }

    sf_pool_init(&rm.str_pool, 0);

    fs_cwd(buf, PATH_MAX);
    rm.res_path = sf_pool_alloc(&rm.str_pool, strlen(buf) + 1);
    strcpy(rm.res_path, buf);
#if 0
    if ((ret = fs_cd(save_path)) != SF_OK) {
        return ret;
    }
#endif
    fs_cwd(buf, PATH_MAX);
    rm.save_path = sf_pool_alloc(&rm.str_pool, strlen(buf) + 1);
    strcpy(rm.save_path, buf);

    sf_memzero(&def, sizeof(def));
    def.size = sizeof(struct texture_slot);
    def.free = texture_slot_free;
    sf_list_init(&rm.textures, &def);

    def.size = sizeof(struct record_slot);
    def.free = record_slot_free;
    sf_list_init(&rm.records, &def);
    sf_list_init(&rm.user_define_records, &def);

    return SF_OK;
}

void rm_term(void)
{
    sf_list_destroy(&rm.textures);
    sf_list_destroy(&rm.records);
    sf_list_destroy(&rm.user_define_records);
    sf_pool_destroy(&rm.str_pool);
    fs_term();
}

static struct texture_slot *find_texture(const char *filename)
{
    sf_list_iter_t iter;

    if (sf_list_begin(&rm.textures, &iter)) do {
        struct texture_slot *ts = sf_list_iter_elt(&iter);

        if (strcmp(ts->name, filename) == 0) {
            return ts;
        }
    } while (sf_list_iter_next(&iter));

    return NULL;
}

struct texture *rm_load_texture(const char *filename)
{
    char buf[PATH_MAX];
    char *ch;
    struct texture_slot *ptr = find_texture(filename);
    struct texture_slot ts;

    if (ptr) {
        return &ptr->texture;
    }

    ts.name = sf_pool_alloc(&rm.str_pool, strlen(filename) + 1);
    strcpy(ts.name, filename);

    strcpy(buf, rm.res_path);
    strcat(buf, filename);
    ch = strrchr(buf, '/');
    *ch = '\0';
    fs_cd(buf);
    if (texture_load_2d(&ts.texture, ch + 1) != SF_OK) {
        abort();
    }

    ptr = sf_list_push(&rm.textures, &ts);

    assert(find_texture(filename) != NULL);

    return &ptr->texture;
}

static struct record_slot *find_record(const char *filename)
{
    sf_list_iter_t iter;

    if (sf_list_begin(&rm.records, &iter)) do {
        struct record_slot *rs = sf_list_iter_elt(&iter);

        if (strcmp(rs->name, filename) == 0) {
            return rs;
        }
    } while (sf_list_iter_next(&iter));

    return NULL;
}

struct record *rm_load_record(const char *filename)
{
    return NULL;
}

#define RM_LAST_RECORD_FILENAME "LastRecord.epr"

struct record *rm_load_last_record(void)
{
    struct record_slot *ptr;
    struct record_slot rs;

    if ((ptr = find_record(RM_LAST_RECORD_FILENAME))) {
        return &ptr->record;
    }

    ptr = sf_list_push(&rm.records, &rs);

    ptr->name = sf_pool_alloc(&rm.str_pool, strlen(RM_LAST_RECORD_FILENAME) + 1);
    strcpy(ptr->name, RM_LAST_RECORD_FILENAME);

    fs_cd(rm.save_path);
    if (record_load(&ptr->record, RM_LAST_RECORD_FILENAME) != SF_OK) {
        record_init(&ptr->record);
    }

    assert(find_record(RM_LAST_RECORD_FILENAME) != NULL);

    return &ptr->record;
}

int rm_save_last_record(void)
{
    struct record_slot *ptr;

    ptr = find_record(RM_LAST_RECORD_FILENAME);
    assert(ptr != NULL);

    fs_cd(rm.save_path);

    return record_save(&ptr->record, RM_LAST_RECORD_FILENAME);
}
