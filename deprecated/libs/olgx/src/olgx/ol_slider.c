/*#ident "@(#)ol_slider.c	1.23 93/06/28 SMI" */

/*
 * Copyright 1990 Sun Microsystems
 */

/*
 * OPEN LOOK object drawing package
 * 
 * ol_slider.c Slider Module
 */

#include <stdio.h>
#include <X11/Xlib.h>

#include <olgx_private/olgx_impl.h>

/*
 * Private function declarations
 */

void            olgx_draw_slider_control();

/*
 * olgx_draw_slider
 * 
 * Presents a uniform interface to the other slider routines:
 * olgx_draw_vertical_slider olgx_draw_horizontal_slider
 * olgx_update_vertical_slider olgx_update_horizontal_slider
 */

void
olgx_draw_slider(info, win, x, y, width, oldval, newval, state)
    Graphics_info  *info;
    Window          win;
    int             x, y, width, oldval, newval, state;
{


    if (state & OLGX_VERTICAL) {

	/*
	 * newval and oldval are incremented so that if the newval is 0 the
	 * control will not be drawn after the bottom of the slider
	 */

	newval += HorizSliderControl_Width(info);
	oldval += HorizSliderControl_Width(info);

	/*
	 * The width and height is subtracted from the width to make the
	 * newval and oldval originate from the bottom for Vertical Gauges
	 */

	if (state & OLGX_UPDATE) 
	    olgx_update_vertical_slider(info, win,
					x, y, width, width - oldval,
					width - newval, state);
        else
	    olgx_draw_vertical_slider(info, win,
				      x, y, width, width - newval, state);
	
	if (state & OLGX_INACTIVE)	
	   olgx_stipple_rect(info, win, x, y,HorizSliderControl_Height(info),width);

    } else {  /* Horizontal Sliders */

	if (state & OLGX_UPDATE) 
	    olgx_update_horizontal_slider(info, win,
					  x, y, width, oldval, newval, state);
	else 
	    olgx_draw_horizontal_slider(info, win,
					x, y, width, newval, state);
	if (state & OLGX_INACTIVE)	
	   olgx_stipple_rect(info, win, x, y,width,HorizSliderControl_Height(info));
    }
}



/*
 * olgx_draw_horizontal_slider
 */

void
olgx_draw_horizontal_slider(info, win, x, y, width, value, state)
    Graphics_info  *info;
    Window          win;
    int             x, y, width;
    int             value;	/* Slider control position  */
    int             state;

{
    XTextItem       item;
    int             inside_width;
    char            string[STRING_SIZE];
    short           add_ins[STRING_SIZE];
    register        y_slider_offset;
    register int    i;
    int             num_add;

    /* inside_width is the width excluding the endcaps */

    inside_width = width - (info->se_width << 1);
    y_slider_offset = y + info->slider_offset;

    item.font = None;
    item.chars = string;
    item.delta = 0;
    if (!info->three_d) {

	/* 2d - Unlike 3d, the slider is not split into the */
	/* left and right side, it is drawn in one piece */

	num_add = calc_add_ins(inside_width, add_ins);
	item.nchars = 2 + num_add;
	string[0] = HORIZ_SLIDER_LEFT_ENDCAP_OUTLINE;
	VARIABLE_LENGTH_MACRO(1, HORIZ_SLIDER_OUTLINE_1);
	string[1 + num_add] = HORIZ_SLIDER_RIGHT_ENDCAP_OUTLINE;
	XDrawText(info->dpy, win, info->gc_rec[OLGX_BLACK]->gc, x, y_slider_offset, 
                  &item, 1);

	/* The left filled part of the slider  */

	num_add = calc_add_ins(value, add_ins);
	item.nchars = 1 + num_add;
	string[0] = HORIZ_SLIDER_LEFT_ENDCAP_FILL;
	VARIABLE_LENGTH_MACRO(1, HORIZ_SLIDER_FILL_1);
	XDrawText(info->dpy, win, info->gc_rec[OLGX_BLACK]->gc, x, y_slider_offset, 
                  &item, 1);

    } else {			/* 3d */

	/*
	 * draw the left part of the slider
	 */

	num_add = calc_add_ins(value, add_ins);
	item.nchars = 1 + num_add;
	string[0] = HORIZ_SLIDER_UL;	/* upper portion of left side */
	VARIABLE_LENGTH_MACRO(1, BUTTON_TOP_1);
	XDrawText(info->dpy, win, info->gc_rec[OLGX_BLACK]->gc, x, y_slider_offset,
		  &item, 1);

	string[0] = HORIZ_SLIDER_LL;	/* lower left */
	VARIABLE_LENGTH_MACRO(1, HORIZ_SLIDER_BOTTOM_1);
	XDrawText(info->dpy, win, info->gc_rec[OLGX_BLACK]->gc, x, y_slider_offset,
		  &item, 1);

	string[0] = HORIZ_SLIDER_LEFT_ENDCAP_FILL;
	VARIABLE_LENGTH_MACRO(1, HORIZ_SLIDER_FILL_1);
	XDrawText(info->dpy, win, info->gc_rec[OLGX_BLACK]->gc, x, y_slider_offset,
		  &item, 1);

	/* now draw the shading */

	item.nchars = num_add;
	VARIABLE_LENGTH_MACRO(0, BUTTON_TOP_1);
	XDrawText(info->dpy, win, info->gc_rec[OLGX_BG2]->gc, x + info->se_width,
		  y_slider_offset + 1, &item, 1);


	/*
	 * draw the right part of the slider
	 */

	num_add = calc_add_ins(inside_width - value, add_ins);
	item.nchars = 1 + num_add;

	VARIABLE_LENGTH_MACRO(0, BUTTON_TOP_1);
	string[i] = HORIZ_SLIDER_UR;	/* upper portion of right side */
	XDrawText(info->dpy, win, info->gc_rec[OLGX_BG3]->gc,
		  x + value + info->se_width, y_slider_offset, &item, 1);

	VARIABLE_LENGTH_MACRO(0, HORIZ_SLIDER_BOTTOM_1);
	string[i] = HORIZ_SLIDER_LR;	/* lower right */
	XDrawText(info->dpy, win, info->gc_rec[OLGX_WHITE]->gc,
		  x + value + info->se_width, y_slider_offset, &item, 1);

	VARIABLE_LENGTH_MACRO(0, HORIZ_SLIDER_FILL_1);
	string[i] = HORIZ_SLIDER_RIGHT_ENDCAP_FILL;
	XDrawText(info->dpy, win, info->gc_rec[OLGX_BG2]->gc,
		  x + value + info->se_width, y_slider_offset, &item, 1);

    }

    olgx_draw_slider_control(info, win, x + value, y, state);

}

