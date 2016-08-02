#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)win_input.c 20.208 93/06/28";
#endif
#endif

/*
 * (c) Copyright 1989 Sun Microsystems, Inc. Sun design patents pending in
 * the U.S. and foreign countries. See LEGAL NOTICE file for terms of the
 * license.
 */

/*
 * Win_input.c: Implement the input functions of the win_struct.h interface.
 */

#include <stdio.h>
#include <errno.h>
#include <sys/time.h>
/* mbuck@debian.org */
#if 1
#ifdef SUNOS5
#include <X11/Xlib.h>		/* required by Xutil.h */
#endif
#include <X11/Xlibint.h>	/* required by Xutil.h */
#else
#include <X11/Xlib.h>		/* required by Xutil.h */
#endif
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <xview_private/svr_impl.h>
#define X11R6

/*
 * Note: ntfy.h must be before notify.h. (draw_impl.h includes pkg.h which
 * includes notify.h)
 */
#include <xview_private/i18n_impl.h>
#include <xview_private/portable.h>
#include <xview_private/ntfy.h>
#include <xview_private/draw_impl.h>
#include <xview_private/ndis.h>
#include <xview_private/svr_atom.h>
#include <xview_private/win_info.h>
#include <xview_private/win_keymap.h>
#include <xview_private/windowimpl.h>
#include <xview_private/fm_impl.h>
#include <xview/defaults.h>
#include <xview/frame.h>
#include <xview/fullscreen.h>
#include <xview/icon.h>
#include <xview/server.h>
#include <xview/win_notify.h>
#include <xview/win_screen.h>
#include <xview/dragdrop.h>
#include <xview/screen.h>
#include <xview/panel.h>
#include <xview/textsw.h>
#include <xview/termsw.h>

static void     tvdiff();
static void     win_handle_quick_selection();
static int      BlockForEvent();
static int      GetButtonEvent();
static int      win_translate_KP_keysym();
static int 	translate_key();

static int
  xevent_to_event(Display *display, XEvent *xevent,
                  Event *event, Xv_object *pwindow);

extern struct rectlist *win_get_damage();
extern void     server_set_timestamp();
extern Xv_opaque server_get_timestamp();
extern Xv_object win_data();
extern void     window_update_cache_rect();
extern char    *xv_app_name;

Xv_private void xv_get_cmdline_str();
Xv_private void xv_get_cmdline_argv();
Xv_private void window_release_selectbutton();
Xv_private void window_x_allow_events();
Xv_private void server_do_xevent_callback();
Xv_private void win_get_cmdline_option();
Xv_private void win_set_wm_command_prop();
Xv_private void server_refresh_modifiers();
/* ACC_XVIEW */
Xv_private int  win_handle_window_accel();
Xv_private int  win_handle_menu_accel();
/* ACC_XVIEW */

#ifndef __linux
FILE           *fopen(), *fexp;
#endif

Xv_object       xview_x_input_readevent();
extern          input_imnull();

static int      process_clientmessage_events();
static int      process_property_events();
static int      process_wm_pushpin_state();
Pkg_private int win_handle_compose();

struct _XKeytrans {
        struct _XKeytrans *next;/* next on list */
        char *string;           /* string to return when the time comes */
        int len;                /* length of string (since NULL is legit)*/
        KeySym key;             /* keysym rebound */
        unsigned int state;     /* modifier state */
        KeySym *modifiers;      /* modifier keysyms you want */
        int mlen;               /* length of modifier list */
};

#define AllMods (ShiftMask|LockMask|ControlMask| \
		 Mod1Mask|Mod2Mask|Mod3Mask|Mod4Mask|Mod5Mask)

#define 	BUFFERSIZE	256
#define		DND_ERROR_CODE  XV_MSG("Unexpected event type in ACTION_DROP_PREVIEW event")

/*
 * Are we in xv_window_loop ?
 */
int             xv_in_loop;

Xv_private void
win_xmask_to_im(xevent_mask, im)
    register unsigned int xevent_mask;
    register Inputmask *im;

{
    register int    i;

    input_imnull(im);

    if (xevent_mask & ExposureMask)
	win_setinputcodebit(im, WIN_REPAINT);

    if (xevent_mask & PointerMotionMask)
	win_setinputcodebit(im, LOC_MOVE);

    if (xevent_mask & EnterWindowMask)
	win_setinputcodebit(im, LOC_WINENTER);

    if (xevent_mask & LeaveWindowMask)
	win_setinputcodebit(im, LOC_WINEXIT);

    if (xevent_mask & ButtonMotionMask)
	win_setinputcodebit(im, LOC_DRAG);

    if (xevent_mask & KeymapStateMask)
	win_setinputcodebit(im, KBD_MAP);

    if (xevent_mask & VisibilityChangeMask)
	win_setinputcodebit(im, WIN_VISIBILITY_NOTIFY);

    if (xevent_mask & StructureNotifyMask)
	win_setinputcodebit(im, WIN_STRUCTURE_NOTIFY);

    if (xevent_mask & SubstructureNotifyMask)
	win_setinputcodebit(im, WIN_SUBSTRUCTURE_NOTIFY);

    if (xevent_mask & ResizeRedirectMask)
	win_setinputcodebit(im, WIN_RESIZE_REQUEST);

    if (xevent_mask & PropertyChangeMask)
	win_setinputcodebit(im, WIN_PROPERTY_NOTIFY);

    if (xevent_mask & ColormapChangeMask)
	win_setinputcodebit(im, WIN_COLORMAP_NOTIFY);

    if (xevent_mask & SubstructureRedirectMask)
	win_setinputcodebit(im, WIN_SUBSTRUCTURE_REDIRECT);

    if (xevent_mask & KeyPressMask) {
	im->im_flags |= IM_ASCII;
	for (i = 1; i <= (KEY_LEFTLAST - KEY_LEFTFIRST); i++)
	    win_setinputcodebit(im, KEY_LEFT(i));

	for (i = 1; i <= (KEY_RIGHTLAST - KEY_RIGHTFIRST); i++)
	    win_setinputcodebit(im, KEY_RIGHT(i));

	for (i = 1; i <= (KEY_TOPLAST - KEY_TOPFIRST); i++)
	    win_setinputcodebit(im, KEY_TOP(i));
    }
    if (xevent_mask & KeyReleaseMask)
	im->im_flags |= (IM_NEGASCII | IM_NEGEVENT | IM_NEGMETA);

    if (xevent_mask & FocusChangeMask) {
	win_setinputcodebit(im, KBD_USE);
	win_setinputcodebit(im, KBD_DONE);
    }
    if ((xevent_mask & ButtonPressMask) || (xevent_mask & ButtonReleaseMask)) {
	for (i = 1; i <= (BUT_LAST - BUT_FIRST); i++)
	    win_setinputcodebit(im, BUT(i));
	if (xevent_mask & ButtonReleaseMask)
	    im->im_flags |= IM_NEGEVENT;
    }
}


/*
 * Convert Sunview events to xevents.
 */
Xv_private unsigned int
win_im_to_xmask(window, im)
    Xv_object       window;
    register Inputmask *im;
{
    register unsigned int xevent_mask = 0;
    register int    i;

    /*
     * BUG The events that cannot be generated in X are: LOC_STILL,
     * LOC_RGN{EXIT,ENTER}, WIN_STOP, LOC_TRAJECTORY, KBD_REQUEST
     */
    if (win_getinputcodebit(im, LOC_MOVE))
	xevent_mask |= PointerMotionMask;
    if (win_getinputcodebit(im, LOC_WINENTER))
	xevent_mask |= EnterWindowMask;
    if (win_getinputcodebit(im, LOC_WINEXIT))
	xevent_mask |= LeaveWindowMask;
    if (win_getinputcodebit(im, KBD_MAP))
	xevent_mask |= KeymapStateMask;
    if (win_getinputcodebit(im, WIN_VISIBILITY_NOTIFY))
	xevent_mask |= VisibilityChangeMask;
    if ((win_getinputcodebit(im, WIN_CIRCULATE_NOTIFY)) ||
	(win_getinputcodebit(im, WIN_DESTROY_NOTIFY)) ||
	(win_getinputcodebit(im, WIN_GRAVITY_NOTIFY)) ||
	(win_getinputcodebit(im, WIN_MAP_NOTIFY)) ||
	(win_getinputcodebit(im, WIN_REPARENT_NOTIFY)) ||
    /*
     * (win_getinputcodebit(im, WIN_RESIZE)) ||
     */
	(win_getinputcodebit(im, WIN_UNMAP_NOTIFY)))
	xevent_mask |= StructureNotifyMask;

    if (win_getinputcodebit(im, WIN_SUBSTRUCTURE_NOTIFY))
	xevent_mask |= SubstructureNotifyMask;

    if (win_getinputcodebit(im, WIN_RESIZE_REQUEST))
	xevent_mask |= ResizeRedirectMask;

    if (win_getinputcodebit(im, WIN_PROPERTY_NOTIFY))
	xevent_mask |= PropertyChangeMask;

    if (win_getinputcodebit(im, WIN_COLORMAP_NOTIFY))
	xevent_mask |= ColormapChangeMask;

    if ((win_getinputcodebit(im, WIN_CIRCULATE_REQUEST)) ||
	(win_getinputcodebit(im, WIN_CONFIGURE_REQUEST)) ||
	(win_getinputcodebit(im, WIN_MAP_REQUEST)))
	xevent_mask |= SubstructureRedirectMask;

    if (win_getinputcodebit(im, LOC_DRAG)) {
	xevent_mask |= ButtonMotionMask;
    }
    /* BUT(1-3) are MS_{LEFT, MIDDLE, RIGHT} */
    for (i = 1; i <= (BUT_LAST - BUT_FIRST); i++) {
	if (win_getinputcodebit(im, BUT(i))) {
	    xevent_mask |= ButtonPressMask;
	    break;
	}
    }

    /* Set ButtonReleaseMask if some button consumed and IM_NEGEVENT */
    if (xevent_mask & ButtonPressMask && im->im_flags & IM_NEGEVENT)
	xevent_mask |= ButtonReleaseMask;

    if (win_getinputcodebit(im, WIN_REPAINT)) {
	xevent_mask |= ExposureMask;
    }
    /* Enable focus change events if consuming KBD_USE/KBD_DONE */
    if (win_getinputcodebit(im, KBD_USE) ||
	win_getinputcodebit(im, KBD_DONE)) {
	xevent_mask |= FocusChangeMask;
    }
    /*
     * if top level window AND it does not have no decor flag set then turn
     * on StructureNotify  and PropertyChangeMask
     */
    if ((window) && (Bool) xv_get(window, WIN_TOP_LEVEL) &&
	!(Bool) xv_get(window, WIN_TOP_LEVEL_NO_DECOR)) {
	xevent_mask |= StructureNotifyMask | PropertyChangeMask;
    }
    /*
     * NOTE:  If interested in any keyboard events, must set ButtonPressMask
     * and FocusChangeMask for Click to Type.
     * 
     * BUG ALERT: If you are interested in any keyboard events, you will see
     * button press, KBD_USE, and KBD_DONE events whether you want them or
     * not.
     */
    if ((im->im_flags & IM_NEGASCII) ||
	(im->im_flags & IM_NEGMETA))
	xevent_mask |= KeyReleaseMask | ButtonPressMask | FocusChangeMask;

    /*
     * NOTE:  Anything below this point only deal with KeyPressMask.
     */
    if (im->im_flags & IM_ASCII) {
	xevent_mask |= KeyPressMask | FocusChangeMask;
	goto Return;
    }
    for (i = 1; i <= KEY_LEFTLAST - KEY_LEFTFIRST; i++)
	if (win_getinputcodebit(im, KEY_LEFT(i))) {
	    xevent_mask |= KeyPressMask | FocusChangeMask;
	    goto Return;
	}
    for (i = 1; i <= KEY_RIGHTLAST - KEY_RIGHTFIRST; i++)
	if (win_getinputcodebit(im, KEY_RIGHT(i))) {
	    xevent_mask |= KeyPressMask | FocusChangeMask;
	    goto Return;
	}
    for (i = 1; i <= KEY_TOPLAST - KEY_TOPFIRST; i++)
	if (win_getinputcodebit(im, KEY_TOP(i))) {
	    xevent_mask |= KeyPressMask | FocusChangeMask;
	    goto Return;
	}
Return:
    /* Set KeyReleaseMask if consuming keyboard events and IM_NEGEVENT */
    if (im->im_flags & IM_NEGEVENT && xevent_mask & KeyPressMask)
	xevent_mask |= KeyReleaseMask;
    return (xevent_mask);
}

