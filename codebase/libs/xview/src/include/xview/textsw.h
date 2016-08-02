/*	@(#)textsw.h 20.56 93/06/28 SMI	*/

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#ifndef xview_textsw_DEFINED
#define xview_textsw_DEFINED

/*
 ***********************************************************************
 *			Include Files
 ***********************************************************************
 */

/*
 * Programmatic interface to textsw
 */
#include <pixrect/pixrect.h>
#include <xview/rect.h>
#include <xview/rectlist.h>

/*
 * New window_create() defs 
 */
#include <xview/window.h>
#include <xview/openwin.h>
#include <xview/pkg.h>
#include <xview/attrol.h>
#include <xview/sel_attrs.h>
/*
 ***********************************************************************
 *			Definitions and Macros
 ***********************************************************************
 */

/*
 * PUBLIC #defines
 */

#define TEXTSW         		&xv_textsw_pkg
#define TEXTSW_VIEW		&xv_textsw_view_pkg

#define	TEXTSW_NULL		((Textsw)0)

#define	TEXTSW_INFINITY		((Textsw_index)0x77777777)
#define	TEXTSW_CANNOT_SET	((Textsw_index)0x80000000)
#define	TEXTSW_UNIT_IS_CHAR	SELN_LEVEL_FIRST
#define	TEXTSW_UNIT_IS_WORD	SELN_LEVEL_FIRST+1
#define	TEXTSW_UNIT_IS_LINE	SELN_LEVEL_LINE

#define TEXTSW_FONT		XV_FONT
/*
 * fields flags 
 */
#define	TEXTSW_NOT_A_FIELD		0 /* This tells the field code don't 
					   * do the search, only do a match.
					   */	
#define	TEXTSW_FIELD_FORWARD		1
#define	TEXTSW_FIELD_BACKWARD		2
#define	TEXTSW_FIELD_ENCLOSE		3
#define TEXTSW_DELIMITER_FORWARD	4
#define TEXTSW_DELIMITER_BACKWARD	5
#define TEXTSW_DELIMITER_ENCLOSE	6

/*
 * Support for marking of positions in the textsw 
 */
#define	TEXTSW_NULL_MARK		((Textsw_mark)0)

/*
 * Flags for use with textsw_add_mark 
 */
#define	TEXTSW_MARK_DEFAULTS		0x0
#define	TEXTSW_MARK_MOVE_AT_INSERT	0x1
#define	TEXTSW_MARK_READ_ONLY		0x2

/*
 * PRIVATE #defines
 */

#define TEXTSW_ATTR(type, ordinal)	ATTR(ATTR_PKG_TEXTSW, type, ordinal)
#define TEXTSW_ATTR_LIST(ltype, type, ordinal)	\
	TEXTSW_ATTR(ATTR_LIST_INLINE((ltype), (type)), (ordinal))

/*
 * A special scrollbar value for TEXTSW_VIEW_SCROLLBAR 
 */
#define	TEXTSW_DEFAULT_SCROLLBAR	((caddr_t)1)

/*
 * Flag values for TEXTSW_NOTIFY_LEVEL attribute.
 */
#define	TEXTSW_NOTIFY_NONE		0x00
#define	TEXTSW_NOTIFY_DESTROY_VIEW	0x01
#define	TEXTSW_NOTIFY_EDIT_DELETE	0x02
#define	TEXTSW_NOTIFY_EDIT_INSERT	0x04
#define	TEXTSW_NOTIFY_EDIT		(TEXTSW_NOTIFY_EDIT_DELETE | \
					 TEXTSW_NOTIFY_EDIT_INSERT)
#define	TEXTSW_NOTIFY_PAINT		0x08
#define	TEXTSW_NOTIFY_REPAINT		0x10
#define	TEXTSW_NOTIFY_SCROLL		0x20
#define	TEXTSW_NOTIFY_SPLIT_VIEW	0x40
#define	TEXTSW_NOTIFY_STANDARD		0x80
#define	TEXTSW_NOTIFY_ALL		(TEXTSW_NOTIFY_DESTROY_VIEW | \
					 TEXTSW_NOTIFY_EDIT	    | \
					 TEXTSW_NOTIFY_PAINT	    | \
					 TEXTSW_NOTIFY_REPAINT	    | \
					 TEXTSW_NOTIFY_SCROLL	    | \
					 TEXTSW_NOTIFY_SPLIT_VIEW   | \
					 TEXTSW_NOTIFY_STANDARD)

#define	TEXTSW_ATTR_RECT_PAIR		ATTR_TYPE(ATTR_BASE_RECT_PTR, 2)
#define	TEXTSW_ATTR_REPLACE_5		ATTR_TYPE(ATTR_BASE_INT, 5)

/* 
 * Bit flags returned by textsw_process_event 
 */
#define	TEXTSW_PE_BUSY			0x1
#define TEXTSW_PE_READ_ONLY		0x2
#define TEXTSW_PE_USED			0x4

/*
 * Reset actions for Load/Reset/Save/Store
 */
#define	TEXTSW_LRSS_CURRENT		0
#define	TEXTSW_LRSS_ENTITY_START	1
#define	TEXTSW_LRSS_LINE_START		2

/*
 * The magic number for smart filters 
 */
#define	TEXTSW_FILTER_MAGIC		0xFF012003

/*
 * Flag values for textsw_add_glyph_on_line(). 
 */
