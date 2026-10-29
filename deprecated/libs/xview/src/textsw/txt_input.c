#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)txt_input.c 20.109 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * User input interpreter for text subwindows.
 */

#include <xview_private/primal.h>
#include <xview_private/txt_impl.h>
#include <xview_private/ev_impl.h>
#include <xview_private/txt_18impl.h>
#include <errno.h>
/* Only needed for SHIFTMASK and CTRLMASK */
#include <pixrect/pr_util.h>

#ifdef __STDC__ 
#ifndef CAT
#define CAT(a,b)        a ## b 
#endif 
#endif
#include <pixrect/memvar.h>

#include <xview/notice.h>
#include <xview/frame.h>
#include <xview/win_struct.h>
#include <xview/win_input.h>	/* needed for event_shift_is_down() */
#include <xview/cursor.h>
#include <xview/screen.h>
#include <xview_private/win_keymap.h>
#ifdef SVR4 
#include <stdlib.h> 
#endif /* SVR4 */

extern int      errno;

Pkg_private Xv_Cursor move_cursor, dup_cursor;	/* laf */
Pkg_private void termsw_menu_set();
Pkg_private Key_map_handle textsw_do_filter();
Pkg_private Ev_finger_handle ev_add_finger();
Pkg_private struct pixrect *textsw_get_stopsign_icon();
Pkg_private struct pixrect *textsw_get_textedit_icon();
Pkg_private int textsw_load_done_proc();
Pkg_private void textsw_init_timer();
Pkg_private int textsw_timer_active;
Pkg_private void textsw_do_save();
static	void	textsw_do_undo();
static int textsw_do_newline();

#ifdef OW_I18N
Pkg_private     void textsw_implicit_commit_doit();
#endif

#define SPACE_CHAR 0x20

Pkg_private int
textsw_flush_caches(view, flags)
    register Textsw_view_handle view;
    register int    flags;
{
    register Textsw_folio textsw = FOLIO_FOR_VIEW(view);
    register int    count;
    register int    end_clear = (flags & TFC_SEL);

    count = (textsw->func_state & TXTSW_FUNC_FILTER)
	? 0
	: (textsw->to_insert_next_free - textsw->to_insert);
    if (flags & TFC_DO_PD) {
	if ((count > 0) || ((flags & TFC_PD_IFF_INSERT) == 0)) {
	    ev_set(view->e_view, EV_CHAIN_DELAY_UPDATE, TRUE, NULL);
	    (void) textsw_do_pending_delete(view, EV_SEL_PRIMARY,
					    end_clear | TFC_INSERT);
	    ev_set(view->e_view, EV_CHAIN_DELAY_UPDATE, FALSE, NULL);
	    end_clear = 0;
	}
    }
    if (end_clear) {
	if ((count > 0) || ((flags & TFC_SEL_IFF_INSERT) == 0)) {
	    (void) textsw_set_selection(
					VIEW_REP_TO_ABS(view),
				  ES_INFINITY, ES_INFINITY, EV_SEL_PRIMARY);
	}
    }
    if (flags & TFC_INSERT) {
	if (count > 0) {
	    /*
	     * WARNING!  The cache pointers must be updated BEFORE calling
	     * textsw_do_input, so that if the client is being notified of
	     * edits and it calls textsw_get, it will not trigger an infinite
	     * recursion of textsw_get calling textsw_flush_caches calling
	     * textsw_do_input calling the client calling textsw_get calling
	     * ...
	     */
	    textsw->to_insert_next_free = textsw->to_insert;
	    (void) textsw_do_input(view, textsw->to_insert, count,
				   TXTSW_UPDATE_SCROLLBAR_IF_NEEDED);
	}
    }
}

Pkg_private void
textsw_flush_std_caches(textsw)
    Textsw          textsw;
{
    Textsw_view_handle view = VIEW_ABS_TO_REP(textsw);

    (void) textsw_flush_caches(view, TFC_STD);

}


/*ARGSUSED*/
Pkg_private void
textsw_read_only_msg(textsw, locx, locy)
    Textsw_folio    textsw;
    int             locx, locy;
{
    Frame	frame = FRAME_FROM_FOLIO_OR_VIEW(textsw);
    Xv_Notice	text_notice;

    text_notice = (Xv_Notice)xv_get(frame, 
                                XV_KEY_DATA, text_notice_key, 
				NULL);
    if (!text_notice)  {
        text_notice = xv_create(frame, NOTICE,
                        NOTICE_LOCK_SCREEN, FALSE,
			NOTICE_BLOCK_THREAD, TRUE,
                        NOTICE_MESSAGE_STRINGS,
			XV_MSG("The text is read-only and cannot be edited.\n\
Press \"Continue\" to proceed."),
                        0,
                        NOTICE_BUTTON_YES, XV_MSG("Continue"),
                        XV_SHOW, TRUE,
                        NULL);

        xv_set(frame, 
                XV_KEY_DATA, text_notice_key, text_notice,
                NULL);
    }
    else  {
        xv_set(text_notice, 
                NOTICE_LOCK_SCREEN, FALSE,
		NOTICE_BLOCK_THREAD, TRUE,
                NOTICE_MESSAGE_STRINGS,
		XV_MSG("The text is read-only and cannot be edited.\n\
Press \"Continue\" to proceed."),
                0,
                NOTICE_BUTTON_YES, XV_MSG("Continue"),
                XV_SHOW, TRUE, 
                NULL);
    }
}

Pkg_private int
textsw_note_event_shifts(textsw, ie)
    register Textsw_folio textsw;
    register struct inputevent *ie;
{
    int             result = 0;

    if (ie->ie_shiftmask & SHIFTMASK)
	textsw->state |= TXTSW_SHIFT_DOWN;
    else {
#ifdef VT_100_HACK
	if (textsw->state & TXTSW_SHIFT_DOWN) {
	    /* Hack for VT-100 keyboard until PIT is available. */
	    result = 1;
	}
#endif
	textsw->state &= ~TXTSW_SHIFT_DOWN;
    }
    if (ie->ie_shiftmask & CTRLMASK)
	textsw->state |= TXTSW_CONTROL_DOWN;
    else
	textsw->state &= ~TXTSW_CONTROL_DOWN;
    return (result);
}

#ifdef GPROF
static void
textsw_gprofed_routine(view, ie)
    register Textsw_view_handle view;
    register Event *ie;
{
}

#endif

    static int      textsw_scroll_event();
    static int      textsw_function_key_event();
    static int      textsw_mouse_event();
    static int      textsw_edit_function_key_event();
    static int      textsw_caret_motion_event();
    static int      textsw_field_event();
    static int      textsw_file_operation();
    static int      textsw_erase_action();


