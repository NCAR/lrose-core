#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)p_txt.c 20.217 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE 
 *	file for terms of the license.
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <X11/X.h>
#include <xview_private/draw_impl.h>
#include <xview_private/panel_impl.h>
#include <xview/cursor.h>
#include <xview/defaults.h>
#include <xview/notice.h>
#include <xview/screen.h>
#include <xview/pixwin.h>
#include <xview/font.h>
#include <xview/sel_svc.h>

#ifdef OW_I18N
#include <stdlib.h>
#endif /* OW_I18N */

#ifdef OW_I18N
#if defined(SEL_DEBUG) && defined(DEBUG)
#define sdfprintf(a)	fprintf a
#else
#define sdfprintf(a)
#endif
#endif /* OW_I18N */

/* External procedures */
#ifdef OW_I18N
Xv_public struct pr_size  xv_pf_textwidth_wc();
Xv_public wchar_t	 _xv_null_string_wc[];
#else
Xv_public struct pr_size  xv_pf_textwidth();
#endif /*OW_I18N*/
Xv_private void	    screen_adjust_gc_color();
Xv_private void	    win_grab_quick_sel_keys();

/* XView functions */
Pkg_private int text_init();
Pkg_private Xv_opaque text_set_avlist();
Pkg_private Xv_opaque text_get_attr();
Pkg_private int text_destroy();

/* Panel Item Operations */
static void     text_handle_event();
static void     text_begin_preview();
static void     text_cancel_preview();
static void     text_accept_preview();
static void     text_accept_key();
static void	text_clear();
static void     text_paint();
static void     text_remove();
static void     text_restore();
static void	text_layout();
static void     text_accept_kbd_focus();
static void     text_yield_kbd_focus();
#ifdef OW_I18N
static void 	ml_panel_moded_interm();
#ifdef notdef
/* FIX_ME: Should be removed ? */
static void 	ml_panel_simple_display();
#endif
static void 	paint_value_and_interm();
#endif /* OW_I18N */

/* Local functions */
static int	char_position();
static void	draw_scroll_btn();
static void     horizontal_scroll();
#ifdef PAINT_BOX
static void	paint_box();
#endif /* PAINT_BOX */
static void     paint_caret();
static void     paint_text();
static void     paint_value();
static void     panel_find_word();
static void	panel_multiclick_handler();
static void     panel_select_line();
static void	text_add_selection();
static void	text_alarm();
static int	text_convert_proc();
static void	text_lose_proc();
static void	text_lose_rank();
static void	text_seln_dehighlight();
static void	text_seln_delete();
#ifdef OW_I18N
static void	text_seln_done_proc();
#endif
static void     text_seln_highlight();
static void	text_seln_init();
static void	text_set_clipboard();
#ifndef OW_I18N
static void	text_set_sel_data();
#endif
static void     update_caret_offset();
static void     update_text_rect();
static void     update_value();
static void     update_value_offset();
#ifdef OW_I18N
static int	wslen_in_byte();
#endif

static Panel_ops ops = {
    text_handle_event,			/* handle_event() */
    text_begin_preview,			/* begin_preview() */
    text_begin_preview,			/* update_preview() */
    text_cancel_preview,		/* cancel_preview() */
    text_accept_preview,		/* accept_preview() */
    NULL,				/* accept_menu() */
    text_accept_key,			/* accept_key() */
    text_clear,				/* clear() */
    text_paint,				/* paint() */
    NULL,				/* resize() */
    text_remove,			/* remove() */
    text_restore,			/* restore() */
    text_layout,			/* layout() */
    text_accept_kbd_focus,		/* accept_kbd_focus() */
    text_yield_kbd_focus,		/* yield_kbd_focus() */
    NULL				/* extension: reserved for future use */
};

/* Global data */
Xv_public char xv_iso_cancel;
Xv_public char xv_iso_next_element;

/* Local data */
static Panel_info *primary_seln_panel, *secondary_seln_panel;
static Rect     primary_seln_rect, secondary_seln_rect;
static int      primary_seln_first, secondary_seln_first;
static int      primary_seln_last, secondary_seln_last;
static short	delim_init = FALSE; /* delim_table initialized */
static char	delim_table[256];   /* TRUE= character is a word delimiter */
static Highlight seln_highlight = HL_NONE;
static char	no_buttons; /* Flag indicating that no space needs to be
			    * reserved for the scroll buttons.
			    */


/* ========================================================================= */

/* -------------------- XView Functions  -------------------- */
/*ARGSUSED*/
Pkg_private int
text_init(panel_public, item_public, avlist)
    Panel           panel_public;
    Panel_item      item_public;
    Attr_avlist     avlist;
{
    Text_info	   *dp;
    int		    chrht;
    char	   *delims;   /* printf formatted text.delimChars default */
    char	    delim_chars[256];	/* delimiter characters */
    int		    i;
    Item_info	   *ip = ITEM_PRIVATE(item_public);
    Xv_Drawable_info *info;
    Xv_panel_text  *item_object = (Xv_panel_text *) item_public;
    Panel_info	   *panel = PANEL_PRIVATE(panel_public);
    int		    pc_home_y;
    Xv_Window	    pw;	   /* paint window */
#ifdef OW_I18N
    XFontSetExtents *font_set_extents;
#else
    XFontStruct	   *x_font_info;
#endif /*OW_I18N*/

    dp = xv_alloc(Text_info);

    /* link to object */
    item_object->private_data = (Xv_opaque) dp;
    dp->public_self = item_public;

    if (!panel->sel_owner[PANEL_SEL_PRIMARY]) {
	text_seln_init(panel);
	PANEL_EACH_PAINT_WINDOW(panel, pw)
	    win_grab_quick_sel_keys(pw);
	PANEL_END_EACH_PAINT_WINDOW
    }

    ip->ops = ops;
    if (panel->event_proc)
	ip->ops.panel_op_handle_event = (void (*) ()) panel->event_proc;
    ip->item_type = PANEL_TEXT_ITEM;
    if (ip->notify == panel_nullproc)
	ip->notify = (int (*) ()) panel_text_notify;
    panel_set_bold_label_font(ip);

#ifdef OW_I18N
    /*  Default count for PANEL_VALUE_STORED_LENGTH is bytes */
    dp->flags &= ~STORED_LENGTH_WC;

    font_set_extents = XExtentsOfFontSet(panel->std_fontset_id);
    pc_home_y = font_set_extents->max_logical_extent.y;
#else
    x_font_info = (XFontStruct *)xv_get(ip->value_font, FONT_INFO);
    pc_home_y = -x_font_info->ascent;
#endif /* OW_I18N */
    if (pc_home_y < dp->font_home)
        dp->font_home = pc_home_y;

    dp->font_home = -dp->font_home;

    dp->display_length = 80;
    dp->display_width = panel_col_to_x(ip->value_font, dp->display_length);
    dp->flags |= UNDERLINED;
#ifdef OW_I18N
    dp->mask_wc = '\0';
#else
    dp->mask = '\0';
#endif /*OW_I18N*/
    dp->notify_level = PANEL_SPECIFIED;
    dp->scroll_btn_height = TextScrollButton_Height(panel->ginfo);
    dp->scroll_btn_width = TextScrollButton_Width(panel->ginfo) +
	SCROLL_BTN_GAP;
    dp->stored_length = 80;

#ifdef OW_I18N
    dp->terminators_wc = (wchar_t *)_xv_mbstowcsdup("\n\r\t");
#else
    dp->terminators = (char *) panel_strsave("\n\r\t");
#endif /*OW_I18N*/

#ifdef OW_I18N
    dp->undo_buffer_wc = (wchar_t *) xv_calloc(1, (u_int) (dp->stored_length + 1)*sizeof(wchar_t));
#else
    dp->undo_buffer = (char *) xv_calloc(1, (u_int) (dp->stored_length + 1));
#endif /*OW_I18N*/

    dp->undo_direction = INVALID;

#ifdef OW_I18N
    dp->value_wc = (wchar_t *) xv_calloc(1, (u_int) (dp->stored_length + 1)*sizeof (wchar_t));
    if (!dp->undo_buffer_wc || !dp->value_wc)
#else
    dp->value = (char *) xv_calloc(1, (u_int) (dp->stored_length + 1));
    if (!dp->undo_buffer || !dp->value)
#endif /*OW_I18N*/
	return XV_ERROR;

    ip->value_rect.r_width = dp->display_width;
    chrht = xv_get(ip->value_font, FONT_DEFAULT_CHAR_HEIGHT);
    ip->value_rect.r_height = MAX(chrht + BOX_Y, dp->scroll_btn_height);

    dp->dnd = xv_create(panel_public, DRAGDROP,
			SEL_CONVERT_PROC, text_convert_proc,
#ifdef OW_I18N
			SEL_DONE_PROC, text_seln_done_proc,
#endif
			XV_KEY_DATA, PANEL, panel_public,
			NULL);
#ifndef OW_I18N
    dp->dnd_item = xv_create(dp->dnd, SELECTION_ITEM, NULL);
#endif
    dp->drop_site = xv_create(panel_public, DROP_SITE_ITEM,
			      DROP_SITE_REGION, &ip->value_rect,
			      NULL);

    ip->flags |= WANTS_KEY | WANTS_ISO | WANTS_ADJUST;

    /* If the pixmap used to save and restore the pixels underneath the
     * caret hasn't been created yet, then do so now.
     */
    if (panel->caret_bg_pixmap == XV_ZERO) {
	DRAWABLE_INFO_MACRO(panel_public, info);
	panel->caret_bg_pixmap = XCreatePixmap(xv_display(info),
	    xv_get(xv_get(xv_screen(info), XV_ROOT), XV_XID),
	    MAX(panel->active_caret_width, panel->inactive_caret_width),
	    MAX(panel->active_caret_height, panel->inactive_caret_height),
	    xv_depth(info));
    }

    /*
     * Initialize the word delimiter table
     */
    if (!delim_init) {
	delims = (char *) defaults_get_string("text.delimiterChars",
	    "Text.DelimiterChars", " \t,.:;?!\'\"`*/-+=(){}[]<>\\|~@#$%^&");
	/* Print the string into an array to parse the potential
	 * octal/special characters.
	 */
	sprintf(delim_chars, delims);
	/* Mark off the delimiters specified */
	for (i = 0; i < 256; i++)
	    delim_table[i] = FALSE;
	for (delims = delim_chars; *delims; delims++)
	    delim_table[*delims] = TRUE;
	delim_init = TRUE;
    }

    /* A Text Item is, by default, a First-Class (primary) focus client */
    xv_set(item_public,
	   PANEL_PAINT, PANEL_NONE,
	   XV_FOCUS_RANK, XV_FOCUS_PRIMARY,
	   NULL);

    /* The panel now contains (at least one) First-Class (primary)
     * focus client
     */
    xv_set(panel_public, XV_FOCUS_RANK, XV_FOCUS_PRIMARY, NULL);

    return XV_OK;
}


Pkg_private     Xv_opaque
text_set_avlist(item_public, avlist)
    Panel_item      item_public;
    register Attr_avlist avlist;
{
    int		    display_width_set = FALSE;
    Text_info	   *dp = TEXT_PRIVATE(item_public);
    Item_info	   *ip = ITEM_PRIVATE(item_public);
#ifdef OW_I18N
    wchar_t        *new_value = '\0';
#else
    char           *new_value = NULL;
#endif /* OW_I18N */
    short	    select_line = FALSE;
    short           value_rect_changed = FALSE;
    Panel_info     *panel = ip->panel;
    int		    pc_home_y;
    Xv_opaque       result;
    int		    retstatus;
    int             no_redisplay_item_state;
#ifdef OW_I18N
    char	    buf;
    XFontSetExtents *font_set_extents;
#else
    XFontStruct	   *x_font_info;
#endif /* OW_I18N */

    /* if a client has called panel_item_parent this item may not */
    /* have a parent -- do nothing in this case */
    if (panel == NULL) {
	return ((Xv_opaque) XV_ERROR);
    }

    /* XV_END_CREATE is not used here, so return. */
    if (*avlist == XV_END_CREATE)
	return XV_OK;

    /* Parse Panel Item Generic attributes before Text Field attributes.
     * Prevent panel_redisplay_item from being called in item_set_avlist.
     */
    no_redisplay_item_state = ip->panel->no_redisplay_item;
    ip->panel->no_redisplay_item = TRUE;
    result = xv_super_set_avlist(item_public, &xv_panel_text_pkg, avlist);
    if (!no_redisplay_item_state)
        ip->panel->no_redisplay_item = FALSE;
    if (result != XV_OK)
	return result;

    for (; *avlist; avlist = attr_next(avlist)) {
	switch (avlist[0]) {
#ifdef OW_I18N
	  case PANEL_ITEM_IC_ACTIVE:
	    if (avlist[1])
	       ip->flags |= IC_ACTIVE;
	    else
	       ip->flags &= ~IC_ACTIVE;
	    break;

	  case PANEL_VALUE:
	    if (ip->panel->preedit_item && 
	           *ip->panel->preedit->text->string.wide_char) {        
		panel_implicit_commit(ip->panel->preedit_item);
	    }
	    new_value = (wchar_t *) _xv_mbstowcsdup((char *)avlist[1]);
	    break;

	  case PANEL_VALUE_WCS:
	    if (ip->panel->preedit_item && 
	           *ip->panel->preedit->text->string.wide_char) {        
		panel_implicit_commit(ip->panel->preedit_item);
	    }
	    new_value = (wchar_t *) panel_strsave_wc((wchar_t *)avlist[1]);
	    break;
#else
	  case PANEL_VALUE:
	    new_value = (char *) avlist[1];
	    break;
#endif /*OW_I18N*/

	  case PANEL_VALUE_FONT:
#ifdef OW_I18N
            font_set_extents = 
		XExtentsOfFontSet(ip->value_fontset_id);
            pc_home_y = font_set_extents->max_logical_extent.y;
#else
	    x_font_info = (XFontStruct *) xv_get(ip->value_font, FONT_INFO);
	    pc_home_y = -x_font_info->ascent;
#endif /* OW_I18N */
	    if (pc_home_y < dp->font_home)
		dp->font_home = pc_home_y;
	    dp->font_home = -dp->font_home;
	    if (!display_width_set)
		dp->display_width = panel_col_to_x(ip->value_font,
						   dp->display_length);
	    value_rect_changed = TRUE;
	    break;

	  case PANEL_VALUE_UNDERLINED:
	    if (avlist[1]) {
		dp->flags |= UNDERLINED;
	    } else {
		dp->flags &= ~UNDERLINED;
	    }
	    break;
	    /* laf end */

	  case PANEL_NOTIFY_LEVEL:
	    dp->notify_level = (Panel_setting) avlist[1];
	    break;

#ifdef OW_I18N
	  case PANEL_NOTIFY_STRING:
	    if (dp->terminators)
		xv_free(dp->terminators);
	    if (dp->terminators_wc)
		xv_free(dp->terminators_wc);
	    dp->terminators_wc = (wchar_t *) _xv_mbstowcsdup((char *) avlist[1]);
	    break;

	  case PANEL_NOTIFY_STRING_WCS:
	    if (dp->terminators)
		xv_free(dp->terminators);
	    if (dp->terminators_wc)
		xv_free(dp->terminators_wc);
	    dp->terminators_wc = (wchar_t *) panel_strsave_wc((wchar_t *)avlist[1]);
	    break;
#else
	  case PANEL_NOTIFY_STRING:
	    if (dp->terminators)
		free(dp->terminators);
	    dp->terminators = (char *) panel_strsave(avlist[1]);
	    break;
#endif /*OW_I18N*/

#ifdef OW_I18N
	  case PANEL_VALUE_STORED_LENGTH:
	    dp->flags &= ~STORED_LENGTH_WC;
	    dp->stored_length = (int) avlist[1];
	    dp->undo_buffer_wc = (wchar_t *) realloc(dp->undo_buffer_wc,
		(u_int) (dp->stored_length + 1)*sizeof(wchar_t));
	    dp->value_wc = (wchar_t *) realloc(dp->value_wc,
		(u_int) (dp->stored_length + 1)*sizeof(wchar_t));
	    break;

	  case PANEL_VALUE_STORED_LENGTH_WCS:
	    dp->flags |= STORED_LENGTH_WC;
	    dp->stored_length = (int) avlist[1];
	    dp->undo_buffer_wc = (wchar_t *) realloc(dp->undo_buffer_wc,
		(u_int) (dp->stored_length + 1)*sizeof(wchar_t));
	    dp->value_wc = (wchar_t *) realloc(dp->value_wc,
		(u_int) (dp->stored_length + 1)*sizeof(wchar_t));
	    break;

#else
	  case PANEL_VALUE_STORED_LENGTH:
	    dp->stored_length = (int) avlist[1];
	    dp->undo_buffer = (char *) realloc(dp->undo_buffer,
		(u_int) (dp->stored_length + 1));
	    dp->value = (char *) realloc(dp->value,
		(u_int) (dp->stored_length + 1));
	    if(dp->display_length >= dp->stored_length) {
	        no_buttons = TRUE;
		value_rect_changed = TRUE;
	    }
	    else
	        no_buttons = FALSE;
	    break;

#endif /*OW_I18N*/

	  case PANEL_VALUE_DISPLAY_LENGTH:
	    dp->display_length = (int) avlist[1];
	    dp->display_width = panel_col_to_x(ip->value_font,
					       dp->display_length);

	    if(dp->display_length >= dp->stored_length)
	        no_buttons = TRUE;
            else 
                no_buttons = FALSE; 
	    display_width_set = TRUE;
	    value_rect_changed = TRUE;
	    break;

	  case PANEL_VALUE_DISPLAY_WIDTH:
	    dp->display_width = (int) avlist[1];
	    dp->display_length = panel_x_to_col(ip->value_font,
					       dp->display_width);
	    dp->display_width = panel_col_to_x(ip->value_font,
					       dp->display_length);

	    if(dp->display_length >= dp->stored_length)
	        no_buttons = TRUE;
            else 
                no_buttons = FALSE; 
	    display_width_set = TRUE;
	    value_rect_changed = TRUE;
	    break;

#ifdef OW_I18N
	  case PANEL_MASK_CHAR:
	    buf = (char) avlist[1];
	    mbtowc(&dp->mask_wc, &buf, MB_CUR_MAX);
	    ip->flags &= ~IC_ACTIVE;
	    break;

	  case PANEL_MASK_CHAR_WC:
	    dp->mask_wc = (wchar_t) avlist[1];
	    ip->flags &= ~IC_ACTIVE;
	    break;
#else
	  case PANEL_MASK_CHAR:
	    dp->mask = (char) avlist[1];
	    break;
#endif /* OW_I18N */

	  case PANEL_INACTIVE:
	    if (avlist[1] && panel->sel_holder[PANEL_SEL_PRIMARY] == ip) {
		/* Inactivating a text item:
		 * Lose the primary selection if the item owns it.
		 */
		xv_set(panel->sel_owner[PANEL_SEL_PRIMARY],
		       SEL_OWN, FALSE,
		       NULL);
	    }
	    break;

	  case PANEL_READ_ONLY:
	    if (avlist[1]) {
		dp->flags |= PTXT_READ_ONLY;
		ip->flags &= ~WANTS_KEY;
		if (panel->kbd_focus_item == ip) {
		    /*
		     * Text item had caret: move caret to next text item, if
		     * any
		     */
		    paint_caret(panel->kbd_focus_item, FALSE);
		    panel->kbd_focus_item = panel_next_kbd_focus(panel, TRUE);
		    if (panel->kbd_focus_item) {
			if (panel->kbd_focus_item->item_type ==
			    PANEL_TEXT_ITEM) {
			    paint_caret(panel->kbd_focus_item, TRUE);
			} else {
			    panel_accept_kbd_focus(panel);
			}
		    }
		    /* Remove primary selection from item, if any */
		    if (panel->sel_holder[PANEL_SEL_PRIMARY] == ip)
			xv_set(panel->sel_owner[PANEL_SEL_PRIMARY],
			       SEL_OWN, FALSE,
			       NULL);

		    /* Don't let item take drops while read-only! */
		    xv_set ( dp->drop_site,
			    DROP_SITE_DELETE_REGION,	&ip->value_rect,
			    NULL );
		}
	    } else {
		dp->flags &= ~PTXT_READ_ONLY;
		ip->flags |= WANTS_KEY;

		/* want to take drops again */
		xv_set ( dp->drop_site, 
			DROP_SITE_REGION,	&ip->value_rect,
			NULL );
	    }
	    break;

	  case PANEL_TEXT_SELECT_LINE:
	    select_line = TRUE;
	    break;

	  default:
	    break;
	}
    }

    if (new_value) {
#ifdef OW_I18N
	/* FIX ME: put a subroutine here instead??
	 * Check whether stored_length was specified by STORED_LENGTH,
	 * or STORED_LENGTH_WCS attribute:
	 *   1.  If STORED_LENGTH -
	 *	 convert wide char string to see how many bytes
	 *	 it consumes, and compare with dp->stored_length
	 *   2.  If STORED_LENGTH_WCS -
	 *	 strictly a character comparison
	 */
	 if ((dp->flags & STORED_LENGTH_WC) == 0) {

	    register wchar_t	        *n, *v;
	    char			 p[MB_LEN_MAX + 1];
	    register int		 j, nbytes;

	    v = dp->value_wc;
	    n = new_value;
	    for (nbytes = dp->stored_length; nbytes > 0 && *n; n++) {
		if ((j = wctomb(p, *n)) < 0)
		    break;
		nbytes -= j;
		if (nbytes >= 0)
		     *v++ = *n;
	    }
	    *v = '\0';
	} else {
	    (void) wsncpy(dp->value_wc, new_value, dp->stored_length);
	}
	xv_free(new_value);
#else
	(void) strncpy(dp->value, new_value, dp->stored_length);
#endif /* OW_I18N */
	if (created(ip) && !hidden(ip) && panel->kbd_focus_item == ip)
	    paint_caret(ip, FALSE);
	update_value_offset(ip, 0, 0, 1);
	update_value(ip,
#ifdef OW_I18N
		     FALSE	/* Not a wchar */,
#endif
		     ACTION_LINE_END,	/* action */
		     FALSE,		/* ok_to_insert */
		     TRUE,		/* synthetic event */
		     &retstatus );
	if (created(ip) && !hidden(ip) && panel->kbd_focus_item == ip)
	    paint_caret(ip, TRUE);
    }

    /*
     * update the value & items rect if the width or height of the value has
     * changed.
     */
    if (value_rect_changed) {
	int		chrht;
	ip->value_rect.r_width = dp->display_width;
        if ((ip->value_rect.r_width < 2*dp->scroll_btn_width) && !no_buttons )
	    ip->value_rect.r_width = 2*dp->scroll_btn_width;
	chrht = xv_get(ip->value_font, FONT_DEFAULT_CHAR_HEIGHT);
	ip->value_rect.r_height = MAX(chrht + BOX_Y, dp->scroll_btn_height);
	ip->rect = panel_enclosing_rect(&ip->label_rect, &ip->value_rect);
	xv_set(dp->drop_site,
	       DROP_SITE_DELETE_REGION, NULL,
	       DROP_SITE_REGION, &ip->value_rect,
	       NULL);
    }

#ifdef OW_I18N
    if (select_line && wslen(dp->value_wc))
#else
    if (select_line && strlen(dp->value))
#endif /* OW_I18N */
    {
	/* Select line and position caret at the end of the line */
	dp->select_click_cnt[PANEL_SEL_PRIMARY] = 3;  /* fake a triple-click */
	update_text_rect(ip);
	panel_select_line(ip, NULL, PANEL_SEL_PRIMARY);
	dp->delete_pending = TRUE;
	if (xv_set(panel->sel_owner[PANEL_SEL_PRIMARY], SEL_OWN, TRUE, NULL)
	    == XV_OK) {
	    if (panel->sel_holder[PANEL_SEL_PRIMARY])
		text_seln_dehighlight(panel->sel_holder[PANEL_SEL_PRIMARY],
				      PANEL_SEL_PRIMARY);
	    panel->sel_holder[PANEL_SEL_PRIMARY] = ip;
	    text_seln_highlight(panel, ip, PANEL_SEL_PRIMARY);
#ifdef OW_I18N
	    /*
	     * We do not have to make a copy (yet).
	     */
#else
	    text_set_sel_data(panel, dp, PANEL_SEL_PRIMARY);
#endif
	}
	update_caret_offset(ip, 0, 0);
	panel_set_kbd_focus(panel, ip);
    }

    return XV_OK;
}


/*ARGSUSED*/
Pkg_private     Xv_opaque
text_get_attr(item_public, status, which_attr, avlist)
    Panel_item      item_public;
    int            *status;
    register Attr_attribute which_attr;
    va_list         avlist;
{
    Text_info	   *dp = TEXT_PRIVATE(item_public);
#ifdef OW_I18N
    Item_info	   *ip = ITEM_PRIVATE(item_public);
    char	    temp_mask[4];
#endif /* OW_I18N */

    switch (which_attr) {
#ifdef OW_I18N
      case PANEL_ITEM_IC_ACTIVE:
	return (Xv_opaque) (ic_active(ip));

      case PANEL_VALUE:
	if (ip->panel->preedit_item && 
		*ip->panel->preedit->text->string.wide_char) {        
	    panel_implicit_commit(ip->panel->preedit_item);
	}
	dp->value = (char *) _xv_wcstombsdup(dp->value_wc);
	return (Xv_opaque) dp->value;

      case PANEL_VALUE_WCS:
	if (ip->panel->preedit_item && 
		*ip->panel->preedit->text->string.wide_char) {        
	    panel_implicit_commit(ip->panel->preedit_item);
	}
	return (Xv_opaque) dp->value_wc;
#else
      case PANEL_VALUE:
	return (Xv_opaque) dp->value;
#endif /*OW_I18N*/

      case PANEL_VALUE_UNDERLINED:
	return (Xv_opaque) (dp->flags & UNDERLINED) ? TRUE : FALSE;

#ifdef OW_I18N
      case PANEL_VALUE_STORED_LENGTH:
	if ((dp->flags & STORED_LENGTH_WC) == 0)
	    return (Xv_opaque) dp->stored_length;
	else
	    return (-1);

      case PANEL_VALUE_STORED_LENGTH_WCS:
	if ((dp->flags & STORED_LENGTH_WC) == 1)
	    return (Xv_opaque) dp->stored_length;
	else
	    return (-1);
#else
      case PANEL_VALUE_STORED_LENGTH:
	return (Xv_opaque) dp->stored_length;

#endif /* OW_I18N */

      case PANEL_VALUE_DISPLAY_LENGTH:
	return (Xv_opaque) dp->display_length;

      case PANEL_VALUE_DISPLAY_WIDTH:
	return (Xv_opaque) dp->display_width;

#ifdef OW_I18N
      case PANEL_MASK_CHAR: 
	wctomb(temp_mask, dp->mask_wc);
	dp->mask = temp_mask[0];
	return (Xv_opaque) dp->mask;

      case PANEL_MASK_CHAR_WC:
	return (Xv_opaque) dp->mask_wc;
#else
      case PANEL_MASK_CHAR:
	return (Xv_opaque) dp->mask;
#endif /*OW_I18N*/

      case PANEL_NOTIFY_LEVEL:
	return (Xv_opaque) dp->notify_level;

#ifdef OW_I18N
      case PANEL_NOTIFY_STRING:
	dp->terminators = (char *)_xv_wcstombsdup(dp->terminators_wc);
	return (Xv_opaque) dp->terminators;

      case PANEL_NOTIFY_STRING_WCS:
	return (Xv_opaque) dp->terminators_wc;
#else
      case PANEL_NOTIFY_STRING:
	return (Xv_opaque) dp->terminators;
#endif /* OW_I18N */

      case PANEL_READ_ONLY:
	if (dp->flags & PTXT_READ_ONLY)
	    return (Xv_opaque) TRUE;
	else
	    return (Xv_opaque) FALSE;

      default:
	*status = XV_ERROR;
	return (Xv_opaque) 0;
    }
}


