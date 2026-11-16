/**
 * @file format_info.c
 * @brief Column width measurement (see format_info.h).
 */
#include <lsx/flags/flags.h>
#include <lsx/print/format_info.h>
#include <lsx/print/print_utils.h>
#include <string.h>
#include <file_data.h>

#include "file_vector.h"

void lsx_format_info_init(struct lsx_format_info *self)
{
    self->max_links_width   = 0;
    self->max_owner_width   = 0;
    self->max_group_width   = 0;
    self->max_size_width    = 0;
    self->max_name_width    = 0;
    self->max_blocks_width  = 0;
    self->max_inode_width   = 0;
}

/** @brief `*old_max = max(*old_max, val)`. */
static void format_info_max(size_t *old_max, size_t val)
{
	if (val > *old_max)
	{
		*old_max = val;
	}
}

void lsx_format_info_update(struct lsx_format_info *format_info,
                           const struct file_data *file_info,
						   const struct lsx_flags *flags)
{
    /* Measure the same stat buffer the printers will display. */
	file_data_mode mode = flags->follow_symlinks ? FILE_DATA_TARGET : FILE_DATA_LINK;
    const struct stat *st = get_stat(file_info, mode);
	
    /* size: digits, or the "2.5M" form with -h */
    size_t size_width = flags->is_human_readable ?
                     lsx_human_readable_len(st->st_size) :
                     len(st->st_size);

	format_info_max(&(format_info->max_size_width), size_width);

    /* name, including the optional -F/-p indicator */
    const size_t name_width = get_fname_width(flags, strlen(file_info->name));

	format_info_max(&(format_info->max_name_width), name_width);

    /* uid len: name when resolvable and not -n, numeric otherwise */
    size_t uid_len;

    if (flags->print_owner_name && file_info->uname != NULL)
    {
        uid_len = strlen(file_info->uname);
    }
    else
    {
        uid_len = len(st->st_uid);
    }

    format_info_max(&(format_info->max_owner_width), uid_len);

	/* gid len: same rule as uid */
    size_t gid_len;

    if (flags->print_group_name && file_info->gname != NULL)
    {
        gid_len = strlen(file_info->gname);
    }
    else 
    {
        gid_len = len(st->st_gid);
    }

    format_info_max(&(format_info->max_group_width), gid_len);

    // hardlinks count len
    const size_t nlink_len = len(st->st_nlink);

    format_info_max(&(format_info->max_links_width), nlink_len);


    // blocksize in kilobytes len
    /* st_blocks counts 512-byte blocks; ls reports 1K blocks, hence /2
     * (rounded up) or *512 bytes for the human-readable form. */
    const size_t blocks_width = flags->is_human_readable ? lsx_human_readable_len(st->st_blocks * 512) : len((st->st_blocks + 1) / 2);

    format_info_max(&(format_info->max_blocks_width), blocks_width);

    // inode len
    const size_t inode_len = len(st->st_ino);

    format_info_max(&(format_info->max_inode_width), inode_len);
}

void lsx_get_format_info(struct lsx_format_info *info,
                        const struct fdata_ptr_vector *files,
                        const struct lsx_flags *flags)
{
    for (size_t i = 0; i < files->size; i++)
    {
        lsx_format_info_update(info, (files->data)[i], flags);
    }
}
