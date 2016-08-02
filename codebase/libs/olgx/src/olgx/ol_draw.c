/*#ident "@(#)ol_draw.c	1.34 93/06/28 SMI" */

/*
 * Copyright 1990 Sun Microsystems
 */

/*
 * OPEN LOOK object drawing package Sun Microsystems, Inc.
 */

#include <stdio.h>
#include <X11/Xlib.h>

#include <olgx_private/olgx_impl.h>

int
calc_add_ins(width, add_ins)
    int             width;
    short           add_ins[STRING_SIZE];
{
    register int    this_log2 = 4;
    register int    this_bit;
    int             nchars = 0;

    this_bit = 1 << this_log2;

    for (this_bit = 1 << this_log2;
	 this_log2 >= 0 && width && nchars < STRING_SIZE;
	 this_bit = this_bit >> 1, this_log2--) {

	while (width >= this_bit) {
	    width -= this_bit;
	    add_ins[nchars++] = this_log2;

	}

    }
    return (nchars);
}

void
olgx_draw_box(info, win, x, y, width, height, state, fill_in)
    Graphics_info  *info;
    Window          win;
    int             x, y, width, height, state;
    Bool            fill_in;

{


    width -= 1;			/* This takes care of the fact , that the
				 * width passed is the including the endlines
				 * of the box */
    height -= 1;



    if (info->three_d) {

	XPoint          point[5];

	if (fill_in && width > 1 && height > 1) {

	    XFillRectangle(info->dpy, win,
		       (state & OLGX_INVOKED) ? info->gc_rec[OLGX_BG2]->gc :
			   info->gc_rec[OLGX_BG1]->gc,
			   x + 1, y + 1, width - 1, height - 1);

	}
	point[0].x = x;
	point[1].x = x;
	point[1].y = y;
	point[2].y = y;
	point[0].y = y + height;
	point[2].x = x + width;

	XDrawLines(info->dpy, win,
		   (state & OLGX_INVOKED) ? info->gc_rec[OLGX_BG3]->gc :
		   info->gc_rec[OLGX_WHITE]->gc, point, 3, CoordModeOrigin);

	point[0].y = y + height;
	point[1].x = x + width;
	point[1].y = y + height;
	point[2].x = x + width;
	/* special case for vertical lines so that there is still some
	   3d effect */
	if (width<=1) {
	    point[2].y = y+1;
	    point[0].x = x;
	} else {
	    point[2].y = y;
	    point[0].x = x + 1;
	}
	if (info->three_d == OLGX_3D_MONO) {

	    /* Add the extra line needed for monochrome 3D */
	    /* Tricky drawing , to get everything on one  */
	    /* sever request , we use something like _||  */
	    /* to achieve double width line for mono3D    */

	    point[3].x = x + width - 1;
	    point[3].y = y;
	    point[4].x = x + width - 1;
	    point[4].y = y + height - 1;
	    XDrawLines(info->dpy, win,
		     (state & OLGX_INVOKED) ? info->gc_rec[OLGX_WHITE]->gc :
		       info->gc_rec[OLGX_BG3]->gc,
		       point, 5, CoordModeOrigin);

	} else {

	    XDrawLines(info->dpy, win,
		     (state & OLGX_INVOKED) ? info->gc_rec[OLGX_WHITE]->gc :
		       info->gc_rec[OLGX_BG3]->gc,
		       point, 3, CoordModeOrigin);

	}


    } else {

	/* 2d */

	if (state & OLGX_ERASE)
	   XFillRectangle(info->dpy, win, info->three_d ?
                                            info->gc_rec[OLGX_BG1]->gc :
		                            info->gc_rec[OLGX_WHITE]->gc,
                          x, y, width + 1, height + 1);

	if (state & OLGX_INVOKED) {
	    if (fill_in)
		XFillRectangle(info->dpy, win, info->gc_rec[OLGX_BLACK]->gc, x, y,
                               width + 1, height + 1);

	    else {

		/* Draw the special invoked state */

		XRectangle      rect[3];

		rect[0].x = x;
		rect[0].y = y;
		rect[0].width = width;
		rect[0].height = height;
		rect[1].x = x + 1;
		rect[1].y = y + 1;
		rect[1].width = width - 2;
		rect[1].height = height - 2;
		XDrawRectangles(info->dpy,win, info->gc_rec[OLGX_BLACK]->gc, rect, 2);

	    }

	} else
	    XDrawRectangle(info->dpy, win, info->gc_rec[OLGX_BLACK]->gc, x, y,
                           width, height);

    }
    if (state & OLGX_INACTIVE) {

	/*
	 * Inactive State grey out the entire thing
	 */

	olgx_stipple_rect(info, win, x, y, width, height);

    }
    
}


