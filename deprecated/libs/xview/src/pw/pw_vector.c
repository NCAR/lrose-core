#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)pw_vector.c 20.20 89/07/31";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE 
 *	file for terms of the license.
 */

/*
 * Pw_vector.c: Implement the pw_vector functions of the pixwin.h interface.
 */

#include <xview_private/pw_impl.h>

Xv_public int
xv_vector(window, x0, y0, x1, y1, op, cms_index)
    Xv_opaque       window;
    int             op;
    register int    x0, y0, x1, y1;
    int             cms_index;
{
    Xv_Drawable_info *info;
    Display        *display;
    Drawable        d;
    GC              gc;

    DRAWABLE_INFO_MACRO(window, info);
    display = xv_display(info);
    d = xv_xid(info);
    gc = xv_find_proper_gc(display, info, PW_VECTOR);

    if (op == PIX_NOT(PIX_DST)) {
	xv_set_gc_op(display, info, gc, op, XV_USE_CMS_FG, XV_DEFAULT_FG_BG);
    } else {
	if (!PIX_OPCOLOR(op)) {
	    op |= PIX_COLOR(cms_index);
	}
	xv_set_gc_op(display, info, gc, op, XV_USE_OP_FG, XV_DEFAULT_FG_BG);
    }
    XDrawLine(display, d, gc, x0, y0, x1, y1);
}
