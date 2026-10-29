#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)item_set.c 20.104 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */


#include <xview_private/panel_impl.h>
#include <xview_private/draw_impl.h>
#include <xview/openmenu.h>

Xv_public struct pr_size xv_pf_textwidth();
#ifdef  OW_I18N
extern struct pr_size xv_pf_textwidth_wc();
#endif  /* OW_I18N */

Xv_private int	    panel_item_parent();
Xv_private void	    win_set_no_focus();
Xv_private Graphics_info *xv_init_olgx();

static void item_adjust_label_size();

static
  fix_positions(register Item_info *ip);

extern Notify_value panel_base_event_handler();

Pkg_private     Xv_opaque
item_set_avlist(item_public, avlist)
    Panel_item      item_public;
    Attr_avlist     avlist;
{
    Attr_avlist	    attrs;
    Item_info	   *ip = ITEM_PRIVATE(item_public);
    Panel_image	   *label = &ip->label;
    Panel_info	   *panel = ip->panel;
    int		    check_wants_focus = FALSE;
#ifdef ATTR_CONSUME_BUGS_FIXED
    int		    consumed;
#endif
    Rect            deltas;
    Xv_Drawable_info *info;
    int             item_x;
    short           item_x_changed = FALSE;
    int		    item_y;
    short           item_y_changed = FALSE;
    int		    label_boxed = image_boxed(label);
    Xv_opaque       label_data = 0;
    short           label_font_changed = FALSE;
    int             label_inverted = image_inverted(label);
    struct pr_size  label_size;
    short           label_size_changed = FALSE;
    int             label_type = -1;	/* -1 = no new label yet */
    short	    label_width_changed = FALSE;
    int             label_x;
    short           label_x_changed = FALSE;
    int		    label_y;
    short           label_y_changed = FALSE;
    short           layout_changed = FALSE;
    int             next_col_gap;
    int             next_row_gap;
    short           potential_new_rect = FALSE;
    Xv_opaque	    pw;	/* paint window */
    Panel_setting   saved_repaint = ip->repaint;
    int             start_new_col = FALSE;
    int             start_new_row = FALSE;
    int             value_x;
    short           value_x_changed = FALSE;
    int		    value_y;
    short           value_y_changed = FALSE;
    int		    wants_focus;

    /* If a client has called panel_item_parent this item may not
     * have a parent; do nothing in this case.
     */
    if (panel == NULL) {
	return ((Xv_opaque) XV_ERROR);
    }

    for (attrs = avlist; *attrs; attrs = attr_next(attrs)) {
#ifdef ATTR_CONSUME_BUGS_FIXED
	consumed = TRUE;
#endif
	switch (attrs[0]) {

	  case PANEL_EVENT_PROC:
	    ip->ops.panel_op_handle_event = (void (*) ()) attrs[1];
	    break;

	  case XV_SHOW:
	    if ((int) attrs[1]) {
		if (hidden(ip)) {
		    ip->flags &= ~HIDDEN;
		    if (ip->ops.panel_op_restore)
			(*ip->ops.panel_op_restore) (ITEM_PUBLIC(ip));
		    panel_update_extent(panel, ip->rect);
		}
	    } else if (!hidden(ip)) {
		panel_clear_item(ip);
		ip->flags |= HIDDEN;
		if (ip->ops.panel_op_remove)
		    (*ip->ops.panel_op_remove) (ITEM_PUBLIC(ip));
	    }
	    check_wants_focus = wants_key(ip);
	    break;

	  case PANEL_CHILD_CARET_ITEM:
	    ip->child_kbd_focus_item = (Panel_item) attrs[1];
	    break;

	  case PANEL_ITEM_OWNER:
	    ip->owner = (Panel_item) attrs[1];
	    break;

	  /* PANEL_ITEM_X attr must still be supported in addition to XV_X
	   * and XV_Y because they can be used on the panel. When used
	   * on the panel PANEL_ITEM_X != XV_X on the panel.
	   */
	  case XV_X:
	  case PANEL_ITEM_X:
	    item_x = (int) attrs[1];
	    ip->flags |= ITEM_X_FIXED;
	    item_x_changed = TRUE;
	    potential_new_rect = TRUE;
	    break;

	  case XV_Y:
	  case PANEL_ITEM_Y:
	    item_y = (int) attrs[1];
	    ip->flags |= ITEM_Y_FIXED;
	    item_y_changed = TRUE;
	    potential_new_rect = TRUE;
	    break;

	  case PANEL_NEXT_COL:
	    next_col_gap = (int) attrs[1];
	    if (next_col_gap == 0)
		next_col_gap = 1;
	    start_new_col = TRUE;
	    ip->flags |= ITEM_X_FIXED | ITEM_Y_FIXED;
	    break;

	  case PANEL_NEXT_ROW:
	    next_row_gap = (int) attrs[1];
	    if (next_row_gap == 0)
		next_row_gap = 1;
	    start_new_row = TRUE;
	    ip->flags |= ITEM_X_FIXED | ITEM_Y_FIXED;
	    break;

	  case PANEL_LABEL_X:
	    label_x = (int) attrs[1];
	    ip->flags |= LABEL_X_FIXED;
	    label_x_changed = TRUE;
	    potential_new_rect = TRUE;
	    break;

	  case PANEL_LABEL_Y:
	    label_y = (int) attrs[1];
	    ip->flags |= LABEL_Y_FIXED;
	    label_y_changed = TRUE;
	    potential_new_rect = TRUE;
	    break;

	  case PANEL_VALUE_FONT:
	    if (ip->value_font != panel->std_font)
		xv_set(ip->value_font, XV_DECREMENT_REF_COUNT, NULL);
	    if (attrs[1]) {
		int three_d;
		ip->value_font = (Xv_Font) attrs[1];
#ifdef OW_I18N
		ip->value_fontset_id = (XFontSet)
			xv_get(ip->value_font, FONT_SET_ID);
#else
		ip->value_font_xid = (XID) xv_get(ip->value_font, XV_XID);
#endif /* OW_I18N */
		if (ip->value_font != panel->std_font)
		    xv_set(ip->value_font, XV_INCREMENT_REF_COUNT, NULL);
		three_d = panel->status.three_d ? TRUE : FALSE;
		ip->value_ginfo = xv_init_olgx(PANEL_PUBLIC(panel),
		    &three_d, attrs[1]);
	    } else {
		ip->value_font = panel->std_font;
#ifdef OW_I18N
		ip->value_fontset_id = panel->std_fontset_id;
#else
		ip->value_font_xid = panel->std_font_xid;
#endif /* OW_I18N */
		ip->value_ginfo = panel->ginfo;
	    }
	    break;

	  case PANEL_VALUE_X:
	    value_x = (int) attrs[1];
	    ip->flags |= VALUE_X_FIXED;
	    value_x_changed = TRUE;
	    potential_new_rect = TRUE;
	    break;

	  case PANEL_VALUE_Y:
	    value_y = (int) attrs[1];
	    ip->flags |= VALUE_Y_FIXED;
	    value_y_changed = TRUE;
	    potential_new_rect = TRUE;
	    break;

	  case PANEL_LABEL_IMAGE:
	    label_type = PIT_SVRIM;
	    label_data = attrs[1];
	    potential_new_rect = TRUE;
	    break;

#ifdef OW_I18N
          case PANEL_LABEL_STRING:
            label_type = PIT_STRING;
            /* Panel will only process wchar_t strings. Make temp space for
             * this string and free it at the end of this routine.
             * panel_make_image will do the real malloc for this string.
             * Note: Can optimize this code for performance.
             */
            label_data = (Xv_opaque) _xv_mbstowcsdup((char *)attrs[1]);
            potential_new_rect = TRUE;  
            break;

          case PANEL_LABEL_STRING_WCS:
            label_type = PIT_STRING; 
            label_data = (Xv_opaque) panel_strsave_wc((wchar_t *)attrs[1]); 
            potential_new_rect = TRUE;
            break;
#else
	  case PANEL_LABEL_STRING:
	    label_type = PIT_STRING;
	    label_data = attrs[1];
	    potential_new_rect = TRUE;
	    break;
#endif /* OW_I18N */

	  case PANEL_LABEL_FONT:
	    panel_image_set_font(&ip->label, (Xv_Font) attrs[1]);
	    if (attrs[1]) {
		int three_d = panel->status.three_d ? TRUE : FALSE;
		image_ginfo(&ip->label) = xv_init_olgx(PANEL_PUBLIC(panel),
		    &three_d, attrs[1]);
		panel->status.three_d = three_d;
	    }
	    label_font_changed = TRUE;
	    potential_new_rect = TRUE;
	    break;

	  case PANEL_LABEL_BOLD:
	    if (ip->item_type == PANEL_MESSAGE_ITEM) {
		if (attrs[1] != image_bold(&ip->label)) {
		    if (attrs[1]) {
			panel_image_set_font(&ip->label, panel->bold_font);
			image_set_bold(&ip->label, TRUE);
		    } else {
			panel_image_set_font(&ip->label, panel->std_font);
			image_set_bold(&ip->label, FALSE);
		    }
		    label_font_changed = TRUE;
		    potential_new_rect = TRUE;
		}
	    }
	    break;

	  case PANEL_LABEL_BOXED:
	    label_boxed = (int) attrs[1];
	    image_set_boxed(label, label_boxed);
	    break;

	  case PANEL_LABEL_INVERTED:
	    label_inverted = (int) attrs[1];
	    image_set_inverted(label, label_inverted);
	    break;

	  case PANEL_LABEL_WIDTH:
	    if (ip->label_width != (int) attrs[1]) {
		ip->label_width = (int) attrs[1];
		label_width_changed = TRUE;
	    }
	    break;

	  case PANEL_LAYOUT:
	    switch ((Panel_setting) attrs[1]) {
	      case PANEL_HORIZONTAL:
	      case PANEL_VERTICAL:
		ip->layout = (Panel_setting) attrs[1];
		layout_changed = TRUE;
		potential_new_rect = TRUE;
		break;

	      default:
		/* invalid layout */
		break;
	    }
	    break;

	  case PANEL_MENU_ITEM:
	    if (attrs[1])
		ip->flags |= IS_MENU_ITEM;
	    else
		ip->flags &= ~IS_MENU_ITEM;
	    break;

#ifdef OW_I18N
          case PANEL_NOTIFY_PROC:
            ip->notify = (int (*) ()) attrs[1];
            ip->flags &= ~WCHAR_NOTIFY;
            if (!ip->notify)
                ip->notify = panel_nullproc;
            break;

          case PANEL_NOTIFY_PROC_WCS:
            ip->notify_wc = (int (*) ()) attrs[1];
            ip->flags |= WCHAR_NOTIFY;
            if (!ip->notify_wc)
                ip->notify_wc = panel_nullproc;
            break;
#else
	  case PANEL_NOTIFY_PROC:
	    ip->notify = (int (*) ()) attrs[1];
	    if (!ip->notify)
		ip->notify = panel_nullproc;
	    break;
#endif /* OW_I18N */

	  case PANEL_NOTIFY_STATUS:
	    ip->notify_status = (int) attrs[1];
	    break;

	  case PANEL_PAINT:
	    ip->repaint = (Panel_setting) attrs[1];
	    break;

	  case PANEL_ACCEPT_KEYSTROKE:
	    if (attrs[1]) {
		check_wants_focus = !wants_key(ip);
		ip->flags |= WANTS_KEY;
	    } else {
		check_wants_focus = wants_key(ip);
		ip->flags &= ~WANTS_KEY;
	    }
	    break;

	  case PANEL_ITEM_MENU:
	    if (ip->menu)
		xv_set(ip->menu, XV_DECREMENT_REF_COUNT, NULL);
	    if (ip->menu = (Xv_opaque) attrs[1])
		xv_set(ip->menu,
		       XV_INCREMENT_REF_COUNT,
		       NULL);
	    if (label_type == -1) {
		label_type = label->im_type;	/* force label (image) to be
						 * remade */
		if (label_type == PIT_STRING)
#ifdef OW_I18N
		{
		    if (label_data) xv_free(label_data);
		    label_data = (Xv_opaque) panel_strsave_wc(image_string_wc(label));
		}    
#else
		    label_data = (Xv_opaque) image_string(label);
#endif /* OW_I18N */
		else
		    label_data = (Xv_opaque) image_svrim(label);
	    }
	    break;

	  case PANEL_CLIENT_DATA:
	    ip->client_data = attrs[1];
	    break;

	  case PANEL_BUSY:
	    if (attrs[1])
		ip->flags |= BUSY;
	    else
		ip->flags &= ~BUSY;
	    ip->flags |= BUSY_MODIFIED;
	    break;

	  case PANEL_INACTIVE:
	    if (attrs[1]) {
		ip->flags |= INACTIVE;
		if (panel->kbd_focus_item == ip) {
		    /* Item has keyboard focus: Move keyboard focus to next
		     * item that wants keystrokes, if any.
		     */
		    panel_yield_kbd_focus(panel);
		    panel->kbd_focus_item = panel_next_kbd_focus(panel, TRUE);
		    if (panel->kbd_focus_item)
			panel_accept_kbd_focus(panel);
		}
	    } else
		ip->flags &= ~INACTIVE;
	    check_wants_focus = wants_key(ip);
	    break;

	  case PANEL_ITEM_COLOR:
	    DRAWABLE_INFO_MACRO(panel->paint_window->pw, info);
	    if (xv_depth(info) > 1)
		ip->color_index = (int) attrs[1];
	    break;

	  case PANEL_OPS_VECTOR:
	    ip->ops = *(Panel_ops *) attrs[1];
	    if (panel->event_proc)
		ip->ops.panel_op_handle_event =
		    (void (*) ()) panel->event_proc;
	    break;

	  case PANEL_ITEM_DEAF:
	    if (attrs[1])
		ip->flags |= DEAF;
	    else
		ip->flags &= ~DEAF;
	    break;
	
	  case PANEL_ITEM_WANTS_ADJUST:
	    if (attrs[1])
		ip->flags |= WANTS_ADJUST;
	    else
		ip->flags &= ~WANTS_ADJUST;
	    break;
	
/* OW_I18N:  May need to add a wchar_t parallel attribute??? *
 * PANEL_ITEM_WANTS_WC					     */

	  case PANEL_ITEM_WANTS_ISO:
	    if (attrs[1])
		ip->flags |= WANTS_ISO;
	    else
		ip->flags &= ~WANTS_ISO;
	    break;

	  case PANEL_ITEM_LABEL_RECT:
	    ip->label_rect = *(Rect *) attrs[1];
	    break;

	  case PANEL_ITEM_VALUE_RECT:
	    ip->value_rect = *(Rect *) attrs[1];
	    break;

	  case PANEL_ITEM_RECT:
	  case XV_RECT:
	    ip->rect = *(Rect *) attrs[1];
	    break;

	case PANEL_POST_EVENTS:
	    if (attrs[1]) {
		ip->flags |= POST_EVENTS;
		notify_set_event_func(item_public, 
				      panel_base_event_handler, 
				      NOTIFY_IMMEDIATE);
	    }
	    else
		ip->flags &= ~POST_EVENTS;
	    break;

	  case XV_END_CREATE:
#ifdef ATTR_CONSUME_BUGS_FIXED
	    consumed = FALSE;
#endif
	    /* Append the new item to the list of items in the panel.
	     * Note: This is done at XV_END_CREATE time since an item
	     * might create an imbedded item, in which case the imbedded
	     * item needs to be listed first in the panel item list.
	     * Examples of this are PANEL_NUMERIC_TEXT, PANEL_SLIDER
	     * and PANEL_LIST, each of which create an imbedded PANEL_TEXT
	     * item.
	     */
	    panel_append(ip);

	    ip->flags |= CREATED;
	    panel_check_item_layout(ip);
	    check_wants_focus = TRUE;

	    /*
	     * for scrolling computations, note new extent of panel, tell
	     * scrollbars
	     */
	    if (!hidden(ip))
		panel_update_extent(panel, ip->rect);
	    break;

	  default:
#ifdef ATTR_CONSUME_BUGS_FIXED
	    consumed = FALSE;
#endif
	    /* Note: xv_check_bad_attr is not called since item_set_avlist
	     * may be called via xv_super_set_avlist in one of the
	     * panel item set routines.
	     */
	    break;
	}
#ifdef ATTR_CONSUME_BUGS_FIXED
	/* BUG ALERT:  Item set routines call xv_super_set_avlist,
	 * which calls this routine.  If this routine consumes attrs,
	 * then when xv_super_set_avlist returns to the item set routines,
	 * the switch (attrs[0]) which then gets executed will not see
	 * the attributes item_set_avlist just consumed.
	 */
	if (consumed)
	    ATTR_CONSUME(attrs[0]);
#endif
    }

    if (check_wants_focus) {
	wants_focus = panel_wants_focus(panel);
	PANEL_EACH_PAINT_WINDOW(panel, pw)
	    win_set_no_focus(pw, !wants_focus);
	PANEL_END_EACH_PAINT_WINDOW
    }

    /*
     * Handle any item position changes.
     */
    if (start_new_row) {
	if (!item_x_changed)
	    item_x = PANEL_ITEM_X_START;
	item_y = panel->lowest_bottom +
	    (next_row_gap >= 0 ? next_row_gap : panel->item_y_offset);
    }
    if (start_new_col) {
	if (!item_y_changed)
	    item_y = PANEL_ITEM_Y_START;
	item_x = panel->rightmost_right +
	    (next_col_gap >= 0 ? next_col_gap : panel->item_x_offset);
	panel->current_col_x = item_x;
    }
    if (start_new_row || start_new_col || item_x_changed || item_y_changed) {
	rect_construct(&deltas, 0, 0, 0, 0);
	/* compute item offset */
	if (start_new_row || start_new_col || item_x_changed)
	    deltas.r_left = item_x - ip->rect.r_left;
	if (start_new_row || start_new_col || item_y_changed)
	    deltas.r_top = item_y - ip->rect.r_top;
	if (deltas.r_left || deltas.r_top)
	    /*
	     * ITEM_X or ITEM_Y has changed, so re-layout item in order to
	     * cause the entire item to change position.
	     */
	    panel_item_layout(ip, &deltas);
    }
    /*
     * Now handle label size changes .
     */

    /*
     * update label font & boldness if needed. Note that this may be set
     * again below if a new label string or image has been given.
     */
    if (is_string(label) && label_font_changed) {
#ifdef OW_I18N
	label_size = xv_pf_textwidth_wc(STRLEN(image_string_wc(label)),
				     image_font(label), image_string_wc(label));
#else
	label_size = xv_pf_textwidth(strlen(image_string(label)),
				     image_font(label), image_string(label));
#endif /* OW_I18N */
	if (ip->label_width)
	    label_size.x = ip->label_width;
	item_adjust_label_size(ip->item_type, PIT_STRING, &label_size,
	    ip->menu ? TRUE : FALSE, panel->ginfo);
	ip->label_rect.r_width = label_size.x;
	ip->label_rect.r_height = label_size.y;
	label_size_changed = TRUE;
    }
    /* free old label, allocate new */
    if (set(label_type)) {
	/* Note: panel_make_image sets label->im_type */
	label_size = panel_make_image(image_font(label), label,
	    label_type, label_data, image_bold(label), label_inverted);
	if (ip->label_width)
	    label_size.x = ip->label_width;
	item_adjust_label_size(ip->item_type, image_type(label), &label_size,
	    ip->menu ? TRUE : FALSE, panel->ginfo);
	ip->label_rect.r_width = label_size.x;
	ip->label_rect.r_height = label_size.y;
	label_size_changed = TRUE;
    }
    if (!label_size_changed && !rect_isnull(&ip->label_rect) &&
	label_width_changed) {
	/* Adjust the label width */
	ip->label_rect.r_width = ip->label_width +
	    2*ButtonEndcap_Width(panel->ginfo);
	if (ip->menu)
	    ip->label_rect.r_width += 2*MenuMark_Width(panel->ginfo);
	label_size_changed = TRUE;
    }

    /*
     * use default positions for label or value if not specified.
     */
    if (layout_changed || label_size_changed || label_x_changed ||
	label_y_changed) {
	/*
	 * layout, label position or size has changed, so re-compute default
	 * value position.
	 */
	if (label_x_changed)
	    ip->label_rect.r_left = label_x;
	if (label_y_changed)
	    ip->label_rect.r_top = label_y;

	/* now move the value if it's not fixed */
	fix_positions(ip);
    }
    if (value_x_changed || value_y_changed) {
	/*
	 * value position has changed, so re-compute default label position.
	 */
	rect_construct(&deltas, 0, 0, 0, 0);
	if (value_x_changed) {
	    deltas.r_left = value_x - ip->value_rect.r_left;
	    ip->value_rect.r_left = value_x;
	}
	if (value_y_changed) {
	    deltas.r_top = value_y - ip->value_rect.r_top;
	    ip->value_rect.r_top = value_y;
	}
	if (deltas.r_left || deltas.r_top) {
	    /*
	     * VALUE_X or VALUE_Y has changed, so tell item to shift all its
	     * components (choices, marks etc.).
	     */
	    if (ip->ops.panel_op_layout)
		(*ip->ops.panel_op_layout) (ITEM_PUBLIC(ip), &deltas);

	    /* now move the label if it's not fixed */
	    fix_positions(ip);
	}
    }
    /* make sure the item rect encloses the label and value */
    ip->rect = panel_enclosing_rect(&ip->label_rect, &ip->value_rect);

    /* for scrolling computations, note new extent of panel, tell scrollbars */
    if (potential_new_rect && !hidden(ip))
	panel_update_extent(panel, ip->rect);

    if (panel->status.painted &&
        !panel->no_redisplay_item)
        panel_redisplay_item(ip, ip->repaint);

    /*
     * Restore the item's original repaint behavior.  This allows clients to
     * specify a 'one time' repaint policy for this call only.
     */
    ip->repaint = saved_repaint;

    return XV_OK;
}


