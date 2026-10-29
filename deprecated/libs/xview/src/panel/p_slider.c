#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)p_slider.c 20.84 93/06/28 Copyr 1984 Sun Micro";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#include <X11/Xlib.h>
#include <xview/sun.h>
#include <xview_private/panel_impl.h>
#include <xview/pixwin.h>
#include <xview/font.h>
#include <xview_private/draw_impl.h>

#define SLIDER_PRIVATE(item)	\
	XV_PRIVATE(Slider_info, Xv_panel_slider, item)
#define SLIDER_PUBLIC(item)	XV_PUBLIC(item)

#define	SLIDER_FROM_ITEM(ip)	SLIDER_PRIVATE(ITEM_PUBLIC(ip))

#define	TEXT_VALUE_GAP	6	/* padding on either side of text value */
#define TICK_THICKNESS	2
#define MIN_TICK_GAP	4	/* minimum gap between ticks */
#define POINTER_GAP	3	/* gap between pointer and drag box after
				 * warping pointer after incrementing or
				 * decrementing slider */

Xv_public struct pr_size xv_pf_textwidth();
#ifdef OW_I18N
Xv_public struct pr_size xv_pf_textwidth_wc();
#endif /* OW_I18N */

/* XView functions */
Pkg_private int slider_init();
Pkg_private Xv_opaque slider_set_avlist();
Pkg_private Xv_opaque slider_get_attr();
Pkg_private int slider_destroy();

/* Panel Item Operations */
static void     slider_begin_preview(), slider_update_preview(),
		slider_cancel_preview(), slider_accept_preview(),
		slider_accept_key(), slider_paint(), slider_remove(),
		slider_layout(), slider_accept_kbd_focus(),
		slider_yield_kbd_focus();

/* Local functions */
static void	adjust_slider();
static void	check_endbox_entered();
static Panel_setting get_value();
static void     paint_slider();
static void     update_rects();

static Panel_ops ops = {
    panel_default_handle_event,		/* handle_event() */
    slider_begin_preview,		/* begin_preview() */
    slider_update_preview,		/* update_preview() */
    slider_cancel_preview,		/* cancel_preview() */
    slider_accept_preview,		/* accept_preview() */
    NULL,				/* accept_menu() */
    slider_accept_key,			/* accept_key() */
    panel_default_clear_item,		/* clear() */
    slider_paint,			/* paint() */
    NULL,				/* resize() */
    slider_remove,			/* remove() */
    NULL,				/* restore() */
    slider_layout,			/* layout() */
    slider_accept_kbd_focus,		/* accept_kbd_focus() */
    slider_yield_kbd_focus,		/* yield_kbd_focus() */
    NULL				/* extension: reserved for future use */
};

typedef enum {
    SLIDER_MIN,
    SLIDER_MAX,
    SLIDER_INCREMENT,
    SLIDER_DECREMENT,
    SLIDER_JUMP_INCREMENT,
    SLIDER_JUMP_DECREMENT
} Slider_adjust;

typedef struct {	/* data for a slider */
    Panel_item      public_self;/* back pointer to object */
    int             actual;	/* # of pixels the slider box is to the right of
				 * or below the start of the slider bar before
				 * or after a drag-slider-box operation. */
    int             apparent;	/* # of pixels the slider box is to the right of
				 * or below the start of the slider bar during
				 * a drag-slider-box operation. */
    u_int           flags;
    int		    jump_delta;	/* # of client units to move on JUMP keyboard
				 * commands */
    struct timeval  last_click_time;	/* time of last SELECT-up event */
    int		    last_delta;		/* last increment/decrement delta */
    Rect	    max_endbox_rect;	/* maximum end box rect */
    Rect            max_range_rect;	/* maximum range rect */
    int		    max_range_size;	/* # of characters in max_value */
    CHAR	   *max_tick_string;
    int		    max_tick_string_width;  /* in pixels */
    int             max_value;	/* in client units */
    CHAR	   *max_value_string;
    Rect	    min_endbox_rect;	/* left or top end box rect */
    Rect            min_range_rect;	/* minimum range rect */
    int		    min_range_size;	/* # of characters in min_value */
    CHAR	   *min_tick_string;
    int		    min_tick_string_width;  /* in pixels */
    int             min_value;	/* in client units */
    CHAR	   *min_value_string;
    int		    nticks;	/* nbr of tick marks on slider */
    int             print_value;/* value from PANEL_VALUE (in client units) */
    int             restore_print_value:1;
    Rect            sliderbox;  /* rect containing the position of last
				 * slider box */
    Rect            sliderrect;	/* rect containing slider */
    Rect	    tickrect;	/* rect containing tick marks */
    Rect	    tickstringrect; /* rect containg tick strings */
    int             use_print_value:1;
    Rect            valuerect;	/* rect containing (text) value of slider */
    int             valuesize;	/* max # of characters in current value */
    int             value_offset;
    /*
     * When a user drags the slider box, the value should actually be just to
     * the left of or below the slider box. Value_offset is the distance between
     * event_{x,y}(event) and where the real value should be
     */
    Panel_item      value_textitem;	/* for displaying value as a text
					 * item */
    int             width;
} Slider_info;

static int etoi(Slider_info *dp, int value);
static int itoe(Slider_info *dp, int value);

/* flags */
#define	SHOWRANGE	0x01	/* show high and low scale numbers */
#define	SHOWVALUE	0x02	/* show current value */
#define SHOWENDBOXES	0x04	/* show end boxes */
#define	CONTINUOUS	0x08	/* notify on any change */
#define READONLY	0x10	/* text value is read-only */
#define	VERTICAL	0x20	/* slider orientation (0=>horizontal) */
#define MIN_ENDBOX_SELECTED	0x40	/* min end box is selected */
#define MAX_ENDBOX_SELECTED	0x80	/* max end box is selected */

#define vertical(dp) ((dp)->flags & VERTICAL)


/* ========================================================================= */

/* -------------------- XView Functions  -------------------- */
/*ARGSUSED*/
Pkg_private int
slider_init(panel_public, item_public, avlist)
    Panel           panel_public;
    Panel_item      item_public;
    Attr_avlist     avlist;
{
    Panel_info     *panel = PANEL_PRIVATE(panel_public);
    register Item_info *ip = ITEM_PRIVATE(item_public);
    Xv_panel_slider *item_object = (Xv_panel_slider *) item_public;
    register Slider_info *dp;

    dp = xv_alloc(Slider_info);

    /* link to object */
    item_object->private_data = (Xv_opaque) dp;
    dp->public_self = item_public;

    ip->ops = ops;
    if (panel->event_proc)
	ip->ops.panel_op_handle_event = (void (*) ()) panel->event_proc;
    ip->item_type = PANEL_SLIDER_ITEM;
    panel_set_bold_label_font(ip);

    /* Initialize non-zero dp fields */
    dp->flags = SHOWRANGE | SHOWVALUE;
    dp->jump_delta = 10;	/* # of client units to jump */
    dp->max_range_size = 3;	/* # of characters in max_value */
    dp->max_value = 100;
    dp->min_range_size = 1;	/* # of characters in min_value */
    dp->valuesize = 3;		/* max # of characters in current value */
    dp->width = 100;

    if (panel->status.mouseless)
	ip->flags |= WANTS_KEY;

    return XV_OK;
}


