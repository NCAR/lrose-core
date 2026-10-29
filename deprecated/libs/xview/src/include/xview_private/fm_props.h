/*	@(#)fm_props.h 1.33 93/06/28  */

/***********************************************************************/
/*	            frame_props.h/fm_props.h		               */
/*	
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license. 
 */
/***********************************************************************/

#ifndef _frame_props_h_already_included
#define _frame_props_h_already_included

/* standard includes */
#ifndef FILE
#if !defined(SVR4) && !defined(__linux) && !defined(__APPLE__)
#undef NULL
#endif /* SVR4 */
#include <stdio.h>
#endif /* FILE */
#include <sys/time.h>
#include <xview/notify.h>
#include <xview/rect.h>
#include <xview/rectlist.h>

#ifndef	pixrect_hs_DEFINED
#define	pixrect_hs_DEFINED
/* <pixrect/pixrect_hs.h> without frame buffer variable include files */
#include <sys/types.h>
#include <pixrect/pixrect.h>
#include <pixrect/pr_dblbuf.h>
#include <pixrect/pr_line.h>
#include <pixrect/pr_planegroups.h>
#include <pixrect/pr_util.h>
#include <pixrect/traprop.h>

#ifdef __STDC__ 
#ifndef CAT
#define CAT(a,b)        a ## b 
#endif 
#endif
#include <pixrect/memvar.h>

#include <pixrect/pixfont.h>
#include <rasterfile.h>
#include <pixrect/pr_io.h>
#endif /*	pixrect_hs_DEFINED */

#include <xview/win_struct.h>	/* for WL_ links */
#include <xview/win_input.h>

/* all this for wmgr.h */
#include <xview/win_screen.h>
#include <xview/wmgr.h>
#include <xview_private/wmgr_decor.h>

#include <xview/attrol.h>
#include <xview/frame.h>
#include <xview/icon.h>
#include <xview/openmenu.h>
#include <xview/panel.h>

/* all this for XWMHints */
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#define	FRAME_PROPS_PRIVATE(f)	XV_PRIVATE(Frame_props_info, Xv_frame_props, f)
#define FRAME_PROPS_PUBLIC(f)   XV_PUBLIC(f)
#define FRAME_CLASS_FROM_PROPS(f) FRAME_PRIVATE(FRAME_PROPS_PUBLIC(f))

#define FRAME_PROPS_FLAGS WMDecorationHeader | WMDecorationCloseButton | \
                          WMDecorationPushPin | WMDecorationOKButton | \
                          WMDecorationResizeable

typedef	struct	{
    Frame	public_self;	/* back pointer to object */
    WM_Win_Type	win_attr;	/* _OL_WIN_ATTR */			
    struct {
	BIT_FIELD(show_label); 		/* show label or not */
	BIT_FIELD(show_footer); 	/* show footer or not */
	BIT_FIELD(pushpin_in);		/* is pushpin in or out */
    } status_bits;
    Panel	 panel;
    int		 (*apply_proc)();
    void	 (*reset_proc)();
} Frame_props_info;

/* frame_props.c */
Pkg_private Notify_value	frame_props_input();
Pkg_private int		frame_props_init();

/* frame_props_get.c */
Pkg_private Xv_opaque	frame_props_get_attr();

/* frame_props_set.c */
Pkg_private Xv_opaque	frame_props_set_avlist();

/* frame_props_destroy.c */
Pkg_private int			frame_props_destroy();

#endif