#define	TEXTSW_GLYPH_DISPLAY		0x0000001
#define	TEXTSW_GLYPH_LINE_START		0x0000002
#define	TEXTSW_GLYPH_WORD_START		0x0000004
#define	TEXTSW_GLYPH_LINE_END		0x0000008

/*
 * For SunView 1 compatibility
 */
#define	TEXTSW_NO_CD			TEXTSW_DISABLE_CD
#define TEXTSW_LEFT_MARGIN		XV_LEFT_MARGIN
#define TEXTSW_RIGHT_MARGIN		XV_RIGHT_MARGIN
#define TEXTSW_SCROLLBAR		WIN_VERTICAL_SCROLLBAR
#define TEXTSW_MENU			WIN_MENU
#define TEXTSW_TOOL			WIN_FRAME
#define TEXTSW_PIXWIN			WIN_PIXWIN
#define TEXTSW_TYPE	       		ATTR_PKG_TEXTSW

/*
 ***********************************************************************
 *		Typedefs, enumerations, and structs
 ***********************************************************************
 */

#if lint
	typedef void * Textsw_opaque;
#else
	typedef char * Textsw_opaque;
#endif

typedef Xv_opaque	Textsw;
typedef Xv_opaque	Textsw_view;
typedef long int	Textsw_index;
typedef Textsw_opaque	Textsw_mark;

/*
 * Enumerations
 */

/*
 * Attributes for textsw_build, textsw_init, textsw_set and textsw_get. 
 */
