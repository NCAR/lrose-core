#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)cnvs_cb.c 50.32 93/06/28";
#endif
#endif

#ifdef OW_I18N
/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE 
 *	file for terms of the license.
 */

#include <X11/Xatom.h>
#include <xview_private/cnvs_impl.h>
#include <xview_private/draw_impl.h>
#include <xview/frame.h>
#include <xview/panel.h>
#include <xview/font.h>
#include <xview/notice.h>

#define MAX_PREEDIT_CHAR         512	/* characters */
#define PANEL_TEXT_RIM             6	/* pixel */
#define INPUT_WINDOW_WIDTH        50	/* columns */


Xv_private void			cache_text_state();

Pkg_private void		canvas_pew_done();
Pkg_private Notify_value	canvas_pew_destory();

static Notify_value	canvas_pew_pi_event_proc();


Pkg_private void
canvas_preedit_start(ic, client_data, callback_data)
    XIC		ic;
    XPointer	client_data;		
    XPointer	callback_data;
{
    Canvas_info	*canvas;
    Canvas_pew	*pew;
    

    canvas = CANVAS_PRIVATE((Canvas) client_data);
    pew = canvas->pew;

    /*
     * Display the pew (PreEdit display Window).  And update the
     * status and counters.
     */
    xv_set(pew->frame, XV_SHOW, TRUE, NULL);
    pew->active_count++;
    status_set(canvas, preedit_exist);
    
    /*
     * start preediting with the input panel
     */
    panel_text_start(ic, pew->panel, NULL);
}


Pkg_private void
canvas_preedit_draw(ic, client_data, callback_data)
    XIC		ic;
    XPointer	client_data;
    XPointer	callback_data;
{
    Canvas_info 			*canvas;
    XIMPreeditDrawCallbackStruct 	*preedit;

    canvas = CANVAS_PRIVATE((Canvas) client_data);
    preedit = (XIMPreeditDrawCallbackStruct *) callback_data;

    /*
     * Can't use PANEL_PRIVATE macro here, can't include panel_impl.h
     * due to name clash between status element of panel_info and
     * canvas's status macro.
     */
    panel_preedit_display(
	((Xv_panel_or_item *) canvas->pew->ptxt)->private_data,
	preedit, FALSE);
}


