#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)notice_set.c 1.21 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */
#include <stdio.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <xview_private/i18n_impl.h>
#include <xview_private/draw_impl.h>
#include <xview_private/noticeimpl.h>
#include <xview/font.h>
#include <xview/frame.h>
#include <xview/panel.h>
#include <xview/win_input.h>
#include <xview/cms.h>

/* extern char		*strchr(); */
Pkg_private Xv_opaque	notice_generic_set();
#ifdef  OW_I18N
static CHAR     **notice_string_set();
#endif

Pkg_private Xv_opaque
notice_set_avlist(notice_public, avlist)
    register Xv_Notice notice_public;
    register Attr_attribute *avlist;
{
    Notice_info	*notice = NOTICE_PRIVATE(notice_public);

    return(notice_generic_set(notice, avlist, TRUE));
}

Pkg_private Xv_opaque
notice_generic_set(notice, avlist, is_object)
    register	notice_handle notice;
    register	Attr_attribute *avlist;
    Bool	is_object;
{
    notice_buttons_handle last_button = NULL;
    notice_buttons_handle reuse_buttons = notice->button_info;
    int		yes_button_seen = FALSE;
    int		no_button_seen = FALSE;
    int		num_butt = 0;
    int		num_strs = 0;
    int		trigger_set = 0;
    caddr_t	value;
    CHAR	*str;
    CHAR	**new_msg = NULL;
    CHAR	*one_msg[2];
#ifdef OW_I18N
    CHAR	**wc_msg;
#endif
    Bool	butt_changed = FALSE;
    Bool	show_seen = FALSE;
    Bool	bad_attr;

    for (; *avlist; avlist = attr_next(avlist)) {
        value = (caddr_t) avlist[1];
	bad_attr = FALSE;
	switch (avlist[0]) {

	/*
	 * GENERIC NOTICE ATTRIBUTES
	 * - Attributes used by ALL NOTICES
	 */
        case NOTICE_LOCK_SCREEN:
	    notice->lock_screen = (value != NULL);
	break;

        case NOTICE_BLOCK_THREAD:
	    notice->block_thread = (value != NULL);
	break;

#ifdef OW_I18N
          case NOTICE_MESSAGE_STRINGS_ARRAY_PTR:
	    /* Convert mbs to wchar before passing to new_msg */
	    {	int 	str_count, i;
		char 	**pptr = NULL;
		char	*str;

		pptr = (char **)value;
	        for (str_count = 0, i =0, str = pptr[i]; str; str = pptr[++i]) {
		     str_count++;
	        }
	        wc_msg = xv_calloc(str_count+1, sizeof(CHAR *));
		pptr =(char **)value;
	        for (i=0; i < str_count ; i++) {
		    wc_msg[i] = _xv_mbstowcsdup(pptr[i]);
		}
		wc_msg[str_count] = (CHAR *)NULL;
	        new_msg = (CHAR **)wc_msg;
	    }
            break;

          case NOTICE_MESSAGE_STRINGS_ARRAY_PTR_WCS:
            new_msg = (CHAR **)value;
            break;

          case NOTICE_MESSAGE_STRINGS:
	    /* Convert mbs to wchar before passing to new_msg */
	    {	int 	str_count, i;
		char 	**pptr = NULL;
		char	*str;

		pptr = (char **)&avlist[1];
	        for (str_count = 0, i =0, str = pptr[i]; str; str = pptr[++i]) {
		     str_count++;
	        }
	        wc_msg = xv_calloc(str_count+1, sizeof(CHAR *));
		pptr =(char **)&avlist[1];
	        for (i=0; i < str_count ; i++) {
		    wc_msg[i] = _xv_mbstowcsdup(pptr[i]);
		}
		wc_msg[str_count] = (CHAR *)NULL;
	        new_msg = (CHAR **)wc_msg;
	    }
            break;
 
          case NOTICE_MESSAGE_STRINGS_WCS:
            new_msg = (CHAR **)&avlist[1];
            break;
 
          case NOTICE_MESSAGE_STRING:
	    one_msg[0] = (wchar_t *)_xv_mbstowcsdup((char *)avlist[1]);
	    one_msg[1] = (CHAR *)NULL;
	    new_msg = (CHAR **) one_msg;
	    break;

          case NOTICE_MESSAGE_STRING_WCS:
	    one_msg[0] = (CHAR *)avlist[1];
	    one_msg[1] = (CHAR *)NULL;
	    new_msg = (CHAR **) one_msg;
            break;
 
          case NOTICE_BUTTON_YES:
          case NOTICE_BUTTON_YES_WCS:{
                notice_buttons_handle button;
 
                if (!yes_button_seen) {
                    yes_button_seen = TRUE;
                } else {
                    (void) xv_error(XV_ZERO,
                                    ERROR_STRING, 
				    XV_MSG("Only one NOTICE_BUTTON_YES attr allowed. Attr ignored."),
				    ERROR_PKG, NOTICE,
                                    NULL);
                    break;
                }

                /*   
                 * Button structs are reused for notices
                 * If there were no buttons to start off with, they
                 * are allocated
                 */
                if (reuse_buttons)  {
                    last_button = button = reuse_buttons;
                    reuse_buttons = reuse_buttons->next;
                    if (button->string)  {
                        free(button->string);
                        button->string = (CHAR *)NULL;
                    }
                }
                else  {
                    button = (notice_buttons_handle) notice_create_button_struct();
                    button->panel_item = (Panel_item)NULL;
                    button->next = (notice_buttons_handle)NULL;
                    (void) notice_add_button_to_list(notice, button);
                }

                if (avlist[0] == NOTICE_BUTTON_YES)
                  button->string = (wchar_t *)_xv_mbstowcsdup((char *)avlist[1]);
                else
                  button->string = XV_STRSAVE((CHAR *) avlist[1]);
                button->is_yes = TRUE;
                button->value = NOTICE_YES;
                notice->yes_button_exists = TRUE;
                num_butt++;
		butt_changed = TRUE;
                break;
	  }	

          case NOTICE_BUTTON_NO:
          case NOTICE_BUTTON_NO_WCS:{
                notice_buttons_handle button;
 
                if (!no_button_seen) {
                    no_button_seen = TRUE;
                } else {
                    xv_error(XV_ZERO,
                             ERROR_STRING,
                    	     XV_MSG("Only one NOTICE_BUTTON_NO attr allowed. Attr ignored."),
			     ERROR_PKG, NOTICE,
                             NULL);
                    break;
                }

                if (reuse_buttons)  {
                    last_button = button = reuse_buttons;
                    reuse_buttons = reuse_buttons->next;
                    if (button->string)  {
                        free(button->string);
                        button->string = (CHAR *)NULL;
                    }
                }
                else  {
                    button = (notice_buttons_handle) notice_create_button_struct();
                    button->panel_item = (Panel_item)NULL;
                    button->next = (notice_buttons_handle)NULL;
                    (void) notice_add_button_to_list(notice, button);
                }

                if( avlist[0] == NOTICE_BUTTON_NO )
                  button->string = (wchar_t *)_xv_mbstowcsdup((char *)avlist[1]);
                else
                  button->string = XV_STRSAVE((CHAR *) avlist[1]);
                button->is_no = TRUE;
                button->value = NOTICE_NO;
                notice->no_button_exists = TRUE;
                num_butt++;
		butt_changed = TRUE;

                break;
            }

          case NOTICE_BUTTON:
          case NOTICE_BUTTON_WCS:{
                notice_buttons_handle button;
 
                if (reuse_buttons)  {         
                    last_button = button = reuse_buttons;
                    reuse_buttons = reuse_buttons->next;
                    if (button->string)  {
                        free(button->string);
                        button->string = (CHAR *)NULL;
                    }
                }
                else  {
                    button = (notice_buttons_handle) notice_create_button_struct();
                    button->panel_item = (Panel_item)NULL;
                    button->next = (notice_buttons_handle)NULL;
                    (void) notice_add_button_to_list(notice, button);
                }

                if( avlist[0] == NOTICE_BUTTON )
                  button->string = (wchar_t *)_xv_mbstowcsdup((char *)avlist[1]);
                else
                  button->string = XV_STRSAVE((CHAR *) avlist[1]);
                button->value = (int) avlist[2];
		num_butt++;
		butt_changed = TRUE;

                break;
            }
#else
        case NOTICE_MESSAGE_STRINGS_ARRAY_PTR:
            new_msg = (char **) value;
        break;

        case NOTICE_MESSAGE_STRINGS:
            new_msg = (char **) &avlist[1];
        break;

        case NOTICE_MESSAGE_STRING:
            one_msg[0] = (char *)avlist[1];
            one_msg[1] = (char *)NULL;
            new_msg = (char **) one_msg;
        break;

        case NOTICE_BUTTON_YES:{
            notice_buttons_handle button;

            if (!yes_button_seen) {
                yes_button_seen = TRUE;
            } else {
                (void) xv_error(XV_ZERO,
                            ERROR_STRING,
                            XV_MSG("Only one NOTICE_BUTTON_YES attr allowed. Attr ignored."),
                            ERROR_PKG, NOTICE,
                            NULL);
                break;
            }

	    /*
	     * Button structs are reused for notices
	     * If there were no buttons to start off with, they
	     * are allocated
	     */
	    if (reuse_buttons)  {
                last_button = button = reuse_buttons;
		reuse_buttons = reuse_buttons->next;
		if (button->string)  {
		    free(button->string);
		    button->string = (char *)NULL;
		}
	    }
	    else  {
                button = (notice_buttons_handle) notice_create_button_struct();
		button->panel_item = (Panel_item)NULL;
		button->next = (notice_buttons_handle)NULL;
                (void) notice_add_button_to_list(notice, button);
	    }

	    /*
	     * Space has to be malloc for string.
	     * For non-locking notices that use panel items, this
	     * Is not necessary, since the string is cached by the
	     * panel pkg. But for screen locking notices, we have to cache
	     * the strings. So, we do it for all cases.
	     * Doing it for one case only will make things more
	     * complicated than it already is, since we can switch
	     * back and forth from non-screen-locking to screen-locking
	     * notices.
	     */
            button->string = xv_strsave((char *)avlist[1]);
            button->is_yes = TRUE;
            button->value = NOTICE_YES;
            notice->yes_button_exists = TRUE;
            num_butt++;
	    butt_changed = TRUE;

            break;
        }

        case NOTICE_BUTTON_NO:{
            notice_buttons_handle button;

            if (!no_button_seen) {
                no_button_seen = TRUE;
            } else {
                xv_error(XV_ZERO,
                    ERROR_STRING,
                    XV_MSG("Only one NOTICE_BUTTON_NO attr allowed. Attr ignored."),
                    ERROR_PKG, NOTICE,
                    NULL);
                break;
            }

	    if (reuse_buttons)  {
                last_button = button = reuse_buttons;
		reuse_buttons = reuse_buttons->next;
		if (button->string)  {
		    free(button->string);
		    button->string = (char *)NULL;
		}
	    }
	    else  {
                button = (notice_buttons_handle) notice_create_button_struct();
		button->panel_item = (Panel_item)NULL;
		button->next = (notice_buttons_handle)NULL;
                (void) notice_add_button_to_list(notice, button);
	    }

            button->string = xv_strsave((char *)avlist[1]);
            button->is_no = TRUE;
            button->value = NOTICE_NO;
            notice->no_button_exists = TRUE;
            num_butt++;
	    butt_changed = TRUE;

            break;
        }

        case NOTICE_BUTTON:{
            notice_buttons_handle button;
    
	    if (reuse_buttons)  {
                last_button = button = reuse_buttons;
		reuse_buttons = reuse_buttons->next;
		if (button->string)  {
		    free(button->string);
		    button->string = (char *)NULL;
		}
	    }
	    else  {
                button = (notice_buttons_handle) notice_create_button_struct();
		button->panel_item = (Panel_item)NULL;
		button->next = (notice_buttons_handle)NULL;
                (void) notice_add_button_to_list(notice, button);
	    }

            button->string = xv_strsave((char *)avlist[1]);
            button->value = (int) avlist[2];
            num_butt++;
	    butt_changed = TRUE;

            break;
        }

#endif
        case NOTICE_FONT:
	    /*
	     * NOTICE_FONT is a create only attribute
	     */
	    if (notice->new)  {
                notice->notice_font = (Xv_Font) avlist[1];
	    }
        break;

        case NOTICE_NO_BEEPING:
            if (value)  {
                notice->dont_beep = 1;
            }
            else  {
                notice->dont_beep = 0;
            }
        break;

	/*
	 * END of GENERIC NOTICE ATTRIBUTES
	 */

	/*
	 * ATTRIBUTES FOR SCREEN LOCKING NOTICES
	 */
        case NOTICE_FOCUS_XY:
            /*
             * needs to be implemented
             */
            notice->focus_x = (int) avlist[1];
            notice->focus_y = (int) avlist[2];
            notice->focus_specified = TRUE;
            break;

        case NOTICE_TRIGGER:
            notice->default_input_code = (int) value;
            trigger_set = 1;
        break;

        case NOTICE_TRIGGER_EVENT:
            if ((Event *) value)  {
                notice->event = (Event *)value;
            }
        break;

        case NOTICE_STATUS:
            if ((int *) value)  {
                notice->result_ptr = (int *)value;
            }
	break;

	/*
	 * END OF SCREEN LOCKING ATTRIBUTES
	 */

	/*
	 * ATTRIBUTES FOR NON SCREEN LOCKING ATTRIBUTES
	 */
        case NOTICE_EVENT_PROC:
	    if (( void (*)() ) avlist[1])  {
		notice->event_proc = ( void (*)() ) avlist[1];
	    }
        break;

        case NOTICE_BUSY_FRAMES:
	    if (notice->lock_screen)  {
		break;
	    }

	    if ((Frame)value)  {
	        Frame	*busy_frames;
		int		i;
	        int		count = 0;

		if (notice->busy_frames)  {
		    free(notice->busy_frames);
		}

		/*
		 * Count frames and alloc space for list
		 */
		for (i = 1; avlist[i]; ++i, ++count);
		busy_frames = (Frame *)xv_calloc(count+1, sizeof(Frame));

		/*
		 * Copy frames into list
		 */
		for (i = 1; avlist[i]; ++i)  {
		    busy_frames[i-1] = avlist[i];
		}
		/*
		 * End list with NULL
		 */
		busy_frames[count] = XV_ZERO;

		notice->busy_frames = busy_frames;
	    }
        break;


        case XV_SHOW:
            if (!is_object)  {
                break;
            }

	    /*
	     * If the notice is already in the state we want to set it to,
	     * skip
	     */
	    if ( (value && notice->show)  || 
                 (!value && !(notice->show)) )  {
		break;
	    }

	    /*
	     * Set flag apprpriately
	     */
	    notice->show = (value != NULL);
	    show_seen = TRUE;

	    break;

        case XV_END_CREATE:
            /*
             * If no font specified, try to get client_window font
             */
            if (!notice->notice_font)  {
	        int		e;
	        if ((e = notice_determine_font(notice->client_window, 
				notice)) != XV_OK)  {
	            /*
	             * If error occurred during font determination, 
		     * return error code
	             */
	            return(e);
	        }
            }

	    /*
	     * Pop up notice below if show == TRUE
	     */
	    if (notice->show)  {
		show_seen = TRUE;
	    }

	    /*
	     * Set the new flag to false so that the notice can be pop'd up
	     * if needed below
	     */
            notice->new = FALSE;

        break;

        default:
	    bad_attr = TRUE;
            xv_check_bad_attr(&xv_notice_pkg, avlist[0]);
        break;

        }

	if (!bad_attr)  {
	    ATTR_CONSUME(avlist[0]);
	}
    }

    if (notice->new && (num_butt == 0) && (trigger_set == 0))  {
	notice->default_input_code = (int) ACTION_STOP;
    }

    /*
     * New notice message strings specified
     */
    if (new_msg)  {
	notice_msgs_handle	msg, cur_msg;
	int			i;
#ifdef OW_I18N
	wchar_t	ret = (wchar_t)'\n';
#else
	char	ret = '\n';
#endif
	CHAR	*curStr;
	CHAR	**cur_str_ptr;

	/*
	 * Count new strings
	 */
        for (i = 0, num_strs = 0, str = new_msg[i]; 
            str; 
            num_strs++, str = new_msg[++i])  {

	    /*
	     * for every return character increment number of strings
	     */
	    while (1)  {
	        if ((curStr = STRCHR(str, ret)) == NULL)  {
		    /*
		     * If return character found, break out of loop
		     */
		    break;
	        }
		str = curStr+1;
		num_strs++;
	    }
	}

	/*
	 * If only one string specified, set count to 1
	 */
	if (!num_strs && new_msg[0])  {
	    num_strs = 1;
	}

	/*
	 * If new string count is more than previous, create new
	 * message structs
	 */
	if (num_strs > notice->number_of_strs)  {
	    for (i = notice->number_of_strs; i < num_strs; ++i)  {
		msg = notice_create_msg_struct();

		msg->panel_item = (Panel_item)NULL;

		msg->string = (CHAR *)NULL;
		msg->next = (notice_msgs_handle)NULL;
		notice_add_msg_to_list(notice, msg);
	    }
	}
	else  {
	    /*
	     * If new string count is less than previous, free excess
	     * message structs
	     */
	    if (num_strs < notice->number_of_strs)  {
	        for (i = num_strs; i < notice->number_of_strs; ++i)  {
		    msg = notice->msg_info;

		    if (!msg)  {
			break;
		    }

		    if (msg->string)  {
		        free(msg->string);
		        msg->string = (CHAR *)NULL;
		    }

		    if (msg->panel_item)  {
		        xv_destroy(msg->panel_item);
		    }

		    notice->msg_info = msg->next;

		    free((char *)msg);
	        }
	    }
	}

	/*
	 * At this point the number of message structs == the number of
	 * message strings that we need to store
	 */

	/*
	 * cur_str_ptr is for traversing ptr passed on avlist
	 */
	cur_str_ptr = new_msg;
	cur_msg = notice->msg_info;

	while(*cur_str_ptr)  {
	    /*
	     * retPtr is used for hunting down return's
	     */
	    CHAR	*retPtr = *cur_str_ptr;
	    int		len;

	    /*
	     * curStr is used for traversing the current string
	     */
	    curStr = *cur_str_ptr;

	    /*
	     * Save string:
	     * Each string on the avlist can contain more than one
	     * return terminated string.
	     *
	     * Do until no more return chars found:
	     */
	    while (retPtr)  {
		/*
		 * Search for return character
		 */
		retPtr = STRCHR(curStr, ret);

		if (retPtr)  {
		    CHAR	*tmp;

		    /*
		     * If return character found, calculate length of 
		     * needed to be alloc'd.
		     */
		    len = retPtr - curStr + 1;
	            tmp = xv_calloc(len, sizeof(CHAR));
		    /*
		     * copy string
		     */
#ifdef OW_I18N
		    STRNCPY(tmp, curStr, len-1);
			 
#else
#ifdef SVR4
		    memmove(tmp, curStr, len-1);
#else
		    bcopy(curStr, tmp, len-1);
#endif /* SVR4 */
#endif /* OW_I18N */
		    /*
		     * Pad with terminating null character
		     */
#ifdef OW_I18N
		    tmp[len-1] = (wchar_t)'\0';
#else
		    tmp[len-1] = '\0';
#endif
	            cur_msg->string = tmp;

		    /*
		     * Set current str to one character AFTER return
		     * character
		     */
		    curStr = retPtr+1;
		}
		else  {
		    /*
		     * The current string is already null terminated
		     */
	            cur_msg->string = XV_STRSAVE(curStr);
		}

		/*
		 * Advance to next message item in notice
		 */
	        cur_msg = cur_msg->next;
	    }


	    /*
	     * Advance to next string on avlist
	     */
	    cur_str_ptr++;
	}

	/*
	 * Update number of notice message strings
	 */
        notice->number_of_strs = num_strs;

	/*
	 * If we are in lock screen mode, make sure we set the layout
	 * flag so that when we switch to non screen locking mode,
	 * we do a layout
	 */
	if (notice->lock_screen)  {
	    notice->need_layout = 1;
	}
    }

    /*
     * If there were new buttons specified and not all the old 
     * button structs were used, free old button structs
     * and string space
     */
    if (butt_changed)  {
        notice_buttons_handle cur, prev;

	/*
	 * If there were button strcuts that were NOT reused, free them
	 */
	if (reuse_buttons)  {
	    cur = prev = reuse_buttons;
	    while(cur)  {
		prev = cur;
		cur = cur->next;
		if (prev->string)  {
		    free(prev->string);
		    prev->string = (CHAR *)NULL;
		}

		if (prev->panel_item)  {
		    xv_destroy(prev->panel_item);
		}

		free((CHAR *)prev);
	    }

	    /*
	     * End the new list with NULL
	     */
	    if (last_button)  {
	        last_button->next = NULL;
	    }

	}
        notice->number_of_buttons = num_butt;

	if (notice->lock_screen)  {
	    notice->need_layout = 1;
	}

	/*
	 * Also, if did not see NOTICE_BUTTON_YES (default button),
	 * make the first button the default
	 */
	if (!yes_button_seen)  {
	    notice->button_info->is_yes = TRUE;
            notice->yes_button_exists = TRUE;
	}
    }

    /*
     * If no buttons specified, give default
     */
    if (notice->number_of_buttons == 0)  {
        notice_add_default_button(notice);
    }

    /*
     * is_object == FALSE means we are called
     * from notice_prompt()
     */
    if (is_object)  {
	/*
	 * We don't need to do layout if the notice is screen locking
	 */
	if (!notice->lock_screen)  {
	    /*
	     * Do layout only if the layout flag is set(this is done
	     * only when in screen locking mode), or new message
	     * or buttons were specified.
	     */
            if (notice->need_layout || new_msg || butt_changed)  {
		notice_subframe_layout(notice, TRUE, TRUE);
            }
        }
    }

    /*
     * If this is not within xv_create
     */
    if (is_object && !(notice->new) && show_seen)  {
	notice_do_show(notice);
    }

    return(XV_OK);

}

