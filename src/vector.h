#ifndef VECTOR_H
#define VECTOR_H


#include <inttypes.h>
#include "3dmath.h"


struct vec4 {
    union {
        struct {
            scalar_t x, y, z, w;
        };

        struct {
            scalar_t r, g, b, a;
        };
    };
};


struct vec3 {
    union {
        struct {
            scalar_t x, y, z;
        };

        struct {
            scalar_t r, g, b;
        };
    };
};


struct vec3 *vec3_negative(struct vec3 *out, struct vec3 *in);
struct vec3 *vec3_minus(struct vec3 *out, struct vec3 *lhs, struct vec3 *rhs);
struct vec3 *vec3_add(struct vec3 *out, struct vec3 *lhs, struct vec3 *rhs);
struct vec3 *vec3_mul(struct vec3 *v, scalar_t val);
scalar_t vec3_length(struct vec3 *v);
struct vec3 *vec3_normalize(struct vec3 *v);
int vec3_isequal(struct vec3 *lhs, struct vec3 *rhs);


struct vec2 {
    scalar_t x, y;
};


int vec2_isequal(struct vec2 *lhs, struct vec2 *rhs);


struct ivec2 {
    int32_t x, y;
};

int ivec2_isequal(struct ivec2 *lhs, struct ivec2 *rhs);


#endif /* VERTOR_H */
