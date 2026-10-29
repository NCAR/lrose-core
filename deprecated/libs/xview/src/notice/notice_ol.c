#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)notice_ol.c 1.15 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1990 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE 
 *	file for terms of the license.
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <xview_private/noticeimpl.h>
#include <olgx/olgx.h>
#include <xview_private/draw_impl.h>
#include <xview_private/pw_impl.h>
#include <xview_private/windowimpl.h>
#include <xview/font.h>
#include <xview/frame.h>
#include <xview/panel.h>
#include <xview/win_input.h>
#include <xview/cms.h>
#include <xview/server.h>

/*
 * notice_ol.c
 * Routines for drawing notice borders - open look compliance.
 * The table of values for notice dimensions is also here.
 */

Xv_private void		win_change_property();

/*
 * Public routines - these routines are private to the notice pkg
 */
Pkg_private int		notice_subframe_layout();
Pkg_private int		notice_determine_font();
Pkg_private void	notice_draw_borders();
Pkg_private int		notice_center();
Pkg_private void	notice_get_notice_size();
Pkg_private void	notice_layout();
Pkg_private void	notice_drawbox();
Pkg_private void	notice_do_buttons();

/*
 * Private/static routines - these routines are private to this file
 */
static int		notice_position_items();
static int		notice_offset_from_baseline();

/*
 * OPEN LOOK geometry
 * Numbers represent pixels
 * Should use points and convert to pixels.
 */

int			notice_dimensions_init = 1;
Notice_config	Notice_dimensions[] = {
    /* NOTICE_SMALL */
    332, 30, 16, 36, 32, 10, 2, 2, 3, 3, 8,

    /* NOTICE_MEDIUM */
    390, 36, 20, 42, 36, 12, 2, 2, 3, 3, 10,

    /* NOTICE_LARGE */
    448, 42, 24, 50, 40, 14, 2, 2, 3, 3, 12,

    /* NOTICE_EXTRALARGE */
    596, 54, 32, 64, 48, 19, 2, 2, 3, 3, 16,
};

/*
 ****************************************
 * Routines for non-screen-locking notice
 ****************************************
 */

/*
 * Routine to create sub_frame and panels
 */
Pkg_private int
notice_subframe_layout(notice, do_msg, do_butt)
Notice_info	*notice;
Bool		do_msg;
Bool		do_butt;
{
    if (notice->lock_screen)  {
	return 0;
    }

    /*
     * Create base frame and panel for notice
     */
    notice_create_base(notice);

    /*
     **********************************************************
     **********************************************************
     * Position panel items within panel according to OPEN LOOK
     **********************************************************
     **********************************************************
     */
    (void)notice_position_items(notice, do_msg, do_butt);

    /*
     * Calculations to center the sub_frame within the parent
     */
    notice_center(notice);

    /*
     * Set _OL_DFLT_BTN property on window if notice.jumpCursor resource set
     */
    if (notice_jump_cursor)  {
	Panel_item	panel_default_item;
	Rect		*rect;
	int		button_x, button_y;
	int		data[6];
	
	panel_default_item = (Panel_item)xv_get(notice->panel, PANEL_DEFAULT_ITEM);

	/*
	 * Do only if default item exists
	 */
	if (panel_default_item)  {
	    rect = (Rect *)xv_get(panel_default_item, PANEL_ITEM_RECT);

	    /*
	     * Check for Rect returned to avoid seg fault
	     */
	    if (rect)  {
	        win_translate_xy(notice->panel, notice->sub_frame, 
			rect->r_left, rect->r_top, &button_x, &button_y);
	
	        data[0] = button_x + rect->r_width / 2;
	        data[1] = button_y + rect->r_height / 2;
	        data[2] = button_x;
	        data[3] = button_y;
	        data[4] = rect->r_width;
	        data[5] = rect->r_height;

		/*
		 * Set property on notice frame
		 */
	        win_change_property(notice->sub_frame, 
		    SERVER_WM_DEFAULT_BUTTON, XA_INTEGER, 32,  data, 6);
            }
        }
    }
    else  {
        /*
	 * notice.jumpCursor NOT set.
         * Set zero length property on notice frame so that the ptr is not warped.
         */
        win_change_property(notice->sub_frame, 
                    SERVER_WM_DEFAULT_BUTTON, XA_INTEGER, 32,  NULL, NULL);
    }

    notice->need_layout = 0;
}

/*
 * Routine to create and arrange panel items
 */
