#ifndef UTIL_H
#define UTIL_H

#include <stddef.h>

char *xstrdup(const char *text);
char *join_path(const char *directory, const char *name);
int decimal_width_uintmax(unsigned long long value);
long parse_block_size(const char *value);

#endif
