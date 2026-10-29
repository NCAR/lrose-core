/*#ident "@(#)ol_sb.c	1.28 92/01/15 SMI" */

/*
 * Copyright 1990 Sun Microsystems
 */

/*
 * OPEN LOOK object drawing package
 * 
 * ol_sb.c Scrollbar Module
 */

#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <olgx_private/olgx_impl.h>


/*
 * olgx_draw_scrollbar() Renders the whole scrollbar including the elevator,
 * anchor and proportional indicator Public Routine
 */

void
olgx_draw_scrollbar(ginfo, win, x, y, length, elev_pos, old_elev_pos,
		    prop_pos, prop_length, state)
    Graphics_info  *ginfo;
    Window          win;
    int             x, y;	/* Origin of the scrollbar  */
    int             length;	/* Length of the Scrolbar   */
    int             elev_pos, old_elev_pos;	/* Elevator Position New and
						 * Old */
    int             prop_pos, prop_length;	/* Proportional Indicator
						 * Posistion and lenght */
    int             state;


{
    int             cable_offset;	/* Cable Offset distance from the
					 * Scrollbar origin x,y */
    int             cable_width;
    short           sbheight;	/* Elevator Height */
    XRectangle      rectangles[2];
    XRectangle      rect1, rect2, *result_rect = (XRectangle *) 0;
    XRectangle     *olgx_compute_intersection();
    int		    tmp_dimension;


    if (state & OLGX_ABBREV)
	sbheight = ginfo->abbsb_height;
    else
	sbheight = ScrollbarElevator_Height(ginfo);

    cable_offset = ginfo->cable_offset;
    cable_width = ginfo->cable_width;



    /*
     * Assumption: If Prop_length is 0, we are assuming the prop_pos
     * coincides with the elev_pos or falls within the elevator
     */

    if (state & OLGX_HORIZONTAL) {

	if (state & OLGX_UPDATE) {

	    /* Erase the region which is necessary */

	    rect1.x = elev_pos;
	    rect1.y = y;
	    rect2.x = old_elev_pos;
	    rect2.y = y;
	    rect1.width = rect2.width = sbheight;
	    rect1.height = rect2.height = ScrollbarElevator_Width(ginfo);

	    result_rect = olgx_compute_intersection(&rect2, &rect1);
	    XClearArea(ginfo->dpy, win,
                          result_rect->x - 1, result_rect->y, 
                          result_rect->width + 1, result_rect->height + 1,0);


	}
	/* draw the 50% grey cable  */

	rectangles[0].y = rectangles[1].y = y + cable_offset;
	rectangles[0].x = x + cable_offset + 2;
	rectangles[1].x = (prop_length) ? (prop_pos + prop_length)
	    : (elev_pos + sbheight);
	rectangles[0].height = rectangles[1].height = cable_width;

	tmp_dimension  = (prop_length) ? (prop_pos - rectangles[0].x)
	    : (elev_pos - rectangles[0].x - 1);
	rectangles[0].width= (tmp_dimension > 0)?tmp_dimension:0;

	tmp_dimension = (x + length - cable_offset - 2)
	    - rectangles[1].x;
	rectangles[1].width= (tmp_dimension > 0)?tmp_dimension:0;

	olgx_scroll_stipple_rects(ginfo, win, &rectangles[0], 2);


	/*
	 * render the black(bg3) proportional indicator cable of the
	 * scrollbar
	 */

	if (prop_length) {

	    rectangles[0].y = rectangles[1].y = y + cable_offset;
	    rectangles[0].x = prop_pos;
	    rectangles[1].x = elev_pos + sbheight + 1;
	    rectangles[0].height = rectangles[1].height = cable_width;
	    rectangles[0].width = elev_pos - prop_pos;

	    if (rectangles[0].width)
		rectangles[0].width -= 1;

	    tmp_dimension = (prop_pos + prop_length) - 
                                   (elev_pos + sbheight - 1);
	    rectangles[1].width= (tmp_dimension > 0)?tmp_dimension:0;

	    XFillRectangles(ginfo->dpy, win, (ginfo->three_d) ?
			    ginfo->gc_rec[OLGX_BG3]->gc :
			    ginfo->gc_rec[OLGX_BLACK]->gc,
			    &rectangles[0], 2);
	}
	olgx_draw_elevator(ginfo, win, elev_pos, y,
			   state);

    } else {			/* Vertical Scrollbar */


	if (state & OLGX_UPDATE) {

	    /* Erase the region which is necessary */

	    rect1.x = x;
	    rect1.y = elev_pos;
	    rect2.x = x;
	    rect2.y = old_elev_pos;
	    rect1.width = rect2.width = ScrollbarElevator_Width(ginfo);
	    rect1.height = rect2.height = sbheight + 1;

	    result_rect = olgx_compute_intersection(&rect2, &rect1);

	    XClearArea(ginfo->dpy, win, 
                           result_rect->x - 1, result_rect->y - 1,
                           result_rect->width + 1, result_rect->height + 1,0);


	}
	/* render the 50% grey cable  of the scrollbar  */

	rectangles[0].x = rectangles[1].x = x + cable_offset;
	rectangles[0].y = y + cable_offset + 2;
	rectangles[1].y = (prop_length) ? (prop_pos + prop_length)
	    : (elev_pos + sbheight + 1);
	rectangles[0].width = rectangles[1].width = cable_width;

	tmp_dimension = (prop_length) ? (prop_pos - rectangles[0].y)
	    : (elev_pos - rectangles[0].y - 1);

	rectangles[0].height = (tmp_dimension > 0)?tmp_dimension:0;

	tmp_dimension = (y + length - cable_offset - 2)
	    - rectangles[1].y;
	rectangles[1].height = (tmp_dimension > 0)?tmp_dimension:0;

	olgx_scroll_stipple_rects(ginfo, win, rectangles, 2);

	/*
	 * render the black(bg3) proportional indicator cable of the
	 * scrollbar
	 */

	if (prop_length) {

	    rectangles[0].x = rectangles[1].x = x + cable_offset;
	    rectangles[0].y = prop_pos;
	    rectangles[1].y = elev_pos + sbheight + 1;
	    rectangles[0].width = rectangles[1].width = cable_width;
	    rectangles[0].height = elev_pos - prop_pos;
	    if (rectangles[0].height)
		rectangles[0].height -= 1;

	    tmp_dimension = (prop_pos + prop_length) - 
                                   (elev_pos + sbheight - 1);
	    rectangles[1].height = (tmp_dimension > 0)?tmp_dimension:0;

	    XFillRectangles(ginfo->dpy, win, (ginfo->three_d) ? 
                                               ginfo->gc_rec[OLGX_BG3]->gc :
			                       ginfo->gc_rec[OLGX_BLACK]->gc,
			    rectangles, 2);
	}
	olgx_draw_elevator(ginfo, win, x, elev_pos, state);
    }

    if (result_rect != (XRectangle *) 0)
	free((char *) result_rect);

}