static int
notice_position_items(notice, do_msg, do_butt)
Notice_info	*notice;
Bool		do_msg;
Bool		do_butt;
{
    Panel	panel = notice->panel;
    Rect	**msg_rect_list;
    Rect	**button_rect_list;
    int		i;
    int		panelWidth = 0, panelHeight = 0;
    int		maxButHeight = 0;
    int		totalButWidth = 0;
    int		panelItemX, panelItemY;
    int		numMsgStr = notice->number_of_strs;
    int		numButtons = notice->number_of_buttons;
    struct notice_buttons	*curButton = notice->button_info;
    struct notice_msgs		*curMsg = notice->msg_info;

    msg_rect_list = (Rect **)malloc( numMsgStr * (sizeof(Rect *)) );
    button_rect_list = (Rect **)malloc( numButtons * (sizeof(Rect *)) );

    /*
     * Set the message string Panel Items
     */
    /*
     * Check if any first
     */
    if (!curMsg)  {
	if (numMsgStr)  {
	    numMsgStr = 0;
	}
    }

    for(i=0; i < numMsgStr; curMsg = curMsg->next, ++i)  {
	if (do_msg)  {
	    if (curMsg->panel_item)  {
                xv_set(curMsg->panel_item, 
#ifdef OW_I18N
			PANEL_LABEL_STRING_WCS, curMsg->string,
#else
			PANEL_LABEL_STRING, curMsg->string,
#endif /* OW_I18N */
			NULL);
	    }
	    else  {
                curMsg->panel_item = xv_create(notice->panel, 
			PANEL_MESSAGE,
#ifdef OW_I18N
			PANEL_LABEL_STRING_WCS, curMsg->string,
#else
			PANEL_LABEL_STRING, curMsg->string, 
#endif /* OW_I18N */
			XV_HELP_DATA, "xview:notice",
			NULL);
	    }
	}

	/*
	 * Get the created panel item's rect info
	 */
	msg_rect_list[i] = (Rect *)xv_get(curMsg->panel_item, PANEL_ITEM_RECT);

	/*
	 * Update panel width/height
	 */
	panelWidth = MAX(panelWidth, msg_rect_list[i]->r_width);
	panelHeight += msg_rect_list[i]->r_height;

	/* 
	 * vertical distance is vertical gap
	 * - add only if not last message
	 */
	if (i < (numMsgStr - 1))  {
	    panelHeight += MSG_VERT_GAP(notice->scale);
	}
    }

    /*
     * Add margins to panel width
     */
    panelWidth += 2 * HORIZ_MSG_MARGIN(notice->scale);

    /*
     * Set button Panel Items
     */
    for(i = 0; i < numButtons; ++i, curButton = curButton->next)  {
	/*
	 * Create buttons
	 */
	if (do_butt)  {
            if (curButton->panel_item)  {
                xv_set(curButton->panel_item,
#ifdef OW_I18N
                    PANEL_LABEL_STRING_WCS,	curButton->string,
#else
                    PANEL_LABEL_STRING,	curButton->string,
#endif /* OW_I18N */
                    PANEL_NOTIFY_PROC,	notice_button_panel_proc,
	            XV_KEY_DATA, 	notice_context_key, notice,
                    NULL);
	    }
	    else  {
                curButton->panel_item = xv_create(notice->panel,
		    PANEL_BUTTON,
#ifdef OW_I18N
                    PANEL_LABEL_STRING_WCS,	curButton->string,
#else
                    PANEL_LABEL_STRING,	curButton->string,
#endif /* OW_I18N */
                    PANEL_NOTIFY_PROC,	notice_button_panel_proc,
	            XV_KEY_DATA, 	notice_context_key, notice,
		    XV_HELP_DATA, "xview:notice",
                    NULL);
	    }

	    if (curButton->is_yes)  {
		xv_set(notice->panel, 
			PANEL_DEFAULT_ITEM, curButton->panel_item, 
			NULL);
	    }

	}

	/*
	 * Get rect info
	 */
        button_rect_list[i] = (Rect *)xv_get(curButton->panel_item, 
						PANEL_ITEM_RECT);

	/*
	 * Update total button width and max button height
	 */
        totalButWidth += button_rect_list[i]->r_width;
        maxButHeight = MAX(maxButHeight, button_rect_list[i]->r_height);
    }

    /*
     * Add button horizontal gap(s) to total width
     */
    totalButWidth += ((numButtons - 1) * BUT_HORIZ_GAP(notice->scale));

    /*
     * The button portion height is max of the OPENLOOK value
     * and the actual button height
     */
    BUT_PORTION_HEIGHT(notice->scale) = MAX(BUT_PORTION_HEIGHT(notice->scale), 
						maxButHeight);

    /*
     * Add to panel height the top/bottom margins and the height of
     * buttons.
     */
    panelHeight += (2 * VERT_MSG_MARGIN(notice->scale)) + 
			BUT_PORTION_HEIGHT(notice->scale);

    /*
     * Panel width is max of current panel width and the width of the buttons
     */
    if (panelWidth < (totalButWidth + (2 * HORIZ_MSG_MARGIN(notice->scale))))  {
	panelWidth = totalButWidth + (2 * HORIZ_MSG_MARGIN(notice->scale));
    }

    /*
     * Set panel width/height
     */
    xv_set(panel, 
	XV_WIDTH, panelWidth, 
	XV_HEIGHT, panelHeight, 
	NULL);

    /*
     * Reset button/msg pointers
     */
    curButton = notice->button_info;
    curMsg = notice->msg_info;

    /*
     * Position messages
     * Start at top and work downwards
     */
    panelItemY = VERT_MSG_MARGIN(notice->scale);
    for(i=0; i < numMsgStr; ++i, curMsg = curMsg->next)  {
	/*
	 * Center messages
	 */
	panelItemX = (panelWidth - msg_rect_list[i]->r_width)/2;
	/*
	 * Set panel item coordinates
	 */
	xv_set(curMsg->panel_item, 
		XV_X, panelItemX, 
		XV_Y, panelItemY, 
		NULL);

	/*
	 * Add message height to vertical position
	 */
	panelItemY += msg_rect_list[i]->r_height;

	/* 
	 * vertical distance is vertical gap
	 * - add only if not last message
	 */
	if (i < (numMsgStr - 1))  {
	    panelItemY += MSG_VERT_GAP(notice->scale);
	}

    }

    /*
     * Position buttons
     * Center buttons in button area vertically and horizontally
     */
    panelItemX = (panelWidth - totalButWidth)/2;
    panelItemY += VERT_MSG_MARGIN(notice->scale) + 
		((BUT_PORTION_HEIGHT(notice->scale) - maxButHeight)/2);

    for(i=0; i < numButtons; ++i, curButton = curButton->next)  {
	/*
	 * Set button coordinates
	 */
	xv_set(curButton->panel_item, 
		XV_X, panelItemX, 
		XV_Y, panelItemY, 
		NULL);

	/*
	 * horizontal distance is button width + button gap
	 */
	panelItemX += button_rect_list[i]->r_width + BUT_HORIZ_GAP(notice->scale);

    }

    /*
     * Free rect list
     */
    free((char *)msg_rect_list);
    free((char *)button_rect_list);
}


