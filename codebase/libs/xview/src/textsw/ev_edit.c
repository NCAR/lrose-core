#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)ev_edit.c 20.27 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * Initialization and finalization of entity views.
 */
#include <xview/pkg.h>
#include <xview/attrol.h>
#include <xview_private/primal.h>
#include <xview_private/ev_impl.h>
#include <xview_private/txt_18impl.h>
#include <sys/time.h>
#include <pixrect/pr_util.h>

#ifdef __STDC__ 
#ifndef CAT
#define CAT(a,b)        a ## b 
#endif 
#endif
#include <pixrect/memvar.h>

#include <pixrect/pixfont.h>

Pkg_private void     ev_notify();
Pkg_private void     ev_clear_from_margins();
Pkg_private void     ev_check_insert_visibility();
Pkg_private int      ev_clear_rect();
Pkg_private struct rect ev_rect_for_line();
Pkg_private void ev_update_after_edit();


Pkg_private unsigned
ev_span(views, position, first, last_plus_one, group_spec)
    register Ev_chain views;
    Es_index        position, *first, *last_plus_one;
    int             group_spec;
{
    struct ei_span_result span_result;
    CHAR            buf[EV_BUFSIZE];
    struct es_buf_object esbuf;
    esbuf.esh = views->esh;
    esbuf.buf = buf;
    esbuf.sizeof_buf = SIZEOF(buf);
    esbuf.first = esbuf.last_plus_one = 0;
    span_result = ei_span_of_group(
				   views->eih, &esbuf, group_spec, position);
    *first = span_result.first;
    *last_plus_one = span_result.last_plus_one;
    return (span_result.flags);
}

Pkg_private     Es_index
ev_line_start(view, position)
    register Ev_handle view;
    register Es_index position;
{
    Es_index        dummy, result;
    register Ev_impl_line_seq seq = (Ev_impl_line_seq)
    view->line_table.seq;
    if (position >= seq[0].pos) {
	/* Optimization: First try the view's line_table */
	int             lt_index = ft_bounding_index(
					       &view->line_table, position);
	if (lt_index < view->line_table.last_plus_one - 1)
	    return (seq[lt_index].pos);
	/*
	 * -1 is required to avoid mapping all large positions to the hidden
	 * extra line entry.
	 */
    }
    ev_span(view->view_chain, position, &result, &dummy,
	    EI_SPAN_LINE | EI_SPAN_LEFT_ONLY);
    if (result == ES_CANNOT_SET) {
	result = position;
    }
    return (result);
}

/*
 * Pkg_private Es_index ev_get_insert(views) Ev_chain	 views; {
 * return((EV_CHAIN_PRIVATE(views))->insert_pos); }
 * 
 * Pkg_private Es_index ev_set_insert(views, position) Ev_chain	 views;
 * Es_index	 position; { Es_handle		esh = views->esh;
 * Ev_chain_pd_handle	private = EV_CHAIN_PRIVATE(views); Es_index
 * esult;
 * 
 * result = es_set_position(esh, position); if (result != ES_CANNOT_SET) {
 * private->insert_pos = result; } return(result); }
 */

Pkg_private void
ev_update_lt_after_edit(table, before_edit, delta)
    register Ev_line_table *table;
    Es_index        before_edit;
    register long int delta;
{
    /*
     * Modifies the entries in table as follows: delta > 0 =>
     * (before_edit..EOS) incremented by delta delta < 0 =>
     * (before_edit+delta..before_edit] mapped to before_edit+delta,
     * (before_edit..EOS) decremented by ABS(delta).
     */
    register        lt_index;
    Ev_impl_line_seq line_seq = (Ev_impl_line_seq) table->seq;

    if (delta > 0) {
	/*
	 * Only entries greater than before_edit are affected.  In
	 * particular, entries at the original insertion position do not
	 * extend.
	 */
	if (before_edit < line_seq[0].pos && table->last_plus_one > 0) {
	    ft_add_delta(*table, 0, delta);
	} else {
	    lt_index = ft_bounding_index(table, before_edit);
	    if (lt_index+1 < table->last_plus_one)
		ft_add_delta(*table, lt_index + 1, delta);
	}
    } else {
	/*
	 * Entries in the deleted range are set to the range's beginning.
	 * Entries greater than before_edit are simply decremented.
	 */
	ft_set_esi_span(
			*table, before_edit + delta + 1, before_edit,
			before_edit + delta, 0);
	/* For now, let the compiler do all the cross jumping */
	if (before_edit - 1 < line_seq[0].pos && table->last_plus_one > 0) {
	    ft_add_delta(*table, 0, delta);
	} else {
	    lt_index = ft_bounding_index(table, before_edit - 1);
	    if (lt_index < table->last_plus_one)
		ft_add_delta(*table, lt_index + 1, delta);
	}
    }
}

