/*	@(#)noticeimpl.h 20.38 93/06/28	*/

/* ------------------------------------------------------------------ */
/*	
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE 
 *	file for terms of the license. 
 */
/* ------------------------------------------------------------------ */

#ifndef notice_impl_h_already_defined
#define notice_impl_h_already_defined

#ifdef OW_I18N
#include <xview/xv_i18n.h>
#endif /* OW_I18N */
#include <xview_private/i18n_impl.h>
#include <olgx/olgx.h>
#include <xview/pkg.h>
#include <xview/generic.h>
#include <xview/svrimage.h>
#include <xview/panel.h>
#include <xview/frame.h>
#include <xview/fullscreen.h>
#include <xview/font.h>
#include <xview/defaults.h>

#include <xview/notice.h>

#define NOTICE_PRIVATE(notice_public)	XV_PRIVATE(Notice_info, Xv_notice_struct, notice_public)
#define	NOTICE_PUBLIC(notice)	XV_PUBLIC(notice)
#define NOTICE_HELP		(NOTICE_TRIGGERED-1)
#define NOTICE_ACTION_DO_IT	'\015'

#define VERT_MSG_MARGIN(scale)		Notice_dimensions[scale].vert_msg_margin
#define HORIZ_MSG_MARGIN(scale)		Notice_dimensions[scale].horiz_msg_margin
#define APEX_DIST(scale)		Notice_dimensions[scale].apex_dist
#define BUT_PORTION_HEIGHT(scale)	Notice_dimensions[scale].but_portion_height
#define FONT_POINTSIZE(scale)		Notice_dimensions[scale].font_pointsize
#define FONT_POINTSIZE(scale)		Notice_dimensions[scale].font_pointsize
#define NOTICE_BORDER_WIDTH(scale)	Notice_dimensions[scale].border_width
#define PANE_BORDER_WIDTH(scale)	Notice_dimensions[scale].pane_border_width
#define PANE_NOTICE_BORDER_DIST(scale)	Notice_dimensions[scale].pane_notice_border_dist
#define MSG_VERT_GAP(scale)		Notice_dimensions[scale].msg_vert_gap
#define BUT_HORIZ_GAP(scale)		Notice_dimensions[scale].but_horiz_gap

#define NOTICE_NOT_TOPLEVEL		0
#define NOTICE_IS_TOPLEVEL		1

#define PANE_XY(is_toplevel_window, scale)		\
		( is_toplevel_window ?			\
		    (NOTICE_BORDER_WIDTH(scale)+	\
		    PANE_NOTICE_BORDER_DIST(scale)+	\
		    PANE_BORDER_WIDTH(scale)) :		\
		    PANE_BORDER_WIDTH(scale)		\
		)
#define	PANE_NOTICE_DIFF(is_toplevel_window, scale) \
		(2 * (PANE_XY(is_toplevel_window, scale)+1))

#define		NOTICE_SMALL		0
#define		NOTICE_MEDIUM		1
#define		NOTICE_LARGE		2
#define		NOTICE_EXTRALARGE	3

/* ------------------------------------------------------------------ */
/* -------------- opaque types and useful typedefs  ----------------- */
/* ------------------------------------------------------------------ */

typedef struct notice {
    Xv_Notice		public_self;

    Frame		client_window;
    Frame		owner_window;

    /*
     * XView objects that make up the non-screen locking
     * notice
     */
    Frame		sub_frame;
    Panel		panel;
    Frame		*busy_frames;
    void		(*event_proc)();

    Fullscreen		fullscreen;
    Xv_object		fullscreen_window;

    int			result;
    int			*result_ptr;

    int			default_input_code;
    Event		*event;
    Event		help_event;

    Xv_Font		notice_font;

    int			beeps;
    
    int			focus_x;
    int			focus_y;

    int			old_mousex;
    int			old_mousey;

    CHAR		**message_items;

    int			number_of_buttons;
    int			number_of_strs;
    struct notice_buttons *button_info;
    struct notice_msgs 	*msg_info;
    char 		*help_data;

    Graphics_info	*ginfo;
    int			three_d;

    /*
     * Notice scale
     */
    int			scale;

    /* flags */
    unsigned		lock_screen:1;
    unsigned		yes_button_exists:1;
    unsigned		no_button_exists:1;
    unsigned		focus_specified:1;
    unsigned		dont_beep:1;
    unsigned		need_layout:1;
    unsigned		show:1;
    unsigned		new:1;
    unsigned		block_thread:1;

} Notice_info;

typedef struct notice	*notice_handle;

struct notice_msgs {
    Panel			panel_item;
    CHAR			*string;
    struct rect			 msg_rect;
    struct notice_msgs		*next;
};

struct notice_buttons {
    Panel			panel_item;
    CHAR			*string;
    int				 value;
    int				 is_yes;
    int				 is_no;
    struct rect			 button_rect;
    struct notice_buttons	*next;
};

typedef struct notice_buttons	*notice_buttons_handle;
typedef struct notice_msgs	*notice_msgs_handle;

typedef struct {
    unsigned int	width;			/* (a) */
    unsigned int	vert_msg_margin;	/* (b) */
    unsigned int	horiz_msg_margin;	/* (c) */
    unsigned int	apex_dist;		/* (d) */
    unsigned int	but_portion_height;	/* (e) */
    unsigned int	font_pointsize;		/* (f) */
    unsigned int	border_width;		/* extra */
    unsigned int	pane_border_width;	/* extra */
    unsigned int	pane_notice_border_dist;/* extra */
    unsigned int	msg_vert_gap;		/* extra */
    unsigned int	but_horiz_gap;		/* extra */
}Notice_config;

Pkg_private Notice_config	Notice_dimensions[];

Pkg_private void			notice_add_default_button();
Pkg_private void			notice_defaults();
Pkg_private void			notice_add_button_to_list();
Pkg_private void			notice_add_msg_to_list();
Pkg_private void			notice_free_button_structs();
Pkg_private void			notice_free_msg_structs();
Pkg_private void			notice_do_bell();
Pkg_private void			notice_init_cursor();
Pkg_private void			notice_draw_borders();
Pkg_private void			notice_get_notice_size();
Pkg_private void			notice_layout();
Pkg_private void			notice_do_buttons();
Pkg_private void			notice_drawbox();
Pkg_private void			notice_build_button();
Pkg_private void			notice_button_panel_proc();
Pkg_private int				notice_determine_font();
Pkg_private int				notice_center();
Pkg_private int				notice_subframe_layout();
Pkg_private int				notice_text_width();
Pkg_private int				notice_button_width();
Pkg_private int				notice_get_owner_frame();
Pkg_private notice_buttons_handle	notice_create_button_struct();
Pkg_private notice_msgs_handle		notice_create_msg_struct();

Pkg_private Xv_opaque	notice_set_avlist();
Pkg_private Xv_opaque	notice_generic_set();
Pkg_private Xv_opaque	notice_get_attr();
Pkg_private int		notice_destroy_internal();
Pkg_private int		notice_init_internal();

Pkg_private int		default_beeps;
Pkg_private int		notice_use_audible_bell;
Pkg_private int		notice_jump_cursor;
Pkg_private int		notice_context_key;
Pkg_private Defaults_pairs bell_types[];

#endif /* notice_impl_h_already_defined */
