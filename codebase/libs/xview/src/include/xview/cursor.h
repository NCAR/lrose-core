/*	@(#)cursor.h 20.36 93/06/28 SMI	*/

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#ifndef  xview_cursor_DEFINED
#define  xview_cursor_DEFINED

/*
 ***********************************************************************
 *			Include Files
 ***********************************************************************
 */

#include <stdio.h>
#include <xview/generic.h>
#include <xview/pkg.h>
#include <sys/types.h>
#include <pixrect/pixrect.h>

#ifdef __STDC__ 
#ifndef CAT
#define CAT(a,b)        a ## b 
#endif 
#endif
#include <pixrect/memvar.h>

#include <xview/rect.h>

/*
 ***********************************************************************
 *			Definitions and Macros
 ***********************************************************************
 */

#define CURSOR				&xv_cursor_pkg

#define	CURSOR_ATTR(type, ordinal)	ATTR(ATTR_PKG_CURSOR, type, ordinal)

/*
 * For Sunview 1 compatibility
 */
#define CURSOR_MAX_IMAGE_BYTES		32	/* max. # of image bytes */
#define CURSOR_MAX_IMAGE_WORDS		16

#define CUR_MAXIMAGEBYTES		CURSOR_MAX_IMAGE_BYTES
#define CUR_MAXIMAGEWORDS		CURSOR_MAX_IMAGE_WORDS

#define NOFONTCURSOR			-1

/*
 * OPEN LOOK Cursor character definitions
 *
 *	Those marked with "--" are not official OPEN LOOK cursors and should
 *	not be used if possible.
 */
#define OLC_BASIC_PTR           	0	/* pointer */
#define OLC_BASIC_MASK_PTR      	1
#define OLC_MOVE_PTR            	2	/* move */
#define OLC_MOVE_MASK_PTR       	3
#define OLC_COPY_PTR            	4	/* copy */
#define OLC_COPY_MASK_PTR       	5
#define OLC_BUSY_PTR            	6	/* busy */
#define OLC_BUSY_MASK_PTR       	7
#define OLC_STOP_PTR            	8 	/* -- stop */
#define OLC_STOP_MASK_PTR       	9
#define OLC_PANNING_PTR         	10	/* pan */
#define OLC_PANNING_MASK_PTR    	11
#define OLC_NAVIGATION_LEVEL_PTR        12	/* navigation */
#define OLC_NAVIGATION_LEVEL_MASK_PTR	13
/*
 * WARNING:  The following glyphs are not in the MIT X11 R4 distribution.
 */
