/**
 * @file file_data.h
 * @brief Metadata record for a single file system entry.
 *
 * A struct file_data holds both lstat() and stat() results so the program
 * can describe either the symbolic link itself or its target without
 * touching the file system again. Owner and group names are resolved once
 * at construction time.
 *
 * Records are always heap-allocated: file_data_create() returns a new
 * record and file_data_destroy() releases it. They are stored in a
 * struct fdata_ptr_vector (see file_vector.h), which owns them.
 */

#ifndef FILE_DATA_H
#define FILE_DATA_H

#include <sys/stat.h>
#include <stdbool.h>

/**
 * @brief Everything my_ls knows about one entry.
 *
 * Strings are heap-allocated and owned by the record; the whole record is
 * released with file_data_destroy().
 */
struct file_data
{
	struct stat st;    /**< stat() result (follows symlinks). Zeroed if stat() failed. */
    struct stat lst;   /**< lstat() result (the entry itself). */
	char *name;        /**< Display name (basename or operand as typed). */
	char *path;        /**< Path usable for further system calls. */
	char *uname;       /**< Owner name, or NULL if not resolvable. */
	char *gname;       /**< Group name, or NULL if not resolvable. */
	int stat_status;   /**< 1 if stat() succeeded, 0 otherwise (dangling link). */
};

/**
 * @brief Which stat buffer a query should look at.
 *
 * Most predicates take this so callers can ask "is the link a directory?"
 * (FILE_DATA_LINK, always false) or "is the target a directory?"
 * (FILE_DATA_TARGET).
 */
typedef enum
{
    FILE_DATA_LINK,    /**< Use lst: the entry itself. */
    FILE_DATA_TARGET   /**< Use st: the symlink target (or the file itself). */
} file_data_mode;

/**
 * @brief Allocate a record from already-collected stat data.
 *
 * Copies both stat buffers, duplicates @p name and @p path and resolves the
 * owner and group names via getpwuid()/getgrgid(). The uid/gid are taken
 * from the target when @p follow_symlinks is set and stat() succeeded,
 * otherwise from the link.
 *
 * @param st              stat() result; may be zeroed when @p stat_status is 0.
 * @param lst             lstat() result.
 * @param name            Display name (copied).
 * @param path            Path for later system calls (copied).
 * @param stat_status     1 if @p st is valid, 0 if stat() failed.
 * @param follow_symlinks Whether -L is in effect.
 * @return New record to release with file_data_destroy(), or NULL when an
 *         argument is NULL or an allocation failed.
 */
struct file_data *file_data_create(const struct stat *st,
                                   const struct stat *lst,
                                   const char *name,
                                   const char *path,
                                   int stat_status,
                                   bool follow_symlinks);

/** @brief Release a record and its strings. NULL is ignored. */
void file_data_destroy(struct file_data *self);

/* Type predicates. Each checks the stat buffer selected by `mode`. Except
 * is_symlink(), they return false when stat() failed (stat_status == 0). */
bool is_directory                    (const struct file_data *file_info, file_data_mode mode); /**< S_ISDIR. */
bool is_symlink                      (const struct file_data *file_info, file_data_mode mode); /**< S_ISLNK. */
bool is_executable                   (const struct file_data *file_info, file_data_mode mode); /**< Any execute bit set. */
bool is_block_device                 (const struct file_data *file_info, file_data_mode mode); /**< S_ISBLK. */
bool is_character_device             (const struct file_data *file_info, file_data_mode mode); /**< S_ISCHR. */
bool is_pipe                         (const struct file_data *file_info, file_data_mode mode); /**< S_ISFIFO. */
bool is_socket                       (const struct file_data *file_info, file_data_mode mode); /**< S_ISSOCK. */



#endif