Pkg_private int
textsw_process_event(view_public, ie, arg)
    Textsw_view     view_public;
    register Event *ie;
    Notify_arg      arg;
{
    Pkg_private void     textsw_update_scrollbars();

    int             caret_was_up;
    int             result = TEXTSW_PE_USED;
    register Textsw_view_handle view = VIEW_PRIVATE(view_public);
    register Textsw_folio textsw = FOLIO_FOR_VIEW(view);
    int             action = event_action(ie);
    register int    down_event = event_is_down(ie);
#ifdef OW_I18N
    char	    byte;
#endif

    caret_was_up = textsw->caret_state & TXTSW_CARET_ON;
    /* Watch out for caret turds */
    if ((action == LOC_MOVE
	 || action == LOC_WINENTER
	 || action == LOC_WINEXIT)
	&& !TXTSW_IS_BUSY(textsw)) {
	/* leave caret up */
    } else {
	textsw_take_down_caret(textsw);
    }
    switch (textsw_note_event_shifts(textsw, ie)) {
#ifdef VT_100_HACK
      case 1:
	if (textsw->func_state & TXTSW_FUNC_GET) {
	    textsw_end_get(view);
	}
#endif
      default:
	break;
    }

#ifdef OW_I18N    
    if (event_is_string(ie) && down_event && textsw->ic) {
	int	num_of_char = mbstowcs(textsw->to_insert_next_free, event_string(ie), ((TXTSW_UI_BUFLEN - (textsw->to_insert_next_free - textsw->to_insert)) - 1));
	textsw->to_insert_next_free += num_of_char;

	if (!textsw->preedit_start)
	    textsw_flush_caches(view, TFC_STD);
	else {	/* This is for partial commit */
	    Es_index	mark_pos, old_insert_pos, tmp;

	    /* Set the insertion position front of the preedit text */
	    old_insert_pos = EV_GET_INSERT(textsw->views);
	    mark_pos = textsw_find_mark_internal(textsw, textsw->preedit_start);
	    EV_SET_INSERT(textsw->views, mark_pos, tmp);
	    ev_remove_all_op_bdry(textsw->views, mark_pos, mark_pos,
			EV_SEL_PRIMARY | EV_SEL_SECONDARY, EV_BDRY_TYPE_ONLY);

	    /* Resume undo and again for commited text */
            textsw->state &= ~(TXTSW_NO_UNDO_RECORDING | TXTSW_NO_AGAIN_RECORDING);
	    /* Insert the commited text */
	    textsw_flush_caches(view, TFC_STD);

	    /* Turn off undo and again */
	    textsw->state |= (TXTSW_NO_UNDO_RECORDING | TXTSW_NO_AGAIN_RECORDING);
	    /* Update preedit_start front of the preedit text */
	    textsw_remove_mark_internal(textsw, textsw->preedit_start);
	    textsw->preedit_start = textsw_add_mark_internal(textsw,
					EV_GET_INSERT(textsw->views), NULL);

	    /* Reset the insertion position after the preedit text */
	    EV_SET_INSERT(textsw->views, old_insert_pos + num_of_char, tmp);
	}
	if ((textsw->state & TXTSW_EDITED) == 0)
	    textsw_possibly_edited_now_notify(textsw);
	goto Done;
    }
#endif /* OW_I18N */
    /* NOTE: This is just a hack for performance */
    if ((action >= SPACE_CHAR) && (action <= ASCII_LAST))
	goto Process_ASCII;


    if (action == TXTSW_STOP) {
	textsw_abort(textsw);
    } else if (action == ACTION_HELP || action == ACTION_MORE_HELP ||
	       action == ACTION_TEXT_HELP || action == ACTION_MORE_TEXT_HELP ||
	       action == ACTION_INPUT_FOCUS_HELP) {
	if (down_event)
	    xv_help_show(WINDOW_FROM_VIEW(view),
			 xv_get(view_public, XV_HELP_DATA), ie);
#ifdef GPROF
    } else if (action == KEY_RIGHT(13)) {
	if (down_event) {
	    if (textsw->state & TXTSW_SHIFT_DOWN) {
		moncontrol(0);
	    }
	} else {
	    if ((textsw->state & TXTSW_SHIFT_DOWN) == 0) {
		moncontrol(1);
	    }
	}
    } else if (action == KEY_RIGHT(15)) {
	if (down_event) {
	} else {
	    moncontrol(1);
	    textsw_gprofed_routine(view, ie);
	    moncontrol(0);
	}
#endif
    } else if (event_action(ie) == WIN_RESIZE) {
	    textsw_resize(view);
	    ev_set(view->e_view, EV_NO_REPAINT_TIL_EVENT, TRUE, NULL);
	    textsw_display_view(VIEW_REP_TO_ABS(view), (Rect*) 0);
	    ev_set(view->e_view, EV_NO_REPAINT_TIL_EVENT, FALSE, NULL);
    } else if (textsw_mouseless_scroll_event(view, ie, arg)) {
	    /* taken care of -- new mouseless function  jcb 12/21/90 */
    } else if (textsw_mouseless_select_event( view, ie, arg)) {
	    /* taken care of -- new mouseless function	jcb 1/2/91 */
    } else if (textsw_mouseless_misc_event( view, ie)) {
	    /* taken care of -- new mouseless function	jcb 1/2/91 */
    } else if (textsw_scroll_event(view, ie, arg)) {
	/* Its already taken care by the procedure */

    } else if ((textsw->track_state &
	     (TXTSW_TRACK_ADJUST | TXTSW_TRACK_POINT | TXTSW_TRACK_WIPE)) &&
	       textsw_track_selection(view, ie)) {
	/*
	 * Selection tracking and function keys. textsw_track_selection() always
	 * called if tracking.  It consumes the event unless function key up
	 * while tracking secondary selection, which stops tracking and then
	 * does function.
	 */
    } else if (textsw_function_key_event(view, ie, &result)) {
	/* Its already taken care by the procedure */

    } else if (textsw_mouse_event(view, ie)) {
	/* Its already taken care by the procedure */

    } else if (textsw_edit_function_key_event(view, ie, &result)) {
	if (result & TEXTSW_PE_READ_ONLY)
	    goto Read_Only;

    } else if (textsw_caret_motion_event(view, ie)) {
	/* Its already taken care by the procedure */

    } else if (action == TXTSW_CAPS_LOCK) {
	if (TXTSW_IS_READ_ONLY(textsw))
	    goto Read_Only;
	if (!down_event) {
	    textsw->state ^= TXTSW_CAPS_LOCK_ON;
	    textsw_notify(view, TEXTSW_ACTION_CAPS_LOCK,
			  (textsw->state & TXTSW_CAPS_LOCK_ON), NULL);
	}
	/*
	 * Type-in
	 */
    } else if (textsw->track_state & TXTSW_TRACK_SECONDARY) {
	/* No type-in processing during secondary (function) selections */
    } else if (textsw_field_event(view, ie)) {
	/* Its already taken care by the procedure */
    } else if ((action == TXTSW_EMPTY_DOCUMENT) && down_event) {
	textsw_empty_document(view_public, ie);
	goto Done;
    } else if (textsw_file_operation(view_public, ie)) {
	if (action == TXTSW_LOAD_FILE_AS_MENU)
	    goto Done;

    } else if (textsw_erase_action(view_public, ie)) {
	if (TXTSW_IS_READ_ONLY(textsw))
	    goto Read_Only;
	else
    	    if ((textsw->state & TXTSW_EDITED) == 0)
		textsw_possibly_edited_now_notify(textsw);
	    goto Done;
    } if ((action <= ISO_LAST) &&
	  down_event) {

	if (textsw->func_state & TXTSW_FUNC_FILTER) {
	    if (textsw->to_insert_next_free <
		textsw->to_insert + sizeof(textsw->to_insert)) {
		*textsw->to_insert_next_free++ = (char) action;
	    }
	} else {
    Process_ASCII:
	    switch (action) {
	      case (short) '\r':	/* Fall through */
	      case (short) '\n':
		if (down_event) {
		    if (TXTSW_IS_READ_ONLY(textsw))
			goto Read_Only;
#ifdef OW_I18N
		    /* This is a transaction for bug 1090046. */
		    if (textsw->preedit_start)
			textsw->blocking_newline = TRUE;
		    else
#endif
                    { /* 1030878 */
                        if (textsw->state & TXTSW_DIFF_CR_LF)
                            (void) textsw_do_newline(view, action );
                        else
                            (void) textsw_do_newline(view, '\n' );
                    }
		}
		break;
	      default:
		if (TXTSW_IS_READ_ONLY(textsw))
		    goto Read_Only;
		if (!down_event)
		    break;
		if (textsw->state & TXTSW_CAPS_LOCK_ON) {
		    if ((char) action >= 'a' &&
			(char) action <= 'z')
			ie->ie_code += 'A' - 'a';
		    /* BUG ALERT: above may need to set event_id */
		}
#ifdef OW_I18N
		/*
		 * Asian Textsw dosen't handle NULL character as current spec.
		 * If the spec is changed to handle NULL character, following
		 * statement should be removed.
		 */
		if (action == NULL)
		    break;
		byte = event_action(ie);
#ifdef sun
		if (isascii(byte))
		    /*
		     * Type casting wchar_t is not quite right thing
		     * to do, however we know ASCII works in Sun.
		     */
		    *textsw->to_insert_next_free++ = byte;
		else
#endif
		    mbtowc(textsw->to_insert_next_free++, &byte, 1);
#else /* OW_I18N */
		*textsw->to_insert_next_free++ =
		    (char) event_action(ie);
#endif /* OW_I18N */
		if (textsw->to_insert_next_free ==
		    textsw->to_insert +
		    sizeof(textsw->to_insert)) {
		    textsw_flush_caches(view, TFC_STD);
		}
		break;
	    }
	}
	/*
	 * User filters
	 */
    } else if (textsw_do_filter(view, ie)) {
	/*
	 * Miscellaneous
	 */
    } else {
	result &= ~TEXTSW_PE_USED;
    }
    /*
     * Cleanup
     */
    if ((textsw->state & TXTSW_EDITED) == 0)
	textsw_possibly_edited_now_notify(textsw);
Done:
    if (TXTSW_IS_BUSY(textsw))
	result |= TEXTSW_PE_BUSY;
    return (result);
Read_Only:
    result |= TEXTSW_PE_READ_ONLY;
    goto Done;
}

