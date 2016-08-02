#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)txt_move.c 20.91 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * Text subwindow move & duplicate
 */

#include <xview_private/primal.h>
#include <xview_private/txt_impl.h>
#include <xview_private/ev_impl.h>
#include <xview_private/draw_impl.h>
#include <xview_private/txt_18impl.h>
#include <pixrect/pr_util.h>
#include <pixrect/memvar.h>
#include <pixrect/pixfont.h>
#include <xview/cursor.h>
#include <xview/font.h>
#include <xview/pkg.h>
#include <xview/attrol.h>
#include <xview/screen.h>
#include <xview/seln.h>
#include <xview/fullscreen.h>
#include <xview/svrimage.h>
#include <xview/server.h>
#include <xview/win_struct.h>
#include <xview/pixwin.h>
#include <xview/dragdrop.h>
#include <xview/notice.h>
#ifdef SVR4 
#include <stdlib.h> 
#endif /* SVR4 */

#ifdef OW_I18N		/* for reducing #ifdef OW_I18N */
#define TEXTSW_CONTENTS_I18N		TEXTSW_CONTENTS_WCS
#define textsw_insert_i18n		textsw_insert_wcs
#define textsw_set_selection_i18n	textsw_set_selection_wcs
#define textsw_delete_i18n		textsw_delete_wcs;
#define TEXTSW_INSERTION_POINT_I18N	TEXTSW_INSERTION_POINT_WC
#else
#define TEXTSW_CONTENTS_I18N		TEXTSW_CONTENTS
#define textsw_insert_i18n		textsw_insert
#define textsw_set_selection_i18n	textsw_set_selection
#define textsw_delete_i18n		textsw_delete;
#define TEXTSW_INSERTION_POINT_I18N	TEXTSW_INSERTION_POINT
#endif /* OW_I18N */

static int dnd_data_key = 0; /* XXX: Don't do this at home kids. */
static int dnd_view_key = 0; 
static int DndConvertProc();

Pkg_private Es_handle textsw_esh_for_span();
Pkg_private Es_index ev_resolve_xy();
Pkg_private Es_index textsw_do_balance_beam();
Pkg_private int ev_get_selection();
Pkg_private int xv_pf_text();
Pkg_private int textsw_end_quick_move();
Pkg_private void textsw_finish_move();
Pkg_private void textsw_finish_duplicate();
Pkg_private void textsw_reset_cursor();

static unsigned short    drag_move_arrow_data[] = {
#include <images/text_move_cursor.pr>
};

mpr_static(drag_move_arrow_pr, 16, 16, 1, drag_move_arrow_data);

struct  textsw_context {
    int    size;
    char   *sel_buffer;
};


Pkg_private	void
textsw_save_selection(folio)
    Textsw_folio    folio;
{
    static          repeat_call;

    if (!repeat_call) {
	(void) ev_get_selection(folio->views,
			     &folio->move_first, &folio->move_last_plus_one,
				EV_SEL_PRIMARY);
	repeat_call = TRUE;
    }
}


