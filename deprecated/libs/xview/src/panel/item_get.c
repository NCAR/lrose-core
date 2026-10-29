#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)item_get.c 20.52 93/06/28 Copyr 1984 Sun Micro";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#include <xview_private/panel_impl.h>
#include <xview/cms.h>

Pkg_private     Xv_opaque
item_get_attr(item_public, status, which_attr, valist)	/*ARGSUSED*/
    Panel_item      item_public;
    int            *status;
    register Attr_attribute which_attr;
    va_list         valist;
{
    register Item_info *ip = ITEM_PRIVATE(item_public);

    switch (which_attr) {
      case PANEL_NEXT_ITEM:
	return (Xv_opaque) (ip->next ? ITEM_PUBLIC(ip->next) : 0);

      case PANEL_ITEM_OWNER:
	return (Xv_opaque) ip->owner;

      case PANEL_ITEM_CLASS:
	return (Xv_opaque) (ip->item_type);

      case PANEL_VALUE_X:
	return (Xv_opaque) ip->value_rect.r_left;

      case PANEL_ITEM_X_GAP:
	return (Xv_opaque) ip->x_gap;

      case PANEL_ITEM_RECT:
      case XV_RECT:
	return (Xv_opaque) & ip->rect;

#ifdef OW_I18N
      case PANEL_LABEL_STRING_WCS:
        if (is_string(&ip->label))
            return (Xv_opaque) image_string_wc(&ip->label);
        else
            return (Xv_opaque) NULL;
 
      case PANEL_LABEL_STRING:
        if (is_string(&ip->label))
            return (Xv_opaque)_xv_wcstombsdup(image_string_wc(&ip->label));
        else
            return (Xv_opaque) NULL;
#else
      case PANEL_LABEL_STRING:
	if (is_string(&ip->label))
	    return (Xv_opaque) image_string(&ip->label);
	else
	    return (Xv_opaque) NULL;
#endif /* OW_I18N */

      case PANEL_LABEL_WIDTH:
	if (ip->label_width)
	    return (Xv_opaque) ip->label_width;
	if (ip->item_type == PANEL_BUTTON_ITEM)
	    return (Xv_opaque) ip->label_rect.r_width -
		2*ButtonEndcap_Width(ip->panel->ginfo) -
		(ip->menu ? 2*MenuMark_Width(ip->panel->ginfo) : 0);
	else
	    return (Xv_opaque) ip->label_rect.r_width;

      case PANEL_VALUE_FONT:
	return (Xv_opaque) ip->value_font;

      case PANEL_VALUE_Y:
	return (Xv_opaque) ip->value_rect.r_top;

      case PANEL_CHILD_CARET_ITEM:
	return (Xv_opaque) ip->child_kbd_focus_item;

      case PANEL_ITEM_NTH_WINDOW:
	/* If the item has any embedded windows, the nth window handle
	 * is returned by the item-specific get routine.
	 */
	return (Xv_opaque) NULL;

      case PANEL_ITEM_NWINDOWS:
	/* If the item has any embedded windows, the number of windows
	 * is returned by the item-specific get routine.
	 */
	return (Xv_opaque) 0;

      case PANEL_LABEL_FONT:
	if (is_string(&ip->label))
	    return (Xv_opaque) image_font(&ip->label);
	else
	    return (Xv_opaque) NULL;

      case PANEL_LABEL_BOLD:
	if (is_string(&ip->label))
	    return (Xv_opaque) image_bold(&ip->label);
	else
	    return (Xv_opaque) NULL;

      case PANEL_LABEL_BOXED:
	return (Xv_opaque) image_boxed(&ip->label);

      case PANEL_LABEL_INVERTED:
	return (Xv_opaque) image_inverted(&ip->label);

      case PANEL_LABEL_IMAGE:
	if (is_svrim(&ip->label))
	    return (Xv_opaque) image_svrim(&ip->label);
	else
	    return (Xv_opaque) NULL;

      case PANEL_LABEL_X:
	return (Xv_opaque) ip->label_rect.r_left;

      case PANEL_LABEL_Y:
	return (Xv_opaque) ip->label_rect.r_top;

	/* these attrs must still be supported in addition to XV_X */
	/* and XV_Y because they can be used on the panel. When used */
	/* on the panel PANEL_ITEM_X != XV_X on the panel */
      case XV_X:
      case PANEL_ITEM_X:
	return (Xv_opaque) ip->rect.r_left;

      case XV_Y:
      case PANEL_ITEM_Y:
	return (Xv_opaque) ip->rect.r_top;

      case PANEL_ITEM_Y_GAP:
	return (Xv_opaque) ip->y_gap;

      case XV_WIDTH:
	return (Xv_opaque) ip->rect.r_width;

      case XV_HEIGHT:
	return (Xv_opaque) ip->rect.r_height;

      case XV_SHOW:
	return (Xv_opaque) ! hidden(ip);

      case PANEL_NOTIFY_PROC:
	return (Xv_opaque) ip->notify;

#ifdef OW_I18N
      case PANEL_NOTIFY_PROC_WCS:
        return (Xv_opaque) ip->notify_wc;
#endif /* OW_I18N */

      case PANEL_NOTIFY_STATUS:
	return (Xv_opaque) ip->notify_status;

      case PANEL_EVENT_PROC:
	return (Xv_opaque) ip->ops.panel_op_handle_event;

      case PANEL_LAYOUT:
	return (Xv_opaque) ip->layout;

#ifdef SUNVIEW1
      case PANEL_MENU_TITLE_STRING:
	return is_string(&ip->menu_title) ?
	    (Xv_opaque) image_string(&ip->menu_title) : NULL;

      case PANEL_MENU_TITLE_IMAGE:
	return is_svrim(&ip->menu_title) ?
	    (Xv_opaque) image_svrim(&ip->menu_title) : NULL;

      case PANEL_MENU_TITLE_FONT:
	return is_string(&ip->menu_title) ?
	    (Xv_opaque) image_font(&ip->menu_title) : NULL;

      case PANEL_TYPE_IMAGE:
	return (Xv_opaque) ip->menu_type_pr;

      case PANEL_MENU_CHOICE_STRINGS:
      case PANEL_MENU_CHOICE_IMAGES:
      case PANEL_MENU_CHOICE_FONTS:
      case PANEL_MENU_CHOICE_VALUES:
	return NULL;
#endif

      case PANEL_ACCEPT_KEYSTROKE:
	return (Xv_opaque) wants_key(ip);

      case PANEL_CLIENT_DATA:
	return ip->client_data;

      case PANEL_ITEM_COLOR:
	return (Xv_opaque) ip->color_index;

      case XV_OWNER:
	return ((Xv_opaque) (ip->panel != NULL) ? PANEL_PUBLIC(ip->panel) :
		XV_ZERO);

      case PANEL_BUSY:
	return (Xv_opaque) busy(ip);

      case PANEL_INACTIVE:
	return (Xv_opaque) inactive(ip);

      case PANEL_ITEM_MENU:
	return ip->menu;

      case PANEL_MENU_ITEM:
	return (Xv_opaque) is_menu_item(ip);

      case PANEL_OPS_VECTOR:
	return (Xv_opaque) &ip->ops;

      case PANEL_ITEM_CREATED:
	return (Xv_opaque) created(ip);

      case PANEL_ITEM_DEAF:
	return (Xv_opaque) deaf(ip);

      case PANEL_ITEM_WANTS_ADJUST:
	return (Xv_opaque) wants_adjust(ip);

      case PANEL_ITEM_WANTS_ISO:
	return (Xv_opaque) wants_iso(ip);

      case PANEL_GINFO:
	return (Xv_opaque) image_ginfo(&ip->label);

      case PANEL_ITEM_LABEL_RECT:
	return (Xv_opaque) &ip->label_rect;

      case PANEL_ITEM_VALUE_RECT:
	return (Xv_opaque) &ip->value_rect;

    case PANEL_POST_EVENTS:
	return (Xv_opaque) post_events(ip);

      default:
	if (xv_check_bad_attr(&xv_panel_item_pkg, which_attr) == XV_ERROR) {
	    *status = XV_ERROR;
	}
	return (Xv_opaque) NULL;
    }
}
