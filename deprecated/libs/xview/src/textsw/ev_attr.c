#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)ev_attr.c 20.27 93/06/28";
#endif
#endif
/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * Attribute set/get routines for entity views.
 */
#include <xview/pkg.h>
#include <xview/attrol.h>
#include <xview_private/primal.h>
#include <xview_private/ev_impl.h>

static Ev_status ev_set_internal();
static void     ev_set_rect();


#define SET_BOOL_FLAG(flags, to_test, flag)				\
	if ((unsigned)(to_test) != 0) (flags) |= (flag);		\
	else (flags) &= ~(flag)

#define EV_FLUSH_VIEW_CACHES(ev_private)\
	ev_private->cached_insert_info.edit_number = 0;\
	ev_private->cached_line_info.edit_number = 0;

Pkg_private          Xv_opaque
ev_get(view, attribute, args1, args2, args3)
    Ev_handle       view;
    Ev_attribute    attribute;
    Xv_opaque       args1, args2, args3;
{
    register Ev_pd_handle private = EV_PRIVATE(view);
    Ev_chain        chain;
    Ev_chain_pd_handle chain_private;

    switch (attribute) {
      case EV_CHAIN_AUTO_SCROLL_BY:
      case EV_CHAIN_CARET:
      case EV_CHAIN_CARET_IS_GHOST:
      case EV_CHAIN_DATA:
      case EV_CHAIN_DELAY_UPDATE:
      case EV_CHAIN_EDIT_NUMBER:
      case EV_CHAIN_ESH:
      case EV_CHAIN_GHOST:
      case EV_CHAIN_LOWER_CONTEXT:
      case EV_CHAIN_NOTIFY_LEVEL:
      case EV_CHAIN_NOTIFY_PROC:
      case EV_CHAIN_UPPER_CONTEXT:
	chain = view->view_chain;
	chain_private = EV_CHAIN_PRIVATE(chain);
	switch (attribute) {
	  case EV_CHAIN_AUTO_SCROLL_BY:
	    return ((Xv_opaque) chain_private->auto_scroll_by);
	  case EV_CHAIN_DATA:
	    return ((Xv_opaque) chain->client_data);
	  case EV_CHAIN_DELAY_UPDATE:
	    return ((Xv_opaque)
		    ((EV_PRIVATE(chain->first_view))->state &
		     EV_VS_DELAY_UPDATE));
	  case EV_CHAIN_EDIT_NUMBER:
	    return ((Xv_opaque) chain_private->edit_number);
	  case EV_CHAIN_ESH:
	    return ((Xv_opaque) chain->esh);
	  case EV_CHAIN_LOWER_CONTEXT:
	    return ((Xv_opaque) chain_private->lower_context);
	  case EV_CHAIN_NOTIFY_LEVEL:
	    return ((Xv_opaque) chain_private->notify_level);
	  case EV_CHAIN_NOTIFY_PROC:
	    return ((Xv_opaque) chain_private->notify_proc);
	  case EV_CHAIN_UPPER_CONTEXT:
	    return ((Xv_opaque) chain_private->upper_context);
	  case EV_CHAIN_CARET:
	  case EV_CHAIN_GHOST:{
		struct pixrect **pr = (struct pixrect **) args1;
		int            *hotx = (int *) args2;
		int            *hoty = (int *) args3;

		if (attribute == EV_CHAIN_CARET) {
		    *pr = chain_private->caret_pr;
		    *hotx = chain_private->caret_hotpoint.x;
		    *hoty = chain_private->caret_hotpoint.y;
		} else {
		    *pr = chain_private->ghost_pr;
		    *hotx = chain_private->ghost_hotpoint.x;
		    *hoty = chain_private->ghost_hotpoint.y;
		}
		return (args1);
	    }
	  case EV_CHAIN_CARET_IS_GHOST:
	    return ((Xv_opaque) chain_private->caret_is_ghost);
	}
      case EV_DISPLAY_START:
	return ((Xv_opaque) EV_VIEW_FIRST(view));
      case EV_LEFT_MARGIN:
	return ((Xv_opaque) private->left_margin);
      case EV_NO_REPAINT_TIL_EVENT:{
	    return ((Xv_opaque) (private->state & EV_VS_NO_REPAINT_TIL_EVENT));
	}
      case EV_RECT:{
	    Rect           *rect = (Rect *) args1;
	    *rect = view->rect;
	    return (args1);
	}
      case EV_RIGHT_BREAK:
	return ((Xv_opaque) private->right_break);
      case EV_RIGHT_MARGIN:
	return ((Xv_opaque) private->right_margin);
      default:
	return ((Xv_opaque) 0);
    }
}

