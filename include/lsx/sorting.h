/**
 * @file sorting.h
 * @brief Ordering of directory entries.
 */

#ifndef LSX_SORTING_H
#define LSX_SORTING_H

struct fdata_ptr_vector;
struct lsx_flags;

/**
 * @brief Sort @p file in place according to `flags->sort`,
 *        `flags->time_type` and `flags->sort_reversed`.
 *
 * Only the pointers are reordered; the underlying records stay put.
 * Ties on size or time are broken by name. All keys sort ascending
 * (smallest size / oldest time first); -r negates the comparison.
 * GNU ls shows the largest / newest first for -S and -t, so this
 * implementation currently differs from it on those two keys.
 *
 * @note Not thread-safe: the comparator reads @p flags through a static
 *       variable because qsort() has no user-data parameter.
 */
void lsx_sort_file_vector(struct fdata_ptr_vector *file, const struct lsx_flags *flags);

#endif