Xv_private void 
cache_text_state(pre_edit, pe_cache)
XIMPreeditDrawCallbackStruct *pre_edit, *pe_cache;
{

    /*
     * length = length of updated text
     * chg_first = position in the cache where changes begin 
     * chg_length = number of chars in the cache effected
     * chg_last = last position in cache effected
     * tlength = length of new text
     *
     * we want to take all text from pe_cache up to chg_first, 
     * followed by the changed text, followed by any text
     * after chg_last in pe_cache and assemble them inorder in a buffer,
     * then replace the cache with the buffer.
     *
     * Whether the preedit text changes come in multibyte or
     * wide character form, this function will always return
     * the result in wide characters.  Since XView process
     * only wide characters internally so we are assuming
     * the cached preedit text is ALWAYS in wide characters.
     */

    XIMFeedback *feedback;	/* new structure for preedit screen feedback,
					will replace existing cache */
    int 	 i, idx;	/* Indices */
    wchar_t 	*text;		/* new structure for preedit data, will
					replace existing cache */
    unsigned short length = 0;
    unsigned short length_after = 0;
    int 	 tlength = 0;
    int 	 chg_first = pre_edit->chg_first;
    int 	 chg_length = pre_edit->chg_length;
    int 	 chg_last = chg_first + chg_length;

/*  For insertion or change case */

    if (pre_edit->text && (pre_edit->text->length > 0 )) {
	tlength = (int) pre_edit->text->length;
	length_after = pe_cache->text->length -  chg_last;
	length = chg_first + tlength + length_after;

	feedback = (XIMFeedback *) xv_alloc_n(XIMFeedback, length);
	text = (wchar_t *) xv_alloc_n(wchar_t, length+1);

	/* First copy the cached string up to changed area */
	for (i = 0; i < chg_first; i++) {
	    feedback[i] = pe_cache->text->feedback[i];
	    text[i] = pe_cache->text->string.wide_char[i];
	}
		    
	if (pre_edit->text->encoding_is_wchar) {
	    /* Now copy the new text */
	    for (idx = 0; idx < tlength; idx++) {
		feedback[i] = pre_edit->text->feedback[idx];
		text[i++] = pre_edit->text->string.wide_char[idx];
	    }
	} else {
	    wchar_t *tmp_str_wc;
	    /*  Convert the incoming preedit text from
	     *  multibyte to wide character.
	     */
		   
	    tmp_str_wc = _xv_mbstowcsdup (pre_edit->text->string.multi_byte);
	    for (idx = 0; idx < tlength; idx++) {
		feedback[i] = pre_edit->text->feedback[idx];
		text[i++] = tmp_str_wc[idx];
	    }
	    xv_free(tmp_str_wc);
	}
	/* finally, copy any cached text after the changed region */
	for (idx = chg_last; idx < (int) pe_cache->text->length; idx++) {
	    feedback[i] = pe_cache->text->feedback[idx];
	    text[i++] = pe_cache->text->string.wide_char[idx];
	}

	/* null-terminate the text array */
	text[length] = NULL;

	pe_cache->caret = pre_edit->caret;
	pe_cache->chg_first = 0;
	pe_cache->text->length = (unsigned short) length;
	pe_cache->chg_length = pe_cache->text->length;
	pe_cache->text->encoding_is_wchar = 1;
	if (pe_cache->text->string.wide_char)
	    xv_free(pe_cache->text->string.wide_char);
	pe_cache->text->string.wide_char = text;
	if (pe_cache->text->feedback)
	    xv_free(pe_cache->text->feedback);
	pe_cache->text->feedback = feedback;
    } else {
    /*  For the case of deletion or no change */
	if (chg_length > 0) {
	/* First copy the cached string up to changed area */
	    length = (pe_cache->chg_length >= chg_length) ? 
			pe_cache->chg_length - chg_length : 0;

	    feedback = (XIMFeedback *) xv_alloc_n(XIMFeedback, 
		length ? length : 1);
	    text = (wchar_t *) xv_alloc_n(wchar_t, length+1);
	    for (i = 0; i < chg_first; i++) {
		feedback[i] = pe_cache->text->feedback[i];
		text[i] = pe_cache->text->string.wide_char[i];
	    }
	    /* finally, copy any cached text after the changed region */
	    for (idx = chg_last; idx < (int) pe_cache->text->length; idx++) {
		feedback[i] = pe_cache->text->feedback[idx];
		text[i++] = pe_cache->text->string.wide_char[idx];
	    }

	    /* null-terminate the text array */

	    text[length] = NULL,

	    pe_cache->caret = pre_edit->caret;
	    pe_cache->chg_first = 0;
	    pe_cache->text->length = length;
	    pe_cache->chg_length = pe_cache->text->length;
	    pe_cache->text->encoding_is_wchar = 1;
	    if (pe_cache->text->string.wide_char)
		xv_free(pe_cache->text->string.wide_char);
	    pe_cache->text->string.wide_char = text;
	    if (pe_cache->text->feedback)
		xv_free(pe_cache->text->feedback);
	    pe_cache->text->feedback = feedback;
	}
    }
}


Pkg_private void
canvas_preedit_done(ic, client_data, callback_data)
XIC		ic;
XPointer	client_data;
XPointer	callback_data;

{
    Canvas_info		*canvas;
    Canvas_pew		*pew;

	
    canvas = CANVAS_PRIVATE((Canvas) client_data);
    pew = canvas->pew;
    if ((--canvas->pew->active_count) <= 0) {
	/*
	 * No other canvas's preedit is active, let's take down the
	 * preedit window if pin is out.
	 */
	if (xv_get(pew->frame, FRAME_CMD_PIN_STATE) == FRAME_CMD_PIN_OUT) {
	    xv_set(pew->frame, XV_SHOW, FALSE, NULL);
	}
    }
    status_reset(canvas, preedit_exist);

    panel_text_done(ic, pew->panel, NULL);
}


