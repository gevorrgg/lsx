/**
 * @file error.h
 * @brief Error codes shared by every module of my_ls.
 */

#ifndef LS_LS_ERROR_H
#define LS_LS_ERROR_H

/**
 * @brief Result code returned by most ls_* functions.
 *
 * LS_SUCCESS is zero, so a result can be tested with `if (status)` as well
 * as compared explicitly.
 */
typedef enum {
    LS_SUCCESS = 0,        /**< Operation completed successfully. */
    LS_SORTING_ERR,        /**< Sorting failed (currently unused, reserved). */
    LS_ALLOC_ERR,          /**< malloc/realloc/strdup returned NULL. */
    LS_BUFFER_ERR,         /**< A vector could not be created or extended. */
    LS_FILE_NOT_FOUND,     /**< A directory could not be opened. */
    LS_ERR,                /**< Generic failure (e.g. invalid option). */
    LS_STAT_ERR,           /**< stat()/lstat() failed for a path. */
    LS_UNKNOWN_FLAG_ERR    /**< An unsupported flag character was given. */
} ls_error;

#endif //LS_LS_ERROR_H