void
olgx_draw_slider_control(info, win, x, y, state)
    Graphics_info  *info;
    Window          win;
    int             x, y, state;
{
    XTextItem       item;
    char            string[2];

    item.nchars = 1;
    item.font = None;
    item.chars = string;
    item.delta = 0;

    if (state & OLGX_HORIZONTAL) {
	if (info->three_d) {
	    string[0] = HORIZ_SLIDER_CONTROL_UL;
	    XDrawText(info->dpy, win, (state & OLGX_INVOKED) ?
                                       info->gc_rec[OLGX_BG3]->gc : 
                                       info->gc_rec[OLGX_WHITE]->gc,
                       x, y, &item, 1);

	    string[0] = HORIZ_SLIDER_CONTROL_LR;
	    XDrawText(info->dpy, win, (state & OLGX_INVOKED) ?
                                        info->gc_rec[OLGX_WHITE]->gc :
                                        info->gc_rec[OLGX_BG3]->gc,
                      x, y, &item, 1);

	    string[0] = HORIZ_SLIDER_CONTROL_FILL;
	    XDrawText(info->dpy, win, (state & OLGX_INVOKED) ? 
                                        info->gc_rec[OLGX_BG2]->gc :
                                        info->gc_rec[OLGX_BG1]->gc, 
                      x, y, &item, 1);

	} else {

	    string[0] = HORIZ_SLIDER_CONTROL_OUTLINE;
	    XDrawString(info->dpy, win, info->gc_rec[OLGX_BLACK]->gc, x, y, string,1);

	    string[0] = HORIZ_SLIDER_CONTROL_FILL;
	    XDrawText(info->dpy, win,(state & OLGX_INVOKED) ?
                                       info->gc_rec[OLGX_BLACK]->gc :
                                       info->gc_rec[OLGX_WHITE]->gc,
                      x, y, &item, 1);

	}

    } else {

	if (info->three_d) {

	    string[0] = VERT_SLIDER_CONTROL_UL;
	    XDrawText(info->dpy, win, (state & OLGX_INVOKED) ? 
                                       info->gc_rec[OLGX_BG3]->gc :
                                       info->gc_rec[OLGX_WHITE]->gc, 
                      x, y, &item, 1);

	    string[0] = VERT_SLIDER_CONTROL_LR;
	    XDrawText(info->dpy, win, (state & OLGX_INVOKED) ?
                                       info->gc_rec[OLGX_WHITE]->gc : 
                                       info->gc_rec[OLGX_BG3]->gc,
                      x, y, &item, 1);

	    string[0] = VERT_SLIDER_CONTROL_FILL;
	    XDrawText(info->dpy, win, (state & OLGX_INVOKED) ? 
                                       info->gc_rec[OLGX_BG2]->gc :
                                       info->gc_rec[OLGX_BG1]->gc,
                      x, y, &item, 1);

	} else {

	    string[0] = VERT_SLIDER_CONTROL_OUTLINE;
	    XDrawString(info->dpy, win, info->gc_rec[OLGX_BLACK]->gc, x, y, string, 1);
	    string[0] = VERT_SLIDER_CONTROL_FILL;
	    XDrawText(info->dpy, win, (state & OLGX_INVOKED) ? 
                                        info->gc_rec[OLGX_BLACK]->gc :
      				        info->gc_rec[OLGX_WHITE]->gc,
                      x, y, &item, 1);

	}
    }
}



