#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)fm_set.c 20.110 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE 
 *	file for terms of the license.
 */

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <xview_private/i18n_impl.h>
#include <xview_private/fm_impl.h>
#include <xview_private/draw_impl.h>
#include <xview/canvas.h>
/* ACC_XVIEW */
#include <xview/defaults.h>
#include <xview_private/svr_impl.h>
/* ACC_XVIEW */
#include <xview/cms.h>
#include <xview/server.h>
#include <pixrect/pixrect.h>
#ifdef __STDC__ 
#ifndef CAT
#define CAT(a,b)        a ## b 
#endif 
#endif
#include <pixrect/memvar.h>

/* ACC_XVIEW */
#define		FRAME_MENU_BLOCK	10
Xv_private int				server_parse_keystr();
Pkg_private int				frame_set_menu_acc();
Pkg_private Frame_menu_accelerator	*frame_find_menu_acc();
/* ACC_XVIEW */
Pkg_private void frame_display_busy();
Pkg_private void frame_update_compose_led();
static 	    void frame_adjust_normal_hints();

static void     frame_change_state();
static void 	frame_set_icon(); 

extern unsigned short default_frame_icon_image[256];

/*
 * Note: When the size of this struct is changed, please check
 * function frame_update_compose_led() to see if any size related
 * operations will still work.
 */
typedef struct {
    unsigned long	flags;
    unsigned long	state;
} Frame_win_state;

#define WSSemanticState		(1L<<0)
#define WSSemanticCompose	(1L<<0)

mpr_static(default_frame_icon_mpr, ICON_DEFAULT_WIDTH, ICON_DEFAULT_HEIGHT, 1,
	   default_frame_icon_image);

