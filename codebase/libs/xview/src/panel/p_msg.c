#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)p_msg.c 20.30 93/06/28 Copyr 1987 Sun Micro";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#include <xview_private/panel_impl.h>

/* XView functions */
Pkg_private int	    panel_message_init();

/* Panel Item Operations */
static void         msg_accept_preview();
static void	    msg_paint();

static Panel_ops ops = {
    panel_default_handle_event,		/* handle_event() */
    NULL,				/* begin_preview() */
    NULL,				/* update_preview() */
    NULL,				/* cancel_preview() */
    msg_accept_preview,			/* accept_preview() */
    NULL,				/* accept_menu() */
    NULL,				/* accept_key() */
    panel_default_clear_item,		/* clear() */
    msg_paint,				/* paint() */
    NULL,				/* resize() */
    NULL,				/* remove() */
    NULL,				/* restore() */
    NULL,				/* layout() */
    NULL,				/* accept_kbd_focus() */
    NULL,				/* yield_kbd_focus() */
    NULL				/* extension: reserved for future use */
};


/* ========================================================================= */

/* -------------------- XView Functions  -------------------- */
/* ARGSUSED */
Pkg_private int
panel_message_init(panel_public, item_public, avlist)
    Panel           panel_public;
    Panel_item      item_public;
    Attr_avlist     avlist;
{
    Panel_info     *panel = PANEL_PRIVATE(panel_public);
    register Item_info *ip = ITEM_PRIVATE(item_public);

    ip->ops = ops;
    if (panel->event_proc)
	ip->ops.panel_op_handle_event = (void (*) ()) panel->event_proc;
    ip->item_type = PANEL_MESSAGE_ITEM;

    return XV_OK;
}


/* --------------------  Panel Item Operations  -------------------- */
static void
msg_accept_preview(item_public, event)
    Panel_item	    item_public;
    Event          *event;
{
    Item_info      *ip = ITEM_PRIVATE(item_public);

    (*ip->notify) (item_public, event);
}


static void
msg_paint(item_public)
    Panel_item	    item_public;
{
    Item_info      *ip = ITEM_PRIVATE(item_public);

    panel_paint_image(ip->panel, &ip->label, &ip->label_rect, inactive(ip),
		      ip->color_index);
}
