/*      @(#)xv_xlib.h 20.13 93/06/28 SMI      */

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/* 
 * 	Bridge between View2 implementation and Xlib.
 */

#ifndef _view2_private_xv_xlib_h_already_included
#define _view2_private_xv_xlib_h_already_included

#include <view2/qualifyx.h>
#include <X11/Xlib.h>
#include <view2/unqualifyx.h>

/*
 * To enable applications to get the current clipping list
 * to do direct X graphics.
 */
#define XV_MAX_XRECTS 32
typedef struct {
	XRectangle      rect_array[XV_MAX_XRECTS];
	int             count;
} Xv_xrectlist;

#endif _view2_private_xv_xlib_h_already_included

