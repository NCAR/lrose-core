#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)frame_init.c 1.46 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */
#include <sys/param.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <xview_private/draw_impl.h>
#include <xview/cursor.h>
#include <xview_private/fm_impl.h>
#include <xview/server.h>
#include <xview_private/svr_atom.h>
#include <xview/defaults.h>
#include <xview/font.h>

#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN 64
#endif

extern Attr_avlist attr_find();
Pkg_private void frame_update_compose_led();

static short focus_up_bits[] = {
#include <images/focus_up.cursor>
};

static short focus_right_bits[] = {
#include <images/focus_right.cursor>
};

/*
 * This counter is incremented each time a frame is created and decremented
 * each time a frame is destroyed.  When the value reaches zero, the notifier
 * is stopped, see frame_free().
 */
/* Pkg_private */
int             frame_notify_count = 0;


Pkg_private void
frame_default_done_func(frame)
    Frame           frame;
{
    Xv_Drawable_info 	*info;

    DRAWABLE_INFO_MACRO(frame, info);
    if (xv_get(frame, XV_OWNER) == xv_get(xv_screen(info), XV_ROOT)) {
         (void) xv_destroy_safe(frame);
    }
    else  {
        (void) xv_set(frame, XV_SHOW, FALSE, NULL);
    }
}

/*
 * Convert geometry flags into a gravity that can be used for
 * setting the window hints.
 */
Pkg_private int
frame_gravity_from_flags(flags)
int flags;
{
    switch (flags & (XNegative | YNegative)) {
      case 0:
	return NorthWestGravity;
      case XNegative:
	return NorthEastGravity;
      case YNegative:
	return SouthWestGravity;
      default:
	return SouthEastGravity;
    }
}

