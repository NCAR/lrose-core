#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)ev_display.c 20.60 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * Display of entity views.
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
#include <xview/window.h>
#include <xview/pixwin.h>
#include <xview_private/pw_impl.h>
#include <X11/Xlib.h>
#include <xview_private/tty_impl.h>
#include <xview_private/txt_impl.h>
#define X11R6

Pkg_private void     ev_notify();
Pkg_private int      ev_remove_op_bdry();
Pkg_private unsigned ev_op_bdry_info(), ev_op_bdry_info_merge();
Pkg_private Es_index ev_line_start();
Pkg_private Op_bdry_handle ev_add_op_bdry();
Pkg_private void ev_extend_to_view_bottom();

Pkg_private Es_index ev_index_for_line();
Pkg_private struct ei_process_result ev_display_internal();
Pkg_private void ev_display_in_rect();

/*
 * The following code is a short-circuit to use simpler output for the carets
 * that the initial xv_rop() implementation. It uses pixmaps that are just
 * put onto the display as needed.
 * 
 * Note that this is initialized the first time that the caret rendering code is
 * called.  	jcb 5/1/89
 * 
 */

#define	NORMAL_CARET	1
#define	GHOST_CARET	2
#define CARET_WIDTH	7
#define CARET_HEIGHT	7

static int EV_GHOST_KEY;
static int EV_NORMAL_KEY;
int EV_GC_KEY;

static char     normal_bits[] = {0x08, 0x1c, 0x1c, 0x3e, 0x3e, 0x7f, 0x7f};
static char     ghost_bits[] = {0x08, 0x14, 0x2a, 0x55, 0x2a, 0x14, 0x08};

static void
  ev_swap_line_table(Ev_line_table  *table1, Ev_line_table  *table2);

/*
 * create pixmaps and gc for rendering
 */
static	void
ev_init_X_carets(window)
    Xv_opaque       window;
{
    Xv_Drawable_info *info;
    Display        *display;
    Window          drawable;
    int             fore, back;
    unsigned long   gcvaluemask;
    XGCValues       gcvalues;
    GC             *gc;
    Pixmap   	    ghost, normal;

    DRAWABLE_INFO_MACRO(window, info);
    display = xv_display(info);
    drawable = xv_xid(info);
    fore = xv_fg(info);
    back = xv_bg(info);

    ghost = XCreateBitmapFromData(display, drawable,
				  ghost_bits, CARET_WIDTH, CARET_HEIGHT);
    xv_set(xv_screen(info), XV_KEY_DATA, EV_GHOST_KEY, ghost, NULL);


    normal = XCreateBitmapFromData(display, drawable,
				   normal_bits, CARET_WIDTH, CARET_HEIGHT);
    xv_set(xv_screen(info), XV_KEY_DATA, EV_NORMAL_KEY, normal, NULL);

    gcvalues.function = GXxor;
    gcvalues.plane_mask = AllPlanes;
    gcvalues.foreground = fore ^ back;	/* this causes the inversion */
    gcvalues.background = back;

    gcvaluemask = GCFunction | GCPlaneMask |
	GCForeground | GCBackground;

	gc = (GC *) calloc(1, sizeof(GC));
	gc[0] = XCreateGC(display, drawable, gcvaluemask, &gcvalues);
	xv_set(window, XV_KEY_DATA, EV_GC_KEY, gc, NULL);
}

static	void
ev_put_caret(window, which, x, y)
    Xv_opaque       window;
    int             which;
    int             x, y;
{
    Display        *display;
    Window          drawable;
    Xv_Drawable_info *info;
    Pixmap          pixmap;
    int             new_fg, new_bg;
    XGCValues       gcvalues, *gv;
    GC         *gc_list;
    GC          gc;
#ifdef X11R6
    XGCValues		lumpi_tmp;
#endif
    static Xv_opaque last_screen = 0;
    static Pixmap      ghost, normal;
    

    DRAWABLE_INFO_MACRO(window, info);
    display = xv_display(info);
    drawable = xv_xid(info);

    if (!EV_GHOST_KEY) {
	EV_GHOST_KEY = xv_unique_key();
	EV_NORMAL_KEY = xv_unique_key();
    }
    if (!EV_GC_KEY)
	EV_GC_KEY = xv_unique_key();
    gc_list = (GC *) xv_get(window, XV_KEY_DATA, EV_GC_KEY);
    if (!gc_list) {
	ev_init_X_carets(window);
	gc_list = (GC *) xv_get(window, XV_KEY_DATA, EV_GC_KEY);
    }
    gc = gc_list[0];

    if (last_screen != xv_screen(info) ) {
	normal = xv_get(xv_screen(info), XV_KEY_DATA, EV_NORMAL_KEY);
	ghost = xv_get(xv_screen(info), XV_KEY_DATA, EV_GHOST_KEY);
	last_screen = xv_screen(info);
    }
    switch (which) {
      case NORMAL_CARET:
	pixmap = normal;
	break;
      case GHOST_CARET:
	pixmap = ghost;
	break;
    }

#ifdef X11R6
	/* lumpi@dobag.in-berlin.de */
	XGetGCValues(display, gc, GCForeground|GCBackground, &lumpi_tmp);
    gv = &lumpi_tmp;
#else
    gv = &gc->values;
#endif
    new_fg = xv_fg(info);
    new_bg = xv_bg(info);
    if ((new_fg != gv->foreground) || (new_bg != gv->background)) {
	gcvalues.foreground = new_fg ^ new_bg;
	gcvalues.background = new_bg;
	XChangeGC(display, gc, GCForeground | GCBackground, &gcvalues);
    }

    /* NOTE: caching the mask (ie -- not setting every time) flunks. */
    /* set mask & orientation correctly and fill the mask */
    XSetClipMask(display, gc, pixmap);
    XSetClipOrigin(display, gc, x, y);
    XFillRectangle(display, drawable, gc, x, y,
		   CARET_WIDTH, CARET_HEIGHT);
		   
}

/* NOT USED */
/* ARGSUSED */
Pkg_private void
ev_note_esh_modified(views, first, last_plus_one, by)
    register Ev_chain views;
    Es_index        first, last_plus_one;	/* Currently unused */
    int             by;		/* Currently unused */
/*
 * This routine must be called by clients who modify the esh associated with
 * the views directly, thereby allowing any cached information dependent on
 * the esh to be updated or invalidated.
 */
{
    register Ev_handle view;
    register Ev_pd_handle private;

    FORALLVIEWS(views, view) {
	private = EV_PRIVATE(view);
	/* For now, be overly conservative. */
	private->cached_insert_info.edit_number = 0;
	private->cached_line_info.edit_number = 0;
    }
}

Pkg_private int
ev_check_cached_pos_info(view, pos, cache)
    register Ev_handle view;
    register Es_index pos;
    register Ev_pos_info *cache;
{
    struct rect     rect;

    if (!EV_CACHED_POS_INFO_IS_VALID(view, pos, cache)) {
	Ev_chain_pd_handle chain_private =
	EV_CHAIN_PRIVATE(view->view_chain);
	cache->index_of_first = EV_VIEW_FIRST(view);
	cache->pos = pos;
	cache->edit_number = chain_private->edit_number;
	if (pos == ES_CANNOT_SET) {
	    cache->lt_index = EV_NOT_IN_VIEW_LT_INDEX;
	} else {
	    switch (ev_xy_in_view(view, pos, &cache->lt_index, &rect)) {
	      case EV_XY_VISIBLE:
		cache->pr_pos.x = rect.r_left;
		cache->pr_pos.y = rect_bottom(&rect);
		break;
	      case EV_XY_RIGHT:
		cache->pr_pos.x = rect_right(&view->rect) + 1;
		cache->pr_pos.y = EV_NULL_DIM;
		break;
	      default:
		cache->lt_index = EV_NOT_IN_VIEW_LT_INDEX;
		break;
	    }
	}
    }
    return (cache->lt_index != EV_NOT_IN_VIEW_LT_INDEX);
}

/* NOT USED */
Pkg_private 	int
ev_insert_to_xy(view, x, y)
    register Ev_handle view;
    int            *x, *y;
{
    Ev_pd_handle    private = EV_PRIVATE(view);

    if (EV_INSERT_VISIBLE_IN_VIEW(view)) {
	*x = private->cached_insert_info.pr_pos.x;
	*y = private->cached_insert_info.pr_pos.y;
	return (TRUE);
    } else {
	return (FALSE);
    }
}

#ifdef OW_I18N
#ifdef FULL_R5
Pkg_private 	int
ev_caret_to_xy(view, x, y)
    register Ev_handle view;
    int            *x, *y;
{
    Ev_pd_handle    private = EV_PRIVATE(view);

    if (EV_INSERT_VISIBLE_IN_VIEW(view)) {
	*x = private->caret_pr_pos.x;
	*y = private->caret_pr_pos.y;
	return (TRUE);
    } else {
	return (FALSE);
    }
}
#endif /* FULL_R5 */	    
#endif /* OW_I18N */	


Pkg_private 	void
ev_check_insert_visibility(views)
    register Ev_chain views;
{
    /*
     * This routine does not use EV_INSERT_VISIBLE_IN_VIEW to avoid procedure
     * call overhead, redundant field extraction and to make better use of
     * registers.
     */
    register Ev_handle view;
    register Ev_pd_handle private;
    Ev_chain_pd_handle chain_private = EV_CHAIN_PRIVATE(views);
    Es_index        insert_pos;

    insert_pos = chain_private->insert_pos;
    FORALLVIEWS(views, view) {
	private = EV_PRIVATE(view);
	if (ev_check_cached_pos_info(view, insert_pos,
				     &private->cached_insert_info)) {
	    private->state |= EV_VS_INSERT_WAS_IN_VIEW;
	    if (rect_includespoint(
				   &view->rect,
				   private->cached_insert_info.pr_pos.x,
				   private->cached_insert_info.pr_pos.y)) {
		private->state |= EV_VS_INSERT_WAS_IN_VIEW_RECT;
	    } else {
		private->state &= ~EV_VS_INSERT_WAS_IN_VIEW_RECT;
	    }
	} else {
	    if (!(private->state & EV_VS_BUFFERED_OUTPUT))
	        private->state &= ~(EV_VS_INSERT_WAS_IN_VIEW |
				EV_VS_INSERT_WAS_IN_VIEW_RECT);
	}
    }
}

