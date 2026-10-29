#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)xv_stencil.c 20.24 89/07/31";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE 
 *	file for terms of the license.
 */

/*
 * xv_stencil.c:
 */

#include <xview_private/pw_impl.h>
#include <xview_private/i18n_impl.h>
#include <xview/screen.h>

#ifdef __STDC__ 
#ifndef CAT
#define CAT(a,b)        a ## b 
#endif 
#endif
#include <pixrect/memvar.h>



/*
 * Replaces the old pw_stencil routine.  This routine also handles the cases
 * where either/or/both stpr and spr are remote pixrects(server_image
 * package.
 */
/* ARGSUSED */
Xv_private int
xv_stencil_internal(display, info, d, gc, dx, dy, width, height, stpr, stx, sty,
		    spr, sx, sy, dest_info)
    Display        *display;
    Xv_Drawable_info *info;
    Drawable        d;
    GC              gc;
    int             dx, dy, width, height, stx, sty, sx, sy;
    Xv_opaque       stpr, spr;
    Xv_Drawable_info *dest_info;
{
    short           st_type;
    Pixmap          temp_st;
    GC              st_gc;
    Screen_visual  *visual;
    Xv_Drawable_info temp_info;
    Xv_Drawable_info *stpr_info;

    if (!stpr) {
	return (XV_ERROR);
    }
    /*
     * if the stencil is a memory pr, create a pixmap and rop the bits over
     * to the server before doing XSetClipMask
     */
    st_type = PR_TYPE(stpr);
    if (st_type == MEMORY_PR) {
	if (((Pixrect *) stpr)->pr_depth != 1) {
	    xv_error(stpr,
		     ERROR_STRING, 
			XV_MSG("xv_stencil: stencil has to be of depth 1"),
		     NULL);
	    return (XV_ERROR);
	}
	if (!(temp_st = XCreatePixmap(display, d,
		((Pixrect *) stpr)->pr_width, ((Pixrect *) stpr)->pr_height,
				      ((Pixrect *) stpr)->pr_depth))) {
	    return (XV_ERROR);
	}
	temp_info.visual = visual = (Screen_visual *)xv_get(xv_screen(dest_info),
	    SCREEN_IMAGE_VISUAL, d, ((Pixrect *)stpr)->pr_depth);
	temp_info.private_gc = 0;
	temp_info.cms = dest_info->cms;
	st_gc = visual->gc;

	xv_set_gc_op(display, info, st_gc, PIX_SRC, XV_USE_CMS_FG,
		     XV_DEFAULT_FG_BG);
	if (xv_rop_internal(display, temp_st, st_gc, 0, 0,
		((Pixrect *) stpr)->pr_width, ((Pixrect *) stpr)->pr_height,
			    stpr, 0, 0, &temp_info) == XV_ERROR) {
	    return (XV_ERROR);
	}
    } else {
	DRAWABLE_INFO_MACRO(stpr, stpr_info);
	temp_st = xv_xid(stpr_info);
    }

    XSetClipMask(display, gc, temp_st);
    XSetClipOrigin(display, gc, dx, dy);
    if (spr) {
	if (xv_rop_internal(display, d, gc, dx, dy, width, height, spr, sx, sy,
			    dest_info) == XV_ERROR) {
	    return (XV_ERROR);
	}
    } else {
	/*
	 * If the spr is NULL, the color specified in the op to xv_stencil
	 * must be painted through the stencil
	 */
	XFillRectangle(display, d, gc, dx, dy, width, height);
    }

    if (st_type == MEMORY_PR) {
	XFreePixmap(display, temp_st);
    }
    return (XV_OK);
}

Xv_public int
xv_stencil(window, dx, dy, width, height, op, stpr, stx, sty, spr, sx, sy)
    Xv_opaque       window;
    int             op, dx, dy, width, height;
    struct pixrect *stpr;
    int             stx, sty;
    struct pixrect *spr;
    int             sx, sy;
{
    Xv_Drawable_info *info;
    Display        *display;
    GC              gc;

    DRAWABLE_INFO_MACRO(window, info);
    display = xv_display(info);
    gc = xv_find_proper_gc(display, info, PW_STENCIL);

    if (!spr) {
	xv_set_gc_op(display, info, gc, op, XV_USE_OP_FG,
		     XV_DEFAULT_FG_BG);
    } else {
	xv_set_gc_op(display, info, gc, op,
		     PIX_OPCOLOR(op) ? XV_USE_OP_FG : XV_USE_CMS_FG,
		     XV_DEFAULT_FG_BG);
    }

    if (xv_stencil_internal(display, info, xv_xid(info), gc, dx, dy, width, height,
			    stpr, stx, sty, spr, sx, sy, info) == XV_ERROR) {
	xv_error(XV_ZERO,
		 ERROR_STRING, 
		 XV_MSG("xv_stencil: xv_stencil_internal failed"),
		 NULL);
    }
}
