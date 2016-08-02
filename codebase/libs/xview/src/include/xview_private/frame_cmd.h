#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)frame_cmd.h 1.39 93/06/28";
#endif
#endif

/***********************************************************************/
/*	                      frame_cmd.h		               */
/*	
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE 
 *	file for terms of the license. 
 */
/***********************************************************************/

#ifndef _frame_cmd_h_already_included
#define _frame_cmd_h_already_included

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

#include <xview/win_struct.h>	/* for WL_ links */
#include <xview/win_input.h>

/* all this for wmgr.h */
#include <xview/win_screen.h>
#include <xview/wmgr.h>
#include <xview_private/wmgr_decor.h>

#include <xview/pkg.h>
#include <xview/attrol.h>
#include <xview_private/fm_impl.h>
#include <xview/icon.h>
#include <xview/openmenu.h>
#include <xview/panel.h>

/* all this for XWMHints */
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#define	FRAME_CMD_PRIVATE(f)	XV_PRIVATE(Frame_cmd_info, Xv_frame_cmd, f)
#define FRAME_CMD_PUBLIC(f)	XV_PUBLIC(f)
#define FRAME_CLASS_FROM_CMD(f) FRAME_PRIVATE(FRAME_CMD_PUBLIC(f))

#define	FRAME_PROPS_PRIVATE(f)	FRAME_CMD_PRIVATE(f)
#define FRAME_PROPS_PUBLIC(f)   FRAME_CMD_PUBLIC(f)
#define FRAME_CLASS_FROM_PROPS(f) FRAME_CLASS_FROM_CMD(f)


#define FRAME_CMD_FLAGS		WMDecorationHeader | WMDecorationCloseButton | \
				WMDecorationPushPin
			
#define FRAME_PROPS_FLAGS	FRAME_CMD_FLAGS
			
typedef	struct	{
    Frame	public_self;	/* back pointer to object */
    WM_Win_Type	win_attr;	/* _OL_WIN_ATTR */			

    struct {
	BIT_FIELD(show_label); 		/* show label or not */
	BIT_FIELD(pushpin_in);		/* is pushpin in or out */
	BIT_FIELD(warp_pointer);	/* whether to warp the pointer when window is mapped */
	BIT_FIELD(show_resize_corner);	/* show resize corner or not */
	BIT_FIELD(default_pin_state);   /* default (or initial) pin state */
	BIT_FIELD(default_pin_state_valid);   /* whether default_pin_state has useful value */
    } status_bits;
    Panel	panel;
} Frame_cmd_info;

#define	Frame_props_info	Frame_cmd_info

/* frame_cmd.c */
Pkg_private Notify_value	frame_cmd_input();
Pkg_private int		frame_cmd_init();

/* frame_cmd_get.c */
Pkg_private Xv_opaque	frame_cmd_get_attr();

/* frame_cmd_set.c */
Pkg_private Xv_opaque	frame_cmd_set_avlist();

/* frame_cmd_destroy.c */
Pkg_private int			frame_cmd_destroy();

#endif
