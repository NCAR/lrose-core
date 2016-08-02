#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)fs_impl.h 20.19 93/06/28";
#endif
#endif

/* 
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE 
 *	file for terms of the license.
 */

#ifndef _fullscreen_impl_h_already_included
#define _fullscreen_impl_h_already_included

#include <xview/pkg.h>
#include <xview/window.h>
#include <xview/base.h>
#include <xview/fullscreen.h>

#define fullscreen_attr_next(attr) (Fullscreen_attr *)attr_next((caddr_t *)attr)

typedef struct {
    Fullscreen		public_self;	/* back pointer */
    Xv_Window		root_window;	/* screen's root window */
    Xv_Window		input_window;	/* event input window */
    Xv_Window		cursor_window;	/* cursor confine-to window */
    Inputmask		inputmask;      /* current mask */
    Inputmask		cached_im;	/* saved inputmask */
    int			im_changed;	/* whether im was set */
    Xv_Cursor		cursor;		/* current cursor */
    int			sync_mode_now;  /* current grab mode */
    int			grab_pointer;   /* whether pointer is grabbed */
    int			grab_kbd;	/* whether keyboard is grabbed */
    int 		grab_server; 	/* whether server is grabbed */
    Fullscreen_grab_mode pointer_ptr_mode;  /* pointer mode for pointer */
    Fullscreen_grab_mode pointer_kbd_mode;  /* kbd mode for pointer */
    Fullscreen_grab_mode keyboard_ptr_mode; /* pointermode for keyboard */ 
    Fullscreen_grab_mode keyboard_kbd_mode; /* kbd mode for keyboard */ 
    int			owner_events;	/* mode for event reporting */
} Fullscreen_info;

#define	FULLSCREEN_PRIVATE(fullscreen)	\
	XV_PRIVATE(Fullscreen_info, Xv_fullscreen, fullscreen)
#define	FULLSCREEN_PUBLIC(fullscreen)	XV_PUBLIC(fullscreen)

/* fullscreen_get.c */
Pkg_private Xv_opaque		fullscreen_get_attr();

/* fullscreen_set.c */
Pkg_private Xv_opaque		fullscreen_set_avlist();

/* fullscreen_data.c */
Pkg_private int			fullscreen_init_internal();
Pkg_private int 		fullscreen_destroy_internal();

#endif
