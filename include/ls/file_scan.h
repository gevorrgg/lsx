/**
 * @file file_scan.h
 * @brief Reading file metadata and directory contents into vectors.
 *
 * This module is the bridge between the file system (stat, lstat, readdir)
 * and the in-memory representation used by the rest of the program
 * (struct file_data records owned by a struct fdata_ptr_vector).
 */

#ifndef FILE_SCAN_H
#define FILE_SCAN_H

#include <ls/error.h>
#include <stdbool.h>

struct ls_format_info;
struct fdata_ptr_vector;
struct ls_flags;
struct stat;
struct file_data;

/**
 * @brief Create a file_data record for a single path.
 *
 * Always calls lstat() on @p path. Additionally calls stat() to resolve a
 * symbolic link; if stat() fails (e.g. a dangling link) the record is still
 * created with `stat_status == 0` and zeroed target metadata.
 *
 * The caller owns the resulting record and must release it with
 * file_data_destroy() (or hand it to an owning vector).
 *
 * @param out             Receives the new record, or NULL when lstat() or
 *                        the allocation failed. A record is also returned
 *                        together with LS_STAT_ERR for a dangling link under
 *                        @p follow_symlinks, so the caller can still list it.
 * @param path            Path used for the system calls.
 * @param name            Display name stored in the record (usually the
 *                        basename, or the operand as typed by the user).
 * @param follow_symlinks When true, a failed stat() is reported as
 *                        LS_STAT_ERR; otherwise a dangling link is not an error.
 * @return LS_SUCCESS, LS_STAT_ERR if lstat() failed (or stat() failed while
 *         following links), LS_ALLOC_ERR if the record could not be created.
 */
ls_error ls_get_file_info(struct file_data **out,
						  const char *path,
                          const char *name,
						  bool follow_symlinks);

/**
 * @brief Read every entry of a directory into @p files.
 *
 * Entries are filtered according to `flags->hidden` ("." and ".." are kept
 * only with -a; dot-files are kept with -a or -A). Entries whose metadata
 * cannot be read are reported on stderr but still appended.
 *
 * @param files     Initialised owning vector that receives the entries.
 * @param directory Path of the directory to read.
 * @param flags     Parsed flags (hidden-file mode, symlink following).
 * @return LS_SUCCESS, LS_FILE_NOT_FOUND if opendir() failed,
 *         LS_ALLOC_ERR / LS_BUFFER_ERR on allocation failure.
 */
ls_error ls_read_directory(struct fdata_ptr_vector *files,
                           const char *directory,
                           const struct ls_flags *flags);

/**
 * @brief Collect the file operands from the command line.
 *
 * Skips arguments that look like flags (start with '-') until "--" is seen;
 * everything after "--" is treated as an operand. If no operand is present,
 * "." is used. Operands that cannot be accessed are reported on stderr and
 * skipped.
 *
 * @param files           Initialised owning vector that receives one record
 *                        per accessible operand.
 * @param argc            Argument count.
 * @param argv            Arguments (flags and operands mixed).
 * @param follow_symlinks Passed through to ls_get_file_info().
 * @param file_args       Optional output: number of operands given (or 1
 *                        when "." was implied). Used to decide whether
 *                        directory headers must be printed.
 * @return LS_SUCCESS or LS_BUFFER_ERR when a record could not be appended.
 */
ls_error ls_retrieve_files(struct fdata_ptr_vector *files,
                           int argc,
                           const char *argv[],
						   bool follow_symlinks,
						   int *file_args);

/**
 * @brief Build a non-owning view of @p source.
 *
 * Only records for which @p filter returns true are included; a NULL filter
 * includes everything. The view borrows the pointers, so it must be
 * released with fdata_ptr_vector_free() before @p source is destroyed.
 *
 * @param span   Initialised vector that receives the borrowed pointers.
 * @param source Owning vector.
 * @param flags  Passed through to @p filter.
 * @param filter Predicate deciding which records to include, or NULL.
 * @return LS_SUCCESS or LS_BUFFER_ERR.
 */
ls_error ls_get_fdata_span(struct fdata_ptr_vector *span,
                           const struct fdata_ptr_vector *source,
                           const struct ls_flags *flags,
                           bool (*filter)(const struct file_data *file, const struct ls_flags *flags));

/**
 * @brief Join a directory and an entry name into a heap-allocated path.
 *
 * A '/' separator is inserted unless @p directory already ends with one.
 * An empty @p directory is treated as ".".
 *
 * @return Newly allocated string (free() it), or NULL on NULL input or
 *         allocation failure.
 */
char * ls_make_path        (const char *directory, 
                            const char *name);

#endif
