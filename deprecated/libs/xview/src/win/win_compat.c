#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)win_compat.c% 20.23 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * Win_compat.c: SunView 1.X compatibility routines.
 */

#include <xview_private/i18n_impl.h>
#include <xview/pkg.h>
#include <xview/window.h>
#include <xview/win_input.h>
#include <xview/fullscreen.h>

/* ARGSUSED */
void
win_getinputmask(window, im, nextwindownumber)
    Xv_object       window;
    Inputmask      *im;
    Xv_opaque      *nextwindownumber;
{
    *im = *((Inputmask *) xv_get(window, WIN_INPUT_MASK));
}

/* ARGSUSED */
win_setinputmask(window, im, im_flush, nextwindownumber)
    Xv_object       window;
    Xv_opaque       nextwindownumber;
    Inputmask      *im, *im_flush;
{

    if (xv_get(window, WIN_IS_IN_FULLSCREEN_MODE)) {
	fprintf(stderr,
		XV_MSG(" Attempting to set the input mask of a window in fullscreen mode!\n"));
	abort();
    }
    xv_set(window, WIN_INPUT_MASK, im, NULL);
}


coord
win_getheight(window)
    Xv_object       window;
{

    return ((int) window_get(window, WIN_GET_HEIGHT));
}

coord
win_getwidth(window)
    Xv_object       window;
{

    return ((int) window_get(window, WIN_GET_WIDTH));
}