Pkg_private          Ev_status
#ifdef ANSI_FUNC_PROTO
ev_set(Ev_handle view, ...)
#else
ev_set(view, va_alist)
    Ev_handle       view;
va_dcl
#endif
{
    AVLIST_DECL;
    Ev_chain        chain;
    va_list         args;

    VA_START(args, view);
    if (view) {
	chain = view->view_chain;
    } else {
	chain = (Ev_chain) va_arg(args, Ev_chain);
    }
    MAKE_AVLIST( args, avlist );
    va_end(args);
    return (ev_set_internal(view, chain, avlist));
}


/*
 * Private routines to support ev_get/set.
 */

static void
ev_adjust_start(chain, only_view, to, incremental)
    register Ev_chain chain;
    register Ev_handle only_view;
    register Es_index to;
    int             incremental;
{
    register Ev_handle next;
    Es_index        first, last_plus_one;

    FORALLVIEWS(chain, next) {
	if (!((only_view == EV_NULL) || (next == only_view)))
	    continue;
	if (to & EV_START_SPECIAL) {
	    switch (to) {
	      case EV_START_CURRENT_POS:
		ev_view_range(next, &first, &last_plus_one);
		if (first < es_get_length(chain->esh)) {
		    break;
		}
		/* else fall through and reset */
	      default:
		/* BUG ALERT! Don't do most specials yet. */
		/* next->line_table.seq[0] = 0; */
		if (next->line_table.last_plus_one > 0)
		    ft_set(next->line_table, 0, 1, 0, NULL);
	    }
	    ev_display_view(next);
	} else {
	    if (incremental) {
		ev_set_start(next, to);
	    } else {
		/* next->line_table.seq[0] = to; */
		if (next->line_table.last_plus_one > 0)
		    ft_set(next->line_table, 0, 1, to, NULL);
		ev_display_view(next);
	    }
	}
    }
}