/*
 * Meaning of returned ei_span_result.flags>>16 is: 0 success 1 illegal
 * edit_action 2 unable to set insert
 */
Pkg_private struct ei_span_result
ev_span_for_edit(views, edit_action)
    Ev_chain        views;
    int             edit_action;
{
    Ev_chain_pd_handle private = EV_CHAIN_PRIVATE(views);
    struct ei_span_result span_result;
    int             group_spec = 0;
    CHAR            buf[EV_BUFSIZE];
    struct es_buf_object esbuf;

    switch (edit_action) {
      case EV_EDIT_BACK_CHAR:{
	    group_spec = EI_SPAN_CHAR | EI_SPAN_LEFT_ONLY;
	    break;
	}
      case EV_EDIT_BACK_WORD:{
	    group_spec = EI_SPAN_WORD | EI_SPAN_LEFT_ONLY;
	    break;
	}
      case EV_EDIT_BACK_LINE:{
	    group_spec = EI_SPAN_LINE | EI_SPAN_LEFT_ONLY;
	    break;
	}
      case EV_EDIT_CHAR:{
	    group_spec = EI_SPAN_CHAR | EI_SPAN_RIGHT_ONLY;
	    break;
	}
      case EV_EDIT_WORD:{
	    group_spec = EI_SPAN_WORD | EI_SPAN_RIGHT_ONLY;
	    break;
	}
      case EV_EDIT_LINE:{
	    group_spec = EI_SPAN_LINE | EI_SPAN_RIGHT_ONLY;
	    break;
	}
      default:{
	    span_result.flags = 1 << 16;
	    goto Return;
	}
    }
    esbuf.esh = views->esh;
    esbuf.buf = buf;
    esbuf.sizeof_buf = SIZEOF(buf);
    esbuf.first = esbuf.last_plus_one = 0;
    span_result = ei_span_of_group(
		       views->eih, &esbuf, group_spec, private->insert_pos);
    if (span_result.first == ES_CANNOT_SET) {
	span_result.flags = 2 << 16;
	goto Return;
    }
    if (((group_spec & EI_SPAN_CLASS_MASK) == EI_SPAN_WORD) &&
	(span_result.flags & EI_SPAN_NOT_IN_CLASS) &&
	(span_result.flags & EI_SPAN_HIT_NEXT_LEVEL) == 0) {
	/*
	 * On a FORWARD/BACK_WORD, skip over preceding/trailing white space
	 * and delete the preceding word.
	 */
	struct ei_span_result span2_result;
	span2_result = ei_span_of_group(
					views->eih, &esbuf, group_spec,
					(group_spec & EI_SPAN_LEFT_ONLY)
			   ? span_result.first : span_result.last_plus_one);
	if (span2_result.first != ES_CANNOT_SET) {
	    if (group_spec & EI_SPAN_LEFT_ONLY)
		span_result.first = span2_result.first;
	    else
		span_result.last_plus_one = span2_result.last_plus_one;
	}
    }
Return:
    return (span_result);
}

Pkg_private int
ev_delete_span(views, first, last_plus_one, delta)
    Ev_chain        views;
    register Es_index first, last_plus_one;
    Es_index       *delta;
{
    Es_handle       esh = views->esh;
    register        Ev_chain_pd_handle
                    private = EV_CHAIN_PRIVATE(views);
    register Es_index old_length = es_get_length(esh);
    Es_index        new_insert_pos, private_insert_pos = private->insert_pos;
    int             result, used;

    /* Since *delta depends on last_plus_one, normalize ES_INFINITY */
    if (last_plus_one > old_length) {
	last_plus_one = old_length;
    }
    /* See if operation makes sense */
    if (old_length == 0) {
	result = 1;
	goto Return;
    }
    /* We cannot assume where the esh is positioned, so position first. */
    if (first != es_set_position(esh, first)) {
	result = 2;
	goto Return;
    }
    new_insert_pos = es_replace(esh, last_plus_one, 0, 0, &used);
    if (new_insert_pos == ES_CANNOT_SET) {
	result = 3;
	goto Return;
    }
    *delta = first - last_plus_one;
    private->insert_pos = new_insert_pos;
    /* Above assignment required to make following call work! */
    ev_update_after_edit(
			 views, last_plus_one, *delta, old_length, first);
    if (first < private_insert_pos) {
	if (last_plus_one < private_insert_pos)
	    private->insert_pos = private_insert_pos + *delta;
	else
	    /* Don't optimize out in case kludge above vanishes. */
	    private->insert_pos = new_insert_pos;
    } else
	private->insert_pos = private_insert_pos;
    if (private->notify_level & EV_NOTIFY_EDIT_DELETE) {
	ev_notify(views->first_view,
		  EV_ACTION_EDIT, first, old_length, first, last_plus_one,
		  0,
		  0);
    }
    result = 0;
Return:
    return (result);
}

