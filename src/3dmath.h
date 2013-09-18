#ifndef THREEDMATH_H
#define THREEDMATH_H


typedef float scalar_t;


#include "vector.h"
#include "matrix.h"


int scalar_isequal(scalar_t lhs, scalar_t rhs);

#define SCALAR_CLAMP(a, min, max) do {      \
    if ((a) < (min)) {                      \
        (a) = (min);                        \
    } else if ((a) > (max)) {               \
        (a) = (max);                        \
    }                                       \
} while(0)


#endif /* THREEDMATH_H */
