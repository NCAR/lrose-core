#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)p_num_txt.c 20.47 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#include <ctype.h>
#include <string.h>
#include <X11/X.h>
#include <xview/sun.h>
#include <xview_private/panel_impl.h>
#include <xview_private/draw_impl.h>
#include <xview/defaults.h>
#include <xview/screen.h>
#include <xview/pixwin.h>

/* Offset of increment button from text field's right edge of rect */
#define BUTTON_OFFSET		5

/* Macros */
#define inactive_btn(dp) (((dp)->btn_state & OLGX_INACTIVE) == OLGX_INACTIVE)
#define UP_SELECTED OLGX_SCROLL_BACKWARD
#define UP_INACTIVE OLGX_SCROLL_NO_BACKWARD
#define DOWN_SELECTED OLGX_SCROLL_FORWARD
#define DOWN_INACTIVE OLGX_SCROLL_NO_FORWARD

#define NUM_TEXT_PRIVATE(item)	\
	XV_PRIVATE(Num_text_info, Xv_panel_num_text, item)
#define NUM_TEXT_PUBLIC(item)	XV_PUBLIC(item)

#define	NUM_TEXT_FROM_ITEM(ip)	NUM_TEXT_PRIVATE(ITEM_PUBLIC(ip))


/* XView functions */
Pkg_private int     panel_num_text_init();
Pkg_private Xv_opaque panel_num_text_set_avlist();
Pkg_private Xv_opaque panel_num_text_get_attr();
Pkg_private int     panel_num_text_destroy();

/* Panel Item Operations */
static void	    num_txt_begin_preview();
static void	    num_txt_cancel_preview();
static void	    num_txt_accept_preview();
static void	    num_txt_paint();
static void	    num_txt_layout();

/* Local functions */
static int	    check_dimming();
static int	    get_value();
static Bool	    notify_user();
static void	    num_txt_paint_btn();
static void	    num_txt_paint_value();
static int	    set_value();
static Panel_setting text_notify_proc();
static Notify_value num_textitem_scroll_itimer_func();


static Panel_ops ops = {
    panel_default_handle_event,		/* handle_event() */
    num_txt_begin_preview,		/* begin_preview() */
    num_txt_begin_preview,		/* update_preview() */
    num_txt_cancel_preview,		/* cancel_preview() */
    num_txt_accept_preview,		/* accept_preview() */
    NULL,				/* accept_menu() */
    NULL,				/* accept_key() */
    panel_default_clear_item,		/* clear() */
    num_txt_paint,			/* paint() */
    NULL,				/* resize() */
    NULL,				/* remove() */
    NULL,				/* restore() */
    num_txt_layout,			/* layout() */
    NULL,				/* accept_kbd_focus() */
    NULL,				/* yield_kbd_focus() */
    NULL				/* extension: reserved for future use */
};


typedef struct {
    Panel_item	    public_self;   /* back pointer to object */
    Rect	    btn_rect;
    int		    btn_state;     /* OLGX state of numeric scroll button */
    int		    flags;
    int		    jump_delta;	   /* amount to add/subtract on jump up/down */
    int		    max_value;
    int		    min_value;
    Panel_setting   notify_level;  /* NONE, SPECIFIED, NON_PRINTABLE, ALL */
    char	   *terminators;
    Panel_item	    text_field;
#ifdef OW_I18N
    wchar_t	   *terminators_wc;
#endif /* OW_I18N */
} Num_text_info;

/* Numeric Text flags */
#define NTX_READ_ONLY 0x1



/* ========================================================================= */

