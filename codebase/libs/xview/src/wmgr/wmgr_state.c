#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)wmgr_state.c 20.23 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE 
 *	file for terms of the license.
 */

/*
 * Window mgr open/close and top/bottom.
 */

#include <stdio.h>
#include <xview/wmgr.h>
#include <xview/win_struct.h>

Xv_public void
wmgr_changelevel(window, parent, top)
    register Xv_object window;
    int             parent;
    int             top;
{
    int             topchildnumber, bottomchildnumber;

    /*
     * Don't try to optimize by not doing anything if already at desired
     * level.  Doing so messes up the fixup list because callers of this
     * routine do partial repair which incorrectly removes some stuff from
     * the damage list if a win_remove/win_insert pair hasn't been done.
     */
    /*
     * Remove from tree
     */
    (void) win_remove(window);
    /*
     * Set new links
     */
    if (top) {
	topchildnumber = win_getlink(parent, WL_TOPCHILD);
	(void) win_setlink(window, WL_COVERED, topchildnumber);
	(void) win_setlink(window, WL_COVERING, WIN_NULLLINK);
    } else {
	bottomchildnumber = win_getlink(parent, WL_BOTTOMCHILD);
	(void) win_setlink(window, WL_COVERING, bottomchildnumber);
	(void) win_setlink(window, WL_COVERED, WIN_NULLLINK);
    }
    /*
     * Insert into tree
     */
    (void) win_insert(window);
    return;
}
