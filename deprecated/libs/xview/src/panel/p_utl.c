#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)p_utl.c 20.100 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#include <X11/X.h>
#include <xview_private/i18n_impl.h>
#include <xview_private/panel_impl.h>
#include <xview/cms.h>
#include <xview/cursor.h>
#include <xview/defaults.h>
#include <xview/font.h>
#include <xview/openmenu.h>
#include <xview/pixwin.h>
#include <xview/scrollbar.h>
#include <xview/server.h>
#include <xview/svrimage.h>
#include <xview/sun.h>
#include <xview/xv_xrect.h>
#include <xview_private/draw_impl.h>
#include <xview_private/pw_impl.h>

extern struct pr_size 	 xv_pf_textwidth();
Xv_private Xv_xrectlist *screen_get_clip_rects();
extern void 		 screen_adjust_gc_color();
Xv_private GC		 xv_find_proper_gc();
Xv_private void		 xv_set_gc_op();
#ifdef OW_I18N
extern struct pr_size    xv_pf_textwidth_wc();
extern wchar_t          _xv_null_string_wc[];
#endif /* OW_I18N */

static short qmark_cursor_data [] = {
#include <images/qmark.cursor>
};


/*****************************************************************************/
/* panel_enclosing_rect                                                      */
/*****************************************************************************/

Pkg_private     Rect
panel_enclosing_rect(r1, r2)
    register Rect  *r1, *r2;
{
    /* if r2 is undefined then return r1 */
    if (r2->r_left == -1)
	return (*r1);

    return rect_bounding(r1, r2);
}

/*****************************************************************************/
/* panel_update_extent                                                       */
/* called from panel_attr.c                                                  */
/*****************************************************************************/

Pkg_private void
panel_update_extent(panel, rect)
    Panel_info     *panel;
    Rect            rect;
{
    int             v_end, h_end;

    if (!panel->paint_window->view)
	return;	/* not a Scrollable Panel */

    v_end = panel_height(panel);
    if (rect.r_top + rect.r_height > v_end) {
	v_end = rect.r_top + rect.r_height + panel->extra_height;
	(void) xv_set(PANEL_PUBLIC(panel),
		      CANVAS_MIN_PAINT_HEIGHT, v_end, NULL);
    }
    h_end = panel_width(panel);
    if (rect.r_left + rect.r_width > h_end) {
	h_end = rect.r_left + rect.r_width + panel->extra_width;
	(void) xv_set(PANEL_PUBLIC(panel),
		      CANVAS_MIN_PAINT_WIDTH, h_end, NULL);
    }
}


/****************************************************************************/
/* font char/pixel conversion routines                                      */
/****************************************************************************/

Pkg_private int
panel_col_to_x(font, col)
    Xv_Font         font;
    int             col;
{
#ifdef OW_I18N
    int		chrwth = xv_get(font, FONT_COLUMN_WIDTH);
#else
    int		chrwth = xv_get(font, FONT_DEFAULT_CHAR_WIDTH);
#endif /* OW_I18N */
    return (col * chrwth);
}


Pkg_private int
panel_row_to_y(font, line)
    Xv_Font         font;
    int             line;
{
    int		chrht = xv_get(font, FONT_DEFAULT_CHAR_HEIGHT);
    return (line * chrht);
}


Pkg_private int
panel_x_to_col(font, x)
    Xv_Font         font;
    int             x;
{
#ifdef OW_I18N
    int		chrwth = xv_get(font, FONT_COLUMN_WIDTH);
#else
    int		chrwth = xv_get(font, FONT_DEFAULT_CHAR_WIDTH);
#endif /* OW_I18N */
    return (x / chrwth);
}


Pkg_private int
panel_y_to_row(font, y)
    Xv_Font         font;
    int             y;
{
    int		chrht = xv_get(font, FONT_DEFAULT_CHAR_HEIGHT);
    return (y / chrht);
}


/*****************************************************************************
 * panel_make_image                                                          *
 ****************************************************************************/
Pkg_private struct pr_size
panel_make_image(font, dest, type_code, value, bold_desired,
		 inverted_desired)
    Xv_Font         font;
    Panel_image    *dest;
    int             type_code;
    Xv_opaque       value;
    int             bold_desired;
    int             inverted_desired;
{
    int		    default_char_height;
    int		    height;
    int		    i;
    int		    length;
    int		    line_start;	/* char index of the beginning of a new line */
    int		    max_width;
    struct pr_size  size;
    CHAR           *str;
    CHAR	   *value_str;

    if (is_string(dest))
#ifdef OW_I18N
    {
        if (image_string_wc(dest))
            xv_free(image_string_wc(dest));
    }
#else
#ifndef __linux
    {
        if (image_string(dest))
            xv_free(image_string(dest));
    }
#endif
#endif

    size.x = size.y = 0;
    dest->im_type = type_code;
    image_set_inverted(dest, inverted_desired);
    switch (type_code) {
      case PIT_STRING:
	if (value)
	    value_str = (CHAR *) value;
	else
#ifdef OW_I18N
	    value_str = panel_strsave_wc(_xv_null_string_wc);
	/*  No need to copy the string in panel_make_image() because the 
	 *  widechar copy has already been made.
	 */
	if (!(str = (wchar_t *)value_str))
	    return (size);
	image_set_string_wc(dest, str);
#else
	    value_str = "";
	if (!(str = (char *) panel_strsave((u_char *) value_str)))
	    return (size);
#ifdef __linux
/* XView bug: This routine sometimes used a value that was already freed,
 * leading to clobbered menu items. The problem is the 
 * 'xv_free(image_string(dest))' above. In some cases the new 'value' 
 * was the same memory area as 'image_string(dest)'. If free() modifies
 * a freed buffer, the bug becomes visible. Kludgy fix for Linux. The
 * real fix should probably be somewhere else. (lmfken Oct-93) */
        if (image_string(dest))
            xv_free(image_string(dest));
#endif
	image_set_string(dest, str);
#endif /* OW_I18N */
	panel_image_set_font(dest, font);
	image_set_bold(dest, bold_desired);
	default_char_height = (int) xv_get(font, FONT_DEFAULT_CHAR_HEIGHT);
	height = 0;
	length = STRLEN(str);
	line_start = 0;
	max_width = 0;
	for (i=0; i<=length; i++) {
	    if (i == length || str[i] == '\n') {
		if(length)
#ifdef OW_I18N
		   size=xv_pf_textwidth_wc(i-line_start,font,&str[line_start]);
#else
		   size = xv_pf_textwidth(i-line_start, font, &str[line_start]);
#endif /* OW_I18N */
		line_start = i + 1;
		max_width = MAX(max_width, size.x);
		height += default_char_height;
	    }
	}
	size.x = max_width;
	size.y = height;
	break;

      case PIT_SVRIM:
	if (!value || PR_NOT_SERVER_IMAGE(value))
	    xv_error(value,
		     ERROR_STRING, 
			XV_MSG("Invalid Server Image specified"),
		     ERROR_PKG, PANEL,
		     NULL);
	image_set_svrim(dest, (Server_image) value);
	size = ((Pixrect *) value)->pr_size;
	break;
    }
    return size;
}

