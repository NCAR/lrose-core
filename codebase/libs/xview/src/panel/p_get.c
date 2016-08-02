#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)p_get.c 20.38 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */


#include <xview_private/panel_impl.h>
#include <xview/font.h>
#include <xview/scrollbar.h>

static int      shrink_to_fit();

static
  panel_shrink_margin(register Panel_info *panel);

#define MAX_NEGATIVE_SHRINK 2000
#define SHRINK_MARGIN       4


/*ARGSUSED*/
Pkg_private     Xv_opaque
panel_get_attr(panel_public, status, attr, valist)
    Panel           panel_public;
    int            *status;
    register Attr_attribute attr;
    va_list         valist;
{
    register Panel_info *panel = PANEL_PRIVATE(panel_public);
    int             arg;

    switch (attr) {
      case CANVAS_NTH_PAINT_WINDOW:
	if (panel->paint_window && !panel->paint_window->view)
	    return (Xv_opaque) panel_public;
	else {
	    /* Scrollable Panel: let Canvas package return result */
	    *status = XV_ERROR;
	    return (Xv_opaque) 0;
	}
      case OPENWIN_VIEW_CLASS:
	return (Xv_opaque) PANEL_VIEW;
      case PANEL_CLIENT_DATA:
	return panel->client_data;

	/* laf */
      case PANEL_BOLD_FONT:
	return (Xv_opaque) panel->bold_font;

      case PANEL_LABEL_INVERTED:
	return (Xv_opaque) label_inverted_flag(panel);

      case PANEL_BLINK_CARET:
	return (Xv_opaque) panel->status.blinking ? TRUE : FALSE;

      case PANEL_CARET_ITEM:
	return (Xv_opaque) (panel->kbd_focus_item ?
	    ITEM_PUBLIC(panel->kbd_focus_item) : 0);

      case PANEL_FIRST_ITEM:
	return (Xv_opaque) (panel->items ? ITEM_PUBLIC(panel->items) : 0);

      case PANEL_FIRST_PAINT_WINDOW:
	return (Xv_opaque) panel->paint_window;

      case PANEL_EVENT_PROC:
	return (Xv_opaque) panel->event_proc;

      case PANEL_BACKGROUND_PROC:
	return (Xv_opaque) panel->ops.panel_op_handle_event;

      case PANEL_REPAINT_PROC:
	return (Xv_opaque) panel->repaint_proc;

      case PANEL_ITEM_X:
	return (Xv_opaque) panel->item_x;

      case PANEL_ITEM_Y:
	return (Xv_opaque) panel->item_y;

      case PANEL_ITEM_X_GAP:
	return (Xv_opaque) panel->item_x_offset;

      case PANEL_ITEM_Y_GAP:
	return (Xv_opaque) panel->item_y_offset;

      case PANEL_EXTRA_PAINT_WIDTH:
	return (Xv_opaque) panel->extra_width;

      case PANEL_EXTRA_PAINT_HEIGHT:
	return (Xv_opaque) panel->extra_height;

      case PANEL_LAYOUT:
	return (Xv_opaque) panel->layout;

      case PANEL_ACCEPT_KEYSTROKE:
	return (Xv_opaque) wants_key(panel);

      case PANEL_DEFAULT_ITEM:
	return (panel->default_item ? panel->default_item :
		panel->items ? ITEM_PUBLIC(panel->items) : 0);

    case PANEL_BORDER:
	return (Xv_opaque) panel->show_border;

      case PANEL_CURRENT_ITEM:
	return panel->current ? ITEM_PUBLIC(panel->current) : 0;

      case PANEL_FOCUS_PW:
	return panel->focus_pw;
    
      case PANEL_GINFO:
	return (Xv_opaque) panel->ginfo;
    
      case PANEL_ITEM_X_POSITION:
	return (Xv_opaque) panel->item_x;

      case PANEL_ITEM_Y_POSITION:
	return (Xv_opaque) panel->item_y;

      case PANEL_PRIMARY_FOCUS_ITEM:
	return panel->primary_focus_item ?
	    ITEM_PUBLIC(panel->primary_focus_item) : 0;

      case PANEL_STATUS:
	return (Xv_opaque) &panel->status;

      case PANEL_NO_REDISPLAY_ITEM:
	return (Xv_opaque) panel->no_redisplay_item;

      case WIN_FIT_WIDTH:
        arg = va_arg(valist, int);
	return (Xv_opaque) shrink_to_fit(panel, TRUE,
	    arg ? arg : panel_shrink_margin(panel));

      case WIN_FIT_HEIGHT:
        arg = va_arg(valist, int);
	return (Xv_opaque) shrink_to_fit(panel, FALSE,
	    arg ? arg : panel_shrink_margin(panel));

      case WIN_TYPE:		/* SunView1.X compatibility */
	return (Xv_opaque) PANEL_TYPE;

      default:
	xv_check_bad_attr(&xv_panel_pkg, attr);
	*status = XV_ERROR;
	return (Xv_opaque) 0;

    }
}


static int
shrink_to_fit(panel, do_width, excess)
    register Panel_info *panel;
    int             do_width;
    register int    excess;
{
    register Item_info *ip;
    register int    low_point = 0;
    register int    right_point = 0;
    int             new_size;
    Scrollbar       sb;

    if (!do_width) {
	for (ip = panel->items; ip; ip = ip->next)
	    low_point = MAX(low_point, ip->rect.r_top + ip->rect.r_height);
	new_size = low_point + excess + panel->extra_height;
	sb = xv_get(PANEL_PUBLIC(panel), WIN_HORIZONTAL_SCROLLBAR);
	if (sb)
	    new_size += (int) xv_get(sb, XV_HEIGHT);
    } else {
	for (ip = panel->items; ip; ip = ip->next)
	    right_point = MAX(right_point, ip->rect.r_left + ip->rect.r_width);
	new_size = right_point + excess + panel->extra_width;
	sb = xv_get(PANEL_PUBLIC(panel), WIN_VERTICAL_SCROLLBAR);
	if (sb)
	    new_size += (int) xv_get(sb, XV_WIDTH);
    }
    return new_size;
}

static
  panel_shrink_margin(register Panel_info *panel)
{
    Xv_opaque	    font;
    int		    font_size;

    font = xv_get(PANEL_PUBLIC(panel), XV_FONT);
    font_size = (int) xv_get(font, FONT_SIZE);
    if (font_size == FONT_NO_SIZE)
	font_size = (int) xv_get(font, FONT_DEFAULT_CHAR_HEIGHT);
    if (font_size <= 10)
	return 8;
    else if (font_size <= 12)
	return 12;
    else if (font_size <= 14)
	return 16;
    else if (font_size <= 16)
	return 20;
    else if (font_size <= 19)
	return 23;
    else if (font_size <= 24)
	return 28;
    else
	return 32;
}
