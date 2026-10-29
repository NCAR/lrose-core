#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)rect_util.c 20.13 93/06/28 Copyr 1984 Sun Micro";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * Various rectangle utilities
 */

#include <xview/rect.h>

static int
  rect_nearest_edge(int minimum, int delta, int val);

/*
 * Compute the distance from rect to (x, y). If (x, y) is in rect, zero is
 * returned. If x_used or y_used are non-zero, the projection point is
 * returned.
 */
int
rect_distance(rect, x, y, x_used, y_used)
    register Rect  *rect;
    register int    x, y;
    register int   *x_used, *y_used;
{
    int             near_x, near_y;
    register int    dist_sq, temp;

    near_x = rect_nearest_edge(rect->r_left, rect->r_width, x);
    near_y = rect_nearest_edge(rect->r_top, rect->r_height, y);
    temp = (near_x - x);
    dist_sq = temp * temp;
    temp = (near_y - y);
    dist_sq += temp * temp;
    if (x_used)
	*x_used = near_x;
    if (y_used)
	*y_used = near_y;
    return dist_sq;
}


static int
  rect_nearest_edge(int minimum, int delta, int val)
{
    return ((val <= minimum) ? minimum
	    : ((val > (minimum + delta)) ? (minimum + delta) : val));
}

int
rect_right_of(rect1, rect2)
    Rect           *rect1, *rect2;
{
    /* first, determine whether or not to the right-of rect1-> */
    if ((rect1->r_left + rect1->r_width <= rect2->r_left) &&
	!(rect1->r_top + rect1->r_height < rect2->r_top) &&
	!(rect1->r_top < rect2->r_top + rect2->r_height))
	return 1;
    else
	return 0;
}

int
rect_below(rect1, rect2)
    Rect           *rect1, *rect2;
{
    /* first, determine if not directly below rect1-> */
    if ((rect1->r_top + rect1->r_height <= rect2->r_top) &&
	!(rect1->r_left > rect2->r_left + rect2->r_width) &&
	!(rect1->r_left + rect1->r_width < rect2->r_left))
	return 1;
    else
	return 0;
}