static          Key_map_handle
find_key_map(textsw, ie)
    register Textsw_folio textsw;
    register Event *ie;
{
    register Key_map_handle current_key = textsw->key_maps;

    while (current_key) {
	if (current_key->event_code == event_action(ie)) {
	    break;
	}
	current_key = current_key->next;
    }
    return (current_key);
}

Pkg_private     Key_map_handle
textsw_do_filter(view, ie)
    register Textsw_view_handle view;
    register Event *ie;
{
    register Textsw_folio textsw = FOLIO_FOR_VIEW(view);
    register Key_map_handle result = find_key_map(textsw, ie);
    Frame	frame;
    Xv_Notice	text_notice;

    if (result == 0)
	goto Return;
    if (event_is_down(ie)) {
	switch (result->type) {
	  case TXTSW_KEY_SMART_FILTER:
	  case TXTSW_KEY_FILTER:
	    textsw_flush_caches(view, TFC_STD);
	    textsw->func_state |= TXTSW_FUNC_FILTER;
	    result = 0;
	    break;
	}
	goto Return;
    }
    switch (result->type) {
      case TXTSW_KEY_SMART_FILTER:
      case TXTSW_KEY_FILTER:{
	    Pkg_private int      textsw_call_smart_filter();
	    Pkg_private int      textsw_call_filter();
	    int             again_state, filter_result;

	    again_state = textsw->func_state & TXTSW_FUNC_AGAIN;
	    (void) textsw_record_filter(textsw, ie);
	    textsw->func_state |= TXTSW_FUNC_AGAIN;
	    textsw_checkpoint_undo(VIEW_REP_TO_ABS(view),
				   (caddr_t) TEXTSW_INFINITY - 1);
	    if (result->type == TXTSW_KEY_SMART_FILTER) {
		filter_result = textsw_call_smart_filter(view, ie,
						 (char **) result->maps_to);
	    } else {
		filter_result = textsw_call_filter(view,
						 (char **) result->maps_to);
	    }
	    textsw_checkpoint_undo(VIEW_REP_TO_ABS(view),
				   (caddr_t) TEXTSW_INFINITY - 1);
	    switch (filter_result) {
	      case 0:
	      default:
		break;
	      case 1:{
		    char            msg[300];
		    if (errno == ENOENT) {
			(void) sprintf(msg, 
			XV_MSG("Cannot locate filter '%s'."),
				       ((char **) result->maps_to)[0]);
		    } else {
			(void) sprintf(msg, 
			XV_MSG("Unexpected problem with filter '%s'."),
				       ((char **) result->maps_to)[0]);
		    }
		    frame = xv_get(VIEW_REP_TO_ABS(view), WIN_FRAME);
                    text_notice = (Xv_Notice)xv_get(frame, 
                                    XV_KEY_DATA, text_notice_key, 
				    NULL);

                    if (!text_notice)  {
                        text_notice = xv_create(frame, NOTICE,
                            NOTICE_LOCK_SCREEN, FALSE,
			    NOTICE_BLOCK_THREAD, TRUE,
                            NOTICE_MESSAGE_STRINGS, msg, 0,
                            NOTICE_BUTTON_YES, 
				XV_MSG("Continue"),
                            XV_SHOW, TRUE,
                            NULL);

                        xv_set(frame, 
                            XV_KEY_DATA, text_notice_key, text_notice,
                            NULL);
                    }
                    else  {
                        xv_set(text_notice, 
                            NOTICE_LOCK_SCREEN, FALSE,
			    NOTICE_BLOCK_THREAD, TRUE,
                            NOTICE_MESSAGE_STRINGS, msg, 0,
                            NOTICE_BUTTON_YES, 
				XV_MSG("Continue"),
                            XV_SHOW, TRUE, 
                            NULL);
                    }
		    break;
		}
	    }
	    textsw->func_state &= ~TXTSW_FUNC_FILTER;
	    textsw->to_insert_next_free = textsw->to_insert;
	    if (again_state == 0)
		textsw->func_state &= ~TXTSW_FUNC_AGAIN;
	    result = 0;
	}
    }
Return:
    return (result);
}


#ifdef OW_I18N
Pkg_private	int
#else
static int
#endif
textsw_do_newline(view, action)
    register Textsw_view_handle view;
    int action;
{
    register Textsw_folio folio = FOLIO_FOR_VIEW(view);
    int             delta;
    Es_index        first = EV_GET_INSERT(folio->views), last_plus_one,
                    previous;
    CHAR            newline_str[2];
    
    newline_str[0] = action; /* 1030878 */
    newline_str[1] = XV_ZERO;

    textsw_flush_caches(view, TFC_INSERT | TFC_PD_SEL);
    if (folio->state & TXTSW_AUTO_INDENT)
	first = EV_GET_INSERT(folio->views);
    delta = textsw_do_input(view, newline_str, 1, TXTSW_UPDATE_SCROLLBAR);
    if (folio->state & TXTSW_AUTO_INDENT) {
	previous = first;
	textsw_find_pattern(folio, &previous, &last_plus_one,
			    newline_str, 1, EV_FIND_BACKWARD);
	if (previous != ES_CANNOT_SET) {
	    CHAR            buf[100];
	    struct es_buf_object esbuf;
	    register CHAR  *c;

	    esbuf.esh = folio->views->esh;
	    esbuf.buf = buf;
	    esbuf.sizeof_buf = SIZEOF(buf);
	    if (es_make_buf_include_index(&esbuf, previous, 0) == 0) {
		if (AN_ERROR(buf[0] != '\n')) {
		} else {
		    for (c = buf + 1; c < buf + SIZEOF(buf); c++) {
			switch (*c) {
			  case '\t':
			  case ' ':
			    break;
			  default:
			    goto Did_Scan;
			}
		    }
	    Did_Scan:
		    if (c != buf + 1) {
			delta += textsw_do_input(view, buf + 1,
						 (int) (c - buf - 1),
					  TXTSW_UPDATE_SCROLLBAR_IF_NEEDED);
		    }
		}
	    }
	}
    }
    return (delta);
}

Pkg_private     Es_index
textsw_get_saved_insert(textsw)
    register Textsw_folio textsw;
{
    Ev_finger_handle saved_insert_finger;

    saved_insert_finger = ev_find_finger(
			      &textsw->views->fingers, textsw->save_insert);
    if (saved_insert_finger)
	return(saved_insert_finger->pos);
    else {
	/*
	 * Don't believe that this triple check is necessary, but since
	 * we are making this change without a full working knowledge of
	 * the circumstances under which this routine can be called, let's
	 * be over safe.
	 */
	if (!textsw->first_view ||
	  !textsw->first_view->e_view ||
	  !textsw->first_view->e_view->view_chain)
	    return(ES_INFINITY);
	else {
	    Ev_chain_pd_handle chain_private =
		EV_CHAIN_PRIVATE(textsw->first_view->e_view->view_chain);

	    return(chain_private->insert_pos);
	}
    }
}

Pkg_private int
textsw_clear_pending_func_state(textsw)
    register Textsw_folio textsw;
{
    if (!EV_MARK_IS_NULL(&textsw->save_insert)) {
	if (textsw->func_state & TXTSW_FUNC_PUT) {
	    Es_index        old_insert = textsw_get_saved_insert(textsw);
	    if AN_ERROR
		(old_insert == ES_INFINITY) {
	    } else {
		textsw_set_insert(textsw, old_insert);
	    }
	} else
	    ASSUME(textsw->func_state & TXTSW_FUNC_GET);
	ev_remove_finger(&textsw->views->fingers, textsw->save_insert);
	EV_INIT_MARK(textsw->save_insert);
    }
    if (textsw->func_state & TXTSW_FUNC_FILTER) {
	textsw->to_insert_next_free = textsw->to_insert;
    }
    textsw->func_state &= ~(TXTSW_FUNC_ALL | TXTSW_FUNC_EXECUTE);
}

