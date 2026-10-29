/*      @(#)flist_impl.h 1.11 93/06/28 SMI      */

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE
 *	file for terms of the license.
 */


#include <sys/param.h>
#include <dirent.h>
#if defined(__linux) || defined(__APPLE__)
#define __USE_GNU
#include <regex.h>
#undef __USE_GNU
#endif
#include <xview_private/xv_path_util.h>
#include <xview/file_list.h>
#include <xview_private/i18n_impl.h>


typedef struct {
    Xv_opaque		public_self; 		/* public handle */
    Panel		owner;			/* parent panel */
    Frame		frame;			/* parent frame */
    char *		directory; 		/* validated directory name */
    DIR *		dir_ptr;		/* cached directory pointer */
    char *		previous_dir;		/* last directory displayed */
    char *		regex_pattern; 		/* regular expression */
#if !defined(__linux) && !defined(__APPLE__)
    char *		regex_compile;		/* compiled form of regex */
#else
    regex_t *           regex_compile;          /* compiled form of regex */
    int                 regcomp_return_val;     /* return val from regcomp */
#endif
    Server_image	match_glyph;		/* matched regex glyph */
    Server_image	match_glyph_mask;	/* matched regex glyph mask */
    unsigned short	filter_mask;		/* mask for filter_func */
    File_list_op	(* client_filter)();	/* filter func */
    int			(* client_notify)();	/* client's PANEL_NOTIFY_PROC */
    int			(* compare_func)(); 	/* qsort function */
    int			(* cd_func)();		/* tell client we are changing dir */
    char *		dotdot_string;		/* string displayed for ".." entry */
    Server_image	file_glyph; 		/* file glyph */
    Server_image	directory_glyph;	/* folder glyph */
    Server_image	dotdot_glyph;		/* dotdot_glyph */
    struct {
	unsigned int	show_dotfiles : 1;	/* show "." file flag */
	unsigned int	auto_update : 1;	/* auto update list when attr changes */
	unsigned int	show_dir : 1;		/* show directory in title */
	unsigned int	abbrev_view : 1;	/* gray out or ignore invalid entries */
	unsigned int	use_frame : 1;		/* can i use the current frame? */
	unsigned int	created : 1;		/* reached xv_end_create */
	unsigned int	new_dir : 1;		/* opened a new directory */
#ifdef OW_I18N
	unsigned	wchar_notify : 1; 	/* convert to wide char */
	unsigned	wchar_list_notify : 1;  /* PANEL_NOTIFY_WCS was set */
#endif
    } f;

#ifdef OW_I18N
    wchar_t *		directory_wcs;
    wchar_t *		regex_pattern_wcs;
    wchar_t *		dotdot_string_wcs;
#endif /* OW_I18N */
} File_list_private;


#define FILE_LIST_PUBLIC(item)	XV_PUBLIC(item)
#define FILE_LIST_PRIVATE(item)	XV_PRIVATE(File_list_private, File_list_public, item)


/* OL specified defaults */
#define DEFAULT_DOTDOT_STRING	XV_MSG("...Go up one folder...")   
#define DEFAULT_LIST_ROWS	10
#define MIN_LIST_ROWS		3


/*
 * Number of File_list_row structures to allocate 
 * at a single time.
 */
#define ALLOC_INCR		24