/* Should be static */
Pkg_private void
ev_blink_caret(focus_view, views, on)
    Xv_Window	    focus_view; /* view window with the kbd focus */
    register Ev_chain views;
    int             on;
{
    struct pr_pos   hotpoint;
    register        Ev_chain_pd_handle
                    chain_private = EV_CHAIN_PRIVATE(views);
    register Ev_pd_handle private;
    register Ev_handle view;
    int             which;
    

    /* static int pcount, rcount;  */

    if (chain_private->caret_is_ghost) {
	hotpoint = chain_private->ghost_hotpoint;
	which = GHOST_CARET;
    } else {
	hotpoint = chain_private->caret_hotpoint;
	which = NORMAL_CARET;
    }
    if (on) {
	if (chain_private->insert_pos != ES_CANNOT_SET) {
	    ev_check_insert_visibility(views);
	    FORALLVIEWS(views, view) {
		private = EV_PRIVATE(view);

		if (private->state & EV_VS_INSERT_WAS_IN_VIEW_RECT) {
		    private->caret_pr_pos.x =
			private->cached_insert_info.pr_pos.x - hotpoint.x;
		    private->caret_pr_pos.y =
			private->cached_insert_info.pr_pos.y - hotpoint.y;
		    if (view->pw == focus_view) {
			ev_put_caret(view->pw, which,
				     private->caret_pr_pos.x,
				     private->caret_pr_pos.y);
		    }
		    /*
		     * printf("%3d: place  at %3d,%3d\n", ++pcount,
		     * private->caret_pr_pos.x, private->caret_pr_pos.y );
		     */
		} /*else printf("No caret on2\n");*/
	    }
	} /*else printf("No caret on1\n");*/
    } else {
	FORALLVIEWS(views, view) {
	    private = EV_PRIVATE(view);
	    if (private->caret_pr_pos.x != EV_NULL_DIM) {
		if (view->pw == focus_view) {
		    ev_put_caret(view->pw, which,
				 private->caret_pr_pos.x,
				 private->caret_pr_pos.y);
		}
		private->caret_pr_pos.x = EV_NULL_DIM;
		private->caret_pr_pos.y = EV_NULL_DIM;
	    } /*else printf("No caret off2\n");*/
	}
    }
}

#define EI_PR_OUT_OF_RANGE	EI_PR_CLIENT_REASON(1)

#ifdef DEBUG
unsigned short           gray25[4] = {
    0x8888, 0x2222, 0x4444, 0x1111	/* 25 % gray pattern   */
};

mpr_static(gray25_pr, 16, 4, 1, gray25);
#endif

Pkg_private	void
ev_clear_rect(view, rect)
    Ev_handle       view;
    register struct rect *rect;
{
    /*
     * jcb     pw_writebackground(view->pw, rect->r_left, rect->r_top,
     * rect->r_width, rect->r_height, PIX_SRC);
     */
    tty_background(view->pw,
	 rect->r_left, rect->r_top, rect->r_width, rect->r_height, PIX_CLR);
}

#ifdef DEBUG
/* NOT USED */
Pkg_private	void
ev_debug_clear_rect(view, rect)
    Ev_handle       view;
    register struct rect *rect;
{
    xv_replrop(view->pw,
	       rect->r_left, rect->r_top, rect->r_width, rect->r_height,
	       PIX_SRC, &gray25_pr, 0, 0);
}

#endif

Pkg_private	void
ev_clear_to_bottom(view, rect, new_top, clear_current_first)
    register Ev_handle view;
    register Rect  *rect;
    int             new_top;
    unsigned        clear_current_first;
{
    register Ev_pd_handle private = EV_PRIVATE(view);

    if (clear_current_first)
	ev_clear_rect(view, rect);
    rect->r_top = new_top;
    rect->r_left = view->rect.r_left;
    rect->r_width = view->rect.r_width;

    if (private->left_margin > 0) {
	rect->r_left -= private->left_margin;
	rect->r_width += private->left_margin;
    }
    if (private->right_margin > 0) {
	rect->r_width += private->right_margin;
    }
    ev_extend_to_view_bottom(view, rect);
    ev_clear_rect(view, rect);
}

/* just for ev_ft_for_rect and ev_display_internal ... */
static struct ev_impl_line_data line_data = {-1, -1, -1, -1};

Pkg_private	Ev_line_table
ev_ft_for_rect(eih, rect)
    Ei_handle       eih;
    struct rect    *rect;
{
    /*
     * The allocated line_table has an entry for every complete line in the
     * view, and an extra entry (hence the +1 below) for the first entity not
     * visible in the view (usually because it is on a partial line, but
     * possibly because it is below the view).
     */
    Pkg_private Ev_line_table ft_create();
    Ev_line_table   result;

    result =
	FT_CLIENT_CREATE(ei_lines_in_rect(eih, rect) + 1,
			 struct ev_impl_line_seq);
    if (result.last_plus_one > 0)
	ft_set(result, 0, result.last_plus_one, ES_INFINITY,
	       &line_data);
    result.seq[0] = 0;
    return (result);
}

#define	SELECTION_FOR_TYPE(chain_private, type)				\
	((EV_SEL_BASE_TYPE(type) == EV_SEL_PRIMARY)			\
	 ? (chain_private->selection)					\
	 : (chain_private->secondary) )
#define	NO_SELECTION(ev_finger_id_pair)					\
	EV_MARK_IS_NULL(&(ev_finger_id_pair)[0])

static	Ev_range
ev_get_selection_range(private, type, mode)
    Ev_chain_pd_handle private;
    unsigned        type;
    int            *mode;
{
    extern Op_bdry_handle ev_find_op_bdry();
    register Op_bdry_handle obh;
    Ev_mark         to_use;
    Ev_range        result;	/* Struct cannot be register */

    ASSUME(0 < type && type < 0x34);
    to_use = SELECTION_FOR_TYPE(private, type);
    obh = ev_find_op_bdry(private->op_bdry, to_use[0]);
    if (obh == 0)
	goto BadReturn;
    result.first = obh->pos;
    if (mode)
	*mode = ((type == EV_SEL_SECONDARY) ?
		 (obh->flags & EV_SEL_PD_SECONDARY) :
		 (obh->flags & EV_SEL_PD_PRIMARY));
    obh = ev_find_op_bdry(private->op_bdry, to_use[1]);
    if (obh == 0)
	goto BadReturn;
    result.last_plus_one = obh->pos;
    ASSERT(result.first <= result.last_plus_one);
    return (result);

BadReturn:
    result.first = result.last_plus_one = ES_INFINITY;
    if (mode)
	*mode = 0;
    return (result);
}

Pkg_private	void
ev_clear_selection(chain, type)
    Ev_chain        chain;
    unsigned        type;
{
    Pkg_private void     ev_display_range();
    register Ev_chain_pd_handle private = EV_CHAIN_PRIVATE(chain);
    Ev_mark         to_use;

    ASSUME(0 < type && type < 0x34);
    to_use = SELECTION_FOR_TYPE(private, type);
    if (!NO_SELECTION(to_use)) {
	Ev_range        selection;
	selection = ev_get_selection_range(private, type, (int *) 0);
	ev_remove_op_bdry(
			  &private->op_bdry, selection.first,
			  type, EV_BDRY_TYPE_ONLY);
	ev_remove_op_bdry(
			  &private->op_bdry, selection.last_plus_one,
			  type | EV_BDRY_END, EV_BDRY_TYPE_ONLY);
	ev_display_range(
			 chain, selection.first, selection.last_plus_one);
	EV_INIT_MARK(to_use[0]);
	/* Don't disturb [1] as it remembers the ids */
    }
}

Pkg_private void
ev_view_range(view, first, last_plus_one)
    Ev_handle       view;
    Es_index       *first, *last_plus_one;
{
    register Ev_impl_line_seq seq = (Ev_impl_line_seq)
    view->line_table.seq;
    *first = seq[0].pos;
    if ((*last_plus_one = seq[view->line_table.last_plus_one - 1].pos) ==
	ES_INFINITY) {
	*last_plus_one = es_get_length(view->view_chain->esh);
    }
}

Pkg_private	int
ev_xy_in_view(view, pos, lt_index, rect)
    register Ev_handle view;
    register Es_index pos;
    register int   *lt_index;
    register Rect  *rect;
{
    extern struct rect ev_rect_for_line();
    register Ev_impl_line_seq seq = (Ev_impl_line_seq)
    view->line_table.seq;
    unsigned        at_end_of_stream = 0;
    struct ei_process_result display_result;

    if (pos < seq[0].pos)
	return (EV_XY_ABOVE);
    if (pos > seq[view->line_table.last_plus_one - 1].pos)
	return (EV_XY_BELOW);
    *lt_index = ft_bounding_index(&view->line_table, pos);
    if (pos == seq[*lt_index].pos) {
	if (*lt_index + 1 < view->line_table.last_plus_one &&
	    seq[*lt_index + 1].pos == ES_INFINITY) {
	    at_end_of_stream = 1;
	} else if (*lt_index + 1 == view->line_table.last_plus_one) {
	    if (pos == es_get_length(view->view_chain->esh)) {
		at_end_of_stream = 1;
	    } else {
		return (EV_XY_BELOW);
	    }
	}
	if (at_end_of_stream && *lt_index > 0)
	    (*lt_index)--;
    }
    *rect = ev_rect_for_line(view, *lt_index);
    if (pos != seq[*lt_index].pos || at_end_of_stream) {
	es_set_position(view->view_chain->esh, seq[*lt_index].pos);
	display_result = ev_display_internal(
			view, rect, *lt_index, pos, EI_OP_MEASURE, EV_QUIT);
	switch (display_result.break_reason) {
	  case EI_PR_HIT_RIGHT:
	    if (*lt_index + 1 == view->line_table.last_plus_one)
		return (EV_XY_BELOW);
	    else
		return (EV_XY_RIGHT);
	  case EI_PR_NEWLINE:
	    if (at_end_of_stream) {
		*lt_index += 1;
		rect->r_top += ei_line_height(view->view_chain->eih);
		if (rect_bottom(&view->rect) < rect_bottom(rect)) {
		    return (EV_XY_BELOW);
		}
		break;
	    }
	    /* else fall through. */
	  default:{
		rect->r_width += rect->r_left - display_result.pos.x;
		rect->r_left = display_result.pos.x;
	    }
	}
    }
    return (EV_XY_VISIBLE);
}

