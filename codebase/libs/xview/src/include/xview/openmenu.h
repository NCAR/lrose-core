/*	@(#)openmenu.h 20.61 93/06/28		*/
/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE 
 *	file for terms of the license.
 */

#ifndef xview_walkmenu_DEFINED
#define xview_walkmenu_DEFINED

/*
 ***********************************************************************
 *			Include Files
 ***********************************************************************
 */

#include <xview/generic.h>
#include <xview/pkg.h>
#include <xview/attrol.h>
#include <xview/window.h>


/*
 ***********************************************************************
 *			Definitions and Macros
 ***********************************************************************
 */

/*
 * PUBLIC #defines 
 */

#define	MENU				&xv_command_menu_pkg
#define MENU_COMMAND_MENU		&xv_command_menu_pkg
#define MENU_CHOICE_MENU		&xv_choice_menu_pkg
#define MENU_TOGGLE_MENU		&xv_toggle_menu_pkg
#define MENUITEM 			&xv_menu_item_pkg

#define MENUITEM_SPACE			MENUITEM, \
					MENU_STRING, "", \
					MENU_FEEDBACK, FALSE

#define MENU_BUT  			ACTION_MENU
#define	MENU_NULL			((Menu)0)
#define	MENU_ITEM_NULL			((Menu_item)0)
#define MENU_NO_ITEM			MENU_ITEM_NULL
#define MENU_NO_VALUE			0

#define MENU_DEFAULT_NOTIFY_PROC 	menu_return_value

/*
 * PRIVATE #defines 
 */

#define	MENU_ATTR(type, ordinal)	ATTR(ATTR_PKG_MENU, type, ordinal)
#define MENU_ATTR_LIST(ltype, type, ordinal) \
	MENU_ATTR(ATTR_LIST_INLINE((ltype), (type)), (ordinal))

/*
 * Fake types -- This should be resolved someday 
 */
#define ATTR_MENU			ATTR_OPAQUE
#define ATTR_IMAGE			ATTR_OPAQUE
#define ATTR_MENU_ITEM			ATTR_OPAQUE
#define ATTR_MENU_ITEM_PAIR		ATTR_INT_PAIR
/* Alpha compatibility, mbuck@debian.org */
#if defined(__alpha)
#define ATTR_STRING_VALUE_PAIR		ATTR_OPAQUE_PAIR
#define ATTR_IMAGE_VALUE_PAIR		ATTR_OPAQUE_PAIR
#define ATTR_STRING_MENU_PAIR		ATTR_OPAQUE_PAIR
#define ATTR_IMAGE_MENU_PAIR		ATTR_OPAQUE_PAIR
#define ATTR_STRING_FUNCTION_PAIR	ATTR_OPAQUE_PAIR
#define ATTR_IMAGE_FUNCTION_PAIR	ATTR_OPAQUE_PAIR
#define ATTR_INT_MENU_ITEM_PAIR		ATTR_OPAQUE_PAIR
#else
#define ATTR_STRING_VALUE_PAIR		ATTR_INT_PAIR
#define ATTR_IMAGE_VALUE_PAIR		ATTR_INT_PAIR
#define ATTR_STRING_MENU_PAIR		ATTR_INT_PAIR
#define ATTR_IMAGE_MENU_PAIR		ATTR_INT_PAIR
#define ATTR_STRING_FUNCTION_PAIR	ATTR_INT_PAIR
#define ATTR_IMAGE_FUNCTION_PAIR	ATTR_INT_PAIR
#define ATTR_INT_MENU_ITEM_PAIR		ATTR_INT_PAIR
#endif

/* Reserved for future use */
#define	MENU_ATTR_UNUSED_FIRST		 0
#define	MENU_ATTR_UNUSED_LAST		31

#define MENU_BORDER  			2
#define MENU_PKG 			menu_pkg

/*
 * PUBLIC #defines
 * For SunView 1 compatibility. 
 */
#define menu_destroy(menu_public)	xv_destroy(menu_public)

/*
 ***********************************************************************
 *		Typedefs, Enumerations, and Structures
 ***********************************************************************
 */

typedef	Xv_opaque 		Menu;
typedef	Xv_opaque 		Menu_item;

