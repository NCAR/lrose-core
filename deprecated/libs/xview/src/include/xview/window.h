/*      @(#)window.h 20.99 93/06/28 SMI      */

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#ifndef xview_window_DEFINED
#define xview_window_DEFINED

/*
 ***********************************************************************
 *			Include Files
 ***********************************************************************
 */

#include <xview/generic.h>
#include <xview/server.h>
#include <xview/screen.h>
#include <xview/drawable.h>
#include <xview/win_input.h>
#include <xview/rect.h>
#include <X11/Xlib.h>

/*
 ***********************************************************************
 *			Definitions and Macros
 ***********************************************************************
 */

/*
 * PUBLIC #defines 
 */

#define	WINDOW				&xv_window_pkg

#define WIN_EXTEND_TO_EDGE 		-1
#define WIN_DEFAULT_BORDER_WIDTH	1
#define WIN_MESSAGE_DATA_SIZE	        20

#define window_main_loop	xv_main_loop

typedef enum {                  /* values for WIN_SCALE_STATE */
	WIN_SCALE_SMALL,
	WIN_SCALE_MEDIUM,
	WIN_SCALE_LARGE,
	WIN_SCALE_EXTRALARGE
} Window_rescale_state;

#ifndef ForgetGravity
/* Bit Gravity */
 
#define ForgetGravity           0
#define NorthWestGravity        1
#define NorthGravity            2
#define NorthEastGravity        3
#define WestGravity             4
#define CenterGravity           5
#define EastGravity             6
#define SouthWestGravity        7
#define SouthGravity            8
#define SouthEastGravity        9
#define StaticGravity           10

 /* Window gravity + bit gravity above */

#define UnmapGravity            0
#endif


/*
 * Useful conversion macros
 */
#define XV_SCREEN_FROM_WINDOW(window) \
	(Xv_Screen) xv_get(window, XV_SCREEN)

#define XV_SERVER_FROM_WINDOW(window) \
	(Xv_Server) xv_get(xv_get(window, XV_SCREEN), SCREEN_SERVER)

#define XV_DISPLAY_FROM_WINDOW(window) \
	(Display *) xv_get(xv_get(xv_get(window, XV_SCREEN), \
		SCREEN_SERVER), XV_DISPLAY)

/*
 * Window-fitting macros 
 */
#define window_fit(win) \
   (void)window_set(win, WIN_FIT_HEIGHT, 0, WIN_FIT_WIDTH, 0, NULL)

#define window_fit_height(win) \
   (void)window_set(win, WIN_FIT_HEIGHT, 0, NULL)

#define window_fit_width(win) \
   (void)window_set(win, WIN_FIT_WIDTH, 0, NULL)

/*
 * PRIVATE #defines
 */

/*
 * Attribute macros 
 */
#define	WIN_ATTR(type, ordinal)		ATTR(ATTR_PKG_WIN, type, ordinal)
#define WIN_ATTR_LIST(ltype, type, ordinal) \
		WIN_ATTR(ATTR_LIST_INLINE((ltype), (type)), (ordinal))
/*
 * Fake types -- This should be resolved someday 
 */
#define ATTR_IMASK			ATTR_OPAQUE

/*
 * WIN_RECT_INFO flags for package implementors 
 */
#define	WIN_X_SET			0x1
#define	WIN_Y_SET			0x2
#define	WIN_WIDTH_SET			0x4
#define	WIN_HEIGHT_SET			0x8

/*
 * PUBLIC #defines
 * For SunView 1 Compatibility Only 
 */

#define WIN_X				XV_X
#define WIN_Y				XV_Y
#define WIN_WIDTH			XV_WIDTH
#define WIN_HEIGHT			XV_HEIGHT
#define WIN_FONT			XV_FONT
#define WIN_DEVICE_NAME			XV_XNAME
#define WIN_DEVICE_NUMBER		XV_DEVICE_NUMBER
#define WIN_TOP_MARGIN			XV_TOP_MARGIN
#define WIN_BOTTOM_MARGIN		XV_BOTTOM_MARGIN
#define WIN_LEFT_MARGIN			XV_LEFT_MARGIN
#define WIN_RIGHT_MARGIN		XV_RIGHT_MARGIN
#define WIN_NAME			XV_NAME
#define WIN_OWNER			XV_OWNER
#define WIN_FD				XV_SELF
#define WIN_PIXWIN			XV_SELF
#define WIN_RECT			XV_RECT

#define WINDOW_TYPE			ATTR_PKG_WIN

