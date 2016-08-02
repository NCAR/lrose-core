#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)wmgr_rect.c 20.20 93/06/28 Copyr 1985 Sun Micro";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE 
 *	file for terms of the license.
 */

/*
 * Window mgr move, stretch and refresh.
 */

#include <xview/wmgr.h>

Xv_public void
wmgr_completechangerect(
		   window, rectnew, rectoriginal, parentprleft, parentprtop)
    Xv_opaque       window;
    Rect           *rectnew, *rectoriginal;
    int             parentprleft, parentprtop;
{
    (void) win_setrect(window, rectnew);
}

Xv_public void
wmgr_refreshwindow(window)
    Xv_opaque       window;
{
    Rect            rectoriginal, rectdif;
    int             marginchange = -1;

    (void) win_lockdata(window);
    (void) win_getrect(window, &rectoriginal);
    /*
     * A position change is supposed to invoke a repaint of the entire window
     * and its children. So, change position by 1,1 and then back to original
     * position.
     */
    rectdif = rectoriginal;
    if ((rectoriginal.r_width == 0) || (rectoriginal.r_height == 0))
	marginchange = 1;
    rect_marginadjust(&rectdif, marginchange);
    (void) win_setrect(window, &rectdif);
    (void) win_setrect(window, &rectoriginal);
    (void) win_unlockdata(window);
    return;
}
