/*	@(#)sel_attrs.h 20.22 93/06/28	*/

#ifndef	xview_selection_attributes_DEFINED
#define	xview_selection_attributes_DEFINED

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 ***********************************************************************
 *			Include Files
 ***********************************************************************
 */

#include <xview/attr.h>

/*
 ***********************************************************************
 *			Definitions and Macros
 ***********************************************************************
 */

/*
 * PRIVATE #defines 
 */

/*
 *	Common requests a client may send to a selection-holder
 */
#define ATTR_PKG_SELN	ATTR_PKG_SELN_BASE

#define SELN_ATTR(type, n)	ATTR(ATTR_PKG_SELN, type, n)

#define SELN_ATTR_LIST(list_type, type, n)	\
	ATTR(ATTR_PKG_SELN, ATTR_LIST_INLINE(list_type, type), n)

/*
 ***********************************************************************
 *		Typedefs, Enumerations, and Structures
 ***********************************************************************
 */

/*
 * Public Enumerations 
 */

/*
 *	Attributes of selections
 * 	The numbering scheme has to match the scheme of Sunview selection_attributes.h
 */
typedef enum	{
	/*
	 * Public Attributes 
	 */
	__SELN_REQ_BYTESIZE	= SELN_ATTR(ATTR_INT,		         1),
	__SELN_REQ_COMMIT_PENDING_DELETE	
				= SELN_ATTR(ATTR_NO_VALUE,	         65),
	__SELN_REQ_CONTENTS_ASCII= SELN_ATTR_LIST(ATTR_NULL, ATTR_CHAR,  2),

#ifdef OW_I18N
	__SELN_REQ_CONTENTS_WCS = SELN_ATTR_LIST(ATTR_NULL, ATTR_WCHAR, 202),
        __SELN_REQ_CHARSIZE     = SELN_ATTR(ATTR_INT,                   204),
	__SELN_REQ_FIRST_WC	= SELN_ATTR(ATTR_INT,		        205),
	__SELN_REQ_LAST_WC	= SELN_ATTR(ATTR_INT,		        206),
#endif /*OW_I18N*/

	__SELN_REQ_CONTENTS_PIECES= SELN_ATTR_LIST(ATTR_NULL, ATTR_CHAR,  3),
	__SELN_REQ_DELETE	= SELN_ATTR(ATTR_NO_VALUE,	        66),
	__SELN_REQ_END_REQUEST	= SELN_ATTR(ATTR_NO_VALUE,	        253),
	__SELN_REQ_FAILED	= SELN_ATTR(ATTR_INT,		        255),
	__SELN_REQ_FAKE_LEVEL	= SELN_ATTR(ATTR_INT,		        98),
	__SELN_REQ_FILE_NAME	= SELN_ATTR_LIST(ATTR_NULL, ATTR_CHAR,  9),
	__SELN_REQ_FIRST	= SELN_ATTR(ATTR_INT,		        4),
	__SELN_REQ_FIRST_UNIT	= SELN_ATTR(ATTR_INT,		        5),
	__SELN_REQ_LAST		= SELN_ATTR(ATTR_INT,		        6),
	__SELN_REQ_LAST_UNIT	= SELN_ATTR(ATTR_INT,		        7),
	__SELN_REQ_LEVEL	= SELN_ATTR(ATTR_INT,		        8),
	__SELN_REQ_RESTORE	= SELN_ATTR(ATTR_NO_VALUE,	        67),
	__SELN_REQ_SET_LEVEL	= SELN_ATTR(ATTR_INT,		        99),
	__SELN_REQ_UNKNOWN	= SELN_ATTR(ATTR_INT,		        254),
	__SELN_REQ_YIELD	= SELN_ATTR(ATTR_ENUM,		        97),
	/*
	 * Private Attributes 
	 */
#ifdef OW_I18N
        __SELN_REQ_CONTENTS_CT  = SELN_ATTR_LIST(ATTR_NULL, ATTR_CHAR,  203),
#endif /*OW_I18N*/

	__SELN_AGENT_INFO	= SELN_ATTR(ATTR_OPAQUE,                100),
	__SELN_REQ_FUNC_KEY_STATE= SELN_ATTR(ATTR_INT,		 	101),
	__SELN_REQ_SELECTED_WINDOWS= SELN_ATTR_LIST(ATTR_NULL, ATTR_INT, 102),
	__SELN_REQ_CONTENTS_OBJECT= SELN_ATTR_LIST(ATTR_NULL, ATTR_CHAR, 103),
	__SELN_REQ_OBJECT_SIZE	= SELN_ATTR(ATTR_INT, 			104),
	__SELN_REQ_IS_READONLY	= SELN_ATTR(ATTR_BOOLEAN,	       105),
	__SELN_TRACE_ACQUIRE	= SELN_ATTR(ATTR_BOOLEAN,	       193),
	__SELN_TRACE_DONE	= SELN_ATTR(ATTR_BOOLEAN,	       194),
	__SELN_TRACE_DUMP	= SELN_ATTR(ATTR_ENUM,		       200),
	__SELN_TRACE_HOLD_FILE	= SELN_ATTR(ATTR_BOOLEAN,	       195),
	__SELN_TRACE_INFORM	= SELN_ATTR(ATTR_BOOLEAN,	       196),
	__SELN_TRACE_INQUIRE	= SELN_ATTR(ATTR_BOOLEAN,	       197),
	__SELN_TRACE_STOP	= SELN_ATTR(ATTR_BOOLEAN,	       199),
	__SELN_TRACE_YIELD	= SELN_ATTR(ATTR_BOOLEAN,	       198)
}	Seln_attribute;

