#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)frame.h 20.77 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE 
 *	file for terms of the license.
 */

#ifndef xview_frame_DEFINED
#define xview_frame_DEFINED

/*
 ***********************************************************************
 *			Include Files
 ***********************************************************************
 */

#include <xview/window.h>
#include <xview/attrol.h>
#include <X11/X.h>

/*
 ***********************************************************************
 *			Definitions and Macros
 ***********************************************************************
 */

/*
 * PUBLIC #defines 
 */

#define FRAME			FRAME_BASE
#define FRAME_BASE		&xv_frame_base_pkg
#define FRAME_CMD		&xv_frame_cmd_pkg
#define FRAME_PROPS		FRAME_CMD
#define FRAME_HELP		&xv_frame_help_pkg
#define FRAME_CLASS		&xv_frame_class_pkg

#define	ROOT_FRAME		((Frame)0)

#define FRAME_SHOW_HEADER	FRAME_SHOW_LABEL
#define FRAME_FOCUS_UP_WIDTH	13
#define FRAME_FOCUS_UP_HEIGHT	13
#define FRAME_FOCUS_RIGHT_WIDTH		13
#define FRAME_FOCUS_RIGHT_HEIGHT	13

/*
 * Utility Macros 
 */

#define frame_fit_all(frame) 					\
{ 								\
    Xv_Window win; 						\
    int n = 1; 							\
    while (win = window_get(frame, FRAME_NTH_SUBWINDOW, n++, NULL))\
	window_fit(win); 					\
    window_fit(frame); 						\
}

#define frame_done_proc(frame) 					\
   (((void (*)())window_get(frame, FRAME_DONE_PROC))(frame))

#define frame_default_done_proc(frame) 				\
   (((void (*)())window_get(frame, FRAME_DEFAULT_DONE_PROC))(frame))

/*
 * PRIVATE #defines 
 */

#define	FRAME_ATTR(type, ordinal)	ATTR(ATTR_PKG_FRAME, type, ordinal)
#define FRAME_ATTR_LIST(ltype, type, ordinal) \
	FRAME_ATTR(ATTR_LIST_INLINE((ltype), (type)), (ordinal))
#define FRAME_ATTR_OPAQUE_5		ATTR_TYPE(ATTR_BASE_OPAQUE, 5)

/*
 * BUG: maybe these should be attributes 
 */


/*
 * width of border around a frame 
 */
#define FRAME_BORDER_WIDTH      (0)
/*
 * width of border around a subwindow 
 */
#define FRAME_SUBWINDOW_SPACING (FRAME_BORDER_WIDTH)


/*
 * PUBLIC #defines 
 * For SunView 1 Compatibility
 */

#define FRAME_TYPE		ATTR_PKG_FRAME

#define	FRAME_ARGS		XV_INIT_ARGS
#define	FRAME_ARGC_PTR_ARGV	XV_INIT_ARGC_PTR_ARGV
#define	FRAME_CMDLINE_HELP_PROC	XV_USAGE_PROC
#define	FRAME_LABEL		XV_LABEL
#ifdef OW_I18N
#define	FRAME_LABEL_WCS		XV_LABEL_WCS
#endif
#define FRAME_OPEN_RECT		WIN_RECT

/*
 ***********************************************************************
 *		Typedefs, Enumerations, and Structures	
 ***********************************************************************
 */

typedef	Xv_opaque 	Frame;
typedef Xv_opaque	Frame_cmd;
typedef Xv_opaque	Frame_props;
typedef Xv_opaque	Frame_help;

typedef struct {
    Xv_window_struct	parent_data;
    Xv_opaque		private_data;
} Xv_frame_class;

typedef struct {
    Xv_frame_class	parent_data;
    Xv_opaque		private_data;
} Xv_frame_base;

typedef Xv_frame_base 	Xv_frame_cmd;
typedef Xv_frame_base 	Xv_frame_props;
typedef Xv_frame_base 	Xv_frame_help;

typedef enum {
    FRAME_FOCUS_UP,
    FRAME_FOCUS_RIGHT
} Frame_focus_direction;

