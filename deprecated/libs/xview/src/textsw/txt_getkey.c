#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)txt_getkey.c 20.36 93/06/29";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * GET key processing.
 */

#include <xview/pkg.h>
#include <xview/attrol.h>
#include <xview_private/primal.h>
#include <xview_private/txt_impl.h>
#include <xview_private/ev_impl.h>	/* For declaration of ev_add_finder */
#include <errno.h>

extern int      errno;

static void     textsw_do_get();
Pkg_private Es_index textsw_find_mark_internal();
Pkg_private Es_index textsw_insert_pieces();

Pkg_private int
textsw_begin_get(view)
    Textsw_view_handle view;
{
    register Textsw_folio textsw = FOLIO_FOR_VIEW(view);

    textsw_begin_function(view, TXTSW_FUNC_GET);
    (void) textsw_inform_seln_svc(textsw, TXTSW_FUNC_GET, TRUE);
}

Pkg_private int
textsw_end_get(view)
    register Textsw_view_handle view;
{
    int             easy, result = 0;
    register Textsw_folio folio = FOLIO_FOR_VIEW(view);
    Seln_holder holder;
    Xv_Server server = (Xv_Server)XV_SERVER_FROM_WINDOW(VIEW_REP_TO_ABS(view));
    
    /* textsw_begin_get() eventually tried to acquire the CARET, but in
       doing so, it could have lost it because another textsw in the same
       process had it and lost it from a SelectionClear from the server.
       So if it doesn't exist, then we need to acquire it again because
       execute_fn() (which is eventually called by textsw_inform_seln_svc)
       actually tries to get the CARET and might fail. */
    holder = selection_inquire(server, SELN_CARET);
    if (holder.state != SELN_EXISTS) {
	textsw_acquire_seln(folio, SELN_CARET);
    }

    easy = textsw_inform_seln_svc(folio, TXTSW_FUNC_GET, FALSE);
    if ((folio->func_state & TXTSW_FUNC_GET) == 0)
	return (0);
    if ((folio->func_state & TXTSW_FUNC_EXECUTE) == 0)
	goto Done;
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
    textsw_do_get(view, easy);
    ASSUME(allock());
    textsw_checkpoint_undo(VIEW_REP_TO_ABS(view),
			   (caddr_t) TEXTSW_INFINITY - 1);
Done:
    textsw_end_function(view, TXTSW_FUNC_GET);

    textsw_update_scrollbars(folio, TEXTSW_VIEW_NULL);

    return (result);
}

Pkg_private     Es_index
textsw_read_only_boundary_is_at(folio)
    register Textsw_folio folio;
{
    register Es_index result;

    if (EV_MARK_IS_NULL(&folio->read_only_boundary)) {
	result = 0;
    } else {
	result = textsw_find_mark_internal(folio,
					   folio->read_only_boundary);
	if AN_ERROR
	    (result == ES_INFINITY)
		result = 0;
    }
    return (result);
}