Pkg_private int
text_destroy(item_public, status)
    Panel_item      item_public;
    Destroy_status  status;
{
    Text_info	   *dp = TEXT_PRIVATE(item_public);

    if ((status == DESTROY_CHECKING) || (status == DESTROY_SAVE_YOURSELF))
	return XV_OK;

    text_remove(item_public);

#ifndef OW_I18N
    xv_destroy(dp->dnd_item);
#endif
    xv_destroy(dp->dnd);
    xv_destroy(dp->drop_site);
#ifdef OW_I18N
    if (dp->undo_buffer_wc) xv_free(dp->undo_buffer_wc);
    if (dp->value_wc) xv_free(dp->value_wc);
    if (dp->terminators_wc) xv_free(dp->terminators_wc);
    if (dp->undo_buffer) xv_free(dp->undo_buffer);
    if (dp->value) xv_free(dp->value);
    if (dp->terminators) xv_free(dp->terminators);
#else
    free(dp->undo_buffer);
    free(dp->value);
    free(dp->terminators);
#endif /* OW_I18N */
    free((char *) dp);

    return XV_OK;
}



/* --------------------  Panel Item Operations  -------------------- */
static void
text_handle_event(item_public, event)
    Panel_item      item_public;
    register Event *event;
{
    Text_info	   *dp = TEXT_PRIVATE(item_public);
    int		    format;
    Frame	    frame;
    Item_info      *ip = ITEM_PRIVATE(item_public);
    long	    length;
    Xv_Notice	    notice;
    int		    own_primary_seln;
    Panel_info	   *panel = ip->panel;
    struct pr_size  size;
    int		    take_down_caret;
    int		    retstatus;

    take_down_caret =
	(event_is_down(event) || event_action(event) == xv_iso_cancel ||
	 !event_is_iso(event)) &&
	event_action(event) != ACTION_MENU
#ifdef OLD
	&& !server_get_seln_function_pending(xv_server(info));
#else
	;
#endif
    if (take_down_caret && panel->kbd_focus_item) {
	/* turn caret off */
	paint_caret(panel->kbd_focus_item, FALSE);
    }

    update_text_rect(ip);

    if ( (event_action(event) == ACTION_DRAG_COPY 
	  || event_action(event) == ACTION_DRAG_MOVE)
	&& !(dp->flags & PTXT_READ_ONLY) ) {	/* no drops for read-only! */

	if (dnd_decode_drop(panel->sel_req, event) != XV_ERROR) {
	    /* Set caret to position pointed to by cursor */
	    dp->caret_offset = event->ie_locx - dp->text_rect.r_left;
	    if (dp->caret_offset < 0)
		/* Set caret offset to leftmost pixel */
		dp->caret_offset = 0;
	    if (dp->caret_offset > dp->text_rect.r_width)
		/* Set caret_offset to rightmost pixel + 1 */
		dp->caret_offset = dp->text_rect.r_width;
	    update_caret_offset(ip, 0, 0);
	    if (panel->sel_holder[PANEL_SEL_PRIMARY] != ip ||
		dp->caret_position < dp->seln_first[PANEL_SEL_PRIMARY] ||
		dp->caret_position > dp->seln_last[PANEL_SEL_PRIMARY]) {
		/* We haven't dropped on the selection itself:
		 * Insert the dragged selection contents into the text field.
		 * Otherwise, the drop is ignored.
		 */
#ifdef OW_I18N
		update_value(ip, FALSE, event_action(event), TRUE, FALSE, &retstatus);
#else
		update_value(ip, event_action(event), TRUE, FALSE, &retstatus);
#endif

                if (!(dp->flags & SELECTION_REQUEST_FAILED) ||
                     (retstatus & DROP_OR_PASTE_FAILED)) {

                    if (event_action(event) == ACTION_DRAG_MOVE &&
                        !(retstatus & DRAG_MOVE_FILENAME)) {
			/* Post delete request back to owner */
			xv_set(panel->sel_req, SEL_TYPE, panel->atom.delete, NULL);
			(void) xv_get(panel->sel_req, SEL_DATA, &length, &format);
		    } else if (dp->flags & TEXT_SELECTED) {
			/* ACTION_DRAG_COPY to the same text field:
			 * Set the caret at the beginning of the primary selection.
			 */
			dp->caret_position = dp->seln_first[PANEL_SEL_PRIMARY];
#ifdef OW_I18N
			size.x = XwcTextEscapement(ip->value_fontset_id,
				 &dp->value_wc[dp->first_char],
				 dp->caret_position - dp->first_char);
#else
			size = xv_pf_textwidth(dp->caret_position - dp->first_char,
					       ip->value_font,
					       &dp->value[dp->first_char]);
#endif /* OW_I18N */
			dp->caret_offset = size.x;
		    }
		    /* dp->dnd_sel_first and dp->dnd_sel_last point to text
		     * that was dropped into text field.  Make this text the
		     * primary selection.
		     */
		    own_primary_seln = xv_get(panel->sel_owner[PANEL_SEL_PRIMARY],
					      SEL_OWN);
		    if (own_primary_seln) {
			if (panel->sel_holder[PANEL_SEL_PRIMARY]) {
			    if (panel->sel_holder[PANEL_SEL_PRIMARY] == ip)
				text_seln_dehighlight(ip, PANEL_SEL_PRIMARY);
			    else
				text_lose_rank(panel, PANEL_SEL_PRIMARY);
			}
		    } else
			own_primary_seln =
			    xv_set(panel->sel_owner[PANEL_SEL_PRIMARY],
				   SEL_OWN, TRUE,
				   NULL) == XV_OK;
		    if (own_primary_seln) {
			panel->sel_holder[PANEL_SEL_PRIMARY] = ip;
			if (dp->dnd_sel_last > dp->last_char) {
			    /* Scroll right so that the last character inserted
			     * is visible.
			     */
			    do {
				update_value_offset(ip, 0, 1, 1);
				update_caret_offset(ip, -1, 0);
			    } while (dp->dnd_sel_last > dp->last_char);
			    paint_value(ip, PV_HIGHLIGHT);
			}
			dp->seln_first[PANEL_SEL_PRIMARY] =
			    MAX(dp->first_char, dp->dnd_sel_first);
			dp->seln_last[PANEL_SEL_PRIMARY] = dp->dnd_sel_last;
			dp->flags |= TEXT_SELECTED;
			dp->delete_pending = TRUE;
			dp->select_click_cnt[PANEL_SEL_PRIMARY] = 1;
			text_seln_highlight(panel, ip, PANEL_SEL_PRIMARY);
#ifdef OW_I18N
			/*
			 * We do not need make copy yet.
			 */
#else
			text_set_sel_data(panel, dp, PANEL_SEL_PRIMARY);
#endif
		    }
		}
	    }
	    dnd_done(panel->sel_req);
	    if (panel->kbd_focus_item != ip)
		panel_set_kbd_focus(panel, ip);
	    if (!panel->status.has_input_focus)
		win_set_kbd_focus(PANEL_PUBLIC(panel),
				  (XID) xv_get(event_window(event), XV_XID));
	} else {
	    frame = xv_get(PANEL_PUBLIC(panel), WIN_FRAME);
	    notice = xv_create(frame, NOTICE,
		NOTICE_MESSAGE_STRINGS,
		    XV_MSG("Drag and Drop failed:"),
		    XV_MSG("Unable to decode Drag and Drop message"),
		    0,
		XV_SHOW, TRUE,
		NULL);
	    xv_destroy(notice);
	}
	panel->current = NULL;
    } else
	panel_default_handle_event(item_public, event);

    /* Note: The panel item with the keyboard focus may have changed. */
    if (take_down_caret && panel->kbd_focus_item &&
	panel->kbd_focus_item->item_type == PANEL_TEXT_ITEM) {
#ifdef OW_I18N
        /*  If the item with focus is panel text item,
         *  and this particular panel text item should not
         *  allow pre-edit text, then we disable the entire
         *  panel's IC temporarily...  Until the next focus
         *  change.  This may not be the best place to do
         *  this, but it's the best I've found so far.
         */
        {
            Panel       panel_public = PANEL_PUBLIC(panel);

            if (ic_active(panel->kbd_focus_item) == FALSE)
                xv_set(panel_public, WIN_IC_ACTIVE, FALSE, NULL);
            else
                xv_set(panel_public, WIN_IC_ACTIVE, TRUE, NULL);
        }

#endif /* OW_I18N */
	/* turn text caret back on */
	paint_caret(panel->kbd_focus_item, TRUE);
    }
}

 
static          Notify_value
textitem_scroll_itimer_func( item, which)
    Panel_item          item;
    int                 which;
{
    Text_info      *dp = TEXT_PRIVATE(item);
    Item_info      *ip = ITEM_PRIVATE(item);
    Panel_info     *panel = ip->panel;
 
    if (dp->flags & LEFT_SCROLL_BTN_SELECTED)
    {
        if (!dp->first_char)
        {
            dp->flags &= ~SELECTING_SCROLL_BTN;
            dp->flags &= ~LEFT_SCROLL_BTN_SELECTED;
            panel_autoscroll_stop_itimer( item );
            return( NOTIFY_DONE );
        }
        if (panel->kbd_focus_item)
            paint_caret(panel->kbd_focus_item, FALSE);
        horizontal_scroll(ip, -1);
        if (panel->kbd_focus_item)
            paint_caret(panel->kbd_focus_item, TRUE);
    }
    else if (dp->flags & RIGHT_SCROLL_BTN_SELECTED)
    {
#ifdef OW_I18N
        if (dp->last_char >= (int)wslen(dp->value_wc) - 1)
#else
        if (dp->last_char >= (int)strlen(dp->value) - 1)
#endif /* OW_I18N */
        {
            dp->flags &= ~SELECTING_SCROLL_BTN;
            dp->flags &= ~RIGHT_SCROLL_BTN_SELECTED;
            panel_autoscroll_stop_itimer( item );
            return( NOTIFY_DONE );
        }
        if (panel->kbd_focus_item)
            paint_caret(panel->kbd_focus_item, FALSE);
        horizontal_scroll(ip, 1);
        if (panel->kbd_focus_item)
            paint_caret(panel->kbd_focus_item, TRUE);
    }    
 
    return( NOTIFY_DONE );
} /* textitem_scroll_itimer_func */


