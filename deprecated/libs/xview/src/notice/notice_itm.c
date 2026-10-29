#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)notice_itm.c 1.18 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <X11/Xlib.h>
#include <xview_private/noticeimpl.h>
#include <olgx/olgx.h>
#include <xview_private/draw_impl.h>
#include <xview_private/pw_impl.h>
#include <xview_private/windowimpl.h>
#include <xview_private/wmgr_decor.h>
#include <xview/font.h>
#include <xview/frame.h>
#include <xview/panel.h>
#include <xview/win_input.h>
#include <xview/cms.h>
#include <xview/server.h>

Xv_private Graphics_info	*xv_init_olgx();
Xv_private void			frame_set_accept_default_focus();
static void			subframe_event_proc();
static Notify_value		subframe_destroy_proc();
static int			notice_get_default_value();
extern char			xv_iso_default_action;

/*
 * Event proc for notice sub frame
 * Note:
 * Since the Window pkg does not support variable width
 * window borders, we fake the extra thickness by drawing a rectangle 
 * inside the window.
 * Also, if detect ACTION_DISMISS, simulate pressing of default button.
 */
static void
subframe_event_proc(window, event)
Xv_window               window;
Event                   *event;
{
    XEvent		*xEvent;
    Notice_info		*notice; 
    int			width, height;
    Xv_Notice		notice_public;
    int			notice_value;

    xEvent = event_xevent(event);
    notice = (Notice_info *)xv_get(window, 
			XV_KEY_DATA, notice_context_key, 
		        NULL);

    /*
     * Check if notice exists as key data on window
     */
    if (notice)  {
	/*
	 * Check for X events
	 */
        switch(xEvent->type)  {
        case Expose:
	    /*
	     * Expose event detected - draw fake border
	     */
	    width = xv_get(window, XV_WIDTH);
	    height = xv_get(window, XV_HEIGHT);

	    notice_draw_borders(window, 0, 0, width, height, NOTICE_NOT_TOPLEVEL);

        break;
        }

        notice_public = NOTICE_PUBLIC(notice);

	/*
	 * Check for XView Semantice events
	 */
        switch (event_action(event))  {
        case ACTION_DISMISS:
	    /*
	     ********************************************
	     * When see ACTION_DISMISS, 
	     * Simulate the default button being pressed.
	     ********************************************
	     */
	    notice_value = notice_get_default_value(notice);

            /*
             * Store the value in the result field of notice for
             * later retrieval
             * notice->result_ptr is the address specified by user via
             * NOTICE_STATUS
             */
	    notice->result = notice_value;
	    if (notice->result_ptr)  {
    	        *(notice->result_ptr) = notice_value;
	    }

	    /*
	     * Call notice event proc if any
	     */
	    if (notice->event_proc)  {
	        (notice->event_proc)(notice_public, notice_value, event);
	    }

	    /*
	     * Pop down notice
	     */
	    if (notice->block_thread)  {
	        xv_window_return(XV_OK);
	    }
	    else  {
	        xv_set(notice_public, XV_SHOW, FALSE, NULL);
	    }
        break;
        }

	/*
	 * If detect default action, post it to panel
	 */
	if (event_action(event) == xv_iso_default_action)  {
	    if (notice->panel)  {
		event_set_action(event, ACTION_DEFAULT_ACTION);
                notify_post_event(notice->panel, (Notify_event)event, NOTIFY_IMMEDIATE);
	    }
	}
    }

}

/*
 * Destroy interpose proc for notice subframe
 * This is needed if the notice subframe is killed
 * via QUIT by the user.
 * olwm will not put any wmgr menus on the notice subframe
 * if _OL_WT_NOTICE is set on the notice subframe, but
 * other window wmgrs might.
 */
static Notify_value
subframe_destroy_proc(sub_frame, status)
Notify_client	sub_frame;
Destroy_status	status;
{
    Notice_info		*notice; 
    Xv_Notice		notice_public;
    Event		event;
    int			notice_value;

    /*
     * Get notice private data hanging off subframe
     */
    notice = (Notice_info *)xv_get(sub_frame, 
			XV_KEY_DATA, notice_context_key, 
		        NULL);

    if (!notice)  {
	/*
	 * Call next destroy proc if notice object missing
	 */
	return notify_next_destroy_func(sub_frame, status);
    }

    if (!notice->show)  {
	/*
	 * Call next destroy proc if notice is not currenty visible
	 */
	return notify_next_destroy_func(sub_frame, status);
    }

    notice_public = NOTICE_PUBLIC(notice);

    switch (status)  {
    case DESTROY_PROCESS_DEATH:
    break;

    case DESTROY_CHECKING:
    break;

    case DESTROY_CLEANUP:
	/*
	 ********************************************
	 * Simulate the default button being pressed.
	 ********************************************
	 */


	notice_value = notice_get_default_value(notice);

        /*
         * Store the value in the result field of notice for
         * later retrieval
         * notice->result_ptr is the address specified by user via
         * NOTICE_STATUS
         */
	notice->result = notice_value;
	if (notice->result_ptr)  {
    	    *(notice->result_ptr) = notice_value;
	}

	/*
	 * Call notice event proc if any
	 */
	if (notice->event_proc)  {
	    event_init((&event));
	    (notice->event_proc)(notice_public, notice_value, &event);
	}

	/*
	 * Pop down notice
	 */
	if (notice->block_thread)  {
	    xv_window_return(XV_OK);
	}
	else  {
	    xv_set(notice_public, XV_SHOW, FALSE, NULL);
	}

	/*
	 * Set sub_frame to NULL so that it will be re-created
	 * next time this notice is mapped
	 */
	notice->sub_frame = (Frame)NULL;

	/*
	 * Call next destroy proc
	 */
	return notify_next_destroy_func(sub_frame, status);
    break;

    case DESTROY_SAVE_YOURSELF:
    break;
    }

    return(NOTIFY_DONE);
}

