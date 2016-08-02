#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)window_cms.c 20.57 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#include <xview_private/i18n_impl.h>
#include <xview/base.h>
#include <xview/frame.h>
#include <xview/window.h>
#include <xview/server.h>
#include <xview/cms.h>
#include <xview_private/windowimpl.h>
#include <X11/Xatom.h>


Attr_attribute  xv_cms_name_key = XV_ZERO;
void            window_set_cms();
void		window_set_cmap_property();

Xv_private int
window_set_cms_name(win_public, new_name)
    Xv_Window       win_public;
    char           *new_name;
{
    Xv_Drawable_info 	*info;
    char           	*cur_name = NULL;
    Cms			cms;

    DRAWABLE_INFO_MACRO(win_public, info);
    if (!xv_cms_name_key) {
	xv_cms_name_key = xv_unique_key();
    }
    cur_name = (char *) xv_get(win_public, XV_KEY_DATA, xv_cms_name_key);

    cms = xv_find(xv_screen(info), CMS,
		  XV_VISUAL, xv_get(win_public, XV_VISUAL), 
		  XV_AUTO_CREATE, FALSE,
		  CMS_NAME, new_name,
		  NULL);
    if (cms != XV_ZERO) {
	if (cur_name) {
	    free(cur_name);
	    xv_set(win_public, XV_KEY_DATA, xv_cms_name_key, NULL, NULL);
	}
	if (xv_cms(info) != cms)
	    window_set_cms(win_public, cms, 0, 
			   (int)xv_get(cms, CMS_SIZE, NULL) - 1);
    } else {
	if (cur_name) {
	    if (strcmp(cur_name, new_name) == 0)
		return 0;
	    else
		xv_free(cur_name);
	}
	cur_name = (char *) xv_malloc(strlen(new_name) + 1);
	strcpy(cur_name, new_name);
	xv_set(win_public, XV_KEY_DATA, xv_cms_name_key, cur_name, NULL);
    }
}

Xv_private int
window_set_cms_data(win_public, cms_data)
    Xv_Window       win_public;
    Xv_cmsdata     *cms_data;
{
    Xv_Drawable_info 	*info;
    char           	*cur_name = NULL;
    Cms			cms;
    Xv_singlecolor      *cms_colors;
    register int	i;
    int			visual_class;
    Visual		*visual;
   
    DRAWABLE_INFO_MACRO(win_public, info);
    /* This hack is here so that Guide 1.1 can still work on monochrome */
    if ((xv_depth(info) < 2)  && (cms_data) && (cms_data->size > 2))
      return 0;

    visual_class = (int)xv_get(win_public, XV_VISUAL_CLASS);
    visual = (Visual *)xv_get(win_public, XV_VISUAL);
    if ((cms_data->type == XV_DYNAMIC_CMS) && !(visual_class % 2)) {
	xv_error(XV_ZERO,
		 ERROR_STRING,
		   XV_MSG("Cannot set a dynamic colormap segment on a window created\n\
with a static visual"),
		 ERROR_PKG, WINDOW,
		 NULL);
	return -1;
    }
    if (!xv_cms_name_key) {
	xv_error(XV_ZERO,
	  ERROR_STRING,
	    XV_MSG("Attempting to set colormap segment data before naming the segment"),
	  ERROR_PKG, WINDOW,
	  NULL);
	return -1;
    }
    cur_name = (char *) xv_get(win_public, XV_KEY_DATA, xv_cms_name_key);

    cms_colors = (Xv_singlecolor *)xv_malloc(cms_data->rgb_count *
					sizeof(Xv_singlecolor));
    for (i = 0; i < cms_data->rgb_count; i++) {
	cms_colors[i].red = cms_data->red[i];
	cms_colors[i].green = cms_data->green[i];
	cms_colors[i].blue = cms_data->blue[i];
    }

    if (cur_name != NULL) {
	cms = (Cms) xv_find(xv_screen(info), CMS,
			    XV_VISUAL, visual,
			    XV_AUTO_CREATE, FALSE,
			    CMS_NAME, cur_name,
			    NULL);
	if (cms == XV_ZERO) {
	    cms = xv_create(xv_screen(info), CMS, 
			XV_VISUAL, visual,
			CMS_NAME, cur_name,
			CMS_TYPE, cms_data->type,
			CMS_SIZE, cms_data->size,
			CMS_INDEX, cms_data->index,
			CMS_COLOR_COUNT, 
			cms_data->rgb_count,
			CMS_COLORS, cms_colors,
			NULL);
	} else {
	    xv_set(cms,
		CMS_INDEX, cms_data->index,
		CMS_COLOR_COUNT, cms_data->rgb_count,
		CMS_COLORS, cms_colors,
		NULL);
	}
    } else {
	cms = xv_cms(info);
	xv_set(cms,
	    CMS_INDEX, cms_data->index,
	    CMS_COLOR_COUNT, cms_data->rgb_count,
	    CMS_COLORS, cms_colors,
	    NULL);
    } 

    xv_free(cms_colors);
    if (cms == XV_ZERO) {
	cms = (Cms) xv_get(xv_screen(info), SCREEN_DEFAULT_CMS);
    }
    window_set_cms(win_public, cms, 0, 
		   (int)xv_get(cms, CMS_SIZE, NULL) - 1);
    if (cur_name) {
	xv_free(cur_name);
	xv_set(win_public, XV_KEY_DATA, xv_cms_name_key, NULL, NULL);
    }
}