void
olgx_update_horizontal_slider(info, win, x, y, width,
			      old_value, new_value, state)
    Graphics_info  *info;
    Window          win;
    int             x, y, width, new_value, old_value;
    int             state;
{
    XTextItem       item;
    char            string[STRING_SIZE];
    int             y_slider_offset, inside_width;
    int             xstart, xwidth;

    if (old_value == new_value) {
        /* The slider control needs to be updated anyway to satisfy
         * a particular case where the control box state changes
         * with no change in slider values 
         */
	olgx_draw_slider_control(info, win, x + new_value,
				 y, state);
	return;
    }
    item.nchars = 1;
    item.font = None;
    item.chars = string;
    item.delta = 0;


    /* calculate width minus the slider endcaps */

    inside_width = (width - (info->se_width << 1));

    /* calculate the y offset of the slider control from the channel */

    y_slider_offset = y + info->slider_offset;

    if (old_value < new_value) {

	/* calculate start & width of area needing repaint */

	xstart = x + old_value;
	xwidth = (new_value - old_value);

	/*
	 * erase the old slider
	 */

	XClearArea(info->dpy, win, xstart, y,
		       xwidth, info->slider_height,0);


	/*
	 * repair the damaged area
	 */

	XFillRectangle(info->dpy, win, info->gc_rec[OLGX_BLACK]->gc,
		       xstart, y_slider_offset, xwidth, info->se_height);

	/* now draw the shading */
	/*
	 * following should be OLGX_BG3 according to spec but OLGX_BG2 looks
	 * better
	 */

	XDrawLine(info->dpy, win, info->three_d ? info->gc_rec[OLGX_BG2]->gc :
		  info->gc_rec[OLGX_BLACK]->gc, xstart, y_slider_offset + 1,
		  xstart + xwidth - 1, y_slider_offset + 1);

	if (old_value < 4) {	/* Draw the leftendcap when necessary */

	    if (!info->three_d) {	/* 2d  */

		XFillRectangle(info->dpy, win, info->gc_rec[OLGX_WHITE]->gc, x,
			  y_slider_offset, info->se_width, info->se_height);

		string[0] = HORIZ_SLIDER_LEFT_ENDCAP_OUTLINE;
		XDrawText(info->dpy, win, info->gc_rec[OLGX_BLACK]->gc, x, 
                          y_slider_offset, &item, 1);

		string[0] = HORIZ_SLIDER_LEFT_ENDCAP_FILL;
		XDrawText(info->dpy, win, info->gc_rec[OLGX_BLACK]->gc, x,
                          y_slider_offset, &item, 1);

	    } else {		/* 3d  */

		XFillRectangle(info->dpy, win, info->gc_rec[OLGX_BG1]->gc, x,
                               y_slider_offset, info->se_width, info->se_height);

		string[0] = HORIZ_SLIDER_UL;
		XDrawText(info->dpy, win, info->gc_rec[OLGX_BLACK]->gc, x, 
                          y_slider_offset, &item, 1);

		string[0] = HORIZ_SLIDER_LL;
		XDrawText(info->dpy, win, info->gc_rec[OLGX_BLACK]->gc, x, 
                         y_slider_offset, &item, 1);

		string[0] = HORIZ_SLIDER_LEFT_ENDCAP_FILL;
		XDrawText(info->dpy, win, info->gc_rec[OLGX_BLACK]->gc, x, 
                          y_slider_offset, &item, 1);

	    }
	}

	/* draw the new one */

	olgx_draw_slider_control(info, win, x + new_value, y, state);

    } else {

	xstart = x + new_value;
	xwidth = (old_value - new_value) + info->slider_width;

	/*
	 * erase the old slider
	 */
       XClearArea(info->dpy, win,xstart, y, xwidth,info->slider_height + 1,0);


	/* redraw the damaged area */

	XDrawLine(info->dpy, win, info->three_d ?
                                    info->gc_rec[OLGX_BG3]->gc :
		                    info->gc_rec[OLGX_BLACK]->gc,
                  xstart, y_slider_offset, xstart + xwidth - 1, y_slider_offset);

	XDrawLine(info->dpy, win, info->three_d ?
                                   info->gc_rec[OLGX_WHITE]->gc :
		                   info->gc_rec[OLGX_BLACK]->gc, 
                  xstart, y_slider_offset + (info->se_height - 1), xstart+xwidth - 1,
                  y_slider_offset + (info->se_height - 1));

	XFillRectangle(info->dpy, win, info->three_d ?
                                         info->gc_rec[OLGX_BG2]->gc :
		                         info->gc_rec[OLGX_WHITE]->gc, xstart,
                      y_slider_offset + 1, xwidth, (info->se_height - 2));

	if (old_value + info->slider_width > width - 4) {

	    /* Draw the endcap when necessary */

	    if (!info->three_d) {	/* 2d  */

		XFillRectangle(info->dpy, win, info->gc_rec[OLGX_WHITE]->gc,
			 x + inside_width + info->se_width, y_slider_offset,
			       info->se_width, info->se_height);

		string[0] = HORIZ_SLIDER_RIGHT_ENDCAP_OUTLINE;
		XDrawText(info->dpy, win, info->gc_rec[OLGX_BLACK]->gc,
		       x + inside_width + info->se_width, y_slider_offset, &item, 1);

		string[0] = HORIZ_SLIDER_RIGHT_ENDCAP_FILL;
		XDrawText(info->dpy, win, info->gc_rec[OLGX_WHITE]->gc,
		       x + inside_width + info->se_width, y_slider_offset, &item, 1);

	    } else {		/* 3d  */

		XFillRectangle(info->dpy, win, info->gc_rec[OLGX_BG1]->gc,
			 x + inside_width + info->se_width, y_slider_offset,
			       info->se_width, info->se_height);

		string[0] = HORIZ_SLIDER_UR;
		XDrawText(info->dpy, win, info->gc_rec[OLGX_BLACK]->gc,
   		       x + inside_width + info->se_width, y_slider_offset, &item, 1);

		string[0] = HORIZ_SLIDER_LR;
		XDrawText(info->dpy, win, info->gc_rec[OLGX_WHITE]->gc,
		       x + inside_width + info->se_width, y_slider_offset, &item, 1);

		string[0] = HORIZ_SLIDER_RIGHT_ENDCAP_FILL;
		XDrawText(info->dpy, win, info->gc_rec[OLGX_BG2]->gc,
		      x + inside_width + info->se_width, y_slider_offset, &item, 1);

	    }
	}

	/* draw the new one */

	olgx_draw_slider_control(info, win, x + new_value, y, state);

    }


}