/*
 * ==========================================================
 * 
 * Input mask initialization and setting.
 * 
 * ==========================================================
 */

static struct inputmask basemask_kbd;
#ifdef SUNVIEW1
static struct inputmask basemask_pick;
#endif

static int      masks_have_been_initialized;	/* Defaults to FALSE */

static void
setupmasks()
{
    register struct inputmask *mask;
    register int    i;

    /*
     * Set up the standard kbd mask.
     */
    mask = &basemask_kbd;
    input_imnull(mask);
    mask->im_flags |= IM_ASCII | IM_NEGEVENT | IM_META | IM_NEGMETA;
    for (i = 1; i < 17; i++) {
	win_setinputcodebit(mask, KEY_LEFT(i));
	win_setinputcodebit(mask, KEY_TOP(i));
	win_setinputcodebit(mask, KEY_RIGHT(i));
    }
    /* Unset TOP and OPEN because will look for them in pick mask */
    /*
     * Use win_keymap_*inputcodebig() for events that are semantic, i.e.,
     * have no actual inputmask bit assignment.  This is done in
     * textsw_set_base_mask() below since we need the fd to know what keymap
     * to look in.
     */
    win_setinputcodebit(mask, KBD_USE);
    win_setinputcodebit(mask, KBD_DONE);
#ifdef VT_100_HACK
    win_setinputcodebit(mask, SHIFT_LEFT);	/* Pick up the shift */
    win_setinputcodebit(mask, SHIFT_RIGHT);	/* keys for VT-100 */
    win_setinputcodebit(mask, SHIFT_LOCK);	/* compatibility */
#endif
#ifdef SUNVIEW1
    /*
     * Set up the standard pick mask.
     */
    mask = &basemask_pick;
    input_imnull(mask);
#endif
    win_setinputcodebit(mask, WIN_STOP);
    /*
     * LOC_WINENTER & LOC_WINEXIT not needed since click-to-type is the
     * default. Will be needed when follow-cursor is implemented.
     */
    /*
     * win_setinputcodebit(mask, LOC_WINENTER); win_setinputcodebit(mask,
     * LOC_WINEXIT);
     */
    win_setinputcodebit(mask, LOC_DRAG);
    win_setinputcodebit(mask, MS_LEFT);
    win_setinputcodebit(mask, MS_MIDDLE);
    win_setinputcodebit(mask, MS_RIGHT);
    win_setinputcodebit(mask, WIN_REPAINT);
    win_setinputcodebit(mask, WIN_VISIBILITY_NOTIFY);
    mask->im_flags |= IM_NEGEVENT;
    masks_have_been_initialized = TRUE;
}

Pkg_private int
textsw_set_base_mask(win)
    Xv_object       win;
{

    if (masks_have_been_initialized == FALSE) {
	setupmasks();
    }
    win_keymap_set_smask_class(win, KEYMAP_FUNCT_KEYS);
    win_keymap_set_smask_class(win, KEYMAP_EDIT_KEYS);
    win_keymap_set_smask_class(win, KEYMAP_MOTION_KEYS);
    win_keymap_set_smask_class(win, KEYMAP_TEXT_KEYS);

    win_setinputmask(win, &basemask_kbd, 0, 0);

    xv_set(win, WIN_CONSUME_EVENT, ACTION_HELP, NULL);

}

/*
 * ==========================================================
 * 
 * Functions invoked by function keys.
 * 
 * ==========================================================
 */

Pkg_private void
textsw_begin_function(view, function)
    Textsw_view_handle view;
    unsigned        function;
{
    register Textsw_folio folio = FOLIO_FOR_VIEW(view);

    textsw_flush_caches(view, TFC_STD);
    if ((folio->state & TXTSW_CONTROL_DOWN) &&
	!TXTSW_IS_READ_ONLY(folio))
	folio->state |= TXTSW_PENDING_DELETE;
    folio->track_state |= TXTSW_TRACK_SECONDARY;
    folio->func_state |= function | TXTSW_FUNC_EXECUTE;
    ASSUME(EV_MARK_IS_NULL(&folio->save_insert));
    EV_MARK_SET_MOVE_AT_INSERT(folio->save_insert);
    ev_add_finger(&folio->views->fingers, EV_GET_INSERT(folio->views),
		  0, &folio->save_insert);
    textsw_init_timer(folio);
    if AN_ERROR
	(folio->func_state & TXTSW_FUNC_SVC_SAW(function))
	/* Following covers up inconsistent state with Seln. Svc. */
	    folio->func_state &= ~TXTSW_FUNC_SVC_SAW(function);
}

Pkg_private void
textsw_init_timer(folio)
    Textsw_folio    folio;
{
    /*
     * Make last_point/_adjust/_ie_time close (but not too close) to current
     * time to avoid overflow in tests for multi-click.
     */
    folio->last_point.tv_sec -= 1000;
    folio->last_adjust = folio->last_point;
    folio->last_ie_time = folio->last_point;
}


Pkg_private void
textsw_end_function(view, function)
    Textsw_view_handle view;
    unsigned        function;
{
    Pkg_private void textsw_end_selection_function();
    register Textsw_folio folio = FOLIO_FOR_VIEW(view);

    /* restore insertion point */
    if (!EV_MARK_IS_NULL(&folio->save_insert)) {
	ev_remove_finger(&folio->views->fingers, folio->save_insert);
	EV_INIT_MARK(folio->save_insert);
    }
    folio->state &= ~TXTSW_PENDING_DELETE;
    folio->track_state &= ~TXTSW_TRACK_SECONDARY;
    folio->func_state &=
	~(function | TXTSW_FUNC_SVC_SAW(function) | TXTSW_FUNC_EXECUTE);
    textsw_end_selection_function(folio);
}

static
textsw_begin_again(view)
    Textsw_view_handle view;
{
#ifdef OW_I18N
    /*
     * Hopefully, wanted to put it right before textsw_do_again(),
     * but textsw_begin_function() makes another checkpoint
     * by calling textsw_checkpoint_again() for AGAIN action.
     */
    textsw_implicit_commit(FOLIO_FOR_VIEW(view));
#endif
    textsw_begin_function(view, TXTSW_FUNC_AGAIN);
}

static
textsw_end_again(view, x, y)
    Textsw_view_handle view;
    int             x, y;
{
    textsw_do_again(view, x, y);
    textsw_end_function(view, TXTSW_FUNC_AGAIN);
    /*
     * Make sure each sequence of again will not accumulate. Like find,
     * delete and insert.
     */
    textsw_checkpoint_again(VIEW_REP_TO_ABS(view));
}

Pkg_private int
textsw_again(view, x, y)
    Textsw_view_handle view;
    int             x, y;
{
    textsw_begin_again(view);
    textsw_end_again(view, x, y);
}

static
textsw_begin_delete(view)
    Textsw_view_handle view;
{
    register Textsw_folio folio = FOLIO_FOR_VIEW(view);

    textsw_begin_function(view, TXTSW_FUNC_DELETE);
    if (!TXTSW_IS_READ_ONLY(folio))
	folio->state |= TXTSW_PENDING_DELETE;
    /* Force pending-delete feedback as it is implicit in DELETE */
    (void) textsw_inform_seln_svc(folio, TXTSW_FUNC_DELETE, TRUE);
}