Pkg_private void
window_set_cms(win_public, cms, cms_bg, cms_fg)
    Xv_Window       	win_public;
    Cms 		cms;
    int 		cms_bg;
    int                 cms_fg;
{
    Window_info    		*win = WIN_PRIVATE(win_public);
    Xv_Drawable_info 		*info;
    XSetWindowAttributes 	attrs;
    Colormap			old_cmap, new_cmap;
    unsigned long 	        new_fg, new_bg, old_fg, old_bg, val_mask = 0;

    if ( !cms )
      return;
    
    /* This hack is here so that Guide 1.1 can still work on monochrome */
    DRAWABLE_INFO_MACRO(win_public, info);
    if ((xv_depth(info) < 2) && ((int)xv_get(cms, CMS_SIZE) > 2))
      return;

    if (XVisualIDFromVisual((Visual *)xv_get(win_public, XV_VISUAL)) != 
	XVisualIDFromVisual((Visual *)xv_get(cms, XV_VISUAL))) {
	    xv_error(XV_ZERO, 
		     ERROR_STRING,
		     XV_MSG("Can not set a CMS on a window that was created with a different visual"),
		     ERROR_PKG, WINDOW,
		     NULL);
	    return;
    }
    
    old_fg = xv_fg(info);
    new_fg = (unsigned long)xv_get(cms, CMS_PIXEL, cms_fg, NULL);
    old_bg = xv_bg(info);
    new_bg = (unsigned long)xv_get(cms, CMS_PIXEL, cms_bg, NULL);
    old_cmap = (Colormap)xv_get(xv_cms(info), XV_XID);
    new_cmap = (Colormap)xv_get(cms, XV_XID);
    
    xv_cms(info) = cms;
    
    if (old_cmap != new_cmap){
            window_set_cmap_property(win_public);
	    attrs.colormap = new_cmap;
	    val_mask |= CWColormap;
#ifdef OW_I18N
#ifdef FULL_R5
            /*
             * Need to set XNColormap when colormap updated or WIN_CMS set.
             */
            if (win->win_use_im && win->xic) {
                XVaNestedList       list;

                list = XVaCreateNestedList(NULL,
                        XNColormap, new_cmap,
                        NULL);
                /* FIX_ME: Should check input style here before setting */
                XSetICValues(win->xic,
                        XNPreeditAttributes, list,
                        NULL);
                XSetICValues(win->xic,
                        XNStatusAttributes, list,
                        NULL);
		if (list)
			XFree(list);
            }
		
#endif
#endif /* OW_I18N */
    }

    xv_cms_fg(info) = cms_fg;
    if (old_fg != new_fg) {
	    attrs.border_pixel = xv_fg(info) = new_fg;
	    val_mask |= CWBorderPixel;
#ifdef OW_I18N
#ifdef FULL_R5
            /*
             * Need to set XNForeground when WIN_FOREGROUND_COLOR is set.
             */
            if (win->win_use_im && win->xic) {
                XVaNestedList       list;

                list = XVaCreateNestedList(NULL,
                        XNForeground, new_fg,
                        NULL);
                /* FIX_ME: Should check input style here before setting */
                XSetICValues(win->xic,
                        XNPreeditAttributes, list,
                        NULL);
                XSetICValues(win->xic,
                        XNStatusAttributes, list,
                        NULL);
		if (list)
			XFree(list);
            }
#endif 
#endif /* OW_I18N */
    }
    xv_cms_bg(info) = cms_bg;
    if (old_bg != new_bg) {
	xv_bg(info) = new_bg;
	/* Transparent windows have no background color */
	if (!win->transparent) {
	    attrs.background_pixel = new_bg;
	    val_mask |= CWBackPixel;
#ifdef OW_I18N
#ifdef FULL_R5
            /*
             * Need to set XNBackground when WIN_BACKGROUND_COLOR is set.
             */
            if (win->win_use_im && win->xic) {
                XVaNestedList       list;

                list = XVaCreateNestedList(NULL,
                        XNBackground, new_bg,
                        NULL);
                /* FIX_ME: Should check input style here before setting */
                XSetICValues(win->xic,
                        XNPreeditAttributes, list,
                        NULL);
                XSetICValues(win->xic,
                        XNStatusAttributes, list,
                        NULL);
		XFree(list);
            }
#endif 
#endif /* OW_I18N */
	}
    }
    if (val_mask) {
	XChangeWindowAttributes(xv_display(info), xv_xid(info),
				val_mask, &attrs);
	if (val_mask & CWBackPixel) {
	    XClearWindow(xv_display(info), xv_xid(info));
	}
    }

    /*
     * The colormap segment change is posted to the passed object. 
     * The assumption is that the object understands how to propagate the 
     * change in its environment. An example might be an object like canvas 
     * that has multiple windows. 
     */
    if (win->created) {
        xv_set(win_public, WIN_CMS_CHANGE, NULL);
    }
}

