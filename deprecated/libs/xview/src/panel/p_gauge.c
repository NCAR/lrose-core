#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)p_gauge.c 1.30 93/06/28 Copyr 1984 Sun Micro";
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
#include <xview_private/pw_impl.h>

#define GAUGE_PRIVATE(item)	\
	XV_PRIVATE(Gauge_info, Xv_panel_gauge, item)
#define GAUGE_PUBLIC(item)	XV_PUBLIC(item)

#define	GAUGE_FROM_ITEM(ip)	GAUGE_PRIVATE(ITEM_PUBLIC(ip))

#define TICK_THICKNESS	2
#define MIN_TICK_GAP	4	/* minimum gap between ticks */
#define MIN_TICK_STRING_GAP 6

#ifdef OW_I18N
extern struct pr_size xv_pf_textwidth_wc();
#else
extern struct pr_size xv_pf_textwidth();
#endif /* OW_I18N */

/* XView functions */
Pkg_private int gauge_init();
Pkg_private Xv_opaque gauge_set_avlist();
Pkg_private Xv_opaque gauge_get_attr();
Pkg_private int gauge_destroy();

/* Panel Item Operations */
static void     gauge_paint();
static void	gauge_layout();

/* Local functions */
static void     paint_gauge();
static void     update_rects();

static Panel_ops ops = {
    panel_default_handle_event,		/* handle_event() */
    NULL,				/* begin_preview() */
    NULL,				/* update_preview() */
    NULL,				/* cancel_preview() */
    NULL,				/* accept_preview() */
    NULL,				/* accept_menu() */
    NULL,				/* accept_key() */
    panel_default_clear_item,		/* clear() */
    gauge_paint,			/* paint() */
    NULL,				/* resize() */
    NULL,				/* remove() */
    NULL,				/* restore() */
    gauge_layout,			/* layout() */
    NULL,				/* accept_kbd_focus() */
    NULL,				/* yield_kbd_focus() */
    NULL				/* extension: reserved for future use */
};

typedef struct {	/* data for a gauge */
    Panel_item      public_self;/* back pointer to object */
    int             actual;	/* # of pixels the "fluid" is to the right of
				 * or below the start of the gauge
				 * ("internal"). */
    Rect            gaugerect;	/* rect containing gauge */
    Rect            max_range_rect;  /* maximum range rect */
    CHAR	   *max_tick_string;
    int		    max_tick_string_width;  /* in pixels */
    int             max_value;
    Rect            min_range_rect;  /* minimum range rect */
    CHAR	   *min_tick_string;
    int		    min_tick_string_width;  /* in pixels */
    int             min_value;
    int		    nticks;	/* nbr of tick marks on gauge */
    Rect	    tickrect;	/* rect containing tick marks */
    int		    value;	/* value of gauge in client units
				 * ("external") */
    int             width;

    /* flags */
    unsigned int showrange:1;
    unsigned int vertical:1;
} Gauge_info;

static int
  etoi(Gauge_info *dp, int value);

/* ========================================================================= */

/* -------------------- XView Functions  -------------------- */
/*ARGSUSED*/
Pkg_private int
gauge_init(panel_public, item_public, avlist)
    Panel           panel_public;
    Panel_item      item_public;
    Attr_avlist     avlist;
{
    Panel_info     *panel = PANEL_PRIVATE(panel_public);
    register Item_info *ip = ITEM_PRIVATE(item_public);
    Xv_panel_gauge *item_object = (Xv_panel_gauge *) item_public;
    register Gauge_info *dp;

    dp = xv_alloc(Gauge_info);

    /* link to object */
    item_object->private_data = (Xv_opaque) dp;
    dp->public_self = item_public;

    ip->ops = ops;
    if (panel->event_proc)
	ip->ops.panel_op_handle_event = (void (*) ()) panel->event_proc;
    ip->item_type = PANEL_GAUGE_ITEM;
    panel_set_bold_label_font(ip);

    /* Initialize non-zero dp fields */
    dp->showrange = TRUE;
    dp->width = 100;
    dp->max_value = 100;

    return XV_OK;
}