/*
 * Center notice within parent, if any
 */
Pkg_private int
notice_center(notice)
Notice_info	*notice;
{
    Display	*dpy;
    Xv_Screen	screen;
    int		screen_num;
    Frame	parent = notice->owner_window;
    Frame	sub_frame = notice->sub_frame;
    Panel	panel = notice->panel;
    int		xDiff, yDiff, pWidth, pHeight, 
		cWidth, cHeight, cX, cY, pX, pY;

    if (!parent)  {
	return(XV_ERROR);
    }

    if (!sub_frame)  {
	return(XV_ERROR);
    }

    dpy = (Display *)xv_get(sub_frame, XV_DISPLAY, NULL);
    screen = (Xv_Screen)xv_get(sub_frame, XV_SCREEN, NULL);
    screen_num = xv_get(screen, SCREEN_NUMBER, NULL);

    /*
     * Get subframe width, height
     */
    cWidth = xv_get(panel, XV_WIDTH) + 
		PANE_NOTICE_DIFF(NOTICE_NOT_TOPLEVEL, notice->scale);
    cHeight = xv_get(panel, XV_HEIGHT) + 
		PANE_NOTICE_DIFF(NOTICE_NOT_TOPLEVEL, notice->scale);

    if (xv_get(parent, FRAME_CLOSED))  {
        Xv_window	root_window;
	Rect		*mouse_position;

        /*
         * Calculations to center the sub_frame where the pointer is
	 * currently located
         */

	/*
	 * Get root window, and pointer position relative to root
	 * window
	 */
        root_window = (Xv_window)xv_get(sub_frame, XV_ROOT);
	mouse_position = (Rect *)xv_get(root_window, WIN_MOUSE_XY);

	cX = mouse_position->r_left - (cWidth/2);
	cY = mouse_position->r_top - (cHeight/2);
    }
    else  {
	Xv_Drawable_info	*p_info;
	XID			dummy;

        /*
         * Calculations to center the sub_frame within the parent
         */

        pWidth = xv_get(parent, XV_WIDTH);
        pHeight = xv_get(parent, XV_HEIGHT);
        DRAWABLE_INFO_MACRO(parent, p_info);
        XTranslateCoordinates(dpy, xv_xid(p_info), 
        		      (XID)xv_get(xv_root(p_info), XV_XID), 
        		      0, 0, &pX, &pY, &dummy);

        xDiff = (pWidth - cWidth)/2;
        yDiff = (pHeight - cHeight)/2;

        /*
         * Centered positions of sub frame
         */
        cX = pX + xDiff;
        cY = pY + yDiff;
    }

    /*
     * Check if sub frame is off screen
     */

    /*
     * Check x coordinates first
     * The checks for wider than DisplayWidth and less than zero
     * are not exclusive because by correcting for wider than
     * DisplayWidth, the x position can be set to < 0
     */
    if ((cX + cWidth) > DisplayWidth(dpy, screen_num))  {
        cX = DisplayWidth(dpy, screen_num) - cWidth;
    }

    if (cX < 0)  {
        cX = 0;
    }

    /*
     * Check y coordinate
     */
    if ((cY + cHeight) > DisplayHeight(dpy, screen_num))  {
        cY = DisplayHeight(dpy, screen_num) - cHeight;
    }

    if (cY < 0)  {
        cY = 0;
    }

    xv_set(sub_frame,
		XV_X, cX,
		XV_Y, cY,
		XV_WIDTH, cWidth,
		XV_HEIGHT, cHeight,
		NULL);

    /*
     * Location of the panel itself is:
     * notice border width + pane-border distance + pane border width
     */
    xv_set(panel, 
	XV_X, PANE_XY(NOTICE_NOT_TOPLEVEL, notice->scale),
	XV_Y, PANE_XY(NOTICE_NOT_TOPLEVEL, notice->scale), 
	NULL);

    return(XV_OK);

}