Pkg_private int
textsw_do_move(view, selection_is_local)
    Textsw_view_handle view;
    int             selection_is_local;
{
    register Textsw_folio folio = FOLIO_FOR_VIEW(view);
    Es_index        delta, first, last_plus_one, ro_bdry, pos;
    register Es_handle secondary = ES_NULL;
    register Ev_chain chain = folio->views;
    register int    is_pending_delete;
    CHAR           *string;
    int             lower_context =
    (int) ev_get(view->e_view, EV_CHAIN_LOWER_CONTEXT);

    /*
     * First, pre-process the primary selection.
     */
    ev_set(view->e_view, EV_CHAIN_DELAY_UPDATE, TRUE, NULL);
    ro_bdry = textsw_read_only_boundary_is_at(folio);

    is_pending_delete = (EV_SEL_PENDING_DELETE & ev_get_selection(
			    chain, &first, &last_plus_one, EV_SEL_PRIMARY));

    if ((first < last_plus_one) && is_pending_delete) {
	/*
	 * A non-empty pending-delete primary selection exists. It must be
	 * the contents of the trashbin when we are done.
	 */
	if (folio->state & TXTSW_DELETE_REPLACES_CLIPBOARD) {
	    secondary = folio->trash;	/* Recycle old trash pieces */
	    folio->trash = textsw_esh_for_span(view, first, last_plus_one, 
					       ES_NULL);
	} else {
	    secondary = textsw_esh_for_span(view, first, last_plus_one, 
					    ES_NULL);
	}
	pos = last_plus_one;
    } else {
	secondary = ES_NULL;
	pos = EV_GET_INSERT(folio->views);
    }

    if (pos < ro_bdry) {
	textsw_clear_secondary_selection(folio, EV_SEL_SECONDARY);
	return (TEXTSW_PE_READ_ONLY);
    }
    /*
     * Completely process local secondary selection.
     */
    if (selection_is_local) {
	ev_get_selection(chain, &first, &last_plus_one, EV_SEL_SECONDARY);
	if (last_plus_one <= ro_bdry) {
	    textsw_clear_secondary_selection(folio, EV_SEL_SECONDARY);
	    return (TEXTSW_PE_READ_ONLY);
	}
	secondary = textsw_esh_for_span(
				     view, first, last_plus_one, secondary);
	if (folio->state & TXTSW_DELETE_REPLACES_CLIPBOARD) {
	    textsw_delete_span(view, (first < ro_bdry) ? ro_bdry : first,
			       last_plus_one, TXTSW_DS_SHELVE);
	} else {
	    textsw_delete_span(view, (first < ro_bdry) ? ro_bdry : first,
			       last_plus_one, NULL);
	}
	if (first != ES_INFINITY)
	    textsw_set_selection(VIEW_REP_TO_ABS(view),
				 ES_INFINITY, ES_INFINITY, EV_SEL_SECONDARY);
    } else {
	Seln_holder     holder;
	Seln_request   *result;
	char           *data;
	int             is_read_only;

	/* BUG: This is a performance problem we should look at later */
	holder = seln_inquire(SELN_SECONDARY);
	result = seln_ask(&holder, SELN_REQ_IS_READONLY, 0, NULL);
	data = result->data;
	/* Test if is SELN_IS_READONLY */
	data += sizeof(Seln_attribute);
	is_read_only = *(int *) data;

	if (is_read_only) {
	    return (TEXTSW_PE_READ_ONLY);
	} else {
#ifdef OW_I18N
	    result = seln_ask(&holder, SELN_REQ_CONTENTS_WCS, 0, NULL);
#else
	    result = seln_ask(&holder, SELN_REQ_CONTENTS_ASCII, 0, NULL);
#endif
	    data = result->data;
	    /* Test if is SELN_REQ_CONTENTS_ASCII */
	    data += sizeof(Seln_attribute);
	    string = MALLOC(STRLEN((CHAR *)data) + 1);
	    STRCPY(string, (CHAR *)data);
	    result = seln_ask(&holder, SELN_REQ_COMMIT_PENDING_DELETE, 0, NULL);
	}

    }
    /*
     * Third, post-process the primary selection.
     */
    is_pending_delete = (EV_SEL_PENDING_DELETE & ev_get_selection(
			    chain, &first, &last_plus_one, EV_SEL_PRIMARY));
    if (first < last_plus_one) {
	if (is_pending_delete && (ro_bdry < last_plus_one)) {
	    ev_delete_span(chain,
			   (first < ro_bdry) ? ro_bdry : first,
			   last_plus_one, &delta);
	}
	if (first != ES_INFINITY)
	    textsw_set_selection(VIEW_REP_TO_ABS(view),
				 ES_INFINITY, ES_INFINITY, EV_SEL_PRIMARY);
    }
    /*
     * Fourth, insert the text being gotten.
     */
    ev_set(view->e_view, EV_CHAIN_DELAY_UPDATE, FALSE, NULL);
    EV_SET_INSERT(chain, textsw_get_saved_insert(folio), first);
    if (lower_context != EV_NO_CONTEXT) {
	ev_check_insert_visibility(chain);
    }
    if (selection_is_local)
	(void) textsw_insert_pieces(view, first, secondary);
    else {
	(void) textsw_do_input(view, string, STRLEN(string), TXTSW_UPDATE_SCROLLBAR);
	free(string);
    }

    TEXTSW_DO_INSERT_MAKES_VISIBLE(view);
    folio->track_state &= ~TXTSW_TRACK_QUICK_MOVE;
}

