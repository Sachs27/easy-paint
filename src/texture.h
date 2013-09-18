#ifndef TEXTURE_H
#define TEXTURE_H


#include <GL/gl.h>


struct texture {
    GLuint  tid;
    GLenum  type;
    int     w;
    int     h;
};


struct texture *texture_load_2d(const char *pathname);

struct texture *texture_create_2d(int w, int h);

void texture_set_parameteri(struct texture *tex, GLenum pname, GLint param);

void texture_destroy(struct texture *tex);


#endif /* TEXTURE_H */
