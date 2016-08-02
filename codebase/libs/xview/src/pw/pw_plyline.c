#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)pw_plyline.c 20.23 89/07/31";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE 
 *	file for terms of the license.
 */

#include <xview_private/pw_impl.h>
#include <pixrect/pr_line.h>

extern short    pw_tex_dashed[];

Xv_public int
pw_polyline(pw, dx, dy, npts, ptlist, mvlist, brush, tex, op)
    int             npts, op;
    int             dx, dy;
    u_char         *mvlist;
    struct pr_brush *brush;
    Xv_opaque       pw;
    struct pr_pos  *ptlist;
    Pr_texture     *tex;
{

    Xv_Drawable_info *info;
    Display        *display;
    Drawable        xid;
    GC              line_gc;
    XGCValues       gc_val;
    unsigned long   gc_mask = GCLineWidth | GCLineStyle;
    char           *dash_set;
    int             p_len;
    char           *pw_short_to_char();
    register int    i, j;
    short           start, close = 0;
    u_char         *t_mvlist;
    XPoint         *points;

    points = (XPoint *) xv_malloc(npts * sizeof(XPoint));

    for (i = 0; i < npts; i++) {
	points[i].x = (short) ptlist[i].x + dx;
	points[i].y = (short) ptlist[i].y + dy;
    }

    DRAWABLE_INFO_MACRO(pw, info);
    display = xv_display(info);
    xid = xv_xid(info);
    line_gc = xv_find_proper_gc(display, info, PW_POLYLINE);

    xv_set_gc_op(display, info, line_gc, op, XV_USE_OP_FG,
		 XV_DEFAULT_FG_BG);
    gc_val.line_width = (!brush || !(brush->width)) ? 1 : brush->width;
    gc_val.line_style = ((!tex || tex->pattern == pw_tex_dashed) ? LineSolid
			 : LineOnOffDash);

    XChangeGC(display, line_gc, gc_mask, &gc_val);
    if (tex) {
	dash_set = pw_short_to_char(tex->pattern, &p_len);
	XSetDashes(display, line_gc, tex->offset, dash_set, p_len);
    }
    if ((mvlist == POLY_CLOSE) || ((mvlist != NULL) && (mvlist[0]))) {
	close = 1;
    }
    if (mvlist == POLY_DONTCLOSE || mvlist == POLY_CLOSE) {
	XDrawLines(display, xid, line_gc, points, npts, CoordModeOrigin);
	if (close) {
	    XDrawLine(display, xid, line_gc, points[npts - 1].x,
		      points[npts - 1].y, points[0].x, points[0].y);
	}
    } else {
	t_mvlist = ++mvlist;
	i = 1;
	j = npts;
	start = 0;
	while (j--) {
	    if (*t_mvlist || (!j)) {
		/*
		 * we are here if the entry in mv_list is set or if we run
		 * out of points, and have to do the final draw!
		 */
		if (i) {	/* check to see if there is anything needed
				 * to be drawn */
		    XDrawLines(display, xid, line_gc, &(points[start]), i,
			       CoordModeOrigin);
		    if (close) {
			XDrawLine(display, xid, line_gc,
			   points[start + i - 1].x, points[start + i - 1].y,
				  points[start].x, points[start].y);
		    }
		    i = 1;
		    start = npts - j;
		}
	    } else {
		i++;
	    }
	    t_mvlist++;
	}
    }

    (void) free((char *) points);

    if (tex) {
	(void) free((char *) dash_set);
    }
}				/* pw_polyline */
