/*      @(#)generic.h 20.45 93/06/28 SMI      */

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#ifndef xview_generic_DEFINED
#define xview_generic_DEFINED

/*
 * Generic attributes fall into two classes:
 *	(1) Truly generic, implemented by attr_xxx.o or generic.o, use the
 * package ATTR_PKG_GENERIC, shared with attr.h.
 *	(2) Common but not truly generic, implemented by .o's spread
 * across many sub-systems, use the package ATTR_PKG_SV, shared with xview.h.
 * Many of these common attributes pertain to server properties and thus only
 * apply to objects with a window server component.
 *
 * Implementation dependent notes on generic X attributes:
 *	XV_XNAME has the format
 * "<host name>:<display number in decimal>:<xid in decimal>".
 *	XV_DEVICE_NUMBER is the XID of the underlying X object.  XV_XID is
 * provided when a piece of code wants to emphasize that the "X id" is what
 * is needed, rather than an abstract "tree link".
 * 	Most of these attributes are only supported on Drawable objects,
 * but some, like XV_XID, are supported by all objects that have direct
 * underlying X components, e.g. Fonts.
 */

/*
 ***********************************************************************
 *			Include Files
 ***********************************************************************
 */


#include <xview/pkg_public.h>

/*
 ***********************************************************************
 *			Definitions and Macros
 ***********************************************************************
 */

/*
 * PUBLIC #defines 
 */

#define XV_GENERIC_OBJECT	&xv_generic_pkg

/* XV_COPY is "magic" package xv_create checks for to distinguish
 * creation of a new object from creation of a copy of an existing object.
 */
#define XV_COPY			(Xv_pkg *)1

/*
 * Accelerator for XV_HELP and HELP_STRING_FILENAME
 */
#define XV_HELP_DATA 		XV_KEY_DATA, XV_HELP
#define HELP_STRING_FILENAME	XV_KEY_DATA, XV_HELP_STRING_FILENAME

#define XV_XID			XV_DEVICE_NUMBER

#define	XV_DUMMY_WINDOW		0x77777777

/*
 * Focus Client Rank.  Value is of type Xv_focus_rank.
 * Referred to in the Mouseless Model Specification as "Focus Client Classes".
 * Transient and Limited focus classes are Window Manager objects.
 * An Ordinary Focus client has an XV_FOCUS_RANK of XV_FOCUS_SECONDARY.
 * A First Class Focus client has an XV_FOCUS_RANK of XV_FOCUS_PRIMARY.
 */
#define XV_FOCUS_RANK		XV_KEY_DATA, XV_FOCUS_RANK_KEY

#define	XV_RC_SPECIAL		0x20000
#define	XV_RESET_REF_COUNT	XV_REF_COUNT, XV_RC_SPECIAL
#define	XV_INCREMENT_REF_COUNT	XV_REF_COUNT, XV_RC_SPECIAL+1
#define	XV_DECREMENT_REF_COUNT	XV_REF_COUNT, XV_RC_SPECIAL-1


/*
 * PRIVATE #defines 
 */
#define	XV_ATTR(type, ordinal)	ATTR(ATTR_PKG_SV, type, ordinal)

#define XV_ATTR_LIST(ltype, type, ordinal) \
        XV_ATTR(ATTR_LIST_INLINE((ltype), (type)), (ordinal))


/*
 ***********************************************************************
 *		Typedefs, enumerations, and structs
 ***********************************************************************
 */

typedef enum {
    XV_FOCUS_SECONDARY = 0,	/* default value (a.k.a., Ordinary Focus) */
    XV_FOCUS_PRIMARY = 1	/* a.k.a., First Class Focus */
} Xv_focus_rank;

/*
 * WARNING: GENERIC_ATTR shared with attr.h (which claims [0..64)) 
 */
