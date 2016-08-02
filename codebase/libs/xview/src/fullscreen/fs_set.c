#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)fs_set.c 20.31 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE 
 *	file for terms of the license.
 */

#include <xview_private/draw_impl.h>
#include <xview/cursor.h>
#include <xview/window.h>
#include <xview/server.h>
#include <xview_private/fs_impl.h>
#include <xview_private/i18n_impl.h>

Xv_private char *fullscreen_translate_report_code();

static void     fullscreen_allow_events();

Pkg_private Xv_opaque
fullscreen_set_avlist(fullscreen_public, avlist)
    Xv_fullscreen  *fullscreen_public;
    Attr_avlist     avlist;
{
    register Attr_avlist attrs;
    Xv_Drawable_info *info;
    Display        *display;
    Fullscreen_info *fullscreen = FULLSCREEN_PRIVATE(fullscreen_public);
    Xv_Cursor       cursor = fullscreen->cursor;
    int             saw_win_attr = FALSE;
    int             new_im = FALSE;
    int		    saw_allow_events_attr = FALSE, allow_mode;
    int             saw_sync_now_attr = FALSE;
    int             sync_now = FALSE;	/* set only, create taken already */
    int             allow_sync_event = FALSE;	/* set takes care of it */
    int		    grab_change = FALSE, grab_mode_change = FALSE;
    int		    new_pointer_grab = -1, new_kbd_grab = -1,
    	    	    new_server_grab = -1;
    struct fullscreen *myfullscreenstruct =
	(struct fullscreen *)&fullscreen_public->fullscreen_struct;

    DRAWABLE_INFO_MACRO(fullscreen->input_window, info);
    display = xv_display(info);

    for (attrs = avlist; *attrs;
	 attrs = attr_next((Xv_opaque *) attrs)) {
	switch ((int) attrs[0]) {
	  case WIN_CURSOR:
	    cursor = (Xv_Cursor) attrs[1];
            ATTR_CONSUME(attrs[0]);
	    break;

	  case FULLSCREEN_SYNC:
	    sync_now = (int) attrs[1];
            ATTR_CONSUME(attrs[0]);
	    saw_sync_now_attr = TRUE;
	    break;

	  case FULLSCREEN_ALLOW_EVENTS:
	    allow_mode = (int) attrs[1];
	    ATTR_CONSUME(attrs[0]);
	    saw_allow_events_attr = TRUE;
	    break;

	  case FULLSCREEN_GRAB_POINTER:
            new_pointer_grab = (int) attrs[1];
            ATTR_CONSUME(attrs[0]);
            break;
            
          case FULLSCREEN_GRAB_KEYBOARD:
            new_kbd_grab = (int) attrs[1];
            ATTR_CONSUME(attrs[0]);
            grab_change = TRUE;
            break;
            
          case FULLSCREEN_GRAB_SERVER:
            new_server_grab = (int) attrs[1];
            ATTR_CONSUME(attrs[0]);
            grab_change = TRUE;
            break;

	  case FULLSCREEN_KEYBOARD_GRAB_PTR_MODE:
            fullscreen->keyboard_ptr_mode = (Fullscreen_grab_mode) attrs[1];
            ATTR_CONSUME(attrs[0]);
            grab_mode_change = TRUE;
            break;
            
          case FULLSCREEN_KEYBOARD_GRAB_KBD_MODE:
            fullscreen->keyboard_kbd_mode = (Fullscreen_grab_mode) attrs[1];
            ATTR_CONSUME(attrs[0]);
            grab_mode_change = TRUE;
            break;
            
          case FULLSCREEN_POINTER_GRAB_PTR_MODE:
            fullscreen->pointer_ptr_mode = (Fullscreen_grab_mode) attrs[1];
            ATTR_CONSUME(attrs[0]);
            grab_mode_change = TRUE;
            break;
            
          case FULLSCREEN_POINTER_GRAB_KBD_MODE:
            fullscreen->pointer_kbd_mode = (Fullscreen_grab_mode) attrs[1];
            ATTR_CONSUME(attrs[0]);
            grab_mode_change = TRUE;
            break;
            
          case FULLSCREEN_OWNER_EVENTS:
            fullscreen->owner_events = (int) attrs[1];
            ATTR_CONSUME(attrs[0]);
            grab_change = TRUE;
	    break;

	  case FULLSCREEN_ALLOW_SYNC_EVENT:
	    allow_sync_event = TRUE;
	    break;

	  case WIN_INPUT_MASK:
	  case WIN_CONSUME_EVENT:
	  case WIN_IGNORE_EVENT:
	  case WIN_CONSUME_EVENTS:
	  case WIN_IGNORE_EVENTS:
	    saw_win_attr = TRUE;
	    break;

	  default:
	    (void) xv_check_bad_attr(&xv_fullscreen_pkg, attrs[0]);
	    break;
	}
    }

    if (saw_win_attr) {
	/* old inputmask already cached, set new im */
	(void) xv_set_avlist(fullscreen->input_window, avlist);
	win_getinputmask(fullscreen->input_window, &fullscreen->inputmask,
			 (Xv_opaque *)NULL);
	fullscreen->im_changed = TRUE;
	new_im = TRUE;
	for (attrs = avlist; *attrs;
	     attrs = attr_next((Xv_opaque *) attrs)) {
	    switch ((int) attrs[0]) {
	      case WIN_INPUT_MASK:
	      case WIN_CONSUME_EVENT:
	      case WIN_IGNORE_EVENT:
	      case WIN_CONSUME_EVENTS:
	      case WIN_IGNORE_EVENTS:
		ATTR_CONSUME(attrs[0]);
		break;

	      default:
		break;
	    }
	}
    }
    if ((cursor != fullscreen->cursor) || (new_im)) {
	if (cursor != fullscreen->cursor) {
	    if (fullscreen->cursor) {
		(void) xv_set(fullscreen->cursor, XV_DECREMENT_REF_COUNT, NULL);
	    }
	    (void) xv_set(cursor, XV_INCREMENT_REF_COUNT, NULL);
	    fullscreen->cursor = cursor;
	    myfullscreenstruct->cursor = cursor;
	}
	if (new_im)
	    myfullscreenstruct->inputmask = fullscreen->inputmask;
	win_set_grabio_params(fullscreen->input_window,
			      &fullscreen->inputmask, fullscreen->cursor);
    }
    if (new_server_grab==FALSE && fullscreen->grab_server==TRUE) {
        XUngrabServer(display);
        fullscreen->grab_server = FALSE;
    }
    if (new_kbd_grab==FALSE && fullscreen->grab_kbd==TRUE) {
#ifdef OW_I18N
        window_set_xungrabkeyboard(fullscreen->input_window, display, CurrentTime);
#else
        XUngrabKeyboard(display, CurrentTime);
#endif 
        fullscreen->grab_kbd = FALSE;
    }
    if (new_pointer_grab==FALSE && fullscreen->grab_pointer==TRUE) {
        XUngrabPointer(display, CurrentTime);
        fullscreen->grab_pointer = FALSE;
    }
    if (new_pointer_grab==TRUE || new_kbd_grab==TRUE || new_server_grab==TRUE ||
		grab_change==TRUE || grab_mode_change==TRUE) {
	int grab_successful, grab_result;

	grab_result = xv_win_grab(fullscreen->input_window,
			          &fullscreen->inputmask,
			          fullscreen->cursor_window,
			          fullscreen->cursor,
			          fullscreen->grab_pointer,
			          fullscreen->grab_kbd,
			          fullscreen->grab_server,
			          fullscreen->pointer_ptr_mode,
			          fullscreen->pointer_kbd_mode,
			          fullscreen->keyboard_ptr_mode,
			          fullscreen->keyboard_kbd_mode,
			          fullscreen->owner_events,
			          &grab_successful);
	if (!grab_successful) {
	    (void) xv_error((Xv_opaque)fullscreen,
			    ERROR_STRING,
				fullscreen_translate_report_code(grab_result),
			    ERROR_PKG, FULLSCREEN,
			    NULL);
	    return (Xv_opaque) XV_ERROR;
	}
    }
    if (saw_sync_now_attr && sync_now &&
	(fullscreen->sync_mode_now == FALSE)) {
	int             grab_successful, grab_result;

	grab_result = xv_win_grab(fullscreen->input_window,
			          &fullscreen->inputmask,
			          fullscreen->cursor_window,
			          fullscreen->cursor,
			          fullscreen->grab_pointer,
			          fullscreen->grab_kbd,
			          fullscreen->grab_server,
		fullscreen->pointer_ptr_mode = (Fullscreen_grab_mode) FALSE,
		fullscreen->pointer_kbd_mode = (Fullscreen_grab_mode) TRUE,
		fullscreen->keyboard_ptr_mode = (Fullscreen_grab_mode) TRUE,
		fullscreen->keyboard_kbd_mode = (Fullscreen_grab_mode) FALSE,
			          fullscreen->owner_events,
			          &grab_successful);
	fullscreen->sync_mode_now = TRUE;
	if (!grab_successful) {
	    (void) xv_error((Xv_opaque)fullscreen,
			    ERROR_STRING,
				fullscreen_translate_report_code(grab_result),
			    ERROR_PKG, FULLSCREEN,
			    NULL);
	    return (Xv_opaque) XV_ERROR;
	}
    } else if (saw_sync_now_attr && (!sync_now) &&
	       (fullscreen->sync_mode_now == TRUE)) {
	int             grab_successful, grab_result;

	grab_result = xv_win_grab(fullscreen->input_window,
			          &fullscreen->inputmask,
			          fullscreen->cursor_window,
			          fullscreen->cursor,
			          fullscreen->grab_pointer,
			          fullscreen->grab_kbd,
			          fullscreen->grab_server,
		fullscreen->pointer_ptr_mode = (Fullscreen_grab_mode) TRUE,
		fullscreen->pointer_kbd_mode = (Fullscreen_grab_mode) TRUE,
		fullscreen->keyboard_ptr_mode = (Fullscreen_grab_mode) TRUE,
		fullscreen->keyboard_kbd_mode = (Fullscreen_grab_mode) TRUE,
			          fullscreen->owner_events,
			          &grab_successful);
	fullscreen->sync_mode_now = FALSE;
	if (!grab_successful) {
	    (void) xv_error((Xv_opaque)fullscreen,
			    ERROR_STRING,
				fullscreen_translate_report_code(grab_result),
			    ERROR_PKG, FULLSCREEN,
			    NULL);
	    return (Xv_opaque) XV_ERROR;
	}
    }
    if (saw_allow_events_attr) {
	fullscreen_allow_events(display, allow_mode);
    }
    if (allow_sync_event)
	if (fullscreen->sync_mode_now == TRUE) {
	    if (fullscreen->grab_pointer && fullscreen->grab_kbd) {
	        fullscreen_allow_events(display, SyncBoth);
	    } else if (fullscreen->grab_pointer) {
		fullscreen_allow_events(display, SyncPointer);
	    } else if (fullscreen->grab_kbd) 
		fullscreen_allow_events(display, SyncKeyboard);
	} else {
	    xv_error(XV_ZERO,
		     ERROR_STRING, 
		     XV_MSG("Attempt to allow synchronous event processing without first being in synchronous mode. Attribute ignored!"),
		     ERROR_PKG, FULLSCREEN,
		     NULL);
	}

    return ((Xv_opaque) XV_OK);
}

static void
fullscreen_allow_events(display, event_mode)
    Display *display;
    int event_mode;  /* Xlib allow events event_mode argument */
{
    XAllowEvents(display, event_mode, CurrentTime);
}