#define	WIN_CONSUME_KBD_EVENT		WIN_CONSUME_EVENT
#define	WIN_IGNORE_KBD_EVENT		WIN_IGNORE_EVENT
#define	WIN_CONSUME_KBD_EVENTS		WIN_CONSUME_EVENTS	
#define	WIN_IGNORE_KBD_EVENTS		WIN_IGNORE_EVENTS
#define	WIN_PICK_INPUT_MASK		WIN_INPUT_MASK
#define	WIN_CONSUME_PICK_EVENT		WIN_CONSUME_EVENT
#define	WIN_IGNORE_PICK_EVENT		WIN_IGNORE_EVENT
#define	WIN_CONSUME_PICK_EVENTS		WIN_CONSUME_EVENTS
#define	WIN_IGNORE_PICK_EVENTS		WIN_IGNORE_EVENTS

#define WIN_NOTIFY_EVENT_PROC		WIN_NOTIFY_SAFE_EVENT_PROC

#ifdef OW_I18N
#define WIN_IM_PREEDIT_START 		WIN_IC_PREEDIT_START
#define	WIN_IM_PREEDIT_DRAW  		WIN_IC_PREEDIT_DRAW
#define	WIN_IM_PREEDIT_CARET  		WIN_IC_PREEDIT_CARET
#define	WIN_IM_PREEDIT_DONE  		WIN_IC_PREEDIT_DONE
#define	WIN_IM_STATUS_START  		WIN_IC_STATUS_START
#define	WIN_IM_STATUS_DRAW   		WIN_IC_STATUS_DRAW
#define	WIN_IM_STATUS_DONE   		WIN_IC_STATUS_DONE
#define	WIN_IM_LUC_START     		WIN_IC_LUC_START
#define	WIN_IM_LUC_DRAW      		WIN_IC_LUC_DRAW
#define	WIN_IM_LUC_DONE      		WIN_IC_LUC_DONE
#define	WIN_IM_LUC_PROCESS   		WIN_IC_LUC_PROCESS
#endif /* OW_I18N */


/*
 ***********************************************************************
 *		Typedefs, Enumerations, and Structures
 ***********************************************************************
 */

typedef	Xv_opaque 		Xv_Window;
typedef	Xv_opaque 		Xv_window;
typedef	Attr_pkg	 	Window_type;

