/*
 * This file is a product of Sun Microsystems, Inc. and is provided for
 * unrestricted use provided that this legend is included on all tape
 * media and as a part of the software program in whole or part.  Users
 * may copy or modify this file without charge, but are not authorized to
 * license or distribute it to anyone else except as part of a product
 * or program developed by the user.
 *
 * THIS FILE IS PROVIDED AS IS WITH NO WARRANTIES OF ANY KIND INCLUDING THE
 * WARRANTIES OF DESIGN, MERCHANTIBILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE, OR ARISING FROM A COURSE OF DEALING, USAGE OR TRADE PRACTICE.
 *
 * This file is provided with no support and without any obligation on the
 * part of Sun Microsystems, Inc. to assist in its use, correction,
 * modification or enhancement.
 *
 * SUN MICROSYSTEMS, INC. SHALL HAVE NO LIABILITY WITH RESPECT TO THE
 * INFRINGEMENT OF COPYRIGHTS, TRADE SECRETS OR ANY PATENTS BY THIS FILE
 * OR ANY PART THEREOF.
 *
 * In no event will Sun Microsystems, Inc. be liable for any lost revenue
 * or profits or other special, indirect and consequential damages, even
 * if Sun has been advised of the possibility of such damages.
 *
 * Sun Microsystems, Inc.
 * 2550 Garcia Avenue
 * Mountain View, California  94043
 */

#ifndef lint
static char	sccsid[] = "@(#)gfm_load_dir.c	2.22 91/10/15 Copyright 1990 Sun Microsystems";
#endif

#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <memory.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <xview/xview.h>
#include <xview/panel.h>
#include <xview/textsw.h>
#include <xview/svrimage.h>
#include <xview/notify.h>
#include <devguide/group.h>
#include <devguide/gfm.h>

extern int	errno;

typedef struct {
	char		*filename;
	Xv_opaque	glyph;
	GFM_TYPE	type;   /* To be used for client data in the list. */
} GFM_LIST_ENTRY;

#define	STRCMP(s1, s2)	strcmp(s1 ? s1 : "", s2 ? s2 : "")

#define	GFM_LIST_INC	16
static GFM_LIST_ENTRY	*Gfm_list;
static int		Gfm_list_count;
static int		Gfm_list_lth;

static void		list_add_entry();
static void		list_batch_insert();
static void		list_reset();
static int		list_entry_cmp();
static int		is_a_dir();

/*
 * Define glyphs that we will use in the file list
 */
Server_image	Gfm_document_glyph;
Server_image	Gfm_folder_glyph;
Server_image	Gfm_dotdot_glyph;
Server_image	Gfm_executable_glyph;
Server_image	Gfm_broken_link_glyph;
Server_image	Gfm_system_doc_glyph;

static unsigned short folder_bits[] =
{
#include "gfm_folder.pr"
};

static unsigned short dotdot_bits[] =
{
#include "gfm_dotdot.pr"
};

static unsigned short document_bits[] =
{
#include "gfm_data.pr"
};

static unsigned short executable_bits[] =
{
#include "gfm_app.pr"
};

static unsigned short broken_link_bits[] =
{
#include "gfm_unknown.pr"
};

static unsigned short system_doc_bits[] =
{
#include "gfm_system.pr"
};

/*
 * Initialize all glyphs used in the scrolling list
 */
void
gfm_initialize_glyphs()
{
	if (Gfm_folder_glyph)
		return;

	Gfm_folder_glyph = (Server_image) xv_create(0, SERVER_IMAGE,
		XV_WIDTH, 	16,
		XV_HEIGHT, 	16,
		SERVER_IMAGE_BITS, 	folder_bits,
		NULL);

	Gfm_dotdot_glyph = (Server_image) xv_create(0, SERVER_IMAGE,
		XV_WIDTH, 	16,
		XV_HEIGHT, 	16,
		SERVER_IMAGE_BITS, 	dotdot_bits,
		NULL);

	Gfm_executable_glyph = (Server_image) xv_create(0, SERVER_IMAGE,
		XV_WIDTH, 	16,
		XV_HEIGHT, 	16,
		SERVER_IMAGE_BITS, 	executable_bits,
		NULL);

	Gfm_document_glyph = (Server_image) xv_create(0, SERVER_IMAGE,
		XV_WIDTH, 	16,
		XV_HEIGHT, 	16,
		SERVER_IMAGE_BITS, 	document_bits,
		NULL);

	Gfm_broken_link_glyph = (Server_image) xv_create(0, SERVER_IMAGE,
		XV_WIDTH, 	16,
		XV_HEIGHT, 	16,
		SERVER_IMAGE_BITS, 	broken_link_bits,
		NULL);

	Gfm_system_doc_glyph = (Server_image) xv_create(0, SERVER_IMAGE,
		XV_WIDTH, 	16,
		XV_HEIGHT, 	16,
		SERVER_IMAGE_BITS, 	system_doc_bits,
		NULL);
}