/* -------------------- XView Functions  -------------------- */
/*ARGSUSED*/
Pkg_private int
panel_num_text_init(panel_public, item_public, avlist)
    Panel panel_public;
    Panel_item item_public;
    Attr_avlist avlist;
{
    Num_text_info  *dp = xv_alloc(Num_text_info);
    Item_info	   *ip = ITEM_PRIVATE(item_public);
    Xv_panel_num_text *item_object = (Xv_panel_num_text *) item_public;
    Panel_info	   *panel = PANEL_PRIVATE(panel_public);
    Rect	   *text_rect;

    item_object->private_data = (Xv_opaque) dp;
    dp->public_self = item_public;

    ip->ops = ops;
    if (panel->event_proc)
	ip->ops.panel_op_handle_event = (void (*) ())panel->event_proc;

    ip->item_type = PANEL_NUMERIC_TEXT_ITEM;

    panel_set_bold_label_font(ip);
    dp->btn_rect.r_width = NumScrollButton_Width(panel->ginfo);
    dp->btn_rect.r_height = NumScrollButton_Height(panel->ginfo);
    dp->btn_state = OLGX_ERASE | DOWN_INACTIVE;
    dp->jump_delta = 10;
    dp->max_value = 100;

    dp->notify_level = PANEL_SPECIFIED;
#ifdef OW_I18N
    dp->terminators_wc = (wchar_t *)_xv_mbstowcsdup("\n\r\t");
#else
    dp->terminators = (char *) panel_strsave("\n\r\t");
#endif /* OW_I18N */
    if (ip->notify == panel_nullproc)
	ip->notify = (int (*) ()) panel_text_notify;

    dp->text_field = xv_create(panel_public, PANEL_TEXT,
			       PANEL_ITEM_OWNER, item_public,
			       PANEL_VALUE, "0",
			       PANEL_NOTIFY_LEVEL, PANEL_ALL,
			       PANEL_NOTIFY_PROC, text_notify_proc,
			       PANEL_VALUE_DISPLAY_LENGTH, 12,
			       PANEL_VALUE_STORED_LENGTH, 12,
#ifdef OW_I18N
			       PANEL_ITEM_IC_ACTIVE, FALSE,
#endif /* OW_I18N */
			       XV_KEY_DATA, PANEL_NUM_TXT, item_public,
			       NULL);
    ip->child_kbd_focus_item = dp->text_field;
    text_rect = &ITEM_PRIVATE(dp->text_field)->rect;
    dp->btn_rect.r_left = text_rect->r_left + text_rect->r_width +
	BUTTON_OFFSET;
    dp->btn_rect.r_top = text_rect->r_top;

    (void) set_value(dp, 0);

    return XV_OK;
}


