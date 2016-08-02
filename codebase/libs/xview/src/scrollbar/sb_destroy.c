#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)sb_destroy.c 1.35 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * Module:	sb_destroy.c
 * 
 * Description:
 * 
 * cleanups scrollbar as it is destroyed
 * 
 */

/*
 * Include files:
 */
#include <xview_private/sb_impl.h>
#include <xview/canvas.h>
#include <xview/frame.h>
#include <xview/screen.h>

/*
 * Declaration of Functions Defined in This File (in order):
 */

Pkg_private int scrollbar_destroy_internal();

/******************************************************************/

Pkg_private int
scrollbar_destroy_internal(sb_public, status)
    Scrollbar       sb_public;
    Destroy_status  status;
{
    Xv_scrollbar_info *sb = SCROLLBAR_PRIVATE(sb_public);
    Xv_Window	    focus_win;
    Frame	    frame;
	
    if ((status == DESTROY_CLEANUP) || (status == DESTROY_PROCESS_DEATH)) {
	/* If the scrollbar owns the Frame focus window, then ???
	 */
	frame = xv_get(sb_public, WIN_FRAME);
	focus_win = xv_get(frame, FRAME_FOCUS_WIN);
	if (focus_win && xv_get(focus_win, WIN_PARENT) == sb_public) {
	    xv_set(focus_win,
		   WIN_PARENT, frame,  /* the only window guaranteed still
					* to exist. */
		   XV_SHOW, FALSE,
		   NULL);
	    /* BUG ALERT:  If the canvas is the only frame subwindow,
	     *		   we will be left without a current focus subwindow.
	     */
	    xv_set(frame, FRAME_NEXT_PANE, NULL);
	}
	
	/* Clean up menu */
	xv_destroy(sb->menu);
	
	if (status == DESTROY_CLEANUP)
	    free((char *) sb);
    }
    return XV_OK;
}


	