/* The following function is a stub. This will be implemented later */
Pkg_private void
canvas_preedit_caret(ic, client_data, callback_data)
XIC		ic;
XPointer	client_data;
XPointer	callback_data;
{
    /*
    interprete direction and calculate row and col;
    set cursor position
    */
}
		

/*
 * canvas_create_pew: Create the PreEdit Window and register itself to
 * the parent frame as XV_KEY_DATA.
 */
Pkg_private Canvas_pew *
canvas_create_pew(frame)
    Frame	   frame;
{
    Canvas_pew	     *pew;
    Xv_font	      font;
#ifndef PEW_DOES_NOTICE_ON_KBD_USE
    Xv_Drawable_info *info;
    Atom	      prop_array[1];
#endif


    pew = xv_alloc(Canvas_pew);
    font = (Xv_Font) xv_get(frame, XV_FONT);

    pew->frame = (Frame) xv_create(frame, FRAME_CMD,
            XV_LABEL,                   XV_MSG("Preedit Display"),
            WIN_USE_IM,                 FALSE,
            FRAME_CMD_POINTER_WARP,     FALSE,
            FRAME_SHOW_RESIZE_CORNER,   TRUE,
            FRAME_DONE_PROC,            canvas_pew_done,
	    WIN_WIDTH,          	INPUT_WINDOW_WIDTH,
            NULL);

#ifndef PEW_DOES_NOTICE_ON_KBD_USE
    /*
     * Following code can eliminate to have focus event completely.
     */
    DRAWABLE_INFO_MACRO(pew->frame, info);
    prop_array[0] = (Atom) xv_get(xv_server(info), SERVER_WM_DELETE_WINDOW);
    win_change_property(pew->frame, SERVER_WM_PROTOCOLS, XA_ATOM, 32,
							prop_array, 1);
#endif /* PEW_DOES_NOTICE_ON_KBD_USE */

    pew->panel = xv_get(pew->frame, FRAME_CMD_PANEL);

    xv_set(pew->panel,
        WIN_ROWS,           	1,
        WIN_COLUMNS,          	INPUT_WINDOW_WIDTH,
        WIN_IGNORE_X_EVENT_MASK,(KeyPress|KeyRelease
				 |ButtonPress|ButtonRelease),
        XV_FONT,    		font,
        NULL);
 
    notify_interpose_event_func(pew->panel, canvas_pew_pi_event_proc,
					   NOTIFY_SAFE);
    notify_interpose_destroy_func(pew->frame, canvas_pew_destory);
 
    pew->ptxt = (Panel_item) xv_create(pew->panel, PANEL_TEXT,
            PANEL_VALUE_DISPLAY_WIDTH,    INPUT_WINDOW_WIDTH - PANEL_TEXT_RIM,
            PANEL_VALUE_STORED_LENGTH_WCS,MAX_PREEDIT_CHAR,
            				  PANEL_VALUE_FONT, font,
            NULL);
 
    xv_set(frame,
        XV_KEY_DATA,		canvas_pew_key,	pew,
	NULL);

    xv_set(pew->frame,
	WIN_FIT_HEIGHT,		0,
	WIN_FIT_WIDTH,		0,
	NULL);

    return pew;
}


/*
 * canvas_pew_destory: Catch the destory of the preedit window, make
 * sure canvas will no longer reference to it.
 */
Pkg_private Notify_value
canvas_pew_destory(client, status)
    Notify_client	client;
    Destroy_status	status;
{
    if (status == DESTROY_CLEANUP) {
	Frame		 frame_public;
	Canvas_pew	*pew;

	frame_public =  (Frame) xv_get(client, XV_OWNER);
	if ((pew = (Canvas_pew *) xv_get(frame_public, XV_KEY_DATA,
						canvas_pew_key)) != NULL) {
	    xv_set(frame_public,
		   XV_KEY_DATA,	canvas_pew_key, NULL,
		   NULL);
	    xv_free(pew);
	}
    }
    return notify_next_destroy_func(client, status);
}


