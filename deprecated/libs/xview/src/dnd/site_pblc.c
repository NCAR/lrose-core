#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)site_pblc.c 1.13 93/06/28";
#endif
#endif

/*
 *      (c) Copyright 1990 Sun Microsystems, Inc. Sun design patents
 *      pending in the U.S. and foreign countries. See LEGAL NOTICE
 *      file for terms of the license.
 */

/* SUPPRESS 560 */

#include <X11/Xatom.h>
#include <xview/xview.h>
#include <xview_private/site_impl.h>
#include <xview_private/portable.h>

Xv_private Xv_opaque 	DndDropAreaOps();
Xv_private Xv_Window    win_get_top_level();
extern void 		DndSizeOfSite();

/*ARGSUSED*/
Pkg_private int
dnd_site_init(owner, site_public, avlist)
    Xv_Window		owner;
    Xv_drop_site	site_public;
    Attr_avlist		avlist;
{
    Dnd_site_info		*site = NULL;
    Xv_drop_site_struct		*site_object;

    site = xv_alloc(Dnd_site_info);
    site->public_self = site_public;
    site_object = (Xv_drop_site_struct *)site_public;
    site_object->private_data = (Xv_opaque)site;

    status_reset(site, site_id_set);
    status_reset(site, window_set);
    status_reset(site, created);
#ifdef WINDOW_SITES
    status_set(site, is_window_region);
    status_reset(site, is_window_region);
#else
    status_reset(site, is_window_region);
#endif /* WINDOW_SITES */
    site->owner = owner;
    site->owner_xid = (Window) xv_get(owner, XV_XID);
    site->region.windows = NULL;
    site->region.rects = NULL;
    site->num_regions = 0;
    site->site_size = 0;
    site->event_mask = 0;

    return(XV_OK);
}

Pkg_private Xv_opaque
dnd_site_set_avlist(site_public, avlist)
    Xv_drop_site	site_public;
    Attr_attribute	avlist[];
{
    register Dnd_site_info	*site = DND_SITE_PRIVATE(site_public);
    register Attr_avlist	 attrs;

    for (attrs = avlist; *attrs; attrs = attr_next(attrs)) {
        switch (attrs[0]) {
#ifdef WINDOW_SITES
	  case DROP_SITE_TYPE:
	      if (DND_WINDOW_SITE == (int)attrs[1])
		  status_set(site, is_window_region);
	      else
		  status_reset(site, is_window_region);
	      break;
#endif /* WINDOW_SITES */
	  case DROP_SITE_ID:
	      site->site_id = (long)attrs[1];
	      status_set(site, site_id_set);
	      break;
	  case DROP_SITE_DEFAULT:
	      if ((int)attrs[1])
		  site->event_mask |= DND_DEFAULT_SITE;
	      else
		  site->event_mask ^= DND_DEFAULT_SITE;
	      break;
	  case DROP_SITE_EVENT_MASK:
	      site->event_mask &= DND_DEFAULT_SITE;
	      site->event_mask |= (int)attrs[1];
	      break;
	  case DROP_SITE_REGION:
	      if (status(site, is_window_region))
	          (void) DndDropAreaOps(site, Dnd_Add_Window, attrs[1]); 
	      else
	          (void) DndDropAreaOps(site, Dnd_Add_Rect, attrs[1]); 
	      status_set(site, window_set);
	      break;
	  case DROP_SITE_DELETE_REGION:
	      if (!attrs[1])
	          (void) DndDropAreaOps(site, (status(site, is_window_region) ?
					Dnd_Delete_All_Windows :
					Dnd_Delete_All_Rects),
					attrs[1]); 
	      else
	          (void) DndDropAreaOps(site, (status(site, is_window_region) ?
					Dnd_Delete_Window :
					Dnd_Delete_Rect),
					attrs[1]); 
	      break;
	  case DROP_SITE_REGION_PTR:
	      if (status(site, is_window_region))
	          (void) DndDropAreaOps(site, Dnd_Add_Window_Ptr, attrs[1]); 
	      else
	          (void) DndDropAreaOps(site, Dnd_Add_Rect_Ptr, attrs[1]); 
	      status_set(site, window_set);
	      break;
	  case DROP_SITE_DELETE_REGION_PTR:
	      if (!attrs[1])
	          (void) DndDropAreaOps(site, (status(site, is_window_region) ?
					Dnd_Delete_All_Windows :
					Dnd_Delete_All_Rects),
					attrs[1]); 
	      else
	          (void) DndDropAreaOps(site, (status(site, is_window_region) ?
					Dnd_Delete_Window_Ptr :
					Dnd_Delete_Rect_Ptr),
					attrs[1]); 
	      break;
	  case XV_END_CREATE: {
	      if (!status(site, site_id_set))
	          site->site_id = xv_unique_key(); 
#ifdef WINDOW_SITES
	      if (!status(site, window_set) && status(site, is_window_region))
		  (void) DndDropAreaOps(site, Dnd_Add_Window, site->owner); 
#endif /* WIDNOW_SITES */
	      status_set(site, created);
	      xv_set(site->owner, WIN_ADD_DROP_ITEM, DND_SITE_PUBLIC(site), NULL);
	  }
          break;
          default:
             (void)xv_check_bad_attr(&xv_drop_site_item, attrs[0]);
	  break;
        }
    }

    /* When ever some attribute of the drop site changes, we update the
     * intrest property.
     */
    if (status(site, created))
        (void)DndSizeOfSite(site);
    if (status(site, created) && xv_get(site->owner, XV_SHOW)) {
        xv_set(win_get_top_level(site->owner),
			WIN_ADD_DROP_INTEREST, DND_SITE_PUBLIC(site),
			NULL);
    }

    return ((Xv_opaque)XV_OK);
}