#define OLC_BLANK_14 			14	/* -- blank at index 14 */
#define OLC_BLANK_15 			15	/* -- blank at index 15 */
#define OLC_OTHER_PTR 			16	/* -- another basic pointer */
#define OLC_OTHER_MASK_PTR 		17	
#define OLC_RESIZE 			18	/* resize bullseye */
#define OLC_RESIZE_MASK			19	
#define OLC_RIGHT_ARROW 		20	/* -- right arrow */
#define OLC_RIGHT_ARROW_MASK		21	
#define OLC_PLUS 			22	/* -- plus sign */
#define OLC_PLUS_MASK			23	
#define OLC_MULTIPLY 			24	/* -- multiply sign */
#define OLC_MULTIPLY_MASK		25	
#define OLC_HOURGLASS 			26	/* -- hourglass */
#define OLC_HOURGLASS_MASK		27	
#define OLC_DOC 			28	/* doc = document */
#define OLC_DOC_MASK			29	
#define OLC_DOCS			30	/* docs = document stack */
#define OLC_DOCS_MASK			31	
#define OLC_DROP			32	/* drop = drop target */
#define OLC_DROP_MASK			33	
#define OLC_DROP_STEM			34	/* drop with stem */
#define OLC_DROP_STEM_MASK		35	
#define OLC_COPY			36	/* pointer + copy */
#define OLC_COPY_MASK			37	
#define OLC_COPY_EDGE			38	/* copy = copy edge */
#define OLC_COPY_EDGE_MASK		39	
#define OLC_DOC_COPY			40	/* doc + copy */
#define OLC_DOC_COPY_MASK		41	
#define OLC_DOC_COPY_DRAG 		42	/* doc + copy + pointer */
#define OLC_DOC_COPY_DRAG_MASK 		43
#define OLC_DOC_COPY_DROP		44	/* doc + copy + drop */
#define OLC_DOC_COPY_DROP_MASK		45
#define OLC_DOC_COPY_NODROP		46	/* doc + copy + nodrop */
#define OLC_DOC_COPY_NODROP_MASK	47
#define OLC_DOCS_COPY			48	/* docs + copy */
#define OLC_DOCS_COPY_MASK		49	
#define OLC_DOCS_COPY_DRAG 		50	/* docs + copy + pointer */
#define OLC_DOCS_COPY_DRAG_MASK 	51
#define OLC_DOCS_COPY_DROP		52	/* docs + copy + drop */
#define OLC_DOCS_COPY_DROP_MASK		53
#define OLC_DOCS_COPY_NODROP		54	/* docs + copy + nodrop */
#define OLC_DOCS_COPY_NODROP_MASK	55
#define OLC_MOVE			56	/* pointer + move */
#define OLC_MOVE_MASK			57	
#define OLC_MOVE_EDGE			58	/* move = move edge */
#define OLC_MOVE_EDGE_MASK		59	
#define OLC_DOC_MOVE			60	/* doc + move */
#define OLC_DOC_MOVE_MASK		61	
#define OLC_DOC_MOVE_DRAG 		62	/* doc + move + pointer */
#define OLC_DOC_MOVE_DRAG_MASK 		63
#define OLC_DOC_MOVE_DROP		64	/* doc + move + drop */
#define OLC_DOC_MOVE_DROP_MASK		65
#define OLC_DOC_MOVE_NODROP		66	/* doc + move + nodrop */
#define OLC_DOC_MOVE_NODROP_MASK	67
#define OLC_DOCS_MOVE			68	/* docs + move */
#define OLC_DOCS_MOVE_MASK		69	
#define OLC_DOCS_MOVE_DRAG 		70	/* docs + move + pointer */
#define OLC_DOCS_MOVE_DRAG_MASK 	71
#define OLC_DOCS_MOVE_DROP		72	/* docs + move + drop */
#define OLC_DOCS_MOVE_DROP_MASK		73
#define OLC_DOCS_MOVE_NODROP		74	/* docs + move + nodrop */
#define OLC_DOCS_MOVE_NODROP_MASK	75
#define OLC_NODROP			76	/* nodrop */
#define OLC_NODROP_MASK			77
#define OLC_NODROP_STEM			78	/* nodrop with stem */
#define OLC_NODROP_STEM_MASK		79
#define OLC_TEXT_COPY			80	/* text copy */
#define OLC_TEXT_COPY_MASK		81
#define OLC_TEXT_COPY_DRAG		82	/* text copy + pointer */
#define OLC_TEXT_COPY_MOVE_MASK		83
#define OLC_TEXT_COPY_DROP		84	/* text copy + drop */
#define OLC_TEXT_COPY_DROP_MASK		85
#define OLC_TEXT_COPY_NODROP		86	/* text copy + nodrop */
#define OLC_TEXT_COPY_NODROP_MASK	87
#define OLC_TEXT_MOVE			88	/* text move */
#define OLC_TEXT_MOVE_MASK		89
#define OLC_TEXT_MOVE_DRAG		90	/* text move + pointer */
#define OLC_TEXT_MOVE_MOVE_MASK		91
#define OLC_TEXT_MOVE_DROP		92	/* text move + drop */
#define OLC_TEXT_MOVE_DROP_MASK		93
#define OLC_TEXT_MOVE_NODROP		94	/* text move + nodrop */
#define OLC_TEXT_MOVE_NODROP_MASK	95
#define OLC_TEXT_COPY_INSERT		96	/* text copy + insert */
#define OLC_TEXT_COPY_INSERT_MASK	97
#define OLC_TEXT_MOVE_INSERT		98	/* text move + insert */
#define OLC_TEXT_MOVE_INSERT_MASK	99
#define OLC_DATA_COPY_DROP		100	/* data copy + drop */
#define OLC_DATA_COPY_DROP_MASK		101
#define OLC_DATA_COPY_INSERT		102	/* data copy + insert */
#define OLC_DATA_COPY_INSERT_MASK	103
#define OLC_DATA_COPY_NODROP		104	/* data copy + nodrop */
#define OLC_DATA_COPY_NODROP_MASK	105
#define OLC_DATA_MOVE_DROP		106	/* data move + drop */
#define OLC_DATA_MOVE_DROP_MASK		107
#define OLC_DATA_MOVE_INSERT		108	/* data move + insert */
#define OLC_DATA_MOVE_INSERT_MASK	109
#define OLC_DATA_MOVE_NODROP		110	/* data move + nodrop */
#define OLC_DATA_MOVE_NODROP_MASK	111
#define OLC_INSERT			112	/* insert = insert target */
#define OLC_INSERT_MASK			113	
#define OLC_INSERT_STEM			114	/* insert with stem */
#define OLC_INSERT_STEM_MASK		115	
#define OLC_DATA_MOVE 			116	/* data move */
#define OLC_DATA_MOVE_MASK		117	
#define OLC_DATA_COPY			118	/* data copy */
#define OLC_DATA_COPY_MASK		119	
#define OLC_TEXT_POINTER		120	/* text/data pointer */
#define OLC_TEXT_POINTER_MASK		121
#define OLC_FOLDER 			122	/* folder */
#define OLC_FOLDER_MASK			123	
#define OLC_FOLDERS			124	/* folders= folder stack */
#define OLC_FOLDERS_MASK		125	
#define OLC_BLANK_126 			126	/* -- blank at index 126 */
#define OLC_BLANK_127 			127	/* -- blank at index 127 */