/*
 ************************************
 * Routines for screen-locking notice
 ************************************
 */
Pkg_private void
notice_get_notice_size(notice, rect, buttons_width)
    register		notice_handle notice;
    struct rect		*rect;
    int			*buttons_width;
{
    Graphics_info	*ginfo = notice->ginfo;
    notice_msgs_handle	curMsg = notice->msg_info;
    notice_buttons_handle curButton = notice->button_info;
    int		notice_width = 0, notice_height = 0;
    int		maxButHeight = 0;
    int		totalButWidth = 0;
    int         chrht;
    Xv_Font	this_font = (Xv_Font)(notice->notice_font);
    int		i;

    /*
     * get character width and height
     */
    chrht = xv_get(this_font, FONT_DEFAULT_CHAR_HEIGHT);

    /*
     * Scan thru messages
     */
    while (curMsg)  {
        int         str_width = 0;

	/*
	 * Get string width
	 */
        str_width = notice_text_width(this_font, curMsg->string);

	/*
	 * Maintain MAX width of all strings
	 */
	notice_width = MAX(notice_width, str_width);

	/*
	 * For each message string, add height
	 */
	notice_height += chrht;

	/*
	 * Go on to next message string
	 */
        curMsg = curMsg->next;

	/*
	 * Don't add message gaps for last message item
	 */
	if (curMsg)  {
            notice_height += MSG_VERT_GAP(notice->scale);
	}
    }

    /*
     * Add margins to current notice width
     */
    notice_width += (2 * HORIZ_MSG_MARGIN(notice->scale));

    i = 0;

    /*
     * Scan thru buttons
     */
    while (curButton) {
	int             this_buttons_width = 0;

	/*
	 * get button width
	 */
	this_buttons_width = notice_button_width(this_font, ginfo, 
					curButton);
	/*
	 * Increment width of total buttons
	 */
	totalButWidth += this_buttons_width;

	/*
	 * Increment button ct.
	 */
	i++;

	/*
	 * Go on to next button
	 */
	curButton = curButton->next;
    }

    /*
     * All buttons have same height, so take the first one
     */
    maxButHeight = notice->button_info->button_rect.r_height;

    /*
     * Add horizontal gap and margins to total button width
     */
    totalButWidth += (i-1) * BUT_HORIZ_GAP(notice->scale);
    
    /*
     * The button portion height is max of the OPENLOOK value
     * and the actual button height
     */
    BUT_PORTION_HEIGHT(notice->scale) = MAX(BUT_PORTION_HEIGHT(notice->scale), 
						maxButHeight);

    /*
     * Add to panel height the top/bottom margins and the height of
     * buttons.
     */
    notice_height += (2 * VERT_MSG_MARGIN(notice->scale)) + BUT_PORTION_HEIGHT(notice->scale);

    /*
     * notice width is max of strings width(current notice width) and
     * total width of buttons
     */
    notice_width = MAX(notice_width, 
		(totalButWidth + (2 * HORIZ_MSG_MARGIN(notice->scale))) );

    *buttons_width = totalButWidth;

    rect->r_top = 0;
    rect->r_left = 0;
    rect->r_width = notice_width;
    rect->r_height = notice_height;
}

Pkg_private void
notice_layout(notice, rect, totalButWidth)
notice_handle	notice;
struct rect	*rect;
int		totalButWidth;
{
    int				x, y;
    int				paneWidth = rect->r_width;
    notice_msgs_handle		curMsg;
    Xv_Window			window = notice->fullscreen_window;
    Xv_Font			this_font = (Xv_Font)(notice->notice_font);
    /* Additional vars for replacing pw_ calls */
    Xv_Drawable_info		*info;
    Display			*display;
    Drawable			d;
    unsigned long   		value_mask;
    XGCValues       		val;
    GC				gc;
    int				chrht;
    int				ascent = notice_offset_from_baseline(this_font);
#ifdef  OW_I18N
    XFontSet        font_set;
#endif /* OW_I18N */
    /*
     * Set information needed by Xlib calls
     */
    DRAWABLE_INFO_MACRO(window, info);
    display = xv_display(info);
    d = xv_xid(info);

    /*
     * later, consider centering here
     */
    chrht = xv_get(this_font, FONT_DEFAULT_CHAR_HEIGHT);

    y = rect->r_top + VERT_MSG_MARGIN(notice->scale);

    if (notice->msg_info) {
        XID             font;

	/*
	 * Set GC needed by XDrawImageString()
	 */
        gc = (GC)xv_find_proper_gc(display, info, PW_TEXT);

#ifdef OW_I18N
        font_set = (XFontSet)xv_get( this_font , FONT_SET_ID );
#endif /* OW_I18N */
	font = xv_get(this_font, XV_XID);

        gc = (GC)xv_find_proper_gc(display, info, PW_VECTOR);
        val.background = xv_bg(info);
        val.foreground = xv_fg(info);
        val.font = font;
        value_mask = GCForeground | GCBackground | GCFont;
        XChangeGC(display, gc, value_mask, &val);
    }

    curMsg = notice->msg_info;
    while(curMsg) {
        int	str_width, len;
	CHAR	*str;

        str = curMsg->string;

        if (len == STRLEN(str)) {
            str_width = notice_text_width(this_font, (CHAR *)str);

            x = (rect->r_left + (paneWidth - str_width)/2);

#ifdef OW_I18N
            XwcDrawImageString(display, d, font_set, gc,
                        x,
                        y + ascent,
                        str, len);
#else
            XDrawImageString(display, d, gc, 
                        x,
                        y + ascent, 
                        str, len);
#endif /* OW_I18N */
        }

	/*
	 * Add character height to vertical position
	 */
        y += chrht;

	curMsg = curMsg->next;

	/*
	 * Don't add message gaps for last message item
	 */
	if (curMsg)  {
            y += MSG_VERT_GAP(notice->scale);
	}
    }

    notice_do_buttons(notice, rect, y, NULL, totalButWidth);
}