Pkg_private	void
ev_display_range(chain, first, last_plus_one)
    Ev_chain        chain;
    Es_index        first, last_plus_one;
{
    register Ev_handle view;
    struct rect     rect;
    int             lt_index;
    extern struct rect ev_rect_for_line();

    /*
     * Don't display a null range because 1.  We'll probably screw it up, and
     * 2.  What's the point, anyway?
     */
    if (first == last_plus_one) {
	return;
    }
    FORALLVIEWS(chain, view) {
	Ev_impl_line_seq line_table_seq =
	(Ev_impl_line_seq) view->line_table.seq;
	if (last_plus_one <= line_table_seq[0].pos)
	    continue;		/* Above view, nothing to do */
	switch (ev_xy_in_view(view, first, &lt_index, &rect)) {
	  case EV_XY_VISIBLE:{
		es_set_position(chain->esh, first);
		break;
	    }
	  case EV_XY_ABOVE:{
		rect = view->rect;
		lt_index = 0;
		es_set_position(chain->esh, line_table_seq[lt_index].pos);
		break;
	    }
	  case EV_XY_BELOW:
	    continue;
	  case EV_XY_RIGHT:{
		lt_index++;
		if (last_plus_one <= line_table_seq[lt_index].pos)
		    continue;
		rect = ev_rect_for_line(view, lt_index);
		es_set_position(chain->esh, line_table_seq[lt_index].pos);
		break;
	    }
	}
	ev_extend_to_view_bottom(view, &rect);
	(void) ev_display_internal(
	     view, &rect, lt_index, last_plus_one, EI_OP_CLEAR_INTERIOR, 0);
    }
}

/*
 * Cases to consider for arbitrary old(o) and new(n) ranges: ooooo  nnnnn
 * ooooonnnnn ooobbnnn bbbbb (or obbbbbo or nbbbbbn) nnnbbooo nnnnnooooo
 * nnnnn  ooooo
 */
Pkg_private	void
ev_set_selection(chain, first, last_plus_one, type)
    Ev_chain        chain;
    register Es_index first, last_plus_one;
    unsigned        type;
{
    register        Ev_chain_pd_handle
                    private = EV_CHAIN_PRIVATE(chain);
    register Es_index repaint_first = first, repaint_last_plus_one = last_plus_one;
    Ev_mark         to_use;

    ASSUME(0 < type && type < 0x34);
    to_use = SELECTION_FOR_TYPE(private, type);
    if (!NO_SELECTION(to_use)) {
	Ev_range        selection;
	int             was_pending_delete;
	selection = ev_get_selection_range(private, type,
					   &was_pending_delete);
	ev_remove_op_bdry(
			  &private->op_bdry, selection.first,
			  type, EV_BDRY_TYPE_ONLY);
	ev_remove_op_bdry(
			  &private->op_bdry, selection.last_plus_one,
			  type | EV_BDRY_END, EV_BDRY_TYPE_ONLY);
	if (selection.last_plus_one > first &&
	    last_plus_one > selection.first) {
	    /*
	     * Old and new selection overlap: we will repaint the affected
	     * enclosing range once.
	     */
	    if (first == selection.first &&
		(was_pending_delete == (type & EV_SEL_PENDING_DELETE))) {
		/*
		 * Old and new selection have same head: only repaint the
		 * altered tail.
		 */
		if (last_plus_one < selection.last_plus_one) {
		    repaint_first = last_plus_one;
		    repaint_last_plus_one = selection.last_plus_one;
		} else {
		    repaint_first = selection.last_plus_one;
		}
	    } else if (last_plus_one == selection.last_plus_one &&
		   (was_pending_delete == (type & EV_SEL_PENDING_DELETE))) {
		/*
		 * Old and new selection have same tail: only repaint the
		 * altered head.
		 */
		if (first < selection.first) {
		    repaint_last_plus_one = selection.first;
		} else {
		    repaint_first = selection.first;
		    repaint_last_plus_one = first;
		}
	    } else {
		/*
		 * Repaint the entire enclosing range.
		 */
		if (first > selection.first)
		    repaint_first = selection.first;
		if (last_plus_one < selection.last_plus_one)
		    repaint_last_plus_one = selection.last_plus_one;
	    }
	} else {
	    ev_display_range(chain,
			     selection.first, selection.last_plus_one);
	}
    }
    ev_add_op_bdry(&private->op_bdry, first, type, &to_use[0]);
    ev_add_op_bdry(&private->op_bdry, last_plus_one,
		   type | EV_BDRY_END, &to_use[1]);
    ev_display_range(chain, repaint_first, repaint_last_plus_one);
}

Pkg_private	int
ev_get_selection(chain, first, last_plus_one, type)
    Ev_chain        chain;
    Es_index       *first, *last_plus_one;
    unsigned        type;
{
    Ev_chain_pd_handle private = EV_CHAIN_PRIVATE(chain);
    Ev_range        selection;
    int             result;

    selection = ev_get_selection_range(private, type, &result);
    *first = (selection.first == -1) ?
              EV_VIEW_FIRST(chain->first_view) : selection.first;
    *last_plus_one = selection.last_plus_one;
    return (result);
}

Pkg_private	void
ev_display_view(view)
    Ev_handle       view;
{
    ev_display_in_rect(view, (Rect *) 0);
}

/* NOT USED*/
Pkg_private	void
ev_display_views(chain)
    Ev_chain        chain;
{
    Ev_handle       view;

    FORALLVIEWS(chain, view) {
	ev_display_view(view);
    }
}

Pkg_private void
ev_clear_from_margins(view, from_rect, to_rect)
    register Ev_handle view;
    register Rect  *from_rect, *to_rect;
/*
 * If to_rect is NULL, just clear margins around from_rect.
 */
{
    Rect            r_temp;
    Ev_pd_handle    private = EV_PRIVATE(view);

    if (to_rect == (Rect *) 0) {
	r_temp.r_top = from_rect->r_top;
	r_temp.r_height = from_rect->r_height;
    } else if (to_rect->r_top < from_rect->r_top) {
	r_temp.r_top = rect_bottom(to_rect);
	r_temp.r_height =
	    view->rect.r_height - r_temp.r_top;
    } else {
	r_temp.r_top = from_rect->r_top;
	r_temp.r_height = to_rect->r_top - r_temp.r_top;
    }
    if (private->left_margin > 0) {
	r_temp.r_width = private->left_margin;
	r_temp.r_left = view->rect.r_left - r_temp.r_width;
	ev_clear_rect(view, &r_temp);
    }
    if (private->right_margin > 0) {
	r_temp.r_width = private->right_margin;
	r_temp.r_left = rect_right(&view->rect) + 1;
	ev_clear_rect(view, &r_temp);
    }
}

static struct ev_impl_line_data valid_line_data = {-1, -1, -1, -1};
static struct ev_impl_line_data invalid_line_data = {-1, 0, -1, -1};

Pkg_private	void
ev_set_start(view, start)
    Ev_handle       view;
    Es_index        start;
{
    int             lt_index;
    Rect            from_rect;
    Ev_impl_line_seq seq = (Ev_impl_line_seq)
    view->line_table.seq;

    switch (ev_xy_in_view(view, start, &lt_index, &from_rect)) {
      case EV_XY_VISIBLE:
	if ((seq[lt_index].damaged == -1) || (lt_index != 0)) {
	    if (lt_index == 0) {/* Nothing to do */
		return;
	    }
	    if (lt_index > 1)
		if (view->line_table.last_plus_one > 1)
		    ft_set(view->line_table, 1, lt_index,
		       ev_index_for_line(view, lt_index), &valid_line_data);
	    if (view->line_table.last_plus_one > 0)
		ft_set(view->line_table, 0, 1,
		     ev_index_for_line(view, lt_index), &invalid_line_data);
	    ev_update_view_display(view);
	    break;
	}			/* else fall thru */
      default:
	view->line_table.seq[0] = start;
	ev_display_view(view);
	break;
    }
}

Pkg_private void
ev_add_margins(view, rect)
    Ev_handle       view;
    register Rect  *rect;
{
    Ev_pd_handle    private = EV_PRIVATE(view);

    rect->r_left -= private->left_margin;
    rect->r_width += private->left_margin + private->right_margin;
}


Pkg_private	void
/* ARGSUSED */
ev_display_fixup(view)
    register Ev_handle view;
{
#ifdef SUNVIEW1			/******* BUG ev_display_fixup calls pw_restrict_clipping */
    fixup_bound = view->pw->pw_fixup.rl_bound;
    /* Note: following call frees view->pw->pw_fixup! */
    pw_restrict_clipping(view->pw, &view->pw->pw_fixup);
    ev_clear_rect(view, &fixup_bound);
    start = ev_line_for_y(view, fixup_bound.r_top);
    fixup_rect = ev_rect_for_line(view, start);
    stop = ev_line_for_y(view, rect_bottom(&fixup_bound));
    if (stop + 1 < view->line_table.last_plus_one)
	stop++;
    fixup_rect.r_height += fixup_bound.r_height;
    es_set_position(view->view_chain->esh, line_seq[start].pos);
    result = ev_display_internal(
			       view, &fixup_rect, start, line_seq[stop].pos,
				 0, 0);
    es_set_position(view->view_chain->esh, current_pos);
    pw_exposed(view->pw);
#endif
}