static void
textsw_do_get(view, local_operands)
    register Textsw_view_handle view;
    int             local_operands;
{
    /*
     * The following table indicates the final contents of the trashbin.
     * It is based on the modes of the primary and secondary selections.
     * P-d is short for pending-delete. An empty primary selection is treated
     * as not pending-delete.
     * 
     * An empty primary selection is treated as not pending-delete.
     *
     *			    Primary
     *			 ~P-d	 P-d
     *	Secondary	=================
     *	  Empty		| Tbin	| Pri.	|
     *	  ~P-d		| Tbin	| Pri.	|
     *	   P-d		| Sec.	| Pri.	|
     *			=================
     *
     * The variable acquire_shelf is used as a flag if the shelf should
     * be replaced with primary or secondary.
     *
     * The different cases for pasting are:
     * 
     * Paste clipboard at insertion point.
     * Primary exists and clipboard replaces primary.
     * Secondary exists so secondary is inserted at insertion point
     *           and possibly deleted (if pending delete)
     * Secondary and Primary exists so secondary replaces primary and
     *           secondary is possibly deleted (if pending delete)
     *
     */
    extern void     ev_check_insert_visibility(), ev_scroll_if_old_insert_visible();
    extern int      ev_get_selection();
    extern Es_handle textsw_esh_for_span();
    register Textsw_folio folio = FOLIO_FOR_VIEW(view);
    register Ev_chain views = folio->views;
    register Es_handle secondary, primary;
    register int    is_pending_delete, sec_is_pending_delete;
    int             lower_context = (int)
    ev_get(view->e_view, EV_CHAIN_LOWER_CONTEXT);
    int             acquire_shelf = FALSE;
    int		    dtemp;
    Es_index        delta, pri_first, pri_last_plus_one, ro_bdry;
    Es_index        sec_first, sec_last_plus_one;  /* of secondary */
    Es_index        first,last_plus_one;           /* of insertion point */
    int             primary_in_use, secondary_in_use = FALSE; /* keep track
								 of usage */

    /*
     * First, pre-process the primary selection.
     * if primary is set after all this, then there was a pending delete
     * primary.
     */
    secondary = ES_NULL;
    primary = ES_NULL;
    ev_set(view->e_view, EV_CHAIN_DELAY_UPDATE, TRUE, NULL);
    ro_bdry = textsw_read_only_boundary_is_at(folio);
    is_pending_delete = (EV_SEL_PENDING_DELETE & ev_get_selection(
			    views, &pri_first, &pri_last_plus_one, 
			    EV_SEL_PRIMARY));
    if (pri_last_plus_one <= ro_bdry) {
	is_pending_delete = 0;
    }
    /* save any local primary selection */
    if ((pri_first < pri_last_plus_one) && is_pending_delete) {
	primary = textsw_esh_for_span(view, pri_first, pri_last_plus_one, 
				      ES_NULL);
    }
    /*
     * Second, completely process local secondary selection.
     */
    if (local_operands) {
	sec_is_pending_delete = (EV_SEL_PENDING_DELETE & ev_get_selection(
			  views, &sec_first, &sec_last_plus_one, 
			  EV_SEL_SECONDARY));
	if (sec_last_plus_one <= ro_bdry) {
	    sec_is_pending_delete = 0;
	}
	if (sec_first < sec_last_plus_one) {
	    secondary = textsw_esh_for_span(view, sec_first, 
					    sec_last_plus_one, 
					    ES_NULL);
	    if (sec_is_pending_delete) {
		ev_delete_span(views,
			       (sec_first < ro_bdry) ? ro_bdry : sec_first,
			       sec_last_plus_one, &delta);
		/* secondary might have overlapped primary, so
		   get primary again */
		is_pending_delete = (EV_SEL_PENDING_DELETE & 
				     ev_get_selection(views, 
						      &pri_first, 
						      &pri_last_plus_one, 
						      EV_SEL_PRIMARY));
	    }
	} else {
	    sec_is_pending_delete = 0;
	}
	if (sec_first != ES_INFINITY) {
	    textsw_set_selection(VIEW_REP_TO_ABS(view),
				 ES_INFINITY, ES_INFINITY,
				 EV_SEL_SECONDARY);
	}
    }
    /*
     * Third, post-process the primary selection.
     * delete primary if pending delete.
     */
    if (pri_first < pri_last_plus_one) {
	if (is_pending_delete && (ro_bdry < pri_last_plus_one)) {
	    ev_delete_span(views,
			   ((pri_first < ro_bdry) ? ro_bdry : pri_first),
			   pri_last_plus_one, &delta);
	}
    }
    if (pri_first != ES_INFINITY) {
	textsw_set_selection(VIEW_REP_TO_ABS(view),
			     ES_INFINITY, ES_INFINITY, EV_SEL_PRIMARY);
    }
    /*
     * Fourth, insert the text being gotten.
     * At this point, secondary and primary exist if they are non-NULL.
     */
    
    ev_set(view->e_view, EV_CHAIN_DELAY_UPDATE, FALSE, NULL);
    if (local_operands) {
	Es_handle piece_to_insert;
	/* jcb 5/21/90 */
	/* jcb 5/29/90 -- this sometimes causes misplaced pastes, but
	   it's deemed this is better than what it fixes */
	/* jcb 5/31/90 -- backed out again. it makes bad bugs */
	if( TRUE || pri_first == ES_INFINITY ) {
	    
	    dtemp =  textsw_get_saved_insert(folio);
	    
	    EV_SET_INSERT(folio->views, dtemp, first);
	}
	else
	    EV_SET_INSERT(folio->views, first, first);
	
	if AN_ERROR
	    (first == ES_INFINITY) {
		if (secondary)
		    es_destroy(secondary);
		if (primary)
		    es_destroy(primary);
		return;
	    }
	if (lower_context != EV_NO_CONTEXT) {
	    ev_check_insert_visibility(views);
	}
	piece_to_insert = ES_NULL;
	if (secondary) {
	    /* paste secondary selection at insertion point */
	    piece_to_insert = secondary;
	    secondary_in_use = TRUE;
	} else if (primary) {
	    if (folio->trash) {
		piece_to_insert = folio->trash;
	    }
	} else if (folio->trash) {
	    piece_to_insert = folio->trash;
	} /* else don't do any pastes. textsw_insert_pieces() returns 
	     pos if piece_to_insert is NULL. */
	last_plus_one = textsw_insert_pieces(view, first, piece_to_insert);
	/* keep track of again recording */
	if (TXTSW_DO_AGAIN(folio)) {
	    if (piece_to_insert) {
		if (piece_to_insert == folio->trash) {
		    textsw_record_trash_insert(folio);
		} else {
		    textsw_record_piece_insert(folio, piece_to_insert);
		}
	    }
	}
    } else {
	first = EV_GET_INSERT(views);
	if (lower_context != EV_NO_CONTEXT) {
	    ev_check_insert_visibility(views);
	}
	/*
	 * Note: textsw_stuff_selection uses routines that record the
	 * insertion for AGAIN, so we need not worry about that.
	 */
	textsw_stuff_selection(view, (unsigned) (secondary ?
					  EV_SEL_SECONDARY : EV_SEL_SHELF));
	last_plus_one = EV_GET_INSERT(views);
    }
    ev_update_chain_display(views);
    if (lower_context != EV_NO_CONTEXT) {
	ev_scroll_if_old_insert_visible(folio->views,
					last_plus_one,
					last_plus_one - first);
    }
    if (folio->state & TXTSW_DELETE_REPLACES_CLIPBOARD) {
	if (primary) {
	    folio->trash = primary;
	    acquire_shelf = TRUE;
	    primary_in_use = TRUE;
	} else if (secondary && sec_is_pending_delete) {
	    folio->trash = secondary;
	    acquire_shelf = TRUE;
	    secondary_in_use = TRUE;
	}
    }
    if (acquire_shelf)
	textsw_acquire_seln(folio, SELN_SHELF);
    if (primary && (!primary_in_use))
	es_destroy(primary);
    if (secondary && (!secondary_in_use))
	es_destroy(secondary);
}

