#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)xv_win_lp.c 1.11 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1991 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE 
 *	file for terms of the license.
 */
/* Modified by E. Zimmermann <edz@bsn.com> for the wxWindows project
 *
 * Status:       Experimental
 * Distribution: Restricted
 */

#include <X11/Xlib.h>
#include <xview_private/i18n_impl.h>
#include <xview_private/ntfy.h> /* must appear before notify.h */
#include <xview_private/draw_impl.h> /* eventually includes notify.h */
#include <xview_private/windowimpl.h>
#include <xview_private/fm_impl.h>
#include <xview_private/ndet.h>
#include <xview_private/ndis.h>
#include <xview/frame.h>
#include <xview/server.h>

static Frame		xv_loop_frame = (Frame)NULL;
static Xv_opaque	xv_return;

static int count = 0; /* Keep track of modal count */

extern Notify_func	notify_set_scheduler_func();

Xv_public Xv_opaque
xv_window_loop(frame)
Frame   frame;
{
    Frame		save_loop_frame = xv_loop_frame;
    Display		*display;
    Notify_value	(*save_sched_func)();

    count++;
/*
printf("xv_window_loop(%lx) called\n", (unsigned long)frame  );
printf("save_loop_frame = %lx\n", (unsigned long)save_loop_frame);
*/

    if (!frame)  {
        xv_error(XV_ZERO,
            ERROR_STRING, 
            XV_MSG("xv_window_loop() : NULL frame passed."),
            NULL);
        return(XV_ERROR);
    }

    /*
     * If save_loop_frame is set, this means we are
     * re-entering xv_window_loop.
     * Set the window_loop flag for the previous frame and it's subwindows
     * to false
     */
    if (save_loop_frame)  {
	 if (window_set_tree_flag(save_loop_frame, NULL, FALSE, FALSE) != XV_OK)  {
            xv_error(save_loop_frame,
                    ERROR_STRING,
                        XV_MSG("xv_window_loop : Attempt to unblock input to windows failed (1)"),
                    NULL);
            return(XV_ERROR);
        }
    }

    /*
     * Set window_loop flag for frame and it's subwindows
     * so that during xv_window_loop they will be the only
     * non-deaf windows
     */
    if (window_set_tree_flag(frame, NULL, FALSE, TRUE) != XV_OK)  {
        xv_error(frame,
                ERROR_STRING,
                    XV_MSG("xv_window_loop : Attempt to block input to windows failed"),
                NULL);
        return(XV_ERROR);
    }

    /*
     * Make it known now that this frame and it's subwindows are the only
     * ones to be active during xv_window_loop
     */
    xv_loop_frame = frame;

    /*
     * Map the frame
     */
    xv_set(frame, XV_SHOW, TRUE, NULL);

    display = (Display *)xv_get(frame, XV_DISPLAY);

    if (!display)  {
        display = (Display *)xv_get(xv_default_server, XV_DISPLAY);
        if (!display)  {
            xv_error(XV_ZERO,
                ERROR_STRING, 
                XV_MSG("xv_window_loop() : Cannot get handle to display"),
            NULL);
            return(XV_ERROR);
        }
    }

    /*
     * Set in loop flag
     */
    xv_in_loop = 1;

    /*
     * Make signal handler deaf - do nothing
     */
    NTFY_SET_DEAF_INTERRUPT;

    /*
     * Set dispatcher scheduler to special scheduler.
     * This scheduler only dispatches events for the client specified by
     * ndis_set_special_client()
     *
     ********************************************************************
     * NOTE: For applications that use their own schedulers, this will be
     * a problem since we are switching to a different scheduler here.
     * We have to make the dispatcher dispatch only events pending for
     * the display because we do not want the dispatcher to jump the
     * queue and dispatch events queued for other clients on the queue
     * BEFORE the one that dispatched this xv_window_loop() call was
     * done. Events that are dispatched for the display are:
     *	- events occurring on the xv_window_loop frame and it's one 
     *    subwindow
     *  - repaints for all other windows (they are deaf via 
     *    xevent_to_event()
     * All events are dispatched NOTIFY_IMMEDIATE during xv_window_loop()
     ********************************************************************
     */
    save_sched_func = notify_set_scheduler_func(ndis_special_client_scheduler);
    ndis_set_special_client((Notify_client)display);

    /*
     * While xv_window_return() not called and a request to
     * stop the notifier has NOT been made
     */
    while((xv_in_loop || (frame != xv_loop_frame)) && 
		(!(ndet_flags & NDET_STOP)))  {
	XEvent event;
        /*
         * Read X events (and ONLY X events) as they
         * arrive, and post them.
         * This puts them on the dispatcher queue.
	 * BUG: What if we have multiple connections to different displays
         */
	XPeekEvent(display, &event);
        xv_input_pending(display, 0);

        /*
         * Dispatch events on dispatcher queue
         */
        ndis_dispatch();
    }

    /*
     * Reset special client and set the dispatcher scheduler to what it
     * was before
     */
    ndis_unset_special_client();
    (void)notify_set_scheduler_func(save_sched_func);

    /*
     * Make signal handler able to Q events to dispatcher
     * once again
     */
    NTFY_UNSET_DEAF_INTERRUPT;

    /*
     * Unset window_loop flag for frame and its subwindows
     */
if (--count)
    if (window_set_tree_flag(frame, NULL, FALSE, FALSE) != XV_OK)  {
        xv_error(frame,
                ERROR_STRING,
                    XV_MSG("xv_window_loop : Attempt to unblock input to windows failed (2)"),
                NULL);
        return(XV_ERROR);
    }

    /*
     * Bring frame down
     */
    xv_set(frame, XV_SHOW, FALSE, NULL);

    /*
     * If save_loop_frame was set, this means xv_window_loop()
     * was re-entered.
     * Restore the saved frame and its subwindows to be active in 
     * xv_window_loop() once again.
     */
    if (save_loop_frame)  {
        if (window_set_tree_flag(save_loop_frame, NULL, FALSE, TRUE) != XV_OK)  {
            xv_error(save_loop_frame,
                    ERROR_STRING,
                        XV_MSG("xv_window_loop : Attempt to block input to windows failed"),
                    NULL);
            return(XV_ERROR);
        }

	/*
	 * Set the in loop flag back to 1
	 */
	xv_in_loop = 1;

	/*
	 * Make the saved frame and its subwindows active windows during
	 * xv_window_loop() once again
	 */
	xv_loop_frame = save_loop_frame;

    }
    else  {
	/*
	 * If save_loop_frame was NOT set, reset xv_loop_frame back to
	 * NULL
	 */
	xv_loop_frame = (Frame)NULL;
    }

    /*
     * Sync with server
     */
    XFlush(display);

/*
printf("Finishing loop of frame = %lx\n", (unsigned long)frame);
*/

    return xv_return;
}

Xv_public void
xv_window_return(ret)
Xv_opaque     ret;
{
    /*
     * Set exit loop flag
     */
    xv_in_loop = 0;

    /*
     * Set xv_window_loop return value
     */
    xv_return = ret;
}