Pkg_private Xv_opaque
panel_num_text_set_avlist(item_public, avlist)
    Panel_item	    item_public;
    Attr_avlist	    avlist;
{
    Num_text_info  *dp = NUM_TEXT_PRIVATE(item_public);
    Item_info	   *ip = ITEM_PRIVATE(item_public);
    Bool	    check_for_dimming = FALSE;
    Bool	    jump_delta_set = FALSE;
    int		    new_value;
    Panel_info	   *panel = ip->panel;
    Bool	    pos_change = FALSE;
    Bool	    range_change = FALSE;
    Xv_opaque       result;
    Rect	   *text_rect;
    int		    value;
    Bool	    value_change = FALSE;
    Bool	    xv_end_create = FALSE;
#ifdef OW_I18N
    char            tmp_char;
    wchar_t         tmp_char_wc;
#endif /* OW_I18N */

    /*
     * If a client has called panel_item_parent this item may not have a
     * parent -- do nothing in this case
     */
    if (panel == NULL)
	return ((Xv_opaque) XV_ERROR);

    if (*avlist != XV_END_CREATE) {
	/* Parse Panel Item Generic attributes before Text Field attributes.
	 * Prevent panel_redisplay_item from being called in item_set_avlist.
	 */
	ip->panel->no_redisplay_item = TRUE;
	result = xv_super_set_avlist(item_public, &xv_panel_num_text_pkg,
				     avlist);
	ip->panel->no_redisplay_item = FALSE;
	if (result != XV_OK)
	    return result;
    }

    for (; *avlist; avlist = attr_next(avlist)) {
	switch (avlist[0]) {
	  case PANEL_EVENT_PROC:
	  case PANEL_ITEM_COLOR:
	  case PANEL_NEXT_COL:
	  case PANEL_NEXT_ROW:
	  case PANEL_VALUE_DISPLAY_LENGTH:
	  case PANEL_VALUE_DISPLAY_WIDTH:
	  case PANEL_VALUE_FONT:
	  case PANEL_VALUE_STORED_LENGTH:
	  case PANEL_VALUE_UNDERLINED:
	  case PANEL_VALUE_X:
	  case PANEL_VALUE_Y:
	  case XV_SHOW:
	    xv_set(dp->text_field, avlist[0], avlist[1], NULL);
	    pos_change = TRUE;
	    break;

	  case XV_KEY_DATA:
	    if (avlist[1] == XV_HELP)
		xv_set(dp->text_field, XV_HELP_DATA, avlist[2], NULL);
	    break;

	  case XV_X:
	  case PANEL_ITEM_X:
	    xv_set(dp->text_field, PANEL_ITEM_X, ip->value_rect.r_left, NULL);
	    pos_change = TRUE;
	    break;

	  case XV_Y:
	  case PANEL_ITEM_Y:
	    xv_set(dp->text_field, PANEL_ITEM_Y, ip->value_rect.r_top, NULL);
	    pos_change = TRUE;
	    break;

	  case PANEL_READ_ONLY:
	    value = (int) avlist[1];
	    xv_set(dp->text_field, PANEL_READ_ONLY, value, NULL);
	    if (value) {
		dp->flags |= NTX_READ_ONLY;
		dp->btn_state |= OLGX_INACTIVE;
	    } else {
		dp->flags &= ~NTX_READ_ONLY;
		dp->btn_state &= ~OLGX_INACTIVE;
		check_for_dimming = TRUE;
	    }
	    break;

	  case PANEL_INACTIVE:
	    value = (int) avlist[1];
	    xv_set(dp->text_field, PANEL_INACTIVE, value, NULL);
	    if (value)
		dp->btn_state |= OLGX_INACTIVE;
	    else {
		dp->btn_state &= ~OLGX_INACTIVE;
		check_for_dimming = TRUE;
	    }
	    break;

	  case PANEL_JUMP_DELTA:
	    dp->jump_delta = (int) avlist[1];
	    jump_delta_set = TRUE;
	    break;

#ifdef OW_I18N
          case PANEL_MASK_CHAR:
            value = (char) avlist[1];
            tmp_char = (char) value;
            mbtowc(&tmp_char_wc, &tmp_char, MB_CUR_MAX);
            xv_set(dp->text_field, PANEL_MASK_CHAR_WC, tmp_char_wc, NULL);
            value = (value != '\0');
            if (value)
                dp->btn_state |= OLGX_INACTIVE;
            else {
                dp->btn_state &= ~OLGX_INACTIVE;
                check_for_dimming = TRUE;
            }
            break;
          case PANEL_MASK_CHAR_WC:
            value = (wchar_t) avlist[1];
            xv_set(dp->text_field, PANEL_MASK_CHAR_WC, value, NULL);
            value = (value != '\0');
            if (value)
                dp->btn_state |= OLGX_INACTIVE;
            else {
                dp->btn_state &= ~OLGX_INACTIVE;
                check_for_dimming = TRUE;
            }
            break;
#else
	  case PANEL_MASK_CHAR:
	    value = (char) avlist[1];
	    xv_set(dp->text_field, PANEL_MASK_CHAR, value, NULL);
	    value = (value != '\0');
	    if (value)
		dp->btn_state |= OLGX_INACTIVE;
	    else {
		dp->btn_state &= ~OLGX_INACTIVE;
		check_for_dimming = TRUE;
	    }
	    break;
#endif /* OW_I18N */

	  case PANEL_VALUE:
	    value_change = TRUE;
	    new_value = (int) avlist[1];
	    break;

	  case PANEL_MIN_VALUE:
	    dp->min_value = (int) avlist[1];
	    range_change = TRUE;
	    break;

	  case PANEL_MAX_VALUE:
	    dp->max_value = (int) avlist[1];
	    range_change = TRUE;
	    break;

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
            dp->terminators_wc =
		(wchar_t *) panel_strsave_wc((wchar_t *) avlist[1]);
            break;
#else
	  case PANEL_NOTIFY_STRING:
	    if (dp->terminators)
		free(dp->terminators);
	    dp->terminators = (char *) panel_strsave((char *) avlist[1]);
	    break;
#endif /* OW_I18N */

	  case XV_END_CREATE:
	    xv_end_create = TRUE;
	    break;

	  default:
	    break;
	}
    }

    if (range_change) {
	if (dp->max_value < dp->min_value)
	    dp->max_value = dp->min_value;
	if (!jump_delta_set)
	    dp->jump_delta = (dp->max_value - dp->min_value + 1) / 10;
    }

    if (value_change)
	(void) set_value(dp, new_value);
    else if (range_change)
	(void) set_value(dp, get_value(dp));

    if (check_for_dimming)
	(void) check_dimming(dp);

    if (xv_end_create || pos_change) {
	xv_set(dp->text_field,
	       XV_X, ip->value_rect.r_left,
	       XV_Y, ip->value_rect.r_top,
	       NULL);
	text_rect = &ITEM_PRIVATE(dp->text_field)->rect;
	dp->btn_rect.r_left = text_rect->r_left + text_rect->r_width +
	    BUTTON_OFFSET;
	dp->btn_rect.r_top = text_rect->r_top;
	ip->value_rect = panel_enclosing_rect(text_rect, &dp->btn_rect);
	ip->rect = panel_enclosing_rect(&ip->label_rect, &ip->value_rect);
	panel_check_item_layout(ip);
    }

    return XV_OK;
}


