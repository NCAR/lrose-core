#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)panel.c 20.84 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE 
 *	file for terms of the license.
 */

#include <X11/X.h>
#include <xview_private/panel_impl.h>
#include <xview_private/draw_impl.h>
#include <xview/font.h>
#include <xview/defaults.h>
#include <xview/notify.h>
#include <xview/server.h>

#ifdef OW_I18N
Xv_private void _xv_status_start(), _xv_status_done(), _xv_status_draw();
#endif /* OW_I18N */
Xv_private void	win_set_no_focus();
Pkg_private int panel_init();
Pkg_private int panel_view_init();
Pkg_private int panel_destroy();

static int      panel_layout();

static
  panel_unregister_view(Panel_info *panel, Xv_Window view);

#ifndef __linux
Xv_private_data Defaults_pairs xv_kbd_cmds_value_pairs[4];
#else
extern Defaults_pairs xv_kbd_cmds_value_pairs[4];
#endif

/* default timer value */
static struct itimerval PANEL_TIMER = {0, 500000, 0, 500000};

static Defaults_pairs shiftmasks[] = {
    "Shift", SHIFTMASK,
    "Ctrl", CTRLMASK,
    "Meta", META_SHIFT_MASK,
    "Alt", ALTMASK,
    NULL, CTRLMASK,	/* default value */
};

Attr_attribute  panel_context_key;