/*
 * notice_create_base(notice)
 * Create base_frame and panel for notice 
 */
Pkg_private int
notice_create_base(notice)
Notice_info	*notice;
{
    Xv_Drawable_info		*info;
    Xv_Drawable_info		*client_info;

    /*
     * If no font specified, try to get client_window font
     */
    if (!notice->notice_font)  {
        int		e;
        if ((e = notice_determine_font(notice->client_window, 
                            notice)) != XV_OK)  {
            /*
             * If error occurred during font determination, 
             * return error code
             */
            return(e);
        }
    }

    if (notice->lock_screen)  {
	return(XV_OK);
    }

    if (!notice->sub_frame)  {
	Xv_server	server;
	WM_Win_Type	win_attr;

        /*
         * Create sub frame for notice if havent yet
         */
        notice->sub_frame = xv_create(notice->owner_window, FRAME,
			XV_LABEL, "Notice",
		        XV_FONT, notice->notice_font, 
			WIN_BORDER, FALSE,
			WIN_CONSUME_X_EVENT_MASK, 
				ExposureMask | KeyPressMask | FocusChangeMask,
			WIN_EVENT_PROC, subframe_event_proc,
			WIN_FRONT,
#ifdef OW_I18N
			WIN_USE_IM, FALSE,
#endif
			XV_KEY_DATA, notice_context_key, notice,
			XV_HELP_DATA, "xview:notice",
                        NULL);

	/*
	 * Do not grab SELECT button on notice frame
	 */
	xv_set(notice->sub_frame, WIN_UNGRAB_SELECT, NULL);

	/*
	 * Tell frame to accept focus if the panel does not take it.
	 * When not in Full mouseless mode, this is necessary
	 */
	frame_set_accept_default_focus(notice->sub_frame, TRUE);

	/*
	 * Set subframe's destroy proc
	 */
	notify_interpose_destroy_func(notice->sub_frame, subframe_destroy_proc);

	DRAWABLE_INFO_MACRO(notice->sub_frame, info);
	DRAWABLE_INFO_MACRO(notice->owner_window, client_info);
	XSetTransientForHint(xv_display(info), xv_xid(info), xv_xid(client_info));


	/*
	 * Get server object
	 */
	server = (Xv_server)XV_SERVER_FROM_WINDOW(notice->sub_frame);

	/*
	 * Set window attributes
	 *	window type = notice
	 *	pin initial state = none
	 *	menu type = none
	 */
        win_attr.flags = WMWinType;
        win_attr.win_type = (Atom)xv_get(server,SERVER_ATOM, "_OL_WT_NOTICE");
	(void)wmgr_set_win_attr(notice->sub_frame, &win_attr);

        if (xv_depth(info) > 1)  {
            notice->three_d = defaults_get_boolean("OpenWindows.3DLook.Color",
                "OpenWindows.3DLook.Color", TRUE);
        }
	else  {
#ifdef MONO3D
            notice->three_d = defaults_get_boolean("OpenWindows.3DLook.Monochrome",
                "OpenWindows.3DLook.Monochrome", FALSE);
#else
            notice->three_d = FALSE;
#endif
	}
	notice->ginfo = xv_init_olgx(notice->sub_frame, &notice->three_d,
			xv_get(notice->sub_frame, XV_FONT));
    }

    /*
     * Create panel which will contain message and buttons
     */
    if (!notice->panel)  {
        notice->panel = xv_create(notice->sub_frame, PANEL, 
				XV_FONT, notice->notice_font,
#ifdef OW_I18N
				WIN_USE_IM, FALSE,
#endif
				XV_HELP_DATA, "xview:notice",
				NULL);
    }

    /*
     * Get default CMS from panel and make frame use it
     * This is to make frame background be same color as 
     * panel.
     */
    xv_set(notice->sub_frame, 
		WIN_CMS, xv_get(notice->panel, WIN_CMS), NULL);
    
    return(XV_OK);
}

/*
 * Function to return value of the default button on notice
 */
static int
notice_get_default_value(notice)
Notice_info	*notice;
{
    int		numButtons = notice->number_of_buttons;
    struct notice_buttons	*curButton = notice->button_info;
    int		i;

    /*
     * Search thru all buttons
     */
    for (i=0; i < numButtons; ++i, curButton = curButton->next)  {
	/*
	 * Return default/'yes' button value 
	 */
	if (curButton->is_yes)  {
	    return(curButton->value);
	}
    }

    /*
     * If none found, return value of the first button
     */
    return(notice->button_info->value);

}