Pkg_private void
panel_image_set_font(image, font)
    Panel_image    *image;
    Xv_Font         font;
{
    if (image_font(image)) {
        (void) xv_set(image_font(image), XV_DECREMENT_REF_COUNT, NULL);
    }
    image_font(image) = font;
    if (font) {
        (void) xv_set(font, XV_INCREMENT_REF_COUNT, NULL);
    }
}
 
/*****************************************************************************
 * panel_successor -- returns the next unhidden item after ip
 *		      that wants events.
 *****************************************************************************/

Pkg_private Item_info *
panel_successor(ip)
    register Item_info *ip;
{
    if (!ip)
	return NULL;

    for (ip = ip->next; ip && (hidden(ip) || deaf(ip)); ip = ip->next);

    return ip;
}

/*****************************************************************************/
/* panel_append                                                              */
/*****************************************************************************/

Pkg_private void
panel_append(ip)
    register Item_info *ip;
{
    Panel_info     *panel = ip->panel;
    register Item_info *ip_cursor;

    if (!panel->items) {
	panel->items = ip;
	ip->previous = NULL;
    } else {
/*
	for (ip_cursor = panel->items;
	     ip_cursor->next != NULL;
	     ip_cursor = ip_cursor->next);
	ip_cursor->next = ip;
	ip->previous = ip_cursor;
*/
        ip_cursor = panel->last_item;
        ip_cursor->next = ip;
        ip->previous = ip_cursor;
    }
    ip->next = NULL;
    panel->last_item = ip;
}


/*****************************************************************************/
/* panel_unlink                                                              */
/*****************************************************************************/

Pkg_private void
panel_unlink(ip, destroy)
    register Item_info *ip;
    int             destroy;	/* boolean: unlink is part of a destroy
				 * operation */
{
    Panel_info     *panel = ip->panel;
    register Item_info *prev_ip = ip->previous;

    /* if it's default panel item, clear the default value */
    if (panel->default_item == ITEM_PUBLIC(ip))
	panel->default_item = XV_ZERO;

    /* unlinked item is no longer current */
    if (panel->current == ip)
	panel->current = NULL;

    /* we assume that the caret is off by the time we are called */

    if (!destroy && ip->ops.panel_op_remove)
	/* remove from any private list */
	(*ip->ops.panel_op_remove) (ITEM_PUBLIC(ip));

    /* unlink ip */
    if (prev_ip)
	prev_ip->next = ip->next;
    else
	panel->items = ip->next;
    if (ip->next)
	ip->next->previous = prev_ip;
    else
	panel->last_item = prev_ip;

    /* NULL out parent pointer */
    ip->panel = NULL;

    /* update the default position of the next created item */
    (void) panel_find_default_xy(panel, NULL);
}



/****************************************************************************/
/* panel_find_default_xy                                                    */
/* computes panel->item_x, panel->item_y, and panel->max_item_y based on    */
/* the geometry of the current items in the panel.                          */
/* First the lowest "row" is found, then the default position is on that    */
/* row to the right of any items which intersect that row.                  */
/* The max_item_y is set to the height of the lowest item rectangle on the  */
/* lowest row.                                                              */
/****************************************************************************/

