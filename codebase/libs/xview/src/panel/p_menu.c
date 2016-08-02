#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)p_menu.c 20.18 89/07/31";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#include <xview_private/panel_impl.h>
#include <xview/openmenu.h>
#include <xview/svrimage.h>

#define	MARK_XOFFSET	3	/* # of pixels to leave after a menu mark */

extern struct pr_size xv_pf_textwidth();

Pkg_private void
panel_menu_display(ip, event)
    register Item_info *ip;
    register Event *event;
{
    Xv_Window       paint_window = event_window(event);

    if (ip->menu == NULL || paint_window == NULL)
	return;

    menu_show(ip->menu, paint_window, event, NULL);
}


Pkg_private
int
panel_set_menu(ip, avlist)
    register Item_info *ip;
    Attr_avlist     avlist;
{
    register Attr_attribute attr;	/* each attribute */

    while (attr = *avlist++) {
	switch (attr) {
	  case PANEL_SHOW_MENU_MARK:
	    if ((int) *avlist++)
		ip->flags |= SHOW_MENU_MARK;
	    else
		ip->flags &= ~SHOW_MENU_MARK;
	    break;

	  case PANEL_MENU_MARK_IMAGE:
	    ip->menu_mark_on =
		(*avlist ? (Pixrect *) * avlist : &panel_empty_pr);
	    avlist++;
	    ip->menu_mark_width = MARK_XOFFSET +
		MAX(ip->menu_mark_on->pr_width, ip->menu_mark_off->pr_width);
	    ip->menu_mark_height =
		MAX(ip->menu_mark_on->pr_height, ip->menu_mark_off->pr_height);
	    break;

	  case PANEL_MENU_NOMARK_IMAGE:
	    ip->menu_mark_off =
		(*avlist ? (Pixrect *) * avlist : &panel_empty_pr);
	    avlist++;
	    ip->menu_mark_width = MARK_XOFFSET +
		MAX(ip->menu_mark_on->pr_width, ip->menu_mark_off->pr_width);
	    ip->menu_mark_height =
		MAX(ip->menu_mark_on->pr_height, ip->menu_mark_off->pr_height);
	    break;

	  default:
	    /* skip past what we don't care about */
	    avlist = attr_skip(attr, avlist);
	    break;
	}
    }

    return 1;

}


/*
 * add items to a menu from a list of images.
 */
Pkg_private
void
panel_images_to_menu_items(ip, images, last)
    Item_info      *ip;
    Panel_image     images[];
    int             last;
{
    register int    i;		/* counter */
    register Panel_image *image;

    for (i = 0; i <= last; i++) {
	image = &(images[i]);
	switch (image_type(image)) {
	  case IM_STRING:
	    xv_set(ip->menu, MENU_STRING_ITEM, image_string(image), i, NULL);
	    break;

	  case IM_PIXRECT:
	    xv_set(ip->menu, MENU_IMAGE_ITEM, image_pixrect(image), i, NULL);
	    break;
	}
    }
}