typedef enum {
  __MENU_ACTION_IMAGE	= MENU_ATTR(ATTR_IMAGE_FUNCTION_PAIR,	  1),
  __MENU_ACTION_ITEM	= MENU_ATTR(ATTR_STRING_FUNCTION_PAIR,	  3),
  __MENU_APPEND_ITEM	= MENU_ATTR(ATTR_MENU_ITEM,		  9),
  __MENU_CLASS   	= MENU_ATTR(ATTR_ENUM, 			 12),
  __MENU_COLOR		= MENU_ATTR(ATTR_INT,			 15),
  __MENU_COL_MAJOR	= MENU_ATTR(ATTR_BOOLEAN, 		 18),
  __MENU_DEFAULT	= MENU_ATTR(ATTR_INT,			 21),
  __MENU_DEFAULT_ITEM	= MENU_ATTR(ATTR_MENU_ITEM,		 24),
  __MENU_DESCEND_FIRST	= MENU_ATTR(ATTR_BOOLEAN, 		 27), 
  __MENU_DISABLE_ITEM	= MENU_ATTR(ATTR_ENUM,		 30),
  __MENU_DONE_PROC		= MENU_ATTR(ATTR_FUNCTION_PTR,	 31),
  __MENU_FEEDBACK		= MENU_ATTR(ATTR_BOOLEAN,	 33), 
  __MENU_FIRST_EVENT	= MENU_ATTR(ATTR_NO_VALUE,	 36), 
/* Alpha compatibility, mbuck@debian.org */
#if defined(__alpha)
  __MENU_GEN_PIN_WINDOW	= MENU_ATTR(ATTR_OPAQUE_PAIR, 		 39),
#else
  __MENU_GEN_PIN_WINDOW	= MENU_ATTR(ATTR_INT_PAIR, 		 39),
#endif
  __MENU_GEN_PROC	= MENU_ATTR(ATTR_FUNCTION_PTR,		 42), 
  __MENU_GEN_PROC_IMAGE	= MENU_ATTR(ATTR_IMAGE_FUNCTION_PAIR,	 45), 
  __MENU_GEN_PROC_ITEM	= MENU_ATTR(ATTR_STRING_FUNCTION_PAIR,	 48), 
  __MENU_GEN_PULLRIGHT	= MENU_ATTR(ATTR_FUNCTION_PTR,		 51), 
  __MENU_GEN_PULLRIGHT_IMAGE= MENU_ATTR(ATTR_IMAGE_FUNCTION_PAIR,	 54), 
  __MENU_GEN_PULLRIGHT_ITEM = MENU_ATTR(ATTR_STRING_FUNCTION_PAIR,	 57), 
  __MENU_IMAGE		= MENU_ATTR(ATTR_IMAGE,			 60),
  __MENU_IMAGES		= MENU_ATTR_LIST(ATTR_NULL, ATTR_IMAGE,	 63),
  __MENU_IMAGE_ITEM	= MENU_ATTR(ATTR_IMAGE_VALUE_PAIR,	 66),
  __MENU_INACTIVE	= MENU_ATTR(ATTR_BOOLEAN,		 69), 
  __MENU_INSERT		= MENU_ATTR(ATTR_INT_MENU_ITEM_PAIR,	 81),
  __MENU_INSERT_ITEM	= MENU_ATTR(ATTR_MENU_ITEM_PAIR,	 84),
  __MENU_INVERT		= MENU_ATTR(ATTR_BOOLEAN,		 87), 
  __MENU_ITEM		= MENU_ATTR_LIST(ATTR_RECURSIVE, ATTR_AV, 90),
  __MENU_LAST_EVENT = MENU_ATTR(ATTR_NO_VALUE,		 99), 
	/*
	 * MENU_MENU is a fake attribute returned by MENU_TYPE 
	 */
  __MENU_MENU		= MENU_ATTR(ATTR_NO_VALUE,		102), 
  __MENU_NCOLS		= MENU_ATTR(ATTR_INT,			105),
  __MENU_NITEMS	= MENU_ATTR(ATTR_NO_VALUE,		108), 
  __MENU_NOTIFY_PROC	= MENU_ATTR(ATTR_FUNCTION_PTR,		  6),
  __MENU_NOTIFY_STATUS	= MENU_ATTR(ATTR_INT,		111),
	/*
	 * The origin for MENU_NTH_ITEM is 1 
	 */
  __MENU_NTH_ITEM	= MENU_ATTR(ATTR_INT,			114), 
  __MENU_NROWS		= MENU_ATTR(ATTR_INT,			117),
  __MENU_PARENT		= MENU_ATTR(ATTR_OPAQUE,		119),
  __MENU_PIN		= MENU_ATTR(ATTR_BOOLEAN, 		120),
  __MENU_PIN_PROC	= MENU_ATTR(ATTR_FUNCTION_PTR, 		123),
  __MENU_PIN_WINDOW	= MENU_ATTR(ATTR_OPAQUE, 		126),
  __MENU_PULLRIGHT	= MENU_ATTR(ATTR_MENU,			129),
  __MENU_PULLRIGHT_IMAGE= MENU_ATTR(ATTR_IMAGE_MENU_PAIR,	135),
  __MENU_PULLRIGHT_ITEM	= MENU_ATTR(ATTR_STRING_MENU_PAIR,	138),
  __MENU_RELEASE	= MENU_ATTR(ATTR_NO_VALUE,		141), 
  __MENU_RELEASE_IMAGE	= MENU_ATTR(ATTR_NO_VALUE,		144), 
  __MENU_REMOVE		= MENU_ATTR(ATTR_INT,			147),
  __MENU_REMOVE_ITEM	= MENU_ATTR(ATTR_MENU_ITEM,		150),
  __MENU_REPLACE	= MENU_ATTR(ATTR_INT_MENU_ITEM_PAIR,	153),
  __MENU_REPLACE_ITEM	= MENU_ATTR(ATTR_MENU_ITEM_PAIR,	156),
  __MENU_SELECTED	= MENU_ATTR(ATTR_INT,			159),
  __MENU_SELECTED_ITEM	= MENU_ATTR(ATTR_MENU_ITEM,		162),
  __MENU_STRING		= MENU_ATTR(ATTR_STRING,		165),
  __MENU_STRINGS	= MENU_ATTR_LIST(ATTR_NULL, ATTR_STRING,168),
  __MENU_STRING_ITEM	= MENU_ATTR(ATTR_STRING_VALUE_PAIR,	171),
  __MENU_TITLE		= MENU_ATTR(ATTR_NO_VALUE, 		174),
  __MENU_TITLE_IMAGE	= MENU_ATTR(ATTR_IMAGE,			177),
  __MENU_TITLE_ITEM	= MENU_ATTR(ATTR_STRING,		180),
  __MENU_TYPE	= MENU_ATTR(ATTR_NO_VALUE,		183), 
  __MENU_VALID_RESULT	= MENU_ATTR(ATTR_INT,			186),
  __MENU_VALUE		= MENU_ATTR(ATTR_OPAQUE,		189), 
        /* ACC_XVIEW */
  __MENU_ACTION_ACCELERATOR = MENU_ATTR(ATTR_OPAQUE_TRIPLE,	240),
  __MENU_STRING_AND_ACCELERATOR = MENU_ATTR(ATTR_OPAQUE_PAIR,	242),
  __MENU_ACCELERATOR 	= MENU_ATTR(ATTR_STRING,                244),
  __MENU_STRINGS_AND_ACCELERATORS = 
			MENU_ATTR_LIST(ATTR_NULL, ATTR_OPAQUE_PAIR,	246),
        /* ACC_XVIEW */

	/*
	 * Private Attributes 
	 */
  __MENU_BUSY_PROC	= MENU_ATTR(ATTR_FUNCTION_PTR,		195),
  __MENU_BUTTON		= MENU_ATTR(ATTR_INT, 			198), 
  __MENU_CURSOR		= MENU_ATTR(ATTR_OPAQUE,		201),
  __MENU_ENABLE_RECT	= MENU_ATTR(ATTR_INT, 			207),
  __MENU_FD		= MENU_ATTR(ATTR_INT, 			210), 
        /* ACC_XVIEW */
  __MENU_FRAME_ADD	= MENU_ATTR(ATTR_OPAQUE,		248), 
  __MENU_FRAME_DELETE	= MENU_ATTR(ATTR_OPAQUE,		249), 
  __MENU_ACC_KEY	= MENU_ATTR(ATTR_STRING,		250),
  __MENU_ACC_QUAL	= MENU_ATTR(ATTR_STRING,		251),
        /* ACC_XVIEW */
  __MENU_HORIZONTAL_LINE= MENU_ATTR(ATTR_INT,			213),
  __MENU_IE		= MENU_ATTR(ATTR_INT, 			216), 
  __MENU_LINE_AFTER_ITEM= MENU_ATTR(ATTR_INT,			219),
  __MENU_POS		= MENU_ATTR(ATTR_INT_PAIR, 		222), 
  __MENU_POSITION_RECT	= MENU_ATTR(ATTR_INT,			225),
  __MENU_PULLDOWN	= MENU_ATTR(ATTR_INT, 			228),
  __MENU_SHADOW_MENU	= MENU_ATTR(ATTR_OPAQUE,		230),
  __MENU_SHADOW_GC	= MENU_ATTR(ATTR_OPAQUE,		231),
  __MENU_WINDOW_MENU	= MENU_ATTR(ATTR_OPAQUE,		233),
  __MENU_VERTICAL_LINE	= MENU_ATTR(ATTR_INT,			234),
	/*
	 * For SunView 1 Compatibility 
	 */
  __MENU_ACTION		= __MENU_NOTIFY_PROC,
  __MENU_ACTION_PROC	= __MENU_NOTIFY_PROC,
  __MENU_CLIENT_DATA    = MENU_ATTR(ATTR_OPAQUE,         	15),
  __MENU_PULLRIGHT_DELTA= MENU_ATTR(ATTR_INT,           	132)

	/*********************************
	 * Internationalization Attributes
	 *********************************/
#ifdef OW_I18N
                                                                            ,
  __MENU_ACTION_ITEM_WCS    = MENU_ATTR(ATTR_STRING_FUNCTION_PAIR,    4),
  __MENU_GEN_PIN_WINDOW_WCS = MENU_ATTR(ATTR_INT_PAIR,               40),
  __MENU_GEN_PROC_ITEM_WCS  = MENU_ATTR(ATTR_STRING_FUNCTION_PAIR,   49),
  __MENU_GEN_PULLRIGHT_ITEM_WCS = MENU_ATTR(ATTR_STRING_FUNCTION_PAIR,	58),
  __MENU_PULLRIGHT_ITEM_WCS = MENU_ATTR(ATTR_STRING_MENU_PAIR,      139),
  __MENU_STRING_WCS         = MENU_ATTR(ATTR_WSTRING,                166),
  __MENU_STRINGS_WCS        = MENU_ATTR_LIST(ATTR_NULL, ATTR_WSTRING,169),
  __MENU_STRING_ITEM_WCS    = MENU_ATTR(ATTR_STRING_VALUE_PAIR,     172),
  __MENU_TITLE_ITEM_WCS     = MENU_ATTR(ATTR_WSTRING,                181),
        /* ACC_XVIEW */
  __MENU_ACTION_ACCELERATOR_WCS = MENU_ATTR(ATTR_OPAQUE_TRIPLE,	247),
  __MENU_STRING_AND_ACCELERATOR_WCS = MENU_ATTR(ATTR_OPAQUE_PAIR,	243),
  __MENU_ACCELERATOR_WCS 	= MENU_ATTR(ATTR_STRING,                245),
  __MENU_STRINGS_AND_ACCELERATORS_WCS = 
			MENU_ATTR_LIST(ATTR_NULL, ATTR_OPAQUE_PAIR,	241)
        /* ACC_XVIEW */
#endif /* OW_I18N */

} Menu_attribute;