Pkg_private int
textsw_end_delete(view)
    Textsw_view_handle view;
{
    Pkg_private void     textsw_init_selection_object();
    Pkg_private void     textsw_clear_secondary_selection();
    Textsw_selection_object selection;
    int             result = 0;
    register Textsw_folio folio = FOLIO_FOR_VIEW(view);
    int             temp;
#ifdef OW_I18N
    CHAR            dummy[1];
#endif


    (void) textsw_inform_seln_svc(folio, TXTSW_FUNC_DELETE, FALSE);
    if ((folio->func_state & TXTSW_FUNC_DELETE) == 0)
	return (0);
    if ((folio->func_state & TXTSW_FUNC_EXECUTE) == 0)
	goto Done;
#ifdef OW_I18N
    dummy[0] = NULL;
    textsw_init_selection_object(folio, &selection, dummy, 0, FALSE);
#else
    textsw_init_selection_object(folio, &selection, "", 0, FALSE);
#endif
    if (TFS_IS_ERROR(textsw_func_selection(folio, &selection, 0)))
	goto Done;
    if (selection.type & TFS_IS_SELF) {
	temp = textsw_adjust_delete_span(folio, &selection.first,
					 &selection.last_plus_one);
	switch (temp) {
	  case TEXTSW_PE_READ_ONLY:
	    textsw_clear_secondary_selection(folio, EV_SEL_SECONDARY);
	    result = TEXTSW_PE_READ_ONLY;
	    break;
	  case TXTSW_PE_EMPTY_INTERVAL:
	    break;
	  case TXTSW_PE_ADJUSTED:
	    textsw_set_selection(VIEW_REP_TO_ABS(folio->first_view),
				 ES_INFINITY, ES_INFINITY, selection.type);
	    /* Fall through to delete remaining span */
	  default:
	    textsw_checkpoint_undo(VIEW_REP_TO_ABS(view),
				   (caddr_t) TEXTSW_INFINITY - 1);

	    (void) textsw_delete_span(
			     view, selection.first, selection.last_plus_one,
			       (unsigned) ((selection.type & EV_SEL_PRIMARY)
					 ? TXTSW_DS_RECORD | TXTSW_DS_SHELVE
					   : TXTSW_DS_SHELVE));
	    textsw_checkpoint_undo(VIEW_REP_TO_ABS(view),
				   (caddr_t) TEXTSW_INFINITY - 1);
	    break;
	}
    }
Done:
    textsw_end_function(view, TXTSW_FUNC_DELETE);
    textsw_update_scrollbars(folio, TEXTSW_VIEW_NULL);
    return (result);
}

Pkg_private int
textsw_function_delete(view)
    Textsw_view_handle view;
{
    int             result;

    textsw_begin_delete(view);
    result = textsw_end_delete(view);
    return (result);
}

static void
textsw_begin_undo(view)
    Textsw_view_handle view;
{
    textsw_begin_function(view, TXTSW_FUNC_UNDO);
    textsw_flush_caches(view, TFC_SEL);
}

static void
textsw_end_undo(view)
    Textsw_view_handle view;
{
#ifdef OW_I18N
    textsw_implicit_commit(FOLIO_FOR_VIEW(view));
#endif
    textsw_do_undo(view);
    textsw_end_function(view, TXTSW_FUNC_UNDO);
    textsw_update_scrollbars(FOLIO_FOR_VIEW(view), TEXTSW_VIEW_NULL);
}

static void
textsw_undo_notify(folio, start, delta)
    register Textsw_folio folio;
    register Es_index start, delta;
{
    Pkg_private void     textsw_notify_replaced();
    register Ev_chain chain = folio->views;
    register Es_index old_length =
    es_get_length(chain->esh) - delta;
    Es_index        old_insert, temp;

    if (folio->notify_level & TEXTSW_NOTIFY_EDIT)
	old_insert = EV_GET_INSERT(chain);
    EV_SET_INSERT(chain, ((delta > 0) ? start + delta : start), temp);
    ev_update_after_edit(chain,
			 (delta > 0) ? start : start - delta,
			 delta, old_length, start);
    if (folio->notify_level & TEXTSW_NOTIFY_EDIT) {
	textsw_notify_replaced((Textsw_opaque) folio->first_view,
			       old_insert, old_length,
			       (delta > 0) ? start : start + delta,
			       (delta > 0) ? start + delta : start,
			       (delta > 0) ? delta : 0);
    }
    textsw_checkpoint(folio);
}

static void
textsw_do_undo(view)
    Textsw_view_handle view;
{
    Pkg_private int      ev_remove_finger();
    Pkg_private Es_index textsw_set_insert();
    register Textsw_folio folio = FOLIO_FOR_VIEW(view);
    register Ev_finger_handle saved_insert_finger;
    register Ev_chain views = folio->views;
    Ev_mark_object  save_insert;

    if (!TXTSW_DO_UNDO(folio))
	return;
    if (folio->undo[0] == es_get(views->esh, ES_UNDO_MARK)) {
	/*
	 * Undo followed immediately by another undo. Note:
	 * textsw_set_internal guarantees that folio->undo_count != 1
	 */
	XV_BCOPY((caddr_t) & folio->undo[1], (caddr_t) & folio->undo[0],
	      (int) ((folio->undo_count - 2) * sizeof(folio->undo[0])));
	folio->undo[folio->undo_count - 1] = ES_NULL_UNDO_MARK;
    }
    if (folio->undo[0] == ES_NULL_UNDO_MARK)
	return;
    /* Undo the changes to the piece source. */
    (void) ev_add_finger(&views->fingers, EV_GET_INSERT(views), 0,
			 &save_insert);

    ev_set(view->e_view, EV_CHAIN_DELAY_UPDATE, TRUE, NULL);
    es_set(views->esh,
	   ES_UNDO_NOTIFY_PAIR, textsw_undo_notify, (caddr_t) folio,
	   ES_UNDO_MARK, folio->undo[0],
	   NULL);
    ev_set(view->e_view, EV_CHAIN_DELAY_UPDATE, FALSE, NULL);
    ev_update_chain_display(views);
    saved_insert_finger =
	ev_find_finger(&views->fingers, save_insert);
    if AN_ERROR
	(saved_insert_finger == 0) {
    } else {
	(void) textsw_set_insert(folio, saved_insert_finger->pos);
	ev_remove_finger(&views->fingers, save_insert);
    }
    /* Get the new mark. */
    folio->undo[0] = es_get(views->esh, ES_UNDO_MARK);
    /* Check to see if this has undone all edits to the folio. */
    if (textsw_has_been_modified(VIEW_REP_TO_ABS(folio->first_view))
	== 0) {
	CHAR           *name;
	if (textsw_file_name(folio, &name) == 0) {
#ifdef OW_I18N
	    char	*name_mb = _xv_wcstombsdup(name);

	    textsw_notify(view, TEXTSW_ACTION_LOADED_FILE, name_mb,
				TEXTSW_ACTION_LOADED_FILE_WCS, name, NULL);
	    if (name_mb)
		free(name_mb);
#else
	    textsw_notify(view, TEXTSW_ACTION_LOADED_FILE, name, NULL);
#endif
	}
	folio->state &= ~TXTSW_EDITED;
	if (folio->menu && folio->sub_menu_table)
	    xv_set(folio->sub_menu_table[(int) TXTSW_FILE_SUB_MENU], 
		   MENU_DEFAULT, 1, NULL);	

    }
}

Pkg_private int
textsw_undo(textsw)
    Textsw_folio    textsw;
{
    textsw_begin_undo(textsw->first_view);
    textsw_end_undo(textsw->first_view);
}

static int
textsw_scroll_event(view, ie, arg)
    register Textsw_view_handle view;
    register Event *ie;
    Notify_arg      arg;
{
    Textsw_folio    folio = FOLIO_FOR_VIEW(view);
    int             is_scroll_event = FALSE;
    Pkg_private void     textsw_update_scrollbars();
    int             action = event_action(ie);

    if (action == SCROLLBAR_REQUEST) {
	is_scroll_event = TRUE;

	/*
	 * this does both the scroll and the bar update. The call to
	 * textsw_scroll()
	 */
	/*
	 * was removed, but this breaks the "Last Position" scrollbar logic
	 * !! jcb
	 */
	textsw_scroll((Scrollbar) arg);

	textsw_update_scrollbars(folio, TEXTSW_VIEW_NULL);
    }
    return (is_scroll_event);
}

