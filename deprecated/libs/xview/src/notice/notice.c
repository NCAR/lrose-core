#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)notice.c 20.110 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE 
 *	file for terms of the license.
 */

#include <stdio.h>
#include <X11/Xlib.h>
#include <xview_private/i18n_impl.h>
#include <xview_private/draw_impl.h>
#include <xview_private/noticeimpl.h>
#include <xview/font.h>
#include <xview/frame.h>
#include <xview/panel.h>
#include <xview/win_input.h>
#include <xview/cms.h>
#include <xview/screen.h>

#ifdef OW_I18N
extern struct pr_size xv_pf_textwidth_wc();
#else
extern struct pr_size xv_pf_textwidth();
#endif

#ifdef  OW_I18N
static wchar_t notice_default_button_str[8] = {
                        (wchar_t)'C' ,
                        (wchar_t)'o' ,
                        (wchar_t)'n' ,
                        (wchar_t)'f' ,
                        (wchar_t)'i' ,
                        (wchar_t)'r' ,
                        (wchar_t)'m' ,
                        (wchar_t) NULL };

#else
static char *notice_default_button_str = "Confirm";
#endif

Xv_private void		win_beep();

void			notice_add_default_button();
void			notice_defaults();
void			notice_add_button_to_list();
void			notice_free_button_structs();
void			notice_do_bell();
int			notice_determine_font();
notice_buttons_handle	notice_create_button_struct();

Pkg_private void		notice_button_panel_proc();

Defaults_pairs bell_types[] = {
	"never",   0,
	"notices", 1,
	"always",  2,
	NULL,      2,
};

/*
 * Key data for handle to notice private data
 */
int		notice_context_key;

int		default_beeps;
int		notice_use_audible_bell;
int		notice_jump_cursor;

/*
 * ----------------------PRIVATE PROCS-------------------------------
 */

Pkg_private int
notice_init_internal(client_window, notice_public, avlist)
    Xv_Window       	client_window;
    Xv_notice_struct	*notice_public;
    Attr_avlist         avlist;
{
    Notice_info 	*notice;

    if (!client_window) {
	xv_error(XV_ZERO,
		 ERROR_STRING,
	     XV_MSG("NULL parent window passed to NOTICE. Not allowed."),
	         ERROR_PKG, NOTICE,
		 NULL);
	return (XV_ERROR);
    }

    if (!notice_context_key)  {
	notice_context_key = xv_unique_key();
    }

    /*
     * Allocate space for private data
     */
    notice = (Notice_info *) xv_calloc(1, sizeof(Notice_info));
    if (!notice) {
        xv_error(XV_ZERO,
            ERROR_STRING, XV_MSG("Malloc failed."),
	    ERROR_PKG, NOTICE,
        NULL);
        return (XV_ERROR);
    }

    /*
     * Set forward/backward pointers
     */
    notice_public->private_data = (Xv_opaque) notice;
    notice->public_self = (Xv_opaque) notice_public;

    /*
     * Get notice default settings
     */
    notice_use_audible_bell = defaults_get_enum("openWindows.beep",
                                "OpenWindows.Beep", bell_types);
    /*
     * Make notice pkg look for new resource OpenWindows.PopupJumpCursor
     * first before Notice.JumpCursor
     */
    if (defaults_exists("openWindows.popupJumpCursor", "OpenWindows.PopupJumpCursor"))  {
        notice_jump_cursor = (int)defaults_get_boolean("openWindows.popupJumpCursor", 
			"OpenWindows.PopupJumpCursor", (Bool) TRUE);
    }
    else  {
        notice_jump_cursor = (int)defaults_get_boolean("notice.jumpCursor", 
			"Notice.JumpCursor", (Bool) TRUE);
    }
    default_beeps = defaults_get_integer("notice.beepCount", 
			"Notice.BeepCount", 1);

    /*
     * Set fields of notice to default values in preparation
     * for notice_set_avlist
     */
    (void) notice_defaults(notice);

    notice->client_window = client_window;

    (void)notice_get_owner_frame(notice);


    return (XV_OK);
}

/*
 * Add a button struct to the list of buttons in the notice private data
 * and give it default values.
 */
Pkg_private void
notice_add_default_button(notice)
    register notice_handle notice;
{
    notice_buttons_handle button;

    button = (notice_buttons_handle) notice_create_button_struct();
    button->string = (CHAR *) XV_STRSAVE(notice_default_button_str);
    button->is_yes = TRUE;
    button->value = NOTICE_YES;
    button->panel_item = (Panel_item)NULL;
    notice->yes_button_exists = TRUE;
    (void) notice_add_button_to_list(notice, button);
    notice->number_of_buttons++;

}