Pkg_private     Xv_opaque
gauge_set_avlist(item_public, avlist)
    Panel_item      item_public;
    register Attr_avlist avlist;
{
    register Item_info *ip = ITEM_PRIVATE(item_public);
    register Gauge_info *dp = GAUGE_PRIVATE(item_public);
    register Attr_attribute attr;
    int		    adjust_values = FALSE;
    int		    end_create = FALSE;
    Xv_opaque	    result;
    struct pr_size  size;
    int		    size_changed = FALSE;
    int		    ticks_set = FALSE;

    /* if a client has called panel_item_parent this item may not */
    /* have a parent -- do nothing in this case */
    if (ip->panel == NULL) {
	return ((Xv_opaque) XV_ERROR);
    }

    if (*avlist != XV_END_CREATE) {
	/* Parse Panel Item Generic attributes before Gauge attributes.
	 * Prevent panel_redisplay_item from being called in item_set_avlist.
	 */
	ip->panel->no_redisplay_item = TRUE;
	result = xv_super_set_avlist(item_public, &xv_panel_gauge_pkg, avlist);
	ip->panel->no_redisplay_item = FALSE;
	if (result != XV_OK)
	    return result;
    }

    while (attr = *avlist++) {
	switch (attr) {
	  case PANEL_VALUE:
	    dp->value = (int) *avlist++;
	    adjust_values = TRUE;
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
	    dp->min_value = (int) *avlist++;
	    adjust_values = TRUE;
	    size_changed = TRUE;
	    break;

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
	    dp->max_value = (int) *avlist++;
	    adjust_values = TRUE;
	    size_changed = TRUE;
	    break;

	  case PANEL_DIRECTION:
	    if (*avlist++ == PANEL_VERTICAL)
		dp->vertical = TRUE;
	    else
		dp->vertical = FALSE;
	    size_changed = TRUE;
	    break;
		
	  case PANEL_SHOW_RANGE:
	    if ((int) *avlist++)
		dp->showrange = TRUE;
	    else
		dp->showrange = FALSE;
	    size_changed = TRUE;
	    break;

	  case PANEL_TICKS:
	    dp->nticks = (int) *avlist++;
	    if (dp->nticks == 1)
		dp->nticks = 2;
	    size_changed = TRUE;
	    ticks_set = TRUE;
	    break;

	  case PANEL_GAUGE_WIDTH:
	    dp->width = (int) *avlist++;
	    adjust_values = TRUE;
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
	if (dp->nticks > dp->width / (TICK_THICKNESS+MIN_TICK_GAP))
	    dp->nticks = dp->width / (TICK_THICKNESS+MIN_TICK_GAP);
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
    }
    if (dp->max_tick_string) {
#ifdef OW_I18N
        size = xv_pf_textwidth_wc(wslen(dp->max_tick_string),
            ip->panel->std_font, dp->max_tick_string);
#else
        size = xv_pf_textwidth(strlen(dp->max_tick_string),
            ip->panel->std_font, dp->max_tick_string);
#endif /* OW_I18N */
        dp->max_tick_string_width = size.x;
    }
    dp->width = MAX(dp->width,
		    dp->min_tick_string_width + MIN_TICK_STRING_GAP +
		    dp->max_tick_string_width);
    /* Set external (client unit) and internal (pixel) values */
    if (adjust_values) {
	if (dp->value < dp->min_value)
	    dp->value = dp->min_value;
	else if (dp->value > dp->max_value)
	    dp->value = dp->max_value;
	dp->actual = etoi(dp, dp->value);
    }
    if (size_changed || end_create)
	update_rects(ip);
    if (end_create)
	panel_check_item_layout(ip);

    return XV_OK;
}


Pkg_private     Xv_opaque
gauge_get_attr(item_public, status, which_attr, valist)	/*ARGSUSED*/
    Panel_item      item_public;
    int            *status;
    Attr_attribute  which_attr;
    va_list         valist;
{
    register Gauge_info *dp = GAUGE_PRIVATE(item_public);
#ifdef OW_I18N
    char		*string_mbs = NULL;
#endif /* OW_I18N */

    switch (which_attr) {
      case PANEL_VALUE:
	return (Xv_opaque) dp->value;

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

      case PANEL_DIRECTION:
	return (Xv_opaque) (dp->vertical ? PANEL_VERTICAL : PANEL_HORIZONTAL);

      case PANEL_SHOW_RANGE:
	return (Xv_opaque) (dp->showrange ? TRUE : FALSE);

      case PANEL_TICKS:
	return (Xv_opaque) dp->nticks;

      case PANEL_GAUGE_WIDTH:
	return (Xv_opaque) dp->width;

      default:
	*status = XV_ERROR;
	return (Xv_opaque) 0;
    }
}


Pkg_private int
gauge_destroy(item_public, status)
    Panel_item      item_public;
    Destroy_status  status;
{
    Gauge_info    *dp = GAUGE_PRIVATE(item_public);

    if ((status == DESTROY_CHECKING) || (status == DESTROY_SAVE_YOURSELF))
	return XV_OK;

    if (dp->min_tick_string)
	xv_free(dp->min_tick_string);
    if (dp->max_tick_string)
	xv_free(dp->max_tick_string);

    free((char *) dp);

    return XV_OK;
}



/* --------------------  Panel Item Operations  -------------------- */
static void
gauge_paint(item_public)
    Panel_item	    item_public;
{
    CHAR            buf[10];
    Gauge_info	   *dp = GAUGE_PRIVATE(item_public);
    GC             *gc_list;	/* Graphics Context list */
    Xv_Drawable_info *info;
    Item_info      *ip = ITEM_PRIVATE(item_public);
    Xv_Window       pw;		/* Paint Window */
    Rect           *r;
    CHAR	   *str;

    r = &(ip->label_rect);

    /* paint the label */
    panel_paint_image(ip->panel, &ip->label, r, inactive(ip), ip->color_index);

    /* paint the range */
    if (dp->showrange) {
	if (dp->min_tick_string)
	    str = dp->min_tick_string;
	else {
	    SPRINTF(buf, "%d", dp->min_value);
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
		gc_list = (GC *)xv_get(xv_screen(info),  SCREEN_OLGC_LIST, pw);
		screen_adjust_gc_color(pw, SCREEN_INACTIVE_GC);
		XFillRectangle(xv_display(info), xv_xid(info),
			       gc_list[SCREEN_INACTIVE_GC],
			       r->r_left, r->r_top, r->r_width, r->r_height);
	    }
	PANEL_END_EACH_PAINT_WINDOW

	if (dp->max_tick_string)
	    str = dp->max_tick_string;
	else {
	    SPRINTF(buf, "%d", dp->max_value);
	    str = buf;
	}
	PANEL_EACH_PAINT_WINDOW(ip->panel, pw)
	    r = &dp->max_range_rect;
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
		gc_list = (GC *)xv_get(xv_screen(info),  SCREEN_OLGC_LIST, pw);
		screen_adjust_gc_color(pw, SCREEN_INACTIVE_GC);
		XFillRectangle(xv_display(info), xv_xid(info),
			       gc_list[SCREEN_INACTIVE_GC],
			       r->r_left, r->r_top, r->r_width, r->r_height);
	    }
	PANEL_END_EACH_PAINT_WINDOW
    }

    /* paint the gauge */
    paint_gauge(ip);
}