typedef enum {
	/*
	 * Public Attributes 
	 */
	__TEXTSW_ADJUST_IS_PENDING_DELETE= TEXTSW_ATTR(ATTR_BOOLEAN,	  1),
	__TEXTSW_AGAIN_RECORDING	= TEXTSW_ATTR(ATTR_BOOLEAN,	  2),
	__TEXTSW_AUTO_INDENT		= TEXTSW_ATTR(ATTR_BOOLEAN,	  4),
	__TEXTSW_AUTO_SCROLL_BY		= TEXTSW_ATTR(ATTR_INT,		  6),
	__TEXTSW_BLINK_CARET		= TEXTSW_ATTR(ATTR_BOOLEAN,	  8),
	__TEXTSW_BROWSING		= TEXTSW_ATTR(ATTR_BOOLEAN,	 10),
	__TEXTSW_CHECKPOINT_FREQUENCY	= TEXTSW_ATTR(ATTR_INT,		 12),
	__TEXTSW_CLIENT_DATA		= TEXTSW_ATTR(ATTR_OPAQUE,	 14),
	__TEXTSW_CONFIRM_OVERWRITE	= TEXTSW_ATTR(ATTR_BOOLEAN,	 16),
	__TEXTSW_CONTENTS		= TEXTSW_ATTR(ATTR_STRING,	 18),
#ifdef OW_I18N	
	__TEXTSW_CONTENTS_WCS		= TEXTSW_ATTR(ATTR_WSTRING,	 19),
#endif		
	__TEXTSW_CONTROL_CHARS_USE_FONT	= TEXTSW_ATTR(ATTR_BOOLEAN,	 20),
	__TEXTSW_DISABLE_CD		= TEXTSW_ATTR(ATTR_BOOLEAN,	 22),
	__TEXTSW_DISABLE_LOAD		= TEXTSW_ATTR(ATTR_BOOLEAN,	 24),
	__TEXTSW_SUBMENU_EDIT		= TEXTSW_ATTR(ATTR_NO_VALUE,	 26),
	__TEXTSW_EDIT_COUNT		= TEXTSW_ATTR(ATTR_INT,		 28),
	__TEXTSW_EXTRAS_CMD_MENU	= TEXTSW_ATTR(ATTR_INT,		 30),
	__TEXTSW_FILE			= TEXTSW_ATTR(ATTR_STRING,	 32),
#ifdef OW_I18N	
	__TEXTSW_FILE_WCS		= TEXTSW_ATTR(ATTR_WSTRING,	 33),
#endif		
	__TEXTSW_SUBMENU_FILE		= TEXTSW_ATTR(ATTR_NO_VALUE,	 34),
	__TEXTSW_FILE_CONTENTS		= TEXTSW_ATTR(ATTR_STRING,	 36),
#ifdef OW_I18N	
	__TEXTSW_FILE_CONTENTS_WCS	= TEXTSW_ATTR(ATTR_WSTRING,	 37),
#endif	
	__TEXTSW_SUBMENU_FIND		= TEXTSW_ATTR(ATTR_NO_VALUE,	 38),
	__TEXTSW_FIRST			= TEXTSW_ATTR(ATTR_INT,		 40),
#ifdef OW_I18N
	__TEXTSW_FIRST_WC		= TEXTSW_ATTR(ATTR_INT,		 41),
#endif
	__TEXTSW_FIRST_LINE		= TEXTSW_ATTR(ATTR_INT,		 42),
	__TEXTSW_HISTORY_LIMIT		= TEXTSW_ATTR(ATTR_INT,		 44),
	__TEXTSW_IGNORE_LIMIT		= TEXTSW_ATTR(ATTR_INT,		 46),
	__TEXTSW_INSERTION_POINT	= TEXTSW_ATTR(ATTR_INT,		 48),
#ifdef OW_I18N
	__TEXTSW_INSERTION_POINT_WC	= TEXTSW_ATTR(ATTR_INT,		 49),
#endif
	__TEXTSW_INSERT_FROM_FILE	= TEXTSW_ATTR(ATTR_STRING,	 50),
#ifdef OW_I18N	
	__TEXTSW_INSERT_FROM_FILE_WCS	= TEXTSW_ATTR(ATTR_WSTRING,	 51),
#endif	
	__TEXTSW_INSERT_MAKES_VISIBLE	= TEXTSW_ATTR(ATTR_ENUM,	 52),
	__TEXTSW_LENGTH			= TEXTSW_ATTR(ATTR_INT,		 54),
#ifdef OW_I18N	
	__TEXTSW_LENGTH_WC		= TEXTSW_ATTR(ATTR_INT,	 	 55),
#endif	
	__TEXTSW_LINE_BREAK_ACTION	= TEXTSW_ATTR(ATTR_ENUM,	 56),
	__TEXTSW_LOWER_CONTEXT		= TEXTSW_ATTR(ATTR_INT,		 58),
	__TEXTSW_MEMORY_MAXIMUM		= TEXTSW_ATTR(ATTR_INT,		 60),
	__TEXTSW_MODIFIED		= TEXTSW_ATTR(ATTR_BOOLEAN,	 62),
	__TEXTSW_MULTI_CLICK_SPACE	= TEXTSW_ATTR(ATTR_INT,		 64),
	__TEXTSW_MULTI_CLICK_TIMEOUT	= TEXTSW_ATTR(ATTR_INT,		 66),
	__TEXTSW_NOTIFY_PROC		= TEXTSW_ATTR(ATTR_FUNCTION_PTR, 68),
	__TEXTSW_READ_ONLY		= TEXTSW_ATTR(ATTR_BOOLEAN,	 70),
	__TEXTSW_STATUS			= TEXTSW_ATTR(ATTR_OPAQUE,	 72),
	__TEXTSW_STORE_CHANGES_FILE 	= TEXTSW_ATTR(ATTR_BOOLEAN,	 74),
	__TEXTSW_UPDATE_SCROLLBAR	= TEXTSW_ATTR(ATTR_NO_VALUE,	 78),
	__TEXTSW_UPPER_CONTEXT		= TEXTSW_ATTR(ATTR_INT,		 80),
	__TEXTSW_SUBMENU_VIEW		= TEXTSW_ATTR(ATTR_NO_VALUE,	 82),
	__TEXTSW_ACCELERATE_MENUS	= TEXTSW_ATTR(ATTR_BOOLEAN,	 83),
	/*
	 * Private Attributes 
	 */
	__TEXTSW_AGAIN_LIMIT		= TEXTSW_ATTR(ATTR_INT,		 84),
	__TEXTSW_COALESCE_WITH		= TEXTSW_ATTR(ATTR_OPAQUE,	 86),
	__TEXTSW_DESTROY_ALL_VIEWS	= TEXTSW_ATTR(ATTR_BOOLEAN,	 92),
	__TEXTSW_EDIT_BACK_CHAR		= TEXTSW_ATTR(ATTR_CHAR,	 98),
	__TEXTSW_EDIT_BACK_LINE		= TEXTSW_ATTR(ATTR_CHAR,	100),
	__TEXTSW_EDIT_BACK_WORD		= TEXTSW_ATTR(ATTR_CHAR,	102),
	__TEXTSW_ERROR_MSG		= TEXTSW_ATTR(ATTR_STRING,	104),
	__TEXTSW_ES_CREATE_PROC		= TEXTSW_ATTR(ATTR_FUNCTION_PTR,106),
	__TEXTSW_INSERT_ONLY		= TEXTSW_ATTR(ATTR_BOOLEAN,	108),
	__TEXTSW_LOAD_DIR_IS_CD		= TEXTSW_ATTR(ATTR_ENUM,	110),
#ifdef DEBUG
	__TEXTSW_MALLOC_DEBUG_LEVEL	= TEXTSW_ATTR(ATTR_INT,		112),
#endif
	__TEXTSW_MUST_SHOW_CARET	= TEXTSW_ATTR(ATTR_BOOLEAN,	114),
	__TEXTSW_NAME_TO_USE		= TEXTSW_ATTR(ATTR_STRING,	116),
	__TEXTSW_NOTIFY_LEVEL		= TEXTSW_ATTR(ATTR_INT,		118),
	__TEXTSW_NO_REPAINT_TIL_EVENT	= TEXTSW_ATTR(ATTR_BOOLEAN,	120),
	__TEXTSW_NO_RESET_TO_SCRATCH	= TEXTSW_ATTR(ATTR_BOOLEAN,	122),
	__TEXTSW_NO_SELECTION_SERVICE	= TEXTSW_ATTR(ATTR_BOOLEAN,	124),
	__TEXTSW_RESET_MODE		= TEXTSW_ATTR(ATTR_ENUM,	128),
	__TEXTSW_RESET_TO_CONTENTS	= TEXTSW_ATTR(ATTR_NO_VALUE,	130),
	__TEXTSW_SPARE_1		=
			TEXTSW_ATTR(ATTR_LIST_INLINE(ATTR_NULL, ATTR_INT),
									132),
	__TEXTSW_SPARE_2		=
			TEXTSW_ATTR(ATTR_LIST_INLINE(ATTR_NULL, ATTR_INT),
									134),
	__TEXTSW_TAB_WIDTH		= TEXTSW_ATTR(ATTR_INT,		136),
	__TEXTSW_TAB_WIDTHS	= TEXTSW_ATTR_LIST(ATTR_NULL, ATTR_INT, 138),
	__TEXTSW_TEMP_FILENAME		= TEXTSW_ATTR(ATTR_STRING,	140),
	__TEXTSW_WRAPAROUND_SIZE	= TEXTSW_ATTR(ATTR_INT,		142),
	/*
	 * Make individual view changes affect all views 
	 */
	__TEXTSW_END_ALL_VIEWS		= TEXTSW_ATTR(ATTR_NO_VALUE,	144),
	__TEXTSW_FOR_ALL_VIEWS		= TEXTSW_ATTR(ATTR_NO_VALUE,	146),
	/*1030878*/
        __TEXTSW_DIFFERENTIATE_CR_LF    = TEXTSW_ATTR(ATTR_BOOLEAN,	148)
#ifdef OW_I18N
       ,__TEXTSW_CONTENTS_NO_COMMIT	= TEXTSW_ATTR(ATTR_WSTRING,	150),
	__TEXTSW_CONTENTS_WCS_NO_COMMIT	= TEXTSW_ATTR(ATTR_WSTRING,	152)
#endif
} Textsw_attribute;