typedef enum {
	/*
	 * Public Attributes 
	 */
	__WIN_ALARM		= WIN_ATTR(ATTR_NO_VALUE, 	  1),	/* --S */
	__WIN_BACK		= WIN_ATTR(ATTR_NO_VALUE,	  2),   /* -S- */
	__WIN_BACKGROUND_PIXMAP	= WIN_ATTR(ATTR_OPAQUE,		  3),
	__WIN_BELOW		= WIN_ATTR(ATTR_OPAQUE,		  4),	/* --S */
	__WIN_CLIENT_DATA	= WIN_ATTR(ATTR_OPAQUE,		  8),
	__WIN_COLUMNS		= WIN_ATTR(ATTR_INT,		 12),
	__WIN_COLUMN_GAP	= WIN_ATTR(ATTR_INT,		 16),
	__WIN_COLUMN_WIDTH	= WIN_ATTR(ATTR_INT,		 20),
#ifdef OW_I18N
        __WIN_IC_PREEDIT_START  = WIN_ATTR(ATTR_INT_PAIR,        21),   /* C-S */
        __WIN_IC_PREEDIT_DRAW   = WIN_ATTR(ATTR_INT_PAIR,        22),   /* C-S */
        __WIN_IC_PREEDIT_DONE   = WIN_ATTR(ATTR_INT_PAIR,        23),   /* C-S */
#endif /* OW_I18N */
	__WIN_CONSUME_EVENT	= WIN_ATTR(ATTR_ENUM,		 24),	/* --S */
#ifdef OW_I18N
        __WIN_IC_STATUS_START   = WIN_ATTR(ATTR_INT_PAIR,        25),   /* C-S */
        __WIN_IC_STATUS_DRAW    = WIN_ATTR(ATTR_INT_PAIR,        26),   /* C-S */
        __WIN_IC_STATUS_DONE    = WIN_ATTR(ATTR_INT_PAIR,        27),   /* C-S */
#endif /* OW_I18N */
	__WIN_CONSUME_EVENTS	
			= WIN_ATTR_LIST(ATTR_NULL, ATTR_ENUM,	 28),	/* --S */
#ifdef OW_I18N
        __WIN_IC_LUC_START      = WIN_ATTR(ATTR_INT_PAIR,        29),   /* C-S */
        __WIN_IC_LUC_DRAW       = WIN_ATTR(ATTR_INT_PAIR,        30),   /* C-S */
        __WIN_IC_LUC_DONE       = WIN_ATTR(ATTR_INT_PAIR,        31),   /* C-S */
#endif /* OW_I18N */
	__WIN_CURSOR		= WIN_ATTR(ATTR_CURSOR_PTR,	 32),
#ifdef OW_I18N
        __WIN_IC_LUC_PROCESS    = WIN_ATTR(ATTR_INT_PAIR,        33),   /* C-S */
        __WIN_IC_PREEDIT_CARET  = WIN_ATTR(ATTR_INT_PAIR,        34),   /* C-S */
#endif /* OW_I18N */

	__WIN_DEPTH		= WIN_ATTR(ATTR_INT,		102),	/* CG- */
	/* WIN_DEPTH_V2 is needed to keep backwards compatibility with
	 * V2.  WIN_DEPTH was mistakenly defined as and ATTR_NO_VALUE,
	 * which meant that despite the documentation you couldn't
	 * use it in the creation of a window.  Thus we created
	 * a new WIN_DEPTH that is of the correct type, and moved
	 * the old WIN_DEPTH to WIN_DEPTH_V2.  This can be removed
	 * when it has sufficiently aged.
	 */
	__WIN_DEPTH_V2		= WIN_ATTR(ATTR_NO_VALUE,	 36),	/* -G- */

	__WIN_DESIRED_HEIGHT	= WIN_ATTR(ATTR_INT, 		 40),
	__WIN_DESIRED_WIDTH	= WIN_ATTR(ATTR_INT, 		 44),
	__WIN_ERROR_MSG		= WIN_ATTR(ATTR_STRING,		 48),
#ifdef  OW_I18N
        WIN_ERROR_MSG_WCS       = WIN_ATTR(ATTR_WSTRING,          49),
#endif  /* OW_I18N */
	__WIN_EVENT_PROC	= WIN_ATTR(ATTR_FUNCTION_PTR,	 52),
	__WIN_FIT_HEIGHT	= WIN_ATTR(ATTR_Y,		 60),
	__WIN_FIT_WIDTH		= WIN_ATTR(ATTR_X,		 64),
	__WIN_FRONT		= WIN_ATTR(ATTR_NO_VALUE,	 66),   /* -S- */
	__WIN_GLYPH_FONT	= WIN_ATTR(ATTR_OPAQUE,		 67),
	__WIN_GRAB_ALL_INPUT	= WIN_ATTR(ATTR_BOOLEAN,	 68),	/* --S */
	__WIN_HORIZONTAL_SCROLLBAR= WIN_ATTR(ATTR_OPAQUE,		 72),
#ifdef OW_I18N
#ifdef  FULL_R5
	__WIN_X_IM_STYLE_MASK	= WIN_ATTR(ATTR_LONG,		 73),	/* CG- */
#endif  /* FULL_R5 */
        __WIN_IC                = WIN_ATTR(ATTR_OPAQUE,          74),   /* -GS */
        __WIN_USE_IM            = WIN_ATTR(ATTR_BOOLEAN,         75),   /* CG- */
#endif /* OW_I18N */
	__WIN_IGNORE_EVENT	= WIN_ATTR(ATTR_ENUM,		 76),	/* --S */
	__WIN_IGNORE_EVENTS
			= WIN_ATTR_LIST(ATTR_NULL, ATTR_ENUM,	 80),	/* --S */
#ifdef OW_I18N
	__WIN_IC_COMMIT_STRING	= WIN_ATTR(ATTR_STRING,		 81),  /* -G- */
	__WIN_IC_COMMIT_STRING_WCS= WIN_ATTR(ATTR_WSTRING,	 82),  /* -G- */
	__WIN_IC_CONVERSION	= WIN_ATTR(ATTR_BOOLEAN,	 83),  /* -GS */
#endif /* OW_I18N */
	__WIN_INPUT_MASK	= WIN_ATTR(ATTR_IMASK,		 84),
#ifdef OW_I18N
	__WIN_IC_RESET		= WIN_ATTR(ATTR_NO_VALUE,	 85),  /* --S */
	__WIN_IC_ACTIVE		= WIN_ATTR(ATTR_BOOLEAN,	 86),  /* CGS */
#endif /* OW_I18N */
	__WIN_IS_CLIENT_PANE	= WIN_ATTR(ATTR_NO_VALUE,	 88),	
	__WIN_MENU		= WIN_ATTR(ATTR_OPAQUE,		 92),
	__WIN_MOUSE_XY		= WIN_ATTR(ATTR_XY,		 96),	/* -GS */
	__WIN_PARENT		= WIN_ATTR(ATTR_OPAQUE,		100),	/* -GS */
	__WIN_PERCENT_HEIGHT	= WIN_ATTR(ATTR_INT,		104),
	__WIN_PERCENT_WIDTH	= WIN_ATTR(ATTR_INT, 		108),
	__WIN_RIGHT_OF		= WIN_ATTR(ATTR_OPAQUE,		116),	/* --S */
	__WIN_ROWS		= WIN_ATTR(ATTR_INT,		120),
	__WIN_ROW_GAP		= WIN_ATTR(ATTR_INT,		124),
	__WIN_ROW_HEIGHT	= WIN_ATTR(ATTR_INT,		128),
	__WIN_SCREEN_RECT	= WIN_ATTR(ATTR_NO_VALUE,	132),	/* -G- */
	__WIN_SET_FOCUS		= WIN_ATTR(ATTR_NO_VALUE,	228),	/* --S */
	__WIN_VERTICAL_SCROLLBAR= WIN_ATTR(ATTR_OPAQUE,		140),
	__WIN_MESSAGE_TYPE	= WIN_ATTR(ATTR_OPAQUE, 	141),
	__WIN_MESSAGE_FORMAT	= WIN_ATTR(ATTR_INT, 		142),
	__WIN_MESSAGE_DATA	= WIN_ATTR(ATTR_OPAQUE,		143),
	__WIN_BORDER		= WIN_ATTR(ATTR_BOOLEAN, 	148),
	/*
	 * Private Attributes 
	 */
	__WIN_ALARM_DATA	= WIN_ATTR(ATTR_OPAQUE, 	144),	/* -G- */
	__WIN_FINDINTERSECT	= WIN_ATTR(ATTR_XY, 		152),	/* -G- */
	__WIN_FRAME		= WIN_ATTR(ATTR_OPAQUE, 	156),
	__WIN_INPUT_ONLY	= WIN_ATTR(ATTR_NO_VALUE,	160),
	__WIN_IS_IN_FULLSCREEN_MODE = WIN_ATTR(ATTR_INT, 		164),
	__WIN_IS_ROOT		= WIN_ATTR(ATTR_NO_VALUE,	168),
	__WIN_KBD_FOCUS		= WIN_ATTR(ATTR_BOOLEAN, 	172),
	__WIN_LAYOUT_PROC	= WIN_ATTR(ATTR_FUNCTION_PTR,	176),
	__WIN_LOCKDATA		= WIN_ATTR(ATTR_NO_VALUE, 	180), 	/* --S */
	__WIN_MAP		= WIN_ATTR(ATTR_BOOLEAN,	184),
	__WIN_NOTIFY_SAFE_EVENT_PROC 	= WIN_ATTR(ATTR_FUNCTION_PTR,	192),
	__WIN_NOTIFY_IMMEDIATE_EVENT_PROC	= WIN_ATTR(ATTR_FUNCTION_PTR,	193),
	__WIN_OUTER_RECT	= WIN_ATTR(ATTR_RECT_PTR, 	200),	
	__WIN_RECT_INFO		= WIN_ATTR(ATTR_INT, 		204),
	__WIN_RETAINED		= WIN_ATTR(ATTR_BOOLEAN, 	208),
	__WIN_SELECTION_WINDOW	= WIN_ATTR(ATTR_NO_VALUE, 	212),	/* --S */
	__WIN_TOP_LEVEL		= WIN_ATTR(ATTR_BOOLEAN, 	216),
	__WIN_TOP_LEVEL_NO_DECOR= WIN_ATTR(ATTR_BOOLEAN, 	220),
	__WIN_TRANSPARENT	= WIN_ATTR(ATTR_NO_VALUE,	223),	/* C-- */
	__WIN_SAVE_UNDER	= WIN_ATTR(ATTR_BOOLEAN, 	226),
	__WIN_REMOVE_CARET	= WIN_ATTR(ATTR_NO_VALUE,	227),   /* --S */
	__WIN_X_PAINT_WINDOW	= WIN_ATTR(ATTR_BOOLEAN,        229),
	__WIN_INHERIT_COLORS	= WIN_ATTR(ATTR_BOOLEAN,        230),
	__WIN_CMS		= WIN_ATTR(ATTR_OPAQUE,		231),
        __WIN_DYNAMIC_VISUAL    = WIN_ATTR(ATTR_BOOLEAN,        232),
	__WIN_CMS_CHANGE	= WIN_ATTR(ATTR_NO_VALUE,	241),
	__WIN_COLOR_INFO	= WIN_ATTR(ATTR_OPAQUE, 	242),
	__WIN_CMD_LINE		= WIN_ATTR(ATTR_STRING, 	244),
	__WIN_NO_CLIPPING	= WIN_ATTR(ATTR_BOOLEAN,	245),
	__WIN_ADD_DROP_INTEREST = WIN_ATTR(ATTR_OPAQUE,         246),
	__WIN_DELETE_DROP_INTEREST  = WIN_ATTR(ATTR_OPAQUE,       247),
	__WIN_ADD_DROP_ITEM     = WIN_ATTR(ATTR_OPAQUE,         252),
	__WIN_DELETE_DROP_ITEM  = WIN_ATTR(ATTR_LONG,           253),
	__WIN_UNGRAB_SELECT	= WIN_ATTR(ATTR_NO_VALUE,	255),
	__WIN_COLLAPSE_MOTION_EVENTS = WIN_ATTR(ATTR_OPAQUE,	201),

	/*
         * Public Attributes
         */
	__WIN_X_CLIP_RECTS	= WIN_ATTR(ATTR_OPAQUE,		233),  /* -G- */
	__WIN_CMS_DATA		= WIN_ATTR(ATTR_OPAQUE,		235),
	__WIN_CMS_NAME		= WIN_ATTR(ATTR_STRING,		236),
	__WIN_BIT_GRAVITY       = WIN_ATTR(ATTR_INT,            237),
	__WIN_WINDOW_GRAVITY    = WIN_ATTR(ATTR_INT,            238),
	__WIN_FOREGROUND_COLOR	= WIN_ATTR(ATTR_INT,            239),
	__WIN_BACKGROUND_COLOR	= WIN_ATTR(ATTR_INT,            240),
	__WIN_X_COLOR_INDICES	= WIN_ATTR(ATTR_OPAQUE,         243),
	__WIN_CONSUME_X_EVENT_MASK = WIN_ATTR(ATTR_LONG,		248),
	__WIN_IGNORE_X_EVENT_MASK = WIN_ATTR(ATTR_LONG,		249),
	__WIN_X_EVENT_MASK 	= WIN_ATTR(ATTR_LONG,		250),
	__WIN_COLLAPSE_EXPOSURES= WIN_ATTR(ATTR_BOOLEAN,	251),
	__WIN_SOFT_FNKEY_LABELS = WIN_ATTR(ATTR_STRING, 	203),

	/*
	 * Public Attributes 
	 * For SunView 1 Compatibility
	 */
	__WIN_TYPE		= WIN_ATTR(ATTR_ENUM,		224)
} Window_attribute;