Pkg_private     Xv_opaque
slider_set_avlist(item_public, avlist)
    Panel_item      item_public;
    register Attr_avlist avlist;
{
    Attr_avlist     attrs;
    register Item_info *ip = ITEM_PRIVATE(item_public);
    register Slider_info *dp = SLIDER_PRIVATE(item_public);
    register Attr_attribute attr;
    CHAR            buf[16];
    CHAR            buf2[16];
    int             char_width;
    int		    end_create = FALSE;
    int		    jump_delta_set = FALSE;
    int             max_value = dp->max_value;
    int             min_value = dp->min_value;
    int		    range_set = FALSE;
    Xv_opaque       result;
    int		    show_item = -1;
    struct pr_size  size;
    int		    size_changed = FALSE;
    int		    ticks_set = FALSE;
    int             width = dp->width;
    int             value;
    int		    value_set = FALSE;

    /* If a client has called panel_item_parent this item may not 
     * have a parent -- do nothing in this case.
     */
    if (ip->panel == NULL) {
	return ((Xv_opaque) XV_ERROR);
    }

    /* PANEL_SLIDER does not accept PANEL_LAYOUT.  Consume PANEL_LAYOUT. */
    for (attrs = avlist; *attrs; attrs = attr_next(attrs)) {
	if (attrs[0] ==  PANEL_LAYOUT)
	    ATTR_CONSUME(attrs[0]);
    }

    if (*avlist != XV_END_CREATE) {
	/* Parse Panel Item Generic attributes before Slider attributes.
	 * Prevent panel_redisplay_item from being called in item_set_avlist.
	 */
	ip->panel->no_redisplay_item = TRUE;
	result = xv_super_set_avlist(item_public, &xv_panel_slider_pkg, avlist);
	ip->panel->no_redisplay_item = FALSE;
	if (result != XV_OK)
	    return result;
    }

    while (attr = *avlist++) {
	switch (attr) {
	  case PANEL_ITEM_COLOR:
	    if (dp->value_textitem)
		xv_set(dp->value_textitem,
		       PANEL_ITEM_COLOR, (char *) *avlist++,
		       NULL);
	    else
		avlist++;
	    break;

	  case PANEL_READ_ONLY:
	    if (*avlist++)
		dp->flags |= READONLY;
	    else
		dp->flags &= ~READONLY;
	    break;

	  case PANEL_VALUE_FONT:
	    /* Sunview1 compatibility attribute: not used */
	    avlist++;
	    break;

	  case PANEL_NOTIFY_LEVEL:
	    if ((Panel_setting) * avlist++ == PANEL_ALL)
		dp->flags |= CONTINUOUS;
	    else
		dp->flags &= ~CONTINUOUS;
	    break;

	  case PANEL_VALUE:
	    value = (int) *avlist++;
	    value_set = TRUE;
	    dp->use_print_value = FALSE;
	    break;

	  case PANEL_JUMP_DELTA:
	    dp->jump_delta = (int) *avlist++;
	    jump_delta_set = TRUE;
	    break;

#ifdef OW_I18N
	  case PANEL_MIN_TICK_STRING:
	    if (dp->min_tick_string)
		xv_free(dp->min_tick_string);
	    if (*avlist) {
		dp->min_tick_string = 
		    (wchar_t *) _xv_mbstowcsdup((char *) *avlist);
	    } else
		dp->min_tick_string = NULL;
	    size_changed = TRUE;
	    avlist++;
	    break;

	  case PANEL_MIN_TICK_STRING_WCS:
	    if (dp->min_tick_string)
		xv_free(dp->min_tick_string);
	    if (*avlist) {
		dp->min_tick_string =
		    (wchar_t *) panel_strsave_wc((wchar_t *) *avlist);
	    } else
		dp->min_tick_string = NULL;
	    size_changed = TRUE;
	    avlist++;
	    break;
#else
	  case PANEL_MIN_TICK_STRING:
	    if (dp->min_tick_string)
		xv_free(dp->min_tick_string);
	    if (*avlist)
		dp->min_tick_string = panel_strsave((char *) *avlist);
	    else
		dp->min_tick_string = NULL;
	    size_changed = TRUE;
	    avlist++;
	    break;
#endif /* OW_I18N */

	  case PANEL_MIN_VALUE:
	    min_value = (int) *avlist++;
	    range_set = TRUE;
	    size_changed = TRUE;
	    break;

#ifdef OW_I18N
	  case PANEL_MIN_VALUE_STRING:
	    if (dp->min_value_string)
		xv_free(dp->min_value_string);
	    if (*avlist) {
		dp->min_value_string = 
		    (wchar_t *) _xv_mbstowcsdup((char *) *avlist);
	    } else
		dp->min_value_string = NULL;
	    size_changed = TRUE;
	    avlist++;
	    break;

	  case PANEL_MIN_VALUE_STRING_WCS:
	    if (dp->min_value_string)
		xv_free(dp->min_value_string);
	    if (*avlist) {
		dp->min_value_string =
		    (wchar_t *) panel_strsave_wc((wchar_t *) *avlist);
	    } else
		dp->min_value_string = NULL;
	    size_changed = TRUE;
	    avlist++;
	    break;
#else
	  case PANEL_MIN_VALUE_STRING:
	    if (dp->min_value_string)
		xv_free(dp->min_value_string);
	    if (*avlist)
		dp->min_value_string = panel_strsave((char *) *avlist);
	    else
		dp->min_value_string = NULL;
	    size_changed = TRUE;
	    avlist++;
	    break;
#endif /* OW_I18N */

#ifdef OW_I18N
	  case PANEL_MAX_TICK_STRING:
	    if (dp->max_tick_string)
		xv_free(dp->max_tick_string);
	    if (*avlist) {
		dp->max_tick_string = 
		    (wchar_t *) _xv_mbstowcsdup((char *) *avlist);
	    } else
		dp->max_tick_string = NULL;
	    size_changed = TRUE;
	    avlist++;
	    break;

	  case PANEL_MAX_TICK_STRING_WCS:
	    if (dp->max_tick_string)
		xv_free(dp->max_tick_string);
	    if (*avlist) {
		dp->max_tick_string =
		    (wchar_t *) panel_strsave_wc((wchar_t *) *avlist);
	    } else
		dp->max_tick_string = NULL;
	    size_changed = TRUE;
	    avlist++;
	    break;
#else
	  case PANEL_MAX_TICK_STRING:
	    if (dp->max_tick_string)
		xv_free(dp->max_tick_string);
	    if (*avlist)
		dp->max_tick_string = panel_strsave((char *) *avlist);
	    else
		dp->max_tick_string = NULL;
	    size_changed = TRUE;
	    avlist++;
	    break;
#endif /* OW_I18N */

	  case PANEL_MAX_VALUE:
	    max_value = (int) *avlist++;
	    range_set = TRUE;
	    size_changed = TRUE;
	    break;

#ifdef OW_I18N
	  case PANEL_MAX_VALUE_STRING:
	    if (dp->max_value_string)
		xv_free(dp->max_value_string);
	    if (*avlist) {
		dp->max_value_string = 
		    (wchar_t *) _xv_mbstowcsdup ((char *) *avlist);
	    } else
		dp->max_value_string = NULL;
	    size_changed = TRUE;
	    avlist++;
	    break;

	  case PANEL_MAX_VALUE_STRING_WCS:
	    if (dp->max_value_string)
	    	xv_free(dp->max_value_string);
	    if (*avlist) {
		dp->max_value_string =
		    (wchar_t *) panel_strsave_wc((wchar_t *) *avlist);
	    } else
		dp->max_value_string = NULL;
	    size_changed = TRUE;
	    avlist++;
	    break;
#else
	  case PANEL_MAX_VALUE_STRING:
	    if (dp->max_value_string)
		xv_free(dp->max_value_string);
	    if (*avlist)
		dp->max_value_string = panel_strsave((char *) *avlist);
	    else
		dp->max_value_string = NULL;
	    size_changed = TRUE;
	    avlist++;
	    break;
#endif /* OW_I18N */

	  case PANEL_DIRECTION:
	    if (*avlist++ == PANEL_VERTICAL)
		dp->flags |= VERTICAL;
	    else
		dp->flags &= ~VERTICAL;
	    size_changed = TRUE;
	    break;
		
	  case PANEL_SLIDER_END_BOXES:
	    if (*avlist++) {
		dp->flags |= SHOWENDBOXES;
	    } else
		dp->flags &= ~SHOWENDBOXES;
	    size_changed = TRUE;
	    break;

	  case PANEL_TICKS:
	    dp->nticks = (int) *avlist++;
	    if (dp->nticks == 1)
		dp->nticks = 2;
	    size_changed = TRUE;
	    ticks_set = TRUE;
	    break;

	  case PANEL_SLIDER_WIDTH:
	    width = (int) *avlist++;
	    range_set = TRUE;
	    break;

	  case PANEL_VALUE_DISPLAY_LENGTH:
	    char_width = (int) *avlist++;
	    width = panel_col_to_x(ip->panel->std_font, char_width);
	    range_set = TRUE;
	    break;

	  case XV_SHOW:
	    show_item = (short) *avlist++;
	    break;

	  case PANEL_SHOW_VALUE:
	    if (*avlist++) {
		dp->flags |= SHOWVALUE;
	    } else
		dp->flags &= ~SHOWVALUE;
	    size_changed = TRUE;
	    break;

	  case PANEL_SHOW_RANGE:
	    if ((int) *avlist++)
		dp->flags |= SHOWRANGE;
	    else
		dp->flags &= ~SHOWRANGE;
	    size_changed = TRUE;
	    break;

	  case XV_END_CREATE:
	    end_create = TRUE;
	    break;

	  default:
	    /* skip past what we don't care about */
	    avlist = attr_skip(attr, avlist);
	    break;
	}
    }

    if (ticks_set) {
	if (dp->nticks > width / (TICK_THICKNESS+MIN_TICK_GAP))
	    dp->nticks = width / (TICK_THICKNESS+MIN_TICK_GAP);
    }
    if (dp->min_tick_string) {
#ifdef OW_I18N
	size = xv_pf_textwidth_wc(wslen(dp->min_tick_string),
	    ip->panel->std_font, dp->min_tick_string);
#else
	size = xv_pf_textwidth(strlen(dp->min_tick_string),
	    ip->panel->std_font, dp->min_tick_string);
#endif /* OW_I18N */
	dp->min_tick_string_width = size.x;
    } else
	dp->min_tick_string_width = 0;
    if (dp->max_tick_string) {
#ifdef OW_I18N
	size = xv_pf_textwidth_wc(wslen(dp->max_tick_string),
	    ip->panel->std_font, dp->max_tick_string);
#else
	size = xv_pf_textwidth(strlen(dp->max_tick_string),
	    ip->panel->std_font, dp->max_tick_string);
#endif /* OW_I18N */
	dp->max_tick_string_width = size.x;
    } else
	dp->max_tick_string_width = 0;
    if (range_set) {
	/* get the current value */
	if (!value_set) {
	    value = itoe(dp, dp->actual);
	    value_set = TRUE;
	}
	dp->min_value = min_value;
	/* don't let the max value be <= the min value */
	dp->max_value =
	    (max_value <= dp->min_value) ? dp->min_value + 1 : max_value;
	SPRINTF(buf, "%d", dp->min_value);
	SPRINTF(buf2, "%d", dp->max_value);
	dp->min_range_size = STRLEN(buf);
	dp->max_range_size = STRLEN(buf2);
	dp->valuesize = MAX(dp->min_range_size, dp->max_range_size);
	if (vertical(dp))
	    dp->width = width;
	else
	    dp->width = MAX(width,
			    dp->min_tick_string_width + TEXT_VALUE_GAP +
			    dp->max_tick_string_width);
	size_changed = TRUE;
	dp->use_print_value = FALSE;   /* print_value is no longer valid */
	if (!jump_delta_set)
	    dp->jump_delta = (dp->max_value - dp->min_value + 1) / 10;
    }
    /* set apparent & actual value */
    if (value_set) {
	if (value < dp->min_value)
	    value = dp->min_value;
	else if (value > dp->max_value)
	    value = dp->max_value;
	dp->apparent = dp->actual = etoi(dp, value);
	dp->print_value = value;
	dp->use_print_value = TRUE;
    }
    if (!end_create) {
	/* Note: There's no need to go through this code twice when the user
	 * creates the slider.  So, we only do this on the SET pass of the
	 * xv_create, and not the END_CREATE pass.  See the note below about
	 * the imbedded PANEL_TEXT item to understand why we want to execute
	 * this code in the SET pass.
	 */
	if (dp->flags & SHOWVALUE) {
	    if (show_item == -1)
		show_item = hidden(ip) ? FALSE : TRUE;
	    if (!dp->use_print_value) {
		dp->print_value = itoe(dp, dp->actual);
		dp->use_print_value = TRUE;
	    }
	    (void) SPRINTF(buf, "%d", dp->print_value);
	    if (dp->value_textitem) {
		xv_set(dp->value_textitem,
		       PANEL_INACTIVE, inactive(ip),
		       PANEL_ITEM_COLOR, ip->color_index,
		       PANEL_READ_ONLY, dp->flags & READONLY ?
			   TRUE : FALSE,
		       XV_SHOW, show_item,
#ifdef OW_I18N
		       PANEL_VALUE_WCS, buf,
#else
		       PANEL_VALUE, buf,
#endif /* OW_I18N */
		       PANEL_VALUE_DISPLAY_LENGTH, dp->valuesize,
		       NULL);
	    } else {
		/* Note that we are creating the imbedded PANEL_TEXT item
		 * before our (PANEL_SLIDER) item's XV_END_CREATE is processed.
		 * This insures that the PANEL_TEXT item is listed first in
		 * the panel's item list.  (The XV_END_CREATE arm in
		 * item_set_avlist does the panel_append call.)
		 * Otherwise, clicking SELECT over the slider's value text item
		 * would not set the input focus there, since panel_find_item
		 * would think the SELECT is over the slider rect.
		 */
		dp->value_textitem =
		    xv_create(PANEL_PUBLIC(ip->panel), PANEL_TEXT,
			      PANEL_ITEM_OWNER, item_public,
			      PANEL_CLIENT_DATA, ip,
			      PANEL_INACTIVE, inactive(ip),
			      PANEL_ITEM_COLOR, ip->color_index,
			      PANEL_ITEM_X, rect_right(&ip->label_rect) +
				  TEXT_VALUE_GAP,
			      PANEL_ITEM_Y, ip->label_rect.r_top,
			      PANEL_NOTIFY_PROC, get_value,
			      PANEL_READ_ONLY, dp->flags & READONLY ?
				  TRUE : FALSE,
			      XV_SHOW, show_item,
#ifdef OW_I18N
			      PANEL_VALUE_WCS, buf,
#else
			      PANEL_VALUE, buf,
#endif /* OW_I18N */
			      PANEL_VALUE_DISPLAY_LENGTH, dp->valuesize,
			      NULL);
		size_changed = TRUE;
	    }
	} else if (dp->value_textitem) {
	    /* Hide value */
	    xv_set(dp->value_textitem,
		   XV_SHOW, FALSE,
		   NULL);
	}
    }
    if (size_changed || end_create)
	update_rects(ip);
    if (end_create)
	panel_check_item_layout(ip);

    return XV_OK;
}


