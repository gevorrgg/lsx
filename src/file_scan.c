/**
 * @file file_scan.c
 * @brief Reading file metadata and directory contents (see file_scan.h).
 */
#include <lsx/file_scan.h>
#include <file_data.h>
#include <lsx/print/format_info.h>
#include <lsx/error.h>
#include <lsx/flags/flags.h>
#include <dirent.h>
#include <file_vector.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

/**
 * @brief Decide whether a directory entry is hidden by the current -a/-A mode.
 *
 * "." and ".." are shown only with -a; other dot-files with -a or -A.
 */
static bool should_skip(const char *file_name, const struct lsx_flags* flags)
{
    if (strcmp(file_name, ".") == 0 || strcmp(file_name, "..") == 0)
    {
        return flags->hidden != SHOW_ALL;
    }

    if (file_name[0] == '.')
        return flags->hidden == HIDE_DEFAULT;

    return false;
}

lsx_error lsx_get_file_info(struct file_data **out, const char *path, const char *name, bool follow_symlinks)
{
    struct stat st;
    struct stat lst;
	int stat_succeed = 1;

    *out = NULL;

	/* lstat() describes the entry itself; it must succeed for the entry to exist. */
	if (lstat(path, &lst) != 0)
	{
		return LSX_STAT_ERR;
	}

	/* stat() follows symlinks. It may fail for a dangling link; in that case
	 * the record is still built with zeroed target data and stat_status = 0. */
    if (stat(path, &st) != 0)
	{
		stat_succeed = 0;
		memset(&st, 0, sizeof(st));
	}

    *out = file_data_create(&st, &lst, name, path, stat_succeed, follow_symlinks);

    if (*out == NULL)
    {    
		return LSX_ALLOC_ERR;
	}

    /* A dangling link is only an error when the caller wants the target (-L). */
    return follow_symlinks ? (stat_succeed ? LSX_SUCCESS : LSX_STAT_ERR) :  LSX_SUCCESS;
}

/**
 * @brief True for arguments that are flags rather than operands.
 *
 * "-" alone (conventionally stdin) and "--" are not flags.
 */
static bool is_flag(const char *arg)
{
    if (strcmp(arg, "--") == 0)
        return false;

    if (strcmp(arg, "-") == 0)

        return false;

    return arg[0] == '-';
}

lsx_error lsx_retrieve_files(struct fdata_ptr_vector *files,
                           int argc,
                           const char *argv[],
						   bool follow_symlinks,
						   int *file_args)
{
    bool read_files = false;   /* set after "--": everything is an operand */
    bool seen_args  = false;   /* at least one operand was given */

	int args_count = 0;

    for (int i = 0; i < argc; i++)
    {
        if (strcmp(argv[i], "--") == 0)
        {
            read_files = true;
            continue;
        }

        if (is_flag(argv[i]) && !read_files)
            continue;  // skip flags

		args_count += 1;

        struct file_data *fdata;

        seen_args = true;

        /* Inaccessible operands are reported but do not abort the run,
         * matching "ls: cannot access 'x': No such file or directory". */
        if (lsx_get_file_info(&fdata, argv[i], argv[i], follow_symlinks) != LSX_SUCCESS)
        {
            fprintf(stderr, "ls: cannot access '%s': %s\n", argv[i], strerror(errno));
            file_data_destroy(fdata);   /* NULL unless it was a dangling link under -L */
            continue;
        }

        /* The vector takes ownership of the record. */
        const fdata_err append_status = fdata_ptr_vector_append(files, fdata);

        if (append_status != FDATA_SUCCESS)
        {
			if (file_args)
			{
				*file_args = args_count;
			}

            file_data_destroy(fdata);

            return LSX_BUFFER_ERR;
        }
    }

    if (!seen_args) // no file arguments
    {
        /* Plain "ls" lists the current directory. */
		args_count += 1;
		
		if (file_args)
		{
			*file_args = args_count;
		}

        struct file_data *fdata;

        lsx_error finfo_status = lsx_get_file_info(&fdata, ".", ".", follow_symlinks);

        if (finfo_status != LSX_SUCCESS)
        {
            file_data_destroy(fdata);

            return finfo_status;
        }

        if (fdata_ptr_vector_append(files, fdata) != FDATA_SUCCESS)
        {
            file_data_destroy(fdata);

            return LSX_BUFFER_ERR;
        }
    }

	if (file_args)
	{
		*file_args = args_count;
	}

    return LSX_SUCCESS;
}

char *lsx_make_path(const char *dir, const char *name)
{
    if (dir == NULL || name == NULL)
        return NULL;

    size_t dir_len = strlen(dir);

    if (dir_len == 0)
    {
        dir = ".";
        dir_len = 1;
    }

    size_t buf_len = dir_len + strlen(name) + 1;   /* +1 for the terminator */
    const char last_char = dir[dir_len - 1];
    const bool need_slash = last_char != '/';

    if (need_slash)
    {
        buf_len += 1;
    }

    char *buf = malloc(sizeof(char) * buf_len);

    if (buf == NULL)
        return NULL;

    snprintf(buf, buf_len, "%s%s%s", dir, need_slash ? "/" : "", name);

    return buf;
}

lsx_error lsx_read_directory(struct fdata_ptr_vector *files, 
						   const char *directory,
						   const struct lsx_flags *flags)
{
        DIR *dir = opendir(directory);

        if (dir == NULL)
        {
			    fprintf(stderr, "ls: cannot open directory '%s': %s\n", directory, strerror(errno));

                return LSX_FILE_NOT_FOUND;
        }

        struct dirent *dent;

        /* readdir() returns entries in file-system order; sorting is done
         * later on the vector, not here. */
        while ((dent = readdir(dir)) != NULL)
        {
                const char *name = dent->d_name;

                if (should_skip(name, flags))
                    continue;  // skip unnecessary files

                /* stat() needs the full path; the record keeps only the name
                 * for display plus the path for later readlink() calls. */
				char *path = lsx_make_path(directory, name);

				if (path == NULL)
				{
					closedir(dir);

					return LSX_ALLOC_ERR;
				}
		
				struct file_data *fdata;

                const lsx_error stat_status = lsx_get_file_info(&fdata, path, name, flags->follow_symlinks);

                /* A dangling symlink under -L is reported but still listed
                 * (its fields are printed as "?" by the long printer). An
                 * entry that vanished between readdir() and lstat() yields
                 * no record and is skipped. */
                if (stat_status != LSX_SUCCESS)
                {
					fprintf(stderr, "ls: cannot access '%s': %s\n", name, strerror(errno));

                    if (fdata == NULL)
                    {
                        free(path);
                        continue;
                    }
                }

                /* The vector takes ownership of the record. */
                const fdata_err append_status = fdata_ptr_vector_append(files, fdata);

                if (append_status != FDATA_SUCCESS)
				{	
						free(path);
						closedir(dir);
						file_data_destroy(fdata);

                        return LSX_BUFFER_ERR;
				}

				free(path);
        }

		closedir(dir);

        return LSX_SUCCESS;
}

lsx_error lsx_get_fdata_span(struct fdata_ptr_vector *span,
                           const struct fdata_ptr_vector *source,
                           const struct lsx_flags *flags,
                           bool (*filter)(const struct file_data *file, const struct lsx_flags *flags))
{
    for (size_t i = 0; i < source->size; i++)
    {
        if (!filter || filter(source->data[i], flags))
        {
            const fdata_err append_status = fdata_ptr_vector_append(span, source->data[i]);

            if (append_status != FDATA_SUCCESS)
                return LSX_BUFFER_ERR;
        }
    }

    return LSX_SUCCESS;
}