#define WIN_ALARM ((Attr_attribute) __WIN_ALARM)
#define WIN_BACK ((Attr_attribute) __WIN_BACK)
#define WIN_BACKGROUND_PIXMAP ((Attr_attribute) __WIN_BACKGROUND_PIXMAP)
#define WIN_BELOW ((Attr_attribute) __WIN_BELOW)
#define WIN_CLIENT_DATA ((Attr_attribute) __WIN_CLIENT_DATA)
#define WIN_COLUMNS ((Attr_attribute) __WIN_COLUMNS)
#define WIN_COLUMN_GAP ((Attr_attribute) __WIN_COLUMN_GAP)
#define WIN_COLUMN_WIDTH ((Attr_attribute) __WIN_COLUMN_WIDTH)
#ifdef OW_I18N
#define WIN_IC_PREEDIT_START ((Attr_attribute) __WIN_IC_PREEDIT_START)
#define WIN_IC_PREEDIT_DRAW ((Attr_attribute) __WIN_IC_PREEDIT_DRAW)
#define WIN_IC_PREEDIT_DONE ((Attr_attribute) __WIN_IC_PREEDIT_DONE)
#endif /* OW_I18N */
#define WIN_CONSUME_EVENT ((Attr_attribute) __WIN_CONSUME_EVENT)
#ifdef OW_I18N
#define WIN_IC_STATUS_START ((Attr_attribute) __WIN_IC_STATUS_START)
#define WIN_IC_STATUS_DRAW ((Attr_attribute) __WIN_IC_STATUS_DRAW)
#define WIN_IC_STATUS_DONE ((Attr_attribute) __WIN_IC_STATUS_DONE)
#endif /* OW_I18N */
#define WIN_CONSUME_EVENTS ((Attr_attribute) __WIN_CONSUME_EVENTS)
#ifdef OW_I18N
#define WIN_IC_LUC_START ((Attr_attribute) __WIN_IC_LUC_START)
#define WIN_IC_LUC_DRAW ((Attr_attribute) __WIN_IC_LUC_DRAW)
#define WIN_IC_LUC_DONE ((Attr_attribute) __WIN_IC_LUC_DONE)
#endif /* OW_I18N */
#define WIN_CURSOR ((Attr_attribute) __WIN_CURSOR)
#ifdef OW_I18N
#define WIN_IC_LUC_PROCESS ((Attr_attribute) __WIN_IC_LUC_PROCESS)
#define WIN_IC_PREEDIT_CARET ((Attr_attribute) __WIN_IC_PREEDIT_CARET)
#endif /* OW_I18N */