static void
  ev_swap_line_table(Ev_line_table  *table1, Ev_line_table  *table2)
{
    ft_object       temp_table;

    temp_table.last_plus_one = table1->last_plus_one;
    temp_table.sizeof_element = table1->sizeof_element;
    temp_table.last_bounding_index = table1->last_bounding_index;
    temp_table.first_infinity = table1->first_infinity;
    temp_table.seq = table1->seq;

    table1->last_plus_one = table2->last_plus_one;
    table1->sizeof_element = table2->sizeof_element;
    table1->last_bounding_index = table2->last_bounding_index;
    table1->first_infinity = table2->first_infinity;
    table1->seq = table2->seq;

    table2->last_plus_one = temp_table.last_plus_one;
    table2->sizeof_element = temp_table.sizeof_element;
    table2->last_bounding_index = temp_table.last_bounding_index;
    table2->first_infinity = temp_table.first_infinity;
    table2->seq = temp_table.seq;

}


Pkg_private	Es_index
ev_scroll_lines(view, line_count, scroll_by_display_lines)
    register Ev_handle view;
    int             line_count;
    int             scroll_by_display_lines;
{
    Ev_impl_line_seq line_seq = (Ev_impl_line_seq)
    view->line_table.seq;
    if (line_count >= 0 && line_count < view->line_table.last_plus_one) {
	ev_set_start(view, line_seq[line_count].pos);
    } else {
	register Ev_chain chain = view->view_chain;
	struct ei_span_result span_result;
	CHAR            buf[EV_BUFSIZE];
	struct es_buf_object esbuf;
	register int    i;
	Es_index        pos, pos_to_remember;
	Pkg_private void ev_lt_format();

	esbuf.esh = chain->esh;
	esbuf.buf = buf;
	esbuf.sizeof_buf = SIZEOF(buf);
	esbuf.first = esbuf.last_plus_one = 0;
	if (line_count < 0) {
	    i = -line_count;
	    if ((pos = line_seq[0].pos) == ES_INFINITY)
		pos = es_get_length(esbuf.esh);
	    pos_to_remember = pos;
	    while (i > 0 && pos > 0) {
		span_result = ei_span_of_group(
		       chain->eih, &esbuf, EI_SPAN_LINE | EI_SPAN_LEFT_ONLY,
					       pos - 1);
		if (span_result.first == ES_CANNOT_SET) {
		    if (pos > 1) {
			/* Must have hit a gap in stream */
			span_result.first =
			    es_bounds_of_gap(esbuf.esh, pos - 1, 0, 0x1);
			i++;
		    } else
			span_result.first = 0;	/* Must be line */
		}
		pos = span_result.first;
                {
#define MAX_SUB_LINES 128 /* max sublines per a line */
                        struct ei_process_result lpo_res, ev_line_lpo();
                        int count = 0;
                        unsigned long tmp_pos[MAX_SUB_LINES];
                        tmp_pos[0] = pos;
                        for (;;) {
                                lpo_res = ev_line_lpo(view, tmp_pos[count]);
				count++;
                                tmp_pos[count] = lpo_res.last_plus_one;
                                if ( count == MAX_SUB_LINES || 
					lpo_res.last_plus_one > span_result.last_plus_one)
                                        break;
                        }
                        if (i < count) {
                                pos = tmp_pos[count - i];
                                i = 0;
                        }
                        else
                                i -= count;
                }

	    }
	} else {
	    i = line_count - view->line_table.last_plus_one + 1;
	    pos = line_seq[view->line_table.last_plus_one - 1].pos;
	    pos_to_remember = pos;
	    while (i--) {
		span_result = ei_span_of_group(
		      chain->eih, &esbuf, EI_SPAN_LINE | EI_SPAN_RIGHT_ONLY,
					       pos);
		if (span_result.first == ES_CANNOT_SET)
		    break;
		pos = span_result.last_plus_one;
	    }
	}
	/* Try to preserve any available bits. */
	if (span_result.first != ES_CANNOT_SET && line_count < 0 &&
	    -line_count < view->line_table.last_plus_one - 1) {
	    if (scroll_by_display_lines) {
		while (line_seq[-line_count].pos !=
		       pos_to_remember) {
		    if (view->line_table.last_plus_one > 0)
			ft_set(view->line_table, 0, 1, pos, &invalid_line_data);
		    ev_lt_format(view, &view->tmp_line_table, &view->line_table);

		    line_seq = (Ev_impl_line_seq) view->tmp_line_table.seq;
		    pos = line_seq[1].pos;
		    if (line_seq[0].pos == 0) break; 
		}
		(void) ev_swap_line_table(&view->line_table, &view->tmp_line_table);
		if (ev_get(view, EV_NO_REPAINT_TIL_EVENT) == 0)
		    ev_lt_paint(view, &view->line_table, &view->tmp_line_table);
	    } else {
		if (view->line_table.last_plus_one > 0)
		    ft_set(view->line_table, 0, 1, pos, &invalid_line_data);
		ev_update_view_display(view);
	    }
	} else {
	    ev_set_start(view, pos);
	}
    }
    ASSERT(allock());
    return (line_seq[0].pos);
}

Pkg_private	void
ev_display_in_rect(view, rect)
    register Ev_handle view;
    register Rect  *rect;
{
    register Es_handle esh = view->view_chain->esh;
    struct rect     rect_intersect_view;
    Ev_chain_pd_handle private =
    EV_CHAIN_PRIVATE(view->view_chain);
    register Es_index length = es_get_length(esh);
    register Es_index pos;
    Ev_pd_handle    view_private = EV_PRIVATE(view);


    if (view_private->state & EV_VS_SET_CLIPPING) {
	win_set_clip(view->pw, (Rectlist *) 0);
	view_private->state &= ~EV_VS_SET_CLIPPING;
    }
    if (rect) {
	rect_intersection(rect, &view->rect, &rect_intersect_view);
	rect = &rect_intersect_view;
	ev_clear_rect(view, rect);
    } else {
	if ((private->notify_level & EV_NOTIFY_SCROLL) ||
	    (private->glyph_count)) {
	    rect = &rect_intersect_view;
	    *rect = view->rect;
	    ev_add_margins(view, rect);
	} else {
	    rect = &view->rect;
	}
	ev_clear_rect(view, rect);
	rect = &view->rect;
    }
    pos = ev_index_for_line(view, 0);
    /* Make sure something will be displayed */
    if (pos >= length && length > 0) {
	((Ev_impl_line_seq) view->line_table.seq)[0].pos = length + 1;
	/* Above defeats the optimization in ev_line_start */
	pos = ev_line_start(view, length);
    }
    if (view->line_table.last_plus_one > 0)
	ft_set(view->line_table, 0, view->line_table.last_plus_one,
	       pos, &invalid_line_data);
    ev_update_view_display(view);

    /* Notify the client of the painting. */
    if (private->notify_level & EV_NOTIFY_PAINT) {
	ev_notify(view, EV_ACTION_PAINT, rect, 0);
    }
}


Pkg_private	int
ev_fill_esbuf(esbuf, ptr_to_last_plus_one)
    Es_buf_handle   esbuf;
    Es_index       *ptr_to_last_plus_one;
{
    register Es_index next, last_plus_one = *ptr_to_last_plus_one;
    int             read, result;

TryRead:
    next = es_read(esbuf->esh, esbuf->sizeof_buf, esbuf->buf, &read);
    if (result = READ_AT_EOF(last_plus_one, next, read)) {
    } else if (read == 0) {
	last_plus_one = next;
	goto TryRead;
    } else {
	esbuf->first = last_plus_one;
	esbuf->last_plus_one = last_plus_one + read;
	*ptr_to_last_plus_one = next;
    }
    return (result);
}

/*
 * ev_range_info was originally only called by ev_display_internal, which
 * only uses ei_op and last_plus_one, and ignores op_bdry_state and next_i
 * completely.  on first call to ev_op_bdry_info_merge, range->next_i isn't
 * even set.
 */
Pkg_private	void
ev_range_info(op_bdry, pos, range)
    Ev_line_table   op_bdry;
    Es_index        pos;
    register struct range *range;
{
    register unsigned op_bdry_state;	/* Optimization  */
    register int    ei_op;	/* temporaries */

    if (pos == ES_INFINITY) {
	op_bdry_state = ev_op_bdry_info_merge(
	      op_bdry, range->next_i, &range->next_i, range->op_bdry_state);
    } else
	op_bdry_state = ev_op_bdry_info(op_bdry, pos, &range->next_i);
    ei_op = 0;
    if (op_bdry_state & EV_SEL_PRIMARY)
	/* invert destination (primary) selection */
	ei_op |= EI_OP_INVERT;
    else if (op_bdry_state & EV_SEL_PD_SECONDARY)
	/* strike-through Quick Move source (secondary) selection */
	ei_op |= EI_OP_STRIKE_THRU;
    else if (op_bdry_state & EV_SEL_SECONDARY)
	/* underline Quick Copy source (secondary) selection */
	ei_op |= EI_OP_STRIKE_UNDER;
    if (op_bdry_state & EV_SEL_PENDING_DELETE)
	ei_op |= EI_OP_INVERT;
    if (op_bdry_state & EV_BDRY_OVERLAY)
	ei_op |= EI_OP_EV_OVERLAY;
    range->op_bdry_state = op_bdry_state;
    range->ei_op = ei_op;
    if (range->next_i < op_bdry.last_plus_one)
	range->last_plus_one =
	    ft_position_for_index(op_bdry, range->next_i);
    else
	range->last_plus_one = ES_INFINITY;
}

/*
 * ev_process and friends were intended to be used to clean up
 * ev_display_internal(). No one has had time yet to convert
 * ev_display_internal(). "first" should always be at the start of a display
 * line, or measuring may be inaccurate due to tabs or word-wrap.
 */