typedef enum {    /* do not change this order */
    FRAME_CMD_PIN_OUT,
    FRAME_CMD_PIN_IN
} Frame_cmd_pin_state;

typedef struct frame_accelerator {
    short	    code;   /* = event->ie_code */
    KeySym	    keysym; /* X keysym */
    void	  (*notify_proc)(); /* accelerator notify proc */
    Xv_opaque	    data;   /* opaque data handle */
    struct frame_accelerator *next;
} Frame_accelerator;
 
typedef enum {
	/*
	 * PUBLIC attributes 
	 */
	__FRAME_BACKGROUND_COLOR= FRAME_ATTR(ATTR_SINGLE_COLOR_PTR,	   5),
	__FRAME_BUSY		= FRAME_ATTR(ATTR_BOOLEAN,		  10),
	__FRAME_CLOSED	= FRAME_ATTR(ATTR_BOOLEAN,		          15),
	__FRAME_CLOSED_RECT	= FRAME_ATTR(ATTR_RECT_PTR,		  20),
	__FRAME_WM_COMMAND_STRINGS = FRAME_ATTR_LIST(ATTR_NULL, ATTR_STRING, 21),
	__FRAME_WM_COMMAND_ARGC_ARGV
				= FRAME_ATTR(ATTR_INT_PAIR, 		  22),
	__FRAME_WM_COMMAND_ARGV	= FRAME_ATTR(ATTR_OPAQUE,		  23),
	__FRAME_WM_COMMAND_ARGC	= FRAME_ATTR(ATTR_INT,			  24),
	__FRAME_CMD_PANEL	= FRAME_ATTR(ATTR_OPAQUE,		  25),
	__FRAME_CURRENT_RECT	= FRAME_ATTR(ATTR_RECT_PTR,		  35),
	__FRAME_OLD_RECT        = FRAME_ATTR(ATTR_RECT_PTR,               36),
	__FRAME_DEFAULT_DONE_PROC = FRAME_ATTR(ATTR_FUNCTION_PTR,	  40),
	__FRAME_DONE_PROC	= FRAME_ATTR(ATTR_FUNCTION_PTR,		  45),
	__FRAME_FOCUS_WIN	= FRAME_ATTR(ATTR_INT_PAIR,		 165),
	__FRAME_FOCUS_DIRECTION	= FRAME_ATTR(ATTR_ENUM,			 170),
	__FRAME_FOREGROUND_COLOR = FRAME_ATTR(ATTR_SINGLE_COLOR_PTR,	  50),
	__FRAME_ICON		= FRAME_ATTR(ATTR_OPAQUE,		  55),
	__FRAME_INHERIT_COLORS	= FRAME_ATTR(ATTR_BOOLEAN,		  60),
	__FRAME_LEFT_FOOTER	= FRAME_ATTR(ATTR_STRING,		  65),
#ifdef OW_I18N
	__FRAME_LEFT_FOOTER_WCS	= FRAME_ATTR(ATTR_WSTRING,		  66),
#endif
	__FRAME_NEXT_PANE	= FRAME_ATTR(ATTR_NO_VALUE,		  67),
	__FRAME_NO_CONFIRM	= FRAME_ATTR(ATTR_BOOLEAN,		  70),
	__FRAME_NTH_SUBFRAME	= FRAME_ATTR(ATTR_INT,			  75),
	__FRAME_NTH_SUBWINDOW	= FRAME_ATTR(ATTR_INT,			  80),
	__FRAME_PREVIOUS_ELEMENT = FRAME_ATTR(ATTR_NO_VALUE,		  81),
	__FRAME_PREVIOUS_PANE	= FRAME_ATTR(ATTR_NO_VALUE,		  82),
	__FRAME_PROPERTIES_PROC	= FRAME_ATTR(ATTR_FUNCTION_PTR,		  85),
	__FRAME_CMD_PUSHPIN_IN	= FRAME_ATTR(ATTR_BOOLEAN,		 105),
	__FRAME_CMD_DEFAULT_PIN_STATE = FRAME_ATTR(ATTR_ENUM,		 106),
	__FRAME_CMD_PIN_STATE	= FRAME_ATTR(ATTR_ENUM,		 	 107),
	__FRAME_RIGHT_FOOTER	= FRAME_ATTR(ATTR_STRING,		 115),
#ifdef OW_I18N
	__FRAME_RIGHT_FOOTER_WCS = FRAME_ATTR(ATTR_WSTRING,		 116),
#endif
	__FRAME_SHOW_FOOTER	= FRAME_ATTR(ATTR_BOOLEAN,		 125),
	__FRAME_SHOW_RESIZE_CORNER = FRAME_ATTR(ATTR_BOOLEAN,		 128),
	__FRAME_SHOW_LABEL	= FRAME_ATTR(ATTR_BOOLEAN,		 130),
	__FRAME_GROUP_LEADER	= FRAME_ATTR(ATTR_BOOLEAN,		 135),
	__FRAME_MIN_SIZE	= FRAME_ATTR(ATTR_INT_PAIR,	 	 136),
	__FRAME_MAX_SIZE	= FRAME_ATTR(ATTR_INT_PAIR,	 	 137),
        /* ACC_XVIEW */
	__FRAME_MENUS		= FRAME_ATTR_LIST(ATTR_NULL,ATTR_OPAQUE,245),
	__FRAME_MENU_ADD	= FRAME_ATTR(ATTR_OPAQUE,		 246),
	__FRAME_MENU_DELETE	= FRAME_ATTR(ATTR_OPAQUE,		 247),
	__FRAME_MENU_COUNT	= FRAME_ATTR(ATTR_INT,			 248),
        /* ACC_XVIEW */
	/*
	 * PRIVATE attributes 
	 */
	__FRAME_NEXT_CHILD	= FRAME_ATTR(ATTR_OPAQUE,		 140),
	__FRAME_PREVIOUS_CHILD	= FRAME_ATTR(ATTR_OPAQUE,		 142),
	__FRAME_SCALE_STATE	= FRAME_ATTR(ATTR_INT,			 145),
	__FRAME_SUBWINDOWS_ADJUSTABLE	
				= FRAME_ATTR(ATTR_BOOLEAN,		 150),
        __FRAME_COUNT           = FRAME_ATTR(ATTR_INT,                   160),
	__FRAME_FOCUS_UP_IMAGE	= FRAME_ATTR(ATTR_OPAQUE,		 175),
	__FRAME_FOCUS_RIGHT_IMAGE = FRAME_ATTR(ATTR_OPAQUE,		 180),
	__FRAME_FOCUS_GC	= FRAME_ATTR(ATTR_OPAQUE,		 185),
	__FRAME_ORPHAN_WINDOW	= FRAME_ATTR(ATTR_INT,			 190),
	__FRAME_FOOTER_WINDOW	= FRAME_ATTR(ATTR_BOOLEAN,               195),
#ifdef OW_I18N
	__FRAME_IMSTATUS_WINDOW	= FRAME_ATTR(ATTR_BOOLEAN,               196),
#endif	
	__FRAME_ACCELERATOR	= FRAME_ATTR(ATTR_INT_TRIPLE,		 200),
	__FRAME_X_ACCELERATOR	= FRAME_ATTR(ATTR_INT_TRIPLE,		 205),
        /* ACC_XVIEW */
	__FRAME_MENU_ACCELERATOR = FRAME_ATTR(ATTR_OPAQUE_TRIPLE,	 207),
	__FRAME_MENU_REMOVE_ACCELERATOR	= FRAME_ATTR(ATTR_STRING,	 208),
	__FRAME_MENU_X_ACCELERATOR = FRAME_ATTR(FRAME_ATTR_OPAQUE_5,     209),
#ifdef OW_I18N
	__FRAME_MENU_ACCELERATOR_WCS = FRAME_ATTR(ATTR_OPAQUE_TRIPLE,    250),
	__FRAME_MENU_REMOVE_ACCELERATOR_WCS= FRAME_ATTR(ATTR_STRING,	 255),
#endif	
        /* ACC_XVIEW */
#ifdef OW_I18N
	__FRAME_LEFT_IMSTATUS_WCS= FRAME_ATTR(ATTR_WSTRING,		 210),
	__FRAME_LEFT_IMSTATUS     = FRAME_ATTR(ATTR_STRING, 		 215),
	__FRAME_RIGHT_IMSTATUS_WCS= FRAME_ATTR(ATTR_WSTRING,		 220),
        __FRAME_RIGHT_IMSTATUS    = FRAME_ATTR(ATTR_STRING, 		 225),
	__FRAME_SHOW_IMSTATUS	= FRAME_ATTR(ATTR_BOOLEAN,               230),
	__FRAME_INACTIVE_IMSTATUS = FRAME_ATTR(ATTR_BOOLEAN,             231),
	__FRAME_CMD_POINTER_WARP = FRAME_ATTR(ATTR_BOOLEAN,		 240),
#ifdef FULL_R5
/* This attr is private to XView */
	__FRAME_IMSTATUS_RECT	= FRAME_ATTR(ATTR_RECT_PTR,		 242),
#endif /* FULL_R5 */	
#endif
	__FRAME_COMPOSE_STATE	= FRAME_ATTR(ATTR_BOOLEAN,               235)
	
} Frame_attribute;