static int
textsw_function_key_event(view, ie, result)
    register Textsw_view_handle view;
    register Event *ie;
    int            *result;
{
    Textsw_folio    folio = FOLIO_FOR_VIEW(view);
    int             is_function_key_event = FALSE;
    register int    action = event_action(ie);
    register int    down_event = event_is_down(ie);


    if (action == TXTSW_AGAIN) {
	is_function_key_event = TRUE;
	if (down_event) {
	    textsw_begin_again(view);
	} else if (folio->func_state & TXTSW_FUNC_AGAIN) {
	    textsw_end_again(view, ie->ie_locx, ie->ie_locy);
	}
#ifdef VT_100_HACK
    } else if (action == TXTSW_AGAIN) {
	is_function_key_event = TRUE;
	/* Bug in releases through K3 only generates down, no up */
	if (down_event) {
	    textsw_begin_again(view);
	    textsw_end_again(view, ie->ie_locx, ie->ie_locy);
	} else if (folio->func_state & TXTSW_FUNC_AGAIN) {

	}
#endif
    } else if (action == TXTSW_UNDO) {
	is_function_key_event = TRUE;
	if (TXTSW_IS_READ_ONLY(folio)) {
	    *result |= TEXTSW_PE_READ_ONLY;
	}
	if (down_event) {
	    textsw_begin_undo(view);
	} else if (folio->func_state & TXTSW_FUNC_UNDO) {
	    textsw_end_undo(view);
	}
#ifdef VT_100_HACK
    } else if (action == TXTSW_UNDO) {
	/* Bug in releases through K3 only generates down, no up */
	is_function_key_event = TRUE;
	if (down_event) {
	    textsw_begin_undo(view);
	    textsw_end_undo(view);
	} else if (folio->func_state & TXTSW_FUNC_UNDO) {

	}
#endif
    } else if ((action == TXTSW_TOP) ||
	       (action == TXTSW_BOTTOM) ||
	       (action == TXTSW_OPEN) ||
	       (action == TXTSW_PROPS)) {
	is_function_key_event = TRUE;
	/* These key should only work on up event */
	if (!down_event)
	    textsw_notify(view, TEXTSW_ACTION_TOOL_MGR, ie, NULL);
    } else if ((action == TXTSW_FIND_FORWARD) ||
	       (action == TXTSW_FIND_BACKWARD) ||
	       (action == TXTSW_REPLACE)) {
	is_function_key_event = TRUE;

	if (down_event) {
	    textsw_begin_find(view);
	    folio->func_x = ie->ie_locx;
	    folio->func_y = ie->ie_locy;
	    folio->func_view = view;
	} else
	    textsw_end_find(view, action, ie->ie_locx, ie->ie_locy);
    }
    return (is_function_key_event);
}


static void
textsw_set_copy_or_quick_move_cursor(folio)
    register Textsw_folio folio;
{
    if (folio->holder_state & TXTSW_HOLDER_OF_CARET) {
	/* Local case */
	if (folio->func_state & TXTSW_FUNC_DELETE) {
	    folio->track_state |= TXTSW_TRACK_QUICK_MOVE;
	}
    } else {
	/* Remote case */
	Xv_object       screen = xv_get(FOLIO_REP_TO_ABS(folio), XV_SCREEN);
	Xv_object       server = xv_get(screen, SCREEN_SERVER);

	if (server_get_seln_function_pending(server)) {
	    Seln_holder     holder;
	    char           *data;

	    holder = seln_inquire(SELN_CARET);
	    if (holder.state != SELN_NONE) {
		Seln_request   *result;

		result = seln_ask(&holder,
				  SELN_REQ_FUNC_KEY_STATE, 0,
				  NULL);
		data = (char *)result->data;
		data += sizeof(Seln_attribute);

		if ((int) SELN_FN_DELETE == *(int *) data) {
		    folio->track_state |= TXTSW_TRACK_QUICK_MOVE;
		}
	    }
	}
    }
}

static int
textsw_mouse_event(view, ie)
    register Textsw_view_handle view;
    register Event *ie;
{
    Textsw_folio    folio = FOLIO_FOR_VIEW(view);
    int             is_mouse_event = TRUE;
    register int    action = event_action(ie);
    register int    down_event = event_is_down(ie);
    static int      point_down_within_selection;
    static short    dnd_last_click_x = 0, dnd_last_click_y = 0; 

    switch (action) {
      case TXTSW_POINT:{
	    Es_index        first, last_plus_one, pos;
	    int             delta;

	    if (event_is_down(ie)) {
#ifdef VT_100_HACK
		if ((folio->state & TXTSW_SHIFT_DOWN) &&
		    (textsw->func_state & TXTSW_FUNC_ALL) == 0) {
		    /* Hack for VT-100 keyboard until PIT is available. */
		    textsw_begin_get(view);
		    folio->func_x = ie->ie_locx;
		    folio->func_y = ie->ie_locy;
		    folio->func_view = view;
		    break;
		}
#endif
		textsw_set_copy_or_quick_move_cursor(folio);
		(void) ev_get_selection(folio->views, &first, &last_plus_one, EV_SEL_PRIMARY);
		pos = ev_resolve_xy(view->e_view, event_x(ie), event_y(ie));
		dnd_last_click_x = ie->ie_locx;
		dnd_last_click_y = ie->ie_locy;
		delta = (ie->ie_time.tv_sec - folio->last_point.tv_sec) * 1000;
		delta += ie->ie_time.tv_usec / 1000;
		delta -= folio->last_point.tv_usec / 1000;
		point_down_within_selection = ((pos >= first && pos < last_plus_one) && (delta >= folio->multi_click_timeout));
#ifdef OW_I18N
		/* commit implicitly only when mouse is single click */
		if (delta >= folio->multi_click_timeout)
		    textsw_implicit_commit(folio);
#endif
		if (!point_down_within_selection)
		    textsw_start_seln_tracking(view, ie);
	    } else if (point_down_within_selection) {
		/* If point down is within a selection, then look at mouse up */
		textsw_start_seln_tracking(view, ie);
		textsw_track_selection(view, ie);
		point_down_within_selection = FALSE;
		/* textsw_invert_caret(folio);  */
	    }
	    /* Discard negative events that get to here, as state is wrong. */
	    break;
	}
      case TXTSW_ADJUST:
	if (down_event) {
#ifdef OW_I18N
	    textsw_implicit_commit(folio);
#endif
	    textsw_start_seln_tracking(view, ie);
	}
	/* Discard negative events that get to here, as state is wrong. */
	break;
      case LOC_DRAG:{
	    Es_index        first, last_plus_one, pos;
#ifdef OW_I18N
	    /*
	     * LOC_DRAG always occur afer TXTSW_POINT.
	     * It does not need to call textsw_implicit_commit() here.
	     */
#endif
	    (void) ev_get_selection(folio->views, &first, &last_plus_one, EV_SEL_PRIMARY);
	    pos = ev_resolve_xy(view->e_view, event_x(ie), event_y(ie));

	    if (pos >= first && pos < last_plus_one)
		if ((short)(abs(event_x(ie) - dnd_last_click_x))
                                                > folio->drag_threshold
                        || (short)(abs(event_y(ie) - dnd_last_click_y))
                                                > folio->drag_threshold)
		     textsw_do_drag_copy_move(view, ie, event_ctrl_is_down(ie));
	    break;
	}
      case LOC_WINENTER:
	/* This is done for performance improvment */
	folio->state |= TXTSW_DELAY_SEL_INQUIRE;
	break;
      case LOC_WINEXIT:
      case KBD_DONE:
	textsw_may_win_exit(folio);
	break;
	/* Menus */
      case TXTSW_MENU:
	if (down_event) {
	    Pkg_private          textsw_do_menu();
	    textsw_flush_caches(view, TFC_STD);

	    textsw_do_menu(view, ie);
	}
	break;
      case ACTION_DRAG_MOVE:
#ifdef OW_I18N
	/* Event is always down event */
	textsw_implicit_commit(folio);
#endif
	textsw_do_remote_drag_copy_move(view, ie, (short) 0);
	break;
      case ACTION_DRAG_COPY:
#ifdef OW_I18N
	/* Event is always down event */
	textsw_implicit_commit(folio);
#endif
	textsw_do_remote_drag_copy_move(view, ie, (short) 1);
	break;
      default:
	is_mouse_event = FALSE;
	break;
    }

    return (is_mouse_event);
}

static int
textsw_edit_function_key_event(view, ie, result)
    register Textsw_view_handle view;
    register Event *ie;
    int            *result;
{
    Textsw_folio    folio = FOLIO_FOR_VIEW(view);
    int             is_edit_function_key_event = FALSE;
    register int    action = event_action(ie);
    register int    down_event = event_is_down(ie);
    /*
     *  BIG BUG:  should check for conversion on.
     */

    if (action == TXTSW_DELETE) {
	is_edit_function_key_event = TRUE;
	if (down_event) {
	    textsw_begin_delete(view);
	    folio->func_x = ie->ie_locx;
	    folio->func_y = ie->ie_locy;
	    folio->func_view = view;
	} else {

	    if ((folio->track_state & TXTSW_TRACK_QUICK_MOVE) ||
		textsw_is_seln_nonzero(folio, EV_SEL_SECONDARY)) {
		*result |= textsw_end_quick_move(view);
	    } else
		*result |= textsw_end_delete(view);
	}
    } else if (action == TXTSW_GET) {
	is_edit_function_key_event = TRUE;
	if (down_event) {
	    textsw_begin_get(view);
	    folio->func_x = ie->ie_locx;
	    folio->func_y = ie->ie_locy;
	    folio->func_view = view;
	} else {
	    *result |= textsw_end_get(view);
	}
    } else if (action == TXTSW_PUT) {
	is_edit_function_key_event = TRUE;
	if (down_event) {
	    textsw_begin_put(view, TRUE);
	    folio->func_x = ie->ie_locx;
	    folio->func_y = ie->ie_locy;
	    folio->func_view = view;
	} else {
	    *result |= textsw_end_put(view);
	}
    }
    return (is_edit_function_key_event);
}