Pkg_private int
textsw_end_quick_move(view)
    Textsw_view_handle view;
{
    extern void     textsw_init_selection_object();
    extern void     textsw_clear_secondary_selection();
    int             result = 0;
    register Textsw_folio folio = FOLIO_FOR_VIEW(view);
    int             selection_is_local;


    selection_is_local = textsw_inform_seln_svc(folio, TXTSW_FUNC_DELETE, FALSE);
    if ((folio->func_state & TXTSW_FUNC_DELETE) == 0)
	return (0);
    /*
     * if ((folio->func_state & TXTSW_FUNC_EXECUTE) == 0) goto Done;
     */

    if (TXTSW_IS_READ_ONLY(folio)) {
	result = TEXTSW_PE_READ_ONLY;
	textsw_clear_secondary_selection(folio, EV_SEL_SECONDARY);
	goto Done;
    }
#ifdef OW_I18N
    textsw_implicit_commit(folio);
#endif
    textsw_checkpoint_undo(VIEW_REP_TO_ABS(view),
			   (caddr_t) TEXTSW_INFINITY - 1);
    ASSUME(allock());
    result = textsw_do_move(view, selection_is_local);
    ASSUME(allock());
    textsw_checkpoint_undo(VIEW_REP_TO_ABS(view),
			   (caddr_t) TEXTSW_INFINITY - 1);

Done:
    textsw_end_function(view, TXTSW_FUNC_DELETE);
    textsw_update_scrollbars(folio, TEXTSW_VIEW_NULL);
    folio->track_state &= ~TXTSW_TRACK_QUICK_MOVE;
    return (result);


}

Pkg_private void
textsw_track_move(view, ie)
    Textsw_view_handle view;
    register Event *ie;
{

    if (win_inputnegevent(ie))
	textsw_finish_move(view, ie);
}

Pkg_private void
textsw_finish_move(view, ie)
    Textsw_view_handle view;
    register Event *ie;
{
    Textsw_folio    folio = FOLIO_FOR_VIEW(view);

    xv_do_move(view, ie);
    folio->track_state &= ~TXTSW_TRACK_MOVE;
    textsw_reset_cursor(view);
}

Pkg_private	void
textsw_clear_move(view)
    Textsw_view_handle view;
{
    Textsw_folio    folio = FOLIO_FOR_VIEW(view);

    folio->track_state &= ~TXTSW_TRACK_MOVE;
    textsw_reset_cursor(view);
}

Pkg_private void
textsw_track_duplicate(view, ie)
    Textsw_view_handle view;
    register Event *ie;
{

    if (win_inputnegevent(ie))
	textsw_finish_duplicate(view, ie);
}

Pkg_private void
textsw_finish_duplicate(view, ie)
    Textsw_view_handle view;
    register Event *ie;
{
    Textsw_folio    folio = FOLIO_FOR_VIEW(view);

    textsw_do_duplicate(view, ie);
    folio->track_state &= ~TXTSW_TRACK_DUPLICATE;
    textsw_reset_cursor(view);
}

Pkg_private void
textsw_clear_duplicate(view)
    Textsw_view_handle view;
{
    Textsw_folio    folio = FOLIO_FOR_VIEW(view);

    folio->track_state &= ~TXTSW_TRACK_DUPLICATE;
    textsw_reset_cursor(view);
}

Pkg_private void
textsw_reset_cursor(view)
    Textsw_view_handle view;
{
    Xv_Cursor       main_cursor;
    Xv_object       screen, server;

    screen = xv_get(VIEW_REP_TO_ABS(view), XV_SCREEN);
    server = xv_get(screen, SCREEN_SERVER);
    main_cursor = (Xv_Cursor) xv_get(server,
				     XV_KEY_DATA, CURSOR_BASIC_PTR);
    xv_set(VIEW_REP_TO_ABS(view), WIN_CURSOR, main_cursor, NULL);
}

/*
 * 1) if two spaces are left after deleting, they should be collapsed. 2) if
 * the selection is a word, when it is moved or duplicated, if there is a
 * space on one end there should be a space on both ends.
 */
/*
 * Menu proc() is called... sets SELN_FN_MOVE to true saves the location of
 * the selection in the folio. As long as track state is TXTSW_TRACK_MOVE,
 * all input events go to track move(). On button up, xv_do_move() is called...
 * resets track state.
 */
