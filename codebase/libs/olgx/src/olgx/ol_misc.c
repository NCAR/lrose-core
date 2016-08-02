/*#ident "@(#)ol_misc.c	1.26 93/06/28 SMI" */

/*
 * Copyright 1990 Sun Microsystems
 */

/*
 * OPEN LOOK object drawing package Sun Microsystems, Inc.
 * 
 * ol_misc.c Window adornment module
 */

#include <stdio.h>
#include <X11/Xlib.h>

#include <olgx_private/olgx_impl.h>

void
olgx_draw_resize_corner(info, win, x, y, type, state)
    Graphics_info  *info;
    Window          win;
    int             x, y, type;
    int             state;
{
    int             top_color    = OLGX_WHITE; 
    int             fill_color   = OLGX_BG1;
    int             bottom_color = OLGX_BG3;
    XTextItem       item;
    char            string[2];

    item.nchars = 1;
    item.font = None;
    item.chars = string;
    item.delta = 0;

    if (info->three_d) {

	if (state & OLGX_INVOKED) {

	    top_color = OLGX_BG3;
	    fill_color = OLGX_BG2;
	    bottom_color = OLGX_WHITE;

	}
	/* draw the upper left part */

	string[0] = UL_RESIZE_UL + (3 * type);
	XDrawText(info->dpy, win, info->gc_rec[top_color]->gc, x, y, &item, 1);

	/* draw the lower right part */

	string[0] = UL_RESIZE_LR + (3 * type);
	XDrawText(info->dpy, win, info->gc_rec[bottom_color]->gc, x, y, &item, 1);

	string[0] = UL_RESIZE_FILL + (3 * type);
	XDrawText(info->dpy, win, info->gc_rec[fill_color]->gc, x, y, &item, 1);

    } else {

	string[0] = UL_RESIZE_OUTLINE + type;
	XDrawText(info->dpy, win, info->gc_rec[OLGX_BLACK]->gc, x, y, &item, 1);
	string[0] = UL_RESIZE_FILL + (3 * type);
	XDrawText(info->dpy, win, (state & OLGX_INVOKED) ? 
                                   info->gc_rec[OLGX_BLACK]->gc :
		                   info->gc_rec[OLGX_WHITE]->gc,
                  x, y, &item, 1);

    }
}

void
olgx_draw_pushpin(info, win, x, y, type)
    Graphics_info  *info;
    Window          win;
    int             x, y, type;
{
    int             top_color = OLGX_WHITE;
    int             fill_color = OLGX_BG2;
    int             bottom_color = OLGX_BG3;
    XTextItem       item;
    char            string[2];

    item.nchars = 1;
    item.font = None;
    item.chars = string;
    item.delta = 0;

    if (type & OLGX_ERASE)
	/*
	 * 29 and 17 is a hack,max width is 29 for allpoint sizes and 17 is
	 * the max height.should'nt rub anything closeby because of other
	 * open look specifications
	 */

	XClearArea(info->dpy, win, x, y, 29, 17,0);

    if (info->three_d) {

	/* draw the upper left part */

	if ((type & OLGX_PUSHPIN_OUT) && (type & OLGX_DEFAULT)) {

	    string[0] = PUSHPIN_OUT_DEFAULT_TOP;
	    XDrawText(info->dpy, win, info->gc_rec[top_color]->gc, x, y, &item, 1);
	    string[0] = PUSHPIN_OUT_DEFAULT_BOTTOM;
	    XDrawText(info->dpy, win, info->gc_rec[bottom_color]->gc, x, y, &item, 1);
	    string[0] = PUSHPIN_OUT_DEFAULT_MIDDLE;
	    XDrawText(info->dpy, win, info->gc_rec[fill_color]->gc, x, y, &item, 1);

	} else {

	    string[0] = (type & OLGX_PUSHPIN_IN) ? PUSHPIN_IN_TOP :
		PUSHPIN_OUT_TOP;
	    XDrawText(info->dpy, win, info->gc_rec[top_color]->gc, x, y, &item, 1);

	    /* draw the lower right part */

	    string[0] = (type & OLGX_PUSHPIN_IN) ? PUSHPIN_IN_BOTTOM :
		PUSHPIN_OUT_BOTTOM;
	    XDrawText(info->dpy, win, info->gc_rec[bottom_color]->gc, x, y, &item, 1);

	    string[0] = (type & OLGX_PUSHPIN_IN) ? PUSHPIN_IN_MIDDLE :
		PUSHPIN_OUT_MIDDLE;
	    XDrawText(info->dpy, win, info->gc_rec[fill_color]->gc, x, y, &item, 1);

	}

    } else {			/* 2d */

	if ((type & OLGX_DEFAULT) && (type & OLGX_PUSHPIN_OUT)) {

	    string[0] = OLG_MENU_DEFAULT_PIN_OUT;
	    XDrawText(info->dpy, win, info->gc_rec[OLGX_BLACK]->gc, x, y, &item, 1);

	} else {

	    string[0] = (type & OLGX_PUSHPIN_IN) ? OLG_MENU_PIN_IN :
		OLG_MENU_PIN_OUT;
	    XDrawText(info->dpy, win, info->gc_rec[OLGX_BLACK]->gc, x, y, &item, 1);

	}

    }

    if (type & OLGX_INACTIVE)
	olgx_stipple_rect(info, win, x, y, 29, 12);

}