Pkg_private     Xv_opaque
slider_get_attr(item_public, status, which_attr, valist)	/*ARGSUSED*/
    Panel_item      item_public;
    int            *status;
    Attr_attribute  which_attr;
    va_list         valist;
{
    register Slider_info *dp = SLIDER_PRIVATE(item_public);
#ifdef OW_I18N
    char		 *string_mbs = NULL;
#endif /* OW_I18N */

    switch (which_attr) {
      case PANEL_READ_ONLY:
	return (Xv_opaque)
	    (dp->flags & READONLY ? TRUE : FALSE);

      case PANEL_NOTIFY_LEVEL:
	return (Xv_opaque)
	    (dp->flags & CONTINUOUS ? PANEL_ALL : PANEL_DONE);

      case PANEL_VALUE:
	return (Xv_opaque) itoe(dp, dp->actual);

      case PANEL_JUMP_DELTA:
	return (Xv_opaque) dp->jump_delta;

#ifdef OW_I18N
      case PANEL_MIN_TICK_STRING:
	if (string_mbs) xv_free(string_mbs);
	string_mbs = (char *)_xv_wcstombsdup((wchar_t *)dp->min_tick_string);
	return (Xv_opaque) string_mbs;

      case PANEL_MIN_TICK_STRING_WCS:
	return (Xv_opaque) dp->min_tick_string;
#else
      case PANEL_MIN_TICK_STRING:
	return (Xv_opaque) dp->min_tick_string;
#endif /* OW_I18N */

      case PANEL_MIN_VALUE:
	return (Xv_opaque) dp->min_value;

#ifdef OW_I18N
      case PANEL_MIN_VALUE_STRING:
	if (string_mbs) xv_free(string_mbs);
	string_mbs = (char *)_xv_wcstombsdup((wchar_t *)dp->min_value_string);
	return (Xv_opaque) string_mbs;

      case PANEL_MIN_VALUE_STRING_WCS:
	return (Xv_opaque) dp->min_value_string;
#else
      case PANEL_MIN_VALUE_STRING:
	return (Xv_opaque) dp->min_value_string;
#endif /* OW_I18N */

#ifdef OW_I18N
      case PANEL_MAX_TICK_STRING:
	if (string_mbs) xv_free(string_mbs);
	string_mbs = (char *)_xv_wcstombsdup((wchar_t *)dp->max_tick_string);
	return (Xv_opaque) string_mbs;

      case PANEL_MAX_TICK_STRING_WCS:
	return (Xv_opaque) dp->max_tick_string;
#else
      case PANEL_MAX_TICK_STRING:
	return (Xv_opaque) dp->max_tick_string;
#endif /* OW_I18N */

      case PANEL_MAX_VALUE:
	return (Xv_opaque) dp->max_value;

#ifdef OW_I18N
      case PANEL_MAX_VALUE_STRING:
	if (string_mbs) xv_free(string_mbs);
	string_mbs = (char *)_xv_wcstombsdup((wchar_t *)dp->max_value_string);
	return (Xv_opaque) string_mbs;

      case PANEL_MAX_VALUE_STRING_WCS:
	return (Xv_opaque) dp->max_value_string;
#else
      case PANEL_MAX_VALUE_STRING:
	return (Xv_opaque) dp->max_value_string;
#endif /* OW_I18N */

      case PANEL_DIRECTION:
	return (Xv_opaque) (dp->flags & VERTICAL ? PANEL_VERTICAL :
	    PANEL_HORIZONTAL);

      case PANEL_SLIDER_END_BOXES:
	return (Xv_opaque) (dp->flags & SHOWENDBOXES ? TRUE : FALSE);

      case PANEL_TICKS:
	return (Xv_opaque) dp->nticks;

      case PANEL_SLIDER_WIDTH:
	return (Xv_opaque) dp->width;

      case PANEL_SHOW_VALUE:
	return (Xv_opaque) (dp->flags & SHOWVALUE ? TRUE : FALSE);

      case PANEL_SHOW_RANGE:
	return (Xv_opaque) (dp->flags & SHOWRANGE ? TRUE : FALSE);

      default:
	*status = XV_ERROR;
	return (Xv_opaque) 0;
    }
}


Pkg_private int
slider_destroy(item_public, status)
    Panel_item      item_public;
    Destroy_status  status;
{
    Slider_info    *dp = SLIDER_PRIVATE(item_public);

    if ((status == DESTROY_CHECKING) || (status == DESTROY_SAVE_YOURSELF))
	return XV_OK;

    slider_remove(item_public);
    if (dp->value_textitem)
	xv_destroy(dp->value_textitem);
    if (dp->min_tick_string)
	xv_free(dp->min_tick_string);
    if (dp->max_tick_string)
	xv_free(dp->max_tick_string);
    if (dp->min_value_string)
	xv_free(dp->min_value_string);
    if (dp->max_value_string)
	xv_free(dp->max_value_string);

    free((char *) dp);

    return XV_OK;
}



