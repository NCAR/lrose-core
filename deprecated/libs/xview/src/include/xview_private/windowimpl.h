/*      @(#)windowimpl.h 20.83 93/06/28 SMI      */

/***********************************************************************/
/*	                      window_impl.h			       */
/*	
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license. 
 */
/***********************************************************************/

#ifndef window_impl_DEFINED
#define window_impl_DEFINED

#include <sys/types.h>
#include <sys/time.h>

#include <xview/font.h>
#include <xview/pkg.h>
#define	_NOTIFY_MIN_SYMBOLS
#include <xview/notify.h>
#undef	_NOTIFY_MIN_SYMBOLS
#include <xview/rect.h>
#include <xview/rectlist.h>

/* BUG: try to get rid of win_struct.h */
#include <xview/win_struct.h>
#include <xview/win_input.h>
#include <xview/window.h>
#include <X11/Xutil.h>

#include <xview/cursor.h>
#include <xview_private/draw_impl.h>
#include <xview_private/xv_list.h>
#include <xview_private/portable.h>
#include <xview/dragdrop.h>

#ifdef OW_I18N
#include <xview_private/i18n_impl.h>
#endif

#define window_attr_next(attr) (Window_attribute *)attr_next((caddr_t *)attr)

/* 
   MAX_FUNC_KEY is maximum number of function keys + number of buttons in the
   mouse.
*/
#define MAX_KEYCODE		128
#define BITS_PER_BYTE		8
#define WINDOW_KEYMASK		(MAX_KEYCODE/((sizeof(char))* BITS_PER_BYTE))

#define DEFAULT_X_Y		0
#define DEFAULT_WIDTH_HEIGHT	64

/* For the rect of the window */
#define         EMPTY_VALUE                     0x7fff


#define WIN_DEFAULT_RECT(_rect) (((_rect)->r_left == DEFAULT_X_Y) && \
				 ((_rect)->r_top == DEFAULT_X_Y) && \
				 ((_rect)->r_width == DEFAULT_WIDTH_HEIGHT) && \
				 ((_rect)->r_height == DEFAULT_WIDTH_HEIGHT)) 

#define	WIN_PRIVATE(win)	XV_PRIVATE(Window_info, Xv_window_struct, win)
#define	WIN_PUBLIC(win)		XV_PUBLIC(win)

#define WIN_SET_DEAF(_win_info, flag) (_win_info->deaf = flag)
#define WIN_IS_DEAF(_win_info) (_win_info->deaf)

#define WIN_SET_LOOP(_win_info, flag) (_win_info->window_loop = flag)
#define WIN_IS_LOOP(_win_info) (_win_info->window_loop)

#ifdef OW_I18N
#define WIN_SET_GRAB(_win_info, flag) (_win_info->active_grab = flag)
#define WIN_IS_GRAB(_win_info) (_win_info->active_grab)

#define WIN_SET_PASSIVE_GRAB(_win_info, flag) (_win_info->passive_grab = flag)
#define WIN_IS_PASSIVE_GRAB(_win_info) (_win_info->passive_grab)
#endif /* OW_I18N */

/* windows are in charge of their own borders */

/***********************************************************************/
/*	        	Structures 				       */
/***********************************************************************/
typedef struct window_client_msg {
    Xv_opaque		type;
    unsigned char	format;
    union {
    	char		b[WIN_MESSAGE_DATA_SIZE];
	short		s[WIN_MESSAGE_DATA_SIZE/sizeof(short)];
	int		l[WIN_MESSAGE_DATA_SIZE/sizeof(int)];
	} data;
} Window_client_msg;

typedef struct win_drop_site_list {
    Xv_sl_link 		 next;
    Xv_drop_site	 drop_item;
} Win_drop_site_list;

typedef enum win_drop_site_mode {
	Win_Drop_Site,
	Win_Drop_Interest,
} Win_drop_site_mode;

