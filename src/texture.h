#ifndef TEXTURE_H
#define TEXTURE_H

#ifdef GLES2
# include <GLES2/gl2.h>
#else
# include <GL/gl.h>
#endif

struct texture {
    GLuint  tid;
    GLenum  type;
    int     w;
    int     h;
};

int texture_copy(struct texture *dst, struct texture *src);

int texture_init_2d(struct texture *tex, int w, int h);

int texture_load_2d(struct texture *tex, const char *pathname);

void texture_set_parameteri(struct texture *tex, GLenum pname, GLint param);

void texture_destroy(struct texture *tex);


#endif /* TEXTURE_H */
