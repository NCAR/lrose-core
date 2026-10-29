/*      @(#)font.h 20.35 93/06/28 SMI      */

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#ifndef xview_font_DEFINED
#define xview_font_DEFINED

/*
 ***********************************************************************
 *			Include files
 ***********************************************************************
 */

#include <xview/generic.h>
#ifdef OW_I18N
#include <xview/xv_i18n.h>
#endif /*OW_I18N*/


/*
 ***********************************************************************
 *			Definitions and Macros
 ***********************************************************************
 */

/*
 * Public #defines 
 */
#ifndef XV_ATTRIBUTES_ONLY

#ifdef OW_I18N
#define FONT_SET        &xv_font_pkg
#define FONT            FONT_SET
#else
#define FONT                    &xv_font_pkg
#endif /*OW_I18N*/


/*
 * font family/style values available 
 */
#define FONT_FAMILY_DEFAULT		"FONT_FAMILY_DEFAULT"
#define FONT_FAMILY_DEFAULT_FIXEDWIDTH	"FONT_FAMILY_DEFAULT_FIXEDWIDTH"

#define FONT_FAMILY_LUCIDA		"FONT_FAMILY_LUCIDA"
#define FONT_FAMILY_LUCIDA_FIXEDWIDTH	"FONT_FAMILY_LUCIDA_FIXEDWIDTH"

#define FONT_FAMILY_ROMAN	"FONT_FAMILY_ROMAN"
#define FONT_FAMILY_SERIF	"FONT_FAMILY_SERIF"
#define FONT_FAMILY_COUR	"FONT_FAMILY_COUR"
#define FONT_FAMILY_CMR		"FONT_FAMILY_CMR"
#define FONT_FAMILY_GALLENT	"FONT_FAMILY_GALLENT"
#define FONT_FAMILY_HELVETICA	"FONT_FAMILY_HELVETICA"
#define FONT_FAMILY_OLGLYPH	"FONT_FAMILY_OLGLYPH"
#define FONT_FAMILY_OLCURSOR	"FONT_FAMILY_OLCURSOR"

#ifdef OW_I18N
#define FONT_FAMILY_SANS_SERIF      "FONT_FAMILY_SANS_SERIF"
#endif /*OW_I18N*/

#define FONT_STYLE_DEFAULT	"FONT_STYLE_DEFAULT"
#define FONT_STYLE_NORMAL	"FONT_STYLE_NORMAL"
#define FONT_STYLE_BOLD		"FONT_STYLE_BOLD"
#define FONT_STYLE_ITALIC	"FONT_STYLE_ITALIC"
#define FONT_STYLE_OBLIQUE	"FONT_STYLE_OBLIQUE"
#define FONT_STYLE_BOLD_ITALIC	"FONT_STYLE_BOLD_ITALIC"
#define FONT_STYLE_BOLD_OBLIQUE	"FONT_STYLE_BOLD_OBLIQUE"

#define FONT_SIZE_DEFAULT	-99
#define FONT_NO_SIZE		-66
#define FONT_NO_SCALE		-55
#define FONT_SCALE_DEFAULT	-33

#endif	/* ~XV_ATTRIBUTES_ONLY */

/*
 * Private #defines 
 */
#define	FONT_ATTR(type, ordinal)	ATTR(ATTR_PKG_FONT, type, ordinal)
#define FONT_QUAD_ATTR			ATTR_TYPE(ATTR_BASE_INT, 4)

/*
 ***********************************************************************
 *		Typedefs, enumerations, and structs
 ***********************************************************************
 */

/*
 * Public types 
 */
#ifndef XV_ATTRIBUTES_ONLY

typedef	Xv_opaque 	Xv_Font;
typedef	Xv_opaque 	Xv_font;

#endif 	/* ~XV_ATTRIBUTES_ONLY */

