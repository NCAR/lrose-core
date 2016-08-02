/*      @(#)notice.h 20.23 91/01/11  */

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#ifndef xview_notice_DEFINED
#define xview_notice_DEFINED

/*
 ***********************************************************************
 *			Include Files
 ***********************************************************************
 */

#include <xview/xv_c_types.h>
#include <xview/attrol.h>
#include <xview/generic.h>
#include <xview/window.h>

/*
 ***********************************************************************
 *			Definitions and Macros	
 ***********************************************************************
 */

/*
 * PUBLIC #defines 
 */
#define NOTICE			&xv_notice_pkg

/*
 * the following constant, NOTICE_FAILED is returned if notice_prompt() 
 * failed for an unspecified reason.
 */
#define NOTICE_YES			 1
#define NOTICE_NO			 0
#define NOTICE_FAILED			-1
#define NOTICE_TRIGGERED		-2

/*
 * PRIVATE #defines 
 */

#define NOTICE_ATTR(type, ordinal)	ATTR(ATTR_PKG_NOTICE, type, ordinal)
#define NOTICE_ATTR_LIST(ltype, type, ordinal) \
	NOTICE_ATTR(ATTR_LIST_INLINE((ltype), (type)), (ordinal))
#define NOTICE_BUTTON_VALUE_PAIR	ATTR_INT_PAIR


/*
 ***********************************************************************
 *		Typedefs, enumerations, and structs
 ***********************************************************************
 */

typedef Xv_opaque	Xv_Notice;
typedef Xv_opaque	Xv_notice;

typedef enum {
	/*
	 * Public attributes 
	 */
	__NOTICE_BUTTON		= NOTICE_ATTR(NOTICE_BUTTON_VALUE_PAIR,	 1),
	__NOTICE_BUTTON_NO	= NOTICE_ATTR(ATTR_STRING,		 5),
	__NOTICE_BUTTON_YES	= NOTICE_ATTR(ATTR_STRING,		10),
	__NOTICE_FOCUS_XY	= NOTICE_ATTR(ATTR_XY,			15),
	__NOTICE_FONT		= NOTICE_ATTR(ATTR_PIXFONT_PTR,		20),
	__NOTICE_MESSAGE_STRINGS	
			= NOTICE_ATTR_LIST(ATTR_NULL, ATTR_STRING,	25),
	__NOTICE_MESSAGE_STRINGS_ARRAY_PTR = NOTICE_ATTR(ATTR_STRING,	30),
	__NOTICE_NO_BEEPING	= NOTICE_ATTR(ATTR_BOOLEAN,	 	35),
	__NOTICE_TRIGGER	= NOTICE_ATTR(ATTR_INT,			40),
	__NOTICE_MESSAGE_STRING = NOTICE_ATTR(ATTR_STRING,              45),
#ifdef	OW_I18N
	__NOTICE_MESSAGE_STRINGS_WCS	
			= NOTICE_ATTR_LIST(ATTR_NULL, ATTR_WSTRING,	50),
	__NOTICE_MESSAGE_STRINGS_ARRAY_PTR_WCS
			= NOTICE_ATTR(ATTR_WSTRING,			55),
	__NOTICE_MESSAGE_STRING_WCS
				= NOTICE_ATTR(ATTR_WSTRING,		60),
	__NOTICE_BUTTON_WCS	= NOTICE_ATTR(NOTICE_BUTTON_VALUE_PAIR,	65),
	__NOTICE_BUTTON_NO_WCS	= NOTICE_ATTR(ATTR_WSTRING,		70),
	__NOTICE_BUTTON_YES_WCS	= NOTICE_ATTR(ATTR_WSTRING,		75),
#endif
	__NOTICE_LOCK_SCREEN	= NOTICE_ATTR(ATTR_BOOLEAN,		80),
	__NOTICE_TRIGGER_EVENT	= NOTICE_ATTR(ATTR_INT,			85),
	__NOTICE_STATUS		= NOTICE_ATTR(ATTR_OPAQUE,		95),
	__NOTICE_EVENT_PROC	= NOTICE_ATTR(ATTR_FUNCTION_PTR,	100),
	__NOTICE_BUSY_FRAMES = NOTICE_ATTR_LIST(ATTR_NULL, ATTR_OPAQUE,	105),
	__NOTICE_BLOCK_THREAD	= NOTICE_ATTR(ATTR_BOOLEAN,		110)
} Notice_attribute;


#define NOTICE_BUTTON ((Attr_attribute) __NOTICE_BUTTON)
#define NOTICE_BUTTON_NO ((Attr_attribute) __NOTICE_BUTTON_NO)
#define NOTICE_BUTTON_YES ((Attr_attribute) __NOTICE_BUTTON_YES)
#define NOTICE_FOCUS_XY ((Attr_attribute) __NOTICE_FOCUS_XY)
#define NOTICE_FONT ((Attr_attribute) __NOTICE_FONT)
#define NOTICE_MESSAGE_STRINGS ((Attr_attribute) __NOTICE_MESSAGE_STRINGS)
#define NOTICE_MESSAGE_STRINGS_ARRAY_PTR ((Attr_attribute) __NOTICE_MESSAGE_STRINGS_ARRAY_PTR)
#define NOTICE_NO_BEEPING ((Attr_attribute) __NOTICE_NO_BEEPING)
#define NOTICE_TRIGGER ((Attr_attribute) __NOTICE_TRIGGER)
#define NOTICE_MESSAGE_STRING ((Attr_attribute) __NOTICE_MESSAGE_STRING)
#ifdef	OW_I18N
#define NOTICE_MESSAGE_STRINGS_WCS ((Attr_attribute) __NOTICE_MESSAGE_STRINGS_WCS)
#define NOTICE_MESSAGE_STRINGS_ARRAY_PTR_WCS ((Attr_attribute) __NOTICE_MESSAGE_STRINGS_ARRAY_PTR_WCS)
#define NOTICE_MESSAGE_STRING_WCS ((Attr_attribute) __NOTICE_MESSAGE_STRING_WCS)
#define NOTICE_BUTTON_WCS ((Attr_attribute) __NOTICE_BUTTON_WCS)
#define NOTICE_BUTTON_NO_WCS ((Attr_attribute) __NOTICE_BUTTON_NO_WCS)
#define NOTICE_BUTTON_YES_WCS ((Attr_attribute) __NOTICE_BUTTON_YES_WCS)
#endif
#define NOTICE_LOCK_SCREEN ((Attr_attribute) __NOTICE_LOCK_SCREEN)
#define NOTICE_TRIGGER_EVENT ((Attr_attribute) __NOTICE_TRIGGER_EVENT)
#define NOTICE_STATUS ((Attr_attribute) __NOTICE_STATUS)
#define NOTICE_EVENT_PROC ((Attr_attribute) __NOTICE_EVENT_PROC)
#define NOTICE_BUSY_FRAMES ((Attr_attribute) __NOTICE_BUSY_FRAMES)
#define NOTICE_BLOCK_THREAD ((Attr_attribute) __NOTICE_BLOCK_THREAD)

/*
 * Notice public struct
 */
typedef struct  {
    Xv_generic_struct	parent_data;
    Xv_opaque		private_data;
} Xv_notice_struct;

/*
 ***********************************************************************
 *				Globals
 ***********************************************************************
 */

/*
 * Public Functions 
 */
EXTERN_FUNCTION (int		notice_prompt, (Xv_Window window,
					Event *return_event,
					DOTDOTDOT));

extern Xv_pkg		xv_notice_pkg;

#endif /* ~xview_notice_DEFINED */
