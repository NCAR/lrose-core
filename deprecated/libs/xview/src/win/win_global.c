#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)win_global.c 20.39 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * win_global.c: Implement the functions of the win_struct.h interface that
 * exercise global influence on the whole window environment.
 */

#include <stdio.h>
#include <xview_private/draw_impl.h>
#include <xview/win_input.h>
#include <xview/window.h>
#include <xview/server.h>
#include <X11/Xlib.h>

#define POINTERMASK  (ButtonPressMask | ButtonReleaseMask | EnterWindowMask |\
		      LeaveWindowMask | PointerMotionMask |\
		      PointerMotionHintMask | Button1MotionMask |\
		      Button2MotionMask | Button3MotionMask |\
		      Button4MotionMask | Button5MotionMask| ButtonMotionMask |\
		      KeymapStateMask)


/*
 * When not zero will not actually acquire data lock so that the debugger
 * wouldn't get hung. USE ONLY DURING DEBUGGING WHEN YOU KNOW WHAT YOUR
 * DOING!
 */
int             win_lockdatadebug;
/*
 * When not zero will not actually acquire exclusive io access rights so that
 * the debugger wouldn't get hung. USE ONLY DURING DEBUGGING WHEN YOU KNOW
 * WHAT YOUR DOING!
 */
int             win_grabiodebug;

/*
 * Server operations applying globally.
 */
void win_lockdata(window)
    Xv_object       window;
{
    Xv_Drawable_info *info;

    if (win_lockdatadebug)
	return;
    DRAWABLE_INFO_MACRO(window, info);

    XGrabServer(xv_display(info));
}

void win_unlockdata(window)
    Xv_object       window;
{
    Xv_Drawable_info *info;

    if (win_lockdatadebug)
	return;
    DRAWABLE_INFO_MACRO(window, info);

    XUngrabServer(xv_display(info));
}

/*
 * This function is created for the FULLSCREEN package and is intended as a 
 * blanket over the Xlib grab routines.  The grab_* params specify whether
 * or not to grab that device.  The *_async params are toggles to say whether
 * that device should be grabbed Asynchronous=TRUE or Synchronous=FALSE.
 * If grab successful, status = TRUE=1.  Return value is result from XGrabPointer
 * or XGrabKeyboard
 */