/*ARGSUSED*/
Pkg_private Xv_opaque
dnd_site_get_attr(site_public, error, attr, args)
    Xv_drop_site	 site_public;
    int			*error;
    Attr_attribute	 attr;
    va_list		 args;
{
    Dnd_site_info	*site = DND_SITE_PRIVATE(site_public);
    Xv_opaque		 value;

    switch (attr) {
#ifdef WINDOW_SITES
	case DROP_SITE_TYPE:
	   if (status(site, is_window_region))
               value = (Xv_opaque)DND_WINDOW_SITE;
	   else
               value = (Xv_opaque)DND_RECT_SITE;
           break;
#endif /* WINDOW_SITES */
        case DROP_SITE_SIZE:
	   value = (Xv_opaque)site->site_size;
	   break;
        case DROP_SITE_ID:
           value = (Xv_opaque)site->site_id;
           break;
        case DROP_SITE_DEFAULT:
           value = (Xv_opaque)((site->event_mask & DND_DEFAULT_SITE) ?
		TRUE : FALSE);
           break;
        case DROP_SITE_EVENT_MASK:
           value = (Xv_opaque)(site->event_mask ^ DND_DEFAULT_SITE);
           break;
        case DROP_SITE_REGION:
	   if (status(site, is_window_region))
               value = (Xv_opaque)DndDropAreaOps(site, Dnd_Get_Window, NULL);
	   else
               value = (Xv_opaque)DndDropAreaOps(site, Dnd_Get_Rect, NULL);
	   if (value == XV_ERROR) *error = XV_ERROR;
           break;
        case DROP_SITE_REGION_PTR:
	   if (status(site, is_window_region))
               value = (Xv_opaque)DndDropAreaOps(site, Dnd_Get_Window_Ptr,NULL);
	   else
               value = (Xv_opaque)DndDropAreaOps(site, Dnd_Get_Rect_Ptr, NULL);
	   if (value == XV_ERROR) *error = XV_ERROR;
           break;
        default:
           if (xv_check_bad_attr(&xv_drop_site_item, attr) == XV_ERROR)
		*error = XV_ERROR;
	   value = XV_ZERO;
	   break;
    }

    return(value);
}

Pkg_private int
dnd_site_destroy(site_public, state)
    Xv_drop_site	site_public;
    Destroy_status	state;
{
    Dnd_site_info	*site = DND_SITE_PRIVATE(site_public);

    if (state == DESTROY_CLEANUP) {
	xv_set(site->owner, WIN_DELETE_DROP_ITEM, DND_SITE_PUBLIC(site), NULL);
        xv_set(win_get_top_level(site->owner), WIN_DELETE_DROP_INTEREST,
						  DND_SITE_PUBLIC(site), NULL);
	if (status(site, is_window_region))
            (void)DndDropAreaOps(site, Dnd_Delete_All_Windows, NULL);
	else
            (void)DndDropAreaOps(site, Dnd_Delete_All_Rects, NULL);
	xv_free(site);
    }

    return(XV_OK);
}