/*
 * olgx_draw_elvator() Private Routine Renders the elevator of the scrollbar
 */


void
olgx_draw_elevator(info, win, x, y, state)
    Graphics_info  *info;
    Window          win;
    int             state;
{
    char            string[2];
    int             initial;
    int             newy = 0;
    int             newx = 0;
    int		    sbheight;

    if (state & OLGX_ABBREV)
	sbheight = info->abbsb_height;
    else
	sbheight = ScrollbarElevator_Height(info);

    if ((state & OLGX_ERASE))
	XClearArea(info->dpy, win, 
                       x - 1, y - 1,
		       (state & OLGX_HORIZONTAL) ? 
		          (sbheight + 2) : ((ScrollbarElevator_Width(info)) + 2),
		       (state & OLGX_VERTICAL) ? 
		          (sbheight + 2) : ((ScrollbarElevator_Width(info)) + 2),0);

    XFillRectangle(info->dpy, win, info->three_d ? info->gc_rec[OLGX_BG1]->gc :
                                             info->gc_rec[OLGX_WHITE]->gc,x,y,
           	   (state & OLGX_HORIZONTAL) ? 
	            (sbheight-1 ) : ((ScrollbarElevator_Width(info))-1),
		   (state & OLGX_VERTICAL) ? 
		    (sbheight-1):((ScrollbarElevator_Width(info))-1));

    if (info->three_d) {	/* 3d begins */


	if (state & OLGX_ABBREV)
	    initial = (state & OLGX_HORIZONTAL) ? 
                       HORIZ_ABBREV_SB_UL : VERT_ABBREV_SB_UL;
	else
	    initial = (state & OLGX_HORIZONTAL) ? HORIZ_SB_UL : VERT_SB_UL;

	string[0] = initial;
	XDrawString(info->dpy, win, info->gc_rec[OLGX_WHITE]->gc, x, y,
		    string, 1);

	string[0] = initial + 1;
	XDrawString(info->dpy, win, info->gc_rec[OLGX_BG3]->gc, x, y, string, 1);

	if (state & OLGX_SCROLL_BACKWARD) {

	    newx = x;
	    newy = y;
	    string[0] = (state & OLGX_VERTICAL) ? VERT_SB_TOPBOX_FILL
		: HORIZ_SB_LEFTBOX_FILL;
	    XDrawString(info->dpy, win, info->gc_rec[OLGX_BG2]->gc, newx, y,
			string, 1);

	} else if (state & OLGX_SCROLL_FORWARD) {
	    newy = (state & OLGX_HORIZONTAL) ? y :
		y + (((ScrollbarElevator_Height(info) - 2) / 3) << 1);
	    newx = (state & OLGX_VERTICAL) ? x :
		x + (((ScrollbarElevator_Height(info) - 2) / 3) << 1);

	    if (state & OLGX_ABBREV) {

		if (state & OLGX_VERTICAL)
		    newy = y + ((ScrollbarElevator_Height(info) - 2) / 3);
		else if (state & OLGX_HORIZONTAL)
		    newx = x + ((ScrollbarElevator_Height(info) - 2) / 3);
	    }
	    string[0] = (state & OLGX_VERTICAL) ? VERT_SB_BOTBOX_FILL :
		HORIZ_SB_RIGHTBOX_FILL;
	    XDrawString(info->dpy, win, info->gc_rec[OLGX_BG2]->gc, newx, newy,
                        string, 1);

	} else if (state & OLGX_SCROLL_ABSOLUTE) {

	    newx = (state & OLGX_VERTICAL) ? x :
		x + ((ScrollbarElevator_Height(info) - 2) / 3) + 1;
	    newy = (state & OLGX_HORIZONTAL) ? y - 1 :
		y + ((ScrollbarElevator_Height(info) - 2) / 3);

	    string[0] = DIMPLE_UL;
	    XDrawString(info->dpy, win, info->gc_rec[OLGX_BLACK]->gc, newx, newy,
			string, 1);

	    string[0] = DIMPLE_LR;
	    XDrawString(info->dpy, win, info->gc_rec[OLGX_WHITE]->gc, newx, newy,
			string, 1);

	    string[0] = DIMPLE_FILL;
	    XDrawString(info->dpy, win, info->gc_rec[OLGX_BG2]->gc, newx, newy,
			string, 1);

	    newx = 0;
	    newy = 0;
	}
	if ((newy) || (newx)) {

	    string[0] = (state & OLGX_VERTICAL) ? VERT_SB_BOX_UL : HORIZ_SB_BOX_UL;
	    XDrawString(info->dpy, win, info->gc_rec[OLGX_BG3]->gc, newx, newy,
                        string, 1);

	    string[0] = (state & OLGX_VERTICAL) ? VERT_SB_BOX_LR : HORIZ_SB_BOX_LR;
	    XDrawString(info->dpy, win, info->gc_rec[OLGX_WHITE]->gc, newx, newy, 
                        string, 1);

	}
       if (state & OLGX_INACTIVE)
        olgx_stipple_rect(info, win, x, y, 
 		         (state & OLGX_HORIZONTAL) ? 
		          (sbheight + 2) : ((ScrollbarElevator_Width(info))+2),
		         (state & OLGX_VERTICAL) ? 
		          (sbheight + 2):((ScrollbarElevator_Width(info))+2));
    }

     /* 3d ends */ 

    else {

	/* 2d begins */

	if (state & OLGX_ABBREV) {	/* abbrev scroll bar */

	    if (state & OLGX_SCROLL_BACKWARD)
		initial = (state & OLGX_HORIZONTAL) ?
		    OLG_HSB_REDUCED_ELEVATOR_LINE_BACKWARD :
		    OLG_VSB_REDUCED_ELEVATOR_LINE_BACKWARD;

	    else if (state & OLGX_SCROLL_FORWARD)
		initial = (state & OLGX_HORIZONTAL) ?
		    OLG_HSB_REDUCED_ELEVATOR_LINE_FORWARD :
		    OLG_VSB_REDUCED_ELEVATOR_LINE_FORWARD;

	    else		/* default - normal -uninvoked scrollbar */
		initial = (state & OLGX_HORIZONTAL) ?
		    OLG_HSB_REDUCED_ELEVATOR :
		    OLG_VSB_REDUCED_ELEVATOR;

	} else {		/* normal scrollbar */

	    if (state & OLGX_SCROLL_BACKWARD)
		initial = (state & OLGX_HORIZONTAL) ?
		    OLG_HSB_ELEVATOR_LINE_BACKWARD :
		    OLG_VSB_ELEVATOR_LINE_BACKWARD;

	    else if (state & OLGX_SCROLL_NO_BACKWARD)
		initial = (state & OLGX_HORIZONTAL) ?
		    HORIZ_SB_NO_BACK_OUTLINE :
		    VERT_SB_NO_BACK_OUTLINE;

	    else if (state & OLGX_SCROLL_NO_FORWARD)
		initial = (state & OLGX_HORIZONTAL) ?
		    HORIZ_SB_NO_FWD_OUTLINE :
		    VERT_SB_NO_FWD_OUTLINE;

	    else if (state & OLGX_INACTIVE)
		initial = (state & OLGX_HORIZONTAL) ?
		    HORIZ_SB_INACTIVE_OUTLINE :
		    VERT_SB_INACTIVE_OUTLINE;

	    else if (state & OLGX_SCROLL_FORWARD)
		initial = (state & OLGX_HORIZONTAL) ?
		    OLG_HSB_ELEVATOR_LINE_FORWARD :
		    OLG_VSB_ELEVATOR_LINE_FORWARD;

	    else if (state & OLGX_SCROLL_ABSOLUTE)
		initial = (state & OLGX_HORIZONTAL) ?
		    OLG_HSB_ELEVATOR_ABSOLUTE :
		    OLG_VSB_ELEVATOR_ABSOLUTE;

	    else		/* default - normal -uninvoked scrollbar */
		initial = (state & OLGX_HORIZONTAL) ? OLG_HSB_ELEVATOR :
		    OLG_VSB_ELEVATOR;


	}

	string[0] = initial;
	XDrawString(info->dpy, win, info->gc_rec[OLGX_BLACK]->gc, x, y, string, 1);
    }

}



