#include <math.h>

#include "3dmath.h"


int scalar_isequal(scalar_t lhs, scalar_t rhs) {
    if (fabsf(lhs - rhs) < 0.00001) {
        return 1;
    }

    return 0;
}