/*ARGSUSED*/
Pkg_private int
panel_init(parent, panel_public, avlist)
    Xv_Window       parent, panel_public;
    Attr_avlist     avlist;
{
    Xv_Drawable_info *info;
    Panel_info	   *panel;
    Xv_panel       *panel_object = (Xv_panel *) panel_public;
    Xv_Server	    server;

#ifdef OW_I18N
    Frame	    frame;
#endif /* OW_I18N */

    DRAWABLE_INFO_MACRO(panel_public, info);
    server = XV_SERVER_FROM_WINDOW(panel_public);

    /* initial context key if necessary */
    if (panel_context_key == (Attr_attribute) 0) {
	panel_context_key = xv_unique_key();
    }
    panel = xv_alloc(Panel_info);

    /* link to object */
    panel_object->private_data = (Xv_opaque) panel;
    panel->public_self = panel_public;

#ifdef OW_I18N
    panel->atom.compound_text =
	    (Atom) xv_get(server, SERVER_ATOM, "COMPOUND_TEXT");
    panel->atom.length_chars =
	    (Atom) xv_get(server, SERVER_ATOM, "LENGTH_CHARS");
#endif /*OW_I18N*/
    panel->atom.delete = (Atom) xv_get(server, SERVER_ATOM, "DELETE");
    panel->atom.length = (Atom) xv_get(server, SERVER_ATOM, "LENGTH");
    panel->atom.null = (Atom) xv_get(server, SERVER_ATOM, "NULL");
    panel->atom.secondary = (Atom) xv_get(server, SERVER_ATOM, "SECONDARY");
    panel->atom.selection_end = (Atom) xv_get(server,
	SERVER_ATOM, "_SUN_SELECTION_END");
    panel->atom.seln_yield = (Atom) xv_get(server,
	SERVER_ATOM, "_SUN_SELN_YIELD"); /* for SunView1 selection clients */
    panel->caret = XV_ZERO;
    panel->caret_on = FALSE;
    panel->current_col_x = PANEL_ITEM_X_START;
    panel->drag_threshold = defaults_get_integer("openWindows.dragThreshold",
	"OpenWindows.DragThreshold", 5);
    panel->duplicate_shiftmask =
	defaults_get_enum("openWindows.duplicateKey",
			  "OpenWindows.DuplicateKey", shiftmasks);
    panel->extra_height = 1;
    panel->extra_width = 1;
    panel->event_proc = NULL;
    panel->h_margin = 4;
    panel->item_x = PANEL_ITEM_X_START;
    panel->item_x_offset = ITEM_X_GAP;
    panel->item_y = PANEL_ITEM_Y_START;
    panel->item_y_offset = ITEM_Y_GAP;
    panel->flags = IS_PANEL;
    panel->layout = PANEL_HORIZONTAL;
    panel->ops.panel_op_handle_event = panel_default_handle_event;
    panel->ops.panel_op_paint = panel_display;
    panel->repaint = PANEL_CLEAR;
    if (defaults_get_enum("openWindows.keyboardCommands",
			  "OpenWindows.KeyboardCommands",
			  xv_kbd_cmds_value_pairs) == KBD_CMDS_FULL)
	panel->status.mouseless = TRUE;
    if (defaults_get_boolean("OpenWindows.SelectDisplaysMenu",
	"OpenWindows.SelectDisplaysMenu", FALSE))
	/* SELECT => Display menu default */
	panel->status.select_displays_menu = TRUE;
    if (xv_depth(info) > 1)
	panel->status.three_d = defaults_get_boolean("OpenWindows.3DLook.Color",
	    "OpenWindows.3DLook.Color", TRUE);
    else
#ifdef MONO3D
	panel->status.three_d =
	    defaults_get_boolean("OpenWindows.3DLook.Monochrome",
	    			 "OpenWindows.3DLook.Monochrome", FALSE);
#else
	panel->status.three_d = FALSE;
#endif
    panel->timer_full = PANEL_TIMER;
    panel->v_margin = 4;
    panel->multiclick_timeout = 100 *
	defaults_get_integer_check("openWindows.multiClickTimeout",
				   "OpenWindows.MultiClickTimeout", 4, 2, 10);

    panel->layout_proc = (int (*) ()) xv_get(panel_public, WIN_LAYOUT_PROC);

    (void) xv_set(panel_public,
		  WIN_TOP_MARGIN, PANEL_ITEM_Y_START,
		  WIN_LEFT_MARGIN, PANEL_ITEM_X_START,
		  WIN_ROW_GAP, ITEM_Y_GAP,
		  WIN_LAYOUT_PROC, panel_layout,
		  XV_HELP_DATA, "xview:panel",
		  NULL);

    if (xv_get(panel_public, XV_IS_SUBTYPE_OF, CANVAS)) {
	(void) xv_set(panel_public,
		      WIN_NOTIFY_SAFE_EVENT_PROC, panel_notify_panel_event,
		      WIN_NOTIFY_IMMEDIATE_EVENT_PROC, panel_notify_panel_event,
		      CANVAS_AUTO_EXPAND, TRUE,
		      CANVAS_AUTO_SHRINK, TRUE,
		      CANVAS_FIXED_IMAGE, FALSE,
		      CANVAS_REPAINT_PROC, panel_redisplay,
		      CANVAS_RETAINED, FALSE,
		      OPENWIN_SHOW_BORDERS, FALSE,
		      XV_FOCUS_RANK, XV_FOCUS_SECONDARY,
		      NULL);
	win_set_no_focus(panel_public, TRUE);	/* panel sets own focus */
    } else
	panel_view_init(panel_public, XV_NULL, NULL);

#ifdef OW_I18N

    frame = xv_get(panel_public, WIN_FRAME);

    /* Initialize the panel preedit callback structs */
    xv_set(panel_public, 
	   WIN_IC_PREEDIT_START, (XIMProc)panel_text_start, 
		(XPointer)panel_public,
	   WIN_IC_PREEDIT_DRAW, (XIMProc)panel_text_draw, 
		(XPointer)panel_public,
	   WIN_IC_PREEDIT_DONE, (XIMProc)panel_text_done, 
		(XPointer)panel_public, 
	   WIN_IC_STATUS_START, (XIMProc)_xv_status_start,
		(XPointer)frame,
	   WIN_IC_STATUS_DRAW, (XIMProc)_xv_status_draw,
		(XPointer)frame,
	   WIN_IC_STATUS_DONE, (XIMProc)_xv_status_done,
		(XPointer)frame, NULL);

    /* allocate and initialize space for caching
     * preedit information
     */
    panel->preedit = (XIMPreeditDrawCallbackStruct *) 
		xv_alloc(XIMPreeditDrawCallbackStruct);
    panel->preedit->text = (XIMText *) xv_alloc(XIMText);
    panel->preedit->text->encoding_is_wchar = 1;
    panel->preedit->text->string.wide_char = (wchar_t *) 
		xv_alloc(wchar_t);
    panel->preedit->text->string.wide_char[0] = NULL;

    /* Need to allocate feedback array??

    panel->preedit->text->feedback = (XIMFeedback *)
		xv_alloc(XIMFeedback);
     */
#endif /* OW_I18N */

    return XV_OK;
}