/*
 * gfm_load_dir
 *
 * The workhorse of this program.  It loads the directory into a newly
 * created PANEL_LIST.
 */
void
#ifdef __STDC__
gfm_load_dir(gfm_popup_objects *ip, char *dir)
#else
gfm_load_dir(ip, dir)
	gfm_popup_objects	*ip;
	char			*dir;
#endif
{
	/* Directory stuff */
	DIR		*dirp;
	struct dirent	*dp;
	struct stat	buf;
	int		l;
	int		matched;
	int		chunk_len;
	char		*s;
	char		*chunk_start = NULL;
	char		*chunk_stop = NULL;
	char		*path_rest = NULL;
	char		*path_end = NULL;
	char		new_dir[MAXPATHLEN + 1];
	char		path_prefix[MAXPATHLEN + 1];
	char		sym_link[MAXPATHLEN + 1];
	char		*current_dir;
	GFM_PRIVATE	*gfm_private;

	memset(new_dir, '\0', sizeof(new_dir));
	memset(path_prefix, '\0', sizeof(path_prefix));

	xv_set(ip->popup, FRAME_BUSY, TRUE, NULL);
	gfm_private = (GFM_PRIVATE *) xv_get(ip->popup, XV_KEY_DATA, GFM_KEY);
	current_dir = (char *)xv_get(ip->directory, PANEL_VALUE);

	/*
	 * Check to see if new directory is absolute or relative
	 */
	expand_path(dir, new_dir);
	path_rest = new_dir;

	/*
	 * Remove trailing "/" that expand path might add on
	 */
	if (((l = strlen(new_dir) - 1) > 1) && (new_dir[l] == '/'))
		new_dir[l] = '\0';

	if (*new_dir == '/')
		strcpy(path_prefix, "");
	else
		strcpy(path_prefix, current_dir);

	/*
	 * Incrementally add directory segments
	 */
	for (chunk_start = new_dir;
	     chunk_start <= (new_dir + strlen(new_dir));
	     chunk_start = path_rest = chunk_stop + 1) {

		if (!(chunk_stop = strchr(path_rest, '/'))) {
			chunk_stop = path_rest + strlen(path_rest);
		} else if (chunk_stop == path_rest)
			continue;

		chunk_len = chunk_stop - chunk_start;

		/* Ignore embedded "." and "/" */
		if ((strncmp(chunk_start, ".", chunk_len) == 0) ||
		    (strncmp(chunk_start, "/", chunk_len) == 0))
			continue;

		/* Remove one directory from path */
		if (strncmp(chunk_start, "..", chunk_len) == 0) {
			if (s = strrchr(path_prefix, '/'))
				*s = '\0';
			continue;
		}

		l = strlen(path_prefix);
		path_end = path_prefix + l;
		if ((l == 0) || (path_prefix[l - 1] != '/'))
			strcat(path_prefix, "/");
		strncat(path_prefix, chunk_start, chunk_len);

		if (!is_a_dir(path_prefix)) {
			*path_end = '\0';
			break;
		}
	}

	if (strcmp(path_prefix, "") == 0)
		strcat(path_prefix, "/");

	if (!path_rest || !*path_rest || path_rest > (new_dir + strlen(new_dir)))
		path_rest = NULL;

	/*
	 * If there were any remaining portions, put them in the file
	 * field.  If it contained a /, beep.
	 * XXX - In V3 we should select this but we can't because the
	 * window must have focus for the select to take effect.
	 */
	if (path_rest && *path_rest) {
		if (strchr(path_rest, '/'))
			xv_set(ip->popup, WIN_ALARM, NULL);
		xv_set(ip->file, PANEL_VALUE, path_rest, NULL);
	} else
		xv_set(ip->file, PANEL_VALUE, "", NULL);

	if (stat(path_prefix, &buf) < 0)
	{
		fprintf(stderr,
			xv_dgettext("libguidexv",
					 "gfm: Could not \"stat\" directory %s\n"),
			path_prefix);
		xv_set(ip->popup, FRAME_BUSY, FALSE, NULL);
		return;
	}

	/*
	 * Return if we are loading the same directory and the directory
	 * hasn't been modified.
	 */
	if (current_dir && *current_dir &&
	    (strcmp(path_prefix, current_dir) == 0) &&
	    (buf.st_mtime == gfm_private->dir_mtime)) {
		xv_set(ip->popup, FRAME_BUSY, FALSE, NULL);
		return;
	}

	gfm_private->dir_mtime = buf.st_mtime;

	/*
	 * Open the directory for reading, then set the directory field
	 */
	if ((dirp = opendir(path_prefix)) == NULL)
	{
		fprintf(stderr, xv_dgettext("libguidexv",
					 "gfm: Could not open directory %s\n"),
			path_prefix);
		xv_set(ip->popup, FRAME_BUSY, FALSE, NULL);
		return;
	}
	xv_set(ip->directory, PANEL_VALUE, path_prefix, NULL);

	/*
	 * Set up up path prefix since we don't * do a chdir().  All our
	 * stats will need to be absolute...
	 */
	if (path_prefix[strlen(path_prefix) - 1] != '/') {
		strcat(path_prefix, "/");
	}
	path_end = path_prefix + strlen(path_prefix);

	if (gfm_private->filter_pattern)
		gfm_compile_regex(gfm_private->filter_pattern);

	while (dp = readdir(dirp)) {
		if (strcmp(dp->d_name, "..") == 0) {
			list_add_entry(ip, gfm_private,
				       GFM_DOTDOT_STR, GFM_FOLDER);
			continue;
		}

		/* Don't add '.' */
		if (strcmp(dp->d_name, ".") == 0)
			continue;

		if (!gfm_private->show_dotfiles && (*(dp->d_name) == '.'))
			continue;

		strcpy(path_end, dp->d_name);

		/* Error!! */
		if (stat(path_prefix, &buf) < 0) {
			if (errno == ENOENT)
				list_add_entry(ip, gfm_private,
					       dp->d_name, GFM_BROKENLINK);
			continue;
		}

		if ((buf.st_mode & S_IFMT) == S_IFDIR) {
			list_add_entry(ip, gfm_private, dp->d_name, GFM_FOLDER);
			continue;
		}

		if (gfm_private->filter_pattern)
			matched = gfm_match_regex(path_end);
		else
			matched = TRUE;

		if (gfm_private->filter_callback) {
			if (gfm_private->filter_callback(ip, path_prefix) && matched)
				list_add_entry(ip, gfm_private,
					       dp->d_name, GFM_USERDEF);
			continue;
		} else if (gfm_private->filter_pattern) {
			if (matched)
				list_add_entry(ip, gfm_private,
					       dp->d_name, GFM_USERDEF);
			continue;
		}

		/*
		 * We can't just and the mode with S_IFMT
		 * because executable permissions are in a different
		 * group of bits.  We shift the mask to also
		 * take the cases of only owner, group or others
		 * have permission to execute.
		 */
		if (strcmp(dp->d_name, "core") == 0) {
			list_add_entry(ip, gfm_private,
				       dp->d_name, GFM_SYSDOC);
			continue;
		}

		if (buf.st_mode & S_IEXEC) {
			list_add_entry(ip, gfm_private,
				       dp->d_name, GFM_APPLICATION);
			continue;
		}

		switch (buf.st_mode & S_IFMT) {
		case S_IFREG:
			list_add_entry(ip, gfm_private,
				       dp->d_name, GFM_DOCUMENT);
			break;
		case S_IFDIR:
			list_add_entry(ip, gfm_private,
				       dp->d_name, GFM_FOLDER);
			break;
		case S_IFCHR:
		case S_IFIFO:
		case S_IFBLK:
		case S_IFSOCK:
			list_add_entry(ip, gfm_private,
				       dp->d_name, GFM_SYSDOC);
			break;
		case S_IFLNK:
			if ((l = readlink(dp->d_name, sym_link, sizeof (sym_link))) > 0) {
				sym_link[l] = '\0';
				if (stat(sym_link, &buf) < 0) {
					/* Error.  Broken link. */
					list_add_entry(ip, gfm_private,
						       dp->d_name, GFM_BROKENLINK);
					continue;
				}
				if (buf.st_mode & (S_IEXEC | (S_IEXEC>>1) | (S_IEXEC>>2))) {
					list_add_entry(ip, gfm_private,
						       dp->d_name, GFM_APPLICATION);
					continue;
				}
				switch (buf.st_mode & S_IFMT) {
				case S_IEXEC:
					list_add_entry(ip, gfm_private,
						       dp->d_name, GFM_APPLICATION);
					break;
				case S_IFREG:
					list_add_entry(ip, gfm_private,
						       dp->d_name, GFM_DOCUMENT);
					break;
				case S_IFDIR:
					list_add_entry(ip, gfm_private,
						       dp->d_name, GFM_FOLDER);
					break;
				case S_IFCHR:
				case S_IFIFO:
				case S_IFBLK:
				case S_IFSOCK:
				case S_IFLNK:
					list_add_entry(ip, gfm_private,
						       dp->d_name, GFM_SYSDOC);
					break;
				default:
					list_add_entry(ip, gfm_private,
						       dp->d_name, GFM_BROKENLINK);
				}
			}
			break;
		default:
			break;
		}
	}

	closedir(dirp);

	/*
	 * Sort the list, start sorting at the second position so that ".."
	 * is always at the top.
	 */
	qsort((char *)&Gfm_list[1], Gfm_list_count-1, sizeof (GFM_LIST_ENTRY), list_entry_cmp);

	list_batch_insert(ip, gfm_private);
	list_reset(ip, gfm_private);
	xv_set(ip->popup, FRAME_BUSY, FALSE, NULL);
}

