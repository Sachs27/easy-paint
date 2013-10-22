#include <assert.h>
#include <string.h>
#define _USE_MATH_DEFINES 1
#include <math.h>

#include "matrix.h"


#define ANGLE_TO_RAD(angle) ((angle) / 180.0 * M_PI)


struct mat4 *mat4_identity(struct mat4 *out) {
    memset(out->m, 0, sizeof(out->m));

    out->m00 = out->m11 = out->m22 = out->m33 = 1.0f;

    return out;
}

struct mat4 *mat4_mul(struct mat4 *out, struct mat4 *lhs, struct mat4 *rhs) {
    struct mat4 m;

    m.m00 = lhs->m00 * rhs->m00 + lhs->m01 * rhs->m10
          + lhs->m02 * rhs->m20 + lhs->m03 * rhs->m30;

    m.m01 = lhs->m00 * rhs->m01 + lhs->m01 * rhs->m11
          + lhs->m02 * rhs->m21 + lhs->m03 * rhs->m31;

    m.m02 = lhs->m00 * rhs->m02 + lhs->m01 * rhs->m12
          + lhs->m02 * rhs->m22 + lhs->m03 * rhs->m32;

    m.m03 = lhs->m00 * rhs->m03 + lhs->m01 * rhs->m13
          + lhs->m02 * rhs->m23 + lhs->m03 * rhs->m33;

    m.m10 = lhs->m10 * rhs->m00 + lhs->m11 * rhs->m10
          + lhs->m12 * rhs->m20 + lhs->m13 * rhs->m30;

    m.m11 = lhs->m10 * rhs->m01 + lhs->m11 * rhs->m11
          + lhs->m12 * rhs->m21 + lhs->m13 * rhs->m31;

    m.m12 = lhs->m10 * rhs->m02 + lhs->m11 * rhs->m12
          + lhs->m12 * rhs->m22 + lhs->m13 * rhs->m32;

    m.m13 = lhs->m10 * rhs->m03 + lhs->m11 * rhs->m13
          + lhs->m12 * rhs->m23 + lhs->m13 * rhs->m33;

    m.m20 = lhs->m20 * rhs->m00 + lhs->m21 * rhs->m10
          + lhs->m22 * rhs->m20 + lhs->m23 * rhs->m30;

    m.m21 = lhs->m20 * rhs->m01 + lhs->m21 * rhs->m11
          + lhs->m22 * rhs->m21 + lhs->m23 * rhs->m31;

    m.m22 = lhs->m20 * rhs->m02 + lhs->m21 * rhs->m12
          + lhs->m22 * rhs->m22 + lhs->m23 * rhs->m32;

    m.m23 = lhs->m20 * rhs->m03 + lhs->m21 * rhs->m13
          + lhs->m22 * rhs->m23 + lhs->m23 * rhs->m33;

    m.m30 = lhs->m30 * rhs->m00 + lhs->m31 * rhs->m10
          + lhs->m32 * rhs->m20 + lhs->m33 * rhs->m30;

    m.m31 = lhs->m30 * rhs->m01 + lhs->m31 * rhs->m11
          + lhs->m32 * rhs->m21 + lhs->m33 * rhs->m31;

    m.m32 = lhs->m30 * rhs->m02 + lhs->m31 * rhs->m12
          + lhs->m32 * rhs->m22 + lhs->m33 * rhs->m32;

    m.m33 = lhs->m30 * rhs->m03 + lhs->m31 * rhs->m13
           + lhs->m32 * rhs->m23 + lhs->m33 * rhs->m33;

    memcpy(out->m, m.m, sizeof(out->m));

    return out;
}

struct mat4 *mat4_scale(struct mat4 *out, scalar_t sx, scalar_t sy,
                        scalar_t sz) {
    mat4_identity(out);
    out->m00 = sx;
    out->m11 = sy;
    out->m22 = sz;
    return out;
}