Pkg_private     Xv_opaque
frame_set_avlist(frame_public, avlist)
    Frame		 frame_public;
    Attr_attribute	 avlist[];
{
    Attr_avlist		 attrs;
    Frame_class_info	*frame = FRAME_PRIVATE(frame_public);
    int			 is_subframe,
    			 result = XV_OK,
    			 paint_footers = FALSE;
    Xv_object		 owner;
    static int		 set_icon_rect = FALSE;
    static Rect		 icon_rect;	
    Bool		 update_hints = FALSE;
    Cms			 new_frame_cms = (Cms)NULL;
    unsigned long	 new_frame_fg = XV_ZERO;
    Bool		 new_frame_fg_set = FALSE;
    Bool		 update_footer_color = FALSE;
#ifdef OW_I18N
    int			 paint_imstatus = FALSE;
    /*
     * pswcs is used to take advantage of the _xv_pswcs functions
     * that malloc/convert mb/wcs strings.
     */
    _xv_pswcs_t     	 pswcs = {0, NULL, NULL};
#endif

    for (attrs = avlist; *attrs; attrs = attr_next(attrs)) {
	switch (attrs[0]) {
	  case FRAME_CLOSED:
	  case FRAME_CLOSED_RECT: 
	    owner = xv_get(frame_public, XV_OWNER);
	    is_subframe = (int) xv_get(owner, XV_IS_SUBTYPE_OF, FRAME_CLASS);
	    switch (attrs[0]) {
	      case FRAME_CLOSED:
		frame_change_state(frame, attrs[1]);
		break;
		/* SunView1 compatibility only */
	      case FRAME_CLOSED_RECT: 
		/* can't set closed rect if subframe */
		if (!is_subframe) {
		    if (frame->icon) {
			Xv_Drawable_info *info;

			DRAWABLE_INFO_MACRO(frame_public, info);

		        (void) win_setrect(frame->icon, (Rect *) attrs[1]);
	    		frame->wmhints.flags |= IconPositionHint;
	    		frame->wmhints.icon_x = ((Rect *)attrs[1])->r_left;
	    		frame->wmhints.icon_y = ((Rect *)attrs[1])->r_top;
		        XSetWMHints(xv_display(info), xv_xid(info),
							     &(frame->wmhints));
		    } else {
		        set_icon_rect = TRUE;
			icon_rect = *(Rect *) attrs[1];
		    }
		}
		break;
	    }
	    break;

	    /* SunView1 compatibility only */
	  case FRAME_CURRENT_RECT:
	    (void) xv_set((frame_is_iconic(frame)) ? frame->icon : frame_public,
			  WIN_RECT, (Rect *) attrs[1], NULL);
	    break;

	  case FRAME_DEFAULT_DONE_PROC:
	    frame->default_done_proc = (void (*) ()) attrs[1];
	    if (!frame->default_done_proc)
		frame->default_done_proc = frame_default_done_func;
	    break;

	  case FRAME_DONE_PROC:
	    frame->done_proc = (void (*) ()) attrs[1];
	    break;

	  case FRAME_NO_CONFIRM:
	    status_set(frame, no_confirm, (int) attrs[1]);
	    break;
	    
	  case FRAME_INHERIT_COLORS:
	    xv_set(frame_public, WIN_INHERIT_COLORS, TRUE, NULL);
	    break;
	    
	  case FRAME_FOCUS_DIRECTION:
	    if (frame->focus_window) {
		xv_set(frame->focus_window,
		       XV_KEY_DATA, FRAME_FOCUS_DIRECTION, attrs[1],
		       XV_WIDTH, attrs[1] == FRAME_FOCUS_UP ?
		       FRAME_FOCUS_UP_WIDTH : FRAME_FOCUS_RIGHT_WIDTH,
		       XV_HEIGHT, attrs[1] == FRAME_FOCUS_UP ?
		       FRAME_FOCUS_UP_HEIGHT : FRAME_FOCUS_RIGHT_HEIGHT,
		       NULL);
	    }
	    break;
	    
	  case FRAME_FOREGROUND_COLOR: {
	    /* 
	     * Create-only attributes must be processed here since it can be 
	     * set through the cmdline.
	     */
	    if (!status_get(frame, created)) {
		frame->fg.red = (unsigned short)(((Xv_Singlecolor *)
						  attrs[1])->red);
		frame->fg.green = (unsigned short)(((Xv_Singlecolor *)
						    attrs[1])->green);
		frame->fg.blue = (unsigned short)(((Xv_Singlecolor *)
						   attrs[1])->blue);
		status_set(frame, frame_color, TRUE);
	    }
	    break;
	  }
	    
	  case FRAME_BACKGROUND_COLOR: {
	    if (!status_get(frame, created)) {
		frame->bg.red = (unsigned short)(((Xv_Singlecolor *)
						  attrs[1])->red);
		frame->bg.green = (unsigned short)(((Xv_Singlecolor *)
						    attrs[1])->green);
		frame->bg.blue = (unsigned short)(((Xv_Singlecolor *)
						   attrs[1])->blue);
		status_set(frame, frame_color, TRUE);
	    }
	    break;
	  }
	    
	  case FRAME_SUBWINDOWS_ADJUSTABLE:
	    status_set(frame, bndrymgr, (int) attrs[1]);
	    break;
	    
	  case FRAME_ICON:
	    frame_set_icon(frame, (Icon)attrs[1], &set_icon_rect, icon_rect);
	    break;

	  case FRAME_BUSY:
	    status_set(frame, busy, FALSE);
	    if ((int) attrs[1]) {
		if (xv_deaf(frame_public, TRUE) != XV_OK)  {
		    xv_error(frame_public,
				ERROR_STRING,
				XV_MSG("Attempt to make frame deaf failed"),
				NULL);
		    result = XV_ERROR;
		    break;
		}
		frame_display_busy(frame, WMWindowIsBusy);
		status_set(frame, busy, TRUE);
	    } else {
		if (xv_deaf(frame_public, FALSE) != XV_OK)  {
		    xv_error(frame_public,
				ERROR_STRING,
				XV_MSG("Attempt to make frame undeaf failed"),
				NULL);
		    result = XV_ERROR;
		    break;
		}
		frame_display_busy(frame, WMWindowNotBusy);
		status_set(frame, busy, FALSE);
	    }

	    break;

	  case FRAME_SCALE_STATE:
	    /*
	     * set the local rescale state bit, then tell the WM the current
	     * state, and then set the scale of our subwindows
	     */
	    wmgr_set_rescale_state(frame_public, attrs[1]);
	    frame_rescale_subwindows(frame_public, attrs[1]);
	    break;

	  case FRAME_NEXT_PANE: {
    	    Xv_Window pane = frame->focus_subwindow;
	    do {
		pane = xv_get(pane, XV_KEY_DATA, FRAME_NEXT_CHILD);
		if (!pane)
		    pane = frame->first_subwindow;
		if (pane == frame->focus_subwindow)
		    break;
	    } while (xv_set(pane, WIN_SET_FOCUS, NULL) != XV_OK);
	    xv_set(pane,
		   XV_FOCUS_ELEMENT, 0,  /* first element */
		   NULL);
	    break;
	  }

	  case FRAME_PREVIOUS_ELEMENT:
	  case FRAME_PREVIOUS_PANE: {
	    Xv_Window pane = xv_get(frame->focus_subwindow,
			   XV_KEY_DATA, FRAME_PREVIOUS_CHILD);
	    if (!pane)
		pane = frame_last_child(frame->first_subwindow);
	    if (xv_get(pane, XV_IS_SUBTYPE_OF, OPENWIN)) {
		int   is_canvas = (int) xv_get(pane, XV_IS_SUBTYPE_OF, CANVAS),
		      nbr_views = (int) xv_get(pane, OPENWIN_NVIEWS),
		      last_view = (int) xv_get(pane, OPENWIN_NTH_VIEW,
					       nbr_views-1);
		Xv_Window sb;

		if (attrs[0] == FRAME_PREVIOUS_ELEMENT && is_canvas) {
		    /* FRAME_PREVIOUS_ELEMENT: set focus to
		     *	    horizontal scrollbar,
		     *	    vertical scrollbar, or
		     *	    Openwin
		     */
		    sb = xv_get(pane, OPENWIN_HORIZONTAL_SCROLLBAR, last_view);
		    if (!sb)
			sb = xv_get(pane, OPENWIN_VERTICAL_SCROLLBAR,
				    last_view);
		    if (sb)
			xv_set(sb, WIN_SET_FOCUS, NULL);
		    else {
			/* No scrollbars: set focus to Canvas */
			if (pane != frame->focus_subwindow)
			    xv_set(pane, WIN_SET_FOCUS, NULL);
			xv_set(pane,
			       XV_FOCUS_ELEMENT, -1,  /* last element */
			       NULL);
		    }
		} else {
		    Xv_Window pw;
		    /* FRAME_PREVIOUS_PANE, or FRAME_PREVIOUS_ELEMENT on a
		     * non-canvas (e.g., textsw): set focus to last paint or
		     * view window
		     */
		    if (is_canvas)
			pw = xv_get(last_view, CANVAS_VIEW_PAINT_WINDOW);
		    else
			pw = last_view;
		    xv_set(pw, WIN_SET_FOCUS, NULL);
		    xv_set(pane,
			   XV_FOCUS_ELEMENT, -1,  /* last element */
			   NULL);
		}
	    } else {
		do {
		    if (pane == frame->focus_subwindow ||
			xv_set(pane, WIN_SET_FOCUS, NULL) == XV_OK)
			break;
		    pane = xv_get(pane, XV_KEY_DATA, FRAME_PREVIOUS_CHILD);
		    if (!pane)
			pane = frame_last_child(frame->first_subwindow);
		} while (TRUE);
		xv_set(pane,
		       XV_FOCUS_ELEMENT, -1,  /* last element */
		       NULL);
	    }
	    break;
	  }
	  case FRAME_ACCELERATOR: {
            Frame_accelerator	*new_accel,
				*accel;
	    unsigned int	mask;

	    mask = (unsigned int)xv_get(frame_public, WIN_CONSUME_X_EVENT_MASK);
	    if (!(mask & KeyPressMask) ||
		!(mask & FocusChangeMask))  {
		xv_set(frame_public, 
			WIN_CONSUME_X_EVENT_MASK, KeyPressMask | FocusChangeMask, NULL);
	    }

	    new_accel = xv_alloc(Frame_accelerator);
	    new_accel->code = (short) attrs[1];
	    new_accel->notify_proc = (void (*)()) attrs[2];
	    new_accel->data = (Xv_opaque) attrs[3];
	    if (frame->accelerators) {
		for (accel = frame->accelerators; accel->next;
		     accel = accel->next) {
		    if (accel->code == new_accel->code) {
			/* Replace accelerator info */
			accel->notify_proc = (void (*)()) attrs[2];
			accel->data = (Xv_opaque) attrs[3];
			xv_free(new_accel);
			break;
		    }
		}
		if (!accel->next) {
		    /* Add new accelerator */
		    accel->next = new_accel;
		}
	    } else {
		/* Add (first) new accelerator */
		frame->accelerators = new_accel;
	    }
	    break;
	  }
	  case FRAME_X_ACCELERATOR: {
            Frame_accelerator	*new_accel,
				*accel;
	    unsigned int	mask;

	    mask = (unsigned int)xv_get(frame_public, WIN_CONSUME_X_EVENT_MASK);
	    if (!(mask & KeyPressMask) ||
		!(mask & FocusChangeMask))  {
		xv_set(frame_public, 
			WIN_CONSUME_X_EVENT_MASK, KeyPressMask | FocusChangeMask, NULL);
	    }

	    new_accel = xv_alloc(Frame_accelerator);
	    new_accel->keysym = (KeySym) attrs[1];
	    new_accel->notify_proc = (void (*)()) attrs[2];
	    new_accel->data = (Xv_opaque) attrs[3];
	    if (frame->accelerators) {
		for (accel = frame->accelerators; accel->next;
		     accel = accel->next) {
		    if (accel->keysym == new_accel->keysym) {
			/* Replace accelerator info */
			accel->notify_proc = (void (*)()) attrs[2];
			accel->data = (Xv_opaque) attrs[3];
			xv_free(new_accel);
			break;
		    }
		}
		if (!accel->next) {
		    /* Add new accelerator */
		    accel->next = new_accel;
		}
	    } else {
		/* Add (first) new accelerator */
		frame->accelerators = new_accel;
	    }
	    break;
	  }

          /* ACC_XVIEW */
#ifdef OW_I18N
	  case FRAME_MENU_ACCELERATOR:
	  case FRAME_MENU_ACCELERATOR_WCS: {
            Attr_attribute    which_attr = attrs[0];
#else
	  case FRAME_MENU_ACCELERATOR: {
#endif /* OW_I18N */
	    Xv_server	server_public;
            Frame_menu_accelerator	*new_accel,
				*accel;
	    CHAR		*keystr = (CHAR *)NULL;
            void		(*notify_proc)();
	    unsigned int	modifiers = 0;
	    short		keycode;
	    KeySym		keysym;
	    Xv_opaque		data;

#ifdef OW_I18N
	    /*
	     * Use different macro to duplicate/convert strings depending
	     * on whether mb/wcs attribute was used.
	     */
	    if (which_attr == FRAME_MENU_ACCELERATOR)  {
                _xv_pswcs_mbsdup(&pswcs, (char *)attrs[1]);
	    }
	    else  {
		_xv_pswcs_wcsdup(&pswcs, (wchar_t *)attrs[1]);
	    }

	    keystr = pswcs.value;
#else
	    keystr = (CHAR *)attrs[1];
#endif /* OW_I18N */

	    notify_proc = (void (*)()) attrs[2];
	    data = (Xv_opaque) attrs[3];

	    server_public = XV_SERVER_FROM_WINDOW(frame_public);

	    /*
	     * Parse accelerator string 
	     */
	    result = server_parse_keystr(server_public, keystr, 
	                    &keysym, &keycode, &modifiers, 0, NULL);


	    if (result != XV_OK)  {
                result = XV_ERROR;
		break;
	    }

	    /*
	     * Search for existing menu accelerator with matching
	     * modifier and keycode/keysym
	     */
	    accel = frame_find_menu_acc(frame_public, keycode, modifiers, 
					keysym, FALSE);

	    /*
	     * Break if accelerator found
	     */
	    if (accel) {
                result = XV_ERROR;
		break;
	    }

	    /*
	     * Duplicate key string before calling frame_set_menu_acc()
	    if ((char *)attrs[1])  {
	        keystr = xv_strsave((char *)attrs[1]);
	    }
	     */

	    /*
	     * Add accelerator to list on frame
	     */
            (void)frame_set_menu_acc(frame_public, keycode, modifiers, keysym, 
			keystr, notify_proc, data);
	    break;
	  }

#ifdef OW_I18N
	  case FRAME_MENU_REMOVE_ACCELERATOR:
	  case FRAME_MENU_REMOVE_ACCELERATOR_WCS:  {
            Attr_attribute    which_attr = attrs[0];
#else
	  case FRAME_MENU_REMOVE_ACCELERATOR: {
#endif
	    Xv_server		server_public;
            Frame_menu_accelerator *accel, *prev_accel;
	    CHAR		*keystr;
	    unsigned int	modifiers = 0;
	    short		keycode;
	    KeySym		keysym;
	    int			found;

	    if (!(CHAR *)attrs[1])  {
                result = XV_ERROR;
		break;
	    }

#ifdef OW_I18N
	    /*
	     * Use different macro to duplicate/convert strings depending
	     * on whether mb/wcs attribute was used.
	     */
	    if (which_attr == FRAME_MENU_REMOVE_ACCELERATOR)  {
                _xv_pswcs_mbsdup(&pswcs, (char *)attrs[1]);
	    }
	    else  {
		_xv_pswcs_wcsdup(&pswcs, (wchar_t *)attrs[1]);
	    }

	    keystr = pswcs.value;
#else
	    keystr = (CHAR *)attrs[1];
#endif /* OW_I18N */

	    /*
	     * Get server object handle
	     */
	    server_public = XV_SERVER_FROM_WINDOW(frame_public);

	    /*
	     * Parse accelerator string 
	     */
	    result = server_parse_keystr(server_public, keystr, 
	                    &keysym, &keycode, &modifiers, 0, NULL);

	    if (result != XV_OK)  {
                result = XV_ERROR;
		break;
	    }

	    /*
	     * Search for existing menu accelerator with matching
	     * keycode/modifier or keysym
	     */
	    accel = frame_find_menu_acc(frame_public, keycode, modifiers, 
					keysym, TRUE);

	    if (accel)  {
		/*
		 * Remove/decrement entry in server accelerator map
		 */
		xv_set(server_public, SERVER_REMOVE_ACCELERATOR_MAP,
		                    accel->keysym, accel->modifiers,
		                    NULL);

		/*
		 * Free keystr and accelerator struct
		 * BUG ALERT: What about accel->data ??
		 */
#ifdef OW_I18N
		_xv_free_ps_string_attr_dup(&accel->keystr);
#else
		xv_free(accel->keystr);
#endif /* OW_I18N */
		xv_free(accel);
	    }
	    break;
	  }

	  case FRAME_MENU_X_ACCELERATOR: {
            Frame_menu_accelerator	*accel;
	    unsigned int		mask;
            void			(*notify_proc)();
	    unsigned int		modifiers = 0;
	    short			keycode;
	    KeySym			keysym;
	    Xv_opaque			data;

	    /*
	     * Get parameters of xv_set
	     */
	    keycode = (short)attrs[1];
	    modifiers = (unsigned int)attrs[2];
	    keysym = (KeySym)attrs[3];
	    notify_proc = (void (*)()) attrs[4];
	    data = (Xv_opaque) attrs[5];

	    /*
	     * Search for existing menu accelerator with matching
	     * keycode/modifier or keysym
	     */
	    accel = frame_find_menu_acc(frame_public, keycode, modifiers, 
				keysym, FALSE);

	    /*
	     * Break if accelerator found
	     */
	    if (accel) {
                result = XV_ERROR;
		break;
	    }

	    /*
	     * Add accelerator to list on frame
	     * Note: string field is NULL
	     */
            (void)frame_set_menu_acc(frame_public, keycode, modifiers, keysym, 
			(CHAR *)NULL, notify_proc, data);

	    break;
	  }
          /* ACC_XVIEW */

	  case FRAME_SHOW_FOOTER:
	    {
		int show_footer;
		
		attrs[0] = (Frame_attribute) ATTR_NOP(attrs[0]);
		show_footer = (int)attrs[1];
		
		if (status_get(frame, show_footer) != show_footer) {
		    if (status_get(frame, created)) {
			if (show_footer) {
			    if (frame->footer == XV_ZERO) {
				frame->footer = frame_create_footer(frame);
			    } else {
				xv_set(frame->footer, XV_SHOW, TRUE, NULL);
			    }
			    frame_adjust_normal_hints(frame,
					 (int)xv_get(frame->footer, XV_HEIGHT),
					 &update_hints);
			} else {
			    if (frame->footer != XV_ZERO) {
				xv_set(frame->footer, XV_SHOW, FALSE, NULL);
			        frame_adjust_normal_hints(frame,
					-(int)xv_get(frame->footer, XV_HEIGHT),
					&update_hints);
			    }
			}
		    }
		    status_set(frame, show_footer, show_footer);
		}
	    }
	    break;
		
	  case FRAME_LEFT_FOOTER:
#ifdef OW_I18N
		*attrs = (Frame_attribute)ATTR_NOP(*attrs);
		_xv_set_mbs_attr_dup(&frame->left_footer, (char *) attrs[1]);
		if (status_get(frame, show_footer))
		  paint_footers = TRUE;
#else
	    {
		int length;

		*attrs = (Frame_attribute)ATTR_NOP(*attrs);
		if ((char *)attrs[1] == (char *)NULL)
		  length = -1;
		else
		  length = strlen((char *)attrs[1]);
		if (frame->left_footer)
		  free(frame->left_footer);
		if (length != -1) {
		    frame->left_footer = (char *)xv_malloc(length + 1);
		    strcpy(frame->left_footer, (char *)attrs[1]);
		} else {
		    frame->left_footer = NULL;
		}
		if (status_get(frame, show_footer))
		  paint_footers = TRUE;
	    }
#endif /* OW_I18N */
	    break;
		
	  case FRAME_RIGHT_FOOTER:
#ifdef OW_I18N
		*attrs = (Frame_attribute)ATTR_NOP(*attrs);
		_xv_set_mbs_attr_dup(&frame->right_footer, (char *) attrs[1]);
		if (status_get(frame, show_footer))
		  paint_footers = TRUE;
#else
	    {
		int length;
		
		*attrs = (Frame_attribute)ATTR_NOP(*attrs);
		if ((char *)attrs[1] == (char *)NULL)
		  length = -1;
		else
		  length = strlen((char *)attrs[1]);
		if (frame->right_footer)
		  free(frame->right_footer);
		if (length != -1) {
		    frame->right_footer = (char *)xv_malloc(length + 1);
		    strcpy(frame->right_footer, (char *)attrs[1]);
		} else {
		    frame->right_footer = NULL;
		}
		if (status_get(frame, show_footer))
		  paint_footers = TRUE;
	    }
#endif /* OW_I18N */
	    break;

#ifdef OW_I18N
          case FRAME_LEFT_FOOTER_WCS:
                *attrs = (Frame_attribute)ATTR_NOP(*attrs);
                _xv_set_wcs_attr_dup(&frame->left_footer,
				     (wchar_t *) attrs[1]);
                if (status_get(frame, show_footer))
                  paint_footers = TRUE;
            break;
             
          case FRAME_RIGHT_FOOTER_WCS:
                *attrs = (Frame_attribute)ATTR_NOP(*attrs);
		_xv_set_wcs_attr_dup(&frame->right_footer,
				     (wchar_t *) attrs[1]);
                if (status_get(frame, show_footer))
                  paint_footers = TRUE;
            break;
#endif /* OW_I18N */      

          /* ACC_XVIEW */
	  case FRAME_MENUS:{
		Menu		*frame_menus;
		Menu_item	item;
                int             i,k;
                int             count = 0, item_number;
	        Xv_server	server_public;

	        server_public = XV_SERVER_FROM_WINDOW(frame_public);

		if(!SERVER_PRIVATE(server_public)->acceleration)
			break;
		/*
		 * Count number of menus on avlist
		 */
                for (i = 1; attrs[i]; ++i, ++count);

		/*
		 * For each menu currently in list, remove
		 * accelerators that exist for it.
		 */
                for (i = 0; i < frame->menu_count; ++i)  {
		    Menu	menu;

                    menu = frame->menu_list[i];

		    /*
		     * Get number of menu items on this menu
		     */
		    item_number = (int)xv_get(menu, MENU_NITEMS);

		    /* 
		     * for each menu item remove it's accelerator on the frame
		     */
		    for(k = 1;k < item_number+1; k++){
		        item = (Menu) xv_get(menu, MENU_NTH_ITEM, k);
			menu_set_acc_on_frame(frame_public, menu, item, FALSE);
		    }
		    /* 
		     * Remove the frame from this menu's list of frames 
		     */
		    xv_set(menu, MENU_FRAME_DELETE, frame_public, NULL); 
                }

		/*
		 * If new count is more than current list capacity
		 */
		if (count > frame->max_menu_count)  {
		    xv_free(frame->menu_list);

		    frame->max_menu_count += FRAME_MENU_BLOCK;
		    frame->menu_list = 
			(Menu *)xv_calloc(frame->max_menu_count, sizeof(Menu));
		}

		frame->menu_count = count;

		/*
		 * Copy menu list from avlist to frame
		 */
		for (i=0; i < count; ++i)  {
		    Menu	menu;
		    Menu	pullright;

                    menu = frame->menu_list[i] = attrs[i+1];

		    /*
		     * Get number of menu items
		     */
		    item_number = (int)xv_get(menu, MENU_NITEMS);

		    /* 
		     * For each menu item set the accelerator on the frame
		     */
		    for(k = 1;k < item_number+1; k++){
		        item = (Menu) xv_get(menu, MENU_NTH_ITEM,k);

			/*
			 * Add accelerator to frame
			 * Check first if this is a pullright. If it is,
			 * use FRAME_MENU_ADD on it.
			 */
			if (pullright = (Menu)xv_get(item, MENU_PULLRIGHT))  {
				xv_set(frame_public, FRAME_MENU_ADD,
					pullright, NULL);
			} else  {
			    menu_set_acc_on_frame(frame_public, menu, 
							item, TRUE);
			}
		    }

		    /* 
		     * add the frame to this menu's list of frames 
		     */
		    xv_set(menu, MENU_FRAME_ADD, frame_public, NULL); 
		}

            break;
          }

	  case FRAME_MENU_ADD:{
	    Xv_server	server_public;
	    int		i, found = FALSE, item_number;
	    Menu 	menu = attrs[1];

	    server_public = XV_SERVER_FROM_WINDOW(frame_public);

             if(!SERVER_PRIVATE(server_public)->acceleration)
			break;

	    /*
	     * Add one menu to menu list in frame
	     */

	    /*
	     * Search for menu in current list
	     */
	    for (i = 0; i < frame->menu_count; ++i)  {
		if (menu == frame->menu_list[i])  {
		    found = TRUE;
		    break;
		}
	    }

	    /*
	     * menu not found in current list
	     */
	    if (!found)  {
		/*
		 * Adding this menu will overflow list
		 * We need to alloc a new list
		 */
		if (frame->menu_count + 1 > frame->max_menu_count)  {
		    Menu	*new_list;

		    frame->max_menu_count += FRAME_MENU_BLOCK;

		    /*
		     * Alloc new list
		     */
		    new_list = (Menu *)xv_calloc(frame->max_menu_count,
			        sizeof(Menu));

		    /*
		     * Copy old list to new list if there was an old list
		     */
		    if (frame->menu_count)  {
		        int		j;

		        for (j = 0; j < frame->menu_count; ++j)  {
			    new_list[j] = frame->menu_list[j];
		        }

			/*
			 * Free old list
			 */
		        xv_free(frame->menu_list);
		    }

		    frame->menu_list = new_list;
		}

		frame->menu_list[frame->menu_count] = menu;
		frame->menu_count++;
	    }

	    /*
	     * Regardless of whether the menu was on the current list or not,
	     * we redo it's accelerators
	     */

	    /*
	     * Get number of menu items
	     */
            item_number = (int)xv_get(menu, MENU_NITEMS);

            /* 
             * for each menu item on the menu set the accelerator on the frame
             * using FRAME_MENU_ACCELERATOR
             */
            for(i = 1;i < item_number+1; i++){
		Menu_item	item;
		Menu		pullright;

                item = (Menu) xv_get(menu, MENU_NTH_ITEM, i);

		/*
		 * Add accelerator to frame
		 * Check first if this is a pullright. If it is,
		 * use FRAME_MENU_ADD recursively on it.
		 */
		if (pullright = (Menu)xv_get(item,MENU_PULLRIGHT))  {
			xv_set(frame_public, FRAME_MENU_ADD, pullright, NULL);
		}
		else {
                    menu_set_acc_on_frame(frame_public, menu, item, TRUE);
		}
            }

            /* 
	     * Add the frame to this menu's list of frames 
	     */
            xv_set(menu, MENU_FRAME_ADD, frame_public, NULL); 

            break;
	  }

	  case FRAME_MENU_DELETE:{
	    int		i, found = FALSE;
	    Menu 	menu = attrs[1];

	    /*
	     * Remove one menu from the menu list of the frame
	     */

	    /*
	     * Search for menu in current list
	     */
	    for (i = 0; i < frame->menu_count; ++i)  {
		if (menu == frame->menu_list[i])  {
		    found = TRUE;
		    break;
		}
	    }

	    if (found)  {
	        int		j, item_number;
		Menu_item	item;

		/*
		 * Shift everything by one to fill gap
		 */
		for (j = i + 1; j < frame->menu_count; ++i, ++j)  {
		    frame->menu_list[i] = frame->menu_list[j];
		}

		frame->menu_count--;

		/*
		 * Get menu item count
		 */
		item_number = (int)xv_get(menu, MENU_NITEMS);

                /* 
                 * for each menu item on the menu unset/remove the accelerator 
		 * on the frame using FRAME_MENU_ACCELERATOR
                 */
                for(i = 1;i < item_number+1; i++){
		    Menu_item	item;
		    Menu	pullright;

                    item = (Menu) xv_get(menu, MENU_NTH_ITEM, i);

		    /*
		     * Remove accelerator from frame
		     * Check first if this is a pullright. If it is,
		     * use FRAME_MENU_DELETE recursively on it.
		     */
		    if (pullright = (Menu)xv_get(item, MENU_PULLRIGHT))  {
			xv_set(frame_public, FRAME_MENU_DELETE, pullright, NULL);
		    }else
                       menu_set_acc_on_frame(frame_public, menu, item, FALSE);
                }

	    }

            break;
	  }
          /* ACC_XVIEW */

	  case XV_RECT:
	    /* Intercept attempt to set the rect and adjust for footer */
	    if (status_get(frame, show_footer) && frame->footer) {
		Rect *rect = (Rect *)attrs[1];
		int footer_height = 0;
		
		footer_height = (int)xv_get(frame->footer, XV_HEIGHT);
		rect->r_height += footer_height;
		(void)win_setrect(frame_public, rect);
		rect->r_height -= footer_height;
		ATTR_CONSUME(attrs[0]);
	    }
#ifdef OW_I18N
            if (status_get(frame, show_imstatus) && frame->imstatus) {
                Rect *rect = (Rect *)attrs[1];
                int IMstatus_height = 0;

                IMstatus_height = (int)xv_get(frame->imstatus, XV_HEIGHT);
                rect->r_height += IMstatus_height;
                (void)win_setrect(frame_public, rect);
                rect->r_height -= IMstatus_height;
                ATTR_CONSUME(attrs[0]);
            }
#endif
	    break;
	    
	  case XV_HEIGHT: 
	    /* Intercept attempt to set the height and adjust for footer */
	    if (status_get(frame, show_footer) && frame->footer) {
		int height = (int)attrs[1];
		Rect rect;
		
		height += (int)xv_get(frame->footer, XV_HEIGHT);
		(void)win_getrect(frame_public, &rect);
		rect.r_height = height;
		win_setrect(frame_public, &rect);
		ATTR_CONSUME(attrs[0]);
	    }	  
#ifdef OW_I18N
            if (status_get(frame, show_imstatus) && frame->imstatus) {
                int height = (int)attrs[1];
                Rect rect;

                height += (int)xv_get(frame->imstatus, XV_HEIGHT);
                (void)win_getrect(frame_public, &rect);
                rect.r_height = height;
                win_setrect(frame_public, &rect);
                ATTR_CONSUME(attrs[0]);
            }
#endif
	    break;
	    
	  case XV_SHOW:
	    /*
	     * Map the icon. The FRAME pkg should not let the window package
	     * know about icons and wmgr.
	     */
	    if (frame_is_iconic(frame) && (int) attrs[1]) {
		attrs[0] = (Attr_attribute) WIN_MAP;
		wmgr_top(frame_public);
	    }

	    /* If the frame is going withdrawn, then update the initial
	     * state hint so that when it is mapped again, it will be in
	     * the same state as it was when it was unmapped.
	     */
	    if (!(int)attrs[1]) {
		Xv_Drawable_info *info;
		DRAWABLE_INFO_MACRO(frame_public, info);

    		frame->wmhints.initial_state =
			(status_get(frame, initial_state) ? IconicState :
								  NormalState);
    		frame->wmhints.flags |= StateHint;
    		XSetWMHints(xv_display(info), xv_xid(info), &(frame->wmhints));
	    }
	    /* Don't expect a change of state if we are already in the state */
	    /* requested.  Also if our initial state is iconic, don't expect */
	    /* the frame to see a change of state and if we are iconic going */
	    /* to withdrawn, don't expect a state change.		     */
	    if ((window_get_map_cache(frame_public) != (int)attrs[1]) &&
		 (((int)attrs[1] && !status_get(frame, initial_state)) ||
		  (!(int)attrs[1] && !frame_is_iconic(frame))))
	    	status_set(frame, map_state_change, TRUE);

	    break;

	  case XV_X:
	  case XV_Y:
	    if (!(frame->normal_hints.flags & PPosition)) {
		frame->normal_hints.flags |= PPosition;
		update_hints = TRUE;
	    }
	    break;
	  
	  case FRAME_MIN_SIZE: {
	    int footer_height = 0;

	    if (!(int)attrs[1] && !(int)attrs[2])
		frame->normal_hints.flags &= ~PMinSize;
	    else
	        frame->normal_hints.flags |= PMinSize;

	    if (status_get(frame, show_footer) && frame->footer &&
		(frame->normal_hints.flags & PMinSize))
		footer_height += (int)xv_get(frame->footer, XV_HEIGHT);
#ifdef OW_I18N
            if (status_get(frame, show_imstatus) && frame->imstatus &&
		(frame->normal_hints.flags & PMinSize))
                footer_height += (int)xv_get(frame->imstatus, XV_HEIGHT);
#endif

	    frame->normal_hints.min_width = (int)attrs[1];
	    frame->normal_hints.min_height = (int)attrs[2] + footer_height;

	    update_hints = True;
	    break;
	  }

	  case FRAME_MAX_SIZE: {
	    int footer_height = 0;

	    if (!(int)attrs[1] && !(int)attrs[2])
		frame->normal_hints.flags &= ~PMaxSize;
	    else
	        frame->normal_hints.flags |= PMaxSize;

	    if (status_get(frame, show_footer) && frame->footer &&
		(frame->normal_hints.flags & PMinSize))
		footer_height += (int)xv_get(frame->footer, XV_HEIGHT);
#ifdef OW_I18N
            if (status_get(frame, show_imstatus) && frame->imstatus &&
		(frame->normal_hints.flags & PMinSize))
                footer_height += (int)xv_get(frame->imstatus, XV_HEIGHT);
#endif 

	    frame->normal_hints.max_width = (int)attrs[1];
	    frame->normal_hints.max_height = (int)attrs[2] + footer_height;

	    update_hints = True;
	    break;
	  }

	  case FRAME_COMPOSE_STATE:
	    frame_update_compose_led(frame, (int)attrs[1]);
	    break;

#ifdef OW_I18N
	  case FRAME_SHOW_IMSTATUS:
	    {
		int show_imstatus;
		
		attrs[0] = (Frame_attribute) ATTR_NOP(attrs[0]);
		show_imstatus = (int)attrs[1];
		
		if (status_get(frame, show_imstatus) != show_imstatus) {
                    if (status_get(frame, created)) {
                        if (show_imstatus) {
                            if (frame->imstatus == NULL) {
                                frame->imstatus = frame_create_IMstatus(frame);
                            } else {
                                xv_set(frame->imstatus, XV_SHOW, TRUE, NULL);
                            }
			    /* Adjust frame's min/max size hints. */
			    frame_adjust_normal_hints(frame,
				       (int)xv_get(frame->imstatus, XV_HEIGHT),
				       &update_hints);
                        } else {
                            if (frame->imstatus != NULL) {
                                xv_set(frame->imstatus, XV_SHOW, FALSE, NULL);
			        /* Adjust frame's min/max size hints. */
			        frame_adjust_normal_hints(frame,
				      -(int)xv_get(frame->imstatus, XV_HEIGHT),
				      &update_hints);
			    }
			}
                    }    
                    status_set(frame, show_imstatus, show_imstatus);
                }
	    }
	    break;

	  case FRAME_INACTIVE_IMSTATUS:
	    status_set(frame, inactive_imstatus, (int) attrs[1]); 
	    if (status_get(frame, show_imstatus))
	       paint_imstatus = TRUE;
	    break;

          case FRAME_LEFT_IMSTATUS:
            *attrs = (Frame_attribute) ATTR_NOP(*attrs);
	    _xv_set_mbs_attr_dup(&frame->left_IMstatus, (char *) attrs[1]);
            if (status_get(frame, show_imstatus))
               paint_imstatus = TRUE;
            break;
 
          case FRAME_LEFT_IMSTATUS_WCS:
            *attrs = (Frame_attribute) ATTR_NOP(*attrs);
	    _xv_set_wcs_attr_dup(&frame->left_IMstatus, (wchar_t *) attrs[1]);
            if (status_get(frame, show_imstatus))
               paint_imstatus = TRUE;
            break;
 
          case FRAME_RIGHT_IMSTATUS:
            *attrs = (Frame_attribute) ATTR_NOP(*attrs);
	    _xv_set_mbs_attr_dup(&frame->right_IMstatus, (char *) attrs[1]);
            if (status_get(frame, show_imstatus))
               paint_imstatus = TRUE;
            break;
 
          case FRAME_RIGHT_IMSTATUS_WCS:
            *attrs = (Frame_attribute) ATTR_NOP(*attrs);
	    _xv_set_wcs_attr_dup(&frame->right_IMstatus, (wchar_t *) attrs[1]);
            if (status_get(frame, show_imstatus))
               paint_imstatus = TRUE;
            break;
#endif /* OW_I18N */

	  case WIN_CMS:
	    /*
	     * Remember new cms set on frame, and set flag to update
	     * footer/imstatus window color
	     */
	    new_frame_cms = (Cms)attrs[1];
	    update_footer_color = TRUE;
	    break;

	  case WIN_FOREGROUND_COLOR:
	    /*
	     * Remember new fg color set on frame, set flag to indicate fg 
	     * color was set, and set flag to update footer/imstatus window 
	     * color
	     */
	    new_frame_fg = (int)attrs[1];
	    new_frame_fg_set = TRUE;
	    update_footer_color = TRUE;
	    break;

	  case XV_END_CREATE:{
		unsigned long   icon_id;
		Xv_Drawable_info *info;

		DRAWABLE_INFO_MACRO(frame_public, info);

		owner = xv_get(frame_public, XV_OWNER);
		is_subframe = (int)xv_get(owner, XV_IS_SUBTYPE_OF, FRAME) ||
			      (int)xv_get(owner, XV_IS_SUBTYPE_OF, FRAME_CMD) ||
			      (int)xv_get(owner, XV_IS_SUBTYPE_OF, FRAME_HELP);

		/*
		 * if this is a subframe, we need to reparent it to be a
		 * child of the root window and not of the main frame
		 */
		if (is_subframe) {
		    xv_set(frame_public,
			   WIN_PARENT, xv_get(frame_public, XV_ROOT),
			   NULL);
		} else if (!frame->icon) {
		    /* Create a default icon for a main frame */
		    frame->icon = frame->default_icon = 
		      xv_create(owner, ICON,
		          	XV_OWNER,	frame_public,
			  	ICON_IMAGE,	&default_frame_icon_mpr,
			  	NULL);

		    icon_id = (unsigned long) xv_get(frame->icon, XV_XID);
		    if (set_icon_rect) {
		        (void) win_setrect(frame->icon, icon_rect);
		    }
		    frame->wmhints.flags |= IconWindowHint;
		    frame->wmhints.icon_window = icon_id;
		    if (set_icon_rect) {
		        frame->wmhints.flags |= IconPositionHint;
	    	        frame->wmhints.icon_x = icon_rect.r_left;
	    	        frame->wmhints.icon_y = icon_rect.r_top;
		    }
		    XSetWMHints(xv_display(info), xv_xid(info),
							     &(frame->wmhints));
		    set_icon_rect = FALSE;
		}
		if (set_icon_rect) {
		    set_icon_rect = FALSE;
		    (void) win_setrect(frame->icon, icon_rect);
	    	    frame->wmhints.flags |= IconPositionHint;
	    	    frame->wmhints.icon_x = icon_rect.r_left;
	    	    frame->wmhints.icon_y = icon_rect.r_top;
		    XSetWMHints(xv_display(info), xv_xid(info),
							     &(frame->wmhints));
		}

		/*
		 * set the command line options.  This gives the command
		 * line the highest precedence: default, client attrs,
		 * command line.
		 *
		 */

		/*
		 * Set command line options that apply to ALL frames
		 * These include window background/foreground color,
		 * window fonts.
		 */
                if (frame_all_set_cmdline_options(frame_public) != XV_OK)  {
		    result = XV_ERROR;
		}

		/*
		 * These command line options apply to toplevel frames only
		 */
		if (!is_subframe) {
		    if (frame_set_cmdline_options(frame_public, FALSE) != XV_OK)
			result = XV_ERROR;
		    if (frame_set_icon_cmdline_options(frame_public) != XV_OK)
			result = XV_ERROR;
		}
		/*
		 * Now position the frame if its position is not fixed yet.
		 */
		frame_set_position(owner, frame);

		/*
		 * On color, set frame colors, if any. Make icon inherit 
		 * frame's fg/bg. 
		 */
		if (status_get(frame, frame_color)) {
		    frame_set_color(frame, &frame->fg, &frame->bg);
		}
		
		if (frame->icon) {
		    Cms	cms;
		    
		    /* 
		     * Make sure we have the same visuals for the icon
		     * and the frame, then inherit colors.
		     */
		    if (XVisualIDFromVisual((Visual *)xv_get(frame_public,
							     XV_VISUAL)) ==
			XVisualIDFromVisual((Visual *)xv_get(frame->icon,
							     XV_VISUAL))) {
			/*
			 * Dont override icon's colormap segment if it has
			 * been set programatically.
			 */
			cms = (Cms) xv_get(frame->icon, WIN_CMS);
			if (xv_get(cms, CMS_DEFAULT_CMS)&&(cms != xv_cms(info)))
			  xv_set(frame->icon, WIN_CMS, xv_cms(info), NULL );
		    }
		    
		}

		/* Create a footer if needed */
		if (status_get(frame, show_footer)) {
		    frame->footer = frame_create_footer(frame);
		    /* Adjust frame's min/max size hints. */
		    frame_adjust_normal_hints(frame,
				       (int)xv_get(frame->footer, XV_HEIGHT),
				       &update_hints);
		}

#ifdef OW_I18N
                if (xv_get(frame_public, WIN_USE_IM) == TRUE
		    && (XIM)xv_get((Xv_opaque)xv_server(info), XV_IM) != 0)    		    status_set(frame, show_imstatus, TRUE);
                /* Create a IMstatus if needed */
                if (status_get(frame, show_imstatus)) {
                    frame->imstatus = frame_create_IMstatus(frame);
		    /* Adjust frame's min/max size hints. */
		    frame_adjust_normal_hints(frame,
				       (int)xv_get(frame->imstatus, XV_HEIGHT),
				       &update_hints);
                }
#endif
		status_set(frame, created, TRUE);
		break;
	    }
	    
	  default:
	    xv_check_bad_attr(&xv_frame_class_pkg, attrs[0]);
	    break;
	}
    }

    /*
     * If the frame's color was changed via WIN_CMS or WIN_FOREGROUND_COLOR,
     * make sure they are inherited by the footer/im status window
     */
    if (update_footer_color)  {
	int	repaint_needed = FALSE;

        frame_update_status_win_color(frame_public, frame->footer, new_frame_cms, 
			new_frame_fg, new_frame_fg_set, &repaint_needed);

        if (repaint_needed && status_get(frame, show_footer)) 
	    paint_footers = TRUE;

#ifdef OW_I18N
        frame_update_status_win_color(frame_public, frame->imstatus, new_frame_cms, 
			new_frame_fg, new_frame_fg_set, &repaint_needed);

        if (repaint_needed && status_get(frame, show_imstatus)) 
	    paint_imstatus = TRUE;
#endif /* OW_I18N */
    }

    if (status_get(frame, created) && paint_footers)
      frame_display_footer(frame_public, TRUE);
#ifdef OW_I18N
    if (status_get(frame, created) && paint_imstatus)
      frame_display_IMstatus(frame_public, TRUE);
#endif

    if (update_hints) {
	Xv_Drawable_info *info;

	DRAWABLE_INFO_MACRO(frame_public, info);
        XSetWMNormalHints(xv_display(info), xv_xid(info), &frame->normal_hints);
    }

#ifdef OW_I18N
    if (pswcs.storage != NULL)
        xv_free(pswcs.storage);
#endif /* OW_I18N */

    return (Xv_opaque) result;
}


/*
 * Change the state of frame to iconic or open.
 */
static void
frame_change_state(frame, to_iconic)
    Frame_class_info *frame;
    int               to_iconic;
{
    Frame             frame_public = FRAME_PUBLIC(frame);
    Frame	      child;
    Xv_Drawable_info *info;

    DRAWABLE_INFO_MACRO(frame_public, info);
    status_set(frame, iconic, to_iconic);

    /* If the window is not mapped yet, then we just change the intial state  */
    /* so that when the window is mapped, it will appear as the user expected */
    frame->wmhints.initial_state = (to_iconic ? IconicState : NormalState);
    frame->wmhints.flags |= StateHint;
    XSetWMHints(xv_display(info), xv_xid(info), &(frame->wmhints));
    status_set(frame, initial_state, to_iconic);

    /* Insure that unmapped subframes maintain the same initial state as
     * their owners.  The window manager use to enforce this policy but
     * as of 5//91, it does not anymore.
     */
    FRAME_EACH_SUBFRAME(frame, child)
	if (!xv_get(child, XV_SHOW))
	    xv_set(child, FRAME_CLOSED, to_iconic, NULL);
    FRAME_END_EACH

    /* If the window is already mapped, then change it from Normal to Iconic */
    /* or Iconic to Normal.		     				     */
    if (xv_get(frame_public, XV_SHOW)) {
        /* icon is dealt with in wmgr_close/open() */
    	if (to_iconic)
	    XIconifyWindow(xv_display(info), xv_xid(info),
					xv_get(xv_screen(info), SCREEN_NUMBER));
        else {
	    status_set(frame, map_state_change, TRUE);
	    XMapWindow(xv_display(info), xv_xid(info));
	}
    }
}

Pkg_private void
frame_update_compose_led(frame, state)
    Frame_class_info    *frame;
    int			 state;
{
    Xv_Drawable_info 	*info;
    Frame_win_state	 win_state;

    DRAWABLE_INFO_MACRO(FRAME_PUBLIC(frame), info);

    if ((status_get(frame, compose_led) != state) &&
	/* Check to see if the wm supports the compose LED protocol. */
        xv_get(xv_screen(info), SCREEN_SUN_WINDOW_STATE)) {

	win_state.flags = WSSemanticState;
	if (state)
	    /* Turn compose key on. */
	    win_state.state = WSSemanticCompose;
	else
	    /* Turn compose key off. */
	    win_state.state = 0;

	status_set(frame, compose_led, state);

	XChangeProperty(xv_display(info), xv_xid(info),
			xv_get(xv_server(info), SERVER_ATOM,
			       "_SUN_WINDOW_STATE"),
			XA_INTEGER, 32, PropModeReplace,
                        (unsigned char *)&win_state, 
			/*
			 * We want to specify how many 32 bit elements.
			 * sizeof returns # of bytes, so we have
			 * to divide by bytes per long 
			 */
			sizeof(Frame_win_state)/sizeof(unsigned long));
	XFlush(xv_display(info));
    }
}

static void
frame_adjust_normal_hints(frame, adjustment, update_hints)
    Frame_class_info    *frame;
    int			 adjustment;
    Bool		*update_hints;
{
    /* Adjust frame's min/max size hints. */
    /* Typically, we are compensating for the footer. */

    if (frame->normal_hints.flags & PMinSize) {
	frame->normal_hints.min_height += adjustment;
	*update_hints = True;
    }

    if (frame->normal_hints.flags & PMaxSize) {
	frame->normal_hints.max_height += adjustment;
	*update_hints = True;
    }
}

static void
frame_set_icon(frame, icon, set_icon_rect, icon_rect) 
    Frame_class_info    *frame;
    Icon		 icon;
    int			*set_icon_rect;
    Rect		 icon_rect;	
{
    unsigned long   	 icon_id;
    Xv_Drawable_info 	*info;
    short            	 sameicon = FALSE;
    Cms			 cms;

    if ((frame->default_icon) && (frame->default_icon != icon)) {
	xv_destroy(frame->default_icon);
	frame->default_icon = XV_ZERO;
    }
		
    if (frame->icon == icon) {
	/*
	 * this will prevent notifying WM with the same icon
	 * window. The current WM will destroy the window
	 * even though it is the same one.  This will cause
	 * no icon to appear.
	 */
	sameicon = TRUE;
    } else if (!icon) {
	/* If they passed in NULL as an icon, revert back to the
	 * default icon.
	 */
	if (!frame->default_icon)
	    frame->default_icon = xv_create(
			xv_get(FRAME_PUBLIC(frame), XV_OWNER), ICON,
				    XV_OWNER,       FRAME_PUBLIC(frame),
				    ICON_IMAGE,     &default_frame_icon_mpr,
				    NULL);

	icon = frame->default_icon;
    }
    frame->icon = icon;
    xv_set(frame->icon, XV_OWNER, FRAME_PUBLIC(frame), NULL);
    icon_id = (unsigned long) xv_get(frame->icon, XV_XID);
    if (*set_icon_rect) {
        (void) win_setrect(frame->icon, icon_rect);
    }
		
    if (!sameicon) {
        frame->wmhints.flags |= IconWindowHint;
	frame->wmhints.icon_window = icon_id;
	if (*set_icon_rect) {
	    frame->wmhints.flags |= IconPositionHint;
	    frame->wmhints.icon_x = icon_rect.r_left;
	    frame->wmhints.icon_y = icon_rect.r_top;
	}
	DRAWABLE_INFO_MACRO(FRAME_PUBLIC(frame), info);
	XSetWMHints(xv_display(info), xv_xid(info), &(frame->wmhints));

	/* Set the cms if appropriate */
	if (XVisualIDFromVisual((Visual *)xv_get(FRAME_PUBLIC(frame),
							XV_VISUAL)) ==
		XVisualIDFromVisual((Visual *)xv_get(frame->icon, XV_VISUAL))) {
	    /*
	     * Dont override icon's colormap segment if it has
	     * been set programatically.
	     */
	    cms = (Cms) xv_get(frame->icon, WIN_CMS);
	    if (xv_get(cms, CMS_DEFAULT_CMS) && (cms != xv_cms(info)))
	        xv_set(frame->icon, WIN_CMS, xv_cms(info), NULL);
	}
    }
    *set_icon_rect = FALSE;
}

/* ACC_XVIEW */
Pkg_private	int
frame_set_menu_acc(frame_public, keycode, state, keysym, keystr, 
					notify_proc, data)
Frame		frame_public;
int		keycode;
unsigned int	state;
KeySym		keysym;
CHAR		*keystr;
void		(*notify_proc)();
Xv_opaque		data;
{
    Xv_server			server_public;
    Frame_class_info		*frame = FRAME_CLASS_PRIVATE(frame_public);
    Frame_menu_accelerator	*new_accel;

    new_accel = xv_alloc(Frame_menu_accelerator);
    new_accel->notify_proc = notify_proc;
    new_accel->data = data;
    new_accel->code = keycode;
    new_accel->keysym = keysym;

    new_accel->modifiers = state;

#ifdef OW_I18N
    _xv_set_wcs_attr_dup(&new_accel->keystr, keystr);
#else
    /*
     * Use already malloc'd string
     */
    new_accel->keystr = xv_strsave(keystr);
#endif /* OW_I18N */

    /*
     * Link to current list on frame
     */
    new_accel->next = frame->menu_accelerators;
    
    /*
     * Add (first) new accelerator
     */
    if (!frame->menu_accelerators) {
        unsigned int	mask;


        /*
         * If this is the first accelerator for the
         * frame, make sure it has KeyPress events
         * selected
         */
        mask = (unsigned int)xv_get(frame_public, 
                                    WIN_CONSUME_X_EVENT_MASK);
        if (!(mask & KeyPressMask) || !(mask & FocusChangeMask))  {
            xv_set(frame_public, WIN_CONSUME_X_EVENT_MASK, 
                            KeyPressMask | FocusChangeMask, 
                            NULL);
        }
    }

    /* 
     * Make this new accelerator the first in the list
     */
    frame->menu_accelerators = new_accel;

    /*
     * Get handle to server object
     */
    server_public = XV_SERVER_FROM_WINDOW(frame_public);

    /*
     * Set entry in server accelerator map
     */
    xv_set(server_public, SERVER_ADD_ACCELERATOR_MAP, 
                        keysym, state,
                        NULL);
}
/* ACC_XVIEW */