/*
 * Comparison routine for quicksort of list entries in Gfm_list
 */
static int
#ifdef __STDC__
list_entry_cmp(GFM_LIST_ENTRY *entry1, GFM_LIST_ENTRY *entry2)
#else
list_entry_cmp(entry1, entry2)
	GFM_LIST_ENTRY	*entry1;
	GFM_LIST_ENTRY	*entry2;
#endif
{
	return STRCMP(entry1->filename, entry2->filename);
}

/*
 * Add and entry to the list
 */
/*ARGSUSED*/
static void
#ifdef __STDC__
list_add_entry(gfm_popup_objects *ip, GFM_PRIVATE *gfm_private,
	       char *name, GFM_TYPE type)
#else
list_add_entry(ip, gfm_private, name, type)
	gfm_popup_objects *ip;
	GFM_PRIVATE	  *gfm_private;
	char		  *name;
	GFM_TYPE	  type;
#endif
{
	if (Gfm_list_count == Gfm_list_lth)
	{
		if (Gfm_list)
			Gfm_list = (GFM_LIST_ENTRY *)
				realloc(Gfm_list,
					(Gfm_list_lth += GFM_LIST_INC) *
					sizeof (GFM_LIST_ENTRY));
		else
			Gfm_list = (GFM_LIST_ENTRY *)
				malloc((Gfm_list_lth += GFM_LIST_INC) *
				       sizeof(GFM_LIST_ENTRY));
	}

	Gfm_list[Gfm_list_count].filename = strdup(name);
	Gfm_list[Gfm_list_count].type = type;

	switch (type) {
	case GFM_FOLDER:
		if (strcmp(name, (char *)GFM_DOTDOT_STR) == 0)
			Gfm_list[Gfm_list_count].glyph = Gfm_dotdot_glyph;
		else
			Gfm_list[Gfm_list_count].glyph = Gfm_folder_glyph;
		break;
	case GFM_APPLICATION:
		Gfm_list[Gfm_list_count].glyph = Gfm_executable_glyph;
		break;
	case GFM_BROKENLINK:
		Gfm_list[Gfm_list_count].glyph = Gfm_broken_link_glyph;
		break;
	case GFM_DOCUMENT:
		Gfm_list[Gfm_list_count].glyph = Gfm_document_glyph;
		break;
	case GFM_USERDEF:
		Gfm_list[Gfm_list_count].glyph = gfm_private->user_glyph;
		break;
	case GFM_SYSDOC:
		Gfm_list[Gfm_list_count].glyph = Gfm_system_doc_glyph;
		break;
	default:
		Gfm_list[Gfm_list_count].glyph = 0;
		Gfm_list[Gfm_list_count].type = GFM_BROKENLINK;
	}

	Gfm_list_count++;
}

