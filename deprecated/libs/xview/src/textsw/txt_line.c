#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)txt_line.c 1.26 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * Text line selection popup frame creation and support.
 */

#include <xview_private/primal.h>
#include <xview_private/txt_impl.h>
#include <xview_private/ev_impl.h>
#include <sys/time.h>
#include <signal.h>
#include <xview/notice.h>
#include <xview/frame.h>
#include <xview/panel.h>
#include <xview/textsw.h>
#include <xview/openmenu.h>
#include <xview/wmgr.h>
#include <xview/pixwin.h>
#include <xview/win_struct.h>
#include <xview/win_screen.h>



#define		MAX_DISPLAY_LENGTH      22
#define   	MAX_STR_LENGTH		8

#define HELP_INFO(s) XV_HELP_DATA, s,

/* for select line number */
typedef enum {
    SEL_LINE_ITEM = 0,
    SEL_LINE_NUMBER_ITEM = 1,
}               Sel_line_panel_item_enum;

Pkg_private Panel_item sel_line_panel_items[];

Pkg_private Textsw_view_handle text_view_frm_p_itm();
Pkg_private Xv_Window frame_from_panel_item();

/*ARGSUSED*/
static int
do_sel_line_proc(folio, ie)
    Textsw_folio    folio;
    Event          *ie;
{
    Es_index        prev;
    CHAR            buf[10];
    unsigned        buf_fill_len;
    char           *line_number;
    int             line_no;
    Es_index        first, last_plus_one;
    Textsw_view_handle view = VIEW_FROM_FOLIO_OR_VIEW(folio);
    Frame           popup_frame;

    line_number = (char *) xv_get(
	sel_line_panel_items[(int) SEL_LINE_NUMBER_ITEM], PANEL_VALUE);
    line_no = atoi(line_number);

    if (line_no == 0) {
	window_bell(WINDOW_FROM_VIEW(view));
	return TRUE;
    } else {
	buf[0] = '\n';
	buf_fill_len = 1;
	if (line_no == 1) {
	    prev = 0;
	} else {
	    ev_find_in_esh(folio->views->esh, buf, buf_fill_len,
		       (Es_index) 0, (u_int) line_no - 1, 0, &first, &prev);
	    if (first == ES_CANNOT_SET) {
		window_bell(WINDOW_FROM_VIEW(view));
		return TRUE;
	    }
	}
	ev_find_in_esh(folio->views->esh, buf, buf_fill_len,
		       prev, 1, 0, &first, &last_plus_one);
	if (first == ES_CANNOT_SET) {
	    window_bell(WINDOW_FROM_VIEW(view));
	    return TRUE;
	}
	textsw_possibly_normalize_and_set_selection(VIEW_REP_TO_ABS(view), prev,
					     last_plus_one, EV_SEL_PRIMARY);
	(void) textsw_set_insert(folio, last_plus_one);
	popup_frame =
	    frame_from_panel_item(sel_line_panel_items[(int) SEL_LINE_ITEM]);
	(void) xv_set(popup_frame, XV_SHOW, FALSE, NULL);
	return FALSE;
    }
}


static void
sel_line_cmd_proc(item, event)
    Panel_item      item;
    Event          *event;
{
    Textsw_view_handle view = text_view_frm_p_itm(item);
    Textsw_folio    folio = FOLIO_FOR_VIEW(view);
    int             error;

    if (item == sel_line_panel_items[(int) SEL_LINE_ITEM] ||
	item == sel_line_panel_items[(int) SEL_LINE_NUMBER_ITEM])
	error = do_sel_line_proc(folio, event);
    if (error) {
	xv_set(item, PANEL_NOTIFY_STATUS, XV_ERROR, NULL);
    }
}

/* This creates all of the panel_items */
static void
create_sel_line_items(panel, view)
    Panel           panel;
    Textsw_view_handle view;
{

    CHAR            line_number[MAX_STR_LENGTH];
    Es_index        dummy;

    line_number[0] = XV_ZERO;
    (void) textsw_get_selection(view, &dummy, &dummy, line_number, MAX_STR_LENGTH);

    sel_line_panel_items[(int) SEL_LINE_NUMBER_ITEM] =
	panel_create_item(panel, PANEL_TEXT,
			  PANEL_LABEL_X, ATTR_COL(0),
			  PANEL_LABEL_Y, ATTR_ROW(0),
			  PANEL_VALUE_DISPLAY_LENGTH, MAX_DISPLAY_LENGTH,
			  PANEL_VALUE_STORED_LENGTH, MAX_STR_LENGTH,
#ifdef OW_I18N
			  PANEL_VALUE_WCS, line_number,
#else
			  PANEL_VALUE, line_number,
#endif
			  PANEL_LABEL_STRING, 
				XV_MSG("Line Number:"),
			  PANEL_NOTIFY_LEVEL, PANEL_SPECIFIED,
			  PANEL_NOTIFY_STRING, "\n\r",
			  PANEL_NOTIFY_PROC, sel_line_cmd_proc,
			  HELP_INFO("textsw:linenumber")
			  NULL);

    xv_set(panel, PANEL_CARET_ITEM,
	   sel_line_panel_items[(int) SEL_LINE_NUMBER_ITEM], NULL);

    sel_line_panel_items[(int) SEL_LINE_ITEM] =
	panel_create_item(panel,
			  PANEL_BUTTON,
			  PANEL_LABEL_X, ATTR_COL(10),
			  PANEL_LABEL_Y, ATTR_ROW(1),
			  PANEL_LABEL_STRING, 
				XV_MSG("Select Line at Number"),
			  PANEL_NOTIFY_PROC, sel_line_cmd_proc,
			  HELP_INFO("textsw:selectline")
			  NULL);
    (void) xv_set(panel,
	      PANEL_DEFAULT_ITEM, sel_line_panel_items[(int) SEL_LINE_ITEM],
		  NULL);
}
Pkg_private          Panel
textsw_create_sel_line_panel(frame, view)
    Frame           frame;
    Textsw_view_handle view;
{
    Panel           panel;

    panel = (Panel) xv_get(frame, FRAME_CMD_PANEL,
			   XV_HELP_DATA, "textsw:sellinepanel",
			   NULL);
    (void) create_sel_line_items(panel, view);

    return (panel);
}
