/**
 * @file print_commas.c
 * @brief Comma-separated layout (-m): see ls_print_commas() in print_catalog.h.
 *
 * Entries are written as "a, b, c" on one line. Before each entry the code
 * checks whether entry + comma would overflow the line and, if so, starts a
 * new line first. Unlike the column layouts, -i and -s values here are not
 * padded to a common width (GNU ls behaves the same).
 */
#include <ls/print/print_utils.h>
#include <stdio.h>
#include <file_vector.h>
#include <ls/print/format_info.h>
#include <ls/flags/flags.h>
#include <string.h>
#include <unistd.h>

#define LS_IS_TERMINAL() isatty(STDOUT_FILENO)
#define LS_DEFAULT_CONSOLE_WIDTH 80

void ls_print_commas(const struct fdata_ptr_vector *files,
                     const struct ls_flags *flags)
{
    size_t printed_characters = 0;	/* cursor position on the current line */

    size_t console_width = LS_IS_TERMINAL()
        ? get_console_width()
        : LS_DEFAULT_CONSOLE_WIDTH;

    for (size_t i = 0; i < files->size; i++)
    {
        const file_data_mode mode =
            flags->follow_symlinks ? FILE_DATA_TARGET : FILE_DATA_LINK;

        const struct file_data *file = files->data[i];

        const struct stat *st = get_stat(file, mode);

        size_t fname_width = get_fname_width(flags, strlen(file->name));

        size_t comma_width = 1;
        size_t space_width = 1;

        size_t inode_width = 0;
        size_t blocks_width = 0;

        bool should_print_comma = (i != files->size - 1);	/* no comma after the last entry */

        if (flags->show_inode)
            inode_width = len(st->st_ino);

        if (flags->show_blocks)
            blocks_width = len(st->st_blocks);

        /* Width the line would have after printing this entry. */
        size_t total_len = printed_characters + fname_width;

        if (flags->show_inode)
            total_len += inode_width + 2 * space_width;

        if (flags->show_blocks)
            total_len += blocks_width + space_width;

        if (should_print_comma)
            total_len += comma_width;

        /* The separating space is dropped when the comma lands exactly on
         * the last column, so the line never exceeds the width. */
        bool should_print_space = should_print_comma &&
                                   total_len < console_width;

        if (total_len > console_width)
        {
            printf("\n");
            printed_characters = 0;
        }

        /* inode */
        if (flags->show_inode)
        {
            printf("%zu ", (size_t)st->st_ino);
            printed_characters += inode_width + space_width;
        }

        /* blocks (-s) */
        if (flags->show_blocks)
        {
            printf("%zu ", (size_t)st->st_blocks);
            printed_characters += blocks_width + space_width;
        }

        print_file_name(file, flags);
        printed_characters += fname_width;

        if (should_print_comma)
        {
            printf(",");
            printed_characters += comma_width;

            if (should_print_space)
            {
                printf(" ");
                printed_characters += space_width;
            }
        }
    }

    /* Terminate the last line; an empty listing still prints a newline. */
    if (files->size == 0 || printed_characters != 0)
        printf("\n");
}