typedef struct window_info {
    Xv_Window		 public_self;	     /* back pointer to public struct */
    int			(*layout_proc)();
    void                (*event_proc)();
    void                (*notify_safe_event_proc)();
    void                (*notify_immediate_event_proc)();
    struct window_info	*owner;
    Xv_Window		 parent;
    Xv_opaque		 menu;
    Xv_font		 font;
    Xv_Font		 glyph_font;      /* OL glyph font for window's scale */
    int			 desired_width;
    int			 desired_height;
    Xv_Cursor		 cursor;
    Xv_Cursor		 normal_cursor;     /* A place to store normal cursor */
					    /* when switching to busy cursor  */
    Xv_opaque		 client_data; 
    Rect		 cache_rect;
    unsigned int	 xmask;   			      /* X event mask */
    int	 		 scale;
    char		*cmdline;
    Pixmap		 background_pixmap;
    Window_client_msg	 client_message; 	       /* Client Message Info */
    Win_drop_site_list  *dropSites;		  /* Drop sites in the window */
    Win_drop_site_list  *dropInterest; /* Only used in top-level windows.
					* Holds the list of drop items.       */

    /* margin info */
    short		 top_margin;
    short		 bottom_margin;
    short		 left_margin;
    short		 right_margin;
    short		 row_height;
    short		 column_width;
    short		 row_gap;
    short		 column_gap;

    /* flags */
    unsigned		 has_kbd:1;
    unsigned		 map:1;			  /* change to map when ready */
    unsigned		 rect_info:4;		/* x, y, width, or height set */
    unsigned		 top_level:1; 
    unsigned		 top_level_no_decor:1;     /* does window have decors */
    unsigned		 created:1; 
    unsigned		 has_border:1;             /* does window have border */
    unsigned		 being_rescaled:1;
    unsigned 		 input_only:1;            /* is the window input only */
    unsigned 		 transparent:1;          /* is background pixmap=None */
    unsigned 		 in_fullscreen_mode:1;   /* window in fullscreen mode */
    unsigned 		 is_client_pane:1;     /* is the window a client pane */
    unsigned		 x_paint_window:1;      /* window used for X graphics */
    unsigned		 inherit_colors:1;
    unsigned		 no_clipping:1;     /* dont set clip rects on repaint */
    unsigned		 collapse_exposures:1;      /* colapse expose events  */
						    /* into a single event.   */
						    /* count = 0              */
    unsigned             collapse_motion_events:1;  /* colapse motion events  */
    unsigned		 deaf:1;                  	   /* is window deaf? */
    unsigned		 window_loop:1;          /* is window in window_loop? */
    unsigned		 softkey_flag:1;          /* is soft key labels set? */
#ifdef OW_I18N
    unsigned		 win_use_im:1; 	 /* does window need an input method? */
    unsigned		 ic_conversion:1;              /* is IC conversion on */
    unsigned		 ic_created:1;                       /* is IC created */
    unsigned		 ic_active:1;     /*is IC active? for read-only modes */
    unsigned             active_grab:1;           /* is window in active grab */    
    unsigned             passive_grab:1;           /* is window in passive grab
*/

    /* input method data */
    XIC			 xic;			      /* X Input Context (IC) */
    XID			 ic_focus_win;    	           /* IC focus window */
    XID			 tmp_ic_focus_win;    	      /* temp IC focus window */
    char		 *win_ic_committed;   /* mbyte implicit commit string */
    wchar_t		 *win_ic_committed_wcs;/*widec implicit commit string */
    XIMCallback          start_pecb_struct;	    /* preedit start callback */
    XIMCallback          draw_pecb_struct;	     /* preedit draw callback */
    XIMCallback          caret_pecb_struct;	    /* preedit caret callback */
    XIMCallback          done_pecb_struct;	     /* preedit done callback */
    XIMCallback          start_stcb_struct;	     /* status start callback */
    XIMCallback          draw_stcb_struct;	      /* status draw callback */
    XIMCallback          done_stcb_struct;	      /* status done callback */
#ifdef FULL_R5
    unsigned long	 x_im_style_mask;	 /* X input method style mask */
#endif /* FULL_R5 */
#endif /* OW_I18N */

} Window_info;


/* 
 * Package private
 */

#define	actual_row_height(win)		 \
    (win->row_height ? win->row_height : \
				xv_get(win->font, FONT_DEFAULT_CHAR_HEIGHT))

#ifdef OW_I18N
#define	actual_column_width(win)	     \
    (win->column_width ? win->column_width : \
				 xv_get(win->font, FONT_COLUMN_WIDTH))
#else /* OW_I18N */
#define	actual_column_width(win)	     \
    (win->column_width ? win->column_width : \
				 xv_get(win->font, FONT_DEFAULT_CHAR_WIDTH))
#endif /* OW_I18N */

#define	actual_rescale_row_height(par,win) \
    (win->row_height ? win->row_height :   \
				xv_get(par->font, FONT_DEFAULT_CHAR_HEIGHT))

#ifdef OW_I18N
#define	actual_rescale_column_width(par,win) \
    (win->column_width ? win->column_width : \
				xv_get(par->font, FONT_COLUMN_WIDTH))
#else /* OW_I18N */
#define	actual_rescale_column_width(par,win) \
    (win->column_width ? win->column_width : \
				xv_get(par->font, FONT_DEFAULT_CHAR_WIDTH))
#endif /* OW_I18N */ 

/* window.c */
Pkg_private Notify_value	window_default_event_func();
Xv_private int			window_getrelrect();

/* window_set.c */
Pkg_private Xv_opaque		window_set_avlist();
Pkg_private int			win_appeal_to_owner();

/* window_get.c */
Pkg_private Xv_opaque		window_get_attr();

/* window_layout.c */
Pkg_private int			window_layout();

/* window_compat.c */
Xv_private void			window_scan_and_convert_to_pixels();

/* windowdrop.c */
Pkg_private void		win_add_drop_item();
Pkg_private Xv_opaque		win_delete_drop_item();
Pkg_private void		win_add_drop_interest();
Pkg_private void		win_update_dnd_property();
Xv_private Xv_opaque		win_get_top_level();

/* windowutil.c */
Xv_private void			win_set_wm_command();
Xv_private void			win_set_wm_class();
Xv_private int			window_set_tree_flag();
Xv_private int			window_set_tree_child_flag();
Xv_private int			window_set_flag_cursor();


#endif /* ~window_impl_DEFINED */
