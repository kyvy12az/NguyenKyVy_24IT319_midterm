#include "entry.h"
#include "util.h"

#include <stdlib.h>
#include <string.h>

/* qsort() không nhận context theo POSIX, nên comparator dùng cấu hình này. */
static const Options *sort_options;

void entry_list_init(EntryList *list) {
    *list = (EntryList){0};
}

int entry_list_add(EntryList *list, const char *name, const char *path,
                   const struct stat *info, int stat_ok) {
    Entry *resized;

    if (list->length == list->capacity) {
        size_t new_capacity = list->capacity == 0 ? 16 : list->capacity * 2;
        resized = realloc(list->items, new_capacity * sizeof(*resized));
        if (resized == NULL) {
            return -1;
        }
        list->items = resized;
        list->capacity = new_capacity;
    }
    list->items[list->length].name = xstrdup(name);
    list->items[list->length].path = xstrdup(path);
    list->items[list->length].stat_ok = stat_ok;
    list->items[list->length].info = stat_ok ? *info : (struct stat){0};
    ++list->length;
    return 0;
}

void entry_list_destroy(EntryList *list) {
    size_t index;
    for (index = 0; index < list->length; ++index) {
        free(list->items[index].name);
        free(list->items[index].path);
    }
    free(list->items);
    *list = (EntryList){0};
}

const struct timespec *entry_time(const Entry *entry, TimeField field) {
    if (field == TIME_STATUS_CHANGED) {
        return &entry->info.st_ctim;
    }
    if (field == TIME_ACCESSED) {
        return &entry->info.st_atim;
    }
    return &entry->info.st_mtim;
}

static int compare_entries(const void *left_ptr, const void *right_ptr) {
    const Entry *left = left_ptr;
    const Entry *right = right_ptr;
    int result = 0;

    /* Kích thước/thời gian là khóa chính; tên là khóa phụ ổn định. */
    if (sort_options->sort_size && left->info.st_size != right->info.st_size) {
        result = left->info.st_size > right->info.st_size ? -1 : 1;
    } else if (sort_options->sort_time) {
        const struct timespec *left_time = entry_time(left, sort_options->time_field);
        const struct timespec *right_time = entry_time(right, sort_options->time_field);
        if (left_time->tv_sec != right_time->tv_sec) {
            result = left_time->tv_sec > right_time->tv_sec ? -1 : 1;
        } else if (left_time->tv_nsec != right_time->tv_nsec) {
            result = left_time->tv_nsec > right_time->tv_nsec ? -1 : 1;
        }
    }
    if (result == 0) {
        result = strcoll(left->name, right->name);
    }
    return sort_options->reverse ? -result : result;
}

void entry_list_sort(EntryList *list, const Options *options) {
    if (options->unsorted || list->length < 2) {
        return;
    }
    sort_options = options;
    qsort(list->items, list->length, sizeof(*list->items), compare_entries);
}
