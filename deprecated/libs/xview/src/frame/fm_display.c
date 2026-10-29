#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)fm_display.c 20.83 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE 
 *	file for terms of the license.
 */

/*
 * Handle frame displaying and size changes.
 */

#include <X11/Xlib.h>
#include <xview_private/fm_impl.h>
#include <xview_private/draw_impl.h>
#include <xview/server.h>
#include <xview/screen.h>
#include <xview/font.h>
#include <xview/cms.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>


Pkg_private void
frame_display_label(frame)
    register Frame_class_info *frame;
{
#ifdef OW_I18N
    Xv_Drawable_info *info;
    wchar_t		*wp;
    XICCEncodingStyle	 style;
    XTextProperty	 text_prop;

    DRAWABLE_INFO_MACRO(FRAME_PUBLIC(frame), info);

    /*
     * DEPEND_ON_BUG_1100305: We should able to use XStdICCTextStyle
     * which should elminate to test ASCII or not.
     */
    if (frame->label.pswcs.value) {
        for (wp = frame->label.pswcs.value; *wp; wp++)
        {
            if (! iswascii(*wp))
            {
                /*
	         * There are non ASCII characters, so, we have to send it
	         * as Compound Text Atom.
	         */
                style = XCompoundTextStyle;
	        goto send;

            }
        }
 
        /*
         * There are only ASCII characters, we can send it as STRING atom.
         */
        style = XStringStyle;

send:
        if (_xv_XwcTextListToTextProperty(FRAME_PUBLIC(frame), FRAME,
				          xv_display(info),
				          &frame->label.pswcs.value, 1, style,
				          &text_prop) >= 0) {
	    XSetTextProperty(xv_display(info), xv_xid(info),
			     &text_prop, XA_WM_NAME);
	    XFree(text_prop.value);
        }
    }

    return;
#else
    Xv_Drawable_info *info;

    DRAWABLE_INFO_MACRO(FRAME_PUBLIC(frame), info);
    XStoreName(xv_display(info), xv_xid(info), frame->label);
#endif
}

