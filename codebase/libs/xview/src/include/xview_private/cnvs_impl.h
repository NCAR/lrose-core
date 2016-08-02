/*      @(#)cnvs_impl.h 20.43 93/06/28 SMI      */
/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE 
 *	file for terms of the license.
 */

#include <xview/pkg.h>
#include <xview/attrol.h>
#include <xview/canvas.h>
#include <xview_private/portable.h>

#ifdef OW_I18N
#include <xview_private/i18n_impl.h>
#include <xview/frame.h>
#include <xview/panel.h>
#endif /*OW_I18N*/

#define	CANVAS_PRIVATE(c)	XV_PRIVATE(Canvas_info, Xv_canvas, c)
#define	CANVAS_PUBLIC(canvas)	XV_PUBLIC(canvas)

#define	CANVAS_VIEW_PRIVATE(c)	XV_PRIVATE(Canvas_view_info, Xv_canvas_view, c)
#define	CANVAS_VIEW_PUBLIC(canvas_view)	XV_PUBLIC(canvas_view)


#define	BIT_FIELD(field)	unsigned field : 1


typedef void	(*Function)();

#ifdef OW_I18N
/*
 * pew (PreEdit Window) data structure, this will be hanging off on
 * the parent frame using XV_KEY_DATA.
 */
typedef struct {
    Frame	frame;
    Panel	panel;
    Panel_item	ptxt;
    int		reference_count;
    int		active_count;
} Canvas_pew;
#endif /* OW_I18N */

typedef struct {
    Canvas	public_self;	/* back pointer to public self */
    int		margin;		/* view pixwin margin */
    int		width, height;
    int		min_paint_width;
    int		min_paint_height;
    Function 	repaint_proc;
    Function 	resize_proc;
    Attr_avlist	paint_avlist; 	/* cached pw avlist on create */
    Attr_avlist	paint_end_avlist;

    struct {
	BIT_FIELD(auto_expand);		/* auto expand canvas with window */
	BIT_FIELD(auto_shrink);		/* auto shrink canvas with window */
	BIT_FIELD(fixed_image);		/* canvas is a fixed size image */
	BIT_FIELD(retained);		/* canvas is a retained window */
	BIT_FIELD(created);		/* first paint window is created */
	BIT_FIELD(x_canvas);		/* treat canvas as an X drawing surface */
	BIT_FIELD(no_clipping);		/* ignore clip rects on repaint */
	BIT_FIELD(cms_repaint);         /* generate repaint on cms changes */
#ifdef OW_I18N
	BIT_FIELD(preedit_exist);       /* keep track of preedit status */
#endif
    } status_bits;

#ifdef OW_I18N
    /*
     * pe_cache is used to cache the current preedit string for each canvas.
     * Since all canvasses in a frame share the preedit window, but
     * we show the string for the canvas that has the focus, we need to
     * cache for each canvas.
     */
    XIC		ic;			/* cache the ic */
    XIMPreeditDrawCallbackStruct *pe_cache; /* cache current preedit string */
    Canvas_pew	*pew;			/* handle to preedit window (Frame) */
    Canvas_paint_window	focus_pwin;	/* Last paint win which had a focus */
#ifdef FULL_R5
    XIMStyle	     xim_style;
#endif /*FULL_R5*/
#endif /*OW_I18N*/
} Canvas_info;

typedef struct {
    Canvas_view	public_self;	/* back pointer to public self */
    Canvas_info		*private_canvas;
    Xv_Window		paint_window;
} Canvas_view_info;


#define	status(canvas, field)		((canvas)->status_bits.field)
#define	status_set(canvas, field)	status(canvas, field) = TRUE
#define	status_reset(canvas, field)	status(canvas, field) = FALSE

Pkg_private  Attr_attribute  canvas_context_key;
Pkg_private  Attr_attribute  canvas_view_context_key;

#ifdef OW_I18N
/*
 * The following variables are declared as Attr_attribute, because
 * they are used in as keys XV_KEY_DATA for the frame.
 */
/*
 * preedit window (frame_cmd) hanging off from parent frame.
 */
Pkg_private  Attr_attribute  canvas_pew_key;
#endif /*OW_I18N*/

/* from canvas_input.c */
Pkg_private Notify_value	canvas_view_event();
Pkg_private Notify_value	canvas_paint_event();

/* from canvas_set.c */
Pkg_private Xv_opaque	canvas_set_avlist();

/* from canvas_get.c */
Pkg_private Xv_opaque	canvas_get_attr();

/* from canvas_resze.c */
Pkg_private void		canvas_resize_paint_window();

/* from canvas_scroll.c */
Pkg_private void	canvas_scroll();
Pkg_private void	canvas_set_scrollbar_object_length();
/* from canvas.c */
Pkg_private int      canvas_init();
Pkg_private int      canvas_destroy();
/*from canvas_view.c */
Pkg_private int      canvas_view_init();
Pkg_private Xv_opaque        canvas_view_get();
Pkg_private Xv_opaque        canvas_paint_get();
Pkg_private Xv_opaque        canvas_paint_set();
Pkg_private int      canvas_view_destroy();

#ifdef OW_I18N
/*
 * Warning: Accessing panel callbacks from behind.
 */
Xv_private void				 panel_preedit_display();
Xv_private XIMPreeditDrawCallbackStruct	*panel_get_preedit();
Xv_private void				 panel_set_preeedit();
Xv_private				 panel_text_start();
Xv_private				 panel_text_done();

/*
 * XIM Preedit callback routines.
 */
Pkg_private void			 canvas_preedit_start();
Pkg_private void			 canvas_preedit_draw();
Pkg_private void			 canvas_preedit_done();

/*
 * Support routines for the pew (PreEdit display Window).
 */
EXTERN_FUNCTION (Pkg_private Canvas_pew *canvas_create_pew, (Frame frame));
EXTERN_FUNCTION (Pkg_private Notify_value canvas_pew_destory,
			(Notify_client client,	Destroy_status status));
EXTERN_FUNCTION (Pkg_private Notify_value canvas_pew_event_proc,
			(Window parent_win, Event *event,
			 Notify_arg arg, Notify_event_type type));
#endif /* OW_I18N */
