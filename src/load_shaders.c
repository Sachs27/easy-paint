#include <GL/glew.h>
#include "load_shaders.h"
#include "debug.h"


static int attach_shader(GLuint program, struct shader_info *info) {
    GLint   compile_status;
    long    size;
    FILE   *f;
    GLuint  shader;

    shader = glCreateShader(info->type);
    info->shader = shader;
    /* read shader source */
    f = fopen(info->pathname, "rb");
    if (!f) {
        dprintf("Failed to open file: %s\n", info->pathname);
        return -1;
    }
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fseek(f, 0, SEEK_SET); {
        GLchar src[size + 1];
        const GLchar *psrc = src;
        fread(src, 1, size, f);
        src[size] = '\0';
        glShaderSource(shader, 1, &psrc, NULL);
    }
    fclose(f);
    /* compile shader */
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);
    if (compile_status != GL_TRUE) {
#ifndef NDEBUG
        GLsizei len;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len); {
            GLchar log[len];
            glGetShaderInfoLog(shader, len, NULL, log);
            dprintf("%s: %s\n", info->pathname, log);
        }
#endif /* NDEBUG */
        return -1;
    }

    glAttachShader(program, shader);
    return 0;
}

GLuint load_shaders(struct shader_info *info) {
    struct shader_info *entry;
    GLint               link_status;
    GLuint              program;

    program = glCreateProgram();

    for (entry = info; entry->type != GL_NONE; ++entry) {
        entry->shader = 0;
        if (attach_shader(program, entry) != 0) {
            goto load_shaders_fail;
        }
    }

    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &link_status);
    if (link_status != GL_TRUE) {
#ifndef NDEBUG
        GLsizei len;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len); {
            GLchar log[len];
            glGetProgramInfoLog(program, len, NULL, log);
            dprintf("Failed to link program: %s\n", log);
        }
#endif /* NDEBUG */
        goto load_shaders_fail;
    }

    return program;

load_shaders_fail:
    for (entry = info; entry->type != GL_NONE; ++entry) {
        if (entry->shader) {
            glDeleteShader(entry->shader);
            entry->shader = 0;
        }
    }
    glDeleteProgram(program);
    return 0;
}