void
olgx_update_vertical_slider(info, win, x, y, height,
			    old_value, new_value, state)
    Graphics_info  *info;
    Window          win;
    int             x, y, height, new_value, old_value;
    int             state;
{
    int             ystart, yheight;
    int             inside_height, x_slider_offset;
    XTextItem       item;
    char            string[2];

    item.nchars = 1;
    item.font = None;
    item.chars = string;
    item.delta = 0;

    if (old_value == new_value)  {
        /* The slider control needs to be updated anyway to satisfy
         * a particular case where the control box state changes
         * with no change in slider values 
         */
	olgx_draw_slider_control(info, win, x, y + new_value, state);
	return;
    }

    /* inside_width is the width excluding the channel endcaps */

    inside_height = height - (info->se_width << 1);

    /* slider_offset is the x offset of the slider from the channel */

    x_slider_offset = x + info->slider_offset;

    if (old_value > new_value) {

	/* calculate start & width of area needing repaint */

	ystart = y + new_value + info->slider_width;
	yheight = (old_value - new_value);

	/*
	 * erase the old slider
	 */

	XClearArea(info->dpy, win, x, ystart,
		       info->slider_height, yheight,0);


	/*
	 * repair the damaged area
	 */

	XFillRectangle(info->dpy, win, info->gc_rec[OLGX_BLACK]->gc, x_slider_offset,
		       ystart, info->se_height, yheight);

	/* now draw the shading */
	/*
	 * following should be OLGX_BG3 according to spec but OLGX_BG2 looks
	 * better
	 */

	XDrawLine(info->dpy, win, info->three_d ? info->gc_rec[OLGX_BG2]->gc :
		  info->gc_rec[OLGX_BLACK]->gc, x_slider_offset + 1, ystart,
		  x_slider_offset + 1, ystart + yheight - 1);

	if (old_value + info->slider_width > height - 4) {

	    /* Draw the endcap when necessary */

	    if (!info->three_d) {	/* 2d  */

		XFillRectangle(info->dpy, win, info->gc_rec[OLGX_WHITE]->gc,
                               x_slider_offset, y + inside_height + info->se_width, 
                               info->se_height, info->se_width);

		string[0] = VERT_SLIDER_BOTTOM_ENDCAP_OUTLINE;
		XDrawText(info->dpy, win, info->gc_rec[OLGX_BLACK]->gc, 
                          x_slider_offset, y + inside_height + info->se_width,
                          &item, 1);

		string[0] = VERT_SLIDER_BOTTOM_ENDCAP_FILL;
		XDrawText(info->dpy, win, info->gc_rec[OLGX_BLACK]->gc,
                          x_slider_offset, y + inside_height + info->se_width, 
                          &item, 1);

	    } else {		/* 3d  */

		XFillRectangle(info->dpy, win, info->gc_rec[OLGX_BG1]->gc, 
                               x_slider_offset, y + inside_height + info->se_width, 
                               info->se_height, info->se_width);

		string[0] = VERT_SLIDER_LL;
		XDrawText(info->dpy, win, info->gc_rec[OLGX_BLACK]->gc,
                          x_slider_offset, y + inside_height + info->se_width,
                          &item, 1);

		string[0] = VERT_SLIDER_LR;
		XDrawText(info->dpy, win, info->gc_rec[OLGX_BLACK]->gc, 
                          x_slider_offset, y + inside_height + info->se_width, 
                          &item, 1);

		string[0] = VERT_SLIDER_BOTTOM_ENDCAP_FILL;
		XDrawText(info->dpy, win, info->gc_rec[OLGX_BLACK]->gc, 
                          x_slider_offset, y + inside_height + info->se_width, 
                          &item, 1);

	    }
	}

	/* draw the new one */

	olgx_draw_slider_control(info, win, x, y + new_value, state);

    } else {

	ystart = y + old_value;
	yheight = (new_value - old_value);

	/*
	 * erase the old slider
	 */

	XClearArea(info->dpy, win, x, ystart, info->slider_height, yheight,0);



	/* redraw the damaged area */

	XDrawLine(info->dpy, win, info->three_d ? info->gc_rec[OLGX_BG3]->gc :
		  info->gc_rec[OLGX_BLACK]->gc, x_slider_offset, ystart,
		  x_slider_offset, ystart + yheight - 1);

	XDrawLine(info->dpy, win, info->three_d ? info->gc_rec[OLGX_WHITE]->gc :
		  info->gc_rec[OLGX_BLACK]->gc,
		  x_slider_offset + (info->se_height - 1), ystart,
	     x_slider_offset + (info->se_height - 1), ystart + yheight - 1);

	XFillRectangle(info->dpy, win, info->three_d ? info->gc_rec[OLGX_BG2]->gc :
		  info->gc_rec[OLGX_WHITE]->gc, x_slider_offset + 1, ystart,
		       (info->se_height - 2), yheight);

	if (old_value < 4) {	/* Draw the endcap when necessary */

	    if (!info->three_d) {	/* 2d  */

		XFillRectangle(info->dpy, win, info->gc_rec[OLGX_WHITE]->gc,
                               x_slider_offset, y, info->se_height, 3);

		string[0] = VERT_SLIDER_TOP_ENDCAP_OUTLINE;
		XDrawText(info->dpy, win, info->gc_rec[OLGX_BLACK]->gc,
                          x_slider_offset, y, &item, 1);

		string[0] = VERT_SLIDER_TOP_ENDCAP_FILL;
		XDrawText(info->dpy, win, info->gc_rec[OLGX_WHITE]->gc,
                          x_slider_offset, y, &item, 1);

	    } else {		/* 3d  */

		XFillRectangle(info->dpy, win, info->gc_rec[OLGX_BG1]->gc, 
                               x_slider_offset, y, info->se_height, 3);

		string[0] = VERT_SLIDER_UL;
		XDrawText(info->dpy, win, info->gc_rec[OLGX_BG3]->gc,
                          x_slider_offset, y, &item, 1);

		string[0] = VERT_SLIDER_UR;
		XDrawText(info->dpy, win, info->gc_rec[OLGX_WHITE]->gc, 
                          x_slider_offset, y, &item, 1);

		string[0] = VERT_SLIDER_TOP_ENDCAP_FILL;
		XDrawText(info->dpy, win, info->gc_rec[OLGX_BG2]->gc, 
                          x_slider_offset, y, &item, 1);

	    }
	}

	/* draw the new one */

	olgx_draw_slider_control(info, win, x, y + new_value, state);

    }

}