#define MENU_ACTION_IMAGE ((Attr_attribute) __MENU_ACTION_IMAGE)
#define MENU_ACTION_ITEM ((Attr_attribute) __MENU_ACTION_ITEM)
#define MENU_APPEND_ITEM ((Attr_attribute) __MENU_APPEND_ITEM)
#define MENU_CLASS ((Attr_attribute) __MENU_CLASS)
#define MENU_COLOR ((Attr_attribute) __MENU_COLOR)
#define MENU_COL_MAJOR ((Attr_attribute) __MENU_COL_MAJOR)
#define MENU_DEFAULT ((Attr_attribute) __MENU_DEFAULT)
#define MENU_DEFAULT_ITEM ((Attr_attribute) __MENU_DEFAULT_ITEM)
#define MENU_DESCEND_FIRST ((Attr_attribute) __MENU_DESCEND_FIRST)
#define MENU_DISABLE_ITEM ((Attr_attribute) __MENU_DISABLE_ITEM)
#define MENU_DONE_PROC ((Attr_attribute) __MENU_DONE_PROC)
#define MENU_FEEDBACK ((Attr_attribute) __MENU_FEEDBACK)
#define MENU_FIRST_EVENT ((Attr_attribute) __MENU_FIRST_EVENT)

#if defined(__alpha)
#define MENU_GEN_PIN_WINDOW ((Attr_attribute) __MENU_GEN_PIN_WINDOW)
#else
#define MENU_GEN_PIN_WINDOW ((Attr_attribute) __MENU_GEN_PIN_WINDOW)
#endif

