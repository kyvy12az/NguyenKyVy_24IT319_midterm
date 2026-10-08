#include "format.h"
#include "util.h"

#include <ctype.h>
#include <grp.h>
#include <inttypes.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <wchar.h>
#include <wctype.h>

#ifdef __linux__
#include <sys/sysmacros.h>
#endif

#ifndef S_ISVTX
#define S_ISVTX 01000
#endif

static uintmax_t blocks_in_units(const struct stat *info, long block_size) {
    /* POSIX quy định st_blocks theo đơn vị 512 byte. */
    uintmax_t bytes = (uintmax_t)info->st_blocks * 512U;
    return (bytes + (uintmax_t)block_size - 1U) / (uintmax_t)block_size;
}

void format_size(uintmax_t size, SizeStyle style, char *output,
                 size_t output_size) {
    static const char units[] = "BKMGTPE";
    double value = (double)size;
    size_t unit = 0;

    if (style != SIZE_HUMAN) {
        snprintf(output, output_size, "%" PRIuMAX, size);
        return;
    }
    while (value >= 1024.0 && unit < sizeof(units) - 2) {
        value /= 1024.0;
        ++unit;
    }
    if (unit == 0 || value >= 10.0) {
        snprintf(output, output_size, "%.0f%c", value, units[unit]);
    } else {
        snprintf(output, output_size, "%.1f%c", value, units[unit]);
    }
}

void format_mode(mode_t mode, char output[11]) {
    /* Một ký tự loại tệp và ba nhóm quyền owner/group/other. */
    output[0] = S_ISREG(mode) ? '-' : S_ISDIR(mode) ? 'd' :
                S_ISLNK(mode) ? 'l' : S_ISCHR(mode) ? 'c' :
                S_ISBLK(mode) ? 'b' : S_ISFIFO(mode) ? 'p' :
                S_ISSOCK(mode) ? 's' : '?';
#ifdef S_ISWHT
    if (S_ISWHT(mode)) output[0] = 'w';
#endif
    output[1] = mode & S_IRUSR ? 'r' : '-';
    output[2] = mode & S_IWUSR ? 'w' : '-';
    output[3] = mode & S_ISUID ? (mode & S_IXUSR ? 's' : 'S')
                               : (mode & S_IXUSR ? 'x' : '-');
    output[4] = mode & S_IRGRP ? 'r' : '-';
    output[5] = mode & S_IWGRP ? 'w' : '-';
    output[6] = mode & S_ISGID ? (mode & S_IXGRP ? 's' : 'S')
                               : (mode & S_IXGRP ? 'x' : '-');
    output[7] = mode & S_IROTH ? 'r' : '-';
    output[8] = mode & S_IWOTH ? 'w' : '-';
    output[9] = mode & S_ISVTX ? (mode & S_IXOTH ? 't' : 'T')
                               : (mode & S_IXOTH ? 'x' : '-');
    output[10] = '\0';
}

void print_display_name(const char *name, bool replace_nonprintable) {
    const char *cursor = name;
    mbstate_t state = {0};

    if (!replace_nonprintable) {
        fputs(name, stdout);
        return;
    }
    /* Duyệt theo ký tự đa byte để không phá hỏng tên UTF-8 hợp lệ. */
    while (*cursor != '\0') {
        wchar_t wide;
        size_t count = mbrtowc(&wide, cursor, MB_CUR_MAX, &state);
        if (count == (size_t)-1 || count == (size_t)-2) {
            putchar('?');
            ++cursor;
            memset(&state, 0, sizeof(state));
        } else if (count == 0) {
            break;
        } else {
            if (iswprint(wide)) fwrite(cursor, 1, count, stdout);
            else putchar('?');
            cursor += count;
        }
    }
}

static char classify_suffix(mode_t mode) {
    if (S_ISDIR(mode)) return '/';
    if (S_ISLNK(mode)) return '@';
    if (S_ISSOCK(mode)) return '=';
    if (S_ISFIFO(mode)) return '|';
    if (S_ISREG(mode) && (mode & (S_IXUSR | S_IXGRP | S_IXOTH))) return '*';
#ifdef S_ISWHT
    if (S_ISWHT(mode)) return '%';
#endif
    return '\0';
}

static void size_text(const struct stat *info, const Options *options,
                      char *buffer, size_t size) {
    if (S_ISCHR(info->st_mode) || S_ISBLK(info->st_mode)) {
        snprintf(buffer, size, "%u, %u", (unsigned)major(info->st_rdev),
                 (unsigned)minor(info->st_rdev));
    } else {
        format_size((uintmax_t)info->st_size, options->size_style, buffer, size);
    }
}

static void owner_text(const struct stat *info, bool numeric, char *buffer,
                       size_t size) {
    struct passwd *owner = numeric ? NULL : getpwuid(info->st_uid);
    if (owner != NULL) snprintf(buffer, size, "%s", owner->pw_name);
    else snprintf(buffer, size, "%u", (unsigned)info->st_uid);
}