Pkg_private int
frame_init(owner, frame_public, avlist)
    Xv_Window       owner;
    Frame           frame_public;
    Attr_avlist	    avlist;
{
    Xv_frame_class 	*frame_object = (Xv_frame_class *) frame_public;
    Frame_class_info 	*frame;
    int                  is_subframe = owner && xv_get(owner, XV_IS_SUBTYPE_OF,
								   FRAME_CLASS);
    short	         userHeight = False, userWidth = False;
    Attr_avlist	    	 attrs;
    Xv_Drawable_info 	*info;
    Atom            	 property_array[3];
    XTextProperty	 WMMachineName;
    char		 hostname[MAXHOSTNAMELEN + 1];

    DRAWABLE_INFO_MACRO(frame_public, info);

    frame = xv_alloc(Frame_class_info);

    /* link to object */
    frame_object->private_data = (Xv_opaque) frame;
    frame->public_self = frame_public;

    /* Tell window manager to leave our input focus alone */
    frame->wmhints.input = FALSE;
    frame->wmhints.flags |= InputHint;
    frame->normal_hints.flags = 0;

    property_array[0] = (Atom) xv_get(xv_server(info), SERVER_WM_TAKE_FOCUS);
    property_array[1] = (Atom) xv_get(xv_server(info), SERVER_WM_DELETE_WINDOW);
    property_array[2] = (Atom) xv_get(xv_server(info), SERVER_WM_SAVE_YOURSELF);

    win_change_property(frame_public, SERVER_WM_PROTOCOLS, XA_ATOM, 32, 
							     property_array, 3);

    /* Set WM_CLIENT_MACHINE property */
    WMMachineName.value = (unsigned char *)hostname;
    WMMachineName.encoding = XA_STRING;
    WMMachineName.format = 8;
    WMMachineName.nitems = (int) xv_get_hostname(hostname, MAXHOSTNAMELEN + 1);
    XSetWMClientMachine(xv_display(info), xv_xid(info), &WMMachineName);

    /* Set WM_CLASS property */
    win_set_wm_class(frame_public);

    /* initialize the footer fields */
    status_set(frame, show_footer, FALSE);
    frame->footer = (Xv_Window)NULL;
    frame->ginfo = (Graphics_info *)NULL;
    frame->default_icon = (Icon)NULL;
#ifdef OW_I18N
    frame->left_footer.pswcs.value = (wchar_t *)NULL;
    frame->right_footer.pswcs.value = (wchar_t *)NULL;
#else
    frame->left_footer = (char *)NULL;
    frame->right_footer = (char *)NULL;
#endif

#ifdef OW_I18N
    /* initialize the IMstatus fields */
    status_set(frame, show_imstatus, FALSE);
    status_set(frame, inactive_imstatus, FALSE);
    frame->imstatus = (Xv_Window)NULL;
    frame->left_IMstatus.pswcs.value = (wchar_t *)NULL;
    frame->right_IMstatus.pswcs.value = (wchar_t *)NULL;
#endif

    /* set default frame flags */
    if (is_subframe) {
	Xv_Drawable_info	*owner_info;
	DRAWABLE_INFO_MACRO(owner, owner_info);
	frame->wmhints.window_group = xv_xid(owner_info);
	frame->normal_hints.flags = PPosition | PSize;
    } else {
	/* Must parse size and position defaults here (instead of in
	 * fm_cmdline.c) to get the sizing hints set correctly.
	 */
	frame->geometry_flags = 0;
	frame->user_rect.r_left = 0;
	frame->user_rect.r_top = 0;
	frame->user_rect.r_width = 1;
	frame->user_rect.r_height = 1;
	frame->wmhints.window_group = xv_xid(info);

	/* created another frame */
	frame_notify_count++;

	frame_set_cmdline_options(frame_public, TRUE);

	/* Adjust normal_hints to be in sync with geometry_flags */
	if (frame->geometry_flags & (XValue | YValue))
	    frame->normal_hints.flags |= USPosition;
	if (frame->geometry_flags & (WidthValue | HeightValue))
	    frame->normal_hints.flags |= USSize;

	/* set the bit gravity of the hints */
	if (frame->normal_hints.flags & USPosition) {
	    frame->normal_hints.win_gravity =
			        frame_gravity_from_flags(frame->geometry_flags);
	    frame->normal_hints.flags |= PWinGravity;
	}
    }
    
    frame->wmhints.flags |= WindowGroupHint;
    frame->default_done_proc = frame_default_done_func;
    
    /*
     * OPEN LOOK change: no cofirmer by default
     */
    status_set(frame, no_confirm, (int) TRUE);


    /*
     * Iconic state and name stripe have to be determined before any size
     * parameters are interpreted, so the attribute list is mashed and
     * explicitly interrogated for them here.
     */
    for (attrs = avlist; *attrs; attrs = attr_next(attrs)) {
	switch ((int) *attrs) {

	  case FRAME_CLOSED:
	    status_set(frame, iconic, (int) attrs[1]);
	    status_set(frame, initial_state, (int) attrs[1]);
	    frame->wmhints.initial_state = ((int) attrs[1] ?
						   IconicState : NormalState);
	    frame->wmhints.flags |= StateHint;
	    ATTR_CONSUME(*attrs);
	    break;
	  case XV_X:
	  case XV_Y:
	  case XV_RECT:
	    frame->normal_hints.flags |= PPosition;
	    break;
	  case WIN_ROWS:
	  case XV_HEIGHT:
	    userHeight = True;
	    break;
	  case WIN_COLUMNS:
	  case XV_WIDTH:
	    userWidth = True;
	    break;
	  default:
	    break;
	}
    }

    /*
     * Initialize the map cache which we use later for determining the
     * state of the window: mapped, withdrawn, iconic.
     */
    window_set_map_cache(frame_public, False);

    (void) xv_set(frame_public,
		  WIN_INHERIT_COLORS, TRUE,
		  WIN_LAYOUT_PROC, frame_layout,
		  WIN_NOTIFY_SAFE_EVENT_PROC, frame_input,
		  WIN_NOTIFY_IMMEDIATE_EVENT_PROC, frame_input,

    /*
     * clear rect info since we are the ones setting (rows, columns). this
     * will let us check later (frame_set_avlist()) if the client has set the
     * position or size.
     */

		  WIN_RECT_INFO, 0,

		  WIN_CONSUME_EVENTS,
		      WIN_MAP_NOTIFY, WIN_UNMAP_NOTIFY,
		      NULL,
		  WIN_IGNORE_EVENT, WIN_REPAINT,
		  NULL, NULL);
    /*
     * Only if the user has not set the width (columns) or height (rows)
     * do we set the default rows and columns.  
     * REMIND: We really need to see about moving this to END_CREATE.
     */
    /* If both width and height is not set, set it to default in one
     * xv_set call to avoid making two seperate ConfigureRequests to
     * the server.
     */
    if ((!userWidth) && (!userHeight)) 
	(void) xv_set(frame_public,
			WIN_COLUMNS, 80,
			WIN_ROWS, 34,
			NULL);
    else if (!userHeight)
	(void) xv_set(frame_public, WIN_ROWS, 34, NULL);
    else if (!userWidth)
	(void) xv_set(frame_public, WIN_COLUMNS, 80, NULL);

    /* Set cached version of rect */
    (void) win_getsize(frame_public, &frame->rectcache);

    if (!is_subframe) {
	int             frame_count;

	frame_count = (int) xv_get(xv_server(info), XV_KEY_DATA, FRAME_COUNT);
	xv_set(xv_server(info), XV_KEY_DATA, FRAME_COUNT, ++frame_count, NULL);
    }

    /* Initialise default foreground and background */
    frame->fg.red = frame->fg.green = frame->fg.blue = (unsigned short)0;
    frame->bg.red = frame->bg.green = frame->bg.blue = (unsigned short)~0;

    XSetWMHints(xv_display(info), xv_xid(info), &(frame->wmhints));

    /* Use old XSetNormalHints function for non-ICCCM wm's */
    if (!defaults_get_boolean("xview.icccmcompliant",
			      "XView.ICCCMCompliant", TRUE))
    	XSetNormalHints(xv_display(info), xv_xid(info), &frame->normal_hints);
    else {
	frame->normal_hints.flags |= PSize; 
	frame->normal_hints.base_width = frame->rectcache.r_width;
	frame->normal_hints.width = frame->rectcache.r_width;
	frame->normal_hints.base_height = frame->rectcache.r_height;
	frame->normal_hints.height = frame->rectcache.r_height;
	frame->normal_hints.x = frame->rectcache.r_left;
	frame->normal_hints.y = frame->rectcache.r_top;
    	XSetWMNormalHints(xv_display(info), xv_xid(info), &frame->normal_hints);
    }

    /* Create the Location Cursor (Focus) Window */
    frame->focus_window = xv_create(xv_root(info), WINDOW,
	WIN_BORDER, FALSE,
	WIN_EVENT_PROC, frame_focus_win_event_proc,
	WIN_FRONT,
	WIN_PARENT, frame_public,
	WIN_RETAINED, FALSE,	/* insure a repaint call on each mapping */
	WIN_SAVE_UNDER, TRUE,
	XV_SHOW, FALSE,
	XV_WIDTH, FRAME_FOCUS_UP_WIDTH,
	XV_HEIGHT, FRAME_FOCUS_UP_HEIGHT,
	XV_KEY_DATA, FRAME_FOCUS_DIRECTION, FRAME_FOCUS_UP,
	XV_KEY_DATA, FRAME_FOCUS_UP_IMAGE,
	    xv_create(xv_screen(info), SERVER_IMAGE,
		XV_WIDTH, 16,
		XV_HEIGHT, 16,
		SERVER_IMAGE_BITS, focus_up_bits,
		SERVER_IMAGE_DEPTH, 1,
		NULL),
	XV_KEY_DATA, FRAME_FOCUS_RIGHT_IMAGE,
	    xv_create(xv_screen(info), SERVER_IMAGE,
		XV_WIDTH, 16,
		XV_HEIGHT, 16,
		SERVER_IMAGE_BITS, focus_right_bits,
		SERVER_IMAGE_DEPTH, 1,
		NULL),
	NULL);

    frame_update_compose_led(frame, False);

    return XV_OK;
}