static void
gauge_layout(item_public, deltas)
    Panel_item	    item_public;
    Rect           *deltas;
{
    Gauge_info	   *dp = GAUGE_PRIVATE(item_public);
    Item_info      *ip = ITEM_PRIVATE(item_public);

    if (!created(ip))
	return;
    dp->gaugerect.r_left += deltas->r_left;
    dp->gaugerect.r_top += deltas->r_top;
    dp->tickrect.r_left += deltas->r_left;
    dp->tickrect.r_top += deltas->r_top;
    dp->min_range_rect.r_left += deltas->r_left;
    dp->min_range_rect.r_top += deltas->r_top;
    dp->max_range_rect.r_left += deltas->r_left;
    dp->max_range_rect.r_top += deltas->r_top;
}



/* --------------------  Local Routines  -------------------- */

/*
 * Convert external value (client units) to internal value (pixels).
 */
static int
  etoi(Gauge_info *dp, int value)
{
    if (value <= dp->min_value)
	return (0);

    if (value >= dp->max_value)
	return (dp->width);

    return (panel_round(((value - dp->min_value) * dp->width),
		        (dp->max_value - dp->min_value + 1)));
}


static void
paint_gauge(ip)
    Item_info      *ip;
{
    register Gauge_info    *dp = GAUGE_FROM_ITEM(ip);
    GC             *gc_list;	/* Graphics Context list */
    Graphics_info  *ginfo = ip->panel->ginfo;
    int		    height;
    Xv_Drawable_info *info;
    int		    limit;
    int		    olgx_state;
    int		    pixel_value;
    Xv_Window       pw;
    int		    save_black;
    int		    tick;
    int		    tick_gap;
    int		    width;
    int		    x;
    Drawable	    xid;
    int		    y;

    olgx_state = dp->vertical ? OLGX_VERTICAL : OLGX_HORIZONTAL;
    if (inactive(ip))
	olgx_state |= OLGX_INACTIVE;
    if (ip->color_index >= 0)
        save_black = olgx_get_single_color(ginfo, OLGX_BLACK);

    PANEL_EACH_PAINT_WINDOW(ip->panel, pw)
	DRAWABLE_INFO_MACRO(pw, info);
	xid = xv_xid(info);
        if (ip->color_index >= 0) {
            olgx_set_single_color(ginfo, OLGX_BLACK,
                                  xv_get(xv_cms(info), CMS_PIXEL,
                                  ip->color_index), OLGX_SPECIAL);
        }
	pixel_value = Gauge_EndCapOffset(ginfo) + dp->actual;
	olgx_draw_gauge(ginfo, xid,
			dp->gaugerect.r_left, dp->gaugerect.r_top,
			dp->width + 2*Gauge_EndCapOffset(ginfo),
			pixel_value, pixel_value, olgx_state);
	if (dp->nticks) {
	    x = dp->tickrect.r_left;
	    y = dp->tickrect.r_top;
	    if (dp->vertical) {
                if (dp->nticks == 1)
                    tick_gap == dp->tickrect.r_height;
                else
                    tick_gap = dp->tickrect.r_height / (dp->nticks-1);
		width = dp->tickrect.r_width;
		height = TICK_THICKNESS;
		limit = dp->tickrect.r_top+dp->tickrect.r_height-TICK_THICKNESS;
	    } else {
                if (dp->nticks == 1)
                    tick_gap == dp->tickrect.r_width;
                else
                    tick_gap = dp->tickrect.r_width / (dp->nticks-1);
		width = TICK_THICKNESS;
		height = dp->tickrect.r_height;
		limit = dp->tickrect.r_left+dp->tickrect.r_width-TICK_THICKNESS;
	    }
	    for (tick = 1; tick <= dp->nticks; tick++) {
		olgx_draw_box(ginfo, xid,
			      x, y, width, height, OLGX_NORMAL, FALSE);
		if (dp->vertical) {
		    y += tick_gap;
		    if (y > limit)
			y = limit; 
		} else {
		    x += tick_gap;
		    if (x > limit)
			x = limit;
		}
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
	    }
	}
    PANEL_END_EACH_PAINT_WINDOW
 
    if (ip->color_index >= 0)
        olgx_set_single_color(ginfo, OLGX_BLACK, save_black, OLGX_SPECIAL);
}


