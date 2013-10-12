#ifndef SYSTEM_H
#define SYSTEM_H

/**
 * @filter A buffer containing pairs of null-terminated filter strings. The
 *         last string in the buffer must be terminated by two NULL characters.
 *         The first string in each pair is a display string that describes the
 *         filter (for example, "Text Files"), and the second string specifies
 *         the filter pattern (for example, "*.TXT"). To specify multiple
 *         filter patterns for a single display string, use a semicolon to
 *         separate the patterns (for example, "*.TXT;*.DOC;*.BAK"). A pattern
 *         string can be a combination of valid file name characters and the
 *         asterisk (*) wildcard character. Do not include spaces in the
 *         pattern string.
 */
const char *get_open_file_name(const char *filter);

const char *get_save_file_name(const char *filter);


#endif /* SYSTEM_H */