/*
 * canvas_pew_event_proc((Window)pew_win, (Event)event, arg) handles
 * the resizing of the canvas pew.  It changes the display length of
 * the text item.
 */
static Notify_value
canvas_pew_pi_event_proc(panel, event, arg, type)
    Notify_client	panel;
    Event		*event;
    Notify_arg		arg;
    Notify_event_type	type;
{
    Notify_value	ret;
    Panel_item		ip;
    int			width;
	

    switch ((Notify_event) event_action(event)) {
      case WIN_RESIZE:
      case WIN_REPAINT:
	ret = notify_next_event_func(panel, (Notify_event) event, arg, type);

	ip = (Panel_item) xv_get(panel, PANEL_FIRST_ITEM);

	width = (int) xv_get(panel, WIN_WIDTH) - PANEL_TEXT_RIM;
	xv_set(ip, PANEL_VALUE_DISPLAY_WIDTH, width, NULL);

	return ret;

#ifdef PEW_DOES_NOTICE_ON_KBD_USE
      /*
       * Bringup notice is desirable behavior, however this can not
       * done with current notice_prompt or notice pkg.  Because once
       * user hit "OK" button, notice will give input focus back to
       * the pew, and pew will see the KBD_USE event again, again, and
       * again....
       */
      case KBD_USE:

	notice_prompt(xv_get(panel, XV_OWNER), NULL,
			NOTICE_MESSAGE_STRING,	XV_MSG("\
You can not type in to the Preedit Display popup window,\n\
this is display only window, you should type in to the\n\
canvas window itself instead."),
                        NOTICE_BUTTON_YES, XV_MSG("OK"),
                        NULL);
	/*FALLTHROUGH*/

      case KBD_DONE:
	return NOTIFY_DONE;
#endif /* PEW_DOES_NOTICE_ON_KBD_USE */
    }

    return notify_next_event_func(panel, (Notify_event) event, arg, type);
}


/*
 * canvas_pew_event_proc: event interpose proc for the pew (in case of
 * the parent is FRAME_CMD).  Sync up pew with frame_cmd in case for
 * the open/close event.
 */
Pkg_private Notify_value
canvas_pew_event_proc(parent_win, event, arg, type)
    Window		parent_win;
    Event		*event;
    Notify_arg		arg;
    Notify_event_type	type;
{
    Canvas_pew		*pew;

    switch ((Notify_event) event_action(event)) {
      case ACTION_OPEN:
      case ACTION_CLOSE:
	pew = (Canvas_pew *) xv_get(parent_win, XV_KEY_DATA, canvas_pew_key);

	if ((Notify_event) event_action(event) == ACTION_CLOSE) {
	    xv_set(pew->frame, XV_SHOW, FALSE, NULL);

	} else {
	    if (pew->active_count > 0
	     || xv_get(pew->frame, FRAME_CMD_PIN_STATE) == FRAME_CMD_PIN_IN)
		xv_set(pew->frame, XV_SHOW, TRUE, NULL);
	}
	break;
    }
    return notify_next_event_func((Notify_client) parent_win,
				  (Notify_event) event, arg, type);
}


/*
 * canvas_pew_done: Make sure, we will not unmap the pew while there
 * are still active preedit session.
 */
Pkg_private void
canvas_pew_done(frame)
    Frame	frame; /* frame for preedit window */
{
    Canvas_pew		*pew;

    pew = (Canvas_pew *) xv_get(xv_get(frame, XV_OWNER),
				XV_KEY_DATA, canvas_pew_key);
    if (pew->active_count > 0) {

        notice_prompt(pew->frame, NULL,
		NOTICE_MESSAGE_STRING,	XV_MSG("\
The Preedit Display popup window cannot be\n\
dismissed while input method conversion is\n\
still on in one of the canvas windows."),
		NOTICE_BUTTON_YES,	XV_MSG("Continue"),
		NULL);
    } else
	xv_set(pew->frame, XV_SHOW, FALSE, NULL);
}
#endif /*OW_I18N*/
