/**
 * @file print.h
 * @brief Top-level printing dispatcher.
 */

#ifndef LSX_PRINT_H
#define LSX_PRINT_H

struct fdata_ptr_vector;
struct lsx_flags;
struct lsx_format_info;

/**
 * @brief Print one listing (the contents of one directory, or the
 *        non-directory operands) in the layout chosen by `flags->format`.
 *
 * Dispatches to the matching lsx_print_* function from print_catalog.h.
 * The entries must already be filtered and sorted.
 *
 * @param file_ptrs Entries to print, in output order.
 * @param flags     Parsed flags.
 */
void lsx_print_catalog(const struct fdata_ptr_vector *file_ptrs,
                      const struct lsx_flags *flags);

#endif