Pkg_private     Es_index
textsw_insert_pieces(view, pos, pieces)
    Textsw_view_handle view;
    register Es_index pos;
    Es_handle       pieces;
{
    Pkg_private Es_index textsw_set_insert();
    register Textsw_folio folio = FOLIO_FOR_VIEW(view);
    register Ev_chain chain = folio->views;
    Es_index        delta, old_insert_pos, old_length = es_get_length(chain->esh), new_insert_pos,
                    temp;

    if (pieces == ES_NULL)
	return (pos);
    if (folio->notify_level & TEXTSW_NOTIFY_EDIT)
	old_insert_pos = EV_GET_INSERT(chain);
    EV_SET_INSERT(chain, pos, temp);
    /* Required since es_set(ES_HANDLE_TO_INSERT) bypasses ev code. */
    es_set(chain->esh, ES_HANDLE_TO_INSERT, pieces, NULL);
    new_insert_pos = es_get_position(chain->esh);
    (void) textsw_set_insert(folio, new_insert_pos);
    delta = new_insert_pos - pos;
    /*
     * The esh may simply swallow the pieces (in the cmdsw), so check to see
     * if any change actually occurred.
     */
    if (delta) {
	ev_update_after_edit(chain, pos, delta, old_length, pos);
	if (folio->notify_level & TEXTSW_NOTIFY_EDIT) {
	    textsw_notify_replaced((Textsw_opaque) folio->first_view,
			       old_insert_pos, old_length, pos, pos, delta);
	}
	textsw_checkpoint(folio);
    }
    return (new_insert_pos);
}