void
olgx_draw_vertical_slider(info, win, x, y, height, value, state)
    Graphics_info  *info;
    Window          win;
    int             x, y, height;
    int             value;	/* Slider control position */
    int             state;
{
    XTextItem       item;
    char            string[STRING_SIZE];
    int             inside_height, x_slider_offset;

    /* inside_height is the height excluding the channel endcaps */

    inside_height = height - (info->se_width << 1);

    /* slider_offset is the x offset of the slider from the channel */

    x_slider_offset = x + info->slider_offset;


    item.nchars = 1;
    item.font = None;
    item.chars = string;
    item.delta = 0;

    /* 2d - Unlike 3d, the slider is not split into the */
    /* left and right side, it is drawn in one piece */

    if (!info->three_d) {

	string[0] = VERT_SLIDER_BOTTOM_ENDCAP_OUTLINE;
	XDrawText(info->dpy, win, info->gc_rec[OLGX_BLACK]->gc,
	     x_slider_offset, y + info->se_width + inside_height, &item, 1);

	string[0] = VERT_SLIDER_BOTTOM_ENDCAP_FILL;
	XDrawText(info->dpy, win, info->gc_rec[OLGX_BLACK]->gc,
	     x_slider_offset, y + info->se_width + inside_height, &item, 1);

	XFillRectangle(info->dpy, win, info->gc_rec[OLGX_BLACK]->gc,
		       x_slider_offset, y + value, info->se_height,
		       (height - value - info->se_width));

	XDrawLine(info->dpy, win, info->gc_rec[OLGX_BLACK]->gc,
		  x_slider_offset, y + info->se_width, x_slider_offset,
		  y + value + info->se_width - 1);
	XDrawLine(info->dpy, win, info->gc_rec[OLGX_BLACK]->gc,
		x_slider_offset + (info->se_height - 1), y + info->se_width,
		  x_slider_offset + (info->se_height - 1),
		  y + value + info->se_width - 1);

	XFillRectangle(info->dpy, win, info->gc_rec[OLGX_WHITE]->gc,
	     x_slider_offset + 1, y + info->se_width, (info->se_height - 2),
		       value);

	string[0] = VERT_SLIDER_TOP_ENDCAP_OUTLINE;
	XDrawText(info->dpy, win, info->gc_rec[OLGX_BLACK]->gc, x_slider_offset, y,
		  &item, 1);

	string[0] = VERT_SLIDER_TOP_ENDCAP_FILL;
	XDrawText(info->dpy, win, info->gc_rec[OLGX_WHITE]->gc, x_slider_offset, y,
		  &item, 1);

    } else {

	/*
	 * draw the bottom part of the slider (NOTE: this code could be
	 * optimized to use 1 XDrawText call)
	 */

	string[0] = VERT_SLIDER_LR;
	XDrawText(info->dpy, win, info->gc_rec[OLGX_BLACK]->gc,
	     x_slider_offset, y + inside_height + info->se_width, &item, 1);

	string[0] = VERT_SLIDER_LL;
	XDrawText(info->dpy, win, info->gc_rec[OLGX_BLACK]->gc,
	     x_slider_offset, y + inside_height + info->se_width, &item, 1);

	string[0] = VERT_SLIDER_BOTTOM_ENDCAP_FILL;
	XDrawText(info->dpy, win, info->gc_rec[OLGX_BLACK]->gc,
	     x_slider_offset, y + inside_height + info->se_width, &item, 1);

	XFillRectangle(info->dpy, win, info->gc_rec[OLGX_BLACK]->gc,
		       x_slider_offset, y + value, info->se_height,
		       (height - value - info->se_width));


	/*
	 * draw the top part of the slider
	 */

	XDrawLine(info->dpy, win, info->gc_rec[OLGX_BG3]->gc,
		  x_slider_offset, y + info->se_width, x_slider_offset,
		  y + value + info->se_width - 1);

	string[0] = VERT_SLIDER_UL;	/* upper portion of left side */
	XDrawText(info->dpy, win, info->gc_rec[OLGX_BG3]->gc, x_slider_offset, y,
		  &item, 1);

	XDrawLine(info->dpy, win, info->gc_rec[OLGX_WHITE]->gc,
		x_slider_offset + (info->se_height - 1), y + info->se_width,
		  x_slider_offset + (info->se_height - 1),
		  y + value + info->se_width - 1);

	string[0] = VERT_SLIDER_UR;	/* lower left */
	XDrawText(info->dpy, win, info->gc_rec[OLGX_WHITE]->gc, x_slider_offset,
		  y, &item, 1);

	XFillRectangle(info->dpy, win, info->gc_rec[OLGX_BG2]->gc,
		       x_slider_offset + 1, y + info->se_width,
		       (info->se_height - 2), value);

	string[0] = VERT_SLIDER_TOP_ENDCAP_FILL;
	XDrawText(info->dpy, win, info->gc_rec[OLGX_BG2]->gc, x_slider_offset, y, 
                  &item, 1);

	/* now draw the shading on the bottom */
	/*
	 * following should be OLGX_BG3 according to spec, but OLGX_BG2 looks
	 * better
	 */

	XDrawLine(info->dpy, win, info->gc_rec[OLGX_BG2]->gc,
		  x_slider_offset + 1, y + value + info->se_width,
	     x_slider_offset + 1, y + inside_height + (info->se_width - 1));

    }

    olgx_draw_slider_control(info, win, x, y + value, state);
}







