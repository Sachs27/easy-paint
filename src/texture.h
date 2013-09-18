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

void texture_destroy(struct texture *tex);


#endif /* TEXTURE_H */