#define WIN_DEPTH ((Attr_attribute) __WIN_DEPTH)
#define WIN_DEPTH_V2 ((Attr_attribute) __WIN_DEPTH_V2)
#define WIN_DESIRED_HEIGHT ((Attr_attribute) __WIN_DESIRED_HEIGHT)
#define WIN_DESIRED_WIDTH ((Attr_attribute) __WIN_DESIRED_WIDTH)
#define WIN_ERROR_MSG ((Attr_attribute) __WIN_ERROR_MSG)
#ifdef  OW_I18N
#define WIN_ERROR_MSG_WCS ((Attr_attribute) __WIN_ERROR_MSG_WCS)
#endif  /* OW_I18N */
#define WIN_EVENT_PROC ((Attr_attribute) __WIN_EVENT_PROC)
#define WIN_FIT_HEIGHT ((Attr_attribute) __WIN_FIT_HEIGHT)
#define WIN_FIT_WIDTH ((Attr_attribute) __WIN_FIT_WIDTH)
#define WIN_FRONT ((Attr_attribute) __WIN_FRONT)
#define WIN_GLYPH_FONT ((Attr_attribute) __WIN_GLYPH_FONT)
#define WIN_GRAB_ALL_INPUT ((Attr_attribute) __WIN_GRAB_ALL_INPUT)
#define WIN_HORIZONTAL_SCROLLBAR ((Attr_attribute) __WIN_HORIZONTAL_SCROLLBAR)
#ifdef OW_I18N
#ifdef  FULL_R5
#define WIN_X_IM_STYLE_MASK ((Attr_attribute) __WIN_X_IM_STYLE_MASK)
#endif  /* FULL_R5 */
#define WIN_IC ((Attr_attribute) __WIN_IC)
#define WIN_USE_IM ((Attr_attribute) __WIN_USE_IM)
#endif /* OW_I18N */
#define WIN_IGNORE_EVENT ((Attr_attribute) __WIN_IGNORE_EVENT)
#define WIN_IGNORE_EVENTS ((Attr_attribute) __WIN_IGNORE_EVENTS)
#ifdef OW_I18N
#define WIN_IC_COMMIT_STRING ((Attr_attribute) __WIN_IC_COMMIT_STRING)
#define WIN_IC_COMMIT_STRING_WCS ((Attr_attribute) __WIN_IC_COMMIT_STRING_WCS)
#define WIN_IC_CONVERSION ((Attr_attribute) __WIN_IC_CONVERSION)
#endif /* OW_I18N */
#define WIN_INPUT_MASK ((Attr_attribute) __WIN_INPUT_MASK)
#ifdef OW_I18N
#define WIN_IC_RESET ((Attr_attribute) __WIN_IC_RESET)
#define WIN_IC_ACTIVE ((Attr_attribute) __WIN_IC_ACTIVE)
#endif /* OW_I18N */
#define WIN_IS_CLIENT_PANE ((Attr_attribute) __WIN_IS_CLIENT_PANE)
#define WIN_MENU ((Attr_attribute) __WIN_MENU)
#define WIN_MOUSE_XY ((Attr_attribute) __WIN_MOUSE_XY)
#define WIN_PARENT ((Attr_attribute) __WIN_PARENT)
#define WIN_PERCENT_HEIGHT ((Attr_attribute) __WIN_PERCENT_HEIGHT)
#define WIN_PERCENT_WIDTH ((Attr_attribute) __WIN_PERCENT_WIDTH)
#define WIN_RIGHT_OF ((Attr_attribute) __WIN_RIGHT_OF)
#define WIN_ROWS ((Attr_attribute) __WIN_ROWS)
#define WIN_ROW_GAP ((Attr_attribute) __WIN_ROW_GAP)
#define WIN_ROW_HEIGHT ((Attr_attribute) __WIN_ROW_HEIGHT)
#define WIN_SCREEN_RECT ((Attr_attribute) __WIN_SCREEN_RECT)
#define WIN_SET_FOCUS ((Attr_attribute) __WIN_SET_FOCUS)
#define WIN_VERTICAL_SCROLLBAR ((Attr_attribute) __WIN_VERTICAL_SCROLLBAR)
#define WIN_MESSAGE_TYPE ((Attr_attribute) __WIN_MESSAGE_TYPE)
#define WIN_MESSAGE_FORMAT ((Attr_attribute) __WIN_MESSAGE_FORMAT)
#define WIN_MESSAGE_DATA ((Attr_attribute) __WIN_MESSAGE_DATA)
#define WIN_BORDER ((Attr_attribute) __WIN_BORDER)
#define WIN_ALARM_DATA ((Attr_attribute) __WIN_ALARM_DATA)
#define WIN_FINDINTERSECT ((Attr_attribute) __WIN_FINDINTERSECT)
#define WIN_FRAME ((Attr_attribute) __WIN_FRAME)
#define WIN_INPUT_ONLY ((Attr_attribute) __WIN_INPUT_ONLY)
#define WIN_IS_IN_FULLSCREEN_MODE ((Attr_attribute) __WIN_IS_IN_FULLSCREEN_MODE)
#define WIN_IS_ROOT ((Attr_attribute) __WIN_IS_ROOT)
#define WIN_KBD_FOCUS ((Attr_attribute) __WIN_KBD_FOCUS)
#define WIN_LAYOUT_PROC ((Attr_attribute) __WIN_LAYOUT_PROC)
#define WIN_LOCKDATA ((Attr_attribute) __WIN_LOCKDATA)
#define WIN_MAP ((Attr_attribute) __WIN_MAP)
#define WIN_NOTIFY_SAFE_EVENT_PROC ((Attr_attribute) __WIN_NOTIFY_SAFE_EVENT_PROC)
#define WIN_NOTIFY_IMMEDIATE_EVENT_PROC ((Attr_attribute) __WIN_NOTIFY_IMMEDIATE_EVENT_PROC)
#define WIN_OUTER_RECT ((Attr_attribute) __WIN_OUTER_RECT)
#define WIN_RECT_INFO ((Attr_attribute) __WIN_RECT_INFO)
#define WIN_RETAINED ((Attr_attribute) __WIN_RETAINED)
#define WIN_SELECTION_WINDOW ((Attr_attribute) __WIN_SELECTION_WINDOW)
#define WIN_TOP_LEVEL ((Attr_attribute) __WIN_TOP_LEVEL)
#define WIN_TOP_LEVEL_NO_DECOR ((Attr_attribute) __WIN_TOP_LEVEL_NO_DECOR)
#define WIN_TRANSPARENT ((Attr_attribute) __WIN_TRANSPARENT)
#define WIN_SAVE_UNDER ((Attr_attribute) __WIN_SAVE_UNDER)
#define WIN_REMOVE_CARET ((Attr_attribute) __WIN_REMOVE_CARET)
#define WIN_X_PAINT_WINDOW ((Attr_attribute) __WIN_X_PAINT_WINDOW)
#define WIN_INHERIT_COLORS ((Attr_attribute) __WIN_INHERIT_COLORS)
#define WIN_CMS ((Attr_attribute) __WIN_CMS)
#define WIN_DYNAMIC_VISUAL ((Attr_attribute) __WIN_DYNAMIC_VISUAL)
#define WIN_CMS_CHANGE ((Attr_attribute) __WIN_CMS_CHANGE)
#define WIN_COLOR_INFO ((Attr_attribute) __WIN_COLOR_INFO)
#define WIN_CMD_LINE ((Attr_attribute) __WIN_CMD_LINE)
#define WIN_NO_CLIPPING ((Attr_attribute) __WIN_NO_CLIPPING)
#define WIN_ADD_DROP_INTEREST ((Attr_attribute) __WIN_ADD_DROP_INTEREST)
#define WIN_DELETE_DROP_INTEREST ((Attr_attribute) __WIN_DELETE_DROP_INTEREST)
#define WIN_ADD_DROP_ITEM ((Attr_attribute) __WIN_ADD_DROP_ITEM)
#define WIN_DELETE_DROP_ITEM ((Attr_attribute) __WIN_DELETE_DROP_ITEM)
#define WIN_UNGRAB_SELECT ((Attr_attribute) __WIN_UNGRAB_SELECT)
#define WIN_COLLAPSE_MOTION_EVENTS ((Attr_attribute) __WIN_COLLAPSE_MOTION_EVENTS)
#define WIN_X_CLIP_RECTS ((Attr_attribute) __WIN_X_CLIP_RECTS)
#define WIN_CMS_DATA ((Attr_attribute) __WIN_CMS_DATA)
#define WIN_CMS_NAME ((Attr_attribute) __WIN_CMS_NAME)
#define WIN_BIT_GRAVITY ((Attr_attribute) __WIN_BIT_GRAVITY)
#define WIN_WINDOW_GRAVITY ((Attr_attribute) __WIN_WINDOW_GRAVITY)
#define WIN_FOREGROUND_COLOR ((Attr_attribute) __WIN_FOREGROUND_COLOR)
#define WIN_BACKGROUND_COLOR ((Attr_attribute) __WIN_BACKGROUND_COLOR)
#define WIN_X_COLOR_INDICES ((Attr_attribute) __WIN_X_COLOR_INDICES)
#define WIN_CONSUME_X_EVENT_MASK ((Attr_attribute) __WIN_CONSUME_X_EVENT_MASK)
#define WIN_IGNORE_X_EVENT_MASK ((Attr_attribute) __WIN_IGNORE_X_EVENT_MASK)
#define WIN_X_EVENT_MASK ((Attr_attribute) __WIN_X_EVENT_MASK)
#define WIN_COLLAPSE_EXPOSURES ((Attr_attribute) __WIN_COLLAPSE_EXPOSURES)
#define WIN_SOFT_FNKEY_LABELS ((Attr_attribute) __WIN_SOFT_FNKEY_LABELS)
#define WIN_TYPE ((Attr_attribute) __WIN_TYPE)


