/**
 * @file print_catalog.h
 * @brief One printer per output layout.
 *
 * Every function takes the same arguments and prints the whole list in its
 * own layout. They are selected by ls_print_catalog() based on
 * `flags->format`. All printers honour -i (inode), -s (blocks) and -F/-p
 * (type indicators); only ls_print_long() shows full metadata.
 */

#ifndef LS_PRINT_CATALOG_H
#define LS_PRINT_CATALOG_H

struct fdata_ptr_vector;
struct ls_flags;
struct ls_format_info;

/**
 * @brief Long format (-l): "total N" line, then one line per entry with
 *        type, permissions, links, owner, group, size, date and name.
 *        Symbolic links are shown as "name -> target" unless -L is set.
 */
void   ls_print_long                 (const struct fdata_ptr_vector *files,
                                      const struct ls_flags *flags);

/**
 * @brief Comma-separated format (-m): "a, b, c" wrapped at the terminal
 *        width (80 columns when stdout is not a terminal).
 */
void   ls_print_commas               (const struct fdata_ptr_vector *files,
                                      const struct ls_flags *flags);

/** @brief One entry per line (-1). */
void   ls_print_one_per_line         (const struct fdata_ptr_vector *files,
                                      const struct ls_flags *flags);

/**
 * @brief Horizontal columns (-x): entries fill each row left to right
 *        before moving to the next row.
 */
void   ls_print_horizontal           (const struct fdata_ptr_vector *files,
                                      const struct ls_flags *flags);

/**
 * @brief Vertical columns (-C): entries fill each column top to bottom
 *        before moving to the next column. The grid always fits the
 *        terminal width.
 */
void   ls_print_columns              (const struct fdata_ptr_vector *files,
                                      const struct ls_flags *flags);

/**
 * @brief Default layout: vertical columns when stdout is a terminal,
 *        one entry per line otherwise (matches `ls` piped into a file).
 */
void   ls_print_standard             (const struct fdata_ptr_vector *files,
                                      const struct ls_flags *flags);

#endif //LS_PRINT_CATALOG_H