#define TEXTSW_ADJUST_IS_PENDING_DELETE ((Attr_attribute) __TEXTSW_ADJUST_IS_PENDING_DELETE)
#define TEXTSW_AGAIN_RECORDING ((Attr_attribute) __TEXTSW_AGAIN_RECORDING)
#define TEXTSW_AUTO_INDENT ((Attr_attribute) __TEXTSW_AUTO_INDENT)
#define TEXTSW_AUTO_SCROLL_BY ((Attr_attribute) __TEXTSW_AUTO_SCROLL_BY)
#define TEXTSW_BLINK_CARET ((Attr_attribute) __TEXTSW_BLINK_CARET)
#define TEXTSW_BROWSING ((Attr_attribute) __TEXTSW_BROWSING)
#define TEXTSW_CHECKPOINT_FREQUENCY ((Attr_attribute) __TEXTSW_CHECKPOINT_FREQUENCY)
#define TEXTSW_CLIENT_DATA ((Attr_attribute) __TEXTSW_CLIENT_DATA)
#define TEXTSW_CONFIRM_OVERWRITE ((Attr_attribute) __TEXTSW_CONFIRM_OVERWRITE)
#define TEXTSW_CONTENTS ((Attr_attribute) __TEXTSW_CONTENTS)
#ifdef OW_I18N	
#define TEXTSW_CONTENTS_WCS ((Attr_attribute) __TEXTSW_CONTENTS_WCS)
#endif		
#define TEXTSW_CONTROL_CHARS_USE_FONT ((Attr_attribute) __TEXTSW_CONTROL_CHARS_USE_FONT)
#define TEXTSW_DISABLE_CD ((Attr_attribute) __TEXTSW_DISABLE_CD)
#define TEXTSW_DISABLE_LOAD ((Attr_attribute) __TEXTSW_DISABLE_LOAD)
#define TEXTSW_SUBMENU_EDIT ((Attr_attribute) __TEXTSW_SUBMENU_EDIT)
#define TEXTSW_EDIT_COUNT ((Attr_attribute) __TEXTSW_EDIT_COUNT)
#define TEXTSW_EXTRAS_CMD_MENU ((Attr_attribute) __TEXTSW_EXTRAS_CMD_MENU)
#define TEXTSW_FILE ((Attr_attribute) __TEXTSW_FILE)
#ifdef OW_I18N	
#define TEXTSW_FILE_WCS ((Attr_attribute) __TEXTSW_FILE_WCS)
#endif		
#define TEXTSW_SUBMENU_FILE ((Attr_attribute) __TEXTSW_SUBMENU_FILE)
#define TEXTSW_FILE_CONTENTS ((Attr_attribute) __TEXTSW_FILE_CONTENTS)
#ifdef OW_I18N	
#define TEXTSW_FILE_CONTENTS_WCS ((Attr_attribute) __TEXTSW_FILE_CONTENTS_WCS)
#endif	
#define TEXTSW_SUBMENU_FIND ((Attr_attribute) __TEXTSW_SUBMENU_FIND)
#define TEXTSW_FIRST ((Attr_attribute) __TEXTSW_FIRST)
#ifdef OW_I18N
#define TEXTSW_FIRST_WC ((Attr_attribute) __TEXTSW_FIRST_WC)
#endif
#define TEXTSW_FIRST_LINE ((Attr_attribute) __TEXTSW_FIRST_LINE)
#define TEXTSW_HISTORY_LIMIT ((Attr_attribute) __TEXTSW_HISTORY_LIMIT)
#define TEXTSW_IGNORE_LIMIT ((Attr_attribute) __TEXTSW_IGNORE_LIMIT)
#define TEXTSW_INSERTION_POINT ((Attr_attribute) __TEXTSW_INSERTION_POINT)
#ifdef OW_I18N
#define TEXTSW_INSERTION_POINT_WC ((Attr_attribute) __TEXTSW_INSERTION_POINT_WC)
#endif
#define TEXTSW_INSERT_FROM_FILE ((Attr_attribute) __TEXTSW_INSERT_FROM_FILE)
#ifdef OW_I18N	
#define TEXTSW_INSERT_FROM_FILE_WCS ((Attr_attribute) __TEXTSW_INSERT_FROM_FILE_WCS)
#endif	
#define TEXTSW_INSERT_MAKES_VISIBLE ((Attr_attribute) __TEXTSW_INSERT_MAKES_VISIBLE)
#define TEXTSW_LENGTH ((Attr_attribute) __TEXTSW_LENGTH)
#ifdef OW_I18N	
#define TEXTSW_LENGTH_WC ((Attr_attribute) __TEXTSW_LENGTH_WC)
#endif	
#define TEXTSW_LINE_BREAK_ACTION ((Attr_attribute) __TEXTSW_LINE_BREAK_ACTION)
#define TEXTSW_LOWER_CONTEXT ((Attr_attribute) __TEXTSW_LOWER_CONTEXT)
#define TEXTSW_MEMORY_MAXIMUM ((Attr_attribute) __TEXTSW_MEMORY_MAXIMUM)
#define TEXTSW_MODIFIED ((Attr_attribute) __TEXTSW_MODIFIED)
#define TEXTSW_MULTI_CLICK_SPACE ((Attr_attribute) __TEXTSW_MULTI_CLICK_SPACE)
#define TEXTSW_MULTI_CLICK_TIMEOUT ((Attr_attribute) __TEXTSW_MULTI_CLICK_TIMEOUT)
#define TEXTSW_NOTIFY_PROC ((Attr_attribute) __TEXTSW_NOTIFY_PROC)
#define TEXTSW_READ_ONLY ((Attr_attribute) __TEXTSW_READ_ONLY)
#define TEXTSW_STATUS ((Attr_attribute) __TEXTSW_STATUS)
#define TEXTSW_STORE_CHANGES_FILE ((Attr_attribute) __TEXTSW_STORE_CHANGES_FILE)
#define TEXTSW_UPDATE_SCROLLBAR ((Attr_attribute) __TEXTSW_UPDATE_SCROLLBAR)
#define TEXTSW_UPPER_CONTEXT ((Attr_attribute) __TEXTSW_UPPER_CONTEXT)
#define TEXTSW_SUBMENU_VIEW ((Attr_attribute) __TEXTSW_SUBMENU_VIEW)
#define TEXTSW_ACCELERATE_MENUS ((Attr_attribute) __TEXTSW_ACCELERATE_MENUS)
#define TEXTSW_AGAIN_LIMIT ((Attr_attribute) __TEXTSW_AGAIN_LIMIT)
#define TEXTSW_COALESCE_WITH ((Attr_attribute) __TEXTSW_COALESCE_WITH)
#define TEXTSW_DESTROY_ALL_VIEWS ((Attr_attribute) __TEXTSW_DESTROY_ALL_VIEWS)
#define TEXTSW_EDIT_BACK_CHAR ((Attr_attribute) __TEXTSW_EDIT_BACK_CHAR)
#define TEXTSW_EDIT_BACK_LINE ((Attr_attribute) __TEXTSW_EDIT_BACK_LINE)
#define TEXTSW_EDIT_BACK_WORD ((Attr_attribute) __TEXTSW_EDIT_BACK_WORD)
#define TEXTSW_ERROR_MSG ((Attr_attribute) __TEXTSW_ERROR_MSG)
#define TEXTSW_ES_CREATE_PROC ((Attr_attribute) __TEXTSW_ES_CREATE_PROC)
#define TEXTSW_INSERT_ONLY ((Attr_attribute) __TEXTSW_INSERT_ONLY)
#define TEXTSW_LOAD_DIR_IS_CD ((Attr_attribute) __TEXTSW_LOAD_DIR_IS_CD)
#ifdef DEBUG
#define TEXTSW_MALLOC_DEBUG_LEVEL ((Attr_attribute) __TEXTSW_MALLOC_DEBUG_LEVEL)
#endif
#define TEXTSW_MUST_SHOW_CARET ((Attr_attribute) __TEXTSW_MUST_SHOW_CARET)
#define TEXTSW_NAME_TO_USE ((Attr_attribute) __TEXTSW_NAME_TO_USE)
#define TEXTSW_NOTIFY_LEVEL ((Attr_attribute) __TEXTSW_NOTIFY_LEVEL)
#define TEXTSW_NO_REPAINT_TIL_EVENT ((Attr_attribute) __TEXTSW_NO_REPAINT_TIL_EVENT)
#define TEXTSW_NO_RESET_TO_SCRATCH ((Attr_attribute) __TEXTSW_NO_RESET_TO_SCRATCH)
#define TEXTSW_NO_SELECTION_SERVICE ((Attr_attribute) __TEXTSW_NO_SELECTION_SERVICE)
#define TEXTSW_RESET_MODE ((Attr_attribute) __TEXTSW_RESET_MODE)
#define TEXTSW_RESET_TO_CONTENTS ((Attr_attribute) __TEXTSW_RESET_TO_CONTENTS)
#define TEXTSW_SPARE_1 ((Attr_attribute) __TEXTSW_SPARE_1)
#define TEXTSW_SPARE_2 ((Attr_attribute) __TEXTSW_SPARE_2)
#define TEXTSW_TAB_WIDTH ((Attr_attribute) __TEXTSW_TAB_WIDTH)
#define TEXTSW_TAB_WIDTHS ((Attr_attribute) __TEXTSW_TAB_WIDTHS)
#define TEXTSW_TEMP_FILENAME ((Attr_attribute) __TEXTSW_TEMP_FILENAME)
#define TEXTSW_WRAPAROUND_SIZE ((Attr_attribute) __TEXTSW_WRAPAROUND_SIZE)
#define TEXTSW_END_ALL_VIEWS ((Attr_attribute) __TEXTSW_END_ALL_VIEWS)
#define TEXTSW_FOR_ALL_VIEWS ((Attr_attribute) __TEXTSW_FOR_ALL_VIEWS)
#define TEXTSW_DIFFERENTIATE_CR_LF ((Attr_attribute) __TEXTSW_DIFFERENTIATE_CR_LF)
#ifdef OW_I18N
#define TEXTSW_CONTENTS_NO_COMMIT ((Attr_attribute) __TEXTSW_CONTENTS_NO_COMMIT)
#define TEXTSW_CONTENTS_WCS_NO_COMMIT ((Attr_attribute) __TEXTSW_CONTENTS_WCS_NO_COMMIT)
#endif