#define FRAME_BACKGROUND_COLOR ((Attr_attribute) __FRAME_BACKGROUND_COLOR)
#define FRAME_BUSY ((Attr_attribute) __FRAME_BUSY)
#define FRAME_CLOSED ((Attr_attribute) __FRAME_CLOSED)
#define FRAME_CLOSED_RECT ((Attr_attribute) __FRAME_CLOSED_RECT)
#define FRAME_WM_COMMAND_STRINGS ((Attr_attribute) __FRAME_WM_COMMAND_STRINGS)
#define FRAME_WM_COMMAND_ARGC_ARGV ((Attr_attribute) __FRAME_WM_COMMAND_ARGC_ARGV)
#define FRAME_WM_COMMAND_ARGV ((Attr_attribute) __FRAME_WM_COMMAND_ARGV)
#define FRAME_WM_COMMAND_ARGC ((Attr_attribute) __FRAME_WM_COMMAND_ARGC)
#define FRAME_CMD_PANEL ((Attr_attribute) __FRAME_CMD_PANEL)
#define FRAME_CURRENT_RECT ((Attr_attribute) __FRAME_CURRENT_RECT)
#define FRAME_OLD_RECT ((Attr_attribute) __FRAME_OLD_RECT)
#define FRAME_DEFAULT_DONE_PROC ((Attr_attribute) __FRAME_DEFAULT_DONE_PROC)
#define FRAME_DONE_PROC ((Attr_attribute) __FRAME_DONE_PROC)
#define FRAME_FOCUS_WIN ((Attr_attribute) __FRAME_FOCUS_WIN)
#define FRAME_FOCUS_DIRECTION ((Attr_attribute) __FRAME_FOCUS_DIRECTION)
#define FRAME_FOREGROUND_COLOR ((Attr_attribute) __FRAME_FOREGROUND_COLOR)
#define FRAME_ICON ((Attr_attribute) __FRAME_ICON)
#define FRAME_INHERIT_COLORS ((Attr_attribute) __FRAME_INHERIT_COLORS)
#define FRAME_LEFT_FOOTER ((Attr_attribute) __FRAME_LEFT_FOOTER)