Pkg_private int
xv_do_move(view, ie)
    Textsw_view_handle view;
    register Event *ie;
{
    Textsw_folio    folio = FOLIO_FOR_VIEW(view);
    Textsw          textsw = VIEW_REP_TO_ABS(view);
    Es_index        first, last_plus_one;
    Es_index        pos, original_pos;
    CHAR            sel[1024], buf[2];
    int             sel_len;

    (void) ev_get_selection(folio->views, &first, &last_plus_one,
			    EV_SEL_PRIMARY);

    textsw_get_selection_as_string(folio, EV_SEL_PRIMARY, sel, 1024);
    sel_len = STRLEN(sel);

    pos = ev_resolve_xy(view->e_view, ie->ie_locx, ie->ie_locy);
    pos = textsw_do_balance_beam(view, ie->ie_locx, ie->ie_locy, pos, pos + 1);

    /* don't do anything if destination is within selection */
    if (pos >= first && pos <= last_plus_one)
	return 0;

    original_pos = pos;

    /* if spaces on either side, collapse them */
    xv_get(textsw, TEXTSW_CONTENTS_I18N, first - 1, buf, 2);
    if (buf[0] == ' ') {
	xv_get(textsw, TEXTSW_CONTENTS_I18N, last_plus_one, buf, 1);
	if (buf[0] == ' ') {
	    last_plus_one++;
	}
    }
    /* delete the source */
    textsw_delete_i18n(textsw, first, last_plus_one);

    /* correct for deletion */
    if (original_pos > first)
	pos -= last_plus_one - first;

    /* if punctuation to the right and space to the left, delete space */
    xv_get(textsw, TEXTSW_CONTENTS_I18N, first - 1, buf, 2);
    if (buf[1] == '.' || buf[1] == ',' || buf[1] == ';' || buf[1] == ':') {
	if (buf[0] == ' ') {
	    textsw_delete_i18n(textsw, first - 1, first);
	    if (original_pos > first)
		pos--;
	}
    }
    xv_set(textsw, TEXTSW_INSERTION_POINT_I18N, pos, NULL);


    /* add leading/trailing space to selection if needed for new location */
    xv_get(textsw, TEXTSW_CONTENTS_I18N, pos - 1, buf, 2);
    if (buf[1] == ' ') {
	if (buf[0] != ' ') {
	    /* add leading space */
	    BCOPY(sel, sel + 1, sel_len);
	    sel[0] = ' ';
	    sel_len++;
	    sel[sel_len] = '\0';
	    textsw_insert_i18n(textsw, sel, sel_len);
	    /* reset selection to original span */
	    textsw_set_selection_i18n(textsw,
				      pos + 1, pos + sel_len, EV_SEL_PRIMARY);
	    return 0;
	}
    } else {
	if (buf[0] == ' ') {
	    /* add trailing space */
	    sel[sel_len] = ' ';
	    sel_len++;
	    sel[sel_len] = '\0';
	    textsw_insert_i18n(textsw, sel, sel_len);
	    /* reset selection to original span */
	    textsw_set_selection_i18n(textsw,
				      pos, pos + sel_len - 1, EV_SEL_PRIMARY);
	    /* correct insertion point */
	    xv_set(textsw, TEXTSW_INSERTION_POINT_I18N, pos + sel_len - 1, NULL);
	    return 0;
	}
    }
    if (buf[1] == '.' ||
	buf[1] == ',' ||
	buf[1] == ';' ||
	buf[1] == ':') {
	/* before punctuation mark -- add leading space */
	BCOPY(sel, sel + 1, STRLEN(sel));
	sel[0] = ' ';
	sel_len++;
	sel[sel_len] = '\0';
	textsw_insert_i18n(textsw, sel, sel_len);
	/* reset selection to original span */
	textsw_set_selection_i18n(textsw,
				  pos + 1, pos + sel_len, EV_SEL_PRIMARY);
	return 0;
    } else {
	/* don't add any spaces */
	textsw_insert_i18n(textsw, sel, sel_len);
	/* reset selection to original span */
	textsw_set_selection_i18n(textsw, pos, pos + sel_len, EV_SEL_PRIMARY);
	return 0;
    }
}