/*
 * Following are actions defined for client provided notify_proc.
 * Two standard notify procs are textsw_default_notify and textsw_nop_notify.
 */
typedef enum {
	/*
	 * Public Attributes
	 */
	TEXTSW_ACTION_CAPS_LOCK		= TEXTSW_ATTR(ATTR_BOOLEAN,	 1),
	TEXTSW_ACTION_CHANGED_DIRECTORY	= TEXTSW_ATTR(ATTR_STRING,	 5),
	TEXTSW_ACTION_EDITED_FILE	= TEXTSW_ATTR(ATTR_STRING,	10),
	TEXTSW_ACTION_EDITED_MEMORY	= TEXTSW_ATTR(ATTR_NO_VALUE,	15),
	TEXTSW_ACTION_FILE_IS_READONLY	= TEXTSW_ATTR(ATTR_STRING,	20),
	TEXTSW_ACTION_LOADED_FILE	= TEXTSW_ATTR(ATTR_STRING,	25),
	TEXTSW_ACTION_TOOL_CLOSE	= TEXTSW_ATTR(ATTR_NO_VALUE,	30),
	TEXTSW_ACTION_TOOL_DESTROY	= TEXTSW_ATTR(ATTR_OPAQUE,	35),
	TEXTSW_ACTION_TOOL_MGR		= TEXTSW_ATTR(ATTR_OPAQUE,	40),
	TEXTSW_ACTION_TOOL_QUIT		= TEXTSW_ATTR(ATTR_OPAQUE,	45),
	TEXTSW_ACTION_USING_MEMORY	= TEXTSW_ATTR(ATTR_NO_VALUE,	50),
#ifdef OW_I18N
	TEXTSW_ACTION_CHANGED_DIRECTORY_WCS = TEXTSW_ATTR(ATTR_WSTRING,	51),
	TEXTSW_ACTION_EDITED_FILE_WCS	= TEXTSW_ATTR(ATTR_WSTRING,	52),
	TEXTSW_ACTION_LOADED_FILE_WCS	= TEXTSW_ATTR(ATTR_WSTRING,	53),
#endif		
	/*
	 * Private Attributes
	 */
	TEXTSW_ACTION_DESTROY_VIEW	= TEXTSW_ATTR(ATTR_NO_VALUE,	55),
	TEXTSW_ACTION_PAINTED		= TEXTSW_ATTR(ATTR_RECT_PTR,	60),
	TEXTSW_ACTION_REPLACED		= 
				TEXTSW_ATTR(TEXTSW_ATTR_REPLACE_5,	65),
	TEXTSW_ACTION_SAVING_FILE	= TEXTSW_ATTR(ATTR_NO_VALUE,	70),
	TEXTSW_ACTION_SCROLLED		= 
				TEXTSW_ATTR(TEXTSW_ATTR_RECT_PAIR,	75),
	TEXTSW_ACTION_SPLIT_VIEW	= TEXTSW_ATTR(ATTR_OPAQUE,	80),
	TEXTSW_ACTION_STORING_FILE	= TEXTSW_ATTR(ATTR_STRING,	85),
	TEXTSW_ACTION_WRITE_FAILED	= TEXTSW_ATTR(ATTR_NO_VALUE,	90)
} Textsw_action;

