#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)fs_get.c 20.18 90/06/21";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#include <xview/win_input.h>
#include <xview_private/fs_impl.h>
#include <xview_private/portable.h>

/*VARARGS3*/ /*ARGSUSED*/
/* Caller turns varargs into va_list that has already been va_start'd */
Pkg_private     Xv_opaque
fullscreen_get_attr(fullscreen_public, status, attr, args)
    Fullscreen      fullscreen_public;
    int            *status;	/* initialized by caller */
    Fullscreen_attr attr;
    va_list         args;
{
    Fullscreen_info    *fullscreen = FULLSCREEN_PRIVATE(fullscreen_public);
    Xv_fullscreen      *fs_real_public = (Xv_fullscreen *)fullscreen_public;
    struct fullscreen  *fs_public =
	(struct fullscreen *)&fs_real_public->fullscreen_struct;

    switch (attr) {
      case FULLSCREEN_INPUT_WINDOW:
	return ((Xv_opaque)fullscreen->input_window);

      case FULLSCREEN_CURSOR_WINDOW:
	return ((Xv_opaque)fullscreen->cursor_window);

      case FULLSCREEN_PAINT_WINDOW:
	return ((Xv_opaque)fullscreen->root_window);

      case FULLSCREEN_GRAB_KEYBOARD:
	return ((Xv_opaque)fullscreen->grab_kbd);

      case FULLSCREEN_GRAB_POINTER:
	return ((Xv_opaque)fullscreen->grab_pointer);

      case FULLSCREEN_GRAB_SERVER:
	return ((Xv_opaque)fullscreen->grab_server);
	
      case FULLSCREEN_KEYBOARD_GRAB_PTR_MODE:
	return ((Xv_opaque)fullscreen->keyboard_ptr_mode);

      case FULLSCREEN_KEYBOARD_GRAB_KBD_MODE:
	return ((Xv_opaque)fullscreen->keyboard_kbd_mode);

      case FULLSCREEN_POINTER_GRAB_PTR_MODE:
	return ((Xv_opaque)fullscreen->pointer_ptr_mode);

      case FULLSCREEN_POINTER_GRAB_KBD_MODE:
	return ((Xv_opaque)fullscreen->pointer_kbd_mode);

      case FULLSCREEN_OWNER_EVENTS:
	return ((Xv_opaque)fullscreen->owner_events);

      case FULLSCREEN_SYNC:
	return ((Xv_opaque)fullscreen->sync_mode_now);

      case FULLSCREEN_RECT:
	return ((Xv_opaque)&fs_public->fs_screenrect);

      case WIN_CURSOR:
	return ((Xv_opaque)fullscreen->cursor);

      case WIN_INPUT_MASK:
	if (fullscreen->im_changed) {
	    win_getinputmask(fullscreen->input_window,
			     &fullscreen->inputmask, (Xv_opaque *)NULL);
	    return ((Xv_opaque)&fullscreen->inputmask);
	} else
	    return ((Xv_opaque)&fullscreen->cached_im);

      default:
	if (xv_check_bad_attr(&xv_fullscreen_pkg,
			      (Attr_attribute)attr)
	    == XV_ERROR)
	    *status = XV_ERROR;
	return ((Xv_opaque)NULL);
    }
    /*NOTREACHED*/
}
