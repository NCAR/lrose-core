/*      @(#)unqualifyx.h 20.9 93/06/28 SMI      */

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/* 
 * 	Bridge between sv implementation and Xlib.
 */

/* This file can be included many times.  Each time it is
 * included, the effect of qualify_x.h is undone.
 * It is expected that qualify_x.h and unqualify_x.h are included in
 * matched pairs.
 */
#undef _view2_private_qualify_x_h_already_included

/*
 * The following un-definitions undo the defines found in qualify_x.h.
 */
#undef Colormap
#undef Cursor
#undef Depth
#undef Device
#undef Display
#undef Drawable
#undef Font
#undef Pixmap
#undef Screen
#undef ScreenFormat
#undef Visual
#undef Window