Pkg_private void
notice_do_buttons(notice, rect, starty, this_button_only, totalButWidth)
notice_handle	notice;
struct rect	*rect;
int		starty;
notice_buttons_handle	this_button_only;
int		totalButWidth;
{
    Graphics_info		*ginfo = notice->ginfo;
    int				three_d = notice->three_d;
    int				x, y;
    int				paneWidth = rect->r_width;
    notice_buttons_handle	curButton;
    notice_msgs_handle		curMsg;
    Xv_Window			window = notice->fullscreen_window;
    Xv_Font			this_font = (Xv_Font)(notice->notice_font);
    /* Additional vars for replacing pw_ calls */
    int				chrht;

    if (starty < 0)  {
        /*
         * later, consider centering here
         */
        chrht = xv_get(this_font, FONT_DEFAULT_CHAR_HEIGHT);

        y = rect->r_top + VERT_MSG_MARGIN(notice->scale);

        curMsg = notice->msg_info;
        while(curMsg)  {
	    /*
	     * Add character height to vertical position
	     */
            y += chrht;

	    curMsg = curMsg->next;

	    /*
	     * Don't add message gaps for last message item
	     */
	    if (curMsg)  {
                y += MSG_VERT_GAP(notice->scale);
	    }
        }
    }
    else  {
	y = starty;
    }

    curButton = notice->button_info;

    x = rect->r_left + (paneWidth - totalButWidth)/2;
    y += VERT_MSG_MARGIN(notice->scale) + 
	((BUT_PORTION_HEIGHT(notice->scale) - curButton->button_rect.r_height)/2);

    while(curButton) {
	if (this_button_only)  {
	    if (this_button_only == curButton)  {
                notice_build_button(window, x, y, curButton, ginfo, three_d);
		curButton = NULL;
	    }
	    else  {
	        x += curButton->button_rect.r_width + BUT_HORIZ_GAP(notice->scale);
    
	        curButton = curButton->next;
	    }
	}
	else  {
            notice_build_button(window, x, y, curButton, ginfo, three_d);
	    x += curButton->button_rect.r_width + BUT_HORIZ_GAP(notice->scale);

	    curButton = curButton->next;
	} 

    }

}


