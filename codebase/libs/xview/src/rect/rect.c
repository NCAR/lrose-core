#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)rect.c 20.17 93/06/28 Copyr 1984 Sun Micro";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * Overview:	Implements the interface described by rect.h which defines
 * the rectangle structure
 */

#include <stdio.h>
#include <xview_private/i18n_impl.h>
#include <xview/base.h>		/* HUH? xv_error.h also does this */
#include <xview/rect.h>
#include <xview/xv_error.h>

/*
 * Rect geometry functions
 */
void rect_intersection(r1, r2, r)
    register struct rect *r1, *r2, *r;
{
    r->r_left = MAX(r1->r_left, r2->r_left);
    r->r_top = MAX(r1->r_top, r2->r_top);
    r->r_width = MIN(r1->r_left + r1->r_width, r2->r_left + r2->r_width) -
	r->r_left;
    if (r->r_width < 0)
	r->r_width = 0;
    r->r_height = MIN(r1->r_top + r1->r_height, r2->r_top + r2->r_height) -
	r->r_top;
    if (r->r_height < 0)
	r->r_height = 0;
    return;
}

unsigned
rect_clipvector(r, x1arg, y1arg, x2arg, y2arg)
    register struct rect *r;
    int            *x1arg, *y1arg, *x2arg, *y2arg;
{				/* clip vector 1=>2 to r, return TRUE if
				 * visible. Cohen-Sutherland algorithm. */
    register short  x1 = *x1arg, y1 = *y1arg, x2 = *x2arg, y2 = *y2arg;
    register char   bits1, bits2, bits;
    coord           t, n, d, p;
    unsigned        done, accept;

    done = FALSE;
    do {
	bits1 = 0;
	if (y1 < r->r_top)
	    bits1 |= 1;
	if (y1 > rect_bottom(r))
	    bits1 |= 2;
	if (x1 > rect_right(r))
	    bits1 |= 4;
	if (x1 < r->r_left)
	    bits1 |= 8;
	bits2 = 0;
	if (y2 < r->r_top)
	    bits2 |= 1;
	if (y2 > rect_bottom(r))
	    bits2 |= 2;
	if (x2 > rect_right(r))
	    bits2 |= 4;
	if (x2 < r->r_left)
	    bits2 |= 8;

	accept = ((bits1 | bits2) == 0);
	if (accept)
	    done = TRUE;	/* vector all inside clip volume */
	else {
	    accept = ((bits1 & bits2) == 0);
	    if (!accept)
		done = TRUE;	/* vec all outside clip vol  */
	    else {		/* vector needs clip	 */
		if (bits1 == 0) {	/* make sure v1 is outside	 */
		    p = x1;
		    x1 = x2;
		    x2 = p;
		    p = y1;
		    y1 = y2;
		    y2 = p;
		    bits = bits1;
		    bits1 = bits2;
		    bits2 = bits;
		}
		if (bits1 & 1) {/* clip above	 */
		    n = (r->r_top - y1);
		    d = (y2 - y1);
		    t = (x2 - x1);
		    t *= n;
		    t /= d;
		    x1 = x1 + t;
		    y1 = r->r_top;
		} else if (bits1 & 2) {	/* clip below	 */
		    n = (rect_bottom(r) - y1);
		    d = (y2 - y1);
		    t = (x2 - x1);
		    t *= n;
		    t /= d;
		    x1 = x1 + t;
		    y1 = rect_bottom(r);
		} else if (bits1 & 4) {	/* clip right	 */
		    n = (rect_right(r) - x1);
		    d = (x2 - x1);
		    t = (y2 - y1);
		    t *= n;
		    t /= d;
		    y1 = y1 + t;
		    x1 = rect_right(r);
		} else if (bits1 & 8) {	/* clip r_left	 */
		    n = (r->r_left - x1);
		    d = (x2 - x1);
		    t = (y2 - y1);
		    t *= n;
		    t /= d;
		    y1 = y1 + t;
		    x1 = r->r_left;
		}
	    }
	}
    } while (!done);
    *x1arg = x1;
    *y1arg = y1;
    *x2arg = x2;
    *y2arg = y2;
    return (accept);
}

unsigned
rect_order(r1, r2, sortorder)
    struct rect    *r1, *r2;
    int             sortorder;
{				/* Return true if r1 & r2 are in the
				 * specified relationship. */
    switch (sortorder) {
      case RECTS_TOPTOBOTTOM:
	if (r1->r_top <= r2->r_top)
	    return (TRUE);
	break;
      case RECTS_BOTTOMTOTOP:
	if (rect_bottom(r1) >= rect_bottom(r2))
	    return (TRUE);
	break;
      case RECTS_LEFTTORIGHT:
	if (r1->r_left <= r2->r_left)
	    return (TRUE);
	break;
      case RECTS_RIGHTTOLEFT:
	if (rect_right(r1) >= rect_right(r2))
	    return (TRUE);
	break;
      case RECTS_UNSORTED:
	return (TRUE);
	break;
      default:
	xv_error(XV_ZERO,
		 ERROR_STRING, 
		 XV_MSG("Bad sortorder arg in mostRect"),
		 NULL);
	break;
    }
    return (FALSE);
}

struct rect
rect_bounding(r1, r2)
    register struct rect *r1, *r2;
{
    struct rect     r;

    if (rect_isnull(r1))
	r = *r2;
    else if (rect_isnull(r2))
	r = *r1;
    else {
	r.r_left = MIN(r1->r_left, r2->r_left);
	r.r_top = MIN(r1->r_top, r2->r_top);
	r.r_width = MAX(r1->r_left + r1->r_width, r2->r_left + r2->r_width)
	    - r.r_left;
	r.r_height = MAX(r1->r_top + r1->r_height, r2->r_top + r2->r_height)
	    - r.r_top;
    }
    return (r);
}