static void group_text(const struct stat *info, bool numeric, char *buffer,
                       size_t size) {
    struct group *group = numeric ? NULL : getgrgid(info->st_gid);
    if (group != NULL) snprintf(buffer, size, "%s", group->gr_name);
    else snprintf(buffer, size, "%u", (unsigned)info->st_gid);
}

void calculate_widths(const EntryList *list, const Options *options,
                      ColumnWidths *widths) {
    size_t index;
    *widths = (ColumnWidths){1, 1, 1, 1, 1, 1};

    for (index = 0; index < list->length; ++index) {
        const Entry *entry = &list->items[index];
        char owner[64], group[64], size[32];
        int width;
        if (!entry->stat_ok) continue;
        width = decimal_width_uintmax((unsigned long long)entry->info.st_ino);
        if (width > widths->inode_width) widths->inode_width = width;
        if (options->size_style == SIZE_HUMAN) {
            char block_text[32];
            format_size((uintmax_t)entry->info.st_blocks * 512U, SIZE_HUMAN,
                        block_text, sizeof(block_text));
            width = (int)strlen(block_text);
        } else {
            width = decimal_width_uintmax(
                blocks_in_units(&entry->info, options->block_size));
        }
        if (width > widths->blocks_width) widths->blocks_width = width;
        width = decimal_width_uintmax((unsigned long long)entry->info.st_nlink);
        if (width > widths->links_width) widths->links_width = width;
        owner_text(&entry->info, options->numeric_ids, owner, sizeof(owner));
        group_text(&entry->info, options->numeric_ids, group, sizeof(group));
        size_text(&entry->info, options, size, sizeof(size));
        if ((int)strlen(owner) > widths->owner_width) widths->owner_width = (int)strlen(owner);
        if ((int)strlen(group) > widths->group_width) widths->group_width = (int)strlen(group);
        if ((int)strlen(size) > widths->size_width) widths->size_width = (int)strlen(size);
    }
}

static void print_timestamp(const Entry *entry, const Options *options) {
    char buffer[64];
    time_t timestamp = entry_time(entry, options->time_field)->tv_sec;
    struct tm local;
    time_t now = time(NULL);
    localtime_r(&timestamp, &local);
    /* Giống ls truyền thống: tệp cũ hơn khoảng 6 tháng sẽ hiện năm. */
    if (timestamp > now + 3600 || timestamp < now - (time_t)15552000) {
        strftime(buffer, sizeof(buffer), "%b %e  %Y", &local);
    } else {
        strftime(buffer, sizeof(buffer), "%b %e %H:%M", &local);
    }
    printf("%s", buffer);
}

void print_entry(const Entry *entry, const Options *options,
                 const ColumnWidths *widths) {
    char mode[11], owner[64], group[64], size[32];

    if (options->print_inode) {
        printf("%*ju ", widths->inode_width, (uintmax_t)entry->info.st_ino);
    }
    if (options->print_blocks) {
        uintmax_t blocks = blocks_in_units(&entry->info, options->block_size);
        if (options->size_style == SIZE_HUMAN) {
            char block_text[32];
            format_size((uintmax_t)entry->info.st_blocks * 512U, SIZE_HUMAN,
                        block_text, sizeof(block_text));
            printf("%*s ", widths->blocks_width, block_text);
        } else {
            printf("%*ju ", widths->blocks_width, blocks);
        }
    }
    if (options->long_format) {
        format_mode(entry->info.st_mode, mode);
        owner_text(&entry->info, options->numeric_ids, owner, sizeof(owner));
        group_text(&entry->info, options->numeric_ids, group, sizeof(group));
        size_text(&entry->info, options, size, sizeof(size));
        printf("%s %*ju %-*s %-*s %*s ", mode, widths->links_width,
               (uintmax_t)entry->info.st_nlink, widths->owner_width, owner,
               widths->group_width, group, widths->size_width, size);
        print_timestamp(entry, options);
        putchar(' ');
    }
    print_display_name(entry->name, options->quote_nonprintable);
    if (options->classify) {
        char suffix = classify_suffix(entry->info.st_mode);
        if (suffix != '\0') putchar(suffix);
    }
    if (options->long_format && S_ISLNK(entry->info.st_mode)) {
        char target[4096];
        ssize_t length = readlink(entry->path, target, sizeof(target) - 1);
        if (length >= 0) {
            target[length] = '\0';
            printf(" -> ");
            print_display_name(target, options->quote_nonprintable);
        }
    }
    putchar('\n');
}

void print_total(const EntryList *list, const Options *options) {
    size_t index;
    uintmax_t total_bytes = 0;
    char text[32];

    for (index = 0; index < list->length; ++index) {
        total_bytes += (uintmax_t)list->items[index].info.st_blocks * 512U;
    }
    if (options->size_style == SIZE_HUMAN) {
        format_size(total_bytes, SIZE_HUMAN, text, sizeof(text));
    } else {
        uintmax_t blocks = (total_bytes + (uintmax_t)options->block_size - 1U) /
                           (uintmax_t)options->block_size;
        snprintf(text, sizeof(text), "%ju", blocks);
    }
    printf("total %s\n", text);
}
