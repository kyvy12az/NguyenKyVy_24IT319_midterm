#ifndef OPTIONS_H
#define OPTIONS_H

#include <stdbool.h>
#include <sys/types.h>

typedef enum {
    TIME_MODIFIED,
    TIME_STATUS_CHANGED,
    TIME_ACCESSED
} TimeField;

typedef enum {
    SIZE_DEFAULT,
    SIZE_KILOBYTES,
    SIZE_HUMAN
} SizeStyle;

typedef struct {
    /* Mỗi trường ánh xạ trực tiếp tới một hành vi được mô tả trong ls(1). */
    bool almost_all;
    bool all;
    bool directory_as_file;
    bool classify;
    bool unsorted;
    bool print_inode;
    bool long_format;
    bool numeric_ids;
    bool recursive;
    bool reverse;
    bool sort_size;
    bool print_blocks;
    bool sort_time;
    bool quote_nonprintable;
    TimeField time_field;
    SizeStyle size_style;
    long block_size;
} Options;

void options_init(Options *options);
int options_parse(Options *options, int argc, char **argv);
void print_usage(const char *program_name);

#endif
