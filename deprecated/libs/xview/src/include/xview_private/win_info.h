/*	@(#)win_info.h 20.17 93/06/28 SMI	*/

/****************************************************************************/
/*	
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license. 
 */
/****************************************************************************/

#ifndef _xview_win_visual_h_already_included
#define _xview_win_visual_h_already_included

#include <xview_private/scrn_vis.h>

typedef struct {
    XID			 xid;
    Screen_visual	*visual;
	/* Flags */
    unsigned		 private_gc	: 1;	/* Should be gc itself? */
} Win_info;

#define	win_xid(info)		((info)->xid)
#define	win_display(info)	((info)->visual->display)
#define	win_server(info)	((info)->visual->server)
#define	win_screen(info)	((info)->visual->screen)
#define	win_root(info)		((info)->visual->root_window)
#define	win_depth(info)		((info)->visual->depth)
#define	win_image(info)		((info)->visual->image)

#define	window_gc(window, info) \
	((info)->private_gc ? window_private_gc((window)) : (info)->visual->gc)

#define	win_set_image(info, im)	(info)->visual->image = im

extern GC		window_private_gc();
extern Win_info		*window_info();
extern Xv_object	win_data();

#define CONTEXT		1
#endif