/* Adjust size for borders around label */
static void
item_adjust_label_size(item_type, label_type, size, menu_attached, ginfo)
    Panel_item_type item_type;
    int		    label_type;	/* PIT_STRING or PIT_SVRIM */
    struct pr_size *size;
    int		    menu_attached;	/* boolean */
    Graphics_info  *ginfo;
{

    if (item_type != PANEL_BUTTON_ITEM)
	return;

    if (label_type == PIT_STRING) {
        size->x += 2*ButtonEndcap_Width(ginfo);
	size->y = Button_Height(ginfo);
    } else {
        size->x += OLGX_VAR_HEIGHT_BTN_MARGIN;
	size->y += OLGX_VAR_HEIGHT_BTN_MARGIN;
    }
    if (menu_attached) {
	size->x += 2*MenuMark_Width(ginfo);
	if (size->y < MenuMark_Height(ginfo))
	    size->y = MenuMark_Height(ginfo);
    }
}


/* fix_positions - of label and value rects */

static
  fix_positions(register Item_info *ip)
{
    if (!value_fixed(ip)) {
	struct rect     deltas;

	/* compute the value position which is to the right of the label */
	/* remember the old value rect position */
	rect_construct(&deltas, 0, 0, 0, 0);
	deltas.r_left = ip->value_rect.r_left;
	deltas.r_top = ip->value_rect.r_top;
	switch (ip->layout) {
	  case PANEL_HORIZONTAL:
	    /* after => to right of */
	    ip->value_rect.r_left = rect_right(&ip->label_rect) + 1 +
		(ip->label_rect.r_width ? LABEL_X_GAP : 0);
	    ip->value_rect.r_top = ip->label_rect.r_top;
	    break;

	  case PANEL_VERTICAL:
	    /* after => below */
	    ip->value_rect.r_left = ip->label_rect.r_left;
	    ip->value_rect.r_top = rect_bottom(&ip->label_rect) + 1 +
		(ip->label_rect.r_height ? LABEL_Y_GAP : 0);
	    break;
	}
	/* delta is new postion minus old position */
	deltas.r_left = ip->value_rect.r_left - deltas.r_left;
	deltas.r_top = ip->value_rect.r_top - deltas.r_top;
	if (deltas.r_left || deltas.r_top)
	    /*
	     * VALUE_X or VALUE_Y has changed, so tell item to shift all its
	     * components (choices, marks etc.).
	     */
	    if (ip->ops.panel_op_layout)
		(*ip->ops.panel_op_layout) (ITEM_PUBLIC(ip), &deltas);
    }
    panel_fix_label_position(ip);
}


