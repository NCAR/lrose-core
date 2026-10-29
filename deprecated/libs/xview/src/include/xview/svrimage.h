/*	@(#)svrimage.h 20.30 93/06/28 SMI	*/

#ifndef xview_server_image_DEFINED
#define xview_server_image_DEFINED

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE 
 *	file for terms of the license.
 */

/*
 ***********************************************************************
 *				Include Files
 ***********************************************************************
 */

#include <xview/generic.h>
#include <xview/pkg.h>
#include <xview/drawable.h>
#include <sys/types.h>				/* needed for pixrect.h */
#include <pixrect/pixrect.h>
#include <pixrect/pixfont.h>
#ifdef OW_I18N
#include <widec.h>
#endif


/*
 ***********************************************************************
 *			Definitions and Macros
 ***********************************************************************
 */

/*
 * PUBLIC #defines 
 */

#define	SERVER_IMAGE 			&xv_server_image_pkg

#define	server_image_begin_bits(name)	unsigned short name[] = {
#define	server_image_end_bits(name)	};

/*
 * PRIVATE #defines 
 */

#define SERVER_IMAGE_ATTR(type, ordinal)	\
				ATTR(ATTR_PKG_SERVER_IMAGE, type, ordinal)

#define SERVER_IMAGE_TYPE  	ATTR_PKG_SERVER_IMAGE

#define SERVER_IMAGE_ATTR_LIST(ltype, type, ordinal) \
        SERVER_IMAGE_ATTR(ATTR_LIST_INLINE((ltype), (type)), (ordinal))

/*
 ***********************************************************************
 *		Typedefs, Enumerations, and Structures
 ***********************************************************************
 */

typedef Xv_opaque	 	Server_image;

typedef enum {
	/*
	 * Public attributes 
	 */
	__SERVER_IMAGE_DEPTH	= 	SERVER_IMAGE_ATTR(ATTR_INT, 	1),
	__SERVER_IMAGE_BITS	= 	SERVER_IMAGE_ATTR(ATTR_OPAQUE, 	2),
	__SERVER_IMAGE_X_BITS	=	SERVER_IMAGE_ATTR(ATTR_OPAQUE, 	3),
	__SERVER_IMAGE_COLORMAP = 	SERVER_IMAGE_ATTR(ATTR_STRING,  4),
	__SERVER_IMAGE_BITMAP_FILE=     SERVER_IMAGE_ATTR(ATTR_STRING,  5),
	__SERVER_IMAGE_PIXMAP   =       SERVER_IMAGE_ATTR(ATTR_OPAQUE,  6),
	__SERVER_IMAGE_SAVE_PIXMAP=     SERVER_IMAGE_ATTR(ATTR_BOOLEAN, 7),
#ifdef  OW_I18N
        __SERVER_IMAGE_BITMAP_FILE_WCS= SERVER_IMAGE_ATTR(ATTR_WSTRING,  8),
#endif 
	__SERVER_IMAGE_CMS	=	SERVER_IMAGE_ATTR(ATTR_OPAQUE,  9)
} Server_image_attribute;

#define SERVER_IMAGE_DEPTH ((Attr_attribute) __SERVER_IMAGE_DEPTH)
#define SERVER_IMAGE_BITS ((Attr_attribute) __SERVER_IMAGE_BITS)
#define SERVER_IMAGE_X_BITS ((Attr_attribute) __SERVER_IMAGE_X_BITS)
#define SERVER_IMAGE_COLORMAP ((Attr_attribute) __SERVER_IMAGE_COLORMAP)
#define SERVER_IMAGE_BITMAP_FILE ((Attr_attribute) __SERVER_IMAGE_BITMAP_FILE)
#define SERVER_IMAGE_PIXMAP ((Attr_attribute) __SERVER_IMAGE_PIXMAP)
#define SERVER_IMAGE_SAVE_PIXMAP ((Attr_attribute) __SERVER_IMAGE_SAVE_PIXMAP)
#ifdef  OW_I18N
#define SERVER_IMAGE_BITMAP_FILE_WCS ((Attr_attribute) __SERVER_IMAGE_BITMAP_FILE_WCS)
#endif 
#define SERVER_IMAGE_CMS ((Attr_attribute) __SERVER_IMAGE_CMS)


typedef struct {
	Xv_drawable_struct	parent_data;
	Xv_opaque		private_data;
	Xv_embedding		embedding_data;
	Pixrect			pixrect;
}   Xv_server_image;

/*
 ***********************************************************************
 *				Globals
 ***********************************************************************
 */

Xv_public Xv_pkg  xv_server_image_pkg;

/*
 * PUBLIC functions 
 */
EXTERN_FUNCTION (int server_image_rop, (Xv_opaque dest, int dx,	int dy, int dw, int dh, unsigned long op, Xv_opaque src, int sx, int sy));
EXTERN_FUNCTION (int 	server_image_stencil, (Xv_opaque dest, int dx, int dy, int dw, int dh, unsigned long op, Xv_opaque st, int stx, int dty, Xv_opaque src, int sx, int sy));
EXTERN_FUNCTION (int 	server_image_destroy, (Pixrect *pr));
EXTERN_FUNCTION (int 	server_image_get, (Xv_opaque dest, int x, int y));
EXTERN_FUNCTION (int 	server_image_put, (Xv_opaque dest, int x, int y, int value));
EXTERN_FUNCTION (int 	server_image_vector, (Xv_opaque dest, int x0, int y0, int x1, int y1, int op, int value));
EXTERN_FUNCTION (Pixrect *server_image_region, (Xv_opaque dest, int x, int y, int w, int h));
EXTERN_FUNCTION (int 	server_image_colormap, (Xv_opaque dest, int index, int count, unsigned char *red, unsigned char *green, unsigned char *blue));
EXTERN_FUNCTION (int server_image_replrop, (Xv_opaque dest, int dx, int dy, int dw, int dh, unsigned long op, Xv_opaque src, int sx, int sy ));
EXTERN_FUNCTION (int server_image_pf_text, (struct pr_prpos rpr, int
op, Pixfont *font, char *string ));
#ifdef OW_I18N
EXTERN_FUNCTION (int server_image_pf_text_wc, (struct pr_prpos rpr, int
op, Pixfont *font, wchar_t *string ));
#endif /* OW_I18N */


#endif /* xview_server_image_DEFINED */