/*
 * Attributes for smart filters 
 */
typedef enum {
	__TEXTSW_FATTR_INPUT		=	TEXTSW_ATTR(ATTR_OPAQUE,1),
	__TEXTSW_FATTR_INPUT_EVENT	=	TEXTSW_ATTR(ATTR_OPAQUE,5),
	__TEXTSW_FATTR_INSERTION_POINTS	=	TEXTSW_ATTR(ATTR_OPAQUE,10),
	__TEXTSW_FATTR_INSERTION_LINE	=	TEXTSW_ATTR(ATTR_OPAQUE,15),
	__TEXTSW_FATTR_SELECTION_ENDPOINTS=	TEXTSW_ATTR(ATTR_OPAQUE,20)
} Textsw_filter_attribute;

#define TEXTSW_FATTR_INPUT ((Attr_attribute) __TEXTSW_FATTR_INPUT)
#define TEXTSW_FATTR_INPUT_EVENT ((Attr_attribute) __TEXTSW_FATTR_INPUT_EVENT)
#define TEXTSW_FATTR_INSERTION_POINTS ((Attr_attribute) __TEXTSW_FATTR_INSERTION_POINTS)
#define TEXTSW_FATTR_INSERTION_LINE ((Attr_attribute) __TEXTSW_FATTR_INSERTION_LINE)
#define TEXTSW_FATTR_SELECTION_ENDPOINTS ((Attr_attribute) __TEXTSW_FATTR_SELECTION_ENDPOINTS)

/*
 * Status values for textsw_build and textsw_init. 
 */
typedef enum {
	TEXTSW_STATUS_OKAY,
	TEXTSW_STATUS_OTHER_ERROR,
	TEXTSW_STATUS_CANNOT_ALLOCATE,
	TEXTSW_STATUS_CANNOT_OPEN_INPUT,
	TEXTSW_STATUS_BAD_ATTR,
	TEXTSW_STATUS_BAD_ATTR_VALUE,
	TEXTSW_STATUS_CANNOT_INSERT_FROM_FILE,
	TEXTSW_STATUS_OUT_OF_MEMORY
} Textsw_status;

