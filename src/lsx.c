/**
 * @file lsx.c
 * @brief Top-level control flow of lsx.
 *
 * Responsibilities:
 *  - parse flags and collect operands;
 *  - print non-directory operands as one listing;
 *  - print each directory operand, recursing with -R;
 *  - detect directory loops during recursion (a symlink pointing at an
 *    ancestor) using a "visited" set keyed by device+inode.
 *
 * Memory ownership: every listing builds one owning fdata_ptr_vector of
 * records. Where a subset is needed (operands split into files and
 * directories) a non-owning view is built over it and released first.
 */
#include <stdbool.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <sys/stat.h>
#include <lsx.h>
#include <lsx/file_scan.h>
#include <lsx/print/format_info.h>
#include <lsx/flags/flags.h>
#include <lsx/sorting.h>
#include <lsx/print/print.h>
#include <file_vector.h>
#include <lsx/flags/flag_parser.h>
#include <lsx/flags/flag_setters.h>


/** @brief True unless -U / -f disabled sorting. */
static bool should_sort(const struct lsx_flags* flags)
{
    return flags->sort != SORT_NONE;
}

/* ------------------------------------------------------------------------
 * Visited set for -R loop detection.
 *
 * A directory is identified by the device and inode of its lstat() result,
 * i.e. by the path through which it was listed. Each entry is compared by
 * its own lstat() data, so a directory reached again through the same kind
 * of path is recognised and skipped.
 * --------------------------------------------------------------------- */

/** @brief Identity of a listed directory. */
struct visited_entry
{
    dev_t dev;
    ino_t ino;
};

/** @brief Growable array of visited_entry. */
struct visited_set
{
    struct visited_entry *data;
    size_t size;
    size_t capacity;
};

static void visited_init(struct visited_set *set)
{
    set->data     = NULL;
    set->size     = 0;
    set->capacity = 0;
}

static void visited_free(struct visited_set *set)
{
    free(set->data);
    visited_init(set);
}

/**
 * @brief Record @p directory in the visited set.
 *
 * @return LSX_SUCCESS, LSX_STAT_ERR if the path cannot be lstat()-ed,
 *         LSX_ALLOC_ERR if the array could not grow.
 */
static lsx_error visited_append(struct visited_set *set, const char *directory)
{
    struct stat lst;

    if (lstat(directory, &lst) != 0)
    {
        return LSX_STAT_ERR;
    }

    if (set->size == set->capacity)
    {
        const size_t new_capacity = set->capacity == 0 ? 8 : set->capacity * 2;
        struct visited_entry *tmp = realloc(set->data, new_capacity * sizeof(*tmp));

        if (tmp == NULL)
        {
            return LSX_ALLOC_ERR;
        }

        set->data     = tmp;
        set->capacity = new_capacity;
    }

    set->data[set->size].dev = lst.st_dev;
    set->data[set->size].ino = lst.st_ino;
    set->size += 1;

    return LSX_SUCCESS;
}

/** @brief True if an entry with the same lstat() identity was listed before. */
static bool visited_contains(const struct visited_set *set, const struct file_data *file_info)
{
    for (size_t i = 0; i < set->size; i++)
    {
        if (set->data[i].dev == file_info->lst.st_dev &&
            set->data[i].ino == file_info->lst.st_ino)
        {
            return true;
        }
    }

    return false;
}

/**
 * @brief Decide whether -R should descend into @p file_info.
 *
 * Descends only when all of the following hold:
 *  - -R is set;
 *  - the entry is not "." or "..";
 *  - the entry is a directory (a symlink to a directory counts only with -L);
 *  - the directory has not been listed yet (loop protection).
 *
 * @param is_visited Optional output, set to true when the only reason for
 *                   not recursing is that the directory was already listed.
 *                   The caller uses it to print the same diagnostic GNU ls
 *                   prints in that situation.
 */
