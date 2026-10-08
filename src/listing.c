#include "listing.h"
#include "entry.h"
#include "format.h"
#include "util.h"

#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static int had_error;

static int is_dot_or_dotdot(const char *name) {
    return strcmp(name, ".") == 0 || strcmp(name, "..") == 0;
}

static int should_include(const char *name, const Options *options) {
    if (name[0] != '.') return 1;
    if (options->all) return 1;
    if (options->almost_all && !is_dot_or_dotdot(name)) return 1;
    return 0;
}

static void report_error(const char *path) {
    fprintf(stderr, "myls: %s: %s\n", path, strerror(errno));
    had_error = 1;
}

static int read_directory(const char *path, const Options *options,
                          EntryList *entries) {
    DIR *directory = opendir(path);
    struct dirent *item;

    if (directory == NULL) {
        report_error(path);
        return -1;
    }
    errno = 0;
    while ((item = readdir(directory)) != NULL) {
        char *full_path;
        struct stat info;
        if (!should_include(item->d_name, options)) continue;
        /* lstat giữ nguyên metadata của symlink thay vì metadata tệp đích. */
        full_path = join_path(path, item->d_name);
        if (lstat(full_path, &info) != 0) {
            report_error(full_path);
            free(full_path);
            continue;
        }
        if (entry_list_add(entries, item->d_name, full_path, &info, 1) != 0) {
            fprintf(stderr, "myls: out of memory\n");
            free(full_path);
            closedir(directory);
            exit(2);
        }
        free(full_path);
        errno = 0;
    }
    if (errno != 0) report_error(path);
    closedir(directory);
    entry_list_sort(entries, options);
    return 0;
}

static void display_entries(const EntryList *entries, const Options *options,
                            int directory_listing) {
    ColumnWidths widths;
    size_t index;

    calculate_widths(entries, options, &widths);
    if (directory_listing && (options->long_format || options->print_blocks)) {
        print_total(entries, options);
    }
    for (index = 0; index < entries->length; ++index) {
        print_entry(&entries->items[index], options, &widths);
    }
}

static void list_directory(const char *path, const Options *options,
                           int print_header, int *printed_anything) {
    EntryList entries;
    size_t index;

    entry_list_init(&entries);
    if (read_directory(path, options, &entries) != 0) {
        entry_list_destroy(&entries);
        return;
    }
    if (print_header) {
        if (*printed_anything) putchar('\n');
        print_display_name(path, options->quote_nonprintable);
        printf(":\n");
    }
    display_entries(&entries, options, 1);
    *printed_anything = 1;

    if (options->recursive) {
        /* Không đi theo symlink trong thư mục, tránh vòng lặp đệ quy. */
        for (index = 0; index < entries.length; ++index) {
            const Entry *entry = &entries.items[index];
            if (S_ISDIR(entry->info.st_mode) && !is_dot_or_dotdot(entry->name)) {
                list_directory(entry->path, options, 1, printed_anything);
            }
        }
    }
    entry_list_destroy(&entries);
}

static int operand_is_directory(const char *path, const Options *options,
                                struct stat *info) {
    struct stat followed;

    if (options->directory_as_file) {
        return lstat(path, info) == 0 ? 0 : -1;
    }
    if (lstat(path, info) != 0) return -1;
    if (S_ISLNK(info->st_mode)) {
        /* Mặc định symlink tới thư mục được mở; -d đã được xử lý phía trên. */
        if (stat(path, &followed) != 0) return -1;
        return S_ISDIR(followed.st_mode) ? 1 : 0;
    }
    return S_ISDIR(info->st_mode) ? 1 : 0;
}

int list_operands(int operand_count, char **operands, const Options *options) {
    char *default_operand = ".";
    EntryList files, directories;
    int index;
    int printed_anything = 0;

    had_error = 0;
    entry_list_init(&files);
    entry_list_init(&directories);
    if (operand_count == 0) {
        operand_count = 1;
        operands = &default_operand;
    }

    /* Manual yêu cầu tệp thường được in trước và thư mục được xử lý sau. */
    for (index = 0; index < operand_count; ++index) {
        struct stat info;
        int directory = operand_is_directory(operands[index], options, &info);
        EntryList *target;
        if (directory < 0) {
            report_error(operands[index]);
            continue;
        }
        target = directory ? &directories : &files;
        if (entry_list_add(target, operands[index], operands[index], &info, 1) != 0) {
            fprintf(stderr, "myls: out of memory\n");
            exit(2);
        }
    }

    entry_list_sort(&files, options);
    entry_list_sort(&directories, options);
    if (files.length > 0) {
        display_entries(&files, options, 0);
        printed_anything = 1;
    }
    for (index = 0; index < (int)directories.length; ++index) {
        int header = operand_count > 1 || options->recursive;
        list_directory(directories.items[index].path, options, header,
                       &printed_anything);
    }

    entry_list_destroy(&files);
    entry_list_destroy(&directories);
    return had_error ? 1 : 0;
}
