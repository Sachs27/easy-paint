#include <assert.h>
#include <errno.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

#include <zip.h>

#include <sf/utils.h>
#include <sf/log.h>
#include <sf/list.h>

#include "filesystem.h"

#if defined(__WIN32__)
    static char seperator = '\\';
#else
    static char seperator = '/';
#endif


struct directory {
    char name[NAME_MAX];

    int isopened;
    int iszip;
    union {
        struct zip *zip;
        DIR        *dir;
    };

    int nopens;
};

static struct filesystem {
    int isinited;
    sf_list_t         directories;
} fs;


static int get_directory_pathname(struct directory *begin,
                                  struct directory *last,
                                  char *buf, size_t count)
{
    struct directory *ptr;
    sf_list_iter_t iter;
    int nwritten = 0;

    if (!sf_list_begin(&fs.directories, &iter)) {
        buf[0] = '\0';
        return nwritten;
    }

    ptr = sf_list_iter_elt(&iter);
    while (begin != ptr) {
        sf_list_iter_next(&iter);
        ptr = sf_list_iter_elt(&iter);
    }

    do {
        int n;
        int len;

        ptr = sf_list_iter_elt(&iter);
        len = strlen(ptr->name) + 1;
        n = snprintf(buf, count, "%s/", ptr->name);
        nwritten += n;
        if (n != len) {
            break;
        }
        buf += n;
        count -= n;
    } while (ptr != last && sf_list_iter_next(&iter));

    return nwritten;
}

static int directory_init(struct directory *d, const char *pathname)
{
    struct stat statbuf;

    if (stat(pathname, &statbuf) < 0) {
        sf_log(SF_LOG_ERR, "failed to stat file %s", pathname);
        return SF_ERR;
    }

    strncpy(d->name, pathname, NAME_MAX);

    if (S_ISDIR(statbuf.st_mode)) {
        d->isopened = 0;
        d->iszip  = 0;
    } else {
        int err;
        if ((d->zip = zip_open(pathname, 0, &err)) == NULL) {
            char buf[1024];
            zip_error_to_str(buf, 1024, err, errno);
            sf_log(SF_LOG_ERR, "failed to open %s: %s\n",
                   pathname, buf);
            return SF_ERR;
        }
        d->iszip = 1;
        d->isopened = 1;
    }

    d->nopens = 0;

    return SF_OK;
}

static int directory_open(struct directory *d)
{
    if (d->isopened) {
        return SF_OK;
    }

    if (d->iszip) {
        assert(0);
    } else {
        char buf[PATH_MAX];

        get_directory_pathname(sf_list_head(&fs.directories), d, buf,
                               PATH_MAX);
        if ((d->dir = opendir(buf)) == NULL) {
            sf_log(SF_LOG_ERR, "failed to open directory %s", d->name);
            return SF_ERR;
        }
    }
    d->isopened = 1;
    return SF_OK;
}

static void directory_close(void *elt)
{
    struct directory *d = elt;

    if (d->isopened) {
        if (d->iszip) {
            if (d->nopens == 0) {
                zip_close(d->zip);
            }
        } else {
            closedir(d->dir);
        }
    }
}