static void
update_rects(ip)
    register Item_info *ip;
{
    register Gauge_info *dp = GAUGE_FROM_ITEM(ip);
    Graphics_info  *ginfo = ip->panel->ginfo;
    CHAR            min_buf[16];
    CHAR            max_buf[16];
    int		    min_range_width;	/* pixels */
    int		    max_range_width;	/* pixels */
    struct pr_size  size;

    if (dp->min_tick_string_width)
	min_range_width = dp->min_tick_string_width;
    else {
	SPRINTF(min_buf, "%d", dp->min_value);
#ifdef OW_I18N
	size = xv_pf_textwidth_wc(wslen(min_buf), ip->panel->std_font, min_buf);
#else
	size = xv_pf_textwidth(strlen(min_buf), ip->panel->std_font, min_buf);
#endif /* OW_I18N */
	min_range_width = size.x;
    }
    if (dp->max_tick_string_width)
	max_range_width = dp->max_tick_string_width;
    else {
	SPRINTF(max_buf, "%d", dp->max_value);
#ifdef OW_I18N
	size = xv_pf_textwidth_wc(wslen(max_buf), ip->panel->std_font, max_buf);
#else
	size = xv_pf_textwidth(strlen(max_buf), ip->panel->std_font, max_buf);
#endif /* OW_I18N */
	max_range_width = size.x;
    }

    dp->gaugerect.r_left = ip->value_rect.r_left;
    dp->gaugerect.r_top = ip->value_rect.r_top;
    if (dp->vertical) {
	/* Create the gauge rect */
	dp->gaugerect.r_width = Gauge_EndCapHeight(ginfo);
	dp->gaugerect.r_height = dp->width + 2*Gauge_EndCapOffset(ginfo);

	/* Create the tick rect */
	dp->tickrect.r_top = dp->gaugerect.r_top + Gauge_EndCapOffset(ginfo);
	dp->tickrect.r_left = dp->gaugerect.r_left + dp->gaugerect.r_width;
	if (dp->nticks)
	    dp->tickrect.r_width = 
		dp->gaugerect.r_width/2 - 1;
	else
	    dp->tickrect.r_width = 0;
	dp->tickrect.r_height = dp->width;

	/* Create the maximum range rect */
	if (dp->showrange) {
	    dp->max_range_rect.r_height = 
		xv_get(ip->panel->std_font, FONT_DEFAULT_CHAR_HEIGHT);
	    dp->max_range_rect.r_width = max_range_width;
	} else {
	    dp->max_range_rect.r_height = 0;
	    dp->max_range_rect.r_width = 0;
	}
	dp->max_range_rect.r_top = dp->tickrect.r_top - dp->max_range_rect.r_height/2;
	dp->max_range_rect.r_left = dp->tickrect.r_left + dp->tickrect.r_width;

	/* Create the minimum range rect */
	rect_construct(&dp->min_range_rect,
	    dp->max_range_rect.r_left,
	    dp->tickrect.r_top + dp->tickrect.r_height -
	        dp->max_range_rect.r_height/2,
	    dp->showrange ? min_range_width : 0,
	    dp->max_range_rect.r_height);

	/* Update the item value rect */
	ip->value_rect.r_width =
	    dp->gaugerect.r_width + dp->tickrect.r_width +
	    MAX(dp->min_range_rect.r_width, dp->max_range_rect.r_width);
	ip->value_rect.r_height = dp->gaugerect.r_height;

    } else {  /*** Horizontal ***/
	/* Create the gauge rect */
	if (dp->showrange)
	    dp->gaugerect.r_left += MAX(min_range_width/2 -
					Gauge_EndCapOffset(ginfo), 0);
	dp->gaugerect.r_width = dp->width + 2*Gauge_EndCapOffset(ginfo);
	dp->gaugerect.r_height = Gauge_EndCapHeight(ginfo);

	/* Create the tick rect */
	dp->tickrect.r_top = dp->gaugerect.r_top + dp->gaugerect.r_height;
	dp->tickrect.r_left = dp->gaugerect.r_left + Gauge_EndCapOffset(ginfo);
	dp->tickrect.r_width = dp->width;
	if (dp->nticks)
	    dp->tickrect.r_height = 
		dp->gaugerect.r_height/2 - 1;
	else
	    dp->tickrect.r_height = 0;

	/* Create the minimum range rect */
	dp->min_range_rect.r_top = dp->tickrect.r_top + dp->tickrect.r_height;
	dp->min_range_rect.r_left = dp->tickrect.r_left - min_range_width/2;
	if (dp->showrange) {
	    dp->min_range_rect.r_height 
			= xv_get(ip->panel->std_font, FONT_DEFAULT_CHAR_HEIGHT);
	    dp->min_range_rect.r_width = min_range_width;
	} else {
	    dp->min_range_rect.r_height = 0;
	    dp->min_range_rect.r_width = 0;
	}

	/* Create the maximum range rect */
	rect_construct(&dp->max_range_rect,
	    dp->tickrect.r_left + dp->tickrect.r_width - max_range_width/2,
	    dp->tickrect.r_top + dp->tickrect.r_height,
	    dp->showrange ? max_range_width : 0,
	    dp->min_range_rect.r_height);

	/* Update the item value rect */
	ip->value_rect.r_width = MAX(rect_right(&dp->gaugerect),
				     rect_right(&dp->max_range_rect))
				 - ip->value_rect.r_left + 1;
	ip->value_rect.r_height = dp->gaugerect.r_height +
	    dp->tickrect.r_height + dp->min_range_rect.r_height;
    }

    /* Update the item rect */
    ip->rect = panel_enclosing_rect(&ip->label_rect, &ip->value_rect);
}