#ifdef OW_I18N
#include <xview_private/txt_impl.h>

/* Return *delta as number of the deleted bytes. */
Pkg_private int
ev_delete_span_bytes(folio, first, last_plus_one, delta)
    Textsw_folio    folio;
    register Es_index first, last_plus_one;
    Es_index       *delta;
{
    Ev_chain        views = folio->views;
    Es_handle       esh = views->esh;
    register        Ev_chain_pd_handle
                    private = EV_CHAIN_PRIVATE(views);
    register Es_index old_length = es_get_length(esh);
    Es_index        new_insert_pos, private_insert_pos = private->insert_pos;
    Es_index        byte_delta;
    int             result, used;

    /* Since *delta depends on last_plus_one, normalize ES_INFINITY */
    if (last_plus_one > old_length) {
	last_plus_one = old_length;
    }
    /* See if operation makes sense */
    if (old_length == 0) {
	result = 1;
	goto Return;
    }
    /* We cannot assume where the esh is positioned, so position first. */
    if (first != es_set_position(esh, first)) {
	result = 2;
	goto Return;
    }

    byte_delta = (Es_index) -textsw_get_bytes_span(folio, first, last_plus_one);
    /*
     * need the following statement as textsw_get_bytes_span() changes the
     * es_position.
     */
    es_set_position(esh, first);

    new_insert_pos = es_replace(esh, last_plus_one, 0, 0, &used);
    if (new_insert_pos == ES_CANNOT_SET) {
	result = 3;
	goto Return;
    }
    *delta = first - last_plus_one;
    private->insert_pos = new_insert_pos;
    /* Above assignment required to make following call work! */
    ev_update_after_edit(
			 views, last_plus_one, *delta, old_length, first);
    if (first < private_insert_pos) {
	if (last_plus_one < private_insert_pos)
	    private->insert_pos = private_insert_pos + *delta;
	else
	    /* Don't optimize out in case kludge above vanishes. */
	    private->insert_pos = new_insert_pos;
    } else
	private->insert_pos = private_insert_pos;
    if (private->notify_level & EV_NOTIFY_EDIT_DELETE) {
	ev_notify(views->first_view,
		  EV_ACTION_EDIT, first, old_length, first, last_plus_one,
		  0,
		  0);
    }
    result = 0;
    *delta = byte_delta;
Return:
    return (result);
}
#endif /* OW_I18N */

static void
ev_update_fingers_after_edit(ft, insert, delta)
    register Ev_finger_table *ft;
    register Es_index insert;
    register int    delta;
/*
 * This routine differs from the similar ev_update_lt_after_edit in that it
 * makes use of the extra Ev_finger_info fields in order to potentially
 * adjust entries at the insert point.
 */
{
    register Ev_finger_handle fingers;
    register int    lt_index;

    if (delta != 0)
	ev_update_lt_after_edit(ft, insert, delta);
    if (delta > 0) {
	/* Correct entries that move_at_insert in views->fingers */
	lt_index = ft_bounding_index(ft, insert);
	if (lt_index < ft->last_plus_one) {
	    fingers = (Ev_finger_handle) ft->seq;
	    while (fingers[lt_index].pos == insert) {
		if (EV_MARK_MOVE_AT_INSERT(fingers[lt_index].info)) {
		    fingers[lt_index].pos += delta;
		    if (lt_index-- > 0)
			continue;
		}
		break;
	    }
	}
    }
}


Pkg_private void
ev_update_view_display(view)
    register Ev_handle view;
{
    Es_index       *line_seq;

    ev_lt_format(view, &view->tmp_line_table, &view->line_table);
    ASSERT(allock());
    /*
     * Swap line tables before paint so that old ev_display utilities may be
     * called during paint phase.
     */
    line_seq = view->line_table.seq;
    view->line_table.seq = view->tmp_line_table.seq;
    view->tmp_line_table.seq = line_seq;
    if ((int) ev_get(view, EV_NO_REPAINT_TIL_EVENT) == 0)
	ev_lt_paint(view, &view->line_table, &view->tmp_line_table);
    ASSERT(allock());
}

Pkg_private void
ev_update_after_edit(views, last_plus_one, delta, old_length, min_insert_pos)
    register Ev_chain views;
    Es_index        last_plus_one;
    int             delta;
    Es_index        old_length, min_insert_pos;