Pkg_private	Ev_process_handle
ev_process_init(ph, view, first, stop_plus_one, rect, buf, sizeof_buf)
    register Ev_process_handle ph;
    Ev_handle       view;
    Es_index        first;
    Es_index        stop_plus_one;
/* ES_INFINITY ==> max buf size */
    Rect           *rect;
    CHAR           *buf;
    int             sizeof_buf;
{
    if (!ph)
	return (ph);
    ph->view = view;
    ph->rect = *rect;
    ph->whole_buf.buf = ph->esbuf.buf = buf;
    ph->whole_buf.sizeof_buf = ph->esbuf.sizeof_buf = sizeof_buf;
    ph->whole_buf.esh = ph->esbuf.esh = view->view_chain->esh;
    ph->whole_buf.first = ph->esbuf.first = first;
    ph->whole_buf.last_plus_one = stop_plus_one;
    ph->esbuf.last_plus_one = first;
    ph->last_plus_one = first;
    ph->pos_for_line = first;
    ph->esbuf_status = 0;
    ph->result.last_plus_one = first;
    ph->result.considered = first;
    ph->result.break_reason = EI_PR_BUF_EMPTIED;
    ph->result.pos.x = ph->ei_xy.x = rect->r_left;
    ph->result.pos.y = ph->ei_xy.y = rect->r_top;
    ph->flags = 0;
    /* what about result.bounds? */
    es_set_position(ph->whole_buf.esh, first);
    return (ph);
}

Pkg_private	int
ev_process_update_buf(ph)
    register Ev_process_handle ph;
/* Non-zero return says we're done */
{
    int             result = 0;
    Es_index        length = es_get_length(ph->esbuf.esh);

    /* Increment past already processed entities. */
    if (ph->result.break_reason != EI_PR_BUF_EMPTIED) {
	/*
	 * ei_process leaves result.last_plus_one at the newline, but the
	 * next character to be positioned in the line table is one past the
	 * newline.
	 */
	if (ph->result.break_reason == EI_PR_NEWLINE) {
	    ph->result.last_plus_one++;
	}
	ph->pos_for_line = ph->result.last_plus_one;
	ph->esbuf.buf += ph->result.last_plus_one - ph->esbuf.first;
	ph->esbuf.sizeof_buf -=
	    ph->result.last_plus_one - ph->esbuf.first;
	ph->esbuf.first = ph->result.last_plus_one;
    }
    if (ph->flags & EV_PROCESS_PROCESSED_BUF
	&& (ph->result.break_reason == EI_PR_BUF_EMPTIED
	    || ph->esbuf.last_plus_one >= ph->whole_buf.last_plus_one)) {
	if (ph->result.last_plus_one == length)
	    ph->result.last_plus_one = ES_INFINITY;
	return (1);
    } else {
	ph->flags |= EV_PROCESS_PROCESSED_BUF;
    }

    /* Make sure esbuf is full. */
    if (ph->esbuf.first >= ph->esbuf.last_plus_one
	|| ph->result.break_reason == EI_PR_BUF_EMPTIED) {
	ph->esbuf.buf = ph->whole_buf.buf;
	if ((ph->whole_buf.last_plus_one < ES_INFINITY) &&
	    (ph->whole_buf.last_plus_one > ph->last_plus_one) &&
	    (ph->whole_buf.last_plus_one - ph->last_plus_one
	     < EV_BUFSIZE)) {
	    ph->esbuf.sizeof_buf =
		ph->whole_buf.last_plus_one - ph->last_plus_one;
	} else {
	    ph->esbuf.sizeof_buf = EV_BUFSIZE;
	}
	ph->last_plus_one = ph->esbuf.last_plus_one = ph->esbuf.first;
	es_set_position(ph->esbuf.esh, ph->last_plus_one);
	if (result = ev_fill_esbuf(&ph->esbuf, &ph->last_plus_one)) {
	    if (ph->result.last_plus_one == length)
		ph->result.last_plus_one = ES_INFINITY;
	    if (ph->result.break_reason != EI_PR_OUT_OF_RANGE)
		ph->result.break_reason |= EI_PR_END_OF_STREAM;
	} else if (ph->esbuf.last_plus_one >
		   ph->whole_buf.last_plus_one) {
	    ph->esbuf.sizeof_buf =
		ph->whole_buf.last_plus_one - ph->esbuf.first;
	    es_set_position(ph->esbuf.esh,
			    (ph->last_plus_one =
			     ph->esbuf.last_plus_one =
			     ph->whole_buf.last_plus_one));
	}
    }
    return (result);
}

/* Flags for ev_process() */
#define	EV_PROCESS_NEXT_LINE	0x0000001

Pkg_private unsigned long			/* break_reason */
ev_process(ph, flags, op, rop, pw)
    register Ev_process_handle ph;
    long unsigned   flags;
    int             op;
    int             rop;
    Xv_Window       pw;

{
    Ei_handle       eih = ph->view->view_chain->eih;

    if (flags & EV_PROCESS_NEXT_LINE) {
	ph->result.pos.y += ei_line_height(eih);
    }
    ph->result = ei_process(
			    eih, op, &ph->esbuf,
			    ph->ei_xy.x, ph->ei_xy.y,
			    rop, pw, &ph->rect, ph->rect.r_left);
    switch (ph->result.break_reason) {
      case EI_PR_HIT_RIGHT:
	switch ((EV_PRIVATE(ph->view))->right_break) {
	  case EV_CLIP:{
		Es_index        span_first, span_lpo;
		(void) ev_span(ph->view->view_chain,
			   ph->result.last_plus_one, &span_first, &span_lpo,
			       EI_SPAN_RIGHT_ONLY | EI_SPAN_LINE);
		if (span_first != ES_CANNOT_SET) {
		    ph->result.last_plus_one = span_lpo;
		    ph->last_plus_one = ph->result.last_plus_one;
		    ph->esbuf.last_plus_one = ph->result.last_plus_one;
		    es_set_position(ph->esbuf.esh,
				    ph->result.last_plus_one);
		}
		break;
	    }
	  case EV_WRAP_AT_WORD:{
		Es_index        span_first, span_lpo;
		unsigned        span_flags;
		struct ei_process_result ei_measure, ev_ei_process();

		span_flags = ev_span(ph->view->view_chain,
			   ph->result.last_plus_one, &span_first, &span_lpo,
				     EI_SPAN_SP_AND_TAB);
		if (span_first != ES_CANNOT_SET) {
		    /*
		     * if white space, break after, else if word and not
		     * first word, break before, else revert to character
		     * wrap.
		     */
		    if ((span_flags & EI_SPAN_NOT_IN_CLASS) == 0) {
			if (span_flags & EI_SPAN_RIGHT_HIT_NEXT_LEVEL)
			    ++span_lpo;
			ph->result.last_plus_one = span_lpo;
			/* because we clip white space */
			ph->result.considered = span_lpo;
		    } else if (span_first > ph->pos_for_line) {
			ph->result.last_plus_one = span_first;
			ei_measure =
			    ev_ei_process(ph->view, ph->pos_for_line, span_first);
			ph->result.bounds.r_width =
			    ei_measure.bounds.r_width -
			    (ph->result.bounds.r_left -
			     ei_measure.bounds.r_left);
			if (ph->result.bounds.r_width < 0) {
			    ph->result.bounds.r_left -= ph->result.bounds.r_width;
			    ph->result.bounds.r_width = 0;
			}
			if (span_first < ph->esbuf.first) {
			    ph->last_plus_one = span_first - 1;
			    break;	/* goto NextBuffer; */
			}
		    } else
			break;
		    ph->last_plus_one = ph->result.last_plus_one;
		    ph->esbuf.last_plus_one = ph->result.last_plus_one;
		    es_set_position(ph->esbuf.esh,
				    ph->result.last_plus_one);
		}
		break;
	    }
	  default:
	    break;
	}			/* switch right_break */
	break;
      default:
	break;
    }				/* switch break_reason */
    ph->ei_xy.x = ph->view->rect.r_left;
    return (ph->result.break_reason);
}

Pkg_private	struct ei_process_result
ev_ei_process(view, start, stop_plus_one)
    register Ev_handle view;
    Es_index        start;	/* Entity to start measuring at */
    Es_index        stop_plus_one;	/* 1st entity not measured */
/*
 * This routine mirrors ev_display_internal for those cases where the
 * view->line_table should not be disturbed. "start" should always be at the
 * start of a display line, or measuring may be inaccurate due to tabs or
 * word-wrap.
 */
{
    CHAR            buf[EV_BUFSIZE];
    Rect            rect;
    Ev_process_object process_object;
    register Ev_process_handle ph;

    rect.r_left = view->rect.r_left;
    rect.r_top = 0;
    rect.r_width = view->rect.r_width;
    rect.r_height = 32000;	/* Close enough to Infinity */

    ph = ev_process_init(&process_object, view, start, stop_plus_one,
			 &rect, buf, EV_BUFSIZE);
    while (!ev_process_update_buf(ph)) {

	(void) ev_process(ph, EV_PROCESS_NEXT_LINE,
			  EI_OP_MEASURE, PIX_SRC, 0);
    }
    return (ph->result);
}

/*
 * Find the index of the last character on a line started by line_start, plus
 * one.
 */
Pkg_private	struct ei_process_result
ev_line_lpo(view, line_start)
    Ev_handle       view;
    Es_index        line_start;
{
    CHAR            buf[EV_BUFSIZE];
    Rect            rect;
    Ev_process_object process_object;
    register Ev_process_handle ph;
    Ev_process_handle ev_process_init();

    rect.r_left = view->rect.r_left;
    rect.r_top = 0;
    rect.r_width = view->rect.r_width;
    rect.r_height = 32000;	/* Close enough to Infinity */

    ph = ev_process_init(&process_object, view, line_start, ES_INFINITY,
			 &rect, buf, EV_BUFSIZE);
    while (!ev_process_update_buf(ph)) {

	(void) ev_process(ph, 0,
			  EI_OP_MEASURE, PIX_SRC, 0);
	if (ph->result.break_reason != EI_PR_BUF_EMPTIED)
	    break;
    }
    if (ph->result.break_reason == EI_PR_NEWLINE)
	ph->result.last_plus_one++;

    return (ph->result);
}

