/**
 * @file format_info.h
 * @brief Column width computation for aligned output.
 *
 * Before printing, every entry is examined once to find the widest value of
 * each field (size, owner, link count, ...). The printers then pad each
 * field to that width so the columns line up, exactly like GNU ls does.
 */

#ifndef LSX_FORMAT_INFO_H
#define LSX_FORMAT_INFO_H

#include <stddef.h>
#include <stdbool.h>

/**
 * @brief Maximum printed width of each field over a set of entries.
 *
 * All widths are character counts. For column layouts a separate
 * lsx_format_info is kept per column so that each column is padded
 * independently.
 */
struct lsx_format_info
{
    size_t max_links_width;   /**< Widest hard-link count. */
    size_t max_owner_width;   /**< Widest owner name (or UID with -n). */
    size_t max_group_width;   /**< Widest group name (or GID with -n). */
    size_t max_size_width;    /**< Widest file size (human-readable if -h). */
    size_t max_name_width;    /**< Widest file name, including -F/-p suffix. */
    size_t max_blocks_width;  /**< Widest block count (-s). */
    size_t max_inode_width;   /**< Widest inode number (-i). */
    bool is_human_readable;   /**< Unused; sizes follow lsx_flags instead. */
};

struct file_data;
struct lsx_flags;
struct fdata_ptr_vector;

/** @brief Zero every width in @p self. */
void lsx_format_info_init(struct lsx_format_info *self);

/**
 * @brief Grow the widths in @p info to accommodate @p file.
 *
 * Which stat buffer (link or target) is measured depends on
 * `flags->follow_symlinks`; size and block widths depend on
 * `flags->is_human_readable`.
 */
void lsx_format_info_update(struct lsx_format_info *info,
                           const struct file_data *file,
						   const struct lsx_flags *flags);

/**
 * @brief Compute widths over every entry in @p files.
 *
 * Equivalent to calling lsx_format_info_update() for each element.
 * @p info must already be initialised.
 */
void lsx_get_format_info(struct lsx_format_info *info,
                        const struct fdata_ptr_vector *files,
                        const struct lsx_flags *flags);

#endif