int fs_init(int argc, char **argv)
{
    char buf[PATH_MAX];
    int  len;
    char *ptr;
    sf_list_def_t def;

    if (fs.isinited) {
        sf_log(SF_LOG_WARN, "fs_init: filesystem already initialized.");
        return SF_OK;
    }

    sf_memzero(&def, sizeof(def));
    def.size = sizeof(struct directory);
    def.free = directory_close;
    sf_list_init(&fs.directories, &def);

#ifdef __WIN32__
    if (argv[0][1] == ':') {
#else
    if (argv[0][0] == '/') {
#endif
        strncpy(buf, argv[0], PATH_MAX);
    } else {
        len = PATH_MAX;
        getcwd(buf, len);
        len = strlen(buf);
        buf[len++] = seperator;
        buf[len] = '\0';
        strncat(buf, argv[0], PATH_MAX - len);
    }
    ptr = strrchr(buf, seperator);
    assert(ptr != NULL);
    *ptr = '\0';

    if (fs_cd(buf) != SF_OK) {
        sf_list_destroy(&fs.directories);
        return SF_ERR;
    }

    fs.isinited = 1;
    return SF_OK;
}

void fs_term(void)
{
    sf_list_destroy(&fs.directories);
}

int fs_cwd(char *buf, size_t count)
{
    return get_directory_pathname(sf_list_head(&fs.directories),
                                  sf_list_tail(&fs.directories),
                                  buf, count - 1);
}

static int is_cwd_zip(void)
{
    if (sf_list_cnt(&fs.directories)) {
        struct directory *d;

        d = sf_list_tail(&fs.directories);
        if (d->iszip) {
            return 1;
        }
    }

    return 0;
}

static int mkdir_rc(const char *pathname)
{
    int ret;
    char buf[PATH_MAX];
    char *ptr = NULL;
    size_t len;

    strncpy(buf, pathname, PATH_MAX);
    len = strlen(buf);
    if(buf[len - 1] == '/') {
        buf[len - 1] = 0;
    }

    /*
     * if pathname is absolutely, then pass the root.
     * or pathname[0] must exist;
     */
    ptr = buf + 1;

    while ((ptr = strchr(ptr, '/'))) {
        *ptr = '\0';
#if !defined(__WIN32__)
        if (!fs_is_file_exist(buf) && (ret = mkdir(buf, S_IRWXU))) {
#else
        if (!fs_is_file_exist(buf) && (ret = mkdir(buf))) {
#endif
            return ret;
        }
        *ptr = '/';
        ++ptr;
    }

#if !defined(__WIN32__)
    return mkdir(buf, S_IRWXU);
#else
    return mkdir(buf);
#endif
}

int fs_mkdir(const char *pathname)
{
    if (is_cwd_zip()) {
        assert(0);
    }

    return mkdir_rc(pathname);
}

int fs_is_file_exist(const char *pathname)
{
    if (is_cwd_zip()) {
        assert(0);
    }

    return !access(pathname, F_OK);
}

static int get_zip_cwd(char *buf, size_t count)
{
    sf_list_iter_t iter;
    int iszip = 0;

    buf[0] = '\0';

    if (sf_list_begin(&fs.directories, &iter)) do {
        struct directory *d = sf_list_iter_elt(&iter);
        if (d->iszip) {
            iszip = 1;
            break;
        }
    } while (sf_list_iter_next(&iter));

    if (iszip && sf_list_iter_next(&iter)) {
        return get_directory_pathname(sf_list_iter_elt(&iter),
                                      sf_list_tail(&fs.directories),
                                      buf, count);
    }

    return 0;
}

static int fs_cd_file_zip(struct directory *parent, const char *filename)
{
    int isdirexist = 0;
    char buf[NAME_MAX];
    int len;
    zip_int64_t nentries, i;
    struct directory dir;

    if (strcmp(filename, "..") == 0) {
        sf_list_pop(&fs.directories);
        return SF_OK;
    }

    get_zip_cwd(buf, NAME_MAX);
    strcat(buf, filename);
    len = strlen(buf);
    buf[len++] = '/';
    buf[len] = '\0';

    nentries = zip_get_num_entries(parent->zip, 0);
    for (i = 0; i < nentries; ++i) {
        if (strncmp(zip_get_name(parent->zip, i, 0), buf, len) == 0) {
            isdirexist = 1;
            break;
        }
    }

    if (isdirexist) {
        strncpy(dir.name, filename, NAME_MAX);
        dir.iszip = 1;
        dir.isopened = 1;
        dir.zip = parent->zip;
        dir.nopens = parent->nopens + 1;
        sf_list_push(&fs.directories, &dir);
        return SF_OK;
    } else {
        sf_log(SF_LOG_ERR, "not a directory: %s", filename);
        return SF_ERR;
    }
}

static int fs_cd_file(const char *filename)
{
    struct directory d;

    if (is_cwd_zip()) {
        return fs_cd_file_zip(sf_list_tail(&fs.directories), filename);
    }

    if (strcmp(filename, "..") == 0) {
        if (sf_list_cnt(&fs.directories)) {
            sf_list_pop(&fs.directories);
        }
        return chdir("..");
    } else {
        if (directory_init(&d, filename) != SF_OK) {
            return SF_ERR;
        }
        sf_list_push(&fs.directories, &d);
        if (!d.iszip) {
            return chdir(filename);
        }
    }

    return SF_OK;
}

int fs_cd(const char *pathname)
{
    if (pathname[0] == seperator || pathname[0] == '/') {
    /* absolute path */
        struct directory d;
        char buf[PATH_MAX];
        char *ptr;

        fs_cwd(buf, PATH_MAX);
        ptr = strrchr(buf, '/');
        if (ptr) {
            *ptr = '\0';
        }
        if (strcmp(pathname, buf)) {
            sf_list_clear(&fs.directories);
            directory_init(&d, "/");
            d.name[0] = '\0';
            sf_list_push(&fs.directories, &d);
            chdir("/");
            if (pathname[1] != '\0') {
                return fs_cd(pathname + 1);
            }
        }
#ifdef __WIN32__
    } else if (pathname[1] == ':') {
        /* swich device */
        char buf[3];

        snprintf(buf, 3, "%c:", pathname[0]);
        sf_list_clear(&fs.directories);
        chdir(buf);
        if (pathname[2] != '\0') {
            fs_cd(pathname + 2);
        } else {
            fs_cd("/");
        }
#endif
    } else {
    /* relative path */
        char buf[PATH_MAX];
        char *ptr;
        char delim[2];
        int i = 0;

        strncpy(buf, pathname, PATH_MAX);

        delim[i++] = '/';
#ifdef __WIN32__
        delim[i++] = seperator;
#endif
        delim[i] = '\0';

        ptr = strtok(buf, delim);
        while (ptr) {
            if (strcmp(ptr, ".") != 0) {
                if (fs_cd_file(ptr) != SF_OK) {
                    break;
                }
            }
            ptr = strtok(NULL, delim);
        }
    }
    return SF_OK;
}

static int fs_file_walker_dir(struct directory *d,
                              int (*func)(int type, const char *filename,
                                          void *arg),
                              void *arg)
{
    int ret = SF_OK;
    struct dirent *dirp;

    while ((dirp = readdir(d->dir)) != NULL) {
        int type;
        struct stat statbuf;

        if (strcmp(dirp->d_name, ".") == 0
            || strcmp(dirp->d_name, "..") == 0) {
            continue;
        }

        stat(dirp->d_name, &statbuf);
        if (S_ISDIR(statbuf.st_mode)) {
            type = FS_DIR;
        } else {
            type = FS_FILE;
        }

        if ((ret = func(type, dirp->d_name, arg)) != SF_OK) {
            break;
        }
    }

    rewinddir(d->dir);

    return ret;
}

static int fs_file_walker_zip(struct directory *d,
                              int (*func)(int type, const char *filename,
                                          void *arg),
                              void *arg)
{
    char buf[PATH_MAX];
    size_t len;
    int ret = SF_OK;
    zip_int64_t nentries = zip_get_num_entries(d->zip, 0);
    zip_int64_t i;

    get_zip_cwd(buf, PATH_MAX);
    len = strlen(buf);

    for (i = 0; i < nentries; ++i) {
        const char *filename = zip_get_name(d->zip, i, 0);
        char *ptr;
        if (strncmp(filename, buf, len) == 0) {
            filename += len;
            if (*filename == '\0') {
                continue;
            }
            if ((ptr = strchr(filename, '/')) == NULL) {
                if ((ret = func(FS_FILE, filename, arg)) != SF_OK) {
                    break;
                }
            } else if (ptr == filename + strlen(filename) - 1) {
                char tmp[NAME_MAX];
                strncpy(tmp, filename, strlen(filename) - 1);
                tmp[strlen(filename) - 1] = '\0';
                if ((ret = func(FS_DIR, tmp, arg)) != SF_OK) {
                    break;
                }
            }
        }
    }
    return ret;
}

int fs_file_walker(int (*func)(int type, const char *filename, void *arg),
                   void *arg)
{
    int ret = SF_OK;
    struct directory *d;

    if (sf_list_cnt(&fs.directories) == 0) {
        return SF_OK;
    }

    d = sf_list_tail(&fs.directories);
    if (!d->isopened) {
        if ((ret = directory_open(d)) != SF_OK) {
            return ret;
        }
    }

    if (!d->iszip) {
        return fs_file_walker_dir(d, func, arg);
    } else {
        return fs_file_walker_zip(d, func, arg);
    }
}

static int fs_file_open_in_zip(struct fs_file *f, const char *filename)
{
    struct directory *d;
    char buf[NAME_MAX];

    d = sf_list_tail(&fs.directories);

    get_zip_cwd(buf, NAME_MAX);
    strcat(buf, filename);

    f->isinzip = 1;
    if ((f->zip_file = zip_fopen(d->zip, buf, 0)) == NULL) {
        sf_log(SF_LOG_ERR, "failed to open %s", filename);
        return SF_ERR;
    }

    return SF_OK;
}

int fs_file_open(struct fs_file *f, const char *filename, const char *mode)
{
    if (is_cwd_zip()) {
        return fs_file_open_in_zip(f, filename);
    }

    f->isinzip = 0;
    f->file = fopen(filename, mode);

    return f->file ? 0 : -1;
}

int fs_file_close(struct fs_file *f)
{
    if (f->isinzip) {
        return zip_fclose(f->zip_file);
    } else {
        return fclose(f->file);
    }
}

ssize_t fs_file_write(struct fs_file *f, const void *buf, size_t count)
{
    if (f->isinzip) {
        /* for now, don't support for zip writing */
        assert(0);
        return 0;
    } else {
        return fwrite(buf, count, 1, f->file);
    }
}

ssize_t fs_file_read(const struct fs_file *f, void *buf, size_t count)
{
    if (f->isinzip) {
        return zip_fread(f->zip_file, buf, count);
    } else {
        return fread(buf, 1, count, f->file);
    }
}

size_t fs_file_size(const char *filename)
{
    if (is_cwd_zip()) {
        struct directory *d;
        struct zip_stat sb;
        char buf[NAME_MAX];

        d = sf_list_tail(&fs.directories);

        get_zip_cwd(buf, NAME_MAX);
        strcat(buf, filename);

        zip_stat(d->zip, buf, 0, &sb);
        return sb.size;
    } else {
        struct stat sb;
        stat(filename, &sb);
        return sb.st_size;
    }
}

int fs_file_del(const char *filename)
{
    if (is_cwd_zip()) {
        assert(0);
        return 0;
    } else {
        return remove(filename);
    }

}