Pkg_private void
notice_drawbox(pw, rectp, quadrant, leftoff, topoff)
Xv_Window		pw;
struct rect		*rectp;
int			quadrant;
int			leftoff;
int			topoff;
{
    notice_handle	notice;
    Display		*display;
    Drawable		d;
    GC			gc;
    GC			fill_gc;
    XGCValues		gc_val;
    XPoint		points[4];
    Xv_Drawable_info	*info;
    Cms			cms;
    struct rect		rect;
    unsigned long	bg2;
    unsigned long	bg3;

    /*
     * Set information neede by Xlib calls
     */
    DRAWABLE_INFO_MACRO(pw, info);
    display = xv_display(info);
    d = xv_xid(info);

    notice = (notice_handle)xv_get(pw, XV_KEY_DATA, notice_context_key);

    rect = *rectp;

    /*
     * Determine which colors to use
     */
    cms = xv_get(pw, WIN_CMS);
    bg2 = xv_get(cms, CMS_PIXEL, 1);
    bg3 = xv_get(cms, CMS_PIXEL, 2);

    /*
     * Most of the code here is straight out from libxvin/pw/pw_plygon2.c
     * Using the server images created, get/set the gc to be used by XFillPolygon
     * The reason, the GCs are NOT get/set separately, as are the server images,
     * is because xv_find_proper_gc may return the same GC when called the 2nd
     * time. 
     */
    fill_gc = (GC)xv_find_proper_gc(display, info, PW_POLYGON2);
    switch (quadrant) {
      case 0:			/* break down and to right */
	/*
	 * 1st triangle of the shadow
	 */
	points[0].x = 0;
	points[0].y = 0;
	points[1].x = rect.r_width + PANE_NOTICE_DIFF(NOTICE_IS_TOPLEVEL, notice->scale) + leftoff;
	points[1].y = topoff;
	points[2].x = leftoff;
	points[2].y = topoff;

	gc_val.fill_style = FillSolid;
	gc_val.foreground = bg2;
	XChangeGC(display, fill_gc, GCForeground | GCFillStyle, &gc_val);
	XFillPolygon(display, d, fill_gc, points, 3, Complex, CoordModeOrigin);

	/*
	 * 2nd triangle of the shadow
	 */
	points[1].x = leftoff;
	points[1].y = topoff;
	points[2].x = leftoff;
	points[2].y = rect.r_height + PANE_NOTICE_DIFF(NOTICE_IS_TOPLEVEL, notice->scale) + topoff;

	gc_val.fill_style = FillSolid;
	gc_val.foreground = bg3;
	XChangeGC(display, fill_gc, GCForeground | GCFillStyle, &gc_val);
	XFillPolygon(display, d, fill_gc, points, 3, Complex, CoordModeOrigin);

	break;
      case 1:			/* break down and to left */
	points[0].x = rect.r_width + PANE_NOTICE_DIFF(NOTICE_IS_TOPLEVEL, notice->scale) + leftoff;
	points[0].y = 0;
	points[1].x = rect.r_width + PANE_NOTICE_DIFF(NOTICE_IS_TOPLEVEL, notice->scale);
	points[1].y = rect.r_height + PANE_NOTICE_DIFF(NOTICE_IS_TOPLEVEL, notice->scale) + topoff;
	points[2].x = rect.r_width + PANE_NOTICE_DIFF(NOTICE_IS_TOPLEVEL, notice->scale);
	points[2].y = topoff;

	gc_val.fill_style = FillSolid;
	gc_val.foreground = bg3;
	XChangeGC(display, fill_gc, GCForeground | GCFillStyle, &gc_val);
	XFillPolygon(display, d, fill_gc, points, 3, Complex, CoordModeOrigin);

	points[1].x = 0;
	points[1].y = topoff;

	gc_val.fill_style = FillSolid;
	gc_val.foreground = bg2;
	XChangeGC(display, fill_gc, GCForeground | GCFillStyle, &gc_val);
	XFillPolygon(display, d, fill_gc, points, 3, Complex, CoordModeOrigin);

	break;
      case 2:			/* break up and to left */
	points[0].x = rect.r_width + PANE_NOTICE_DIFF(NOTICE_IS_TOPLEVEL, notice->scale) + leftoff;
	points[0].y = rect.r_height + PANE_NOTICE_DIFF(NOTICE_IS_TOPLEVEL, notice->scale) + topoff;
	points[1].x = rect.r_width + PANE_NOTICE_DIFF(NOTICE_IS_TOPLEVEL, notice->scale);
	points[1].y = 0;
	points[2].x = rect.r_width + PANE_NOTICE_DIFF(NOTICE_IS_TOPLEVEL, notice->scale);
	points[2].y = rect.r_height + PANE_NOTICE_DIFF(NOTICE_IS_TOPLEVEL, notice->scale);

	gc_val.fill_style = FillSolid;
	gc_val.foreground = bg2;
	XChangeGC(display, fill_gc, GCForeground | GCFillStyle, &gc_val);
	XFillPolygon(display, d, fill_gc, points, 3, Complex, CoordModeOrigin);

	points[1].x = 0;
	points[1].y = rect.r_height + PANE_NOTICE_DIFF(NOTICE_IS_TOPLEVEL, notice->scale);

	gc_val.fill_style = FillSolid;
	gc_val.foreground = bg3;
	XChangeGC(display, fill_gc, GCForeground | GCFillStyle, &gc_val);
	XFillPolygon(display, d, fill_gc, points, 3, Complex, CoordModeOrigin);

	break;
      case 3:			/* break up and to right */
	points[0].x = 0;
	points[0].y = rect.r_height + PANE_NOTICE_DIFF(NOTICE_IS_TOPLEVEL, notice->scale) + topoff;
	points[1].x = rect.r_width + PANE_NOTICE_DIFF(NOTICE_IS_TOPLEVEL, notice->scale) + leftoff;
	points[1].y = rect.r_height + PANE_NOTICE_DIFF(NOTICE_IS_TOPLEVEL, notice->scale);
	points[2].x = leftoff;
	points[2].y = rect.r_height + PANE_NOTICE_DIFF(NOTICE_IS_TOPLEVEL, notice->scale);

	gc_val.fill_style = FillSolid;
	gc_val.foreground = bg3;
	XChangeGC(display, fill_gc, GCForeground | GCFillStyle, &gc_val);
	XFillPolygon(display, d, fill_gc, points, 3, Complex, CoordModeOrigin);

	points[1].x = leftoff;
	points[1].y = 0;

	gc_val.fill_style = FillSolid;
	gc_val.foreground = bg2;
	XChangeGC(display, fill_gc, GCForeground | GCFillStyle, &gc_val);
	XFillPolygon(display, d, fill_gc, points, 3, Complex, CoordModeOrigin);

	break;
    }

    /*
     * draw box
     */

    /*
     * Clear the box
     * Replace pw_ call with Xlib call
     * Get/set GC for XFillRectangle call
     */
    gc = xv_find_proper_gc(display, info, PW_ROP_NULL_SRC);
    xv_set_gc_op(display, info, gc, PIX_CLR, XV_USE_OP_FG, XV_DEFAULT_FG_BG);
    XFillRectangle(display, d, gc, 
			rect.r_left - PANE_XY(NOTICE_IS_TOPLEVEL, notice->scale),
			rect.r_top - PANE_XY(NOTICE_IS_TOPLEVEL, notice->scale),
			rect.r_width + PANE_NOTICE_DIFF(NOTICE_IS_TOPLEVEL, notice->scale),
			rect.r_height + PANE_NOTICE_DIFF(NOTICE_IS_TOPLEVEL, notice->scale));

    notice_draw_borders(pw, 
		rect.r_left - PANE_XY(NOTICE_IS_TOPLEVEL, notice->scale), 
		rect.r_top - PANE_XY(NOTICE_IS_TOPLEVEL, notice->scale), 
		rect.r_width + PANE_NOTICE_DIFF(NOTICE_IS_TOPLEVEL, notice->scale), 
		rect.r_height + PANE_NOTICE_DIFF(NOTICE_IS_TOPLEVEL, notice->scale),
		NOTICE_IS_TOPLEVEL);
}