Pkg_private int
textsw_function_get(view)
    Textsw_view_handle view;
{
    int             result;

    textsw_begin_get(view);
    result = textsw_end_get(view);
    return (result);
}

Pkg_private int
textsw_put_then_get(view)
    Textsw_view_handle view;
{
    extern Es_handle textsw_esh_for_span();
    register Textsw_folio textsw = FOLIO_FOR_VIEW(view);
    Es_index        first, last_plus_one, insert;
    register int    is_pending_delete;
    int             seln_nonzero;

    if (seln_nonzero = textsw_is_seln_nonzero(textsw, EV_SEL_PRIMARY)) {
	textsw_checkpoint_undo(VIEW_REP_TO_ABS(view),
			       (caddr_t) TEXTSW_INFINITY - 1);
	if (seln_nonzero == 2) {
	    is_pending_delete = (EV_SEL_PENDING_DELETE &
				 ev_get_selection(textsw->views,
				   &first, &last_plus_one, EV_SEL_PRIMARY));
	    if (first < last_plus_one) {
		insert = EV_GET_INSERT(textsw->views);
		textsw->trash = textsw_esh_for_span(
				 view, first, last_plus_one, textsw->trash);
		textsw_set_selection(VIEW_REP_TO_ABS(view),
				  ES_INFINITY, ES_INFINITY, EV_SEL_PRIMARY);
		if (!is_pending_delete ||
		    (insert < first) || (last_plus_one < insert))
		    textsw_insert_pieces(view, insert, textsw->trash);
		textsw_acquire_seln(textsw, SELN_SHELF);
	    }
	} else {
	    /*
	     * Item is "Put then Get", but there is a potential race to the
	     * Shelf between us and the Primary Selection holder, so we Copy
	     * primary then Put instead.
	     */
	    textsw_stuff_selection(view, EV_SEL_PRIMARY);
	    textsw_put(view);
	}
	textsw_checkpoint_undo(VIEW_REP_TO_ABS(view),
			       (caddr_t) TEXTSW_INFINITY - 1);
    } else if (textsw_is_seln_nonzero(textsw, EV_SEL_SHELF)) {
	textsw_function_get(view);
    }				/* else menu item should have been grayed
				 * out! */
}

/*
 * ===============================================================
 * 
 * Misc. marking utilities
 * 
 * ===============================================================
 */
Pkg_private void textsw_remove_mark_internal();

Pkg_private     Ev_mark_object
textsw_add_mark_internal(textsw, position, flags)
    Textsw_folio    textsw;
    Es_index        position;
    unsigned        flags;
{
    Ev_mark_object  mark;
    register Ev_mark mark_to_use;

    if (flags & TEXTSW_MARK_READ_ONLY) {
	mark_to_use = &textsw->read_only_boundary;
	textsw_remove_mark_internal(textsw, *mark_to_use);
    } else {
	mark_to_use = &mark;
    }
    EV_INIT_MARK(*mark_to_use);
    if (flags & TEXTSW_MARK_MOVE_AT_INSERT)
	EV_MARK_SET_MOVE_AT_INSERT(*mark_to_use);
    ev_add_finger(&textsw->views->fingers, position, 0, mark_to_use);
    return (*mark_to_use);
}