/*
 * olgx_draw_check_box()
 * 
 * Render a 3D check box.
 */

void
olgx_draw_check_box(info, win, x, y, state)
    Graphics_info  *info;
    Window          win;
    int             x, y, state;
{
    XTextItem       item;
    int             top_color, bottom_color, fill_color;
    char            string[2];

    item.nchars = 1;
    item.font = None;
    item.chars = string;
    item.delta = 0;

    if (!(state & OLGX_CHECKED)) {
        XClearArea(info->dpy,win,x,y,CheckBox_Width(info),CheckBox_Height(info),0);
    }

    if (info->three_d) { /* 3D */

	if (state & OLGX_INVOKED) {

	    fill_color = OLGX_BG2;
	    top_color = OLGX_BG3;
	    bottom_color = OLGX_WHITE;

	} else {

	    fill_color = OLGX_BG1;
	    top_color = OLGX_WHITE;
	    bottom_color = OLGX_BG3;

	}

	/* draw the upper left part */

	string[0] = UNCHECKED_BOX_UL;
	XDrawText(info->dpy, win, info->gc_rec[top_color]->gc, x, y, &item, 1);

	/* draw the lower right part */

	string[0] = UNCHECKED_BOX_LR;
	XDrawText(info->dpy, win, info->gc_rec[bottom_color]->gc, x, y, &item, 1);

    } else {			/* 2d */

	string[0] = UNCHECKED_BOX_OUTLINE;
	XDrawText(info->dpy, win, info->gc_rec[OLGX_BLACK]->gc, x, y, &item, 1);
    }


    if (state & OLGX_CHECKED) {

	/* draw the check mark */

	string[0] = CHECK_MARK;
	XDrawText(info->dpy, win, info->gc_rec[OLGX_BLACK]->gc, x, y, &item, 1);

	/* fill the check box */

	string[0] = CHECKED_BOX_FILL;
	XDrawText(info->dpy, win, info->three_d ? info->gc_rec[fill_color]->gc :
		  info->gc_rec[OLGX_WHITE]->gc,
		  x, y, &item, 1);

    } else {

	/*
	 * Clear the outside of the check box. This removes any pieces of a
	 * check mark that stick outside the check box proper.
	 */

/*
	string[0] = CHECK_BOX_CLEAR_FILL;
	XDrawText(info->dpy, win, info->three_d ? info->gc_rec[OLGX_BG1]->gc :
		  info->gc_rec[OLGX_WHITE]->gc,
		  x, y, &item, 1);
*/
	/*
	 * fill the check box.
	 */

	string[0] = UNCHECKED_BOX_FILL;
	XDrawText(info->dpy, win, info->three_d ? info->gc_rec[fill_color]->gc :
		  info->gc_rec[OLGX_WHITE]->gc,
		  x, y, &item, 1);

    }

    if (state & OLGX_INACTIVE)
	olgx_stipple_rect(info, win, x, y, CheckBox_Width(info),
			  CheckBox_Height(info));

}

/*
 * Keep this function here. It's only one line, but later on it will handle
 * the distinction between 2D and 3D text ledges. They are different, and the
 * application shouldn't handle this.
 */

void
olgx_draw_text_ledge(info, win, x, y, width)
    Graphics_info  *info;
    Window          win;
    int             x, y, width;
{

    if (info->three_d)
	olgx_draw_box(info, win, x, y, width, 2, OLGX_NORMAL, 1);
    else
      XDrawLine(info->dpy,win, info->gc_rec[OLGX_BLACK]->gc, x, y, x + width - 1, y);

}
