/*	
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license. 
 */

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#ifndef	xview_openwin_DEFINED
#define	xview_openwin_DEFINED

/*
 * Module:	openwin.h
 * Product:	SunView 2.0
 *
 * Level:	Public
 *
 * Description:
 *
 *	Defines attributes for the window that implements a LAF
 *  subwindow.
 *
 */


/*
 ***********************************************************************
 *			Include Files
 ***********************************************************************
 */
 
#include <xview/window.h>
#include <xview/attrol.h>

/*
 ***********************************************************************
 *			Definitions and Macros
 ***********************************************************************
 */

/*
 * PUBLIC #defines 
 */

#define OPENWIN   			&xv_openwin_pkg

#define OPENWIN_SPLIT_NEWVIEW_IN_PLACE 	-1
#define OPENWIN_CANNOT_EXPAND		-1

#define	OPENWIN_EACH_VIEW(openwin, view)	\
   {int _view_cnt = 0; 				\
   while (((view) = (Xv_Window) xv_get((openwin), OPENWIN_NTH_VIEW, _view_cnt++)) != XV_NULL) { 					\

#define	OPENWIN_END_EACH	}}


/*
 * PRIVATE #defines 
 */

#ifndef ATTR_PKG_OPENWIN
#define ATTR_PKG_OPENWIN 		(ATTR_PKG_UNUSED_FIRST + 8)
#endif
#define	OPENWIN_ATTR(type, ordinal)	ATTR(ATTR_PKG_OPENWIN, type, ordinal)
#define OPENWIN_ATTR_LIST(ltype, type, ordinal) \
    OPENWIN_ATTR(ATTR_LIST_INLINE((ltype), (type)), (ordinal))

#define OPENWIN_SET_GC		0
#define OPENWIN_CLR_GC		1
#define OPENWIN_TEXT_GC		2
#define OPENWIN_BOLD_GC		3
#define OPENWIN_GLYPH_GC	4
#define OPENWIN_INACTIVE_GC	5
#define OPENWIN_DIM_GC		6
#define OPENWIN_INVERT_GC	7
#define OPENWIN_NONSTD_GC	8	/* Color or non-standard font */
#define OPENWIN_NBR_GCS		9

/*
 ***********************************************************************
 *		Typedefs, Enumerations, and Structures
 ***********************************************************************
 */
 
typedef Xv_opaque  Openwin;

typedef struct {
	Xv_window_struct	parent_data;
	Xv_opaque		private_data;
}  Xv_openwin;

typedef enum {
	OPENWIN_SPLIT_HORIZONTAL,
	OPENWIN_SPLIT_VERTICAL
} Openwin_split_direction;

typedef enum {
   /*
    * Public Attributes 
    */
   __OPENWIN_ADJUST_FOR_HORIZONTAL_SCROLLBAR
				= OPENWIN_ATTR(ATTR_BOOLEAN, 	 1),
   __OPENWIN_ADJUST_FOR_VERTICAL_SCROLLBAR
				= OPENWIN_ATTR(ATTR_BOOLEAN, 	 5),
   __OPENWIN_AUTO_CLEAR		= OPENWIN_ATTR(ATTR_BOOLEAN, 	10),
   __OPENWIN_HORIZONTAL_SCROLLBAR	= OPENWIN_ATTR(ATTR_OPAQUE, 	15),
   __OPENWIN_NVIEWS		= OPENWIN_ATTR(ATTR_INT, 	20),
   __OPENWIN_NO_MARGIN		= OPENWIN_ATTR(ATTR_BOOLEAN, 	25),
   __OPENWIN_NTH_VIEW     	= OPENWIN_ATTR(ATTR_OPAQUE, 	30),
   __OPENWIN_SELECTED_VIEW	= OPENWIN_ATTR(ATTR_OPAQUE, 	35),
   __OPENWIN_SHOW_BORDERS		= OPENWIN_ATTR(ATTR_BOOLEAN, 	40),
   __OPENWIN_SPLIT		= OPENWIN_ATTR_LIST(ATTR_RECURSIVE, ATTR_AV, 45),
   __OPENWIN_SPLIT_DIRECTION  	= OPENWIN_ATTR(ATTR_ENUM, 	50),
   __OPENWIN_SPLIT_POSITION	= OPENWIN_ATTR(ATTR_X, 		55),
   __OPENWIN_SPLIT_VIEW		= OPENWIN_ATTR(ATTR_OPAQUE, 	60),
   __OPENWIN_SPLIT_VIEW_START	= OPENWIN_ATTR(ATTR_X, 		65),
   __OPENWIN_VERTICAL_SCROLLBAR	= OPENWIN_ATTR(ATTR_OPAQUE, 	70),
   __OPENWIN_VIEW_ATTRS		= OPENWIN_ATTR_LIST(ATTR_RECURSIVE, ATTR_AV, 75),
   __OPENWIN_SPLIT_INIT_PROC	= OPENWIN_ATTR(ATTR_FUNCTION_PTR, 76),
   __OPENWIN_SPLIT_DESTROY_PROC	= OPENWIN_ATTR(ATTR_FUNCTION_PTR, 77),
   /*
    * Private Attributes 
    */
   __OPENWIN_VIEW_CLASS		= OPENWIN_ATTR(ATTR_OPAQUE,  	80)
} Openwin_attribute;

#define OPENWIN_ADJUST_FOR_HORIZONTAL_SCROLLBAR ((Attr_attribute) __OPENWIN_ADJUST_FOR_HORIZONTAL_SCROLLBAR)
#define OPENWIN_ADJUST_FOR_VERTICAL_SCROLLBAR ((Attr_attribute) __OPENWIN_ADJUST_FOR_VERTICAL_SCROLLBAR)
#define OPENWIN_AUTO_CLEAR ((Attr_attribute) __OPENWIN_AUTO_CLEAR)
#define OPENWIN_HORIZONTAL_SCROLLBAR ((Attr_attribute) __OPENWIN_HORIZONTAL_SCROLLBAR)
#define OPENWIN_NVIEWS ((Attr_attribute) __OPENWIN_NVIEWS)
#define OPENWIN_NO_MARGIN ((Attr_attribute) __OPENWIN_NO_MARGIN)
#define OPENWIN_NTH_VIEW ((Attr_attribute) __OPENWIN_NTH_VIEW)
#define OPENWIN_SELECTED_VIEW ((Attr_attribute) __OPENWIN_SELECTED_VIEW)
#define OPENWIN_SHOW_BORDERS ((Attr_attribute) __OPENWIN_SHOW_BORDERS)
#define OPENWIN_SPLIT ((Attr_attribute) __OPENWIN_SPLIT)
#define OPENWIN_SPLIT_DIRECTION ((Attr_attribute) __OPENWIN_SPLIT_DIRECTION)
#define OPENWIN_SPLIT_POSITION ((Attr_attribute) __OPENWIN_SPLIT_POSITION)
#define OPENWIN_SPLIT_VIEW ((Attr_attribute) __OPENWIN_SPLIT_VIEW)
#define OPENWIN_SPLIT_VIEW_START ((Attr_attribute) __OPENWIN_SPLIT_VIEW_START)
#define OPENWIN_VERTICAL_SCROLLBAR ((Attr_attribute) __OPENWIN_VERTICAL_SCROLLBAR)
#define OPENWIN_VIEW_ATTRS ((Attr_attribute) __OPENWIN_VIEW_ATTRS)
#define OPENWIN_SPLIT_INIT_PROC ((Attr_attribute) __OPENWIN_SPLIT_INIT_PROC)
#define OPENWIN_SPLIT_DESTROY_PROC ((Attr_attribute) __OPENWIN_SPLIT_DESTROY_PROC)
#define OPENWIN_VIEW_CLASS ((Attr_attribute) __OPENWIN_VIEW_CLASS)


/*
 ***********************************************************************
 *				Globals
 ***********************************************************************
 */
 
extern Xv_pkg xv_openwin_pkg;

#endif	 /* ~xview_openwin_DEFINED */