/*
 * Status values for textsw_expand. 
 */
typedef enum {
	TEXTSW_EXPAND_OK,
	TEXTSW_EXPAND_FULL_BUF,
	TEXTSW_EXPAND_OTHER_ERROR
} Textsw_expand_status;

typedef enum {
	TEXTSW_NEVER		= 0,
	/*
	 * Additional values for TEXTSW_LOAD_DIR_IS_CD 
	 */
	TEXTSW_ALWAYS		= 1,
	TEXTSW_ONLY		= 2,
	/*
	 * Additional values for TEXTSW_INSERT_MAKES_VISIBLE 
	 */
	TEXTSW_IF_AUTO_SCROLL,
	/*
	 * Valid values for TEXTSW_LINE_BREAK_ACTION 
	 */
	TEXTSW_CLIP,
	TEXTSW_WRAP_AT_CHAR,
	TEXTSW_WRAP_AT_WORD,
	TEXTSW_WRAP_AT_LINE
} Textsw_enum;

/*
 * Menu command tokens
 */
typedef enum {
	TEXTSW_MENU_NO_CMD,
	/*
	 * File sub-menu   
	 */	
	TEXTSW_MENU_LOAD,
	TEXTSW_MENU_SAVE,
	TEXTSW_MENU_STORE,
	TEXTSW_MENU_FILE_STUFF,
	TEXTSW_MENU_RESET,
	/*
	 * Edit sub-menu  
	 */	
	TEXTSW_MENU_AGAIN,
	TEXTSW_MENU_UNDO,
	TEXTSW_MENU_UNDO_ALL,
	TEXTSW_MENU_COPY,
	TEXTSW_MENU_PASTE,
	TEXTSW_MENU_CUT,
	/*
	 * View sub-menu  
	 */	
	TEXTSW_MENU_NORMALIZE_LINE,
	TEXTSW_MENU_COUNT_TO_LINE,
	TEXTSW_MENU_NORMALIZE_INSERTION,
	/*
	 * Change line wrap sub-menu 
	 */
	TEXTSW_MENU_WRAP_LINES_AT_CHAR,
	TEXTSW_MENU_WRAP_LINES_AT_WORD,
	TEXTSW_MENU_CLIP_LINES,
	/*
	 * Find sub-menu  
	 */	
	TEXTSW_MENU_FIND_AND_REPLACE,
	TEXTSW_MENU_FIND,
	TEXTSW_MENU_FIND_BACKWARD,
	/*
	 * Replace field sub-menu 
	 */
	TEXTSW_MENU_SEL_MARK_TEXT,
	TEXTSW_MENU_SEL_ENCLOSE_FIELD,   /* Select |> field <| in pending and 
					  * delete mode 
					  */
	TEXTSW_MENU_SEL_NEXT_FIELD,
	TEXTSW_MENU_SEL_PREV_FIELD,
#ifdef _TEXTSW_FIND_RE
	TEXTSW_MENU_FIND_RE,
	TEXTSW_MENU_FIND_RE_BACKWARD,
	TEXTSW_MENU_FIND_TAG,
	TEXTSW_MENU_FIND_TAG_BACKWARD,
#endif
	TEXTSW_MENU_FILE_CMDS,
	TEXTSW_MENU_EDIT_CMDS,
	TEXTSW_MENU_VIEW_CMDS,
	TEXTSW_MENU_FIND_CMDS,
	TEXTSW_MENU_EXTRAS_CMDS,
	TEXTSW_MENU_LAST_CMD
} Textsw_menu_cmd;

/*
 * Commands for smart filters
 */
typedef enum {
	TEXTSW_FILTER_DELETE_RANGE	= 0,
	TEXTSW_FILTER_INSERT		= 1,
	TEXTSW_FILTER_SEND_RANGE	= 2,
	TEXTSW_FILTER_SET_INSERTION	= 3,
	TEXTSW_FILTER_SET_SELECTION	= 4
} Textsw_filter_command;

/*
 * Structs
 */
typedef struct {
        Xv_openwin    parent_data;
        Xv_opaque    private_data;
} Xv_textsw;

typedef struct {
        Xv_window_struct    parent_data;
        Xv_opaque    	    private_data;
} Xv_textsw_view;

/*
 ***********************************************************************
 *		Global Variables and Functions
 ***********************************************************************
 */

extern  Xv_pkg	xv_textsw_pkg;
extern  Xv_pkg	xv_textsw_view_pkg;
extern  int     TEXTSW_MENU_DATA_KEY;


/*
 * Public functions
 */

EXTERN_FUNCTION (Textsw_mark textsw_add_mark, (Textsw textsw, Textsw_index position, unsigned flags));

EXTERN_FUNCTION (int textsw_append_file_name, (Textsw textsw, char *name));

EXTERN_FUNCTION (Textsw_index textsw_delete, (Textsw textsw, Textsw_index first, Textsw_index last_plus_one));

EXTERN_FUNCTION (Textsw_index textsw_edit, (Textsw textsw, unsigned int unit, unsigned int count, unsigned int direction));

EXTERN_FUNCTION (Textsw_index textsw_erase, (Textsw textsw, Textsw_index first, Textsw_index last_plus_one));

EXTERN_FUNCTION (void textsw_file_lines_visible, (Textsw textsw, int *top, int *bottom));

