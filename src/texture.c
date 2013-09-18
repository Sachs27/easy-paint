#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <GL/glew.h>
#include <GL/glu.h>
#include <IL/il.h>

#include "texture.h"


struct texture_inner {
    ILuint il_id;
    GLsizei width;
    GLsizei height;
    GLenum format;
    GLenum type;
    const GLvoid *pixels;
};


static int texture_inner_init(struct texture_inner *tex_inner,
                              const char *pathname) {
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

struct texture *texture_load_2d(const char *pathname) {
    GLenum err;
    struct texture *tex;
    struct texture_inner tex_inner;

    tex = calloc(1, sizeof(*tex));
    tex->type = GL_TEXTURE_2D;
    glGenTextures(1, &tex->tid);

    if (texture_inner_init(&tex_inner, pathname) != 0) {
        goto sf_texture_load_failed;
    }

    tex->w = tex_inner.width;
    tex->h = tex_inner.height;

    glBindTexture(GL_TEXTURE_2D, tex->tid);
    glTexImage2D(GL_TEXTURE_2D, 0, tex_inner.format,
                 tex_inner.width, tex_inner.height, 0,
                 tex_inner.format, tex_inner.type, tex_inner.pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    texture_inner_uninit(&tex_inner);
    if ((err = glGetError()) != GL_NO_ERROR) {
        fprintf(stderr, "%s\n", gluErrorString(err));
        goto sf_texture_load_failed;
    }

    return tex;

sf_texture_load_failed:
    fprintf(stderr, "Failed to load texture: %s.\n", pathname);
    texture_destroy(tex);
    return NULL;
}

void texture_destroy(struct texture *tex) {
    glDeleteTextures(1, &tex->tid);
    free(tex);
}
