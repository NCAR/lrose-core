/*	@(#)qualifyx.h 20.9 93/06/28 SMI	*/

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/* 
 * 	Bridge between sv implementation and Xlib.
 */

#ifndef _view2_private_qualify_x_h_already_included
#define _view2_private_qualify_x_h_already_included

/*
 * The following definitions rename those Xlib types
 * that are in conflict with Sv (or likely to be).
 * Atom could not be redefined as Xatom.h use "Atom" in
 * the defining macros. The preprocessor does not replace strings
 * in macro definitions.
 */
#define Colormap	XColormap_t
#define Cursor		XCursor_t
#define Depth		XDepth_t
#define Device		XDevice_t
#define Display		XDisplay_t
#define Drawable	XDrawable_t
#define Font		XFont_t
#define Pixmap		XPixmap_t
#define Screen		XScreen_t
#define ScreenFormat	XScreenFormat_t
#define Visual		XVisual_t
#define Window		XWindow_t

#endif _view2_private_qualify_x_h_already_included
