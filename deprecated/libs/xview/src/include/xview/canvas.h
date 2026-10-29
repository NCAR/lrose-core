/*      @(#)canvas.h 20.38 93/06/28 SMI      */

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE 
 *	file for terms of the license.
 */

#ifndef xview_canvas_DEFINED
#define xview_canvas_DEFINED

/*
 ***********************************************************************
 *			Include Files
 ***********************************************************************
 */
#include <xview/openwin.h>
#include <xview/pixwin.h>
#include <xview/win_input.h>

/*
 ***********************************************************************
 *			Definitions and Macros
 ***********************************************************************
 */

/*
 * PUBLIC #defines 
 */

#define CANVAS			&xv_canvas_pkg
#define CANVAS_VIEW		&xv_canvas_view_pkg
#define CANVAS_PAINT_WINDOW	&xv_canvas_pw_pkg

#define	CANVAS_PIXWIN		CANVAS_NTH_PAINT_WINDOW, 0
#define	CANVAS_AUTO_CLEAR	OPENWIN_AUTO_CLEAR

/*
 * Some useful macros
 */
#define	canvas_pixwin(canvas)	\
	((Pixwin *)xv_get(canvas, CANVAS_NTH_PAINT_WINDOW, NULL))
#define	canvas_paint_window(canvas) \
	((Xv_Window)xv_get(canvas, CANVAS_NTH_PAINT_WINDOW, NULL))