static int
textsw_caret_motion_event(view, ie)
    register Textsw_view_handle view;
    register Event *ie;
{
    int             is_caret_motion_event = TRUE;
    register int    action = event_action(ie);

    Pkg_private void textsw_move_down_a_line();
    Pkg_private void textsw_move_up_a_line();
    Pkg_private void textsw_move_forward_a_word();
    Pkg_private void textsw_move_backward_a_word();
    Pkg_private void textsw_move_to_line_end();
    Pkg_private void textsw_move_to_line_start();
    Pkg_private void textsw_move_caret();

    if (event_is_up(ie))
	return (FALSE);

    switch (action) {
      case TXTSW_MOVE_RIGHT:
#ifdef OW_I18N
    textsw_implicit_commit(FOLIO_FOR_VIEW(view));
#endif
	textsw_move_caret(view, TXTSW_CHAR_FORWARD);
	break;
      case TXTSW_MOVE_LEFT:
#ifdef OW_I18N
    textsw_implicit_commit(FOLIO_FOR_VIEW(view));
#endif
	textsw_move_caret(view, TXTSW_CHAR_BACKWARD);
	break;
      case TXTSW_MOVE_UP:
#ifdef OW_I18N
    textsw_implicit_commit(FOLIO_FOR_VIEW(view));
#endif
	textsw_move_caret(view, TXTSW_PREVIOUS_LINE);
	break;
      case TXTSW_MOVE_DOWN:
#ifdef OW_I18N
    textsw_implicit_commit(FOLIO_FOR_VIEW(view));
#endif
	textsw_move_caret(view, TXTSW_NEXT_LINE);
	break;
      case TXTSW_MOVE_WORD_BACKWARD:
#ifdef OW_I18N
    textsw_implicit_commit(FOLIO_FOR_VIEW(view));
#endif
	textsw_move_caret(view, TXTSW_WORD_BACKWARD);
	break;
      case TXTSW_MOVE_WORD_FORWARD:
#ifdef OW_I18N
    textsw_implicit_commit(FOLIO_FOR_VIEW(view));
#endif
	textsw_move_caret(view, TXTSW_WORD_FORWARD);
	break;
      case TXTSW_MOVE_WORD_END:
#ifdef OW_I18N
    textsw_implicit_commit(FOLIO_FOR_VIEW(view));
#endif
	textsw_move_caret(view, TXTSW_WORD_END);
	break;
      case TXTSW_MOVE_TO_LINE_START:
#ifdef OW_I18N
    textsw_implicit_commit(FOLIO_FOR_VIEW(view));
#endif
	textsw_move_caret(view, TXTSW_LINE_START);
	break;
      case TXTSW_MOVE_TO_LINE_END:
#ifdef OW_I18N
    textsw_implicit_commit(FOLIO_FOR_VIEW(view));
#endif
	textsw_move_caret(view, TXTSW_LINE_END);
	break;
      case TXTSW_MOVE_TO_NEXT_LINE_START:
#ifdef OW_I18N
    textsw_implicit_commit(FOLIO_FOR_VIEW(view));
#endif
	textsw_move_caret(view, TXTSW_NEXT_LINE_START);
	break;
      case TXTSW_MOVE_TO_DOC_START:
#ifdef OW_I18N
    textsw_implicit_commit(FOLIO_FOR_VIEW(view));
#endif
	textsw_move_caret(view, TXTSW_DOCUMENT_START);
	break;
      case TXTSW_MOVE_TO_DOC_END:
#ifdef OW_I18N
    textsw_implicit_commit(FOLIO_FOR_VIEW(view));
#endif
	textsw_move_caret(view, TXTSW_DOCUMENT_END);
	break;
      default:
	is_caret_motion_event = FALSE;
	break;

    }
    if (is_caret_motion_event)
	textsw_set_selection(VIEW_REP_TO_ABS(view),
			     ES_INFINITY, ES_INFINITY, EV_SEL_PRIMARY);


    return (is_caret_motion_event);
}

