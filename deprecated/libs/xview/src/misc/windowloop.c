#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)windowloop.c 20.25 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#include <stdio.h>
#include <xview/xview.h>
#include <xview/frame.h>
#include <xview/fullscreen.h>

static short    no_return /* = FALSE */ ;
static Xv_opaque return_value;



/* BUG: this whole routine is suspect */

Xv_private Xv_opaque
_xv_block_loop(frame)
    register Frame  frame;
{
    register Xv_Window first_window;
#ifdef SUNVIEW1
    register Frame  shadow;
#endif
    Pw_pixel_cache *pixel_cache;
    struct fullscreen *fs;
    Rect            rect;
    Event           event;
    int             left, top;
    Inputmask       im;

    input_imnull(&im);

    /*
     * we don't support recursive xv_block_loop() calls yet. Also, no support
     * for already-shown confirmer. Also, can't be iconic.
     */
    if (no_return || window_get(frame, XV_SHOW) ||
	window_get(frame, FRAME_CLOSED))
	return (Xv_opaque) 0;

    /*
     * insert the frame in the window tree. Note that this will not alter the
     * screen until the WIN_REPAINT is sent below.
     */
    (void) window_set(frame, XV_SHOW, TRUE, NULL);

    /*  screen_rect = *(Rect *) window_get(frame, WIN_SCREEN_RECT); */
    first_window = window_get(frame, FRAME_NTH_SUBWINDOW, NULL);

#ifdef SUNVIEW1
    shadow = window_get(frame, FRAME_SHADOW, NULL);
#endif

    /*
     * lock the window tree in hopes of preventing screen updates until the
     * grabio.
     */
    (void) win_lockdata(frame);

    /*
     * make sure the frame and first_window are in sync with their current
     * size. NOTE: Assuming WIN_REPAINT will not happens after WIN_RESIZE.
     */
#ifdef SUNVIEW1
    (void) pw_exposed(window_get(frame, WIN_PIXWIN));
    if (shadow != NULL)
	(void) pw_exposed(window_get(shadow, WIN_PIXWIN));
    (void) pw_exposed(window_get(first_window, WIN_PIXWIN));
#endif
    (void) win_post_id(first_window, WIN_RESIZE, NOTIFY_IMMEDIATE);
    (void) win_post_id(frame, WIN_RESIZE, NOTIFY_IMMEDIATE);

    rect = *((Rect *) window_get(frame, WIN_RECT));
    rect.r_left = rect.r_top = 0;
    fs = fullscreen_init(frame);

#ifdef SUNVIEW1
    /* account for the shadow area if present */
    if (shadow != NULL) {
	Rect            shadow_rect;

	/*
	 * use win_getrect() rather than WIN_RECT to get rect in same space
	 * as the frame (screen coords.).
	 */
	shadow_rect = *((Rect *) window_get(shadow, WIN_RECT));
	rect = rect_bounding(&rect, &shadow_rect);
    }
#endif

    /* convert rect to screen coords */
    win_translate_xy(fs->fs_windowfd, xv_get(fs->fs_windowfd, XV_ROOT),
		     rect.r_left, rect.r_top, &left, &top);
    rect.r_left = left;
    rect.r_top = top;

    if ((pixel_cache = pw_save_pixels(fs->fs_rootwindow, &rect)) == 0) {
	(void) win_unlockdata(frame);
	(void) fullscreen_destroy(fs);
	return (Xv_opaque) 0;
    }
    /*
     * (void)pw_preparesurface(fs->fs_rootwindow, &rect);
     */
    (void) fullscreen_destroy(fs);

    (void) win_grabio(first_window);
    (void) win_unlockdata(frame);

    /* paint the frame */
    (void) win_post_id(frame, WIN_REPAINT, NOTIFY_IMMEDIATE);

    /* paint the first subwindow */
    (void) win_post_id(first_window, WIN_REPAINT, NOTIFY_IMMEDIATE);
    (void) win_post_id(first_window, LOC_WINENTER, NOTIFY_IMMEDIATE);
    (void) win_post_id(first_window, KBD_USE, NOTIFY_IMMEDIATE);

    /*
     * Moved to before win_post_id(first_window, WIN_RESIZE,
     * NOTIFY_IMMEDIATE) (void)win_grabio(first_window);
     * 
     * (void)win_unlockdata(frame);
     */

    /*
     * read and post events to the first subwindow until xv_block_return() is
     * called.
     */
    win_setinputcodebit(&im, LOC_WINENTER);
    win_setinputcodebit(&im, LOC_WINEXIT);
    win_setinputcodebit(&im, LOC_WINEXIT);
    win_setinputcodebit(&im, KBD_USE);
    win_setinputcodebit(&im, KBD_DONE);
    im.im_flags = IM_ASCII;
    no_return = TRUE;
    while (no_return) {
	(void) input_readevent(first_window, &event);
	switch (event_action(&event)) {
	  case LOC_WINENTER:
	  case LOC_WINEXIT:
	  case KBD_USE:
	  case KBD_DONE:
	  case ACTION_FRONT:
	  case ACTION_BACK:	/* top/bottom */
	  case ACTION_OPEN:
	  case ACTION_CLOSE:	/* close/open */
	    break;

	  default:
	    /*
	     * don't mess with the data lock -- leave the grabio lock on;
	     * either the kernel will be fixed to allow all windows in this
	     * process to paint, or we require that only the first_window
	     * window can paint. (void)win_lockdata(frame);
	     * (void)win_releaseio(first_window);
	     */

	    (void) win_post_event(first_window, &event, NOTIFY_IMMEDIATE);

	    /*
	     * (void)win_grabio(first_window); (void)win_unlockdata(frame);
	     */
	    break;
	}
    }

    /*
     * lock the window tree until the bits are restored.
     */
    (void) win_lockdata(frame);
    (void) win_releaseio(first_window);

    (void) win_post_id(first_window, KBD_DONE, NOTIFY_IMMEDIATE);
    (void) win_post_id(first_window, LOC_WINEXIT, NOTIFY_IMMEDIATE);

    fs = fullscreen_init(frame);
    pw_restore_pixels(fs->fs_rootwindow, pixel_cache);
    (void) fullscreen_destroy(fs);

    (void) window_set(frame, XV_SHOW, FALSE, NULL);
#ifdef SUNVIEW1
    if (shadow)
	(void) window_set(shadow, XV_SHOW, FALSE, NULL);
#endif

    (void) win_unlockdata(frame);

    no_return = FALSE;
    return return_value;
}


Xv_private void
_xv_block_return(value)
    Xv_opaque       value;
{
    no_return = FALSE;
    return_value = value;
}
