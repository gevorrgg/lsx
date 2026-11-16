/**
 * @file error.h
 * @brief Error codes shared by every module of lsx.
 */

#ifndef LSX_ERROR_H
#define LSX_ERROR_H

/**
 * @brief Result code returned by most lsx_* functions.
 *
 * LSX_SUCCESS is zero, so a result can be tested with `if (status)` as well
 * as compared explicitly.
 */
typedef enum {
    LSX_SUCCESS = 0,        /**< Operation completed successfully. */
    LSX_SORTING_ERR,        /**< Sorting failed (currently unused, reserved). */
    LSX_ALLOC_ERR,          /**< malloc/realloc/strdup returned NULL. */
    LSX_BUFFER_ERR,         /**< A vector could not be created or extended. */
    LSX_FILE_NOT_FOUND,     /**< A directory could not be opened. */
    LSX_ERR,                /**< Generic failure (e.g. invalid option). */
    LSX_STAT_ERR,           /**< stat()/lstat() failed for a path. */
    LSX_UNKNOWN_FLAG_ERR    /**< An unsupported flag character was given. */
} lsx_error;

#endif //LSX_ERROR_H