void
olgx_draw_choice_item(info, win, x, y, width, height, label, state)
    Graphics_info  *info;
    Window          win;
    void           *label;
    int             x, y, width, height, state;
{
    int             def_decr;
    int             centerx = 0;
    int             centery = 0;


    /*
     * Special inactive case, so pass invoked state to the draw box routine
     * and swish that out later
     */

    if (!(info->three_d) && (state & OLGX_INACTIVE))
	state |= OLGX_INVOKED;

    /* draw a box (3d or 2d) outline, filling it in only if invoked */

    olgx_draw_box(info, win, x, y, width, height,
		  state, (info->three_d) ? 1 : 0);

    if (!(info->three_d) && (state & OLGX_INACTIVE))
	state ^= OLGX_INVOKED;

    /* the default ring rectangle looks better with width-5 for 3d */

    def_decr = info->three_d ? 6 : 5;

    if (state & OLGX_DEFAULT) {

	/* draw an inner box for a default setting */

	XDrawRectangle(info->dpy, win, (info->three_d) ?
                                         info->gc_rec[OLGX_BG3]->gc :
		                         info->gc_rec[OLGX_BLACK]->gc,
                       x + 2, y + 2, (width - def_decr), (height - def_decr));
    }
    /*
     * Now place the label
     */


    if (label) {

	if (state & OLGX_LABEL_IS_PIXMAP) {

	    centerx = ((width - ((Pixlabel *) label)->width) >> 1);
	    centery = ((height - ((Pixlabel *) label)->height) >> 1);
	    olgx_draw_pixmap_label(info, win,
				   ((Pixlabel *) label)->pixmap,
				   x + ((centerx > 0) ? centerx : 0),
				   y + ((centery > 0) ? centery : 0),
				   ((Pixlabel *) label)->width,
				   ((Pixlabel *) label)->height,
				   state);

	} else {

	    int             flag = 0;

	    /*
	     * special case for choice invoked in drawing label where the
	     * invoked state is changed to uninvoked and sent to the label
	     * drawing routines
	     */

	    if (state & OLGX_INVOKED) {

		state ^= OLGX_INVOKED;
		flag = 1;

	    }
	    olgx_draw_text(info, win,
#ifdef OW_I18N
			   label,
#else
			   (char *) label,
#endif
	    /*
	     * a small hack to make sure , that the between the left side of
	     * the choice item and the text is okay under 14pt and 19pt
	     * size.. we are using the same info->base_off value
	     */
			   x + ((info->button_height > 20) ?
				info->base_off + 2 : info->base_off),
			   y + height - info->base_off,
			   width - ((info->button_height > 20) ?
				    info->base_off + 2 : info->base_off),
			   state & ~OLGX_INACTIVE); /* don't double grey out */

	    /* reset to invoked state */

	    if (flag)
		state = state | OLGX_INVOKED;

	}

    }
    if (state & OLGX_INACTIVE) {

	/*
	 * Inactive State grey out the entire thing
	 */

	olgx_stipple_rect(info, win, x, y, width, height);

    }
}