typedef enum {
    /*
     * Public attributes. 
     */
    __FONT_CHAR_WIDTH	= FONT_ATTR(ATTR_CHAR,		 1),	/* G 	*/
    __FONT_CHAR_HEIGHT	= FONT_ATTR(ATTR_CHAR,		 5),	/* G 	*/

#ifdef OW_I18N
    __FONT_CHAR_WIDTH_WC  = FONT_ATTR(ATTR_WCHAR,          6),    /* G    */
    __FONT_CHAR_HEIGHT_WC = FONT_ATTR(ATTR_WCHAR,          7),    /* G    */
    __FONT_NAMES          = FONT_ATTR(ATTR_OPAQUE,        26),    /* C-G  */
    __FONT_SET_SPECIFIER  = FONT_ATTR(ATTR_STRING,        27),    /* C-G  */
#endif /*OW_I18N*/

    __FONT_DEFAULT_CHAR_HEIGHT
    			= FONT_ATTR(ATTR_NO_VALUE,	10),	/* G 	*/
    __FONT_DEFAULT_CHAR_WIDTH
    			= FONT_ATTR(ATTR_NO_VALUE,	15),	/* G 	*/
    __FONT_FAMILY	= FONT_ATTR(ATTR_STRING,	20),	/* C-G 	*/
    __FONT_NAME		= FONT_ATTR(ATTR_STRING,	25),	/* C-G 	*/
    __FONT_RESCALE_OF	= FONT_ATTR(ATTR_OPAQUE_PAIR,	30),	/* F-C 	*/
    __FONT_SCALE	= FONT_ATTR(ATTR_INT,		40),	/* C-G 	*/
    __FONT_SIZE		= FONT_ATTR(ATTR_INT,		45),	/* C-G 	*/
    __FONT_SIZES_FOR_SCALE= ATTR(ATTR_PKG_FONT,
    					FONT_QUAD_ATTR, 50),	/* C-S-G*/
    __FONT_STRING_DIMS	= FONT_ATTR(ATTR_OPAQUE_PAIR,	55),	/* G 	*/

#ifdef OW_I18N
    __FONT_STRING_DIMS_WC = FONT_ATTR(ATTR_OPAQUE_PAIR,   56),    /* G    */
#endif /*OW_I18N*/

    __FONT_STYLE	= FONT_ATTR(ATTR_STRING,	60),	/* C-G 	*/
    __FONT_TYPE		= FONT_ATTR(ATTR_ENUM,		65),	/* C-S-G */
    __FONT_PIXFONT	= FONT_ATTR(ATTR_OPAQUE,	67),	/* G */
    __FONT_INFO		= FONT_ATTR(ATTR_OPAQUE,	80),	/* G    */

#ifdef OW_I18N
    __FONT_LOCALE       = FONT_ATTR(ATTR_STRING,        68),    /* C-G */
    __FONT_SET_ID       = FONT_ATTR(ATTR_OPAQUE,        69),    /* G    */
    __FONT_COLUMN_WIDTH	= FONT_ATTR(ATTR_NO_VALUE,	85),	/* G   */
#endif /*OW_I18N*/

    /*
     * Private attributes. 
     */
    __FONT_HEAD		= FONT_ATTR(ATTR_INT,		70),	/* Key 	*/
    __FONT_UNKNOWN_HEAD	= FONT_ATTR(ATTR_INT,		75)	/* Key 	*/

} Font_attribute;

#define FONT_CHAR_WIDTH ((Attr_attribute) __FONT_CHAR_WIDTH)
#define FONT_CHAR_HEIGHT ((Attr_attribute) __FONT_CHAR_HEIGHT)
#ifdef OW_I18N
#define FONT_CHAR_WIDTH_WC ((Attr_attribute) __FONT_CHAR_WIDTH_WC)
#define FONT_CHAR_HEIGHT_WC ((Attr_attribute) __FONT_CHAR_HEIGHT_WC)
#define FONT_NAMES ((Attr_attribute) __FONT_NAMES)
#define FONT_SET_SPECIFIER ((Attr_attribute) __FONT_SET_SPECIFIER)
#endif /*OW_I18N*/
#define FONT_DEFAULT_CHAR_HEIGHT ((Attr_attribute) __FONT_DEFAULT_CHAR_HEIGHT)
#define FONT_DEFAULT_CHAR_WIDTH ((Attr_attribute) __FONT_DEFAULT_CHAR_WIDTH)
#define FONT_FAMILY ((Attr_attribute) __FONT_FAMILY)
#define FONT_NAME ((Attr_attribute) __FONT_NAME)
#define FONT_RESCALE_OF ((Attr_attribute) __FONT_RESCALE_OF)
#define FONT_SCALE ((Attr_attribute) __FONT_SCALE)
#define FONT_SIZE ((Attr_attribute) __FONT_SIZE)
#define FONT_SIZES_FOR_SCALE ((Attr_attribute) __FONT_SIZES_FOR_SCALE)
#define FONT_STRING_DIMS ((Attr_attribute) __FONT_STRING_DIMS)
#ifdef OW_I18N
#define FONT_STRING_DIMS_WC ((Attr_attribute) __FONT_STRING_DIMS_WC)
#endif /*OW_I18N*/
#define FONT_STYLE ((Attr_attribute) __FONT_STYLE)
#define FONT_TYPE ((Attr_attribute) __FONT_TYPE)
#define FONT_PIXFONT ((Attr_attribute) __FONT_PIXFONT)
#define FONT_INFO ((Attr_attribute) __FONT_INFO)
#ifdef OW_I18N
#define FONT_LOCALE ((Attr_attribute) __FONT_LOCALE)
#define FONT_SET_ID ((Attr_attribute) __FONT_SET_ID)
#define FONT_COLUMN_WIDTH ((Attr_attribute) __FONT_COLUMN_WIDTH)
#endif /*OW_I18N*/
#define FONT_HEAD ((Attr_attribute) __FONT_HEAD)
#define FONT_UNKNOWN_HEAD ((Attr_attribute) __FONT_UNKNOWN_HEAD)

typedef enum {
	FONT_TYPE_TEXT = 0,
	FONT_TYPE_CURSOR = 1,
	FONT_TYPE_GLYPH = 2
} Font_type;

#ifndef XV_ATTRIBUTES_ONLY

/*
 * Got rid of pixfont struct
 * It now exists in the private part where it will be allocated on demand
 */
typedef struct {
    Xv_generic_struct 	parent_data;
    Xv_opaque	 	private_data;
    /*
    Xv_embedding	embedding_data;
    char		*pixfont[2+(5*256)];
    */
} Xv_font_struct;

typedef struct {
    int     		width, height;
} Font_string_dims;

#endif /* ~XV_ATTRIBUTES_ONLY */

/*
 ***********************************************************************
 *			Globals
 ***********************************************************************
 */

#ifndef XV_ATTRIBUTES_ONLY
extern Xv_pkg		xv_font_pkg;
#endif /* ~XV_ATTRIBUTES_ONLY */

#endif /* ~xview_font_DEFINED */