Xv_public          Textsw_mark
#ifdef OW_I18N
textsw_add_mark_wc(abstract, position, flags)
#else
textsw_add_mark(abstract, position, flags)
#endif
    Textsw          abstract;
    Es_index        position;
    unsigned        flags;
{
    Textsw_view_handle view = VIEW_ABS_TO_REP(abstract);
    return ((Textsw_mark) textsw_add_mark_internal(
				    FOLIO_FOR_VIEW(view), position, flags));
}

#ifdef OW_I18N
Xv_public          Textsw_mark
textsw_add_mark(abstract, position, flags)
    Textsw          abstract;
    Es_index        position;
    unsigned        flags;
{
    Textsw_view_handle view = VIEW_ABS_TO_REP(abstract);
    Textsw_folio folio = FOLIO_FOR_VIEW(view);

    return ((Textsw_mark) textsw_add_mark_internal(folio,
			      textsw_wcpos_from_mbpos(folio, position), flags));
}
#endif /* OW_I18N */

Pkg_private     Es_index
textsw_find_mark_internal(textsw, mark)
    Textsw_folio    textsw;
    Ev_mark_object  mark;
{
    Ev_finger_handle finger;

    finger = ev_find_finger(&textsw->views->fingers, mark);
    return (finger ? finger->pos : ES_INFINITY);
}

Xv_public          Textsw_index
#ifdef OW_I18N
textsw_find_mark_wc(abstract, mark)
#else
textsw_find_mark(abstract, mark)
#endif
    Textsw          abstract;
    Textsw_mark     mark;
{
    Textsw_view_handle view = VIEW_ABS_TO_REP(abstract);
    Ev_mark_object *dummy_for_compiler = (Ev_mark_object *) & mark;

#ifdef	lint
    view->magic = *dummy_for_compiler;	/* To get rid of unused msg */
    return ((Textsw_index) 0);
#else	/* lint */
    return ((Textsw_index) textsw_find_mark_internal(FOLIO_FOR_VIEW(view),
						     *dummy_for_compiler));
#endif	/* lint */
}

#ifdef OW_I18N
Xv_public          Textsw_index
textsw_find_mark(abstract, mark)
    Textsw          abstract;
    Textsw_mark     mark;
{
    Textsw_view_handle view = VIEW_ABS_TO_REP(abstract);
    Textsw_folio folio = FOLIO_FOR_VIEW(view);
    Ev_mark_object *dummy_for_compiler = (Ev_mark_object *) & mark;

#ifdef	lint
    view->magic = *dummy_for_compiler;	/* To get rid of unused msg */
    return ((Textsw_index) 0);
#else	/* lint */
    return ((Textsw_index) textsw_mbpos_from_wcpos(folio,
		textsw_find_mark_internal(folio, *dummy_for_compiler)));
#endif	/* lint */
}
#endif /* OW_I18N */

Pkg_private void
textsw_remove_mark_internal(textsw, mark)
    Textsw_folio    textsw;
    Ev_mark_object  mark;
{
    if (!EV_MARK_IS_NULL(&mark)) {
	if (EV_MARK_ID(mark) == EV_MARK_ID(textsw->read_only_boundary)) {
	    EV_INIT_MARK(textsw->read_only_boundary);
	}
	ev_remove_finger(&textsw->views->fingers, mark);
    }
}

Xv_public void
textsw_remove_mark(abstract, mark)
    Textsw          abstract;
    Textsw_mark     mark;
{
    Textsw_view_handle view = VIEW_ABS_TO_REP(abstract);
    long unsigned  *dummy_for_compiler = (long unsigned *) &mark;

    textsw_remove_mark_internal(FOLIO_FOR_VIEW(view),
				*((Ev_mark) dummy_for_compiler));
}