/*
 * Fill in fields of private_data with default values
 */
Pkg_private void
notice_defaults(notice)
    notice_handle   notice;
{
    notice->client_window = 
    notice->owner_window = 
    notice->fullscreen_window = (Xv_window)NULL;
    notice->sub_frame = (Frame)NULL;
    notice->panel = (Panel)NULL;
    notice->busy_frames = (Frame *)NULL;
    notice->event_proc = (void(*)())NULL;

    notice->result_ptr = NULL;

    notice->default_input_code = '\0';	/* ASCII NULL */
    notice->event = (Event *)NULL;

    notice->notice_font = (Xv_Font) NULL;

    notice->beeps = default_beeps;

    notice->number_of_buttons = 0;
    notice->number_of_strs = 0;
    notice->button_info = (notice_buttons_handle) NULL;
    notice->msg_info = (notice_msgs_handle) NULL;
    notice->help_data = "xview:notice";

    notice->lock_screen = 0;
    notice->yes_button_exists = 0;
    notice->no_button_exists = 0;
    notice->focus_specified = 0;
    notice->dont_beep = 0;
    notice->need_layout = 1;
    notice->show = 0;
    notice->new = 1;

    notice->block_thread = 1;

    notice->scale = 1;

}

/*
 * Add button to list of buttons on notice
 */
Pkg_private void
notice_add_button_to_list(notice, button)
    register notice_handle notice;
    notice_buttons_handle button;
{
    notice_buttons_handle curr;

    if (notice->button_info) {
	for (curr = notice->button_info; curr; curr = curr->next)
	    if (curr->next == NULL) {
		curr->next = button;
		break;
	    }
    } else
	notice->button_info = button;
}

/*
 * Add string to list of messages on notice
 */
Pkg_private void
notice_add_msg_to_list(notice, msg)
    register notice_handle notice;
    notice_msgs_handle msg;
{
    notice_msgs_handle curr;

    if (notice->msg_info) {
	for (curr = notice->msg_info; curr; curr = curr->next)
	    if (curr->next == NULL) {
		curr->next = msg;
		break;
	    }
    } else
	notice->msg_info = msg;
}

/*
 * Create a button struct
 */
Pkg_private notice_buttons_handle
notice_create_button_struct()
{
    notice_buttons_handle pi = NULL;

    pi = (notice_buttons_handle) xv_calloc(1, sizeof(struct notice_buttons));
    if (!pi) {
	xv_error(XV_ZERO,
	         ERROR_STRING,
	     XV_MSG("calloc failed in notice_create_button_struct()."),
	         ERROR_PKG, NOTICE,
		 NULL);
    } else {
	pi->is_yes = (int)FALSE;
    }
    return pi;
}


/*
 * Create a button struct
 */
Pkg_private notice_msgs_handle
notice_create_msg_struct()
{
    notice_msgs_handle pi = NULL;

    pi = (notice_msgs_handle) xv_calloc(1, sizeof(struct notice_msgs));
    if (!pi) {
	xv_error(XV_ZERO,
	         ERROR_STRING,
	     XV_MSG("calloc failed in notice_create_msg_struct()."),
	         ERROR_PKG, NOTICE,
		 NULL);
    }
    return pi;
}

/*
 * Free a message struct
 */
Pkg_private void
notice_free_msg_structs(first)
    notice_msgs_handle first;
{
    notice_msgs_handle current;
    notice_msgs_handle next;

    if (!first)
	return;
    for (current = first; current != NULL; current = next) {
	next = current->next;
	free(current->string);

	if (current->panel_item)  {
	    xv_destroy(current->panel_item);
	}

	free(current);
    }
}

/*
 * Free a button struct
 */
Pkg_private void
notice_free_button_structs(first)
    notice_buttons_handle first;
{
    notice_buttons_handle current;
    notice_buttons_handle next;

    if (!first)
	return;
    for (current = first; current != NULL; current = next) {
	next = current->next;
	free(current->string);

	if (current->panel_item)  {
	    xv_destroy(current->panel_item);
	}

	free(current);
    }
}