Pkg_private int
textsw_do_duplicate(view, ie)
    Textsw_view_handle view;
    register Event *ie;
{
    Textsw_folio    folio = FOLIO_FOR_VIEW(view);
    Textsw          textsw = VIEW_REP_TO_ABS(view);
    Es_index        position, pos;
    CHAR            buf[1024];
    int             len;

    pos = ev_resolve_xy(view->e_view, ie->ie_locx, ie->ie_locy);
    position = textsw_do_balance_beam(view, ie->ie_locx, ie->ie_locy, pos, pos + 1);
    xv_set(textsw, TEXTSW_INSERTION_POINT_I18N, position, NULL);

    xv_get(textsw, TEXTSW_CONTENTS_I18N, position, buf, 1);
    if (buf[0] == ' ') {
	xv_get(textsw, TEXTSW_CONTENTS_I18N, position - 1, buf, 1);
	if (buf[0] != ' ') {
	    /* space after -- add leading space */
	    buf[0] = ' ';
	    textsw_get_selection_as_string(folio, EV_SEL_PRIMARY, buf + 1, 1024);
	    textsw_insert_i18n(textsw, buf, STRLEN(buf));
	    /* reset selection to original span */
	    textsw_set_selection_i18n(textsw, position + 1,
				      position + STRLEN(buf), EV_SEL_PRIMARY);
	    return 0;
	}
    } else {
	xv_get(textsw, TEXTSW_CONTENTS_I18N, position - 1, buf, 1);
	if (buf[0] == ' ') {
	    /* space before -- add trailing space */
	    textsw_get_selection_as_string(folio, EV_SEL_PRIMARY, buf, 1024);
	    len = STRLEN(buf);
	    buf[len] = ' ';
	    buf[len + 1] = '\0';
	    textsw_insert_i18n(textsw, buf, STRLEN(buf));
	    textsw_set_selection_i18n(textsw, position,
				position + STRLEN(buf) - 1, EV_SEL_PRIMARY);
	    xv_set(textsw, TEXTSW_INSERTION_POINT_I18N,
		   position + STRLEN(buf) - 1, NULL);
	    return 0;
	}
    }

    xv_get(textsw, TEXTSW_CONTENTS_I18N, position, buf, 1);
    if (buf[0] == '.' ||
	buf[0] == ',' ||
	buf[0] == ';' ||
	buf[0] == ':') {
	/* before punctuation mark -- add leading space */
	buf[0] = ' ';
	textsw_get_selection_as_string(folio, EV_SEL_PRIMARY, buf + 1, 1024);
	textsw_insert_i18n(textsw, buf, STRLEN(buf));
	/* reset selection to original span */
	textsw_set_selection_i18n(textsw, position + 1,
				  position + STRLEN(buf), EV_SEL_PRIMARY);
	return 0;
    } else {
	/* don't add any spaces */
	textsw_get_selection_as_string(folio, EV_SEL_PRIMARY, buf, 1024);
	textsw_insert_i18n(textsw, buf, STRLEN(buf));
	/* reset selection to original span */
	textsw_set_selection_i18n(textsw, position,
				  position + STRLEN(buf), EV_SEL_PRIMARY);
	return 0;
    }
}

Pkg_private int
textsw_clean_up_move(view, first, last_plus_one)
    Textsw_view_handle view;
    Es_index        first, last_plus_one;
{
    Textsw          textsw = VIEW_REP_TO_ABS(view);
    CHAR            first_buf[1], last_buf[1];
    int             shift_left;

    /* if spaces on either side, collapse them */
    xv_get(textsw, TEXTSW_CONTENTS_I18N, first - 1, first_buf, 1);
    if (first_buf[0] == ' ') {
	xv_get(textsw, TEXTSW_CONTENTS_I18N, last_plus_one, last_buf, 1);
	if (last_buf[0] == ' ')
	    shift_left = TRUE;
    }
    xv_get(textsw, TEXTSW_CONTENTS_I18N, last_plus_one, last_buf, 1);
    /* if punctuation to the right and space to the left, delete space */
    if (last_buf[0] == '.' || last_buf[0] == ',' || last_buf[0] == ';'
	|| last_buf[0] == ':') {
	if (first_buf[0] == ' ')
	    shift_left = TRUE;
    }
    if (shift_left)
	textsw_delete_i18n(textsw, first - 1, first);
    return (shift_left);
}

static void
display_notice(public_view, dnd_status)
    Xv_opaque	 public_view;
    int		 dnd_status;
{
    char 	*error_msg;
    Xv_Notice	 notice;

    switch (dnd_status) {
      case XV_OK:
        return;
      case DND_TIMEOUT:
        error_msg = XV_MSG("Operation timed out");
        break;
      case DND_ILLEGAL_TARGET:
        error_msg = XV_MSG("Illegal drop target");
        break;
      case DND_SELECTION:
        error_msg = XV_MSG("Unable to acquire selection");
        break;
      case DND_ROOT:
        error_msg = XV_MSG("Root window is not a valid drop target");
        break;
      case XV_ERROR:
        error_msg = XV_MSG("Unexpected internal error");
        break;
    }
    notice = xv_create((Frame)xv_get(public_view, WIN_FRAME), NOTICE,
                    NOTICE_MESSAGE_STRINGS,
                        XV_MSG("Drag and Drop failed:"),
                        error_msg,
                        0,
                    XV_SHOW, TRUE,
                    NULL);
    xv_destroy(notice);
}

#define MAX_CHARS_SHOWN	 5	/* most chars shown in the drag move cursor */
#define SELECTION_BUF_SIZE	MAX_CHARS_SHOWN + 2

