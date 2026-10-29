#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)windowdrop.c 1.14 93/06/28";
#endif
#endif

/*
 *      (c) Copyright 1990 Sun Microsystems, Inc. Sun design patents 
 *      pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *      file for terms of the license.
 */

#include <xview_private/windowimpl.h>
#include <xview/server.h>
#include <assert.h>

Pkg_private	void		win_add_drop_item();
Pkg_private	Xv_opaque	win_delete_drop_item();
Xv_private      int		DndStoreSiteData();
Pkg_private	void		win_update_dnd_property();


/* 
 * Save the drop site information.  It is stored in a linked list, where
 * each site is a node in the list.  The head of the node is never used.
 */

Pkg_private void
win_add_drop_item(win, dropItem)
    Window_info 	*win;
    Xv_drop_site	 dropItem;
{
    Win_drop_site_list  *winDropSiteNode;

    if (!win->dropSites) {
				/* Create the head of the list.  Never used. */
        win->dropSites = xv_alloc(Win_drop_site_list);
	XV_SL_INIT(win->dropSites);
    }

				/* Create a new drop site node. */
    winDropSiteNode = xv_alloc(Win_drop_site_list);
    winDropSiteNode->drop_item = dropItem;

			/* Add the new drop site. */
    XV_SL_ADD_AFTER(win->dropSites, win->dropSites, winDropSiteNode);
}

Pkg_private Xv_opaque
win_delete_drop_item(win, dropItem, mode)
    Window_info 	*win;
    Xv_drop_site	 dropItem;
    Win_drop_site_mode   mode;
{
    Win_drop_site_list  *nodePrev,
			*nodeHead,
			*winDropSiteNode;

    /* 
     * when mode equaled Win_Drop_Interest, we use to
     * call win_update_dnd_property() from this routine;
     * however this caused severe performance degradation
     * so call to this function was moved up a level
     */
    if (mode == Win_Drop_Site) {
        if (!win->dropSites) return(XV_ERROR); 
	nodeHead = winDropSiteNode = win->dropSites;
        assert(dropItem != XV_ZERO);
    } else {
        if (!win->dropInterest) return(XV_ERROR); 
	nodeHead = winDropSiteNode = win->dropInterest;
    }

    nodePrev = winDropSiteNode;

    while(winDropSiteNode = (Win_drop_site_list *)
				(XV_SL_SAFE_NEXT(winDropSiteNode))) {
	if (winDropSiteNode->drop_item == dropItem) {
	    xv_free(XV_SL_REMOVE_AFTER(nodeHead, nodePrev));
	    return(XV_OK);
	}
	nodePrev = winDropSiteNode;
    }

    return ((Xv_opaque)XV_ERROR);
}

Pkg_private void
win_add_drop_interest(win, dropItem)
    Window_info 	*win;
    Xv_drop_site	 dropItem;
{
    Win_drop_site_list	*winDropSiteNode;

		                 /* win->dropInterest is a list of drop items.
		 		  */
    if (!win->dropInterest) {
				/* Create the head of the list.  Never used. */
        win->dropInterest = xv_alloc(Win_drop_site_list);
	XV_SL_INIT(win->dropInterest);
    }

    winDropSiteNode = win->dropInterest;

		/* See if this drop site exists in the interest list. */
    while(winDropSiteNode = (Win_drop_site_list *)
				(XV_SL_SAFE_NEXT(winDropSiteNode))) {
	if (winDropSiteNode->drop_item == dropItem)
	    break;
    }

		/* We couldn't find it in the list, so add it. */
    if (winDropSiteNode == (Win_drop_site_list *)XV_SL_NULL) {
	winDropSiteNode = xv_alloc(Win_drop_site_list);
	winDropSiteNode->drop_item = dropItem;
	XV_SL_ADD_AFTER(win->dropInterest, win->dropInterest, winDropSiteNode);
    }

    /* 
     * Interest property used to be updated here (by calling
     * win_update_dnd_property()).  However, this caused 
     * severe performance degradation when there were many
     * panel items.  Thus, a call to win_update_dnd_property()
     * was moved up a level.
     */
}

Pkg_private void
win_update_dnd_property(win)
    Window_info *win;
{
    Win_drop_site_list	*winDropInterest;
    int		 size = 2;                 /* Version # + site count*/
    long 	*data, *head;
    Window	 window = (Window)xv_get(win->public_self, XV_XID);
    Display 	*dpy = XV_DISPLAY_FROM_WINDOW(win->public_self);
    Atom	 DragDropInterest =
				 xv_get(XV_SERVER_FROM_WINDOW(win->public_self),
				        SERVER_ATOM, "_SUN_DRAGDROP_INTEREST");

    if (!win->dropInterest) return;

		/* The head of the list is never used. */
    winDropInterest = ((Win_drop_site_list *)
				    (XV_SL_SAFE_NEXT(win->dropInterest)));

		/* If there are no sites left in the list then 
		 * remove the interest property.
		 */
    if (winDropInterest == (Win_drop_site_list *)XV_SL_NULL) {
	XDeleteProperty(dpy, window, DragDropInterest);
	return;
    }

    winDropInterest = win->dropInterest;
    while(winDropInterest = (Win_drop_site_list *)
				(XV_SL_SAFE_NEXT(winDropInterest))) {
	size += (int)xv_get(winDropInterest->drop_item, DROP_SITE_SIZE);
    }

    head = xv_alloc_n(long, size);
    data = head;

    *data++ = DND_VERSION;
    head[1] = 0;
    *data++;    /* Site Count, will update later as head[1] */

		/* Store the site descriptions for the property. */
    winDropInterest = win->dropInterest;
    while(winDropInterest = (Win_drop_site_list *)
				(XV_SL_SAFE_NEXT(winDropInterest))) {
	head[1] += DndStoreSiteData(winDropInterest->drop_item, &data);
    }

		/* Update the interest property. */
    XChangeProperty(dpy, window, DragDropInterest,
		    DragDropInterest, 32, PropModeReplace,
                    (unsigned char *)head, size);
    free(head);
}

Xv_private Xv_opaque
win_get_top_level(window)
    Xv_Window window;
{
    register Window_info *info = WIN_PRIVATE(window);

                /* Find the top level window. */
    do {
        if (info->top_level)
            break;
        info = WIN_PRIVATE(info->parent);
    } while(info && info->parent);

    if (!info) return((Xv_opaque)XV_ERROR);

    return(info->public_self);
}