Pkg_private void
panel_find_default_xy(panel, item)
    Panel_info     *panel;
    Item_info	   *item;
{
    register Item_info *ip;
    int		    lowest_bottom = PANEL_ITEM_Y_START;
    int		    lowest_top = PANEL_ITEM_Y_START;
    int		    rightmost_right = PANEL_ITEM_X_START;
    int		    x_offset;
    int		    y_offset;

    if (item && item->x_gap >= 0)
	x_offset = item->x_gap;
    else
	x_offset = panel->item_x_offset;
    if (item && item->y_gap >= 0)
	y_offset = item->y_gap;
    else
	y_offset = panel->item_y_offset;

    if (!panel->items) {
	panel->max_item_y = 0;
	panel->item_x = PANEL_ITEM_X_START;
	panel->item_y = PANEL_ITEM_Y_START;
	return;
    }

    /*
     * Horizontal layout: find the lowest row of any item
     * Vertical layout: find the lowest row in the current column
     */
    for (ip = panel->items; ip; ip = ip->next) {
        if (panel->layout == PANEL_VERTICAL) {
            if (ip->rect.r_left >= panel->current_col_x)
                lowest_bottom = MAX(lowest_bottom, rect_bottom(&ip->rect));
        } else {
            lowest_top = MAX(lowest_top, ip->rect.r_top);
            lowest_bottom = MAX(lowest_bottom, rect_bottom(&ip->rect));
        }
    }
 
    /*
     * Horizontal layout: find the rightmost position on the lowest row
     * Vertical layout: find the rightmost position of any item
     */
    for (ip = panel->items; ip; ip = ip->next)
        if (panel->layout == PANEL_VERTICAL ||
            rect_bottom(&ip->rect) >= lowest_top)
            rightmost_right = MAX(rightmost_right, rect_right(&ip->rect));


    /* Update the panel info */
    panel->max_item_y = lowest_bottom - lowest_top; /* offset to next row */
    panel->item_x = rightmost_right + x_offset;
    panel->item_y = lowest_top;
    panel->lowest_bottom = lowest_bottom;
    panel->rightmost_right = rightmost_right;

    /* Advance to the next row if vertical layout or past right edge */
    if (panel->layout == PANEL_VERTICAL ||
	panel->item_x > panel_viewable_width(panel, panel->paint_window->pw)) {
	panel->item_x = panel->current_col_x;
	panel->item_y = lowest_bottom + y_offset;
	panel->max_item_y = 0;
    }
}


/****************************************************************************/
/* panel_item_layout                                                        */
/* lays out the generic item, label & value rects in ip and calls the       */
/* item's layout proc.                                                      */
/****************************************************************************/

Pkg_private void
panel_item_layout(ip, deltas)
    register Item_info *ip;
    register Rect  *deltas;
{
    /* item rect */
    ip->rect.r_left += deltas->r_left;
    ip->rect.r_top += deltas->r_top;

    /* label rect */
    ip->label_rect.r_left += deltas->r_left;
    ip->label_rect.r_top += deltas->r_top;

    /* value rect */
    ip->value_rect.r_left += deltas->r_left;
    ip->value_rect.r_top += deltas->r_top;

    /* item */
    if (ip->ops.panel_op_layout)
	(*ip->ops.panel_op_layout) (ITEM_PUBLIC(ip), deltas);
}


Pkg_private void
panel_check_item_layout(ip)
    Item_info *ip;
{
    Rect	    deltas;
    Rect	   *view_rect;

    /* Move the item to the start of the next row if its position is not fixed,
     * it doesn't start at the left margin, and it extends past the right edge
     * of the panel's viewable rectangle.
     */
    if (!(item_fixed(ip) || label_fixed(ip) || value_fixed(ip)) &&
	ip->rect.r_left > PANEL_ITEM_X_START)  {
	view_rect = panel_viewable_rect(ip->panel, ip->panel->paint_window->pw);
	if (rect_right(&ip->rect) > rect_right(view_rect)) {
	    deltas.r_left = PANEL_ITEM_X_START - ip->rect.r_left;
	    deltas.r_top = ip->panel->max_item_y + ip->panel->item_y_offset;
	    /* tell the item to move */
	    panel_item_layout(ip, &deltas);
	}
    }
}


Xv_public void
panel_paint_label(item_public)
    Panel_item	    item_public;
{
    Item_info	   *ip = ITEM_PRIVATE(item_public);

    panel_paint_image(ip->panel, &ip->label, &ip->label_rect, inactive(ip),
		      ip->color_index);
}