#ifdef OW_I18N
#define FRAME_LEFT_FOOTER_WCS ((Attr_attribute) __FRAME_LEFT_FOOTER_WCS)
#endif

#define FRAME_NEXT_PANE ((Attr_attribute) __FRAME_NEXT_PANE)
#define FRAME_NO_CONFIRM ((Attr_attribute) __FRAME_NO_CONFIRM)
#define FRAME_NTH_SUBFRAME ((Attr_attribute) __FRAME_NTH_SUBFRAME)
#define FRAME_NTH_SUBWINDOW ((Attr_attribute) __FRAME_NTH_SUBWINDOW)
#define FRAME_PREVIOUS_ELEMENT ((Attr_attribute) __FRAME_PREVIOUS_ELEMENT)
#define FRAME_PREVIOUS_PANE ((Attr_attribute) __FRAME_PREVIOUS_PANE)
#define FRAME_PROPERTIES_PROC ((Attr_attribute) __FRAME_PROPERTIES_PROC)
#define FRAME_CMD_PUSHPIN_IN ((Attr_attribute) __FRAME_CMD_PUSHPIN_IN)
#define FRAME_CMD_DEFAULT_PIN_STATE ((Attr_attribute) __FRAME_CMD_DEFAULT_PIN_STATE)
#define FRAME_CMD_PIN_STATE ((Attr_attribute) __FRAME_CMD_PIN_STATE)
#define FRAME_RIGHT_FOOTER ((Attr_attribute) __FRAME_RIGHT_FOOTER)

