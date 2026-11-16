/**
 * @file print_catalog.h
 * @brief One printer per output layout.
 *
 * Every function takes the same arguments and prints the whole list in its
 * own layout. They are selected by lsx_print_catalog() based on
 * `flags->format`. All printers honour -i (inode), -s (blocks) and -F/-p
 * (type indicators); only lsx_print_long() shows full metadata.
 */

#ifndef LSX_PRINT_CATALOG_H
#define LSX_PRINT_CATALOG_H

struct fdata_ptr_vector;
struct lsx_flags;
struct lsx_format_info;

/**
 * @brief Long format (-l): "total N" line, then one line per entry with
 *        type, permissions, links, owner, group, size, date and name.
 *        Symbolic links are shown as "name -> target" unless -L is set.
 */
void   lsx_print_long                 (const struct fdata_ptr_vector *files,
                                      const struct lsx_flags *flags);

/**
 * @brief Comma-separated format (-m): "a, b, c" wrapped at the terminal
 *        width (80 columns when stdout is not a terminal).
 */
void   lsx_print_commas               (const struct fdata_ptr_vector *files,
                                      const struct lsx_flags *flags);

/** @brief One entry per line (-1). */
void   lsx_print_one_per_line         (const struct fdata_ptr_vector *files,
                                      const struct lsx_flags *flags);

/**
 * @brief Horizontal columns (-x): entries fill each row left to right
 *        before moving to the next row.
 */
void   lsx_print_horizontal           (const struct fdata_ptr_vector *files,
                                      const struct lsx_flags *flags);

/**
 * @brief Vertical columns (-C): entries fill each column top to bottom
 *        before moving to the next column. The grid always fits the
 *        terminal width.
 */
void   lsx_print_columns              (const struct fdata_ptr_vector *files,
                                      const struct lsx_flags *flags);

/**
 * @brief Default layout: vertical columns when stdout is a terminal,
 *        one entry per line otherwise (matches `ls` piped into a file).
 */
void   lsx_print_standard             (const struct fdata_ptr_vector *files,
                                      const struct lsx_flags *flags);

#endif //LSX_PRINT_CATALOG_H
