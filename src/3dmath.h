#ifndef THREEDMATH_H
#define THREEDMATH_H


typedef float scalar_t;


#include "vector.h"
#include "matrix.h"


int scalar_isequal(scalar_t lhs, scalar_t rhs);

#define SCALAR_CLAMP(a, min, max) do {      \
    scalar_t __scalar_clamp_min = (min);   \
    scalar_t __scalar_clamp_max = (max);   \
    if ((a) < __scalar_clamp_min) {         \
        (a) = __scalar_clamp_min;           \
    } else if ((a) > __scalar_clamp_max) {  \
        (a) = __scalar_clamp_max;           \
    }                                       \
} while(0)


#endif /* THREEDMATH_H */