/****************************************************************************/
/* panel_paint_image                                                        */
/* paints image in pw in rect.                                              */
/****************************************************************************/
Pkg_private void
panel_paint_image(panel, image, rect, inactive_item, color_index)
    Panel_info     *panel;
    Panel_image    *image;
    Rect           *rect;
    int    	    inactive_item;
    int             color_index;
{
    int             baseline_x;	/* x-coordinate of text baseline */
    int             baseline_y;	/* y-coordinate of text baseline */
    int		    chrht;	/* default character height */
    int		    i;
    Xv_Drawable_info *info;
#ifdef OW_I18N
    XFontSet	    fontset_id;
#else
    XID		    font_xid;
#endif /* OW_I18N */
    int		    length;
    int		    line_start;
    int		    lines;
    int		    newline_found;
    GC             *gc_list;
    Xv_Window       pw;		/* paint window */
    Xv_Screen       screen;
    struct pr_size  size;
    CHAR	   *str;

    chrht = xv_get(image_font(image), FONT_DEFAULT_CHAR_HEIGHT);
    PANEL_EACH_PAINT_WINDOW(panel, pw)
	switch (image->im_type) {
	  case PIT_STRING:
#ifdef OW_I18N
	    str = image_string_wc(image);
#else
	    str = image_string(image);
#endif /* OW_I18N */
	    length = STRLEN(str);
	    lines = 1;
	    for (i=0; i<length; i++)
		if (str[i] == '\n')
		    lines++;
	    baseline_y = rect->r_top + panel_fonthome(image_font(image));
#ifdef OW_I18N
	    if (image_font(image))
		fontset_id = (XFontSet) xv_get(image_font(image), FONT_SET_ID);
	    else if (image_bold(image))
		fontset_id = panel->bold_fontset_id;
	    else
		fontset_id = panel->std_fontset_id;
#else
	    if (image_font(image))
		font_xid = (XID) xv_get(image_font(image), XV_XID);
	    else if (image_bold(image))
		font_xid = panel->bold_font_xid;
	    else
		font_xid = panel->std_font_xid;
#endif /* OW_I18N */
	    if (lines == 1) {
		/* Center single line within label rect */
		baseline_y += (rect->r_height - chrht) / 2;
#ifdef OW_I18N
		panel_paint_text(pw, fontset_id, color_index,
				 rect->r_left, baseline_y,
				 (wchar_t *) image_string_wc(image));
#else
		panel_paint_text(pw, font_xid, color_index,
				 rect->r_left, baseline_y,
				 (u_char *) image_string(image));
#endif /* OW_I18N */
	    } else {
		/* Paint multiple lines starting from top of label rect */
		line_start = 0;
		for (i=0; i<=length; i++) {
		    newline_found = str[i] == '\n';
		    if (i == length || newline_found) {
			if (newline_found)
			    str[i] = 0;
#ifdef OW_I18N
			size = xv_pf_textwidth_wc(i - line_start,
					       image_font(image),
					       &str[line_start]);
			baseline_x = rect->r_left + rect->r_width - size.x;
			panel_paint_text(pw, fontset_id, color_index,
			    baseline_x, baseline_y,
			    (wchar_t *) &str[line_start]);
#else
			size = xv_pf_textwidth(i - line_start,
					       image_font(image),
					       &str[line_start]);
			baseline_x = rect->r_left + rect->r_width - size.x;
			panel_paint_text(pw, font_xid, color_index,
			    baseline_x, baseline_y,
			    (u_char *) &str[line_start]);
#endif /* OW_I18N */
			if (newline_found)
			    str[i] = '\n';
			baseline_y += chrht;
			line_start = i + 1;
		    }
		}
	    }
	    break;

	  case PIT_SVRIM:
	    panel_paint_svrim(pw, (Pixrect *) image_svrim(image), rect->r_left,
			      rect->r_top, color_index, (Pixrect *)NULL);
	    break;
	}

	if (image_boxed(image)) {
	    if (color_index >= 0) {
		xv_draw_rectangle(pw, rect->r_left, rect->r_top,
				  rect->r_width - 1, rect->r_height - 1,
				  LineSolid, PIX_SRC | PIX_COLOR(color_index));
	    } else {
		DRAWABLE_INFO_MACRO(pw, info);
		screen = xv_screen(info);
		gc_list = (GC *)xv_get(screen, SCREEN_OLGC_LIST, pw);
		screen_adjust_gc_color(pw, SCREEN_SET_GC);
		XDrawRectangle(xv_display(info), xv_xid(info),
			       gc_list[SCREEN_SET_GC],
			       rect->r_left, rect->r_top,
			       rect->r_width - 1, rect->r_height - 1);
	    }
	}

	if (image_inverted(image))
	    panel_pw_invert(pw, rect, color_index);

	if (inactive_item) {
	    DRAWABLE_INFO_MACRO(pw, info);
	    screen = xv_screen(info);
	    gc_list = (GC *)xv_get(screen, SCREEN_OLGC_LIST, pw);
	    screen_adjust_gc_color(pw, SCREEN_INACTIVE_GC);
	    XFillRectangle(xv_display(info), xv_xid(info),
			   gc_list[SCREEN_INACTIVE_GC],
			   rect->r_left, rect->r_top,
			   rect->r_width, rect->r_height);
	}
    PANEL_END_EACH_PAINT_WINDOW
}


/****************************************************************************/
/* panel_invert                                                             */
/* inverts the rect r using panel's pixwin.                                 */
/****************************************************************************/
Pkg_private void
panel_invert(panel, r, color_index)
    Panel_info     *panel;
    register Rect  *r;
    int		    color_index;
{
    Xv_Window       pw;

    PANEL_EACH_PAINT_WINDOW(panel, pw)
	panel_pw_invert(pw, r, color_index);
    PANEL_END_EACH_PAINT_WINDOW
}

Pkg_private void
panel_pw_invert(pw, rect, color_index)
    Xv_Window pw;
    register Rect *rect;
    int color_index;
{
    XGCValues	gc_value;
    Xv_Drawable_info *info;
    Xv_Screen      screen;
    GC             *gc_list;

    DRAWABLE_INFO_MACRO(pw, info);
    screen = xv_screen(info);
    gc_list = (GC *)xv_get(screen, SCREEN_OLGC_LIST, pw);
    if (color_index >= 0)
	gc_value.foreground = xv_get(xv_cms(info), CMS_PIXEL, color_index);
    else
	gc_value.foreground = xv_fg(info);
    gc_value.background = xv_bg(info);
    gc_value.plane_mask = gc_value.foreground ^ gc_value.background;
    XChangeGC(xv_display(info), gc_list[SCREEN_INVERT_GC],
	      GCForeground | GCBackground | GCPlaneMask, &gc_value);
    XFillRectangle(xv_display(info), xv_xid(info),
		   gc_list[SCREEN_INVERT_GC],
		   rect->r_left, rect->r_top,
		   rect->r_width, rect->r_height);
}


/****************************************************************************/
/* panel_strsave                                                            */
/****************************************************************************/

Pkg_private char *
panel_strsave(source)
    char         *source;
{
    char         *dest;

    dest = (char *) xv_malloc(strlen(source) + 1);
    if (!dest)
	return NULL;

    (void) strcpy(dest, source);
    return dest;
}

#ifdef OW_I18N

/****************************************************************************/
/* panel_strsave_wc                                                         */
/****************************************************************************/

Pkg_private wchar_t *
panel_strsave_wc(source)
    wchar_t      *source;
{
    wchar_t      *dest;

    dest = (wchar_t *) xv_alloc_n(wchar_t, (wslen(source) + 1));
    if (!dest)
	return NULL;

    (void) wscpy(dest, source);
    return dest;
}

#endif /* OW_I18N */

/****************************************************************************/
/* miscellaneous utilities                                                  */
/****************************************************************************/

/*
 * Return max baseline offset for specified string.  This should be the same
 * value returned by XTextExtents in overall_return.descent.
 */
#ifdef OW_I18N