Pkg_private	Es_index
ev_display_line_start(view, pos)
    register Ev_handle view;
    Es_index        pos;
/*
 * Return the index of the entity at the start of the display line containing
 * pos.
 */
{
    CHAR            buf[EV_BUFSIZE];
    Rect            rect;
    Ev_process_object process_object;
    register Ev_process_handle ph;
    register Es_index line_start;
    int             line_index;

    switch (ev_xy_in_view(view, pos, &line_index, &rect)) {
      case EV_XY_RIGHT:
      case EV_XY_VISIBLE:
	return (ev_index_for_line(view, line_index));
      default:

	rect.r_left = view->rect.r_left;
	rect.r_top = 0;
	rect.r_width = view->rect.r_width;
	rect.r_height = 32000;	/* Close enough to Infinity */

	line_start = ev_line_start(view, pos);
	if (line_start == pos)
	    return (line_start);

	ph = ev_process_init(&process_object, view, line_start, pos,
			     &rect, buf, EV_BUFSIZE);
	while (!ev_process_update_buf(ph)) {

	    line_start = ph->result.last_plus_one;
	    (void) ev_process(ph, EV_PROCESS_NEXT_LINE,
			      EI_OP_MEASURE, PIX_SRC, 0);
	}
	if (pos == es_get_length(view->view_chain->esh))
	    return (line_start);

	/*
	 * BUG ALERT: if pos-line_start == EV_BUFSIZE, ei_process will return
	 * BUF_EMPTIED instead of HIT_RIGHT.
	 */
	ASSERT(pos - line_start < EV_BUFSIZE);
	ph = ev_process_init(&process_object, view, line_start, pos + 1,
			     &rect, buf, EV_BUFSIZE);
	while (!ev_process_update_buf(ph)) {
	    (void) ev_process(ph, EV_PROCESS_NEXT_LINE,
			      EI_OP_MEASURE, PIX_SRC, 0);
	    if (ph->result.break_reason == EI_PR_HIT_RIGHT) {
		line_start = ph->result.last_plus_one;
		break;
	    }
	}
	return (line_start);
    }
}


Pkg_private	void
ev_do_glyph(view, glyph_pos_ptr, glyph_ptr, result)
    register Ev_handle view;
    Es_index       *glyph_pos_ptr;
    Ev_overlay_handle *glyph_ptr;
    struct ei_process_result *result;
{
    Ev_overlay_handle glyph = *glyph_ptr;
    Ev_chain        chain = view->view_chain;
    Ei_handle       eih = chain->eih;
    Ev_pd_handle    private = EV_PRIVATE(view);
    int             height = ei_line_height(eih);
    int             width = -glyph->offset_x;
    int             left;
    Rect            margin_rect;

    if (glyph->flags & EV_OVERLAY_RIGHT_MARGIN) {
	left = rect_right(&(view->rect)) + 1;
	width = glyph->pr->pr_width;
	if (width > private->right_margin)
	    width = private->right_margin;
	margin_rect.r_left = left;
	margin_rect.r_top = result->bounds.r_top;
	margin_rect.r_width = private->right_margin;
	margin_rect.r_height = height;
	ev_clear_rect(view, &margin_rect);
    } else {
	left = rect_right(&(result->bounds)) + 1 + glyph->offset_x;
	if (height > glyph->pr->pr_height)
	    height = glyph->pr->pr_height;
	if (width > glyph->pr->pr_width)
	    width = glyph->pr->pr_width;
	if (left < view->rect.r_left) {
	    margin_rect.r_left =
		view->rect.r_left - private->left_margin;
	    margin_rect.r_top = result->bounds.r_top;
	    margin_rect.r_width = private->left_margin;
	    margin_rect.r_height = height;
	    ev_clear_rect(view, &margin_rect);
	}
    }
    xv_rop(view->pw,
	   left, result->bounds.r_top, width, height,
	   glyph->pix_op, glyph->pr, 0, 0);
    *glyph_ptr = (Ev_overlay_handle) 0;
    *glyph_pos_ptr = ES_INFINITY;
}

/*
 * As of 2/23/87, ev_display_internal may assume that the line table is
 * correct.  If the line table is not correct, the caller should be going
 * through ev_update_view_display();
 */
