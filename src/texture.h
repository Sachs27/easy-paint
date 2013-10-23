#ifndef TEXTURE_H
#define TEXTURE_H

#include <zip.h>
#include <GL/gl.h>

struct texture {
    GLuint  tid;
    GLenum  type;
    int     w;
    int     h;
};


int texture_load_2d(struct texture *tex, const char *pathname);
int texture_load_2d_zip(struct texture *tex, struct zip *archive,
                        const char *pathname);

struct texture *texture_create_2d(int w, int h);

void texture_set_parameteri(struct texture *tex, GLenum pname, GLint param);

void texture_destroy(struct texture *tex);


#endif /* TEXTURE_H */
