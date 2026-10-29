#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)p_drop.c 1.22 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1990 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#include <stdlib.h>
#include <xview_private/draw_impl.h>
#include <xview_private/i18n_impl.h>
#include <xview_private/panel_impl.h>
#include <xview_private/pw_impl.h>
#include <xview/server.h>
#include <xview/svrimage.h>

#define DROP_PRIVATE(item) \
	XV_PRIVATE(Drop_info, Xv_panel_drop, item)

/* Item specific definitions */
#define BORDER_WIDTH	2
#define DEFAULT_IMAGE_HEIGHT 16
#define DEFAULT_IMAGE_WIDTH 12
#define GLYPH_MARGIN    1

/* Item status flags */
#define FULL		0x00000001
#define SELECT_DOWN	0x00000002
#define FREE_GLYPH	0x00000008
#define FREE_BUSY_GLYPH	0x00000010

/* XView functions */
Pkg_private int panel_drop_init();
Pkg_private Xv_opaque panel_drop_set_avlist();
Pkg_private Xv_opaque panel_drop_get_attr();
Pkg_private int panel_drop_destroy();

/* Panel Item Operations */
static void	drop_handle_event();
static void	drop_cancel_preview();
static void	drop_paint();
static void	drop_remove();
static void	drop_restore();
static void	drop_layout();

/* Local functions */
static void	drop_paint_value();

/* Local data */
static unsigned short normal_glyph_bits[] = {
    0xFFFF, 0x0000, 0xFFFF, 0x0000,
    0xFFFF, 0x0000, 0xFFFF, 0x0000,
    0xFFFF, 0x0000, 0xFFFF, 0x0000,
    0xFFFF, 0x0000, 0xFFFF, 0x0000
};

static unsigned short busy_glyph_bits[] = {
    0xCA60, 0x0000, 0xCA60, 0x0000,
    0xCA60, 0x0000, 0xCA60, 0x0000,
    0xCA60, 0x0000, 0xCA60, 0x0000,
    0xCA60, 0x0000, 0xCA60, 0x0000
};


/*
 * Panel Operations Vector Table
 */
static Panel_ops ops = {
    drop_handle_event,			/* handle_event() */
    NULL,				/* begin_preview() */
    NULL,				/* update_preview() */
    drop_cancel_preview,		/* cancel_preview() */
    NULL,				/* accept_preview() */
    NULL,				/* accept_menu() */
    NULL,				/* accept_key() */
    panel_default_clear_item,		/* clear() */
    drop_paint,				/* paint() */
    NULL,				/* resize() */
    drop_remove,			/* remove() */
    drop_restore,			/* restore() */
    drop_layout,			/* layout() */
    NULL,				/* accept_kbd_focus() */
    NULL,				/* yield_kbd_focus() */
    NULL				/* extension: reserved for future use */
};

typedef struct drop_info {
    Panel_item      public_self;/* back pointer to object */
    /*
     * Drop private data
     */
    Server_image    busy_glyph;
    Drag_drop	    dnd;	/* Drag and Drop object */
    Drop_site_item  drop_site;  /* Drag and Drop drop site item */
    Server_image    glyph;	/* normal glyph */
    Selection_requestor sel_req; /* Selection requestor object */
    int		    select_down_x; /* x coordinate of SELECT-down event */
    int		    select_down_y; /* y coordinate of SELECT-down event */
    unsigned long   status;
    int		    del_move; /* boolean for delete on drag move */
    Panel_drop_dnd_type       dndtype;
} Drop_info;



/* ========================================================================= */

/* -------------------- XView Functions  -------------------- */
/*ARGSUSED*/
Pkg_private int
panel_drop_init(panel_public, item_public, avlist)
    Panel           panel_public;
    Panel_item      item_public;
    Attr_avlist     avlist;
{
    Drop_info      *dp;
    Item_info	   *ip = ITEM_PRIVATE(item_public);
    Xv_panel_drop  *item_object = (Xv_panel_drop *) item_public;
    Panel_info	   *panel = PANEL_PRIVATE(panel_public);

    dp = xv_alloc(Drop_info);

    item_object->private_data = (Xv_opaque) dp;
    dp->public_self = item_public;

    ip->ops = ops;
    if (panel->event_proc)
	ip->ops.panel_op_handle_event = (void (*) ()) panel->event_proc;
    ip->item_type = PANEL_DROP_TARGET_ITEM;
    ip->value_rect.r_height = DEFAULT_IMAGE_HEIGHT + 2*GLYPH_MARGIN +
	2*BORDER_WIDTH;
    ip->value_rect.r_width = DEFAULT_IMAGE_WIDTH + 2*GLYPH_MARGIN +
	2*BORDER_WIDTH;

    /*
     * Initialize non-zero private data
     */
    dp->drop_site = xv_create(panel_public, DROP_SITE_ITEM,
	DROP_SITE_EVENT_MASK, DND_ENTERLEAVE,
	NULL);
    dp->sel_req = xv_create(panel_public, SELECTION_REQUESTOR, NULL);
    dp->del_move = TRUE;

    return XV_OK;
}