void
olgx_draw_drop_target(info, win,label, x, y, state)
    Graphics_info  *info;
    Window          win;
    void           *label;
    int             x, y, state;
{
   

 

     XPoint         points[7];
     register short width;
     register short height;
     register short ewidth;
     int      max_line_y,line_y;


     width = info->dtarget_width;
     height = info->dtarget_height;
     ewidth = info->dtarget_ewidth;

     points[0].x = x;
     points[0].y = y;
     points[1].x = x + width;
     points[1].y = y ;
     points[2].x = x + width - ewidth;
     points[2].y = y + ewidth;
     points[3].x = x + ewidth;
     points[3].y = y + ewidth;
     points[4].x = x + ewidth;
     points[4].y = y + height - ewidth;
     points[5].x = x ;
     points[5].y = y + height;

     XFillRectangle(info->dpy,win,(info->three_d)?info->gc_rec[OLGX_BG2]->gc:
		                                  info->gc_rec[OLGX_WHITE]->gc,
		    x,y,width,height);
     XFillPolygon(info->dpy,win,(info->three_d)?info->gc_rec[OLGX_BG3]->gc:
                                               info->gc_rec[OLGX_BLACK]->gc,
                  points, 6, Convex,CoordModeOrigin);
     points[0].x = x + width ;
     points[0].y = y ;
     points[1].x = x + width - ewidth ;
     points[1].y = y + ewidth ;
     points[2].x = x + width - ewidth;
     points[2].y = y + height - ewidth ;
     points[3].x = x + ewidth;
     points[3].y = y + height - ewidth ;
     points[4].x = x  ;
     points[4].y = y + height - ((info->three_d) ? 0 : 1);
     points[5].x = x + width - ((info->three_d) ? 0 : 1);
     points[5].y = y + height- ((info->three_d) ? 0 : 1);
     points[6].x = x + width- ((info->three_d) ? 0 : 1);
     points[6].y = y ;                

     if (info->three_d) 
       XFillPolygon(info->dpy,win,info->gc_rec[OLGX_WHITE]->gc,points,6,Convex,CoordModeOrigin);
     else { /* 2d - render hollow polygon */
       
       XSetLineAttributes(info->dpy,info->gc_rec[OLGX_BLACK]->gc,info->dtarget_swidth,
			  LineSolid,CapNotLast,JoinRound);
       XDrawLines(info->dpy,win,info->gc_rec[OLGX_BLACK]->gc,&points[1],3,CoordModeOrigin);
       XDrawLines(info->dpy,win,info->gc_rec[OLGX_BLACK]->gc,&points[4],3,CoordModeOrigin);
       XSetLineAttributes(info->dpy,info->gc_rec[OLGX_BLACK]->gc,0,
			  LineSolid,CapNotLast,JoinRound);

     }
     /* Draw the lines label */

     if (state & OLGX_INVOKED) {
       max_line_y = y + height - ewidth - 1;
       for ( line_y = y+ewidth +1; line_y < max_line_y; line_y += 2) {
	 XDrawLine(info->dpy,win,(info->three_d)?info->gc_rec[OLGX_BLACK]->gc:
		   info->gc_rec[OLGX_BLACK]->gc,
		   x + ewidth + 1, line_y, x + width - ewidth - 2,line_y );
       } 
     }

     if (state & OLGX_BUSY) {

       if (!info->gc_rec[OLGX_BUSYGC])
	 olgx_initialise_gcrec(info,OLGX_BUSYGC);

       XFillRectangle(info->dpy,win,info->gc_rec[OLGX_BUSYGC]->gc, x + ewidth,y+ewidth,
		      width - (2 * ewidth), height - (2 * ewidth));

/*
       char dashes[] = {2,1};

       XSetDashes(info->dpy,(info->three_d)?info->gc_rec[OLGX_WHITE]->gc:
		  info->gc_rec[OLGX_BLACK]->gc,
		  0,dashes,2);

       XSetLineAttributes(info->dpy,(info->three_d)? info->gc_rec[OLGX_WHITE]->gc :
			  info->gc_rec[OLGX_BLACK]->gc,
			  0,LineOnOffDash,
			  CapNotLast,JoinRound);
       max_line_y = y + height - ewidth -1;
       for ( line_y = y+ewidth +1; line_y < max_line_y; line_y += 2) {
	 XDrawLine(info->dpy,win,(info->three_d)? info->gc_rec[OLGX_WHITE]->gc :
		   info->gc_rec[OLGX_BLACK]->gc,
		   x + ewidth + 1, line_y, x + width - ewidth - 2,line_y );
       }

       XSetLineAttributes(info->dpy,(info->three_d)? info->gc_rec[OLGX_WHITE]->gc :
			  info->gc_rec[OLGX_BLACK]->gc,
			  0,LineSolid,
			  CapNotLast,JoinRound);
*/

     }


     if (state & OLGX_INACTIVE) 
       olgx_stipple_rect(info,win,x,y,width,height);

     
   }
     
     

     