Pkg_private void
panel_fix_label_position(ip)
    register Item_info *ip;
{
    if (label_fixed(ip))
	return;

    /* compute the label position as before the value. */
    switch (ip->layout) {
      case PANEL_HORIZONTAL:
	/* before => to left of */
	ip->label_rect.r_left = ip->value_rect.r_left;
	if (ip->label_rect.r_width > 0)
	    ip->label_rect.r_left -= ip->label_rect.r_width + LABEL_X_GAP;
	ip->label_rect.r_top = ip->value_rect.r_top;
	break;

      case PANEL_VERTICAL:
	/* before => above */
	ip->label_rect.r_left = ip->value_rect.r_left;
	ip->label_rect.r_top = ip->value_rect.r_top;
	if (ip->label_rect.r_height > 0)
	    ip->label_rect.r_top -= ip->label_rect.r_height + LABEL_Y_GAP;
	break;
    }
}


/****************************************************************

                 panel_item_parent

panel_item_parent has been supplied for internal SPD use only.
In particular the information management (database) group uses
it. The function is designed to let a client unparent and reparent
a panel item. One can implement a cache of items with this function
where the items are shared between multiple panels. We provided
this routine rather then making XV_OWNER settable
because too much of the item code expects to have a parent. We
would have had to rewrite too much of the panel code to make this
work from an attribute. As it is we provide this functionality
by letting the client use this routine to unset and reset an
item's parent. While an item has no parent a user can't set
any of its attributes. Code has been put in to prevent this.
The client can however get an items attributes that aren't
depenedent on the panel. If the item's attriibute does depend on
the panel NULL is returned.


The parent parameter is used to both reset and unset an item's
parent. If parent == NULL then the item will not have a parent
after it is removed from its current panel. If the parent != NULL
then the item will be reparented to parent

****************************************************************/

Xv_private
panel_item_parent(item, parent)
    Panel_item      item;
    Panel           parent;
{
    register Item_info *ip = ITEM_PRIVATE(item);
    register Panel_info *new_panel = NULL;
    register Panel_info *current_panel = ip->panel;

    if (parent != XV_ZERO) {
	new_panel = PANEL_PRIVATE(parent);
    }
    if (current_panel != NULL) {
	/* clear the  item */
	if (!current_panel->status.destroying) {
	    if (ip->item_type == PANEL_TEXT_ITEM &&
		ip == current_panel->kbd_focus_item) {
		panel_text_caret_on(current_panel, FALSE);
	    }
	    panel_clear_item(ip);
	}
	/* unlink the item */
	panel_unlink(ip, FALSE);/* unlink is not part of a destroy */
    }
    if (new_panel != NULL) {
	ip->panel = new_panel;

	/* relink the item */
	panel_append(ip);
	if (ip->ops.panel_op_restore)
	    (*ip->ops.panel_op_restore) (ITEM_PUBLIC(ip));

	/* don't repaint -- the client is probably doing a set */
	/* right after this to reposition the item */
    }
}