/*
 * Utilities
 */
input_imnull(im)
    struct inputmask *im;
{
    int             i;
    /* BUG:  Use XV_BZERO here */
    im->im_flags = 0;
    for (i = 0; i < IM_MASKSIZE; i++)
	im->im_keycode[i] = 0;
}

input_imall(im)
    struct inputmask *im;
{
    int             i;

    (void) input_imnull(im);
    im->im_flags = IM_ASCII | IM_META;
    for (i = 0; i < IM_MASKSIZE; i++)
	im->im_keycode[i] = 1;
}

/*
 * Find an event for req_window. Block till the event for this window is
 * found.
 */
Xv_object
input_readevent(window, event)
    Xv_object       window;
    Event          *event;
{
    register Xv_Drawable_info *info;
    XEvent          xevent;
    Xv_object       retval;

    DRAWABLE_INFO_MACRO(window, info);
    retval = xview_x_input_readevent(xv_display(info), event, window, TRUE, FALSE, 0, &xevent);
    
    /* don't depend on the appl to do an XAllowEvent */
    if (retval && event_id(event) == MS_LEFT)
	window_release_selectbutton(window, event);

    return (retval);
}

/* BUG: implement or throw out all this focus stuff */

/* ARGSUSED */
win_refuse_kbd_focus(window)
    Xv_object       window;
{
}

/* ARGSUSED */
win_release_event_lock(window)
    Xv_object       window;
{
}

int
win_set_kbd_focus(window, xid)
    Xv_object       window;
    XID             xid;
{
    int             rtn = 0;
    register Xv_Drawable_info *info;
    Xv_opaque       server_public;
    register Window_info *win;


    DRAWABLE_INFO_MACRO(window, info);
    /* Get server info */
    server_public = xv_server(info);

    if (xid == (XID) WIN_NULLLINK)
	xid = None;
    if (!xv_has_focus(info)) {
	Display        *display = xv_display(info);

	rtn = XSetInputFocus(display, xid, RevertToParent,
			     server_get_timestamp(server_public));
	win = WIN_PRIVATE(window);
	if (win->softkey_flag) {
	    xv_set(server_public, SERVER_FOCUS_TIMESTAMP, 
				  server_get_timestamp(server_public),NULL);
	}
    }
    /*
     * if(xv_get(xv_server(info),SERVER_JOURNALLING))
     * xv_set(xv_server(info),SERVER_JOURNAL_SYNC_EVENT,1,0);
     */
    return (rtn);
}

XID
win_get_kbd_focus(window)
    Xv_object       window;
{
    register Xv_Drawable_info *info;
    XID             xid;
    int             state;

    DRAWABLE_INFO_MACRO(window, info);
    /* PERFORMANCE: Round trip to the server!!! */
    XGetInputFocus(xv_display(info), &xid, &state);
    return (xid == None ? (XID) WIN_NULLLINK : xid);
}

/*
 * Set no-focus state for window.  If true, don't set input focus on click.
 */
void
win_set_no_focus(window, state)
    Xv_object       window;
    int             state;
{
    register Xv_Drawable_info *info;

    DRAWABLE_INFO_MACRO(window, info);
    xv_set_no_focus(info, state);
}

#define	SET_SHIFTS(e, state, metamask, altmask)	\
	event_set_shiftmask(e, \
	    (((state) & ShiftMask) ? SHIFTMASK : 0) | \
	    (((state) & ControlMask) ? CTRLMASK : 0) | \
	    (((state) & metamask) ? META_SHIFT_MASK : 0) | \
	    (((state) & altmask) ? ALTMASK : 0) | \
	    (((state) & Button1Mask) ? MS_LEFT_MASK : 0) | \
	    (((state) & Button2Mask) ? MS_MIDDLE_MASK : 0) | \
	    (((state) & Button3Mask) ? MS_RIGHT_MASK : 0))

Xv_private_data Event xv_last_event;
Xv_private_data XEvent xv_last_x_event;

typedef struct {
    Xv_object       window_requested;
    Event           event;
}               Event_info;


/*
 * Predicate for XCheckIfEvent or XIfEvent Checks to see if the xevent
 * belongs to req_window or not.
 */
static Bool
is_reqwindow(display, xevent, info)
    Display        *display;
    XEvent         *xevent;
    char           *info;
{
    Xv_object       ie_window;
    XAnyEvent      *any = (XAnyEvent *) xevent;
    int             event_type = (xevent->type & 0177);
    Event_info     *event_info = (Event_info *)info;

    /*
     * Check for proper window before calling xevent_to_event, so translation
     * only takes place if event is wanted.
     */
    if (event_type > 1 &&
	event_info->window_requested == win_data(display, any->window) &&
	!xevent_to_event(display, xevent, &event_info->event, &ie_window))
	return (TRUE);
    else
	return (FALSE);
}

/*
 * Find an event for window.  Block until the event for the window is found.
 */
Xv_object
xv_input_readevent(window, event, block, type, im)
    Xv_object       window;
    Event          *event;
    int             block, type;
    Inputmask      *im;
{
    register Xv_Drawable_info *info;
    unsigned int    xevent_mask;
    XEvent          xevent;
    extern Xv_object xv_default_display;
    Xv_object	    retval;

    if (im) {
	xevent_mask = win_im_to_xmask(window, im);
	if (((Bool) xv_get(window, WIN_TOP_LEVEL) == TRUE) &&
	    !((Bool) xv_get(window, WIN_TOP_LEVEL_NO_DECOR) == TRUE))
	    xevent_mask &= ~StructureNotifyMask & ~PropertyChangeMask;
    }
   
    if (window) 
      DRAWABLE_INFO_MACRO(window, info);


    retval = xview_x_input_readevent((window ? xv_display(info) : 
				     (Display *) xv_default_display),
				    event, window, block, type, xevent_mask, 
				    &xevent);
    
    /* don't depend on the app. to do an XAllowEvent */
    if (retval && event_id(event) == MS_LEFT)
	window_release_selectbutton(window, event);

    return (retval);
}


/*
 * Read an event from an This needs to be rewritten
 */
Xv_object
xview_x_input_readevent(display, event, req_window, block, type, xevent_mask, rep)
    Display        *display;
    register Event *event;
    Xv_object       req_window;
    int             block, type;
    unsigned int    xevent_mask;
    XEvent         *rep;

{
    Xv_object       window = 0;
    Xv_Drawable_info *info;
    Server_info     *server;

    /*
     * Read an event for the req_window.
     */
    if (req_window) {
	Event_info      event_info;

	DRAWABLE_INFO_MACRO(req_window, info);
	if (type) {
	    if (block) {
		XWindowEvent(display, xv_xid(info), xevent_mask, rep);
		(void) xevent_to_event(display, rep, event, &window);
	    } else {
		if (!XCheckWindowEvent(display, xv_xid(info), xevent_mask, rep))
		    return (0);
		(void) xevent_to_event(display, rep, event, &window);
	    }
	    event_set_window(event, req_window);
	} else {
	    event_info.window_requested = req_window;
	    if (block)
		XIfEvent(display, rep, is_reqwindow, (char *)&event_info);
	    else if (!XCheckIfEvent(display, rep, is_reqwindow, (char *)&event_info))
		return (0);	/* window. pending and last event not set */
	    window = event_info.window_requested;
	    *event = event_info.event;
	    /* set the window in the event */
	    event_set_window(event, window);
	}
    } else {
	XNextEvent(display, rep);
	(void) xevent_to_event(display, rep, event, &window);
    }

    if (win_data(display, ((XAnyEvent *)rep)->window)) {
        XV_SL_TYPED_FOR_ALL(SERVER_PRIVATE(xv_default_server), server,
			    Server_info *) 
	    if (server->xidlist && server->xdisplay == display)
		server_do_xevent_callback (server, display, rep);
    }

    /*
     * pending = QLength(display);
     */
    return (window);
}


/*
 * This converts an xevent to an XView event. If the event was invalid i.e.
 * no window wanted it ; *pwindow = 0; return 1 else *pwindow = window to
 * which event is to be posted to ; return 0
 * 
 * NOTE: Code has been placed here for handling click to type.  This isn't a
 * terribly appropriate place for it, but it is convenient, since all X
 * events pass through here before becoming  SunView events.  A modification
 * has been made to is_reqwindow, so each X event should be seen here exactly
 * once.
 */