Pkg_private Xv_opaque
panel_drop_set_avlist(item_public, avlist)
    Panel_item	    item_public;
    Attr_avlist	    avlist;
{
    Drop_info	   *dp = DROP_PRIVATE(item_public);
    Item_info	   *ip = ITEM_PRIVATE(item_public);
    int		    full_set = FALSE;
    int		    full_value;
    Xv_opaque       result;
    Xv_Screen	    screen;
    Xv_Server	    server;

    /* Parse Panel Item Generic attributes before Text Field attributes.
     * Prevent panel_redisplay_item from being called in item_set_avlist.
     */
    if (*avlist != XV_END_CREATE) {
	ip->panel->no_redisplay_item = TRUE;
	result = xv_super_set_avlist(item_public, &xv_panel_text_pkg, avlist);
	ip->panel->no_redisplay_item = FALSE;
	if (result != XV_OK)
	    return result;
    }

    /* Parse Attribute-Value List.  Complete initialization upon
     * receipt of XV_END_CREATE.
     */
    for ( ; *avlist; avlist = attr_next(avlist)) {
        switch ((int) avlist[0]) {
	  case PANEL_DROP_BUSY_GLYPH:
	    if (avlist[1] && PR_NOT_SERVER_IMAGE(avlist[1])) {
		xv_error(avlist[1],
			 ERROR_STRING, 
			    XV_MSG("Invalid Server Image specified"),
			 ERROR_PKG, PANEL,
			 NULL);
		break;
	    }
	    if (dp->busy_glyph && dp->status & FREE_BUSY_GLYPH) {
		xv_set(dp->busy_glyph, XV_DECREMENT_REF_COUNT, NULL);
		xv_destroy(dp->busy_glyph);
		dp->status &= ~FREE_BUSY_GLYPH;
	    }
	    dp->busy_glyph = (Server_image) avlist[1];
	    break;
	  case PANEL_DROP_DND:
	    dp->dnd = (Drag_drop) avlist[1];
	    break;
	  case PANEL_DROP_FULL:
	    full_value = (int) avlist[1];
	    full_set = TRUE;
	    break;
	  case PANEL_DROP_DELETE:
	    dp->del_move = (int) avlist[1];
	    break;
	  case PANEL_DROP_DND_TYPE:
	    switch( (int) avlist[1] ) {
	        case PANEL_DROP_USERDEF:
                case PANEL_DROP_COPY_ONLY:
                case PANEL_DROP_MOVE_ONLY:
                  dp->dndtype = (int) avlist[1];
                  break;
                default:
                  xv_error(avlist[1],
                           ERROR_STRING,
                              XV_MSG("Invalid value for PANEL_DROP_DND_TYPE"),
                           ERROR_PKG, PANEL,
                           NULL);
                  break;
            }
	    break;
	  case PANEL_DROP_GLYPH:
	    if (avlist[1] && PR_NOT_SERVER_IMAGE(avlist[1])) {
		xv_error(avlist[1],
			 ERROR_STRING, 
			    XV_MSG("Invalid Server Image specified"),
			 ERROR_PKG, PANEL,
			 NULL);
		break;
	    }
	    if (dp->glyph && dp->status & FREE_GLYPH) {
		xv_set(dp->glyph, XV_DECREMENT_REF_COUNT, NULL);
		xv_destroy(dp->glyph);
		dp->status &= ~FREE_GLYPH;
	    }
	    dp->glyph = (Server_image) avlist[1];
	    break;
	  case PANEL_DROP_HEIGHT:
	    ip->value_rect.r_height = (int) avlist[1] + 2*GLYPH_MARGIN +
		2*BORDER_WIDTH;
	    break;
	  case PANEL_DROP_SITE_DEFAULT:
	    xv_set(dp->drop_site, DROP_SITE_DEFAULT, avlist[1], NULL);
	    ip->panel->default_drop_site_item = ip;
	    break;
	  case PANEL_DROP_WIDTH:
	    ip->value_rect.r_width = (int) avlist[1] + 2*GLYPH_MARGIN +
		2*BORDER_WIDTH;
	    break;
	  case XV_END_CREATE:
	    screen = xv_get(PANEL_PUBLIC(ip->panel), XV_SCREEN);
	    server = xv_get(screen, SCREEN_SERVER);
	    if (!dp->glyph) {
		dp->glyph = xv_get(server, XV_KEY_DATA, PANEL_DROP_GLYPH);
		if (!dp->glyph) {
		    dp->glyph = xv_create(screen, SERVER_IMAGE,
			XV_WIDTH, DEFAULT_IMAGE_WIDTH,
			XV_HEIGHT, DEFAULT_IMAGE_HEIGHT,
			SERVER_IMAGE_DEPTH, 1,
			SERVER_IMAGE_BITS, normal_glyph_bits,
			NULL);
		    xv_set(server,
			   XV_KEY_DATA, PANEL_DROP_GLYPH, dp->glyph,
			   NULL);
		}
		xv_set(dp->glyph, XV_INCREMENT_REF_COUNT, NULL);
		dp->status |= FREE_GLYPH;
	    }
	    if (!dp->busy_glyph) {
		dp->busy_glyph = xv_get(server,
					XV_KEY_DATA, PANEL_DROP_BUSY_GLYPH);
		if (!dp->busy_glyph) {
		    dp->busy_glyph = xv_create(screen, SERVER_IMAGE,
			XV_WIDTH, DEFAULT_IMAGE_WIDTH,
			XV_HEIGHT, DEFAULT_IMAGE_HEIGHT,
			SERVER_IMAGE_DEPTH, 1,
			SERVER_IMAGE_BITS, busy_glyph_bits,
			NULL);
		    xv_set(server,
			   XV_KEY_DATA, PANEL_DROP_BUSY_GLYPH, dp->busy_glyph,
			   NULL);
		}
		xv_set(dp->glyph, XV_INCREMENT_REF_COUNT, NULL);
		dp->status |= FREE_BUSY_GLYPH;
	    }
	    break;
	  default:
	    break;
	}
    }

    if (full_set) {
	if (full_value && dp->dnd)
	    dp->status |= FULL;
	else
	    dp->status &= ~FULL;
    }

    if (dp->glyph) {
	ip->value_rect.r_height = MAX(ip->value_rect.r_height, 
	    (int) xv_get(dp->glyph, XV_HEIGHT) + 2*GLYPH_MARGIN +
	    2*BORDER_WIDTH);
	ip->value_rect.r_width = MAX(ip->value_rect.r_width,
	    (int) xv_get(dp->glyph, XV_WIDTH) + 2*GLYPH_MARGIN +
	    2*BORDER_WIDTH);
    }
    if (dp->busy_glyph) {
	ip->value_rect.r_height = MAX(ip->value_rect.r_height,
	    (int) xv_get(dp->busy_glyph, XV_HEIGHT) + 2*GLYPH_MARGIN +
	    2*BORDER_WIDTH);
	ip->value_rect.r_width = MAX(ip->value_rect.r_width,
	    (int) xv_get(dp->busy_glyph, XV_WIDTH) + 2*GLYPH_MARGIN +
	    2*BORDER_WIDTH);
    }

    xv_set(dp->drop_site,
	   DROP_SITE_DELETE_REGION, NULL,  /* throw away previous region ... */
	   DROP_SITE_REGION, &ip->value_rect, /* ... use this one instead */
	   NULL);

    return XV_OK;	/* return XV_ERROR if something went wrong... */
}


