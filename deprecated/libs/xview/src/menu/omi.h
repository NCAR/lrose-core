/*	@(#)omi.h 20.22 93/06/28 	*/

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#ifndef xview_image_DEFINED
#define xview_image_DEFINED

/*
 ***********************************************************************
 *			Include Files
 ***********************************************************************
 */

#include <xview/base.h>
#include <xview/attrol.h>

/*
 ***********************************************************************
 *			Definitions and Macros
 ***********************************************************************
 */

/*
 * PUBLIC #defines
 */

#define	IMAGE_NULL			((Image)0)

/*
 * PRIVATE #defines 
 */

#define	IMAGE_ATTR(type, ordinal)	ATTR(ATTR_PKG_IMAGE, type, ordinal)
#define	IMAGE_ATTRIBUTE_LIST		ATTR_LIST

/*
 * Fake types -- This should be resolved someday 
 */
#define ATTR_IMAGE_PTR			ATTR_OPAQUE
#define ATTR_FONT_PTR			ATTR_OPAQUE
#define ATTR_PIXRECT_INT_INT		ATTR_INT_TRIPLE

/*
 * Reserved for future use 
 */
#define	IMAGE_ATTR_UNUSED_FIRST		 0
#define	IMAGE_ATTR_UNUSED_LAST		31

/*
 ***********************************************************************
 *		Typedefs, enumerations, and structs
 ***********************************************************************
 */

typedef	Xv_opaque 		Image;

typedef enum {
	/*
	 * Private Attributes 
	 */
	IMAGE_ACTIVE		= IMAGE_ATTR(ATTR_NO_VALUE,		 32), 
	IMAGE_CENTER		= IMAGE_ATTR(ATTR_BOOLEAN, 		133),
	IMAGE_ERROR_PROC	= IMAGE_ATTR(ATTR_FUNCTION_PTR,		 49),
	IMAGE_GINFO		= IMAGE_ATTR(ATTR_OPAQUE,		 34),
	IMAGE_HEIGHT		= IMAGE_ATTR(ATTR_INT,			 35),
	IMAGE_INACTIVE		= IMAGE_ATTR(ATTR_NO_VALUE,		 36), 
	IMAGE_INVERT		= IMAGE_ATTR(ATTR_BOOLEAN,		 37), 
	IMAGE_LEFT_PIXRECT	= IMAGE_ATTR(ATTR_PIXRECT_PTR,		 38), 
	IMAGE_PIXRECT		= IMAGE_ATTR(ATTR_PIXRECT_PTR,		 41), 
	IMAGE_REGION		= IMAGE_ATTR(ATTR_PIXRECT_INT_INT,	 50), 
	IMAGE_RELEASE		= IMAGE_ATTR(ATTR_NO_VALUE,		 42), 
	IMAGE_RELEASE_STRING	= IMAGE_ATTR(ATTR_NO_VALUE,		 43), 
	IMAGE_RELEASE_PR	= IMAGE_ATTR(ATTR_NO_VALUE,		 44), 
	IMAGE_RENDER_STRING 	= IMAGE_ATTR(ATTR_BOOLEAN,		 52),
	IMAGE_TITLE		= IMAGE_ATTR(ATTR_BOOLEAN,		 54),
	IMAGE_3D		= IMAGE_ATTR(ATTR_BOOLEAN,		 39),
	IMAGE_WIDTH		= IMAGE_ATTR(ATTR_INT,			 48),
} Image_attribute;

/*
 ***********************************************************************
 *			Globals
 ***********************************************************************
 */

/* 
 * 	Functions
 *
 *  extern Image	image_create(av_list);
 *  extern Xv_opaque	image_get(image|image_item, attr{, data});
 *  extern int		image_set(image|image_item, attr{, data}, value);
 *  extern void		image_destroy(image|image_item);
 *  extern void		image_paint();
 */

   extern Image		 image_create();
   extern Xv_opaque	 image_get();
   extern int		 image_set();
   extern void		 image_destroy();
   extern void		 image_render();

#endif ~xview_image_DEFINED