static int
  xevent_to_event(Display *display, XEvent *xevent,
                  Event *event, Xv_object *pwindow)
{
    register int    	 event_type = (xevent->type & 0177);
    register unsigned 	 temp;
    XAnyEvent      	*any = (XAnyEvent *) xevent;
    Xv_object       	 window = XV_ZERO;
    Xv_Drawable_info 	*info;
    Xv_opaque       	 server_public;
    static XID      	 pointer_window_xid;
    static short    	 nbuttons = 0;
    static Xv_opaque 	 last_server_public = XV_ZERO;
    static unsigned int  but2_mod,
			 but3_mod,
			 chord,
			*key_map;
    static int      	 alt_modmask = 0,
    			 meta_modmask = 0,
    			 num_lock_modmask = 0,
			 sel_modmask = 0,
    			 menu_flag,
    			 chording_timeout,
    			 quote_next_key = FALSE,
    			 suspend_mouseless = FALSE;
    static u_char  	*ascii_sem_map,
     			*key_sem_map,
                        /* ACC_XVIEW */
			*acc_map;
                        /* ACC_XVIEW */
    Bool            	 check_lang_mode();
    static XComposeStatus *compose_status;
#ifdef OW_I18N
    static KeySym	 paste_keysym, cut_keysym;
#endif

    /* XXX: assuming 0 => error, 1 => reply */
    /* XXX: This is bogus!  When will X return an event type < 1??? */
    if (event_type > 1) {
	Window_info    *win;

	window = win_data(display, any->window);
	if (!window) {
	    if (event_type == ClientMessage) {
		XClientMessageEvent *cme = (XClientMessageEvent *) xevent;

		if (cme->data.l[0] == XV_POINTER_WINDOW) {
		    window = win_data(display, pointer_window_xid);
		} else if (cme->data.l[0] == XV_FOCUS_WINDOW) {
		    XID             focus_window_id;
		    int             dummy;

		    (void) XGetInputFocus(display, &focus_window_id, &dummy);
		    window = win_data(display, focus_window_id);
		}
		/* MappingNotify's are handled here because there is no window
		 * associated with the event.
		 */
	    } else if (event_type == MappingNotify) {
	        XMappingEvent  *e = (XMappingEvent *) xevent;
	        if (e->request == MappingKeyboard)
		    XRefreshKeyboardMapping(e);
	        else if (e->request == MappingModifier) {
		   /* Get the server object from the display structure. */
		   if (XFindContext(display, (Window) display,
		                             (XContext)xv_get(xv_default_server,
					                SERVER_DISPLAY_CONTEXT),
					     (caddr_t *)&server_public)) {

		      goto Default;
		   }
		   /* Update our notion of the modifiers. */
		   server_refresh_modifiers(server_public, FALSE);
	           /* Cache the new modifier masks. */
	           alt_modmask = (int) xv_get(server_public,
						SERVER_ALT_MOD_MASK);
	           meta_modmask = (int) xv_get(server_public,
						SERVER_META_MOD_MASK);
	           sel_modmask = (int) xv_get(server_public,
						SERVER_SEL_MOD_MASK);
	           num_lock_modmask = (int) xv_get(server_public,
					 	SERVER_NUM_LOCK_MOD_MASK);
	        }
	    }
	    if (!window) {
#ifdef OW_I18N
                if (XFilterEvent(xevent, NULL) == True) {
	           *pwindow = NULL;
	           return (NULL);
                }
#endif
		goto Default;
	    }
	}
	win = WIN_PRIVATE(window);

    	/* Get server info */
    	DRAWABLE_INFO_MACRO(window, info);
    	server_public = xv_server(info);

    	/*
     	* The following code caches server specific information such as the
     	* keymap, position of the modifier key in the modifier mask and the
     	* number of mouse buttons associated with the mouse.
     	*/
    	if (last_server_public != server_public) {
	   last_server_public = server_public;
	   key_map = (unsigned int *) xv_get(server_public, SERVER_XV_MAP);
	   key_sem_map = (u_char *) xv_get(server_public, SERVER_SEMANTIC_MAP);
	   ascii_sem_map = (u_char *) xv_get(server_public, SERVER_ASCII_MAP);

	   /* Cache the modifier masks. */
	   alt_modmask = (int) xv_get(server_public, SERVER_ALT_MOD_MASK);
	   meta_modmask = (int) xv_get(server_public, SERVER_META_MOD_MASK);
	   sel_modmask = (int) xv_get(server_public, SERVER_SEL_MOD_MASK);
	   num_lock_modmask = (int) xv_get(server_public,
					   SERVER_NUM_LOCK_MOD_MASK);

	   /*
	    * Get the number of phyical mouse buttons and the button modifier
	    * masks.  This info is cached.
	    */
	   nbuttons = (short) xv_get(server_public, SERVER_MOUSE_BUTTONS);
	   but2_mod = (unsigned int) xv_get(server_public, SERVER_BUTTON2_MOD);
	   but3_mod = (unsigned int) xv_get(server_public, SERVER_BUTTON3_MOD);
	   chording_timeout = (int) xv_get(server_public, SERVER_CHORDING_TIMEOUT);
	   chord = (unsigned int) xv_get(server_public, SERVER_CHORD_MENU);
	   compose_status = (XComposeStatus *) xv_get(server_public,
						      SERVER_COMPOSE_STATUS);
#ifdef OW_I18N
	   paste_keysym = (KeySym) xv_get(server_public, SERVER_PASTE_KEYSYM);
	   cut_keysym = (KeySym) xv_get(server_public, SERVER_CUT_KEYSYM);
#endif 
    	}

#ifdef OW_I18N
        /* 
         * Workaround for XGrabKey*() problem using frontend method
	 * Skip XFilterEvent() if the client is grabbing the keyboard
         */
        if (WIN_IS_GRAB(win) || WIN_IS_PASSIVE_GRAB(win)) {
           if (event_type == KeyPress || event_type == KeyRelease) {
	       if (win->active_grab) {
	          goto ContProcess;
	       }
	       if (win->passive_grab) {
                   KeySym      ksym = XKeycodeToKeysym(display, 
				((XKeyEvent *)xevent)->keycode, 0);
                   if (ksym == paste_keysym || ksym == cut_keysym) 
                       goto ContProcess;
	       }
           }
        }
        if (XFilterEvent(xevent, NULL) == True) {
	    *pwindow = NULL;
	    return (NULL);
        }
ContProcess:
#endif

	/*
	 * Check if window is deaf or is NOT involved in xv_window_loop (when
	 * it is active) and the event type is one that we would want it to
	 * ignore. Also check if the select button is pressed so that
	 * XAllowEvents can be called.
	 */
	if ((WIN_IS_DEAF(win) || (WIN_IS_IN_LOOP && !WIN_IS_LOOP(win))) &&
	    ((event_type == KeyPress) ||
	     (event_type == KeyRelease) ||
	     (event_type == ButtonPress) ||
	     (event_type == ButtonRelease) ||
	     (event_type == MotionNotify))) {

	    XButtonEvent   *buttonEvent = (XButtonEvent *) & xevent->xbutton;

	    if ((event_type == ButtonPress)&&(buttonEvent->button == Button1)) {
		window_x_allow_events(display);
	    }
	    goto Default;
	}

    } else {
	XErrorEvent    *er = (XErrorEvent *) & any;

	fprintf(stderr, "Error event: \n");
	fprintf(stderr, "   serial = %d, error code = %d\n", er->serial,
		er->error_code);
	fprintf(stderr, "   request code = %d, minor code = %d\n",
		er->request_code, er->minor_code);
	fprintf(stderr, "   resource id = %d\n", er->resourceid);
	goto Default;
    }


    /* clear out the event */
    event_init(event);

    /* set the window */
    event_set_window(event, window);

    /* make a reference to the XEvent */
    event_set_xevent(event, xevent);

    switch (event_type) {
      case KeyPress:
      case KeyRelease:{
	    XKeyEvent      *ek = (XKeyEvent *) xevent;
	    static char     buffer[BUFFERSIZE];
	    static int      buf_length = 0;
	    KeySym          ksym, sem_ksym, acc_ksym;
	    unsigned int    key_value;
	    int             modifiers = 0,
			    acc_modifiers = 0,
			    sem_action,
			    keyboard_key;
	    int	  	    status = True,
			    old_chars_matched = compose_status->chars_matched;
#ifdef OW_I18N
	    Status	    ret_status;
	    XIC		    ic = NULL;
	    static char	   *buffer_backup = buffer;

	    if (event_type == KeyPress) {
		Window_info    *win = WIN_PRIVATE(window);
		/*
		 * Dosen't use xv_get() for the performance reason.
		 * If false, ic value is NULL.
		 */
		if (win->win_use_im && win->ic_active)
		    ic = win->xic;
	    }
#endif
	    /*
	     * Clear buffer before we fill it again.  Only NULL out the
	     * number of chars that where actually used in the last pass.
	     */
#ifdef OW_I18N
	    if (buffer_backup != buffer) {/*last time was overflowed to backup*/
		xv_free(buffer_backup);
		XV_BZERO(buffer, BUFFERSIZE); /* may have filled the whole
						 array last time */
		buffer_backup = buffer;
	    }
	    else
#endif
	    XV_BZERO(buffer, buf_length);

/* What's this special-casing of NumLock all about? It makes wrong
 * assumptions about Xlib internals, it forces you to use a special keyboard
 * mapping and it simply doesn't work.  So we just use XLookupKeysym instead
 * of this big kludge. This way, not everything works perfect, but the rest
 * of the problems is due to the broken keyboard model of X.
 *
 * Maybe this is all a big misunderstanding, so if somebody can enlighten me,
 * please do so.
 *
 * martin-2.buck@student.uni-ulm.de
 */
#if 0
	    if (ek->state & num_lock_modmask) {
		/* Num Lock is on.  For the keycode, if it has a key pad
		 * keysym in its row, then send event as keypad key. 
		 */
	 	int   index, doLookUp = True;
#ifdef X11R6
	/* lumpi@dobag.in-berlin.de */
		int ksym_pcc;
		XGetKeyboardMapping(display,NoSymbol,0,&ksym_pcc);
		for (index = 0; index < ksym_pcc; index++) {
#else
		for (index = 0; index < display->keysyms_per_keycode; index++) {
#endif
		    if ((ksym = XLookupKeysym(ek, index)) != NoSymbol)
			if (IsKeypadKey(ksym)) {
			    /* See if key has been rebound. */
			    if (!translate_key(display, ksym, ek->state,
					       buffer, BUFFERSIZE)) {
			        (void)win_translate_KP_keysym(ksym, buffer);
			    }
			    buf_length = strlen(buffer);
			    doLookUp = False;
			    break;
			}
		}
		if (doLookUp)
#ifdef OW_I18N
		    goto DoLookup;
#else
		    buf_length = XLookupString(ek, buffer, BUFFERSIZE, &ksym,
					       compose_status);
#endif
	    } else {
#endif
		/*
		 * Num Lock is off: Don't use the default policy in
		 * Xlib for the right keypad navigation keys.
		 */
		ksym = XKeycodeToKeysym(display, ek->keycode, 0);
		switch (ksym) {
		  case XK_Left:
		  case XK_Right:
		  case XK_Up:
		  case XK_Down:
		  case XK_Home:
		  case XK_R7:	/* Home key on Sun Type-4 keyboard */
		  case XK_End:
		  case XK_R13:	/* End key on Sun Type-4 keyboard */
		  case XK_R9:	/* PgUp key */
		  case XK_R15:	/* PgDn key */
		    buf_length = 0;
		    break;
		  default:
#ifdef OW_I18N
DoLookup:
		    ksym = NoSymbol;
		    if (ic) { /* then keyPress case */
			buf_length = XmbLookupString(ic, ek, buffer_backup,
						BUFFERSIZE, &ksym, &ret_status);
			if (ret_status == XBufferOverflow) {
			    buffer_backup = (char *)xv_malloc(buf_length + 1);
			    buf_length = XmbLookupString(ic, ek, buffer_backup,
						buf_length, &ksym, &ret_status);
			}
		    }
		    else {
			buf_length = XLookupString(ek, buffer, BUFFERSIZE,
						   &ksym, compose_status);
			ret_status = (ksym == NoSymbol) ? XLookupNone :
			       ((buf_length > 1) ? XLookupBoth : XLookupKeySym);
		    }
		    if (ret_status == XLookupNone)
			goto Default;
#else /* OW_I18N */
		    buf_length = XLookupString(ek, buffer, BUFFERSIZE, &ksym,
					       compose_status);
#endif /* OW_I18N */
		    break;
		}
#if 0
	    }
#endif

	    if (event_type == KeyPress)
		event_set_down(event);
	    else
		event_set_up(event);

	    if (compose_status->chars_matched || old_chars_matched)
	        status = win_handle_compose(event, compose_status,
					    old_chars_matched);

	    /*
	     * If the event is a synthetic event (the event came from
	     * SendEvent request), and the key-pressed is not the help key
	     * (for wm); we should ignore it for security reasons.
	     */
	    if (((xevent->xany.send_event) &&
#ifdef OW_I18N
		(((XKeyEvent *)xevent)->keycode) &&
#endif
		 (ksym != XK_Help)) && !defeat_event_security) {
		goto Default;
	    }

	    /*
	     * Determine if this ksym is in the Keyboard Keysym set.  These
	     * are basically your Function, Shift, Ctrl, Meta, Keypad, etc
	     * keys.
	     */
	    keyboard_key = ((ksym & KEYBOARD_KYSM_MASK) == KEYBOARD_KYSM);

	    /* Set the modifier states */
	    SET_SHIFTS(event, ek->state, meta_modmask, alt_modmask);

	    /*
	     * Determine offsets into the semantic mapping tables.
	     */
	    if (event_ctrl_is_down(event))
		modifiers += 0x100;
	    if (event_meta_is_down(event))
		modifiers += 0x200;
	    if (event_alt_is_down(event))
		modifiers += 0x400;

	    /*
	     * Keep separate keysym and modifier offsets for menu 
	     * accelerators
	     */
	    acc_modifiers = modifiers;
	    acc_ksym = ksym;

	    /*
	     * Shift handling
	     */
	    if (event_shift_is_down(event) && keyboard_key)  {
		/*
		 * Add shift offset for menu accelerator lookup
		 */
		acc_modifiers += 0x800;

	        /*
	         * Add Shift offset only if a keyboard key is pressed (i.e.
	         * function keys, etc..). 
	         */
		modifiers += 0x800;
	    }

	    /*
	     * If the keysym is in the keyboard keysym set, check to see if
	     * it maps into an XView ie_code.  (eg. KEY_LEFT(5)...)
	     */
#ifdef OW_I18N
	    key_value = (unsigned char) buffer_backup[0];
#else
	    key_value = (unsigned char) buffer[0];
#endif
	    if (keyboard_key)
		key_value = ((key_map[(int) ksym & 0xFF] == ksym) ||
			     (!key_map[(int) ksym & 0xFF])) ? key_value :
		              key_map[(int) ksym & 0xFF];
	    event_set_id(event, key_value);

	    /*
	     * The semantic table only wants to see a shifted key when the
	     * shift key is down, not when it is caused by the lock key.  So
	     * if the lock key is down and not the shift key, we unshift the
	     * ksym and then do our semantic lookup.
	     */
	    sem_ksym = ksym;
	    if ((ek->state & LockMask) && !(ek->state & ShiftMask) &&
		(ksym >= 'A' && ksym <= 'Z'))
		sem_ksym = ksym | 0x20;

	    /*
	     * Look up in a semantic table to see if the event has an XView
	     * semantic event mapping (eg. ACTION_OPEN).  There is one table
	     * for keyboard keysysm and one for ascii keysyms.
	     */
	    if (keyboard_key)
		sem_action = key_sem_map[(sem_ksym & 0xFF) + modifiers] |
		    XVIEW_SEMANTIC;
	    else
		sem_action = ascii_sem_map[(sem_ksym & 0xFF) + modifiers] |
		    XVIEW_SEMANTIC;

	    /*
	     * If the keypress is modified by Meta alone, and there is no
	     * semantic action defined, then consider the event a Window
	     * Level Accelerator.
	     */
	    if (event_is_down(event) &&
	        (modifiers & 0xF00) == 0x200 && !event_shift_is_down(event) &&
			sem_action == ACTION_NULL_EVENT)
		sem_action = ACTION_ACCELERATOR;

	    /* ACC_XVIEW */
	    /*
	     * Check if this event is a global/menu accelerator
	     * This overrides window accelerators as well as semantic actions
	     */
	    acc_map = (u_char *) xv_get(server_public, SERVER_ACCELERATOR_MAP);
	    if (acc_map && acc_map[(acc_ksym & 0xFF) + acc_modifiers])  {
		XEvent		*xevent = event_xevent(event);
                XKeyEvent	*ek = (XKeyEvent *) xevent;
		Frame		frame;
		Frame_menu_accelerator *menu_accelerator;

		/*
		 * Check if this menu accelerator exists for the frame
		 * containing this window
		 */
		frame = xv_get(event_window(event), WIN_FRAME);
                menu_accelerator = (Frame_menu_accelerator *) xv_get(frame,
                                        FRAME_MENU_X_ACCELERATOR, 
					ek->keycode, ek->state, NoSymbol);
		/*
		 * Accelerator exists - set semantic action
		 */
                if (menu_accelerator) {
		        sem_action = ACTION_ACCELERATOR;
                }
	    }
	    /* ACC_XVIEW */

	    if (event_is_down(event) &&
	      (key_value < VKEY_FIRSTSHIFT || key_value > VKEY_LASTSHIFT)) {
		/* Non-shift keypress event */
		/* -- Priority 1: next key is quoted */
		if (quote_next_key) {
		    sem_action = ACTION_NULL_EVENT;
		    quote_next_key = FALSE;
		}
		/* -- Priority 2: ResumeMouseless */
		if (sem_action == ACTION_RESUME_MOUSELESS) {
		    suspend_mouseless = FALSE;
		    *pwindow = 0;
		    return (TRUE);
		}
		/* -- Priority 3: mouseless is suspended */
		if (suspend_mouseless)
		    sem_action = ACTION_NULL_EVENT;
		/* -- Priority 4: QuoteNextKey or SuspendMouseless */
		if (sem_action == ACTION_QUOTE_NEXT_KEY) {
		    quote_next_key = TRUE;
		    *pwindow = 0;
		    return (TRUE);
		} else if (sem_action == ACTION_SUSPEND_MOUSELESS) {
		    suspend_mouseless = TRUE;
		    *pwindow = 0;
		    return (TRUE);
		}
	    }
	    if (!status ||
#ifdef OW_I18N
		((ret_status == XLookupNone) && (sem_action == ACTION_NULL_EVENT))) {
#else
		((ksym == NoSymbol) && (sem_action == ACTION_NULL_EVENT))) {
#endif
		*pwindow = 0;
		return (TRUE);
	    }
	    /*
	     * Make sure the keystroke is sent to the appropriate window.  In
	     * the X input model, keystrokes are sent to the outmost (leaf)
	     * window even it is not the focus window.  To correct this
	     * behavior, we search up the tree for a parent that has the
	     * focus and redirect the keystroke to it.
	     */
	    if (!xv_has_focus(info)) {
		int             found_focuswindow = FALSE;
		Xv_object       dummy_window = window;

		while (window = xv_get(window, WIN_PARENT)) {
		    Xv_Drawable_info	*draw_info;
		    DRAWABLE_INFO_MACRO(window, draw_info);
		    if (xv_has_focus(draw_info)) {
			found_focuswindow = TRUE;
			event_set_window(event, window);
			break;
		    }
		}
		if ((!found_focuswindow) && (sem_action == ACTION_HELP)) {
		    Inputmask      *im;

		    window = dummy_window;

		    for (;;) {
			if (!window) {
			    *pwindow = 0;
			    return (TRUE);
			}
			im = (Inputmask *) xv_get(window, WIN_INPUT_MASK);
			if ((im) && (im->im_flags & (IM_ASCII | IM_NEGASCII))) {
			    event_set_window(event, window);
			    break;
			}
			window = xv_get(window, WIN_PARENT);
		    }
	        /* If we didn't find the focus window, send it to its original
		 * destination.  The focus might have been lost, but due
		 * to a grab, we still get the key events. 
		 */
		} else if (!found_focuswindow) 
		    window = dummy_window;
	    }
	    server_set_timestamp(server_public, &event->ie_time, ek->time);
	    server_set_seln_function_pending(server_public,
					     ek->state & sel_modmask);
	    event_set_x(event, ek->x);
	    event_set_y(event, ek->y);

	    /*
	     * If more than one character was returned tell the client that
	     * there is a string in event_string().  Can use
	     * event_is_string() to check.
	     */
#ifdef OW_I18N
	    /*
	     * only when buf_length>1 then put the string in ie_string,
	     * else ie_code suffices to store it
	     */
	    if ((buf_length > 1) &&
		((ret_status == XLookupBoth) || (ret_status == XLookupChars)))
		event->ie_string = buffer_backup;
	    else
		event->ie_string = NULL;
#else
	    event_set_string(event, (buf_length > 1) ? buffer : NULL);
#endif
	    event_set_action(event, sem_action);

	    if ((sem_action == ACTION_PASTE || sem_action == ACTION_CUT) &&
		(event_is_down(event))) 
		win_handle_quick_selection(info, event);

	    if (check_lang_mode(server_public, display, event)) {
		goto Default;
	    }
	    break;
	}

      case ButtonPress:
      case ButtonRelease:{
	    int             button, action;
	    XButtonEvent   *e = (XButtonEvent *) xevent;
	    Window_info    *win;

	    /*
	     * If the event is mouse-press and it is a synthetic event ( the
	     * event came from SendEvent request ), we should ignore it for
	     * security reasons.
	     */
	    if (xevent->xany.send_event && !defeat_event_security)
		goto Default;

	    server_set_timestamp(server_public, &event->ie_time, e->time);
	    temp = e->state;
	    server_set_seln_function_pending(server_public,
					     e->state & sel_modmask);
	    event_set_x(event, e->x);
	    event_set_y(event, e->y);

	    switch (e->button) {
	      case Button3:
		button = MS_RIGHT;
		action = ACTION_MENU;
		break;
	      case Button2:
		/*
		 * if mouse chording is on simulate MENU by chording SELECT
		 * and ADJUST.
		 */
		if (chord) {
		    if (chording(display, e, chording_timeout)) {
			button = MS_RIGHT;
			action = ACTION_MENU;
			temp = (temp | Button3Mask) & ~Button1Mask;
			menu_flag = 1;
		    } else {
			button = MS_MIDDLE;
			action = ACTION_ADJUST;
			menu_flag = 0;
		    }
		} else {
		    /*
		     * OL says on two button mice, the right button is the
		     * menu button.
		     */
		    if (nbuttons == 2) {
			button = MS_RIGHT;
			action = ACTION_MENU;
			temp = (temp | Button3Mask) & ~Button2Mask;
		    } else {
			button = MS_MIDDLE;
			action = ACTION_ADJUST;
		    }
		}

		break;
	      case Button1:
		if (chord) {
		    if (chording(display, e, chording_timeout)) {
			button = MS_RIGHT;
			action = ACTION_MENU;
			temp = (temp | Button2Mask) & ~Button1Mask;
			menu_flag = 1;
		    } else {
			button = MS_LEFT;
			action = ACTION_SELECT;
			menu_flag = 0;
		    }
		} else {
		    if ((but2_mod & e->state) &&
			((nbuttons == 2) || (nbuttons == 1))) {
			button = MS_MIDDLE;
			action = ACTION_ADJUST;
			temp = (temp | Button2Mask) & ~Button1Mask;
		    } else if ((but3_mod & e->state) && (nbuttons == 1)) {
			button = MS_RIGHT;
			action = ACTION_MENU;
			temp = (temp | Button3Mask) & ~Button1Mask;
		    } else {
			button = MS_LEFT;
			action = ACTION_SELECT;
		    }
		}
		break;

	      default:
		goto Default;
	    }
	    SET_SHIFTS(event, temp, meta_modmask, alt_modmask);
	    event_set_id(event, button);
	    event_set_action(event, action);
	    if (event_type == ButtonPress) {
		event_set_down(event);
	    } else {
		event_set_up(event);
	    }

	    win = WIN_PRIVATE(event_window(event));
	    /*
	     * For click to type, if pressed SELECT, window does not appear
	     * to have input focus, window appears to be capable of accepting
	     * focus (i.e. accepts KBD_USE), and no selection function is
	     * pending, set input focus to this window.
	     */
	    if (action == ACTION_SELECT &&
		event_type == ButtonPress &&
		!xv_has_focus(info) &&
		!xv_no_focus(info) &&
		(win->xmask & FocusChangeMask) &&
		!server_get_seln_function_pending(server_public)) {
		(void) win_set_kbd_focus(window, xv_xid(info));
	    }
	    if (server_get_seln_function_pending(server_public))
		win_handle_quick_selection(info, event);
	    break;
	}

      case MotionNotify:{
	    XMotionEvent  *e = (XMotionEvent *)xevent;

            if((int)xv_get(window,WIN_COLLAPSE_MOTION_EVENTS)){
	       /* collapse motion events already in queue */

	       XEvent		new_event;

               while(QLength(display)) {
                 XNextEvent(display, &new_event);
                 if(new_event.type == MotionNotify) {
		    *xevent = new_event;
	            e = (XMotionEvent *)xevent;
                 } else {
                   XPutBackEvent(display, &new_event);
                   break;
                 }
              }
	    }

	    server_set_timestamp(server_public,
				&event->ie_time,e->time);

	    event_set_x(event, e->x);
	    event_set_y(event, e->y);

	    temp = e->state;
	    if (menu_flag)
		temp = (temp | Button3Mask);
	    SET_SHIFTS(event, temp, meta_modmask, alt_modmask);

	    if (event_button_is_down(event)) {
		if (nbuttons == 2) {
		    if ((but3_mod & temp) && (temp & Button1Mask))
			temp = (temp | Button2Mask) & ~Button1Mask;
		    else if (temp & Button2Mask)
			temp = (temp | Button3Mask) & ~Button2Mask;
		} else if (nbuttons == 1) {	/* assume it is button 1 */
		    if (but3_mod & temp)
			temp = (temp | Button3Mask) & ~Button1Mask;
		    else if (but2_mod & temp)
			temp = (temp | Button2Mask) & ~Button1Mask;
		}
		event_set_id(event, LOC_DRAG);
		event_set_down(event);
		if (server_get_seln_function_pending(server_public))
		    win_handle_quick_selection(info, event);
	    } else {
		event_set_id(event, LOC_MOVE);
		event_set_up(event);
	    }
	    SET_SHIFTS(event, temp, meta_modmask, alt_modmask);
	    break;
	}

      case EnterNotify:
      case LeaveNotify:{
	    XCrossingEvent *e = (XCrossingEvent *) xevent;
	    Xv_Drawable_info *info;

	    server_set_timestamp(server_public, &event->ie_time, e->time);

	    event_set_down(event);
	    temp = e->state;
	    SET_SHIFTS(event, temp, meta_modmask, alt_modmask);
	    server_set_seln_function_pending(server_public,
					     e->state & sel_modmask);
	    event_set_x(event, e->x);
	    event_set_y(event, e->y);
	    if (event_type == EnterNotify) {
		event_set_id(event, LOC_WINENTER);
		DRAWABLE_INFO_MACRO(event_window(event), info);
		pointer_window_xid = xv_xid(info);
	    } else {
		event_set_id(event, LOC_WINEXIT);
	    }
	    break;
	}

      case ConfigureNotify:{
	    Rect            temp_rect1;

	    temp_rect1.r_width = xevent->xconfigure.width;
	    temp_rect1.r_height = xevent->xconfigure.height;
	    temp_rect1.r_left = xevent->xconfigure.x;
	    temp_rect1.r_top = xevent->xconfigure.y;

	    if ((Bool) xv_get(window, WIN_TOP_LEVEL) == TRUE)
		window_update_cache_rect(window, &temp_rect1);

	    event_set_id(event, WIN_RESIZE);
	    break;
	}
	/*
	 * BUG ALERT:to this correctly need tos store XID of parent instead
	 * of Window handle (or both)
	 */
      case ReparentNotify:{
	    XReparentEvent *e = (XReparentEvent *) xevent;
	    Xv_Window       parent;

	    if (parent = win_data(display, e->parent))
		window_set_parent(window, parent);

	    event_set_id(event, WIN_REPARENT_NOTIFY);
	    event_set_x(event, e->x);
	    event_set_y(event, e->y);
	}
	break;

      case MapNotify:
	if ((Bool) xv_get(window, WIN_TOP_LEVEL) &&
	    !(Bool) xv_get(window, WIN_TOP_LEVEL_NO_DECOR)) {
	    event_set_id(event, WIN_MAP_NOTIFY);  /* id must be set before action */
	    event_set_action(event, ACTION_OPEN);
        }
	else
	    event_set_id(event, WIN_MAP_NOTIFY);
	break;

      case UnmapNotify:
	if ((Bool) xv_get(window, WIN_TOP_LEVEL) &&
	    !(Bool) xv_get(window, WIN_TOP_LEVEL_NO_DECOR)) {
	    event_set_id(event, WIN_UNMAP_NOTIFY);  /* id must be set before action */
	    event_set_action(event, ACTION_CLOSE);
        }
	else
	    event_set_id(event, WIN_UNMAP_NOTIFY);
	break;

	/*
	 * BUG ALERT: this is not exactly correct should really translate
	 * GraphicsExpose to Expose events to be exactly correct
	 */
      case GraphicsExpose:
      case Expose:{
	    XExposeEvent   *e = (XExposeEvent *) xevent;
	    Window_info    *win = WIN_PRIVATE(window);

	    if (win_do_expose_event(display, event, e, &window,
				    win->collapse_exposures)) {
		*pwindow = 0;
		return (1);
	    }
	    if (event_type == Expose)
		event_set_id(event, WIN_REPAINT);
	    else
		event_set_id(event, WIN_GRAPHICS_EXPOSE);
	    break;
	}

      case NoExpose:
	event_set_id(event, WIN_NO_EXPOSE);
	break;

      case SelectionClear:
		event_set_id(event, SEL_CLEAR);
		break;
      case SelectionRequest:
		event_set_id(event, SEL_REQUEST);
		break;
      case SelectionNotify:
		event_set_id(event, SEL_NOTIFY);
		break;
      case ClientMessage:{
	    XClientMessageEvent *cme = (XClientMessageEvent *) xevent;

	    if (process_clientmessage_events(window, cme, event)) {
		*pwindow = 0;
		return (1);
	    }
	    break;
	}

      case PropertyNotify:{
	    XPropertyEvent *pne = (XPropertyEvent *) xevent;

	    if (!xv_sel_handle_property_notify(pne)) {
		if (process_property_events(window, pne, event)) {
		    *pwindow = 0;
		    return (1);
		}
	    }
	    break;
	}

      case FocusIn:{
	    XFocusChangeEvent *fce = (XFocusChangeEvent *) xevent;

	    if (fce->detail == NotifyAncestor ||
		fce->detail == NotifyInferior ||
		fce->detail == NotifyNonlinear) {
		if (xv_get(xv_server(info), SERVER_JOURNALLING))
		    xv_set(xv_server(info), SERVER_JOURNAL_SYNC_EVENT, 1, NULL);
		xv_set_has_focus(info, TRUE);
		event_set_id(event, KBD_USE);
	    } else {
		/*
		 * BUG: We are dropping Notify{Virtual, NonlinearVirt, pntr,
		 * etc} on the floor.
		 */
		*pwindow = 0;
		return (1);
	    }
	    break;
	}

      case FocusOut:{
	    XFocusChangeEvent  *fce = (XFocusChangeEvent *) xevent;
	    Frame		frame = xv_get(event_window(event), WIN_FRAME);

		/* On FocusOut events turn off the compose LED */
	    if (xv_get(frame, FRAME_COMPOSE_STATE)) {
	        xv_set((Frame)xv_get(event_window(event), WIN_FRAME),
				FRAME_COMPOSE_STATE, False,
				NULL);
		compose_status->chars_matched = 0;
	    }

	    if (fce->detail == NotifyAncestor ||
		fce->detail == NotifyInferior ||
		fce->detail == NotifyNonlinear) {
		xv_set_has_focus(info, FALSE);
		event_set_id(event, KBD_DONE);
	    } else {
		*pwindow = 0;
		return (1);
	    }
	    break;
	}
      case KeymapNotify:
	event_set_id(event, KBD_MAP);
	break;
      case VisibilityNotify:{
	    XVisibilityEvent *e = (XVisibilityEvent *) xevent;
	    event_set_id(event, WIN_VISIBILITY_NOTIFY);
	    event_set_flags(event, e->state);	/* VisibilityUnobscured,
						 * VisibilityPartiallyObscured
						 * VisibilityObscured
						 */
	    break;
	}
      case GravityNotify:{
	    XGravityEvent  *e = (XGravityEvent *) xevent;
	    event_set_id(event, WIN_GRAVITY_NOTIFY);
	    event_set_x(event, e->x);
	    event_set_y(event, e->y);
	    break;
	}
      case CirculateNotify:{
	    XCirculateEvent *e = (XCirculateEvent *) xevent;
	    event_set_id(event, WIN_CIRCULATE_NOTIFY);
	    event_set_flags(event, e->place);	/* PlaceOnTop or
						 * PlaceOnButton */
	    break;
	}
      case ColormapNotify:
	event_set_id(event, WIN_COLORMAP_NOTIFY);
	/*
	 * BUG: This needs support macros to allow user to get at the
	 * colormap, new and state fields in the xevent.
	 */
	break;
	/* Events a window manger (not toolkit) would be interested in.   */
      case CreateNotify:
	event_set_id(event, WIN_CREATE_NOTIFY);
	break;
      case DestroyNotify:
	event_set_id(event, WIN_DESTROY_NOTIFY);
	break;
      case MapRequest:
	event_set_id(event, WIN_MAP_REQUEST);
	break;
      case ResizeRequest:
	event_set_id(event, WIN_RESIZE_REQUEST);
	break;
      case ConfigureRequest:
	event_set_id(event, WIN_CONFIGURE_REQUEST);
	break;
      case CirculateRequest:
	event_set_id(event, WIN_CIRCULATE_REQUEST);
	break;

	/*
	 * If we get an event that is not defined in the X protocol, assume
	 * it is an event generated from a server extension.  If an extension
	 * proc has been set on this server, we will call it back with the
	 * display, extension event, and a window object that is possibly
	 * NULL.
	 */
      default:{
	    /*
	     * I would like to cache the extensionProc, but then we run into
	     * the problem where the extensionProc is sporatically changed by
	     * the programmer.
	     */
	    void (*extensionProc)() = (void (*)()) xv_get(server_public,
						     SERVER_EXTENSION_PROC);
	    if (extensionProc)
		(*extensionProc) (display, xevent, window);
	}
Default:
	*pwindow = 0;
	return (1);
    }

    *pwindow = window;
    return (0);
}

/* read the input and post to the proper window */
/* ARGSUSED */
Xv_private      Notify_value
xv_input_pending(dpy, fd)
    Display         *dpy;
    int              fd;                /* Not Used */
{
    Event           event;
    int             events_handled = QLength(dpy);
    Xv_object       window;
    XEvent          xevent;

                /* If there are no events on Xlib's internal queue, then
                 * flush and go to the wire for them.
                 */
    if (!events_handled) {
	events_handled += XPending(dpy);
	/* If we still don't have any events on the queue, then the connection
	 * might have been dropped.  To test for this, we ping the server.
	 */
	if (!events_handled)
	    /* XSync will not return if the connection has been lost. */
	    XSync(dpy, False);
    }

    while (QLength(dpy)) {
	window = xview_x_input_readevent(dpy, &event, NULL, FALSE, FALSE,
				         NULL, &xevent);
	if (window)
	    switch (event_id(&event)) {

	      case WIN_GRAPHICS_EXPOSE:
	      case WIN_REPAINT:{
		    if ((!xv_get(window, WIN_X_PAINT_WINDOW)) &&
			(!xv_get(window, WIN_NO_CLIPPING))) {
			/*
			 * Force the clipping list for the window to be the
			 * damage list while the client is processing the
			 * WIN_REPAINT, then clear the clipping list back to
			 * its normal state.
			 */
			Rectlist       *rl;

			rl = win_get_damage(window);
			win_set_clip(window, rl);
			(void) win_post_event(window, &event,
			   WIN_IS_IN_LOOP ? NOTIFY_IMMEDIATE : NOTIFY_SAFE);
			win_set_clip(window, RECTLIST_NULL);
		    } else {
			(void) win_post_event(window, &event,
			   WIN_IS_IN_LOOP ? NOTIFY_IMMEDIATE : NOTIFY_SAFE);
		    }
		    win_clear_damage(window);
		    break;
		}
	      case MS_LEFT:{
		    Window_info    *win = WIN_PRIVATE(window);

		    /*
		     * Since we have a synchronous grab on the select button,
		     * after we set the focus, we need to release the events
		     * with XAllowEvents().
		     */
		    window_release_selectbutton(window, &event);

		    /*
		     * Do not post MS_LEFT events if the user hasn't asked
		     * for them.  XView needs them for click-to-type and we
		     * get them reguardless of whether they are in the event
		     * mask beacuse of the passive button grab.
		     */
		    if ((win->xmask&ButtonPressMask && event_is_down(&event)) ||
		        (win->xmask & ButtonReleaseMask && event_is_up(&event)))
			(void) win_post_event(window, &event,
			   WIN_IS_IN_LOOP ? NOTIFY_IMMEDIATE : NOTIFY_SAFE);

		    break;
		}

	      default:
		if (event_action(&event) == ACTION_ACCELERATOR) {
		    /* ACC_XVIEW */
		    /*
		     * First check if the event is a menu accelerator.
		     * If it is, call the proper notify proc.
		     * If not, check if it is a window accelerator.
		     * If it is, call the proper notify proc.
		     * If not, nullify the semantic action and post the event
		     */
		    if (!win_handle_menu_accel(&event))  {
			if (!win_handle_window_accel(&event))  {
			    event_set_action(&event, ACTION_NULL_EVENT);
			    win_post_event(window, &event,
			       WIN_IS_IN_LOOP ? NOTIFY_IMMEDIATE : NOTIFY_SAFE);
			}
		    }
		    /* ACC_XVIEW */
		} else {
		    win_post_event(window, &event,
			   WIN_IS_IN_LOOP ? NOTIFY_IMMEDIATE : NOTIFY_SAFE);
		}
		break;
	    }

	/* If we've handled all the events on the wire and we've not handled */
	/* more than 25 events on this go-round (as we don't want to deprive */
	/* other input src's [eg. another server]), see if any more events   */
	/* have arrived.					             */
	if (!QLength(dpy) && (events_handled < 25))
	    events_handled += XEventsQueued(dpy, QueuedAfterFlush);
    };
    return NOTIFY_DONE;
}

/* ACC_XVIEW */
/*
 * Returns TRUE if the window accelerator was found/handled
 */
Xv_private int
win_handle_window_accel(event)
Event		*event;
{
    XEvent	*xevent = event_xevent(event);
    XKeyEvent	*ek = (XKeyEvent *) xevent;
    Frame	frame;
    Frame_accelerator *accelerator;
    KeySym          keysym;

    keysym = XKeycodeToKeysym(ek->display, ek->keycode, 0);
    frame = xv_get(event_window(event), WIN_FRAME);
    accelerator = (Frame_accelerator *) xv_get(frame,
                            FRAME_ACCELERATOR, event_id(event), keysym);
    if (accelerator) {
        accelerator->notify_proc(accelerator->data, event);
        return(TRUE);
    }

    return(FALSE);
}

/*
 * Returns TRUE if the menu accelerator was found/handled
 * The keysym passed in is unmodified i.e. entry 0 in the 
 * keysym list
 */
Xv_private int
win_handle_menu_accel(event)
Event		*event;
{
    XEvent	*xevent = event_xevent(event);
    XKeyEvent	*ek = (XKeyEvent *) xevent;
    Frame	frame;
    Frame_menu_accelerator *menu_accelerator;

    frame = xv_get(event_window(event), WIN_FRAME);
    menu_accelerator = (Frame_menu_accelerator *) xv_get(frame,
                    FRAME_MENU_X_ACCELERATOR, 
			ek->keycode, ek->state, NoSymbol);

    if (menu_accelerator) {
        /*
         * We consume both the up/down event but the notify
         * proc is called only for the down
         */
        if (event_is_down(event))  {
            menu_accelerator->notify_proc(menu_accelerator->data, event);
        }
        return(TRUE);
    }
    return (FALSE);
}
/* ACC_XVIEW */

Xv_private void
win_get_cmdline_option(window, str, appl_cmdline)
    Xv_object		 window;
    char		*str;
    char		*appl_cmdline;
{
    Window		 root = 0,
    			 parent,
			*children;
    unsigned int    	 nchildren;
    int             	 temp,
    			 icon_x,
			 icon_y;
    Rect		*rect;
    char		 iconic[6];
    Icon		 icon;
    XWindowAttributes	 xwin_attr;
    Xv_Drawable_info 	*icon_info,
    			*info;

    DRAWABLE_INFO_MACRO(window, info);
    rect = (Rect *) xv_get(window, WIN_RECT);
    if (xv_get(window, XV_SHOW)) {
	temp = XQueryTree(xv_display(info), xv_xid(info), &root, &parent,
			  &children, &nchildren);
	if (temp) {
	    XGetWindowAttributes(xv_display(info), parent, &xwin_attr);
	    if (nchildren)
	      XFree((char *)children);
	}
    } else
	XGetWindowAttributes(xv_display(info), xv_xid(info), &xwin_attr);

    rect->r_left = xwin_attr.x;
    rect->r_top = xwin_attr.y;

    icon = (Icon) xv_get(window, FRAME_ICON);
    DRAWABLE_INFO_MACRO(icon, icon_info);
    if (!root)
	root = (int) xv_get(xv_root(icon_info), XV_XID);

    win_translate_xy_internal(xv_display(info), xv_xid(icon_info), root, 0, 0,
			      &icon_x, &icon_y);

    iconic[0] = '\0';
    if (xv_get(window, FRAME_CLOSED))
	sprintf(iconic, " -Wi");
    else
	sprintf(iconic, " +Wi");

    /*
     * Create the string with:
     *	APP_NAME -Wp WINDOW_X WINDOW_Y -Ws WINDOW_WIDTH WINDOW_HEIGHT 
     *	-WP ICON_X ICON_Y
     */
    sprintf(str,
	    "%s -Wp %d %d -Ws %d %d -WP %d %d %s",
	    xv_app_name,
	    rect->r_left, rect->r_top, 
	    rect->r_width, rect->r_height,
	    icon_x, icon_y, iconic);
    /*
     * Append to str all other XView cmd line options
     */
    xv_get_cmdline_str(str);

    /*
     * add any application specific cmdline args
     */
    if (appl_cmdline)  {
	/* put in a space to separate from rest of string */
	strcat(str, " ");
	strcat(str, appl_cmdline);
    }
}


Xv_private void
win_set_wm_command_prop(window, argv, appl_cmdline_argv, appl_cmdline_argc)
    Xv_object       window;
    char           **argv;
    char	    **appl_cmdline_argv;
    int		    appl_cmdline_argc;
{
    Xv_Drawable_info *info;
    Window          root = 0;
    Window          parent, *children;
    unsigned int    nchildren;
    int             temp;
    XWindowAttributes xwin_attr;
    Rect           *rect;
    char            icon_x_str[50], icon_y_str[50];
    char            window_width[50], window_height[50];
    char            window_x[50], window_y[50];
    Icon            icon;
    Xv_Drawable_info *icon_info;
    int             icon_x, icon_y;
    int			argc = 0;

    argv[argc++] = xv_app_name;

    DRAWABLE_INFO_MACRO(window, info);
    rect = (Rect *) xv_get(window, WIN_RECT);

    if (xv_get(window, XV_SHOW)) {
	temp = XQueryTree(xv_display(info), xv_xid(info), &root, &parent,
			  &children, &nchildren);
	if (temp) {
	    XGetWindowAttributes(xv_display(info), parent, &xwin_attr);
	    if (nchildren)
	      XFree((char *)children);
	}
    } else
	XGetWindowAttributes(xv_display(info), xv_xid(info), &xwin_attr);

    /*
     * Window position
     */
    window_x[0] = window_y[0] = '\0';
    sprintf(window_x, "%d", xwin_attr.x);
    sprintf(window_y, "%d", xwin_attr.y);
    argv[argc++] = "-Wp";
    argv[argc++] = window_x;
    argv[argc++] = window_y;

    /*
     * Put size in size string, if valid rect returned
     */
    if (rect)  {
        window_width[0] = window_height[0] = '\0';
	sprintf(window_width, "%d", rect->r_width);
	sprintf(window_height, "%d", rect->r_height);
	argv[argc++] = "-Ws";
	argv[argc++] = window_width;
	argv[argc++] = window_height;
    }

    icon = (Icon) xv_get(window, FRAME_ICON);

    /*
     * Put icon position in icon position string if frame icon present
     */
    if (icon)  {
        DRAWABLE_INFO_MACRO(icon, icon_info);
        if (!root)
	    root = (int) xv_get(xv_root(icon_info), XV_XID);

        win_translate_xy_internal(xv_display(info), xv_xid(icon_info), root, 0, 0,
			      &icon_x, &icon_y);

        icon_x_str[0] = icon_y_str[0] = '\0';
	sprintf(icon_x_str, "%d", icon_x);
	sprintf(icon_y_str, "%d", icon_y);

	argv[argc++] = "-WP";
	argv[argc++] = icon_x_str;
	argv[argc++] = icon_y_str;
    }

    if (xv_get(window, FRAME_CLOSED))  {
	argv[argc++] = "-Wi";
    }
    else  {
	argv[argc++] = "+Wi";
    }

    /*
     * Append to str all other XView cmd line options
     */
    xv_get_cmdline_argv(argv, &argc);

    if (appl_cmdline_argv)  {
	int	i = 0;

	for (i=0; i < appl_cmdline_argc; ++i)  {
	    argv[argc++] = appl_cmdline_argv[i];
	}
    }

    XSetCommand(xv_display(info), xv_xid(info), argv, argc);
}

static int
process_clientmessage_events(window, clientmessage, event)
    Xv_object       window;
    XClientMessageEvent *clientmessage;
    Event          *event;
{
    Xv_Drawable_info *info;
    Xv_opaque       server_public;
    Server_atom_type atom_type;
    int             result = 0;
    char           *string;
    Bool            keyboard_key;
    unsigned int    key_value;
    unsigned int   *key_map;
    static char    *ascii_sem_map, *key_sem_map;
    KeySym          ksym, sem_ksym;
    int             modifiers = 0;
    int             sem_action;
    static int      meta_modmask = 0;
    static int      alt_modmask = 0;

    DRAWABLE_INFO_MACRO(window, info);
    server_public = xv_server(info);
    atom_type = server_get_atom_type(server_public, clientmessage->message_type);

    switch (atom_type) {
      case SERVER_WM_DISMISS_TYPE:
	event_set_action(event, ACTION_DISMISS);
	break;
      case SERVER_DO_DRAG_MOVE_TYPE:
      case SERVER_DO_DRAG_COPY_TYPE:
      case SERVER_DO_DRAG_LOAD_TYPE:
	{
	    int             final_x, final_y;

	    /*
	     * the xy is in the sender's coordinate, need to translate it
	     */
	    (void) win_translate_xy_internal(xv_display(info),
					     (XID) clientmessage->data.l[3],
					     xv_xid(info),
					     (int) clientmessage->data.l[1],
					     (int) clientmessage->data.l[2],
					     &final_x, &final_y);
	    event_set_x(event, final_x);
	    event_set_y(event, final_y);

	    /*
	     * save off the clientmessage info into the window struct
	     */
	    window_set_client_message(window, clientmessage);

	    switch (atom_type) {
	      case SERVER_DO_DRAG_MOVE_TYPE:
		event_set_action(event, ACTION_DRAG_MOVE);
		break;
	      case SERVER_DO_DRAG_COPY_TYPE:
		event_set_action(event, ACTION_DRAG_COPY);
		break;
	      case SERVER_DO_DRAG_LOAD_TYPE:
		event_set_action(event, ACTION_DRAG_LOAD);
		break;
	    }
	}
	break;
      case SERVER_WM_DRAGDROP_TRIGGER_TYPE: {
	    int actual_x, actual_y, msg_x, msg_y;

	    /* Decode the x, y position in top level coord space */
	    msg_x = (clientmessage->data.l[2] >> 16) & 0xffff;
	    msg_y = clientmessage->data.l[2] & 0xffff;

	    (void) win_translate_xy_internal(xv_display(info),
					     xv_get(xv_root(info), XV_XID),
					     xv_xid(info), msg_x, msg_y,
					     &actual_x, &actual_y);
	    event_set_x(event, actual_x);
	    event_set_y(event, actual_y);

	    /* save off the clientmessage info into the window struct */
	    window_set_client_message(window, clientmessage);

	    /* Set the time of the preview event */
	    event->ie_time.tv_sec =
		((unsigned long) clientmessage->data.l[1]) / 1000;
	    event->ie_time.tv_usec =
		(((unsigned long) clientmessage->data.l[1]) % 1000) * 1000;

	    if (clientmessage->data.l[4] & DND_MOVE_FLAG)
		event_set_action(event, ACTION_DRAG_MOVE);
	    else
		event_set_action(event, ACTION_DRAG_COPY);

		/* If the event has been forwarded from some other site,
		 * inform the client.
		 */
	    if (clientmessage->data.l[4] & DND_FORWARDED_FLAG)
		event_set_flags(event, DND_FORWARDED);
	}
	break;
      case SERVER_WM_DRAGDROP_PREVIEW_TYPE: {
	    int actual_x, actual_y, msg_x, msg_y;

	    /* Decode the x, y position in top level coord space */
	    msg_x = (clientmessage->data.l[2] >> 16) & 0xffff;
	    msg_y = clientmessage->data.l[2] & 0xffff;

	    (void) win_translate_xy_internal(xv_display(info),
					     xv_get(xv_root(info), XV_XID),
					     xv_xid(info), msg_x, msg_y,
					     &actual_x, &actual_y);
	    event_set_x(event, actual_x);
	    event_set_y(event, actual_y);

	    /* save off the clientmessage info into the window struct */
	    window_set_client_message(window, clientmessage);

	    /* Set the time of the preview event */
	    event->ie_time.tv_sec =
		((unsigned long) clientmessage->data.l[1]) / 1000;
	    event->ie_time.tv_usec =
		(((unsigned long) clientmessage->data.l[1]) % 1000) * 1000;

	    switch (clientmessage->data.l[0]) {
	      case EnterNotify:
		event_set_id(event, LOC_WINENTER);
		break;
	      case LeaveNotify:
		event_set_id(event, LOC_WINEXIT);
		break;
	      case MotionNotify:
		event_set_id(event, LOC_DRAG);
		break;
	      default:
		xv_error(event_window(event), ERROR_STRING, DND_ERROR_CODE, NULL);
	    }
	    event_set_action(event, ACTION_DRAG_PREVIEW);

		/* If the event has been forwarded from some other site,
		 * inform the client.
		 */
	    if (clientmessage->data.l[4] & DND_FORWARDED_FLAG)
		event_set_flags(event, DND_FORWARDED);
	}
	break;

      case SERVER_WM_PROTOCOLS_TYPE:{
	    switch (server_get_atom_type(server_public,
					 clientmessage->data.l[0])) {
	      case SERVER_WM_SAVE_YOURSELF_TYPE:
		    xv_destroy_save_yourself(window);
		    win_set_wm_command(window);
		    XFlush(xv_display(info));
		    break;
	      case SERVER_WM_DELETE_WINDOW_TYPE:
		if (xv_get(window, XV_OWNER) == xv_get(xv_screen(info),
						       XV_ROOT) &&
		    ((Xv_pkg *)xv_get(window, XV_TYPE) == FRAME))
		    xv_destroy_safe(window);
		else
		    event_set_action(event, ACTION_DISMISS);
		break;
	      case SERVER_WM_TAKE_FOCUS_TYPE:
		server_set_timestamp(server_public, &event->ie_time,
				     clientmessage->data.l[1]);
		event_set_action(event, ACTION_TAKE_FOCUS);
		break;
	    }
	    break;
	}
      default:
	/*
	 * Set the id to WIN_CLIENT_MESSAGE and the content of the gets
	 * stuffed into window struct
	 */

	if (clientmessage->message_type == xv_get(server_public, SERVER_ATOM,
						  "_OL_TRANSLATED_KEY")) {

	    /* Initialise an xevent  */

	    string = XKeysymToString((KeySym) (clientmessage->data.l[0]));
	    ksym = (KeySym) clientmessage->data.l[0];
	    keyboard_key = ((ksym & KEYBOARD_KYSM_MASK) == KEYBOARD_KYSM);
	    key_value = ksym;

	    key_map = (unsigned int *) xv_get(server_public, SERVER_XV_MAP);
	    key_sem_map = (char *) xv_get(server_public, SERVER_SEMANTIC_MAP);
	    ascii_sem_map = (char *) xv_get(server_public, SERVER_ASCII_MAP);

	    if (keyboard_key)
		key_value = ((key_map[(int) ksym & 0xFF] == ksym) ||
			     (!key_map[(int) ksym & 0xFF])) ? key_value :
		    key_map[(int) ksym & 0xFF];
	    event_set_id(event, key_value);

	    if ((int) (clientmessage->data.l[1]) == KeyPress)
		event_set_down(event);
	    else if ((int) (clientmessage->data.l[1]) == KeyRelease)
		event_set_up(event);

	    event->ie_win = window;
	    event->ie_string = string;

	    alt_modmask = (int) xv_get(server_public, SERVER_ALT_MOD_MASK);
	    meta_modmask = (int) xv_get(server_public, SERVER_META_MOD_MASK);

	    /* Get the Semantic Action part of the XView Event initialised */

	    if (clientmessage->data.l[2] & ControlMask)
		modifiers += 0x100;
	    if (clientmessage->data.l[2] & meta_modmask)
		modifiers += 0x200;
	    if (clientmessage->data.l[3] & alt_modmask)
		modifiers += 0x400;
	    if ((clientmessage->data.l[3] & ShiftMask) && (keyboard_key))
		modifiers += 0x800;


	    sem_ksym = ksym;
	    if ((clientmessage->data.l[2] & LockMask) &&
		!(clientmessage->data.l[2] & ShiftMask) &&
		(ksym >= 'A' && ksym <= 'Z'))
		sem_ksym = ksym | 0x20;

	    if (keyboard_key)
		sem_action = key_sem_map[(sem_ksym & 0xFF) + modifiers] | XVIEW_SEMANTIC;
	    else
		sem_action = ascii_sem_map[(sem_ksym & 0xFF) + modifiers] | XVIEW_SEMANTIC;
	    event_set_action(event, sem_action);

	    /* Initialise the xevent to NULL for now */

	    event->ie_xevent = NULL;

	} else {

	    event_set_id(event, WIN_CLIENT_MESSAGE);
	    window_set_client_message(window, clientmessage);

	}
    }
    return (result);
}

static int
process_property_events(window, property, event)
    Xv_object       window;
    XPropertyEvent *property;
    Event          *event;
{
    Xv_Drawable_info *info;
    Xv_opaque       server_public;
    Server_atom_type atom_type;

    DRAWABLE_INFO_MACRO(window, info);
    server_public = xv_server(info);
    atom_type = server_get_atom_type(server_public, property->atom);

    switch (atom_type) {
      case SERVER_WM_PIN_STATE_TYPE:
	return (process_wm_pushpin_state(window, property->atom, event));
      default:
	event_set_id(event, WIN_PROPERTY_NOTIFY);
    }
    return FALSE;
}


static int
process_wm_pushpin_state(window, atom, event)
    Xv_object       window;
    Atom            atom;
    Event          *event;
{
    Xv_Drawable_info *info;
    int             status;
    Atom            type;
    int             format;
    unsigned long   nitems;
    unsigned long   bytes;
    unsigned char  *prop;
    long           *pinstate;

    DRAWABLE_INFO_MACRO(window, info);
    status = XGetWindowProperty(xv_display(info), xv_xid(info), atom,
				0, 1, False, XA_INTEGER,
				&type, &format, &nitems, &bytes, &prop);
    if (status != Success)
	return 1;

    if (!prop)
	return 1;

    if (format != 32) {
	XFree((char *)prop);
	return 1;
    }
    pinstate = (long *) prop;
    switch (*pinstate) {
      case WMPushpinIsIn:
	event_set_action(event, ACTION_PININ);
	break;
      case WMPushpinIsOut:
	event_set_action(event, ACTION_PINOUT);
	break;
    }
    XFree((char *)prop);
    return 0;
}


Xv_private void
win_event_to_proc_with_ptr(window_public, event_type, sender_id, x, y)
    Xv_opaque       window_public;
    Atom            event_type;
    XID             sender_id;
    int             x, y;
{
    Xv_Drawable_info *info;
    XClientMessageEvent event_struct;

    DRAWABLE_INFO_MACRO(window_public, info);
    event_struct.type = ClientMessage;
    event_struct.message_type = event_type;

    event_struct.window = XV_DUMMY_WINDOW;	/* Put anything in here, the
						 * server will not use this */
    event_struct.format = 32;
    event_struct.data.l[0] = x;
    event_struct.data.l[1] = y;
    event_struct.data.l[2] = sender_id;
    XSendEvent(xv_display(info), PointerWindow, False, NoEventMask,
	       (XEvent *)&event_struct);
    XFlush(xv_display(info));
}


chording(display, bEvent, timeout)
    Display        *display;
    XButtonEvent   *bEvent;
    int             timeout;
{
    XEvent          xevent;

    /* XView does a passive grab on the SELECT button! */
    window_x_allow_events(display);

    return BlockForEvent(display, &xevent, timeout * 1000, GetButtonEvent,
			 (char *) bEvent);
}



/*
 * BlockForEvent
 * 
 * Scan the input queue for the specified event. If there aren't any events in
 * the queue, select() for them until a certain timeout period has elapsed.
 * Return value indicates whether the specified event  was seen.
 */
static int
BlockForEvent(display, xevent, usec, predicate, eventType)
    Display        *display;
    XEvent         *xevent;
    long            usec;
    int             (*predicate) ();
    char           *eventType;
{
    fd_set          rfds;
    int             result;
    struct timeval  timeout;
    struct timeval  starttime, curtime, diff1, diff2;
    extern int      errno;

    timeout.tv_sec = 0;
    timeout.tv_usec = usec;

    (void) gettimeofday(&starttime, NULL);
    XFlush(display);
    XSync(display, False);
    while (1) {
	/*
	 * Check for data on the connection.  Read it and scan it.
	 */
	if (XCheckIfEvent(display, xevent, predicate, eventType))
	    return (TRUE);

	/*
	 * We've drained the queue, so we must select for more.
	 */
	FD_ZERO(&rfds);
	FD_SET(ConnectionNumber(display), &rfds);

	result = select(ConnectionNumber(display) + 1, &rfds, NULL, NULL, &timeout);

	if (result == 0) {
	    /* we timed out without getting anything */
	    return FALSE;
	}
	/*
	 * Report errors. If we were interrupted (errno == EINTR), we simply
	 * continue around the loop. We scan the input queue again.
	 */
	if (result == -1 && errno != EINTR)
	    perror("Select");

	/*
	 * Either we got interrupted or the descriptor became ready. Compute
	 * the remaining time on the timeout.
	 */
	(void) gettimeofday(&curtime, NULL);
	tvdiff(&starttime, &curtime, &diff1);
	tvdiff(&diff1, &timeout, &diff2);
	timeout = diff2;
	starttime = curtime;
	if (timeout.tv_sec < 0)
	    return False;
    }
}



/*
 * Predicate function for XCheckIfEvent
 * 
 */
/* ARGSUSED */
static int
GetButtonEvent(display, xevent, args)
    Display        *display;
    XEvent         *xevent;
    char           *args;
{
    XButtonEvent    prevEvent, *newEvent;
    static int      mFlg;

    if (((xevent->type & 0177) != ButtonPress) &&
	((xevent->type & 0177) != ButtonRelease)) {
	mFlg = 0;
	return FALSE;
    }
    newEvent = (XButtonEvent *) xevent;

    XV_BCOPY((char *) args, (char *) &prevEvent, sizeof(XButtonEvent));

    switch (xevent->type) {
      case ButtonPress:
	if ((newEvent->button == prevEvent.button) || newEvent->button == 3) {
	    mFlg = 0;
	    return FALSE;
	}
	mFlg = 1;
	break;
      case ButtonRelease:
	if (mFlg) {
	    mFlg = 0;
	    return TRUE;
	}
	return FALSE;
    }
    return TRUE;
}




/* compute t2 - t1 and return the time value in diff */
static void
tvdiff(t1, t2, diff)
    struct timeval *t1, *t2, *diff;
{
    diff->tv_sec = t2->tv_sec - t1->tv_sec;
    diff->tv_usec = t2->tv_usec - t1->tv_usec;
    if (diff->tv_usec < 0) {
	diff->tv_sec -= 1;
	diff->tv_usec += 1000000;
    }
}


Bool
check_lang_mode(server, display, event)
    Xv_server       server;
    Display        *display;
    Event          *event;

{

    static short    lang_mode = 0;
    Atom     enter_lang_atom, exit_lang_atom;
    Atom     translate_key_atom;
    XClientMessageEvent xclientm_event;
    Window          xv_get_softkey_xid();
    static Window   sft_key_win;
    XKeyEvent      *keyevent;




    if (!event){
          /* KBD_DONE event. So get out of the languages mode */
          lang_mode = 0;
          return (1);
    }
    keyevent = (XKeyEvent *) event->ie_xevent;

    if (event_action(event) == ACTION_TRANSLATE) {

	sft_key_win = xv_get_softkey_xid(server, display);

	if (sft_key_win == None)/* There is no soft keys process running */
	    return (0);


	enter_lang_atom = xv_get(server, SERVER_ATOM, "_OL_ENTER_LANG_MODE");
	exit_lang_atom = xv_get(server, SERVER_ATOM, "_OL_EXIT_LANG_MODE");

	if (event_is_down(event)) {

	    lang_mode = 1;

	    /* Turn off Auto repeat for the LANG key */

	    /*
	     * keycontrol_values.key              =
	     * event->ie_xevent->xkey.keycode;
	     * keycontrol_values.auto_repeat_mode = AutoRepeatModeOff;
	     * XChangeKeyboardControl(display,KBKey | KBAutoRepeatMode,
	     * &keycontrol_values); XFlush(display);
	     */


	    /* Construct an Event */
	    xclientm_event.type = ClientMessage;
	    xclientm_event.window = sft_key_win;
	    xclientm_event.message_type = enter_lang_atom;
	    xclientm_event.format = 32;
	    XSendEvent(display, sft_key_win, False, 0, (XEvent *)&xclientm_event);
	    return (1);

	} else {

	    lang_mode = 0;

	    /* Restore AutoRepeat Default mode back to the key */
	    /*
	     * keycontrol_values.key              =
	     * event->ie_xevent->xkey.keycode;
	     * keycontrol_values.auto_repeat_mode = AutoRepeatModeDefault;
	     * 
	     * XChangeKeyboardControl(display,KBKey | KBAutoRepeatMode,
	     * &keycontrol_values); XFlush(display);
	     */

	    xclientm_event.type = ClientMessage;
	    xclientm_event.window = sft_key_win;
	    xclientm_event.message_type = exit_lang_atom;
	    xclientm_event.format = 32;
	    XSendEvent(display, sft_key_win, False, 0, (XEvent *)&xclientm_event);
	    return (1);
	}
    }
    if (lang_mode) {

	/* start sending keys to the virtual keyboard */

	if ((event->ie_code < 33) || (event->ie_code == 127))
	    return (0);		/* Do not send unwanted events */

	translate_key_atom = xv_get(server, SERVER_ATOM, "_OL_TRANSLATE_KEY");

	/* Construct an Event */

	xclientm_event.type = ClientMessage;
	xclientm_event.window = sft_key_win;
	xclientm_event.message_type = translate_key_atom;
	xclientm_event.format = 16;
	xclientm_event.data.l[0] = keyevent->keycode;
	xclientm_event.data.l[1] = keyevent->type;
	xclientm_event.data.l[2] = keyevent->state;
	XSendEvent(display, sft_key_win, False, 0, (XEvent *)&xclientm_event);
	return (1);

    }
    return (0);

}


/*
 * Get the Soft Key Labels win through the selection mecahanism the Soft key
 * window owns the selction "_OL_SOFT_KEYS_PROCESS" By querying the owner of
 * this selection , we get hold of the soft key window
 */


Xv_private      XID
xv_get_softkey_xid(server, display)
    Xv_server       server;
    Display        *display;

{
    Atom            sftk_process_atom;
    Window          seln_owner;

    sftk_process_atom = xv_get(server, SERVER_ATOM, "_OL_SOFT_KEYS_PROCESS");
    seln_owner = XGetSelectionOwner(display, sftk_process_atom);
    return (seln_owner);

}

static void
win_handle_quick_selection(info, event)
    Xv_Drawable_info 	*info;
    Event		*event;
{

    Atom		 key_type = xv_get(xv_server(info), SERVER_ATOM,
			                   (event_action(event) == ACTION_CUT ?
				           "MOVE" : "DUPLICATE"));
    Atom		 property = xv_get(xv_server(info), SERVER_ATOM,
					    "_SUN_QUICK_SELECTION_KEY_STATE");

    switch(event_action(event)) {
      case ACTION_PASTE:
      case ACTION_CUT:

          /* Always store the data on "property" on screen 0 of the display */

              XChangeProperty(xv_display(info), RootWindow(xv_display(info),0),
			      property, XA_ATOM, 32, PropModeReplace,
		              (unsigned char *) &key_type, 1);
	  break;
      case ACTION_SELECT:
      case ACTION_ADJUST:
      case ACTION_MENU:
      case LOC_DRAG:
	{
	  Atom		 notUsed;
	  int		 format;
	  unsigned long	 nitems,
	  		 bytes_after;
	  unsigned char	*data;

          /* Always get the data from "property" on screen 0 of the display */

	  if (XGetWindowProperty(xv_display(info),
				 RootWindow(xv_display(info),0),
				 property, 0L, 1, False, XA_ATOM,
				 &notUsed, &format, &nitems, &bytes_after,
				 &data) != Success)
	      return;
	  else {
	      /* key_type == DUPLICATE in this case */
	      if (key_type == *(Atom *)data)
		  event_set_quick_copy(event);
	      else
		  event_set_quick_move(event);
	      XFree((char *)data);
	  }
	  break;
	}
    }
}

Pkg_private int
win_handle_compose(event, c_status, last)
    Event		*event;
    XComposeStatus	*c_status;
    int			 last;
{
    Frame		 frame = xv_get(event_window(event), WIN_FRAME);
    int			 current = c_status->chars_matched;

    /* State table for the compose key. */
    /* current == 0:  Not in Compose
     * current == 1:  1 char matched 
     * current == 2:  2 char matched 
     * current == 3:  Accepted
     */
    /* Return True if we want to pass event on to application. 
     * return False if we want to eat this event.
     */

    if (last == 0 || last == 3) {
	if (current == 0 || current == 3)
	    return(True);
	else if (current == 1 || current == 2) {
	    xv_set(frame, FRAME_COMPOSE_STATE, True, NULL);
	    return(False);
	}
    } else if (last == 1) {
	if (current == 0) {
	    xv_set(frame, FRAME_COMPOSE_STATE, False, NULL);
	    return(False);
	} else if (current == 1 || current == 3)
	    return(False);
	else if (current == 2)
	    return(False);
    } else if (last == 2) {
	if (current == 0) {
	    xv_set(frame, FRAME_COMPOSE_STATE, False, NULL);
	    return(False);
	} else if (current == 1 || current == 2) {
	    return(False);
	} else if (current == 3) {
	    xv_set(frame, FRAME_COMPOSE_STATE, False, NULL);
	    return(True);
	}
    }
    return(False); /* Eat this event */
}

/* See the comment above.  martin-2.buck@student.uni-ulm.de */
#if 0
/*
 * win_translate_KP_keysym - This function is taken from the Xlib function
 * XTranslateKeySym(). It EXPECTS a keypad keysym and fills the provided
 * buffer with the corresponding string. The length of this string is
 * returned. Two restrictions to be aware of:
 * It EXPECTS the buffer to have space for 1 character.
 */

static int
win_translate_KP_keysym(keysym, buffer)
    KeySym    keysym;
    char     *buffer;
{
    register unsigned char c;
    unsigned long hiBytes;

    /* We MUST only be passed keypad (XK_KP_mumble) keysyms */
    /* if X keysym, convert to ascii by grabbing low 7 bits */
 
    hiBytes = keysym >> 8;
 
    if (keysym == XK_KP_Space)
        c = XK_space & 0x7F; /* patch encoding botch */
    else if (hiBytes == 0xFF)
        c = keysym & 0x7F;
    else
        c = keysym & 0xFF;
    buffer[0] = c;
    return 1;
}

/* The following code is copied from Xlib.  For a given keysym it checks
 * to see if it has been rebound via XRebindKeysym, if so, it returns the
 * string in buffer.  This code is dependent on the private Xlib
 * structure _XKeytrans.  XView's definition of _XKeytrans must track
 * Xlib's.
 */

#ifdef X11R6
/* lumpi@dobag.in-berlin.de */
static int
translate_key(dpy, symbol, modifiers, buffer, nbytes)
    Display 		*dpy;
    register KeySym	 symbol;
    unsigned int 	 modifiers;
    char 		*buffer;
    int 		 nbytes;
{
	/* This is _very_ rude ! */
	strcpy(buffer,XKeysymToString(symbol));
}

#else

static int
translate_key(dpy, symbol, modifiers, buffer, nbytes)
    Display 		*dpy;
    register KeySym	 symbol;
    unsigned int 	 modifiers;
    char 		*buffer;
    int 		 nbytes;
{
    struct _XKeytrans 	*p;
    int 		 length;

    if (!symbol)
        return 0;

    /* see if symbol rebound, if so, return that string. */
    for (p = (struct _XKeytrans *)dpy->key_bindings; p; p = p->next) {
        if (((modifiers & AllMods) == p->state) && (symbol == p->key)) {
            length = p->len;
            if (length > nbytes) length = nbytes;
            XV_BCOPY (p->string, buffer, length);
            return length;
        }
    }
    return 0;
}
#endif
#endif
