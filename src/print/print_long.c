/**
 * @file print_long.c
 * @brief -l layout: full metadata, one entry per line.
 *
 * Line structure (fields in brackets are optional):
 *
 *   [inode] [blocks] type+perms links owner group size date name [-> target] [indicator]
 *
 * Every numeric field is right-aligned to the widest value in the listing
 * (struct lsx_format_info). Dangling symlinks under -L have no target
 * metadata, so their fields are printed as "?" the way GNU ls does.
 */
#include <lsx/print/print_catalog.h>
#include <file_data.h>
#include <file_vector.h>
#include <stdio.h>
#include <sys/stat.h>
#include <lsx/print/print_utils.h>
#include <lsx/print/format_info.h>
#include <lsx/flags/flags.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/**
 * @brief First character of the mode column: d, l, s, p, c, b or '-'.
 *
 * Symlinks are always detected on the link itself (FILE_DATA_LINK) so
 * that "l" is shown even when the target is a directory, unless the
 * caller passed FILE_DATA_TARGET and the target is a directory (-L).
 */
static void print_type(const struct file_data *file, const file_data_mode mode)
{
    if (is_directory(file, mode))
    {
        printf("d");
    }
    else if (is_symlink(file, FILE_DATA_LINK))
    {
        printf("l");
    }
    else if (is_socket(file, mode))
    {
        printf("s");
    }
    else if (is_pipe(file, mode))
    {
        printf("p");
    }
    else if (is_character_device(file, mode))
    {
        printf("c");
    }
    else if (is_block_device(file, mode))
    {
        printf("b");
    }
    else
    {
        printf("-");
    }
}

/**
 * @brief The nine permission characters, e.g. "rwxr-xr-x".
 *
 * Special bits replace the execute character: setuid/setgid show as
 * 's' (or 'S' when the execute bit is clear), sticky as 't' / 'T'.
 */
static void print_permissions(const struct file_data *file, const file_data_mode mode)
{
	if (is_symlink(file, FILE_DATA_LINK) && !(file->stat_status) && mode == FILE_DATA_TARGET)
	{
		printf("????????? ");
		return;
	}
	
    const struct stat *st = get_stat(file, mode);
    mode_t m = st->st_mode;

    char p[10];

    // user
    p[0] = (m & S_IRUSR) ? 'r' : '-';
    p[1] = (m & S_IWUSR) ? 'w' : '-';
    p[2] = (m & S_ISUID) ? ((m & S_IXUSR) ? 's' : 'S')
                      : ((m & S_IXUSR) ? 'x' : '-');

    // group
    p[3] = (m & S_IRGRP) ? 'r' : '-';
    p[4] = (m & S_IWGRP) ? 'w' : '-';
    p[5] = (m & S_ISGID) ? ((m & S_IXGRP) ? 's' : 'S')
                      : ((m & S_IXGRP) ? 'x' : '-');

    // other
    p[6] = (m & S_IROTH) ? 'r' : '-';
    p[7] = (m & S_IWOTH) ? 'w' : '-';
    p[8] = (m & S_ISVTX) ? ((m & S_IXOTH) ? 't' : 'T')
                     : ((m & S_IXOTH) ? 'x' : '-');

    p[9] = '\0';

    printf("%s ", p);
}

/** @brief Hard-link count, right-aligned to max_links_width. */
static void print_hardlinks_count(const struct file_data *file,
                            const struct lsx_format_info *format,
                            const file_data_mode mode)
{
	if (is_symlink(file, FILE_DATA_LINK) && !(file->stat_status) && mode == FILE_DATA_TARGET)
	{
        print_chars(' ', format->max_links_width - 1);
		printf("? ");
		return;
	}

    const struct stat *st = get_stat(file, mode);
    const size_t hardlinks_count = st->st_nlink;
    const size_t spaces_count = format->max_links_width - len(hardlinks_count);

    print_chars(' ', spaces_count);

    printf("%zu ", hardlinks_count);
}