typedef enum {
	/*
	 * PUBLIC and Generic 
	 */
	/*
	 * For "contexts": key & data (& optional destroy for data) 
	 */
	__XV_KEY_DATA		= GENERIC_ATTR(ATTR_INT_PAIR,	 64),
	__XV_KEY_DATA_COPY_PROC	= GENERIC_ATTR(ATTR_OPAQUE_PAIR, 65),
	__XV_KEY_DATA_REMOVE	= GENERIC_ATTR(ATTR_INT,	 66), /* -S- */
	__XV_KEY_DATA_REMOVE_PROC = GENERIC_ATTR(ATTR_OPAQUE_PAIR, 67),
	/*
	 * For "reference count" on shared objects, e.g. fonts & menus 
	 */
	__XV_REF_COUNT		= GENERIC_ATTR(ATTR_INT,	 68),
	/*
	 * Type of object 
	 */
	__XV_TYPE 		= GENERIC_ATTR(ATTR_OPAQUE,	 69), /* --G */
	__XV_IS_SUBTYPE_OF 	= GENERIC_ATTR(ATTR_OPAQUE,	 70), /* --G */
	/*
	 * Miscellaneous 
	 */
	__XV_LABEL		= GENERIC_ATTR(ATTR_STRING,	 71),
	__XV_NAME			= GENERIC_ATTR(ATTR_STRING,	 72),
	__XV_STATUS		= GENERIC_ATTR(ATTR_INT,	 73),
	__XV_STATUS_PTR		= GENERIC_ATTR(ATTR_OPAQUE,	 74),
	__XV_HELP			= GENERIC_ATTR(ATTR_STRING,	 80),
	__XV_HELP_STRING_FILENAME	= GENERIC_ATTR(ATTR_STRING,	 82),
	__XV_SHOW			= GENERIC_ATTR(ATTR_BOOLEAN,	 81),
#ifdef OW_I18N
        __XV_LABEL_WCS            = GENERIC_ATTR(ATTR_WSTRING,     164),
        __XV_NAME_WCS             = GENERIC_ATTR(ATTR_WSTRING,     161),
        __XV_HELP_WCS             = GENERIC_ATTR(ATTR_WSTRING,     162),
        __XV_HELP_STRING_FILENAME_WCS
                                = GENERIC_ATTR(ATTR_WSTRING,     163),
#endif /* OW_I18N */
	/*
	 * Required by package implementations, used only by xv_create 
	 */
	__XV_COPY_OF		= GENERIC_ATTR(ATTR_OPAQUE,	 75), /* -S- */
	__XV_END_CREATE		= GENERIC_ATTR(ATTR_NO_VALUE,	 76), /* -S- */
	/*
	 * To simplify SunView1.X compatibility 
	 */
	__XV_SELF			= GENERIC_ATTR(ATTR_OPAQUE,	 77), /* --G */
	/*
	 * Managing (usually containing) object 
	 */
	__XV_OWNER		= GENERIC_ATTR(ATTR_OPAQUE,	 78),
    	/*
	 * Required by package implementations, used only by xv_find 
	 */
	__XV_AUTO_CREATE		= GENERIC_ATTR(ATTR_INT,	 79), /* C-- */
	/*
	 * PUBLIC but only Common 
	 */
	/*
	 * For layout 
	 */
	__XV_FONT			= XV_ATTR(ATTR_OPAQUE,		 64),
	__XV_MARGIN		= XV_ATTR(ATTR_INT,		 65),
	__XV_LEFT_MARGIN		= XV_ATTR(ATTR_INT,		 66),
	__XV_TOP_MARGIN		= XV_ATTR(ATTR_INT,		 67),
	__XV_RIGHT_MARGIN		= XV_ATTR(ATTR_INT,		 68),
	__XV_BOTTOM_MARGIN	= XV_ATTR(ATTR_INT,		 69),
	/*
	 * Origin is usually parent's most upper-left coord inside margins 
	 */
	__XV_X			= XV_ATTR(ATTR_X,		 70),
	__XV_Y			= XV_ATTR(ATTR_Y,		 71),
	__XV_WIDTH		= XV_ATTR(ATTR_X,		 72),
	__XV_HEIGHT		= XV_ATTR(ATTR_Y,		 73),
	__XV_RECT			= XV_ATTR(ATTR_RECT_PTR,	 74),
	/*
	 * Server specific or dependent 
	 */
	__XV_XNAME		= XV_ATTR(ATTR_STRING,		 96), /* C-G */
	__XV_DEVICE_NUMBER	= XV_ATTR(ATTR_LONG,		 97), /* C-G */
	__XV_ROOT			= XV_ATTR(ATTR_OPAQUE,		 98), /* --G */
	__XV_VISUAL		= XV_ATTR(ATTR_OPAQUE,          125), /* C-G */
	__XV_VISUAL_CLASS		= XV_ATTR(ATTR_INT,		117), /* C-G */
	__XV_DEPTH		= XV_ATTR(ATTR_INT,		126), /* C-G */
	__XV_DISPLAY		= XV_ATTR(ATTR_OPAQUE,		110), /* --G */
	__XV_SCREEN		= XV_ATTR(ATTR_OPAQUE,		116), /* --G */
	__XV_APP_NAME		= XV_ATTR(ATTR_STRING,		112), /* -SG */
#ifdef OW_I18N
	__XV_APP_NAME_WCS		= XV_ATTR(ATTR_STRING,		111), /* CSG */
#endif
	/*
	 * Mouseless Model support
	 */
	__XV_FOCUS_ELEMENT	= XV_ATTR(ATTR_INT,		118), /* -S- */
	__XV_FOCUS_RANK_KEY	= XV_ATTR(ATTR_ENUM,		119), /* CSG */

	/*
	 * Added to support the Xrm resource database
	 */
	__XV_USE_DB		= XV_ATTR_LIST(ATTR_RECURSIVE, ATTR_AV,	120),
	__XV_INSTANCE_NAME	= XV_ATTR(ATTR_STRING,		125),
	__XV_INSTANCE_QLIST	= XV_ATTR(ATTR_OPAQUE,		130),
	__XV_QUARK		= XV_ATTR(ATTR_OPAQUE,		135),
	__XV_USE_INSTANCE_RESOURCES=XV_ATTR(ATTR_OPAQUE,		140),

#ifdef OW_I18N
	/*
	 * The I18N Level 4 attribute XV_IM goes here:
	 */
	 __XV_IM                   = XV_ATTR(ATTR_OPAQUE,          150),
#endif /* OW_I18N */

	/*
	 * Added to support locale announcement
	 */
	__XV_LC_BASIC_LOCALE	= XV_ATTR(ATTR_STRING, 		155),
	__XV_LC_DISPLAY_LANG	= XV_ATTR(ATTR_STRING, 		160),
	__XV_LC_INPUT_LANG	= XV_ATTR(ATTR_STRING, 		165),
	__XV_LC_NUMERIC		= XV_ATTR(ATTR_STRING, 		170),
	__XV_LC_TIME_FORMAT	= XV_ATTR(ATTR_STRING,		175),
 	__XV_LOCALE_DIR		= XV_ATTR(ATTR_STRING, 		180),
	__XV_USE_LOCALE		= XV_ATTR(ATTR_BOOLEAN,		185),
#ifdef OW_I18N
#ifdef FULL_R5
	__XV_IM_STYLES		= XV_ATTR(ATTR_OPAQUE,		186),
	__XV_IM_PREEDIT_STYLE	= XV_ATTR(ATTR_STRING, 		190),
	__XV_IM_STATUS_STYLE	= XV_ATTR(ATTR_STRING,		195),
#endif
#endif /* OW_I18N */

	/*
	 * PRIVATE now, but ... 
	 */
	__XV_GC			= XV_ATTR(ATTR_OPAQUE,		113)  /* --G */
} Xv_generic_attr;

