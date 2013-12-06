#ifndef HAD_CONFIG_H
#define HAD_CONFIG_H
/* #undef HAVE__CLOSE */
/* #undef HAVE__DUP */
/* #undef HAVE__FDOPEN */
/* #undef HAVE__FILENO */
/* #undef HAVE__OPEN */
/* #undef HAVE__SNPRINTF */
/* #undef HAVE__STRCMPI */
/* #undef HAVE__STRDUP */
/* #undef HAVE__STRICMP */
#define HAVE_FSEEKO
#define HAVE_FTELLO
#define HAVE_MKSTEMP
/* #undef HAVE_MOVEFILEEXA */
#define HAVE_SNPRINTF
#define HAVE_STRCASECMP
/* #undef HAVE_STRINGS_H */
/* #undef HAVE_STRUCT_TM_TM_ZONE */
#define HAVE_UNISTD_H
#define PACKAGE "libzip"
#define VERSION "0.10.b"

/* #undef HAVE_SSIZE_T */

#ifndef HAVE_SSIZE_T

#ifndef SIZE_T_LIBZIP
#define SIZE_T_LIBZIP 4
#endif
#ifndef INT_LIBZIP
#define INT_LIBZIP 4
#endif
#ifndef LONG_LIBZIP
#define LONG_LIBZIP 4
#endif
#ifndef LONG_LONG_LIBZIP
#define LONG_LONG_LIBZIP 8
#endif
#ifndef SIZEOF_OFF_T
#define SIZEOF_OFF_T 4
#endif

#endif

#endif /* HAD_CONFIG_H */
