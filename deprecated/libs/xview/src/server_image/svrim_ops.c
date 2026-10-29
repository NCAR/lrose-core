#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)svrim_ops.c 20.44 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE 
 *	file for terms of the license.
 */

#include <stdio.h>

#include <xview_private/i18n_impl.h>
#include <xview_private/svrim_impl.h>
#include <pixrect/pixrect.h>
#include <pixrect/pixfont.h>
#include <pixrect/memvar.h>

#ifdef OW_I18N
#include <xview/font.h>
#endif /* OW_I18N */

Xv_public int
server_image_rop(dest, dx, dy, dw, dh, op, src, sx, sy)
    Xv_opaque       dest, src;
    int             dx, dy, dw, dh, sx, sy;
    unsigned long   op;
{

    short           dest_type = PR_TYPE(dest);
    short           src_type = PR_TYPE(src);

    switch (dest_type) {
      case MEMORY_PR:
	if (src_type == SERVER_IMAGE_PR) {
	    Xv_Drawable_info *info;

	    DRAWABLE_INFO_MACRO(src, info);
	    xv_read_internal(dest, dx, dy, dw, dh, op, xv_display(info),
			     xv_xid(info), sx, sy);
	} else {
	    xv_error(XV_ZERO,
		     ERROR_STRING,
		         XV_MSG("server_image_rop(): src is not a server image"),
		     ERROR_PKG, SERVER_IMAGE,
		     NULL);
	    return (PIX_ERR);
	}
	break;
      case SERVER_IMAGE_PR:{
	    Xv_Drawable_info *info;
	    Display        *display;
	    XID             xid;
	    GC              gc;
	    Pixrect        *npr;

	    DRAWABLE_INFO_MACRO(dest, info);
	    display = xv_display(info);
	    xid = xv_xid(info);
	    if ((src_type == MEMORY_PR) ||
		(src_type == SERVER_IMAGE_PR)) {
		gc = xv_find_proper_gc(display, info, PW_ROP);
		xv_set_gc_op(display, info, gc, op,
			     PIX_OPCOLOR(op) ? XV_USE_OP_FG : XV_USE_CMS_FG,
			     XV_DEFAULT_FG_BG);
		xv_rop_internal(display, xid, gc,
				dx, dy, dw, dh, src, sx, sy, info);
	    } else {
		npr = xv_mem_create(dw, dh, ((Pixrect *) src)->pr_depth);
		pr_rop(npr, 0, 0, dw, dh, PIX_SRC, src, sx, sy);
		gc = xv_find_proper_gc(display, info, PW_ROP);
		xv_set_gc_op(display, info, gc, op,
			     PIX_OPCOLOR(op) ? XV_USE_OP_FG : XV_USE_CMS_FG,
			     XV_DEFAULT_FG_BG);
		xv_rop_internal(display, xid, gc,
				dx, dy, dw, dh, npr, 0, 0, info);
	    }
	    break;
	}
      default:
	xv_error(XV_ZERO,
		 ERROR_STRING,
	XV_MSG("server_image_rop(): dest is not a memory pixrect or a server_image"),
		 ERROR_PKG, SERVER_IMAGE,
		 NULL);
	return (PIX_ERR);
    }
    return (0);
}