/*ARGSUSED*/
Pkg_private Xv_opaque
panel_drop_get_attr(item_public, status, which_attr, avlist)
    Panel_item      item_public;
    int            *status;	/* set to XV_ERROR if something goes wrong */
    register Attr_attribute which_attr;
    va_list         avlist;
{
    Drop_info	   *dp = DROP_PRIVATE(item_public);
    Item_info	   *ip = ITEM_PRIVATE(item_public);

    switch (which_attr) {
      case PANEL_DROP_BUSY_GLYPH:
	return (Xv_opaque) dp->busy_glyph;
      case PANEL_DROP_DND:
	return (Xv_opaque) dp->dnd;
      case PANEL_DROP_FULL:
	return (Xv_opaque) (dp->status & FULL) ? TRUE : FALSE;
      case PANEL_DROP_DELETE:
	return (Xv_opaque) dp->del_move;
      case PANEL_DROP_DND_TYPE:
	return (Xv_opaque) dp->dndtype;
      case PANEL_DROP_GLYPH:
	return (Xv_opaque) dp->glyph;
      case PANEL_DROP_HEIGHT:
	return (Xv_opaque) ip->value_rect.r_height -
	    2*(GLYPH_MARGIN + BORDER_WIDTH);
      case PANEL_DROP_SEL_REQ:
	return (Xv_opaque) dp->sel_req;
      case PANEL_DROP_SITE_DEFAULT:
	return xv_get(dp->drop_site, DROP_SITE_DEFAULT);
      case PANEL_DROP_WIDTH:
	return (Xv_opaque) ip->value_rect.r_width -
	    2*(GLYPH_MARGIN + BORDER_WIDTH);
      default:
	*status = XV_ERROR;
	return (Xv_opaque) 0;
    }
}
      

