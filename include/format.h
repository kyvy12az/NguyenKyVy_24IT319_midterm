#ifndef FORMAT_H
#define FORMAT_H

#include "entry.h"
#include "options.h"

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

typedef struct {
    int inode_width;
    int blocks_width;
    int links_width;
    int owner_width;
    int group_width;
    int size_width;
} ColumnWidths;

void calculate_widths(const EntryList *list, const Options *options,
                      ColumnWidths *widths);
void print_entry(const Entry *entry, const Options *options,
                 const ColumnWidths *widths);
void print_total(const EntryList *list, const Options *options);
void format_mode(mode_t mode, char output[11]);
void format_size(uintmax_t size, SizeStyle style, char *output,
                 size_t output_size);
void print_display_name(const char *name, bool replace_nonprintable);

#endif