#define MENU_GEN_PROC ((Attr_attribute) __MENU_GEN_PROC)
#define MENU_GEN_PROC_IMAGE ((Attr_attribute) __MENU_GEN_PROC_IMAGE)
#define MENU_GEN_PROC_ITEM ((Attr_attribute) __MENU_GEN_PROC_ITEM)
#define MENU_GEN_PULLRIGHT ((Attr_attribute) __MENU_GEN_PULLRIGHT)
#define MENU_GEN_PULLRIGHT_IMAGE ((Attr_attribute) __MENU_GEN_PULLRIGHT_IMAGE)
#define MENU_GEN_PULLRIGHT_ITEM ((Attr_attribute) __MENU_GEN_PULLRIGHT_ITEM)
#define MENU_IMAGE ((Attr_attribute) __MENU_IMAGE)
#define MENU_IMAGES ((Attr_attribute) __MENU_IMAGES)
#define MENU_IMAGE_ITEM ((Attr_attribute) __MENU_IMAGE_ITEM)
#define MENU_INACTIVE ((Attr_attribute) __MENU_INACTIVE)
#define MENU_INSERT ((Attr_attribute) __MENU_INSERT)
#define MENU_INSERT_ITEM ((Attr_attribute) __MENU_INSERT_ITEM)
#define MENU_INVERT ((Attr_attribute) __MENU_INVERT)
#define MENU_ITEM ((Attr_attribute) __MENU_ITEM)
#define MENU_LAST_EVENT ((Attr_attribute) __MENU_LAST_EVENT)
#define MENU_MENU ((Attr_attribute) __MENU_MENU)
#define MENU_NCOLS ((Attr_attribute) __MENU_NCOLS)
#define MENU_NITEMS ((Attr_attribute) __MENU_NITEMS)
#define MENU_NOTIFY_PROC ((Attr_attribute) __MENU_NOTIFY_PROC)
#define MENU_NOTIFY_STATUS ((Attr_attribute) __MENU_NOTIFY_STATUS)
#define MENU_NTH_ITEM ((Attr_attribute) __MENU_NTH_ITEM)
#define MENU_NROWS ((Attr_attribute) __MENU_NROWS)
#define MENU_PARENT ((Attr_attribute) __MENU_PARENT)
#define MENU_PIN ((Attr_attribute) __MENU_PIN)
#define MENU_PIN_PROC ((Attr_attribute) __MENU_PIN_PROC)
#define MENU_PIN_WINDOW ((Attr_attribute) __MENU_PIN_WINDOW)
#define MENU_PULLRIGHT ((Attr_attribute) __MENU_PULLRIGHT)
#define MENU_PULLRIGHT_IMAGE ((Attr_attribute) __MENU_PULLRIGHT_IMAGE)
#define MENU_PULLRIGHT_ITEM ((Attr_attribute) __MENU_PULLRIGHT_ITEM)
#define MENU_RELEASE ((Attr_attribute) __MENU_RELEASE)
#define MENU_RELEASE_IMAGE ((Attr_attribute) __MENU_RELEASE_IMAGE)
#define MENU_REMOVE ((Attr_attribute) __MENU_REMOVE)
#define MENU_REMOVE_ITEM ((Attr_attribute) __MENU_REMOVE_ITEM)
#define MENU_REPLACE ((Attr_attribute) __MENU_REPLACE)
#define MENU_REPLACE_ITEM ((Attr_attribute) __MENU_REPLACE_ITEM)
#define MENU_SELECTED ((Attr_attribute) __MENU_SELECTED)
#define MENU_SELECTED_ITEM ((Attr_attribute) __MENU_SELECTED_ITEM)
#define MENU_STRING ((Attr_attribute) __MENU_STRING)
#define MENU_STRINGS ((Attr_attribute) __MENU_STRINGS)
#define MENU_STRING_ITEM ((Attr_attribute) __MENU_STRING_ITEM)
#define MENU_TITLE ((Attr_attribute) __MENU_TITLE)
#define MENU_TITLE_IMAGE ((Attr_attribute) __MENU_TITLE_IMAGE)
#define MENU_TITLE_ITEM ((Attr_attribute) __MENU_TITLE_ITEM)
#define MENU_TYPE ((Attr_attribute) __MENU_TYPE)
#define MENU_VALID_RESULT ((Attr_attribute) __MENU_VALID_RESULT)
#define MENU_VALUE ((Attr_attribute) __MENU_VALUE)
#define MENU_ACTION_ACCELERATOR ((Attr_attribute) __MENU_ACTION_ACCELERATOR)
#define MENU_STRING_AND_ACCELERATOR ((Attr_attribute) __MENU_STRING_AND_ACCELERATOR)
#define MENU_ACCELERATOR ((Attr_attribute) __MENU_ACCELERATOR)
#define MENU_STRINGS_AND_ACCELERATORS ((Attr_attribute) __MENU_STRINGS_AND_ACCELERATORS)
#define MENU_BUSY_PROC ((Attr_attribute) __MENU_BUSY_PROC)
#define MENU_BUTTON ((Attr_attribute) __MENU_BUTTON)
#define MENU_CURSOR ((Attr_attribute) __MENU_CURSOR)
#define MENU_ENABLE_RECT ((Attr_attribute) __MENU_ENABLE_RECT)
#define MENU_FD ((Attr_attribute) __MENU_FD)
#define MENU_FRAME_ADD ((Attr_attribute) __MENU_FRAME_ADD)
#define MENU_FRAME_DELETE ((Attr_attribute) __MENU_FRAME_DELETE)
#define MENU_ACC_KEY ((Attr_attribute) __MENU_ACC_KEY)
#define MENU_ACC_QUAL ((Attr_attribute) __MENU_ACC_QUAL)
#define MENU_HORIZONTAL_LINE ((Attr_attribute) __MENU_HORIZONTAL_LINE)
#define MENU_IE ((Attr_attribute) __MENU_IE)
#define MENU_LINE_AFTER_ITEM ((Attr_attribute) __MENU_LINE_AFTER_ITEM)
#define MENU_POS ((Attr_attribute) __MENU_POS)
#define MENU_POSITION_RECT ((Attr_attribute) __MENU_POSITION_RECT)
#define MENU_PULLDOWN ((Attr_attribute) __MENU_PULLDOWN)
#define MENU_SHADOW_MENU ((Attr_attribute) __MENU_SHADOW_MENU)
#define MENU_SHADOW_GC ((Attr_attribute) __MENU_SHADOW_GC)
#define MENU_WINDOW_MENU ((Attr_attribute) __MENU_WINDOW_MENU)
#define MENU_VERTICAL_LINE ((Attr_attribute) __MENU_VERTICAL_LINE)
#define MENU_ACTION ((Attr_attribute) __MENU_ACTION)
#define MENU_ACTION_PROC ((Attr_attribute) __MENU_ACTION_PROC)
#define MENU_CLIENT_DATA ((Attr_attribute) __MENU_CLIENT_DATA)
#define MENU_PULLRIGHT_DELTA ((Attr_attribute) __MENU_PULLRIGHT_DELTA)