/*
 * notice_do_show(notice)
 * Pops up the notice. Handles both the screen locking and non 
 * screen locking case
 */
Pkg_private int
notice_do_show(notice)
Notice_info	*notice;
{
    Frame	*busy_frames;

    /*
     * Check if this the screen locking type of notice
     */
    switch (notice->lock_screen)  {
    /*
     * Screen-locking notice
     */
    case 1:
        if (notice->show)  {
            int			result;

	    notice->show = 1;

            result = notice_block_popup(notice);

            notice->result = result;
	    notice->show = 0;
        }

    break;

    /*
     * Non-screen-locking notice
     */
    case 0:
	busy_frames = notice->busy_frames;

        if (notice->show)  {
	    Xv_window	root = (Xv_window)xv_get(notice->sub_frame, XV_ROOT, NULL);
	    Rect	*rect;

	    /*
	     * Get current ptr position on root window
	     */
	    rect = (Rect *) xv_get(root, WIN_MOUSE_XY);
    
	    /*
	     * Save this for later - when notice is popped down we want to
	     * warp the pointer back to the saved (x,y) position
	     */
	    notice->old_mousex = rect->r_left;
	    notice->old_mousey = rect->r_top;
    
            /*
             * Make subframe busy
             */
	    if (!notice->block_thread)  {
                xv_set(notice->client_window, FRAME_BUSY, TRUE, NULL);
	    }

	    /*
	     * Make frames busy
	     */
	    if (busy_frames)  {
	        while(*busy_frames)  {
                    xv_set(*busy_frames, FRAME_BUSY, TRUE, NULL);
		    busy_frames++;
	        }
	    }

            /*
             * Ring bell
             */
            (void) notice_do_bell(notice);

            /*
             * Make notice visible
             */
	    if (notice->block_thread)  {
		/*
		 * For thread blocking case, use xv_window_loop()
		 */
	        xv_window_loop(notice->sub_frame);

		/*
		 * Make busy frames NOT busy again
		 */
	        busy_frames = notice->busy_frames;
	        if (busy_frames)  {
	            while(*busy_frames)  {
                        xv_set(*busy_frames, FRAME_BUSY, FALSE, NULL);
		        busy_frames++;
	            }
	        }

	        notice->show = 0;

	    }
	    else  {
		/*
		 * Non thread blocking case
		 */
                xv_set(notice->sub_frame, XV_SHOW, TRUE, NULL);
	    }
        }
        else  {
            /*
             * Make client frame not busy
             */
	    if (!notice->block_thread)  {
                xv_set(notice->client_window, FRAME_BUSY, FALSE, NULL);
	    }

	    if (busy_frames)  {
	        while(*busy_frames)  {
                    xv_set(*busy_frames, FRAME_BUSY, FALSE, NULL);
		    busy_frames++;
	        }
	    }

            /*
             * Pop down notice sub frame
	     *
	     * Do only if non blocking.
	     * For thread blocking case, this was done above already.
             */
	    if (!notice->block_thread)  {
                xv_set(notice->sub_frame, XV_SHOW, FALSE, NULL);
	    }
        }
    break;

    }

    return(XV_OK);

}