/*ARGSUSED*/
Pkg_private Xv_opaque
panel_num_text_get_attr(item_public, status, which_attr, valist)
    Panel_item item_public;
    int  *status;
    register Attr_attribute which_attr;
    va_list valist;
{
    register Num_text_info *dp = NUM_TEXT_PRIVATE(item_public);

    switch (which_attr) {
      case PANEL_JUMP_DELTA:
	return (Xv_opaque) dp->jump_delta;

      case PANEL_VALUE:
	return (Xv_opaque) get_value(dp);

      case PANEL_MIN_VALUE:
	return (Xv_opaque) dp->min_value;

      case PANEL_MAX_VALUE:
	return (Xv_opaque) dp->max_value;

      case PANEL_NOTIFY_LEVEL:
	return (Xv_opaque) dp->notify_level;

#ifdef OW_I18N
      case PANEL_NOTIFY_STRING:
        dp->terminators = _xv_wcstombsdup((wchar_t *)dp->terminators_wc);
        return (Xv_opaque) dp->terminators;

      case PANEL_NOTIFY_STRING_WCS:
        return (Xv_opaque) dp->terminators_wc;
#else
      case PANEL_NOTIFY_STRING:
	return (Xv_opaque) dp->terminators;
#endif /* OW_I18N */

      case PANEL_VALUE_FONT:
      case PANEL_VALUE_STORED_LENGTH:
      case PANEL_VALUE_DISPLAY_LENGTH:
      case PANEL_VALUE_UNDERLINED:
      case PANEL_READ_ONLY:
      case PANEL_MASK_CHAR:
#ifdef OW_I18N
      case PANEL_MASK_CHAR_WC:
#endif /* OW_I18N */
	return xv_get(dp->text_field, which_attr);

      default:
	*status = XV_ERROR;
	return (Xv_opaque) 0;
    }
}


Pkg_private int
panel_num_text_destroy(item_public, status)
    Panel_item item_public;
    Destroy_status status;
{
    Num_text_info *dp = NUM_TEXT_PRIVATE(item_public);

    if (status == DESTROY_CHECKING || status == DESTROY_SAVE_YOURSELF)
	return XV_OK;

#ifdef OW_I18N
    if (dp->terminators_wc)
        xv_free( dp->terminators_wc );
#endif /* OW_I18N */
    if (dp->terminators)
        xv_free( dp->terminators );
    xv_destroy(dp->text_field);

    free((char *) dp);

    return XV_OK;
}