#ifdef OW_I18N
                                                                            ,
#define MENU_ACTION_ITEM_WCS ((Attr_attribute) __MENU_ACTION_ITEM_WCS)
#define MENU_GEN_PIN_WINDOW_WCS ((Attr_attribute) __MENU_GEN_PIN_WINDOW_WCS)
#define MENU_GEN_PROC_ITEM_WCS ((Attr_attribute) __MENU_GEN_PROC_ITEM_WCS)
#define MENU_GEN_PULLRIGHT_ITEM_WCS ((Attr_attribute) __MENU_GEN_PULLRIGHT_ITEM_WCS)
#define MENU_PULLRIGHT_ITEM_WCS ((Attr_attribute) __MENU_PULLRIGHT_ITEM_WCS)
#define MENU_STRING_WCS ((Attr_attribute) __MENU_STRING_WCS)
#define MENU_STRINGS_WCS ((Attr_attribute) __MENU_STRINGS_WCS)
#define MENU_STRING_ITEM_WCS ((Attr_attribute) __MENU_STRING_ITEM_WCS)
#define MENU_TITLE_ITEM_WCS ((Attr_attribute) __MENU_TITLE_ITEM_WCS)
#define MENU_ACTION_ACCELERATOR_WCS ((Attr_attribute) __MENU_ACTION_ACCELERATOR_WCS)
#define MENU_STRING_AND_ACCELERATOR_WCS ((Attr_attribute) __MENU_STRING_AND_ACCELERATOR_WCS)
#define MENU_ACCELERATOR_WCS ((Attr_attribute) __MENU_ACCELERATOR_WCS)
#define MENU_STRINGS_AND_ACCELERATORS_WCS ((Attr_attribute) __MENU_STRINGS_AND_ACCELERATORS_WCS)