#define	CANVAS_EACH_PAINT_WINDOW(canvas, pw)	\
   {int _pw_cnt = 0; \
   while (((pw) = (Xv_Window) xv_get((canvas), CANVAS_NTH_PAINT_WINDOW, _pw_cnt++)) != XV_NULL) { \

#define	CANVAS_END_EACH	}}

/*
 * PRIVATE #defines 
 */

#define	CANVAS_ATTR(type, ordinal)	ATTR(ATTR_PKG_CANVAS, type, ordinal)
#define	CANVAS_VIEW_ATTR(type, ordinal)	ATTR(ATTR_PKG_CANVAS_VIEW,type, ordinal)
#define	CANVAS_PAINT_ATTR(type, ordinal) ATTR(ATTR_PKG_CANVAS_PAINT_WINDOW,type, ordinal)
#define CANVAS_ATTR_LIST(ltype, type, ordinal) \
    CANVAS_ATTR(ATTR_LIST_INLINE((ltype), (type)), (ordinal))


/*
 * For SunView 1 compatibility 
 */
#define	CANVAS_TYPE	ATTR_PKG_CANVAS
#define CANVAS_MARGIN	CANVAS_VIEW_MARGIN

/*
 ***********************************************************************
 *		Typedefs, Enumerations, and Structs
 ***********************************************************************
 */

typedef Xv_opaque	Canvas;
typedef Xv_opaque	Canvas_view;
typedef Xv_opaque	Canvas_paint_window;

/*
 * Enumerations 
 */

typedef enum {
	/*
 	 * Public attributes 
	 */
  __CANVAS_AUTO_EXPAND		= CANVAS_ATTR(ATTR_BOOLEAN, 	  1),
  __CANVAS_AUTO_SHRINK		= CANVAS_ATTR(ATTR_BOOLEAN, 	  5),
  __CANVAS_FIXED_IMAGE		= CANVAS_ATTR(ATTR_BOOLEAN, 	 10),
  __CANVAS_HEIGHT		= CANVAS_ATTR(ATTR_Y, 		 15),
  __CANVAS_MIN_PAINT_HEIGHT 	= CANVAS_ATTR(ATTR_Y, 		 20),
  __CANVAS_MIN_PAINT_WIDTH  	= CANVAS_ATTR(ATTR_X, 		 25),
  __CANVAS_NTH_PAINT_WINDOW	= CANVAS_ATTR(ATTR_OPAQUE,  	 30),
  __CANVAS_REPAINT_PROC		= CANVAS_ATTR(ATTR_FUNCTION_PTR, 35),
  __CANVAS_RESIZE_PROC		= CANVAS_ATTR(ATTR_FUNCTION_PTR, 40),
  __CANVAS_RETAINED		= CANVAS_ATTR(ATTR_BOOLEAN, 	 45),
  __CANVAS_VIEW_MARGIN		= CANVAS_ATTR(ATTR_INT, 	 50),
  __CANVAS_VIEWABLE_RECT 	= CANVAS_ATTR(ATTR_RECT_PTR, 	 55),
  __CANVAS_WIDTH		= CANVAS_ATTR(ATTR_X, 		 60),
  __CANVAS_X_PAINT_WINDOW	= CANVAS_ATTR(ATTR_BOOLEAN,      65),
  __CANVAS_PAINTWINDOW_ATTRS	= CANVAS_ATTR_LIST(ATTR_RECURSIVE, ATTR_AV, 70),
  __CANVAS_NO_CLIPPING		= CANVAS_ATTR(ATTR_BOOLEAN,      75),
  __CANVAS_CMS_REPAINT          = CANVAS_ATTR(ATTR_BOOLEAN,      80),
#ifdef OW_I18N
  __CANVAS_IM_PREEDIT_FRAME     = CANVAS_ATTR(ATTR_OPAQUE,       85)
#endif
} Canvas_attribute;

#define XV_KEY_DATA ((Attr_attribute) __XV_KEY_DATA)

#define CANVAS_AUTO_EXPAND ((Attr_attribute) __CANVAS_AUTO_EXPAND)
#define CANVAS_AUTO_SHRINK ((Attr_attribute) __CANVAS_AUTO_SHRINK)
#define CANVAS_FIXED_IMAGE ((Attr_attribute) __CANVAS_FIXED_IMAGE)
#define CANVAS_HEIGHT ((Attr_attribute) __CANVAS_HEIGHT)
#define CANVAS_MIN_PAINT_HEIGHT ((Attr_attribute) __CANVAS_MIN_PAINT_HEIGHT)
#define CANVAS_MIN_PAINT_WIDTH ((Attr_attribute) __CANVAS_MIN_PAINT_WIDTH)
#define CANVAS_NTH_PAINT_WINDOW ((Attr_attribute) __CANVAS_NTH_PAINT_WINDOW)
#define CANVAS_REPAINT_PROC ((Attr_attribute) __CANVAS_REPAINT_PROC)
#define CANVAS_RESIZE_PROC ((Attr_attribute) __CANVAS_RESIZE_PROC)
#define CANVAS_RETAINED ((Attr_attribute) __CANVAS_RETAINED)
#define CANVAS_VIEW_MARGIN ((Attr_attribute) __CANVAS_VIEW_MARGIN)
#define CANVAS_VIEWABLE_RECT ((Attr_attribute) __CANVAS_VIEWABLE_RECT)
#define CANVAS_WIDTH ((Attr_attribute) __CANVAS_WIDTH)
#define CANVAS_X_PAINT_WINDOW ((Attr_attribute) __CANVAS_X_PAINT_WINDOW)
#define CANVAS_PAINTWINDOW_ATTRS ((Attr_attribute) __CANVAS_PAINTWINDOW_ATTRS)
#define CANVAS_NO_CLIPPING ((Attr_attribute) __CANVAS_NO_CLIPPING)
#define CANVAS_CMS_REPAINT ((Attr_attribute) __CANVAS_CMS_REPAINT)
#ifdef OW_I18N
#define CANVAS_IM_PREEDIT_FRAME ((Attr_attribute) __CANVAS_IM_PREEDIT_FRAME)
#endif

typedef enum {
  __CANVAS_VIEW_PAINT_WINDOW = CANVAS_VIEW_ATTR(ATTR_OPAQUE, 1),
  __CANVAS_VIEW_CANVAS_WINDOW = CANVAS_VIEW_ATTR(ATTR_OPAQUE, 2)
} Canvas_view_attribute;

#define CANVAS_VIEW_PAINT_WINDOW ((Attr_attribute) __CANVAS_VIEW_PAINT_WINDOW)
#define CANVAS_VIEW_CANVAS_WINDOW ((Attr_attribute) __CANVAS_VIEW_CANVAS_WINDOW)

typedef enum {
  __CANVAS_PAINT_CANVAS_WINDOW = CANVAS_PAINT_ATTR(ATTR_OPAQUE, 1),
  __CANVAS_PAINT_VIEW_WINDOW   = CANVAS_PAINT_ATTR(ATTR_OPAQUE, 2)
} Canvas_paint_attribute;

#define CANVAS_PAINT_CANVAS_WINDOW ((Attr_attribute) __CANVAS_PAINT_CANVAS_WINDOW)
#define CANVAS_PAINT_VIEW_WINDOW ((Attr_attribute) __CANVAS_PAINT_VIEW_WINDOW)

/*
 * Structures 
 */

typedef struct {
    Xv_openwin	parent_data;
    Xv_opaque	private_data;
} Xv_canvas;

typedef struct {
    Xv_window_struct	parent_data;
    Xv_opaque		private_data;
} Xv_canvas_view;

typedef struct {
    Xv_window_struct	parent_data;
    Xv_opaque		private_data;
} Xv_canvas_pw;


/*
 ***********************************************************************
 *			Globals
 ***********************************************************************
 */

/*
 * Variables 
 */
extern Xv_pkg	xv_canvas_pkg;
extern Xv_pkg	xv_canvas_view_pkg;
extern Xv_pkg	xv_canvas_pw_pkg;

/*
 * Functions
 */
EXTERN_FUNCTION (Event * canvas_event, (Canvas canvas_obj, Event *event));
EXTERN_FUNCTION (Event * canvas_window_event, (Canvas canvas_obj, Event *event));

#endif /* ~xview_canvas_DEFINED */
