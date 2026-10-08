#include "options.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void options_init(Options *options) {
    *options = (Options){0};
    options->time_field = TIME_MODIFIED;
    options->size_style = SIZE_DEFAULT;
    options->block_size = parse_block_size(getenv("BLOCKSIZE"));
    if (options->block_size <= 0) {
        options->block_size = 512;
    }
    options->quote_nonprintable = isatty(STDOUT_FILENO);
}

void print_usage(const char *program_name) {
    fprintf(stderr,
            "usage: %s [-AacdFfhiklnqRrSstuw] [file ...]\n",
            program_name);
}

int options_parse(Options *options, int argc, char **argv) {
    int option;

    opterr = 0;
    while ((option = getopt(argc, argv, "AacdFfhiklnqRrSstuw")) != -1) {
        switch (option) {
        case 'A': options->almost_all = true; break;
        case 'a': options->all = true; break;
        case 'c': options->time_field = TIME_STATUS_CHANGED; break;
        case 'd':
            options->directory_as_file = true;
            options->recursive = false;
            break;
        case 'F': options->classify = true; break;
        case 'f': options->unsorted = true; break;
        case 'h':
            options->size_style = SIZE_HUMAN;
            options->block_size = 512;
            break;
        case 'i': options->print_inode = true; break;
        case 'k':
            options->size_style = SIZE_KILOBYTES;
            options->block_size = 1024;
            break;
        case 'l':
            options->long_format = true;
            options->numeric_ids = false;
            break;
        case 'n':
            options->long_format = true;
            options->numeric_ids = true;
            break;
        case 'q': options->quote_nonprintable = true; break;
        case 'R':
            options->recursive = true;
            options->directory_as_file = false;
            break;
        case 'r': options->reverse = true; break;
        case 'S': options->sort_size = true; break;
        case 's': options->print_blocks = true; break;
        case 't': options->sort_time = true; break;
        case 'u': options->time_field = TIME_ACCESSED; break;
        case 'w': options->quote_nonprintable = false; break;
        default:
            if (optopt != 0) {
                fprintf(stderr, "%s: illegal option -- %c\n", argv[0], optopt);
            }
            print_usage(argv[0]);
            return -1;
        }
    }
    return optind;
}