EXTERN_FUNCTION (int textsw_find_bytes, (Textsw textsw, Textsw_index *first, Textsw_index *last_plus_one, char *buf, unsigned int buf_len, unsigned int flags));

EXTERN_FUNCTION (Textsw_index textsw_find_mark, (Textsw textsw, Textsw_mark mark));

EXTERN_FUNCTION (Textsw textsw_first, (Textsw textsw));

EXTERN_FUNCTION (Textsw_index textsw_index_for_file_line, (Textsw textsw, int line));

EXTERN_FUNCTION (Textsw_index textsw_insert, (Textsw textsw, char *buf, int buf_len));

EXTERN_FUNCTION (int textsw_match_bytes, (Textsw textsw, Textsw_index *first, Textsw_index *last_plus_one, char * start_sym, int start_sym_len, char *end_sym, int end_sym_len, unsigned int field_flag));

EXTERN_FUNCTION (Textsw textsw_next, (Textsw previous));

EXTERN_FUNCTION (void textsw_remove_mark, (Textsw textsw, Textsw_mark mark));

EXTERN_FUNCTION (Textsw_index textsw_replace_bytes, (Textsw textsw, Textsw_index first, Textsw_index last_plus_one, char *buf, long int buf_len));

EXTERN_FUNCTION (void textsw_reset, (Textsw textsw, int locx, int locy));

EXTERN_FUNCTION (unsigned int textsw_save, (Textsw textsw, int locx, int locy));

EXTERN_FUNCTION (int textsw_screen_line_count, (Textsw textsw));

EXTERN_FUNCTION (void textsw_scroll_lines, (Textsw abstract, int count));

EXTERN_FUNCTION (unsigned int textsw_store_file, (Textsw textsw, char *filename, int locx, int locy));

#ifdef OW_I18N
EXTERN_FUNCTION (Textsw_mark textsw_add_mark_wc, (Textsw textsw, Textsw_index position, unsigned flags));

EXTERN_FUNCTION (Textsw_index textsw_delete_wcs, (Textsw textsw, Textsw_index first, Textsw_index last_plus_one));

EXTERN_FUNCTION (Textsw_index textsw_edit_wcs, (Textsw textsw, unsigned int unit, unsigned int count, unsigned int direction));

EXTERN_FUNCTION (Textsw_index textsw_erase_wcs, (Textsw textsw, Textsw_index first, Textsw_index last_plus_one));

EXTERN_FUNCTION (Textsw_index textsw_find_mark_wc, (Textsw textsw, Textsw_mark mark));

EXTERN_FUNCTION (Textsw_index textsw_index_for_file_line_wc, (Textsw textsw, int line));

EXTERN_FUNCTION (int textsw_append_file_name_wcs, (Textsw textsw, wchar_t *name));

EXTERN_FUNCTION (int textsw_find_wcs, (Textsw textsw, Textsw_index *first, Textsw_index *last_plus_one, wchar_t *buf, unsigned int buf_len, unsigned int flags));

EXTERN_FUNCTION (Textsw_index textsw_insert_wcs, (Textsw textsw, wchar_t *buf, int buf_len));

EXTERN_FUNCTION (int textsw_match_wcs, (Textsw textsw, Textsw_index *first, Textsw_index *last_plus_one, wchar_t * start_sym, int start_sym_len, wchar_t *end_sym, int end_sym_len, unsigned int field_flag));

EXTERN_FUNCTION (Textsw_index textsw_replace_wcs, (Textsw textsw, Textsw_index first, Textsw_index last_plus_one, wchar_t *buf, long int buf_len));

EXTERN_FUNCTION (unsigned int textsw_store_file_wcs, (Textsw textsw, wchar_t *filename, int locx, int locy));
#endif /* OW_I18N */

/*
 * Private functions
 */

#ifdef OW_I18N
EXTERN_FUNCTION (Textsw_expand_status textsw_expand, (Textsw textsw, Textsw_index start, Textsw_index stop_plus_one, wchar_t *out_buf, int out_buf_len, int *total_chars));
#else
EXTERN_FUNCTION (Textsw_expand_status textsw_expand, (Textsw textsw, Textsw_index start, Textsw_index stop_plus_one, char *out_buf, int out_buf_len, int *total_chars));
#endif /* OW_I18N */

EXTERN_FUNCTION (int textsw_default_notify, (Textsw textsw, Attr_avlist attrs));

EXTERN_FUNCTION (int textsw_nop_notify, (Textsw textsw, Attr_avlist attrs));

#ifdef _OTHER_TEXTSW_FUNCTIONS

EXTERN_FUNCTION (void textsw_normalize_view, (Textsw textsw, Textsw_index pos));
EXTERN_FUNCTION (void textsw_possibly_normalize, (Textsw textsw, Textsw_index pos));
EXTERN_FUNCTION (void textsw_set_selection, (Textsw textsw, Textsw_index first, Textsw_index last_plus_one, unsigned int type));

#ifdef OW_I18N
EXTERN_FUNCTION (void textsw_normalize_view_wc, (Textsw textsw, Textsw_index pos));
EXTERN_FUNCTION (void textsw_possibly_normalize_wc, (Textsw textsw, Textsw_index pos));
EXTERN_FUNCTION (void textsw_set_selection_wcs, (Textsw textsw, Textsw_index first, Textsw_index last_plus_one, unsigned int type));
#endif /* OW_I18N */

#endif /* _OTHER_TEXTSW_FUNCTIONS */

#endif /* xview_textsw_DEFINED */