/* --------------------  Panel Item Operations  -------------------- */
static void
slider_begin_preview(item_public, event)
    Panel_item	    item_public;
    Event          *event;
{
    Slider_info	   *dp = SLIDER_PRIVATE(item_public);
    Item_info      *ip = ITEM_PRIVATE(item_public);

    /*
     * We only begin preview if the user clicks on the slider, not
     * when he/she drags in the slider w/ a button down.
     */
    if (dp->last_delta &&
        panel_is_multiclick(ip->panel, &dp->last_click_time,
			    &event_time(event)))
	return;	/* Multiclick after increment or decrement */
    if (rect_includespoint(&dp->sliderbox, event_x(event), event_y(event))) {
	ip->panel->status.current_item_active = TRUE;
	if (vertical(dp))
	    dp->value_offset = rect_bottom(&dp->sliderbox) - event_y(event) + 1;
	else
	    dp->value_offset = event_x(event) - dp->sliderbox.r_left + 1;
	/* save status of print value */
	dp->restore_print_value = dp->use_print_value;

	/* update the preview */
	slider_update_preview(item_public, event);
    } else
	check_endbox_entered(ip, event);
}

static void
slider_update_preview(item_public, event)
    Panel_item	    item_public;
    Event          *event;
{
    CHAR            buf[20];
    Slider_info	   *dp = SLIDER_PRIVATE(item_public);
    Xv_Drawable_info *info;
    Item_info      *ip = ITEM_PRIVATE(item_public);
    register int    new;
    Xv_Window       pw;
    Rect            r;

    if ((dp->flags & MIN_ENDBOX_SELECTED) && 
	!(rect_includespoint(&dp->min_endbox_rect, event_x(event),
	event_y(event)))) {
	dp->flags &= ~MIN_ENDBOX_SELECTED;
	PANEL_EACH_PAINT_WINDOW(ip->panel, pw)
	    DRAWABLE_INFO_MACRO(pw, info);
	    olgx_draw_box(ip->panel->ginfo, xv_xid(info),
			  dp->min_endbox_rect.r_left,
			  dp->min_endbox_rect.r_top,
			  dp->min_endbox_rect.r_width,
			  dp->min_endbox_rect.r_height,
			  OLGX_ERASE, TRUE);
	PANEL_END_EACH_PAINT_WINDOW
	return;
    } else if ((dp->flags & MAX_ENDBOX_SELECTED) &&
	!(rect_includespoint(&dp->max_endbox_rect, event_x(event),
	event_y(event)))) {
	dp->flags &= ~MAX_ENDBOX_SELECTED;
	PANEL_EACH_PAINT_WINDOW(ip->panel, pw)
	    DRAWABLE_INFO_MACRO(pw, info);
	    olgx_draw_box(ip->panel->ginfo, xv_xid(info),
			  dp->max_endbox_rect.r_left,
			  dp->max_endbox_rect.r_top,
			  dp->max_endbox_rect.r_width,
			  dp->max_endbox_rect.r_height,
			  OLGX_ERASE, TRUE);
	PANEL_END_EACH_PAINT_WINDOW
	return;
    } else
	check_endbox_entered(ip, event);

    if (!ip->panel->status.current_item_active)
	return;

    r = dp->sliderrect;
    rect_marginadjust(&r, -1);
    if (vertical(dp))
	new = rect_bottom(&r) - event_y(event) - dp->value_offset;
    else
	new = event_x(event) - r.r_left - dp->value_offset;
    if (new == dp->apparent) {
	if (event_action(event) == ACTION_SELECT)
	    paint_slider(ip, OLGX_UPDATE);
	return;			/* state and display both correct */
    }

    dp->apparent = new;
    dp->use_print_value = FALSE;
    paint_slider(ip, OLGX_UPDATE);
    if (dp->flags & SHOWVALUE) {
	(void) SPRINTF(buf, "%d", itoe(dp, dp->apparent));
#ifdef OW_I18N
	xv_set(dp->value_textitem, PANEL_VALUE_WCS, buf, NULL);
#else
	xv_set(dp->value_textitem, PANEL_VALUE, buf, NULL);
#endif /* OW_I18N */
    }
    if (dp->flags & CONTINUOUS)
	(*ip->notify) (ITEM_PUBLIC(ip), itoe(dp, dp->apparent), event);
}


static void
slider_cancel_preview(item_public, event)
    Panel_item	    item_public;
    Event          *event;
{
    Slider_info	   *dp = SLIDER_PRIVATE(item_public);
    CHAR            buf[20];
    Item_info      *ip = ITEM_PRIVATE(item_public);

    ip->panel->status.current_item_active = FALSE;
    dp->flags &= ~(MIN_ENDBOX_SELECTED | MAX_ENDBOX_SELECTED);

    if (dp->apparent != dp->actual) {
	dp->apparent = dp->actual;
	dp->use_print_value = dp->restore_print_value;
	if (dp->flags & CONTINUOUS)
	    (*ip->notify) (ITEM_PUBLIC(ip), itoe(dp, dp->actual), event);
	paint_slider(ip, OLGX_UPDATE);
	if (dp->flags & SHOWVALUE) {
	    SPRINTF(buf, "%d", itoe(dp, dp->apparent));
#ifdef OW_I18N
	    xv_set(dp->value_textitem, PANEL_VALUE_WCS, buf, NULL);
#else
	    xv_set(dp->value_textitem, PANEL_VALUE, buf, NULL);
#endif /* OW_I18N */
	}
    }
}


static void
slider_accept_preview(item_public, event)
    Panel_item	    item_public;
    Event          *event;
{
    int		    delta=0;  /* 0=> no change, +1= increment, -1= decrement */
    Slider_info	   *dp = SLIDER_PRIVATE(item_public);
    Item_info      *ip = ITEM_PRIVATE(item_public);
    int		    mouse_left;
    int		    mouse_top;

    ip->panel->status.current_item_active = FALSE;

    if (dp->flags & MIN_ENDBOX_SELECTED) {
	dp->flags &= ~MIN_ENDBOX_SELECTED;
	if (rect_includespoint(&dp->min_endbox_rect, event_x(event),
	    event_y(event))) {
	    adjust_slider(ip, event, SLIDER_MIN);
	}
	dp->last_delta = 0;
	return;
    } else if (dp->flags & MAX_ENDBOX_SELECTED) {
	dp->flags &= ~MAX_ENDBOX_SELECTED;
	if (rect_includespoint(&dp->max_endbox_rect, event_x(event),
	    event_y(event))) {
	    adjust_slider(ip, event, SLIDER_MAX);
	}
	dp->last_delta = 0;
	return;
    }

    if (dp->apparent != dp->actual) {
	dp->actual = dp->apparent;
	/* print_value is no longer valid */
	dp->use_print_value = FALSE;
	(*ip->notify) (ITEM_PUBLIC(ip), itoe(dp, dp->actual), event);
    } else if (dp->last_delta &&
	       panel_is_multiclick(ip->panel, &dp->last_click_time,
				   &event_time(event)))
	delta = dp->last_delta;
    else if (vertical(dp)) {
	if (event_y(event) >= dp->sliderrect.r_top &&
	    event_y(event) < dp->sliderbox.r_top) {
	    /* Increment slider by 1 */
	    delta = 1;
	} else if (event_y(event) > rect_bottom(&dp->sliderbox) &&
	    event_y(event) <= rect_bottom(&dp->sliderrect)) {
	    /* Decrement slider by 1 */
	    delta = -1;
	}
    } else {
	if (event_x(event) >= dp->sliderrect.r_left &&
	    event_x(event) < dp->sliderbox.r_left) {
	    /* Decrement slider by 1 */
	    delta = -1;
	} else if (event_x(event) > rect_right(&dp->sliderbox) &&
	    event_x(event) <= rect_right(&dp->sliderrect)) {
	    /* Increment slider by 1 */
	    delta = 1;
	}
    }
    if (delta) {
	if (delta == -1) {
	    adjust_slider(ip, event, SLIDER_DECREMENT);
	    if (vertical(dp))
		/* Position pointer below the slider drag box. */
		mouse_top = MIN(rect_bottom(&dp->sliderrect),
				rect_bottom(&dp->sliderbox) + POINTER_GAP);
	    else
		/* Position pointer to the left of the slider drag box. */
		mouse_left = MAX(dp->sliderrect.r_left,
				 dp->sliderbox.r_left - POINTER_GAP);
	} else {
	    adjust_slider(ip, event, SLIDER_INCREMENT);
	    if (vertical(dp))
		/* Position pointer above the slider drag box. */
		mouse_top = MAX(dp->sliderrect.r_top,
				dp->sliderbox.r_top - POINTER_GAP);
	    else
		/* Position pointer to the right of the slider drag box. */
		mouse_left = MIN(rect_right(&dp->sliderrect),
				 rect_right(&dp->sliderbox) + POINTER_GAP);
	}
	if (vertical(dp))
	    mouse_left = dp->sliderbox.r_left + dp->sliderbox.r_width/2;
	else
	    mouse_top = dp->sliderbox.r_top + dp->sliderbox.r_height/2;
	xv_set(PANEL_PUBLIC(ip->panel),
	       WIN_MOUSE_XY, mouse_left, mouse_top,
	       NULL);
    } else
	paint_slider(ip, OLGX_UPDATE);
    dp->last_click_time = event_time(event);
    dp->last_delta = delta;
}