#endif /* OW_I18N */

typedef enum {
	MENU_COMMAND, 
	MENU_CHOICE, 
	MENU_TOGGLE
} Menu_class;

typedef enum {
	MENU_PROVIDE_FEEDBACK, 			/* not default, selected     */
	MENU_REMOVE_FEEDBACK,			/* not default, not selected */
	MENU_SELECTED_DEFAULT_FEEDBACK,		/* default, selected         */
	MENU_DEFAULT_FEEDBACK,			/* default, not selected     */
	MENU_BUSY_FEEDBACK				/* 50% gray */
} Menu_feedback;

/*
 * New generate names intended to be less confusing 
 */
typedef enum {
	MENU_DISPLAY, 
	MENU_DISPLAY_DONE, 
	MENU_NOTIFY, 
	MENU_NOTIFY_DONE
} Menu_generate;

typedef struct {
	Xv_generic_struct	parent_data;
	Xv_opaque		private_data;
} Xv_menu;

typedef struct {
	Xv_generic_struct	parent_data;
	Xv_opaque		private_data;
} Xv_menu_item;

/*
 ***********************************************************************	
 *				Globals
 ***********************************************************************
 */

/*
 * PUBLIC variables 
 */

extern Xv_pkg		xv_command_menu_pkg;
extern Xv_pkg		xv_choice_menu_pkg;
extern Xv_pkg		xv_toggle_menu_pkg;
extern Xv_pkg		xv_menu_item_pkg;