/*
 ***********************************************************************
 *		Typedefs, enumerations, and structs
 ***********************************************************************
 */

typedef Xv_opaque	Xv_Cursor;
typedef Xv_opaque	Xv_cursor;

typedef enum {
    CURSOR_NEUTRAL = 0,
    CURSOR_ACCEPT = 1,
    CURSOR_REJECT = 2
} Cursor_drag_state;

typedef enum {
    CURSOR_MOVE = 0,
    CURSOR_DUPLICATE = 1
} Cursor_drag_type;

typedef enum {
	/*
 	 * Public Attributes 
 	 */
  __CURSOR_BACKGROUND_COLOR	= CURSOR_ATTR(ATTR_SINGLE_COLOR_PTR,    27),
  __CURSOR_DRAG_STATE		= CURSOR_ATTR(ATTR_ENUM,		2),
  __CURSOR_DRAG_TYPE		= CURSOR_ATTR(ATTR_ENUM,		3),
  __CURSOR_FOREGROUND_COLOR	= CURSOR_ATTR(ATTR_SINGLE_COLOR_PTR,    26),
  __CURSOR_IMAGE		= CURSOR_ATTR(ATTR_PIXRECT_PTR,	  	1),
  __CURSOR_MASK_CHAR		= CURSOR_ATTR(ATTR_INT,		  	5),
  __CURSOR_OP			= CURSOR_ATTR(ATTR_INT,		 	10),
  __CURSOR_SRC_CHAR		= CURSOR_ATTR(ATTR_INT,		 	15),
  __CURSOR_STRING		= CURSOR_ATTR(ATTR_STRING,		16),
#ifdef OW_I18N
  __CURSOR_STRING_WCS		= CURSOR_ATTR(ATTR_WSTRING,	 	17),
#endif
  __CURSOR_XHOT			= CURSOR_ATTR(ATTR_INT,		 	20),
  __CURSOR_YHOT			= CURSOR_ATTR(ATTR_INT,		 	25),
	/*
	 * OPEN LOOK cursors.  Used as index to XV_KEY_DATA on server.
	 */
  __CURSOR_BASIC_PTR		= CURSOR_ATTR(ATTR_OPAQUE,		30),
  __CURSOR_MOVE_PTR		= CURSOR_ATTR(ATTR_OPAQUE,		35),
  __CURSOR_COPY_PTR		= CURSOR_ATTR(ATTR_OPAQUE,	 	40),
  __CURSOR_BUSY_PTR		= CURSOR_ATTR(ATTR_OPAQUE,		45),
  __CURSOR_PANNING_PTR		= CURSOR_ATTR(ATTR_OPAQUE,		50),
  __CURSOR_NAVIGATION_LEVEL_PTR	= CURSOR_ATTR(ATTR_OPAQUE,		55),
  __CURSOR_STOP_PTR		= CURSOR_ATTR(ATTR_OPAQUE,		60),
  __CURSOR_QUESTION_MARK_PTR	= CURSOR_ATTR(ATTR_OPAQUE,		65)
} Cursor_attribute;