Pkg_private int
panel_fonthome(font)
    Xv_Font         font;
{
    register int    max_home = 0;
    XFontSet        font_set;
    XFontSetExtents *font_set_extents;
    int			pc_home_y;


    font_set = (XFontSet)xv_get(font, FONT_SET_ID);
    font_set_extents = XExtentsOfFontSet(font_set);
    pc_home_y = font_set_extents->max_logical_extent.y;

    if (pc_home_y < max_home)
        max_home = pc_home_y;
    return -(max_home);
}

#else

Pkg_private int
panel_fonthome(font)
    Xv_Font         font;
{
    register int    max_home = 0;
    XFontStruct		*x_font_info;
    int			pc_home_y;

    x_font_info = (XFontStruct *)xv_get(font, FONT_INFO);
    pc_home_y = -x_font_info->ascent;
    if (pc_home_y < max_home)
        max_home = pc_home_y;
    return -(max_home);
}

#endif /* OW_I18N */

/*VARARGS*/
panel_nullproc()
{
    return 0;
}


Pkg_private void
panel_free_choices(choices, first, last)
    Panel_image    *choices;
    int             first, last;
{
    register int    i;		/* counter */

    if (!choices || last < 0)
	return;

    /* free the choice strings */
    for (i = first; i <= last; i++) {
	panel_free_image(&choices[i]);
    }

    free((char *) choices);
}				/* panel_free_choices */

Pkg_private void
panel_free_image(image)
    Panel_image    *image;
{
    if (is_string(image)) {
#ifdef OW_I18N
        if (image_string(image))
                xv_free(image_string(image));
        if (image_string_wc(image))
                xv_free(image_string_wc(image));
#else
	free(image_string(image));
#endif /* OW_I18N */
    }
}

Pkg_private void
panel_set_bold_label_font(ip)
    register Item_info *ip;
{
    panel_image_set_font(&ip->label, ip->panel->bold_font);
    image_set_bold(&ip->label, TRUE);
}


Pkg_private void
panel_paint_text(pw, font_xid, color_index, x, y, str)
    Xv_opaque	pw;	/* paint window */
#ifdef OW_I18N
    XFontSet	font_xid;
#else
    Font	font_xid;
#endif /* OW_I18N */
    int		color_index;
    int		x, y;	/* baseline starting position */
    CHAR        *str;	/* text to paint */
{
    Display	*display;
    XGCValues	gc_value;
    Xv_Drawable_info *info;
    Drawable	xid;
    Xv_Screen      screen;
    GC             *gc_list;

    DRAWABLE_INFO_MACRO(pw, info);
    display = xv_display(info);
    xid = xv_xid(info);

    screen = xv_screen(info);
    gc_list = (GC *)xv_get(screen, SCREEN_OLGC_LIST, pw);
    if (color_index >= 0)
	gc_value.foreground = xv_get(xv_cms(info), CMS_PIXEL, color_index);
    else
	gc_value.foreground = xv_fg(info);
    gc_value.background = xv_bg(info);
    gc_value.function = GXcopy;
    gc_value.plane_mask = xv_plane_mask(info);
    gc_value.fill_style = FillSolid;
#ifdef OW_I18N
    XChangeGC(display, gc_list[OPENWIN_NONSTD_GC],
        GCForeground | GCBackground | GCFunction | GCPlaneMask |
        GCFillStyle, &gc_value);
    /* 1091366 - vmh */
    /* The question is, whether XwcDrawString shouldn't test itself
     * for a NULL string, not us
     */
    if (STRLEN(str) > 0)
      XwcDrawString(display, xid, font_xid, gc_list[OPENWIN_NONSTD_GC],
              x, y, str, wslen(str));
#else
    gc_value.font = font_xid;
    XChangeGC(display, gc_list[SCREEN_NONSTD_GC],
	      GCForeground | GCBackground | GCFunction | GCPlaneMask |
	      GCFillStyle | GCFont, &gc_value);
    XDrawString(display, xid, gc_list[SCREEN_NONSTD_GC], x, y, str,
		strlen(str));
#endif /* OW_I18N */
}


Pkg_private void
panel_paint_svrim(pw, pr, x, y, color_index, mask_pr)
    Xv_Window pw;	/* paint window */
    Pixrect *pr;	/* a Pixrect or a Server Image */
    int x;
    int y;
    int color_index;
    Pixrect *mask_pr;
{
    Display	   *display;
    GC             *gc_list;
    unsigned long   gc_mask;
    XGCValues	    gc_value;
    int		    mono_svrim_pw; /* paint window and Server Image are
				    * 1 bit deep */
    Xv_Drawable_info *svrim_info;  /* Server Image info */
    Xv_Drawable_info *pw_info;	   /* paint window info */
    Xv_Screen       screen;
    XID		    xid;

    DRAWABLE_INFO_MACRO(pw, pw_info);
    display = xv_display(pw_info);
    xid = xv_xid(pw_info);
    screen = xv_screen(pw_info);
    gc_list = (GC *)xv_get(screen, SCREEN_OLGC_LIST, pw);
    if (color_index >= 0)
	gc_value.foreground = xv_get(xv_cms(pw_info), CMS_PIXEL, color_index);
    else
	gc_value.foreground = xv_fg(pw_info);
    gc_value.background = xv_bg(pw_info);
    gc_mask = GCForeground | GCBackground;
    mono_svrim_pw = FALSE;
    if (PR_IS_SERVER_IMAGE(pr) && xv_depth(pw_info) == 1) {
	DRAWABLE_INFO_MACRO((Xv_opaque) pr, svrim_info);
	if (xv_depth(svrim_info) == 1) {
	    mono_svrim_pw = TRUE;
	    gc_value.ts_x_origin = x;
	    gc_value.ts_y_origin = y;
	    gc_value.stipple = xv_xid(svrim_info);
	    gc_value.fill_style = FillOpaqueStippled;
	    gc_mask |= GCTileStipXOrigin | GCTileStipYOrigin |
		GCStipple | GCFillStyle;
	}
    }

    if ( mask_pr ) {
	Xv_Drawable_info * mask_info;
	
	DRAWABLE_INFO_MACRO((Xv_opaque)mask_pr, mask_info);
	gc_value.clip_mask = xv_xid(mask_info);
	gc_value.clip_x_origin = x;
	gc_value.clip_y_origin = y;
	gc_mask |= GCClipMask | GCClipXOrigin | GCClipYOrigin;
    }

    XChangeGC(display, gc_list[SCREEN_NONSTD_GC],
	      gc_mask, &gc_value);
    if (mono_svrim_pw) {
	/* Note: xv_rop_internal messes up on monochrome screens
	 * whose BlackPixel is 0 and WhitePixel is 1.  Doing the
	 * painting here fixes this problem.
	 */
	XFillRectangle(display, xid, gc_list[SCREEN_NONSTD_GC], x, y,
		       pr->pr_width, pr->pr_height);
    } else {
	xv_rop_internal(display, xid, gc_list[SCREEN_NONSTD_GC],
			x, y, pr->pr_width, pr->pr_height,
			(Xv_opaque) pr, 0, 0, pw_info);
    }

    if ( mask_pr )
	XSetClipMask(display, gc_list[SCREEN_NONSTD_GC], None);
}