/**
 * @brief Print one owner or group cell, padded to @p width.
 *
 * Follows GNU coreutils: a name is left-aligned and padded on the right,
 * a numeric id is right-aligned. Numeric ids appear with -n or when the id
 * has no entry in the user/group database. A single space always follows.
 */
static void print_id_cell(const char *name, unsigned id, size_t width)
{
    if (name != NULL)
    {
        printf("%s", name);
        print_chars(' ', width - strlen(name) + 1);
    }
    else
    {
        print_chars(' ', width - len(id));
        printf("%u ", id);
    }
}

/**
 * @brief Owner column: user name, or numeric UID with -n or when the
 *        name could not be resolved. Padded to max_owner_width.
 */
static void print_owner(const struct file_data *file,
                        const struct lsx_format_info *format,
                        const struct lsx_flags *flags)
{
	if (is_symlink(file, FILE_DATA_LINK) && !(file->stat_status) && flags->follow_symlinks)
	{
		printf("?");
		print_chars(' ', format->max_owner_width);
		return;
	}

    const file_data_mode mode = flags->follow_symlinks ? FILE_DATA_TARGET : FILE_DATA_LINK;
    const struct stat *st = get_stat(file, mode);

    const char *name = flags->print_owner_name ? file->uname : NULL;

    print_id_cell(name, (unsigned)st->st_uid, format->max_owner_width);
}

/** @brief Group column; same rules as print_owner(). */
static void print_group(const struct file_data *file,
                 const struct lsx_format_info *format,
                 const struct lsx_flags *flags)
{
	if (is_symlink(file, FILE_DATA_LINK) && !(file->stat_status) && flags->follow_symlinks)
	{
		printf("?");
		print_chars(' ', format->max_group_width);
		return;
	}

    const file_data_mode mode = flags->follow_symlinks ? FILE_DATA_TARGET : FILE_DATA_LINK;
    const struct stat *st = get_stat(file, mode);

    const char *name = flags->print_group_name ? file->gname : NULL;

    print_id_cell(name, (unsigned)st->st_gid, format->max_group_width);
}

/** @brief Size column in bytes, or human-readable with -h. */
static void print_size(const struct file_data *file,
                       const struct lsx_flags *flags,
                       const struct lsx_format_info *format)
{
	if (is_symlink(file, FILE_DATA_LINK) && !(file->stat_status) && flags->follow_symlinks)
	{
		if (format) print_chars(' ', format->max_size_width - 1);
		printf("? ");
		return;
	}


    const file_data_mode mode = flags->follow_symlinks ? FILE_DATA_TARGET : FILE_DATA_LINK;

    const struct stat *st = get_stat(file, mode);

    const size_t size = st->st_size;

    if (flags->is_human_readable)
    {
        size_t spaces_count = format->max_size_width - lsx_human_readable_len(size);
        print_chars(' ', spaces_count);
        print_human_readable(size);
    }
    else
    {
        size_t spaces_count = format->max_size_width - len(size);
        print_chars(' ', spaces_count);
        printf("%lu ", size);
    }
}

/** @brief Select the timestamp requested by -u / -c (default: mtime). */
time_t get_time(const struct stat *file_stat, const struct lsx_flags *flags)
{
    switch (flags->time_type)
    {
        case TIME_CHANGE:
            return file_stat->st_ctime;
        case TIME_ACCESS:
            return file_stat->st_atime;
        default:
            return file_stat->st_mtime;
    }
}

/** @brief Three-letter English month abbreviation for tm_mon (0..11). */
static const char *get_month(const int month)
{
    if (month < 0 || month > 11)
    {
        perror("get_month");
        return NULL;
    }

    static const char *months[12] = { "Jan", "Feb", "Mar",
                                     "Apr", "May", "Jun",
                                     "Jul", "Aug", "Sep",
                                     "Oct", "Nov", "Dec" };

    return months[month];
}

/**
 * @brief GNU ls rule: timestamps more than six months away from now (in
 *        either direction) show the year instead of the time of day.
 */
static bool should_print_year(const time_t t)
{
    const time_t now = time(NULL);
    const time_t halfyear = 31556952 / 2;   /* 365.2425 days in seconds, halved */

    return (t < now - halfyear || t > now + halfyear);
}

