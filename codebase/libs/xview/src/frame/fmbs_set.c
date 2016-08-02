#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)fmbs_set.c 1.44 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#include <X11/Xlib.h>
#include <xview_private/fm_impl.h>
#include <xview_private/frame_base.h>
#include <xview_private/draw_impl.h>
#include <xview_private/wmgr_decor.h>
#include <xview/server.h>

Pkg_private     Xv_opaque
frame_base_set_avlist(frame_public, avlist)
    Frame           frame_public;
    Attr_attribute  avlist[];
{
    Attr_avlist	    attrs;
    Frame_base_info *frame = FRAME_BASE_PRIVATE(frame_public);
    Xv_Drawable_info *info;
    Xv_opaque       server_public;
    int             result = XV_OK;
    int             add_decor, delete_decor;
    Atom            add_decor_list[WM_MAX_DECOR], delete_decor_list[WM_MAX_DECOR];
    char	    **cmd_line = NULL;
    int		    cmd_line_count = 0;

    DRAWABLE_INFO_MACRO(frame_public, info);
    server_public = xv_server(info);
    add_decor = delete_decor = 0;

    for (attrs = avlist; *attrs; attrs = attr_next(attrs)) {
	switch (attrs[0]) {

	  case FRAME_WM_COMMAND_STRINGS:
	    attrs[0] = (Frame_attribute) ATTR_NOP(attrs[0]);
	    if ((int)attrs[1] == -1)  {
	        cmd_line = (char **)-1;
	        cmd_line_count = 0;
	    }
	    else  {
	        cmd_line = (char **) &attrs[1];

	        /* count strings */
	        for (cmd_line_count = 0; 
			cmd_line[cmd_line_count]; 
			++cmd_line_count);

	    }
	    break;

	  case FRAME_WM_COMMAND_ARGC_ARGV:
	    attrs[0] = (Frame_attribute) ATTR_NOP(attrs[0]);
	    cmd_line_count = (int) attrs[1];
	    cmd_line = (char **) attrs[2];
	    break;

	  case FRAME_SHOW_LABEL:	/* same as FRAME_SHOW_HEADER */
	    attrs[0] = (Frame_attribute) ATTR_NOP(attrs[0]);
	    if (status_get(frame, show_label) == (int) attrs[1])
		break;

	    status_set(frame, show_label, (int) attrs[1]);

	    if ((int) attrs[1])
		add_decor++;
	    else
	      delete_decor++;

	    break;

          case FRAME_SHOW_RESIZE_CORNER:
            attrs[0] = (Frame_attribute) ATTR_NOP(attrs[0]);
            if (status_get(frame, show_resize_corner) == (int) attrs[1])
                break;

            status_set(frame, show_resize_corner, (int) attrs[1]);

	    if ((int) attrs[1])
		add_decor++;
	    else
	      delete_decor++;

            break;

	  case FRAME_PROPERTIES_PROC:
	    attrs[0] = (Frame_attribute) ATTR_NOP(attrs[0]);
	    frame->props_proc = (void (*) ()) attrs[1];

            /* This props_active is a sunview carry over.
             * If we decide to add FRAME_PROPS_ACTIVE later, to 
             * activate the "props" menu item, this placed there
             */

            status_set(frame, props_active, TRUE);

	    break;

	  case FRAME_SCALE_STATE:
	    attrs[0] = (Frame_attribute) ATTR_NOP(attrs[0]);
	    /*
	     * set the local rescale state bit, then tell the WM the current
	     * state, and then set the scale of our subwindows
	     */
	    /*
	     * WAIT FOR NAYEEM window_set_rescale_state(frame_public,
	     * attrs[1]);
	     */
	    wmgr_set_rescale_state(frame_public, attrs[1]);
	    frame_rescale_subwindows(frame_public, attrs[1]);
	    break;

	  case XV_LABEL:
	    {
		Frame_class_info *frame_class = FRAME_CLASS_FROM_BASE(frame);

		*attrs = (Frame_attribute) ATTR_NOP(*attrs);
#ifdef OW_I18N
		_xv_set_mbs_attr_dup(&frame_class->label, (char *) attrs[1]);
#else 
		if (frame_class->label)
		    free(frame_class->label);
		if ((char *) attrs[1]) {
		    frame_class->label = (char *)
			xv_calloc(1, strlen((char *) attrs[1]) + 1);
		    strcpy(frame_class->label, (char *) attrs[1]);
		} else {
		    frame_class->label = NULL;
		}
#endif /* OW_I18N */
		(void) frame_display_label(frame_class);
	    }
	    break;

#ifdef OW_I18N
          case XV_LABEL_WCS:
            {
                Frame_class_info *frame_class = FRAME_CLASS_FROM_BASE(frame);

                *attrs = (Frame_attribute) ATTR_NOP(*attrs);
		_xv_set_wcs_attr_dup(&frame_class->label, (wchar_t *) attrs[1]);
                (void) frame_display_label(frame_class);
            }
            break;
#endif /* OW_I18N */

	  case XV_END_CREATE:
	    (void)wmgr_set_win_attr(frame_public, &(frame->win_attr));
	    break;

	  default:
	    break;

	}
    }


    /* If command line strings specified, cache them on object */
    if (cmd_line)  {
	int	i;

	/*
	 * If old command line strings exists, free them
	 */
	if (frame->cmd_line_strings_count > 0)  {
	    char	**old_strings = frame->cmd_line_strings;

	    for (i = 0; i < frame->cmd_line_strings_count; ++i)  {
		if (old_strings[i])  {
		    free(old_strings[i]);
		}
	    }

#ifndef OW_I18N
	    /*
	     * Free array holding strings
	     */
	    free(old_strings);
#endif
	}

	/*
	 * Check if special flag -1 passed
	 * If yes, set string count to 0
	 * Otherwise, save passed strings 
	 */
	if ((long int)cmd_line == -1)  {
	    frame->cmd_line_strings_count = 0;
	    frame->cmd_line_strings = (char **)-1;
	}
	else  {
	    /*
	     * Check count
	     */
	    if (cmd_line_count < 0)  {
		cmd_line_count = 0;
	    }

	    /*
	     * Set count to new string count
	     */
	    frame->cmd_line_strings_count = cmd_line_count;

	    /*
	     * Allocate array to hold strings
	     */
	    frame->cmd_line_strings = (char **)xv_calloc(cmd_line_count, sizeof(char *));

	    /*
	     * Copy strings passed in one by one
	     */
	    for (i = 0; i < cmd_line_count; ++i)  {
	        frame->cmd_line_strings[i] = strdup(cmd_line[i]);
	    }
	}
    }

    /* recompute wmgr decorations */

    if (add_decor || delete_decor) { 
	Atom	check_atom;

	add_decor = delete_decor = 0;

	check_atom = (Atom)xv_get(server_public, SERVER_ATOM, "_SUN_OL_WIN_ATTR_5");

        if ((check_atom != (Atom)None) && 
		screen_check_sun_wm_protocols(xv_screen(info), check_atom))  {
	    /*
	     * Tell wmgr not to write icon labels - for now this will be done
	     * by XView
	     */
	    delete_decor_list[delete_decor++] =
		(Atom) xv_get(server_public, SERVER_ATOM, "_OL_DECOR_ICON_NAME");
	}

	if (status_get(frame, show_label))
	    add_decor_list[add_decor++] =
		(Atom) xv_get(server_public, SERVER_WM_DECOR_HEADER);
	else
	    delete_decor_list[delete_decor++] =
		(Atom) xv_get(server_public, SERVER_WM_DECOR_HEADER);

	if (status_get(frame, show_resize_corner))
	    add_decor_list[add_decor++] =
		(Atom) xv_get(server_public, SERVER_WM_DECOR_RESIZE);
	else
	    delete_decor_list[delete_decor++] =
		(Atom) xv_get(server_public, SERVER_WM_DECOR_RESIZE);

	wmgr_add_decor(frame_public, add_decor_list, add_decor);

	wmgr_delete_decor(frame_public, delete_decor_list, delete_decor);
    }

    return (Xv_opaque) result;
}
