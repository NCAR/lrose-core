#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)fm_input.c 20.59 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE 
 *	file for terms of the license.
 */

#include <xview_private/fm_impl.h>
#include <xview_private/draw_impl.h>
#include <xview/notice.h>
#include <xview/sel_svc.h>

static int frame_set_focus();


/* ARGSUSED */
Pkg_private     Notify_value
frame_input(frame_public, event, arg, type)
    Frame           frame_public;
    Event          *event;
    Notify_arg      arg;
    Notify_event_type type;
{
    Frame_class_info *frame = FRAME_CLASS_PRIVATE(frame_public);
    unsigned int    action = event_action(event);
    char           *help_data;

    /* Tell the selection service about GET, PUT, FIND, DELETE */

    switch (action) {
      case ACTION_COPY:
      case ACTION_PASTE:
      case ACTION_CUT:
      case ACTION_FIND_FORWARD:
      case ACTION_FIND_BACKWARD:
	seln_report_event((Seln_client)frame_public, event);
	break;
      default:
	break;
    }
    switch (action) {
      case ACTION_CUT:
	/* only want up of function keys */
	if (event_is_down(event))
	    goto Done;
	break;

      case ACTION_HELP:
      case ACTION_MORE_HELP:
      case ACTION_TEXT_HELP:
      case ACTION_MORE_TEXT_HELP:
      case ACTION_INPUT_FOCUS_HELP:
	if (event_is_down(event)) {
	    help_data = (char *) xv_get(frame_public, XV_HELP_DATA);
	    if (help_data)
		xv_help_show(frame_public, help_data, event);
	}
	return NOTIFY_DONE;

      case ACTION_RESCALE:
	frame_rescale_subwindows(frame_public, (int) arg);
	goto Done;

      case WIN_RESIZE:

	(void) win_getsize(frame_public, &frame->rectcache);

			/* Set width and height size hints for backwards 
			 * compatibility with pre-ICCCM window managers  */
	if (!defaults_get_boolean("xview.icccmcompliant",
				  "XView.ICCCMCompliant", TRUE)) {
		XSizeHints		sizeHints;
		Xv_Drawable_info	*info;

		DRAWABLE_INFO_MACRO(frame_public, info);
		sizeHints.flags = PSize;
		sizeHints.width = frame->rectcache.r_width; 
		sizeHints.height = frame->rectcache.r_height; 
		XSetNormalHints(xv_display(info), xv_xid(info), &sizeHints);
	}
#ifdef OW_I18N
        if (status_get(frame, show_imstatus)) {
            if (status_get(frame,show_footer))
                xv_set(frame->imstatus,
                       XV_WIDTH, frame->rectcache.r_width,
                       XV_Y, frame->rectcache.r_height -
                             xv_get(frame->footer, XV_HEIGHT) -
                             xv_get(frame->imstatus, XV_HEIGHT),
                       NULL);
            else
                xv_set(frame->imstatus,
                       XV_WIDTH, frame->rectcache.r_width,
                       XV_Y, frame->rectcache.r_height -
                             xv_get(frame->imstatus, XV_HEIGHT),
                       NULL);

        }
#endif
	if (status_get(frame, show_footer)) {
	    xv_set(frame->footer,
		   XV_WIDTH, frame->rectcache.r_width,
		   XV_Y, frame->rectcache.r_height -
					      xv_get(frame->footer, XV_HEIGHT),
		   NULL);
	}	    
	(void) frame_layout_subwindows(frame_public);
        goto Done;

      case ACTION_TAKE_FOCUS:{
	    Xv_Window       child;
            int		    subw_found = FALSE;

	    /*
	     * Check if empty frame i.e. no subwindows
	     * If yes, set focus to the frame
	     * - ICONs don't really count as subwindows
	     */
	    FRAME_EACH_CHILD(frame->first_subwindow, child)
	        if (!xv_get(child, XV_IS_SUBTYPE_OF, ICON))  {
	            subw_found = TRUE;
	            break;
	        }
	    FRAME_END_EACH
		
	    if (!subw_found)  {
	        if (frame_set_focus(frame_public) == XV_OK)  {
	            goto Done;
	        }
	    }

	    /* Set the input focus to the last primary focus subwindow that had
	     * the input focus.  If no primary focus subwindow ever had the
	     * input focus, then set the input focus to the first primary focus
	     * subwindow.  If there are no primary focus subwindows, then
	     * restore the input focus to the last subwindow that had the input
	     * focus.  If no subwindow has ever had the input focus, set the
	     * input focus to the first subwindow that can take the input focus.
	     * So, in summary, here are the priorities:
	     *    1. Most recent primary focus subwindow
	     *    2. First primary focus subwindow
	     *    3. Most recent (ordinary) focus subwindow
	     *    4. First (ordinary) focus subwindow
	     */

	    /* Priority 1: Most recent primary focus subwindow */
	    if (frame->primary_focus_sw &&
		frame_set_focus(frame->primary_focus_sw) == XV_OK)
		goto Done;

	    /* Priority 2: First primary focus subwindow */
	    FRAME_EACH_CHILD(frame->first_subwindow, child)
		if (xv_get(child, XV_FOCUS_RANK) == XV_FOCUS_PRIMARY &&
		    frame_set_focus(child) == XV_OK)
		    goto Done;
	    FRAME_END_EACH

	    /* Priority 3: Most recent ordinary focus subwindow */
	    if (frame->focus_subwindow &&
		frame_set_focus(frame->focus_subwindow) == XV_OK)
		goto Done;

	    /* Priority 4: First ordinary focus subwindow.
	     *
	     * Go through the list of subwindows and make sure none of them
	     * have the focus, then find the first subwindow that can accept
	     * keyboard input and assign input focus to it.
	     */
	    FRAME_EACH_CHILD(frame->first_subwindow, child)
		if (xv_get(child, WIN_KBD_FOCUS) == (Xv_opaque) TRUE)
		    goto Done;
	    FRAME_END_EACH
	    FRAME_EACH_CHILD(frame->first_subwindow, child)
		if (xv_set(child, WIN_SET_FOCUS, NULL) == XV_OK)
		    goto Done;
	    FRAME_END_EACH

	    /*
	     * If got this far, focus was not set.
	     * Check if frame should take the default focus. Done only when:
	     *		accelerators are present
	     *		accept_default_focus flag set
	     */
	    if (frame->menu_accelerators || frame->accelerators || 
			status_get(frame, accept_default_focus)) {
	        if (frame_set_focus(frame_public) == XV_OK)  {
	            goto Done;
	        }
	    }

		break;
	}

      default:
	/* Cannot ignore up events. Forward it */
	if (event_is_up(event))
	    goto Done;
	break;
    }

    switch (action) {

      case ACTION_OPEN:
	if (status_get(frame, map_state_change)) {
	    status_set(frame, map_state_change, FALSE);
	} else {
	    status_set(frame, iconic, FALSE);
	    status_set(frame, initial_state, FALSE);
	} 
	break;

      case ACTION_CLOSE:
	if (status_get(frame, map_state_change)) {
	    status_set(frame, map_state_change, FALSE);
	} else {
	    status_set(frame, iconic, TRUE);
	    status_set(frame, initial_state, TRUE);
	} 
	break;

      case ACTION_DISMISS:
	status_set(frame, dismiss, TRUE);
	if (frame->done_proc)
	    (void) (frame->done_proc) (frame_public);
	else
	    (void) (frame->default_done_proc) (frame_public);
	break;

      case ACTION_PROPS:
	frame_handle_props(frame_public);
	break;

      case ACTION_PININ:	/* reset dismiss state, if in it */
	status_set(frame, dismiss, FALSE);
	break;
    }

Done:
    return notify_next_event_func((Notify_client) frame_public,
				  (Notify_event) event, arg, type);
}