Pkg_private void
frame_display_footer(frame_public, clear_first)
    Frame frame_public;
    int clear_first;
{
    Frame_class_info	*frame = FRAME_PRIVATE(frame_public);
    Xv_Drawable_info	*info;
    Xv_Drawable_info	*frame_info;
    Cms			frame_cms; 	/* frame cms */
    int 		left_width, right_width;
    int 		max_left_width, max_right_width;
    int 		margin;
    int 		gap;
    int 		baseline;
    int 		footer_width;     
    int 		quarter_width;
    int			save_black;	/* saved value for OLGX_BLACK */
    int			new_black;	/* new value for OLGX_BLACK */
    short 		change_black;	/* flag to check if 
					 * save_black != new_black 
					 */
    Frame_rescale_state scale;
    
    DRAWABLE_INFO_MACRO(frame_public, frame_info);
    DRAWABLE_INFO_MACRO(frame->footer, info);
    frame_cms = xv_cms(frame_info);

    /*
     * Make sure footer text gets drawn in same color as fg color of frame.
     *
     * The original value for OLGX_BLACK is saved in 'save_black'. It will
     * be restored at the end of this function.
     * 'new_black' is the desired value for OLGX_BLACK. It is the fg 
     * color of the footer. At this point in time, this should be the same
     * as the fg color of the frame, unless the frame does not have a control 
     * cms.
     * 
     * The OLGX_BLACK value of the ginfo is changed (and restored later) only 
     * if 'save_black' != 'new_black'.
     */
    save_black = olgx_get_single_color(frame->ginfo, OLGX_BLACK);
    new_black = xv_get(xv_cms(info), CMS_PIXEL, 
			xv_get(frame->footer, WIN_FOREGROUND_COLOR)); 

    change_black = (save_black != new_black);

    if (change_black)  {
        olgx_set_single_color(frame->ginfo, OLGX_BLACK, new_black, OLGX_SPECIAL);
    }

    scale = xv_get(xv_get(frame_public, XV_FONT), FONT_SCALE);

#ifdef OW_I18N
    if (frame->left_footer.pswcs.value == NULL) 
      left_width = 0;
    else 
      left_width = XwcTextEscapement(TextFont_Set(frame->ginfo),
			      frame->left_footer.pswcs.value, 
			      wslen(frame->left_footer.pswcs.value));
    if (frame->right_footer.pswcs.value == NULL)
      right_width = 0;
    else
      right_width = XwcTextEscapement(TextFont_Set(frame->ginfo),
			       frame->right_footer.pswcs.value, 
			       wslen(frame->right_footer.pswcs.value));
#else
    if (frame->left_footer == NULL) 
      left_width = 0;
    else 
      left_width = XTextWidth(frame->ginfo->textfont, 
			      frame->left_footer, 
			      strlen(frame->left_footer));
    if (frame->right_footer == NULL)
      right_width = 0;
    else
      right_width = XTextWidth(frame->ginfo->textfont, 
			       frame->right_footer, 
			       strlen(frame->right_footer));
#endif
    
    margin = frame_footer_margin(scale);
    gap = frame_inter_footer_gap(scale);  
    footer_width = (int)xv_get(frame_public, XV_WIDTH) - 2 * margin;
    quarter_width = footer_width / 4;
    baseline = (int)xv_get(frame->footer, XV_HEIGHT) -
		    frame_footer_baseline(scale);

    if ((left_width + gap + right_width) <= footer_width) {
	/* They both fit, no clipping */
	max_left_width = left_width;
	max_right_width = right_width;
    } else if (right_width < quarter_width) {
	/* right footer takes less than 1/4 of the footer */
	max_left_width = footer_width - gap - right_width;
	max_right_width = right_width;
    } else if (left_width < (footer_width - quarter_width - gap)) {
	/* left footer takes less than 3/4 of the footer */
	max_left_width = left_width;
	max_right_width = footer_width - max_left_width - gap;
    } else {
	/* must truncate both */
	max_left_width = footer_width - quarter_width - gap;
	max_right_width = quarter_width;
    }    

    if (clear_first)
      XClearWindow(xv_display(info), xv_xid(info));
#ifdef OW_I18N
    if (frame->left_footer.pswcs.value != NULL) {
	olgx_draw_text(frame->ginfo, xv_xid(info), frame->left_footer.pswcs.value, 
		       margin, baseline, max_left_width, 
		       OLGX_NORMAL | OLGX_MORE_ARROW | OLGX_LABEL_IS_WCS);
    }
    if (frame->right_footer.pswcs.value != NULL) {
	olgx_draw_text(frame->ginfo, xv_xid(info), frame->right_footer.pswcs.value,
		       footer_width + margin - max_right_width, baseline,
		       max_right_width,
		       OLGX_NORMAL | OLGX_MORE_ARROW | OLGX_LABEL_IS_WCS);
    }
#else
    if (frame->left_footer != NULL) {
	olgx_draw_text(frame->ginfo, xv_xid(info), frame->left_footer, 
		       margin, baseline, max_left_width, 
		       OLGX_NORMAL | OLGX_MORE_ARROW);
    }
    if (frame->right_footer != NULL) {
	olgx_draw_text(frame->ginfo, xv_xid(info), frame->right_footer,
		       footer_width + margin - max_right_width, baseline,
		       max_right_width, OLGX_NORMAL | OLGX_MORE_ARROW);
    }
#endif
    XFlush(xv_display(info));

    /*
     * Restore OLGX_BLACK in ginfo only if it was changed before
     */
    if (change_black)  {
        olgx_set_single_color(frame->ginfo, OLGX_BLACK, save_black, OLGX_SPECIAL);
    }
}