#define CURSOR_BACKGROUND_COLOR ((Attr_attribute) __CURSOR_BACKGROUND_COLOR)
#define CURSOR_DRAG_STATE ((Attr_attribute) __CURSOR_DRAG_STATE)
#define CURSOR_DRAG_TYPE ((Attr_attribute) __CURSOR_DRAG_TYPE)
#define CURSOR_FOREGROUND_COLOR ((Attr_attribute) __CURSOR_FOREGROUND_COLOR)
#define CURSOR_IMAGE ((Attr_attribute) __CURSOR_IMAGE)
#define CURSOR_MASK_CHAR ((Attr_attribute) __CURSOR_MASK_CHAR)
#define CURSOR_OP ((Attr_attribute) __CURSOR_OP)
#define CURSOR_SRC_CHAR ((Attr_attribute) __CURSOR_SRC_CHAR)
#define CURSOR_STRING ((Attr_attribute) __CURSOR_STRING)
#ifdef OW_I18N
#define CURSOR_STRING_WCS ((Attr_attribute) __CURSOR_STRING_WCS)
#endif
#define CURSOR_XHOT ((Attr_attribute) __CURSOR_XHOT)
#define CURSOR_YHOT ((Attr_attribute) __CURSOR_YHOT)
#define CURSOR_BASIC_PTR ((Attr_attribute) __CURSOR_BASIC_PTR)
#define CURSOR_MOVE_PTR ((Attr_attribute) __CURSOR_MOVE_PTR)
#define CURSOR_COPY_PTR ((Attr_attribute) __CURSOR_COPY_PTR)
#define CURSOR_BUSY_PTR ((Attr_attribute) __CURSOR_BUSY_PTR)
#define CURSOR_PANNING_PTR ((Attr_attribute) __CURSOR_PANNING_PTR)
#define CURSOR_NAVIGATION_LEVEL_PTR ((Attr_attribute) __CURSOR_NAVIGATION_LEVEL_PTR)
#define CURSOR_STOP_PTR ((Attr_attribute) __CURSOR_STOP_PTR)
#define CURSOR_QUESTION_MARK_PTR ((Attr_attribute) __CURSOR_QUESTION_MARK_PTR)

#define CURSOR_SHOW_CURSOR		XV_SHOW

typedef struct {
	Xv_generic_struct	parent_data;
	Xv_opaque		private_data;
}   Xv_cursor_struct;


/*
 ***********************************************************************
 *			Globals
 ***********************************************************************
 */

extern Xv_pkg		xv_cursor_pkg;

/*
 * Public Functions 
 */

EXTERN_FUNCTION (void		cursor_set_cursor, (Xv_object window, Xv_Cursor cursor));

/*
 * For Sunview 1 compatibility 
 */
EXTERN_FUNCTION (Xv_Cursor cursor_create, (Attr_attribute attr1, DOTDOTDOT));
EXTERN_FUNCTION (void		cursor_destroy, (Xv_Cursor cursor));
EXTERN_FUNCTION (Xv_Cursor	cursor_copy, (Xv_Cursor cursor));
EXTERN_FUNCTION (Xv_opaque	cursor_get, (Xv_Cursor cursor, Cursor_attribute  attr));
EXTERN_FUNCTION (int		cursor_set, (Xv_Cursor cursor, DOTDOTDOT));

#endif	/* xview_cursor_DEFINED */