Pkg_private int
panel_destroy(panel_public, status)
    Panel           panel_public;
    Destroy_status  status;
{
    Xv_Window	    focus_win;
    Frame	    frame;
    Panel_item      item;
    Panel_info     *panel = PANEL_PRIVATE(panel_public);
    Panel_paint_window *pw;
    int		    rank;

    if (status == DESTROY_PROCESS_DEATH)
	panel->status.destroying = TRUE;
    else if (status == DESTROY_CLEANUP) {
	/* unlink layout procs */
	xv_set(panel_public,
	       WIN_LAYOUT_PROC, panel->layout_proc,
	       NULL);
	panel_itimer_set(panel_public, NOTIFY_NO_ITIMER);
	panel->status.destroying = TRUE;
    }
    PANEL_EACH_ITEM(panel_public, item)
	if (xv_destroy_status(item, status) != XV_OK)
	    return XV_ERROR;
    PANEL_END_EACH

    frame = xv_get(panel_public, WIN_FRAME);
    focus_win = xv_get(frame, FRAME_FOCUS_WIN);
    if (xv_get(focus_win, WIN_PARENT) == panel_public) {
	xv_set(frame, FRAME_NEXT_PANE, NULL);
	xv_set(focus_win, WIN_PARENT, frame, NULL);
    }

    if (status == DESTROY_CLEANUP) {

	/* Free storage used for selections */
	for (rank = 0; rank < NBR_PANEL_SELECTIONS; rank++) {
#ifndef OW_I18N
	    if (panel->sel_item[rank])
		xv_destroy(panel->sel_item[rank]);
#endif
	    if (panel->sel_owner[rank])
		xv_destroy(panel->sel_owner[rank]);
	}
	if (panel->sel_req)
	    xv_destroy(panel->sel_req);
#ifdef OW_I18N
        if (panel->clipboard.storage != NULL)
            xv_free(panel->clipboard.storage);
#endif

	/* Free storage for each paint window */
	while (panel->paint_window != NULL) {
	    pw = panel->paint_window->next;
	    xv_free(panel->paint_window);
	    panel->paint_window = pw;
	}

#ifdef OW_I18N
        /*  Free storage used for preedit
         *  and preedit callbacks
         */

	if (panel->preedit_own_by_others != TRUE && panel->preedit) {
	    if (panel->preedit->text->string.wide_char)
                xv_free(panel->preedit->text->string.wide_char);
	    if (panel->preedit->text)
	        xv_free(panel->preedit->text);
	    /* Need to free feedback array if allocated??
	    if (panel->preedit->text->feedback)
	        xv_free(panel->preedit->text->feedback);
	    */

	    xv_free(panel->preedit);
	}
#endif /* OW_I18N */

	xv_free(panel);
    }
    return XV_OK;
}


static
panel_layout(panel_public, child, op, d1, d2, d3, d4, d5)
    Panel           panel_public;
    Xv_Window       child;
    Window_layout_op op;
/* Alpha compatibility, mbuck@debian.org, FIXME: I don't understand this */
#if defined(__alpha)
    unsigned long   d1, d2, d3, d4, d5;
#else
    int             d1, d2, d3, d4, d5;
#endif
{
    Panel_info     *panel = PANEL_PRIVATE(panel_public);

    switch (op) {
      case WIN_DESTROY:
	panel_unregister_view(panel, child);
	break;
      case WIN_CREATE:
	if ((int) xv_get(child, XV_IS_SUBTYPE_OF, PANEL_VIEW) == TRUE) {
	    panel_register_view(panel, child);
	}
      default:
	break;
    }

    if (panel->layout_proc != NULL) {
	return (panel->layout_proc(panel_public, child, op, d1, d2, d3, d4, d5));
    } else {
	return TRUE;
    }
}


Pkg_private void
panel_register_view(panel, view)
    Panel_info     *panel;
    Xv_Window       view;
{
    Panel_paint_window	   *paint_window_data;
    Xv_Window		    pw;
    Panel_paint_window	   *ppw;

    /* Fill in paint window data */
    pw = view ? xv_get(view, CANVAS_VIEW_PAINT_WINDOW) : panel->public_self;
    paint_window_data = xv_alloc(Panel_paint_window);
    paint_window_data->pw = pw;
    paint_window_data->view = view;

    /* Append to end of paint window data list */
    if (panel->paint_window) {
	for (ppw = panel->paint_window; ppw->next; ppw = ppw->next);
	ppw->next = paint_window_data;
    } else
	panel->paint_window = paint_window_data;

    /* Don't set focus if there are no keyboard focus items yet */
    win_set_no_focus(pw, panel->kbd_focus_item == NULL);

    (void) xv_set(pw,
		  XV_KEY_DATA, panel_context_key, panel,
		  NULL);
}

static
panel_unregister_view(Panel_info *panel, Xv_Window view)
{
    Panel_paint_window *previous = NULL;
    Panel_paint_window *pw_data;

    for (pw_data = panel->paint_window;
	 pw_data != NULL;
	 pw_data = pw_data->next) {
	if (pw_data->view == view) {
	    if (previous != NULL) {
		previous->next = pw_data->next;
	    } else {
		panel->paint_window = pw_data->next;
	    }
	    xv_free(pw_data);
	    break;
	} else
	    previous = pw_data;
    }

    return (XV_OK);
}