static bool should_recurse(const struct file_data *file_info,
                           const struct lsx_flags *flags,
						   const struct visited_set *visited,
						   bool *is_visited)
{
	if (is_visited)
		*is_visited = false;

    if (!flags->is_recursive)
        return false;

    if (strcmp(file_info->name, ".") == 0 ||
        strcmp(file_info->name, "..") == 0)
        return false;

    const bool is_link = is_symlink(file_info, FILE_DATA_LINK);

    /* For a symlink we must look at the target to know if it is a directory. */
    const file_data_mode mode = is_link ? FILE_DATA_TARGET : FILE_DATA_LINK;

    if (is_link && !flags->follow_symlinks)
    {   
		return false;
	}

    if (!is_directory(file_info, mode))
    {  
		return false;
	}

	if (visited_contains(visited, file_info))
	{
		if (is_visited)
		{
			*is_visited = true;	
		}

		return false;
	}
 
   return true;
}

/**
 * @brief List one directory and, with -R, every subdirectory below it.
 *
 * Steps: read entries -> sort -> print the "dir:" header (only with -R)
 * -> print -> mark as visited -> recurse.
 *
 * @param directory Path to list.
 * @param flags     Parsed flags.
 * @param visited   Directories already listed in this -R walk.
 * @param is_first  True for the top-level call; suppresses the blank line
 *                  that otherwise separates consecutive listings.
 * @return LSX_SUCCESS, or the last non-success status from any level.
 *         A failure inside a subdirectory does not stop the walk.
 */
static lsx_error lsx_recursive(const char *directory, const struct lsx_flags *flags, struct visited_set *visited, bool is_first)
{
    struct fdata_ptr_vector files;   /* owning */

    if (fdata_ptr_vector_init(&files, 0) != FDATA_SUCCESS)
    {
        return LSX_BUFFER_ERR;
    }

    const lsx_error read_status = lsx_read_directory(&files, directory, flags);

    if (read_status != LSX_SUCCESS)
    {
        fdata_ptr_vector_destroy(&files);

        return read_status;
    }

    if (should_sort(flags))
    {
        lsx_sort_file_vector(&files, flags);
    }

    /* GNU ls prints "path:" before each directory only in recursive mode
     * (the multi-operand case is handled by the caller, lsx()). */
    if (flags->is_recursive)
    {
		if (!is_first) 
			printf("\n");

        printf("%s:\n", directory);
    }

	lsx_print_catalog(&files, flags);

    /* Mark this directory *before* descending so that a child symlink
     * pointing back here is detected. */
    const lsx_error append_status = visited_append(visited, directory);

    if (append_status != LSX_SUCCESS)
    {
        fdata_ptr_vector_destroy(&files);

        return append_status;
    }

	lsx_error lsx_status = LSX_SUCCESS;
	
    // recursively printing all directories
    if (flags->is_recursive)
    {
        /* Children are visited in sorted order so the output matches ls -R. */
        for (size_t i = 0; i < files.size; i++)
        {
            const struct file_data *file_info = files.data[i];
			bool is_visited;

            if (!should_recurse(file_info, flags, visited, &is_visited))
			{
				if (is_visited)
				{
					fprintf(stderr, "ls: %s: not listing already-listed directory\n", file_info->name);
				}

                continue;
			}

            char *next_path = lsx_make_path(directory, file_info->name);
            const lsx_error lsx_current_status = lsx_recursive(next_path, flags, visited, false);	// recursive step

			if (lsx_current_status != LSX_SUCCESS)
			{
				lsx_status = lsx_current_status;
			}

            free(next_path);
        }
    }

    fdata_ptr_vector_destroy(&files);

    return lsx_status;
}


/**
 * @brief Operand filter: everything that is printed as a plain entry.
 *
 * With -d every operand is a plain entry; otherwise only non-directories.
 */
bool filter_files(const struct file_data *file, const struct lsx_flags *flags)
{
    return !flags->enter_dir || !is_directory(file, FILE_DATA_TARGET);
}

/** @brief Operand filter: directories whose contents must be listed. */
bool filter_directories(const struct file_data *file, const struct lsx_flags *flags)
{
	return flags->enter_dir && is_directory(file, FILE_DATA_TARGET);
}

/**
 * @brief List a single directory operand with a fresh visited set.
 *
 * Each operand gets its own loop-detection state, like GNU ls.
 */
