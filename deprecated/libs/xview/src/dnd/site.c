#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)site.c 1.13 93/06/28";
#endif
#endif

/*
 *      (c) Copyright 1990 Sun Microsystems, Inc. Sun design patents 
 *      pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *      file for terms of the license.
 */

/* SUPPRESS 560 */

#include <assert.h>
#include <xview_private/xv_list.h>
#include <xview_private/site_impl.h>
#include <xview_private/windowimpl.h>
#ifdef SVR4 
#include <stdlib.h> 
#endif /* SVR4 */

static     void 	TransCoords();
Xv_private Xv_Window	win_get_top_level();

Pkg_private Xv_opaque
DndDropAreaOps(site, mode, area)
    register Dnd_site_info	*site;
    register Dnd_region_ops	 mode;
    register Xv_opaque		 area;
{
    switch(mode) {
      case Dnd_Add_Window: {
	  register Dnd_window_list *winNode;

		/* Create the head of the list.  Never used. */
	  if (!site->region.windows) {
	      site->region.windows = xv_alloc(Dnd_window_list);
	      XV_SL_INIT(site->region.windows);
	  }

		/* Create the next node and place window in it. */
	  winNode = xv_alloc(Dnd_window_list);
	  winNode->window = (Xv_Window)area;

	  site->num_regions++;

		/* Add the new node to the site list. */
	  XV_SL_ADD_AFTER(site->region.windows, site->region.windows, winNode);
      }
      break;
      case Dnd_Add_Rect: {
	  register Dnd_rect_list *rectNode;
	  register Rect		 *rect;

		/* Create the head of the list.  Never used. */
	  if (!site->region.rects) {
	      site->region.rects = xv_alloc(Dnd_rect_list);
	      XV_SL_INIT(site->region.rects);
	  }
		/* Create the next node and place the rect in it. */
	  rectNode = xv_alloc(Dnd_rect_list);

	  rect = (Rect *)area;
	  rectNode->rect.r_left = rect->r_left;
	  rectNode->rect.r_top = rect->r_top;
	  rectNode->rect.r_width = rect->r_width;
	  rectNode->rect.r_height = rect->r_height;
	  TransCoords(site, rectNode);

	  site->num_regions++;

		/* Add the new node to the site list. */
	  XV_SL_ADD_AFTER(site->region.rects, site->region.rects, rectNode);
      }
      break;
      case Dnd_Delete_Window: {
	  register Dnd_window_list *winNode, *nodePrev;

	  if (!site->region.windows)
	      return(XV_ERROR);

	  nodePrev = winNode = site->region.windows;

	  while(winNode = (Dnd_window_list *) (XV_SL_SAFE_NEXT(winNode))) {
	      if (winNode->window == (Xv_Window)area) {
		  xv_free(XV_SL_REMOVE_AFTER(site->region.windows, nodePrev));
		  site->num_regions--;
		  return(XV_OK);
	      }
	      nodePrev = winNode;
	  }
	  return (XV_ERROR);
      }
      /* NOTREACHED */
      break;
      case Dnd_Delete_Rect: {
	  register Dnd_rect_list *rectNode, *nodePrev;
	  register Rect		 *rect = (Rect *)area;

	  if (!site->region.rects)
	      return(XV_ERROR);

	  nodePrev = rectNode = site->region.rects;

	  while(rectNode = (Dnd_rect_list *) (XV_SL_SAFE_NEXT(rectNode))) {
	      if (rect_equal(rect, &rectNode->rect)) {
		  xv_free(XV_SL_REMOVE_AFTER(site->region.rects, nodePrev));
		  site->num_regions--;
		  return(XV_OK);
	      }
	      nodePrev = rectNode;
	  }
	  return (XV_ERROR);
      }
      /* NOTREACHED */
      break;
      case Dnd_Add_Window_Ptr: {
	  register Xv_Window	   *windows;

		/* Create the head of the list.  Never used. */
	  if (!site->region.windows) {
	      site->region.windows = xv_alloc(Dnd_window_list);
	      XV_SL_INIT(site->region.windows);
	  }

	  for (windows = (Xv_Window *)area ; *windows ; windows++) { 
	      register Dnd_window_list *winNode;

		    /* Create the next node and place a window in it. */
	      winNode = xv_alloc(Dnd_window_list);
	      winNode->window = (Xv_Window)*windows;

	      site->num_regions++;

		   /* Add the new node to the site list. */
	      XV_SL_ADD_AFTER(site->region.windows, site->region.windows,
			      winNode);
	  }
      }
      break;
      case Dnd_Add_Rect_Ptr: {
	  register Rect	   *rects;

		/* Create the head of the list.  Never used. */
	  if (!site->region.rects) {
	      site->region.rects = xv_alloc(Dnd_rect_list);
	      XV_SL_INIT(site->region.rects);
	  }

	  for (rects = (Rect *)area ; rects && !rect_isnull(rects); rects++) { 
	      register Dnd_rect_list *rectNode;

		    /* Create the next node and place a rect in it. */
	      rectNode = xv_alloc(Dnd_rect_list);
	      rectNode->rect.r_left = rects->r_left;
	      rectNode->rect.r_top = rects->r_top;
	      rectNode->rect.r_width = rects->r_width;
	      rectNode->rect.r_height = rects->r_height;
	      TransCoords(site, rectNode);

	      site->num_regions++;

		   /* Add the new node to the site list. */
	      XV_SL_ADD_AFTER(site->region.rects, site->region.rects, rectNode);
	  }
      }
      break;
      case Dnd_Delete_Window_Ptr: {
	  register Xv_Window	*windows;

	  if (!site->region.windows)
	      return(XV_ERROR);

	  /* REMIND: These two loops must be optimized. */

	  for (windows = (Xv_Window *)area ; *windows ; windows++) {
	      register Dnd_window_list *winNode, *nodePrev;

	      nodePrev = winNode = site->region.windows;

	      while(winNode = (Dnd_window_list *) (XV_SL_SAFE_NEXT(winNode))) {
	          if (winNode->window == (Xv_Window)*windows) {
		      xv_free(XV_SL_REMOVE_AFTER(site->region.windows,
			      nodePrev));
		      site->num_regions--;
		      break;
	          }
	          nodePrev = winNode;
	      }
	  }
      }
      break;
      case Dnd_Delete_Rect_Ptr: {
	  register Rect	*rects;

	  if (!site->region.rects)
	      return(XV_ERROR);

	  /* REMIND: These two loops must be optimized. */

	  for (rects = (Rect *)area ; rects && !rect_isnull(rects); rects++) {
	      register Dnd_rect_list *rectNode, *nodePrev;

	      nodePrev = rectNode = site->region.rects;

	      while(rectNode = (Dnd_rect_list *) (XV_SL_SAFE_NEXT(rectNode))) {
	          if (rect_equal(rects, &rectNode->rect)) {
		      xv_free(XV_SL_REMOVE_AFTER(site->region.rects, nodePrev));
		      site->num_regions--;
		      break;
	          }
	          nodePrev = rectNode;
	      }
	  }
      }
      break;
      case Dnd_Get_Window: {
	  register Dnd_window_list *winNode = site->region.windows;

	  if (!site->region.windows)
	      return(XV_ERROR);

		/* Since the head of the list is not used, get the next node. */
	  winNode = (Dnd_window_list *) (XV_SL_SAFE_NEXT(winNode));

	  return(winNode->window);
      }
      /* NOTREACHED */
      break;
      case Dnd_Get_Rect: {
	  register Dnd_rect_list *rectNode = site->region.rects;
	  Rect			 *rect;
	  if (!site->region.rects)
	      return(XV_ERROR);

		/* Since the head of the list is not used, get the next node. */
	  rectNode = (Dnd_rect_list *) (XV_SL_SAFE_NEXT(rectNode));

	  if (!rectNode)
	      return(XV_ERROR);

	  rect = xv_alloc(Rect);
#if defined(SVR4) || defined(__linux) || defined (__APPLE__)
          /* This will probably not work right, but it compiles. */
          /* (rectNode->rect) is of the wrong type. */
          memmove(rect, &(rectNode->rect),  sizeof(Rect));
#else
	  bcopy(rectNode->rect, rect, sizeof(Rect));
#endif /* SVR4 */

	  return((Xv_opaque)rect);
      }
      /* NOTREACHED */
      break;
      case Dnd_Get_Window_Ptr: {
	  register Dnd_window_list *winNode = site->region.windows;
	  register Xv_Window	   *windows;
	  register int	   	    i;

	  if (!site->region.windows)
	      return(XV_ERROR);

				/* One extra window for NULL entry */
	  windows = xv_alloc_n(Xv_Window, (site->num_regions + 1) );

	  for (i = 0; i < site->num_regions; i++) {
	      winNode = (Dnd_window_list *) (XV_SL_SAFE_NEXT(winNode));
	      assert(winNode != NULL);
	      windows[i] = winNode->window;
	  }
	  windows[site->num_regions] = (Xv_Window)NULL;
	  return((Xv_opaque)windows);
      }
      /* NOTREACHED */
      break;
      case Dnd_Get_Rect_Ptr: {
	  register Dnd_rect_list *rectNode = site->region.rects;
	  register Rect	   	 *rects;
	  register int	   	  i;

	  if (!site->region.rects)
	      return(XV_ERROR);

				/* One extra window for NULL entry */
	  rects = xv_alloc_n(Rect, (site->num_regions + 1) );

	  for (i = 0; i < site->num_regions; i++) {
	      rectNode = (Dnd_rect_list *) (XV_SL_SAFE_NEXT(rectNode));
	      assert(rectNode != NULL);
	      rects[i] = rectNode->rect;
	  }
	  rects[site->num_regions].r_width = 0;
	  rects[site->num_regions].r_height = 0;

	  return((Xv_opaque)rects);
      }
      /* NOTREACHED */
      break;
      case Dnd_Delete_All_Windows: {
	  register Dnd_window_list *winNode = site->region.windows;

	  if (!site->region.windows)
	      return(XV_ERROR);

	  while(winNode = (Dnd_window_list *) (XV_SL_SAFE_NEXT(winNode)))
	      xv_free(XV_SL_REMOVE_AFTER(site->region.windows,
					 site->region.windows));

	  xv_free(site->region.windows);
	  site->region.windows = NULL;

	  site->num_regions = 0;
      }
      break;
      case Dnd_Delete_All_Rects: {
	  register Dnd_rect_list *rectNode = site->region.rects;
	  if (!site->region.rects)
	      return(XV_ERROR);

	  rectNode = (Dnd_rect_list *) (XV_SL_SAFE_NEXT(rectNode));
	  while(rectNode) {
	      Dnd_rect_list *rectNodeNext =
				   (Dnd_rect_list *)(XV_SL_SAFE_NEXT(rectNode));

	      xv_free(XV_SL_REMOVE_AFTER(site->region.rects,
					 site->region.rects));
	      rectNode = rectNodeNext;
	  }

	  xv_free(site->region.rects);
	  site->region.rects = NULL;

	  site->num_regions = 0;
      }
      break;
      default:
	  return(XV_ERROR);
    }
    return (XV_OK);
}