Pkg_private	struct ei_process_result
ev_display_internal(view, rect, line, stop_plus_one, ei_op, break_action)
    register        Ev_handle
                    view;
    register Rect  *rect;	/* r_left, r_top are where to start r_width =
				 * view->rect.r_width-r_left r_height is how
				 * far to paint. */
    register int    line;	/* starting index into line_table. */
    register        Es_index
                    stop_plus_one;	/* 1st entity not displayed */
    int             ei_op,	/* EI_OP_* in entity_interpreter.h */
                    break_action;	/* OR of any of EV_QUIT,
					 * EV_Q_DORBREAK */
{
    /*
     * before calling this function, be sure to
     * es_set_position(view->view_chain->esh, the 1st char to be displayed);
     */

    Ev_chain        chain = view->view_chain;
    Ei_handle       eih = chain->eih;
    CHAR            buf[EV_BUFSIZE];
    register Ev_impl_line_seq line_seq =
    (Ev_impl_line_seq)
    view->line_table.seq;
    Es_index        last_plus_one;
    int             save_left = rect->r_left;
    int             save_width = rect->r_width;
    struct range    range;
    Ev_pd_handle    private = EV_PRIVATE(view);
    Ev_chain_pd_handle chain_private =
    EV_CHAIN_PRIVATE(chain);
    Es_buf_object   esbuf;
    Ev_overlay_handle glyph;
    Es_index        glyph_pos;
    struct ei_process_result glyph_save_result, result;
    int             rop_to_use;
    Es_index        esbuf_last_plus_one;
    Es_index        esbuf_sizeof_buf;
    extern struct rect ev_rect_for_line();

#define INCREMENT_LINE	line++; ASSERT(line < view->line_table.last_plus_one)
#define ADVANCE_TO_NEW_LINE						\
			result.pos.x = view->rect.r_left;		\
			result.pos.y += ei_line_height(eih);		\
			INCREMENT_LINE;					\
			if (line+1 == view->line_table.last_plus_one)	\
				goto Done
#define ESBUF_SET_POSITION(pos)						\
	es_set_position(esbuf.esh, 					\
			(last_plus_one = esbuf.last_plus_one = (pos)))

    result.break_reason = EI_PR_OUT_OF_RANGE;
    glyph_save_result.break_reason = EI_PR_OUT_OF_RANGE;
    glyph_pos = ES_INFINITY;
Glyph_restart:
    result.pos.x = rect->r_left;
    result.pos.y = rect->r_top;
    /*
     * WARNING: Anytime that result.last_plus_one is modified it may be
     * necessary to call ev_range_info().
     */
    result.last_plus_one = last_plus_one = es_get_position(chain->esh);
    if AN_ERROR
	(line + 1 >= view->line_table.last_plus_one) {
	result.break_reason = EI_PR_HIT_BOTTOM;
	goto Done;
	}
    if ((ei_op & EI_OP_MEASURE) == 0) {
	ev_range_info(chain_private->op_bdry, last_plus_one, &range);
	range.ei_op |= ei_op;
	if (range.ei_op & EI_OP_EV_OVERLAY) {
	    if (last_plus_one != line_seq[line].pos) {
		/*
		 * Start is part way into the glyph region. Pretend the call
		 * really started at line's start.
		 */
		es_set_position(chain->esh, line_seq[line].pos);
		rect->r_width += (rect->r_left - view->rect.r_left);
		rect->r_left = view->rect.r_left;
		goto Glyph_restart;
	    }
	}
	glyph = (Ev_overlay_handle) 0;
    } else {
	range.last_plus_one = ES_INFINITY;
	range.ei_op = ei_op;
    }
    esbuf.esh = chain->esh;
    FOREVER {
	esbuf.buf = buf;
	if ((stop_plus_one < ES_INFINITY) &&
	    (stop_plus_one > last_plus_one) &&
	    (stop_plus_one - last_plus_one < EV_BUFSIZE)) {
	    esbuf.sizeof_buf = stop_plus_one - last_plus_one;
	} else
	    esbuf.sizeof_buf = EV_BUFSIZE;
	if (ev_fill_esbuf(&esbuf, &last_plus_one)) {
	    if (result.break_reason == EI_PR_OUT_OF_RANGE) {
		if (line + 1 < view->line_table.last_plus_one &&
		    line_seq[line + 1].pos < ES_INFINITY) {
		    break;
		} else
		    goto Done;
	    }
	    result.break_reason |= EI_PR_END_OF_STREAM;
	    break;
	}
	if (esbuf.last_plus_one > range.last_plus_one) {
	    ESBUF_SET_POSITION(range.last_plus_one);
	}
	if (esbuf.last_plus_one > stop_plus_one) {
	    ESBUF_SET_POSITION(stop_plus_one);
	}
	/*
	 * Invariant at this point: esbuf.last_plus_one <= MIN[stop_plus_one,
	 * range.last_plus_one]
	 */
	while (esbuf.first < esbuf.last_plus_one) {
	    if (range.ei_op & EI_OP_EV_OVERLAY) {
		extern Op_bdry_handle ev_find_glyph();
		Op_bdry_handle  glyph_op_bdry;
		range.ei_op &= ~EI_OP_EV_OVERLAY;
		glyph_op_bdry = ev_find_glyph(chain, line_seq[line].pos);
		if (glyph_op_bdry) {
		    glyph = (Ev_overlay_handle)
			glyph_op_bdry->more_info;
		    glyph_pos = glyph_op_bdry->pos;
		    if (esbuf.last_plus_one > glyph_pos) {
			ESBUF_SET_POSITION(glyph_pos);
		    }
		}
	    }
	    rop_to_use = PIX_SRC;
	    esbuf_last_plus_one = esbuf.last_plus_one;
	    esbuf_sizeof_buf = esbuf.sizeof_buf;
	    if (esbuf_last_plus_one > line_seq[line + 1].pos) {
		esbuf.last_plus_one = line_seq[line + 1].pos;
		esbuf.sizeof_buf = esbuf.last_plus_one - esbuf.first;
	    }
	    result = ei_process(
		       eih, range.ei_op, &esbuf, result.pos.x, result.pos.y,
			     rop_to_use, view->pw, rect, view->rect.r_left);
	    /*
	     * BUG ALERT: if we constrained esbuf.last_plus_one with the
	     * start of the next line, ei_process returned EI_PR_BUF_EMPTIED
	     * when it should have been EI_PR_HIT_RIGHT.
	     */
	    if (esbuf.last_plus_one < esbuf_last_plus_one
		&& result.break_reason & EI_PR_BUF_EMPTIED) {
		result.break_reason = EI_PR_HIT_RIGHT;
	    }
	    esbuf.last_plus_one = esbuf_last_plus_one;
	    esbuf.sizeof_buf = esbuf_sizeof_buf;
	    switch (result.break_reason) {
	      case EI_PR_BUF_EMPTIED:
		if (esbuf.last_plus_one == glyph_pos) {
		    ev_do_glyph(view, &glyph_pos, &glyph, &result);
		}
		if (esbuf.last_plus_one == range.last_plus_one) {
		    ev_range_info(chain_private->op_bdry,
				  ES_INFINITY, &range);
		    range.ei_op |= ei_op;
		}
		goto NextBuffer;
	      case EI_PR_NEWLINE:
		if (esbuf.last_plus_one == glyph_pos) {
		    ev_do_glyph(view, &glyph_pos, &glyph, &result);
		}
		if (break_action & EV_QUIT) {
		    goto Done;
		}
		result.last_plus_one++;
		ADVANCE_TO_NEW_LINE;
		rect->r_width += (rect->r_left - view->rect.r_left);
		rect->r_left = view->rect.r_left;
		if (result.last_plus_one == range.last_plus_one) {
		    ev_range_info(chain_private->op_bdry,
				  ES_INFINITY, &range);
		    range.ei_op |= ei_op;
		}
		break;
	      case EI_PR_HIT_RIGHT:
		result.bounds.r_width = MIN(result.bounds.r_width,
					    view->rect.r_width -
				       (int) ev_get(view, EV_RIGHT_MARGIN));
		switch (private->right_break) {
		  case EV_CLIP:{
			struct ei_span_result span_result;
			if (break_action & EV_QUIT
			    && !(break_action & EV_Q_DORBREAK))
			    goto Done;
			span_result = ei_span_of_group(
			     eih, &esbuf, EI_SPAN_RIGHT_ONLY | EI_SPAN_LINE,
						       result.last_plus_one);
			if (span_result.first != ES_CANNOT_SET) {
			    result.last_plus_one = span_result.last_plus_one;
			    ESBUF_SET_POSITION(result.last_plus_one);
			    if (result.last_plus_one >= range.last_plus_one) {
				ev_range_info(chain_private->op_bdry,
					      result.last_plus_one, &range);
				range.ei_op |= ei_op;
			    }
			}
			break;
		    }
		  case EV_WRAP_AT_WORD:{
			Rect            tmp_rect;

			tmp_rect = ev_rect_for_line(view, line);
			if (result.last_plus_one > line_seq[line + 1].pos
			    || rect_right(&tmp_rect) <= rect_right(rect)) {
			    result.last_plus_one = line_seq[line + 1].pos;
			}
			if (break_action & EV_QUIT
			    && !(break_action & EV_Q_DORBREAK))
			    goto Done;
			if (result.last_plus_one < esbuf.first) {
			    last_plus_one = result.last_plus_one - 1;
			    goto NextBuffer;
			}
			ESBUF_SET_POSITION(result.last_plus_one);
			if (result.last_plus_one >= range.last_plus_one) {
			    ev_range_info(chain_private->op_bdry,
					  result.last_plus_one, &range);
			    range.ei_op |= ei_op;
			}
			break;
		    }		/* end case EV_WRAP_AT_WORD */
		  default:
		    if (break_action & EV_QUIT
			&& !(break_action & EV_Q_DORBREAK))
			goto Done;
		    break;
		}		/* switch right_break */
		ADVANCE_TO_NEW_LINE;
		if (break_action & (EV_QUIT | EV_Q_DORBREAK)) {
		    goto Done;
		}
		rect->r_width += (rect->r_left - view->rect.r_left);
		rect->r_left = view->rect.r_left;
		break;		/* case EI_PR_HIT_RIGHT */
	      case EI_PR_HIT_BOTTOM:
		goto Done;
	      default:
		break;
	    }
	    esbuf.buf += result.last_plus_one - esbuf.first;
	    esbuf.sizeof_buf -= result.last_plus_one - esbuf.first;
	    esbuf.first = result.last_plus_one;
	}
NextBuffer:
	if (esbuf.last_plus_one >= stop_plus_one) {
	    if (glyph_pos == ES_INFINITY)
		goto Done;
	    if (glyph_pos < stop_plus_one)
		/*
		 * BUG ALERT! (Maybe) Processing quit because of some other
		 * condition. It is probably safe to quit, so lets try that.
		 */
		goto Done;
	    /*
	     * Have to keep painting until all of text under glyph has been
	     * displayed.
	     */
	    glyph_save_result = result;
	    stop_plus_one = glyph_pos;
	}
    }
Done:
    rect->r_left = save_left;
    rect->r_width = save_width;
    ASSUME(line_table_is_consistent(view->line_table));
    return ((glyph_save_result.break_reason == EI_PR_OUT_OF_RANGE)
	    ? result : glyph_save_result);
}

Pkg_private          Ev_expand_status
ev_expand(view, start, stop_plus_one, out_buf, out_buf_len, total_chars)
    register Ev_handle view;
    Es_index        start;	/* Entity to start expanding at */
    Es_index        stop_plus_one;	/* 1st ent not expanded */
    CHAR           *out_buf;
    int             out_buf_len;
    int            *total_chars;
/*
 * Expand the contents of the view from first to stop_plus_one into the set
 * of characters used to paint them, placing the expanded text into out_buf,
 * returning the number of character placed into out_buf in total_chars, and
 * returning status.
 */
{
    Ev_chain        chain = view->view_chain;
    CHAR            buf[EV_BUFSIZE];
    CHAR            dummy_buf[EV_BUFSIZE];
    Es_buf_object   esbuf;
    Rect            rect;
    struct ei_process_result result;
    struct ei_process_result expand_result;
    struct ei_span_result span_result;
    int             dummy_total = 0;
    Ev_expand_status status = EV_EXPAND_OKAY;

    if (!out_buf)
	out_buf = dummy_buf;
    if (!total_chars)
	total_chars = &dummy_total;
    *total_chars = 0;
    if (start >= stop_plus_one)
	return (status);
    rect = view->rect;
    /*
     * Find the first Es_index in file line.
     */
    esbuf.esh = chain->esh;
    esbuf.buf = buf;
    esbuf.sizeof_buf = SIZEOF(buf);
    esbuf.first = esbuf.last_plus_one = 0;
    span_result = ei_span_of_group(
	       chain->eih, &esbuf, EI_SPAN_LINE | EI_SPAN_LEFT_ONLY, start);
    /* [uejio  7 Apr 92] return if error. */
    if ((span_result.first == ES_CANNOT_SET) ||
        (span_result.last_plus_one == ES_CANNOT_SET)) {
        status = EV_EXPAND_OTHER_ERROR;
        return(status);
    }
    /*
     * Find the x position of start.
     */
    result.last_plus_one = span_result.first;
    result.pos.x = rect.r_left = view->rect.r_left;
    result.pos.y = rect.r_top = view->rect.r_top;
    result.break_reason = EI_PR_HIT_RIGHT;
    es_set_position(chain->esh, span_result.first);
    while (result.break_reason == EI_PR_HIT_RIGHT) {
	result = ev_ei_process(view, result.last_plus_one, start);
	if (result.break_reason == EI_PR_NEWLINE
	    || result.break_reason == EI_PR_END_OF_STREAM) {
	    /* ei_span_of_group() failed; return error. */
	    status = EV_EXPAND_OTHER_ERROR;
	    return (status);
	}
    }
    /*
     * Expand from start to stop_plus_one into out_buf.
     */
    expand_result.last_plus_one = 0;
    expand_result.pos.x = rect.r_left = view->rect.r_left;
    expand_result.pos.y = rect.r_top = view->rect.r_top;
    expand_result.break_reason = EI_PR_HIT_RIGHT;
    /* Offset first, last_plus_one by what's in the buffer */
    (void) es_make_buf_include_index(&esbuf, start, 0);
    esbuf.last_plus_one = MIN(stop_plus_one, start + esbuf.sizeof_buf);
    while ((expand_result.break_reason == EI_PR_HIT_RIGHT
	    || expand_result.break_reason == EI_PR_NEWLINE)
	   && out_buf_len > expand_result.last_plus_one
	   && esbuf.first < esbuf.last_plus_one) {
	expand_result = ei_expand(
				  chain->eih, &esbuf, &rect, result.pos.x,
				  out_buf + expand_result.last_plus_one,
				  out_buf_len - expand_result.last_plus_one,
				  view->rect.r_left);
	if (expand_result.break_reason == EI_PR_HIT_RIGHT) {
	    result.pos.x = view->rect.r_left;
	}
	*total_chars += expand_result.last_plus_one;
    }
    switch (expand_result.break_reason) {
      case EI_PR_BUF_FULL:
	status = EV_EXPAND_FULL_BUF;
	break;
      case EI_PR_BUF_EMPTIED:
	break;
      default:
	status = EV_EXPAND_OTHER_ERROR;
    }
    return (status);
}

#ifdef DEBUG
static int      special;
static int      max_valid_val = 100000;