Pkg_private void
textsw_do_drag_copy_move(view, ie, is_copy)
    Textsw_view_handle 	 view;
    Event          	*ie;
    int             	 is_copy;
{
    Xv_opaque       	 public_view = VIEW_REP_TO_ABS(view);
    Textsw_folio 	 folio = FOLIO_FOR_VIEW(view);
    Xv_Cursor       	 dnd_cursor,
			 dnd_accept_cursor;
    CHAR            	 buf[SELECTION_BUF_SIZE];
    short           	 l;
    Dnd			 dnd;
    int			 DndConvertProc(),
			 dnd_status;
    CHAR 		*buffer;
#ifdef OW_I18N
    char 		*buffer_mb;
#endif
    Es_index  		 first, last_plus_one;

    l = textsw_get_selection_as_string(folio, EV_SEL_PRIMARY, buf,
			MAX_CHARS_SHOWN + 1);

    dnd_cursor = xv_create(public_view, CURSOR,
#ifdef OW_I18N
			CURSOR_STRING_WCS, 	buf,
#else
			CURSOR_STRING, 		buf,
#endif
			CURSOR_DRAG_TYPE,
			     	(is_copy ? CURSOR_DUPLICATE : CURSOR_MOVE),
			NULL);

    dnd_accept_cursor = xv_create(public_view, CURSOR,
#ifdef OW_I18N
			CURSOR_STRING_WCS, 	buf,
#else
			CURSOR_STRING, 		buf,
#endif
			CURSOR_DRAG_TYPE,
				     (is_copy ? CURSOR_DUPLICATE : CURSOR_MOVE),
			CURSOR_DRAG_STATE,	CURSOR_ACCEPT,
			NULL);

    dnd = xv_create(public_view, DRAGDROP,
			DND_TYPE, 		(is_copy ? DND_COPY : DND_MOVE),
			DND_CURSOR, 		dnd_cursor,
			DND_ACCEPT_CURSOR,	dnd_accept_cursor,
			SEL_CONVERT_PROC,	DndConvertProc,
			NULL);

    (void)ev_get_selection(folio->views, &first, &last_plus_one,EV_SEL_PRIMARY);
#ifdef OW_I18N
    buffer = (CHAR *)xv_malloc((last_plus_one - first + 1) * sizeof(CHAR));
#else
    buffer = (char *)xv_malloc(last_plus_one - first + 1);
#endif
    (void)textsw_get_selection_as_string(folio, EV_SEL_PRIMARY, buffer,
					 last_plus_one - first + 1);
					 
    if (!dnd_data_key)
        dnd_data_key = xv_unique_key();
    if (!dnd_view_key)
        dnd_view_key = xv_unique_key();

#ifdef OW_I18N
    buffer_mb = _xv_wcstombsdup(buffer);
    xv_set(dnd, XV_KEY_DATA, dnd_data_key, buffer_mb, NULL);
    if (buffer)
	free((char *)buffer);
#else
    xv_set(dnd, XV_KEY_DATA, dnd_data_key, buffer, NULL);
#endif
    xv_set(dnd, XV_KEY_DATA, dnd_view_key, view, NULL);

    if ((dnd_status = dnd_send_drop(dnd)) != XV_OK) {
       if (dnd_status != DND_ABORTED)
           display_notice(public_view, dnd_status);
       xv_destroy(dnd);
    }

    xv_destroy(dnd_cursor);
    xv_destroy(dnd_accept_cursor);
}