/* ARGSUSED */
static void
text_begin_preview(item_public, event)
    Panel_item	    item_public;
    Event          *event;
{
    Xv_Cursor	    accept_cursor;
    u_char          adjust_right;
    int		    caret_position;	/* index of character underneath
					 * the cursor */
    Cursor_drag_type cursor_drag_type;	/* CURSOR_MOVE or CURSOR_DUPLICATE */
    Text_info	   *dp = TEXT_PRIVATE(item_public);
    int             dragging;
    char	   *error_msg;
    int		    event_offset;
    int             ext_caret_offset;	/* new caret offset when adjusting to
					 * the right */
    Frame	    frame;
    Item_info      *ip = ITEM_PRIVATE(item_public);
    int		    is_multiclick;
    Xv_Cursor	    neutral_cursor;
    Xv_Notice	    notice;
    Panel_info	   *panel = ip->panel;
    int             save_caret_offset;	/* caret doesn't move in secondary
					 * selections */
    CHAR            save_char;
    u_char          save_delete_pending;
    int             sel_rank;	/* selection rank: PANEL_SEL_PRIMARY or
				 * PANEL_SEL_SECONDARY */
    struct pr_size  size;
    int		    status;

    int         idx;
    int         len;
#ifdef OW_I18N1
    unsigned char  *sel_data;
    int		    sel_length;
#endif

#ifdef OW_I18N
/* Commit any uncommitted preedit text first */
    if (panel->preedit_item && 
		*panel->preedit->text->string.wide_char) {
	panel_implicit_commit(panel->preedit_item);
    }
#endif /*OW_I18N*/

    if ((event_action(event) == ACTION_SELECT) 
	|| (event_action(event) == ACTION_ADJUST))
	paint_caret(ip, FALSE);

    dp->undo_direction = INVALID;

    if (event_flags(event) & (IE_QUICK_MOVE | IE_QUICK_COPY))
	sel_rank = PANEL_SEL_SECONDARY;
    else
	sel_rank = PANEL_SEL_PRIMARY;

    /*
     * If primary selection and wiping thru text item, call
     * text_add_selection() to invert the difference in the
     * growing or shrinking selection.
     */
#ifdef OW_I18N
    dragging = (wslen(dp->value_wc) &&
#else
    dragging = (strlen(dp->value) &&
#endif /*OW_I18N*/
	(dp->select_click_cnt[sel_rank] > 0) &&
	(sel_rank == PANEL_SEL_PRIMARY) &&
	(event_action(event) == LOC_DRAG) &&
	(action_select_is_down(event)) &&
	(dp->flags & TEXT_SELECTED));

    /* Ask for kbd focus if this is a primary selection */
    if (sel_rank == PANEL_SEL_PRIMARY)
	win_set_kbd_focus(PANEL_PUBLIC(panel),
			  (XID) xv_get(event_window(event), XV_XID));

    /* Check if one of the horizontal scrolling buttons was selected */
    event_offset = event->ie_locx - ip->value_rect.r_left;
    if (dp->first_char &&
	event_offset >= 0 &&
	event_offset < dp->scroll_btn_width) {
	if ((event_action(event) == ACTION_SELECT ||
	     (event_action(event) == LOC_DRAG && action_select_is_down(event)))
	    && !(dp->flags & LEFT_SCROLL_BTN_SELECTED)) {
            panel_autoscroll_start_itimer( item_public,
		textitem_scroll_itimer_func );

	    /* SELECT-down on left scrolling button: invoke left button */
	    text_cancel_preview(item_public, event);
	    draw_scroll_btn(ip,
			    OLGX_SCROLL_BACKWARD | OLGX_INVOKED | OLGX_ERASE);
	    dp->flags |= SELECTING_SCROLL_BTN | LEFT_SCROLL_BTN_SELECTED;
	} /* else ignore event */
	return;
    } else if (event_offset > ip->value_rect.r_width -
		    dp->scroll_btn_width &&
	       event_offset < ip->value_rect.r_width &&
#ifdef OW_I18N
	       dp->last_char < (int)wslen(dp->value_wc) - 1)
#else
	       dp->last_char < (int)strlen(dp->value) - 1)
#endif /* OW_I18N */
 	{
	if ((event_action(event) == ACTION_SELECT ||
	     (event_action(event) == LOC_DRAG && action_select_is_down(event)))
	    && !(dp->flags & RIGHT_SCROLL_BTN_SELECTED)) {
            panel_autoscroll_start_itimer( item_public,
		textitem_scroll_itimer_func );
	    /* SELECT-down on right scrolling button: invoke right button */
	    text_cancel_preview(item_public, event);
	    draw_scroll_btn(ip,
			    OLGX_SCROLL_FORWARD | OLGX_INVOKED | OLGX_ERASE);
	    dp->flags |= SELECTING_SCROLL_BTN | RIGHT_SCROLL_BTN_SELECTED;
	} /* else ignore event */
	return;
    }

    /*
     * If we started out selecting a scrolling button, then don't
     * start selecting text.  Unhiglight any highlighted scrolling
     * button and return.
     */
    if (dp->flags & SELECTING_SCROLL_BTN) {
	if (dp->flags & LEFT_SCROLL_BTN_SELECTED) {
	    dp->flags &= ~LEFT_SCROLL_BTN_SELECTED;
            panel_autoscroll_stop_itimer( item_public );
	    draw_scroll_btn(ip, OLGX_SCROLL_BACKWARD | OLGX_ERASE);
	} else if (dp->flags & RIGHT_SCROLL_BTN_SELECTED) {
	    dp->flags &= ~RIGHT_SCROLL_BTN_SELECTED;
            panel_autoscroll_stop_itimer( item_public );
	    draw_scroll_btn(ip, OLGX_SCROLL_FORWARD | OLGX_ERASE);
	}
	return;
    }

    /*
     * If nothing is selected on the line being pointed to, then 
     * translate an ADJUST or DRAG action to SELECT.
     */
    save_delete_pending = dp->delete_pending;
    if ((dp->select_click_cnt[sel_rank] == 0 ||
	 panel->sel_holder[sel_rank] != ip) &&
	 (event_action(event) == ACTION_ADJUST ||
	  event_action(event) == LOC_DRAG))
	event_set_action(event, ACTION_SELECT);

    /*
     * Ignore the middle mouse button when the mouse is dragged to a line not
     * containing the caret.
     */
    if ((panel->kbd_focus_item != ip) &&
	((event_action(event) == LOC_DRAG) && event_middle_is_down(event))) {
	return;
    }

    /* The caret offset (position) does not change during secondary
     * selections are drag and drop operations.
     */
    save_caret_offset = dp->caret_offset;

    /*
     * Define rectangle containing the text (i.e., value rect less arrows),
     * and caret offset within that rectangle.
     */
    update_text_rect(ip);

    /* Define caret_offset as offset within text rectangle */
    dp->caret_offset = event->ie_locx - dp->text_rect.r_left;
    if (dp->caret_offset < 0)
	dp->caret_offset = 0;	/* => leftmost pixel */
    if (dp->caret_offset > dp->text_rect.r_width)
	dp->caret_offset = dp->text_rect.r_width;  /* => rightmost pixel + 1 */

    if (event_action(event) == ACTION_SELECT) {
	is_multiclick = panel_is_multiclick(panel, &dp->last_click_time,
				  	    &event_time(event));
	if (!is_multiclick && dp->flags & TEXT_SELECTED &&
	    sel_rank == PANEL_SEL_PRIMARY) {
	    /* If SELECT-down occurred over highlighted text, then this is
	     * a possible Drag and Drop operation.
	     */
	    caret_position = dp->first_char +
#ifdef OW_I18N
		char_position(dp->caret_offset, ip->value_font,
			      &dp->value_wc[dp->first_char], FALSE);
#else
		char_position(dp->caret_offset, ip->value_font,
			      &dp->value[dp->first_char], FALSE);
#endif /* OW_I18N */
	    if (caret_position >= dp->seln_first[PANEL_SEL_PRIMARY] &&
		caret_position <= dp->seln_last[PANEL_SEL_PRIMARY]) {
		dp->select_down_x = event_x(event);
		dp->select_down_y = event_y(event);
		panel->status.current_item_active = TRUE;
		dp->caret_offset = save_caret_offset;
		return;
	    }
	}
	/*
	 * Set seln_first and seln_last to the character pointed to by the
	 * cursor.
	 */
#ifdef OW_I18N
	if (wslen(dp->value_wc) == 0)
#else
	if (strlen(dp->value) == 0)
#endif /* OW_I18N */
	{
	    dp->seln_first[sel_rank] = 0;
	    dp->seln_last[sel_rank] = -1;
	} else {
#ifdef OW_I18N
	    save_char = dp->value_wc[dp->last_char + 1];
	    dp->value_wc[dp->last_char + 1] = '\0';
	    dp->seln_first[sel_rank] = dp->first_char;
	    dp->seln_first[sel_rank] += char_position(dp->caret_offset,
				      ip->value_font,
				      &dp->value_wc[dp->first_char], TRUE);
	    dp->value_wc[dp->last_char + 1] = save_char;
	    if (dp->seln_first[sel_rank] >= (int)wslen(dp->value_wc))
		if ((dp->seln_first[sel_rank] = wslen(dp->value_wc) - 1) < 0)
		    dp->seln_first[sel_rank] = 0;
#else
	    save_char = dp->value[dp->last_char + 1];
	    dp->value[dp->last_char + 1] = 0;
	    dp->seln_first[sel_rank] = dp->first_char;
	    dp->seln_first[sel_rank] += char_position(dp->caret_offset,
				      ip->value_font,
				      &dp->value[dp->first_char], TRUE);
	    dp->value[dp->last_char + 1] = save_char;
	    if (dp->seln_first[sel_rank] >= (int)strlen(dp->value))
		if ((dp->seln_first[sel_rank] = strlen(dp->value) - 1) < 0)
		    dp->seln_first[sel_rank] = 0;
#endif /* OW_I18N */
	    dp->seln_last[sel_rank] = dp->seln_first[sel_rank];
	}
	dp->select_click_cnt[sel_rank]++;
	/*
	 * If this is not a double click, or we've moved to another panel
	 * item, then reset mouse left click count to 1.
	 */
	if (!is_multiclick ||
	    (panel->sel_holder[sel_rank] && panel->sel_holder[sel_rank] != ip))
	    dp->select_click_cnt[sel_rank] = 1;
	dp->last_click_time = event_time(event);
	dp->flags &= ~TEXT_SELECTED;	/* assume we're not selecting text */
	if (dp->select_click_cnt[sel_rank] > 1) {
	    /* Double or triple click */
            if (!event_ctrl_is_down(event))
            {
#ifdef OW_I18N
                if ((int)wslen(dp->value_wc) > 0)
#else
                if ((int)strlen(dp->value) > 0)
#endif /* OW_I18N */
                    dp->flags |= TEXT_SELECTED;
            }
	    (void) panel_multiclick_handler(ip, event, sel_rank);
	} else if (event_ctrl_is_down(event))
	    /* Single click with ctrl */
	    dp->flags |= TEXT_SELECTED;
    }
#ifdef OW_I18N
    else if (wslen(dp->value_wc) && (dp->select_click_cnt[sel_rank] > 0) &&
#else
    else if (strlen(dp->value) && (dp->select_click_cnt[sel_rank] > 0) &&
#endif /* OW_I18N */
	       (event_action(event) == ACTION_ADJUST ||
		(event_action(event) == LOC_DRAG &&
	         (action_select_is_down(event) ||
		  action_adjust_is_down(event))))) {

	if (event_action(event) == LOC_DRAG &&
	    action_select_is_down(event) &&
	    panel->status.current_item_active) {
	    dp->caret_offset = save_caret_offset;
	    if (abs(event_x(event) - dp->select_down_x) >=
		    panel->drag_threshold ||
		abs(event_y(event) - dp->select_down_y) >=
		    panel->drag_threshold) {
		/* We've dragged the cursor past the drag threshold:
		 * Initiate a Drag and Drop operation.
		 * Note that we set current_item_active FALSE before doing
		 * the Drag and Drop.  This lets the ACTION_DRAG_{COPY,MOVE}
		 * event to be sent to the panel item under the pointer in
		 * panel_default_event.  Also in panel_default_event, we don't
		 * call panel_cancel if the event is ACTION_DRAG_COPY,
		 * ACTION_DRAG_MOVE or ACTION_DRAG_PREVIEW.  This prevents the
		 * primary selection from being cancelled.
		 */
		panel->status.current_item_active = FALSE;
		if (panel_duplicate_key_is_down(panel, event)) {
		    cursor_drag_type = CURSOR_DUPLICATE;
		    xv_set(dp->dnd,
			   DND_TYPE, DND_COPY,
			   NULL);
		} else {
		    cursor_drag_type = CURSOR_MOVE;
		    xv_set(dp->dnd,
			   DND_TYPE, DND_MOVE,
			   NULL);
		}
#ifdef OW_I18N
		save_char = dp->value_wc[dp->seln_last[PANEL_SEL_PRIMARY]+1];
		dp->value_wc[dp->seln_last[PANEL_SEL_PRIMARY]+1] = 0;
		neutral_cursor = xv_create(PANEL_PUBLIC(panel), CURSOR,
		    CURSOR_STRING_WCS,
			&dp->value_wc[dp->seln_first[PANEL_SEL_PRIMARY]],
		    CURSOR_DRAG_STATE, CURSOR_NEUTRAL,
		    CURSOR_DRAG_TYPE, cursor_drag_type,
		    NULL);
		if (neutral_cursor)
		    xv_set(dp->dnd,
			   DND_CURSOR, neutral_cursor,
			   NULL);
		accept_cursor = xv_create(PANEL_PUBLIC(panel), CURSOR,
		    CURSOR_STRING_WCS,
			&dp->value_wc[dp->seln_first[PANEL_SEL_PRIMARY]],
		    CURSOR_DRAG_STATE, CURSOR_ACCEPT,
		    CURSOR_DRAG_TYPE, cursor_drag_type,
		    NULL);
#else
		save_char = dp->value[dp->seln_last[PANEL_SEL_PRIMARY]+1];
		dp->value[dp->seln_last[PANEL_SEL_PRIMARY]+1] = 0;
		neutral_cursor = xv_create(PANEL_PUBLIC(panel), CURSOR,
		    CURSOR_STRING,
			&dp->value[dp->seln_first[PANEL_SEL_PRIMARY]],
		    CURSOR_DRAG_STATE, CURSOR_NEUTRAL,
		    CURSOR_DRAG_TYPE, cursor_drag_type,
		    NULL);
		if (neutral_cursor)
		    xv_set(dp->dnd,
			   DND_CURSOR, neutral_cursor,
			   NULL);
		accept_cursor = xv_create(PANEL_PUBLIC(panel), CURSOR,
		    CURSOR_STRING,
			&dp->value[dp->seln_first[PANEL_SEL_PRIMARY]],
		    CURSOR_DRAG_STATE, CURSOR_ACCEPT,
		    CURSOR_DRAG_TYPE, cursor_drag_type,
		    NULL);
#endif /* OW_I18N */
		if (accept_cursor)
		    xv_set(dp->dnd,
			   DND_ACCEPT_CURSOR, accept_cursor,
			   NULL);
#ifdef OW_I18N
		dp->value_wc[dp->seln_last[PANEL_SEL_PRIMARY]+1] = save_char;
#else
		dp->value[dp->seln_last[PANEL_SEL_PRIMARY]+1] = save_char;

		xv_set(dp->dnd_item,
		       SEL_DATA, &dp->value[dp->seln_first[PANEL_SEL_PRIMARY]],
		       SEL_LENGTH,
			   dp->seln_last[PANEL_SEL_PRIMARY] -
			   dp->seln_first[PANEL_SEL_PRIMARY] + 1,
		       NULL);
#endif /* OW_I18N */
		status = dnd_send_drop(dp->dnd);
		if (neutral_cursor)
		    xv_destroy(neutral_cursor);
		if (accept_cursor)
		    xv_destroy(accept_cursor);
		switch (status) {
		  case XV_OK:
		    return;

		  case DND_ABORTED:
		    error_msg = NULL;
		    break;

		  case DND_TIMEOUT:
		    error_msg = XV_MSG("Selection timed out");
		    break;

		  case DND_ILLEGAL_TARGET:
		    error_msg = XV_MSG("Illegal drop target");
		    break;

		  case DND_SELECTION:
		    error_msg = XV_MSG("Unable to acquire selection");
		    break;
		  case DND_ROOT:
		    error_msg = XV_MSG("Root window is not a valid drop target");
		    break;

		  case XV_ERROR:
		    error_msg = XV_MSG("unexpected internal error");
		    break;
		}
		if (error_msg) {
		    frame = xv_get(PANEL_PUBLIC(panel), WIN_FRAME);
		    notice = xv_create(frame, NOTICE,
			NOTICE_MESSAGE_STRINGS,
			    XV_MSG("Drag and Drop failed:"),
			    error_msg,
			    0,
			XV_SHOW, TRUE,
			NULL);
		    xv_destroy(notice);
		}
		panel->current = NULL;
	    }
	    return;
	}

#ifdef OW_I18N
	idx = char_position(dp->caret_offset, ip->value_font,
	      &dp->value_wc[dp->first_char], FALSE);
	len = (int)wslen(dp->value_wc);
#else
        idx = char_position(dp->caret_offset, ip->value_font,
            &dp->value[dp->first_char], FALSE);
        len = strlen( dp->value );
#endif /* OW_I18N */

        if ((len == dp->caret_position) && (idx >= len) &&
            (event_action( event ) == ACTION_ADJUST))
            ;
        else if ((len == dp->caret_position) &&
            (event_action(event) == LOC_DRAG) &&
            (idx >= len) &&
            (action_select_is_down(event) ||
             action_adjust_is_down(event)))
            ;
        else /* define extended selection point */
        {

	/* Define extended selection point.
	 * Get the index of the character that the caret_offset is
	 * within.  Do not use the "balance beam" method.
	 */
#ifdef OW_I18N
	dp->ext_first = char_position(dp->caret_offset,
				      ip->value_font,
				      &dp->value_wc[dp->first_char], FALSE);
	if (dp->first_char)
	    dp->ext_first += dp->first_char;
	if (dp->ext_first >= (int)wslen(dp->value_wc))
	    if ((dp->ext_first = wslen(dp->value_wc) - 1) < 0)
		dp->ext_first = 0;
#else
	dp->ext_first = char_position(dp->caret_offset,
				      ip->value_font,
				      &dp->value[dp->first_char], FALSE);
	if (dp->first_char)
	    dp->ext_first += dp->first_char;
	if (dp->ext_first >= (int)strlen(dp->value))
	    if ((dp->ext_first = strlen(dp->value) - 1) < 0)
		dp->ext_first = 0;
#endif /* OW_I18N */
	dp->ext_last = dp->ext_first;

	if (dp->select_click_cnt[sel_rank] >= 3) {
	    dp->ext_first = dp->seln_first[sel_rank];
	    dp->ext_last = dp->seln_last[sel_rank];
	    panel_select_line(ip, event, sel_rank); /* update caret offset */
	} else {
	    if (dp->select_click_cnt[sel_rank] == 2) {
		panel_find_word(dp, &dp->ext_first, &dp->ext_last);
	    }
#ifdef OW_I18N
	    size.x = XwcTextEscapement(ip->value_fontset_id,
				       &dp->value_wc[dp->first_char],
				       dp->ext_last - dp->first_char + 1);
#else
	    size = xv_pf_textwidth(dp->ext_last - dp->first_char + 1,
				   ip->value_font,
				   &dp->value[dp->first_char]);
#endif /* OW_I18N */
	    ext_caret_offset = size.x;

	    /* Adjust first or last character selected */
	    if (dp->ext_last > dp->seln_last[sel_rank])
		adjust_right = TRUE;
	    else if (dp->ext_first < dp->seln_first[sel_rank])
		adjust_right = FALSE;	/* adjust left */
	    else if ((dp->ext_first - dp->seln_first[sel_rank]) <
		     (dp->seln_last[sel_rank] - dp->ext_first))
		adjust_right = FALSE;	/* adjust left */
	    else
		adjust_right = TRUE;
	    if (adjust_right) {	/* Note: caret must be to the right of the
				 * last selected character, due to a check
				 * made on this assumption in
				 * text_seln_delete(). */
		dp->seln_last[sel_rank] = dp->ext_last;
		dp->caret_offset = ext_caret_offset;
	    } else {		/* adjust left */
		if (dp->seln_last[sel_rank] == dp->seln_first[sel_rank] &&
		    (dp->flags & TEXT_SELECTED) == 0) {
		    /* First drag after SELECT-down: don't include character
		     * to the right of the caret.
		     */
		    dp->seln_last[sel_rank] = dp->caret_position - 1;
		}
		dp->seln_first[sel_rank] = dp->ext_first;
#ifdef OW_I18N
		size.x = XwcTextEscapement(ip->value_fontset_id,
					&dp->value_wc[dp->first_char],
				        dp->ext_first - dp->first_char);
#else
		size = xv_pf_textwidth(dp->ext_first - dp->first_char,
				       ip->value_font,
				       &dp->value[dp->first_char]);
#endif /* OW_I18N */
		dp->caret_offset = size.x;
	    }
	}
	/*
	 * ADJUST or dragging SELECT: select text
	 */
	dp->flags |= TEXT_SELECTED;
	} /* define extended selection point */
    }

    dp->delete_pending = (sel_rank == PANEL_SEL_PRIMARY) &&
	(dp->flags & TEXT_SELECTED);
    update_caret_offset(ip, 0, 0);

    /* If we're selecting text, and there is text to be selected,
     * then highlight the selected text and set the selection data.
     */
    if (dragging) {
	text_add_selection(panel, ip);
#ifndef OW_I18N
	text_set_sel_data(panel, dp, sel_rank);
#endif
    } else {
	if (panel->sel_holder[sel_rank]) {
	    if (panel->sel_holder[sel_rank] == ip)
		text_seln_dehighlight(ip, sel_rank);
	    else
		text_lose_rank(panel, sel_rank);
	}
#ifdef OW_I18N
	if ((dp->flags & TEXT_SELECTED) && (int)wslen(dp->value_wc) > 0)
#else
	if ((dp->flags & TEXT_SELECTED) && (int)strlen(dp->value) > 0)
#endif /* OW_I18N */
	{
	    text_seln_highlight(panel, ip, sel_rank);
#ifndef OW_I18N
	    text_set_sel_data(panel, dp, sel_rank);
#endif
	} else {
#ifdef OW_I18N
	    sdfprintf((stderr, "Set selection to NULL\n"));
	    if (sel_rank == PANEL_SEL_CLIPBOARD) {
	        ip->panel->clipboard.value = 0;
	    } else {
		dp->seln_last[sel_rank] = -1;
	    }
#else
	    xv_set(panel->sel_item[sel_rank],
		   SEL_DATA, 0,
		   SEL_LENGTH, 0,
		   NULL);
#endif
	}
    }

    /* Make this item the selection.  Acquire the {PRIMARY,SECONDARY}
     * selection for this ip.
     */
    if (xv_set(panel->sel_owner[sel_rank], SEL_OWN, TRUE, NULL) == XV_OK) {
	panel->sel_holder[sel_rank] = ip;
    }

    /*
     * If we are getting the SECONDARY selection, restore dp->caret_offset
     * since the insertion point (represented by the caret) doesn't move during
     * secondary selections, and restore dp->delete_pending, since it only
     * applies to primary selections.
     */
    if (sel_rank == PANEL_SEL_SECONDARY) {
	dp->caret_offset = save_caret_offset;
	dp->delete_pending = save_delete_pending;
    }
    dp->flags |= SELECTING_ITEM;
    if (!(dp->flags & PTXT_READ_ONLY) && sel_rank == PANEL_SEL_PRIMARY)
	panel_set_kbd_focus(panel, ip);
}


/* ARGSUSED */
static void
text_cancel_preview(item_public, event)
    Panel_item	    item_public;
    Event          *event;
{
    Text_info	   *dp = TEXT_PRIVATE(item_public);
    Item_info      *ip = ITEM_PRIVATE(item_public);
    Panel_info	   *panel = ip->panel;
    int		    sel_rank;

    if (dp->flags & SELECTING_SCROLL_BTN) {
	dp->flags &= ~SELECTING_SCROLL_BTN;
	if (dp->flags & LEFT_SCROLL_BTN_SELECTED) {
	    dp->flags &= ~LEFT_SCROLL_BTN_SELECTED;
            panel_autoscroll_stop_itimer( item_public );
	    draw_scroll_btn(ip, OLGX_SCROLL_BACKWARD | OLGX_ERASE);
	} else if (dp->flags & RIGHT_SCROLL_BTN_SELECTED) {
	    dp->flags &= ~RIGHT_SCROLL_BTN_SELECTED;
            panel_autoscroll_stop_itimer( item_public );
	    draw_scroll_btn(ip, OLGX_SCROLL_FORWARD | OLGX_ERASE);
	}
    }
    if (dp->flags & SELECTING_ITEM) {
	panel_yield_kbd_focus(panel);
	dp->flags &= ~SELECTING_ITEM;
    }
    if (panel->status.current_item_active) {
	panel->status.current_item_active = FALSE;
    } else {
	for (sel_rank = PANEL_SEL_PRIMARY;
	     sel_rank <= PANEL_SEL_SECONDARY;
	     sel_rank++) {
	    if (panel->sel_holder[sel_rank] == ip)
		xv_set(panel->sel_owner[sel_rank], SEL_OWN, FALSE, NULL);
	}
    }
}


/* ARGSUSED */
static void
text_accept_preview(item_public, event)
    Panel_item	    item_public;
    Event          *event;
{
    Text_info	   *dp = TEXT_PRIVATE(item_public);
    int		    event_offset;
    Item_info      *ip = ITEM_PRIVATE(item_public);

    /* If SELECT-down occurred over a primary selection, but the SELECT-up
     * occurred before the mouse was dragged drag_threshold pixels, then
     * interpret the SELECT-up as a single click which cancels the selection
     * and sets the caret position.
     */
    if (ip->panel->status.current_item_active) {
	ip->panel->status.current_item_active = FALSE;
	text_lose_rank(ip->panel, PANEL_SEL_PRIMARY); /* cancel the selection */
	event_set_down(event);	/* event = SELECT-down */
	text_begin_preview(item_public, event); /* set the caret position */
	return;
    }

    /* Check if one of the horizontal scrolling buttons was selected */
    dp->flags &= ~SELECTING_SCROLL_BTN;
    event_offset = event->ie_locx - ip->value_rect.r_left;
    if (event_action(event) == ACTION_SELECT) {
	if (dp->first_char &&
	    event_offset >= 0 &&
	    event_offset < dp->scroll_btn_width) {
	    /* User clicked on left scrolling button */
	    dp->flags &= ~LEFT_SCROLL_BTN_SELECTED;
            panel_autoscroll_stop_itimer( item_public );
	    horizontal_scroll(ip, -1);	/* scroll left */
	}
#ifdef OW_I18N
	else if (event_offset > ip->value_rect.r_width -
			dp->scroll_btn_width &&
		   event_offset < ip->value_rect.r_width &&
		   dp->last_char < (int)wslen(dp->value_wc) - 1)
#else
	else if (event_offset > ip->value_rect.r_width -
			dp->scroll_btn_width &&
		   event_offset < ip->value_rect.r_width &&
		   dp->last_char < (int)strlen(dp->value) - 1)
#endif /* OW_I18N */
	{
	    /* User clicked on right scrolling button */
	    dp->flags &= ~RIGHT_SCROLL_BTN_SELECTED;
            panel_autoscroll_stop_itimer( item_public );
	    horizontal_scroll(ip, 1);	/* scroll right */
	}
    }

    if (!(dp->flags & SELECTING_ITEM))
	return;

    dp->flags &= ~SELECTING_ITEM;
    update_caret_offset(ip, 0, 0);

}


static void
text_accept_key(item_public, event)
    Panel_item	    item_public;
    register Event *event;
{
    Text_info	   *dp = TEXT_PRIVATE(item_public);
    int             has_caret;
    Item_info      *ip = ITEM_PRIVATE(item_public);
    int             notify_desired = FALSE;
    Panel_setting   notify_rtn_code;
    int             ok_to_insert;
    Panel_info     *panel = ip->panel;
    int		    retstatus;
#ifdef OW_I18N
    wchar_t	   *ie_string_wc = NULL;
    wchar_t	   *tmp_char_wc;
    wchar_t	   *wc_ptr;
    char	    tmp_char;

    if (panel_navigation_action(event) ||
       (event_is_down(event) &&
	 (event_action(event) == ACTION_CUT ||
	  event_action(event) == ACTION_PASTE ||
	  event_action(event) == ACTION_CANCEL ||
	  event_action(event) == ACTION_UNDO ||
	  event_action(event) == xv_iso_cancel)))
	if (panel->preedit_item && 
		*panel->preedit->text->string.wide_char)
	    panel_implicit_commit(ip);

    if (event_string(event))
	    ie_string_wc = (wchar_t *) 
		_xv_mbstowcsdup((char *) event->ie_string);
#endif /* OW_I18N */

#ifdef OW_I18N
    /*
     * OW_I18N case can not use panel_printable_char, since it is not
     * always wide character.  In case of the iso_8859_1 chars, we
     * need to make sure value is not more than 0xff in order to use
     * isprint(3) macro.
     */
    if ((ie_string_wc != NULL && iswprint(*ie_string_wc)) ||
	(event_action(event) < 0xff && isprint(event_action(event))) ||
#else
    if (panel_printable_char(event_action(event)) ||
#endif
	panel_erase_action(event) || panel_navigation_action(event) ||
	(event_is_up(event) &&
	 (event_action(event) == ACTION_CUT ||
	  event_action(event) == ACTION_PASTE ||
	  event_action(event) == ACTION_CANCEL ||
	  event_action(event) == xv_iso_cancel))) {
	if (dp->delete_pending) {
	    if (event_action(event) == ACTION_ERASE_CHAR_BACKWARD ||
		event_action(event) == ACTION_ERASE_CHAR_FORWARD) {
#ifdef OW_I18N
		if ((int)wslen(dp->value_wc) > 0 &&
#else
		if ((int)strlen(dp->value) > 0 &&
#endif /*OW_I18N*/
		    dp->select_click_cnt[PANEL_SEL_PRIMARY] != 0) {
		    text_seln_delete(ip, PANEL_SEL_PRIMARY);
		    if (dp->notify_level == PANEL_NON_PRINTABLE ||
			dp->notify_level == PANEL_ALL) {
			event_set_action(event, ACTION_DELETE_SELECTION);
			(void) (*ip->notify) (ITEM_PUBLIC(ip), event);
		    }
		    return;
		}
	    } else if (panel_erase_action(event) ||
		panel_navigation_action(event) ||
		event_action(event) == ACTION_CANCEL ||
		event_action(event) == xv_iso_cancel) {
		text_seln_dehighlight(ip, PANEL_SEL_PRIMARY);
		dp->delete_pending = FALSE;
	    }
#ifdef OW_I18N
	    else if ((int)wslen(dp->value_wc) > 0 &&
#else
	    else if ((int)strlen(dp->value) > 0 &&
#endif /* OW_I18N */
		dp->select_click_cnt[PANEL_SEL_PRIMARY] > 0) {
		if (event_action(event) == ACTION_CUT) {
		    /* Copy the primary selection data to the clipboard */
#ifdef OW_I18N
		    text_set_clipboard(panel, ip, dp);
#else
		    text_set_clipboard(panel, ip);
#endif
		}
		/* Delete the selected text */
		text_seln_delete(ip, PANEL_SEL_PRIMARY);
	    }
	}
	dp->select_click_cnt[PANEL_SEL_PRIMARY] = 0;
    }
    if (dp->flags & PTXT_READ_ONLY)
	return;

    dp->flags &= ~TEXT_SELECTED;

    if (panel_event_is_xview_semantic(event)) {
	switch (event_action(event)) {
	  case ACTION_COPY:
	    if (event_is_up(event))
		/* Copy the primary selection data to the clipboard */
#ifdef OW_I18N
		text_set_clipboard(panel, ip, dp);
#else
		text_set_clipboard(panel, ip);
#endif
	    return;
	  case ACTION_CUT:
	  case ACTION_PASTE:
#ifdef OW_I18N
	    if (event_is_down(event))
		if (panel->preedit_item && 
			*panel->preedit->text->string.wide_char)
		    panel_implicit_commit(ip);
#endif /* OW_I18N */
	    if (event_is_down(event))
		return;
	    break;
	  case ACTION_UNDO:
	    break;
	  default:
	    return;
	}
    }

    switch (dp->notify_level) {
      case PANEL_ALL:
	notify_desired = TRUE;
	break;
      case PANEL_SPECIFIED:
#ifdef OW_I18N
        if (ie_string_wc != NULL) {
            notify_desired = (((wchar_t *)wschr(dp->terminators_wc, 
                                ie_string_wc[0]) != 0) && 
                                event_is_down(event));
        }
        else if (iswascii(event_action(event))) {
              tmp_char = event_action(event);
              tmp_char_wc = (wchar_t *) xv_malloc(sizeof(wchar_t));
              mbtowc(tmp_char_wc, &tmp_char, MB_CUR_MAX);
              notify_desired = (((wchar_t *) wschr(dp->terminators_wc,
                                      tmp_char_wc[0]) != 0) && 
                                      event_is_down(event));
	      xv_free(tmp_char_wc);
           }
#else
	notify_desired = (event_action(event) <= ISO_LAST &&
			  (strchr(dp->terminators, event_action(event)) != 0) &&
			  event_is_down(event));
#endif /* OW_I18N */
	break;
      case PANEL_NON_PRINTABLE:
	notify_desired = !panel_printable_char(event_action(event));
	break;
      case PANEL_NONE:
	notify_desired = FALSE;
	break;
    }
    if (notify_desired) {
	notify_rtn_code = (Panel_setting)
	    (*ip->notify) (ITEM_PUBLIC(ip), event);
    } else {
	notify_rtn_code = panel_text_notify(ITEM_PUBLIC(ip), event);
    }
#ifdef OW_I18N
    /*  FIX ME:  when XIM passes KeyPress events
     *  Currently only the KeyRelease events of the left
     *  function keys are returned by XIM, so I had to
     *  add ACTION_UNDO at the bottom of the test
     *  to prevent jumping out of the function before
     *  ACTION_UNDO is process.
     */
    if (event_is_up(event) &&
	notify_rtn_code != PANEL_NEXT &&
	notify_rtn_code != PANEL_PREVIOUS &&
	event_action(event) != ACTION_PASTE &&
	event_action(event) != ACTION_CUT)
	/* Primary CUT is handled above under "if (dp->delete_pending)".
	 * Primary PASTE, Secondary PASTE (i.e., Quick Copy) and Secondary CUT
	 * (i.e., Quick Move) are handled in update_value().  update_value()
	 * will ignore Primary CUT.  So, both PASTE-up and CUT-up events are
	 * passed on here.
	 */
	return;
#else
    if (event_is_up(event) &&
	notify_rtn_code != PANEL_NEXT &&
	notify_rtn_code != PANEL_PREVIOUS &&
	event_action(event) != ACTION_PASTE &&
	event_action(event) != ACTION_CUT)
	/* Primary CUT is handled above under "if (dp->delete_pending)".
	 * Primary PASTE, Secondary PASTE (i.e., Quick Copy) and Secondary CUT
	 * (i.e., Quick Move) are handled in update_value().  update_value()
	 * will ignore Primary CUT.  So, both PASTE-up and CUT-up events are
	 * passed on here.
	 */
	return;
#endif /* OW_I18N */
    ok_to_insert = notify_rtn_code == PANEL_INSERT;

    /* Process movement actions that could change lines */
    switch (event_action(event)) {
      /* Note: ACTION_GO_LINE_FORWARD is a SunView1 compatibility action */
      case ACTION_GO_LINE_FORWARD:  /* Go to the start of the next line */
	notify_rtn_code = PANEL_NEXT;
	ok_to_insert = FALSE;
	break;
      case ACTION_GO_CHAR_FORWARD:   /* right arrow */
      case ACTION_GO_CHAR_BACKWARD:  /* left arrow */
	ok_to_insert = FALSE;
	break;
      default:
	break;
    }

    /* If this item has the caret, then turn off the caret.
     * Note: has_caret is calculated after the text item's notify proc is
     *       called since the notify proc may change who has the caret.
     */
    has_caret = panel->status.has_input_focus && panel->kbd_focus_item == ip;

    /*
     * Do something with the character.  We must turn off the caret
     * before moving the caret_offset.
     */
    if (has_caret)
	paint_caret(ip, FALSE);
#ifdef  OW_I18N
        /* if the event is caused by committed string from cm, this
         * ie_string is always non NULL.
         */
        if (ie_string_wc != NULL) {
	    wc_ptr = ie_string_wc;
	    while (*wc_ptr)
		update_value(ip, TRUE, *wc_ptr++, ok_to_insert,
			     FALSE, &retstatus);
        }
        else
	    update_value(ip, FALSE, event_action(event), ok_to_insert,
			 FALSE, &retstatus);
#else
    update_value(ip, event_action(event), ok_to_insert, FALSE, &retstatus);
#endif /* OW_I18N */
    if (has_caret)
	paint_caret(ip, TRUE);

    if (has_caret) {
	switch (notify_rtn_code) {
	  case PANEL_NEXT:
	    (void) panel_advance_caret(PANEL_PUBLIC(panel));
	    break;

	  case PANEL_PREVIOUS:
	    (void) panel_backup_caret(PANEL_PUBLIC(panel));
	    break;

	  default:
	    break;
	}
    }
#ifdef OW_I18N
    if (ie_string_wc != NULL)
        xv_free (ie_string_wc);
#endif
}


static void
text_clear(item_public)
    Panel_item	    item_public;
{
    Text_info	   *dp = TEXT_PRIVATE(item_public);
    Item_info      *ip = ITEM_PRIVATE(item_public);

    if (ip->panel->kbd_focus_item == ip)
    {
        paint_caret(ip, FALSE); 
	ip->panel->caret_on = FALSE;
    }
    panel_default_clear_item(item_public);
    if (dp->flags & TEXT_HIGHLIGHTED) {
	seln_highlight = HL_NONE;
	dp->flags &= ~TEXT_HIGHLIGHTED;
    }
}


static void
text_paint(item_public)
    Panel_item	    item_public;
{
    Item_info      *ip = ITEM_PRIVATE(item_public);

#ifdef OW_I18N
    int		    has_caret = ip->panel->kbd_focus_item == ip;
#endif /* OW_I18N */

    panel_text_paint_label(ip);

#ifdef OW_I18N
    if (has_caret && ip->panel->preedit_item)
        ml_panel_display_interm(ip);
    else
#endif  /* OW_I18N */
	paint_text(ip);
}


static void
text_remove(item_public)
    Panel_item	    item_public;
{
    Text_info	   *dp = TEXT_PRIVATE(item_public);
    int		    i;
    Item_info      *ip = ITEM_PRIVATE(item_public);
    Panel_info	   *panel = ip->panel;

    if (dp->flags & PTXT_READ_ONLY)
	return;

    /* Cancel any selections this item owns. */
    for (i = 0; i < NBR_PANEL_SELECTIONS; i++) {
	if (panel->sel_holder[i] == ip)
	    xv_set(panel->sel_owner[i], SEL_OWN, FALSE, NULL);
    }
	       
    /*
     * Only reassign the keyboard focus to another item if the panel isn't
     * being destroyed.
     */
    if (!panel->status.destroying)
    {
        if (panel->kbd_focus_item == ip)
        {
            /*
             * The caret is cleared, so don't paint the caret off.
             * However, mark that no caret is on.
             */
            panel->caret_on = FALSE;
            if (panel->primary_focus_item == ip)
                panel->primary_focus_item = NULL;
            panel->kbd_focus_item = panel_next_kbd_focus(panel, TRUE);
            if (panel->kbd_focus_item)
            {
                if (panel->kbd_focus_item->item_type == PANEL_TEXT_ITEM)
                {
                    paint_caret(panel->kbd_focus_item, TRUE);
                }
                else
                {
                    panel_accept_kbd_focus(panel);
                }
            }
        }
        /*
         * The primary_focus_item can be different than the
         * kbd_focus_item. Therefore, ifwe delete the
         * primary_focus_item, then we've got to set this field
         * to NULL - fixes bug 1077902
         */
        if (panel->primary_focus_item == ip)
            panel->primary_focus_item = NULL;
    }
    return;
}


/*
 * text_restore
 *
 * Note: This code assumes that the caller has already set ip to be
 *	 not hidden.
 */
static void
text_restore(item_public)
    Panel_item	    item_public;
{
    Text_info	   *dp = TEXT_PRIVATE(item_public);
    Item_info      *ip = ITEM_PRIVATE(item_public);

    if (dp->flags & PTXT_READ_ONLY || hidden(ip))
	return;

    /* see whether selection client is initialized */
    /* may not be if restore is called because of reparent */
    if (!ip->panel->sel_owner[PANEL_SEL_PRIMARY])
	text_seln_init(ip->panel);

    /* If this is the only item, give the caret to this item. */
    if (ip->panel->items == ip && !ip->next) {
	ip->panel->kbd_focus_item = ip;
    }
    return;
}


/*ARGSUSED*/
static void
text_layout(item_public, deltas)
    Panel_item	    item_public;
    Rect           *deltas;
{
    Text_info	   *dp = TEXT_PRIVATE(item_public);
    Item_info      *ip = ITEM_PRIVATE(item_public);

    xv_set(dp->drop_site,
	   DROP_SITE_DELETE_REGION, NULL,
	   DROP_SITE_REGION, &ip->value_rect,
	   NULL);
}


static void
text_accept_kbd_focus(item_public)
    Panel_item	    item_public;
{
    Text_info	   *dp = TEXT_PRIVATE(item_public);
    Item_info      *ip = ITEM_PRIVATE(item_public);

    if (ip->panel->status.painted)
	paint_caret(ip,
	    (dp->flags & SELECTING_ITEM && dp->delete_pending) ? FALSE : TRUE);
}


static void
text_yield_kbd_focus(item_public)
    Panel_item	    item_public;
{
    Item_info      *ip = ITEM_PRIVATE(item_public);

    if (ip->panel->status.painted)
	paint_caret(ip, FALSE);
}



/* --------------------  Local Routines  -------------------- */

/* Find the position of the character to the left of caret_offset */
static int
char_position(caret_offset, font, str, balance_beam)
    int             caret_offset;
    Xv_Font         font;
    CHAR           *str;
    int		    balance_beam;  /* TRUE = return index of char to the
				    * right if caret_offset is in the
				    * right half of the character.
				    * FALSE = return index of char the
				    * caret offset is within.
				    */
{
#ifdef OW_I18N
    XFontSet        font_set;
#else
    XFontStruct	   *x_font_info;
#endif /*OW_I18N*/
    int             i;		/* character string index */
    int             x = 0;	/* desired horizontal position */
    int             x_new = 0;	/* next horizontal position */

#ifdef OW_I18N
    font_set = (XFontSet) xv_get(font, FONT_SET_ID);
    for (i = 0; x_new <= caret_offset && str[i]; i++) {
	x = x_new;
        x_new += XwcTextEscapement(font_set, &str[i], 1);
    }
#else
    x_font_info = (XFontStruct *)xv_get(font, FONT_INFO);
    for (i = 0; x_new <= caret_offset && str[i]; i++) {
	x = x_new;
	if (x_font_info->per_char)  {
	    x_new += x_font_info->per_char[(u_char)str[i] -
	    x_font_info->min_char_or_byte2].width;
	} else
	    x_new += x_font_info->min_bounds.width;
    }
#endif /* OW_I18N */

    /* Return character string index */
    if (x_new <= caret_offset)
	return (i);		/* cursor points to the right of the last
				 * character */
    else if (--i < 0)
	return (0);		/* cursor points to the left of the first
				 * character */
    else if (balance_beam && (caret_offset - x) > (x_new - x) / 2)
	return (i + 1);		/* cursor is in right half of char; point to
				 * next char */
    else
	return (i);		/* cursor is in left half of char, or not
				 * using "balance beam" method; point to
				 * this char */
}


static void
draw_scroll_btn(ip, state)
    register Item_info *ip;
    int		state;
{
    register Text_info *dp = TEXT_FROM_ITEM(ip);
    Xv_Drawable_info *info;
    Xv_Window	    pw;

    PANEL_EACH_PAINT_WINDOW(ip->panel, pw)
	DRAWABLE_INFO_MACRO(pw, info);
	if (state & OLGX_SCROLL_BACKWARD) {
	    /* Draw left scrolling button */
	    olgx_draw_textscroll_button(ip->panel->ginfo, xv_xid(info),
		ip->value_rect.r_left,
		ip->value_rect.r_top +
		    (ip->value_rect.r_height - dp->scroll_btn_height) / 2,
		state);
	} else {
	    /* Draw right scrolling button */
	    olgx_draw_textscroll_button(ip->panel->ginfo, xv_xid(info),
		ip->value_rect.r_left + ip->value_rect.r_width -
		    (dp->scroll_btn_width - SCROLL_BTN_GAP),
		ip->value_rect.r_top +
		    (ip->value_rect.r_height - dp->scroll_btn_height) / 2,
		state);
	}
    PANEL_END_EACH_PAINT_WINDOW
}

/*
  'horizontal_scroll' is only called when the user has
   pressed the select button over the scrollbutton (with
   autoscroll functionality, 'horizontal scroll' is also
   called when the select button is held down over the
   button).

   'update_value_offset' determines the first and last
   character visible (dp->first_char && dp->last_char)
   and also determines the delta (dp->value_offset)
   between the first and last visible character.

   'update_caret_offset' adjusts the caret offset
   (dp->caret_offset) from the left edge of the first
   visible character and the caret position
   (dp->caret_position), which is the index from the
   first character (*not* first visible character) in
   the text item string.
*/
	
static void
horizontal_scroll(ip, shift)
    register Item_info *ip;
    int             shift;	/* number of characters to shift value and
				 * caret: <0 for shift left, >0 for shift
				 * right. */
{
    update_value_offset(ip, 0, shift, 0);
    update_caret_offset(ip, 0, 1);
    paint_value(ip, PV_HIGHLIGHT );
}


#ifdef PAINT_BOX
static void
paint_box(ip, pw)
    Item_info	   *ip;
    Xv_Window	    pw;
{
    GC             *gc_list;
    Xv_Drawable_info *info;

    DRAWABLE_INFO_MACRO(pw, info);
    gc_list = (GC *)xv_get(xv_screen(info), SCREEN_OLGC_LIST, pw);
    screen_adjust_gc_color(pw, SCREEN_SET_GC);
    XDrawRectangle(xv_display(info), xv_xid(info), gc_list[SCREEN_SET_GC],
		   ip->value_rect.r_left, ip->value_rect.r_top,
		   ip->value_rect.r_width-1, ip->value_rect.r_height-1);
}
#endif	/* PAINT_BOX */


static void
paint_caret(ip, on)
    Item_info      *ip;
    int             on;
{
    register Panel_info *panel = ip->panel;
    register Text_info *dp = TEXT_FROM_ITEM(ip);
    Display	   *display;
    XGCValues	    gc_values;
    Xv_Drawable_info *info;
    int		    max_x;
    Rect           *r;
    int             painted_caret_offset;
    Xv_Window       pw;
    XID		    pw_xid;
    char	    str[2];
    int    	    x;
    int		    y;
    Xv_Screen      screen;
    GC             *gc_list;
#ifdef OW_I18N
#ifdef FULL_R5
    XPoint		loc;
    XRectangle	        x_rect;
    XVaNestedList	va_nested_list;
#endif /* FULL_R5 */
#endif /* OW_I18N */		

    if ((on && (panel->caret_on || (dp->flags & TEXT_SELECTED))) ||
	(!on && !panel->caret_on) ||
	panel->caret == XV_ZERO)
	return;
    panel->caret_on = on;

    /* paint the caret after the offset & above descender */
#ifdef OW_I18N
    painted_caret_offset = dp->mask_wc ? 0 : dp->caret_offset;
#else
    painted_caret_offset = dp->mask ? 0 : dp->caret_offset;
#endif /* OW_I18N */
    x = ip->value_rect.r_left + painted_caret_offset - panel->caret_width/2;
    if (dp->first_char)
	x += dp->scroll_btn_width;
    y = ip->value_rect.r_top + dp->font_home;

    PANEL_EACH_PAINT_WINDOW(panel, pw)
	r = panel_viewable_rect(panel, pw);
	max_x = r->r_left + r->r_width;
	if (x <= max_x - panel->caret_width/2) {
	    DRAWABLE_INFO_MACRO(pw, info);
	    screen = xv_screen(info);
	    gc_list = (GC *)xv_get(screen, SCREEN_OLGC_LIST, pw);
	    display = xv_display(info);
	    pw_xid = xv_xid(info);
	    if (panel->caret_on) {
		/* Save pixels that will be overwritten by caret */
		XSync(display, False);  /* insure last paint_caret operation
					 * is painted on the screen */
		XCopyArea(display, pw_xid, panel->caret_bg_pixmap,
			  gc_list[SCREEN_SET_GC],
			  x, y - panel->caret_ascent,
			  panel->caret_width, panel->caret_height,
			  0, 0);
		/* Write caret in foreground color */
		gc_values.foreground = xv_fg(info);
		XChangeGC(display, gc_list[SCREEN_GLYPH_GC],
			  GCForeground, &gc_values);
		str[0] = panel->caret;
		str[1] = XV_ZERO;
		XDrawString(display, pw_xid, gc_list[SCREEN_GLYPH_GC],
			    x, y, str, 1);
#ifdef OW_I18N
#ifdef FULL_R5
    		if (panel->ic && (panel->xim_style &  XIMPreeditPosition )) {          		
     		    loc.x = (short)x + (panel->caret_width/2);
         	    loc.y = (short)y;
        	    x_rect.x = ip->value_rect.r_left;
                    x_rect.y = ip->value_rect.r_top;
                    x_rect.width = ip->value_rect.r_width;
                    x_rect.height = ip->value_rect.r_height;

        	    va_nested_list = XVaCreateNestedList(NULL, 
					     XNSpotLocation, &loc,
					     XNArea, &x_rect,
					     NULL);
        	    XSetICValues(panel->ic, XNPreeditAttributes, va_nested_list,
        	     		 NULL);
        	    XFree(va_nested_list);
    		}
#endif /* FULL_R5 */	    		
#endif /* OW_I18N */	
			    
	    } else {
		/* Restore pixels that were overwritten by caret */
		XCopyArea(display, panel->caret_bg_pixmap, pw_xid,
			  gc_list[SCREEN_SET_GC],
			  0, 0, panel->caret_width, panel->caret_height,
			  x, y - panel->caret_ascent);
	    }
	}
    PANEL_END_EACH_PAINT_WINDOW
}


static void
paint_text(ip)
    Item_info      *ip;
{
    /* compute the caret position */
    update_value_offset(ip, 0, 0, 1);
    update_caret_offset(ip, 0, 0);

    paint_value(ip, PV_HIGHLIGHT );
}


/*
 * paint_value clears the value rect for ip and paints the string value
 * clipped to the left of the rect.
 */
static void
paint_value(ip, highlight)
    register Item_info *ip;
    int          highlight;
{
    register Text_info *dp;
    register Panel_info *panel = ip->panel;
    int             i, j, len;
    Xv_Drawable_info *info;
    Xv_Window       pw;
    CHAR           *str;
    int		    x;
    int		    y;
    int		    btnop;

    if (!panel->status.painted)
	return;
    dp = TEXT_FROM_ITEM(ip);
    x = ip->value_rect.r_left;
    y = ip->value_rect.r_top;

    /* Clear the caret and value rect.
     * Set the colors for the Clear and Set GC's.
     */
    PANEL_EACH_PAINT_WINDOW(panel, pw)
	DRAWABLE_INFO_MACRO(pw, info);
	screen_adjust_gc_color(pw, SCREEN_SET_GC);
	panel_clear_pw_rect(pw, ip->value_rect);
    PANEL_END_EACH_PAINT_WINDOW
    if (panel->kbd_focus_item == ip)
	/* Since part of the caret may lie outside the value rect,
	 * we need to call paint_caret to make sure all of the
	 * caret has been removed.
	 */
	paint_caret(ip, FALSE);

    /* Get the actual characters which will be displayed */
    len = dp->last_char - dp->first_char + 2;
    if (len) {
	str = (CHAR *) xv_malloc(len*sizeof(CHAR));
	for (j = 0, i = dp->first_char; i <= dp->last_char; i++, j++)
#ifdef OW_I18N
	    str[j] = dp->value_wc[i];
#else
	    str[j] = dp->value[i];
#endif /* OW_I18N */
	str[len - 1] = '\0';

	/* Draw the left scrolling button if needed */
	if (dp->first_char)
        {
            btnop = OLGX_SCROLL_BACKWARD;
            if (dp->flags & LEFT_SCROLL_BTN_SELECTED)
                btnop |= OLGX_INVOKED ;
            draw_scroll_btn(ip, btnop );
        }

	/* Draw the text */
	if (dp->first_char)
	    x += dp->scroll_btn_width;
#ifdef OW_I18N
	if (dp->mask_wc == (wchar_t)'\0') /* not masked */
#else
	if (dp->mask == '\0') /* not masked */
#endif /* OW_I18N */
	{
	    PANEL_EACH_PAINT_WINDOW(panel, pw)
#ifdef OW_I18N
		panel_paint_text(pw, ip->value_fontset_id, 
				ip->color_index,
				x, y+dp->font_home, str);
#else
		panel_paint_text(pw, ip->value_font_xid, ip->color_index,
				 x, y+dp->font_home, str);
#endif /* OW_I18N */
	    PANEL_END_EACH_PAINT_WINDOW
	} else {		/* masked */
	    CHAR           *buf;
	    int             length, i;
	    length = dp->last_char - dp->first_char + 2;
	    buf = (CHAR *) xv_malloc(length*sizeof(CHAR));
	    for (j = 0, i = dp->first_char; i <= dp->last_char; i++, j++)
#ifdef OW_I18N
		buf[j] = dp->mask_wc; buf[length - 1] = '\0';
#else
		buf[j] = dp->mask; buf[length - 1] = '\0';
#endif /* OW_I18N */

	    PANEL_EACH_PAINT_WINDOW(panel, pw)
#ifdef OW_I18N
		panel_paint_text(pw, ip->value_fontset_id, 
				 ip->color_index,
				 x, y+dp->font_home, buf);
#else
		panel_paint_text(pw, ip->value_font_xid, ip->color_index,
				 x, y+dp->font_home, buf);
#endif /* OW_I18N */
	    PANEL_END_EACH_PAINT_WINDOW
	    free(buf);
	}

	/* Draw the right scrolling button if needed */
#ifdef OW_I18N
	if (dp->last_char < ((int)wslen(dp->value_wc) - 1))
#else
	if (dp->last_char < ((int)strlen(dp->value) - 1))
#endif /* OW_I18N */
        {
            btnop = OLGX_SCROLL_FORWARD;
            if (dp->flags & RIGHT_SCROLL_BTN_SELECTED)
                btnop |= OLGX_INVOKED ;
            draw_scroll_btn(ip, btnop );
        }

	free((char *) str);

    }
    /* Underline the text (optional) */
    if (dp->flags & UNDERLINED) {
	y = rect_bottom(&ip->value_rect);
	if (ip->panel->status.three_d) {
	    /* 3D text ledge is 2 pixels high.  (2D is 1 pixel high.) */
	    y--;
	}
	PANEL_EACH_PAINT_WINDOW(panel, pw)
	    DRAWABLE_INFO_MACRO(pw, info);
	    olgx_draw_text_ledge(panel->ginfo, xv_xid(info),
	        ip->value_rect.r_left, y,
		ip->value_rect.r_width);
	PANEL_END_EACH_PAINT_WINDOW
    }
    if (highlight) {
	/* re-highlight if this is a selection item */
	if (ip == panel->sel_holder[PANEL_SEL_PRIMARY])
	    text_seln_highlight(panel, ip, PANEL_SEL_PRIMARY);
	if (ip == panel->sel_holder[PANEL_SEL_SECONDARY])
	    text_seln_highlight(panel, ip, PANEL_SEL_SECONDARY);
    }

#ifdef PAINT_BOX
    paint_box(ip, pw);	/* ... used to debug painting problems */
#endif	/* PAINT_BOX */

    if (panel->kbd_focus_item == ip)
	paint_caret(ip, TRUE);

    if (inactive(ip)) {
	Xv_Screen      screen;
	GC             *gc_list;
	DRAWABLE_INFO_MACRO(pw, info);
	screen = xv_screen(info);
	gc_list = (GC *)xv_get(screen, SCREEN_OLGC_LIST, pw);
	screen_adjust_gc_color(pw, SCREEN_INACTIVE_GC);
	XFillRectangle(xv_display(info), xv_xid(info),
		       gc_list[SCREEN_INACTIVE_GC],
		       ip->value_rect.r_left, ip->value_rect.r_top,
		       ip->value_rect.r_width, ip->value_rect.r_height);
    }
}


static void
panel_find_word(dp, first, last)
    register Text_info *dp;
    int            *first, *last;
{
    register int    index;
#ifdef OW_I18N
    int		    wc_type;


    /*
     * When the selected character is in the only ASCII set, then
     * go through the original algorithm
     */
    wc_type = wchar_kind(dp->value_wc[*first]);
    if (iswascii(dp->value_wc[*first])) {
	/*
	 * In the ASCII case, we will utilizes wchar_kind and
	 * delim_table together.
	 */
	/* Find beginning of word */
	index = *first;
	while ((index > dp->first_char)
	       && (wchar_kind(dp->value_wc[index]) == wc_type)
	       && (!delim_table[dp->value_wc[index]]))
		index--;
	if ((index != *first)
	    && ((wchar_kind(dp->value_wc[index]) != wc_type)
	        || (delim_table[dp->value_wc[index]])))
	    index++;/* don't include characters from another codeset */
	*first = index;

	/*
	 * Find end of word.  Note that on a single click, seln_last is set equal
	 * to seln_first.
	 */
	index = *last;
	while ((index < dp->last_char)
	       && (wchar_kind(dp->value_wc[index]) == wc_type)
	       && (!delim_table[dp->value_wc[index]]))
		index++;
	if ((index != *last)
	    && ((wchar_kind(dp->value_wc[index]) != wc_type)
	        || (delim_table[dp->value_wc[index]])))
	    index--; /* don't include characters from another codeset */
	*last = index;
    }
    else {
	/*
	 * In the NONE ASCII case, we will be soley rely on the
	 * wchar_kind.
	 */
	/* Find beginning of word */
	index = *first;
	while ((index > dp->first_char)
	       && (wchar_kind(dp->value_wc[index]) == wc_type))
	    index--;
	if ((index != *first)
	    && (wchar_kind(dp->value_wc[index]) != wc_type))
	    index++;
	*first = index;

	/*
	 * Find end of word.
	 */
	index = *last;
	while ((index < dp->last_char)
	       && (wchar_kind(dp->value_wc[index]) == wc_type))
	    index++;
	if ((index != *last)
	    && (wchar_kind(dp->value_wc[index]) != wc_type))
	    index--;
	*last = index;
    }

#else /* OW_I18N */

	/* Find beginning of word */
	index = *first;
	while ((index > dp->first_char) &&
	       !delim_table[(u_char)dp->value[index]])
	    index--;
	if ((index != *first) && delim_table[(u_char)dp->value[index]])
	    index++;		/* don't include word delimiter */
	*first = index;

	/*
	 * Find end of word.  Note that on a single click, seln_last is set equal
	 * to seln_first.
	 */
	index = *last;
	while ((index < dp->last_char) &&
	       !delim_table[(u_char)dp->value[index]])
	    index++;
	if ((index != *last) && delim_table[(u_char)dp->value[index]])
	    index--;		/* don't include word delimiter */
	*last = index;
#endif /* OW_I18N */
}


static void
panel_multiclick_handler(ip, event, rank)
    Item_info      *ip;
    Event          *event;
    int             rank;   /* PANEL_SEL_PRIMARY or PANEL_SEL_SECONDARY */
{
    register Text_info *dp = TEXT_FROM_ITEM(ip);
    int             left, right;/* left and right pixel coordinates of
				 * selection */
    struct pr_size  size;

    if (dp->select_click_cnt[rank] == 2) {
	/* Double click received: select displayed word containing caret */
	panel_find_word(dp, &dp->seln_first[rank],
			&dp->seln_last[rank]);
	/*
	 * Set caret offset to end of selected word or line closest to mouse
	 * position.
	 */
#ifdef OW_I18N
        size.x = XwcTextEscapement(ip->value_fontset_id,
				&dp->value_wc[dp->first_char],
				dp->seln_first[rank] - dp->first_char);
#else
	size = xv_pf_textwidth(dp->seln_first[rank] - dp->first_char,
			       ip->value_font, &dp->value[dp->first_char]);
#endif /* OW_I18N */
	left = ip->value_rect.r_left + size.x;
	if (dp->first_char)
	    left += dp->scroll_btn_width;
#ifdef OW_I18N
	size.x = XwcTextEscapement(ip->value_fontset_id,
				   &dp->value_wc[dp->seln_first[rank]],
				   dp->seln_last[rank] -
				   dp->seln_first[rank] + 1);
#else
	size = xv_pf_textwidth(dp->seln_last[rank] -
			       dp->seln_first[rank] + 1,
			       ip->value_font,
			       &dp->value[dp->seln_first[rank]]);
#endif /* OW_I18N */
	right = left + size.x;
	if ((event->ie_locx - left) < (right - event->ie_locx))
	    event->ie_locx = left;	/* caret will move to left side */
	else
	    event->ie_locx = right;	/* caret will move to right side */
	dp->caret_offset = event->ie_locx - ip->value_rect.r_left;
	if (dp->first_char)
	    dp->caret_offset -= dp->scroll_btn_width;
    } else {
	/*
	 * Triple click received: select entire line. Repaint the value, with
	 * the first or last character displayed, if the mouse is closer to
	 * the left or right margin, respectively.
	 */
	panel_select_line(ip, event, rank);
	update_text_rect(ip);
	paint_value(ip, PV_HIGHLIGHT);
    }

}


Pkg_private int
panel_printable_char(code)
    int             code;	/* event action code */
{

#ifdef OW_I18N
    return (iswprint ((wchar_t) code));
#else
    return((code >= ' ' && code <= '~')
	    || (code >= 0xA0 && code <= 0xFF)
	  );
#endif /* OW_I18N */
}


static void
panel_select_line(ip, event, rank)
    Item_info      *ip;
    Event          *event;	/* NULL => position caret at end of line */
    int             rank;	/* PANEL_SEL_PRIMARY or PANEL_SEL_SECONDARY */
{
    register Text_info *dp = TEXT_FROM_ITEM(ip);
    int             max_caret = dp->text_rect.r_width;
    int             right;	/* horizontal offset of right margin of
				 * selection */
    int             left_offset;/* horizontal offset of cursor from left
				 * margin of text value */
    int             right_offset;	/* horizontal offset of cursor from
					 * right margin of text value */
    int             x, i;
    struct pr_size  size;

    dp->first_char = dp->seln_first[rank] = 0;
#ifdef OW_I18N
    dp->last_char = dp->seln_last[rank] = wslen(dp->value_wc) - 1;
    right = dp->text_rect.r_width;
    size.x = XwcTextEscapement(ip->value_fontset_id, dp->value_wc,
			       wslen(dp->value_wc));
#else
    dp->last_char = dp->seln_last[rank] = strlen(dp->value) - 1;
    right = dp->text_rect.r_width;
    size = xv_pf_textwidth(strlen(dp->value), ip->value_font, dp->value);
#endif /* OW_I18N */
    if (size.x < right)
	right = size.x;
    if (event) {
	left_offset = event_x(event) - dp->text_rect.r_left;
	right_offset = right + dp->text_rect.r_left - event_x(event);
    }
    if (event && left_offset < right_offset) {
	/*
	 * Repaint (later) with first character displayed. dp->last_char =
	 * the last character in the string, starting from first_char, that
	 * can be fully displayed within the rectangle reserved for the text
	 * value string.
	 */
	dp->caret_offset = 0;
#ifdef OW_I18N
	dp->last_char = char_position(right, ip->value_font, dp->value_wc,
	    TRUE) - 1;
#else
	dp->last_char = char_position(right, ip->value_font, dp->value,
	    TRUE) - 1;
#endif /* OW_I18N */
	update_value_offset(ip, 0, 0, 1);  /* fix dp->last_char to account for
					   scrolling button */
    } else {
#ifdef OW_I18N
	x = 0;
	for (i = dp->last_char; (i >= 0) && (x < max_caret); i--)  {
            x += XwcTextEscapement(ip->value_fontset_id, 
				   &dp->value_wc[i], 1);
	}
#else
	XFontStruct	*x_font_info;

	x_font_info = (XFontStruct *)xv_get(ip->value_font, FONT_INFO);
	/* Repaint (later) with last character displayed */
	x = 0;
	for (i = dp->last_char; (i >= 0) && (x < max_caret); i--)  {
		if (x_font_info->per_char)  {
		    x += x_font_info->per_char[(u_char)dp->value[i] -
		        x_font_info->min_char_or_byte2].width;
		} else
		    x += x_font_info->min_bounds.width;
        }
#endif /* OW_I18N */
	if (i >= 0)
	    dp->first_char = i + 2;
	dp->caret_offset = ip->value_rect.r_width;
	if (dp->first_char)
	    dp->caret_offset -= dp->scroll_btn_width;
	/* Caret cannot exceed last character of value */
	if (dp->caret_offset > dp->value_offset)
	    dp->caret_offset = dp->value_offset;
    }
}


/*
 * panel_text_caret_on paints the type-in caret if on is true;
 * otherwise it restores the pixels underneath the caret.
 */
Pkg_private void
panel_text_caret_on(panel, on)
    Panel_info     *panel;
    int             on;
{
    if (!panel->kbd_focus_item)
	return;

    paint_caret(panel->kbd_focus_item, on);
}


/* ARGSUSED */
Xv_public       Panel_setting
panel_text_notify(client_item, event)
    Panel_item      client_item;
    register Event *event;
{

#ifdef OW_I18N
    /*
     * OW_I18N case can not use panel_printable_char, since it is not
     * always wide character.  In case of the iso_8859_1 chars, we
     * need to make sure value is not more than 0xff in order to use
     * isprint(3) macro.
     */
    if (panel_erase_action(event) ||
	(event_action(event) < 0xff && isprint(event_action(event))) ||
	event_is_string(event) ||
#else
    if (panel_erase_action(event) ||
	panel_printable_char(event_action(event)) ||
#endif /* OW_I18N */
	event_action(event) == ACTION_CUT ||
	event_action(event) == ACTION_PASTE ||
	event_action(event) == ACTION_UNDO)
	return PANEL_INSERT;
    else if (event_is_down(event) &&
	     (event_action(event) == xv_iso_next_element ||
	      event_action(event) == '\r' ||
	      event_action(event) == '\n'))
	return (event_shift_is_down(event) ? PANEL_PREVIOUS : PANEL_NEXT);
    else
	return PANEL_NONE;
}


Pkg_private void
panel_text_paint_label(ip)
    register Item_info *ip;
{
    Rect            text_label_rect;
    struct pr_size  image_size;
    int             image_width;

    text_label_rect = ip->label_rect;
    if (ip->label_width) {
	if (is_string(&ip->label)) {
#ifdef OW_I18N
	    image_size = xv_pf_textwidth_wc(wslen(image_string_wc(&ip->label)),
			  image_font(&ip->label), image_string_wc(&ip->label));
#else
	    image_size = xv_pf_textwidth(strlen(image_string(&ip->label)),
			  image_font(&ip->label), image_string(&ip->label));
#endif /* OW_I18N */
	    image_width = image_size.x;
	} else
	    image_width = ((Pixrect *)image_svrim(&ip->label))->pr_width;
	text_label_rect.r_left += ip->label_rect.r_width - image_width;
    }
    panel_paint_image(ip->panel, &ip->label, &text_label_rect, inactive(ip),
		      ip->color_index);
}


/*
 * This is to fix the flashing problem during dragging in primary selection
 * of panel text item. Instead of dehiliting the old selection and then
 * hiliting the new one (which is a continuation of the old one), just invert
 * the difference in selection.  That is either hiliting more if selection
 * has grown, or dehiliting if selection has shrunk.
 */
static void
text_add_selection(panel, ip)
    Panel_info	   *panel;
    Item_info      *ip;
{
    Text_info      *dp = TEXT_FROM_ITEM(ip);
    Rect            rect;
    struct pr_size  size;
    int             diff_first;	/* index of first character to invert */
    int		    diff_last;	/* index of last character to invert */

    rect = ip->value_rect;

    if ((dp->seln_first[PANEL_SEL_PRIMARY] == primary_seln_first) &&
	(dp->seln_last[PANEL_SEL_PRIMARY] == primary_seln_last))
	return;			/* no change */

    if (dp->seln_first[PANEL_SEL_PRIMARY] == primary_seln_first) {
	if (dp->seln_last[PANEL_SEL_PRIMARY] > primary_seln_last) {
	    /* more at the end */
	    diff_first = primary_seln_last + 1;
	    diff_last = dp->seln_last[PANEL_SEL_PRIMARY];
	} else if (dp->seln_last[PANEL_SEL_PRIMARY] < primary_seln_last) {
	    /* less at the end */
	    diff_first = dp->seln_last[PANEL_SEL_PRIMARY] + 1;
	    diff_last = primary_seln_last;
	}
    } else if (dp->seln_last[PANEL_SEL_PRIMARY] == primary_seln_last) {
	if (dp->seln_first[PANEL_SEL_PRIMARY] > primary_seln_first) {
	    /* less at the beg */
	    diff_first = primary_seln_first;
	    diff_last = dp->seln_first[PANEL_SEL_PRIMARY] - 1;
	} else if (dp->seln_first[PANEL_SEL_PRIMARY] < primary_seln_first) {
	    /* more at the beg */
	    diff_first = dp->seln_first[PANEL_SEL_PRIMARY];
	    diff_last = primary_seln_first - 1;
	}
    }
    /* Highlight characters bounded by diff_first and diff_last */
    if (diff_first >= dp->first_char) {
#ifdef OW_I18N
	size.x = XwcTextEscapement(ip->value_fontset_id,
			           &dp->value_wc[dp->first_char],
				   (diff_first - dp->first_char));
#else
	size = xv_pf_textwidth((diff_first - dp->first_char),
			       ip->value_font, &dp->value[dp->first_char]);
#endif /* OW_I18N */
	rect.r_left += size.x;
    }
    if (dp->first_char)
	rect.r_left += dp->scroll_btn_width;
#ifdef OW_I18N
    size.x = XwcTextEscapement(ip->value_fontset_id,
			       &dp->value_wc[diff_first],
			       diff_last - diff_first + 1);
#else
    size = xv_pf_textwidth(diff_last - diff_first + 1,
			   ip->value_font, &dp->value[diff_first]);
#endif /* OW_I18N */
    rect.r_width = size.x;
    if (rect.r_width > dp->text_rect.r_width)
	rect.r_width = dp->text_rect.r_width;
    rect.r_height--;		/* don't disturb underlining */

    seln_highlight = HL_INVERT;
    panel_invert(panel, &rect, ip->color_index);

    /* restore the rect */
    rect = ip->value_rect;
    /*
     * Update rect to be bounded by seln_first and seln_last this is
     * necessary so that primary_seln_rect is always the entire selection.
     */
    if (dp->seln_first[PANEL_SEL_PRIMARY] >= dp->first_char) {
#ifdef OW_I18N
        size.x = XwcTextEscapement(ip->value_fontset_id,
			           &dp->value_wc[dp->first_char],
				   dp->seln_first[PANEL_SEL_PRIMARY] 
				   - dp->first_char);
#else
	size = xv_pf_textwidth(
	    dp->seln_first[PANEL_SEL_PRIMARY] - dp->first_char,
	    ip->value_font, &dp->value[dp->first_char]);
#endif /* OW_I18N */
	rect.r_left += size.x;
    }
    if (dp->first_char)
	rect.r_left += dp->scroll_btn_width;
#ifdef OW_I18N
    size.x = XwcTextEscapement(ip->value_fontset_id,
	    	&dp->value_wc[dp->seln_first[PANEL_SEL_PRIMARY]],
		dp->seln_last[PANEL_SEL_PRIMARY] -
                dp->seln_first[PANEL_SEL_PRIMARY] + 1);
#else
    size = xv_pf_textwidth(
	dp->seln_last[PANEL_SEL_PRIMARY] -
	    dp->seln_first[PANEL_SEL_PRIMARY] + 1,
	ip->value_font, &dp->value[dp->seln_first[PANEL_SEL_PRIMARY]]);
#endif /* OW_I18N */
    rect.r_width = size.x;
    if (rect.r_width > dp->text_rect.r_width)
	rect.r_width = dp->text_rect.r_width;
    rect.r_height--;		/* don't disturb underlining */

    /* update globals */
    primary_seln_panel = panel;	/* save panel */
    primary_seln_rect = rect;	/* save rectangle coordinates */
    primary_seln_first = dp->seln_first[PANEL_SEL_PRIMARY];
    primary_seln_last = dp->seln_last[PANEL_SEL_PRIMARY];
}


static void
text_alarm(ip)
    Item_info	   *ip;
{
    Xv_Drawable_info *info;
    struct timeval  wait;

    wait.tv_sec = wait.tv_usec = 0;
    win_bell( PANEL_PUBLIC(ip->panel), wait, 0 );
}


static int
text_convert_proc(sel_own, type, data, length, format)
    Selection_owner sel_own;
    Atom	   *type;
    Xv_opaque	   *data;
    unsigned long  *length;
    int		   *format;
{
    Text_info	   *dp;
    Panel_info	   *panel;
    Atom	    rank_atom;
    int		    rank_index;
#ifdef OW_I18N
    Panel	      panel_public; 
    wchar_t	      saved_char;
#endif /* OW_I18N */

#ifdef OW_I18N
    panel_public = xv_get(sel_own, XV_KEY_DATA, PANEL);
    panel = PANEL_PRIVATE(panel_public);
    rank_atom = (Atom) xv_get(sel_own, SEL_RANK);
    for (rank_index = PANEL_SEL_PRIMARY; rank_index < NBR_PANEL_SELECTIONS;
								rank_index++) {
	if (panel->sel_rank[rank_index] == rank_atom)
		break;
    }
    if (rank_index >= NBR_PANEL_SELECTIONS) {
	/*
	 * Fallback to PANEL_SEL_PRIMARY is necessary for the drag and
	 * drop operation.
	 */
        rank_index = PANEL_SEL_PRIMARY;
    }
    if (panel->sel_holder[rank_index])
        dp = TEXT_FROM_ITEM(panel->sel_holder[rank_index]);
    else
	dp = NULL;
    sdfprintf((stderr, "convert_proc type %d, rank %d, rankatom %d\n",
	       *type, rank_index, rank_atom));

    if (*type == XA_STRING) {
	if (dp == NULL)
	    goto Done;
	if (rank_index == PANEL_SEL_CLIPBOARD) {
	    if (panel->clipboard.value != NULL)
		*data = (Xv_opaque) _xv_wcstombsdup(panel->clipboard.value);
	    else
		goto Done;
	} else {
		if (dp->seln_last[rank_index] < dp->seln_first[rank_index])
		    goto Done;
		saved_char = dp->value_wc[dp->seln_last[rank_index] + 1];
		dp->value_wc[dp->seln_last[rank_index] + 1] = 0;
		*data = (Xv_opaque) _xv_wcstombsdup(
				&dp->value_wc[dp->seln_first[rank_index]]);
		dp->value_wc[dp->seln_last[rank_index] + 1] = saved_char;
	}
	*length = strlen((char *) *data);
	sdfprintf((stderr, "STRING: Sending [%s]\n", *data));
	*format = 8;
	return TRUE;

    } else if (*type == panel->atom.compound_text) {
	Xv_Drawable_info *info;
	int		  state;
	XTextProperty     text_prop;
	wchar_t		 *wcs;

	if (dp == NULL)
	    goto Done;
	if (rank_index == PANEL_SEL_CLIPBOARD) {
	    if (panel->clipboard.value != NULL) {
		wcs = panel->clipboard.value;
		saved_char = 0;
	    } else {
		goto Done;
	    }
	} else {
		if (dp->seln_last[rank_index] < dp->seln_first[rank_index])
		    goto Done;
		saved_char = dp->value_wc[dp->seln_last[rank_index] + 1];
		dp->value_wc[dp->seln_last[rank_index] + 1] = 0;
		wcs = &dp->value_wc[dp->seln_first[rank_index]];
	}
	sdfprintf((stderr, "CTEXT: Sending [%ws]\n", wcs));
	DRAWABLE_INFO_MACRO(panel_public, info);
	state = _xv_XwcTextListToTextProperty(panel_public, PANEL,
					  xv_display(info), &wcs, 1,
					  XCompoundTextStyle, &text_prop);
	if (saved_char != 0)
	    dp->value_wc[dp->seln_last[rank_index] + 1] = saved_char;
	if (state < 0)
	    return FALSE;
	*data = (Xv_opaque) text_prop.value;
	*length = strlen((char *) *data);
	*format = 8;
	return TRUE;

    } else if (*type == panel->atom.delete) {

	text_seln_delete(panel->sel_holder[rank_index], rank_index);

    } else if (*type == panel->atom.selection_end) {

	/* Lose the Secondary Selection */
	xv_set(sel_own, SEL_OWN, FALSE, NULL);

    } else if (*type == panel->atom.seln_yield) {

	/* Lose the Secondary Selection */
	xv_set(sel_own, SEL_OWN, FALSE, NULL);

	if (dp == NULL)
	    goto Done;
        dp->sel_yield_data = SELN_SUCCESS;
        *data = (Xv_opaque) &dp->sel_yield_data;
        *length = 1;
        *format = 32;
        return TRUE;

    } else if (*type == panel->atom.length_chars) {
	if (dp == NULL)
	    goto Done;
	if (rank_index == PANEL_SEL_CLIPBOARD) {
	    if (panel->clipboard.value == NULL)
		dp->sel_length_data = 0;
	    else
		dp->sel_length_data = wslen(panel->clipboard.value);
	} else {
		if (dp->seln_last[rank_index] < dp->seln_first[rank_index]) {
		    dp->sel_length_data = 0;
		} else {
		    dp->sel_length_data = dp->seln_last[rank_index]
					- dp->seln_first[rank_index] + 1;
		}
	}
	sdfprintf((stderr, "Length_Chars %d\n", dp->sel_length_data));
	goto length;

    } else if (*type == panel->atom.length) {
	char	*mbs;

	/*
	 * This is only used by SunView1 selection clients for
	 * clipboard and secondary selections.
	 */
	if (dp == NULL)
	    goto Done;
	if (rank_index == PANEL_SEL_CLIPBOARD) {
	    if (panel->clipboard.value == NULL) {
		    dp->sel_length_data = 0;
		    sdfprintf((stderr, "Length %d\n", dp->sel_length_data));
		    goto length;
	    } else
		mbs = _xv_wcstombsdup(panel->clipboard.value);
	} else {
		if (dp->seln_last[rank_index] < dp->seln_first[rank_index]) {
		    dp->sel_length_data = 0;
		    sdfprintf((stderr, "Length %d\n", dp->sel_length_data));
		    goto length;
		}
		saved_char = dp->value_wc[dp->seln_last[rank_index] + 1];
		dp->value_wc[dp->seln_last[rank_index] + 1] = 0;
		mbs = _xv_wcstombsdup(
				&dp->value_wc[dp->seln_first[rank_index]]);
		dp->value_wc[dp->seln_last[rank_index] + 1] = saved_char;
	}
	dp->sel_length_data = strlen(mbs);
	xv_free(mbs);
	sdfprintf((stderr, "Length %d\n", dp->sel_length_data));
length:
	*data = (Xv_opaque) &dp->sel_length_data;
	*length = 1;
	*format = 32;
	return TRUE;

    }
    sdfprintf((stderr, "Ooops type %d is not implemented yet\n", *type));

    return sel_convert_proc(sel_own, type, data, (unsigned long *)length,
			    format);

#else /* OW_I18N */
    panel = PANEL_PRIVATE(xv_get(sel_own, XV_KEY_DATA, PANEL));
    rank_atom = (Atom) xv_get(sel_own, SEL_RANK);
    if (*type == panel->atom.delete) {

	if (rank_atom == panel->atom.secondary)
	    rank_index = PANEL_SEL_SECONDARY; /* Quick Move */
	else
	    rank_index = PANEL_SEL_PRIMARY;  /* Drag and Drop */
	/* Delete selection */
	text_seln_delete(panel->sel_holder[rank_index], rank_index);

    } else if (*type == panel->atom.selection_end) {

	/* Lose the Secondary Selection */
	xv_set(sel_own, SEL_OWN, FALSE, NULL);

    } else if (*type == panel->atom.seln_yield) {

	/* Lose the Secondary Selection */
	xv_set(sel_own, SEL_OWN, FALSE, NULL);
        if (rank_atom == panel->atom.secondary)
            rank_index = PANEL_SEL_SECONDARY;
        else
            rank_index = PANEL_SEL_CLIPBOARD;
        if (!panel->sel_holder[rank_index])
          goto Done;
        dp = TEXT_FROM_ITEM(panel->sel_holder[rank_index]);
        *type = panel->atom.seln_yield;
        dp->sel_yield_data = SELN_SUCCESS;
        *data = (Xv_opaque) &dp->sel_yield_data;
        *length = 1;
        *format = 32;
        return TRUE;

    } else if (*type == panel->atom.length) {
	/* This is only used by SunView1 selection clients for
	 * clipboard and secondary selections.
	 */
	if (rank_atom == panel->atom.secondary)
	    rank_index = PANEL_SEL_SECONDARY;
	else
	    rank_index = PANEL_SEL_CLIPBOARD;
        if (!panel->sel_holder[rank_index])
          goto Done;
	dp = TEXT_FROM_ITEM(panel->sel_holder[rank_index]);
	dp->sel_length_data =
	    (unsigned long) xv_get(panel->sel_item[rank_index], SEL_LENGTH);
	*data = (Xv_opaque) &dp->sel_length_data;
	*length = 1;
	*format = 32;
	return TRUE;
    } else {
	/* Use default Selection Package convert procedure */
	return sel_convert_proc(sel_own, type, data, (unsigned long *)length,
                                format);
    }
#endif /* OW_I18N */
Done:
    *type = panel->atom.null;
    *data = XV_ZERO;
    *length = 0;
    *format = 32;
    return TRUE;
}


/* Panel Text Item has lost ownership of the selection. */
static void
text_lose_proc(sel_owner)
    Selection_owner sel_owner;
{
    Panel_info	   *panel;
    Atom	    rank_atom;
    int		    rank_index;

    panel = PANEL_PRIVATE(xv_get(sel_owner, XV_KEY_DATA, PANEL));
    rank_atom = (Atom) xv_get(sel_owner, SEL_RANK);
    for (rank_index = 0; rank_index < NBR_PANEL_SELECTIONS; rank_index++) {
	if (rank_atom == panel->sel_rank[rank_index])
	    break;
    }
    if (panel->sel_holder[rank_index])
	text_lose_rank(panel, rank_index);
}


static void
text_lose_rank(panel, rank)
    Panel_info	   *panel;
    int		    rank;
{
    Text_info	   *dp;

    if (!panel->sel_holder[rank])
	return;
    text_seln_dehighlight(panel->sel_holder[rank], rank);
    if (rank <= PANEL_SEL_SECONDARY) {
	dp = TEXT_FROM_ITEM(panel->sel_holder[rank]);
	if (rank == PANEL_SEL_PRIMARY)
	    dp->flags &= ~TEXT_SELECTED;
	dp->select_click_cnt[rank] = 0;
	dp->seln_first[rank] = 0;
	dp->seln_last[rank] = 0;
    }
    panel->sel_holder[rank] = NULL;
}


/* Dehighlight whatever was last highlighted */
static void
text_seln_dehighlight(ip, rank)
    Item_info      *ip;
    int		    rank;
{
    Xv_Drawable_info *info;
    Panel_info     *seln_panel = (Panel_info *) 0;
    Rect           *seln_rect_handle;
    Text_info      *dp = TEXT_FROM_ITEM(ip);
    int             seln_first, seln_last;
    CHAR            save_seln_last_char;
    Xv_Window       pw;

    switch (rank) {
      case PANEL_SEL_PRIMARY:
	seln_panel = primary_seln_panel;
	seln_rect_handle = &primary_seln_rect;
	seln_first = primary_seln_first;
	seln_last = primary_seln_last;
	primary_seln_panel = 0;	/* no longer valid */
	break;
      case PANEL_SEL_SECONDARY:
	seln_panel = secondary_seln_panel;
	seln_rect_handle = &secondary_seln_rect;
	seln_first = secondary_seln_first;
	seln_last = secondary_seln_last;
	secondary_seln_panel = 0;	/* no longer valid */
	break;
    }
    if (seln_panel && dp->flags & TEXT_HIGHLIGHTED) {
	/* Note: The only case where the TEXT_HIGHLIGHTED flag wouldn't be set
	 * is if the PANEL_TEXT item was being hidden (i.e., XV_SHOW being
	 * set to FALSE).  In this case, TEXT_HIGHLIGHTED is cleared in
	 * text_clear.
	 */
	if (seln_highlight == HL_INVERT)
	    panel_invert(seln_panel, seln_rect_handle, ip->color_index);
	else if (ip && (seln_highlight == HL_UNDERLINE ||
			seln_highlight == HL_STRIKE_THRU)) {
#ifdef OW_I18N
        /*  the selected characters actually
         *  taken up, then compare it with
         *  dp->display_width
         */

	    int x;

	    x = XwcTextEscapement(ip->value_fontset_id,  
				       &dp->value_wc[seln_first],
			               (seln_last - seln_first + 1));
            if ((seln_first >= dp->first_char) &&
                (x <= dp->display_width))
#else
	    if ((seln_first >= dp->first_char) &&
		(seln_last - seln_first + 1 <= dp->display_length))
#endif /* OW_I18N */
	    {
		/* ??? variable-width * ??? */
		/* Entire selection is visible */
		panel_clear_rect(ip->panel, *seln_rect_handle);
#ifdef OW_I18N
		save_seln_last_char = dp->value_wc[seln_last + 1];
		dp->value_wc[seln_last + 1] = 0;	/* terminate substring */
#else
		save_seln_last_char = dp->value[seln_last + 1];
		dp->value[seln_last + 1] = 0;	/* terminate substring */
#endif /* OW_I18N */
		PANEL_EACH_PAINT_WINDOW(seln_panel, pw)
#ifdef OW_I18N
		    panel_paint_text(pw, ip->value_fontset_id,
			ip->color_index,
			seln_rect_handle->r_left,
			seln_rect_handle->r_top + dp->font_home,
			dp->value_wc + seln_first);
#else
		    panel_paint_text(pw, ip->value_font_xid,
			ip->color_index,
			seln_rect_handle->r_left,
			seln_rect_handle->r_top + dp->font_home,
			dp->value + seln_first);
#endif /* OW_I18N */
		    if (dp->flags & UNDERLINED && ip->panel->status.three_d) {
			/* Redraw the 3D text ledge */
			DRAWABLE_INFO_MACRO(pw, info);
			olgx_draw_text_ledge(ip->panel->ginfo, xv_xid(info),
			    ip->value_rect.r_left,
			    rect_bottom(&ip->value_rect) - 1,
			    ip->value_rect.r_width);
		    }
		PANEL_END_EACH_PAINT_WINDOW
#ifdef OW_I18N
		dp->value_wc[seln_last + 1] = save_seln_last_char;
#else
		dp->value[seln_last + 1] = save_seln_last_char;
#endif /* OW_I18N */
	    } else
		paint_value(ip, PV_NO_HIGHLIGHT);
	}
	if (seln_highlight != HL_NONE)
	    seln_highlight = HL_NONE;
	dp->flags &= ~TEXT_HIGHLIGHTED;
    }
}


static void
text_seln_delete(ip, rank)
    Item_info      *ip;
    int		    rank;	/* PANEL_SEL_PRIMARY or PANEL_SEL_SECONDARY */
{
    int             caret_shift = 0;
    Text_info	   *dp;
    Event	    event;
    int             last;	/* position of last valid char in value */
    int		    new;	/* new position of char to be moved */
    int		    old;	/* old position of char to be moved */
    struct pr_size  size;
    int		    undo_index;
    int             val_change = 0;

    if (!ip)
	return;
    dp = TEXT_FROM_ITEM(ip);

    if (dp->flags & PTXT_READ_ONLY)
	return;

    if (rank == PANEL_SEL_PRIMARY)
	dp->delete_pending = FALSE;

    /*
     * Calculate number of character positions to move displayed value
     * (val_change) and number of character positions to move caret
     * (caret_shift).
     */
    val_change = dp->seln_first[rank] - dp->seln_last[rank] - 1;
#ifdef OW_I18N
    size.x = XwcTextEscapement(ip->value_fontset_id,
			&dp->value_wc[dp->first_char],
			dp->seln_last[rank] - dp->first_char + 1);
#else
    size = xv_pf_textwidth(dp->seln_last[rank] - dp->first_char + 1,
			   ip->value_font, &dp->value[dp->first_char]);
#endif /* OW_I18N */
    if (dp->caret_offset >= size.x)	/* Is caret at or past right margin of
					 * selection? */
	caret_shift = val_change;	/* Yes: shift caret to left margin of
					 * selection (e.g., ACTION_CUT or
					 * pending delete accept_key), or
					 * account for deleted characters.
					 * (e.g., DRAG_MOVE delete) */

    /* Copy the characters to be deleted to the undo buffer */
    undo_index = 0;
    for (new = dp->seln_first[rank]; new <= dp->seln_last[rank];
	 new++)
#ifdef OW_I18N
	dp->undo_buffer_wc[undo_index++] = dp->value_wc[new];
    dp->undo_buffer_wc[undo_index] = 0;   /* NULL terminate the undo buffer */
#else
	dp->undo_buffer[undo_index++] = dp->value[new];
    dp->undo_buffer[undo_index] = 0;   /* NULL terminate the undo buffer */
#endif /* OW_I18N */
    dp->undo_direction = INSERT;

    /* Delete the selected characters from the value buffer */
    new = dp->seln_first[rank];
    old = dp->seln_last[rank] + 1;
#ifdef OW_I18N
    last = wslen(dp->value_wc);
#else
    last = strlen(dp->value);
#endif /* OW_I18N */

#ifdef OW_I18N
    /*  FIX ME: could be a subroutine??
     *  If STORED_LENGTH attribute was used, then
     *  dp->stored_length is a byte count.  So,
     *  we need to find out how many bytes is
     *  consumed by 0 to new already, then see
     *  if there's room for the rest of old to last
     */
     if ((dp->flags & STORED_LENGTH_WC) == 0) {
	char		mbs[MB_LEN_MAX + 1];
	int		i, j, nbytes = 0;

	for (i=0; i <= new; i++) {
	    if ((j = wctomb(mbs, dp->value_wc[i])) < 0)
		break;
	    nbytes += j;
	}
	for (; nbytes <= (dp->stored_length - 1); new++, old++) {
	    if (old > last) {
		dp->value_wc[new] = 0;
		nbytes++;
	    } else {
	        if ((j = wctomb(mbs, dp->value_wc[old])) < 0)
		    break;
		nbytes += j;
		dp->value_wc[new] = dp->value_wc[old];
	    }
	}
    }
    else {
    /*  dp->stored_length is a character count */

	for (; new <= (dp->stored_length - 1); new++, old++) {
	    if (old > last)
		dp->value_wc[new] = 0;
	    else
		dp->value_wc[new] = dp->value_wc[old];
	}
    }
#else
    for (; new <= dp->stored_length - 1; new++, old++) {
	if (old > last)
	    dp->value[new] = 0;
	else
	    dp->value[new] = dp->value[old];
    }
#endif /* OW_I18N */

    /* Adjust Drag and Drop selection boundaries, if necessary */
    if (dp->dnd_sel_first > dp->seln_first[rank]) {
	dp->dnd_sel_first -= dp->seln_last[rank] - dp->seln_first[rank] + 1;
	dp->dnd_sel_last -= dp->seln_last[rank] - dp->seln_first[rank] + 1;
    }

    /*
     * Selection has been "used up": no mouse-left clicks or primary
     * selection pending.
     */
    dp->select_click_cnt[rank] = 0;
    dp->flags &= ~TEXT_HIGHLIGHTED;
    if (rank == PANEL_SEL_PRIMARY) {
	primary_seln_panel = NULL;
	dp->flags &= ~TEXT_SELECTED;
    } else
	secondary_seln_panel = NULL;


#ifdef OW_I18N
    sdfprintf((stderr, "Set selection to NULL\n"));
    if (rank == PANEL_SEL_CLIPBOARD) {
	ip->panel->clipboard.value = 0;
    } else {
	dp->seln_last[rank] = -1;
    }
#else
    /* BR# 1073493 */
    xv_set(ip->panel->sel_item[rank],
        SEL_DATA, 0,
        SEL_LENGTH, 0,
        NULL);
#endif

    /* Repaint the value */
    update_value_offset(ip, val_change, 0, 1);
    paint_value(ip, PV_HIGHLIGHT);

    if (ip->panel->kbd_focus_item == ip) {
	/* Repaint the caret */
	paint_caret(ip, FALSE);
	dp->caret_offset = -1;  /* caret_offset is now invalid */
	update_caret_offset(ip, caret_shift, 0);
	paint_caret(ip, TRUE);
    }

/*  Need to fake event->ie_string again for secondary selection ??? */

    if (rank == PANEL_SEL_SECONDARY /* BUG ALERT: ??? */
	&& dp->notify_level != PANEL_NONE) {
	event_init(&event);
	event_set_up(&event);
	event_set_action(&event, ACTION_CUT);
	(void) (*ip->notify) (ITEM_PUBLIC(ip), &event);
    }
}


#ifdef OW_I18N
/*
 * done_proc for the selection/dnd
 */
static void
text_seln_done_proc(sel_own, data, target)
    Selection_owner sel_own;
    Xv_opaque	   *data;
    Atom	    target;
{
    Panel_info	   *panel;

    if (data == NULL)
	return;

    panel = PANEL_PRIVATE(xv_get(sel_own, XV_KEY_DATA, PANEL));
    if (target == XA_STRING) {
	xv_free((char *) data);

    } else if (target == panel->atom.compound_text) {
	XFree((char *) data);
    }
}
#endif /* OW_I18N */


/*
 * Highlight selection according to its rank.
 */
static void
text_seln_highlight(panel, ip, rank)
    Panel_info	   *panel;
    Item_info	   *ip;
    int		    rank;	/* PANEL_SEL_PRIMARY or PANEL_SEL_SECONDARY */
{
    Text_info      *dp = TEXT_FROM_ITEM(ip);
    GC             *gc_list;
    Xv_Drawable_info *info;
    Xv_Window       pw;
    Rect            rect;
    Xv_Screen       screen;
    struct pr_size  size;
    int             y;

    rect = ip->value_rect;;
#ifdef OW_I18N
    if (dp->select_click_cnt[rank] == 0 || wslen(dp->value_wc) == 0)
	return;
#else
    if (dp->select_click_cnt[rank] == 0 || strlen(dp->value) == 0)
	return;
#endif /* OW_I18N */

    /* Highlight characters bounded by seln_first and seln_last */
    if (dp->seln_first[rank] > dp->first_char) {
#ifdef OW_I18N
	size.x = XwcTextEscapement(ip->value_fontset_id, 
			 &dp->value_wc[dp->first_char],
			 (dp->seln_first[rank] - dp->first_char));
#else
	size = xv_pf_textwidth((dp->seln_first[rank] - dp->first_char),
			       ip->value_font, &dp->value[dp->first_char]);
#endif /* OW_I18N */
	rect.r_left += size.x;
    }
    if (dp->first_char)
	rect.r_left += dp->scroll_btn_width;
#ifdef OW_I18N
    size.x = XwcTextEscapement(ip->value_fontset_id, 
		       &dp->value_wc[dp->seln_first[rank]],
		       dp->seln_last[rank] - dp->seln_first[rank] + 1);
#else
    size = xv_pf_textwidth(
	dp->seln_last[rank] - dp->seln_first[rank] + 1,
	ip->value_font, &dp->value[dp->seln_first[rank]]);
#endif /* OW_I18N */
    rect.r_width = size.x;
    if (rect.r_width > dp->text_rect.r_width)
	rect.r_width = dp->text_rect.r_width;
    rect.r_height--;		/* don't disturb underlining */

    switch (rank) {
      case PANEL_SEL_PRIMARY:
	primary_seln_panel = panel;	/* save panel */
	primary_seln_rect = rect;	/* save rectangle coordinates */
	primary_seln_first = dp->seln_first[PANEL_SEL_PRIMARY];
	primary_seln_last = dp->seln_last[PANEL_SEL_PRIMARY];
	seln_highlight = HL_INVERT;
	panel_invert(panel, &rect, ip->color_index);
	break;

      case PANEL_SEL_SECONDARY:
	secondary_seln_panel = panel;	/* save panel */
	secondary_seln_rect = rect;	/* save rectangle coordinates */
	secondary_seln_first = dp->seln_first[PANEL_SEL_SECONDARY];
	secondary_seln_last = dp->seln_last[PANEL_SEL_SECONDARY];
	if (panel->status.quick_move) {
	    seln_highlight = HL_STRIKE_THRU;
	    y = rect.r_top + (rect.r_height / 2);
	} else {
	    seln_highlight = HL_UNDERLINE;
	    y = rect_bottom(&rect);
	}
	PANEL_EACH_PAINT_WINDOW(panel, pw)
	    if (ip->color_index >= 0) {
		xv_vector(pw, rect.r_left, y, rect.r_left + rect.r_width - 1, y,
			  ip->color_index < 0 ? PIX_SET :
			      PIX_SRC | PIX_COLOR(ip->color_index),
			  0);
	    } else {
		DRAWABLE_INFO_MACRO(pw, info);
		screen = xv_screen(info);
		gc_list = (GC *)xv_get(screen, SCREEN_OLGC_LIST, pw);
		XDrawLine(xv_display(info), xv_xid(info),
			  gc_list[SCREEN_SET_GC],
			  rect.r_left, y, rect.r_left + rect.r_width - 1, y);
	    }
	PANEL_END_EACH_PAINT_WINDOW
	break;
    }
    dp->flags |= TEXT_HIGHLIGHTED;
}


static void
text_seln_init(panel)
    Panel_info	   *panel;
{
    Panel	    panel_public = PANEL_PUBLIC(panel);

    panel->sel_owner[PANEL_SEL_PRIMARY] =
	xv_create(panel_public, SELECTION_OWNER,
		  SEL_CONVERT_PROC, text_convert_proc,
	          SEL_LOSE_PROC, text_lose_proc,
		  XV_KEY_DATA, PANEL, panel_public,
		  NULL);
    panel->sel_rank[PANEL_SEL_PRIMARY] =
	xv_get(panel->sel_owner[PANEL_SEL_PRIMARY], SEL_RANK);
#ifndef OW_I18N
    panel->sel_item[PANEL_SEL_PRIMARY] =
	xv_create(panel->sel_owner[PANEL_SEL_PRIMARY], SELECTION_ITEM, NULL);
#endif

    panel->sel_owner[PANEL_SEL_SECONDARY] =
	xv_create(panel_public, SELECTION_OWNER,
		  SEL_CONVERT_PROC, text_convert_proc,
	          SEL_LOSE_PROC, text_lose_proc,
#ifdef OW_I18N
		  SEL_DONE_PROC, text_seln_done_proc,
#endif
		  SEL_RANK, panel->atom.secondary,
		  XV_KEY_DATA, PANEL, panel_public,
		  NULL);
    panel->sel_rank[PANEL_SEL_SECONDARY] =
	xv_get(panel->sel_owner[PANEL_SEL_SECONDARY], SEL_RANK);
#ifndef OW_I18N
    panel->sel_item[PANEL_SEL_SECONDARY] =
	xv_create(panel->sel_owner[PANEL_SEL_SECONDARY], SELECTION_ITEM, NULL);
#endif

    panel->sel_owner[PANEL_SEL_CLIPBOARD] =
	xv_create(panel_public, SELECTION_OWNER,
		  SEL_CONVERT_PROC, text_convert_proc,
	          SEL_LOSE_PROC, text_lose_proc,
#ifdef OW_I18N
		  SEL_DONE_PROC, text_seln_done_proc,
#endif
		  SEL_RANK_NAME, "CLIPBOARD",
		  XV_KEY_DATA, PANEL, panel_public,
		  NULL);
    panel->sel_rank[PANEL_SEL_CLIPBOARD] =
	xv_get(panel->sel_owner[PANEL_SEL_CLIPBOARD], SEL_RANK);
#ifndef OW_I18N
    panel->sel_item[PANEL_SEL_CLIPBOARD] =
	xv_create(panel->sel_owner[PANEL_SEL_CLIPBOARD], SELECTION_ITEM, NULL);
#endif

    panel->sel_req = xv_create(panel_public, SELECTION_REQUESTOR, NULL);
}


/* Copy the primary selection data to the clipboard */
static void
#ifdef OW_I18N
text_set_clipboard(panel, ip, dp)
    Panel_info	   *panel;
    Item_info	   *ip;
    Text_info	   *dp;
#else
text_set_clipboard(panel, ip)
    Panel_info	   *panel;
    Item_info	   *ip;
#endif
{
    if (xv_set(panel->sel_owner[PANEL_SEL_CLIPBOARD],
	       SEL_OWN, TRUE,
	       NULL) == XV_OK) {
#ifdef OW_I18N
	wchar_t		*last;
	wchar_t		 saved_char;

	sdfprintf((stderr, "text_set_clipboard: select? %d, first %d, last %d\n",
		   dp->flags & TEXT_SELECTED,
		   dp->seln_first[PANEL_SEL_PRIMARY],
		   dp->seln_last[PANEL_SEL_PRIMARY]));
	if (dp->seln_last[PANEL_SEL_PRIMARY]
			< dp->seln_first[PANEL_SEL_PRIMARY]) {
	    /* temp code */
	    /* Null string case */
	    panel->clipboard.value = NULL;
	    sdfprintf((stderr, "Set clipboard to NULL\n"));
	} else {
	    last = &dp->value_wc[dp->seln_last[PANEL_SEL_PRIMARY] + 1];
	    saved_char = *last;
	    *last = 0;
	    _xv_pswcs_wcsdup(&panel->clipboard,
			 &dp->value_wc[dp->seln_first[PANEL_SEL_PRIMARY]]);
	    *last = saved_char;
	    sdfprintf((stderr, "Set clipboard = [%ws]\n",
						panel->clipboard.value));
	}
#else /* OW_I18N */
	xv_set(panel->sel_item[PANEL_SEL_CLIPBOARD],
	       SEL_DATA,
		   xv_get(panel->sel_item[PANEL_SEL_PRIMARY],
			  SEL_DATA),
	       SEL_LENGTH,
		   xv_get(panel->sel_item[PANEL_SEL_PRIMARY],
			  SEL_LENGTH),
	       NULL);
#endif /* OW_I18N */
	panel->sel_holder[PANEL_SEL_CLIPBOARD] = ip;
    }
}


#ifndef OW_I18N
static void
text_set_sel_data(panel, dp, rank)
    Panel_info	   *panel;
    Text_info	   *dp;
    int		    rank;
{
    xv_set(panel->sel_item[rank],
	   SEL_DATA, &dp->value[dp->seln_first[rank]],
	   SEL_LENGTH, dp->seln_last[rank] - dp->seln_first[rank] + 1,
	   NULL);
}
#endif /* ! OW_I18N */


/*
 * update_caret_offset computes the caret x offset (dp->caret_offset) and
 * character position (dp->caret_position) for ip.
 *
 * If 'caret_shift' is non-zero, then dp->caret_position is incremented
 * or decremented (depending upon value of 'caret_shift'). However, if
 * 'caret_shift' *is* zero, then dp->caret_position is determined by
 * finding appropriate index by examining offset (dp->caret_offset) from
 * first visible character (dp->first_char).
 *
 * Before exiting this function, caret_offset is calculated from delta
 * between first visible char and caret_position.
 *
 */



static void
update_caret_offset(ip, caret_shift, calc_from_caret_pos)
    Item_info      *ip;
    int             caret_shift;  /* char position delta from caret_position */
    int		    calc_from_caret_pos;
{
    register Text_info *dp = TEXT_FROM_ITEM(ip);
    int             max_caret_pos;
#ifndef OW_I18N
    struct pr_size  size;
#endif

    if (caret_shift || calc_from_caret_pos) {
	dp->caret_position += caret_shift;
	if (dp->caret_position < dp->first_char)
	    dp->caret_position = dp->first_char;
    } else if (dp->caret_offset >= 0)
#ifdef OW_I18N
	dp->caret_position = char_position(dp->caret_offset,
	    ip->value_font, &dp->value_wc[dp->first_char], TRUE) +
	    dp->first_char;
    max_caret_pos = wslen(dp->value_wc);
    if (dp->caret_position > max_caret_pos)
	dp->caret_position = max_caret_pos;
    dp->caret_offset  = XwcTextEscapement(ip->value_fontset_id, 
	&dp->value_wc[dp->first_char], 
	dp->caret_position - dp->first_char);

    /*  saved_caret_offset and saved_caret_position are
     *  are updated here as well, because the pre-edit
     *  text has already been committed at this point.
     *  Some libmle's, i.e. Chinese version, calls
     *  preedit_draw again with empty text before
     *  caret offset is updated then the caret gets drawn
     *  at the wrong place.  So updating it
     *  here gets rid of that problem.  I am assuming
     *  this function only gets when it's calculating
     *  caret positions of the committed text, not
     *  pre-edit text.
     */
    dp->saved_caret_offset = dp->caret_offset;
    dp->saved_caret_position = dp->caret_position;
#else
	dp->caret_position = char_position(dp->caret_offset,
	    ip->value_font, &dp->value[dp->first_char], TRUE) +
	    dp->first_char;
    max_caret_pos = strlen(dp->value);
    if (dp->caret_position > max_caret_pos)
	dp->caret_position = max_caret_pos;
    size = xv_pf_textwidth(dp->caret_position - dp->first_char,
			   ip->value_font, &dp->value[dp->first_char]);
    dp->caret_offset = size.x;
#endif /* OW_I18N */

    /* Caret cannot exceed last character of value */
    if (dp->caret_offset > dp->value_offset) {
	dp->caret_offset = dp->value_offset;
	dp->caret_position = dp->last_char + 1;
    }
}


/*
 * Define rectangle containing the text (i.e., value rect less arrows)
 */
static void
update_text_rect(ip)
    Item_info      *ip;
{
    register Text_info *dp = TEXT_FROM_ITEM(ip);

    dp->text_rect = ip->value_rect;
    if (dp->first_char) {
	dp->text_rect.r_left += dp->scroll_btn_width;
	dp->text_rect.r_width -= dp->scroll_btn_width;
    }
#ifdef OW_I18N
    if (dp->last_char < (int)wslen(dp->value_wc) - 1)
#else
    if (dp->last_char < (int)strlen(dp->value) - 1)
#endif /* OW_I18N */
	dp->text_rect.r_width -= dp->scroll_btn_width;
}


/*
 * update_value updates the text item value and cursor position according to
 * the event action.  Actions fall into four categories: editing, navigation,
 * selection and printable characters.  ok_to_insert determines whether editing
 * actions and printable characters are acted upon.
 */
static void
#ifdef OW_I18N
update_value(ip, is_wc, action, ok_to_insert, synthetic_event, retstatus)
    Item_info      *ip;
    register int    action;	/* event action code */
    Bool	    is_wc;
#else
update_value(ip, action, ok_to_insert, synthetic_event, retstatus)
    Item_info      *ip;
    register int    action;	/* event action code */
#endif
    int             ok_to_insert;
    int		    synthetic_event;  /* FALSE: user-generated,
				       * TRUE: not user-generated */
                        /* misc return status - added to return */
                        /* information about whether DRAG_MOVE was */
                        /* a file name or array of text */
    int             *retstatus;
{
    register int    i;		/* counter */
    register Text_info *dp = TEXT_FROM_ITEM(ip);
    register CHAR  *sp;		/* string value */
#ifndef OW_I18N
    int		    ascent = 0;
#endif
    int             caret_shift = 0;	/* number of positions to move caret */
    int             char_code;
#ifndef OW_I18N
    int		    descent = 0;
    int             direction = 0;
#endif
    Xv_Drawable_info *info;
    int             insert_pos;	/* position for character add/delete */
    int             j;
    int		    new_len;	/* new string length */
    int		    orig_caret_position;  /* original caret position */
    int             orig_len;	/* original string length */
    int             orig_offset;/* before caret offset */
    int             orig_text_rect_width;  /* original text rectangle width */
#ifndef OW_I18N
    XCharStruct     overall;
#endif
    Panel_info	   *panel = ip->panel;
    int		    pc_adv_x;
    Xv_Window       pw;
    int		    rank = -1; /* initialize to "invalid" */
    Rect	    rect;
    int		    selection_action = FALSE;
    int		    sel_format; /* size of data element: 8, 16 or 32 */
    long	    sel_length; /* # char's in selection excluding NULL */
    CHAR	   *sel_string; /* contents of selection */
#ifdef OW_I18N
    char	   *sel_mbsorcts = NULL; /* contents of seln in pre-wcs form */
    wchar_t	   **sel_wcslist = NULL;
#endif
    CHAR	   *sel_copy;	/* copy of selected string for pre-parsing */
    int		    sel_length_copy = 0; /* copy of selection length for pre-parsing */
    struct pr_size  size;
    int		    undo_cnt;
    int		    undo_index;
    int             val_change = 0;	/* number of characters added (+) or
					 * deleted (-) */
    int             val_shift = 0;	/* number of characters to shift
					 * value display */
    int             was_clipped;	/* TRUE if value was clipped */
    int		    x;		/* left of insert/del point */
#ifdef OW_I18N
    XRectangle      overall_logical_extents = {0};
    XRectangle      overall_ink_extents = {0};
#else
    XFontStruct	   *x_font_info;
#endif /*OW_I18N*/

#ifndef OW_I18N
    /*
     * Initialize overall to zeros
     * It is not initialized like overall_ink_extents above because the MIT 
     * build (using cc), complains about "no automatic aggregate initialization"
     */
    XV_BZERO(&overall, sizeof(XCharStruct));
#endif /* OW_I18N */

    *retstatus=0;

    /* Define rectangle containing the text (i.e., value rect less arrows) */
    update_text_rect(ip);

    /* Get the insert position for character add/delete */
    if (dp->caret_offset == 0)
	insert_pos = dp->first_char;
    else
	insert_pos = dp->caret_position;

#ifdef OW_I18N
    sp = dp->value_wc;
#else
    sp = dp->value;
#endif /* OW_I18N */
    orig_len = STRLEN(sp);

#ifdef OW_I18N
    if (is_wc)
	goto wc_printable; /*
			    * "wc_printable" is in the default section
			    * of the switch statement in below.
			    */
#endif

    switch (action) {

      /**********************************************************************
       *     Editing actions                                                *
       **********************************************************************/
      case ACTION_ERASE_CHAR_BACKWARD:
	/* Allow notify_proc to override editting characters. */
	/* Supports read-only text fields. */
	if (!ok_to_insert) {
	    text_alarm(ip);
	    return;
	}
	/* Nothing to backspace if caret is at left boundary. */
	if (dp->caret_offset == 0)
	    return;

	/* Can't show result of backspace if display length exceeded and */
	/* caret is to the right of the panel left arrow.  The moral here */
	/* is that you can't delete what you can't see. */
#ifdef OW_I18N
        {
        /*  Need to use pixel comparisons,
         *  instead of character length comparison
         */
	    int x;

            x = XwcTextEscapement(ip->value_fontset_id,
				  sp, orig_len);
            if ((x > dp->display_width) &&
                (dp->first_char) && (dp->caret_offset == 0))
                return;
        }
#else
	if ((orig_len > dp->display_length) &&
	    (dp->first_char) && (dp->caret_offset == 0))
	    return;
#endif /* OW_I18N */

	if ((*sp) && (insert_pos > 0)) {
#ifdef OW_I18N
	    dp->undo_buffer_wc[0] = sp[insert_pos-1];
	    dp->undo_buffer_wc[1] = '\0';
#else
	    dp->undo_buffer[0] = sp[insert_pos-1];
	    dp->undo_buffer[1] = 0;
#endif /* OW_I18N */
	    dp->undo_direction = INSERT;
	    for (i = insert_pos; i < orig_len; i++)
		sp[i - 1] = sp[i];
	    sp[orig_len - 1] = '\0';
	    insert_pos--;
	    caret_shift = -1;
	    val_change = -1;

	    /*
	     * If clipped at left boundary, leave caret alone. Characters
	     * will shift in from the left.
	     */
	    if (dp->first_char) {
		caret_shift = 0;
		dp->caret_position--;
	    }
	}
	break;

      case ACTION_ERASE_CHAR_FORWARD:
	/* Allow notify_proc to override editting characters. */
	/* Supports read-only text fields. */
	if (!ok_to_insert) {
	    text_alarm(ip);
	    return;
	}
	/* Can't show result of forespace if display length exceeded and */
	/* caret is to the left of the panel right arrow.  The moral here */
	/* is that you can't delete what you can't see. */
	if (dp->caret_offset == dp->text_rect.r_width)
	    return;
#ifdef OW_I18N
	size.x = XwcTextEscapement(ip->value_fontset_id,
				   &dp->value_wc[dp->first_char],
				   dp->last_char - dp->first_char);
#else
	size = xv_pf_textwidth(dp->last_char - dp->first_char,
			       ip->value_font, &dp->value[dp->first_char]);
#endif /* OW_I18N */
	if (dp->caret_offset > size.x)
	    return;

	if ((*sp) && (insert_pos >= 0)) {
#ifdef OW_I18N
	    dp->undo_buffer_wc[0] = sp[insert_pos];
	    dp->undo_buffer_wc[1] = '\0';
#else
	    dp->undo_buffer[0] = sp[insert_pos];
	    dp->undo_buffer[1] = 0;
#endif /* OW_I18N */
	    dp->undo_direction = INSERT;
	    for (i = insert_pos; i < orig_len; i++)
		sp[i] = sp[i + 1];
	    sp[orig_len - 1] = '\0';
	    caret_shift = 0;
	    val_change = 0;
	    if ((dp->last_char >= ((int)STRLEN(sp) - 1)) && (dp->last_char > 1)) {
		val_change = -1;
		/*
		 * ???  Why was the following line put in here?  This causes
		 * a bug in deleting the next character when the last char is
		 * displayed and the first character is not.  ???
		 */
		/* if (dp->first_char > 2) caret_shift = 1; */
	    }
	}
	break;

      case ACTION_ERASE_WORD_BACKWARD:
	/* ACTION_ERASE_WORD_BACKWARD is a SunView1 compatibility action */

	/* Allow notify_proc to override editting characters. */
	/* Supports read-only text fields. */
	if (!ok_to_insert) {
	    text_alarm(ip);
	    return;
	}
	/* skip back past blanks */
	if (insert_pos > orig_len)
	    insert_pos -= (dp->first_char - 1);
	for (i = insert_pos - 1; (i >= 0) && (sp[i] == ' '); i--);
#ifdef OW_I18N
	{
	    int	wc_type;

	    wc_type = wchar_kind(sp[i]);
	    if (wc_type == 1) {
		for (; (i >= 0) && (sp[i] != ' ')
			&& (wchar_kind(sp[i]) == wc_type); i--);
	    }
	    else {
		for (; (i >= 0) && (wchar_kind(sp[i]) == wc_type); i--);
	    }
	}
#else
	for (; (i >= 0) && (sp[i] != ' '); i--);
#endif /* OW_I18N */
	if (i < 0)
	    i = 0;
	if (i > 0)
	    i++;
	caret_shift = i - insert_pos;
	val_change = i - insert_pos;
	/* Copy the word to be deleted to the undo buffer */
	undo_index = 0;
	undo_cnt = -caret_shift;
#ifdef OW_I18N
	for (j = 0; j < undo_cnt; j++)
	    dp->undo_buffer_wc[undo_index++] = sp[i+j];
	dp->undo_buffer_wc[undo_index] = '\0';
#else
	for (j = 0; j < undo_cnt; j++)
	    dp->undo_buffer[undo_index++] = sp[i+j];
	dp->undo_buffer[undo_index] = 0;
#endif /* OW_I18N */
	dp->undo_direction = INSERT;
	/* Delete the word */
	for (j = insert_pos; j <= orig_len; j++, i++)
	    sp[i] = sp[j];
	insert_pos += caret_shift;
	break;

      case ACTION_ERASE_WORD_FORWARD:
	/* ACTION_ERASE_WORD_FORWARD is a SunView1 compatibility action */

	/* Allow notify_proc to override editting characters. */
	/* Supports read-only text fields. */
	if (!ok_to_insert) {
	    text_alarm(ip);
	    return;
	}
	/* skip back past blanks */
	for (i = insert_pos; (i < orig_len) && (sp[i] == ' '); i++);
#ifdef OW_I18N
	{
	    int	wc_type;

	    wc_type = wchar_kind(sp[i]);
	    if (wc_type == 1) {
		for (; (i < orig_len) && (sp[i] != ' ')
			&& (wchar_kind(sp[i]) == wc_type); i++);
	    }
	    else {
		for (; (i < orig_len) && 
			(wchar_kind(sp[i]) == wc_type); i++);
	    }
	}
#else
	for (; (i < orig_len) && (sp[i] != ' '); i++);
#endif /* OW_I18N */
	if (i >= orig_len)
	    i = orig_len - 1;
	if (i < (orig_len - 1))
	    i--;
	caret_shift = 0;
	val_change = 0;
	/* Copy the word to be deleted to the undo buffer */
	undo_index = 0;
	undo_cnt = i + 1 - insert_pos;
	for (j = 0; j < undo_cnt; j++)
#ifdef OW_I18N
	    dp->undo_buffer_wc[undo_index++] = sp[insert_pos+j];
	dp->undo_buffer_wc[undo_index] = '\0';
#else
	    dp->undo_buffer[undo_index++] = sp[insert_pos+j];
	dp->undo_buffer[undo_index] = 0;
#endif /* OW_I18N */
	dp->undo_direction = INSERT;
	/* Delete the word */
	for (j = insert_pos; i < orig_len; j++, i++)
	    sp[j] = sp[i + 1];
	break;

      case ACTION_ERASE_LINE_BACKWARD:
	/* ACTION_ERASE_LINE_BACKWARD is a SunView1 compatibility action */
	/* Allow notify_proc to override editting characters. */
	/* Supports read-only text fields. */
	if (!ok_to_insert) {
	    text_alarm(ip);
	    return;
	}
	/* sp[0] = '\0'; */
	caret_shift = -insert_pos;
	val_change = -insert_pos;
	/* Copy the characters to be deleted to the undo buffer */
	undo_index = 0;
	undo_cnt = insert_pos;
	for (i = 0; i < undo_cnt; i++)
#ifdef OW_I18N
	    dp->undo_buffer_wc[undo_index++] = sp[i];
	dp->undo_buffer_wc[undo_index] = '\0';
#else
	    dp->undo_buffer[undo_index++] = sp[i];
	dp->undo_buffer[undo_index] = 0;
#endif /* OW_I18N */
	dp->undo_direction = INSERT;
	/* Delete the line (backward) */
	for (i = 0, j = insert_pos; j <= orig_len; i++, j++)
	    sp[i] = sp[j];
	insert_pos = 0;
	break;

      case ACTION_ERASE_LINE_END:
	/* ACTION_ERASE_LINE_END is a SunView1 compatibility action */

	/* Allow notify_proc to override editting characters. */
	/* Supports read-only text fields. */
	if (!ok_to_insert) {
	    text_alarm(ip);
	    return;
	}
	caret_shift = 0;
	val_change = 0;
	/* Copy the characters to be deleted to the undo buffer */
	undo_index = 0;
	i = insert_pos;
	for (undo_cnt = STRLEN(sp) - insert_pos; undo_cnt >= 0; undo_cnt--)
#ifdef OW_I18N
	    dp->undo_buffer_wc[undo_index++] = sp[i++];
	dp->undo_buffer_wc[undo_index] = '\0';
#else
	    dp->undo_buffer[undo_index++] = sp[i++];
	dp->undo_buffer[undo_index] = 0;
#endif /* OW_I18N */
	dp->undo_direction = INSERT;
	/* Delete the line (forward) */
	sp[insert_pos] = '\0';
	if (dp->first_char > 1) {
	    val_change = STRLEN(sp) - 1 - dp->last_char;
	    if (dp->last_char < (orig_len - 1))
		val_change--;
	    caret_shift = -val_change;
	}
	break;

      case ACTION_ERASE_LINE:
	/* Allow notify_proc to override editting characters. */
	/* Supports read-only text fields. */
	if (!ok_to_insert) {
	    text_alarm(ip);
	    return;
	}
	/* Copy the characters to be deleted to the undo buffer */
#ifdef OW_I18N
	wscpy(sp, dp->undo_buffer_wc);
#else
	strcpy(sp, dp->undo_buffer);
#endif /* OW_I18N */
	dp->undo_direction = INSERT;
	/* Delete the line */
	sp[0] = '\0';
	dp->caret_offset = 0;
	dp->caret_position = 0;
	dp->first_char = 0;
	dp->last_char = 0;
	dp->value_offset = 0;
	update_text_rect(ip);
#ifdef OW_I18N
	if (dp->mask_wc != ' ')
#else
	if (dp->mask != ' ')
#endif /* OW_I18N */
	    paint_value(ip, PV_HIGHLIGHT);
	return;

      case ACTION_DRAG_COPY:
      case ACTION_DRAG_MOVE:
      case ACTION_CUT:
      case ACTION_PASTE:
	/* Allow notify_proc to override editting characters. */
	/* Supports read-only text fields. */
	if (!ok_to_insert) {
	    text_alarm(ip);
	    return;
	}
	if (action == ACTION_CUT || action == ACTION_PASTE) {
	    if (XGetSelectionOwner(XV_DISPLAY_FROM_WINDOW(PANEL_PUBLIC(panel)),
				   panel->atom.secondary) != None) {
		/* Get the contents of the Secondary Selection */
		rank = PANEL_SEL_SECONDARY;
		xv_set(panel->sel_req,
		       SEL_RANK, panel->atom.secondary,
		       NULL);
	    } else if (action == ACTION_PASTE) {
		/* Get the contents of the Clipboard Selection */
		rank = PANEL_SEL_CLIPBOARD;
		xv_set(panel->sel_req,
		       SEL_RANK_NAME, "CLIPBOARD",
		       NULL);
	    } else {
		/* Primary CUT: this has already been processed in
		 * text_accept_key(), so just return.
		 */
		return;
	    }
	} else {
	    /* Get the data from the initiator of the drag and drop */
	    rank = PANEL_SEL_PRIMARY;
	    dp->dnd_sel_first = insert_pos;
	}
	dp->flags &= ~SELECTION_REQUEST_FAILED;  /* assume we succeed */
	/* First try to convert the Selection Type "FILE_NAME" */
	xv_set(panel->sel_req, SEL_TYPE_NAME, "FILE_NAME", NULL);
	
	/*  Data came in as compound text and sel_length is
	 *  a byte count
	 */
#ifdef OW_I18N
	sel_mbsorcts = (char *) xv_get(panel->sel_req, SEL_DATA,
#else
	sel_string = (CHAR *) xv_get(panel->sel_req, SEL_DATA,
#endif
				     &sel_length, &sel_format);
        /* bit 1 => straight text; bit 2 => file name */
        *retstatus |= (sel_length == SEL_ERROR ? DRAG_MOVE_TEXT :
            DRAG_MOVE_FILENAME);

#ifdef OW_I18N
	if (sel_length != SEL_ERROR) {
	    /*
	     * FIX_ME: This is work around for the bug 1102278,
	     * textsw/ttysw should not respond to FILE_NAME, but does
	     * today in local xfer (and also, when contents of the
	     * string is less than 5 byte).  Otherwise, this code
	     * should not be necessary.
	     */
	    sel_string = _xv_mbstowcsdup(sel_mbsorcts);
	    sel_length = wslen(sel_string);
	    sdfprintf((stderr, "Got by FILE_NAME [%ws]\n", sel_string));
	} else {
	    /*
	     * Try Compound text first. if we ask for the STRING target
	     * first, Asian XView client actually try send string as
	     * multibyte characters which is completely against the
	     * ICCCM (however, Asian XView should capable sending by
	     * the multibyte string, because of backward compatibility
	     * with V2 and SunView.
	     */
	    /*
	     * But beforea actually issuing the CTEXT, should ask
	     * LENGTH_CHARS first, this will insure to send us CTEXT
	     * from old selection pkgs.  But we have no use of the
	     * result.
	     */
	    xv_set(panel->sel_req, SEL_TYPE, panel->atom.length_chars, NULL);
	    sel_mbsorcts = (char *) xv_get(panel->sel_req, SEL_DATA,
					   &sel_length, &sel_format);
	    if (sel_length != SEL_ERROR)
		xv_free(sel_mbsorcts);

	    xv_set(panel->sel_req, SEL_TYPE, panel->atom.compound_text, NULL);
	    sel_mbsorcts = (char *) xv_get(panel->sel_req, SEL_DATA,
					  &sel_length, &sel_format);
	    if (sel_length != SEL_ERROR) {
		if (sel_length == 0) {
		    sel_string = _xv_mbstowcsdup("");
		} else {
		    XTextProperty	 text_prop;
		    Panel		 panel_public;
		    int			 state;
		    int			 count;

		    text_prop.value = (unsigned char *) sel_mbsorcts;
		    text_prop.encoding = panel->atom.compound_text;
		    text_prop.format = sel_format;
		    text_prop.nitems = sel_length;
		    panel_public = PANEL_PUBLIC(panel);
		    DRAWABLE_INFO_MACRO(panel_public, info);
		    state = _xv_XwcTextPropertyToTextList(panel_public, PANEL,
						      xv_display(info),
						      &text_prop, &sel_wcslist,
						      &count);
		    if (state < 0) {
		        sel_length = SEL_ERROR;
		        goto error;
		    }
		    sel_string = sel_wcslist[0];
		    sel_length = wslen(sel_string);
		}
		sdfprintf((stderr, "get CTEXT[%ws]\n", sel_string));
	    } else {
error:
#else
	if (sel_length == SEL_ERROR) {
#endif /* OW_I18N */
	    /* Data type is ASCII string */
	    xv_set(panel->sel_req, SEL_TYPE, XA_STRING, NULL);
	    /* Get the contents of the appropriate selection.
	     * Note: the returned string is not NULL-terminated.
	     * 'sel_length' will indicate the string's length.
	     */
#ifdef OW_I18N
	    sel_mbsorcts = (char *) xv_get(panel->sel_req, SEL_DATA,
					 &sel_length, &sel_format);
	    /*
	     * Convert from ASCII to wide char if valid selection
	     * sel_length is a character count.
	     */
	    if (sel_length != SEL_ERROR) {
		sel_string = _xv_mbstowcsdup(sel_mbsorcts);
	  	sel_length = wslen(sel_string);
	        sdfprintf((stderr, "get MBS[%ws]\n", sel_string));
	    } else {
#else /* OW_I18N */
	    sel_string = (char *)xv_get(panel->sel_req, SEL_DATA,
					 &sel_length, &sel_format);
#endif /* OW_I18N */
	    if (sel_length == SEL_ERROR)
	    /* Check that the data arrived okay */
	    {
		dp->flags |= SELECTION_REQUEST_FAILED;
		if (rank != PANEL_SEL_PRIMARY) {
		    char    buf[64];
		    sprintf(buf, XV_MSG("Unable to get contents of %s selection"),
			    xv_get(panel->sel_req, SEL_RANK_NAME));
		    xv_error(XV_ZERO,
			     ERROR_STRING, buf,
			     ERROR_PKG, PANEL,
			     NULL);
		    text_alarm(ip);
		} else {
		    Frame frame = xv_get(PANEL_PUBLIC(panel), WIN_FRAME);
		    Xv_Notice notice = xv_create(frame, NOTICE,
			NOTICE_MESSAGE_STRINGS,
			    XV_MSG("Drag and Drop failed:"),
			    XV_MSG("Unable to get contents of selection"),
			    0,
			XV_SHOW, TRUE,
			NULL);
		    xv_destroy(notice);
		}
		return;
	    }
#ifdef OW_I18N
	    }
	    }
#endif
	}
	/*
	 * HIT says any successive tab characters should be collapsed
	 * into a single space character (1074212).  To do so, use a copy
	 * buffer to save the trouble of shifting left.  otherwise, truncate
	 * before first non-printable character (e.g. NEWLINE).
	 */
#ifdef OW_I18N
	/*
	 * This change should apply to the #else part as well.
	 */
	sel_copy = (CHAR *) xv_calloc( sel_length + 1, sizeof(CHAR) );
	sel_copy[0] = 0;
#else
	sel_copy = (CHAR *) xv_calloc( sel_length, sizeof(CHAR) );
#endif
	for (i = 0; i < sel_length; i++) {
	    if ( sel_string[i] == '\t' ) {
		sel_copy[sel_length_copy] = ' ';
		sel_length_copy++;
		while ( (sel_string[i+1] == '\t') && (i+1 < sel_length) )
		    i++;	/* ignore successive tabs */
	    }
#ifdef OW_I18N
	    else if (!panel_printable_char(sel_string[i])) {
#else
	    else if (!panel_printable_char((u_char) sel_string[i])) {
#endif
		break;
	    }
	    else {
		sel_copy[sel_length_copy] = sel_string[i];
		sel_length_copy++;
	    }
	} /* for() */
#ifdef OW_I18N
	if (sel_wcslist != NULL)
	    XwcFreeStringList(sel_wcslist);
	else
	    xv_free(sel_string);
	if (sel_mbsorcts != NULL)
	    xv_free(sel_mbsorcts);
#else
	xv_free( sel_string );
#endif
	sel_string = sel_copy;
	sel_length = sel_length_copy;

	/* Insure there is room in dp->value for the clipboard contents.
	 * Note: sel_length does not include the NULL terminator.
	 */
#ifdef OW_I18N
	/*  FIX ME: could be subroutine??
	 *  dp->stored_length is a byte count
	 *  figure out how many bytes sp is
	 *  already taking up then see how many
	 *  chars the new string from selection
	 *  will fit into the remaining bytes
	 */
	if ((dp->flags & STORED_LENGTH_WC) == 0) {
	    char	p[MB_LEN_MAX + 1];
	    wchar_t	*w;
	    int		i, j, nbytes;
	    int		nchars = 0;

	    nbytes = dp->stored_length - wslen_in_byte(sp);
	    for (w = sel_string, i = nbytes; i > 0 && *w; w++) {
		if ((j = wctomb(p, *w)) < 0 || j > i)
		    break;
		i -= j;
		nchars++;
	    }
	    sel_length = MIN(sel_length, nchars);
	}
	else {
	/* 
	 * bugfix 1089652 and 1091198; code was:
	 * sel_length = MIN(sel_length, dp->stored_length + 1 - orig_len);
	 * also changed OW_I18N code
	 */
	    sel_length = MIN(sel_length, dp->stored_length - orig_len);
	}
#else
	sel_length = MIN(sel_length, dp->stored_length - orig_len);
#endif /* OW_I18N */

        if ((sel_length<=0) &&	/*1091198*/
           ((action == ACTION_DRAG_COPY) ||
            (action == ACTION_DRAG_MOVE) ||
            (action == ACTION_PASTE)))
        {
            *retstatus |= DROP_OR_PASTE_FAILED;
            dp->dnd_sel_last = dp->dnd_sel_first = 0;
            xv_free( sel_string );
            return;
        }

	if (rank == PANEL_SEL_PRIMARY)
	    dp->dnd_sel_last = insert_pos + sel_length - 1;
	/* Shift everything to the right (including the NULL terminator) */
	for (i = orig_len; i >= insert_pos; i--)
	    sp[i+sel_length] = sp[i];
	/* Copy the characters from the selection to the text field and
	 * the Undo Buffer.
	 */
	for (undo_index = 0, i = insert_pos; undo_index < sel_length;
	     undo_index++, i++) {
	    sp[i] = sel_string[undo_index];
#ifdef OW_I18N
	    dp->undo_buffer_wc[undo_index] = sel_string[undo_index];
#else
	    dp->undo_buffer[undo_index] = sel_string[undo_index];
#endif
	}
	/* Null terminate the Undo Buffer */
#ifdef OW_I18N
	dp->undo_buffer_wc[undo_index] = 0;
#else
	dp->undo_buffer[undo_index] = 0;
#endif

	xv_free( sel_string );

	/* Set caret shift, value shift and undo direction */
	caret_shift = sel_length;
	if (dp->first_char)
	    val_shift = sel_length;
	dp->undo_direction = DELETE;
	/* If this was a drag move, drag copy, or quick move, and we own the
	 * selection (i.e., we moved/copied text from and to the same text
	 * field), and the insert point was before the selection, then adjust
	 * seln_first and seln_last to account for the inserted characters.
	 */
	if ((action == ACTION_DRAG_MOVE || action == ACTION_DRAG_COPY ||
	     action == ACTION_CUT) &&
	    panel->sel_holder[rank] == ip &&
	    insert_pos < dp->seln_first[rank]) {
	    dp->seln_first[rank] += sel_length;
	    dp->seln_last[rank] += sel_length;
	}
	/* If secondary selection, then tell the secondary selection holder
	 * that we are done with the selection.  If Quick Move, first tell
	 * the holder to delete the selection.
	 */
	if (rank == PANEL_SEL_SECONDARY) {
	    int    format;
	    long   length;
	    if (action == ACTION_CUT) {
		/* Post delete request back to owner */
		xv_set(panel->sel_req, SEL_TYPE, panel->atom.delete, NULL);
		(void) xv_get(panel->sel_req, SEL_DATA, &length, &format);
	    }
	    /* We're done with the secondary selection */
	    xv_set(panel->sel_req, SEL_TYPE, panel->atom.selection_end, NULL);
	    (void) xv_get(panel->sel_req, SEL_DATA, &length, &format);
	    if (length == SEL_ERROR) {
		/* SunView1 secondary selection: post a SELN_YIELD request */
		xv_set(panel->sel_req, SEL_TYPE, panel->atom.seln_yield, NULL);
		(void) xv_get(panel->sel_req, SEL_DATA, &length, &format);
	    }
	}
	break;

      case ACTION_UNDO:
	/* Allow notify_proc to override editting characters. */
	/* Supports read-only text fields. */
	if (!ok_to_insert) {
	    text_alarm(ip);
	    return;
	}
	switch (dp->undo_direction) {
	  case INSERT:
#ifdef OW_I18N
	    undo_cnt = wslen(dp->undo_buffer_wc);
#else
	    undo_cnt = strlen(dp->undo_buffer);
#endif /* OW_I18N */
	    /* Shift everything to the right (including the NULL terminator) */
	    for (i = orig_len; i >= insert_pos; i--)
		sp[i+undo_cnt] = sp[i];
	    /* Insert the characters from the Undo Buffer */
	    for (undo_index = 0, i = insert_pos; undo_index < undo_cnt;
		 undo_index++, i++)
#ifdef OW_I18N
		sp[i] = dp->undo_buffer_wc[undo_index];
#else
		sp[i] = dp->undo_buffer[undo_index];
#endif /* OW_I18N */
	    caret_shift = undo_cnt;
	    if (dp->first_char)
		val_shift = undo_cnt;
	    dp->undo_direction = DELETE;
	    break;
	  case DELETE:
#ifdef OW_I18N
	    undo_cnt = wslen(dp->undo_buffer_wc);
#else
	    undo_cnt = strlen(dp->undo_buffer);
#endif /* OW_I18N */
	    for (i = insert_pos; i <= orig_len; i++)
		sp[i-undo_cnt] = sp[i];
	    dp->undo_direction = INSERT;
	    caret_shift = val_change = -undo_cnt;
	    break;
	}
	break;
	
      /**********************************************************************
       *     Navigation & Selection actions                                 *
       **********************************************************************/
      case ACTION_SELECT_CHAR_BACKWARD:
	selection_action = TRUE;
	/* ... fall through to ACTION_GO_CHAR_BACKWARD */
      case ACTION_GO_CHAR_BACKWARD:
	caret_shift = -1;
	if ((dp->first_char) && (dp->caret_offset == 0))
	    val_shift = -1;	/* display will include next char to left */
	dp->undo_direction = INVALID;  /* invalidate undo buffer */
	break;

      case ACTION_SELECT_CHAR_FORWARD:
	selection_action = TRUE;
	/* ... fall through to ACTION_GO_CHAR_FORWARD */
      case ACTION_GO_CHAR_FORWARD:
	caret_shift = 1;
	if (dp->last_char < orig_len - 1 &&
	    dp->caret_offset == dp->value_offset)
	    val_shift = 1;	/* display will include next char to right */
	dp->undo_direction = INVALID;  /* invalidate undo buffer */
	break;

	/*  For all actions involving word movement, code was added
	 *  to calculate Kanji, Hiragana, and katakana word
	 *  separations.  For ASCII set we're going through the
	 *  original algorithm plus making sure we're not stepping
	 *  into other code sets.  For non-ASCII sets we do not
	 *  check for delimiters.
	 */

      case ACTION_SELECT_WORD_BACKWARD:
	selection_action = TRUE;
	/* ... fall through to ACTION_GO_WORD_BACKWARD */
      case ACTION_GO_WORD_BACKWARD:
	/* Skip back to start of current or previous word */
	if (insert_pos > orig_len)
	    insert_pos -= (dp->first_char - 1);
#ifdef OW_I18N

	{
	    int	wc_type;

	    wc_type = wchar_kind(sp[insert_pos - 1]);
	    if (wc_type == 1) {
	        for (i = insert_pos - 1; i >= 0 && delim_table[sp[i]] &&
			(wchar_kind(sp[i]) == wc_type); i--);
	    }
	    else
		i = insert_pos - 1;

	    wc_type = wchar_kind(sp[i]);
	    if (wc_type == 1) {
		for (; i >= 0 && !delim_table[sp[i]] &&
			(wchar_kind(sp[i]) == wc_type); i--);
	    }
	    else {
		for (; i >= 0 && (wchar_kind(sp[i]) == wc_type); i--);
	    }
	}
#else
	for (i = insert_pos - 1; i >= 0 && delim_table[(u_char)sp[i]]; i--);
	for (; i >= 0 && !delim_table[(u_char)sp[i]]; i--);
#endif /* OW_I18N */
	if (i < 0)
	    i = 0;
	if (i > 0)
	    i++;
	caret_shift = i - insert_pos;
	if (i < dp->first_char)
	    /* Shift value so that caret is at correct character */
	    val_shift = caret_shift;
	dp->undo_direction = INVALID;  /* invalidate undo buffer */
	break;

      case ACTION_SELECT_WORD_END:
	selection_action = TRUE;
	/* ... fall through to ACTION_GO_WORD_END */
      case ACTION_GO_WORD_END:
	if (insert_pos < orig_len) {
	    /* Skip forward to end of current or next word */
#ifdef OW_I18N
	{
	    int	wc_type;

	    wc_type = wchar_kind(sp[insert_pos - 1]);
	    if (wc_type == 1) {
		for (i = insert_pos; i < orig_len && delim_table[sp[i]] &&
			(wchar_kind(sp[i]) == wc_type); i++);
	    }
	    else
		i = insert_pos;

	    wc_type = wchar_kind(sp[i]);
	    if (wc_type == 1) {
		for (; i < orig_len && !delim_table[sp[i]] &&
			(wchar_kind(sp[i]) == wc_type); i++);
	    }
	    else {
		for (; i < orig_len && (wchar_kind(sp[i]) == wc_type); i++);
	    }
	}
#else
	    for (i = insert_pos;
		 i < orig_len && delim_table[(u_char)sp[i]];
		 i++);
	    for (; i < orig_len && !delim_table[(u_char)sp[i]]; i++);
#endif /* OW_I18N */
	    caret_shift = i - insert_pos;
	    if (i > dp->last_char)
		/* Shift value so that caret is visible */
		val_shift = i - dp->last_char;
	    dp->undo_direction = INVALID;  /* invalidate undo buffer */
	}
	break;

      case ACTION_GO_WORD_FORWARD:
	/* ACTION_GO_WORD_FORWARD is a SunView1 compatibility action */
	if (insert_pos < orig_len) {
	    /* Skip forward to start of next word */
#ifdef OW_I18N
	{
	    int	wc_type;

	    wc_type = wchar_kind(sp[insert_pos]);
	    if (wc_type == 1) {
		for (i = insert_pos; i < orig_len && !delim_table[sp[i]] &&
			(wchar_kind(sp[i]) == wc_type); i++);
	    }
	    else {
		for (i = insert_pos; i < orig_len && 
			(wchar_kind(sp[i]) == wc_type); i++);
	    }
	}
	    
#else
	    for (i = insert_pos;
		 i < orig_len && !delim_table[(u_char)sp[i]];
		 i++);
	    for (; i < orig_len && delim_table[(u_char)sp[i]]; i++);
#endif /* OW_I18N */
	    caret_shift = i - insert_pos;
	    if (i > dp->last_char)
		/* Shift value so that caret is visible */
		val_shift = i - dp->last_char;
	    dp->undo_direction = INVALID;  /* invalidate undo buffer */
	}
	break;

      case ACTION_SELECT_LINE_START:
	selection_action = TRUE;
	/* ... fall through to ACTION_LINE_START */
      case ACTION_LINE_START:
	if (insert_pos == 0)
	    text_alarm(ip);	/* already at the start of the line */
	/* ... fall through */
      case ACTION_GO_LINE_FORWARD:
	/* ACTION_GO_LINE_FORWARD is a SunView1 compatibility action */
	/* Note: For ACTION_GO_LINE_FORWARD, the caret has already been
	 * advanced to the next line.  We now want to position it at
	 * the beginning of this line.
	 */
	caret_shift = -insert_pos;
	if (dp->first_char)
	    val_shift = -dp->first_char;
	dp->undo_direction = INVALID;  /* invalidate undo buffer */

	/* adjust caret to point to first character because
	   update_value_offset() will determine first and last
	   character based upon caret position */
        dp->caret_position = dp->first_char;

	break;

      case ACTION_SELECT_LINE_END:
      case ACTION_SELECT_ALL:
	selection_action = TRUE;
	/* ... fall through to ACTION_LINE_END */
      case ACTION_LINE_END:
	caret_shift = orig_len - insert_pos;
	if (!synthetic_event && action != ACTION_SELECT_ALL && caret_shift == 0)
	    text_alarm(ip);	/* already at the end of the line */
	if (dp->last_char < orig_len - 1)
	    val_shift = orig_len - dp->last_char;
	dp->undo_direction = INVALID;  /* invalidate undo buffer */
	break;

      /**********************************************************************
       *     Printable characters                                           *
       **********************************************************************/
      default:
#ifdef OW_I18N
	if (! is_wc) {
		char	mb;
		wchar_t	wc;

		mb = action;
		mbtowc(&wc, &mb, 1);
		action = wc;
	}
/*
 * "wc_printable" will be used from top of the this switch block.
 * This is when "action" argument contains wide character instead of
 * event_action.
 */
wc_printable:
#endif
	if (panel_printable_char(action)) {
	    dp->undo_direction = INVALID;  /* invalidate undo buffer */
	    if (ok_to_insert) {	/* insert */
#ifdef OW_I18N
	/*  FIX ME: could be a subroutine??
	 *  dp->stored_length is a byte count.
	 *  So got to figure out if there is
	 *  enough room for the new data
	 *  on a byte basis.  orig_len is a character
	 *  count.
	 */
		if ((dp->flags & STORED_LENGTH_WC) == 0) {
		    int		nbytes;
		    char	mb[MB_LEN_MAX + 1];

		    if ((nbytes = wctomb(mb, (wchar_t)action)) < 0)
			break;
		    if ((nbytes + wslen_in_byte(sp)) <= dp->stored_length) {
			char_code = action;
			for (i = orig_len; i > insert_pos; i--)
			    sp[i] = sp[i - 1];
			sp[insert_pos] = (CHAR) char_code;
			caret_shift = 1;
			val_change = 1;
			sp[orig_len + 1] = '\0';
		    } else		/* no more room */
			text_alarm(ip);
		}
		else {
	/*  dp->stored_length is a character count. 
	 *  Orig_len is a char count.
	 */
		    if (orig_len < dp->stored_length) {/* there is room */
			char_code = action;
			for (i = orig_len; i > insert_pos; i--)
			    sp[i] = sp[i - 1];
			sp[insert_pos] = (CHAR) char_code;
			caret_shift = 1;
			val_change = 1;
			sp[orig_len + 1] = '\0';
		    } else		/* no more room */
			text_alarm(ip);
		}
#else
		if (orig_len < dp->stored_length) {	/* there is room */
		    char_code = action;
		    for (i = orig_len; i > insert_pos; i--)
			sp[i] = sp[i - 1];
		    sp[insert_pos] = (CHAR) char_code;
		    caret_shift = 1;
		    val_change = 1;
		    sp[orig_len + 1] = '\0';
		} else		/* no more room */
		    text_alarm(ip);
#endif /* OW_I18N */

	    } else			/* must be read-only */
		text_alarm(ip);
	}
	break;

    }  /* switch (action) */

    if (selection_action)
	orig_caret_position = dp->caret_position;

    /* determine the new caret offset and position */
    orig_offset = dp->value_offset;
    orig_text_rect_width = dp->text_rect.r_width;
    update_value_offset(ip, val_change, val_shift, 1);
    dp->caret_offset = -1;	/* caret offset is no longer valid */
    update_caret_offset(ip, caret_shift, 0);
    update_text_rect(ip);

    if (selection_action &&
	    /* A selection keyboard command was issued */
	(dp->caret_position != orig_caret_position ||
	    /* The caret moved */
	 action == ACTION_SELECT_ALL)
	) {
        /* caret_position references the character following the caret. */
	switch (action) {
	  case ACTION_SELECT_ALL:
	    dp->seln_first[PANEL_SEL_PRIMARY] = 0;
#ifdef OW_I18N
	    dp->seln_last[PANEL_SEL_PRIMARY] = wslen(dp->value_wc) - 1;
#else
	    dp->seln_last[PANEL_SEL_PRIMARY] = strlen(dp->value) - 1;
#endif /* OW_I18N */
	    break;
	  case ACTION_SELECT_CHAR_FORWARD:
	  case ACTION_SELECT_WORD_END:
	  case ACTION_SELECT_LINE_END:
	   /* For forward movements, we don't want to include the character
	    * following the new caret position in the selection.
	    */
	    if (dp->select_click_cnt[PANEL_SEL_PRIMARY] == 0) {
		dp->seln_first[PANEL_SEL_PRIMARY] = orig_caret_position;
		dp->seln_last[PANEL_SEL_PRIMARY] = dp->caret_position - 1;
	    } else {
		if (dp->seln_last[PANEL_SEL_PRIMARY] < dp->caret_position - 1)
		    /* We're extending the selection to the right */
		    dp->seln_last[PANEL_SEL_PRIMARY] = dp->caret_position - 1;
		else if (dp->seln_first[PANEL_SEL_PRIMARY] == orig_caret_position)
		    /* We're shrinking the selection from the left */
		    dp->seln_first[PANEL_SEL_PRIMARY] = dp->caret_position;
	    }
	    break;
	  default:
	    /*   For backward movements, we don't want to include the character
	     * after the old caret position in the selection.
	     *   Swap dp->seln_first with dp->seln_last so that dp->seln_first
	     * is <= dp->seln_last.
	     */
	    if (dp->select_click_cnt[PANEL_SEL_PRIMARY] == 0) {
		dp->seln_last[PANEL_SEL_PRIMARY] = orig_caret_position - 1;
		dp->seln_first[PANEL_SEL_PRIMARY] = dp->caret_position;
	    } else {
		if (dp->seln_first[PANEL_SEL_PRIMARY] > dp->caret_position)
		    /* We're extending the selection to the left */
		    dp->seln_first[PANEL_SEL_PRIMARY] = dp->caret_position;
		else if (dp->seln_last[PANEL_SEL_PRIMARY] == orig_caret_position - 1)
		    /* We're shrinking the selection from the right */
		    dp->seln_last[PANEL_SEL_PRIMARY] = dp->caret_position - 1;
	    }
	    break;
	}
	/* Set variables to indicate that there's an active primary selection */
	dp->delete_pending = TRUE;
	dp->select_click_cnt[PANEL_SEL_PRIMARY] = 1;
	/* Acquire the Primary Selection and highlight the text */
	if (xv_set(panel->sel_owner[PANEL_SEL_PRIMARY], SEL_OWN, TRUE, NULL)
	    == XV_OK) {
	    if (panel->sel_holder[PANEL_SEL_PRIMARY])
		text_seln_dehighlight(panel->sel_holder[PANEL_SEL_PRIMARY],
				      PANEL_SEL_PRIMARY);
	    panel->sel_holder[PANEL_SEL_PRIMARY] = ip;
#ifndef OW_I18N
	    text_set_sel_data(panel, dp, PANEL_SEL_PRIMARY);
#endif
	    text_seln_highlight(panel, ip, PANEL_SEL_PRIMARY);
	}
    }

    /* update the display */
#ifdef OW_I18N
    if (dp->mask_wc == ' ' || hidden(ip))
#else
    if (dp->mask == ' ' || hidden(ip))
#endif /* OW_I18N */
	return;
#ifdef OW_I18N
    else if (dp->mask_wc ||
#else
    else if (dp->mask ||
#endif /* OW_I18N */
	action == ACTION_DRAG_COPY || action == ACTION_DRAG_MOVE ||
	action == ACTION_CUT || action == ACTION_PASTE || action == ACTION_UNDO)
	paint_value(ip, PV_HIGHLIGHT);
    else {
	/* compute the position of the caret */
	x = dp->text_rect.r_left + dp->caret_offset;
#ifdef OW_I18N
	new_len = wslen(sp);
#else
	new_len = strlen(sp);
#endif /* OW_I18N */
	was_clipped = dp->first_char || dp->last_char < new_len - 1;
	/* erase deleted characters that were displayed */
	if (new_len < orig_len) {
	    /* repaint the whole value if needed */
	    if (was_clipped || dp->text_rect.r_width != orig_text_rect_width)
		paint_value(ip, PV_HIGHLIGHT);
	    else {
		/* clear the deleted characters and everything to the right */
		rect.r_left = x;
		rect.r_top = ip->value_rect.r_top;
		rect.r_width = orig_offset - dp->caret_offset;
		rect.r_height = ip->value_rect.r_height - 2*LINE_Y;
		panel_clear_rect(panel, rect);
		PANEL_EACH_PAINT_WINDOW(panel, pw)
#ifdef OW_I18N
		    panel_paint_text(pw, ip->value_fontset_id,
			ip->color_index, x,
			ip->value_rect.r_top + dp->font_home,
			&sp[insert_pos]);
#else
		    panel_paint_text(pw, ip->value_font_xid,
			ip->color_index, x,
			ip->value_rect.r_top + dp->font_home,
			&sp[insert_pos]);
#endif /* OW_I18N */
		PANEL_END_EACH_PAINT_WINDOW
	    }

	} else if (new_len > orig_len) {
#ifndef OW_I18N
	    x_font_info = (XFontStruct *) xv_get(ip->value_font, FONT_INFO);
#endif /* OW_I18N */
	    /* repaint the whole value if it doesn't fit */
	    if (was_clipped)
		paint_value(ip, PV_HIGHLIGHT);
	    else {
		/* write the new character to the left of the caret */
#ifdef OW_I18N
                pc_adv_x = XwcTextEscapement(ip->value_fontset_id, &sp[insert_pos], 1);
                XwcTextExtents(ip->value_fontset_id, 
			       &sp[insert_pos],
                               wslen(&sp[insert_pos]),
                               &overall_ink_extents, 
			       &overall_logical_extents);
		PANEL_EACH_PAINT_WINDOW(panel, pw)
		    DRAWABLE_INFO_MACRO(pw, info);
		    XClearArea(xv_display(info), xv_xid(info),
			x - pc_adv_x, ip->value_rect.r_top,
                        overall_logical_extents.width, 
			overall_logical_extents.height, False);
		    panel_paint_text(pw, ip->value_fontset_id,
			ip->color_index, x - pc_adv_x,
			ip->value_rect.r_top + dp->font_home,
			&sp[insert_pos]);
		PANEL_END_EACH_PAINT_WINDOW
#else
		if (x_font_info->per_char)  {
		    pc_adv_x = x_font_info->per_char[(u_char) sp[insert_pos] -
			x_font_info->min_char_or_byte2].width;
		} else
		    pc_adv_x = x_font_info->min_bounds.width;
		XTextExtents(x_font_info, &sp[insert_pos],
			     strlen(&sp[insert_pos]),
			     &direction, &ascent, &descent, &overall);
		PANEL_EACH_PAINT_WINDOW(panel, pw)
		    DRAWABLE_INFO_MACRO(pw, info);
		    XClearArea(xv_display(info), xv_xid(info),
			x - pc_adv_x, ip->value_rect.r_top,
			overall.width, ascent + descent, False);
		    panel_paint_text(pw, ip->value_font_xid,
			ip->color_index, x - pc_adv_x,
			ip->value_rect.r_top + dp->font_home,
			&sp[insert_pos]);
		PANEL_END_EACH_PAINT_WINDOW
#endif /* OW_I18N */
	    }
	} else
	    /* Cursor key causes display shift */
	if (val_shift)
	    paint_value(ip, PV_HIGHLIGHT);
    }
}

/*
 * Calculate the first and last char that will be visible in
 * the text field.  Note, if an editing action is occuring,
 * we want the cursor to be visible after the action has occured.
 * However, if user is clicking on the scrollbar, ignore the
 * caret position and shift the string in the specified
 * direction. In the future, it might be helpful to rewrite
 * update_value_offset() and update_caret_offset().
 *
 * Algorithm follows as such...
 *
 * Will text fit into field without scrollbuttons?
 *   Yes... then exit.
 *
 * Text will not fit into field without scrollbuttons...
 *   Determine what we feel will be first char that is visible.
 *   If 1st char visible isn't 1st char of text then add
 *     a scrollbutton to the left side.
 *   Calculate last character visible.
 *   If last character isn't last char of text then add
 *     a scrollbutton on the right side.
 *   After we've calculated first and last character that
 *     will be visible, determine if caret is still visible.
 *     If caret isn't visible, then we must make some additional
 *     adjustments to ensure that caret is still visible.
 */


static void
update_value_offset(ip, val_change, val_shift, caret_sensitive)
    Item_info      *ip;
    int             val_change;	/* number of characters added (+) or deleted
				 * (-) */
    int             val_shift;	/* number of characters to shift value
				 * display */
    int		    caret_sensitive;
{
    register Text_info *dp = TEXT_FROM_ITEM(ip);
#ifndef OW_I18N
    XFontStruct	   *x_font_info;
#endif
    int             full_len;
    struct pr_size  size;
    int             max_caret = ip->value_rect.r_width;
    int             i, x, y;
    int             max_width;
    int		    cpos;

#ifdef OW_I18N
    full_len = wslen(dp->value_wc);
    size.x =  XwcTextEscapement(ip->value_fontset_id, dp->value_wc,
				    full_len);
    if (size.x <= max_caret) {
	size.x = XwcTextEscapement(ip->value_fontset_id,
				   dp->value_wc,
				   full_len);
	dp->first_char = 0;
	dp->last_char = full_len - 1;
	dp->value_offset = size.x;
    }
#else
    full_len = strlen(dp->value);
    size = xv_pf_textwidth(full_len, ip->value_font, dp->value);

    x_font_info = (XFontStruct *)xv_get(ip->value_font, FONT_INFO);

    if (size.x <= max_caret) {
	size = xv_pf_textwidth(full_len, ip->value_font, dp->value);
	dp->first_char = 0;
	dp->last_char = full_len - 1;
	dp->value_offset = size.x;
    }
#endif /* OW_I18N */
    else {			/* there are more characters than can be
				 * displayed */

	if (val_change > 0) {
	    /* Add a character */

	    /*
	     * Inserted characters will always be visible and the caret is
	     * always positioned after the inserted character, unless the
	     * caret is already positioned after the last displayable
	     * character, in which case all the characters to the left of the
	     * inserted character are shifted to the left on the display.
	     */
	    if (dp->caret_position > dp->last_char) {
		/*
		 * We are appending characters to the end of the string.
		 * Compute first_char = first character that can be fully
		 * displayed when the current (just-typed) character is the
		 * last character displayed.
		 */
		x = 0;
		max_width = max_caret - dp->scroll_btn_width;
#ifdef OW_I18N
		if (dp->caret_position < (int)wslen(dp->value_wc) - 1)
		    max_width -= dp->scroll_btn_width;
		for (i = dp->caret_position; x < max_width; i--)  {
                    x += XwcTextEscapement(ip->value_fontset_id, 
					&dp->value_wc[i], 1);
                }
#else
		if (dp->caret_position < (int)strlen(dp->value) - 1)
		    max_width -= dp->scroll_btn_width;
		for (i = dp->caret_position; x < max_width; i--)  {
		    if (x_font_info->per_char)  {
		        x += x_font_info->per_char[(u_char)dp->value[i] -
			    x_font_info->min_char_or_byte2].width;
		    } else
		        x += x_font_info->min_bounds.width;
                }
#endif /* OW_I18N */
		dp->first_char = i + 2;
	    }
	} else if (val_change < 0) {
	    /* Delete 1 or more characters */
	    dp->first_char += val_change;
	    if (dp->first_char < 0)
		dp->first_char = 0;	/* no more clip at left */
	} else {
	    /* Shift the display */
	    dp->first_char += val_shift;
	}

	/*
	 * dp->last_char = the last character in the string, starting from
	 * first_char, that can be fully displayed within the rectangle
	 * reserved for the text value string.  If the last character in the
	 * string cannot be displayed, then recompute dp->last_char to
	 * accomodate the right arrow scroll button.
	 */
	if (dp->first_char)
	    max_caret -= dp->scroll_btn_width;
#ifdef OW_I18N
	dp->last_char = char_position(max_caret, ip->value_font,
	    &dp->value_wc[dp->first_char], FALSE) - 1 + dp->first_char;
	if (dp->last_char < (int)wslen(dp->value_wc) - 1)
#else
	dp->last_char = char_position(max_caret, ip->value_font,
	    &dp->value[dp->first_char], FALSE) - 1 + dp->first_char;
        /*
         * if last character isn't visible, then right scrollbutton
         * must be added.
         */
	if (dp->last_char < (int)strlen(dp->value) - 1)
#endif /* OW_I18N */
	{
	    /*
	     * Decrement dp->last_char until enough space has been made to
	     * draw the right arrow scroll button.
	     */
	    for (x = 0; x < dp->scroll_btn_width;)  {
#ifdef OW_I18N
                x += XwcTextEscapement(ip->value_fontset_id, 
				&dp->value_wc[dp->last_char--], 1);
#else
		if (x_font_info->per_char)  {
		    x += x_font_info->per_char[(u_char)dp->value[dp->last_char--]
			- x_font_info->min_char_or_byte2].width;
		} else
		    x += x_font_info->min_bounds.width;
#endif /* OW_I18N */
            }
        /*
           After traversing through the above logic, the first and
           last visible character have been calculated however
           the caret may no longer be visible.  Therefore, if a char
           was inserted or deleted, we need to make sure the caret
           is visible.  If the caret isn't visible, then readjustment
           of first and last char is necessary. (1063744)

           Additionally, if no characters are inserted or deleted but
           caret is moved, then we must also check to make sure that
           caret falls between first and last char. (1073115)

           Last argument passed to this function is "caret_sensitive"
           If we're not caret_sensitive... meaning that we don't
           care if the caret is visible, then don't do these
           additional calculations.  Note: the only time we don't
           care about seeing the cursor is after a scrollbutton
           click.  After ANY caret navigation OR editing operation
           the caret should be displayed i.e. fall between first
           and last character of visible portion of text field.
         */
	    /* 1063744 */
            if (caret_sensitive)
            {
              cpos = dp->caret_position;
              if (val_shift)
                cpos += val_shift;
              if (val_change < 0)
                cpos += val_change;

              /*
               * determine the char width between last char
               * and the char that precedes caret.
               */
              x=0;
              while (cpos > dp->last_char+1)
              {
#ifdef OW_I18N
		x += XwcTextEscapement(ip->value_fontset_id, 
				&dp->value_wc[cpos], 1);
#else
                if (x_font_info->per_char)
                {
                  x +=
                    x_font_info->per_char[(u_char)dp->value[cpos]
                    - x_font_info->min_char_or_byte2].width;
                }
                else
                  x += x_font_info->min_bounds.width;
#endif /* OW_I18N */
                cpos--;
              }
 
              /* 
               * if caret past last character, then we've got some
               * shifting to do.
               */
 
 
              /* 
               * If 1st character is visible, then we
               * must take into account the left scrollbutton
               * width, i.e. before we start shifting the visible
               * character string to the right, if we haven't
               * already accounted for the left scrollbutton, then
               * we must do that now.
               */
              if (x>0)
              {  
                if (!dp->first_char)
                {
                  for (y = 0; y < dp->scroll_btn_width;)
                  {
#ifdef OW_I18N
		      y += XwcTextEscapement(ip->value_fontset_id, 
				&dp->value_wc[dp->first_char], 1);
#else
                    if (x_font_info->per_char)
                    {
                      y +=
			x_font_info->per_char[(u_char)dp->value[dp->first_char]
                        - x_font_info->min_char_or_byte2].width;
                    }
                    else
                      y += x_font_info->min_bounds.width;
#endif /* OW_I18N */
                    dp->first_char++;
                  }
                }
              } /* x>0 */

              /*
               * okay... we've gotten this far... caret is still
               * positioned past last char.  Start shifting the
               * sting we're going to view until the caret is
               * is visible.
               */
              while (x>0)
              {  
#ifdef OW_I18N
		  x -= XwcTextEscapement(ip->value_fontset_id, 
				&dp->value_wc[dp->first_char], 1);
#else
                if (x_font_info->per_char)
                {
                  x -=
		    x_font_info->per_char[(u_char)dp->value[dp->first_char]
                    - x_font_info->min_char_or_byte2].width;
                } else
                  x -= x_font_info->min_bounds.width;
#endif /* OW_I18N */
                dp->first_char++;
              }
 
              /*
               * caret should now be visible because we the first
               * character we are displaying is now "closer" to the
               * caret.  Now calculate the last character to be
               * displayed.
               */
#ifdef OW_I18N
              dp->last_char = char_position(max_caret, ip->value_font,
                &dp->value_wc[dp->first_char], FALSE) - 1 + dp->first_char;
#else
              dp->last_char = char_position(max_caret, ip->value_font,
                &dp->value[dp->first_char], FALSE) - 1 + dp->first_char;
#endif /* OW_I18N */
 
 
 
              /*
               * And if the last character isn't the last char of
               * the text field, then we need to put up the right
               * scroll button
               */
#ifdef OW_I18N
              if (dp->last_char < (int)wslen(dp->value_wc) - 1)
              {  
                for (x = 0; x < dp->scroll_btn_width;)
                {
		  x += XwcTextEscapement(ip->value_fontset_id, 
				&dp->value_wc[dp->last_char], 1);
#else
              if (dp->last_char < (int)strlen(dp->value) - 1)
              {  
                for (x = 0; x < dp->scroll_btn_width;)
                {
                  if (x_font_info->per_char)
                  {
                    x += x_font_info->per_char[(u_char)dp->value[dp->last_char]
                        - x_font_info->min_char_or_byte2].width;
                  }
                  else
                    x += x_font_info->min_bounds.width;
#endif /* OW_I18N */
                  dp->last_char--;
                }
              }
                
 
            } /* caret_sensitive */
        }
	/* 1063744 */

	/* Compute value offset */
#ifdef OW_I18N
	size.x = XwcTextEscapement(ip->value_fontset_id,
				   &dp->value_wc[dp->first_char],
				   dp->last_char - dp->first_char + 1);
#else
	size = xv_pf_textwidth(dp->last_char - dp->first_char + 1,
			       ip->value_font,
			       &dp->value[dp->first_char]);
#endif /* OW_I18N */
	dp->value_offset = size.x;
    }
}
#ifdef OW_I18N
/*
 * ml_panel_display_interm(ip)
 * displays the intermediate text str at the end of the panel text item.
 * The method to display the interm region is to display it as plain text
 * first, then reverse it, write underline to it, etc.
 * visible_type and visible_pos tells how to display it if the interm text
 * is longer than the panel text display length.  
 */
Xv_private void
ml_panel_display_interm(ip)
    Item_info		*ip;
{
    Text_info 		*dp = TEXT_FROM_ITEM(ip);
    int			 interm_display_len; /* intem display len in x axis */
    int			 real_display_len;   /* actualy display len in x asix
				     * for the text item.
						     */
    wchar_t 		*str;
    XIMFeedback  	*attr;
    int			 has_caret = ip->panel->kbd_focus_item == ip;

    /* set up */
    interm_display_len = XwcTextEscapement(ip->panel->std_fontset_id, 
	ip->panel->preedit->text->string.wide_char,
	ip->panel->preedit->text->length);
    str = ip->panel->preedit->text->string.wide_char;
    attr = ip->panel->preedit->text->feedback;
    /* calculate real display length */
    real_display_len = ip->value_rect.r_width;
    if (dp->first_char)
   	real_display_len-=dp->scroll_btn_width;
    if (dp->last_char < wslen(dp->value_wc) - 1)
   	real_display_len-=dp->scroll_btn_width;

    if (ip->panel->preedit->text->length == 0)
	paint_value(ip, 0);

    if (has_caret)
	paint_caret(ip, FALSE);
	
    if (interm_display_len >= real_display_len) {
	/*
	 * Interm columns is longer than the display length, we check
	 * visible_pos and visible_type to decide how to display.
	 */
#ifdef notdef
	wchar_t		saved_wc;
	int		prev_pos;
	int		display_length;
#endif

/*  FIX_ME:  The new XIM spec does not have visible position
 *  nor visible type, so have to nuke the following code to
 *  meet the spec.  But what to do?  This is a design issue
 */
    } else {
	 /*
	 * The interm text is displayed to the right of the panel text
	 * with right attributes.
	 */
	paint_value_and_interm(ip, str, attr);
    }
}

/*
 * ml_panel_moded_interm(ip, left, str, attr) paints invert, underline,
 * bold, or shaded to the displayed interm region according the attributes.
 * left is the start of the interm region in pixels.
 * str is expected to be null terminated.
 */
static void
ml_panel_moded_interm(ip, left, str, attr)
    Item_info		*ip;
    coord		 left;
    wchar_t		*str; 
    XIMFeedback  	*attr;

{
    Rect		 interm_rect;
    int			 count;
    int			 orig_count; 
    int			 attr_count; 
    int			 y;
    int			 interm_len;
    int			 adv_x;
    Xv_Window		 pw;
    Xv_Drawable_info	*info;
    Xv_Screen		 screen;
    GC			*openwin_gc_list;
    XIMFeedback		 bad_attr;

    interm_len = wslen(str);
    /* now paint special effects according to attributes */
    interm_rect = ip->value_rect; 
    interm_rect.r_left = left;
    y = rect_bottom(&ip->value_rect);
    count = 0;
    while (count < interm_len) {
	switch(attr[count]) {
	case XIMReverse:
	    orig_count = count;
	    while ((count < interm_len) && (attr[count] == XIMReverse))
		    count++;
	    attr_count = count - orig_count;
	    adv_x = XwcTextEscapement(ip->panel->std_fontset_id, 
			&str[orig_count], attr_count);
	    interm_rect.r_width = adv_x;
	    panel_invert(ip->panel, &interm_rect, ip->color_index);
	    interm_rect.r_left += adv_x;
	    break;
	case XIMUnderline:
	    orig_count = count;
	    while ((count < interm_len) && (attr[count] == XIMUnderline))
		    count++;
	    
	    attr_count = count - orig_count;
	    adv_x = XwcTextEscapement(ip->panel->std_fontset_id, 
			&str[orig_count], attr_count);
	    interm_rect.r_width = adv_x;
	    PANEL_EACH_PAINT_WINDOW(ip->panel, pw)
	    if (ip->color_index >= 0) {
		xv_vector(pw, interm_rect.r_left, y-1, 
			  interm_rect.r_left+interm_rect.r_width-1, 
			  y-1, ip->color_index < 0 ? PIX_SET :
			  PIX_SRC | PIX_COLOR(ip->color_index),
			  0);
	     } else {
		DRAWABLE_INFO_MACRO(pw, info);
		screen = xv_screen(info);
		openwin_gc_list = (GC *) xv_get( screen, SCREEN_OLGC_LIST, pw);
		XDrawLine(xv_display(info), xv_xid(info),
			  openwin_gc_list[OPENWIN_SET_GC],
			  interm_rect.r_left, y-2, 
			  interm_rect.r_left+interm_rect.r_width-1, 
			  y-2);
	    }
	    PANEL_END_EACH_PAINT_WINDOW
	    interm_rect.r_left += adv_x;
	    break;
	
	default:
	    /* Bad attr, treat it like XIMPlain */
	    bad_attr = attr[count];
	    orig_count = count;
	    /* skip and do nothig */
	    while ((count < interm_len) && (attr[count] == bad_attr))
		    count++;
	    
	    attr_count = count - orig_count;
	    adv_x = XwcTextEscapement(ip->panel->std_fontset_id, 
			&str[orig_count], attr_count);
	    interm_rect.r_left += adv_x;
	    break;
	}
    }
}

#ifdef notdef
/*
 * ml_panel_simply_display(ip, str, attr) displays str and attr
 * according to textdp(ip)->display_length.  Here, str cannot
 * be displayed in full and we start to display from str
 * and see how far we can go.
 */
static void
ml_panel_simple_display(ip, str, attr)
    register Item_info		*ip;
    register wchar_t		*str;
    register XIMFeedback	*attr;
{
    register Text_info		*dp = TEXT_FROM_ITEM(ip);
    register int		 j; 	/* counting display in x axis */
    register int		 last_pos; /* possible last index */
    wchar_t			 saved_wc;
    int				 real_display_len;


    /* compute real displayable length */
    real_display_len = ip->value_rect.r_width;
    if (dp->first_char)
	    real_display_len-=dp->scroll_btn_width;
    if (dp->last_char < wslen(dp->value_wc) - 1)
	    real_display_len-=dp->scroll_btn_width;

    /*
     * Find the last char we can display, then put a NULL
     * there temporarily and display.
     */
    j = 0;
    last_pos = 0;
    while (j < real_display_len) {
	j += XwcTextEscapement(ip->panel->std_fontset_id, 
				&str[last_pos], 1);
	last_pos++;
    }
    /* Sometimes when boundary is right in the middle */
    /* of a character, j will then go beyond real_display_legth. */
    /* In that case, we need to decrement last_pos by 1 */
    if (j > real_display_len) last_pos -= 1;


    /* Save last char displayable */
    saved_wc = str[last_pos];

    /* set dummy data to run procedure correctly */
    str[last_pos] = (wchar_t)NULL;

    paint_value_and_interm(ip, str, attr);

    /* Restore last char displayable */
    str[last_pos] = saved_wc;
}
#endif

#define	has_right_arrow(x) ((x)->last_char < (strlen((x)->value) - 1) ? 1 : 0)

/* paint_value_and_interm() does
 *	1. clear value rect
 *	2. draw left arrow if needed
 *	3. draw value of left hand side of caret
 *	4. draw intermediate text
 *	5. draw value of right hand side of caret
 *	6. draw right arrow if needed
 */
static void
paint_value_and_interm(ip, interm_str, interm_attr)
    register Item_info		*ip;
    register wchar_t		*interm_str;
    register XIMFeedback  	*interm_attr;
{
    register Text_info		*dp = TEXT_FROM_ITEM(ip);
    register int		 x = ip->value_rect.r_left;
    register int		 y = ip->value_rect.r_top;
    register Panel_info		*panel = ip->panel;
    int				 caret_offset;
    int				 insert_pos;
    int				 interm_display_len;
    int				 real_display_len;
    wchar_t			*str_left;
    wchar_t			*str_right;
    int				 i, j, len;
    Xv_Drawable_info		*info;
    Xv_Window			 pw;
#ifdef	INTERM_SCROLL
    int				 value_right_len;
    int				 value_right_display_len;
#endif	INTERM_SCROLL

    /* Get the column position from current caret position */
    caret_offset = dp->saved_caret_offset;
    if (caret_offset < 0) {
	caret_offset = 0;
	insert_pos = 0;
    }
    else {
	/* Assuming saved_caret_offset and 
	 * saved_caret_position is correct */
	insert_pos =  dp->saved_caret_position;
    }


    /***************************************************/
    /* store panel_value to str_left[] and str_right[] */
    /***************************************************/

    interm_display_len = XwcTextEscapement(ip->panel->std_fontset_id, 
				interm_str, wslen(interm_str));


    /* compute real displayable length */
    real_display_len = ip->value_rect.r_width;
    if (dp->first_char)
	real_display_len-=dp->scroll_btn_width;
    if (dp->last_char < wslen(dp->value_wc) - 1)
   	real_display_len-=dp->scroll_btn_width;

#ifdef	INTERM_SCROLL
    /* if there is no right scroll button, but real_display_len */
    /* is not enough to accomodate left str + interm + right str */
    /* Then we have to temporarily add scroll button on right side */ 
	
    value_right_len = wslen(&dp->value_wc[insert_pos]);
    value_right_display_len = XwcTextEscapement(ip->panel->std_fontset_id,
                        &dp->value_wc[insert_pos], value_right_len);
    if ((dp->last_char == (wslen(dp->value_wc) - 1)) && 
	    ((interm_display_len + dp->saved_caret_offset + 
	    value_right_display_len) > real_display_len)) {
   	real_display_len-=dp->scroll_btn_width;
	temp_last_char = 1;
    }
#endif	INTERM_SCROLL


    if (interm_display_len + dp->saved_caret_offset < 
		real_display_len) {
	/*
	 * store left hand side of caret into str_left[]
	 */
	if (insert_pos > dp->first_char) {
	    len = insert_pos - dp->first_char + 1;
	    str_left = (wchar_t *) xv_alloc_n(wchar_t, len);
	    for (j = 0, i = dp->first_char; i < insert_pos; i++, j++)
		str_left[j] = dp->value_wc[i];
	    str_left[j] = (wchar_t) 0;
	}
	else
	    str_left = (wchar_t *)0;

	/*
	 * store right hand side of caret into str_right[]
	 */
	if (dp->last_char + 1 > insert_pos) {
	    int	display_length;
	    int	prev_pos;

	    len = dp->last_char - insert_pos + 2; /* allocate maximum size */
	    str_right = (wchar_t *) xv_alloc_n(wchar_t, len);

	   
	    display_length = real_display_len - 
			interm_display_len - dp->saved_caret_offset;
	    if (display_length != 0) {
		prev_pos = insert_pos;
		i = insert_pos;
		j = 0;
		while ((j < display_length) && (i < dp->last_char + 1)) {
		    prev_pos = i;
		    j += XwcTextEscapement(ip->panel->std_fontset_id, 
				&dp->value_wc[i], 1);
		    i++;
		}
		/* Sometimes when boundary is right in the middle */
		/* of a character, j will then go beyond display_legth. */
		/* In that case, we need to decrement pre_pos by 1 */
		if (j > display_length) prev_pos -= 1;
		(void) wsncpy(str_right, &dp->value_wc[insert_pos],
				    prev_pos - insert_pos + 1);
		str_right[prev_pos - insert_pos +1] = (wchar_t) 0;
	    } 
	    else {
		/* No need to store str_right anymore. Free memory and */
		/* set str_right to NULL.			       */
		if (str_right) xv_free(str_right);
	    	str_right = (wchar_t *)0;
	    }
	}
	else
	    str_right = (wchar_t *)0;
    }
    else {
	/*
	 * store left hand side of caret into str_left[]
	 */
	if (insert_pos > dp->first_char) {
	    int	display_length;
	    int	prev_pos;

	    len = insert_pos - dp->first_char + 1;
	    str_left = (wchar_t *) xv_alloc_n(wchar_t, len);

#ifdef	INTERM_SCROLL
	    /* since left hand side + interm > display, then we must */
	    /* temporarily add scroll arrow to the left side if it   */
            /* is not there yet. */
	    if (dp->first_char == 0) {
		real_display_len-=dp->scroll_btn_width;
		temp_first_char = 1;
	     }
#endif	INTERM_SCROLL
	
	    display_length = real_display_len - interm_display_len;
				

	    if (display_length != 0) {
	 	prev_pos = insert_pos -1;
	    	j = 0;
	    	i = insert_pos - 1;
 	    	while ((j < display_length) && (i >= dp->first_char)) {
		    prev_pos = i;
		    j += XwcTextEscapement(ip->panel->std_fontset_id, 
				&dp->value_wc[i], 1);
		    i--;
	    	}
	    	/* Sometimes when boundary is right in the middle */
	    	/* of a character, j will then go beyond display_legth. */
	    	/* In that case, we need to increment pre_pos by 1 */
	    	if (j > display_length) prev_pos += 1;
	    	(void) wsncpy(str_left, &dp->value_wc[prev_pos],
				insert_pos - prev_pos);
	    	str_left[insert_pos - prev_pos] = (wchar_t) 0;
	    }
	    else {
		/* No need to store str_right anymore. Free memory and */
		/* set str_right to NULL.			       */
		if (str_left) xv_free(str_left);
	    	str_left = (wchar_t *)0;
	    }
	}
	else
	    str_left = (wchar_t *)0;

	/*
	 * not store into str_right[] because interm text
	 * fill remaining space
	 */
	str_right = (wchar_t *)0;
    }




    /************************/
    /* clear the value rect */ 
    /************************/

    /*
     * Set the colors for the Clear and Set GC's.
     */
    PANEL_EACH_PAINT_WINDOW(panel, pw)
        DRAWABLE_INFO_MACRO(pw, info);
	screen_adjust_gc_color(pw, SCREEN_CLR_GC);
        panel_clear_pw_rect(pw, ip->value_rect);
    PANEL_END_EACH_PAINT_WINDOW

    /**************************************/
    /* draw the left clip arrow if needed */
    /**************************************/

#ifdef	INTERM_SCROLL
    if ((dp->first_char) || (temp_first_char))
#else
    if (dp->first_char)
#endif	INTERM_SCROLL
            draw_scroll_btn(ip, OLGX_SCROLL_BACKWARD);


    /***********************/
    /* Draw Left Hand Side */
    /***********************/


#ifdef	INTERM_SCROLL
    if ((dp->first_char) || (temp_first_char))
#else
    if (dp->first_char)
#endif	INTERM_SCROLL
	x += dp->scroll_btn_width;
    if (dp->mask_wc == '\0') { /* not masked */
	if (str_left) {
	    PANEL_EACH_PAINT_WINDOW(panel, pw)
		panel_paint_text(pw, ip->panel->std_fontset_id, 
				 ip->color_index, x, 
				 y+dp->font_home, &str_left[0]);
            PANEL_END_EACH_PAINT_WINDOW
            x += XwcTextEscapement(ip->panel->std_fontset_id, 
			str_left, wslen(str_left));
	}
    } else {                /* masked */
        wchar_t         *buf;
        int              length, i;
        length = dp->last_char - dp->first_char + 2;
        buf = (wchar_t *) xv_alloc_n(wchar_t, length);
        for (j = 0, i = dp->first_char; i <= dp->last_char; 
			i++, j++) {
	    buf[j] = (wchar_t)dp->mask_wc; 
	    buf[length - 1] = (wchar_t) 0;
	}
        PANEL_EACH_PAINT_WINDOW(panel, pw)
                panel_paint_text(pw, ip->panel->std_fontset_id, 
				 ip->color_index, x, 
				 y+dp->font_home, buf);
        PANEL_END_EACH_PAINT_WINDOW
        x += XwcTextEscapement(ip->panel->std_fontset_id, 
			buf, wslen(buf));
        xv_free(buf);
    }

    /**************************/
    /* Draw Intermediate text */
    /**************************/

    PANEL_EACH_PAINT_WINDOW(ip->panel, pw)
	panel_paint_text(pw, ip->panel->std_fontset_id, 
			 ip->color_index, x, 
			 ip->value_rect.r_top +dp->font_home,
		 	 interm_str);
    PANEL_END_EACH_PAINT_WINDOW
	
    /* now paint special effects according to attributes */
    ml_panel_moded_interm(ip, x, interm_str, interm_attr);
    x += interm_display_len;

    /* update caret end of interm text */
    /* caret_offset is  caret's x offset from right margin */
    /* of left arrow (which may be blank). */
#ifdef	INTERM_SCROLL
    if ((dp->first_char) || (temp_first_char))
#else
    if (dp->first_char)
#endif	INTERM_SCROLL
	dp->caret_offset = x - ip->value_rect.r_left - dp->scroll_btn_width;
    else
   	dp->caret_offset = x - ip->value_rect.r_left;;
    /* display caret */
    paint_caret(ip, TRUE);


    /************************/
    /* Draw Right Hand Side */
    /************************/

    if (dp->mask_wc == '\0') {	/* not masked */
	if (str_right) {
	    PANEL_EACH_PAINT_WINDOW(panel, pw)
		panel_paint_text(pw, ip->panel->std_fontset_id, 
				 ip->color_index, x, 
				 y+dp->font_home, &str_right[0]);
            PANEL_END_EACH_PAINT_WINDOW
            x += XwcTextEscapement(ip->panel->std_fontset_id, 
			str_right, wslen(str_right));
	}
    }
    else {                /* masked */
        wchar_t         *buf;
        int              length, i;
        length = dp->last_char - dp->first_char + 2;
        buf = (wchar_t *) xv_alloc_n(wchar_t, length);
        for (j = 0, i = dp->first_char; i <= dp->last_char; 
			i++, j++) {
	    buf[j] = (wchar_t)dp->mask_wc; 
	    buf[length-1] = (wchar_t) 0;
	}
 
        PANEL_EACH_PAINT_WINDOW(panel, pw)
                panel_paint_text(pw, ip->panel->std_fontset_id, 
				 ip->color_index, x, 
				 y+dp->font_home, buf);
        PANEL_END_EACH_PAINT_WINDOW
        x += XwcTextEscapement(ip->panel->std_fontset_id, buf, wslen(buf));
        xv_free(buf);
    }

    /***************************************/
    /* draw the right clip arrow if needed */
    /***************************************/

#ifdef	INTERM_SCROLL
    if ((dp->last_char < (wslen(dp->value_wc) - 1))  || (temp_last_char))
#else
    if (dp->last_char < (wslen(dp->value_wc) - 1))
#endif	INTERM_SCROLL
            draw_scroll_btn(ip, OLGX_SCROLL_FORWARD);


    /* Underline the text (optional) */
    if (dp->flags & UNDERLINED) {
	y = rect_bottom(&ip->value_rect);
	if (ip->panel->status.three_d) {
	    /* 3D text ledge is 2 pixels high.  (2D is 1 pixel high.) */
	    y--;
	}
	PANEL_EACH_PAINT_WINDOW(panel, pw)
	    DRAWABLE_INFO_MACRO(pw, info);
	    olgx_draw_text_ledge(panel->ginfo, xv_xid(info),
	        ip->value_rect.r_left, y,
		ip->value_rect.r_width);
	PANEL_END_EACH_PAINT_WINDOW
    }

    if (str_left) xv_free(str_left);
    if (str_right) xv_free(str_right);

    /*
     * paint_value_and_interm() does not hilite selection, Because
     * selection while conversion mode does not happen.
     */
}

Pkg_private void
ml_panel_saved_caret(ip)
    Item_info	*ip;
{
    Text_info   *dp = TEXT_FROM_ITEM(ip);

    /* store the current_caret_offset */

    dp->saved_caret_offset = dp->caret_offset;
    dp->saved_caret_position = dp->caret_position;
}

Pkg_private void
panel_implicit_commit(ip)
    Item_info	*ip;
{
    Text_info	*dp = TEXT_FROM_ITEM(ip);
    Panel_info	*panel = ip->panel;
    Panel	 panel_public = PANEL_PUBLIC(panel);
    wchar_t	*committed_string = 0;
    wchar_t	*wc_ptr;
    int		 retstatus;
    int		 i = 0; /* loop counter */

/*  We are still in conversion mode and there is preedit text, so
 *  reset the ic (which turns off covnersion), get the committed string
 *  if there's any.  Be sure the saved caret position and caret offset is
 *  moved to after the implicitly committed text.  The window pkg will
 *  free the committed_string so we no longer need to free it when we're
 *  done with the string. Then we have to turn conversion back on and
 *  clean up panel's private copy of preedit text.
 * 
 *  What happens if it goes beyond the displayed length
 *  or the stored length of the panel text item?
 */
    xv_set(panel_public, WIN_IC_RESET, NULL);
    if ((committed_string = (wchar_t *)xv_get(panel_public,
		    WIN_IC_COMMIT_STRING_WCS)) != NULL) {
	    wc_ptr = committed_string;
	    paint_caret(ip, FALSE);
	    while (*wc_ptr) {
	    dp->undo_buffer_wc[i] = *wc_ptr;
	    update_value(ip, TRUE, *wc_ptr++, 1, FALSE, &retstatus);
	    i++;
	    }
	    dp->undo_buffer_wc[i] = '\0';
	    dp->undo_direction = DELETE;
	    dp->saved_caret_offset = dp->caret_offset;
	    dp->saved_caret_position = dp->caret_position;
    }
    xv_set(panel_public, WIN_IC_CONVERSION, TRUE, NULL);
    panel->preedit->text->string.wide_char = _xv_null_string_wc;

/*  Restore preedit_item handle because getting the
 *  committed string above caused conversion to be
 *  off and set the preedit_item handle to zero.
 *  Reassign panel->preedit_item pointer to point to the
 *  text item under focus
 */
    if ( (panel->kbd_focus_item->item_type == PANEL_TEXT_ITEM)
	  && (panel->preedit_item != panel->kbd_focus_item) ) {
	panel->preedit_item = panel->kbd_focus_item;
    }
}


static int
wslen_in_byte(wcs)
    wchar_t	*wcs;
{
    char	mb[MB_LEN_MAX + 1];
    int		byte_len;
    int		i;

    for (byte_len = 0; *wcs; wcs++) {
	if ((i = wctomb(mb, *wcs)) < 0)
	    break;
	byte_len += i;
    }

    return byte_len;
}
#endif /* OW_I18N */