void
olgx_draw_gauge(info, win, x, y, width, oldval, newval, state)
    Graphics_info  *info;
    Window          win;
    int             x, y;
    int             width;
    int             newval, oldval;
    int             state;

{
    if (state & OLGX_VERTICAL) {

	/*
	 * The width and height is subtracted from the width to make the
	 * newval and oldval originate from the bottom for Vertical Gauges
	 */

	if (state & OLGX_UPDATE)
	    olgx_update_vertical_gauge(info, win, x, y, width, width - oldval, 
                                       width - newval);
	else
	    olgx_draw_vertical_gauge(info, win, x, y, width, width - newval);
        if (state & OLGX_INACTIVE)	
	   olgx_stipple_rect(info, win, x, y,Gauge_EndCapHeight(info),width);
    } else { /* Horizontal Gauges */

	if (state & OLGX_UPDATE)
	    olgx_update_horiz_gauge(info, win, x, y, oldval, newval);
	else
	    olgx_draw_horiz_gauge(info, win, x, y, width, newval);

        if (state & OLGX_INACTIVE)	
           olgx_stipple_rect(info, win, x, y,width,Gauge_EndCapHeight(info));
    }
}


void
olgx_draw_horiz_gauge(info, win, x, y, width, value)
    Graphics_info  *info;
    Window          win;
    int             x, y, width, value;