/* --------------------  Panel Item Operations  -------------------- */
/* ARGSUSED */
static void
num_txt_begin_preview(item_public, event)
    Panel_item	    item_public;
    Event          *event;
{
    Num_text_info  *dp = NUM_TEXT_PRIVATE(item_public);
    Item_info      *ip = ITEM_PRIVATE(item_public);
    Rect	    rect;


/*
    UP_INACTIVE flag: indicates that max numeric value has
        been reached. Cannot further increment numeric text
        value

    DOWN_INACTIVE flag: ditto except applies to min value.

    UP_SELECTED flag: indicates that select button was pressed
        over scrollbutton.  'begin preview' is called when
        mouse action occurs inside numeric text item.
        cancel_preview and accept preview removes this flag
        from 'btn_state'.

    DOWN_SELECTED flag: ditto; except applies to down button.
*/





    if (dp->btn_state & OLGX_INACTIVE)
	return;


    if (!(dp->btn_state & UP_INACTIVE)) {
        /* Up button active: check if pointer is over up button */
        rect_construct(&rect, dp->btn_rect.r_left, dp->btn_rect.r_top,
                       dp->btn_rect.r_width/2, dp->btn_rect.r_height);

        if (rect_includespoint(&rect, event_x(event), event_y(event)))
        {
            if (dp->btn_state & DOWN_SELECTED)
            {
                panel_autoscroll_stop_itimer( item_public );
                dp->btn_state &= ~DOWN_SELECTED;
            }
 
            if (!(dp->btn_state & UP_SELECTED))
            {
                panel_autoscroll_start_itimer( item_public,
                    num_textitem_scroll_itimer_func );
                dp->btn_state |= UP_SELECTED;
            }
        }
        else
        {
            if (dp->btn_state & UP_SELECTED)
            {
                panel_autoscroll_stop_itimer( item_public );
                dp->btn_state &= ~UP_SELECTED;
            }
        }
    }   
    else
    {  
        /* don't think this is needed, but just in case... */
        if (dp->btn_state & UP_SELECTED)
        {
            panel_autoscroll_stop_itimer( item_public );
            dp->btn_state &= ~UP_SELECTED;
        }
    }



    if (!(dp->btn_state & DOWN_INACTIVE)) {
        /* Down button active: check if pointer is over down button */
        rect_construct(&rect, dp->btn_rect.r_left + dp->btn_rect.r_width/2,
                       dp->btn_rect.r_top, dp->btn_rect.r_width/2,
                       dp->btn_rect.r_height);
        if (rect_includespoint(&rect, event_x(event), event_y(event)))
        {
            if (!(dp->btn_state & DOWN_SELECTED))
            {
                panel_autoscroll_start_itimer( item_public,
                    num_textitem_scroll_itimer_func );
                dp->btn_state |= DOWN_SELECTED;
            }
        }
        else
        {
            if (dp->btn_state & DOWN_SELECTED)
            {
                panel_autoscroll_stop_itimer( item_public );
                dp->btn_state &= ~DOWN_SELECTED;
            }
        }
    }
    else
    {
        if (dp->btn_state & DOWN_SELECTED)
        {
            panel_autoscroll_stop_itimer( item_public );
            dp->btn_state &= ~DOWN_SELECTED;
        }
    }

    if (dp->btn_state & (UP_SELECTED | DOWN_SELECTED) &&
	ip->panel->kbd_focus_item != ITEM_PRIVATE(dp->text_field)) {
	/* Move Location Cursor to dp->text_field */
	if (ip->panel->status.has_input_focus)
	    panel_set_kbd_focus(ip->panel, ITEM_PRIVATE(dp->text_field));
	else {
	    ip->panel->kbd_focus_item = ITEM_PRIVATE(dp->text_field);
	    ip->panel->status.focus_item_set = TRUE;
	}
    }
    num_txt_paint_btn(ip, dp);
    
}


/* ARGSUSED */
static void
num_txt_cancel_preview(item_public, event)
    Panel_item	    item_public;
    Event          *event;
{
    Num_text_info  *dp = NUM_TEXT_PRIVATE(item_public);
    Item_info      *ip = ITEM_PRIVATE(item_public);

    panel_autoscroll_stop_itimer( item_public );
    dp->btn_state &= ~(UP_SELECTED | DOWN_SELECTED);
    num_txt_paint_btn(ip, dp);
}


