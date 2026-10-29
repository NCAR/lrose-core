#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)item.c 20.56 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#include <xview_private/panel_impl.h>


Pkg_private int item_init();
Pkg_private int item_destroy();
static void     item_free();

#ifdef OW_I18N
extern wchar_t		_xv_null_string_wc[];
#endif /* OW_I18N */

#define         FOREGROUND      -1


/*ARGSUSED*/
Pkg_private int
item_init(parent, item_public, avlist)
    Xv_Window       parent;
    Panel_item      item_public;
    Attr_avlist     avlist;
{
    register Attr_avlist attrs;
    Panel_info     *panel = PANEL_PRIVATE(parent);
    register Item_info *ip;
    Xv_item        *item_object = (Xv_item *) item_public;

    ip = xv_alloc(Item_info);

    /* link to object */
    item_object->private_data = (Xv_opaque) ip;
    ip->public_self = item_public;

    ip->color_index = -1;	/* use foreground color */
    ip->flags = IS_ITEM;
    ip->item_type = PANEL_EXTENSION_ITEM;  /* default item type */
    ip->layout = PANEL_HORIZONTAL;
    ip->next = NULL;
    ip->notify = panel_nullproc;
#ifdef OW_I18N
    /* Default is to allow input method */
    ip->flags |= IC_ACTIVE;
    ip->notify_wc = panel_nullproc;
    ip->flags &= ~WCHAR_NOTIFY;
#endif /* OW_I18N */
    ip->ops.panel_op_handle_event = (void (*) ()) panel->event_proc;
    ip->panel = panel;
    ip->repaint = panel->repaint;	/* default painting behavior */
    ip->value_font = panel->std_font;
#ifdef OW_I18N
    ip->value_fontset_id = panel->std_fontset_id;
#else
    ip->value_font_xid = panel->std_font_xid;
#endif /* OW_I18N */
    ip->value_ginfo = panel->ginfo;
    ip->x_gap = -1;		/* use panel->item_x_offset */
    ip->y_gap = -1;		/* use panel->item_y_offset */

    image_set_type(&ip->label, PIT_STRING);
    image_set_string(&ip->label, (char *) panel_strsave(""));
#ifdef  OW_I18N
    image_set_string_wc(&ip->label, (wchar_t *) 
		panel_strsave_wc(_xv_null_string_wc));
#endif  /* OW_I18N */
    panel_image_set_font(&ip->label, ip->panel->std_font);
    image_set_bold(&ip->label, FALSE);
    image_set_inverted(&ip->label, label_inverted_flag(ip));
    image_set_color(&ip->label, -1);	/* use foreground color */

    /* nothing is painted yet */
    rect_construct(&ip->painted_rect, 0, 0, 0, 0)

    /*
     * Parse xv_create attributes
     */
    for (attrs = avlist; *attrs; attrs = attr_next(attrs)) {
	switch (attrs[0]) {
	  case PANEL_ITEM_X_GAP:
	    ip->x_gap = (int) attrs[1];
	    break;
	  case PANEL_ITEM_Y_GAP:
	    ip->y_gap = (int) attrs[1];
	    break;
	}
    }

    /*
     * Start the item, label and value rects at the default position
     */
    panel_find_default_xy(panel, ip);
    rect_construct(&ip->rect, ip->panel->item_x, ip->panel->item_y, 0, 0);
    ip->label_rect = ip->rect;
    ip->value_rect = ip->rect;

    return XV_OK;
}


Pkg_private int
item_destroy(item_public, status)
    Panel_item      item_public;
    Destroy_status  status;
{
    Item_info      *item = ITEM_PRIVATE(item_public);
    register Panel_info *panel = item->panel;

    if (status != DESTROY_CLEANUP)
	return XV_OK;

    /* Note:  It is the responsibility of the item's destroy routine
     * to move the keyboard focus to the next keyboard focus item
     * if the item has the keyboard focus at the time its destroy
     * routine is called.  The item's destroy routine is called before
     * item_destroy().
     */

    /* If the panel isn't going away, clear the item */
    if (!panel->status.destroying)
    { 
        panel_default_clear_item(item_public);
        if (item->panel->kbd_focus_item == item)
            item->panel->caret_on = FALSE;
    }

    panel_unlink(item, TRUE);	/* unlink is part of a destroy */

    item_free(item);
    return XV_OK;
}


static void
item_free(ip)
    register Item_info *ip;
{

    /* Free the label storage */
    panel_free_image(&ip->label);

    if (ip->menu) {
	/* Free the menu storage */
	xv_set(ip->menu, XV_DECREMENT_REF_COUNT, NULL);
	xv_destroy(ip->menu);
    }

    free((char *) ip);
}
