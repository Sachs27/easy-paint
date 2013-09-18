#ifndef DEBUG_H
#define DEBUG_H


#ifndef NDEBUG

#include <stdio.h>
#define dprintf(format, ...) fprintf(stderr, format, ##__VA_ARGS__)

#else /* NDEBUG */

#define dprintf(format, ...) (void) ;

#endif /* NDEBUG */


#endif /* DEBUG_H */

