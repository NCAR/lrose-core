#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)txt_dbx.c 20.26 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * Support routines for dbx's use of text subwindows.
 * These routines are only supported for dbxtool use.
 */

#include <xview/pkg.h>
#include <xview/attrol.h>
#include <xview/font.h>
#include <xview_private/primal.h>
#include <xview_private/txt_impl.h>
#include <pixrect/pixfont.h>

Pkg_private Ev_mark_object ev_add_glyph();
Pkg_private Ev_mark_object ev_add_glyph_on_line();
Pkg_private void     ev_line_info();
Pkg_private void ev_remove_glyph();
Pkg_private Es_index ev_position_for_physical_line();

Xv_public          Textsw
textsw_first(any)
    Textsw          any;
{
    Textsw_view_handle view = VIEW_ABS_TO_REP(any);

    return (view ? VIEW_REP_TO_ABS(FOLIO_FOR_VIEW(view)->first_view)
	    : TEXTSW_NULL);
}

Xv_public          Textsw
textsw_next(previous)
    Textsw          previous;
{
    Textsw_view_handle view = VIEW_ABS_TO_REP(previous);

    return ((view && view->next)
	    ? VIEW_REP_TO_ABS(view->next) : TEXTSW_NULL);
}

Pkg_private int
textsw_does_index_not_show(abstract, index, line_index)
    Textsw          abstract;
    Es_index        index;
    int            *line_index;	/* output only.
				 if index does not show, not set. */
{
    Textsw_view_handle view = VIEW_ABS_TO_REP(abstract);
    Rect            rect;
    int             dummy_line_index;

    if (!line_index)
	line_index = &dummy_line_index;
    switch (ev_xy_in_view(view->e_view, index, line_index, &rect)) {
      case EV_XY_VISIBLE:
	return (0);
      case EV_XY_RIGHT:
	return (0);
      case EV_XY_BELOW:
	return (1);
      case EV_XY_ABOVE:
	return (-1);
      default:			/* should never happen */
	return (-1);
    }
}

Xv_public int
textsw_screen_line_count(abstract)
    Textsw          abstract;
{
    Textsw_view_handle view = VIEW_ABS_TO_REP(abstract);

    return (view ? view->e_view->line_table.last_plus_one - 1 : 0);
}

Pkg_private int
textsw_screen_column_count(abstract)
    Textsw          abstract;
{
    Textsw_view_handle view = VIEW_ABS_TO_REP(abstract);
    PIXFONT        *font = (PIXFONT *) xv_get(abstract, WIN_FONT);
    XFontStruct	*x_font_info = (XFontStruct *)xv_get((Xv_opaque)font, FONT_INFO);

    if (x_font_info->per_char)  {
        return (view->e_view->rect.r_width / x_font_info->per_char['m' - x_font_info->min_char_or_byte2].width);
    }
    else  {
        return (view->e_view->rect.r_width / x_font_info->min_bounds.width);
    }
}

/*
 * Following is obsolete; replace by: textsw_set(abstract, TEXTSW_FIRST, pos,
 * 0);
 */
Pkg_private void
textsw_set_start(abstract, pos)
    Textsw          abstract;
    Textsw_index    pos;
{
    Textsw_view_handle view = VIEW_ABS_TO_REP(abstract);

    ev_set_start(view->e_view, pos);
}

Xv_public void
textsw_file_lines_visible(abstract, top, bottom)
    Textsw          abstract;
    int            *top, *bottom;
{
    Textsw_view_handle view = VIEW_ABS_TO_REP(abstract);

    ev_line_info(view->e_view, top, bottom);
    *top -= 1;
    *bottom -= 1;
}

Pkg_private void
textsw_view_line_info(abstract, top, bottom)
    Textsw          abstract;
    int            *top, *bottom;
{
    Textsw_view_handle view = VIEW_ABS_TO_REP(abstract);

    ev_line_info(view->e_view, top, bottom);
}

Pkg_private int
textsw_contains_line(abstract, line, rect)
    register Textsw abstract;
    register int    line;
    register Rect  *rect;
{
    int             lt_index;
    int             top, bottom;
    Es_index        first;
    Textsw_view_handle view = VIEW_ABS_TO_REP(abstract);

    textsw_view_line_info(abstract, &top, &bottom);
    if (line < top || line > bottom)
	return (-2);
    lt_index = ev_rect_for_ith_physical_line(
			      view->e_view, line - top, &first, rect, TRUE);
    return (lt_index);
}

/* ARGSUSED */
Pkg_private int
textsw_nop_notify(abstract, attrs)
    Textsw          abstract;
    Attr_avlist     attrs;
{
    return (0);
}