static void
TransCoords(site, node)
    register Dnd_site_info *site;
    register Dnd_rect_list *node;
{
    Xv_Window 	frame,
		window;
    int		x, y;
	 
    frame = win_get_top_level(site->owner);
    assert(frame != XV_ERROR);
		      
    x = node->rect.r_left;
    y = node->rect.r_top;
    window = site->owner;

    while (window != frame) {
	int bw = xv_get(window, WIN_BORDER);
	x += xv_get(window, XV_X) + bw;
	y += xv_get(window, XV_Y) + bw;
	window = xv_get(window, XV_OWNER);
    }
    node->real_x = x;
    node->real_y = y;
}

Pkg_private void
DndSizeOfSite(site)
    register Dnd_site_info *site;
{
    site->site_size = 3;             /* Window + site id + flags */

    if (status(site, is_window_region))
        site->site_size += 2 + site->num_regions; 
    else
        site->site_size += 2 + 4 * site->num_regions;
}

Xv_private int
DndStoreSiteData(site_public, prop)
    register Xv_drop_site  	 site_public;
    register long	       **prop;
{
    register Dnd_site_info 	*site = DND_SITE_PRIVATE(site_public);
    register Dnd_window_list 	*windows;
    register Dnd_rect_list   	*rects;
    register int	         i;
    register long		*data;

    data = *prop;
		/* If the site has no regions, then we don't update the
		 * interest property with this site.
		 */
    if (!site->num_regions)
	return(0);

    *data++ = site->owner_xid;
    *data++ = site->site_id;
    *data++ = site->event_mask;

    if (status(site, is_window_region)) {
	*data++ = DND_WINDOW_SITE;
	*data++ = site->num_regions;
	for (i = 0,
	     windows=(Dnd_window_list *)(XV_SL_SAFE_NEXT(site->region.windows));
	     i < site->num_regions;
	     i++, windows = (Dnd_window_list *) (XV_SL_SAFE_NEXT(windows))) {

	     *data++ = (Window)xv_get(windows->window, XV_XID);
	}
    } else {
	*data++ = DND_RECT_SITE;
	*data++ = site->num_regions;
	for (i = 0,
	     rects = (Dnd_rect_list *)(XV_SL_SAFE_NEXT(site->region.rects));
	     i < site->num_regions;
	     i++, rects = (Dnd_rect_list *) (XV_SL_SAFE_NEXT(rects))) {

	     *data++ = rects->real_x;
	     *data++ = rects->real_y;
	     *data++ = (unsigned)rects->rect.r_width;
	     *data++ = (unsigned)rects->rect.r_height;
	}
    }
    *prop = data;
    return(1);
}