static void
slider_accept_key(item_public, event)
    Panel_item	    item_public;
    Event          *event;
{
    Slider_info	   *dp = SLIDER_PRIVATE(item_public);
    Item_info      *ip = ITEM_PRIVATE(item_public);

    if (event_is_up(event))
	return;
    switch(event_action(event)) {
      case ACTION_UP:
	if (vertical(dp))
	    adjust_slider(ip, event, SLIDER_INCREMENT);
	return;
      case ACTION_DOWN:
	if (vertical(dp))
	    adjust_slider(ip, event, SLIDER_DECREMENT);
	return;
      case ACTION_LEFT:
	if (!vertical(dp))
	    adjust_slider(ip, event, SLIDER_DECREMENT);
	return;
      case ACTION_RIGHT:
	if (!vertical(dp))
	    adjust_slider(ip, event, SLIDER_INCREMENT);
	return;
      case ACTION_JUMP_UP:
	if (vertical(dp))
	    adjust_slider(ip, event, SLIDER_JUMP_INCREMENT);
	return;
      case ACTION_JUMP_DOWN:
	if (vertical(dp))
	    adjust_slider(ip, event, SLIDER_JUMP_DECREMENT);
	return;
      case ACTION_JUMP_LEFT:
	if (!vertical(dp))
	    adjust_slider(ip, event, SLIDER_JUMP_DECREMENT);
	return;
      case ACTION_JUMP_RIGHT:
	if (!vertical(dp))
	    adjust_slider(ip, event, SLIDER_JUMP_INCREMENT);
	return;
      case ACTION_LINE_START:
      case ACTION_DATA_START:
	adjust_slider(ip, event, SLIDER_MIN);
	return;
      case ACTION_LINE_END:
      case ACTION_DATA_END:
	adjust_slider(ip, event, SLIDER_MAX);
	return;
    }
}


static void
slider_paint(item_public)
    Panel_item	    item_public;
{
    CHAR            buf[10];
    Slider_info	   *dp = SLIDER_PRIVATE(item_public);
    GC             *gc_list;	/* Graphics Context list */
    Xv_Drawable_info *info;
    Item_info      *ip = ITEM_PRIVATE(item_public);
    Xv_Window       pw;		/* Paint Window */
    Rect           *r;
    CHAR	   *str;
    Item_info      *tp;

    r = &(ip->label_rect);

    /* paint the label */
    panel_paint_image(ip->panel, &ip->label, r, inactive(ip), ip->color_index);

    /* Paint the text item, if not hidden */
    if (dp->value_textitem) {
	tp = ITEM_PRIVATE(dp->value_textitem);
	if (!hidden(tp))
	    (*tp->ops.panel_op_paint) (dp->value_textitem);
    }

    /* paint the range */
    if (dp->flags & SHOWRANGE) {
	if (dp->min_value_string)
	    str = dp->min_value_string;
	else {
	    (void) SPRINTF(buf, "%d", dp->min_value);
	    str = buf;
	}
	r = &dp->min_range_rect;
	PANEL_EACH_PAINT_WINDOW(ip->panel, pw)
#ifdef OW_I18N
	    panel_paint_text(pw, ip->panel->std_fontset_id, 
	    	ip->color_index, r->r_left, r->r_top + 
		panel_fonthome(ip->panel->std_font), str);
#else
	    panel_paint_text(pw, ip->panel->std_font_xid, ip->color_index,
		r->r_left, r->r_top + panel_fonthome(ip->panel->std_font), str);
#endif /* OW_I18N */
	    if (inactive(ip)) {
		DRAWABLE_INFO_MACRO(pw, info);
		gc_list = (GC *)xv_get(xv_screen(info), SCREEN_OLGC_LIST, pw);
		screen_adjust_gc_color(pw, SCREEN_INACTIVE_GC);
		XFillRectangle(xv_display(info), xv_xid(info),
			       gc_list[SCREEN_INACTIVE_GC],
			       r->r_left, r->r_top, r->r_width, r->r_height);
	    }
	PANEL_END_EACH_PAINT_WINDOW

	if (dp->max_value_string)
	    str = dp->max_value_string;
	else {
	    (void) SPRINTF(buf, "%d", dp->max_value);
	    str = buf;
	}
	r = &dp->max_range_rect;
	PANEL_EACH_PAINT_WINDOW(ip->panel, pw)
#ifdef OW_I18N
	    panel_paint_text(pw, ip->panel->std_fontset_id, 
		ip->color_index, r->r_left, r->r_top + 
		panel_fonthome(ip->panel->std_font), str);
#else
	    panel_paint_text(pw, ip->panel->std_font_xid, ip->color_index,
		r->r_left, r->r_top + panel_fonthome(ip->panel->std_font), str);
#endif /* OW_I18N */
	    if (inactive(ip)) {
		DRAWABLE_INFO_MACRO(pw, info);
		gc_list = (GC *)xv_get(xv_screen(info), SCREEN_OLGC_LIST, pw);
		screen_adjust_gc_color(pw, SCREEN_INACTIVE_GC);
		XFillRectangle(xv_display(info), xv_xid(info),
			       gc_list[SCREEN_INACTIVE_GC],
			       r->r_left, r->r_top, r->r_width, r->r_height);
	    }
	PANEL_END_EACH_PAINT_WINDOW
    }

    /* paint the slider */
    paint_slider(ip, 0);
}


static void
slider_remove(item_public)
    Panel_item	    item_public;
{
    Item_info      *ip = ITEM_PRIVATE(item_public);
    Panel_info	   *panel = ip->panel;

    /*
     * Only reassign the keyboard focus to another item if the panel isn't
     * being destroyed.
     */
    if (!panel->status.destroying && panel->kbd_focus_item == ip) {
	panel->kbd_focus_item = panel_next_kbd_focus(panel, TRUE);
	panel_accept_kbd_focus(panel);
    }

    return;
}


static void
slider_layout(item_public, deltas)
    Panel_item	    item_public;
    Rect           *deltas;
{
    Slider_info	   *dp = SLIDER_PRIVATE(item_public);
    Item_info      *ip = ITEM_PRIVATE(item_public);

    if (!created(ip))
	return;
    dp->valuerect.r_left += deltas->r_left;
    dp->valuerect.r_top += deltas->r_top;
    dp->min_endbox_rect.r_left += deltas->r_left;
    dp->min_endbox_rect.r_top += deltas->r_top;
    dp->min_range_rect.r_left += deltas->r_left;
    dp->min_range_rect.r_top += deltas->r_top;
    dp->sliderrect.r_left += deltas->r_left;
    dp->sliderrect.r_top += deltas->r_top;
    dp->tickrect.r_left += deltas->r_left;
    dp->tickrect.r_top += deltas->r_top;
    dp->tickstringrect.r_left += deltas->r_left;
    dp->tickstringrect.r_top += deltas->r_top;
    dp->max_range_rect.r_left += deltas->r_left;
    dp->max_range_rect.r_top += deltas->r_top;
    dp->max_endbox_rect.r_left += deltas->r_left;
    dp->max_endbox_rect.r_top += deltas->r_top;
    if (dp->value_textitem)
	xv_set(dp->value_textitem,
	       PANEL_ITEM_X, dp->valuerect.r_left,
	       PANEL_ITEM_Y, dp->valuerect.r_top,
	       NULL);
}


static void
slider_accept_kbd_focus(item_public)
    Panel_item	    item_public;
{
    Slider_info	   *dp = SLIDER_PRIVATE(item_public);
    Frame	    frame;
    Item_info      *ip = ITEM_PRIVATE(item_public);
    int		    x;
    int		    y;

    frame = xv_get(PANEL_PUBLIC(ip->panel), WIN_FRAME);
    if (vertical(dp)) {
	xv_set(frame, FRAME_FOCUS_DIRECTION, FRAME_FOCUS_RIGHT, NULL);
	x = dp->max_range_rect.r_left - FRAME_FOCUS_RIGHT_WIDTH;
	y = dp->max_range_rect.r_top;
    } else {
	xv_set(frame, FRAME_FOCUS_DIRECTION, FRAME_FOCUS_UP, NULL);
	x = dp->min_range_rect.r_left;
	y = dp->min_range_rect.r_top + dp->min_range_rect.r_height;
    }
    if (x < 0)
	x = 0;
    if (y < 0)
	y = 0;
    panel_show_focus_win(item_public, frame, x, y);
}


static void
slider_yield_kbd_focus(item_public)
    Panel_item	    item_public;
{
    Xv_Window	    focus_win;
    Frame	    frame;
    Item_info      *ip = ITEM_PRIVATE(item_public);

    frame = xv_get(PANEL_PUBLIC(ip->panel), WIN_FRAME);
    focus_win = xv_get(frame, FRAME_FOCUS_WIN);
    xv_set(focus_win, XV_SHOW, FALSE, NULL);
}