static          Ev_status
ev_set_internal(view, chain, attrs)
    register Ev_handle view;
    Ev_chain        chain;
    register Attr_avlist attrs;
{
    Ev_status       status;
    register int    all_views = FALSE;
    Rect           *clip_rect = 0;
    Ev_display_level display_level = EV_DISPLAY;
    Es_index        display_start = ES_CANNOT_SET;
    Ev_chain_pd_handle chain_private = EV_CHAIN_PRIVATE(chain);
    register int    temp;
    register Ev_pd_handle private;
    register Ev_handle next;

    status = EV_STATUS_OKAY;
    if (view)
	private = EV_PRIVATE(view);
    for (; *attrs; attrs = attr_next(attrs)) {
	switch ((Ev_attribute) (*attrs)) {
	  case EV_END_ALL_VIEWS:
	    all_views = FALSE;
	    break;
	  case EV_FOR_ALL_VIEWS:
	    all_views = TRUE;
	    break;
	  case EV_CHAIN_DATA:{
		chain->client_data = attrs[1];
		break;
	    }
	  case EV_CHAIN_DELAY_UPDATE:{
		FORALLVIEWS(chain, next) {
		    if ((int) attrs[1]) {
			EV_PRIVATE(next)->state |= EV_VS_DELAY_UPDATE;
		    } else {
			EV_PRIVATE(next)->state &= ~EV_VS_DELAY_UPDATE;
		    }
		}
		break;
	    }
	  case EV_CHAIN_EDIT_NUMBER:{
		chain_private->edit_number = (int) (attrs[1]);
		break;
	    }
	  case EV_CHAIN_ESH:{
		Es_index        new_length;
		chain->esh = (Es_handle) attrs[1];
		new_length = es_get_length(chain->esh);
		if (chain_private->insert_pos > new_length)
		    chain_private->insert_pos = new_length;
		if (display_level == EV_DISPLAY) {
		    FORALLVIEWS(chain, next) {
			if (!(all_views || (next == view)))
			    continue;
			ev_display_view(next);
		    }
		}
		/* Flush all of our cached information */
		FORALLVIEWS(chain, next) {
		    EV_FLUSH_VIEW_CACHES(EV_PRIVATE(next));
		}
		chain_private->cache_pos_for_file_line.edit_number = 0;
		break;
	    }
	  case EV_CHAIN_AUTO_SCROLL_BY:{
		chain_private->auto_scroll_by = (int) (attrs[1]);
		break;
	    }
	  case EV_CHAIN_LOWER_CONTEXT:{
		chain_private->lower_context = (int) (attrs[1]);
		break;
	    }
	  case EV_CHAIN_NOTIFY_LEVEL:{
		chain_private->notify_level = (int) (attrs[1]);
		break;
	    }
	  case EV_CHAIN_NOTIFY_PROC:{
		chain_private->notify_proc = (int (*) ()) attrs[1];
		if (chain_private->notify_level == 0)
		    chain_private->notify_level = EV_NOTIFY_ALL;
		break;
	    }
	  case EV_CHAIN_UPPER_CONTEXT:{
		chain_private->upper_context = (int) (attrs[1]);
		break;
	    }
	  case EV_CLIP_RECT:
	    clip_rect = (Rect *) attrs[1];
	    break;
	  case EV_DATA:
	    ASSERT(view);
	    view->client_data = attrs[1];
	    break;
	  case EV_DISPLAY_LEVEL:
	    ASSERT(view);
	    switch ((Ev_display_level) (attrs[1])) {
	      case EV_DISPLAY:
		if (display_level != EV_DISPLAY) {
		    ev_adjust_start(chain, (all_views) ? EV_NULL : view,
				    (display_start == ES_CANNOT_SET)
				    ? EV_START_CURRENT_POS :
				    display_start,
				    FALSE);
		}
		/* Fall through */
	      case EV_DISPLAY_NONE:
	      case EV_DISPLAY_ONLY_CLEAR:
	      case EV_DISPLAY_NO_CLEAR:
		display_level = (Ev_display_level) (attrs[1]);
		break;
	      default:
		status = EV_STATUS_BAD_ATTR_VALUE;
	    }
	    break;
	  case EV_DISPLAY_START:{
		ASSERT(view);
		display_start = (int) (attrs[1]);
		if (display_level == EV_DISPLAY) {
		    ev_adjust_start(chain, (all_views) ? EV_NULL : view,
				    display_start, TRUE);
		}
		break;
	    }
	  case EV_LEFT_MARGIN:
	    /* BUG ALERT: does not support all_views */
	    ASSERT(view);
	    if ((int) (attrs[1]) >= 0) {
		temp = private->left_margin;
		private->left_margin = (int) (attrs[1]);
		temp -= private->left_margin;
		view->rect.r_left -= temp;
		view->rect.r_width += temp;
		EV_FLUSH_VIEW_CACHES(EV_PRIVATE(view));
	    }
	    break;
	  case EV_NO_REPAINT_TIL_EVENT:{
		ASSERT(view);
		if (attrs[1])
		    private->state |= EV_VS_NO_REPAINT_TIL_EVENT;
		else
		    private->state &= ~EV_VS_NO_REPAINT_TIL_EVENT;
		break;
	    }
	  case EV_RECT:{
		ASSERT(view);
		ev_set_rect(view, (Rect *) attrs[1], clip_rect);
		break;
	    }
	  case EV_RIGHT_BREAK:
	    ASSERT(view);
	    switch ((Ev_right_break) (attrs[1])) {
	      case EV_CLIP:
	      case EV_WRAP_AT_CHAR:
	      case EV_WRAP_AT_WORD:
		private->right_break = (Ev_right_break) (attrs[1]);
		EV_FLUSH_VIEW_CACHES(EV_PRIVATE(view));
		break;
	      default:
		status = EV_STATUS_BAD_ATTR_VALUE;
	    }
	    break;
	  case EV_RIGHT_MARGIN:
	    /* BUG ALERT: does not support all_views */
	    ASSERT(view);
	    if ((int) (attrs[1]) >= 0) {
		temp = private->right_margin;
		private->right_margin = (int) (attrs[1]);
		temp -= private->right_margin;
		view->rect.r_width += temp;
	    }
	    break;
	  case EV_SAME_AS:{
		Ev_handle       other_view = (Ev_handle) attrs[1];
		ASSERT(view);
		if (other_view != EV_NULL) {
		    Ev_pd_handle    other_private = EV_PRIVATE(other_view);
		    view->rect.r_left +=
			other_private->left_margin - private->left_margin;
		    view->rect.r_width -=
			other_private->left_margin - private->left_margin;
		    private->left_margin = other_private->left_margin;
		    private->right_break = other_private->right_break;
		    view->rect.r_width -=
			other_private->right_margin - private->right_margin;
		    private->right_margin = other_private->right_margin;
		    EV_FLUSH_VIEW_CACHES(EV_PRIVATE(view));
		}
		break;
	    }
	  case EV_CHAIN_CARET:{
		chain_private->caret_pr =
		    (struct pixrect *) attrs[1];
		chain_private->caret_hotpoint.x = (int) (attrs[2]);
		chain_private->caret_hotpoint.y = (int) (attrs[3]);
		break;
	    }
	  case EV_CHAIN_GHOST:{
		chain_private->ghost_pr =
		    (struct pixrect *) attrs[1];
		chain_private->ghost_hotpoint.x = (int) (attrs[2]);
		chain_private->ghost_hotpoint.y = (int) (attrs[3]);
		break;
	    }
	  case EV_CHAIN_CARET_IS_GHOST:{
		chain_private->caret_is_ghost = (int) (attrs[1]);
		break;
	    }
	  default:
	    status = EV_STATUS_BAD_ATTR;
	    break;
	}
    }
    return (status);
}