{
    int             inside_width;
    char            string[STRING_SIZE];
    short           add_ins[STRING_SIZE];
    register int    i;
    int             num_add;
    register        left_offset;


    inside_width = width - (info->gauge_width << 1);

    if (!info->three_d) {	/* 2-D */

	/* Draw the outer ring */

	num_add = calc_add_ins(inside_width, add_ins);
	string[0] = HORIZ_GAUGE_LEFT_ENDCAP_OUTLINE;
	VARIABLE_LENGTH_MACRO(1, HORIZ_GAUGE_OUTLINE_MIDDLE_1);
	string[1 + num_add] = HORIZ_GAUGE_RIGHT_ENDCAP_OUTLINE;
	XDrawString(info->dpy, win, info->gc_rec[OLGX_BLACK]->gc, x, y, string, 
                    2 + num_add);


    } else {			/* 3-D  */

	/* The Following two calls to X */
	/* can be brought down to one   */

	num_add = calc_add_ins(inside_width, add_ins);

	string[0] = HORIZ_GAUGE_UL;
	VARIABLE_LENGTH_MACRO(1, BUTTON_TOP_1);

	string[1 + num_add] = HORIZ_GAUGE_UR;
	XDrawString(info->dpy, win, info->gc_rec[OLGX_BG3]->gc, x, y, string,
                    2 + num_add);

	string[0] = HORIZ_GAUGE_LL;
	VARIABLE_LENGTH_MACRO(1, HORIZ_GAUGE_BOTTOM_1);

	if (info->gauge_height == 10) {

	    XDrawString(info->dpy, win, info->gc_rec[OLGX_WHITE]->gc, x, y, string, 
                        2 + num_add);

	    string[0] = HORIZ_GAUGE_LR;
	    XDrawString(info->dpy, win, info->gc_rec[OLGX_WHITE]->gc,
			x + width - info->gauge_width, y - 1, string, 1);

	} else {

	    string[1 + num_add] = HORIZ_GAUGE_LR;
	    XDrawString(info->dpy, win, info->gc_rec[OLGX_WHITE]->gc, x, y, string, 
                        2 + num_add);

	}

	/* Now draw the middle recessed part */

	string[0] = HORIZ_GAUGE_LEFT_ENDFILL;
	VARIABLE_LENGTH_MACRO(1, HORIZ_GAUGE_MIDDLE_FILL_1);
	string[1 + num_add] = HORIZ_GAUGE_RIGHT_ENDFILL;
	XDrawString(info->dpy, win, info->gc_rec[OLGX_BG2]->gc, x, y, string, 
                    2 + num_add);


    }				/* 3-D End */

    /* Draw the inner slider  without the control common to both 2D & 3D */

    left_offset = 3;
    if (info->gauge_height > 14)
	left_offset = 5;


    /*
     * The following two calls to Xserver can be brought down to 1.. Should
     * work on it later
     */

    inside_width = value - left_offset - info->se_width;

    if (inside_width < 0)
	olgx_error("Negative value passed to gauge\n");

    else {			/* Correct value */

	num_add = calc_add_ins(inside_width, add_ins);

	string[0] = HORIZ_SLIDER_LEFT_ENDCAP_OUTLINE;
	VARIABLE_LENGTH_MACRO(1, HORIZ_SLIDER_OUTLINE_1);
	XDrawString(info->dpy, win, info->gc_rec[OLGX_BLACK]->gc, x + left_offset,
		    y + left_offset, string, 1 + num_add);

	string[0] = HORIZ_SLIDER_LEFT_ENDCAP_FILL;
	VARIABLE_LENGTH_MACRO(1, HORIZ_SLIDER_FILL_1);
	XDrawString(info->dpy, win, info->gc_rec[OLGX_BLACK]->gc, x + left_offset,
		    y + left_offset, string, 1 + num_add);

	/* Special case for size -14 */
	/* Where it needs an extra line */

	if (info->gauge_height == 13)	/* size-14 */
	    XDrawLine(info->dpy, win, info->gc_rec[OLGX_BLACK]->gc, 
                      x + left_offset + 2, y + left_offset + info->se_height,
                      x + value - 1, y + left_offset + info->se_height);

    }

    if (info->three_d)

	/* Draw the special line in the inner slider of the gauge for 3-D */

	XDrawLine(info->dpy, win, info->gc_rec[OLGX_BG2]->gc, 
                  x + left_offset + info->se_width, y + left_offset + 1,
                  x + value - 1, y + left_offset + 1);

}



void
olgx_update_horiz_gauge(info, win, x, y, oldval, newval)
    Graphics_info  *info;
    Window          win;
    int             x, y, oldval, newval;

{
    register        left_offset;

    /* Very Trivial */


    left_offset = 3;

    if (info->gauge_height > 14)
	left_offset = 5;

    if (oldval == newval)
	return;

    if (oldval < newval)
	XFillRectangle(info->dpy, win, info->gc_rec[OLGX_BLACK]->gc, x + oldval,
                       y + left_offset, newval - oldval, 
                       (info->gauge_height == 13) ?
		         info->se_height + 1 : info->se_height);

    else
	XFillRectangle(info->dpy, win, (info->three_d) ? 
                                         info->gc_rec[OLGX_BG2]->gc :
		                         info->gc_rec[OLGX_WHITE]->gc,
                       x + newval, y + left_offset, oldval - newval,
                       (info->gauge_height == 13) ?
		         info->se_height + 1 : info->se_height);

    if (info->three_d)

	/* Draw the special line for 3-D */

	XDrawLine(info->dpy, win, info->gc_rec[OLGX_BG2]->gc, 
                  x + left_offset + info->se_width, y + left_offset + 1,
                  x + newval - 1, y + left_offset + 1);

}

void
olgx_draw_vertical_gauge(info, win, x, y, width, value)
    Graphics_info  *info;
    Window          win;
    int             x, y, width, value;