/* ARGSUSED */
static void
num_txt_accept_preview(item_public, event)
    Panel_item	    item_public;
    Event          *event;
{
    Num_text_info  *dp = NUM_TEXT_PRIVATE(item_public);
    Item_info      *ip = ITEM_PRIVATE(item_public);
    Rect	    rect;

    panel_autoscroll_stop_itimer( item_public );
    dp->btn_state &= ~(UP_SELECTED | DOWN_SELECTED);
    if (dp->btn_state & OLGX_INACTIVE)
	return;
    if ((dp->btn_state & UP_INACTIVE) == 0) {
	/* Up button active: increment value if pointer is over up button */
	rect_construct(&rect, dp->btn_rect.r_left, dp->btn_rect.r_top,
		       dp->btn_rect.r_width/2, dp->btn_rect.r_height);
	if (rect_includespoint(&rect, event_x(event), event_y(event))) {
	    (void) set_value(dp, get_value(dp)+1);
	    if (ip->notify)
		(*ip->notify) (ITEM_PUBLIC(ip), event);
	}
    }
    if ((dp->btn_state & DOWN_INACTIVE) == 0) {
	/* Down button active: decrement value if pointer is over down button */
	rect_construct(&rect, dp->btn_rect.r_left + dp->btn_rect.r_width/2,
		       dp->btn_rect.r_top, dp->btn_rect.r_width/2,
		       dp->btn_rect.r_height);
	if (rect_includespoint(&rect, event_x(event), event_y(event))) {
	    (void) set_value(dp, get_value(dp)-1);
	    if (ip->notify)
		(*ip->notify) (ITEM_PUBLIC(ip), event);
	}
    } 
    num_txt_paint_value(ip);
}


static void
num_txt_paint(item_public)
    Panel_item	    item_public;
{
    Item_info      *ip = ITEM_PRIVATE(item_public);

    panel_text_paint_label(ip);
    num_txt_paint_value(ip);
}


static void
num_txt_layout(item_public, deltas)
    Panel_item	    item_public;
    Rect           *deltas;
{
    Num_text_info  *dp = NUM_TEXT_PRIVATE(item_public);

    dp->btn_rect.r_left += deltas->r_left;
    dp->btn_rect.r_top += deltas->r_top;
    if (dp->text_field)
	xv_set(dp->text_field,
	       XV_X, xv_get(dp->text_field, XV_X) + deltas->r_left,
	       XV_Y, xv_get(dp->text_field, XV_Y) + deltas->r_top,
	       NULL);
}



/* --------------------  Local Routines  -------------------- */
static          Notify_value
num_textitem_scroll_itimer_func( item, which )
    Panel_item          item;
    int                 which;
{
    int                 val;
    Num_text_info  	*dp = NUM_TEXT_PRIVATE(item);
    Item_info      	*ip = ITEM_PRIVATE(item);
    Event		event;
 
    if (dp->btn_state & OLGX_INACTIVE)
        return 0;
 
    if (dp->btn_state & UP_SELECTED)
    {
        if (!(dp->btn_state & UP_INACTIVE))
            (void) set_value(dp, get_value(dp)+1);

	event_init( &event ); /* send empty, bogus event */
        if (ip->notify)
#if 1
            /* martin-2.buck@student.uni-ulm.de */
            (*ip->notify) (ITEM_PUBLIC(ip), &event);
#else
            (*ip->notify) (ITEM_PUBLIC(ip), event);
#endif

        /*
           UP_INACTIVE flag is set in set_value(); if
           we can't increment the value any more, then
           disable the timer
        */
        if (dp->btn_state & UP_INACTIVE)
            panel_autoscroll_stop_itimer( item );
    }
    if (dp->btn_state & DOWN_SELECTED)
    {
        if (!(dp->btn_state & DOWN_INACTIVE))
            (void) set_value(dp, get_value(dp)-1);

	event_init( &event ); /* send empty, bogus event */
        if (ip->notify)
#if 1
            /* martin-2.buck@student.uni-ulm.de */
            (*ip->notify) (ITEM_PUBLIC(ip), &event);
#else
            (*ip->notify) (ITEM_PUBLIC(ip), event);
#endif

        if (dp->btn_state & DOWN_INACTIVE)
            panel_autoscroll_stop_itimer( item );
    }

    return 0;
} /* num_textitem_scroll_itimer_func */