Pkg_private Panel_item
panel_set_kbd_focus(panel, ip)
    Panel_info     *panel;
    Item_info      *ip;
{
    if (ip == NULL || hidden(ip))
	return XV_ZERO;

    panel_yield_kbd_focus(panel);
    panel->kbd_focus_item = ip;
    panel_accept_kbd_focus(panel);
    return ITEM_PUBLIC(ip);
}


Xv_public       Panel_item
panel_advance_caret(panel_public)
    Panel           panel_public;
{
    Panel_info     *panel = PANEL_PRIVATE(panel_public);

    if (!panel->kbd_focus_item)
	return (XV_ZERO);

    (void) panel_set_kbd_focus(panel, panel_next_kbd_focus(panel, TRUE));

    return ITEM_PUBLIC(panel->kbd_focus_item);
}


Xv_public       Panel_item
panel_backup_caret(panel_public)
    Panel           panel_public;
{
    Panel_info     *panel = PANEL_PRIVATE(panel_public);

    if (!panel->kbd_focus_item)
	return (XV_ZERO);

    (void) panel_set_kbd_focus(panel, panel_previous_kbd_focus(panel, TRUE));

    return ITEM_PUBLIC(panel->kbd_focus_item);
}


/* Find the next item that wants keystrokes.  If no other item wants
 * keystrokes, then return NULL.
 */
Pkg_private Item_info *
panel_next_kbd_focus(panel, wrap)
    Panel_info	*panel;
    int		 wrap;	/* TRUE or FALSE */
{
    Item_info	*ip;

    ip = panel->kbd_focus_item;
    if (!ip)
	return NULL;
    do {
	ip = ip->next;
	if (!ip) {
	    if (wrap) {
		ip = panel->items;  /* wrap around to the top */
		if (!ip)
		    return NULL;  /* Safety check (shouldn't occur) */
	    } else
		return NULL;  /* no other item further down wants keystrokes */
	}
	if (ip == panel->kbd_focus_item)
	    return NULL;	/* no other item wants keystrokes */
    } while (!wants_key(ip) || hidden(ip) || inactive(ip));
    return(ip);
}


/* Find the previous item that wants keystrokes.  If no other item wants
 * keystrokes, then return NULL.
 */
Pkg_private Item_info *
panel_previous_kbd_focus(panel, wrap)
    Panel_info	*panel;
    int		 wrap;   /* TRUE or FALSE */
{
    Item_info	*ip;

    ip = panel->kbd_focus_item;
    if (!ip)
	return NULL;
    do {
	ip = ip->previous;
	if (!ip) {
	    if (wrap) {
		ip = panel->last_item;   /* wrap around to the bottom */
		if (!ip)
		    return NULL;  /* Safety check (shouldn't occur) */
	    } else
		return NULL;  /* no other item further up wants keystrokes */
	}
	if (ip == panel->kbd_focus_item)
	    return NULL;	/* no other item wants keystrokes */
    } while (!wants_key(ip) || hidden(ip) || inactive(ip));
    return(ip);
}


Pkg_private void
panel_accept_kbd_focus(panel)
    Panel_info     *panel;
{
    register Item_info *ip = panel->kbd_focus_item;

    if (!panel->status.has_input_focus || !ip)
	return;

    if (ip->ops.panel_op_accept_kbd_focus)
	(*ip->ops.panel_op_accept_kbd_focus) (ITEM_PUBLIC(ip));
    if (xv_get(ITEM_PUBLIC(ip), XV_FOCUS_RANK) == XV_FOCUS_PRIMARY)
	panel->primary_focus_item = ip;
}


Pkg_private void
panel_yield_kbd_focus(panel)
    Panel_info     *panel;
{
    register Item_info *ip = panel->kbd_focus_item;

    if (ip && ip->ops.panel_op_yield_kbd_focus)
	(*ip->ops.panel_op_yield_kbd_focus) (ITEM_PUBLIC(ip));
}


Pkg_private void
panel_clear_pw_rect(pw, rect)
    Xv_window	pw;
    Rect	rect;
{
    Display	   *display;
    Xv_Drawable_info *info;
    Drawable	    xid;

    DRAWABLE_INFO_MACRO(pw, info);
    display = xv_display(info);
    xid = xv_xid(info);
    XClearArea(display, xid, rect.r_left, rect.r_top,
	       rect.r_width, rect.r_height, False);
}