#ifdef OW_I18N
#define FRAME_RIGHT_FOOTER_WCS ((Attr_attribute) __FRAME_RIGHT_FOOTER_WCS)
#endif

#define FRAME_SHOW_FOOTER ((Attr_attribute) __FRAME_SHOW_FOOTER)
#define FRAME_SHOW_RESIZE_CORNER ((Attr_attribute) __FRAME_SHOW_RESIZE_CORNER)
#define FRAME_SHOW_LABEL ((Attr_attribute) __FRAME_SHOW_LABEL)
#define FRAME_GROUP_LEADER ((Attr_attribute) __FRAME_GROUP_LEADER)
#define FRAME_MIN_SIZE ((Attr_attribute) __FRAME_MIN_SIZE)
#define FRAME_MAX_SIZE ((Attr_attribute) __FRAME_MAX_SIZE)
#define FRAME_MENUS ((Attr_attribute) __FRAME_MENUS)
#define FRAME_MENU_ADD ((Attr_attribute) __FRAME_MENU_ADD)
#define FRAME_MENU_DELETE ((Attr_attribute) __FRAME_MENU_DELETE)
#define FRAME_MENU_COUNT ((Attr_attribute) __FRAME_MENU_COUNT)
#define FRAME_NEXT_CHILD ((Attr_attribute) __FRAME_NEXT_CHILD)
#define FRAME_PREVIOUS_CHILD ((Attr_attribute) __FRAME_PREVIOUS_CHILD)
#define FRAME_SCALE_STATE ((Attr_attribute) __FRAME_SCALE_STATE)
#define FRAME_SUBWINDOWS_ADJUSTABLE ((Attr_attribute) __FRAME_SUBWINDOWS_ADJUSTABLE)
#define FRAME_COUNT ((Attr_attribute) __FRAME_COUNT)
#define FRAME_FOCUS_UP_IMAGE ((Attr_attribute) __FRAME_FOCUS_UP_IMAGE)
#define FRAME_FOCUS_RIGHT_IMAGE ((Attr_attribute) __FRAME_FOCUS_RIGHT_IMAGE)
#define FRAME_FOCUS_GC ((Attr_attribute) __FRAME_FOCUS_GC)
#define FRAME_ORPHAN_WINDOW ((Attr_attribute) __FRAME_ORPHAN_WINDOW)
#define FRAME_FOOTER_WINDOW ((Attr_attribute) __FRAME_FOOTER_WINDOW)

#ifdef OW_I18N
#define FRAME_IMSTATUS_WINDOW ((Attr_attribute) __FRAME_IMSTATUS_WINDOW)
#endif	