/* --------------------  Local Routines  -------------------- */
static void
adjust_slider(ip, event, adjustment)
    Item_info	   *ip;
    Event	   *event;
    Slider_adjust   adjustment;
{
    CHAR            buf[20];
    int		    delta = 0;
    Slider_info    *dp = SLIDER_FROM_ITEM(ip);

    switch (adjustment) {
      case SLIDER_MIN:
	dp->actual = dp->apparent = 0;   /* in pixels */
	dp->print_value = dp->min_value;
	dp->use_print_value = TRUE;
	panel_clear_rect(ip->panel, dp->sliderrect);
	paint_slider(ip, 0);
	if (dp->flags & SHOWVALUE) {
	    SPRINTF(buf, "%d", dp->min_value);
#ifdef OW_I18N
	    xv_set(dp->value_textitem, PANEL_VALUE_WCS, buf, NULL);
#else
	    xv_set(dp->value_textitem, PANEL_VALUE, buf, NULL);
#endif /* OW_I18N */
	}
	break;
      case SLIDER_MAX:
	dp->actual = dp->apparent = dp->width;   /* in pixels */
	dp->print_value = dp->max_value;
	dp->use_print_value = FALSE;
	panel_clear_rect(ip->panel, dp->sliderrect);
	paint_slider(ip, 0);
	if (dp->flags & SHOWVALUE) {
	    SPRINTF(buf, "%d", dp->max_value);
#ifdef OW_I18N
	    xv_set(dp->value_textitem, PANEL_VALUE_WCS, buf, NULL);
#else
	    xv_set(dp->value_textitem, PANEL_VALUE, buf, NULL);
#endif /* OW_I18N */
	}
	break;
      case SLIDER_INCREMENT:
	delta = 1;
	break;
      case SLIDER_DECREMENT:
	delta = -1;
	break;
      case SLIDER_JUMP_INCREMENT:
	delta = dp->jump_delta;
	break;
      case SLIDER_JUMP_DECREMENT:
	delta = -dp->jump_delta;
	break;
    }
    if (delta) {
	/* Note: etoi and itoe enforce min/max values */
	dp->print_value =
	    (dp->use_print_value ? dp->print_value : itoe(dp, dp->actual)) +
	    delta;
	if (dp->print_value < dp->min_value)
	    dp->print_value = dp->min_value;
	else if (dp->print_value > dp->max_value)
	    dp->print_value = dp->max_value;
	dp->use_print_value = TRUE;
	dp->apparent = dp->actual = etoi(dp, dp->print_value);  /* in pixels */
	paint_slider(ip, OLGX_UPDATE); /* paint slider & update dp->sliderbox */
	if (dp->flags & SHOWVALUE) {
	    SPRINTF(buf, "%d", dp->print_value);
#ifdef OW_I18N
	    xv_set(dp->value_textitem, PANEL_VALUE_WCS, buf, NULL);
#else
	    xv_set(dp->value_textitem, PANEL_VALUE, buf, NULL);
#endif /* OW_I18N */
	}
   }
   (*ip->notify) (ITEM_PUBLIC(ip), dp->print_value, event);
}


static void
check_endbox_entered(ip, event)
    Item_info      *ip;
    Event          *event;
{
    Slider_info    *dp = SLIDER_FROM_ITEM(ip);
    Xv_Drawable_info *info;
    Xv_Window       pw;

    if (rect_includespoint(&dp->min_endbox_rect, event_x(event),
	event_y(event))) {
	dp->flags |= MIN_ENDBOX_SELECTED;
	PANEL_EACH_PAINT_WINDOW(ip->panel, pw)
	    DRAWABLE_INFO_MACRO(pw, info);
	    olgx_draw_box(ip->panel->ginfo, xv_xid(info),
			  dp->min_endbox_rect.r_left,
			  dp->min_endbox_rect.r_top,
			  dp->min_endbox_rect.r_width,
			  dp->min_endbox_rect.r_height,
			  OLGX_INVOKED, TRUE);
	PANEL_END_EACH_PAINT_WINDOW
    } else if (rect_includespoint(&dp->max_endbox_rect, event_x(event),
	event_y(event))) {
	dp->flags |= MAX_ENDBOX_SELECTED;
	PANEL_EACH_PAINT_WINDOW(ip->panel, pw)
	    DRAWABLE_INFO_MACRO(pw, info);
	    olgx_draw_box(ip->panel->ginfo, xv_xid(info),
			  dp->max_endbox_rect.r_left,
			  dp->max_endbox_rect.r_top,
			  dp->max_endbox_rect.r_width,
			  dp->max_endbox_rect.r_height,
			  OLGX_INVOKED, TRUE);
	PANEL_END_EACH_PAINT_WINDOW
    }
}


/*
 * Convert external value (client units) to internal value (pixels).
 */
static int
  etoi(Slider_info *dp, int value)
{
    if (value <= dp->min_value)
	return (0);

    if (value >= dp->max_value)
	return (dp->width);

    return (panel_round(((value - dp->min_value) * dp->width),
		        (dp->max_value - dp->min_value)));
}


static Panel_setting
  get_value(item_public, event)
    Panel_item     item_public;
    Event          *event;
{
    CHAR            buf[20];
    int		    value_changed = FALSE;
    Item_info      *ip = (Item_info *) xv_get(item_public, PANEL_CLIENT_DATA);
    Slider_info    *dp = SLIDER_FROM_ITEM(ip);
    int             value;

    value = atoi((char *) xv_get(item_public, PANEL_VALUE));
    if (value < dp->min_value) {
	value = dp->min_value;
	value_changed = TRUE;
    } else if (value > dp->max_value) {
	value = dp->max_value;
	value_changed = TRUE;
    }
    if (value_changed) {
	SPRINTF(buf, "%d", value);
#ifdef OW_I18N
	xv_set(item_public, PANEL_VALUE_WCS, buf, NULL);
#else
	xv_set(item_public, PANEL_VALUE, buf, NULL);
#endif /* OW_I18N */
    }
    dp->apparent = dp->actual = etoi(dp, value);
    dp->use_print_value = TRUE;
    dp->print_value = value;
    paint_slider(ip, OLGX_UPDATE);

    (*ip->notify) (ITEM_PUBLIC(ip), value, event);

    return panel_text_notify(item_public, event);
}


/*
 * Convert internal value (pixels) to external value (client units).
 */
static int
  itoe(Slider_info *dp, int value)
{
    /* use the print value if valid */
    if (dp->use_print_value)
	return dp->print_value;

    if (value <= 0)
	return (dp->min_value);

    if (value >= dp->width - 1)
	return (dp->max_value);

    return (dp->min_value +
	    (value * (dp->max_value - dp->min_value + 1)) / dp->width);
}


