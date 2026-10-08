#include "util.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *xstrdup(const char *text) {
    char *copy = strdup(text);
    if (copy == NULL) {
        fprintf(stderr, "myls: out of memory\n");
        exit(2);
    }
    return copy;
}

char *join_path(const char *directory, const char *name) {
    size_t directory_length = strlen(directory);
    size_t name_length = strlen(name);
    int needs_slash = directory_length > 0 && directory[directory_length - 1] != '/';
    char *path = malloc(directory_length + (size_t)needs_slash + name_length + 1);

    if (path == NULL) {
        fprintf(stderr, "myls: out of memory\n");
        exit(2);
    }
    memcpy(path, directory, directory_length);
    if (needs_slash) {
        path[directory_length++] = '/';
    }
    memcpy(path + directory_length, name, name_length + 1);
    return path;
}

int decimal_width_uintmax(unsigned long long value) {
    int width = 1;
    while (value >= 10) {
        value /= 10;
        ++width;
    }
    return width;
}

long parse_block_size(const char *value) {
    char *end;
    long result;
    long multiplier = 1;

    if (value == NULL || *value == '\0') {
        return 0;
    }
    errno = 0;
    result = strtol(value, &end, 10);
    if (errno != 0 || result <= 0) {
        return 0;
    }
    if (*end != '\0') {
        if (end[1] != '\0') {
            return 0;
        }
        switch (*end) {
        case 'k': case 'K': multiplier = 1024L; break;
        case 'm': case 'M': multiplier = 1024L * 1024L; break;
        case 'g': case 'G': multiplier = 1024L * 1024L * 1024L; break;
        default: return 0;
        }
    }
    if (result > LONG_MAX / multiplier) {
        return 0;
    }
    return result * multiplier;
}