/*
 *************************************************************
 * Routines used by BOTH screen and non screen locking notices
 *************************************************************
 */

/*
 * Determines which font to use for notice window.
 * OPEN LOOK specifies it to be one scale larger than
 * the client window font.
 * 
 * The NOTICE pkg however tries to use the font of the client 
 * window. If that is not available, a default font is used.
 * Rescaling of fonts is not done because we dont want to depend
 * on the ability of the server to rescale fonts, or on the 
 * presence of fonts in the sizes/scales we want.
 */
#ifdef  OW_I18N

Pkg_private int
notice_determine_font(client_window, notice)
Xv_Window       client_window;
notice_handle   notice;
{
    Xv_Font     font = NULL;

    if (client_window)
        font = xv_get(client_window, XV_FONT);

    if (font == NULL)
        font = (Xv_Font) xv_find(NULL, FONT,
                                FONT_FAMILY, FONT_FAMILY_DEFAULT,
                                FONT_STYLE, FONT_STYLE_DEFAULT,
                                FONT_SCALE, FONT_SCALE_DEFAULT,
                                NULL);

    if (font == NULL) {
        xv_error(XV_ZERO,
            ERROR_STRING,
                XV_MSG("Unable to find \"fixed\" font. (Notice package)"),
        NULL);
        return(XV_ERROR);
    }

    notice->notice_font = font;
    return(XV_OK);
}

#else /*OW_I18N*/

Pkg_private int
notice_determine_font(client_window, notice)
Xv_Window       client_window;
notice_handle 	notice;
{
    Xv_Font         		client_font = (Xv_Font)NULL;
    Xv_Font			default_font = (Xv_Font)NULL;

    /*
     * Get client window font
     */
    if (client_window) {
	client_font = xv_get(client_window, XV_FONT);
	/*
	 * Should try to rescale here
	 */
    }

    if (!client_font) {
	/*
	 * If cannot find client window font, try to find
	 * default font
	 */
        default_font = (Xv_Font) xv_find(client_window, FONT,
	                            FONT_FAMILY, FONT_FAMILY_DEFAULT,
	                            FONT_STYLE, FONT_STYLE_DEFAULT,
	                            FONT_SCALE, FONT_SCALE_DEFAULT,
	                            NULL);

	if (!default_font)  {
	    /*
	     * If cannot find default font, find fixed font
	     */
	    default_font = (Xv_Font) xv_find(client_window, FONT,
	                                FONT_NAME, "fixed",
	                                NULL);

	    /*
	     * If all the above fails, return error code
	     */
	    if (!default_font) {
	        xv_error(XV_ZERO,
	                ERROR_STRING,
	        XV_MSG("Unable to find \"fixed\" font."),
	                 ERROR_PKG, NOTICE,
	                NULL);
	        return (XV_ERROR);
	    }
	}

    }

    notice->notice_font = client_font ? client_font : default_font;

    return(XV_OK);

}

#endif  /* OW_I18N */

/*
 * notice_draw_borders
 * Draws the notice window border as well as the notice pane border.
 * The x, y, width, and height parameters passed in correspond to the
 * position and dimensions of the notice window.
 */
