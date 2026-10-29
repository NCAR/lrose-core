#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)svr_get.c 20.69 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE 
 *	file for terms of the license.
 */

#include <sys/types.h>
#include <sys/time.h>
#include <xview/win_event.h>
#include <xview_private/svr_impl.h>
#include <xview_private/portable.h>
#include <xview/sel_svc.h>

Xv_private Xv_opaque xv_font_with_name();
Pkg_private Atom server_intern_atom();

Pkg_private     Xv_opaque
server_get_attr(server_public, status, attr, valist)
    Xv_Server       server_public;
    int            *status;
    Server_attr     attr;
    va_list         valist;
{
    register Server_info *server = SERVER_PRIVATE(server_public);
    Xv_opaque result = 0;
    extern Xv_opaque server_get_attr_tier2();

    switch (attr) {
      case SERVER_ATOM: {
	register char *name = va_arg(valist, char *);
	return((Xv_opaque)server_intern_atom(server, name)); 
      }
	
      case SERVER_WM_WIN_BUSY:
	return server_intern_atom(server, "_OL_WIN_BUSY");

      case SERVER_WM_TAKE_FOCUS:
	return server_intern_atom(server, "WM_TAKE_FOCUS");
      case SERVER_WM_SAVE_YOURSELF:
	return server_intern_atom(server, "WM_SAVE_YOURSELF");
      case SERVER_WM_PROTOCOLS:
	return server_intern_atom(server, "WM_PROTOCOLS");
      case SERVER_WM_DELETE_WINDOW:
	return server_intern_atom(server, "WM_DELETE_WINDOW");

      case SERVER_WM_WIN_ATTR:
	return server_intern_atom(server, "_OL_WIN_ATTR");

      case SERVER_FONT_WITH_NAME:{
	    char           *name = va_arg(valist, char *);

	    return xv_font_with_name(server_public, name);
	}

      case XV_DISPLAY:
	return (Xv_opaque)(server->xdisplay);

#ifdef OW_I18N
      case XV_IM:
        return (Xv_opaque) (server->xim);
#endif /* OW_I18N */

      case SERVER_WM_ADD_DECOR:
	return server_intern_atom(server, "_OL_DECOR_ADD");
      case SERVER_WM_DELETE_DECOR:
	return server_intern_atom(server, "_OL_DECOR_DEL");
      case SERVER_WM_DECOR_CLOSE:
	return server_intern_atom(server, "_OL_DECOR_CLOSE");
      case SERVER_WM_DECOR_FOOTER:
	return server_intern_atom(server, "_OL_DECOR_FOOTER");
      case SERVER_WM_DECOR_RESIZE:
	return server_intern_atom(server, "_OL_DECOR_RESIZE");
      case SERVER_WM_DECOR_HEADER:
	return server_intern_atom(server, "_OL_DECOR_HEADER");
      case SERVER_WM_WT_BASE:
	return server_intern_atom(server, "_OL_WT_BASE");
      case SERVER_WM_WT_CMD:
	return server_intern_atom(server, "_OL_WT_CMD");
      case SERVER_WM_MENU_FULL:
	return server_intern_atom(server, "_OL_MENU_FULL");
      case SERVER_WM_MENU_LIMITED:
	return server_intern_atom(server, "_OL_MENU_LIMITED");
      case SERVER_WM_PIN_IN:
	return server_intern_atom(server, "_OL_PIN_IN");
      case SERVER_WM_PIN_OUT:
	return server_intern_atom(server, "_OL_PIN_OUT");
      case SERVER_DO_DRAG_MOVE:
	return server_intern_atom(server, "XV_DO_DRAG_MOVE");
      case SERVER_DO_DRAG_COPY:
	return server_intern_atom(server, "XV_DO_DRAG_COPY");
      case SERVER_WM_DISMISS:
	return server_intern_atom(server, "_OL_WIN_DISMISS");
      case SERVER_WM_COMMAND:
	return server_intern_atom(server, "WM_COMMAND");
      case SERVER_WM_DEFAULT_BUTTON:
	return server_intern_atom(server, "_OL_DFLT_BTN");

      /*
       * Sundae buyback
       */
      case XV_LC_BASIC_LOCALE:
	return (Xv_opaque) server->ollc[OLLC_BASICLOCALE].locale;

      case XV_LC_DISPLAY_LANG:
	return (Xv_opaque) server->ollc[OLLC_DISPLAYLANG].locale;

      case XV_LC_INPUT_LANG:
	return (Xv_opaque) server->ollc[OLLC_INPUTLANG].locale;

      case XV_LC_NUMERIC:
	return (Xv_opaque) server->ollc[OLLC_NUMERIC].locale;

      case XV_LC_TIME_FORMAT:
	return (Xv_opaque) server->ollc[OLLC_TIMEFORMAT].locale;

      /*
       * End of Sundae buyback
       */
#ifdef OW_I18N
#ifdef FULL_R5
      case XV_IM_PREEDIT_STYLE:
	return (Xv_opaque) server->preedit_style;

      case XV_IM_STATUS_STYLE:
	return (Xv_opaque) server->status_style;

      case XV_IM_STYLES:
	return (Xv_opaque) server->supported_im_styles;
#endif /* FULL_R5 */

#endif /* OW_I18N */

      case SERVER_XV_MAP:
	return ((Xv_opaque) server->xv_map);

      case SERVER_SEMANTIC_MAP:
	return ((Xv_opaque) server->sem_map);

      case SERVER_ASCII_MAP:
	return ((Xv_opaque) server->ascii_map);

      /* ACC_XVIEW */
      case SERVER_ACCELERATOR_MAP:
	return ((Xv_opaque) server->acc_map);
      /* ACC_XVIEW */

      case SERVER_CUT_KEYSYM:
	return ((Xv_opaque) server->cut_keysym);

      case SERVER_PASTE_KEYSYM:
	return ((Xv_opaque) server->paste_keysym);

      case SERVER_JOURNALLING:
	return ((Xv_opaque) server->journalling);

      case SERVER_MOUSE_BUTTONS:
	return ((Xv_opaque) server->nbuttons);
	
      case SERVER_BUTTON2_MOD:
	return ((Xv_opaque) server->but_two_mod);

      case SERVER_BUTTON3_MOD:
	return ((Xv_opaque) server->but_three_mod);

      case SERVER_CHORDING_TIMEOUT:
	return ((Xv_opaque) server->chording_timeout);

      case SERVER_CHORD_MENU:
	return ((Xv_opaque) server->chord_menu);

      case SERVER_ALT_MOD_MASK:
	return ((Xv_opaque) server->alt_modmask);

      case SERVER_META_MOD_MASK:
	return ((Xv_opaque) server->meta_modmask);

      case SERVER_NUM_LOCK_MOD_MASK:
	return ((Xv_opaque) server->num_lock_modmask);
	
      case SERVER_SEL_MOD_MASK:
	return ((Xv_opaque) server->sel_modmask);

      case SERVER_ATOM_NAME: {
	register Atom atom = va_arg(valist, Atom);
	return((Xv_opaque)server_get_atom_name(server, atom)); 
      }

      case SERVER_COMPOSE_STATUS:
	return((Xv_opaque)server->composestatus);

      case SERVER_CLEAR_MODIFIERS:
	  return ((Xv_opaque)server->pass_thru_modifiers);

      case SERVER_DISPLAY_CONTEXT:
	  /* context used by XSaveContext and XFindContext to save/retrieve
	   * server_object from a dpy struct.
	   */
	  return ((Xv_opaque)server->svr_dpy_context);
	
      case SERVER_NTH_SCREEN:
      case SERVER_FOCUS_TIMESTAMP:
      case SERVER_WM_DECOR_OK:
      case SERVER_WM_DECOR_PIN:
      case SERVER_WM_SCALE_SMALL:
      case SERVER_WM_SCALE_MEDIUM:
      case SERVER_WM_SCALE_LARGE:
      case SERVER_WM_SCALE_XLARGE:
      case SERVER_WM_PIN_STATE:
      case SERVER_WM_WINMSG_STATE:
      case SERVER_WM_WINMSG_ERROR:
      case SERVER_WM_WT_PROP:
      case SERVER_WM_WT_HELP:
      case SERVER_WM_WT_NOTICE:
      case SERVER_WM_WT_OTHER:
      case SERVER_WM_NONE:
      case SERVER_DO_DRAG_LOAD:
      case SERVER_WM_CHANGE_STATE:
      case SERVER_RESOURCE_DB:
      case XV_LOCALE_DIR:
      case SERVER_JOURNAL_SYNC_ATOM:
      case SERVER_EXTENSION_PROC:
      case XV_NAME:
#ifdef OW_I18N
      case SERVER_COMPOUND_TEXT:
      case XV_APP_NAME_WCS:
      case XV_APP_NAME:
#else
      case XV_APP_NAME:
#endif
      case SERVER_DND_ACK_KEY:
      case SERVER_ATOM_DATA:
      case SERVER_EXTERNAL_XEVENT_PROC:
      case SERVER_PRIVATE_XEVENT_PROC:
      case SERVER_EXTERNAL_XEVENT_MASK:
      case SERVER_PRIVATE_XEVENT_MASK:
	  return (server_get_attr_tier2(server_public, status, attr, valist));

      default:
	if (xv_check_bad_attr(&xv_server_pkg, (Attr_attribute)attr) == XV_ERROR)
	    goto Error;
    }
Error:
    *status = XV_ERROR;
    return (Xv_opaque) 0;
}