#ifdef OW_I18N
Pkg_private void
frame_display_IMstatus(frame_public, clear_first)
    Frame frame_public;
    int clear_first;
{
    Frame_class_info	*frame = FRAME_PRIVATE(frame_public);
    Xv_Drawable_info	*info;
    Xv_Drawable_info	*frame_info;
    Cms			frame_cms;
    int 		left_width, right_width;
    int 		max_left_width, max_right_width;
    int 		margin;
    int 		gap;
    int 		baseline;
    int 		footer_width;     
    int 		footer_height;
    int 		quarter_width;
    int			save_black;	/* saved value for OLGX_BLACK */
    int			new_black;	/* new value for OLGX_BLACK */
    short 		change_black;	/* flag to check if 
					 * save_black != new_black 
					 */
    Frame_rescale_state scale;
    
    DRAWABLE_INFO_MACRO(frame_public, frame_info);
    DRAWABLE_INFO_MACRO(frame->imstatus, info);
    frame_cms = xv_cms(frame_info);
    
    /*
     * Make sure imstatus text gets drawn in same color as fg color of frame.
     *
     * The original value for OLGX_BLACK is saved in 'save_black'. It will
     * be restored at the end of this function.
     * 'new_black' is the desired value for OLGX_BLACK. It is the fg 
     * color of the imstatus. At this point in time, this should be the same
     * as the fg color of the frame, unless the frame does not have a control 
     * cms.
     * 
     * The OLGX_BLACK value of the ginfo is changed (and restored later) only 
     * if 'save_black' != 'new_black'.
     */
    save_black = olgx_get_single_color(frame->ginfo, OLGX_BLACK);
    new_black = xv_get(frame_cms, CMS_PIXEL, 
			xv_get(frame->imstatus, WIN_FOREGROUND_COLOR)); 

    change_black = (save_black != new_black);

    if (change_black)  {
        olgx_set_single_color(frame->ginfo, OLGX_BLACK, new_black, OLGX_SPECIAL);
    }

    scale = xv_get(xv_get(frame_public, XV_FONT), FONT_SCALE);

    if (frame->left_IMstatus.pswcs.value == NULL) 
      left_width = 0;
    else 
      left_width = XwcTextEscapement(TextFont_Set(frame->ginfo),
			      frame->left_IMstatus.pswcs.value, 
			      wslen(frame->left_IMstatus.pswcs.value));
    if (frame->right_IMstatus.pswcs.value == NULL)
      right_width = 0;
    else
      right_width = XwcTextEscapement(TextFont_Set(frame->ginfo), 
			       frame->right_IMstatus.pswcs.value, 
			       wslen(frame->right_IMstatus.pswcs.value));
    
    margin = frame_footer_margin(scale);
    gap = frame_inter_footer_gap(scale);  
    footer_width = (int)xv_get(frame_public, XV_WIDTH) - 2 * margin;
    footer_height = (int)xv_get(frame->imstatus, XV_HEIGHT);
    quarter_width = footer_width / 4;
    baseline = footer_height - frame_footer_baseline(scale);

    if ((left_width + gap + right_width) <= footer_width) {
	/* They both fit, no clipping */
	max_left_width = left_width;
	max_right_width = right_width;
    } else if (right_width < quarter_width) {
	/* right footer takes less than 1/4 of the footer */
	max_left_width = footer_width - gap - right_width;
	max_right_width = right_width;
    } else if (left_width < (footer_width - quarter_width - gap)) {
	/* left footer takes less than 3/4 of the footer */
	max_left_width = left_width;
	max_right_width = footer_width - max_left_width - gap;
    } else {
	/* must truncate both */
	max_left_width = footer_width - quarter_width - gap;
	max_right_width = quarter_width;
    }    

    if (clear_first)
      XClearWindow(xv_display(info), xv_xid(info));
    if (frame->left_IMstatus.pswcs.value != NULL) {
	olgx_draw_text(frame->ginfo, xv_xid(info),
		       frame->left_IMstatus.pswcs.value, 
		       margin, baseline, max_left_width, 
		       OLGX_NORMAL | OLGX_MORE_ARROW | OLGX_LABEL_IS_WCS);
	if (status_get(frame, inactive_imstatus)) 
	   olgx_stipple_rect(frame->ginfo, xv_xid(info),
			margin, 0, max_left_width, footer_height);
    }
    if (frame->right_IMstatus.pswcs.value != NULL) {
	olgx_draw_text(frame->ginfo, xv_xid(info),
		       frame->right_IMstatus.pswcs.value,
		       footer_width + margin - max_right_width, baseline,
		       max_right_width,
		       OLGX_NORMAL | OLGX_MORE_ARROW | OLGX_LABEL_IS_WCS);
	if (status_get(frame, inactive_imstatus)) 
	   olgx_stipple_rect(frame->ginfo, xv_xid(info),
			footer_width + margin - max_right_width, baseline, 
			max_right_width, footer_height);
    }
    XFlush(xv_display(info));

    /*
     * Restore OLGX_BLACK in ginfo only if it was changed before
     */
    if (change_black)  {
        olgx_set_single_color(frame->ginfo, OLGX_BLACK, save_black, OLGX_SPECIAL);
    }
}
#endif /* OW_I18N */