/*
 * Empty current list of all it's contents
 */
/*ARGSUSED*/
static void
#ifdef __STDC__
empty_list(gfm_popup_objects *ip, GFM_PRIVATE *gfm_private)
#else
empty_list(ip, gfm_private)
	gfm_popup_objects	*ip;
	GFM_PRIVATE		*gfm_private;
#endif
{
	int		width;
	int		nrows;
	Xv_opaque	old_list;

	/*
	 * Destroy old list (faster than deleting all entries at the moment)
	 * then recreate it.  Note, we must hide it first to get around
	 * XView bugid 1042729.
	 */
	old_list = ip->list;
	width = (int)xv_get(ip->list, PANEL_LIST_WIDTH);
	nrows = (int)xv_get(ip->list, PANEL_LIST_DISPLAY_ROWS);
	xv_set(ip->list, XV_SHOW, FALSE, NULL);
	xv_destroy_safe(ip->list);
	ip->list = gfm_popup_list_create(ip, ip->controls);

	xv_set(ip->list,
		PANEL_LIST_WIDTH, width,
		PANEL_LIST_DISPLAY_ROWS, nrows,
		NULL);

	/*
	 * Make sure to tell the GROUP pkg that we have replaced the
	 * list with a new one.
	 */
	xv_set(ip->file_list_group,
		GROUP_REPLACE_MEMBER, old_list, ip->list,
		NULL);
}