Pkg_private     Xv_opaque
server_get_attr_tier2(server_public, status, attr, valist)
    Xv_Server       server_public;
    int            *status;
    Server_attr     attr;
    va_list         valist;
{
    register Server_info *server = SERVER_PRIVATE(server_public);
    Xv_opaque result = 0;

    switch (attr) {
      case SERVER_NTH_SCREEN:{
            register int    number = va_arg(valist, int);
 
            if ((number < 0) || (number >= MAX_SCREENS)) {
                goto Error;
            }
            /* create the screen if it doesn't exist */
            if (!server->screens[number]) {
                server->screens[number] =
                    xv_create(server_public, SCREEN, SCREEN_NUMBER, number, NULL);
            }
            if (!server->screens[number])
                goto Error;
            return (server->screens[number]);
        }
 
      case SERVER_FOCUS_TIMESTAMP:
        return (server->focus_timestamp);

      case SERVER_WM_DECOR_OK:
        return server_intern_atom(server, "_OL_DECOR_OK");
      case SERVER_WM_DECOR_PIN:
        return server_intern_atom(server, "_OL_DECOR_PIN");
      case SERVER_WM_SCALE_SMALL:
        return server_intern_atom(server, "_OL_SCALE_SMALL");
      case SERVER_WM_SCALE_MEDIUM:
        return server_intern_atom(server, "_OL_SCALE_MEDIUM");
      case SERVER_WM_SCALE_LARGE:
        return server_intern_atom(server, "_OL_SCALE_LARGE");
      case SERVER_WM_SCALE_XLARGE:
        return server_intern_atom(server, "_OL_SCALE_XLARGE");
      case SERVER_WM_PIN_STATE:
        return server_intern_atom(server, "_OL_PIN_STATE");
      case SERVER_WM_WINMSG_STATE:
        return server_intern_atom(server, "_OL_WINMSG_STATE");
      case SERVER_WM_WINMSG_ERROR:
        return server_intern_atom(server, "_OL_WINMSG_ERROR");
      case SERVER_WM_WT_PROP:
        return server_intern_atom(server, "_OL_WT_PROP");
      case SERVER_WM_WT_HELP:
        return server_intern_atom(server, "_OL_WT_HELP");
      case SERVER_WM_WT_NOTICE:
        return server_intern_atom(server, "_OL_WT_NOTICE");
      case SERVER_WM_WT_OTHER:
        return server_intern_atom(server, "_OL_WT_OTHER");
      case SERVER_WM_NONE:
        return server_intern_atom(server, "_OL_NONE");
      case SERVER_DO_DRAG_LOAD:
        return server_intern_atom(server, "XV_DO_DRAG_LOAD");
      case SERVER_WM_CHANGE_STATE:
        return server_intern_atom(server, "WM_CHANGE_STATE");

      case SERVER_RESOURCE_DB:
        return ((Xv_opaque) server->db);
      case XV_LOCALE_DIR:
        return (Xv_opaque) server->localedir;

      case SERVER_JOURNAL_SYNC_ATOM:
        return ((Xv_opaque) server->journalling_atom);

       case SERVER_EXTENSION_PROC:
        return ((Xv_opaque) server->extensionProc);

      case XV_NAME:
        return ((Xv_opaque) server->display_name);

#ifdef OW_I18N
      case SERVER_COMPOUND_TEXT:
        return server_intern_atom(server, "COMPOUND_TEXT");

      case XV_APP_NAME_WCS:
        return((Xv_opaque) _xv_get_wcs_attr_dup(&server->app_name_string));

      case XV_APP_NAME:
        return((Xv_opaque) _xv_get_mbs_attr_dup(&server->app_name_string));
#else
      case XV_APP_NAME:
        return ((Xv_opaque) server->app_name_string);
#endif

       case SERVER_DND_ACK_KEY:
        return ((Xv_opaque) server->dnd_ack_key);

      case SERVER_ATOM_DATA: {
        register Atom           atom = va_arg(valist, Atom);
        register Xv_opaque      data;

        data = server_get_atom_data(server, atom, status);
        if (*status == XV_ERROR)
           goto Error;
        else
           return((Xv_opaque)data);
      }  

       case SERVER_EXTERNAL_XEVENT_PROC: {
          Server_proc_list *node;
          if (node = server_procnode_from_id(server, va_arg(valist,Xv_opaque)))
              result = (Xv_opaque)(node->extXeventProc);
              return result;
      }
      case SERVER_PRIVATE_XEVENT_PROC: {
          Server_proc_list *node;
          if (node = server_procnode_from_id(server, va_arg(valist,Xv_opaque)))
              result = (Xv_opaque)(node->pvtXeventProc);
              return result;
      }
      case SERVER_EXTERNAL_XEVENT_MASK: {
          Server_mask_list *node;
          if (node = server_masknode_from_xidid (server, 
                                                 va_arg(valist,Xv_opaque),
                                                 va_arg(valist,Xv_opaque)))
              result = (Xv_opaque)(node->extmask);
              return result;
      }
      case SERVER_PRIVATE_XEVENT_MASK: {
          Server_mask_list *node;
          if (node = server_masknode_from_xidid (server,
                                                 va_arg(valist,Xv_opaque),
                                                 va_arg(valist,Xv_opaque)))
              result = (Xv_opaque)(node->pvtmask);
              return result;
      }
    }
Error:
    *status = XV_ERROR;
    return (Xv_opaque) 0;
}

Xv_private      Xv_opaque
server_get_timestamp(server_public)
    Xv_Server       server_public;
{
    Server_info    *server = SERVER_PRIVATE(server_public);
    return ((Xv_opaque) server->xtime);
}

Xv_private      Xv_opaque
server_get_fullscreen(server_public)
    Xv_Server       server_public;
{
    Server_info    *server = SERVER_PRIVATE(server_public);
    return ((Xv_opaque) server->in_fullscreen);
}
