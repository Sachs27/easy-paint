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
    int   isinited;
    struct record record;
};

static void record_slot_free(void *elt)
{
    struct record_slot *rs = elt;

    if (rs->isinited) {
        record_destroy(&rs->record);
    }
}

static struct resmgr {
    char *res_path;
    char *save_path;

    sf_list_t textures;     /* elt: struct texture_slot */

    int       w, h;
    sf_list_t records;      /* elt: struct record_slot */

    sf_list_t user_define_records;

    sf_pool_t str_pool;
} rm;

int rm_init(const char *res_path, const char *save_path)
{
    char origin_path[PATH_MAX];
    char buf[PATH_MAX];
    sf_list_def_t def;

    fs_cwd(origin_path, PATH_MAX);

    if (!fs_is_file_exist(res_path)) {
        fs_mkdir(res_path);
    }
    fs_cd(res_path);

    sf_pool_init(&rm.str_pool, 0);

    fs_cwd(buf, PATH_MAX);
    rm.res_path = sf_pool_alloc(&rm.str_pool, strlen(buf) + 1);
    strcpy(rm.res_path, buf);

    fs_cd(origin_path);

    if (!fs_is_file_exist(save_path)) {
        fs_mkdir(save_path);
    }
    fs_cd(save_path);

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

    rm.w = 0;
    rm.h = 0;

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

int rm_set_record_size(int w, int h)
{
    rm.w = w;
    rm.h = h;
    return SF_OK;
}

static struct record_slot *find_record_by_ref(struct record *r)
{
    sf_list_iter_t iter;

    if (sf_list_begin(&rm.records, &iter)) do {
        struct record_slot *rs = sf_list_iter_elt(&iter);

        if (&rs->record == r) {
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
    struct record *r;
    struct record_slot *ptr;
    struct record_slot rs;

    if ((r = rm_load_user_define_record(RM_LAST_RECORD_FILENAME))) {
        return r;
    }

    ptr = sf_list_push(&rm.records, &rs);

    ptr->name = sf_pool_alloc(&rm.str_pool, strlen(RM_LAST_RECORD_FILENAME) + 1);
    strcpy(ptr->name, RM_LAST_RECORD_FILENAME);

    fs_cd(rm.save_path);
    record_init(&ptr->record, rm.w, rm.h);
    ptr->isinited = 1;

    assert(find_record(RM_LAST_RECORD_FILENAME) != NULL);

    return &ptr->record;
}

int rm_save_last_record(void)
{
    struct record_slot *ptr = find_record(RM_LAST_RECORD_FILENAME);

    if (ptr == NULL) {
        struct record_slot rs;

        ptr = sf_list_push(&rm.records, &rs);

        ptr->name = sf_pool_alloc(&rm.str_pool,
                                  strlen(RM_LAST_RECORD_FILENAME) + 1);
        strcpy(ptr->name, RM_LAST_RECORD_FILENAME);

        record_init(&ptr->record, rm.w, rm.h);
        ptr->isinited = 1;
    }

    return rm_save_user_define_record(&ptr->record);
}

int rm_save_user_define_record(struct record *r)
{
    struct record_slot *ptr = find_record_by_ref(r);

    assert(ptr != NULL);

    fs_cd(rm.save_path);

    return record_save(r, ptr->name);
}

int rm_save_as_user_define_record(struct record *r, const char *filename)
{
    struct record_slot *ptr = find_record_by_ref(r);

    assert(ptr != NULL);

    ptr->name = sf_pool_alloc(&rm.str_pool, strlen(filename) + 1);
    strcpy(ptr->name, filename);

    fs_cd(rm.save_path);

    return record_save(r, filename);
}

struct record *rm_load_user_define_record(const char *filename)
{
    struct record_slot *ptr;
    struct record_slot rs;

    if ((ptr = find_record(filename))) {
        return &ptr->record;
    }

    ptr = sf_list_push(&rm.records, &rs);

    ptr->name = sf_pool_alloc(&rm.str_pool, strlen(filename) + 1);
    strcpy(ptr->name, filename);

    fs_cd(rm.save_path);

    if (record_load(&ptr->record, filename) == SF_OK) {
        ptr->isinited = 1;
        return &ptr->record;
    } else {
        ptr->isinited = 0;
        sf_list_pop(&rm.records);
        return NULL;
    }
}

#define GATHER_NFILENAMES 1024
struct gather_filename_arg {
    char *filenames[GATHER_NFILENAMES];
    size_t n;
    sf_pool_t str_pool;
};

static int filename_cmp(const void *a, const void *b)
{
    char *s1 = *(char **) a;
    char *s2 = *(char **) b;

    return -strcmp(s1, s2);
}

static int gather_filename(int type, const char *filename, void *arg)
{
    struct gather_filename_arg *names = arg;

    if (type == FS_FILE && strcmp(filename, RM_LAST_RECORD_FILENAME)) {
        names->filenames[names->n] = sf_pool_alloc(&names->str_pool,
                                                   strlen(filename) + 1);
        strcpy(names->filenames[names->n], filename);
        ++names->n;

        if (names->n >= GATHER_NFILENAMES) {
            return -1;
        }
    }

    return SF_OK;
}

size_t rm_get_all_user_define_records(struct record **records, size_t len)
{
    int i;
    struct gather_filename_arg names = {{0}, 0};
    names.n = 0;

    if (len == 0) {
        return 0;
    }

    sf_pool_init(&names.str_pool, 0);

    fs_cd(rm.save_path);

    records[0] = rm_load_last_record();

    fs_file_walker(gather_filename, &names);
    qsort(names.filenames, names.n, sizeof(char *), filename_cmp);

    for (i = 0; i < names.n; ++i) {
        if (i + 1 >= len) {
            break;
        }
        records[i + 1] = rm_load_user_define_record(names.filenames[i]);
    }

    sf_pool_destroy(&names.str_pool);
    return i + 1;
}

static int count_filename(int type, const char *filename, void *arg)
{
    int *count = arg;

    if (type == FS_FILE && strcmp(filename, RM_LAST_RECORD_FILENAME)) {
        ++*count;
    }

    return SF_OK;
}

size_t rm_get_user_define_record_count(void)
{
    int count = 1;  /* for LastReocrd.epr */

    fs_cd(rm.save_path);

    fs_file_walker(count_filename, &count);

    return count;
}

int rm_del_user_define_record(const char *filename)
{
    fs_cd(rm.save_path);

    return fs_file_del(filename);
}

int rm_del_last_record(void)
{
    return rm_del_user_define_record(RM_LAST_RECORD_FILENAME);
}