/*
 * Function to make sure the footer/imstatus region inherits the frame's
 * cms/fg. The cms/fg is inherited only if the frame cms is a control 
 * cms.
 *	frame_public		public handle to frame
 *	status_window		either footer or imstatus window
 *	new_frame_cms		new cms (possibly not yet) set on frame. 
 *				Ignored if == NULL
 *	new_frame_fg		new fg color (possibly not yet) set on frame
 *	new_frame_fg_set	flag to indicate whether new_frame_fg should 
 *				be ignored
 *	repaint_needed		TRUE is returned if the cms/fg was actually 
 *				changed.
 */
Pkg_private void
frame_update_status_win_color(frame_public, status_window, new_frame_cms,
		new_frame_fg, new_frame_fg_set, repaint_needed)
Frame		frame_public;
Xv_Window	status_window;
Cms		new_frame_cms;
unsigned long	new_frame_fg;
short		new_frame_fg_set;
int		*repaint_needed;
{
    Frame_class_info *frame = FRAME_CLASS_PRIVATE(frame_public);

    /*
     * Initialize repaint flag to FALSE
     */
    *repaint_needed = FALSE;

    /*
     * Continue only if we are not in the creation process, and
     * the status window exists
     */
    if (status_get(frame, created) && status_window)  {
        Xv_Drawable_info	*status_info;
        Cms			status_cms;

        DRAWABLE_INFO_MACRO(status_window, status_info);
	/*
	 * Get cms of status window
	 */
	status_cms = xv_cms(status_info);

	/*
	 * Set new cms on status window if:
	 *	A valid one was actually passed in
	 *	It was not already inherited
	 *	It is a control cms
	 */
        if ( new_frame_cms && (new_frame_cms != status_cms) && 
		(xv_get(new_frame_cms, CMS_CONTROL_CMS)) )  {

	    /*
	     * Set new cms on status window
	     */
            xv_set(status_window, WIN_CMS, new_frame_cms, NULL);

	    /*
	     * Set repaint flag to TRUE
	     */
            *repaint_needed = TRUE;
        }

	/*
	 * Set new fg color on status window if
	 *	cms of frame is a control cms
	 * If a new cms was not set on the frame(new_frame_cms == NULL),
	 * use the current cms on the frame.
	 */
        if (new_frame_fg_set)  {
	    if (!new_frame_cms)  {
		Xv_Drawable_info	*frame_info;

		DRAWABLE_INFO_MACRO(frame_public, frame_info);
		/*
		 * Get current cms on frame
		 */
		new_frame_cms = xv_cms(frame_info);
	    }

            if (xv_get(new_frame_cms, CMS_CONTROL_CMS))  {

	        /*
	         * Set new fg color on status window
	         */
                xv_set(status_window, WIN_FOREGROUND_COLOR, new_frame_fg, NULL);

		/*
		 * Set repaint flag to TRUE
		 */
                *repaint_needed = TRUE;
            }
        }
    }
}

Pkg_private void
frame_display_busy(frame, status)
    register Frame_class_info *frame;
    int             status;