static int
check_dimming(dp)	/* returns TRUE: button state changed;
				   FALSE: button state not changed */
    register Num_text_info *dp;
{
    int    state_changed = FALSE;
    int    value;

    if (inactive_btn(dp))
	return FALSE;

    sscanf((char *) xv_get(dp->text_field, PANEL_VALUE), "%d", &value);

    if (value == dp->min_value) {
	if ((dp->btn_state & DOWN_INACTIVE) == 0)
	    state_changed = TRUE;
	dp->btn_state |= DOWN_INACTIVE;
    } else {
	if (dp->btn_state & DOWN_INACTIVE)
	    state_changed = TRUE;
	dp->btn_state &= ~DOWN_INACTIVE;
    }

    if (value == dp->max_value) {
	if ((dp->btn_state & UP_INACTIVE) == 0)
	    state_changed = TRUE;
	dp->btn_state |= UP_INACTIVE;
    } else {
	if (dp->btn_state & UP_INACTIVE)
	    state_changed = TRUE;
	dp->btn_state &= ~UP_INACTIVE;
    }

    return state_changed;
}


static int
get_value(dp)
    register Num_text_info *dp;
{
#ifdef OW_I18N
    wchar_t *ptr;
#else
    char *ptr;
#endif /* OW_I18N */
    int value;


#ifdef OW_I18N
    /*
     * if no value in numeric text field, then return 0; this
     * prevents return of bogus value for NULL string.
     */  
    ptr = (wchar_t *)xv_get(dp->text_field, PANEL_VALUE_WCS);
    if (*ptr == '\0')
    {
        if (0 > dp->max_value)
            value = dp->max_value;
        else if (0 < dp->min_value)
            value = dp->min_value;
        else
            value = 0;
    }
    else
        wsscanf(ptr, "%d", &value);
#else
    /*
     * if no value in numeric text field, then return 0; this
     * prevents return of bogus value for NULL string.
     */  
    ptr = (char *) xv_get(dp->text_field, PANEL_VALUE);
    if (*ptr == '\0')
    {
        if (0 > dp->max_value)
            value = dp->max_value;
        else if (0 < dp->min_value)
            value = dp->min_value;
        else
            value = 0;
    }
    else
        sscanf(ptr, "%d", &value);
#endif /* OW_I18N */
    return value;
}


static Bool
notify_user(dp, event)
    Num_text_info *dp;
    Event *event;
{
    switch (dp->notify_level) {
      case PANEL_ALL:
      default:
	return TRUE;

      case PANEL_NONE:
	return FALSE;

#ifdef OW_I18N
      case PANEL_NON_PRINTABLE:
        return (event_is_string(event) || !panel_printable_char(event_action(event)));
 
      case PANEL_SPECIFIED:
        return event_is_down(event) &&
            wschr(dp->terminators_wc, (int) event_action(event)) != 0;
#else
      case PANEL_NON_PRINTABLE:
	return !panel_printable_char(event_action(event));

      case PANEL_SPECIFIED:
	return event_is_down(event) &&
	    XV_INDEX(dp->terminators, event_action(event)) != 0;
#endif /* OW_I18N */
    }
}


static void
num_txt_paint_btn(ip, dp)
    register Item_info *ip;
    register Num_text_info *dp;
{
    Graphics_info  *ginfo = ip->panel->ginfo;
    Xv_Drawable_info *info;
    Xv_Window	    pw;
    int		    save_black;

    if (ip->color_index >= 0)
	save_black = olgx_get_single_color(ginfo, OLGX_BLACK);

    PANEL_EACH_PAINT_WINDOW(ip->panel, pw)
	DRAWABLE_INFO_MACRO(pw, info);
	if (ip->color_index >= 0) {
	    olgx_set_single_color(ginfo, OLGX_BLACK,
				  xv_get(xv_cms(info), CMS_PIXEL,
				  ip->color_index), OLGX_SPECIAL);
	}
	olgx_draw_numscroll_button(ginfo, xv_xid(info),
				   dp->btn_rect.r_left, dp->btn_rect.r_top,
				   dp->btn_state);
    PANEL_END_EACH_PAINT_WINDOW

    if (ip->color_index >= 0)
	olgx_set_single_color(ginfo, OLGX_BLACK, save_black, OLGX_SPECIAL);
}


