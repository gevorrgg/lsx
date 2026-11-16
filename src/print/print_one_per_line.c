/**
 * @file print_one_per_line.c
 * @brief One entry per line (-1): see lsx_print_one_per_line() in print_catalog.h.
 *
 * The simplest layout. Inode and block prefixes (-i, -s) are still padded
 * to a common width so the names line up.
 */
#include <file_vector.h>
#include <lsx/flags/flags.h>
#include <lsx/print/print_utils.h>
#include <lsx/print/format_info.h>
#include <stdio.h>

void   lsx_print_one_per_line         (const struct fdata_ptr_vector *files,
                                      const struct lsx_flags *flags)
{
    struct lsx_format_info format;
    lsx_format_info_init(&format);

    lsx_get_format_info(&format, files, flags);

    for (size_t i = 0; i < files->size; i++)
    {
        const struct file_data *file = (files->data)[i];
        const file_data_mode mode = flags->follow_symlinks ? FILE_DATA_TARGET : FILE_DATA_TARGET;

        if (flags->show_inode)
        {
            print_inode(file, &format, mode);
        }

		if (flags->show_blocks)
		{
			print_blocks(file, &format, flags);
		}

        print_file_name(files->data[i], flags);

        printf("\n");
    }
}