#define SELN_REQ_BYTESIZE ((Attr_attribute) __SELN_REQ_BYTESIZE)
#define SELN_REQ_COMMIT_PENDING_DELETE ((Attr_attribute) __SELN_REQ_COMMIT_PENDING_DELETE)
#define SELN_REQ_CONTENTS_ASCII ((Attr_attribute) __SELN_REQ_CONTENTS_ASCII)

#ifdef OW_I18N
#define SELN_REQ_CONTENTS_WCS ((Attr_attribute) __SELN_REQ_CONTENTS_WCS)
#define SELN_REQ_CHARSIZE ((Attr_attribute) __SELN_REQ_CHARSIZE)
#define SELN_REQ_FIRST_WC ((Attr_attribute) __SELN_REQ_FIRST_WC)
#define SELN_REQ_LAST_WC ((Attr_attribute) __SELN_REQ_LAST_WC)
#endif /*OW_I18N*/

#define SELN_REQ_CONTENTS_PIECES ((Attr_attribute) __SELN_REQ_CONTENTS_PIECES)
#define SELN_REQ_DELETE ((Attr_attribute) __SELN_REQ_DELETE)
#define SELN_REQ_END_REQUEST ((Attr_attribute) __SELN_REQ_END_REQUEST)
#define SELN_REQ_FAILED ((Attr_attribute) __SELN_REQ_FAILED)
#define SELN_REQ_FAKE_LEVEL ((Attr_attribute) __SELN_REQ_FAKE_LEVEL)
#define SELN_REQ_FILE_NAME ((Attr_attribute) __SELN_REQ_FILE_NAME)
#define SELN_REQ_FIRST ((Attr_attribute) __SELN_REQ_FIRST)
#define SELN_REQ_FIRST_UNIT ((Attr_attribute) __SELN_REQ_FIRST_UNIT)
#define SELN_REQ_LAST ((Attr_attribute) __SELN_REQ_LAST)
#define SELN_REQ_LAST_UNIT ((Attr_attribute) __SELN_REQ_LAST_UNIT)
#define SELN_REQ_LEVEL ((Attr_attribute) __SELN_REQ_LEVEL)
#define SELN_REQ_RESTORE ((Attr_attribute) __SELN_REQ_RESTORE)
#define SELN_REQ_SET_LEVEL ((Attr_attribute) __SELN_REQ_SET_LEVEL)
#define SELN_REQ_UNKNOWN ((Attr_attribute) __SELN_REQ_UNKNOWN)
#define SELN_REQ_YIELD ((Attr_attribute) __SELN_REQ_YIELD)

#ifdef OW_I18N
#define SELN_REQ_CONTENTS_CT ((Attr_attribute) __SELN_REQ_CONTENTS_CT)
#endif /*OW_I18N*/

#define SELN_AGENT_INFO ((Attr_attribute) __SELN_AGENT_INFO)
#define SELN_REQ_FUNC_KEY_STATE ((Attr_attribute) __SELN_REQ_FUNC_KEY_STATE)
#define SELN_REQ_SELECTED_WINDOWS ((Attr_attribute) __SELN_REQ_SELECTED_WINDOWS)
#define SELN_REQ_CONTENTS_OBJECT ((Attr_attribute) __SELN_REQ_CONTENTS_OBJECT)
#define SELN_REQ_OBJECT_SIZE ((Attr_attribute) __SELN_REQ_OBJECT_SIZE)
#define SELN_REQ_IS_READONLY ((Attr_attribute) __SELN_REQ_IS_READONLY)
#define SELN_TRACE_ACQUIRE ((Attr_attribute) __SELN_TRACE_ACQUIRE)
#define SELN_TRACE_DONE ((Attr_attribute) __SELN_TRACE_DONE)
#define SELN_TRACE_DUMP ((Attr_attribute) __SELN_TRACE_DUMP)
#define SELN_TRACE_HOLD_FILE ((Attr_attribute) __SELN_TRACE_HOLD_FILE)
#define SELN_TRACE_INFORM ((Attr_attribute) __SELN_TRACE_INFORM)
#define SELN_TRACE_INQUIRE ((Attr_attribute) __SELN_TRACE_INQUIRE)
#define SELN_TRACE_STOP ((Attr_attribute) __SELN_TRACE_STOP)
#define SELN_TRACE_YIELD ((Attr_attribute) __SELN_TRACE_YIELD)


/* Meta-levels available for use with SELN_REQ_FAKE/SET_LEVEL.
 *	SELN_LEVEL_LINE is "text line bounded by newline characters,
 *			    including only the terminating newline"
 */
typedef enum {
	SELN_LEVEL_FIRST	= 0x40000001,
	SELN_LEVEL_LINE		= 0x40000101,
	SELN_LEVEL_ALL		= 0x40008001,
	SELN_LEVEL_NEXT		= 0x4000F001,
	SELN_LEVEL_PREVIOUS	= 0x4000F002
}	Seln_level;

#endif /* ~xview_selection_attributes_DEFINED */