Xv_public int
server_image_stencil(dest, dx, dy, dw, dh, op, st, stx, sty, src, sx, sy)
    Xv_opaque       dest, src, st;
    int             dx, dy, dw, dh, sx, sy, stx, sty;
    unsigned long   op;
{

    short           dest_type = PR_TYPE(dest);
    short           src_type = PR_TYPE(src);
    short           stencil_type = PR_TYPE(st);
    Pixrect        *temp_pr, *temp_st;

    if (stencil_type != MEMORY_PR && stencil_type != SERVER_IMAGE_PR) {
	xv_error(XV_ZERO,
		 ERROR_STRING,
		     XV_MSG("server_image_stencil(): stencil is not a memory pr or a server image"),
		 ERROR_PKG, SERVER_IMAGE,
		 NULL);
	return (PIX_ERR);
    }
    switch (dest_type) {
      case MEMORY_PR:
	if (src_type == SERVER_IMAGE_PR) {
	    Xv_Drawable_info *info;
	    Display        *display;
	    XID             xid;

	    DRAWABLE_INFO_MACRO(src, info);
	    display = xv_display(info);
	    xid = xv_xid(info);
	    temp_pr = xv_mem_create(((Pixrect *) src)->pr_width,
				    ((Pixrect *) src)->pr_height,
				    ((Pixrect *) src)->pr_depth);
	    if (!temp_pr) {
		xv_error(XV_ZERO,
			 ERROR_STRING,
			     XV_MSG("server_image_stencil(): Can't create mpr in server_image_stencil"),
			 ERROR_PKG, SERVER_IMAGE,
			 NULL);
		return (PIX_ERR);
	    }
	    xv_read_internal(temp_pr, dx, dy, dw, dh, PIX_SRC,
			     display, xid, sx, sy);
	    /* if the stencil pr is a remote pr, then read it into memory */
	    if (stencil_type == SERVER_IMAGE_PR) {
		temp_st = xv_mem_create(((Pixrect *) st)->pr_width,
					((Pixrect *) st)->pr_height,
					((Pixrect *) st)->pr_depth);
		if (!temp_st) {
		    xv_error(XV_ZERO,
			     ERROR_STRING,
			         XV_MSG("server_image_stencil(): Can't create mpr in server_image_stencil"),
			     ERROR_PKG, SERVER_IMAGE,
			     NULL);
		    return (PIX_ERR);
		}
		xv_read_internal(temp_st, dx, dy, dw, dh, PIX_SRC,
				 display, st, sx, sy);
		/*
		 * At this point, everything is in memory. Just call the mem
		 * stencil routine to do the job.
		 */
		pr_stencil((Pixrect *) dest, dx, dy, dw, dh, op, temp_st, stx,
			   sty, temp_pr, sx, sy);
		(void) free((char *) temp_st);
		(void) free((char *) temp_pr);
	    } else {
		/*
		 * otherwise, stencil pr is guaranteed to be a mem_pr because
		 * of the check we did at the start of the routine
		 */
		pr_stencil((Pixrect *) dest, dx, dy, dw, dh, op, st, stx, sty,
			   temp_pr, sx, sy);
		(void) free((char *) temp_pr);
	    }
	} else {
	    /*
	     * If the src isn't image pr, and dest IS mpr, then this routine
	     * should never have been called.
	     */
	    xv_error(XV_ZERO,
		     ERROR_STRING,
		  XV_MSG("server_image_stencil(): dest is mpr, src isn't image pr"),
		     ERROR_PKG, SERVER_IMAGE,
		     NULL);
	    return (PIX_ERR);
	}
	break;
      case SERVER_IMAGE_PR:{
	    Xv_Drawable_info *dest_info;
	    Display        *display;
	    Xv_opaque       dest_standard;
	    GC              stencil_gc;

	    DRAWABLE_INFO_MACRO(dest, dest_info);
	    display = xv_display(dest_info);
	    XV_OBJECT_TO_STANDARD(dest, "server_image_stencil",
				  dest_standard);
	    stencil_gc = xv_find_proper_gc(display, dest_info, PW_STENCIL);

	    xv_set_gc_op(display, dest_info, stencil_gc, op,
			 PIX_OPCOLOR(op) ? XV_USE_OP_FG : XV_USE_CMS_FG,
			 XV_DEFAULT_FG_BG);
	    xv_stencil_internal(display, dest_info, xv_xid(dest_info),
				stencil_gc, dx, dy, dw, dy, st, stx, sty, src, sx, sy, dest_info);
	    break;
	}
      default:
	xv_error(XV_ZERO,
		 ERROR_STRING,
	       XV_MSG("server_image_stencil(): dest is not mpr or server_image_pr"),
		 ERROR_PKG, SERVER_IMAGE,
		 NULL);
	return (PIX_ERR);
	break;
    }
    return (0);
}

Xv_public int
server_image_replrop(dest, dx, dy, dw, dh, op, src, sx, sy)
    Xv_opaque       dest, src;
    int             dx, dy, dw, dh, sx, sy;
    unsigned long   op;
{
    short           dest_type = PR_TYPE(dest);
    short           src_type = PR_TYPE(src);
    Pixrect        *temp;

    /* BUG:  sx and sy ignored */
    switch (dest_type) {
      case MEMORY_PR:{
	    if (src_type == SERVER_IMAGE_PR) {
		/*
		 * Create a remote image copy of the (larger) destination
		 * pixrect
		 */
		temp = (Pixrect *) xv_create(0, SERVER_IMAGE,
					     XV_WIDTH, dw,
					     XV_HEIGHT, dh,
			   SERVER_IMAGE_DEPTH, ((Pixrect *) dest)->pr_depth,
					     NULL);
		if (!temp) {
		    xv_error(XV_ZERO,
			     ERROR_STRING,
		    XV_MSG("server_image_replrop(): Unable to create server image"),
			     ERROR_PKG, SERVER_IMAGE,
			     NULL);
		    return (PIX_ERR);
		}
		/* Replrop the src to the remote image copy */
		xv_replrop((Xv_opaque)temp, 0, 0, dw, dh, PIX_SRC,
                           (Pixrect *)src, sx, sy);

		/* Copy the remote image copy to the destination pixrect */
		pr_rop((Pixrect *) dest, dx, dy, dw, dh, op, temp, 0, 0);

		/* Destroy the remote image copy */
		xv_destroy((Xv_opaque)temp);
	    } else {
		xv_error(XV_ZERO,
			 ERROR_STRING,
		  XV_MSG("server_image_replrop(): dest is mpr, src isn't image pr"),
			 ERROR_PKG, SERVER_IMAGE,
			 NULL);
		return (PIX_ERR);
	    }
	    break;
	}
      case SERVER_IMAGE_PR:{
	    Xv_Drawable_info *info;
	    Display        *display;
	    XID             xid;
	    Xv_opaque       dest_standard;
	    GC              replrop_gc;

	    DRAWABLE_INFO_MACRO(dest, info);
	    display = xv_display(info);
	    xid = xv_xid(info);
	    XV_OBJECT_TO_STANDARD(dest, "server_image_replrop",
				  dest_standard);
	    replrop_gc = xv_find_proper_gc(display, info, PW_REPLROP);

	    if (src_type == MEMORY_PR || src_type == SERVER_IMAGE_PR) {
		xv_set_gc_op(display, info, replrop_gc, op,
			     PIX_OPCOLOR(op) ? XV_USE_OP_FG : XV_USE_CMS_FG,
			     XV_DEFAULT_FG_BG);
		xv_replrop_internal(display, info, xid,
			     replrop_gc, dx, dy, dw, dy, src, sx, sy, info);
	    } else {
		xv_error(XV_ZERO,
			 ERROR_STRING,
			 XV_MSG("server_image_replrop(): dest is image pr, src isn't image pr or mpr"),
			 ERROR_PKG, SERVER_IMAGE,
			 NULL);
		return (PIX_ERR);
	    }
	    break;
	}
      default:
	xv_error(XV_ZERO,
		 ERROR_STRING,
	       XV_MSG("server_image_replrop(): dest is not mpr or server_image_pr"),
		 ERROR_PKG, SERVER_IMAGE,
		 NULL);
	return (PIX_ERR);
    }

    return (0);
}

