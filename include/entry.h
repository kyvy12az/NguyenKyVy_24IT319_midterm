#ifndef ENTRY_H
#define ENTRY_H

#include "options.h"

#include <stddef.h>
#include <sys/stat.h>

typedef struct {
    /* name dùng để hiển thị; path dùng cho lời gọi hệ thống và readlink(). */
    char *name;
    char *path;
    struct stat info;
    int stat_ok;
} Entry;

typedef struct {
    /* Mảng động giúp xử lý thư mục có số lượng mục không biết trước. */
    Entry *items;
    size_t length;
    size_t capacity;
} EntryList;

void entry_list_init(EntryList *list);
int entry_list_add(EntryList *list, const char *name, const char *path,
                   const struct stat *info, int stat_ok);
void entry_list_destroy(EntryList *list);
void entry_list_sort(EntryList *list, const Options *options);
const struct timespec *entry_time(const Entry *entry, TimeField field);

#endif