/*
 * last_plus_one:  end of inserted/deleted span delta:	   if positive,
 * amount inserted; else, amount deleted old_length:	   length before
 * change min_insert_pos: beginning of inserted/deleted span
 */
{
    register Ev_handle view;
    register Ev_pd_handle private;
    Ev_chain_pd_handle chain = EV_CHAIN_PRIVATE(views);

    ASSERT(allock());

    /*
     * Update edit number and tables to reflect change to entity stream. This
     * must be done whenever the entity stream is modified, regardless of
     * whether the display is updated.
     */
    chain->edit_number++;
    if (delta != 0) {
#ifdef OW_I18N
	chain->updated = TRUE;
#endif
	ev_update_lt_after_edit(&chain->op_bdry, last_plus_one, delta);
	ev_update_fingers_after_edit(&views->fingers, last_plus_one, delta);
    }
    FORALLVIEWS(views, view) {
	if (ev_lt_delta(view, last_plus_one, delta)) {
	    private = EV_PRIVATE(view);
	    if (private->state & EV_VS_DELAY_UPDATE) {
		private->state |= EV_VS_DAMAGED_LT;
		continue;
	    }
	    ev_update_view_display(view);
	}
    }
}

Pkg_private void
ev_update_chain_display(views)
    register Ev_chain views;
{
    register Ev_handle view;
    register Ev_pd_handle private;

    FORALLVIEWS(views, view) {
	private = EV_PRIVATE(view);
	if (private->state & EV_VS_DAMAGED_LT) {
	    ev_update_view_display(view);
	    private->state &= ~EV_VS_DAMAGED_LT;
	}
    }
}

Pkg_private void
ev_make_visible(view, position, lower_context, auto_scroll_by, delta)
    Ev_handle       view;
    Es_index        position;
    int             lower_context;
    int             auto_scroll_by;
    int             delta;
{
    Pkg_private int      ev_xy_in_view();
    Ev_impl_line_seq line_seq;
    int             top_of_lc;
    int             lt_index;
    struct rect     rect;
    Es_index	    new_top, old_top;
    Es_index        start, lines_above, lines_below, line_start;
    CHAR            newline_str[2];

    newline_str[0] = '\n';
    newline_str[1] = XV_ZERO;
 
    line_seq = (Ev_impl_line_seq) view->line_table.seq;
    top_of_lc = MAX(1,
		    view->line_table.last_plus_one - 1 - lower_context);
    /*
     * Following test works even if line_seq[].pos == ES_INFINITY. The test
     * catches the common cases and saves the expensive call on
     * ev_xy_in_view().
     */
    if (position < line_seq[top_of_lc].pos)
	return;
    switch (ev_xy_in_view(view, position, &lt_index, &rect)) {
      case EV_XY_BELOW:
	/* BUG ALERT: The following heuristic must be replaced! */
#ifdef SCROLL_DEBUG
        printf("delta=%d,pos=%d,lineseqpos=%d,last+one=%d,lcontext=%d,autoscrby=%d\n",
               delta, position, line_seq[top_of_lc].pos,
               view->line_table.last_plus_one, lower_context, auto_scroll_by);
#endif
#ifndef __linux
	delta = MIN(delta, position - line_seq[top_of_lc].pos);
#endif
	if (delta < 50 * view->line_table.last_plus_one
	    	&& lower_context < view->line_table.last_plus_one - 1
	    	&& auto_scroll_by < view->line_table.last_plus_one - 1) {
	    old_top = line_seq[0].pos;
#ifdef SCROLL_DEBUG
	printf("ev_scroll_lines(), MAX()=%d, total=%d\n", 
	      MAX(1, MAX(lower_context, auto_scroll_by) + (delta / 50)),
	      MIN(view->line_table.last_plus_one-1,MAX(1, MAX(lower_context, auto_scroll_by) + (delta / 50)))
	);
#endif
	    new_top = ev_scroll_lines(view,
				      MIN(view->line_table.last_plus_one - 1,
					  MAX(1,
					  MAX(lower_context, auto_scroll_by)
					      + (delta / 50))), FALSE);
	} else {
	    /*Set the view to close to the bottom of a large
	    * scroll, then manually scroll until the position
	    * is visible.
	    */
            line_start = ev_line_start(view, position);
            lines_below = view->line_table.last_plus_one - 1;
            lines_above = lines_below - (lower_context + 1);
            if (lines_above > 0) {
                ev_find_in_esh(view->view_chain->esh, newline_str, 1,
                                line_start, (unsigned) lines_above + 1,
                                EV_FIND_BACKWARD, &start, &line_start);
                if (start == ES_CANNOT_SET)
                    line_start = 0;
            }
	    ev_set_start(view, line_start);
#ifdef SCROLL_DEBUG
	printf("line_start=%d, lines_below=%d, lines_above=%d, scroll(2)\n",
		line_start, lines_below, lines_above);
#endif
	    new_top = ev_scroll_lines(view, 2, FALSE);
	}
	/*
	 * ev_scroll_lines swaps line tables; cannot continue using
	 * line_seq
	 */
        while ((old_top != new_top) &&
               (position >= ev_index_for_line(view, top_of_lc))) {
#ifdef SCROLL_DEBUG
	printf("old_top=%d, new_top=%d, position=%d ev_scroll_lines(2)\n",
		old_top, new_top, position);
#endif
            old_top = new_top;
            new_top = ev_scroll_lines(view,
                        ((position - ev_index_for_line(view, top_of_lc))
                        > 150 ? 3 : 2), FALSE);
        }
	break;
      case EV_XY_RIGHT:
      case EV_XY_VISIBLE:
	/* only scroll if at least 1 newline was inserted */
	if ((EV_PRIVATE(view))->cached_insert_info.lt_index != lt_index) {
	    /* We know lt_index >= top_of_lc */
	    new_top = ev_scroll_lines(view,
				      MIN(lt_index,
					  MAX(lt_index - top_of_lc + 1,
					      auto_scroll_by)), FALSE);
	    ASSUME(new_top >= 0);
	}
	break;
      default:
	break;
    }
}