static int
textsw_field_event(view, ie)
    register Textsw_view_handle view;
    register Event *ie;
{
    register int    action = event_action(ie);
    register int    down_event = event_is_down(ie);
    int             is_field_event = FALSE;

    if (action == TXTSW_NEXT_FIELD) {
#ifdef OW_I18N
	static CHAR bar_gt[] = { '|', '>', 0 };
#endif
	is_field_event = TRUE;

	if (down_event) {
	    textsw_flush_caches(view, TFC_STD);
#ifdef OW_I18N
	    (void) textsw_match_selection_and_normalize(view, bar_gt,
#else
	    (void) textsw_match_selection_and_normalize(view, "|>",
#endif
						      TEXTSW_FIELD_FORWARD);
	}
    } else if (action == TXTSW_PREV_FIELD) {
#ifdef OW_I18N
	static CHAR bar_lt[] = { '<', '|',  0 };
#endif
	is_field_event = TRUE;

	if (down_event) {
	    textsw_flush_caches(view, TFC_STD);
#ifdef OW_I18N
	    (void) textsw_match_selection_and_normalize(view, bar_lt,
#else
	    (void) textsw_match_selection_and_normalize(view, "<|",
#endif
						     TEXTSW_FIELD_BACKWARD);
	}
    } else if (action == TXTSW_MATCH_DELIMITER) {
	CHAR           *start_marker = NULL;

	is_field_event = TRUE;
	if (down_event) {
	    textsw_flush_caches(view, TFC_STD);
	    (void) textsw_match_selection_and_normalize(view, start_marker,
							TEXTSW_NOT_A_FIELD);
	}
    }
    return (is_field_event);
}


static int
textsw_file_operation(abstract, ie)
    register Textsw abstract;
    register Event *ie;
{
    int             is_file_op_event = FALSE;
    Textsw_view_handle view = VIEW_ABS_TO_REP(abstract);
    register Textsw_folio folio = FOLIO_FOR_VIEW(view);
    register int    action = event_action(ie);
    register int    down_event = event_is_down(ie);
    Frame	frame;
    Xv_Notice	text_notice;

    if (action == TXTSW_LOAD_FILE_AS_MENU) {
	Pkg_private int      LOAD_FILE_POPUP_KEY;
	Frame           base_frame, popup;

	is_file_op_event = TRUE;
	if (down_event) {
	    if (folio->state & TXTSW_NO_LOAD) {
		frame = FRAME_FROM_FOLIO_OR_VIEW(folio);
                text_notice = (Xv_Notice)xv_get(frame, 
                                XV_KEY_DATA, text_notice_key, 
				NULL);
                if (!text_notice)  {
                    text_notice = xv_create(frame, NOTICE,
                        NOTICE_LOCK_SCREEN, FALSE,
			NOTICE_BLOCK_THREAD, TRUE,
                        NOTICE_MESSAGE_STRINGS,
			XV_MSG("Illegal Operation.\n\
Load File Has Been Disabled."),
                        0,
                        NOTICE_BUTTON_YES, XV_MSG("Continue"),
                        XV_SHOW, TRUE,
                        NULL);

                    xv_set(frame, 
                            XV_KEY_DATA, text_notice_key, text_notice,
                            NULL);
                }
                else  {
                    xv_set(text_notice, 
                            NOTICE_LOCK_SCREEN, FALSE,
			    NOTICE_BLOCK_THREAD, TRUE,
                            NOTICE_MESSAGE_STRINGS,
			    XV_MSG("Illegal Operation.\n\
Load File Has Been Disabled."),
                            0,
                            NOTICE_BUTTON_YES, XV_MSG("Continue"),
                            XV_SHOW, TRUE, 
                            NULL);
                }

		return is_file_op_event;
	    }
	    base_frame = (Frame) xv_get(abstract, WIN_FRAME);
	    popup = (Frame) xv_get(base_frame, XV_KEY_DATA, LOAD_FILE_POPUP_KEY);
	    if (popup) {
		(void) textsw_set_dir_str((int) TEXTSW_MENU_LOAD);
		(void) textsw_get_and_set_selection(popup, view, (int) TEXTSW_MENU_LOAD);
	    } else {
		(void) textsw_create_popup_frame(view, (int) TEXTSW_MENU_LOAD);
	    }
	}
	is_file_op_event = TRUE;

    } else if (action == TXTSW_STORE_FILE) {
	if (down_event) {
	    textsw_do_save(abstract, folio, view);
	}
	is_file_op_event = TRUE;

    } else if (action == TXTSW_INCLUDE_FILE) {
	int             primary_selection_exists;
	Pkg_private int      FILE_STUFF_POPUP_KEY;
	Frame           base_frame, popup;

	if (down_event) {
	    primary_selection_exists = textsw_is_seln_nonzero(folio,
							    EV_SEL_PRIMARY);
	    if (primary_selection_exists) {
		base_frame = (Frame) xv_get(abstract, WIN_FRAME);
		popup = (Frame) xv_get(base_frame, XV_KEY_DATA, FILE_STUFF_POPUP_KEY);
		if (popup) {
		    (void) textsw_set_dir_str((int) TEXTSW_MENU_FILE_STUFF);
		    (void) textsw_get_and_set_selection(popup, view,
					      (int) TEXTSW_MENU_FILE_STUFF);
		} else {
		    (void) textsw_create_popup_frame(view, (int) TEXTSW_MENU_FILE_STUFF);
		}
	    } else {
		(void) textsw_post_need_selection(abstract, ie);
	    }
	}
	is_file_op_event = TRUE;
    }
    return (is_file_op_event);
}

static int
textsw_erase_action(textsw, ie)
    Textsw          textsw;
    Event          *ie;
{

    Textsw_view_handle view = VIEW_ABS_TO_REP(textsw);
    Textsw_folio    folio = FOLIO_FOR_VIEW(view);
    unsigned        edit_unit;
    int             direction = 0 /* forward direction */ ;
    register int    action = event_action(ie);
    int             result;        /* for EDIT_CHAR deletes selection */
    Es_index        first, lpo;

    switch (action) {
      case ACTION_ERASE_CHAR_BACKWARD:
	edit_unit = EV_EDIT_CHAR;
	direction = EV_EDIT_BACK;
	break;
      case ACTION_ERASE_CHAR_FORWARD:
	edit_unit = EV_EDIT_CHAR;
	break;
      case ACTION_ERASE_WORD_BACKWARD:
	edit_unit = EV_EDIT_WORD;
	direction = EV_EDIT_BACK;
	break;
      case ACTION_ERASE_WORD_FORWARD:
	edit_unit = EV_EDIT_WORD;
	break;
      case ACTION_ERASE_LINE_BACKWARD:
	edit_unit = EV_EDIT_LINE;
	direction = EV_EDIT_BACK;
	break;
      case ACTION_ERASE_LINE_END:
	edit_unit = EV_EDIT_LINE;
	break;
      default:
	edit_unit = 0;
    }

    if (edit_unit == 0) {
	int             id = event_id(ie);

	if (id == (int) folio->edit_bk_char) {
	    edit_unit = EV_EDIT_CHAR;
	    if ((folio->state & TXTSW_SHIFT_DOWN) == 0)
		direction = EV_EDIT_BACK;
	} else if (id == (int) folio->edit_bk_word) {
	    edit_unit = EV_EDIT_WORD;
	    if ((folio->state & TXTSW_SHIFT_DOWN) == 0)
		direction = EV_EDIT_BACK;
	} else if (id == (int) folio->edit_bk_line) {
	    edit_unit = EV_EDIT_LINE;
	    if ((folio->state & TXTSW_SHIFT_DOWN) == 0)
		direction = EV_EDIT_BACK;
	}
    }
    if ((edit_unit == 0) || (TXTSW_IS_READ_ONLY(folio)) ||
	(event_is_up(ie)))
	return (edit_unit);
    /* Delete selection only for single character delete.
       1) Delete the selection for single character edits only if
          there is a primary selection.
       2) If the delete is not a single character delete, then
          clear any primary selections and do the erase action.
       3) Make sure view and scrollbar are updated.
    */

    result = ev_get_selection(folio->views,&first,&lpo,EV_SEL_PRIMARY);

    /* Delete the selection for single character edits. */
    if ((edit_unit == EV_EDIT_CHAR) &&  /* single character */
	((EV_SEL_PRIMARY & result) || (EV_SEL_PD_PRIMARY & result)) &&
	(first < lpo)) {                /* selection has a valid range */
	textsw_flush_caches(view, TFC_DO_PD);
	/* this makes the selected text actually disappear */
	ev_update_chain_display(folio->views); 
    } else { /* don't delete the selection. Do the erase action */
	/* clear any primary selections */
	if (((EV_SEL_PRIMARY & result) || (EV_SEL_PD_PRIMARY & result)) &&
	    (first < lpo)) {
	    textsw_set_selection(textsw,
				 ES_INFINITY, ES_INFINITY, EV_SEL_PRIMARY);
	    /* this makes the selected text actually disappear */
	    ev_update_chain_display(folio->views); 
	}
	textsw_flush_caches(view,TFC_INSERT | TFC_PD_IFF_INSERT | TFC_SEL);
#ifdef OW_I18N
	(void) textsw_do_edit(view, edit_unit, direction, 0);
#else
	(void) textsw_do_edit(view, edit_unit, direction);
#endif
    }
    /* scrollbar updating needed esp. for large deletes */
    textsw_update_scrollbars(folio, TEXTSW_VIEW_NULL);
    return (TRUE);
}

#ifdef OW_I18N
#define	TXT_INSERT_LEN(x) ((TXTSW_UI_BUFLEN - \
			    ((x)->to_insert_next_free - (x)->to_insert)) - 1)
/*
 * textsw package interface to archive implicit commit.
 * Assumption:  1. IC is valid
 *		2. conversion mode is ON
 *		3. preedit text exist.
 *   So, textsw_implicit_commit_doit() has to be called when
 *   folio->preedit_start is non NULL case.
 */
Pkg_private	void
textsw_implicit_commit_doit(folio)
    register Textsw_folio	folio;
{
    register Textsw		textsw_public = TEXTSW_PUBLIC(folio);
    char		       *committed_string = 0;

    xv_set(textsw_public, WIN_IC_RESET, NULL);
    committed_string = (char *)xv_get(textsw_public, WIN_IC_COMMIT_STRING);
    xv_set(textsw_public, WIN_IC_CONVERSION, TRUE, NULL);

    /*
     *  Workaroud for 1092682 and 1094340
     *  If preedit_draw() for erasing the preedit text (after now call it as
     *  preedit_erasing) isn't called yet, call it in order for any processing
     *  after textsw_implicit_commit_doit(). The real preedit_erasing sent by
     *  mle (Htt) will process nothing.
     *  This is a hack for such mle. mle should send the preedit_erasing before
     *  the committed text, when IC is reseted.
     */
    if (folio->preedit_start) { /* preedit text dosen't erase yet. */
	XIMPreeditDrawCallbackStruct	callback_data;

	callback_data.chg_first = 0;
	callback_data.chg_length = EV_GET_INSERT(folio->views) -
			textsw_find_mark_internal(folio, folio->preedit_start);
	callback_data.text = (XIMText *)0;
	textsw_pre_edit_draw(folio->ic, (XPointer)FOLIO_REP_TO_ABS(folio),
			     &callback_data);
    }

    /* insert committed text into window if exists */
    if (committed_string != (char *)NULL && *committed_string != '\0') {
	int	num_of_char = mbstowcs(folio->to_insert_next_free,
				       committed_string,
				       TXT_INSERT_LEN(folio));
	folio->to_insert_next_free += num_of_char;
	textsw_flush_caches(VIEW_FROM_FOLIO_OR_VIEW(folio), TFC_STD);

	if ((folio->state & TXTSW_EDITED) == 0)
	    textsw_possibly_edited_now_notify(folio);
    }
}
#endif /* OW_I18N */
