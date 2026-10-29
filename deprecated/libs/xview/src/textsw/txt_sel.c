#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)txt_sel.c 20.55 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * User interface to selection within text subwindows.
 */

#include <xview/pkg.h>
#include <xview/attrol.h>
#include <xview_private/primal.h>
#include <xview_private/txt_impl.h>
#include <xview_private/ev_impl.h>
#include <xview_private/txt_18impl.h>
#ifdef SVR4 
#include <stdlib.h> 
#endif /* SVR4 */

Pkg_private Es_index 	ev_line_start();
Pkg_private Es_index 	ev_resolve_xy();
Pkg_private void     	ev_make_visible();
Pkg_private Seln_rank 	textsw_acquire_seln();
Pkg_private void	textsw_not_visible_normalize();
Xv_public   void	textsw_set_selection();
#ifdef OW_I18N
Xv_public   void	textsw_set_selection_wcs();
#endif

Xv_public void
#ifdef OW_I18N
textsw_normalize_view_wc(abstract, pos)
#else
textsw_normalize_view(abstract, pos)
#endif
    Textsw          abstract;
    Es_index        pos;
{
    Textsw_view_handle view = VIEW_ABS_TO_REP(abstract);
    int             upper_context;

    upper_context = (int) ev_get(view->e_view, EV_CHAIN_UPPER_CONTEXT);
    textsw_normalize_internal(view, pos, pos,
			      upper_context, 0, TXTSW_NI_DEFAULT);
}

#ifdef OW_I18N
Xv_public void
textsw_normalize_view(abstract, pos)
    Textsw          abstract;
    Es_index        pos;
{
    Textsw_view_handle view = VIEW_ABS_TO_REP(abstract);
    int             upper_context;

    upper_context = (int) ev_get(view->e_view, EV_CHAIN_UPPER_CONTEXT);
    pos = textsw_wcpos_from_mbpos(FOLIO_FOR_VIEW(view), pos);
    textsw_normalize_internal(view, pos, pos,
			      upper_context, 0, TXTSW_NI_DEFAULT);
}
#endif /* OW_I18N */

/*
 *	the following two routines are identical names for the same thing
 */
Xv_public	void
#ifdef OW_I18N
textsw_possibly_normalize_wc(abstract, pos)
#else
textsw_possibly_normalize(abstract, pos)
#endif
    Textsw          abstract;
    Es_index        pos;
{
    textsw_not_visible_normalize(abstract, pos);
}

#ifdef OW_I18N
Xv_public	void
textsw_possibly_normalize(abstract, pos)
    Textsw          abstract;
    Es_index        pos;
{
    Textsw_view_handle view = VIEW_ABS_TO_REP(abstract);

    textsw_not_visible_normalize(abstract,
			textsw_wcpos_from_mbpos(FOLIO_FOR_VIEW(view), pos));
}
#endif /* OW_I18N */

Pkg_private	void
textsw_not_visible_normalize(abstract, pos)
    Textsw          abstract;
    Es_index        pos;
{
    Textsw_view_handle view = VIEW_ABS_TO_REP(abstract);
    int             upper_context;

    upper_context = (int) ev_get(view->e_view, EV_CHAIN_UPPER_CONTEXT);
    textsw_normalize_internal(view, pos, pos, upper_context, 0,
			      TXTSW_NI_NOT_IF_IN_VIEW | TXTSW_NI_MARK);
}

Pkg_private	void
textsw_possibly_normalize_and_set_selection(
				       abstract, first, last_plus_one, type)
    Textsw          abstract;
    Es_index        first, last_plus_one;
    unsigned        type;
{
    int             upper_context;
    Textsw_view_handle view = VIEW_ABS_TO_REP(abstract);

#ifdef OW_I18N
	textsw_implicit_commit(FOLIO_FOR_VIEW(view));
#endif
    upper_context = (int)
	ev_get(view->e_view, EV_CHAIN_UPPER_CONTEXT);

    textsw_normalize_internal(view, first, last_plus_one, upper_context, 0,
	   TXTSW_NI_NOT_IF_IN_VIEW | TXTSW_NI_MARK | TXTSW_NI_SELECT(type));
}