Xv_public          Textsw_index
#ifdef OW_I18N
textsw_index_for_file_line_wc(abstract, line)
#else
textsw_index_for_file_line(abstract, line)
#endif
    Textsw          abstract;
    int             line;
{
    Textsw_view_handle view = VIEW_ABS_TO_REP(abstract);
    Textsw_folio    folio = FOLIO_FOR_VIEW(view);
    Es_index        result;

    result = ev_position_for_physical_line(folio->views, line, 0);
    return ((Textsw_index) result);
}

#ifdef OW_I18N
Xv_public          Textsw_index
textsw_index_for_file_line(abstract, line)
    Textsw          abstract;
    int             line;
{
    Textsw_view_handle view = VIEW_ABS_TO_REP(abstract);
    Textsw_folio    folio = FOLIO_FOR_VIEW(view);
    Es_index        result;

    result = ev_position_for_physical_line(folio->views, line, 0);
    return ((Textsw_index) textsw_mbpos_from_wcpos(folio, result));
}
#endif /* OW_I18N */

/* Following is for compatibility with old client code. */
Pkg_private          Textsw_index
textsw_position_for_physical_line(abstract, physical_line)
    Textsw          abstract;
    int             physical_line;	/* Note: 1-origin, not 0! */
{
#ifdef OW_I18N
    return (textsw_index_for_file_line_wc(abstract, physical_line - 1));
#else
    return (textsw_index_for_file_line(abstract, physical_line - 1));
#endif
}

Xv_public void
textsw_scroll_lines(abstract, count)
    Textsw          abstract;
    int             count;
{
    Textsw_view_handle view = VIEW_ABS_TO_REP(abstract);

    (void) ev_scroll_lines(view->e_view, count, FALSE);
}

Pkg_private          Textsw_mark
textsw_add_glyph(abstract, pos, pr, op, offset_x, offset_y, flags)
    Textsw          abstract;
    Textsw_index    pos;
    Pixrect        *pr;
    int             op;
    int             offset_x, offset_y;
{
    Textsw_view_handle view = VIEW_ABS_TO_REP(abstract);
    Textsw_folio    folio = FOLIO_FOR_VIEW(view);
    Es_index        line_start;
    Ev_mark_object  mark;

    if (flags & TEXTSW_GLYPH_DISPLAY)
	textsw_take_down_caret(folio);

    /*
     * BUG ALERT!  True for only current client (filemerge), but wrong in
     * general.
     */
    line_start = pos;

    /* Assume that TEXTSW_ flags == EV_ flags */
    mark = ev_add_glyph(folio->views, line_start, pos, pr, op,
			offset_x, offset_y);
    return ((Textsw_mark) mark);
}

Pkg_private          Textsw_mark
textsw_add_glyph_on_line(abstract, line, pr, op, offset_x, offset_y, flags)
    Textsw          abstract;
    int             line;	/* Note: 1-origin, not 0! */
    struct pixrect *pr;
    int             op;
    int             offset_x, offset_y;
    int             flags;
{
    Textsw_view_handle view = VIEW_ABS_TO_REP(abstract);
    Textsw_folio    folio = FOLIO_FOR_VIEW(view);
    Ev_mark_object  mark;

    if (flags & TEXTSW_GLYPH_DISPLAY)
	textsw_take_down_caret(folio);

    /* Assume that TEXTSW_ flags == EV_ flags */
    mark = ev_add_glyph_on_line(folio->views, line - 1, pr,
				op, offset_x, offset_y, flags);
    return ((Textsw_mark) mark);
}

Pkg_private void
textsw_remove_glyph(abstract, mark, flags)
    Textsw          abstract;
    Textsw_mark     mark;
    int             flags;
{
    Textsw_view_handle view = VIEW_ABS_TO_REP(abstract);
    long unsigned  *dummy_for_compiler = (long unsigned *) &mark;

    textsw_take_down_caret(FOLIO_FOR_VIEW(view));

    ev_remove_glyph(FOLIO_FOR_VIEW(view)->views,
		    *(Ev_mark) dummy_for_compiler, (unsigned) flags);
}

Pkg_private void
textsw_set_glyph_pr(abstract, mark, pr)
    Textsw          abstract;
    Textsw_mark     mark;
    struct pixrect *pr;
{
    Textsw_view_handle view = VIEW_ABS_TO_REP(abstract);
    long unsigned  *dummy_for_compiler = (long unsigned *) &mark;

    textsw_take_down_caret(FOLIO_FOR_VIEW(view));

    ev_set_glyph_pr(FOLIO_FOR_VIEW(view)->views,
		    *(Ev_mark) dummy_for_compiler, pr);
}

Pkg_private          Textsw_index
textsw_start_of_display_line(abstract, pos)
    Textsw          abstract;
    Textsw_index    pos;
{
    register Textsw_view_handle view = VIEW_ABS_TO_REP(abstract);
    Pkg_private Es_index ev_display_line_start();

    return ((Textsw_index) ev_display_line_start(view->e_view, pos));
}