Pkg_private void
panel_clear_rect(panel, rect)
    Panel_info	*panel;
    Rect	rect;
{
    Xv_window	    pw;

    PANEL_EACH_PAINT_WINDOW(panel, pw)
	panel_clear_pw_rect(pw, rect);
    PANEL_END_EACH_PAINT_WINDOW
}


Pkg_private Rect *
panel_viewable_rect(panel, pw)
    Panel_info *panel;
    Xv_Window pw;
{
    static Rect viewable_rect;

    if (panel->paint_window->view)
	return ((Rect *) xv_get(panel->public_self, CANVAS_VIEWABLE_RECT, pw));
    else {
	viewable_rect = *(Rect *) xv_get(PANEL_PUBLIC(panel), XV_RECT);
	viewable_rect.r_left = 0;
	viewable_rect.r_top = 0;
	return (&viewable_rect);
    }
}


Pkg_private int
panel_viewable_height(panel, pw)
    Panel_info *panel;
    Xv_Window pw;
{
    if (panel->paint_window->view)
	return (((Rect *) xv_get((panel)->public_self, CANVAS_VIEWABLE_RECT,
		(pw)))->r_height);
    else
	return ((int) xv_get(PANEL_PUBLIC(panel), XV_HEIGHT));
}


Pkg_private int
panel_viewable_width(panel, pw)
    Panel_info *panel;
    Xv_Window pw;
{
    if (panel->paint_window->view)
	return (((Rect *) xv_get((panel)->public_self, CANVAS_VIEWABLE_RECT,
		(pw)))->r_width);
    else
	return ((int) xv_get(PANEL_PUBLIC(panel), XV_WIDTH));
}


Pkg_private int
panel_height(panel)
    Panel_info *panel;
{
    if (panel->paint_window->view)
	return ((int) xv_get(PANEL_PUBLIC(panel), CANVAS_HEIGHT));
    else
	return ((int) xv_get(PANEL_PUBLIC(panel), XV_HEIGHT));
}


Pkg_private int
panel_width(panel)
    Panel_info *panel;
{
    if (panel->paint_window->view)
	return ((int) xv_get(PANEL_PUBLIC(panel), CANVAS_WIDTH));
    else
	return ((int) xv_get(PANEL_PUBLIC(panel), XV_WIDTH));
}


/* panel_is_multiclick: Return TRUE if new click time is <= 
 *			OpenWindows.MulticlickTimeout msec since last
 *			click time; else, return FALSE.
 */
Pkg_private int
panel_is_multiclick(panel, last_click_time, new_click_time)
    Panel_info	   *panel;
    struct timeval *last_click_time;
    struct timeval *new_click_time;
{
    int             delta;

    if (last_click_time->tv_sec == 0 && last_click_time->tv_usec == 0)
	return FALSE;
    delta = (new_click_time->tv_sec - last_click_time->tv_sec) * 1000;
    delta += new_click_time->tv_usec / 1000;
    delta -= last_click_time->tv_usec / 1000;
    return (delta <= panel->multiclick_timeout);
}


static void
panel_set_cursor(panel, window, attr)
    Panel_info	   *panel;
    Xv_Window	    window;
    Attr_attribute  attr;	/* CURSOR_QUESTION_MARK_PTR or
				 * NULL = restore default cursor */
{
    Cursor	    cursor;
    Server_image    image;
    Xv_Drawable_info *info;

    DRAWABLE_INFO_MACRO(window, info);
    if (attr) {
	if (panel->status.nonstd_cursor)
	    return;	/* previous cursor not restored yet */
	panel->cursor = xv_get(window, WIN_CURSOR);
	cursor = xv_get(xv_screen(info), XV_KEY_DATA, attr);
	if (!cursor) {
	    /* Note: put a switch statement here to handle more cursors */
	    image = xv_create(xv_screen(info), SERVER_IMAGE,
			      XV_HEIGHT, 16,
			      XV_WIDTH, 16,
			      SERVER_IMAGE_BITS, qmark_cursor_data,
			      NULL);
	    cursor = xv_create(xv_root(info), CURSOR,
			       CURSOR_IMAGE, image,
			       CURSOR_XHOT, 6,
			       CURSOR_YHOT, 12,
			       NULL);
	    xv_set(xv_screen(info), XV_KEY_DATA, attr, cursor, NULL);
	}
	if (cursor) {
	    xv_set(window, WIN_CURSOR, cursor, NULL);
	    panel->status.nonstd_cursor = TRUE;
	}
    } else {
	/* Restore basic pointer */
	if (!panel->status.nonstd_cursor)
	    return;	/* cursor is already the basic pointer */
	xv_set(window, WIN_CURSOR, panel->cursor, NULL);
	panel->status.nonstd_cursor = FALSE;
    }
}


/* panel_user_error:  Show the question mark cursor on down event,
 *		      return to default (basic) cursor on up event.
 */