{

    int             inside_width;
    char            string[1];
    char            string1[1];
    register        left_offset;
    XTextItem       item[2];


    inside_width = width - (info->gauge_width << 1);

    /* Draw the outer ring */

    if (!info->three_d) {	/* 2-D */

	XSegment        segments[2];

	/* draw the topend cap */

	string[0] = VERT_GAUGE_TOPENDCAP;
	XDrawString(info->dpy, win, info->gc_rec[OLGX_BLACK]->gc, x, y, string, 1);

	/* draw the middle lines */

	segments[0].x1 = x;
	segments[0].y1 = y + info->gauge_width;
	segments[0].x2 = x;
	segments[0].y2 = y + width - info->gauge_width;
	segments[1].x1 = x + info->gauge_height - 1;
	segments[1].y1 = y + info->gauge_width;
	segments[1].x2 = x + info->gauge_height - 1;
	segments[1].y2 = y + width - info->gauge_width;
	XDrawSegments(info->dpy, win, info->gc_rec[OLGX_BLACK]->gc, segments, 2);

	/* draw the botttom end cap */

	string[0] = VERT_GAUGE_BOTENDCAP;
	XDrawString(info->dpy, win, info->gc_rec[OLGX_BLACK]->gc, x,
		    y + width - info->gauge_width, string, 1);

    }

    /* End 2-D */

    else {			/* 3-D */


	/* Draw the middle recessed filling */

	string[0] = VERT_GAUGE_TOP_FILL;
	XDrawString(info->dpy, win, info->gc_rec[OLGX_BG2]->gc, x, y, string, 1);

	string[0] = VERT_GAUGE_BOT_FILL;
	XDrawString(info->dpy, win, info->gc_rec[OLGX_BG2]->gc, x, 
                    y + width - info->gauge_width, string, 1);

	XFillRectangle(info->dpy, win, info->gc_rec[OLGX_BG2]->gc, x + 1,
                       y + info->gauge_width, info->gauge_height - 2, inside_width);

	/* Draw the top and bottom endcaps */

	string[0] = VERT_GAUGE_UL;
	XDrawString(info->dpy, win, info->gc_rec[OLGX_BG3]->gc, x, y, string, 1);

	string[0] = VERT_GAUGE_LL;
	XDrawString(info->dpy, win, info->gc_rec[OLGX_BG3]->gc, x, 
                    y + width - info->gauge_width, string, 1);

	string[0] = VERT_GAUGE_UR;
	XDrawString(info->dpy, win, info->gc_rec[OLGX_WHITE]->gc, x, y, string, 1);

	string[0] = VERT_GAUGE_LR;
	XDrawString(info->dpy, win, info->gc_rec[OLGX_WHITE]->gc, x,
		    y + width - info->gauge_width, string, 1);

	/* Draw the middle two lines */

	XDrawLine(info->dpy, win, info->gc_rec[OLGX_BG3]->gc, x,
                  y + info->gauge_width, x, y + width - info->gauge_width);

	XDrawLine(info->dpy, win, info->gc_rec[OLGX_WHITE]->gc,
                  x + info->gauge_height - 1, y + info->gauge_width,
		  x + info->gauge_height - 1, y + width - info->gauge_width);


    }


    /* Draw the inner indicator common to */
    /* common to both 2D and 3D           */

    left_offset = 3;

    if (info->gauge_height > 14)
	left_offset = 5;

    item[0].chars = string;
    item[0].nchars = 1;
    item[0].delta = 0;
    item[0].font = None;
    item[1].chars = string1;
    item[1].nchars = 1;
    item[1].delta = -(info->se_height);
    item[1].font = None;

    string[0] = VERT_SLIDER_BOTTOM_ENDCAP_OUTLINE;
    string1[0] = VERT_SLIDER_BOTTOM_ENDCAP_FILL;

    XDrawText(info->dpy, win, info->gc_rec[OLGX_BLACK]->gc, x + left_offset,
	      y + width - left_offset - info->gauge_width, item, 2);

    XFillRectangle(info->dpy, win, info->gc_rec[OLGX_BLACK]->gc, x + left_offset,
	       y + value, (info->gauge_height == 13) ? info->se_height + 1 :
	 info->se_height, width - value - left_offset - info->se_width - 1);


    if (info->three_d)

	/* Draw the special line for 3-D */

	XDrawLine(info->dpy, win, info->gc_rec[OLGX_BG2]->gc, x + left_offset + 1,
		  y + value, x + left_offset + 1, 
                  y + width - left_offset - info->se_width);

}


void
olgx_update_vertical_gauge(info, win, x, y, width, oldval, newval)
    Graphics_info  *info;
    Window          win;
    int             x, y, width, oldval, newval;

{
    register        left_offset;



    left_offset = 3;

    if (info->gauge_height > 14)
	left_offset = 5;

    if (oldval == newval)
	return;

    if (oldval > newval)
	XFillRectangle(info->dpy, win, info->gc_rec[OLGX_BLACK]->gc, x + left_offset,
                       y + newval, 
                       (info->gauge_height == 13) ?
		        info->se_height + 1 : info->se_height,
                       oldval - newval);

    else

	XFillRectangle(info->dpy, win, (info->three_d) ?
                                         info->gc_rec[OLGX_BG2]->gc :
		                         info->gc_rec[OLGX_WHITE]->gc,
                       x + left_offset, y + oldval, 
                       (info->gauge_height == 13) ?
		          info->se_height + 1 : info->se_height,
                       newval - oldval);

    if (info->three_d)

	/* Draw the special line for 3-D */

	XDrawLine(info->dpy, win, info->gc_rec[OLGX_BG2]->gc, x + left_offset + 1,
		  y + newval, x + left_offset + 1, 
                  y + width - left_offset - info->se_width);

}
