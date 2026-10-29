/*#ident "@(#)ol_button.c	1.62 93/06/28 SMI" */

/*
 * Copyright 1989-1990 Sun Microsystems
 */

/*
 * OPEN LOOK object drawing package Sun Microsystems, Inc.
 * 
 * OLGX_button.c Menu button module
 */

#include <stdio.h>
#include <stdlib.h>  /* for mblen() */
#include <string.h>
#include <X11/Xlib.h>
#include <olgx_private/olgx_impl.h>

/*
 * Private function declarations
 */

void            olgx_set_busy_stipple();
void            olgx_draw_pixmap_label();
void            olgx_draw_ximage_label();

static void     olgx_draw_accel_label_internal();
static void     olgx_draw_diamond_mark();
static void     olgx_draw_label();

void
olgx_draw_button(info, win, x, y, width, height, label, state)
    Graphics_info  *info;
    Window          win;
    int             x, y, width, height;
    void           *label;
    int             state;
{
    XTextItem       item;
    char            string[STRING_SIZE];
    short           add_ins[STRING_SIZE];
    register int    i;
    int             num_add;
    int             inside_width;	/* width minus endcaps */
    int             top_color, bottom_color, fill_color;


    inside_width = width - (2 * info->endcap_width);

    num_add = calc_add_ins(inside_width - 1, add_ins);
    item.nchars = 2 + num_add;
    item.font = None;
    item.chars = string;
    item.delta = 0;


    if (height && (height != Button_Height(info)))
	/* variable height button-- possibly a pixmap label */

	olgx_draw_varheight_button(info, win, x, y, width, height, state);

    else {

	if (info->three_d) {

	    /*
	     * 3d determine what colors we should draw in
	     */

	    if (state & OLGX_INVOKED) {	/* invoked button */
		top_color = OLGX_BG3;
		bottom_color = OLGX_WHITE;
		fill_color = OLGX_BG2;

	    } else if ((state & OLGX_DEFAULT) && (state & OLGX_MENU_ITEM)) {

              	/* default menu item */
		top_color = bottom_color = OLGX_BG3;
		fill_color = OLGX_BG1;

	    } else if (state & OLGX_MENU_ITEM && state & OLGX_BUSY) {
              	/* busy menu item */

		fill_color = top_color = bottom_color = OLGX_BG1;

	    } else if (state & OLGX_MENU_ITEM) {	/* normal menu item */

		fill_color = top_color = bottom_color = NONE;

	    } else {		/* normal panel button */

		top_color = OLGX_WHITE;
		bottom_color = OLGX_BG3;
		fill_color = OLGX_BG1;

	    }

	    if (state & OLGX_BUSY) {

		/*
		 * This routine changes GC information on-the-fly, but it is
		 * assumed that OLGX_BUSY won't be called often, so it makes
		 * sense to use the same GC rather than one for ` each color.
		 */

		if (!info->gc_rec[OLGX_BUSYGC])
		    olgx_initialise_gcrec(info, OLGX_BUSYGC);
		fill_color = OLGX_BUSYGC;

	    }
	    /* only check erase on transparent items */

	    if (fill_color == NONE) {

		if (state & OLGX_ERASE) {

		    /*
		     * to improve performance, we erase a rectangle the size
		     * of a button rather than drawing a real button.
		     */

		   XFillRectangle(info->dpy, win,info->gc_rec[OLGX_BG1]->gc, 
                                  x, y, width, Button_Height(info));
		}
	    } else {		/* if not transparent, actually draw the
				 * button */

		if (top_color != NONE) {

		    /* draw the top part of the button */

		    string[0] = BUTTON_UL;
		    VARIABLE_LENGTH_MACRO(1, BUTTON_TOP_1);
		    string[i + 1] = BUTTON_UR;
		    XDrawText(info->dpy, win,
			      info->gc_rec[top_color]->gc, x, y, &item, 1);

		}
		if (bottom_color != NONE) {

		    /* draw the bottom part of the button */

		    string[0] = BUTTON_LL;
		    VARIABLE_LENGTH_MACRO(1, BUTTON_BOTTOM_1);
		    string[i + 1] = BUTTON_LR;
		    XDrawText(info->dpy, win,
			    info->gc_rec[bottom_color]->gc, x, y, &item, 1);

		}
		/* Fill in the button */

		string[0] = BUTTON_LEFT_ENDCAP_FILL;
		VARIABLE_LENGTH_MACRO(1, BUTTON_FILL_1);
		string[i + 1] = BUTTON_RIGHT_ENDCAP_FILL;
		XDrawText(info->dpy, win,
			  info->gc_rec[fill_color]->gc, x, y, &item, 1);

		/* draw the inner border of a default button (not menu item) */

		if (!(state & OLGX_MENU_ITEM) && (state & OLGX_DEFAULT)) {
		    string[0] = DFLT_BUTTON_LEFT_ENDCAP;
		    VARIABLE_LENGTH_MACRO(1, DFLT_BUTTON_MIDDLE_1);
		    string[i + 1] = DFLT_BUTTON_RIGHT_ENDCAP;
		    XDrawText(info->dpy, win,
			      info->gc_rec[OLGX_BG3]->gc, x, y, &item, 1);
		}
	    }			/* Not transparent */

	}
	/* End 3D */
	else {			/* draw 2d button */

	    if (state & OLGX_ERASE)
                XFillRectangle(info->dpy, win, info->gc_rec[OLGX_WHITE]->gc, x,
                               y, width + 1, Button_Height(info));

	    if ((state & OLGX_INVOKED)) {
		string[0] = BUTTON_FILL_2D_LEFTENDCAP;
		VARIABLE_LENGTH_MACRO(1, BUTTON_FILL_2D_MIDDLE_1);
		string[i + 1] = BUTTON_FILL_2D_RIGHTENDCAP;
		XDrawText(info->dpy, win,
			  info->gc_rec[OLGX_BLACK]->gc, x, y, &item, 1);

	    } else if (state & OLGX_BUSY) {

		if (!info->gc_rec[OLGX_BUSYGC])
		    olgx_initialise_gcrec(info, OLGX_BUSYGC);
		string[0] = BUTTON_FILL_2D_LEFTENDCAP;
		VARIABLE_LENGTH_MACRO(1, BUTTON_FILL_2D_MIDDLE_1);
		string[i + 1] = BUTTON_FILL_2D_RIGHTENDCAP;
		XDrawText(info->dpy, win,
			  info->gc_rec[OLGX_BUSYGC]->gc, x, y, &item, 1);

	    } else if (!(state & OLGX_MENU_ITEM) && (state & OLGX_DEFAULT)) {

		/* draw the 2d default ring if not menu-item */

		string[0] = DFLT_BUTTON_LEFT_ENDCAP;
		VARIABLE_LENGTH_MACRO(1, DFLT_BUTTON_MIDDLE_1);
		string[i + 1] = DFLT_BUTTON_RIGHT_ENDCAP;
		XDrawText(info->dpy, win,
			  info->gc_rec[OLGX_BLACK]->gc, x, y, &item, 1);

	    } else if (state & OLGX_DEFAULT) {

		/* draw the 2d default ring for menu item */

		string[0] = MENU_DFLT_OUTLINE_LEFT_ENDCAP;
		VARIABLE_LENGTH_MACRO(1, MENU_DFLT_OUTLINE_MIDDLE_1);
		string[i + 1] = MENU_DFLT_OUTLINE_RIGHT_ENDCAP;
		XDrawText(info->dpy, win,
			  info->gc_rec[OLGX_BLACK]->gc, x, y, &item, 1);

	    }
	    /* draw the button if it is not a menu item */

	    if (!(state & OLGX_MENU_ITEM)) {
		string[0] = BUTTON_OUTLINE_LEFT_ENDCAP;
		VARIABLE_LENGTH_MACRO(1, BUTTON_OUTLINE_MIDDLE_1);
		string[i + 1] = BUTTON_OUTLINE_RIGHT_ENDCAP;
		XDrawText(info->dpy, win,
			  info->gc_rec[OLGX_BLACK]->gc, x, y, &item, 1);
	    }
	}
    }

    /*
     * Place the label, if specified.
     */

    if (label) {

	if (state & OLGX_LABEL_IS_PIXMAP) {

	    int             centerx, centery;

	    centerx = (width - ((Pixlabel *) label)->width >> 1);
	    centery = (height - ((Pixlabel *) label)->height >> 1);
	    olgx_draw_pixmap_label(info, win,
				   ((Pixlabel *) label)->pixmap,
				   x + ((centerx > 0) ? centerx : 0),
				   y + ((centery > 0) ? centery : 0),
				   (((Pixlabel *) label)->width > width)? 
                                               width:((Pixlabel *)label)->width ,
				  (height) ? 
				   ((((Pixlabel *) label)->height > height) ? 
                                               height : ((Pixlabel *)label)->height)
					: Button_Height(info) - 2, state);

	} else if (state & OLGX_LABEL_IS_XIMAGE) {

	    int             centerx, centery;

	    centerx = (width - ((Pixlabel *) label)->width >> 1);
	    centery = (height - ((Pixlabel *) label)->height >> 1);
	    olgx_draw_ximage_label(info, win,
				   ((Ximlabel *) label)->ximage,
				   x + ((centerx > 0) ? centerx : 0),
				   y + ((centery > 0) ? centery : 0),
				   (((Pixlabel *) label)->width > width)? 
                                               width:((Pixlabel *)label)->width ,
				  (height) ? 
				   ((((Pixlabel *) label)->height > height) ? 
                                               height : ((Pixlabel *)label)->height)
					: Button_Height(info) - 2, state);
       } else {


	   /* need to remove the inactive flag because the button is
	      later covered with a stipple pattern which will cancel
	      out the stipple in olgx_draw_text() */
#ifdef OW_I18N
	    olgx_draw_text(info, win, label,
#else
	    olgx_draw_text(info, win, (char *) label,
#endif /* OW_I18N */
			   x + info->endcap_width,
			   y + info->button_height - info->base_off,
			   inside_width -
			   ((state & OLGX_MENU_MARK) ?
			    info->mm_width : 0),
			   state & ~ OLGX_INACTIVE);
	}
    }
    /*
     * Place the menu mark, if desired.
     */

    if (state & OLGX_MENU_MARK) {

	/*
	 * draw the menu mark. (fill_color != OLGX_BG2) causes the menu mark
	 * to be filled in only when necessary
	 */

	if (info->three_d)
	    olgx_draw_menu_mark(info, win,
			  x + (width - info->endcap_width - info->mm_width),
				y + (info->button_height - info->mm_height -
				     info->base_off),
				state & ~OLGX_INACTIVE, 
				(fill_color != OLGX_BG2));
	else
	    olgx_draw_menu_mark(info, win,
			  x + (width - info->endcap_width - info->mm_width),
				y + (info->button_height - info->mm_height -
				     info->base_off),
				state & ~OLGX_INACTIVE, 
				0);
    }
    /*
     * Mark the item as inactive, if specified
     */

    if (state & OLGX_INACTIVE) {
	olgx_stipple_rect(info, win, x, y, width, 
                          (height) ? height + 1 : Button_Height(info));

    }
}


/*
 * Draw the outline of a variable height button Private Routine
 */

void
olgx_draw_varheight_button(info, win, x, y, width, height, state)
    Graphics_info  *info;
    Window          win;
    int             x, y, width, height;
    int             state;

{


    char            string[2];
    XSegment        seg[4];


    if (info->three_d) {

	/* 3D */
	/* Draw all the four corners */

	if (state & OLGX_INVOKED)
	    XFillRectangle(info->dpy, win, info->gc_rec[OLGX_BG2]->gc, x + 1,
			   y + 1, width - 2, height - 2);
	else
	    XFillRectangle(info->dpy, win, info->gc_rec[OLGX_BG1]->gc, x + 1,
			   y + 1, width - 2, height - 2);

	string[0] = PIXLABEL_BUTTON_UL;
	XDrawString(info->dpy, win, (state & OLGX_INVOKED) ?
	    info->gc_rec[OLGX_BG3]->gc : info->gc_rec[OLGX_WHITE]->gc, x, y,
		    string, 1);

	string[0] = PIXLABEL_BUTTON_UR;
	XDrawString(info->dpy, win, (state & OLGX_INVOKED) ?
		  info->gc_rec[OLGX_BG3]->gc : info->gc_rec[OLGX_WHITE]->gc,
		    x + width - VARHEIGHT_BUTTON_CORNER_DIMEN, y, string, 1);

	string[0] = PIXLABEL_BUTTON_LL;
	XDrawString(info->dpy, win, (state & OLGX_INVOKED) ?
	   info->gc_rec[OLGX_WHITE]->gc : info->gc_rec[OLGX_BG3]->gc, x, y +
		    height - VARHEIGHT_BUTTON_CORNER_DIMEN, string, 1);

	string[0] = PIXLABEL_BUTTON_LR;
	XDrawString(info->dpy, win, (state & OLGX_INVOKED) ?
		  info->gc_rec[OLGX_WHITE]->gc : info->gc_rec[OLGX_BG3]->gc,
		    x + width - VARHEIGHT_BUTTON_CORNER_DIMEN,
		    y + height - VARHEIGHT_BUTTON_CORNER_DIMEN, string, 1);

	seg[0].x1 = x + VARHEIGHT_BUTTON_CORNER_DIMEN;
	seg[0].y1 = seg[0].y2 = y;
	seg[0].x2 = x + width - VARHEIGHT_BUTTON_CORNER_DIMEN;
	seg[1].x1 = seg[1].x2 = x;
	seg[1].y1 = y + VARHEIGHT_BUTTON_CORNER_DIMEN;
	seg[1].y2 = y + height - VARHEIGHT_BUTTON_CORNER_DIMEN;
	XDrawSegments(info->dpy, win, (state & OLGX_INVOKED) ? 
                                        info->gc_rec[OLGX_BG3]->gc :
		                        info->gc_rec[OLGX_WHITE]->gc,
                     seg, 2);
	seg[0].x1 = x + VARHEIGHT_BUTTON_CORNER_DIMEN;
	seg[0].y1 = seg[0].y2 = y + height - 1;
	seg[0].x2 = x + width - VARHEIGHT_BUTTON_CORNER_DIMEN;
	seg[1].x1 = seg[1].x2 = x + width - 1;
	seg[1].y1 = y + VARHEIGHT_BUTTON_CORNER_DIMEN;
	seg[1].y2 = y + height - VARHEIGHT_BUTTON_CORNER_DIMEN;
	XDrawSegments(info->dpy, win, (state & OLGX_INVOKED) ? 
                                        info->gc_rec[OLGX_WHITE]->gc :
		                        info->gc_rec[OLGX_BG3]->gc,
                      seg, 2);


    } else {

	/* 2D */

	if (state & OLGX_INVOKED)
	    XFillRectangle(info->dpy, win, info->gc_rec[OLGX_BLACK]->gc, x + 3,
			   y + 3, width - 6, height - 6);
	else
	    XFillRectangle(info->dpy, win, info->gc_rec[OLGX_WHITE]->gc, x + 1,
			   y + 1, width - 2, height - 2);

	string[0] = PIXLABEL_BUTTON_UL;
	XDrawString(info->dpy, win, info->gc_rec[OLGX_BLACK]->gc, x, y, string, 1);
	string[0] = PIXLABEL_BUTTON_UR;
	XDrawString(info->dpy, win, info->gc_rec[OLGX_BLACK]->gc,
		    x + width - VARHEIGHT_BUTTON_CORNER_DIMEN, y, string, 1);

	string[0] = PIXLABEL_BUTTON_LL;
	XDrawString(info->dpy, win, info->gc_rec[OLGX_BLACK]->gc, x,
		    y + height - VARHEIGHT_BUTTON_CORNER_DIMEN, string, 1);

	string[0] = PIXLABEL_BUTTON_LR;
	XDrawString(info->dpy, win, info->gc_rec[OLGX_BLACK]->gc,
		    x + width - VARHEIGHT_BUTTON_CORNER_DIMEN,
		    y + height - VARHEIGHT_BUTTON_CORNER_DIMEN, string, 1);

	seg[0].x1 = x + VARHEIGHT_BUTTON_CORNER_DIMEN;
	seg[0].y1 = seg[0].y2 = y;
	seg[0].x2 = x + width - VARHEIGHT_BUTTON_CORNER_DIMEN;
	seg[1].x1 = seg[1].x2 = x;
	seg[1].y1 = y + VARHEIGHT_BUTTON_CORNER_DIMEN;
	seg[1].y2 = y + height - VARHEIGHT_BUTTON_CORNER_DIMEN;
	seg[2].x1 = x + VARHEIGHT_BUTTON_CORNER_DIMEN;
	seg[2].y1 = seg[2].y2 = y + height - 1;
	seg[2].x2 = x + width - VARHEIGHT_BUTTON_CORNER_DIMEN;
	seg[3].x1 = seg[3].x2 = x + width - 1;
	seg[3].y1 = y + VARHEIGHT_BUTTON_CORNER_DIMEN;
	seg[3].y2 = y + height - VARHEIGHT_BUTTON_CORNER_DIMEN;
	XDrawSegments(info->dpy, win, info->gc_rec[OLGX_BLACK]->gc, seg, 4);

    }

    /*
     * REMIND: the code below probably uses OLGX_BLACK incorrectly.  It should 
     * be changed to use OLGX_BG3 in 3D mode as appropriate.
     */

    if (state & OLGX_DEFAULT) {

	string[0] = PIXLABEL_DEF_BUTTON_UL;
	XDrawString(info->dpy, win, info->gc_rec[OLGX_BLACK]->gc, x, y,
		    string, 1);

	string[0] = PIXLABEL_DEF_BUTTON_UR;
	XDrawString(info->dpy, win, info->gc_rec[OLGX_BLACK]->gc, x + width -
		    VARHEIGHT_BUTTON_CORNER_DIMEN, y, string, 1);

	string[0] = PIXLABEL_DEF_BUTTON_LL;
	XDrawString(info->dpy, win,
		    info->gc_rec[OLGX_BLACK]->gc, x, y + height -
		    VARHEIGHT_BUTTON_CORNER_DIMEN, string, 1);

	string[0] = PIXLABEL_DEF_BUTTON_LR;
	XDrawString(info->dpy, win,
		    info->gc_rec[OLGX_BLACK]->gc, x + width -
		    VARHEIGHT_BUTTON_CORNER_DIMEN, y + height -
		    VARHEIGHT_BUTTON_CORNER_DIMEN, string, 1);

	seg[0].x1 = x + VARHEIGHT_BUTTON_CORNER_DIMEN;
	seg[0].y1 = seg[0].y2 = y + 2;
	seg[0].x2 = x + width - VARHEIGHT_BUTTON_CORNER_DIMEN - 1;
	seg[1].x1 = seg[1].x2 = x + 2;
	seg[1].y1 = y + VARHEIGHT_BUTTON_CORNER_DIMEN;
	seg[1].y2 = y + height - VARHEIGHT_BUTTON_CORNER_DIMEN - 1;
	seg[2].x1 = x + VARHEIGHT_BUTTON_CORNER_DIMEN;
	seg[2].y1 = seg[2].y2 = y + height - 1 - 2;
	seg[2].x2 = x + width - VARHEIGHT_BUTTON_CORNER_DIMEN - 1;
	seg[3].x1 = seg[3].x2 = x + width - 1 - 2;
	seg[3].y1 = y + VARHEIGHT_BUTTON_CORNER_DIMEN;
	seg[3].y2 = y + height - VARHEIGHT_BUTTON_CORNER_DIMEN - 1;
	XDrawSegments(info->dpy, win,
		      info->gc_rec[OLGX_BLACK]->gc, seg, 4);

    }
    if (state & OLGX_BUSY) {

	if (!info->gc_rec[OLGX_BUSYGC])
	    olgx_initialise_gcrec(info, OLGX_BUSYGC);

	XFillRectangle(info->dpy, win, info->gc_rec[OLGX_BUSYGC]->gc, x + 2,
		       y + 2, width - 4, height - 4);
    }
}


void
olgx_draw_menu_mark(info, win, x, y, state, fill_in)
    Graphics_info  *info;
    Window          win;
    int             state, fill_in;
{
    char            string[3];
    GC              gc;

    if (state & OLGX_VERT_MENU_MARK)
	string[0] = VERT_MENU_MARK_UL;
    else if (state & OLGX_HORIZ_MENU_MARK)
	string[0] = HORIZ_MENU_MARK_UL;
    else if (state & OLGX_HORIZ_BACK_MENU_MARK)
	string[0] = HORIZ_BACK_MENU_MARK_UL;
    else if (state & OLGX_VERT_BACK_MENU_MARK)
	string[0] = VERT_BACK_MENU_MARK_UL;

    string[1] = string[0] + 1;

    if ((state & OLGX_INVOKED) && (!info->three_d)) {
	gc = info->gc_rec[OLGX_WHITE]->gc;
    } else { 
	if (info->three_d) {
	    gc = info->gc_rec[OLGX_BG3]->gc;
	} else {
	    gc = info->gc_rec[OLGX_BLACK]->gc;
	}
    }
    if (state & OLGX_INACTIVE) {
	XSetFillStyle(info->dpy,gc,FillStippled);
    }
    XDrawString(info->dpy, win, gc, x, y, &string[0],
		info->three_d ? 1 : 2);
    if (state & OLGX_INACTIVE) {
	XSetFillStyle(info->dpy,gc,FillSolid);
    }
    if (info->three_d) {
	gc = info->gc_rec[OLGX_WHITE]->gc;
	if (state & OLGX_INACTIVE) {
	    XSetFillStyle(info->dpy,gc,FillStippled);
	}
	XDrawString(info->dpy, win, gc, x, y, &string[1], 1);
	if (state & OLGX_INACTIVE) {
	    XSetFillStyle(info->dpy,gc,FillSolid);
	}
    }

    /* fill in the menu mark, if requested */

    if (fill_in) {	
	string[0] = string[0] + 2;
	if (info->three_d) {
	    gc = info->gc_rec[OLGX_BG2]->gc;
	} else {
	    gc = info->gc_rec[OLGX_BLACK]->gc;
	}
	if (state & OLGX_INACTIVE) {
	    XSetFillStyle(info->dpy,gc,FillStippled);
	}
	XDrawString(info->dpy, win, gc, x, y, &string[0], 1);
	if (state & OLGX_INACTIVE) {
	    XSetFillStyle(info->dpy,gc,FillSolid);
	}
    }
}


void
olgx_draw_abbrev_button(info, win, x, y, state)
    Graphics_info  *info;
    Window          win;
    int             x, y;
    int             state;
{
    XTextItem       item;
    char            string[3];
    int             top_color, bottom_color, fill_color;

    item.nchars = 1;
    item.font = None;
    item.chars = string;
    item.delta = 0;


    if (!info->three_d) {	/* 2d */

	if (state & OLGX_ERASE)
	    XClearArea(info->dpy, win, x, y,
	      Abbrev_MenuButton_Width(info), Abbrev_MenuButton_Width(info),0);

	if (state & OLGX_BUSY) {

	    if (!info->gc_rec[OLGX_BUSYGC])
		olgx_initialise_gcrec(info, OLGX_BUSYGC);
	    string[0] = ABBREV_MENU_FILL;
	    XDrawText(info->dpy, win, info->gc_rec[OLGX_BUSYGC]->gc, x, y, &item, 1);
	}
	if (state & OLGX_INVOKED) {

	    XFillRectangle(info->dpy, win, info->gc_rec[OLGX_WHITE]->gc, x + 2, y + 2,
			   (Abbrev_MenuButton_Width(info) - 4), 
                           (Abbrev_MenuButton_Width(info) - 4));
	    string[0] = OLG_ABBREV_MENU_BUTTON_INVERTED;
	    XDrawText(info->dpy, win, info->gc_rec[OLGX_BLACK]->gc, x, y, &item, 1);

	} else {

	    string[0] = OLG_ABBREV_MENU_BUTTON;
	    XDrawText(info->dpy, win, info->gc_rec[OLGX_BLACK]->gc, x, y, &item, 1);

	}

    } else {			/* 3d */

	if (state & OLGX_INVOKED) {

	    top_color = OLGX_BG3;
	    bottom_color = OLGX_WHITE;
	    fill_color = OLGX_BG2;

	} else {

	    top_color = OLGX_WHITE;
	    bottom_color = OLGX_BG3;
	    fill_color = OLGX_BG1;

	}

	if (state & OLGX_BUSY) {

	    if (!info->gc_rec[OLGX_BUSYGC])
		olgx_initialise_gcrec(info, OLGX_BUSYGC);
	    fill_color = OLGX_BUSYGC;

	}
	string[0] = ABBREV_MENU_UL;
	XDrawText(info->dpy, win, info->gc_rec[top_color]->gc, x, y, &item, 1);

	string[0] = ABBREV_MENU_LR;
	XDrawText(info->dpy, win, info->gc_rec[bottom_color]->gc, x, y, &item, 1);

	string[0] = ABBREV_MENU_FILL;
	XDrawText(info->dpy, win, info->gc_rec[fill_color]->gc, x, y, &item, 1);
	olgx_draw_menu_mark(info, win, x + ((Abbrev_MenuButton_Width(info)
					     - info->mm_width) >> 1),
			    y + ((1 + Abbrev_MenuButton_Height(info) -
				  info->mm_height) >> 1),
			    OLGX_VERT_MENU_MARK & ~OLGX_INACTIVE, 1);

    }

    /* If it is inactive fill the rectangle with inactive pixmap */

    if (state & OLGX_INACTIVE) {

	olgx_stipple_rect(info, win, x, y, Abbrev_MenuButton_Width(info),
			  Abbrev_MenuButton_Height(info));
    }
}


void
olgx_stipple_rect(info, win, x, y, width, height)
    Graphics_info  *info;
    Window          win;
    int             x, y, width, height;
{


    if (!info->gc_rec[OLGX_GREY_OUT])
	olgx_initialise_gcrec(info, OLGX_GREY_OUT);

    XFillRectangle(info->dpy, win, info->gc_rec[OLGX_GREY_OUT]->gc,
		   x, y, width, height);
}

void
olgx_draw_text(info, win, string, x, y, max_width, state)
    Graphics_info  *info;
    Window          win;
#ifdef OW_I18N
    void           *string;
#else
    char           *string;
#endif /* OW_I18N */
    int             x, y, max_width;
    int             state;
{

#ifdef OW_I18N
    int             len;
    int		    mb_len;
    char	   *mbs;
    wchar_t	   *wcs;
#else
    unsigned int   len ;
#endif /* OW_I18N */
    int		    current_width = 0;
    int		    width;
    GC		    gc;
    register int    i;
    short           more_flag = 0;


#ifdef OW_I18N
    if ((Olgx_Flags(info) & OLGX_FONTSET) && (state & OLGX_LABEL_IS_WCS)) {
	wcs = (wchar_t *) string;
	len = wslen(wcs);
    } else {
	mbs = (char *) string;
	len = strlen(mbs);
    }
#else 
        len =  strlen(string);
#endif /* OW_I18N */

    /*
     * if the string is too long, we'll have to truncate it max_width == 0
     * implies don't truncate.
     */

#ifdef OW_I18N
    if (Olgx_Flags(info) & OLGX_FONTSET) {
	if (state & OLGX_LABEL_IS_WCS) {
	    /*
	     * Case for the wide character string
	     */
	    width = XwcTextEscapement(info->textfontset, wcs, len);
	    if (max_width && width > max_width) {
		for (i = 0; (i < len && current_width <= max_width); i++) {
		    current_width +=
			XwcTextEscapement(info->textfontset,
					  &wcs[i], 1);
		}
		len = i - 2;
		if (state & OLGX_MORE_ARROW)
		   more_flag = 1;
	    }
	    width = XwcTextEscapement(info->textfontset, wcs, len);
	} else {
	    /*
	     * Case for the multibyte string.  It might be faster to
	     * convert it to wide char....
	     */
	    width = XmbTextEscapement(info->textfontset, mbs, len);
	    if (max_width && width > max_width) {
		for (i = 0; (i < len && current_width <= max_width); ) {
		    mb_len = mblen(&mbs[i],len);
		    current_width +=
			XmbTextEscapement(info->textfontset,
					  &mbs[i], mb_len);
		    i += mb_len;
		}
		len = i - 2;
		if (state & OLGX_MORE_ARROW)
		   more_flag = 1;
		width = XmbTextEscapement(info->textfontset, mbs, len);
	    }
	}
    } else {
	width = XTextWidth(TextFont_Struct(info), mbs, len);
	if (max_width && width > max_width) {
	    for (i = 0; (i < len && current_width <= max_width); i++) {
		current_width +=
			XTextWidth(TextFont_Struct(info), &mbs[i], 1);
	    }
	    len = i - 2;
	    if (state & OLGX_MORE_ARROW)
		more_flag = 1;
	    width = XTextWidth(TextFont_Struct(info), mbs, len);
	}
    }
#else /* OW_I18N */
    if (max_width && XTextWidth(TextFont_Struct(info), 
				string, len) > max_width) {
	for (i = 0; (i < len && current_width <= max_width); i++) {
	    current_width += XTextWidth(TextFont_Struct(info), &string[i], 1);
	}	
	
	/*
	 * at this point, i-1 represents the number of chars of string that
	 * will fit into max_width.
	 */

	len = i - 2;
	if (state & OLGX_MORE_ARROW) 
	    more_flag = 1;
	width = XTextWidth(TextFont_Struct(info),string,len);
    }	
#endif /* OW_I18N */
    if ((state & OLGX_INVOKED) && !(info->three_d)) {
	if (!info->gc_rec[OLGX_TEXTGC_REV])
	    olgx_initialise_gcrec(info, OLGX_TEXTGC_REV);
	gc = info->gc_rec[OLGX_TEXTGC_REV]->gc;
    } else {
	if (!info->gc_rec[OLGX_TEXTGC])
	    olgx_initialise_gcrec(info, OLGX_TEXTGC);
	gc = info->gc_rec[OLGX_TEXTGC]->gc;
    }
    if (state & OLGX_INACTIVE) {
	XSetFillStyle(info->dpy,gc,FillStippled);
    }

#ifdef OW_I18N
    if (Olgx_Flags(info) & OLGX_FONTSET) {
	if (state & OLGX_LABEL_IS_WCS)
	    XwcDrawString(info->dpy, win, info->textfontset, gc, x, y,
			  wcs, len);
	else
	    XmbDrawString(info->dpy, win, info->textfontset, gc, x, y,
			  mbs, len);
    } else
	XDrawString(info->dpy, win, gc, x, y, mbs, len);

#else /* OW_I18N */
    XDrawString(info->dpy, win, gc, x, y, string, len);
#endif /* OW_I18N */

    if (state & OLGX_INACTIVE) {
	XSetFillStyle(info->dpy,gc,FillSolid);
    }

    if (more_flag) /* render a more arrow at the end of the string */ 

       olgx_draw_menu_mark(info,win,
			   x + width + 1,
			   y-MenuMark_Height(info),OLGX_HORIZ_MENU_MARK,1);
}

void
olgx_draw_pixmap_label(info, win, pix, x, y, width, height, state)
    Graphics_info  *info;
    Window          win;
    Pixmap          pix;
    int             x, y, width, height, state;
{


    unsigned long   savebg1;
    unsigned long   savebg2;
    Window              root;
    int                 x_dummy,y_dummy;
    unsigned int        w_dummy,h_dummy,bw_dummy;
    unsigned int        depth;

    if (!info->gc_rec[OLGX_TEXTGC])
	olgx_initialise_gcrec(info, OLGX_TEXTGC);
    if (!info->three_d)
      if (!info->gc_rec[OLGX_TEXTGC_REV])
	    olgx_initialise_gcrec(info, OLGX_TEXTGC_REV);

    if ((state & OLGX_INVOKED) && (info->three_d)) {

	/*
	 * reset the value of the textgc background from bg1 to bg2 in
	 * invoked mode to get the transparent pixmap effect
	 */

	savebg1 = olgx_get_single_color(info, OLGX_BG1);
	savebg2 = olgx_get_single_color(info, OLGX_BG2);
	olgx_set_single_color(info, OLGX_BG1, savebg2, OLGX_SPECIAL);

    }
    /*
     * Performance Problem - RoundTrip request
     * Depth should be passed as part of Pixlabel struct
     */
 
    XGetGeometry(info->dpy,pix,&root,&x_dummy,&y_dummy,&w_dummy,
                 &h_dummy,&bw_dummy,&depth);
    if (depth > 1)
        XCopyArea(info->dpy,
                  pix,          /* src */
                  win,          /* dest */
                  info->gc_rec[OLGX_TEXTGC]->gc,
                  0, 0,         /* src x,y */
                  width, height,
                  x, y);
    else
        XCopyPlane(info->dpy,
   	           pix,		/* src */
	           win,		/* dest */
	           info->gc_rec[OLGX_TEXTGC]->gc,
	           0, 0,		/* src x,y */
	           width, height,
	           x, y,
	           (unsigned long) 1);	/* bit plane */

    /* Restore the original colors to the textgc  */

    if ((state & OLGX_INVOKED) && (info->three_d))
	olgx_set_single_color(info, OLGX_BG1, savebg1, OLGX_SPECIAL);

}

void
olgx_draw_ximage_label(info, win, xim, x, y, width, height, state)
    Graphics_info  *info;
    Window          win;
    XImage         *xim;
    int             x, y, width, height, state;
{


    unsigned long   savebg1;
    unsigned long   savebg2;
    Window              root;
    int                 x_dummy,y_dummy;
    unsigned int        w_dummy,h_dummy,bw_dummy;
    unsigned int        depth;

    if (!info->gc_rec[OLGX_TEXTGC])
	olgx_initialise_gcrec(info, OLGX_TEXTGC);
    if (!info->three_d)
      if (!info->gc_rec[OLGX_TEXTGC_REV])
	    olgx_initialise_gcrec(info, OLGX_TEXTGC_REV);

    if ((state & OLGX_INVOKED) && (info->three_d)) {

	/*
	 * reset the value of the textgc background from bg1 to bg2 in
	 * invoked mode to get the transparent pixmap effect
	 */

	savebg1 = olgx_get_single_color(info, OLGX_BG1);
	savebg2 = olgx_get_single_color(info, OLGX_BG2);
	olgx_set_single_color(info, OLGX_BG1, savebg2, OLGX_SPECIAL);

    }
     XPutImage(info->dpy,
                  win,          /* dest */
                  info->gc_rec[OLGX_TEXTGC]->gc,
                  xim,
                  0, 0,         /* src x,y */
                  x,y,          /* dest x,y */
                  width, height);

    /* Restore the original colors to the textgc  */

    if ((state & OLGX_INVOKED) && (info->three_d))
	olgx_set_single_color(info, OLGX_BG1, savebg1, OLGX_SPECIAL);

}


void
olgx_draw_textscroll_button(info, win, x, y, state)
    Graphics_info  *info;
    Window          win;
    int             x, y;
    int             state;

{
    char            string[2];
    int             width, height;
    int             arr_x, arr_y;

    /*
     * A small hack to calculate the width, arrow postiton..etc since this
     * routine is expected to tbe used infrequently it is not included as
     * part of the info struct and the follwoing calculations take place each
     * time-- a penalty affordable at the cost of infrequency
     * 
     */

    if ((Abbrev_MenuButton_Height(info)) < 20) {

	width = height = Abbrev_MenuButton_Height(info);

	arr_y = 3;
	arr_x = (width / 3) - 1;

    } else {

	width = height = 25;	/* Special case size-19 */
	arr_y = 5;
	arr_x = 7;

    }


    if (!(info->three_d)) {	/* Start 2-D */

	if (state & OLGX_ERASE)
	    XClearArea(info->dpy, win,x, y, width,
                           height,0);

	if (state & OLGX_SCROLL_FORWARD) {

	    if (state & OLGX_INVOKED)
		string[0] = TEXTSCROLLBUTTON_RIGHT_INV;
	    else
		string[0] = TEXTSCROLLBUTTON_RIGHT;

	    XDrawString(info->dpy, win, info->gc_rec[OLGX_BLACK]->gc, x, y, string,1);

	} else if (state & OLGX_SCROLL_BACKWARD) {

	    if (state & OLGX_INVOKED)
		string[0] = TEXTSCROLLBUTTON_LEFT_INV;
	    else
		string[0] = TEXTSCROLLBUTTON_LEFT;
	    XDrawString(info->dpy, win, info->gc_rec[OLGX_BLACK]->gc, x, y, string,1);
	}
    }
    /* End 2-D */
    else {			/* Start 3-D */

	olgx_draw_box(info, win, x, y, width, height, state, 0);

	if (state & OLGX_SCROLL_FORWARD)
	    olgx_draw_menu_mark(info, win, x + arr_x, y + arr_y,
				OLGX_HORIZ_MENU_MARK | OLGX_INVOKED & ~OLGX_INACTIVE, 1);
	else
	    olgx_draw_menu_mark(info, win, x + arr_x - 1, y + arr_y,
				OLGX_HORIZ_BACK_MENU_MARK | OLGX_INVOKED & ~OLGX_INACTIVE, 1);

    }				/* End 3-D */

    if (state & OLGX_INACTIVE)
	olgx_stipple_rect(info, win, x, y, TextScrollButton_Width(info),
			  TextScrollButton_Height(info));

}






void
olgx_draw_numscroll_button(info, win, x, y, state)
    Graphics_info  *info;
    Window          win;
    int             x, y, state;

{

    char            string[2];
    int             width, height, arr_x, arr_y;

    width = height = TextScrollButton_Height(info);

    if (width < 20) {

	arr_y = 3;
	arr_x = (width / 3) - 1;

    } else {

	arr_y = 5;
	arr_x = 7;

    }


    if (!info->three_d) {	/* draw 2-D */

	if (state & OLGX_ERASE)
	    XClearArea(info->dpy, win,x, y,
                           NumScrollButton_Width(info), height,0);

	if (state & OLGX_SCROLL_FORWARD)
	    string[0] = NUMERIC_SCROLL_BUTTON_RIGHT_INV;

	else if (state & OLGX_SCROLL_BACKWARD)
	    string[0] = NUMERIC_SCROLL_BUTTON_LEFT_INV;

	else
	    string[0] = NUMERIC_SCROLL_BUTTON_NORMAL;

	XDrawString(info->dpy, win, info->gc_rec[OLGX_BLACK]->gc, x, y, string, 1);

    } else {			/* draw 3-D */

	olgx_draw_box(info, win, x, y, width, height,
                      (state & OLGX_SCROLL_BACKWARD) ?
		       OLGX_INVOKED : OLGX_NORMAL, 0);
	olgx_draw_box(info, win, x + width, y, width, height,
		      (state & OLGX_SCROLL_FORWARD) ?
		      OLGX_INVOKED : OLGX_NORMAL, 0);
	olgx_draw_menu_mark(info, win, x + arr_x, y + arr_y,
			    OLGX_VERT_BACK_MENU_MARK | OLGX_INVOKED & ~OLGX_INACTIVE, 1);
	olgx_draw_menu_mark(info, win, x + arr_x + width, y + arr_y,
			    OLGX_VERT_MENU_MARK | OLGX_INVOKED & ~OLGX_INACTIVE, 1);

    }

    if (state & OLGX_INACTIVE)
	olgx_stipple_rect(info, win, x, y, NumScrollButton_Width(info),
			  NumScrollButton_Height(info));

    if (state & OLGX_SCROLL_NO_FORWARD)
	olgx_stipple_rect(info, win, x + TextScrollButton_Width(info) - 1, y,
			  TextScrollButton_Width(info),
			  NumScrollButton_Height(info));

    if (state & OLGX_SCROLL_NO_BACKWARD)
	olgx_stipple_rect(info, win, x, y,
			  TextScrollButton_Width(info) - 2,
			  NumScrollButton_Height(info));
}

static void
olgx_draw_diamond_mark(info, win, x, y, state)
     Graphics_info  *info;
     Window          win;
     int             x, y;
     int             state;  /* unused */
{
    XPoint          point[6];
    int             d_height,d_half_height,d_width,d_half_width;
    GC              gc;
    XGCValues       values;
    char            old_dashes;
    int             old_dash_offset;

    /* diamond height needs to be divisible by 2, but width should be odd. */
    d_half_width = info->mm_height/2;
    d_width = (d_half_width*2)+1;
    d_half_height = info->mm_height/2;
    
    /* Origin (x,y) is in lower left corner.  Points are clockwise from 
       9 O'clock. Point 3 is 1 pixel below point 2 and point 6 is 1 pixel 
       below point 0. */
    /* We need to special case anything greater than or equal to 14pt.
       For large fonts, the diamond looks better a little lower. */
    if (d_half_height < 5) {
	y--;
    }
    /* for 3d, diamond looks better 1 pixel to the left because usually
       it will be part of a menu and needs to line up visually with the
       meta symbol. */
    if (info->three_d) {
	x--;
    }
    point[0].x = x;
    point[0].y = y - d_half_height - 1;
    point[1].x = x + d_half_width;
    point[1].y = point[0].y - d_half_height;
    point[2].x = x + d_width - 1;
    point[2].y = point[0].y;
    point[3].x = point[2].x;
    point[3].y = point[2].y + 1;
    point[4].x = point[1].x;
    point[4].y = y;
    point[5].x = x;
    point[5].y = point[0].y + 1;
	
    /* drawing a diamond in 2d is only doing an XDrawLines() for all
       the points. */

    if (!info->three_d) {
	int dashes_changed = 0;
	if (state & OLGX_INVOKED) {
	    gc = info->gc_rec[OLGX_WHITE]->gc;
	} else {
	    gc = info->gc_rec[OLGX_BLACK]->gc;
	}
	if (state & OLGX_INACTIVE) {
	    values.line_style = LineOnOffDash;
	    XChangeGC(info->dpy, gc, GCLineStyle, &values);
	    dashes_changed = 1;
	}
	XDrawLines(info->dpy, win, gc, point, 6, CoordModeOrigin);
	if (dashes_changed) {
	    values.line_style = LineSolid;
	    XChangeGC(info->dpy, gc, GCLineStyle, &values);
	}
    } else {
	/* draw diamond in 3d colors.
	   light source is from above and diamond is lowered, so
	   draw the upper part of the outline in BG3 and the lower part
	   in white. */

	if (state & OLGX_INACTIVE) {
	    /* just draw all lines together with a dashed pattern. */
	    gc = info->gc_rec[OLGX_BG3]->gc;
	    values.line_style = LineOnOffDash;
	    XChangeGC(info->dpy, gc, GCLineStyle, &values);
	    XDrawLines(info->dpy, win, gc, point, 6, CoordModeOrigin);
	    values.line_style = LineSolid;
	    XChangeGC(info->dpy, gc, GCLineStyle, &values);
	} else {
	    /* fill in diamond first */
	    gc = info->gc_rec[OLGX_BG2]->gc;
	    XFillPolygon(info->dpy, win, gc, point, 6, Convex, 
			 CoordModeOrigin);
	    /* draw upper part in BG3 */
	    gc = info->gc_rec[OLGX_BG3]->gc;
	    XDrawLines(info->dpy, win, gc, point, 3, CoordModeOrigin);
	    /* draw lower part in white */
	    gc = info->gc_rec[OLGX_WHITE]->gc;
	    XDrawLines(info->dpy, win, gc, point+3, 3, CoordModeOrigin);
	}
    }
}

static void
olgx_draw_accel_label_internal(info, win, texty, x, y, width, height, 
			       main_label, m_pos,
			       qualifier_label, q_pos,
			       mark_type, mark_pos,
			       key_label, key_pos,
			       state, centerflag)
    Graphics_info  *info;
    Window          win;
    int             texty;            /* position of text as opposed to real.
                                         This is used by highlight and under-
					 lining */
    int             x, y, width, height;  /* position and size of text */
    void	   *main_label;       /* item main label */
    int             m_pos;            /* position of main label */
    void	   *qualifier_label;  /* qualifier or modifier label */    
    int		    q_pos;	      /* x position of qualifier in pixels */
    int		    mark_type;        /* OLGX_DIAMOND_MARK, MENU_MARK, etc. */
    int		    mark_pos;         /* x position of mark sym. in pixels */
    void	   *key_label;        /* usually a single character */
    int		    key_pos;          /* x position of key char in pixels */
    int             state;            /* state of the actual object */
    int             centerflag;       /* TRUE = center pixmap or ximage,
					 FALSE = place lower left corner
					 at texty */
{
    int text_width;     /* temporary var. for figuring max_width to pass to
			   olgx_draw_text() */
    if (main_label) {
	/* calc. max width of label w.r.t qualifier, mark, accelerator 
	   string, or entire width. */
	if (q_pos) {
	    text_width = q_pos - m_pos;
	} else if (mark_pos) {
	    text_width = mark_pos - m_pos;
	} else if (key_pos) {
	    text_width = key_pos - m_pos;
	} else {
	    text_width = x + width - m_pos;
	}
	/* state can be IS_PIXMAP, IS_XIMAGE, HAS_UNDERLINE, 
	   HAS_HIGHLIGHT or normal. */
	if (state & OLGX_LABEL_IS_PIXMAP) {
	    int              newy;
	    Pixlabel        *plabel;
	    plabel=          (Pixlabel *)main_label;
	    if (centerflag) {
		newy = y + ((height - plabel->height)/2);
	    } else {
		/* put bottom of pixmap at texty */
		newy = texty - plabel->height;
	    }
	    olgx_draw_pixmap_label(info, win,
				   plabel->pixmap,
				   m_pos,
				   newy,
				   (plabel->width > text_width) ? text_width:
				   plabel->width ,
				   height,
				   state);
	} else if (state & OLGX_LABEL_IS_XIMAGE) {
	    int              newy;
	    Ximlabel        *ximlabel;
	    ximlabel =       (Ximlabel *)main_label;
	    if (centerflag) {
		newy = y + ((height - ximlabel->height)/2);
	    } else {
		/* put bottom of pixmap at texty */
		newy = texty - ximlabel->height;
	    }
	    olgx_draw_ximage_label(info, win,
				   ximlabel->ximage,
				   m_pos,
				   newy,
				   (ximlabel->width > text_width)? 
			       text_width:ximlabel->width ,
				   (height) ? height : Button_Height(info) - 2, state);
	} else {
	    int ascent, descent, direction;
	    XCharStruct overall;
	    void *string_label;
	    Underlinelabel *ulabel, *hlabel;
	    char *highlight_char;
	    int has_highlight=0,has_underline=0;
	    int highlightx_pos, highlight_length, highlighty_pos, 
	        highlight_height;
	    if (state & OLGX_LABEL_HAS_UNDERLINE) {
		has_underline = 1;
		ulabel = (Underlinelabel *)main_label;
		string_label = (void *)ulabel->label;
	    } else if (state & OLGX_LABEL_HAS_HIGHLIGHT) {
		GC gc;
		int ascent, descent, direction;
		XCharStruct overall;
		hlabel = (Underlinelabel *)main_label;
		string_label = (void *)hlabel->label;
		if (info->three_d) {
		    gc = info->gc_rec[OLGX_BLACK]->gc;
		} else {
		    if (state & OLGX_INVOKED) {
			if (!info->gc_rec[OLGX_TEXTGC_REV])
			    olgx_initialise_gcrec(info, OLGX_TEXTGC_REV);
			gc = info->gc_rec[OLGX_TEXTGC_REV]->gc;
		    } else {
			if (!info->gc_rec[OLGX_TEXTGC])
			    olgx_initialise_gcrec(info, OLGX_TEXTGC);
			gc = info->gc_rec[OLGX_TEXTGC]->gc;
		    }
		}
#ifdef OW_I18N
		if (Olgx_Flags(info) & OLGX_FONTSET) {
		    XRectangle overall_ink_return, overall_logical_return;
		    int string_width;
		    if (state & OLGX_LABEL_IS_WCS) {
			string_width = XwcTextExtents(info->textfontset,
						      (wchar_t *)string_label,
						      hlabel->position,
						      &overall_ink_return,
						      &overall_logical_return);
			highlight_length = XwcTextEscapement(info->textfontset,
				     (wchar_t *) ((wchar_t *)string_label +
						  hlabel->position), 
							     1);
			highlightx_pos = m_pos + ((hlabel->position) ?
						  string_width :
						  0);
		    } else { /* multibyte case */
			string_width = XmbTextExtents(info->textfontset,
						      (char *)string_label,
						      hlabel->position,
						      &overall_ink_return,
						      &overall_logical_return);
			highlight_char = (char *)string_label+hlabel->position;
			highlight_length = XmbTextEscapement(info->textfontset,
					    highlight_char,
					    mblen(highlight_char,
						  strlen(highlight_char)));
			highlightx_pos = m_pos + ((hlabel->position) ?
						  string_width :
						  0);
		    }
		    highlight_height = overall_logical_return.height + 1;
		    /* overall_logical_return.y is < 0 */
		    highlighty_pos = texty + overall_logical_return.y;
		} else { /* single byte i18n case */
		    XTextExtents(TextFont_Struct(info),(char *)string_label,
				 hlabel->position,&direction,&ascent,&descent,
				 &overall);
		    highlight_length = XTextWidth(TextFont_Struct(info),
						  (char *)string_label + 
						  hlabel->position,
						  1);		
		    highlightx_pos = m_pos + ((hlabel->position) ? 
					      overall.width :
					      0);
		    highlight_height = ascent + descent + 1;
		    highlighty_pos = texty - ascent;
		}
#else
		XTextExtents(TextFont_Struct(info),(char *)string_label,
			     hlabel->position,&direction,&ascent,&descent,
			     &overall);
		highlight_length = XTextWidth(TextFont_Struct(info),
					      (char *)string_label + 
					      hlabel->position,
					      1);		
		highlightx_pos = m_pos + ((hlabel->position) ? 
					  overall.width :
					  0);
		highlight_height = ascent + descent + 1;
		highlighty_pos = texty - ascent;
		
#endif /* OW_I18N */

		if (highlightx_pos < (m_pos + text_width)) {
		    if (state & OLGX_INACTIVE) {
			XSetFillStyle(info->dpy,gc,FillStippled);
		    }
		    XFillRectangle(info->dpy, win, gc, 
				   highlightx_pos,
				   highlighty_pos,
				   highlight_length,
				   highlight_height);
		    if (state & OLGX_INACTIVE) {
			XSetFillStyle(info->dpy,gc,FillSolid);
		    }
		    /* now set flag because highlight actually drawn */
		    has_highlight = 1;
		}
	    } else {
		string_label= main_label;
	    }
	    olgx_draw_text(info, win,
#ifdef OW_I18N
			   string_label,
#else
			   (char *) string_label,
#endif /* OW_I18N */
			   m_pos,
			   texty,
			   text_width,
			   state);
	    if (has_underline) {
		int label_pixel_pos,label_y_pos, label_length;
		GC gc;
		if (!info->gc_rec[OLGX_TEXTGC])
		    olgx_initialise_gcrec(info, OLGX_TEXTGC);
		if (!info->three_d)
		    if (!info->gc_rec[OLGX_TEXTGC_REV])
			olgx_initialise_gcrec(info, OLGX_TEXTGC_REV);
		if ((state & OLGX_INVOKED) && !(info->three_d)) {
		    gc = info->gc_rec[OLGX_TEXTGC_REV]->gc;
		} else {
		    gc = info->gc_rec[OLGX_TEXTGC]->gc;
		}
#ifdef OW_I18N
		if (Olgx_Flags(info) & OLGX_FONTSET) {
		    XRectangle overall_ink_return, overall_logical_return;
		    if (state & OLGX_LABEL_IS_WCS) {
			label_pixel_pos=m_pos +
			    ((ulabel->position) ? 
			     XwcTextEscapement(info->textfontset,
					       (wchar_t *)string_label,
					       ulabel->position) 
			     : 0);
			label_length = XwcTextExtents(info->textfontset,
				     (wchar_t *)((wchar_t *)string_label + 
							 ulabel->position),
						      1,
						      &overall_ink_return,
						      &overall_logical_return);
		    } else {
			label_pixel_pos=m_pos +
			    ((ulabel->position) ? 
			     XmbTextEscapement(info->textfontset,
					       (char *)string_label,
					       ulabel->position) 
			     : 0);
			label_length = XmbTextExtents(info->textfontset,
						      (char *)string_label + 
						      ulabel->position,
					mblen((char *)string_label + 
					      ulabel->position,
					      strlen((char *)string_label +
						     ulabel->position)),
						      &overall_ink_return,
						      &overall_logical_return);
		    }
		    label_y_pos = texty + overall_logical_return.y +
			overall_logical_return.height;
		} else {
		    XTextExtents(TextFont_Struct(info),(char *)string_label,
				 hlabel->position,&direction,&ascent,&descent,
				 &overall);
		    label_pixel_pos=m_pos +
			((ulabel->position) ? XTextWidth(TextFont_Struct(info),
							 (char *)string_label,
							 ulabel->position) 
			 : 0);
		    label_length = XTextWidth(TextFont_Struct(info),
					      (char *)string_label + 
					      ulabel->position,
					      1);
		    label_y_pos = texty + descent;
		}
#else
		XTextExtents(TextFont_Struct(info),(char *)string_label,
			     hlabel->position,&direction,&ascent,&descent,
			     &overall);
		label_pixel_pos=m_pos +
		    ((ulabel->position) ? XTextWidth(TextFont_Struct(info),
						     (char *)string_label,
						     ulabel->position) 
		     : 0);
		label_length = XTextWidth(TextFont_Struct(info),
					  (char *)string_label + 
					  ulabel->position,
					  1);
		label_y_pos = texty + descent; 
#endif /* OW_I18N */
		if (label_pixel_pos < (m_pos + text_width)) {
		    if (state & OLGX_INACTIVE) {
			XSetFillStyle(info->dpy,gc,FillStippled);
		    }
		    XDrawLine(info->dpy, win, 
			      gc,
			      label_pixel_pos, 
			      label_y_pos,
			      label_pixel_pos + 
			      label_length,
			      label_y_pos);
		    if (state & OLGX_INACTIVE) {
			XSetFillStyle(info->dpy,gc,FillSolid);
		    }
		}
	    }
	    if (has_highlight) {
		GC gc;
		XGCValues       values;
		/* cannot call olgx_draw_text() because gc is different */
		if (info->three_d) {
		    gc = info->gc_rec[OLGX_BG1]->gc;
		} else {
		    if (state & OLGX_INVOKED) {
			if (!info->gc_rec[OLGX_TEXTGC])
			    olgx_initialise_gcrec(info, OLGX_TEXTGC);
			gc = info->gc_rec[OLGX_TEXTGC]->gc;
		    } else {
			if (!info->gc_rec[OLGX_TEXTGC_REV])
			    olgx_initialise_gcrec(info, OLGX_TEXTGC_REV);
			gc = info->gc_rec[OLGX_TEXTGC_REV]->gc;
		    }
		}
		/* Since we are drawing the highlight character in the same
		   color that we are drawing the glyphs in, we need to
		   save the glyphfont, then restore it. This is only true
		   for 3d. */
		if (info->three_d) {
		    XGetGCValues(info->dpy,gc,GCFont,&values);
		}
#ifdef OW_I18N
		if (Olgx_Flags(info) & OLGX_FONTSET) {
		    if (state & OLGX_LABEL_IS_WCS) {
			XwcDrawString(info->dpy, win, info->textfontset, gc, 
				      highlightx_pos, texty,
				      (wchar_t *) ((wchar_t *)string_label + 
						   hlabel->position),
				      1);
		    } else {
			XmbDrawString(info->dpy, win, info->textfontset, gc, 
				      highlightx_pos, texty,
				      highlight_char,
				      mblen(highlight_char,
					    strlen(highlight_char)));
		    }
		} else {
		    XSetFont(info->dpy,gc,(TextFont_Struct(info))->fid);
		    XDrawString(info->dpy, win, gc, highlightx_pos, texty, 
			    (char *)((char *)string_label+hlabel->position),1);
		}
#else
		XSetFont(info->dpy,gc,(TextFont_Struct(info))->fid);
		XDrawString(info->dpy, win, gc, highlightx_pos, texty, 
			    (char *)((char *)string_label+hlabel->position),1);
#endif /* OW_I18N */
		/* restore the font if 3d */
		if (info->three_d) {
		    XChangeGC(info->dpy,gc,GCFont,&values);
		}
	    }
	}
    }
    if (qualifier_label) {
	/* calc. max width of label w.r.t mark, accelerator 
	   string, or entire width. */
	if (mark_pos) {
	    text_width = mark_pos - q_pos;
	} else if (key_pos) {
	    text_width = key_pos - q_pos;
	} else {
	    text_width = x + width - q_pos;
	}
	olgx_draw_text(info,win,
#ifdef OW_I18N
		       qualifier_label,
#else
		       (char *)qualifier_label,
#endif /* OW_I18N */
		       q_pos,texty,text_width,state);
    }
    if (mark_type) {
	if (mark_type & OLGX_DIAMOND_MARK) {
	    olgx_draw_diamond_mark(info,win,
				   mark_pos,
				   texty,
				   state);
	} else if (mark_type & OLGX_MENU_MARK) {
	    int fill_color;
	    /* calculate the correct fill_color for menu mark */
	    if (info->three_d) {
		if (state & OLGX_INVOKED) {	/* invoked button */
		    fill_color = OLGX_BG2;
		} else if ((state & OLGX_DEFAULT)&&(state & OLGX_MENU_ITEM)) {
		    fill_color = OLGX_BG1;
		} else if (state & OLGX_MENU_ITEM && state & OLGX_BUSY) {
		    fill_color = OLGX_BG1;
		} else if (state & OLGX_MENU_ITEM) {	/* normal menu item */
		    fill_color = NONE;
		} else {		/* normal panel button */
		    fill_color = OLGX_BG1;
		}
		if (state & OLGX_BUSY) {
		/*
		 * This routine changes GC information on-the-fly, but it is
		 * assumed that OLGX_BUSY won't be called often, so it makes
		 * sense to use the same GC rather than one for each color.
		 */
		    if (!info->gc_rec[OLGX_BUSYGC])
			olgx_initialise_gcrec(info, OLGX_BUSYGC);
		    fill_color = OLGX_BUSYGC;
		}
		olgx_draw_menu_mark(info, win,
				    mark_pos,
				    texty-info->mm_height,
				    mark_type | state,
				    (fill_color != OLGX_BG2));
	    } else {
		olgx_draw_menu_mark(info, win,
				    mark_pos,
				    texty-info->mm_height,
				    mark_type | state,0);
	    }
	}
    }
    if (key_label) {
	olgx_draw_text(info,win,
#ifdef OW_I18N
		       key_label,
#else
		       (char *)key_label,
#endif /* OW_I18N */
		       key_pos,texty,
		       x+width-key_pos,state);
    }
}

void
olgx_draw_accel_label(info, win, x, y, width, height, 
		      main_label, m_pos,
		      qualifier_label, q_pos,
		      mark_type, mark_pos,
		      key_label, key_pos,
		      background_pixmap,
		      state)
    Graphics_info  *info;
    Window          win;
    int             x, y, width, height;  /* position and size of item */
    void	   *main_label;       /* item main label */
    int             m_pos;            /* position of main label */
    void	   *qualifier_label;  /* qualifier or modifier label */    
    int		    q_pos;	      /* x position of qualifier in pixels */
    int		    mark_type;        /* OLGX_DIAMOND_MARK, MENU_MARK, etc. */
    int		    mark_pos;         /* x position of mark sym. in pixels */
    void	   *key_label;        /* usually a single character */
    int		    key_pos;          /* x position of key char in pixels */
    void           *background_pixmap;/* unsupported right now! */
    int             state;            /* state of the actual object */
{
    /* olgx_draw_accel_label_internal() is a separate function because we
       might decide to put something special for text only */
    /* need to adjust y, because the label x,y is lower left corner, but
       olgx_draw_accel_label_internal() wants the upper left corner. */
    olgx_draw_accel_label_internal(info,win,y,x,
				   y - info->button_height + info->base_off,
				   width,height,
				   main_label,m_pos,
				   qualifier_label,q_pos,
				   mark_type,mark_pos,
				   key_label,key_pos,
				   state,
				   0);
}

void
olgx_draw_accel_button(info, win, x, y, width, height, 
		       main_label, m_pos,
		       qualifier_label, q_pos,
		       mark_type, mark_pos,
		       key_label, key_pos,
		       background_pixmap,
		       state)
    Graphics_info  *info;
    Window          win;
    int             x, y, width, height;  /* position and size of item */
    void	   *main_label;       /* item main label */
    int             m_pos;            /* position of main label */
    void	   *qualifier_label;  /* qualifier or modifier label */    
    int		    q_pos;	      /* x position of qualifier in pixels */
    int		    mark_type;        /* OLGX_DIAMOND_MARK, MENU_MARK, etc. */
    int		    mark_pos;         /* x position of mark sym. in pixels */
    void	   *key_label;        /* usually a single character */
    int		    key_pos;          /* x position of key char in pixels */
    void           *background_pixmap;/* unsupported right now! */
    int             state;            /* state of the actual object */
{
    /* don't want to duplicate code, so draw a button with a null label,
       no menu mark, and not inactive (will be done later) */
    olgx_draw_button(info,win,x,y,width,height,(void *)NULL,
		     state&~OLGX_MENU_MARK&~OLGX_INACTIVE);
    olgx_draw_accel_label_internal(info,win,
				   y + 
				   ((height) ? 
				    ((height+info->button_height)/2 + 1) : 
				    info->button_height) - 
				   info->base_off,
				   x+ButtonEndcap_Width(info),
				   y,
				   width-2*ButtonEndcap_Width(info),
				   height,
				   main_label,m_pos,
				   qualifier_label,q_pos,
				   mark_type,mark_pos,
				   key_label,key_pos,
				   state & ~OLGX_INACTIVE,
				   1);
    if (state & OLGX_INACTIVE) {
	olgx_stipple_rect(info, win, x, y, width, 
                          (height) ? height + 1 : Button_Height(info));
    }
}

void
olgx_draw_accel_choice_item(info, win, x, y, width, height, 
		       main_label, m_pos,
		       qualifier_label, q_pos,
		       mark_type, mark_pos,
		       key_label, key_pos,
		       background_pixmap,
		       state)
    Graphics_info  *info;
    Window          win;
    int             x, y, width, height;  /* position and size of item */
    void	   *main_label;       /* item main label */
    int             m_pos;            /* position of main label */
    void	   *qualifier_label;  /* qualifier or modifier label */    
    int		    q_pos;	      /* x position of qualifier in pixels */
    int		    mark_type;        /* OLGX_DIAMOND_MARK, MENU_MARK, etc. */
    int		    mark_pos;         /* x position of mark sym. in pixels */
    void	   *key_label;        /* usually a single character */
    int		    key_pos;          /* x position of key char in pixels */
    void           *background_pixmap;/* unsupported right now! */
    int             state;            /* state of the actual object */
{
    int             flag = 0;
    /* don't want to duplicate code, so draw a choice with a null label,
       no menu mark, and not inactive (will be done later) */
    olgx_draw_choice_item(info,win,x,y,width,height,NULL,
			  state&~OLGX_MENU_MARK&~OLGX_INACTIVE);
    
    /*
     * special case for choice invoked in drawing label where the
     * invoked state is changed to uninvoked and sent to the label
     * drawing routines
     */
    
    if (state & OLGX_INVOKED) {
	state ^= OLGX_INVOKED;
	flag = 1;
    }
    olgx_draw_accel_label_internal(info,win,
				   y + 
				   ((height) ? 
				    ((height+info->button_height)/2 + 1) : 
				    info->button_height) - 
				   info->base_off,
				   x + ((info->button_height > 20) ?
				       info->base_off + 2 : info->base_off),
				   y,
				   width - ((info->button_height > 20) ?
					info->base_off + 2 : info->base_off),
				   height,
				   main_label,m_pos,
				   qualifier_label,q_pos,
				   mark_type,mark_pos,
				   key_label,key_pos,
				   state & ~OLGX_INACTIVE,
				   1);
    if (state & OLGX_INACTIVE) {
	olgx_stipple_rect(info, win, x, y, width, height);
    }
}