/*ARGSUSED*/
Pkg_private	void
frame_focus_win_event_proc(window, event, arg)
    Xv_Window	    window;
    Event	   *event;
    Notify_arg	    arg;
{
    Frame_focus_direction focus_direction;
    GC		    gc;		/* Graphics Context */
    XGCValues	    gc_values;
    int		    height;
    Server_image    image;
    Xv_Drawable_info *image_info;
    Xv_Drawable_info *info;
    int		    width;

    if (event_action(event) == WIN_REPAINT) {
	focus_direction = (Frame_focus_direction) xv_get(window,
	    XV_KEY_DATA, FRAME_FOCUS_DIRECTION);
	if (focus_direction == FRAME_FOCUS_UP) {
	    image = xv_get(window, XV_KEY_DATA, FRAME_FOCUS_UP_IMAGE);
	    width = FRAME_FOCUS_UP_WIDTH;
	    height = FRAME_FOCUS_UP_HEIGHT;
	} else {
	    image = xv_get(window, XV_KEY_DATA, FRAME_FOCUS_RIGHT_IMAGE);
	    width = FRAME_FOCUS_RIGHT_WIDTH;
	    height = FRAME_FOCUS_RIGHT_HEIGHT;
	}
	DRAWABLE_INFO_MACRO(window, info);
	gc = (GC) xv_get(window, XV_KEY_DATA, FRAME_FOCUS_GC);
	if (!gc) {
	    /* Create the Graphics Context for the Focus Window */
	    /* THIS IS ALSO DONE IN notice_show_focus_win() in notice_pt.c */
	    gc_values.fill_style = FillOpaqueStippled;
	    gc = XCreateGC(xv_display(info), xv_xid(info), GCFillStyle,
			   &gc_values);
	    xv_set(window, XV_KEY_DATA, FRAME_FOCUS_GC, gc, NULL);
	}
	DRAWABLE_INFO_MACRO(image, image_info);
	gc_values.background = xv_bg(info);
	gc_values.foreground = xv_fg(info);
	gc_values.stipple = xv_xid(image_info);
	XChangeGC(xv_display(info), gc, GCForeground | GCBackground | GCStipple,
		  &gc_values);
	XFillRectangle(xv_display(info), xv_xid(info), gc, 0, 0,
		       width, height);
    }
}