Xv_private int
xv_win_grab(window, im, cursor_window, cursor, grab_pointer, grab_kbd,
	    grab_server, grap_pointer_pointer_async,
	    grab_pointer_keyboard_async, grap_kbd_pointer_async,
	    grab_kbd_keyboard_async, owner_events, status)
    Xv_object       window;
    Inputmask      *im;
    Xv_object       cursor_window;
    Xv_object       cursor;
    int		    grap_pointer_pointer_async, grab_pointer_keyboard_async,
    		    grap_kbd_pointer_async,     grab_kbd_keyboard_async;
    int		   *status; /* informs whether successful or not */
{
    Xv_Drawable_info *info;
    Display        *display;
    int		    return_code = GrabSuccess;
    unsigned int    xevent_mask = win_im_to_xmask(window, im);

    if (status)
        *status = 1; /* initialize to be OK */
    if (win_grabiodebug)
	return 0;
    DRAWABLE_INFO_MACRO(window, info);
    display = xv_display(info);

    if (xv_get(xv_server(info), SERVER_JOURNALLING))
	xv_set(xv_server(info), SERVER_JOURNAL_SYNC_EVENT, 1, NULL);
    if (grab_server)
    	XGrabServer(display);
    if (grab_pointer)
        if (return_code = XGrabPointer(display, 
        		               xv_xid(info),
        		               owner_events,
        		               xevent_mask & POINTERMASK,
		                       grap_pointer_pointer_async ? 
		                       		GrabModeAsync : GrabModeSync,
		                       grab_pointer_keyboard_async ? 
		                       		GrabModeAsync : GrabModeSync,
		                       cursor_window ? 
		                           (int) xv_get(cursor_window, XV_XID)
									 : None,
		                       cursor ? (int) xv_get(cursor, XV_XID)
									 : None,
		                       CurrentTime) != GrabSuccess) {
	    if (status)
	        *status = 0;
	    if (grab_server)
		XUngrabServer(display);
	    return (return_code);
	}
    if (grab_kbd) {
#ifdef OW_I18N
    	if (return_code = window_set_xgrabkeyboard(window, display,
    		                        xv_xid(info),
    		                        owner_events,
		                        grap_kbd_pointer_async ? 
		                        	GrabModeAsync : GrabModeSync,
		                        grab_kbd_keyboard_async ? 
		                        	GrabModeAsync : GrabModeSync,
		                        CurrentTime) != GrabSuccess) {
#else
    	if (return_code = XGrabKeyboard(display,
    		                        xv_xid(info),
    		                        owner_events,
		                        grap_kbd_pointer_async ? 
		                        	GrabModeAsync : GrabModeSync,
		                        grab_kbd_keyboard_async ? 
		                        	GrabModeAsync : GrabModeSync,
		                        CurrentTime) != GrabSuccess) {
#endif /* OW_I18N */
	    if (grab_pointer)
	        XUngrabPointer(display, CurrentTime);
	    if (grab_server)
		XUngrabServer(display);
	    if (status)
	        *status = 0;
	    return (return_code);
	}
    }
    return (GrabSuccess);
}

Xv_private int
xv_win_ungrab(window, ungrab_pointer, ungrab_kbd, ungrab_server)
    Xv_object       window;
    int		    ungrab_pointer, ungrab_kbd, ungrab_server;
{
    Xv_Drawable_info *info;
    Display        *display;

    if (win_grabiodebug)
	return 0;
    DRAWABLE_INFO_MACRO(window, info);
    display = xv_display(info);

    if (ungrab_server)
    	XUngrabServer(display);
#ifdef OW_I18N
    if (ungrab_kbd)
    	window_set_xungrabkeyboard(window, display, CurrentTime);
#else
    if (ungrab_kbd)
    	XUngrabKeyboard(display, CurrentTime);
#endif
    if (ungrab_pointer)
    	XUngrabPointer(display, CurrentTime);
    if (xv_get(xv_server(info), SERVER_JOURNALLING))
	xv_set(xv_server(info), SERVER_JOURNAL_SYNC_EVENT, 1, NULL);
}

int win_grabio(window)
    Xv_object       window;
{
    Inputmask      *im = (Inputmask *) xv_get(window, WIN_INPUT_MASK);

    return (win_xgrabio_async(window, im, 0, 0));
}

int win_xgrabio_sync(window, im, cursor_window, cursor)
    Xv_object       window;
    Inputmask      *im;
    Xv_object       cursor_window;
    Xv_object       cursor;
{
    Xv_Drawable_info *info;
    Display        *display;
    unsigned int    xevent_mask = win_im_to_xmask(window, im);

    if (win_grabiodebug)
	return 0;
    DRAWABLE_INFO_MACRO(window, info);
    display = xv_display(info);

    if (xv_get(xv_server(info), SERVER_JOURNALLING))
	xv_set(xv_server(info), SERVER_JOURNAL_SYNC_EVENT, 1, NULL);
    if (XGrabPointer(display, 
    		     xv_xid(info),
    		     FALSE,
    		     xevent_mask & POINTERMASK,
		     GrabModeSync,
		     GrabModeAsync,
		     cursor_window ? (int) xv_get(cursor_window, XV_XID) : None,
		     cursor ? (int) xv_get(cursor, XV_XID) : None,
		     CurrentTime) != GrabSuccess)
	return (0);
#ifdef OW_I18N
    if (window_set_xgrabkeyboard(window, display,
    		      xv_xid(info),
    		      FALSE,
		      GrabModeAsync, 
		      GrabModeSync, 
		      CurrentTime) != GrabSuccess) {
#else
    if (XGrabKeyboard(display,
    		      xv_xid(info),
    		      FALSE,
		      GrabModeAsync, 
		      GrabModeSync, 
		      CurrentTime) != GrabSuccess) {
#endif
	XUngrabPointer(display, CurrentTime);
	return(0);
    }
    XGrabServer(display);
    return (1);
}

int win_xgrabio_async(window, im, cursor_window, cursor)
    Xv_object       window;
    Inputmask      *im;
    Xv_object       cursor_window;
    Xv_object       cursor;
{
    Xv_Drawable_info *info;
    Display        *display;
    unsigned int    xevent_mask = win_im_to_xmask(window, im);

    if (win_grabiodebug)
	return 0;
    DRAWABLE_INFO_MACRO(window, info);
    display = xv_display(info);

    if (xv_get(xv_server(info), SERVER_JOURNALLING))
	xv_set(xv_server(info), SERVER_JOURNAL_SYNC_EVENT, 1, NULL);
    if (XGrabPointer(display, 
    		     xv_xid(info), 
    		     FALSE, 
    		     xevent_mask & POINTERMASK,
		     GrabModeAsync, 
		     GrabModeAsync,
		     cursor_window ? (int) xv_get(cursor_window, XV_XID) : None,
		     cursor ? (int) xv_get(cursor, XV_XID) : None,
		     CurrentTime) != GrabSuccess)
	return (0);
#ifdef OW_I18N
    if (window_set_xgrabkeyboard(window, display,
    		      xv_xid(info),
    		      FALSE,
		      GrabModeAsync,
		      GrabModeAsync,
		      CurrentTime) != GrabSuccess) {
#else
    if (XGrabKeyboard(display,
    		      xv_xid(info),
    		      FALSE,
		      GrabModeAsync,
		      GrabModeAsync,
		      CurrentTime) != GrabSuccess) {
#endif
	XUngrabPointer(display, CurrentTime);
	return(0);
    }
    XGrabServer(display);
    return (1);
}

win_set_grabio_params(window, im, cursor)
    Xv_object       window;
    Inputmask      *im;
    Xv_opaque       cursor;
{
    Xv_Drawable_info *info;
    unsigned int    xevent_mask = win_im_to_xmask(window, im);

    DRAWABLE_INFO_MACRO(window, info);
    XChangeActivePointerGrab(xv_display(info),
    			     xevent_mask & POINTERMASK,
			     (int) xv_get(cursor, XV_XID),
			     CurrentTime);
}

int win_releaseio(window)
    Xv_object       window;
{
    Xv_Drawable_info *info;
    Display        *display;

    if (win_grabiodebug)
	return 0;
    DRAWABLE_INFO_MACRO(window, info);
    display = xv_display(info);

    XUngrabServer(display);
#ifdef OW_I18N
    window_set_xungrabkeyboard(window, display, CurrentTime);
#else
    XUngrabKeyboard(display, CurrentTime);
#endif
    XUngrabPointer(display, CurrentTime);
    if (xv_get(xv_server(info), SERVER_JOURNALLING))
	xv_set(xv_server(info), SERVER_JOURNAL_SYNC_EVENT, 1, NULL);
}

win_private_gc(window, create_private_gc)
    Xv_object       window;
    int             create_private_gc;
{
    register Xv_Drawable_info *info;
    register Display *display;
    XID             xid;
    register GC     new;

    DRAWABLE_INFO_MACRO(window, info);
    display = xv_display(info);
    xid = xv_xid(info);
    if (create_private_gc == TRUE) {
	new = XCreateGC(display, xid, 0, (XGCValues *) 0);
	(void) xv_set(window, XV_KEY_DATA, XV_GC, new, NULL);
	info->private_gc = TRUE;
    } else {
	if (info->private_gc) {
	    XFreeGC(display, xv_gc(window, info));
	    (void) xv_set(window, XV_KEY_DATA_REMOVE, XV_GC, NULL);
	    info->private_gc = FALSE;
	}
    }
}