#define FRAME_ACCELERATOR ((Attr_attribute) __FRAME_ACCELERATOR)
#define FRAME_X_ACCELERATOR ((Attr_attribute) __FRAME_X_ACCELERATOR)
#define FRAME_MENU_ACCELERATOR ((Attr_attribute) __FRAME_MENU_ACCELERATOR)
#define FRAME_MENU_REMOVE_ACCELERATOR ((Attr_attribute) __FRAME_MENU_REMOVE_ACCELERATOR)
#define FRAME_MENU_X_ACCELERATOR ((Attr_attribute) __FRAME_MENU_X_ACCELERATOR)

#ifdef OW_I18N
#define FRAME_MENU_ACCELERATOR_WCS ((Attr_attribute) __FRAME_MENU_ACCELERATOR_WCS)
#define FRAME_MENU_REMOVE_ACCELERATOR_WCS ((Attr_attribute) __FRAME_MENU_REMOVE_ACCELERATOR_WCS)
#endif	

#ifdef OW_I18N
#define FRAME_LEFT_IMSTATUS_WCS ((Attr_attribute) __FRAME_LEFT_IMSTATUS_WCS)
#define FRAME_LEFT_IMSTATUS ((Attr_attribute) __FRAME_LEFT_IMSTATUS)
#define FRAME_RIGHT_IMSTATUS_WCS ((Attr_attribute) __FRAME_RIGHT_IMSTATUS_WCS)
#define FRAME_RIGHT_IMSTATUS ((Attr_attribute) __FRAME_RIGHT_IMSTATUS)
#define FRAME_SHOW_IMSTATUS ((Attr_attribute) __FRAME_SHOW_IMSTATUS)
#define FRAME_INACTIVE_IMSTATUS ((Attr_attribute) __FRAME_INACTIVE_IMSTATUS)
#define FRAME_CMD_POINTER_WARP ((Attr_attribute) __FRAME_CMD_POINTER_WARP)
#ifdef FULL_R5
#define FRAME_IMSTATUS_RECT ((Attr_attribute) __FRAME_IMSTATUS_RECT)
#endif /* FULL_R5 */	
#endif

#define FRAME_COMPOSE_STATE ((Attr_attribute) __FRAME_COMPOSE_STATE)


#define	FRAME_PROPS_PUSHPIN_IN	FRAME_CMD_PUSHPIN_IN
#define	FRAME_PROPS_PANEL	FRAME_CMD_PANEL

/*
 *  values for scale state
 */
#define Frame_rescale_state	Window_rescale_state
#define FRAME_SCALE_SMALL	WIN_SCALE_SMALL
#define FRAME_SCALE_MEDIUM	WIN_SCALE_MEDIUM
#define FRAME_SCALE_LARGE	WIN_SCALE_LARGE
#define FRAME_SCALE_EXTRALARGE	WIN_SCALE_EXTRALARGE

/*
 ***********************************************************************
 *				Globals
 ***********************************************************************
 */

extern Xv_pkg	xv_frame_class_pkg;
extern Xv_pkg	xv_frame_base_pkg;
extern Xv_pkg	xv_frame_cmd_pkg;
extern Xv_pkg	xv_frame_props_pkg;
extern Xv_pkg	xv_frame_help_pkg;

/*
 * XView public functions
 * xv_window_loop/xv_window_return are defined in libxview/misc/xv_win_lp.c
 * The are declared here (and not pkg_public.h) as they require frame.h
 * to be included.
 */
EXTERN_FUNCTION (Xv_opaque xv_window_loop, (Frame frame));
EXTERN_FUNCTION (void xv_window_return, (Xv_opaque ret));
EXTERN_FUNCTION (void frame_get_rect, (Frame frame, Rect *rect));
EXTERN_FUNCTION (void frame_set_rect, (Frame frame, Rect *rect));

/*
 * XView Private functions
 */
EXTERN_FUNCTION (void frame_cmdline_help, (char *name));
EXTERN_FUNCTION (void frame_grant_extend_to_edge, (Frame frame, int to_right));
EXTERN_FUNCTION (void frame_kbd_use, (Frame frame, Xv_Window sw, Xv_Window pw));
EXTERN_FUNCTION (void frame_kbd_done, (Frame frame, Xv_Window sw));

#endif /* xview_frame_DEFINED */