static int
line_table_is_consistent(table)
    Ev_line_table   table;
{
    int             lt_index, prev_val = -1;
    Ev_impl_line_seq line_seq = (Ev_impl_line_seq) table.seq;

    if (special == -1)
	return (TRUE);
    for (lt_index = 0; lt_index < table.last_plus_one; lt_index++) {
	if (line_seq[lt_index].pos <= prev_val) {
	    if ((line_seq[lt_index].pos != prev_val) ||
		(line_seq[lt_index].pos != ES_INFINITY)) {
		LINT_IGNORE(ASSUME(0));
		return (FALSE);
	    }
	}
	prev_val = line_seq[lt_index].pos;
	ASSUME(prev_val < max_valid_val || prev_val == ES_INFINITY);
    }
    return (TRUE);
}

#endif

Pkg_private int
ev_line_for_y(view, y)
    Ev_handle       view;
    int             y;
{
    return ((y - view->rect.r_top) / ei_line_height(view->view_chain->eih));
}

Pkg_private     Es_index
ev_index_for_line(view, line)
    Ev_handle       view;
    int             line;
{
    Ev_impl_line_seq line_seq = (Ev_impl_line_seq)
    view->line_table.seq;
    if (line < 0 || line >= view->line_table.last_plus_one)
	line = 0;
    return (line_seq[line].pos);
}

Pkg_private     Es_index
ev_considered_for_line(view, line)
    Ev_handle       view;
    int             line;
{
    Ev_impl_line_seq line_seq = (Ev_impl_line_seq)
    view->line_table.seq;
    if (line < 0 || line >= view->line_table.last_plus_one)
	line = 0;
    return (line_seq[line].pos + line_seq[line].considered);
}

Pkg_private struct rect
ev_rect_for_line(view, line)
    Ev_handle       view;
    int             line;
{
    struct rect     result;

    result = view->rect;
    result.r_height = ei_line_height(view->view_chain->eih);
    result.r_top += line * result.r_height;
    return (result);
}

Pkg_private void
ev_extend_to_view_bottom(view, rect)
    Ev_handle       view;
    struct rect    *rect;
{
    rect->r_height =
	view->rect.r_height - (rect->r_top - view->rect.r_top);
}

Pkg_private     Ev_handle
ev_resolve_xy_to_view(views, x, y)
    Ev_chain        views;
    register int    x, y;
{
    register Ev_handle result;
    FORALLVIEWS(views, result) {
	if (rect_includespoint(&result->rect, x, y))
	    return (result);
    }
    return (EV_NULL);
}

/*
 * Following rect_* should be in rect package!
 */

Pkg_private     Ev_handle
ev_nearest_view(views, x, y, x_used, y_used)
    Ev_chain        views;
    register int    x, y;
    register int   *x_used, *y_used;
{
    register Ev_handle result = ev_resolve_xy_to_view(views, x, y);
    register Ev_handle view;
    int             near_x, near_y;
    int             min_dist_sq = 0x7FFFFFFF;
    register int    dist_sq, temp;

    if (result == EV_NULL) {
	FORALLVIEWS(views, view) {
	    near_x = ((x <= view->rect.r_left) ? view->rect.r_left :
		      ((x > (view->rect.r_left + view->rect.r_width)) ?
		       (view->rect.r_left + view->rect.r_width) : x));
	    near_y = ((y <= view->rect.r_top) ? view->rect.r_top : ((y > (view->rect.r_top + view->rect.r_height)) ? (view->rect.r_top + view->rect.r_height) : y));
	    temp = (near_x - x);
	    dist_sq = temp * temp;
	    temp = (near_y - y);
	    dist_sq += temp * temp;
	    if (dist_sq < min_dist_sq) {
		min_dist_sq = dist_sq;
		result = view;
		if (x_used)
		    *x_used = near_x;
		if (y_used)
		    *y_used = near_y;
	    }
	}
    } else {
	if (x_used)
	    *x_used = x;
	if (y_used)
	    *y_used = y;
    }
    return (result);
}

Pkg_private     Es_index
ev_resolve_xy(view, x, y)
    Ev_handle       view;
    int             x, y;
{
    Es_handle       esh = view->view_chain->esh;
    Es_index        result = ES_INFINITY;
    int             lt_index;
    struct rect     rect;
    Ev_impl_line_seq line_seq;
    struct ei_process_result p_result;

    if (view == EV_NULL)
	return (result);
    line_seq = (Ev_impl_line_seq) view->line_table.seq;
    lt_index = ev_line_for_y(view, y);
    rect = ev_rect_for_line(view, lt_index);
    rect.r_width = x - rect.r_left;

    if (line_seq[lt_index].pos != ES_INFINITY) {
	if (lt_index + 1 == view->line_table.last_plus_one) {
	    /* Incorrect to measure last line in line_table. */
	    p_result.break_reason = EI_PR_HIT_BOTTOM;
	    p_result.last_plus_one = line_seq[lt_index].pos;
	} else {
	    es_set_position(esh, line_seq[lt_index].pos);
	    p_result = ev_display_internal(
					 view, &rect, lt_index, ES_INFINITY,
					   EI_OP_MEASURE, EV_QUIT);
	}
	if (p_result.break_reason == EI_PR_OUT_OF_RANGE ||
	    p_result.break_reason & EI_PR_END_OF_STREAM) {
	    goto AtEnd;
	} else {
	    ASSUME(p_result.break_reason < EI_PR_NEXT_FREE);
	    result = p_result.last_plus_one;
	    if ((p_result.break_reason & EI_PR_HIT_RIGHT)
		&& result >= line_seq[lt_index + 1].pos) {
		--result;
	    }
	}
    } else {
AtEnd:
	result = es_get_length(esh);
    }
    return (result);
}


#ifdef OW_I18N
Pkg_private	void
ev_set_pre_edit_region(chain, first, last_plus_one, type)
    Ev_chain        	    chain;
    register Es_index 	    first, last_plus_one;
    unsigned        	    type;
{
    register Ev_chain_pd_handle
                    	    private = EV_CHAIN_PRIVATE(chain);
    Ev_mark_object  	    first_mark_obj, last_mark_obj;
    Ev_mark         	    to_use;
    Ev_mark_object	    temp[2];
    
    EV_INIT_MARK(first_mark_obj);
    EV_INIT_MARK(last_mark_obj);

    temp[0] = first_mark_obj;
    temp[1] = last_mark_obj;

    to_use = temp;
    ev_add_op_bdry(&private->op_bdry, first, type, &to_use[0]);
    ev_add_op_bdry(&private->op_bdry, last_plus_one,
		   type | EV_BDRY_END, &to_use[1]);
}
#endif /* OW_I18N */


/*
 * This flag is used to prevent unnecessary clearing in paint_batch().
 */
int xv_textsw_doing_refresh;

/*
 * Handle an expose (or graphics expose) event.
 */
void
ev_paint_view(e_view, xv_win, xevent)
	Ev_handle       e_view;
	Xv_window       xv_win;
	XEvent         *xevent;
{
	register Ev_impl_line_seq new;
	register int    index;
	Es_index        length;
	Es_index        next_pos;
	Tty_exposed_lines *exposed;
	Textsw_folio	folio = FOLIO_FOR_VIEW(VIEW_PRIVATE(xv_win));
	Ev_pd_handle	ev_pd_handle = EV_PRIVATE(e_view);
	int		repaint_caret;

	/* figure out the lines that have been damaged */
	exposed = tty_calc_exposed_lines(xv_win, xevent, 
		ev_pd_handle->caret_pr_pos.y);

	repaint_caret = 
		((exposed->caret_line_exposed) &&
		(folio->caret_state & TXTSW_CARET_ON) && 
		(folio->focus_view == xv_win) &&
		(ev_pd_handle->caret_pr_pos.x != EV_NULL_DIM));

	if (repaint_caret) {
		/* Have to erase area because caret is xor'd.
		 * Additionally, clear from the left margin because sometimes
		 * the following sequence happens:
		 * the expose occurs (presumable a forked windowed app exits
		 * returning control to this cmdtool), the caret was 
		 * displayed (xor) as the shell prompt was output, 
		 * leaving xor bit droppings.  By the time it gets here,
		 * the caret's position is changed and it can't be repainted
		 * carefully.
		 */
		tty_background(e_view->pw, 
			0, ev_pd_handle->caret_pr_pos.y,
			CARET_WIDTH + ev_pd_handle->caret_pr_pos.x,
			CARET_HEIGHT,
			PIX_CLR);

#ifdef USE_XCLEARAREA
		Xv_Drawable_info *info;
		DRAWABLE_INFO_MACRO(e_view->pw, info);
		XClearArea(xv_display(info), xv_xid(info),
			0, ev_pd_handle->caret_pr_pos.y,
			CARET_WIDTH + ev_pd_handle->caret_pr_pos.x,
			CARET_HEIGHT,
			FALSE);
#endif
	}

	length = es_get_length(e_view->view_chain->esh);
	new = (Ev_impl_line_seq) e_view->line_table.seq;
	index = 0;

	xv_textsw_doing_refresh = TRUE; /* prevent unneeded clearing */
	/*
	 * Iterate though the lines.  If there is no damage on the
	 * line, don't attempt to paint it.
	 */
	while ((index + 1 < e_view->line_table.last_plus_one) &&
	       (new->pos < length)) {
			
		if (exposed->line_exposed[index]) {
			next_pos = ((new + 1)->pos == ES_INFINITY ?
				    length :
				    (new + 1)->pos);
			ev_display_line(e_view, 0, index, new->pos, next_pos);
		}
		new++;
		index++;
	}
	xv_textsw_doing_refresh = FALSE;

	if(repaint_caret)
		ev_put_caret(e_view->pw, 
			(EV_CHAIN_PRIVATE(folio->views)->caret_is_ghost?
				GHOST_CARET:NORMAL_CARET),
			ev_pd_handle->caret_pr_pos.x,
			ev_pd_handle->caret_pr_pos.y);

	tty_clear_clip_rectangles(e_view->pw);
}