Pkg_private void
notice_draw_borders(window, x, y, width, height, is_toplevel_window)
Xv_window	window;
int		x;
int		y;
int		width;
int		height;
int		is_toplevel_window;
{
    Display		*display;
    XSegment		seg[5];
    unsigned long   	value_mask;
    XGCValues       	val;
    GC			gc;
    unsigned long	white;
    unsigned long	bg3;
    unsigned long	fg;
    Window		w;
    Cms			cms;
    Xv_Drawable_info	*info;
    notice_handle	notice;
    int			paneX;
    int			paneY;
    int			paneWidth;
    int			paneHeight;
    int			pane_notice_edge_dist;
    int			i;

    DRAWABLE_INFO_MACRO(window, info);
    display = xv_display(info);
    w = xv_xid(info);
    notice = (notice_handle)xv_get(window, XV_KEY_DATA, notice_context_key);

    /*
     * Get Cms, pixel values
     */
    cms = xv_get(window, WIN_CMS, NULL);
    bg3 = xv_get(cms, CMS_PIXEL, 2, NULL);
    white = xv_get(cms, CMS_PIXEL, 3, NULL);
    fg = xv_get(cms, CMS_FOREGROUND_PIXEL, NULL);

    /*
     ****************************
     * Draw notice window borders
     ****************************
     */

    /*
     * Find a GC
     */
    gc = (GC)xv_find_proper_gc(display, info, PW_VECTOR);

    /*
     * Notice border color is foreground color
     */
    val.foreground = fg;
    val.line_style = LineSolid;
    val.line_width = 1;
    value_mask = GCLineStyle | GCLineWidth | GCForeground;
    XChangeGC(display, gc, value_mask, &val);

    if (is_toplevel_window)  {
        /*
         * Draw notice border 'notice_border_width' pixels wide
         */
        for (i=0; i < (NOTICE_BORDER_WIDTH(notice->scale)); ++i)  {
            /*
	     * Code to draw border to show 'raised' effect
            olgx_draw_box(ginfo, w, 
                        i,
                        i, 
                        width - (2*i),
                        height - (2*i),
                        OLGX_NORMAL, 0);
            */

            XDrawRectangle(display, w, 
                        gc, 
                        x+i, y+i, 
			(width - 2*i) - 1, 
			(height - 2*i) - 1);
        }
    }

    /*
     **************************
     * Draw notice pane borders
     **************************
     * REMINDER:
     * The notice pkg does its own rendering here. When olgx
     * supports chiseled boxes, then the notice pkg should use
     * that.
     */
    if (is_toplevel_window)  {
        pane_notice_edge_dist = NOTICE_BORDER_WIDTH(notice->scale) + 
				PANE_NOTICE_BORDER_DIST(notice->scale);
    }
    else  {
        pane_notice_edge_dist = 0;
    }
    paneX = x + pane_notice_edge_dist;
    paneY = y + pane_notice_edge_dist;
    paneWidth = width - (2 * pane_notice_edge_dist);
    paneHeight = height - (2 * pane_notice_edge_dist);

    /*
     * Code to draw border line to show 'raised' effect
    olgx_draw_box(ginfo, w, 
			paneX,
			paneY, 
			paneWidth,
			paneHeight,
			OLGX_NORMAL, 0);
    */

    /*
     * Pane border color
     *	light colored lines - white
     *	dark colored lines - bg3
     */
    val.foreground = bg3;
    XChangeGC(display, gc, GCForeground, &val);

    /*
     * Draw lines to give 'chiseled' look
     * Draw dark lines first
     */
    seg[0].x1 = paneX;
    seg[0].y1 = paneY + (paneHeight-1);
    seg[0].x2 = paneX;
    seg[0].y2 = paneY;

    seg[1].x1 = paneX;
    seg[1].y1 = paneY;
    seg[1].x2 = paneX + (paneWidth-1) - 1;
    seg[1].y2 = paneY;

    seg[2].x1 = paneX + (paneWidth-1) - 1;
    seg[2].y1 = paneY + 1;
    seg[2].x2 = paneX + (paneWidth-1) - 1;
    seg[2].y2 = paneY + (paneHeight-1) - 1;

    seg[3].x1 = paneX + (paneWidth-1) - 1;
    seg[3].y1 = paneY + (paneHeight-1) - 1;
    seg[3].x2 = paneX + 2;
    seg[3].y2 = paneY + (paneHeight-1) - 1;

    XDrawSegments(display, w, gc, seg, 4);

    /*
     * Set gc to draw light lines
     */
    val.foreground = white;
    XChangeGC(display, gc, GCForeground, &val);
    
    /*
     * Draw light lines next
     */
    seg[0].x1 = paneX + (paneWidth-1);
    seg[0].y1 = paneY;
    seg[0].x2 = paneX + (paneWidth-1);
    seg[0].y2 = paneY + (paneHeight-1);

    seg[1].x1 = paneX + (paneWidth-1);
    seg[1].y1 = paneY + (paneHeight-1);
    seg[1].x2 = paneX + 1;
    seg[1].y2 = paneY + (paneHeight-1);

    seg[2].x1 = paneX + 1;
    seg[2].y1 = paneY + (paneHeight-1);
    seg[2].x2 = paneX + 1;
    seg[2].y2 = paneY + 1;

    seg[3].x1 = paneX + 1;
    seg[3].y1 = paneY + 1;
    seg[3].x2 = paneX + (paneWidth-1) - 2;
    seg[3].y2 = paneY + 1;

    XDrawSegments(display, w, gc, seg, 4);
}

/*
 * Static routines
 */
static int
notice_offset_from_baseline(font)
    Xv_Font	font;
{
#ifdef  OW_I18N
    XFontSet            font_set;
    XFontSetExtents     *font_set_extents;
#else
    XFontStruct		*x_font_info;
#endif /* OW_I18N */

    if (font == XV_ZERO)
	return (0);

#ifdef OW_I18N
    font_set = (XFontSet)xv_get(font, FONT_SET_ID);
    font_set_extents = XExtentsOfFontSet(font_set);
    return(font_set_extents->max_ink_extent.y);
#else    
    x_font_info = (XFontStruct *)xv_get(font, FONT_INFO);
    return (x_font_info->ascent);
#endif /* OW_I18N */
}