/**
 * @brief Date column: "Mon DD HH:MM" for recent files, "Mon DD  YYYY"
 *        otherwise. Always 12 characters plus a trailing space.
 */
void print_datetime(const struct file_data      *file,
                    const struct lsx_flags       *flags)
{
	if (is_symlink(file, FILE_DATA_LINK) && !(file->stat_status) && flags->follow_symlinks)
	{
		print_chars(' ', 11);
		printf("? ");
		return;
	}

    const file_data_mode mode = flags->follow_symlinks ? FILE_DATA_TARGET : FILE_DATA_LINK;

    const struct stat *st = get_stat(file, mode);
    time_t t = get_time(st, flags);

    struct tm dtime;

    if (!localtime_r(&t, &dtime))
    {
        perror("localtime_r");
        return;
    }

    printf("%s %2d ", get_month(dtime.tm_mon), dtime.tm_mday);

    if (should_print_year(t))
    {
        printf(" %d", dtime.tm_year + 1900);
    }
    else
    {
        printf("%02d:%02d",dtime.tm_hour, dtime.tm_min);
    }

    printf(" ");
}

/** @brief Everything between the optional prefixes and the file name. */
static void print_metadata(const struct file_data *file,
                           const struct lsx_flags *flags,
                           const struct lsx_format_info *format_info)
{
    const file_data_mode mode = flags->follow_symlinks ? FILE_DATA_TARGET : FILE_DATA_LINK;
    print_type(file, mode);
    print_permissions(file, mode);

    print_hardlinks_count(file, format_info, mode);

    if (flags->show_owner)
        print_owner(file, format_info, flags);

    if (flags->show_group)
        print_group(file, format_info, flags);

    print_size(file, flags, format_info);
    print_datetime(file, flags);
}

/**
 * @brief Print " -> target" for a symbolic link.
 *
 * readlink() does not NUL-terminate and returns the number of bytes
 * copied, so the buffer starts at lst.st_size (the link's own length) and
 * doubles until the result fits with room for the terminator.
 */
static void print_pointing_path(const struct file_data *file)
{
    ssize_t size = file->lst.st_size > 0 ? file->lst.st_size : 256;

    while (true)
    {
        char *buf = malloc(size);

        if (buf == NULL)
        {
            perror("malloc");
            return;
        }

        const ssize_t len = readlink(file->path, buf, size);

        if (len == -1)
        {
            free(buf);
            perror("readlink");
            return;
        }

        /* len == size means the target may have been truncated: retry. */
        if (len < size)
        {
            buf[len] = '\0';
            printf(" -> %s", buf);
            free(buf);
            return;
        }

        free(buf);
        size *= 2;
    }
}

/** @brief Print one complete long-format line (without the newline). */
void lsx_print_info_long(const struct file_data *file,
                        const struct lsx_flags *flags,
                        const struct lsx_format_info *format_info)
{
    const file_data_mode mode = flags->follow_symlinks ? FILE_DATA_TARGET : FILE_DATA_LINK;

    if (flags->show_inode)
        print_inode(file, format_info, mode);

    if (flags->show_blocks)
        print_blocks(file, format_info, flags);

    print_metadata(file, flags, format_info);

    printf("%s", file->name);

    /* With -L the target's metadata is shown instead, so no arrow. */
    if (!flags->follow_symlinks && is_symlink(file, FILE_DATA_LINK))
    {
        print_pointing_path(file);
    }

	if (should_print_suffix(flags))
	{
		print_suffix(file, flags);
	}

}

void   lsx_print_long                 (const struct fdata_ptr_vector *files,
                                      const struct lsx_flags *flags)
{
    /* Pass 1: measure column widths. Pass 2: print. */
    struct lsx_format_info format_info;
    lsx_format_info_init(&format_info);

    lsx_get_format_info(&format_info, files, flags);

    print_total(files, flags);

    for (size_t i = 0; i < files->size; i++)
    {
        lsx_print_info_long(files->data[i], flags, &format_info);
        printf("\n");
    }
}