#define XV_KEY_DATA ((Attr_attribute) __XV_KEY_DATA)
#define XV_KEY_DATA_COPY_PROC ((Attr_attribute) __XV_KEY_DATA_COPY_PROC)
#define XV_KEY_DATA_REMOVE ((Attr_attribute) __XV_KEY_DATA_REMOVE)
#define XV_KEY_DATA_REMOVE_PROC ((Attr_attribute) __XV_KEY_DATA_REMOVE_PROC)
#define XV_REF_COUNT ((Attr_attribute) __XV_REF_COUNT)
#define XV_TYPE ((Attr_attribute) __XV_TYPE)
#define XV_IS_SUBTYPE_OF ((Attr_attribute) __XV_IS_SUBTYPE_OF)
#define XV_LABEL ((Attr_attribute) __XV_LABEL)
#define XV_NAME ((Attr_attribute) __XV_NAME)
#define XV_STATUS ((Attr_attribute) __XV_STATUS)
#define XV_STATUS_PTR ((Attr_attribute) __XV_STATUS_PTR)
#define XV_HELP ((Attr_attribute) __XV_HELP)
#define XV_HELP_STRING_FILENAME ((Attr_attribute) __XV_HELP_STRING_FILENAME)
#define XV_SHOW ((Attr_attribute) __XV_SHOW)
#define XV_LABEL_WCS ((Attr_attribute) __XV_LABEL_WCS)
#define XV_NAME_WCS ((Attr_attribute) __XV_NAME_WCS)
#define XV_HELP_WCS ((Attr_attribute) __XV_HELP_WCS)
#define XV_HELP_STRING_FILENAME_WCS ((Attr_attribute) __XV_HELP_STRING_FILENAME_WCS)
#define XV_COPY_OF ((Attr_attribute) __XV_COPY_OF)
#define XV_END_CREATE ((Attr_attribute) __XV_END_CREATE)
#define XV_SELF ((Attr_attribute) __XV_SELF)
#define XV_OWNER ((Attr_attribute) __XV_OWNER)
#define XV_AUTO_CREATE ((Attr_attribute) __XV_AUTO_CREATE)
#define XV_FONT ((Attr_attribute) __XV_FONT)
#define XV_MARGIN ((Attr_attribute) __XV_MARGIN)
#define XV_LEFT_MARGIN ((Attr_attribute) __XV_LEFT_MARGIN)
#define XV_TOP_MARGIN ((Attr_attribute) __XV_TOP_MARGIN)
#define XV_RIGHT_MARGIN ((Attr_attribute) __XV_RIGHT_MARGIN)
#define XV_BOTTOM_MARGIN ((Attr_attribute) __XV_BOTTOM_MARGIN)
#define XV_X ((Attr_attribute) __XV_X)
#define XV_Y ((Attr_attribute) __XV_Y)
#define XV_WIDTH ((Attr_attribute) __XV_WIDTH)
#define XV_HEIGHT ((Attr_attribute) __XV_HEIGHT)
#define XV_RECT ((Attr_attribute) __XV_RECT)
#define XV_XNAME ((Attr_attribute) __XV_XNAME)
#define XV_DEVICE_NUMBER ((Attr_attribute) __XV_DEVICE_NUMBER)
#define XV_ROOT ((Attr_attribute) __XV_ROOT)
#define XV_VISUAL ((Attr_attribute) __XV_VISUAL)
#define XV_VISUAL_CLASS ((Attr_attribute) __XV_VISUAL_CLASS)
#define XV_DEPTH ((Attr_attribute) __XV_DEPTH)
#define XV_DISPLAY ((Attr_attribute) __XV_DISPLAY)
#define XV_SCREEN ((Attr_attribute) __XV_SCREEN)
#define XV_APP_NAME ((Attr_attribute) __XV_APP_NAME)
#define XV_APP_NAME_WCS ((Attr_attribute) __XV_APP_NAME_WCS)
#define XV_FOCUS_ELEMENT ((Attr_attribute) __XV_FOCUS_ELEMENT)
#define XV_FOCUS_RANK_KEY ((Attr_attribute) __XV_FOCUS_RANK_KEY)
#define XV_USE_DB ((Attr_attribute) __XV_USE_DB)
#define XV_INSTANCE_NAME ((Attr_attribute) __XV_INSTANCE_NAME)
#define XV_INSTANCE_QLIST ((Attr_attribute) __XV_INSTANCE_QLIST)
#define XV_QUARK ((Attr_attribute) __XV_QUARK)
#define XV_USE_INSTANCE_RESOURCES ((Attr_attribute) __XV_USE_INSTANCE_RESOURCES)
#define XV_IM ((Attr_attribute) __XV_IM)
#define XV_LC_BASIC_LOCALE ((Attr_attribute) __XV_LC_BASIC_LOCALE)
#define XV_LC_DISPLAY_LANG ((Attr_attribute) __XV_LC_DISPLAY_LANG)
#define XV_LC_INPUT_LANG ((Attr_attribute) __XV_LC_INPUT_LANG)
#define XV_LC_NUMERIC ((Attr_attribute) __XV_LC_NUMERIC)
#define XV_LC_TIME_FORMAT ((Attr_attribute) __XV_LC_TIME_FORMAT)
#define XV_LOCALE_DIR ((Attr_attribute) __XV_LOCALE_DIR)
#define XV_USE_LOCALE ((Attr_attribute) __XV_USE_LOCALE)
#define XV_IM_STYLES ((Attr_attribute) __XV_IM_STYLES)
#define XV_IM_PREEDIT_STYLE ((Attr_attribute) __XV_IM_PREEDIT_STYLE)
#define XV_IM_STATUS_STYLE ((Attr_attribute) __XV_IM_STATUS_STYLE)
#define XV_GC ((Attr_attribute) __XV_GC)