static int
DndConvertProc(dnd, type, data, length, format)
    Dnd      	 dnd;
    Atom        *type;
    Xv_opaque	*data;
    unsigned long  *length;
    int         *format;
{
    Xv_Server server = XV_SERVER_FROM_WINDOW(xv_get((Xv_opaque)dnd, XV_OWNER));
    Textsw_view_handle 	 view = (Textsw_view_handle)xv_get(dnd, XV_KEY_DATA,
							   dnd_view_key);
    Textsw_folio        folio = FOLIO_FOR_VIEW(view);

    if (*type == (Atom)xv_get(server, SERVER_ATOM, "_SUN_DRAGDROP_DONE")) {
	xv_set(dnd, SEL_OWN, False, NULL);
	xv_free((char *)xv_get(dnd, XV_KEY_DATA, dnd_data_key)); 
	xv_destroy_safe(dnd);
	*format = 32;
	*length = 0;
	*data = XV_ZERO;
        *type = (Atom)xv_get(server, SERVER_ATOM, "NULL");
        return(True);
    } else if (*type == (Atom)xv_get(server, SERVER_ATOM, "DELETE")) {
                        /* Destination asked us to delete the selection.
                         * If it is appropriate to do so, we should.
                         */
        Es_index  first, last_plus_one, ro_bound;

	view = (Textsw_view_handle)xv_get(dnd, XV_KEY_DATA, dnd_view_key);
	folio = FOLIO_FOR_VIEW(view);
        (void)ev_get_selection(folio->views, &first, &last_plus_one,
			       EV_SEL_PRIMARY);
	if (!TXTSW_IS_READ_ONLY(folio)) {
	  ro_bound = textsw_read_only_boundary_is_at(folio);
	  if (folio->state & TXTSW_DELETE_REPLACES_CLIPBOARD) {
	    textsw_delete_span(view, (first < ro_bound) ? ro_bound : first,
			       last_plus_one, TXTSW_DS_SHELVE);
	  } else {
	    textsw_delete_span(view, (first < ro_bound) ? ro_bound : first,
					       last_plus_one, NULL);
	  }
	}
        *format = 32;
        *length = 0;
        *data = XV_ZERO;
        *type = (Atom)xv_get(server, SERVER_ATOM, "NULL");
        return(True);
    } else if (*type == (Atom)xv_get(server, SERVER_ATOM,
						   "_SUN_SELN_IS_READONLY")) {
	int results = TXTSW_IS_READ_ONLY(folio);
	*format = 32;
	*length = 1;
	*data = (Xv_opaque)&results;
	*type = XA_INTEGER;
	return(True);
    } else if (*type == XA_STRING || *type == (Atom)xv_get(server, SERVER_ATOM,
								    "TEXT")) {
	char *buf = (char *)xv_get(dnd, XV_KEY_DATA, dnd_data_key);
	*format = 8;
	*length = strlen(buf);
	*data = (Xv_opaque)buf;
	*type = XA_STRING;
	return(True);
    } else if (*type == (Atom)xv_get(server, SERVER_ATOM, "TARGETS")) {
	static Atom atom_list[7];

	atom_list[0] = (Atom)xv_get(server, SERVER_ATOM, "_SUN_DRAGDROP_DONE");
	atom_list[1] = (Atom)xv_get(server, SERVER_ATOM, "DELETE");
	atom_list[2] = (Atom)xv_get(server, SERVER_ATOM,
						"_SUN_SELN_IS_READONLY");
	atom_list[3] = XA_STRING;
	atom_list[4] = (Atom)xv_get(server, SERVER_ATOM, "TEXT");
	atom_list[5] = (Atom)xv_get(server, SERVER_ATOM, "TARGETS");
	atom_list[6] = (Atom)xv_get(server, SERVER_ATOM, "TIMESTAMP");
	*format = 32;
	*length = 7;
	*data = (Xv_opaque)atom_list;
	*type = XA_ATOM;
	return(True);
    }
    return(sel_convert_proc(dnd, type, data, length, format));
}

/*
 * When a textsw gets a ACTION_DRAG_MOVE event, this routines gets called to
 * get the primary selection from another process and do move/copy
 * ACTION_DRAG_MOVE is a result of XSendEvent called by the subwindow that
 * originally sees the button-down that starts the drag move
 */
