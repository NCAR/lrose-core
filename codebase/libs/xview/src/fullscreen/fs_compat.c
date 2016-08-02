#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)fs_compat.c 20.25 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE 
 *	file for terms of the license.
 */

#include <xview/fullscreen.h>

/*
 * Backwards compatible fullscreen utilities
 */

Xv_public struct fullscreen *
fullscreen_init(window)
    Xv_Window       window;
{
    struct fullscreen *fs_result;

    fs_result =
	(struct fullscreen *) xv_create(
					(Xv_Screen) xv_get(window, XV_SCREEN),
					FULLSCREEN,
					FULLSCREEN_INPUT_WINDOW, window,
					NULL);
    if (!fs_result)
	return ((struct fullscreen *)NULL);
    else
	return (fs_result);
}

Xv_public int
fullscreen_set_cursor(fs, cursor)
    struct fullscreen *fs;
    Xv_Cursor       cursor;
{
    (void) xv_set((Xv_opaque)fs, WIN_CURSOR, cursor, NULL);
}

Xv_public int
fullscreen_set_inputmask(fs, im)
    struct fullscreen *fs;
    Inputmask      *im;
{
    (void) xv_set((Xv_opaque)fs, WIN_INPUT_MASK, im, NULL);
}


Xv_public int
fullscreen_destroy(fs)
    struct fullscreen *fs;
{
    (void) xv_destroy((Xv_opaque)fs);
}