/*
 * Reset internal list.
 *	N.B. Space for list array is not freed, kept around for next time
 */
/*ARGSUSED*/
static void
#ifdef __STDC__
list_reset(gfm_popup_objects *ip, GFM_PRIVATE *gfm_private)
#else
list_reset(ip, gfm_private)
	gfm_popup_objects	*ip;
	GFM_PRIVATE		*gfm_private;
#endif
{
	int	i;

	for (i = 0; i < Gfm_list_count; i++)
		free(Gfm_list[i].filename);
	Gfm_list_count = 0;
}

/*
 * Insert internal list into scrolling list in chunks.
 */
static void
#ifdef __STDC__
list_batch_insert(gfm_popup_objects *ip, GFM_PRIVATE *gfm_private)
#else
list_batch_insert(ip, gfm_private)
	gfm_popup_objects	*ip;
	GFM_PRIVATE		*gfm_private;
#endif
{
	int	i = 0;
	void	*attr_array[ATTR_STANDARD_SIZE];
	void	**attr_ptr;
	void	**attr_end;

	empty_list(ip, gfm_private);
	xv_set(ip->list, XV_SHOW, FALSE, NULL);

	attr_ptr = attr_array;
	attr_end = &attr_array[ATTR_STANDARD_SIZE - 20];

	for (i = 0; i < Gfm_list_count; i++) {
		*attr_ptr++ = (void *) PANEL_LIST_INSERT;
		*attr_ptr++ = (void *) i;

		*attr_ptr++ = (void *) PANEL_LIST_STRING;
		*attr_ptr++ = (void *) i;
		*attr_ptr++ = (void *) Gfm_list[i].filename;

		*attr_ptr++ = (void *) PANEL_LIST_GLYPH;
		*attr_ptr++ = (void *) i;
		*attr_ptr++ = (void *) Gfm_list[i].glyph;

		*attr_ptr++ = (void *) PANEL_LIST_CLIENT_DATA;
		*attr_ptr++ = (void *) i;
		*attr_ptr++ = (void *) (Gfm_list[i].type | (i << 3));

		/*
		 * Check for overflow or completion, if full terminate
		 * the ATTR_LIST and set it on the scrolling list.
		 */
		if (attr_ptr >= attr_end || i >= Gfm_list_count - 1) {
			*attr_ptr = (void *) 0;
			xv_set(ip->list, ATTR_LIST, attr_array, NULL);
			attr_ptr = attr_array;
		}
	}

	/*
	 * Select first item and make list visible
	 */
	xv_set(ip->list,
	       PANEL_LIST_SELECT, 0,	TRUE,
	       PANEL_CLIENT_DATA,	GFM_FOLDER,
	       XV_SHOW,		TRUE,
	       NULL);
}

/*
 * Is path name really a directory
 */
static int
#ifdef __STDC__
is_a_dir(char *path)
#else
is_a_dir(path)
	char	*path;
#endif
{
	struct stat	buf;

	if ((stat(path, &buf) < 0) || !((buf.st_mode & S_IFMT) == S_IFDIR))
		return FALSE;
	else
		return TRUE;
}