Pkg_private     Notify_value
frame_footer_input(footer, event, arg, type)
    Xv_Window       footer;
    Event          *event;
    Notify_arg      arg;
    Notify_event_type type;
{
    switch (event_action(event)) {
      case WIN_REPAINT:
	frame_display_footer(xv_get(footer, WIN_PARENT), FALSE);
	break;
      default:
	break;
    }
    return notify_next_event_func(footer, (Notify_event)event, arg, type);
}

#ifdef OW_I18N
Pkg_private     Notify_value
frame_IMstatus_input(IMstatus, event, arg, type)
    Xv_Window       IMstatus;
    Event          *event;
    Notify_arg      arg;
    Notify_event_type type;
{
    switch (event_action(event)) {
      case WIN_REPAINT:
	frame_display_IMstatus(xv_get(IMstatus, WIN_PARENT), FALSE);
	break;
      default:
	break;
    }
    return notify_next_event_func(IMstatus, (Notify_event)event, arg, type);
}
#endif

static int		/* Returns XV_OK or XV_ERROR */
frame_set_focus(sw)
    Xv_Window	    sw;		/* frame subwindow to receive kbd focus */
{
    /* If the subwindow already has the input focus, then don't
     * set the focus on it.  Case in point:  In Click-to-type mode,
     * the second view in a textsw subwindow has the input focus.
     * Clicking SELECT in the window header will generate an
     * ACTION_TAKE_FOCUS event.  If we we're to set WIN_SET_FOCUS on
     * the textsw, the input focus would change to the first view
     * in the textsw.
     */
    if (xv_get(sw, WIN_KBD_FOCUS) == (Xv_opaque) TRUE)
	return XV_OK;

    /* The subwindow doesn't have the input focus, so set the input
     * focus to the subwindow.
     */
    if (xv_set(sw, WIN_SET_FOCUS, NULL) == XV_OK)
	return XV_OK;

    return XV_ERROR;
}

/*
 * XView private function to set the accept_default_focus bit
 * So far, it is used by notices, acceleators.
 */
Xv_private	void
frame_set_accept_default_focus(frame_public, flag)
/* Alpha compatibility, mbuck@debian.org */
#if 1
    Frame frame_public;
#endif
{
    Frame_class_info *frame = FRAME_CLASS_PRIVATE(frame_public);

    status_set(frame, accept_default_focus, flag);
}