#define WIN_SHOW	XV_SHOW

typedef enum {
	WIN_NULL_VALUE = 0,
	WIN_NO_EVENTS,
	WIN_UP_EVENTS,
	WIN_ASCII_EVENTS,
	WIN_UP_ASCII_EVENTS,
	WIN_MOUSE_BUTTONS,
	WIN_IN_TRANSIT_EVENTS,
	WIN_LEFT_KEYS,
	WIN_TOP_KEYS,
	WIN_RIGHT_KEYS,
	WIN_META_EVENTS,
	WIN_UP_META_EVENTS,
	/*
 	 * semantic event classes 
 	 */
	WIN_SUNVIEW_FUNCTION_KEYS,
	WIN_EDIT_KEYS,
	WIN_MOTION_KEYS,
	WIN_TEXT_KEYS
} Window_input_event;

typedef enum {
	WIN_CREATE, 
	WIN_INSERT,
	WIN_REMOVE,
	WIN_DESTROY,
	WIN_GET_RIGHT_OF, 
	WIN_GET_BELOW, 
	WIN_ADJUST_RECT, 
	WIN_GET_X, 
	WIN_GET_Y, 
	WIN_GET_WIDTH, 
	WIN_GET_HEIGHT,
	WIN_GET_RECT, 
	WIN_LAYOUT,
	WIN_INSTALL
} Window_layout_op;