Pkg_private void
panel_user_error(object, event)
    Item_info	   *object;	/* Could also be a (Panel_info *) */
    Event	   *event;
{
    Xv_Window	    confine_window;
    Xv_Drawable_info *confine_info;
    Xv_Drawable_info *grab_info;
    Panel_info	   *panel;

    if (is_panel(object))
	panel = (Panel_info *) object;
    else
	panel = object->panel;
    if (panel->paint_window->view)
	/* Scrollable Panel: confine to the view window */
	confine_window = xv_get(event_window(event), CANVAS_PAINT_VIEW_WINDOW);
    else
	/* Panel: confine to the panel window */
	confine_window = PANEL_PUBLIC(panel);
    DRAWABLE_INFO_MACRO(confine_window, confine_info);
    DRAWABLE_INFO_MACRO(event_window(event), grab_info);
    if (event_is_down(event)) {
	panel_set_cursor(panel, event_window(event), CURSOR_QUESTION_MARK_PTR);
	if (!panel->status.pointer_grabbed) {
	    if (XGrabPointer(
		xv_display(grab_info),	/* display */
		xv_xid(grab_info),	/* grab window: grab on paint window */
		False,		/* owner events: report events relative to
			         * paint window */
		ButtonReleaseMask,	/* event mask */
		GrabModeAsync,	/* pointer mode */
		GrabModeAsync,	/* keyboard mode */
		xv_xid(confine_info),	/* confine to: confine pointer
					 * to view or panel window */
		None,		/* cursor: keep current pattern */
		CurrentTime) == GrabSuccess)
		panel->status.pointer_grabbed = TRUE;
	}
    } else {
	panel_set_cursor(panel, event_window(event), NULL);
	if (panel->status.pointer_grabbed) {
	    XUngrabPointer(xv_display(grab_info), CurrentTime);
	    panel->status.pointer_grabbed = FALSE;
	}
    }
}


Xv_public void
panel_show_focus_win(item_public, frame, x, y)
    Panel_item	    item_public;
    Frame	    frame;
    int		    x;	/* x-coordinate for focus window */
    int		    y;	/* y-coordinate for focus window */
{
    Xv_Window	    focus_win;
    Item_info	   *ip = ITEM_PRIVATE(item_public);
    int		    max_view_start;
    Scrollbar	    sb;
    int		    view_length;
    int		    view_start;
    Xv_Window	    view_window;


    if(ip->panel->status.mouseless == FALSE)
        return;

    if (ip->panel->paint_window->view) {
	/*
	 * Insure that x and y are visible, scrolling if necessary.
	 */
	view_window = xv_get(ip->panel->focus_pw, CANVAS_PAINT_VIEW_WINDOW);
	sb = xv_get(PANEL_PUBLIC(ip->panel), OPENWIN_VERTICAL_SCROLLBAR,
		    view_window);
	if (sb) {
	    view_start = (int) xv_get(sb, SCROLLBAR_VIEW_START);
	    view_length = (int) xv_get(sb, SCROLLBAR_VIEW_LENGTH);
	    if (y < view_start || y >= view_start + view_length) {
		max_view_start = (int) xv_get(sb, SCROLLBAR_OBJECT_LENGTH) -
		    view_length;
		view_start = MIN(y, max_view_start);
		xv_set(sb, SCROLLBAR_VIEW_START, view_start, NULL);
	    }
	}
	sb = xv_get(PANEL_PUBLIC(ip->panel), OPENWIN_HORIZONTAL_SCROLLBAR,
		    view_window);
	if (sb) {
	    view_start = (int) xv_get(sb, SCROLLBAR_VIEW_START);
	    view_length = (int) xv_get(sb, SCROLLBAR_VIEW_LENGTH);
	    if (x < view_start || x >= view_start + view_length) {
		max_view_start = (int) xv_get(sb, SCROLLBAR_OBJECT_LENGTH) -
		    view_length;
		view_start = MIN(x, max_view_start);
		xv_set(sb, SCROLLBAR_VIEW_START, view_start, NULL);
	    }
	}
    }
    focus_win = xv_get(frame, FRAME_FOCUS_WIN);
    xv_set(focus_win,
	   WIN_PARENT, ip->panel->focus_pw,
	   XV_X, x,
	   XV_Y, y,
	   XV_SHOW, TRUE,
	   NULL);
}


Pkg_private int
panel_round(x, y)
    register int    x, y;
{
    register int    z, rem;
    register short  is_neg = FALSE;

    if (x < 0) {
	x = -x;
	if (y < 0)
	    y = -y;
	else
	    is_neg = TRUE;
    } else if (y < 0) {
	y = -y;
	is_neg = TRUE;
    }
    z = x / y;
    rem = x % y;
    /* round up if needed */
    if (2 * rem >= y)
	if (is_neg)
	    z--;
	else
	    z++;

    return (is_neg ? -z : z);
}


Pkg_private int
panel_wants_focus(panel)	/* returns TRUE or FALSE */
    Panel_info	   *panel;
{
    Item_info	   *ip;
    int		    wants_focus;

    wants_focus = wants_key(panel) || panel->kbd_focus_item;
    if (!wants_focus) {
	for (ip = panel->items; ip; ip = ip->next) {
	    if (wants_key(ip) && !hidden(ip) && !inactive(ip)) {
		wants_focus = TRUE;
		break;
	    }
	}
    }
    return wants_focus;
}


/*
 * setup timer that is used to implement autoscroll
 * panel text item and panel numeric text item
 */
    Pkg_private void
panel_autoscroll_start_itimer( item, autoscroll_itimer_func )
    Panel_item          item;
    int			*autoscroll_itimer_func;
{
    struct itimerval    timer;
    int                 delay_time;
    int                 interval_time;

    delay_time = defaults_get_integer_check("scrollbar.repeatDelay",
        "Scrollbar.RepeatDelay", 100, 0, 999);

    interval_time = defaults_get_integer_check("scrollbar.lineInterval",
        "Scrollbar.LineInterval", 1, 0, 999);

    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = delay_time * 1000;

    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = interval_time * 1000;

    (void) notify_set_itimer_func((Notify_client)item,
        (Notify_func)autoscroll_itimer_func, ITIMER_REAL, 
	&timer, NULL );
} /* panel_autoscroll_start_itimer */



    Pkg_private void
panel_autoscroll_stop_itimer( item )
    Panel_item          item;
{
    (void) notify_set_itimer_func((Notify_client)item,
        NOTIFY_FUNC_NULL, ITIMER_REAL, NULL, NULL );
} /* panel_autoscroll_stop_itimer */