Pkg_private int
panel_drop_destroy(item, status)
    Panel_item      item;
    Destroy_status  status;
{
    Drop_info	   *dp = DROP_PRIVATE(item);

    if ((status == DESTROY_CHECKING) || (status == DESTROY_SAVE_YOURSELF))
	return XV_OK;
    if (dp->glyph && dp->status & FREE_GLYPH) {
	xv_set(dp->glyph, XV_DECREMENT_REF_COUNT, NULL);
	xv_destroy(dp->glyph);
    }
    if (dp->busy_glyph && dp->status & FREE_BUSY_GLYPH) {
	xv_set(dp->busy_glyph, XV_DECREMENT_REF_COUNT, NULL);
	xv_destroy(dp->busy_glyph);
    }
    if (dp->dnd)
	xv_destroy(dp->dnd);
    xv_destroy(dp->drop_site);
    xv_destroy(dp->sel_req);
    free(dp);
    return XV_OK;
}



/* --------------------  Panel Item Operations  -------------------- */
static void
drop_handle_event(item_public, event)
    Panel_item	    item_public;
    Event	   *event;
{
    Drop_info	   *dp = DROP_PRIVATE(item_public);
    int		    format;
    Item_info	   *ip = ITEM_PRIVATE(item_public);
    long	    length;
    int		    status;

    switch (event_action(event)) {
      case ACTION_SELECT:
	if (event_is_down(event)) {
	    dp->status |= SELECT_DOWN;
	    dp->select_down_x = event_x(event);
	    dp->select_down_y = event_y(event);
	} else
	    /* Note: we'll only see this if the user didn't start a
	     * drag and drop operation.  Otherwise, the SELECT-up is
	     * consumed by the Drag and Drop Package.
	     */
	    dp->status &= ~SELECT_DOWN;
	return;
      case LOC_DRAG:
	/* Ignore drag if SELECT wasn't depressed over Drop Target,
	 * or this is not a full Drop Target (i.e., no valid data to drag).
	 * Don't initiate drag and drop until the mouse is dragged at
	 * least panel->drag_threshold pixels.
	 */
	if ((dp->status & (SELECT_DOWN | FULL)) != (SELECT_DOWN | FULL) ||
	    (abs(event_x(event) - dp->select_down_x) <
		 ip->panel->drag_threshold &&
	     abs(event_y(event) - dp->select_down_y) <
		 ip->panel->drag_threshold))
	    return;
	ip->flags |= BUSY;
	ip->flags &= ~BUSY_MODIFIED;
	drop_paint_value(ip, dp);

	switch( dp->dndtype ) {
	    case PANEL_DROP_USERDEF:
	        xv_set(dp->dnd,
	               DND_TYPE, panel_duplicate_key_is_down(ip->panel, event) ?
		           DND_COPY : DND_MOVE,
	               NULL);
	        break;
	    case PANEL_DROP_COPY_ONLY:
	        xv_set(dp->dnd, DND_TYPE, DND_COPY, NULL);
	        break;
	    case PANEL_DROP_MOVE_ONLY:
	        xv_set(dp->dnd, DND_TYPE, DND_MOVE, NULL);
	        break;
	}

	(void) (*ip->notify)(item_public, dnd_send_drop(dp->dnd), event);
	dp->status &= ~SELECT_DOWN;
	if (!busy_modified(ip))
	    ip->flags &= ~BUSY;
	else
	    ip->flags &= ~BUSY_MODIFIED;
	drop_paint_value(ip, dp);
	return;
      case ACTION_DRAG_PREVIEW:
	switch (event_id(event)) {
	  case LOC_WINENTER:
	    if (!busy(ip)) {
		ip->flags |= BUSY;
		drop_paint_value(ip, dp);
	    }
	    return;
	  case LOC_WINEXIT:
	    if (busy(ip)) {
		ip->flags &= ~BUSY;
		drop_paint_value(ip, dp);
	    }
	    return;
	}
	return;
      case ACTION_DRAG_COPY:
      case ACTION_DRAG_MOVE:
	ip->flags &= ~BUSY_MODIFIED;
	status = (*ip->notify)(item_public,
			       dnd_decode_drop(dp->sel_req, event),
			       event);
	if (status == XV_OK) {
	    if ((event_action(event) == ACTION_DRAG_MOVE) && dp->del_move) {
		/* Post delete request back to owner */
		xv_set(dp->sel_req, SEL_TYPE, ip->panel->atom.delete, NULL);
		(void) xv_get(dp->sel_req, SEL_DATA, &length, &format);
	    }
	    dnd_done(dp->sel_req);
	}
	if (!busy_modified(ip))
	    ip->flags &= ~BUSY;
	else
	    ip->flags &= ~BUSY_MODIFIED;
	drop_paint_value(ip, dp);
	return;
      default:
	panel_default_handle_event(item_public, event);
	return;
    }
}