/*
 * The only way that this routine will get called is if dest is a
 * server_image
 */
Xv_public int
server_image_vector(dest, x0, y0, x1, y1, op, value)
    Xv_opaque       dest;
    int             x0, y0, x1, y1, op, value;
{
    xv_vector(dest, x0, y0, x1, y1, op, value);
    return (0);
}

Xv_public int
server_image_put(dest, x, y, value)
    Xv_opaque       dest;
    int             x, y, value;
{
    pw_put(dest, x, y, value);
    return (0);
}

Xv_public int
server_image_get(dest, x, y)
    Xv_opaque       dest;
    int             x, y;
{
    pw_get(dest, x, y);
    return (0);
}

Xv_public Pixrect *
server_image_region(dest, x, y, w, h)
    Xv_opaque       dest;
    int             x, y, w, h;
{
    xv_error(XV_ZERO,
	     ERROR_STRING, 
	     XV_MSG("server_image_region: Unsupported operation"),
	     ERROR_PKG, SERVER_IMAGE,
	     NULL);
    return (NULL);
}

Xv_public int
server_image_colormap(dest, index, count, red, green, blue)
    Xv_opaque       dest;
    int             index, count;
    unsigned char   red[], green[], blue[];
{
    xv_error(XV_ZERO,
	     ERROR_STRING, 
	     XV_MSG("Server images do not have associated colormaps"),
	     ERROR_PKG, SERVER_IMAGE,
	     NULL);
    return (PIX_ERR);
}

Xv_public int
server_image_pf_text(rpr, op, font, string)
    struct pr_prpos rpr;
    int             op;
    Pixfont        *font;
    char           *string;
{
    Xv_Drawable_info *info;
    Display        *display;
    XID             xid;
    GC              gc;

    DRAWABLE_INFO_MACRO((Xv_opaque) rpr.pr, info);
    display = xv_display(info);
    xid = xv_xid(info);
    gc = xv_find_proper_gc(display, info, PW_TEXT);

    xv_set_gc_op(display, info, gc, op,
		 PIX_OPCOLOR(op) ? XV_USE_OP_FG : XV_USE_CMS_FG,
		 XV_DEFAULT_FG_BG);
    XSetFont(display, gc, (Font)xv_get((Xv_opaque)font, XV_XID));
    XDrawImageString(display, xid, gc, rpr.pos.x, rpr.pos.y, string, strlen(string));
}


#ifdef OW_I18N
Xv_public int
server_image_pf_text_wc(rpr, op, font, string)
    struct pr_prpos rpr;
    int             op;
    Pixfont        *font;
    CHAR           *string;
{
    Xv_Drawable_info *info;
    Display        *display;
    XID             xid;
    GC              gc;

    DRAWABLE_INFO_MACRO((Xv_opaque) rpr.pr, info);
    display = xv_display(info);
    xid = xv_xid(info);
    gc = xv_find_proper_gc(display, info, PW_TEXT);

    xv_set_gc_op(display, info, gc, op,
		 PIX_OPCOLOR(op) ? XV_USE_OP_FG : XV_USE_CMS_FG,
		 XV_DEFAULT_FG_BG);
    XwcDrawImageString(display, xid,
		       (XFontSet)xv_get((Xv_opaque)font, FONT_SET_ID),
		       gc, rpr.pos.x, rpr.pos.y, string, STRLEN(string));
}
#endif /* OW_I18N */