/*
 * PUBLIC variables 
 * for SunView 1 compatibility
 */

extern struct pixrect 	menu_gray50_pr;

/*
 * PUBLIC functions 
 */
EXTERN_FUNCTION (Xv_opaque menu_return_value, (Menu menu, Menu_item item));
EXTERN_FUNCTION (Xv_opaque menu_return_item, (Menu menu, Menu_item item));
EXTERN_FUNCTION (void menu_show, (Menu menu, Xv_Window win, Event *event, DOTDOTDOT));

/*
 * PRIVATE functions 
 */
EXTERN_FUNCTION (void menu_return_default, (Menu menu, int depth, Event *event));
EXTERN_FUNCTION (void menu_select_default, (Menu menu));

/* 
 * PUBLIC functions 
 * for SunView 1 compatibility 
 */
 
EXTERN_FUNCTION (Menu menu_create, (Attr_attribute attr1, DOTDOTDOT));
EXTERN_FUNCTION (Menu_item menu_create_item, (Attr_attribute attr1, DOTDOTDOT));
EXTERN_FUNCTION (Menu_item 	menu_find, (Menu menu, DOTDOTDOT));
EXTERN_FUNCTION (Xv_opaque 	menu_get, (Menu menu, Xv_opaque attr, Xv_opaque v1));
EXTERN_FUNCTION (Xv_opaque 	menu_set, (Menu menu, DOTDOTDOT));
EXTERN_FUNCTION (void 		menu_destroy_with_proc, (Menu menu, void (*proc)()));

#endif /* xview_walkmenu_DEFINED */