Pkg_private void
#ifdef ANSI_FUNC_PROTO
ev_notify(Ev_handle view, ...)
#else
ev_notify(view, va_alist)
    register Ev_handle view;
va_dcl
#endif
{
    Ev_chain        chain = view->view_chain;
    Ev_chain_pd_handle private = EV_CHAIN_PRIVATE(chain);
    Attr_attribute      attr_argv[ATTR_STANDARD_SIZE];
    va_list         args;

    if (private->notify_proc == 0)
	return;
    VA_START(args, view);
    copy_va_to_av( args, attr_argv + 2, 0 );
    va_end(args);
    attr_argv[0] = (Attr_attribute) EV_ACTION_VIEW;
    attr_argv[1] = (Attr_attribute) view;
    private->notify_proc(chain->client_data, attr_argv);
}


static void
ev_set_rect(view, rect, intersect_rect)
    register Ev_handle view;
    register struct rect *rect, *intersect_rect;
{
    Pkg_private struct rect ev_rect_for_line();
    int             lines;
    register int    old_lines = view->line_table.last_plus_one;
    struct rect     clear_rect;

    EV_FLUSH_VIEW_CACHES(EV_PRIVATE(view));

    /* See note in ev_ft_for_rect about +1 */
    lines = ei_lines_in_rect(view->view_chain->eih, rect) + 1;

    if (EV_VIEW_FIRST(view) != ES_INFINITY) {

	Ev_pd_handle    private = EV_PRIVATE(view);

	if ((view->rect.r_width == rect->r_width) &&
	    (view->rect.r_height >= rect->r_height)) {
	    clear_rect = ev_rect_for_line(view, (lines - 1));
	} else
	    clear_rect = *rect;

	ev_clear_rect(view, &clear_rect);

	private->state |= EV_VS_SET_CLIPPING;
    }
    view->rect = *rect;
    /*
     * BUG ALERT!  Not clear that this is really a safe way to get around
     * problem of calling setting rect to be too small.
     */
    if (view->rect.r_width < 24)
	view->rect.r_width = 24;


    if ((lines - old_lines) != 0) {
	ft_expand(&view->line_table, lines - old_lines);
	ft_expand(&view->tmp_line_table, lines - old_lines);
    }
    if (intersect_rect &&
	rect_intersectsrect(intersect_rect, &view->rect)) {
	ev_display_in_rect(view, intersect_rect);
    }
}
