/*
 * @(#)gfm.h	2.18 91/10/15 Copyright 1990 Sun Microsystems
 *
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

#ifndef guide_gfm_DEFINED
#define guide_gfm_DEFINED

#include	<sys/types.h>
#include	<devguide/gfm_ui.h>
#include	<devguide/c_varieties.h>

#define GFM_OK		0
#define	GFM_ERROR	1
#define	GFM_TYPE_MASK	0x00007
#define GFM_DOTDOT_STR	xv_dgettext("libguidexv", ".. (Go up a level)")
#define	GFM_KEY		12345

typedef enum {
	GFM_LOAD,
	GFM_SAVE,
	GFM_CREATE,
	GFM_DEFAULT
} GFM_MODE;

typedef enum {
	GFM_FOLDER	= 0,
	GFM_APPLICATION	= 1,
	GFM_BROKENLINK	= 2,
	GFM_DOCUMENT	= 3,
	GFM_USERDEF	= 4,
	GFM_SYSDOC	= 5
} GFM_TYPE;

typedef struct {
	char		*filter_pattern;
	int		(*filter_callback)();
	int		show_dotfiles;
	int		height;
	int		width;
	time_t		dir_mtime;
	int		initial_win_height;
	int		initial_list_height;
	int		initial_list_excess;
	Server_image	user_glyph;
	GFM_MODE	mode;
	int		(*callback)();
} GFM_PRIVATE;

EXTERN_FUNCTION( gfm_popup_objects *gfm_initialize,	(gfm_popup_objects *, Xv_opaque, char *) );
EXTERN_FUNCTION( void	gfm_activate,		(gfm_popup_objects *, char *, char *, int (*)(gfm_popup_objects *, char *), int (*)(gfm_popup_objects *, char *, char *), Xv_opaque, GFM_MODE) );
EXTERN_FUNCTION( void	gfm_show_dotfiles,	(gfm_popup_objects *, int) );
EXTERN_FUNCTION( void	gfm_set_action,		(gfm_popup_objects *, char *) );
EXTERN_FUNCTION( void	gfm_compile_regex,	(char *) );
EXTERN_FUNCTION( int	gfm_match_regex,	(char *) );

#endif /* guide_gfm_DEFINED */
