#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)fm_get.c 20.62 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE 
 *	file for terms of the license.
 */

#include <xview_private/fm_impl.h>
#include <xview_private/draw_impl.h>
#include <xview/server.h>

static int      			frame_fit_direction();
/* ACC_XVIEW */
Xv_private int  			server_parse_keystr();
Pkg_private Frame_menu_accelerator	*frame_find_menu_acc();
/* ACC_XVIEW */

Pkg_private     Xv_opaque
frame_get_attr(frame_public, status, attr, valist)
    Frame           frame_public;
    int            *status;
    Attr_attribute  attr;
    va_list	    valist;
{
    register Frame_class_info *frame = FRAME_CLASS_PRIVATE(frame_public);
    static Rect     rect_local;

    switch (attr) {
      case XV_RECT:
	{
	    static Rect rect;
	    
	    win_getrect(frame_public, &rect);
	    /* need to account for the possible footer window */
	    if (status_get(frame, show_footer) && status_get(frame, created)) 
	      rect.r_height -= (int)xv_get(frame->footer, XV_HEIGHT);
#ifdef OW_I18N
	    /* need to account for the possible imstatus window */
	    if (status_get(frame, show_imstatus) && status_get(frame, created)) 
	      rect.r_height -= (int)xv_get(frame->imstatus, XV_HEIGHT);
#endif
	    return (Xv_opaque)&rect;
	}
	
      case XV_HEIGHT: 
	{
	    Rect rect;
	    int height;
	    
	    win_getrect(frame_public, &rect);
	    height = rect.r_height;
	    /* need to account for the possible footer window */
	    if (status_get(frame, show_footer) && status_get(frame, created))
	      height -= (int)xv_get(frame->footer, XV_HEIGHT);
#ifdef OW_I18N
	    /* need to account for the possible imstatus window */
	    if (status_get(frame, show_imstatus) && status_get(frame, created))
	      height -= (int)xv_get(frame->imstatus, XV_HEIGHT);
#endif
	    return (Xv_opaque)height;
	}	  
	
      case WIN_FIT_WIDTH:
	return (Xv_opaque)frame_fit_direction(frame, WIN_DESIRED_WIDTH);

      case WIN_FIT_HEIGHT:
	{
	    int height;
	    
	    height = frame_fit_direction(frame, WIN_DESIRED_HEIGHT);
	    if (status_get(frame, show_footer) && status_get(frame, created))
	      height += (int)xv_get(frame->footer, XV_HEIGHT);
#ifdef OW_I18N
	    if (status_get(frame, show_imstatus) && status_get(frame, created))
	      height += (int)xv_get(frame->imstatus, XV_HEIGHT);
#endif
	    return (Xv_opaque)height;
	}

      case FRAME_ICON:
	return frame->icon;

      case FRAME_NTH_SUBWINDOW:{
	    register Xv_Window sw;
	    register int    n = va_arg(valist, int);

	    FRAME_EACH_SUBWINDOW(frame, sw)
		if (--n == 0)
		return sw;
	    FRAME_END_EACH
		return XV_ZERO;
	}

      case FRAME_OLD_RECT:
	return ((Xv_opaque) & frame->oldrect);

      case FRAME_DEFAULT_DONE_PROC:
	return (Xv_opaque) frame->default_done_proc;

      case FRAME_DONE_PROC:
	return (Xv_opaque) (frame->done_proc ?
			    frame->done_proc : frame->default_done_proc);

      case FRAME_FOCUS_DIRECTION:
	return (Xv_opaque) (frame->focus_window ?
	    xv_get(frame->focus_window, XV_KEY_DATA, FRAME_FOCUS_DIRECTION) :
	    FRAME_FOCUS_UP);

      case FRAME_FOCUS_WIN:
	return (Xv_opaque) frame->focus_window;

      case FRAME_FOREGROUND_COLOR: 
	{
	    static Xv_singlecolor fg;
	    fg.red = (unsigned char)(frame->fg.red >> 8);
	    fg.green = (unsigned char)(frame->fg.green >> 8);
	    fg.blue = (unsigned char)(frame->fg.blue >> 8);
	    return ((Xv_opaque) (&fg));
	}
	
      case FRAME_BACKGROUND_COLOR:
	{
	    static Xv_singlecolor bg;
	    bg.red = (unsigned char)(frame->bg.red >> 8);
	    bg.green = (unsigned char)(frame->bg.green >> 8);
	    bg.blue = (unsigned char)(frame->bg.blue >> 8);
	    return ((Xv_opaque) (&bg));
	}
	    
      case FRAME_SHOW_FOOTER:
	return (Xv_opaque) status_get(frame, show_footer);

      case FRAME_LEFT_FOOTER:
#ifdef OW_I18N
        return (Xv_opaque) _xv_get_mbs_attr_dup(&frame->left_footer);
#else
	return (Xv_opaque) frame->left_footer;
#endif 

#ifdef OW_I18N
      case FRAME_LEFT_FOOTER_WCS:
        return (Xv_opaque) _xv_get_wcs_attr_dup(&frame->left_footer);
#endif 

      case FRAME_RIGHT_FOOTER:
#ifdef OW_I18N
        return (Xv_opaque) _xv_get_mbs_attr_dup(&frame->right_footer);
#else
	return (Xv_opaque) frame->right_footer;
#endif

#ifdef OW_I18N
      case FRAME_RIGHT_FOOTER_WCS:
        return (Xv_opaque) _xv_get_wcs_attr_dup(&frame->right_footer);
#endif
	
      case FRAME_LABEL:
#ifdef OW_I18N
        return (Xv_opaque) _xv_get_mbs_attr_dup(&frame->label);
#else
	return (Xv_opaque) frame->label;
#endif

#ifdef OW_I18N
      case XV_LABEL_WCS:
        return (Xv_opaque) _xv_get_wcs_attr_dup(&frame->label);
#endif

      case FRAME_NO_CONFIRM:
	return (Xv_opaque) status_get(frame, no_confirm);

      case FRAME_NTH_SUBFRAME:{
	    register Xv_Window sw;
	    register int    n = va_arg(valist, int);

	    FRAME_EACH_SUBFRAME(frame, sw)
		if (--n == 0)
		return sw;
	    FRAME_END_EACH
		return XV_ZERO;
	}

      case FRAME_CLOSED:
	/* If the frame is Withdrawn, return the inital_state the frame will
	 * be in when it is mapped.					     */
	if (xv_get(frame_public, XV_SHOW))
	    return (Xv_opaque) frame_is_iconic(frame);
	else
	    return (Xv_opaque) status_get(frame, initial_state);

      case FRAME_INHERIT_COLORS:
	return (Xv_opaque) xv_get(frame_public, WIN_INHERIT_COLORS);

      case FRAME_SUBWINDOWS_ADJUSTABLE:	/* WIN_BOUNDARY_MGR: */
	return (Xv_opaque) status_get(frame, bndrymgr);

      case WIN_TYPE:
	return (Xv_opaque) FRAME_TYPE;

      case FRAME_BUSY:
	return (Xv_opaque) status_get(frame, busy);

#ifdef OW_I18N
      case FRAME_RIGHT_IMSTATUS:
        return (Xv_opaque) _xv_get_mbs_attr_dup(&frame->right_IMstatus);

      case FRAME_LEFT_IMSTATUS:
        return (Xv_opaque) _xv_get_mbs_attr_dup(&frame->left_IMstatus);

      case FRAME_RIGHT_IMSTATUS_WCS:
        return (Xv_opaque) _xv_get_wcs_attr_dup(&frame->right_IMstatus);

      case FRAME_LEFT_IMSTATUS_WCS:
        return (Xv_opaque) _xv_get_wcs_attr_dup(&frame->left_IMstatus);

      case FRAME_SHOW_IMSTATUS:
	return (Xv_opaque) status_get(frame, show_imstatus);

      case FRAME_INACTIVE_IMSTATUS:
	return (Xv_opaque) status_get(frame, inactive_imstatus);

#ifdef FULL_R5
      case FRAME_IMSTATUS_RECT: {
        static Rect    imstatus_rect;

        if (status_get(frame, show_imstatus)
            && status_get(frame, created)
            && frame->imstatus) {
            imstatus_rect = *(Rect *)xv_get(frame->imstatus, WIN_RECT);
        } else {
            imstatus_rect.r_left = 0;
            imstatus_rect.r_top = 0;
            imstatus_rect.r_width = 0;
            imstatus_rect.r_height = 0;
        }
        return (Xv_opaque)&imstatus_rect;
      }  
#endif /* FULL_R5 */	        
#endif

      case FRAME_ACCELERATOR:
      case FRAME_X_ACCELERATOR: {
        short	    code = (short) va_arg(valist, int);
        KeySym	    keysym = (KeySym) va_arg(valist, int);
        Frame_accelerator *accel;

	for (accel = frame->accelerators; accel; accel = accel->next) {
	    if (accel->code == code || accel->keysym == keysym)
		break;
	}
	return (Xv_opaque) accel;
      }

      /* ACC_XVIEW */
      case FRAME_MENUS:
	return((Xv_opaque)frame->menu_list);

      case FRAME_MENU_COUNT:
	return((Xv_opaque)frame->menu_count);

#ifdef OW_I18N
	case FRAME_MENU_ACCELERATOR:
	case FRAME_MENU_ACCELERATOR_WCS: {
	/*
	 * which_attr is to determine which attribute is used.
	 * pswcs is used to take advantage of the _xv_pswcs functions
	 * that malloc/convert mb/wcs strings.
	 */
        Attr_attribute    which_attr = attr;
	_xv_pswcs_t     pswcs = {0, NULL, NULL};
#else
	case FRAME_MENU_ACCELERATOR: {
#endif /* OW_I18N */
	int			keycode, result;
	unsigned int		state;
	KeySym			keysym;
        Frame_menu_accelerator *menu_accel;
	Xv_server		server_public;
	CHAR			*keystr;

#ifdef OW_I18N
	/*
	 * Use different macro to duplicate/convert strings depending
	 * on whether mb/wcs attribute was used.
	 */
	if (which_attr == FRAME_MENU_ACCELERATOR)  {
            _xv_pswcs_mbsdup(&pswcs, (char *)va_arg(valist, char *));
	}
	else  {
	    _xv_pswcs_wcsdup(&pswcs, (CHAR *)va_arg(valist, CHAR *));
	}

	keystr = pswcs.value;
#else
	keystr = (char *)va_arg(valist, char *);
#endif

	/*
	 * Return NULL if key string passed in is NULL
	 */
	if (!keystr)  {
	    return (Xv_opaque)NULL;
	}

	/*
	 * Get server handle
	 */
	server_public = XV_SERVER_FROM_WINDOW(frame_public);

	/*
	 * From keystring, get keycode, state, keysym...
	 */
        result = server_parse_keystr(server_public, keystr, &keysym, 
				&keycode, &state, 0, NULL);

	/*
	 * If get parsing error, return NULL
	 */
	if (result != XV_OK)  {
#ifdef OW_I18N
            if (pswcs.storage != NULL)
                xv_free(pswcs.storage);
#endif /* OW_I18N */
	    return (Xv_opaque)NULL;
	}

	/*
	 * Search for menu accelerator with matching keycode/state
	 * or keysym
	 */
	menu_accel = frame_find_menu_acc(frame_public, keycode, state, 
			keysym, FALSE);

#ifdef OW_I18N
            if (pswcs.storage != NULL)
                xv_free(pswcs.storage);
#endif /* OW_I18N */
	return (Xv_opaque) menu_accel;
      }

      case FRAME_MENU_X_ACCELERATOR: {
	int			keycode;
	unsigned int		state;
	KeySym			keysym;
        Frame_menu_accelerator *menu_accel;

	keycode = (int) va_arg(valist, int);
	state = (unsigned int)va_arg(valist, int);
	keysym = (KeySym)va_arg(valist, int);

	/*
	 * Search for menu accelerator with matching state and 
	 * keycode/keysym
	 */
	menu_accel = frame_find_menu_acc(frame_public, keycode, state, 
				keysym, FALSE);

	return (Xv_opaque) menu_accel;

      }
      /* ACC_XVIEW */

      case FRAME_COMPOSE_STATE:
	return((Xv_opaque)status_get(frame, compose_led));

      case FRAME_MIN_SIZE: {
/* Alpha compatibility, mbuck@debian.org */
#if 1
	 int *width = (int *)va_arg(valist, int *),
	     *height = (int *)va_arg(valist, int *),
#else
	 int *width = (int *)va_arg(valist, int),
	     *height = (int *)va_arg(valist, int),
#endif
	      footer_height = 0;

         if (status_get(frame, show_footer) && frame->footer &&
	     (frame->normal_hints.flags & PMinSize))
             footer_height += (int)xv_get(frame->footer, XV_HEIGHT);
#ifdef OW_I18N
         if (status_get(frame, show_imstatus) && frame->imstatus &&
	     (frame->normal_hints.flags & PMinSize))
             footer_height += (int)xv_get(frame->imstatus, XV_HEIGHT);
#endif

	 *width = frame->normal_hints.min_width;
	 *height = frame->normal_hints.min_height - footer_height;
	 return((Xv_opaque)0);
      }

      case FRAME_MAX_SIZE: {
#if 1
	 int *width = (int *)va_arg(valist, int *),
	     *height = (int *)va_arg(valist, int *),
#else
	 int *width = (int *)va_arg(valist, int),
	     *height = (int *)va_arg(valist, int),
#endif
	      footer_height = 0;

         if (status_get(frame, show_footer) && frame->footer &&
	     (frame->normal_hints.flags & PMaxSize))
             footer_height += (int)xv_get(frame->footer, XV_HEIGHT);
#ifdef OW_I18N
         if (status_get(frame, show_imstatus) && frame->imstatus &&
	     (frame->normal_hints.flags & PMaxSize))
             footer_height += (int)xv_get(frame->imstatus, XV_HEIGHT);
#endif 

	 *width = frame->normal_hints.max_width;
	 *height = frame->normal_hints.max_height - footer_height;
	 return((Xv_opaque)0);
      }

      /* Here for SunView1 compatibility only. */
      case FRAME_CLOSED_RECT:
	{
	    register int is_subframe =
			(int)xv_get(xv_get(frame_public, WIN_OWNER),
					       XV_IS_SUBTYPE_OF, FRAME_CLASS);
	    
	    /* subframes don't have a closed rect */
	    if (is_subframe)
	      return XV_ZERO;
	    (void) win_getrect(frame->icon, &rect_local);
	    return (Xv_opaque) & rect_local;
	}

	/* Here for SunView1 compatibility only. */
      case FRAME_CURRENT_RECT:
	if (frame_is_iconic(frame)) {
	    (void) win_getrect(frame->icon, &rect_local);
	    return (Xv_opaque) & rect_local;
	} else {
	    return xv_get(frame_public, WIN_RECT);
	}

      default:
	if (xv_check_bad_attr(&xv_frame_class_pkg, attr) == XV_ERROR) {
	    *status = XV_ERROR;
	}
	return (Xv_opaque) 0;
    }
}


static int
frame_fit_direction(frame, direction)
    Frame_class_info *frame;
    Window_attribute direction;
{
    Frame           frame_public = FRAME_PUBLIC(frame);
    register Xv_Window sw;
    Rect            rect, rbound;
    register short *value = (direction == WIN_DESIRED_WIDTH) ?
    &rect.r_width : &rect.r_height;

    rbound = rect_null;
    FRAME_EACH_SHOWN_SUBWINDOW(frame, sw)
	(void)win_get_outer_rect(sw, &rect);
    *value += FRAME_BORDER_WIDTH;
    rbound = rect_bounding(&rbound, &rect);
    FRAME_END_EACH
	if (direction == WIN_DESIRED_WIDTH) {
	if (!rbound.r_width)
	    (void) win_getrect(frame_public, &rbound);
	else
        {
            if (rbound.r_left)
                rbound.r_width += rbound.r_left;
            rbound.r_width += FRAME_BORDER_WIDTH;
        }
	return rbound.r_width;
    } else {
	if (!rbound.r_height)
	    (void) win_getrect(frame_public, &rbound);
	else
        {
            if (rbound.r_top)
                rbound.r_height += rbound.r_top;
            rbound.r_height += FRAME_BORDER_WIDTH;
        }
	return rbound.r_height;
    }
}

/* ACC_XVIEW */
/*
 * Search for an existing menu accelerator on the frame
 * that matches the passed state and keycode/keysym.
 *
 * This function is used to look up an accelerator to see if
 * it exists on the frame to determine:
 *	- if a key sequence should be considered an accelerator
 *	  when detected on the frame
 *	- whether an accelerator that is to be added to the frame
 *	  is a duplicate of an already existing one. For example,
 *	  "Meta+plus" is a duplicate of "Meta+Shift+equals", at 
 *	  least on a Sun keyboard
 *
 * This function is passed:
 *	keycode
 *	keysym
 *	state of modifier keys
 *
 * The accelerators stored on the frame also contain the above 
 * information. In doing the search, we need to make sure that
 * the following things match i.e. are the same:
 *	"Meta+A", "Meta+Shift+A", "Meta+Shift+a"
 *
 * This is done by:
 *	1. Get unmodified keysym using 
 *		XKeycodeToKeysym(..., keycode, 0)
 *	2. Get shifted keysym using
 *		XKeycodeToKeysym(..., keycode, 1)
 *	   We will perform a search of the above keysyms in the
 *	   frame accelerator list
 *	NOTE: How should ModeSwitch be handled here?
 *	3. If shifted keysym exists, make sure we ignore ShiftMask
 *	   in our searches later on
 *	4. Look at 'state' mask
 *	   If (state | ShiftMask) 
 *		we perform search on shifted keysym only (found in step 2)
 *	   Else 
 *		perform search on unmodified keysym only (found in step 1)
 *	   We selectively perform the search on only one of the keysyms 
 *	   by NULLifying the other with NoSymbol.
 *
 * From the keycode passed in, we can determine what the modified/shifted 
 * keysyms are, and these will be the keysyms used in the search.
 * 
 * However, if the keysym parameter is != NoSymbol, it will be the 
 * (only) keysym used in the search.
 */
Pkg_private Frame_menu_accelerator *
frame_find_menu_acc(frame_public, keycode, state, keysym, remove)
Frame		frame_public;
int		keycode;
unsigned int	state;
KeySym		keysym;
int		remove;
{
    int				i, ksym_count, shift_ksym_exist, 
				ignore_shift;
    KeySym			keysymlist[2];
    Frame_menu_accelerator 	*menu_accel, *prev;
    Display			*dpy =
			XV_DISPLAY_FROM_WINDOW(frame_public);
    Frame_class_info		*frame = 
			FRAME_CLASS_PRIVATE(frame_public);

    /*
     * Get keysyms for unmodified and Shift modified keycode
     * This has to be done even if the 'keysym' parameter
     * is not NoSymbol because we need to determine if 
     * we want to ignore the Shift mask.
     */

    /*
     * Get keysym for unmodified keycode
     */
    keysymlist[0] = XKeycodeToKeysym(dpy, keycode, 0);

    /*
     * Return NULL if no keysym exist for unmodified keycode
     */
    if ((keysymlist[0] == NoSymbol) && (keysym == NoSymbol))  {
        return (Frame_menu_accelerator *) NULL;
    }

    /*
     * Get keysym for shifted keycode
     */
    keysymlist[1] = XKeycodeToKeysym(dpy, keycode, 1);

    /*
     * This keycode has a valid entry for Shift/Caps Lock
     * if the unmodified entry and the shifted one are not
     * the same, and the shifted keysym is something meaningful
     */
    shift_ksym_exist = ((keysymlist[0] != keysymlist[1]) && 
				(keysymlist[1] != NoSymbol));

    /*
     * We want to ignore the Shift mask i.e. ignore it if the
     * keysym for it exists for this keycode
     */
    ignore_shift = shift_ksym_exist;

    if (shift_ksym_exist)  {
        /*
         * We want to get a match for the right keysym so we
         * NULLify the undesired one.
         *
         * For example, for "Meta+Shift+equals", we want to search
         * using the keysym "plus", and not "equals". So, in this
         * case, we NULLify the "equals" entry with NoSymbol.
         * We check the state mask (for ShiftMask/LockMask) to
         * determine which keysym to NULLify.
	 *
	 * We check if lockmask is meaningful, by using the
	 * isalpha() macro. This macro only takes 7 bit chars,
	 * so we check for that using isascii(). isalpha() does 
	 * accept 8 bit characters in non C locales, but we don't 
	 * need to go into that here.
         */
        if (  isascii((int)keysymlist[0]) && 
		isalpha((int)keysymlist[0]) )  {
            /*
             * For alpha characters we look for either
             * Shift or Lock mask
             */
            if ((state & ShiftMask) || (state & LockMask))  {
                keysymlist[0] = NoSymbol;
            }
            else  {
                keysymlist[1] = NoSymbol;
            }
        }
        else  {
            /*
             * For non alpha characters, we look only for Shift,
             * since LockMask does not apply to those characters
             */
            if (state & ShiftMask)  {
                keysymlist[0] = NoSymbol;
            }
            else  {
                keysymlist[1] = NoSymbol;
            }
        }
    }

    /*
     * Check if a valid keysym was passed to search for.
     * If yes, use the passed keysym only
     * If no, use the 2 keysyms: unmodified, shifted 
     */
    if (keysym != NoSymbol)  {
        keysymlist[0] = keysym;
        ksym_count = 1;
    }
    else  {
        ksym_count = 2;
    }

    /*
     * Search for matching keysym
     */
    for (i=0; i < ksym_count; ++i)  {

        for (prev = menu_accel = frame->menu_accelerators; 
                menu_accel; 
		prev = menu_accel, menu_accel = menu_accel->next) {

	    /*
	     * Skip if keysym is NoSymbol
	     */
            if (keysymlist[i] == NoSymbol)  {
                continue;
            }

            if (menu_accel->keysym == keysymlist[i])  {
                unsigned int mod = menu_accel->modifiers;

		/*
		 * Keysyms match. Now we compare the state
		 * masks.
		 *	'state' is the state searched for
		 *	'mod' is the state of the current keysym
		 *		that matched
		 */

                if (ignore_shift)  {
                    if (state & ShiftMask)  {
                        mod |= ShiftMask;
                    }
                    else  {
                        mod &= ~ShiftMask;
                    }
                }

                /*
                 * We always ignore LockMask
                 */
                if (state & LockMask)  {
                    mod |= LockMask;
                }
                else  {
                    mod &= ~LockMask;
                }

                if (mod == state)  {
		    /*
		     * If remove flag set, unlink found node.
		     * It is up top the caller to free up any data.
		     */
		    if (remove)  {
			if (frame->menu_accelerators == menu_accel)  {
		            /*
		             * Accelerator was first on list, unlink it
		             */
			    frame->menu_accelerators = menu_accel->next;
			}
			else  {
			    prev->next = menu_accel->next;
			}
		    }
                    return (menu_accel);
                }
            }
        }
    }

    return (Frame_menu_accelerator *) NULL;
}
/* ACC_XVIEW */