Pkg_private void
window_set_cmap_property(win_public)
    Xv_Window       	win_public;
{
    Colormap cmap, default_cmap;
    Xv_Drawable_info *info, *frame_info;
    Frame frame_public;
    Atom atom;

    DRAWABLE_INFO_MACRO(win_public, info);
    cmap = (Colormap)xv_get(xv_cms(info), XV_XID, NULL);
    default_cmap = DefaultColormap(xv_display(info), 
				   (int)xv_get(xv_screen(info), SCREEN_NUMBER));

    /* 
     * Tell window manager to install colormap if we aren't using the default.
     * We don't want to set it on the frame, because they are handled by the
     * window manager automatically.
     */
    if ((cmap != default_cmap) &&
	!(int)xv_get(win_public, XV_IS_SUBTYPE_OF, FRAME_CLASS, NULL)) {
	frame_public = (Frame)xv_get(win_public, WIN_FRAME);
        DRAWABLE_INFO_MACRO(frame_public, frame_info);
	
	atom = (Atom)xv_get(xv_server(info), SERVER_ATOM,
			    "WM_COLORMAP_WINDOWS");
	XChangeProperty(xv_display(info), xv_xid(frame_info), atom, XA_WINDOW,
			32, PropModeAppend, (unsigned char *)&xv_xid(info), 1);
    }
}
