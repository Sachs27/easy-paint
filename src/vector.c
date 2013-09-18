#include <string.h>
#include <math.h>
#include <assert.h>

#include "vector.h"


struct vec3 *vec3_negative(struct vec3 *out, struct vec3 *in) {
    out->x = -in->x;
    out->y = -in->y;
    out->z = -in->z;
    return out;
}

struct vec3 *vec3_minus(struct vec3 *out, struct vec3 *lhs, struct vec3 *rhs) {
    out->x = lhs->x - rhs->x;
    out->y = lhs->y - rhs->y;
    out->z = lhs->z - rhs->z;
    return out;
}

struct vec3 *vec3_add(struct vec3 *out, struct vec3 *lhs, struct vec3 *rhs) {
    out->x = lhs->x + rhs->x;
    out->y = lhs->y + rhs->y;
    out->z = lhs->z + rhs->z;
    return out;
}

struct vec3 *vec3_mul(struct vec3 *v, scalar_t val) {
    v->x *= val;
    v->y *= val;
    v->z *= val;
    return v;
}

struct vec3 *vec3_normalize(struct vec3 *v) {
    return vec3_mul(v, 1.0f / vec3_length(v));
}

scalar_t vec3_length(struct vec3 *v) {
    return sqrt(v->x * v->x + v->y * v->y + v->z * v->z);
}

int vec3_isequal(struct vec3 *lhs, struct vec3 *rhs) {
    if (lhs == rhs) {
        return 1;
    }

    if (scalar_isequal(lhs->x, rhs->x) && scalar_isequal(lhs->y, rhs->y)
        && scalar_isequal(lhs->z, rhs->z)) {
        return 1;
    }

    return 0;
}


int vec2_isequal(struct vec2 *lhs, struct vec2 *rhs) {
    if (lhs == rhs) {
        return 1;
    }

    if (scalar_isequal(lhs->x, rhs->x) && scalar_isequal(lhs->y, rhs->y)) {
        return 1;
    }

    return 0;
}


int ivec2_isequal(struct ivec2 *lhs, struct ivec2 *rhs) {
    if (lhs == rhs) {
        return 1;
    }

    if (lhs->x == rhs->x && lhs->y == rhs->y) {
        return 1;
    }

    return 0;
}
