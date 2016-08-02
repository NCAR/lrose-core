#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)pw_plygon2.c 20.28 89/07/31";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE 
 *	file for terms of the license.
 */

#include <xview_private/pw_impl.h>
#include <pixrect/memvar.h>
#include <xview/svrimage.h>

Xv_public int
pw_polygon_2(pw, dx, dy, nbds, npts, vlist, op, spr, sx, sy)
    register struct pixwin *pw;
    register int    dx, dy;
    int             nbds;
    int             npts[];
    struct pr_pos  *vlist;
    int             op;
    struct pixrect *spr;
    int             sx, sy;
{
    Xv_Drawable_info *info;
    Display        *display;
    Drawable        xid;
    GC              fill_gc;
    XGCValues       gc_val;
    unsigned int    gc_mask = GCTileStipXOrigin | GCTileStipYOrigin;
    short           i;
    short           depth;
    int             temp_total = 0;
    Server_image    tile;
    Pixmap          tile_id;
    XPoint         *points;

    DRAWABLE_INFO_MACRO((Xv_opaque) pw, info);
    display = xv_display(info);
    xid = xv_xid(info);
    fill_gc = xv_find_proper_gc(display, info, PW_POLYGON2);
    /*
     * If the passed pixrect is not already a server_image, create a remote
     * pixrect for filling the polygons
     */
    if (spr) {
	if (PR_NOT_SERVER_IMAGE(spr)) {
	    tile = xv_create(xv_screen(info), SERVER_IMAGE,
			     XV_HEIGHT, spr->pr_height,
			     XV_WIDTH, spr->pr_width,
			     SERVER_IMAGE_DEPTH, spr->pr_depth,
			     SERVER_IMAGE_BITS, (mpr_d(spr))->md_image,
			     NULL);
	    tile_id = (unsigned long) xv_get(tile, XV_XID);
	} else {
	    tile_id = (unsigned long) xv_get((Server_image) spr, XV_XID);
	}
#ifdef VAX_8PLANE_ONLY
	depth = 8;
#else
	depth = spr->pr_depth;
#endif

	if (depth == xv_depth(info)) {
	    gc_val.tile = tile_id;
	    gc_val.fill_style = FillTiled;
	    XChangeGC(display, fill_gc, GCFillStyle | GCTile, &gc_val);
#ifndef SVR4
	} else {
#else /* SVR4 */
	} else if (depth == 1) {
#endif /* SVR4 */
	    /*
	     * BUG ALERT! This only works in a 1->8 bit case, but won't it
	     * get called for the 8->1 bit case as well? e.g. you can't
	     * stipple an 8-bit pixmap into a 1-bit deep window. There should
	     * be a third case for this. -DMC
	     */
	    gc_val.stipple = tile_id;
	    gc_val.fill_style = FillOpaqueStippled;
	    XChangeGC(display, fill_gc, GCFillStyle | GCStipple, &gc_val);
	}
    } else {
        /*
	 * The tile pixmap depth must equal the destination drawable depth.
	 * If not equal fill solid is used.
	 */
	XSetFillStyle(display, fill_gc, FillSolid);
    }
    /*
     * Figure out how many XPoint to allocate
     */
    temp_total = 0;
    for (i = 0; i < nbds; i++) {
	temp_total += npts[i];
    }
    points = (XPoint *) xv_malloc(temp_total * sizeof(XPoint));
    for (i = 0; i < temp_total; i++) {
	points[i].x = (short) vlist[i].x + dx;
	points[i].y = (short) vlist[i].y + dy;
    }

    if (!spr) {
	xv_set_gc_op(display, info, fill_gc, op, XV_USE_OP_FG,
		     XV_DEFAULT_FG_BG);
    } else {
	xv_set_gc_op(display, info, fill_gc, op,
		     PIX_OPCOLOR(op) ? XV_USE_OP_FG : XV_USE_CMS_FG,
		     XV_DEFAULT_FG_BG);
    }

    /*
     * Make (dx,dy) in the destination coincide with (sx,sy) in the source
     * pixrect.
     */
    temp_total = 0;
    for (i = 0; i < nbds; i++) {
	/*
	 * BUG ALERT! Wouldn't this be smaller using XSetTSOrigin? -DMC
	 */
	gc_val.ts_x_origin = dx - sx;
	gc_val.ts_y_origin = dy - sy;
	XChangeGC(display, fill_gc, gc_mask, &gc_val);
	XFillPolygon(display, xid, fill_gc, &(points[temp_total]),
		     npts[i], Complex, CoordModeOrigin);
	temp_total += npts[i];
    }
    free(points);
    if (spr && PR_NOT_SERVER_IMAGE(spr)) {
	xv_destroy(tile);
    }
}
