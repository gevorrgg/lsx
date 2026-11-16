/**
 * @file file_vector.c
 * @brief Growable array of record pointers (see file_vector.h).
 *
 * Growth policy: capacity 0 -> 1 -> 2 -> 4 -> ..., with an overflow check
 * on the multiplication.
 */
#include <file_vector.h>
#include <file_data.h>
#include <stdlib.h>
#define CAPACITY_GROWTH_FACTOR 2

fdata_err fdata_ptr_vector_init(struct fdata_ptr_vector* self, size_t capacity)
{
	if (self == NULL)
		return FDATA_NULL_ERR;

	/* malloc(0) may return NULL or a unique pointer; a NULL here is
	 * reported as an error, and callers use capacity 0 only when the
	 * first append is expected to allocate anyway. */
	struct file_data** data_ptr = malloc(sizeof(struct file_data*) * capacity);

	if (data_ptr == NULL)
		return FDATA_ALLOC_ERR;	// could not allocate

	self->data     = data_ptr;
	self->size 	   = 0;
	self->capacity = capacity;

    return FDATA_SUCCESS;
}

fdata_err fdata_ptr_vector_append(struct fdata_ptr_vector* self, struct file_data* value)
{
	if (self == NULL)
		return FDATA_NULL_ERR;

	if (self->size == self->capacity)
	{
		size_t new_capacity;
		
		if (self->capacity == 0)
		{
			new_capacity = 1;
		}
		else
		{
			new_capacity = self->capacity * CAPACITY_GROWTH_FACTOR;
			
			if (new_capacity / 2 != self->capacity) // overflow
				return FDATA_OVERFLOW_ERR;
		}

		fdata_err extend_status = fdata_ptr_vector_extend(self, new_capacity);

		if (extend_status != FDATA_SUCCESS)	// failed to resize
			return extend_status;
	}

	/* Shallow: only the pointer is stored. */
	self->data[self->size] = value;
	self->size += 1;

	return FDATA_SUCCESS;
}

fdata_err fdata_ptr_vector_extend(struct fdata_ptr_vector* self, size_t new_capacity)
{
	if (self == NULL)
		return FDATA_NULL_ERR;

	/* realloc() keeps the old block alive when it fails, so the vector is
	 * left untouched in that case. */
	struct file_data **tmp = realloc(self->data, new_capacity * sizeof(struct file_data *));

	if (tmp == NULL)
		return FDATA_ALLOC_ERR;

	self->data     = tmp;
	self->capacity = new_capacity;

	return FDATA_SUCCESS;
}

void fdata_ptr_vector_free(struct fdata_ptr_vector* self)
{
	if (self == NULL)
		return;

	/* The records belong to an owning vector; only the array is freed. */
	free(self->data);

	self->data     = NULL;
	self->size 	   = 0;
	self->capacity = 0;
}

void fdata_ptr_vector_destroy(struct fdata_ptr_vector* self)
{
	if (self == NULL)
		return;

	for (size_t i = 0; i < self->size; i++)
	{
		file_data_destroy(self->data[i]);
	}

	fdata_ptr_vector_free(self);
}