static void
num_txt_paint_value(ip)
    register Item_info *ip;
{
    register Num_text_info  *dp = NUM_TEXT_FROM_ITEM(ip);

    /* Paint text item */
    panel_redisplay_item(ITEM_PRIVATE(dp->text_field), PANEL_CLEAR);

    /* Paint Numeric Scrolling Button */
    num_txt_paint_btn(ip, dp);
}


static int
set_value(dp, new_value)  /* returns TRUE: button state changed;
				     FALSE: button state not changed */
    register Num_text_info *dp;
    int   new_value;
{
    CHAR  buffer[20];

    if (new_value > dp->max_value)
	new_value = dp->max_value;

    if (new_value < dp->min_value)
	new_value = dp->min_value;

#ifdef OW_I18N
    wsprintf(buffer, "%d", new_value);
    xv_set(dp->text_field, PANEL_VALUE_WCS, buffer, NULL);
#else
    sprintf(buffer, "%d", new_value);
    xv_set(dp->text_field, PANEL_VALUE, buffer, NULL);
#endif /* OW_I18N */

    return check_dimming(dp);
}


static Panel_setting
text_notify_proc(text_field, event)
    Panel_item text_field;
    Event *event;
{
    Panel_item num_text = xv_get(text_field, XV_KEY_DATA, PANEL_NUM_TXT);
    register Item_info *ip = ITEM_PRIVATE(num_text);
    register Num_text_info *dp = NUM_TEXT_PRIVATE(num_text);
    int		    changed;

    if (event_action(event) == ACTION_CUT && dp->notify_level != PANEL_NONE)
	return (Panel_setting) (*ip->notify) (ITEM_PUBLIC(ip), event);

    if (event_is_down(event)) {
	changed = FALSE;
	if ((dp->btn_state & UP_INACTIVE) == 0) {
	    switch (event_action(event)) {
	      case ACTION_UP:
		(void) set_value(dp, get_value(dp) + 1);
		changed = TRUE;
		break;
	      case ACTION_JUMP_UP:
		(void) set_value(dp, get_value(dp) + dp->jump_delta);
		changed = TRUE;
		break;
	      case ACTION_DATA_START:
		(void) set_value(dp, dp->max_value);
		changed = TRUE;
		break;
	    }
	}
	if ((dp->btn_state & DOWN_INACTIVE) == 0) {
	    switch (event_action(event)) {
	      case ACTION_DOWN:
		(void) set_value(dp, get_value(dp) - 1);
		changed = TRUE;
		break;
	      case ACTION_JUMP_DOWN:
		(void) set_value(dp, get_value(dp) - dp->jump_delta);
		changed = TRUE;
		break;
	      case ACTION_DATA_END:
		(void) set_value(dp, dp->min_value);
		changed = TRUE;
		break;
	    }
	} 
	if (changed) {
	    if (ip->notify)
		(*ip->notify) (ITEM_PUBLIC(ip), event);
	    num_txt_paint_value(ip);
	    return PANEL_NONE;
	}
    }

#ifdef OW_I18N
        /*
         *  Disable all input from ie_string
         */
    if (!event_is_string(event) && event_action(event) > ASCII_LAST)
        return panel_text_notify(text_field, event);

    if (event_is_string(event) || ( isprint(event_action(event))
        && !isdigit(event_action(event)) &&
        event_action(event) != '-' && event_action(event) != '+')) {
        window_bell(event_window(event));
        return PANEL_NONE;
#else
    if (event_action(event) > ASCII_LAST)
	return panel_text_notify(text_field, event);

    if (isprint(event_action(event)) && !isdigit(event_action(event)) &&
	event_action(event) != '-' && event_action(event) != '+') {
	window_bell(event_window(event));
	return PANEL_NONE;
#endif /* OW_I18N */
    }

    /*
     * validate text field value before calling user
     */
    switch (event_action(event)) {
      case '\n':
      case '\t':
      case '\r':
	if (set_value(dp, get_value(dp)))
	    num_txt_paint_btn(ip, dp);
	break;
    }

    if (notify_user(dp, event))
	return (Panel_setting) (*ip->notify) (ITEM_PUBLIC(ip), event);

    else
	return panel_text_notify(text_field, event);
}