static lsx_error lsx_dir(const char *directory, const struct lsx_flags *flags)
{
    struct visited_set visited;

    visited_init(&visited);

    const lsx_error lsx_status =  lsx_recursive(directory, flags, &visited, true);

    visited_free(&visited);

    return lsx_status;
}

lsx_error lsx(const int argc, const char *argv[])
{
    /* --- 1. Flags ------------------------------------------------------ */
    const flag_setter *setters = lsx_get_flag_setters();
    char error_flag;
	
    struct lsx_flags flags;

	lsx_flags_init(&flags);

    const lsx_error parsing_status = lsx_parse_flags(&flags, argc, argv, &error_flag, setters);

    if (parsing_status != LSX_SUCCESS)
    {
        fprintf(stderr, "ls: invalid option -- '%c'\n", error_flag);

        return LSX_ERR;
    }

    /* --- 2. Operands --------------------------------------------------- */
    struct fdata_ptr_vector operands;   /* owning */

    if (fdata_ptr_vector_init(&operands, 0) != FDATA_SUCCESS)
    {
        return LSX_BUFFER_ERR;
    }

	int file_args;

    const lsx_error retrieve_status = lsx_retrieve_files(&operands, argc, argv, flags.follow_symlinks, &file_args);

    if (retrieve_status == LSX_BUFFER_ERR)
    {
        fdata_ptr_vector_destroy(&operands);

        return LSX_BUFFER_ERR;
    }

    /* Split operands into two non-owning views over the same records:
     * file_ptrs - printed as one listing right away,
     * dir_ptrs  - each listed separately (possibly recursively). */
    struct fdata_ptr_vector file_ptrs;

    if (fdata_ptr_vector_init(&file_ptrs, 0) != FDATA_SUCCESS)
    {
        fdata_ptr_vector_destroy(&operands);

        return LSX_BUFFER_ERR;
    }

    const lsx_error span_status = lsx_get_fdata_span(&file_ptrs, &operands, &flags, filter_files);

    if (span_status != LSX_SUCCESS)
    {
        fdata_ptr_vector_free(&file_ptrs);
        fdata_ptr_vector_destroy(&operands);

        return span_status;
    }

    struct fdata_ptr_vector dir_ptrs;

    if (fdata_ptr_vector_init(&dir_ptrs, 0) != FDATA_SUCCESS)
    {
        fdata_ptr_vector_free(&file_ptrs);
        fdata_ptr_vector_destroy(&operands);

        return LSX_BUFFER_ERR;
    }

    if (lsx_get_fdata_span(&dir_ptrs, &operands, &flags, filter_directories) != LSX_SUCCESS)
    {
        fdata_ptr_vector_free(&dir_ptrs);
        fdata_ptr_vector_free(&file_ptrs);
        fdata_ptr_vector_destroy(&operands);

        return LSX_BUFFER_ERR;
    }

    /* --- 3. Plain operands first --------------------------------------- */
    if (file_ptrs.size > 0)
	{	
		lsx_print_catalog(&file_ptrs, &flags);

		if (dir_ptrs.size > 0)
			printf("\n");
	}

    fdata_ptr_vector_free(&file_ptrs);

    /* --- 4. Directory operands ----------------------------------------- */
    for (size_t i = 0; i < dir_ptrs.size; i++)
    {
        const struct file_data *dir = dir_ptrs.data[i];

        /* With several operands each directory gets a "name:" header.
         * In recursive mode lsx_recursive() prints the header itself. */
        if (file_args > 1)
        {
            if (!flags.is_recursive)
            {
                printf("%s:\n", dir->name);
            }
        }

        const lsx_error lsx_dir_status = lsx_dir(dir->name, &flags);

        if (lsx_dir_status != LSX_SUCCESS)
        {
		    fdata_ptr_vector_free(&dir_ptrs);
            fdata_ptr_vector_destroy(&operands);

			return lsx_dir_status;
		}

		if (i != dir_ptrs.size - 1)
			printf("\n");
    }

    fdata_ptr_vector_free(&dir_ptrs);
    fdata_ptr_vector_destroy(&operands);

    return LSX_SUCCESS;
}