/*
 * Generic package definition	
 */
typedef struct {
    Xv_base	parent_data;
    Xv_opaque	private_data;
} Xv_generic_struct;

typedef enum {
/* Alpha compatibility, mbuck@debian.org */
#if defined(__alpha)
    __XV_INIT_ARGS             = XV_ATTR(ATTR_OPAQUE_PAIR,       	4),
    __XV_INIT_ARGC_PTR_ARGV    = XV_ATTR(ATTR_OPAQUE_PAIR,       	7),  /* -S- */
#else
    __XV_INIT_ARGS             = XV_ATTR(ATTR_INT_PAIR,         	4),
    __XV_INIT_ARGC_PTR_ARGV    = XV_ATTR(ATTR_INT_PAIR,         	7),  /* -S- */
#endif
    __XV_USAGE_PROC       = XV_ATTR(ATTR_FUNCTION_PTR,     	9),  /* -S- */
    __XV_ERROR_PROC       = XV_ATTR(ATTR_FUNCTION_PTR,    	12),
    __XV_X_ERROR_PROC	= XV_ATTR(ATTR_FUNCTION_PTR,    	15)
} Xv_attr;

#define XV_INIT_ARGS ((Attr_attribute) __XV_INIT_ARGS)
#define XV_INIT_ARGC_PTR_ARGV ((Attr_attribute) __XV_INIT_ARGC_PTR_ARGV)
#define XV_USAGE_PROC ((Attr_attribute) __XV_USAGE_PROC)
#define XV_ERROR_PROC ((Attr_attribute) __XV_ERROR_PROC)
#define XV_X_ERROR_PROC ((Attr_attribute) __XV_X_ERROR_PROC)

/*
 ***********************************************************************
 *				Globals
 ***********************************************************************
 */

extern Xv_pkg		xv_generic_pkg;

/*
 * PUBLIC functions 
 */

EXTERN_FUNCTION (Xv_object xv_init, (Attr_attribute attr1, DOTDOTDOT));
EXTERN_FUNCTION (Attr_attribute xv_unique_key, (void));

#endif /* ~xview_generic_DEFINED */