static void
drop_cancel_preview(item_public, event)
    Panel_item	    item_public;
    Event	   *event;
{
    Drop_info	   *dp = DROP_PRIVATE(item_public);
    Item_info	   *ip = ITEM_PRIVATE(item_public);

    if (event_id(event) == LOC_WINEXIT && busy(ip)) {
	ip->flags &= ~BUSY;
	drop_paint_value(ip, dp);
    }
}


static void
drop_paint(item_public)
    Panel_item      item_public;
{
    Drop_info	   *dp = DROP_PRIVATE(item_public);
    Item_info	   *ip = ITEM_PRIVATE(item_public);

    /* Paint the label */
    panel_paint_label(item_public);

    /* Paint the value */
    drop_paint_value(ip, dp);
}


/*ARGSUSED*/
static void
drop_remove(item_public)
    Panel_item	    item_public;
{
    Drop_info	   *dp = DROP_PRIVATE(item_public);

    /*
     * The item has been made hidden via xv_set(item, XV_SHOW, FALSE, avlist).
     * Remove the drop site region.
     */
    xv_set(dp->drop_site,
	   DROP_SITE_DELETE_REGION, NULL,
	   NULL);
}


/*ARGSUSED*/
static void
drop_restore(item_public)
    Panel_item	    item_public;
{
    Drop_info	   *dp = DROP_PRIVATE(item_public);
    Item_info	   *ip = ITEM_PRIVATE(item_public);

    /*
     * The item has been made visible via xv_set(item, XV_SHOW, TRUE, avlist).
     * Restore the drop site region.
     */
    xv_set(dp->drop_site,
	   DROP_SITE_REGION, &ip->value_rect,
	   NULL);
}


/*ARGSUSED*/
static void
drop_layout(item_public, deltas)
    Panel_item	    item_public;
    Rect	   *deltas;
{
    Drop_info	   *dp = DROP_PRIVATE(item_public);
    Item_info	   *ip = ITEM_PRIVATE(item_public);

    /*
     * The item has been moved.  Update the drop site region.
     */
    xv_set(dp->drop_site,
	   DROP_SITE_DELETE_REGION, NULL,  /* throw away previous region ... */
	   DROP_SITE_REGION, &ip->value_rect, /* ... use this one instead */
	   NULL);
}



/* --------------------  Local Routines  -------------------- */

static void
drop_paint_value(ip, dp)
    Item_info	   *ip;
    Drop_info	   *dp;
{
    Server_image    glyph = XV_ZERO;
    Xv_Drawable_info *info;
    Panel_info	   *panel = ip->panel;
    Xv_Window	    pw;

    if (dp->busy_glyph && busy(ip))
	glyph = dp->busy_glyph;
    else if (dp->glyph && !busy(ip) && (dp->status & FULL) == FULL)
	glyph = dp->glyph;
    PANEL_EACH_PAINT_WINDOW(panel, pw)
	DRAWABLE_INFO_MACRO(pw, info);
	panel_clear_pw_rect(pw, ip->value_rect);
	olgx_draw_box(ip->panel->ginfo, xv_xid(info),
		      ip->value_rect.r_left, ip->value_rect.r_top,
		      ip->value_rect.r_width, ip->value_rect.r_height,
		      OLGX_INVOKED, ip->panel->status.three_d);
	if (glyph)
	    panel_paint_svrim(pw, glyph,
		ip->value_rect.r_left + GLYPH_MARGIN + BORDER_WIDTH,
		ip->value_rect.r_top + GLYPH_MARGIN + BORDER_WIDTH,
		ip->color_index, (Pixrect *)NULL);
    PANEL_END_EACH_PAINT_WINDOW
}

