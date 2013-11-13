#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef GLES2
# include <GLES2/gl2.h>
#else
# include <GL/glew.h>
#endif

#include <png.h>
#include <zip.h>

#include <sf/utils.h>

#include "texture.h"


struct texture_inner {
    uint32_t width;
    uint32_t height;
    GLenum format;
    GLenum type;
    GLvoid *pixels;
};

static struct zip_file *png_zip_read_file = NULL;

static void png_zip_read(png_structp png_ptr, png_bytep data,
                         png_size_t length) {
    zip_fread(png_zip_read_file, data, length);
}

static int readpng(struct texture_inner *tex_inner, struct zip *archive,
                   const char *filename) {
    FILE               *raw_file;
    struct zip_file    *zip_file;
    unsigned char       sig[8];
    png_structp         png;
    png_infop           info;
    int                 bit_depth;
    int                 color_type;
    int                 rowbytes;
    png_bytep          *row_pointers;
    int                 i;

    if (archive) {
        zip_file = zip_fopen(archive, filename, 0);
        if (!zip_file) {
            return -1;
        }
        png_zip_read_file = zip_file;
    } else {
        raw_file = fopen(filename, "rb");
        if (!raw_file) {
            return -1;
        }
    }

#define READ_BYTE(buf, n) do {          \
    if (archive) {                      \
        zip_fread(zip_file, buf, n);    \
    } else {                            \
        fread(buf, 1, n, raw_file);     \
    }                                   \
} while (0)

#define CLOSE() do {                \
    if (archive) {                  \
        zip_fclose(zip_file);       \
        png_zip_read_file = NULL;   \
    } else {                        \
        fclose(raw_file);           \
    }                               \
} while (0)

    READ_BYTE(sig, 8);

    if (png_sig_cmp(sig, 0, 8)) {
        CLOSE();
        return -1;
    }

    png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png) {
        CLOSE();
        return -1;
    }

    info = png_create_info_struct(png);
    if (!info) {
        png_destroy_read_struct(&png, NULL, NULL);
        CLOSE();
        return -1;
    }

    if (setjmp(png_jmpbuf(png))) {
        png_destroy_read_struct(&png, &info, NULL);
        CLOSE();
        return -1;
    }

    if (archive) {
        png_set_read_fn(png, NULL, png_zip_read);
    } else {
        png_init_io(png, raw_file);
    }

    png_set_sig_bytes(png, 8);
    png_read_info(png, info);
    png_get_IHDR(png, info, &tex_inner->width, &tex_inner->height,
                 &bit_depth, &color_type, NULL, NULL, NULL);

    if (color_type != PNG_COLOR_TYPE_RGBA) {
        png_destroy_read_struct(&png, &info, NULL);
        CLOSE();
        return -1;
    }

    png_read_update_info(png, info);

    rowbytes = png_get_rowbytes(png, info);
    tex_inner->pixels = sf_alloc(rowbytes * tex_inner->height);
    if (!tex_inner->pixels) {
        png_destroy_read_struct(&png, &info, NULL);
        CLOSE();
        return -1;
    }
    row_pointers = calloc(tex_inner->height, sizeof(png_bytep));
    if (!row_pointers) {
        png_destroy_read_struct(&png, &info, NULL);
        CLOSE();
        free(tex_inner->pixels);
        return -1;
    }
    for (i = 0; i < tex_inner->height; ++i) {
        row_pointers[i] = tex_inner->pixels + i * rowbytes;
    }
    png_read_image(png, row_pointers);

    tex_inner->format = GL_RGBA;
    tex_inner->type = GL_UNSIGNED_BYTE;

    free(row_pointers);
    CLOSE();
    return 0;
}
#if 0
static int il_init(void) {
    ILenum ilerr;

    ilInit();
    ilClearColor(0, 0, 0, 0);

    ilerr = ilGetError();
    if (ilerr != IL_NO_ERROR) {
        return -1;
    }

    return 0;
}


static int texture_inner_init(struct texture_inner *tex_inner,
                              const char *pathname) {
    static int isil_inited = 0;

    if (!isil_inited) {
        if (il_init() != 0) {
            return -1;
        }
        isil_inited = 1;
    }

    ilGenImages(1, &tex_inner->il_id);
    ilBindImage(tex_inner->il_id);

    if (ilLoadImage(pathname) != IL_TRUE) {
        goto TEX_INIT_FAILED;
    }

    if (ilConvertImage(IL_RGBA, IL_FLOAT) != IL_TRUE)
        goto TEX_INIT_FAILED;

    tex_inner->pixels = ilGetData();
    tex_inner->width = ilGetInteger(IL_IMAGE_WIDTH);
    tex_inner->height = ilGetInteger(IL_IMAGE_HEIGHT);
    tex_inner->format = IL_RGBA;
    tex_inner->type = IL_FLOAT;

    ilBindImage(0);

    return 0;

TEX_INIT_FAILED:
    ilDeleteImages(1, &tex_inner->il_id);
    ilBindImage(0);
    return -1;
}

static void texture_inner_uninit(struct texture_inner *tex_inner) {
    ilDeleteImages(1, &tex_inner->il_id);
}
#endif

int texture_load_2d_zip(struct texture *tex, struct zip *archive,
                        const char *pathname) {
    GLenum err;
    struct texture_inner tex_inner;

    tex->type = GL_TEXTURE_2D;
    glGenTextures(1, &tex->tid);

    if (readpng(&tex_inner, archive, pathname) != 0) {
        goto texture_load_failed;
    }

    tex->w = tex_inner.width;
    tex->h = tex_inner.height;

    glBindTexture(GL_TEXTURE_2D, tex->tid);
    glTexImage2D(GL_TEXTURE_2D, 0, tex_inner.format,
                 tex_inner.width, tex_inner.height, 0,
                 tex_inner.format, tex_inner.type, tex_inner.pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

    free(tex_inner.pixels);

    if ((err = glGetError()) != GL_NO_ERROR) {
        goto texture_load_failed;
    }

    return 0;

texture_load_failed:
    fprintf(stderr, "Failed to load texture: %s.\n", pathname);
    texture_destroy(tex);
    return -1;
}

int texture_load_2d(struct texture *tex, const char *pathname) {
    return texture_load_2d_zip(tex, NULL, pathname);
}

struct texture *texture_create_2d(int w, int h) {
    struct texture *tex;

    tex = sf_alloc(sizeof(*tex));
    tex->type = GL_TEXTURE_2D;
    tex->w = w;
    tex->h = h;
    glGenTextures(1, &tex->tid);
    glBindTexture(GL_TEXTURE_2D, tex->tid);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

    return tex;
}

void texture_set_parameteri(struct texture *tex, GLenum pname, GLint param) {
    glBindTexture(tex->type, tex->tid);
    glTexParameteri(tex->type, pname, param);
    glBindTexture(tex->type, 0);
}

void texture_destroy(struct texture *tex) {
    glDeleteTextures(1, &tex->tid);
    free(tex);
}
