#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)pw_get.c 20.19 93/06/28 Copyr 1985 Sun Micro";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE 
 *	file for terms of the license.
 */

/*
 * Pw_get.c: Implement the pw_get functions of the pixwin.h interface.
 */

#include <xview_private/pw_impl.h>
#include <xview_private/draw_impl.h>

#ifdef __STDC__ 
#ifndef CAT
#define CAT(a,b)        a ## b 
#endif 
#endif
#include <pixrect/memvar.h>

#include <xview/window.h>

Xv_public int
pw_get(drawable, x, y)
    Xv_opaque       drawable;
    int             x, y;
{
    XImage	   	*ximage;
    Xv_Drawable_info  	*info; 
    unsigned 		pixel;

    DRAWABLE_INFO_MACRO(drawable, info);

    ximage = (XImage *)XGetImage(xv_display(info), xv_xid(info),
			    x, y, 1, 1, AllPlanes,
			    (xv_depth(info) == 1) ? XYPixmap : ZPixmap);

    /* 
     * pixel = XGetPixel(ximage, x, y);
     * For some reason, this doesnt work. Xlib bug ?
     */
    pixel = *((unsigned *)(ximage->data));
    pixel = pixel >> (sizeof(unsigned) * 8 - xv_depth(info));

    ximage->f.destroy_image(ximage);

    return((int)pixel);
}