typedef struct {
	Xv_drawable_struct	parent_data;
	Xv_opaque		private_data;
} Xv_window_struct;

typedef struct window_rescale_rect_obj {
    Rect        old_rect;
    Rect        new_rect;
    int         width_change, height_change,x_change,y_change;
    int         adjusted;
    Xv_Window   sw;
/* relationships */
} Window_rescale_rect_obj;

/*
 ***********************************************************************
 *				Globals
 ***********************************************************************
 */

extern Xv_pkg		xv_window_pkg;

/*
 * PUBLIC functions 
 */
EXTERN_FUNCTION (int window_done, (Xv_Window window));
EXTERN_FUNCTION (void xv_main_loop, (Xv_Window window));
EXTERN_FUNCTION (int window_read_event, (Xv_Window window, Event *event));
EXTERN_FUNCTION (void window_bell, (Xv_Window window));
EXTERN_FUNCTION (Xv_public int xv_rows, (Xv_Window window, int rows));
EXTERN_FUNCTION (Xv_public int xv_row, (Xv_Window window, int row));
EXTERN_FUNCTION (Xv_public int xv_cols, (Xv_Window window, int cols));
EXTERN_FUNCTION (Xv_public int xv_col, (Xv_Window window, int col));
EXTERN_FUNCTION (Xv_public int xv_send_message, (Xv_window window, Xv_opaque addresse, char *msg_type, int format, Xv_opaque *data, int len));