Pkg_private Xv_window
frame_create_footer(frame)
    Frame_class_info *frame;
{
    extern Graphics_info *xv_init_olgx();
    
    Frame frame_public = FRAME_PUBLIC(frame);
    Frame_rescale_state scale;
    Xv_window footer;
    int three_d;
    
    scale = xv_get(xv_get(frame_public, XV_FONT), FONT_SCALE);

    footer = (Xv_window)xv_create(frame_public, WINDOW,
        WIN_NOTIFY_SAFE_EVENT_PROC, frame_footer_input,				  
        WIN_NOTIFY_IMMEDIATE_EVENT_PROC, frame_footer_input,				  
	WIN_BIT_GRAVITY, ForgetGravity,
        WIN_INHERIT_COLORS, TRUE,				  
        XV_HEIGHT, frame_footer_height(scale),
	XV_KEY_DATA, FRAME_FOOTER_WINDOW, TRUE,
	NULL);
    
    three_d = defaults_get_boolean("OpenWindows.3DLook.Color",
				   "OpenWindows.3DLook.Color", TRUE);
    frame->ginfo = xv_init_olgx(footer, &three_d, xv_get(footer, XV_FONT));

    return(footer);
}

#ifdef OW_I18N
Pkg_private Xv_window
frame_create_IMstatus(frame)
    Frame_class_info *frame;
{
    extern Graphics_info *xv_init_olgx();

    Frame frame_public = FRAME_PUBLIC(frame);
    Frame_rescale_state scale;
    Xv_window IMstatus;
    int three_d;

    scale = xv_get(xv_get(frame_public, XV_FONT), FONT_SCALE);

    IMstatus = (Xv_window)xv_create(frame_public, WINDOW,
        WIN_NOTIFY_SAFE_EVENT_PROC, frame_IMstatus_input,

        WIN_NOTIFY_IMMEDIATE_EVENT_PROC, frame_IMstatus_input,

        WIN_BIT_GRAVITY, ForgetGravity,
        XV_HEIGHT, frame_IMstatus_height(scale),
        XV_KEY_DATA, FRAME_IMSTATUS_WINDOW, TRUE,
        NULL);

    three_d = defaults_get_boolean("OpenWindows.3DLook.Color",
                                   "OpenWindows.3DLook.Color", TRUE);
    frame->ginfo = xv_init_olgx(IMstatus, &three_d, xv_get(IMstatus,
XV_FONT));

    return(IMstatus);
}
#endif