Pkg_private void
textsw_do_remote_drag_copy_move(view, ie, is_copy)
    register Textsw_view_handle  view;
    Event          		*ie;
    short           		 is_copy;
{
    Selection_requestor 	 sel;
    register Textsw_folio 	 folio = FOLIO_FOR_VIEW(view);
    char           		*string;
#ifdef OW_I18N
    CHAR           		*string_wc = NULL;
#endif
    unsigned long		 length;
    int				 format,
    				*is_read_only;
    Es_index        		 ro_bdry,
				 pos,
				 index,
				 temp;
    void	    		 DndReplyProc();
    struct textsw_context  	 context;

    is_read_only = NULL;
    /*
     * First, process insertion point .
     */
    ev_set(view->e_view, EV_CHAIN_DELAY_UPDATE, TRUE, NULL);
    ro_bdry = textsw_read_only_boundary_is_at(folio);
    pos = ev_resolve_xy(view->e_view,
			event_x(ie), event_y(ie));

    if (pos < ro_bdry) {
	Es_index	insert;

	insert = EV_GET_INSERT(folio->views);
	if (insert >= ro_bdry)
	    pos = insert;
	else
	    return;
    }

    if (!dnd_data_key)
        dnd_data_key = xv_unique_key();

    sel = xv_create(VIEW_REP_TO_ABS(view), SELECTION_REQUESTOR,
			  SEL_REPLY_PROC,	DndReplyProc,
			  SEL_TYPE_NAME,	"_SUN_SELN_IS_READONLY",
			  NULL);
    if (dnd_decode_drop(sel, ie) == XV_ERROR) {
        xv_destroy(sel);
	return;
    }
	/* If the source and the dest is the same process, see if the
	 * drop happened within the primary selection, in which case we
	 * don't do anything.
	 */

    /* make sure drop doesn't happen in read-only text */
    if (TXTSW_IS_READ_ONLY(folio)) {
        dnd_done(sel);
	xv_destroy(sel);
	textsw_read_only_msg(folio,event_x(ie), event_y(ie));
	return;
    }

    if (dnd_is_local(ie)) {
        Es_index 	first,
			last_plus_one;
	
	(void)ev_get_selection(folio->views, &first, &last_plus_one,
			       EV_SEL_PRIMARY);
	pos = ev_resolve_xy(view->e_view, event_x(ie), event_y(ie));
	/* make sure drop doesn't happen in read-only part of cmdtool */
	if (pos < ro_bdry) {
	  Es_index	insert;
	  
	  insert = EV_GET_INSERT(folio->views);
	  if (insert >= ro_bdry)
	    pos = insert;
	  else
	    pos = ro_bdry + 1;
	}

	if (pos >= first && pos < last_plus_one) {
	    dnd_done(sel);
    	    ev_set(view->e_view, EV_CHAIN_DELAY_UPDATE, FALSE, NULL);
	    return;
	}
    }
	/* If this is a move operation, see if the source is read only. */
    if (!is_copy) {
        is_read_only = (int *)xv_get(sel, SEL_DATA, &length, &format);
        if (length == SEL_ERROR) {
	    /* don't know if is_read_only got set, so set it back to NULL */
	    is_read_only = NULL;
	    is_copy = True;
	}
    }
	/* Ask for the selection to be converted into a string. */
    xv_set(sel, SEL_TYPE, XA_STRING, NULL);
    string = (char *)xv_get(sel, SEL_DATA, &length, &format);
    if (length == SEL_ERROR) {
	if (string)
	    xv_free(string);
	if (is_read_only)
	    xv_free(is_read_only);
	dnd_done(sel);
	return;
    }

    string = (char *)xv_get(sel, XV_KEY_DATA, dnd_data_key);
	
    ev_set(view->e_view, EV_CHAIN_DELAY_UPDATE, FALSE, NULL);
    EV_SET_INSERT(folio->views, pos, temp);

#ifdef OW_I18N
    string_wc = _xv_mbstowcsdup(string);
    index = textsw_do_input(view, string_wc, (long int)STRLEN(string_wc),
			    TXTSW_UPDATE_SCROLLBAR);
    if (string_wc)
	free((char *)string_wc);
#else
    index = textsw_do_input(view, (char *)string, (long int)strlen(string),
			    TXTSW_UPDATE_SCROLLBAR);
#endif
        /* If this is a move operation and we were able to insert the text,
	 * ask the source to delete the selection.*/
    if (!is_copy && !*is_read_only && index) {
	xv_set(sel, SEL_TYPE_NAME, "DELETE", NULL);
	(void)xv_get(sel, SEL_DATA, &length, &format);
    }

    free((char *) string);
    if (is_read_only)
	xv_free(is_read_only);
    dnd_done(sel);
    xv_destroy(sel);
    TEXTSW_DO_INSERT_MAKES_VISIBLE(view);
}

void
DndReplyProc(sel, target, type, value, length, format)
    Selection_requestor sel;
    Xv_opaque       value;
    Atom            target;
    Atom            type;
    int             length;
    int             format;
{
    Xv_opaque	 owner = xv_get(sel, XV_OWNER);	
    Xv_Server	 server = XV_SERVER_FROM_WINDOW(owner);
    static int	 incr = False,
		 str_size = 0;
    static char *string;

    if (length == SEL_ERROR) {
        return;
    }

    if (target == XA_STRING) {
	if (type == (Atom)xv_get(server, SERVER_ATOM, "INCR")) {
	    incr = True;
	} else if (!incr) {
	    xv_set(sel, XV_KEY_DATA, dnd_data_key, value, NULL);
	    str_size = 0;
	} else if (length) {
	    if (!str_size)
	        string = (char *)xv_malloc(length);
	    else
		string = (char *)xv_realloc(string, str_size + length);
	    strncpy(string + str_size, (char *)value, length);
	    str_size += length;
	} else {
	    xv_set(sel, XV_KEY_DATA, dnd_data_key, string, NULL);
	    incr = str_size = 0;
	}
    } 
}
