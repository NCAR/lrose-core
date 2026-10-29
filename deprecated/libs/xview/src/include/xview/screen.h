 /*      @(#)screen.h 20.37 93/06/28 SMI      */

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE 
 *	file for terms of the license.
 */

#ifndef xview_screen_DEFINED
#define xview_screen_DEFINED

/*
 ***********************************************************************
 *			Include Files
 ***********************************************************************
 */

#include <xview/generic.h>

/*
 ***********************************************************************
 *			Definitions and Macros
 ***********************************************************************
 */

#ifndef XV_ATTRIBUTES_ONLY

/*
 * PUBLIC #defines 
 */

#define	SCREEN				&xv_screen_pkg

/*
 * PRIVATE #defines 
 */

#define SCREEN_TYPE			ATTR_PKG_SCREEN

#endif /* ~XV_ATTRIBUTES_ONLY */

#define SCREEN_ATTR(type, ordinal)      ATTR(ATTR_PKG_SCREEN, type, ordinal)

/*
 ***********************************************************************
 *		Typedefs, Enumerations, and Structures
 ***********************************************************************
 */

#ifndef XV_ATTRIBUTES_ONLY

typedef	Xv_opaque 	Xv_Screen;
typedef	Xv_opaque 	Xv_screen;

#endif /* ~XV_ATTRIBUTES_ONLY */

typedef enum {
	/*
	 * Public attributes 
	 */
	__SCREEN_NUMBER	  		= SCREEN_ATTR(ATTR_INT,		10),
	__SCREEN_SERVER	  		= SCREEN_ATTR(ATTR_OPAQUE,	15),
	/*
	 * Private attributes 
	 */
	__SCREEN_DEFAULT_VISUAL		= SCREEN_ATTR(ATTR_OPAQUE,      75),  /* G-- */
	__SCREEN_VISUAL		       	= SCREEN_ATTR(ATTR_OPAQUE,	80),  /* G-- */
	/* Format: xv_get(screen, SCREEN_VISUAL, vinfo_template, vinfo_mask); */
	__SCREEN_IMAGE_VISUAL		= SCREEN_ATTR(ATTR_OPAQUE,      85),  /* G-- */
	/* Format: xv_get(screen, SCREEN_IMAGE_VISUAL, xid, depth); */ 
        __SCREEN_DEFAULT_CMS      	= SCREEN_ATTR(ATTR_OPAQUE,     	30),
        __SCREEN_RETAIN_WINDOWS   	= SCREEN_ATTR(ATTR_BOOLEAN,    	40),
	__SCREEN_BG1_PIXMAP		= SCREEN_ATTR(ATTR_OPAQUE,	50),
	__SCREEN_BG2_PIXMAP		= SCREEN_ATTR(ATTR_OPAQUE,	55),
	__SCREEN_BG3_PIXMAP		= SCREEN_ATTR(ATTR_OPAQUE,	60),
	__SCREEN_GINFO			= SCREEN_ATTR(ATTR_OPAQUE,	65),
	__SCREEN_OLGC_LIST		= SCREEN_ATTR(ATTR_OPAQUE,      70),  /* G-- */
	__SCREEN_SUN_WINDOW_STATE		= SCREEN_ATTR(ATTR_BOOLEAN,     90),
	__SCREEN_SELECTION_STATE		= SCREEN_ATTR(ATTR_LONG,        95)

} Screen_attr;


#define SCREEN_NUMBER ((Attr_attribute) __SCREEN_NUMBER)
#define SCREEN_SERVER ((Attr_attribute) __SCREEN_SERVER)
#define SCREEN_DEFAULT_VISUAL ((Attr_attribute) __SCREEN_DEFAULT_VISUAL)
#define SCREEN_VISUAL ((Attr_attribute) __SCREEN_VISUAL)
#define SCREEN_IMAGE_VISUAL ((Attr_attribute) __SCREEN_IMAGE_VISUAL)
#define SCREEN_DEFAULT_CMS ((Attr_attribute) __SCREEN_DEFAULT_CMS)
#define SCREEN_RETAIN_WINDOWS ((Attr_attribute) __SCREEN_RETAIN_WINDOWS)
#define SCREEN_BG1_PIXMAP ((Attr_attribute) __SCREEN_BG1_PIXMAP)
#define SCREEN_BG2_PIXMAP ((Attr_attribute) __SCREEN_BG2_PIXMAP)
#define SCREEN_BG3_PIXMAP ((Attr_attribute) __SCREEN_BG3_PIXMAP)
#define SCREEN_GINFO ((Attr_attribute) __SCREEN_GINFO)
#define SCREEN_OLGC_LIST ((Attr_attribute) __SCREEN_OLGC_LIST)
#define SCREEN_SUN_WINDOW_STATE ((Attr_attribute) __SCREEN_SUN_WINDOW_STATE)
#define SCREEN_SELECTION_STATE ((Attr_attribute) __SCREEN_SELECTION_STATE)


/* Define the different types of GC available in the GC list */
#define SCREEN_SET_GC		0
#define SCREEN_CLR_GC		1
#define SCREEN_TEXT_GC		2
#define SCREEN_BOLD_GC		3
#define SCREEN_GLYPH_GC		4
#define SCREEN_INACTIVE_GC	5
#define SCREEN_DIM_GC		6
#define SCREEN_INVERT_GC	7
#define SCREEN_NONSTD_GC	8	/* Color or non-standard font */
#define SCREEN_RUBBERBAND_GC	9
#define SCREEN_OLGC_LIST_SIZE	10

#ifndef XV_ATTRIBUTES_ONLY

typedef struct {
    Xv_generic_struct	parent;
    Xv_opaque		private_data;
} Xv_screen_struct;

#endif /* ~XV_ATTRIBUTES_ONLY */

/*
 ***********************************************************************
 *				Globals
 ***********************************************************************
 */

/*
 * PUBLIC Variables 
 */

extern Xv_Screen	xv_default_screen;

/*
 * PRIVATE Variables 
 */

extern Xv_pkg	xv_screen_pkg;

#endif /* ~xview_screen_DEFINED */