{
    Frame           frame_public = FRAME_PUBLIC(frame);
    Xv_Drawable_info *info;
    Xv_object       screen, server;


    DRAWABLE_INFO_MACRO(frame_public, info);
    screen = xv_get(frame_public, XV_SCREEN);
    server = xv_get(screen, SCREEN_SERVER);

    XChangeProperty(xv_display(info), xv_xid(info),
		    xv_get(server, SERVER_WM_WIN_BUSY), XA_INTEGER,
		    32, PropModeReplace, (unsigned char *)&status,
		    1);
    XFlush(xv_display(info));
}

/*
 * highlight subwindow border for sw.
 */
Xv_private void
frame_kbd_use(frame_public, sw, pw)
    Frame           frame_public;
    Xv_Window       sw;		/* frame subwindow */
    Xv_Window	    pw;		/* paint window that has the keyboard focus.
				 * This may or may not be equal to sw. */
{
    Frame_class_info *frame = FRAME_CLASS_PRIVATE(frame_public);
    Cms cms;

    if (frame->focus_subwindow != sw) {
	/* Remove caret from current frame focus subwindow */
	if (frame->focus_subwindow)
	    xv_set(frame->focus_subwindow, WIN_REMOVE_CARET, NULL);
	/* Update frame focus subwindow and primary focus subwindow data */
	frame->focus_subwindow = sw;
	if (xv_get(sw, XV_FOCUS_RANK) == XV_FOCUS_PRIMARY)
	    frame->primary_focus_sw = sw;
    }

    /* Set the CMS, foreground and background color of the focus window to be
     * the same as the paint window with the input focus.
     */
    /* BUG: We should be keeping seperate focus window's for different
     * visuals.  Because we don't we can not just set the the cms from the 
     * paint window onto the focus window, because they may be of different visuals.
     */
    
    cms = (Cms)xv_get(pw, WIN_CMS);
    if (XVisualIDFromVisual((Visual *)xv_get(frame->focus_window, XV_VISUAL)) ==
	XVisualIDFromVisual((Visual *)xv_get(cms, XV_VISUAL)))
      xv_set(frame->focus_window,
	     WIN_CMS, cms,
	     WIN_FOREGROUND_COLOR, xv_get(pw, WIN_FOREGROUND_COLOR),
	     WIN_BACKGROUND_COLOR, xv_get(pw, WIN_BACKGROUND_COLOR),
	     NULL);
    
    /* Show caret in frame subwindow with input focus */
    xv_set(sw, WIN_KBD_FOCUS, TRUE, NULL);
}

/*
 * unhighlight subwindow border for client.
 */
/*ARGSUSED*/
Xv_private void
frame_kbd_done(frame_public, sw)
    Frame           frame_public;
    Xv_Window       sw;
{
    xv_set(sw, WIN_KBD_FOCUS, FALSE, NULL);
}

/* ARGSUSED */
Pkg_private int
frame_set_color(frame, fg, bg)
    Frame_class_info *frame;
    XColor *fg;
    XColor *bg;
{
    Frame		frame_public = FRAME_PUBLIC(frame);
    Xv_Drawable_info	*info;
    Cms			cms;	
    XColor		xcolors[2];
    char		name[60];

    DRAWABLE_INFO_MACRO(frame_public, info);
    if (!fg && !bg) {
	return 0;
    }

    xcolors[0].red = bg->red;
    xcolors[0].green = bg->green;
    xcolors[0].blue = bg->blue;

    xcolors[1].red = fg->red;
    xcolors[1].green = fg->green;
    xcolors[1].blue = fg->blue;

    sprintf(name, "xv_frame_class_%d%d%d_%d%d%d", 
        bg->red,
        bg->green,
        bg->blue,
        fg->red,
        fg->green,
        fg->blue);

    cms = (Cms)xv_find(xv_screen(info), CMS,
			 CMS_NAME, name,
			 CMS_SIZE, 2,
			 CMS_X_COLORS, xcolors,
			 CMS_FRAME_CMS, TRUE,
			 CMS_TYPE, XV_STATIC_CMS,
			 XV_VISUAL, xv_get(frame_public, XV_VISUAL),
			 NULL);

    if (cms != (Cms)NULL)
      xv_set(frame_public, WIN_CMS, cms, NULL);

    return 0;
}
