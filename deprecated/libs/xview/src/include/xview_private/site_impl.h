#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)site_impl.h 1.5 93/06/28";
#endif
#endif

/*
 *      (c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *      pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *      file for terms of the license.
 */

#ifndef xview_site_impl_DEFINED
#define xview_site_impl_DEFINED

#include <sys/time.h>
#include <X11/Xlib.h>
#include <xview/pkg.h>
#include <xview/attr.h>
#include <xview/rect.h>
#include <xview/window.h>
#include <xview/dragdrop.h>
#include <xview_private/xv_list.h>

#define BIT_FIELD(field)		unsigned	field:1
#define status(item, field)		((item)->status_bits.field)
#define status_set(item, field)       	status(item, field) = TRUE
#define status_reset(item, field)     	status(item, field) = FALSE


#define DND_SITE_PRIVATE(dnd_site_public) \
		XV_PRIVATE(Dnd_site_info, Xv_drop_site_struct, dnd_site_public)
#define DND_SITE_PUBLIC(site)         XV_PUBLIC(site)

typedef enum dnd_region_ops {
	Dnd_Add_Window, 	Dnd_Delete_Window,
	Dnd_Add_Window_Ptr, 	Dnd_Delete_Window_Ptr,
	Dnd_Add_Rect, 		Dnd_Delete_Rect,
	Dnd_Add_Rect_Ptr, 	Dnd_Delete_Rect_Ptr,
	Dnd_Get_Window,		Dnd_Get_Window_Ptr,
	Dnd_Get_Rect,		Dnd_Get_Rect_Ptr,
	Dnd_Delete_All_Rects,	Dnd_Delete_All_Windows,
} Dnd_region_ops;

typedef struct dnd_window_list {
	Xv_sl_link	next;
	Xv_Window	window;
} Dnd_window_list;

typedef struct dnd_rect_list {
	Xv_sl_link	next;
	int		real_x;
	int		real_y;
	Rect		rect;
} Dnd_rect_list;

typedef struct dnd_site_info {
	Xv_drop_site	 public_self;
	Xv_window	 owner;
	Window		 owner_xid;
	long		 site_id;
	int		 event_mask;
	unsigned int	 site_size;

	struct {
		BIT_FIELD(site_id_set);
		BIT_FIELD(window_set);
		BIT_FIELD(is_window_region);
		BIT_FIELD(created);
	} status_bits;

	union {
	    Dnd_window_list *windows;
	    Dnd_rect_list   *rects;
	} region;
	unsigned int	 num_regions;
} Dnd_site_info; 

Pkg_private int		dnd_site_init();
Pkg_private Xv_opaque	dnd_site_set_avlist();
Pkg_private Xv_opaque	dnd_site_get_attr();
Pkg_private int		dnd_site_destroy();

#endif /* ~xview_site_impl_DEFINED */
