/**
 * @file file_data.c
 * @brief Construction, destruction and type queries for struct file_data
 *        (see file_data.h).
 */
#include "../include/file_data.h"

#include <string.h>
#include <pwd.h>
#include <grp.h>
#include <stdlib.h>

struct file_data *file_data_create(const struct stat *st,
                                   const struct stat *lst,
                                   const char *name,
                                   const char *path,
                                   int stat_status,
                                   bool follow_symlinks)
{
	if (st == NULL || lst == NULL || name == NULL || path == NULL)
		return NULL;

	struct file_data *self = malloc(sizeof(*self));

	if (self == NULL)
		return NULL;

	/* Start fully NULL so a partial failure can be unwound with
	 * file_data_destroy(), which tolerates NULL strings. */
	self->name  = NULL;
	self->path  = NULL;
	self->uname = NULL;
	self->gname = NULL;

	/* Every string is duplicated so the record outlives its sources
	 * (dirent buffers, argv, temporary paths). */
	self->name = strdup(name);
	self->path = strdup(path);

	if (self->name == NULL || self->path == NULL)
	{
		file_data_destroy(self);

		return NULL;
	}

	/* Resolve owner/group from the buffer the user will see: the target
	 * under -L (when stat() worked), the link itself otherwise. */
	const struct stat *id_source = follow_symlinks ? (stat_status ? st : lst) : lst;

	struct passwd *pw = getpwuid(id_source->st_uid);
	struct group  *gr = getgrgid(id_source->st_gid);

	/* Unresolvable ids leave the name NULL; printers then fall back to
	 * the numeric value. Only a failed strdup() of a real name is an error. */
	if (pw && pw->pw_name)
	{
		self->uname = strdup(pw->pw_name);

		if (self->uname == NULL)
		{
			file_data_destroy(self);

			return NULL;
		}
	}

	if (gr && gr->gr_name)
	{
		self->gname = strdup(gr->gr_name);

		if (self->gname == NULL)
		{
			file_data_destroy(self);

			return NULL;
		}
	}

	self->st          = *st;
    self->lst         = *lst;
	self->stat_status = stat_status;

	return self;
}

void file_data_destroy(struct file_data *self)
{
	if (self == NULL)
		return;

	/* free(NULL) is a no-op, so partially built records are safe. */
	free(self->name);
    free(self->uname);
    free(self->gname);
	free(self->path);

	free(self);
}

/* Local copy of the helper in print_utils.c; kept here so this module has
 * no dependency on the printing code. */
static const struct stat *get_stat(const struct file_data *f,
                            file_data_mode mode)
{
    return mode == FILE_DATA_LINK ? &(f->lst) : &(f->st);
}

/* Type predicates. The `&& stat_status` guard makes every query false for
 * a dangling symlink inspected via FILE_DATA_TARGET, whose st is zeroed.
 * is_symlink() has no guard because S_ISLNK only makes sense on lst. */

bool is_directory(const struct file_data *file_info, file_data_mode mode)
{
    const struct stat *st = get_stat(file_info, mode);
	
    return S_ISDIR(st->st_mode) && file_info->stat_status;
}

bool is_symlink(const struct file_data *file_info, file_data_mode mode)
{
    const struct stat *st = get_stat(file_info, mode);

    return S_ISLNK(st->st_mode);
}

bool is_executable(const struct file_data *file_info, file_data_mode mode)
{
    const struct stat *st = get_stat(file_info, mode);

    return st->st_mode & (S_IXUSR | S_IXGRP | S_IXOTH) && file_info->stat_status;
}

bool is_block_device(const struct file_data *file_info, file_data_mode mode)
{
    const struct stat *st = get_stat(file_info, mode);

    return S_ISBLK(st->st_mode) && file_info->stat_status;
}
bool is_character_device(const struct file_data *file_info, file_data_mode mode)
{
    const struct stat *st = get_stat(file_info, mode);

    return S_ISCHR(st->st_mode) && file_info->stat_status;
}
bool is_pipe(const struct file_data *file_info, file_data_mode mode)
{
    const struct stat *st = get_stat(file_info, mode);

    return S_ISFIFO(st->st_mode) && file_info->stat_status;
}

bool is_socket(const struct file_data *file_info, file_data_mode mode)
{
    const struct stat *st = get_stat(file_info, mode);

    return S_ISSOCK(st->st_mode) && file_info->stat_status;
}

