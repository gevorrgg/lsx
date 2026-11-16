/**
 * @file print_utils.h
 * @brief Helpers shared by all printers.
 *
 * Covers: picking the right stat buffer, measuring printed widths,
 * human-readable sizes, the "total" line, inode/blocks prefixes, type
 * indicators, terminal width detection and grid-size computation for the
 * column layouts.
 *
 * Convention for width functions: a `*_len` function returns the number of
 * characters a value would occupy; a `print_*` function prints the value
 * padded to the width stored in struct ls_format_info and followed by a
 * single space.
 */

#ifndef LS_PRINT_UTILS_H
#define LS_PRINT_UTILS_H

#include <file_data.h>
#include <stddef.h>

struct ls_flags;
struct ls_format_info;
struct fdata_ptr_vector;

/**
 * @brief Select the stat buffer for @p f.
 * @param mode FILE_DATA_LINK returns lstat() data, FILE_DATA_TARGET returns
 *             stat() data (the symlink target).
 */
const struct stat *get_stat                         (const struct file_data *f, 
                                                    file_data_mode mode);

/** @brief Number of decimal digits in @p num (1 for zero). */
size_t len                                          (size_t num);

/** @brief Print @p c @p count times. Used for padding. */
void print_chars                                    (char c, 
                                                    size_t count);

/**
 * @brief Width of @p size as printed by print_human_readable().
 *
 * Values below 1024 are printed as plain digits. Larger values get a unit
 * letter and, when the scaled value is below 10, one decimal ("2.5M").
 */
size_t ls_human_readable_len                        (size_t size);

/**
 * @brief Print @p size in -h style ("512", "1.5K", "20M") followed by a space.
 *
 * Uses powers of 1024 and rounds to one decimal; see README for the known
 * differences from GNU coreutils rounding.
 */
void print_human_readable                           (size_t size);

/** @brief True when -F or -p was given. */
bool should_print_suffix                            (const struct ls_flags *flags);

/**
 * @brief Print the type indicator for @p file.
 *
 * "/" for directories (-p and -F), "@" for symlinks and "*" for executables
 * (-F only; "@" is suppressed in long format where "-> target" is shown).
 *
 * @return Number of characters printed (0 or 1).
 */
size_t print_suffix                                 (const struct file_data *file,
                                                     const struct ls_flags *flags);

/**
 * @brief Print the file name and, if enabled, its type indicator.
 * @return Number of characters printed.
 */
size_t print_file_name                              (const struct file_data *file,
                                                     const struct ls_flags *flags);

/**
 * @brief Width of the controlling terminal in columns.
 * @return Result of TIOCGWINSZ, or 80 if it is unavailable.
 */
size_t get_console_width                            ();

/**
 * @brief Width a file name occupies on screen.
 * @param fname_len strlen() of the name.
 * @return @p fname_len plus one when a type indicator will be appended.
 */
size_t get_fname_width                              (const struct ls_flags *flags,
                                                     size_t fname_len);

/**
 * @brief Print the inode (-i) right-aligned to max_inode_width.
 *
 * Prints "?" for a dangling symlink when @p mode is FILE_DATA_TARGET.
 * @p format_info may be NULL to disable padding.
 */
void print_inode                                    (const struct file_data *file,
                                                     const struct ls_format_info *format_info, 
                                                     file_data_mode mode);

/**
 * @brief Print the allocated size in 1K blocks (-s) right-aligned to
 *        max_blocks_width, or human-readable with -h.
 *
 * Prints "?" for a dangling symlink when -L is set.
 */
void print_blocks                                   (const struct file_data *file,
                                                     const struct ls_format_info *format_info,
                                                     const struct ls_flags *flags);

/**
 * @brief Print the "total N" header: sum of allocated 1K blocks over
 *        @p files (human-readable with -h).
 */
void print_total                                    (const struct fdata_ptr_vector *files,
                                                    const struct ls_flags *flags);

/**
 * @brief Compute a separate ls_format_info for each column of a grid.
 *
 * Entries are assigned to columns according to `flags->format`
 * (row-major for FORMAT_HORIZONTAL, column-major otherwise). Inode and
 * block widths are shared across all columns so those prefixes line up.
 *
 * @return calloc()-ed array of @p max_columns entries (free() it), or NULL
 *         on allocation failure.
 */
struct ls_format_info * get_each_column_format_info (const struct fdata_ptr_vector *files,
                                                     const struct ls_flags *flags,
                                                     size_t max_rows,
                                                     size_t max_columns);

/**
 * @brief Find the largest number of columns whose total width (with two
 *        spaces between columns) fits in @p console_width.
 *
 * Tries every column count from files->size down to 1 and stops at the
 * first that fits. Falls back to a single column on allocation failure.
 *
 * @param rows    Output: number of rows in the chosen grid.
 * @param columns Output: number of columns in the chosen grid.
 */
void get_grid_size                                  (const struct fdata_ptr_vector *files,
                                                     const struct ls_flags *flags,
                                                     size_t *rows,
                                                     size_t *columns,
                                                     size_t console_width);

#endif //LS_PRINT_UTILS_H