Pkg_private void
ev_scroll_if_old_insert_visible(views, insert_pos, delta)
    register Ev_chain views;
    register Es_index insert_pos;
    register int    delta;
{
    register Ev_handle view;
    register Ev_pd_handle private;
    Ev_chain_pd_handle chain = EV_CHAIN_PRIVATE(views);

    if (delta <= 0)
	/* BUG ALERT!  We are not handling upper_context auto_scroll. */
	return;
    /* Scroll views which had old_insert_pos visible, but not new. */
    FORALLVIEWS(views, view) {
	private = EV_PRIVATE(view);
	if ((private->state & EV_VS_INSERT_WAS_IN_VIEW) == 0)
	    continue /* FORALLVIEWS */ ;
	ev_make_visible(view, insert_pos,
			chain->lower_context, chain->auto_scroll_by,
			delta);
    }
}

Pkg_private int
ev_input_partial(views, buf, buf_len)
    register Ev_chain views;
    CHAR           *buf;
    long int        buf_len;
{
    register        Ev_chain_pd_handle
                    private = EV_CHAIN_PRIVATE(views);
    register Es_index new_insert_pos, old_insert_pos;
    int             used;

    /* We cannot assume where the esh is positioned, so position first. */
    old_insert_pos = es_set_position(views->esh, private->insert_pos);
    if (old_insert_pos != private->insert_pos) {
	return (ES_CANNOT_SET);
    }
    new_insert_pos = es_replace(views->esh, old_insert_pos, buf_len, buf,
				&used);
    if (new_insert_pos == ES_CANNOT_SET || used != buf_len) {
	return (ES_CANNOT_SET);
    }
    private->insert_pos = new_insert_pos;
    return (0);
}

Pkg_private int
ev_input_after(views, old_insert_pos, old_length)
    register Ev_chain views;
    Es_index        old_insert_pos, old_length;
{
    Ev_chain_pd_handle private = EV_CHAIN_PRIVATE(views);
    Es_index        delta = private->insert_pos - old_insert_pos;

    /* Update all of the views' data structures */
    ev_update_after_edit(
		  views, old_insert_pos, delta, old_length, old_insert_pos);
    if (private->lower_context != EV_NO_CONTEXT) {
	ev_scroll_if_old_insert_visible(views, private->insert_pos,
					delta);
    }
    return (delta);
}

#ifdef EXAMPLE
Pkg_private int
ev_process_input(views, buf, buf_len)
    register Ev_chain views;
    CHAR           *buf;
    int             buf_len;
{
    Ev_chain_pd_handle private = EV_CHAIN_PRIVATE(views);
    Es_index        old_length = es_get_length(views->esh);
    Es_index        old_insert_pos = private->insert_pos;
    int             delta;

    if (private->lower_context != EV_NO_CONTEXT) {
	ev_check_insert_visibility(views);
    }
    delta = ev_input_partial(views, buf, buf_len);
    if (delta != ES_CANNOT_SET)
	delta = ev_input_after(views, old_insert_pos, old_length);
    return (delta);
}

#endif