Pkg_private void
notice_do_bell(notice)
    notice_handle   notice;
{
    Xv_Drawable_info	*info;
    struct timeval  wait;

    if (!notice_use_audible_bell)  {
	return;
    }

    DRAWABLE_INFO_MACRO(notice->client_window, info);
    wait.tv_sec = 0;
    wait.tv_usec = 100000;
    if (!notice->dont_beep && (notice->beeps > 0)) {
	int             i = notice->beeps;
	while (i--)
	    win_beep(xv_display(info), wait);
    }
}

/*
 * Update notice xy position
 */
Pkg_private int
notice_update_xy(notice)
Notice_info	*notice;
{
    Frame	sub_frame = notice->sub_frame;
    Rect	rect;

    frame_get_rect(sub_frame, &rect);
    frame_set_rect(sub_frame, &rect);
}

Pkg_private void
notice_button_panel_proc(item, event)
Panel_item	item;
Event		*event;
{
    Notice_info			*notice_info = (Notice_info *)xv_get(item, 
						XV_KEY_DATA, 
						notice_context_key, NULL);
    Xv_Notice			notice = NOTICE_PUBLIC(notice_info);
    struct notice_buttons	*buttons = notice_info->button_info;
    struct notice_buttons	*cur;


    /*
    * Determine value of button 
    */
    cur = buttons;
    while(cur)  {
        if (cur->panel_item == item)  {
            break;
        }
        else  {
            cur = cur->next;
        }
    }

    /*
     * Store the value in the result field of notice for
     * later retrieval
     * notice->result_ptr is the address specified by user via
     * NOTICE_STATUS
     */
    if (cur)  {
	notice_info->result = cur->value;
	if (notice_info->result_ptr)  {
	    *(notice_info->result_ptr) = cur->value;
	}
    }

    /*
     * Call notice event proc if one was specified
     */
    if ((void(*)())notice_info->event_proc)  {
	if (cur)  {
	    /*
	     * Call notice event proc
	     */
            (notice_info->event_proc)(notice, cur->value, event);
	}
    }

    /*
     * Pop down notice 
     */
    if (notice_info->block_thread)  {
        xv_window_return(XV_OK);
    }
    else  {
        xv_set(notice, XV_SHOW, FALSE, NULL);
    }
}

Pkg_private int
notice_get_owner_frame(notice)
Notice_info	*notice;
{
    Xv_window	client_window, owner_window;

    if (!notice)  {
	return(XV_ERROR);
    }

    owner_window = client_window = notice->client_window;

    if (!client_window)  {
	return(XV_ERROR);
    }

    /*
     * Check if client window is a frame.
     * If not, try to get it
     */
    if (!xv_get(client_window, XV_IS_SUBTYPE_OF, FRAME_CLASS))  {
	/*
	 * client_window not Frame
	 * Get it's WIN_FRAME
	 */
        owner_window = xv_get(client_window, WIN_FRAME);

	if (owner_window)  {
            if (!xv_get(owner_window, XV_IS_SUBTYPE_OF, FRAME_CLASS))  {
		/*
		 * If WIN_FRAME does not return a Frame
		 * reset to NULL so can continue search
		 */
		owner_window = (Frame)NULL;
	    }
	}

	if (!owner_window)  {
	    Frame	frame_obj;

	    /*
	     * Get WIN_FRAME as key data
	     * This case applies to menus
	     */
            owner_window = xv_get(client_window, XV_KEY_DATA, WIN_FRAME);

	    if (owner_window)  {
                if (!xv_get(owner_window, XV_IS_SUBTYPE_OF, FRAME_CLASS))  {
		    /*
		     * If KEY_DATA using WIN_FRAME does not return a Frame
		     * reset to NULL so can continue search
		     */
		    owner_window = (Frame)NULL;
	        }
	    }

	    if (!owner_window)  {
		/*
		 * Traverse XV_OWNER links until find Frame
		 */
	        for (frame_obj = xv_get(client_window, XV_OWNER);
		     frame_obj; 
		     frame_obj = xv_get(frame_obj, XV_OWNER) )  {
		    if (xv_get(frame_obj, XV_IS_SUBTYPE_OF, FRAME_CLASS))  {
			/*
			 * break out of loop when find a Frame
			 */
		        owner_window = frame_obj;
		        break;
		    }
	        }
	    }
	}

    }

    /*
     * BUG:
     * If cannot find a Frame as owner, use client_window
     */
    if (!owner_window)  {
	owner_window = client_window;
    }

    notice->owner_window = owner_window;

    return(XV_OK);
}