static void
paint_slider(ip, olgx_state)
    Item_info      *ip;
    int		    olgx_state;
{
    register Slider_info    *dp = SLIDER_FROM_ITEM(ip);
    int		    height;
    GC             *gc_list;	/* Graphics Context list */
    Xv_Drawable_info *info;
    int		    limit;
    int		    new_value;
    Rect	    old_sliderbox;
    int		    old_value;
    Xv_Window       pw;
    Rect           *r = &dp->sliderrect;
    int		    save_black;
    int		    tick;
    int		    tick_gap;
    int		    width;
    int		    x;
    Drawable	    xid;
    int		    y;

    /* Update the sliderbox location. */
    old_sliderbox = dp->sliderbox;
    new_value = dp->apparent;
    if (new_value < 0)
	new_value = 0;
    else if (new_value > dp->width)
	new_value = dp->width;
    if (vertical(dp)) {
	dp->sliderbox.r_left = r->r_left;
	dp->sliderbox.r_top = rect_bottom(r) - new_value -
	   (dp->sliderbox.r_height - 1);
	old_value = rect_bottom(r) - rect_bottom(&old_sliderbox);
	olgx_state |= OLGX_VERTICAL;
    } else {
	dp->sliderbox.r_left = r->r_left + new_value;
	dp->sliderbox.r_top = r->r_top;
	old_value = old_sliderbox.r_left - dp->sliderrect.r_left;
	olgx_state |= OLGX_HORIZONTAL;
    }
    if (inactive(ip))
	olgx_state |= OLGX_INACTIVE;
    if (ip->panel->status.current_item_active 
	&& (ip->panel->kbd_focus_item == ip))
	olgx_state |= OLGX_INVOKED;

    if (ip->color_index >= 0)
	save_black = olgx_get_single_color(ip->panel->ginfo, OLGX_BLACK);

    PANEL_EACH_PAINT_WINDOW(ip->panel, pw)
	DRAWABLE_INFO_MACRO(pw, info);
	xid = xv_xid(info);
	if (ip->color_index >= 0) {
	    olgx_set_single_color(ip->panel->ginfo, OLGX_BLACK,
				  xv_get(xv_cms(info), CMS_PIXEL,
				  ip->color_index), OLGX_SPECIAL);
	}
	olgx_draw_slider(ip->panel->ginfo, xid,
			 dp->sliderrect.r_left, dp->sliderrect.r_top,
			 dp->width +
			     HorizSliderControl_Width(ip->panel->ginfo),
			 old_value, new_value, olgx_state);
	if ((olgx_state & OLGX_UPDATE) == 0) {
	    if (dp->flags & SHOWENDBOXES) {
		olgx_draw_box(ip->panel->ginfo, xid,
			      dp->min_endbox_rect.r_left,
			      dp->min_endbox_rect.r_top,
			      dp->min_endbox_rect.r_width,
			      dp->min_endbox_rect.r_height,
			      ip->panel->status.three_d ? 0 : OLGX_ERASE, TRUE);
		olgx_draw_box(ip->panel->ginfo, xid,
			      dp->max_endbox_rect.r_left,
			      dp->max_endbox_rect.r_top,
			      dp->max_endbox_rect.r_width,
			      dp->max_endbox_rect.r_height,
			      ip->panel->status.three_d ? 0 : OLGX_ERASE, TRUE);
	    }
	    if (dp->nticks) {
		x = dp->tickrect.r_left;
		y = dp->tickrect.r_top;
		if (vertical(dp)) {
                    if (dp->nticks == 1)
                        tick_gap == dp->tickrect.r_height;
                    else
                        tick_gap = dp->tickrect.r_height / (dp->nticks-1);

		    width = dp->tickrect.r_width;
		    height = TICK_THICKNESS;
		    limit = dp->tickrect.r_top+dp->tickrect.r_height -
			TICK_THICKNESS;
		} else {
                    if (dp->nticks == 1)
                        tick_gap == dp->tickrect.r_width;
                    else
                        tick_gap = dp->tickrect.r_width / (dp->nticks-1);

		    width = TICK_THICKNESS;
		    height = dp->tickrect.r_height;
		    limit = dp->tickrect.r_left+dp->tickrect.r_width -
			TICK_THICKNESS;
		}
		for (tick = 1; tick <= dp->nticks; tick++) {
		    olgx_draw_box(ip->panel->ginfo, xid,
				  x, y, width, height, OLGX_NORMAL, FALSE);
		    if (vertical(dp)) {
			y += tick_gap;
			if (y > limit)
			    y = limit;
		    } else {
			x += tick_gap;
			if (x > limit)
			    x = limit;
		    }
		}
		
		if (dp->min_tick_string) {
		    if (vertical(dp))
			y = rect_bottom(&dp->tickstringrect) -
			    xv_get(ip->panel->std_font,
				   FONT_DEFAULT_CHAR_HEIGHT);
		    else
			y = dp->tickstringrect.r_top;
#ifdef OW_I18N
		    panel_paint_text(pw, ip->panel->std_fontset_id,
			ip->color_index, dp->tickstringrect.r_left,
			y + panel_fonthome(ip->panel->std_font),
			dp->min_tick_string);
#else
		    panel_paint_text(pw, ip->panel->std_font_xid,
			ip->color_index, dp->tickstringrect.r_left,
			y + panel_fonthome(ip->panel->std_font),
			dp->min_tick_string);
#endif /* OW_I18N */
		}

		if (dp->max_tick_string) {
		    if (vertical(dp))
			x = dp->tickstringrect.r_left;
		    else
			x = rect_right(&dp->tickstringrect) -
			    dp->max_tick_string_width;
#ifdef OW_I18N
		    panel_paint_text(pw, ip->panel->std_fontset_id,
			ip->color_index, x,
			dp->tickstringrect.r_top +
			    panel_fonthome(ip->panel->std_font),
			dp->max_tick_string);
#else
		    panel_paint_text(pw, ip->panel->std_font_xid,
			ip->color_index, x,
			dp->tickstringrect.r_top +
			    panel_fonthome(ip->panel->std_font),
			dp->max_tick_string);
#endif /* OW_I18N */
		}
		if (inactive(ip)) {
		    gc_list = (GC *)xv_get(xv_screen(info),
					   SCREEN_OLGC_LIST, pw);
		    screen_adjust_gc_color(pw, SCREEN_INACTIVE_GC);
		    XFillRectangle(xv_display(info), xv_xid(info),
				   gc_list[SCREEN_INACTIVE_GC],
				   dp->tickrect.r_left,
				   dp->tickrect.r_top,
				   dp->tickrect.r_width,
				   dp->tickrect.r_height);
		    XFillRectangle(xv_display(info), xv_xid(info),
				   gc_list[SCREEN_INACTIVE_GC],
				   dp->tickstringrect.r_left,
				   dp->tickstringrect.r_top,
				   dp->tickstringrect.r_width,
				   dp->tickstringrect.r_height);
		}
	    }
	}
    PANEL_END_EACH_PAINT_WINDOW

    if (ip->color_index >= 0)
	olgx_set_single_color(ip->panel->ginfo, OLGX_BLACK, save_black,
			      OLGX_SPECIAL);
}


