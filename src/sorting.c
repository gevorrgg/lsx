/**
 * @file sorting.c
 * @brief qsort()-based ordering of the pointer view (see sorting.h).
 *
 * The comparator needs the flags but qsort() cannot pass user data, so the
 * flags are published through a file-scope pointer for the duration of the
 * call. This makes ls_sort_file_vector() non-reentrant, which is fine for
 * a single-threaded program.
 *
 * Direction: every base comparator sorts *ascending* (smaller size / older
 * time first) and -r negates the result. Note that GNU ls shows the
 * largest / newest entry first for -S and -t, so those two keys currently
 * come out in the opposite order from GNU ls.
 */
#include <ls/sorting.h>
#include <ls/flags/flags.h>
#include <file_data.h>
#include <file_vector.h>
#include <string.h>
#include <stdlib.h>

/* Global context for qsort */

static const struct ls_flags *g_flags = NULL;

/* Comparators */

/** @brief Three-way compare for integers: -1, 0 or 1. */
static int cmp_ll(long a, long b)
{
    return (a > b) - (a < b);
}

/** @brief Byte-wise name comparison (locale-independent, like LC_ALL=C). */
static int cmp_name(const struct file_data *a,
                    const struct file_data *b)
{
    return strcmp(a->name, b->name);
}

/** @brief By size, ties broken by name. */
static int cmp_size(const struct file_data *a,
                    const struct file_data *b)
{
    if (a->st.st_size != b->st.st_size)
        return cmp_ll(a->st.st_size, b->st.st_size);

    return cmp_name(a, b);
}

/** @brief By modification time, ties broken by name. */
static int cmp_mtime(const struct file_data *a,
                     const struct file_data *b)
{
    if (a->st.st_mtime != b->st.st_mtime)
        return cmp_ll(a->st.st_mtime, b->st.st_mtime);

    return cmp_name(a, b);
}

/** @brief By status-change time (-c), ties broken by name. */
static int cmp_ctime(const struct file_data *a,
                     const struct file_data *b)
{
    if (a->st.st_ctime != b->st.st_ctime)
        return cmp_ll(a->st.st_ctime, b->st.st_ctime);

    return cmp_name(a, b);
}

/** @brief By access time (-u), ties broken by name. */
static int cmp_atime(const struct file_data *a,
                     const struct file_data *b)
{
    if (a->st.st_atime != b->st.st_atime)
        return cmp_ll(a->st.st_atime, b->st.st_atime);

    return cmp_name(a, b);
}

/* helpers */

/**
 * @brief Pick the comparator matching g_flags->sort and g_flags->time_type.
 * @return NULL when sorting is disabled (SORT_NONE).
 */
static int (*get_base_cmp(void))
    (const struct file_data *, const struct file_data *)
{
    switch (g_flags->sort)
    {
        case SORT_SIZE:
            return cmp_size;

        case SORT_NAME:
            return cmp_name;

        case SORT_TIME:
        {
            switch (g_flags->time_type)
            {
                case TIME_ACCESS:
                    return cmp_atime;
                case TIME_CHANGE:
                    return cmp_ctime;
                case TIME_MODIFICATION:
                default:
                    return cmp_mtime;
            }
        }

        default:
            return NULL;
    }
}

/**
 * @brief qsort() adapter: the array holds `const struct file_data *`, so
 *        each argument is a pointer to a pointer.
 */
static int qsort_cmp(const void *a, const void *b)
{
    const struct file_data *x = *(const struct file_data **)a;
    const struct file_data *y = *(const struct file_data **)b;

    int (*cmp)(const struct file_data *,
               const struct file_data *) = get_base_cmp();

    if (!cmp)
        return 0;

    int res = cmp(x, y);

    return g_flags->sort_reversed ? -res : res;
}

/* public API */

void ls_sort_file_vector(struct fdata_ptr_vector *file,
                         const struct ls_flags *flags)
{
    if (!file || file->size == 0)
        return;

    g_flags = flags;

    qsort(file->data,
          file->size,
          sizeof(struct file_data *),
          qsort_cmp);

    g_flags = NULL;
}