/*
 * olgx_scroll_stipple_rects () Private Routine olgx_scroll_stipple_rect
 * filles rectangles(struct Xrectangles) with the grey stipple. The
 * diffreence beteween olgx_stipple_rect and olgx_scroll_stipple_rects is  it
 * uses a different background and foreground color for the same stipple and
 * hence a new GC called SCROLL_GREY_GC
 * 
 */


void
olgx_scroll_stipple_rects(info, win, rects, numrects)
    Graphics_info  *info;
    Window          win;
    XRectangle     *rects;
    int             numrects;
{


    /* Check to see if the GC already exists */
    /* else Create a new one                 */

    if (!info->gc_rec[OLGX_SCROLL_GREY_GC])
	olgx_initialise_gcrec(info, OLGX_SCROLL_GREY_GC);

    XFillRectangles(info->dpy, win, info->gc_rec[OLGX_SCROLL_GREY_GC]->gc,
		    rects, numrects);

}





/*
 * olgx_compute_intersection computes the intersection between two rectangles
 * and either: returns the size and pos of the portion of the first rectangle
 * not intersecting with the second rect . or returns NULL if they do not
 * intersect . Private Routine
 * 
 * Callers responsibility to free the result!
 */

XRectangle     *
olgx_compute_intersection(rect1, rect2)
    XRectangle     *rect1;
    XRectangle     *rect2;

{

    Region          region1;
    Region          region2;
    XRectangle     *result_rect;

    /* Create two Regions out of the two rects passed */
    region1 = XCreateRegion();
    XUnionRectWithRegion(rect1, region1, region1);
    region2 = XCreateRegion();
    XUnionRectWithRegion(rect2, region2, region2);

    /* Get the 1st region minus the second region */
    XSubtractRegion(region1, region2, region2);

    /* Get the smallest rect enclsing the result rect */
    result_rect = (XRectangle *) malloc(sizeof(XRectangle));
    XClipBox(region2, result_rect);

    /* Free the region data */
    XDestroyRegion(region1);
    XDestroyRegion(region2);

    /* Callers responsibility to free result_rect if not null */
    return (result_rect);


}
