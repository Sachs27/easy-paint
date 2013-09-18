#ifndef LOAD_SHADERS_H
#define LOAD_SHADERS_H

#include <GL/gl.h>


struct shader_info {
    GLenum       type;
    const char*  pathname;
    GLuint       shader;
};


/**
 * Takes an array of shader_info structures, each of which
 * contains the type of the shader, and a pointer a C-style character
 * string (i.e., a NULL-terminated array of characters) containing the
 * entire shader source.
 *
 * The array of structures is terminated by a final Shader with the
 * "type" field set to GL_NONE.
 *
 * @return The shader program value (as returned by glCreateProgram())
 *         on success, or zero on failure.
 */
GLuint load_shaders(struct shader_info *info);


#endif /* LOAD_SHADERS_H */