Pkg_private int
textsw_normalize_internal(
	    view, first, last_plus_one, upper_context, lower_context, flags)
    register Textsw_view_handle view;
    Es_index        first, last_plus_one;
    int             upper_context, lower_context;
    register unsigned flags;
{
    Rect            rect;
    Es_index        line_start, start;
    int             lines_above, lt_index, lines_below;
    int             normalize = TRUE;
    CHAR            newline_str[2];
    
    newline_str[0] = '\n';
    newline_str[1] = XV_ZERO;

    if (flags & TXTSW_NI_NOT_IF_IN_VIEW) {
	switch (ev_xy_in_view(view->e_view, first, &lt_index, &rect)) {
	  case EV_XY_VISIBLE:
	    normalize = FALSE;
	    break;
	  case EV_XY_RIGHT:
	    normalize = FALSE;
	    break;
	  default:
	    break;
	}
    }
    if (normalize) {
	line_start = ev_line_start(view->e_view, first);
	lines_below = textsw_screen_line_count(VIEW_REP_TO_ABS(view));
	if (flags & TXTSW_NI_AT_BOTTOM) {
	    lines_above = (lines_below > lower_context) ?
		(lines_below - (lower_context + 1)) : (lines_below - 1);
	} else
	    lines_above = (upper_context < lines_below) ? upper_context : 0;
	if (lines_above > 0) {
	    ev_find_in_esh(view->folio->views->esh, newline_str, 1,
			   line_start, (unsigned) lines_above + 1,
			   EV_FIND_BACKWARD, &start, &line_start);
	    if (start == ES_CANNOT_SET)
		line_start = 0;
	}
	/* Ensure no caret turds will leave behind */
	textsw_take_down_caret(FOLIO_FOR_VIEW(view));

	ev_set_start(view->e_view, line_start);

	/* ensure line_start really is visible now */
	lines_below -= (lines_above + 1);
	ev_make_visible(view->e_view, first, lines_below, 0, 0);

	if (!(flags & TXTSW_NI_DONT_UPDATE_SCROLLBAR))
	    textsw_update_scrollbars(FOLIO_FOR_VIEW(view), view);
    }
    /*
    * force textsw find to do pending delete selection
    */
    if (flags & EV_SEL_PD_PRIMARY)
#ifdef OW_I18N
	textsw_set_selection_wcs(VIEW_REP_TO_ABS(view), first, last_plus_one,
#else
	textsw_set_selection(VIEW_REP_TO_ABS(view), first, last_plus_one,
#endif
			     (EV_SEL_BASE_TYPE(flags) | EV_SEL_PD_PRIMARY));
    else if (EV_SEL_BASE_TYPE(flags)) {
#ifdef OW_I18N
	textsw_set_selection_wcs(VIEW_REP_TO_ABS(view), first, last_plus_one,
#else
	textsw_set_selection(VIEW_REP_TO_ABS(view), first, last_plus_one,
#endif
			     EV_SEL_BASE_TYPE(flags));
    }
}

Pkg_private     Es_index
textsw_set_insert(folio, pos)
    register Textsw_folio folio;
    register Es_index pos;
{
    register Es_index set_to;
    Es_index        boundary;

    if (TXTSW_IS_READ_ONLY(folio)) {
	return (EV_GET_INSERT(folio->views));
    }
    if (TXTSW_HAS_READ_ONLY_BOUNDARY(folio)) {
	boundary = textsw_find_mark_internal(folio,
					     folio->read_only_boundary);
	if (pos < boundary) {
	    if AN_ERROR
		(boundary == ES_INFINITY) {
	    } else
		return (EV_GET_INSERT(folio->views));
	}
    }
    /* Ensure timer is set to fix caret display */
    textsw_take_down_caret(folio);
    EV_SET_INSERT(folio->views, pos, set_to);
    ASSUME((pos == ES_INFINITY) || (pos == set_to));
    return (set_to);
}

Pkg_private          caddr_t
textsw_checkpoint_undo(abstract, undo_mark)
    Textsw          abstract;
    caddr_t         undo_mark;
{
    register Textsw_folio folio =
    FOLIO_FOR_VIEW(VIEW_ABS_TO_REP(abstract));
    caddr_t         current_mark;

    /* AGAIN/UNDO support */
    if (((long) undo_mark) < TEXTSW_INFINITY - 1) {
	current_mark = undo_mark;
    } else {
	current_mark = es_get(folio->views->esh, ES_UNDO_MARK);
    }
    if (TXTSW_DO_UNDO(folio) &&
	(((long) undo_mark) != TEXTSW_INFINITY)) {
	if (current_mark != folio->undo[0]) {
	    /* Make room for, and then record the current mark. */
	    XV_BCOPY((char *) (&folio->undo[0]), (char *) (&folio->undo[1]),
		  (int) (folio->undo_count - 1) * sizeof(folio->undo[0]));
	    folio->undo[0] = current_mark;
	}
    }
    return (current_mark);
}

Pkg_private	void
textsw_checkpoint_again(abstract)
    Textsw          abstract;
{
    register Textsw_folio folio =
    FOLIO_FOR_VIEW(VIEW_ABS_TO_REP(abstract));

    /* AGAIN/UNDO support */
    if (!TXTSW_DO_AGAIN(folio))
	return;
    if (folio->func_state & TXTSW_FUNC_AGAIN)
	return;
    folio->again_first = ES_INFINITY;
    folio->again_last_plus_one = ES_INFINITY;
    folio->again_insert_length = 0;
    if (TXTSW_STRING_IS_NULL(&folio->again[0]))
	return;
    if (folio->again_count > 1) {
	/* Make room for this action sequence. */
	textsw_free_again(folio,
			  &folio->again[folio->again_count - 1]);
	XV_BCOPY((char *) (&folio->again[0]), (char *) (&folio->again[1]),
	      (int) (folio->again_count - 1) * sizeof(folio->again[0]));
    }
    folio->again[0] = null_string;
    folio->state &= ~(TXTSW_AGAIN_HAS_FIND | TXTSW_AGAIN_HAS_MATCH);
}

Xv_public	void
#ifdef OW_I18N
textsw_set_selection_wcs(abstract, first, last_plus_one, type)
#else
textsw_set_selection(abstract, first, last_plus_one, type)
#endif
    Textsw          abstract;
    Es_index        first, last_plus_one;
    unsigned        type;
{
    extern int      ev_set_selection();
    Es_index        first_valid,max_len;
    Textsw_view_handle view = VIEW_ABS_TO_REP(abstract);
    register Textsw_folio folio = (view ? FOLIO_FOR_VIEW(view) :
				   FOLIO_ABS_TO_REP(abstract));

    textsw_take_down_caret(folio);
    type &= EV_SEL_MASK;
    if ((first == ES_INFINITY) && (last_plus_one == ES_INFINITY)) {
	ev_clear_selection(folio->views, type);
	return;
    }
    /* make sure selection falls within valid text for WRAPAROUND case
       by trying to read at the first or last_plus_one position */
    max_len = (long)es_get(folio->views->esh,ES_PS_SCRATCH_MAX_LEN);
    if (max_len != ES_INFINITY) {
	int next,result;
	CHAR buf[1];
	if (first < last_plus_one) {
	    es_set_position(folio->views->esh,first);
	} else {
	    es_set_position(folio->views->esh,last_plus_one);
	}
	next = es_read(folio->views->esh,1,buf,&result);
	if (result == 0) {
	    ev_clear_selection(folio->views,type);
	    return;
	}
    }
    ev_set_selection(folio->views, first, last_plus_one, type);
    (void) textsw_acquire_seln(folio, seln_rank_from_textsw_info(type));
    if (type & EV_SEL_PRIMARY) {
	(void) textsw_checkpoint_undo(abstract,
				      (caddr_t) TEXTSW_INFINITY - 1);
    }
}

#ifdef OW_I18N
Xv_public	void
textsw_set_selection(abstract, first, last_plus_one, type)
    Textsw          abstract;
    Es_index        first, last_plus_one;
    unsigned        type;
{
    extern int      ev_set_selection();
    Es_index        first_valid,max_len;
    Textsw_view_handle view = VIEW_ABS_TO_REP(abstract);
    register Textsw_folio folio = (view ? FOLIO_FOR_VIEW(view) :
				   FOLIO_ABS_TO_REP(abstract));

    textsw_take_down_caret(folio);
    type &= EV_SEL_MASK;
    if ((first == ES_INFINITY) && (last_plus_one == ES_INFINITY)) {
	ev_clear_selection(folio->views, type);
	return;
    }

    first = textsw_wcpos_from_mbpos(folio, first);
    last_plus_one = textsw_wcpos_from_mbpos(folio, last_plus_one);
    /* make sure selection falls within valid text for WRAPAROUND case 
       by trying to read at position */
    max_len = (int)es_get(folio->views->esh,ES_PS_SCRATCH_MAX_LEN);
    if (max_len != ES_INFINITY) {
	int next,result;
	CHAR buf[1];
	if (first < last_plus_one) {
	    es_set_position(folio->views->esh,first);
	} else {
	    es_set_position(folio->views->esh,last_plus_one);
	}
	next = es_read(folio->views->esh,1,buf,&result);
	if (result == 0) {
	    ev_clear_selection(folio->views,type);
	    return;
	}
    }
    ev_set_selection(folio->views, first, last_plus_one, type);
    (void) textsw_acquire_seln(folio, seln_rank_from_textsw_info(type));
    if (type & EV_SEL_PRIMARY) {
	(void) textsw_checkpoint_undo(abstract,
				      (caddr_t) TEXTSW_INFINITY - 1);
    }
}
#endif /* OW_I18N */

Pkg_private     Textsw_view_handle
textsw_view_for_entity_view(folio, e_view)
    Textsw_folio    folio;
    Ev_handle       e_view;
/* It is okay for the caller to pass EV_NULL for e_view. */
{
    register Textsw_view_handle textsw_view;

    FORALL_TEXT_VIEWS(folio, textsw_view) {
	if (textsw_view->e_view == e_view)
	    return (textsw_view);
    }
    return ((Textsw_view_handle) 0);
}

/*
 *	jcb	1/2/91
 *
 *	this is new code that determines if the user action is one of the
 *	mouseless keyboard select commands. if this is the case the action
 *	specified is done and the action is consumed upon return.
 *
 *	there are two other routines that handle the mouseless commands
 *	(other than pane navigation). these are in txt_event.c and txt_scroll.c
 */
Pkg_private	int
textsw_mouseless_select_event(view, ie, arg)
    register Textsw_view_handle view;
    register Event *ie;
    Notify_arg      arg;
{
	Textsw_folio    folio		= FOLIO_FOR_VIEW(view);
	char		*msg		= NULL;
	int             action		= event_action(ie);
	int		char_count	= 1;
	int             is_select_event	= TRUE;
	Ev_chain 	chain 		= folio->views;
	short		do_dir		= FALSE;
	Textsw_Caret_Direction	dir	= (Textsw_Caret_Direction)0;
	int		rep_cnt		= 1;
	int		num_lines	= view->e_view->line_table.last_plus_one;
	Es_index	old_position, new_position;
	unsigned 	sel_type;
	Es_index        first, last_plus_one;

	if (win_inputnegevent(ie))
		return FALSE;

	switch( action ) {
	case ACTION_SELECT_DATA_END:
		msg	= "ACTION_SELECT_DATA_END";
		dir	= TXTSW_DOCUMENT_END;
		break;
	case ACTION_SELECT_DATA_START:
		msg	= "ACTION_SELECT_DATA_START";
		dir	= TXTSW_DOCUMENT_START;
		break;
	case ACTION_SELECT_DOWN:
		msg	= "ACTION_SELECT_DOWN";
		dir	= TXTSW_LINE_END;
		break;
	case ACTION_SELECT_JUMP_DOWN:
		msg	= "ACTION_SELECT_JUMP_DOWN";
		dir	= TXTSW_LINE_END;
		rep_cnt	= num_lines / 2 - 1;
		break;
	case ACTION_SELECT_JUMP_LEFT:
		msg	= "ACTION_SELECT_JUMP_LEFT";
		dir	= TXTSW_WORD_BACKWARD;
		break;
	case ACTION_SELECT_JUMP_RIGHT:
		msg	= "ACTION_SELECT_JUMP_RIGHT";
		dir	= TXTSW_WORD_FORWARD;
		break;
	case ACTION_SELECT_JUMP_UP:
		msg	= "ACTION_SELECT_JUMP_UP";
		dir	= TXTSW_LINE_START;
		rep_cnt	= num_lines / 2 - 1;
		break;
	case ACTION_SELECT_LEFT:
		msg	= "ACTION_SELECT_LEFT";
		dir	= TXTSW_CHAR_BACKWARD;
		do_dir	= TRUE; /* enum type is 0 */
		break;
	case ACTION_SELECT_LINE_END:
		msg	= "ACTION_SELECT_LINE_END";
		dir	= TXTSW_LINE_END;
		break;
	case ACTION_SELECT_LINE_START:
		msg	= "ACTION_SELECT_LINE_START";
		dir	= TXTSW_LINE_START;
		break;
	case ACTION_SELECT_RIGHT:
		msg	= "ACTION_SELECT_RIGHT";
		dir	= TXTSW_CHAR_FORWARD;
		break;
	case ACTION_SELECT_PANE_DOWN:
		msg	= "ACTION_SELECT_PANE_DOWN";
		dir	= TXTSW_LINE_START;
		rep_cnt	= num_lines - 2;
		break;
	case ACTION_SELECT_PANE_LEFT:
		msg	= "ACTION_SELECT_PANE_LEFT";
		dir	= TXTSW_LINE_START;
		break;
	case ACTION_SELECT_PANE_RIGHT:
		msg	= "ACTION_SELECT_PANE_RIGHT";
		dir	= TXTSW_LINE_END;
		break;
	case ACTION_SELECT_PANE_UP:
		msg	= "ACTION_SELECT_PANE_UP";
		dir	= TXTSW_LINE_START;
		rep_cnt	= num_lines - 2;
		break;
	case ACTION_SELECT_UP:
		msg	= "ACTION_SELECT_UP";
		dir	= TXTSW_LINE_START;
		break;
	default:
		is_select_event	= FALSE;
		break;
	}

	/* anything to do? */
	if( is_select_event ) {

/*		printf("mouseless select %-40s %d\n", msg, dir); */

		/* do we know what to do? */
		if( dir != (Textsw_Caret_Direction)0 || do_dir ) {

			old_position = EV_GET_INSERT(chain);

			sel_type = textsw_determine_selection_type(folio, TRUE);
			(void) ev_get_selection(folio->views, 
						&first, &last_plus_one,
						sel_type);

			/* do the caret movement */
			do {
				textsw_move_caret(view, dir);
			} while( --rep_cnt > 0 );

			new_position = EV_GET_INSERT(chain);

/*			printf("start: position %d/%d first %d last %d dir %d\n", 
			       old_position, new_position, first, last_plus_one, dir );
*/
			if( new_position == old_position )
				goto getout;

			/* moving `left' in file */
			if( new_position < old_position ) {

				if( first == old_position )
					first	= new_position;
				else if( last_plus_one == old_position )
					last_plus_one = new_position;
				else {
					first	= new_position;
					last_plus_one = old_position;
				}
			}
			else {

				if( last_plus_one == old_position )
					last_plus_one = new_position;
				else if( first == old_position )
					first	= new_position;
				else {
					first = old_position;
					last_plus_one = new_position;
				}
			}

			if( first > last_plus_one ) {
				old_position = first;
				first = last_plus_one;
				last_plus_one = old_position;
			}

/*			printf("do:    position %d/%d first %d last %d sel_type %d\n", 
			       old_position, new_position, first, last_plus_one, sel_type );
*/
#ifdef OW_I18N
			textsw_set_selection_wcs( VIEW_REP_TO_ABS(view),
#else
			textsw_set_selection( VIEW_REP_TO_ABS(view),
#endif
					     first, last_plus_one, 
					     sel_type | EV_SEL_PD_PRIMARY);

#ifdef OW_I18N
			textsw_possibly_normalize_wc(VIEW_REP_TO_ABS(view),
#else
			textsw_possibly_normalize(VIEW_REP_TO_ABS(view),
#endif
						  new_position );
		}
	}

	getout:	

	return is_select_event;
}

static int
update_selection(view, ie)
    Textsw_view_handle view;
    register Event *ie;
{
    register Textsw_folio folio = FOLIO_FOR_VIEW(view);
    register unsigned sel_type;
    register Es_index position;
    Es_index        first, last_plus_one, ro_bdry;

    position = ev_resolve_xy(view->e_view, ie->ie_locx, ie->ie_locy);
    if (position == ES_INFINITY)
	return -1;
    sel_type = textsw_determine_selection_type(folio, TRUE);
    if (position == es_get_length(folio->views->esh)) {
	/* Check for and handle case of empty textsw. */
	if (position == 0) {
	    last_plus_one = 0;
	    goto Do_Update;
	}
	position--;		/* ev_span croaks if passed EOS */
    }
    if (folio->track_state & TXTSW_TRACK_POINT) {
	/* laf */
	if (folio->span_level == EI_SPAN_POINT) {
	    last_plus_one = position;
	} else if (folio->span_level == EI_SPAN_CHAR) {
	    last_plus_one = position;	/* +1; jcb -- no single char sel */
	} else {
	    ev_span(folio->views, position, &first, &last_plus_one,
		    (int) folio->span_level);
	    ASSERT(first != ES_CANNOT_SET);
	    position = first;
	}
	folio->adjust_pivot = position;	/* laf */
    } else if (folio->track_state & TXTSW_TRACK_WIPE) {	/* laf */
	unsigned        save_span_level = folio->span_level;
	if (folio->track_state & TXTSW_TRACK_ADJUST_END)
	    /* Adjust-from-end is char-adjust */
	    folio->span_level = EI_SPAN_CHAR;
	if ((folio->state & TXTSW_ADJUST_IS_PD) &&
	    (sel_type & EV_SEL_PRIMARY)) {
	    /*
	     * laf if (folio->state & TXTSW_CONTROL_DOWN) sel_type &=
	     * ~EV_SEL_PD_PRIMARY; else
	     */
	    sel_type |= EV_SEL_PD_PRIMARY;
	}
	if (position < folio->adjust_pivot) {
	    /* There is nothing to the left of position == 0! */
	    if (position > 0) {
		/*
		 * Note: span left only finds the left portion of what the
		 * entire span would be. The position between lines is
		 * ambiguous: is it at the end of the previous line or the
		 * beginning of the next? EI_SPAN_WORD treats it as being at
		 * the end of the inter-word space terminated by newline.
		 * EI_SPAN_LINE treats it as being at the start of the
		 * following line.
		 */
		switch (folio->span_level) {
		    /* laf */
		  case EI_SPAN_POINT:
		  case EI_SPAN_CHAR:
		    break;
		  case EI_SPAN_WORD:
		    (void) ev_span(folio->views, position + 1,
				   &first, &last_plus_one,
			       (int) folio->span_level | EI_SPAN_LEFT_ONLY);
		    position = first;
		    ASSERT(position != ES_CANNOT_SET);
		    break;
		  case EI_SPAN_LINE:
		    (void) ev_span(folio->views, position,
				   &first, &last_plus_one,
			       (int) folio->span_level | EI_SPAN_LEFT_ONLY);
		    position = first;
		    ASSERT(position != ES_CANNOT_SET);
		    break;
		}
	    }
	    last_plus_one = folio->adjust_pivot;
	} else {
	    /* laf */
	    if (folio->span_level == EI_SPAN_POINT) {
		last_plus_one = position + 1;
	    } else if (folio->span_level == EI_SPAN_CHAR) {
		last_plus_one = position + 1;
	    } else {
		(void) ev_span(folio->views, position, &first, &last_plus_one,
			       (int) folio->span_level | EI_SPAN_RIGHT_ONLY);
		ASSERT(first != ES_CANNOT_SET);
	    }
	    position = folio->adjust_pivot;
	}
	folio->span_level = save_span_level;
    } else {
	unsigned        save_span_level = folio->span_level;
	/*
	 * Adjust
	 */
	if (event_action(ie) != LOC_MOVE) {
	    (void) ev_get_selection(folio->views, &first, &last_plus_one,
				    sel_type);
	    ro_bdry = TXTSW_IS_READ_ONLY(folio) ? last_plus_one
	            : textsw_read_only_boundary_is_at(folio);
            /* 
            * If there is no current selection, get the insertion point.
            * All adjustments for r/w primary selections will be made from 
	    * the insertion point.  All adjustments for ro and secondary
	    * selections will be made from the current point (last
	    * click).  If there is no current point, the insertion 
	    * point will be used.  [7/30/92 sjoe]
            */
	    if (first == last_plus_one) {
                if ( (!((sel_type & (EV_SEL_SECONDARY|EV_SEL_PD_SECONDARY))
			|| (last_plus_one <= ro_bdry)))
			|| (first == TEXTSW_INFINITY) ) {
                    first = EV_GET_INSERT(folio->views);
                    last_plus_one = (position < first) ? first : first + 1;
		}
                else {
                    if (position < first) 
			last_plus_one++;
                }
	    }
	    if ((position == first) || (position + 1 == last_plus_one)) {
		folio->track_state |= TXTSW_TRACK_ADJUST_END;
	    } else
		folio->track_state &= ~TXTSW_TRACK_ADJUST_END;
	    folio->adjust_pivot = (
	    position < (last_plus_one + first) / 2 ? last_plus_one : first);
	}
	if (folio->track_state & TXTSW_TRACK_ADJUST_END)
	    /* Adjust-from-end is char-adjust */
	    folio->span_level = EI_SPAN_CHAR;
	if ((folio->state & TXTSW_ADJUST_IS_PD) &&
	    (sel_type & EV_SEL_PRIMARY)) {
	    if (folio->state & TXTSW_CONTROL_DOWN)
		sel_type &= ~EV_SEL_PD_PRIMARY;
	    else
		sel_type |= EV_SEL_PD_PRIMARY;
	}
	if (position < folio->adjust_pivot) {
	    /* There is nothing to the left of position == 0! */
	    if (position > 0) {
		/*
		 * Note: span left only finds the left portion of what the
		 * entire span would be. The position between lines is
		 * ambiguous: is it at the end of the previous line or the
		 * beginning of the next? EI_SPAN_WORD treats it as being at
		 * the end of the inter-word space terminated by newline.
		 * EI_SPAN_LINE treats it as being at the start of the
		 * following line.
		 */
		switch (folio->span_level) {
		    /* laf */
		  case EI_SPAN_POINT:
		  case EI_SPAN_CHAR:
		    break;
		  case EI_SPAN_WORD:
		    ev_span(folio->views, position + 1,
			    &first, &last_plus_one,
			    (int) folio->span_level | EI_SPAN_LEFT_ONLY);
		    position = first;
		    ASSERT(position != ES_CANNOT_SET);
		    break;
		  case EI_SPAN_LINE:
		    ev_span(folio->views, position,
			    &first, &last_plus_one,
			    (int) folio->span_level | EI_SPAN_LEFT_ONLY);
		    position = first;
		    ASSERT(position != ES_CANNOT_SET);
		    break;
		}
	    }
	    last_plus_one = folio->adjust_pivot;
	} else {
	    if ((folio->span_level == EI_SPAN_CHAR) ||	/* laf */
		(folio->span_level == EI_SPAN_POINT)) {
		last_plus_one = position + 1;
	    } else {
		ev_span(folio->views, position, &first, &last_plus_one,
			(int) folio->span_level | EI_SPAN_RIGHT_ONLY);
		ASSERT(first != ES_CANNOT_SET);
	    }
	    position = folio->adjust_pivot;
	}
	folio->span_level = save_span_level;
    }

Do_Update:
    if (sel_type & (EV_SEL_PD_PRIMARY | EV_SEL_PD_SECONDARY)) {
	ro_bdry = TXTSW_IS_READ_ONLY(folio) ? last_plus_one
	    : textsw_read_only_boundary_is_at(folio);
	if (last_plus_one <= ro_bdry) {
	    sel_type &= ~(EV_SEL_PD_PRIMARY | EV_SEL_PD_SECONDARY);
	}
    }
#ifdef OW_I18N
    textsw_set_selection_wcs(VIEW_REP_TO_ABS(view),
#else
    textsw_set_selection(VIEW_REP_TO_ABS(view),
#endif
			 position, last_plus_one, sel_type);
    if (sel_type & EV_SEL_PRIMARY) {
	textsw_checkpoint_again(VIEW_REP_TO_ABS(view));
    }
    ASSERT(allock());
}

Pkg_private
textsw_start_seln_tracking(view, ie)
    Textsw_view_handle view;
    register Event *ie;
{
    register Textsw_folio folio = FOLIO_FOR_VIEW(view);

    textsw_flush_caches(view, TFC_STD);
    switch (event_action(ie)) {
      case TXTSW_ADJUST:
	folio->track_state |= TXTSW_TRACK_ADJUST;
	folio->last_click_x = ie->ie_locx;
	folio->last_click_y = ie->ie_locy;
	break;
      case TXTSW_POINT:{
	    int             delta;	/* in millisecs */
	    /* laf */
	    if (folio->state & TXTSW_SHIFT_DOWN) {
		folio->track_state |= TXTSW_TRACK_ADJUST;
	    } else
		folio->track_state |= TXTSW_TRACK_POINT;
	    delta = (ie->ie_time.tv_sec - folio->last_point.tv_sec) * 1000;
	    delta += ie->ie_time.tv_usec / 1000;
	    delta -= folio->last_point.tv_usec / 1000;
	    if (delta >= folio->multi_click_timeout) {
		/* laf */
		folio->span_level = EI_SPAN_POINT;
	    } else {
		int             delta_x = ie->ie_locx - folio->last_click_x;
		int             delta_y = ie->ie_locy - folio->last_click_y;
		if (delta_x < 0)
		    delta_x = -delta_x;
		if (delta_y < 0)
		    delta_y = -delta_y;
		if (delta_x > folio->multi_click_space ||
		    delta_y > folio->multi_click_space) {
		    /* laf */
		    folio->span_level = EI_SPAN_POINT;
		} else {
		    switch (folio->span_level) {
			/* laf */
		      case EI_SPAN_POINT:
			if (!(folio->state & TXTSW_CONTROL_DOWN))
			    folio->span_level = EI_SPAN_WORD;
			else
			    folio->span_level = EI_SPAN_CHAR;
			break;
		      case EI_SPAN_WORD:
			folio->span_level = EI_SPAN_LINE;
			break;
		      case EI_SPAN_LINE:
			folio->span_level = EI_SPAN_DOCUMENT;
			break;
		      case EI_SPAN_DOCUMENT:
			/* laf */
			folio->span_level = EI_SPAN_POINT;
			break;
			/* laf */
		      default:
			folio->span_level = EI_SPAN_POINT;
		    }
		}
	    }
	    folio->last_click_x = ie->ie_locx;
	    folio->last_click_y = ie->ie_locy;
	    break;
	}
      default:
	LINT_IGNORE(ASSUME(0));
    }
    /*
     * laf if ((folio->state & TXTSW_CONTROL_DOWN) &&
     */
    if (!TXTSW_IS_READ_ONLY(folio))
	folio->state |= TXTSW_PENDING_DELETE;
    update_selection(view, ie);
}

Pkg_private     Es_index
textsw_do_balance_beam(view, x, y, first, last_plus_one)
    Textsw_view_handle view;
    int             x, y;
    register Es_index first, last_plus_one;
{
    register Ev_handle e_view;
    Es_index        new_insert;
    register Textsw_folio folio = FOLIO_FOR_VIEW(view);
    register int    delta;
    int             lt_index;
    struct rect     rect;

    if (folio->track_state & TXTSW_TRACK_ADJUST) {
	return ((first == folio->adjust_pivot) ? last_plus_one : first);
    }
    new_insert = last_plus_one;
    e_view = view->e_view;
    /*
     * When the user has multi-clicked up to select enough text to occupy
     * multiple physical display lines, linearize the distance to the
     * endpoints, then do the balance beam.
     */
    if (ev_xy_in_view(e_view, first, &lt_index, &rect)
	!= EV_XY_VISIBLE)
	goto Did_balance;
    delta = x - rect.r_left;
    delta += ((y - rect.r_top) / rect.r_height) *
	e_view->rect.r_width;
    switch (ev_xy_in_view(e_view, last_plus_one, &lt_index, &rect)) {
      case EV_XY_BELOW:
	/*
	 * SPECIAL CASE: if last_plus_one was at the start of the last line
	 * in the line table, it is still EV_XY_BELOW, and we should still
	 * treat it like EV_XY_VISIBLE, with rect.r_left ==
	 * e_view->rect.r_left.
	 */
	if (last_plus_one != ft_position_for_index(
						   e_view->line_table,
				    e_view->line_table.last_plus_one - 1)) {
	    new_insert = first;
	    goto Did_balance;
	} else {
	    rect.r_left = e_view->rect.r_left;
	}
	/* FALL THROUGH */
      case EV_XY_VISIBLE:
	if (e_view->rect.r_left == rect.r_left) {
	    /* last_plus_one must be a new_line, so back up */
	    if (ev_xy_in_view(e_view, last_plus_one - 1, &lt_index, &rect)
		!= EV_XY_VISIBLE) {
		new_insert = first;
		goto Did_balance;
	    }
	    if ((view->folio->span_level == EI_SPAN_POINT) &&
	        (view->folio->track_state != TXTSW_TRACK_WIPE) &&
		(x >= rect.r_left) && (y >= rect.r_top) &&
		(y <= rect_bottom(&rect))) {
		/*
		 * SPECIAL CASE: Selecting in the white-space past the end of
		 * a line puts the insertion point at the end of that line,
		 * rather than the beginning of the next line.
		 * 
		 * SPECIAL SPECIAL CASE: Selecting in the white-space past the
		 * end of a wrapped line (due to wide margins or word
		 * wrapping) puts the insertion point at the beginning of the
		 * next line.
		 */
		if (ev_considered_for_line(e_view, lt_index) >=
		    ev_index_for_line(e_view, lt_index + 1))
		    new_insert = last_plus_one;
		else
		    new_insert = last_plus_one - 1;
		goto Did_balance;
	    }
	}
	break;
      default:
	new_insert = first;
	goto Did_balance;
    }
    if (y < rect.r_top)
	rect.r_left += (((rect.r_top - y) / rect.r_height) + 1) *
	    e_view->rect.r_width;
    if (delta < rect.r_left - x)
	new_insert = first;
Did_balance:
    return (new_insert);
}

static	void
done_tracking(view, x, y)
    Textsw_view_handle view;
    register int    x, y;
{
    register Textsw_folio folio = FOLIO_FOR_VIEW(view);

    if (((folio->track_state & TXTSW_TRACK_SECONDARY) == 0) ||
	(folio->func_state & TXTSW_FUNC_PUT)) {
	Es_index        first, last_plus_one, new_insert;

	(void) ev_get_selection(folio->views, &first, &last_plus_one,
			    (unsigned) ((folio->func_state & TXTSW_FUNC_PUT)
				      ? EV_SEL_SECONDARY : EV_SEL_PRIMARY));
	/* laf */
	if (folio->track_state & TXTSW_TRACK_POINT &&
	    folio->span_level == EI_SPAN_POINT) {
	    if (last_plus_one != ES_INFINITY)
		last_plus_one++;
	    if (folio->state & TXTSW_CONTROL_DOWN) {
		Es_index        position;
		position = ev_resolve_xy(view->e_view, x, y);
		(void) ev_span(folio->views, position,
			       &first, &last_plus_one,
			       (int) EI_SPAN_WORD);
	    }
	}
	new_insert = textsw_do_balance_beam(view, x, y, first, last_plus_one);
	if (new_insert != ES_INFINITY) {
	    first = textsw_set_insert(folio, new_insert);
	    ASSUME(first != ES_CANNOT_SET);
	}
    }
    folio->track_state &= ~(TXTSW_TRACK_ADJUST
			    | TXTSW_TRACK_ADJUST_END
			    | TXTSW_TRACK_POINT
			    | TXTSW_TRACK_WIPE);	/* laf */
    /*
     * Reset pending-delete state (although we may be in the middle of a
     * multi-click, there is no way to know that and we must reset now in
     * case user has ADJUST_IS_PENDING_DELETE or had CONTROL_DOWN).
     */
    if ((folio->func_state & TXTSW_FUNC_DELETE) == 0)
	folio->state &= ~TXTSW_PENDING_DELETE;
}

Pkg_private int
textsw_track_selection(view, ie)
    Textsw_view_handle view;
    register Event *ie;
{
    register Textsw_folio folio = FOLIO_FOR_VIEW(view);

    /* laf */
    if ((folio->track_state & TXTSW_TRACK_MOVE) ||
	(folio->track_state & TXTSW_TRACK_DUPLICATE))
	return (FALSE);

    if (win_inputnegevent(ie)) {
	switch (event_action(ie)) {
	  case TXTSW_POINT:{
		folio->last_point = ie->ie_time;
		break;
	    }
	  case TXTSW_ADJUST:{
		folio->last_adjust = ie->ie_time;
		break;
	    }
	  default:
	    if ((folio->track_state & TXTSW_TRACK_SECONDARY) ||
		(folio->func_state & TXTSW_FUNC_ALL)) {
		done_tracking(view, ie->ie_locx, ie->ie_locy);
		return (FALSE);	/* Don't ignore: FUNC up */
	    } else {
		return (TRUE);	/* Ignore: other key up */
	    }
	}
	done_tracking(view, ie->ie_locx, ie->ie_locy);
    } else {
	switch (event_action(ie)) {
	    /*
	     * BUG:  Should interpose on the scrollbar and look for a
	     * WIN_EXIT event
	     */
	  case LOC_DRAG:
	    /* laf */
	    if ((folio->track_state & TXTSW_TRACK_MOVE) ||
		(folio->track_state & TXTSW_TRACK_DUPLICATE)) {
		break;
	    }
	    if (folio->track_state & TXTSW_TRACK_POINT) {
		/*
		 * Don't get out of point tracking until move more than
		 * a specific amount from the original click.
		 */
		if ((ie->ie_locx > (folio->last_click_x+TXTSW_X_POINT_SLOP)) ||
		    (ie->ie_locx < (folio->last_click_x-TXTSW_X_POINT_SLOP)) ||
		    (ie->ie_locy > (folio->last_click_y+TXTSW_Y_POINT_SLOP)) ||
		    (ie->ie_locy < (folio->last_click_y-TXTSW_Y_POINT_SLOP))) {
		    folio->track_state &= ~TXTSW_TRACK_POINT;
		    folio->track_state |= TXTSW_TRACK_WIPE;
		} else
		    break;
	    }
	    if (folio->track_state & TXTSW_TRACK_ADJUST) {
		folio->track_state &= ~TXTSW_TRACK_ADJUST;
		folio->track_state |= TXTSW_TRACK_WIPE;
		folio->track_state |= TXTSW_TRACK_ADJUST_END;
	    }
	    update_selection(view, ie);
	    break;
	  case LOC_WINEXIT:
	    done_tracking(view, ie->ie_locx, ie->ie_locy);
	    textsw_may_win_exit(folio);
	    break;
	  default:;		/* ignore it */
	}
    }
    return (TRUE);
}

/*
 * ===============================================================
 * 
 * Utilities to simplify dealing with selections.
 * 
 * ===============================================================
 */

Pkg_private int
textsw_get_selection_as_int(folio, type, default_value)
    register Textsw_folio folio;
    unsigned        type;
    int             default_value;

{
    Textsw_selection_object selection;
    int             result;
    CHAR            buf[20];

    textsw_init_selection_object(folio, &selection, buf, SIZEOF(buf), 0);
    result = textsw_func_selection_internal(
				  folio, &selection, type, TFS_FILL_ALWAYS);
    if (TFS_IS_ERROR(result)) {
	result = default_value;
    } else {
	buf[selection.buf_len] = '\0';
	result = ATOI(buf);
    }
    return (result);
}

Pkg_private int
textsw_get_selection_as_string(folio, type, buf, buf_max_len)
    register Textsw_folio folio;
    unsigned        type;
    CHAR           *buf;

{
    Textsw_selection_object selection;
    int             result;

    textsw_init_selection_object(folio, &selection, buf, buf_max_len, 0);
    result = textsw_func_selection_internal(
				  folio, &selection, type, TFS_FILL_ALWAYS);
    if (TFS_IS_ERROR(result) || (selection.buf_len == 0)) {
	result = 0;
    } else {
	buf[selection.buf_len] = '\0';
	result = selection.buf_len + 1;
    }
    return (result);
}