static void
update_rects(ip)
    register Item_info *ip;
{
    register Slider_info *dp = SLIDER_FROM_ITEM(ip);
    int		chrht, chrwth;
    int		range_rects_max_width;
    struct pr_size size;
#ifdef OW_I18N
    int		col_width; /* Default width of a column */
#endif /* OW_I18N */

    /* Create the text value rect */
    if (dp->flags & SHOWVALUE) {
	dp->valuerect = *(Rect *) xv_get(dp->value_textitem, PANEL_ITEM_RECT);
	dp->valuerect.r_width += TEXT_VALUE_GAP;
    } else {
	rect_construct(&dp->valuerect,
		       rect_right(&ip->label_rect) + TEXT_VALUE_GAP,
	    	       ip->label_rect.r_top, 0, 0);
    }

    chrht = xv_get(ip->panel->std_font, FONT_DEFAULT_CHAR_HEIGHT);
    chrwth = xv_get(ip->panel->std_font, FONT_DEFAULT_CHAR_WIDTH);
#ifdef OW_I18N
    /*  We need column width because column != byte != char */
    col_width = xv_get(ip->panel->std_font, FONT_COLUMN_WIDTH);
#endif /* OW_I18N */
    if (vertical(dp)) {
	/* Create the maximum range rect */
	dp->max_range_rect.r_top = dp->valuerect.r_top;
	dp->max_range_rect.r_left = dp->valuerect.r_left +
	   dp->valuerect.r_width;
	if (dp->flags & SHOWRANGE) {
	    dp->max_range_rect.r_height = chrht + TEXT_VALUE_GAP;
	    if (dp->max_value_string)
#ifdef OW_I18N
	/*  If there's no max_value_string I am assuming dp->max_range_size
	 *  is the number of numerical letters.  For now, each numerical
	 *  letter, i.e. 1, 2, only occupies one column
	 */
	    {
		size = xv_pf_textwidth_wc(wslen(dp->max_value_string),
				       ip->panel->std_font,
				       dp->max_value_string);
		dp->max_range_rect.r_width = size.x;
	    } else
		dp->max_range_rect.r_width = col_width * dp->max_range_size;
#else
	    {
		size = xv_pf_textwidth(strlen(dp->max_value_string),
				       ip->panel->std_font,
				       dp->max_value_string);
		dp->max_range_rect.r_width = size.x;
	    } else
		dp->max_range_rect.r_width = chrwth * dp->max_range_size;
#endif /* OW_I18N */
	} else {
	    dp->max_range_rect.r_height = 0;
	    dp->max_range_rect.r_width = 0;
	}

	/* Calculate the width and height of the minimum range rect */
	if (dp->flags & SHOWRANGE) {
	    if (dp->min_value_string)
#ifdef OW_I18N
	/*  If there's no min_value_string I am assuming dp->min_range_size
	 *  is the number of numerical letters.  For now, each numerical
	 *  letter, i.e. 1, 2, only occupies one column
	 */
	    {
		size = xv_pf_textwidth_wc(wslen(dp->min_value_string),
				       ip->panel->std_font,
				       dp->min_value_string);
		dp->min_range_rect.r_width = size.x;
	    } else
		dp->min_range_rect.r_width = col_width * dp->min_range_size;
#else
	    {
		size = xv_pf_textwidth(strlen(dp->min_value_string),
				       ip->panel->std_font,
				       dp->min_value_string);
		dp->min_range_rect.r_width = size.x;
	    } else
		dp->min_range_rect.r_width = chrwth * dp->min_range_size;
#endif /* OW_I18N */
	} else
	    dp->min_range_rect.r_width = 0;
	dp->min_range_rect.r_height = dp->max_range_rect.r_height;
	range_rects_max_width = MAX(dp->max_range_rect.r_width,
				    dp->min_range_rect.r_width);

	/* Center the maximum range rect over the slider rect */
	if (dp->flags & SHOWRANGE)
	    dp->max_range_rect.r_left += (range_rects_max_width -
		dp->max_range_rect.r_width) / 2;

	/* Create the maximum end box rect */
	dp->max_endbox_rect.r_top = dp->max_range_rect.r_top +
	    dp->max_range_rect.r_height;
	if (dp->flags & SHOWENDBOXES) {
	    dp->max_endbox_rect.r_width =
		HorizSliderControl_Height(ip->panel->ginfo) - 4;
	    dp->max_endbox_rect.r_height =
		HorizSliderControl_Width(ip->panel->ginfo) - 5;
	} else {
	    dp->max_endbox_rect.r_width = 0;
	    dp->max_endbox_rect.r_height = 0;
	}
	dp->max_endbox_rect.r_left = dp->valuerect.r_left +
	    dp->valuerect.r_width + 2 +
	    (range_rects_max_width - dp->max_endbox_rect.r_width) / 2;

	/* Create the slider rect, centered within the range rects */
	dp->sliderrect.r_top = dp->max_endbox_rect.r_top +
	    dp->max_endbox_rect.r_height +
	    (dp->flags & SHOWENDBOXES ? TEXT_VALUE_GAP : 0);
	dp->sliderrect.r_width = HorizSliderControl_Height(ip->panel->ginfo);
	dp->sliderrect.r_left = dp->valuerect.r_left + dp->valuerect.r_width;
	if (range_rects_max_width > dp->sliderrect.r_width)
	    dp->sliderrect.r_left += (range_rects_max_width -
		dp->sliderrect.r_width) / 2;
	dp->sliderrect.r_height = dp->width +
	    HorizSliderControl_Width(ip->panel->ginfo);

	/* Define width and height of slider drag box */
	dp->sliderbox.r_width = HorizSliderControl_Height(ip->panel->ginfo);
	dp->sliderbox.r_height = HorizSliderControl_Width(ip->panel->ginfo);

	/* Create the tick rect */
	dp->tickrect.r_top = dp->sliderrect.r_top +
		HorizSliderControl_Width(ip->panel->ginfo)/2;
	dp->tickrect.r_left = dp->sliderrect.r_left + dp->sliderrect.r_width;
	if (dp->nticks)
	    dp->tickrect.r_width = 
		HorizSliderControl_Height(ip->panel->ginfo)/2 - 1;
	else
	    dp->tickrect.r_width = 0;
	dp->tickrect.r_height = dp->sliderrect.r_height - 
		HorizSliderControl_Width(ip->panel->ginfo);

	/* Create the tick string rect */
	dp->tickstringrect.r_left = dp->tickrect.r_left + dp->tickrect.r_width +
	chrwth/2;
	dp->tickstringrect.r_top = MAX(dp->tickrect.r_top - chrht/2,
				       dp->valuerect.r_top);
	dp->tickstringrect.r_height = dp->tickrect.r_top +
	    dp->tickrect.r_height - dp->tickstringrect.r_top;
	if (dp->nticks && (dp->min_tick_string || dp->max_tick_string)) {
	    dp->tickstringrect.r_height += chrht/2;
	    dp->tickstringrect.r_width = MAX(dp->min_tick_string_width,
					     dp->max_tick_string_width);
	} else
	    dp->tickstringrect.r_width = 0;

	/* Create the minimum end box rect */
	dp->min_endbox_rect.r_top = dp->sliderrect.r_top +
	    dp->sliderrect.r_height + TEXT_VALUE_GAP;
	dp->min_endbox_rect.r_left = dp->max_endbox_rect.r_left;
	dp->min_endbox_rect.r_width = dp->max_endbox_rect.r_width;
	dp->min_endbox_rect.r_height = dp->max_endbox_rect.r_height;

	/* Calculate the coordinates of the minimum range rect */
	dp->min_range_rect.r_left = dp->valuerect.r_left +
	   dp->valuerect.r_width;
	dp->min_range_rect.r_top = rect_bottom(&dp->min_endbox_rect) +
	    (dp->min_endbox_rect.r_height ? TEXT_VALUE_GAP : 1);
	if (dp->flags & SHOWRANGE)
	    dp->min_range_rect.r_left += (range_rects_max_width -
		dp->min_range_rect.r_width) / 2;

	/* Update the item value rect */
	ip->value_rect.r_width =
	    MAX(MAX(dp->min_range_rect.r_left + dp->min_range_rect.r_width,
		    dp->tickstringrect.r_left + dp->tickstringrect.r_width),
		dp->max_range_rect.r_left + dp->max_range_rect.r_width) -
	    dp->valuerect.r_left;
	ip->value_rect.r_height = dp->min_range_rect.r_top +
	    dp->min_range_rect.r_height - dp->max_range_rect.r_top;

    } else { /*** Horizontal ***/
	dp->min_range_rect.r_top = dp->valuerect.r_top;
	dp->min_range_rect.r_left = dp->valuerect.r_left +
	   dp->valuerect.r_width;
	if (dp->flags & SHOWRANGE) {
	    if (dp->min_value_string)
#ifdef OW_I18N
	/*  If there's no min_value_string I am assuming dp->min_range_size
	 *  is the number of numerical letters.  For now, each numerical
	 *  letter, i.e. 1, 2, only occupies one column
	 */
	    {
		size = xv_pf_textwidth_wc(wslen(dp->min_value_string),
				       ip->panel->std_font,
				       dp->min_value_string);
		dp->min_range_rect.r_width = size.x + TEXT_VALUE_GAP;
	    } else
		dp->min_range_rect.r_width =
		    col_width * dp->min_range_size + TEXT_VALUE_GAP;
#else
	    {
		size = xv_pf_textwidth(strlen(dp->min_value_string),
				       ip->panel->std_font,
				       dp->min_value_string);
		dp->min_range_rect.r_width = size.x + TEXT_VALUE_GAP;
	    } else
		dp->min_range_rect.r_width =
		    chrwth * dp->min_range_size + TEXT_VALUE_GAP;
#endif /* OW_I18N */
	} else
	    dp->min_range_rect.r_width = 0;
	dp->min_range_rect.r_height =
	    HorizSliderControl_Height(ip->panel->ginfo);

	/* Create the minimum end box rect */
	dp->min_endbox_rect.r_top = dp->min_range_rect.r_top + 2;
	dp->min_endbox_rect.r_left = dp->min_range_rect.r_left +
	    dp->min_range_rect.r_width;
	if (dp->flags & SHOWENDBOXES) {
	    dp->min_endbox_rect.r_height =
		HorizSliderControl_Height(ip->panel->ginfo) - 4;
	    dp->min_endbox_rect.r_width =
		HorizSliderControl_Width(ip->panel->ginfo) - 5;
	} else {
	    dp->min_endbox_rect.r_height = 0;
	    dp->min_endbox_rect.r_width = 0;
	}

	/* Create the slider rect */
	dp->sliderrect.r_top = dp->min_range_rect.r_top;
	dp->sliderrect.r_left = dp->min_endbox_rect.r_left +
	    dp->min_endbox_rect.r_width +
	    (dp->flags & SHOWENDBOXES ? TEXT_VALUE_GAP : 0);
	dp->sliderrect.r_width = dp->width + 
	    HorizSliderControl_Width(ip->panel->ginfo);
	dp->sliderrect.r_height = HorizSliderControl_Height(ip->panel->ginfo);

	/* Define width and height of slider drag box */
	dp->sliderbox.r_width = HorizSliderControl_Width(ip->panel->ginfo);
	dp->sliderbox.r_height = HorizSliderControl_Height(ip->panel->ginfo);

	/* Create the maximum end box rect */
	dp->max_endbox_rect.r_left = dp->sliderrect.r_left +
	    dp->sliderrect.r_width + TEXT_VALUE_GAP;
	dp->max_endbox_rect.r_top = dp->min_endbox_rect.r_top;
	dp->max_endbox_rect.r_width = dp->min_endbox_rect.r_width;
	dp->max_endbox_rect.r_height = dp->min_endbox_rect.r_height;

	/* Create the tick rect */
	dp->tickrect.r_top = dp->sliderrect.r_top + dp->sliderrect.r_height;
	dp->tickrect.r_left = dp->sliderrect.r_left +
	    HorizSliderControl_Width(ip->panel->ginfo)/2;
	dp->tickrect.r_width = dp->width;
	if (dp->nticks) {
	    dp->tickrect.r_height = 
		HorizSliderControl_Height(ip->panel->ginfo)/2 - 1;
	} else
	    dp->tickrect.r_height = 0;

	/* Create the tick string rect */
	dp->tickstringrect.r_left = MAX(dp->tickrect.r_left -
	    dp->min_tick_string_width/2, dp->min_endbox_rect.r_left);
	dp->tickstringrect.r_top = dp->tickrect.r_top + dp->tickrect.r_height +
	    chrht/2;
	dp->tickstringrect.r_width = MIN(rect_right(&dp->tickrect) +
	    dp->max_tick_string_width/2, rect_right(&dp->max_endbox_rect)) -
	    dp->tickstringrect.r_left + 1;
	if (dp->nticks && (dp->min_tick_string || dp->max_tick_string))
	    dp->tickstringrect.r_height = chrht;
	else
	    dp->tickstringrect.r_height = 0;

	/* Create the maximum range rect */
	dp->max_range_rect.r_left = rect_right(&dp->max_endbox_rect) +
	    (dp->max_endbox_rect.r_width ? TEXT_VALUE_GAP : 1);
	dp->max_range_rect.r_top = dp->min_range_rect.r_top;
	if (dp->flags & SHOWRANGE) {
	    if (dp->max_value_string)
#ifdef OW_I18N
	/*  If there's no max_value_string I am assuming dp->max_range_size
	 *  is the number of numerical letters.  For now, each numerical
	 *  letter, i.e. 1, 2, only occupies one column
	 */
	    {
		size = xv_pf_textwidth_wc(wslen(dp->max_value_string),
				       ip->panel->std_font,
				       dp->max_value_string);
		dp->max_range_rect.r_width = size.x;
	    } else
		dp->max_range_rect.r_width = col_width * dp->max_range_size;
#else
	    {
		size = xv_pf_textwidth(strlen(dp->max_value_string),
				       ip->panel->std_font,
				       dp->max_value_string);
		dp->max_range_rect.r_width = size.x;
	    } else
		dp->max_range_rect.r_width = chrwth * dp->max_range_size;
#endif /* OW_I18N */
	} else
	    dp->max_range_rect.r_width = 0;
	dp->max_range_rect.r_height = dp->min_range_rect.r_height;

	/* Update the item value rect */
	ip->value_rect.r_width = MAX(rect_right(&dp->max_range_rect),
	    rect_right(&dp->tickstringrect)) + 1 - dp->valuerect.r_left;
	ip->value_rect.r_height = MAX(dp->valuerect.r_height,
	    dp->tickstringrect.r_top + dp->tickstringrect.r_height -
	    dp->sliderrect.r_top);
    }

    ip->value_rect.r_left = dp->valuerect.r_left;

    /* Update the item rect */
    ip->rect = panel_enclosing_rect(&ip->label_rect, &ip->value_rect);
}