/*
 * PRIVATE functions
 */

EXTERN_FUNCTION (void window_set_cache_rect, (Xv_Window window, Rect *rect));

/*
 * The Xv_opaque type for the rect object list is defined in a private 
 * include file windowimpl.h
 */
EXTERN_FUNCTION (Window_rescale_rect_obj *window_create_rect_obj_list, (int num_elems));
EXTERN_FUNCTION (void window_destroy_rect_obj_list, (Window_rescale_rect_obj *rect_obj_list));
EXTERN_FUNCTION (void window_add_to_rect_list, (Window_rescale_rect_obj *rect_obj_list, Xv_Window window, Rect *rect, int i));
EXTERN_FUNCTION (int window_rect_equal_ith_obj, (Window_rescale_rect_obj *rect_obj_list, Rect *rect, int i));
EXTERN_FUNCTION (void window_set_client_message, ( Xv_Window window, XClientMessageEvent *msg));
EXTERN_FUNCTION (Xv_opaque * xv_get_selected_windows, (Xv_object window));
EXTERN_FUNCTION (XID win_pointer_under, (Xv_object window, int x, int y));
EXTERN_FUNCTION (int win_translate_xy, (Xv_object src, Xv_object dst, int src_x, int src_y, int *dst_x, int *dst_y));

/*
 * PUBLIC functions 
 * For SunView 1 Compatibility
 */

EXTERN_FUNCTION (Xv_Window window_create, (Xv_Window window, Xv_pkg *pkg, DOTDOTDOT));
EXTERN_FUNCTION (Xv_opaque window_get, (Xv_Window window, Window_attribute attr, DOTDOTDOT));
EXTERN_FUNCTION (int window_set, (Xv_Window window, DOTDOTDOT));
EXTERN_FUNCTION (int window_destroy, (Xv_Window window));


/*
 * PRIVATE functions 
 * For SunView 1 Compatibility 
 */

EXTERN_FUNCTION (void window_rc_units_to_pixels, (Xv_Window win, Attr_avlist attr));

/*
 * Some wmgr stuff that needs to be here for the split libs.
 * This should be moved out as soon as all the pushpin stuff in moved
 * out of the intrinsic layer.  [csk 3/23/89]
 */

/* value for pushpin_initial_state */
#ifndef WMPushpinIsOut
#define WMPushpinIsOut  0
#endif /* WMPushpinIsOut */
#ifndef WMPushpinIsIn
#define WMPushpinIsIn   1
#endif /* WMPushpinIsIn */

#endif /* ~xview_window_DEFINED */
