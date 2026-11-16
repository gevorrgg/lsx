/**
 * @file file_vector.h
 * @brief Growable array of pointers to struct file_data.
 *
 * One container serves two roles, distinguished only by which function
 * releases it:
 *
 *  - an **owning** vector holds records created by file_data_create() and
 *    is released with fdata_ptr_vector_destroy(), which frees the records
 *    and then the array;
 *  - a **view** holds pointers borrowed from an owning vector (a filtered
 *    subset, for example) and is released with fdata_ptr_vector_free(),
 *    which frees only the array.
 *
 * Views must be released before the owning vector they point into.
 *
 * The array grows geometrically (capacity doubles) and starts from
 * capacity 0.
 */

#ifndef LSX_FILE_VECTOR_H
#define LSX_FILE_VECTOR_H

#include "file_data.h"
#include <stdbool.h>
#include <stddef.h>

/** @brief Result of vector operations. */
typedef enum
{
	FDATA_SUCCESS = 0,     /**< OK. */
	FDATA_NULL_ERR,        /**< Vector pointer was NULL. */
	FDATA_OVERFLOW_ERR,    /**< Doubling the capacity would overflow size_t. */
	FDATA_ALLOC_ERR        /**< malloc()/realloc() failed. */
} fdata_err;

/**
 * @brief Vector of record pointers (owning or borrowed, see file header).
 */
struct fdata_ptr_vector
{
	struct file_data **data;  /**< Heap array of `capacity` pointers, `size` of them valid. */
	size_t size;              /**< Number of pointers stored. */
	size_t capacity;          /**< Number of pointers the array can hold. */
};

/**
 * @brief Create an empty vector with room for @p capacity pointers.
 * @note Capacity 0 is allowed; the first append will allocate.
 */
fdata_err fdata_ptr_vector_init(struct fdata_ptr_vector *self, size_t capacity);

/** @brief Append @p value (the pointer itself; nothing is copied). */
fdata_err fdata_ptr_vector_append(struct fdata_ptr_vector *self, struct file_data *value);

/** @brief Reallocate the array to hold exactly @p new_capacity pointers. */
fdata_err fdata_ptr_vector_extend(struct fdata_ptr_vector *self, size_t new_capacity);

/**
 * @brief Release a view: free the pointer array only.
 *
 * The records are untouched. Leaves @p self empty and reusable.
 */
void fdata_ptr_vector_free(struct fdata_ptr_vector *self);

/**
 * @brief Release an owning vector: destroy every record, then the array.
 *
 * Leaves @p self empty and reusable.
 */
void fdata_ptr_vector_destroy(struct fdata_ptr_vector *self);

#endif