struct mat4 *mat4_rotate(struct mat4 *out, scalar_t angle,
                         scalar_t x, scalar_t y, scalar_t z) {
    scalar_t r = ANGLE_TO_RAD(angle);
    scalar_t s = sin(r);
    scalar_t c = cos(r);
    scalar_t length = sqrt(x * x + y * y + z * z);
    x /= length;
    y /= length;
    z /= length;

    mat4_identity(out);

    out->m00 = c + x * x * (1 - c);
    out->m10 = y * x * (1 - c) + z * s;
    out->m20 = z * x * (1 - c) - y * s;
    out->m01 = x * y * (1 - c) - z * s;
    out->m11 = c + y * y * (1 - c);
    out->m21 = z * y * (1 - c) + x * s;
    out->m02 = x * z * (1 - c) + y * s;
    out->m12 = y * z * (1 - c) - x * s;
    out->m22 = c + z * z * (1 - c);

    return out;
}

struct mat4 *mat4_rotate_x(struct mat4 *out, scalar_t angle) {
    scalar_t r = ANGLE_TO_RAD(angle);
    scalar_t s = sin(r);
    scalar_t c = cos(r);

    mat4_identity(out);
    out->m11 = c;
    out->m12 =-s;
    out->m21 = s;
    out->m22 = c;

    return out;
}

struct mat4 *mat4_rotate_y(struct mat4 *out, scalar_t angle) {
    scalar_t r = ANGLE_TO_RAD(angle);
    scalar_t s = sin(r);
    scalar_t c = cos(r);

    mat4_identity(out);
    out->m00 = c;
    out->m02 = s;
    out->m20 =-s;
    out->m22 = c;

    return out;
}

struct mat4 *mat4_rotate_z(struct mat4 *out, scalar_t angle) {
    scalar_t r = ANGLE_TO_RAD(angle);
    scalar_t s = sin(r);
    scalar_t c = cos(r);

    mat4_identity(out);
    out->m00 = c;
    out->m01 =-s;
    out->m10 = s;
    out->m11 = c;

    return out;
}

struct mat4 *mat4_translate(struct mat4 *out, scalar_t x,
                            scalar_t y, scalar_t z) {
    mat4_identity(out);
    out->m03 = x;
    out->m13 = y;
    out->m23 = z;
    return out;
}

struct mat4 *mat4_perspective(struct mat4 *out, scalar_t fov, scalar_t aspect,
                              scalar_t znear, scalar_t zfar) {
    scalar_t r = ANGLE_TO_RAD(fov / 2);
    scalar_t dz = zfar - znear;
    scalar_t s = sin(r);
    scalar_t cotangent = 0;

    if (dz == 0 || s == 0 || aspect == 0) {
        return NULL;
    }


    cotangent = cos(r) / s;

    mat4_identity(out);
    out->m00 = cotangent / aspect;
    out->m11 = cotangent;
    out->m22 = -(zfar + znear) / dz;
    /*out->m23 = 2 * zfar * znear / dz;*/
    out->m23 = -2 * zfar * znear / dz;
    out->m32 = -1;
    out->m33 = 0;

    return out;
}

struct mat4 *mat4_orthographic(struct mat4 *out,
                               scalar_t left, scalar_t right,
                               scalar_t bottom, scalar_t top,
                               scalar_t near, scalar_t far) {
    assert(left != right && top != bottom && near != far);
    mat4_identity(out);

    out->m00 = 2 / (right - left);
    out->m11 = 2 / (top - bottom);
    out->m22 = -2 / (far - near);
    out->m03 = (right + left) / (left - right);
    out->m13 = (top + bottom) / (bottom - top);
    out->m23 = (far + near) / (near - far);

    return out;
}

struct mat3 *mat4_upleft3(struct mat3 *out, struct mat4 *m) {
    out->m00 = m->m00;
    out->m01 = m->m01;
    out->m02 = m->m02;
    out->m10 = m->m10;
    out->m11 = m->m11;
    out->m12 = m->m12;
    out->m20 = m->m20;
    out->m21 = m->m21;
    out->m22 = m->m22;
    return out;
}
