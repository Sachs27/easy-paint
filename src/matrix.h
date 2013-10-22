#ifndef MATRIX_H
#define MATRIX_H


#include "3dmath.h"


#define MATRIX_GL_TRANSPOSE GL_FALSE


struct mat3 {
    union {
        scalar_t m[9];

        struct {
            scalar_t m00, m10, m20;
            scalar_t m01, m11, m21;
            scalar_t m02, m12, m22;
        };
    };
};


struct mat4 {
    union {
        scalar_t m[16];

        struct {
            scalar_t m00, m10, m20, m30;
            scalar_t m01, m11, m21, m31;
            scalar_t m02, m12, m22, m32;
            scalar_t m03, m13, m23, m33;
        };
    };
};


struct mat4 *mat4_identity(struct mat4 *out);
struct mat4 *mat4_mul(struct mat4 *out, struct mat4 *lhs, struct mat4 *rhs);
struct mat4 *mat4_scale(struct mat4 *out, scalar_t sx, scalar_t sy,
                        scalar_t sz);
struct mat4 *mat4_rotate(struct mat4 *out, scalar_t angle,
                         scalar_t x, scalar_t y, scalar_t z);
struct mat4 *mat4_rotate_x(struct mat4 *out, scalar_t angle);
struct mat4 *mat4_rotate_y(struct mat4 *out, scalar_t angle);
struct mat4 *mat4_rotate_z(struct mat4 *out, scalar_t angle);
struct mat4 *mat4_translate(struct mat4 *out, scalar_t x,
                            scalar_t y, scalar_t z);
struct mat4 *mat4_perspective(struct mat4 *out, scalar_t fov, scalar_t aspect,
                              scalar_t znear, scalar_t zfar);
struct mat4 *mat4_orthographic(struct mat4 *out,
                               scalar_t left, scalar_t right,
                               scalar_t bottom, scalar_t top,
                               scalar_t near, scalar_t far);
struct mat3 *mat4_upleft3(struct mat3 *out, struct mat4 *m);


#endif /* MATRIX_H */
