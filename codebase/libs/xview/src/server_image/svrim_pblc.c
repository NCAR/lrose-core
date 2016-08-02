#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)svrim_pblc.c 20.63 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE 
 *	file for terms of the license.
 */

#include <sys/types.h>
#include <stdio.h>
#include <pixrect/pixrect.h>

#ifdef __STDC__ 
#ifndef CAT
#define CAT(a,b)        a ## b 
#endif 
#endif
#include <pixrect/memvar.h>

#include <xview_private/i18n_impl.h>
#include <xview_private/portable.h>
#include <xview/screen.h>
#include <xview/window.h>
#include <xview/cms.h>
#include <xview/notify.h>
#include <xview_private/svrim_impl.h>
#include <X11/Xutil.h>

extern Pixrect *xv_mem_point(int w, int h, int depth, short *image);

Xv_private_data struct pixrectops server_image_ops = {
    server_image_rop,
    server_image_stencil,
    NULL,                       /* BUG should be batchrop */
    NULL,
    server_image_destroy,
    server_image_get,
    server_image_put,
    server_image_vector,
    server_image_region,
    server_image_colormap,
    server_image_colormap,
    NULL,                       /* BUG: pro_putattributes */
    NULL                        /* BUG: pro_getattributes */
};

Pkg_private int
server_image_create_internal(parent, server_image_public, avlist, offset_ptr)
    Xv_opaque       	    parent;
    Server_image    	    server_image_public;
    Attr_avlist  	    avlist;
    int            	    *offset_ptr;
{

    register Server_image_info 		*server_image;
    register Attr_avlist    		attrs;
    register Pixrect *rpr = 
		&((Xv_server_image *) server_image_public)->pixrect;
    Xv_Drawable_info			*info;
    Xv_Screen				screen;

    DRAWABLE_INFO_MACRO(server_image_public, info);
    ((Xv_server_image *) server_image_public)->private_data =
	(Xv_opaque) xv_alloc(Server_image_info);

    if (!(server_image = SERVER_IMAGE_PRIVATE(server_image_public))) {
	xv_error(XV_ZERO,
		 ERROR_STRING, 
		 XV_MSG("Server image alloc failed"),
		 ERROR_PKG, SERVER_IMAGE,
		 NULL);
	return (XV_ERROR);
    }
    server_image->public_self = (Xv_opaque) rpr;
    screen = server_image->screen = (parent ? parent : xv_default_screen);
    server_image->save_pixmap = FALSE;
    *offset_ptr = xv_set_embedding_data(rpr, server_image_public);

    rpr->pr_ops = &server_image_ops;
    rpr->pr_depth = SERVER_IMAGE_DEFAULT_DEPTH;
    rpr->pr_width = SERVER_IMAGE_DEFAULT_WIDTH;
    rpr->pr_height = SERVER_IMAGE_DEFAULT_HEIGHT;

    /* A server_image returns an object that looks like a pixrect, We must
     * set a destroy handler on this object in order to deal with xv_destroy()
     * requests.
     */
    notify_set_destroy_func(server_image->public_self,
						(Notify_func)xv_destroy_status);

    for (attrs = avlist; *attrs; attrs = attr_next(attrs)) {
	switch ((int)attrs[0]) {
#ifdef OW_I18N
	    case SERVER_IMAGE_BITMAP_FILE_WCS:
#endif /* OW_I18N */
	    case SERVER_IMAGE_BITMAP_FILE: {
		Display *display;
		int	x_hot, y_hot, status;
		Pixmap  bitmap;
#ifdef OW_I18N
		char	*ct_file;

		if( attrs[0] == SERVER_IMAGE_BITMAP_FILE_WCS )
			ct_file = _xv_wcstombsdup((wchar_t *)attrs[1]);
		else
			ct_file = (char *)attrs[1];
#endif /* OW_I18N */
		display = 
		    (Display *)xv_get(xv_get(screen, SCREEN_SERVER), 
				    XV_DISPLAY);
		status = XReadBitmapFile(display,
			    xv_get(xv_get(screen, XV_ROOT), XV_XID),
#ifdef OW_I18N
			    ct_file,
#else
			    (char *)attrs[1],
#endif /* OW_I18N */
			    (unsigned *)(&rpr->pr_size.x),
			    (unsigned *)(&rpr->pr_size.y),
			    &bitmap,
			    &x_hot,
			    &y_hot);
#ifdef OW_I18N
		if( attrs[0] == SERVER_IMAGE_BITMAP_FILE_WCS && ct_file)
			free( ct_file );
#endif /* OW_I18N */
		if (status != BitmapSuccess) {
		    xv_error(XV_ZERO,
			ERROR_STRING, 
			XV_MSG("SERVER_IMAGE_BITMAP_FILE: Server image creation failed"),
			ERROR_PKG, SERVER_IMAGE,
			NULL);
		    return (XV_ERROR);
		}
		
		/* This is always a 1 bit deep bitmap */
		rpr->pr_depth = 1;

		xv_xid(info) = bitmap;
		xv_visual(info) = (Screen_visual *)xv_get(screen,
                    SCREEN_IMAGE_VISUAL, xv_xid(info), rpr->pr_depth);
		xv_plane_mask(info) = ~0;
		/*
		 * Mark this as a bitmap to distinguish from 1-plane windows.
		 * 1 is always foreground color in a bitmap
		 */
		xv_is_bitmap(info) = 1;
		xv_cms(info) = (Cms) xv_get(screen, SCREEN_DEFAULT_CMS); 
		xv_bg(info) = 0;
		xv_fg(info) = 1;

		ATTR_CONSUME(attrs[0]);
		break;
	    } 	

	    default:
		break;
	}
    }
		
    return (XV_OK);
}

Pkg_private     Xv_opaque
server_image_set_internal(server_image_public, avlist)
    Server_image    	  	server_image_public;
    Attr_attribute		*avlist;
{

    Attr_attribute	    *attrs;
    Server_image_info 	    *server_image = 
				SERVER_IMAGE_PRIVATE(server_image_public);
    Xv_Screen       	    screen = server_image->screen;
    short		    new_pixmap = FALSE, new_bits = FALSE;
    short		    new_x_bits = FALSE;
    Xv_Drawable_info 	    *info;
    Pixrect 		    *rpr = 
			&(((Xv_server_image *) server_image_public)->pixrect);

    DRAWABLE_INFO_MACRO(server_image_public, info);

    /* Pixmap id of 0 implies that the pixmap is yet to be created. */
    new_pixmap  = (xv_xid(info) == 0) ? TRUE : FALSE;

    for (attrs = avlist; *attrs; attrs = attr_next(attrs)) {
	switch (attrs[0]) {
	  case XV_WIDTH:
	    if (((int) attrs[1] != 0) && 
		    (rpr->pr_size.x != (int) attrs[1])) {
		rpr->pr_size.x = (int) attrs[1];
		new_pixmap = TRUE;
	    }
	    break;

	  case XV_HEIGHT:
	    if (((int) attrs[1] != 0) &&
		    (rpr->pr_size.y != (int) attrs[1])) {
		rpr->pr_size.y = (int) attrs[1];
		new_pixmap = TRUE;
	    }
	    break;

	  case SERVER_IMAGE_SAVE_PIXMAP:
	      server_image->save_pixmap = (int)attrs[1];
	      break;

	  case SERVER_IMAGE_PIXMAP: {
		Display		*display;
	      	unsigned int	width, height, depth, border_width;
	      	int		x, y;
	      	Window		root;

		if (xv_xid(info) != (Drawable)attrs[1]) {
	      	    display = (Display *) 
			    	xv_get(xv_get(screen, SCREEN_SERVER), XV_DISPLAY);

	      	    if (XGetGeometry(display, (Drawable)attrs[1], &root, &x, &y,
			&width, &height, &border_width, &depth) == 0) { 
		        xv_error(XV_ZERO,
		      	    ERROR_STRING,
		      	        XV_MSG("SERVER_IMAGE_PIXMAP: Unable to get geometry"),
		      	    ERROR_PKG, SERVER_IMAGE,
		      	    NULL);
		  	    return ((Xv_opaque) XV_ERROR);
	      	    }

	      	    if (!server_image->save_pixmap && xv_xid(info)) {
		        XFreePixmap(display, xv_xid(info));
		    }

		    rpr->pr_depth = depth;
		    rpr->pr_width = width;
		    rpr->pr_height = height;

		    xv_xid(info) = (Drawable)attrs[1];
		    xv_visual(info) = (Screen_visual *)xv_get(screen,
		        SCREEN_IMAGE_VISUAL, xv_xid(info), rpr->pr_depth);
		    xv_plane_mask(info) = ~0;
		    xv_is_bitmap(info) = (depth == 1) ? 1 : 0;
		}

		new_pixmap = FALSE;
		break;
	  }

	  case SERVER_IMAGE_DEPTH:
	    if (rpr->pr_depth != (int) attrs[1]) {
		rpr->pr_depth = (int) attrs[1];
		new_pixmap = TRUE;
	    }
	    break;

	  case SERVER_IMAGE_BITS:
	    if ((char *)attrs[1] != NULL) {
	        rpr->pr_data = (char *) attrs[1];
	        new_bits = TRUE;
	    }
	    break;

	  case SERVER_IMAGE_X_BITS:
	    if ((char *)attrs[1] != NULL) {
	        rpr->pr_data = (char *) attrs[1];
	        new_x_bits = TRUE;
	    }
	    break;

	  case SERVER_IMAGE_COLORMAP:{
		info->cms = (Cms) xv_find(screen, CMS, 
					  XV_AUTO_CREATE, FALSE,
					  CMS_NAME, (char *) attrs[1], 
					  NULL);
		if (info->cms == XV_ZERO) {
		    char            error[60];
		    sprintf(error, 
			XV_MSG("Colormap segment %s not found"), 
			(char *) attrs[1]);
		    xv_error(XV_ZERO,
			     ERROR_STRING, error,
			     ERROR_PKG, SERVER_IMAGE,
			     NULL);
		    info->cms = (Cms) xv_get(screen, SCREEN_DEFAULT_CMS);
		}
		info->bg = (unsigned long) xv_get(info->cms, CMS_BACKGROUND_PIXEL);
		info->fg = (unsigned long) xv_get(info->cms, CMS_FOREGROUND_PIXEL);
		break;
	    }

	  case SERVER_IMAGE_CMS:
	    if (attrs[1])
	      info->cms = (Cms)attrs[1];
	    break;
	    
	  default:
	    (void) xv_check_bad_attr(&xv_server_image_pkg, attrs[0]);
	    break;
	}
    }

    /*
     * A Pixmap is created under 2 conditions:
     *   1. A pixmap is yet to be created for this server image.
     *   2. The dimensions/depth of this pixmap has changed. Destroy the 
     *      current pixamp and allocate a new pixmap.
     */
    if (new_pixmap == TRUE) {
	Display        *display;

	display = (Display *) xv_get(xv_get(screen, SCREEN_SERVER), XV_DISPLAY);

	if (!server_image->save_pixmap && xv_xid(info)) {
	    XFreePixmap(display, xv_xid(info));
	}

	if (!(xv_xid(info) = 
		XCreatePixmap(display,
		    xv_get(xv_get(screen, XV_ROOT), XV_XID),
		    rpr->pr_size.x, rpr->pr_size.y, rpr->pr_depth))) {
	    		xv_error(XV_ZERO,
		     	    ERROR_STRING,
			    XV_MSG("Server Image creation failed"),
		     	    ERROR_PKG, SERVER_IMAGE,
		     	    NULL);
	     		return ((Xv_opaque) XV_ERROR);
	}

	xv_is_bitmap(info) = (rpr->pr_depth == 1) ? 1 : 0;
	xv_visual(info) = (Screen_visual *)xv_get(screen,
	    SCREEN_IMAGE_VISUAL, xv_xid(info), rpr->pr_depth);
	xv_plane_mask(info) = ~0;
	if (xv_cms(info) == XV_ZERO) {
	    xv_cms(info) = (Cms) xv_get(screen, SCREEN_DEFAULT_CMS);
	    xv_bg(info) = xv_is_bitmap(info) ? 0 : 
				xv_get(info->cms, CMS_BACKGROUND_PIXEL);
	    xv_fg(info) = xv_is_bitmap(info) ? 1 : 
				xv_get(info->cms, CMS_FOREGROUND_PIXEL);
	}
    } 

    if ((new_bits == TRUE) || (new_x_bits == TRUE)) {
	Pixrect        *pr;

	if (xv_cms(info) == XV_ZERO) {
	    xv_cms(info) = (Cms) xv_get(screen, SCREEN_DEFAULT_CMS);
	    xv_bg(info) = xv_is_bitmap(info) ? 0 : 
				xv_get(info->cms, CMS_BACKGROUND_PIXEL);
	    xv_fg(info) = xv_is_bitmap(info) ? 1 : 
				xv_get(info->cms, CMS_FOREGROUND_PIXEL);
	}

	pr = (Pixrect *) xv_mem_point(rpr->pr_width, rpr->pr_height,
                                      rpr->pr_depth, (short *) rpr->pr_data);
	xv_set_gc_op(xv_display(info), info,
	    	xv_gc(server_image_public, info), PIX_SRC,
		XV_USE_CMS_FG, XV_DEFAULT_FG_BG);

	XSetPlaneMask(xv_display(info), xv_gc(server_image_public, info),
			  (0x1 << rpr->pr_depth) - 1);

	xv_rop_mpr_internal(xv_display(info), xv_xid(info),
	    xv_gc(server_image_public, info), 0, 0, rpr->pr_width,
	    rpr->pr_height, pr, 0, 0, info,
	    (new_bits == TRUE) ? TRUE : FALSE);

	xv_free((char *) pr);
    }
    return ((Xv_opaque) XV_OK);
}


/*ARGSUSED*/
Pkg_private Xv_opaque
server_image_get_internal(server_image_public, status, attr, args)
    Server_image    server_image_public;
    int            *status;
    Server_image_attribute attr;
    va_list         args;
{

    Server_image_info	*server_image =
			    SERVER_IMAGE_PRIVATE(server_image_public);
    register Pixrect	*rpr = 
			    &(((Xv_server_image *) server_image_public)->pixrect);
    Xv_opaque		server;
    Xv_opaque		rv;

    switch (attr) {
      case XV_DISPLAY:
	server = xv_get(server_image->screen, SCREEN_SERVER);
	rv = (Xv_opaque) xv_get(server, XV_DISPLAY);
	break;
      case XV_WIDTH:
	rv = (Xv_opaque) rpr->pr_size.x;
	break;

      case XV_HEIGHT:
	rv = (Xv_opaque) rpr->pr_size.y;
	break;

      case SERVER_IMAGE_DEPTH:
	rv = (Xv_opaque) rpr->pr_depth;
	break;

      case SERVER_IMAGE_BITS:
	rv = (Xv_opaque) rpr->pr_data;
	break;

      case SERVER_IMAGE_PIXMAP:
	rv = (Xv_opaque)xv_get(server_image_public, XV_XID);
	break;

      case SERVER_IMAGE_SAVE_PIXMAP:
	rv = (Xv_opaque)server_image->save_pixmap;
	break;

      case SERVER_IMAGE_COLORMAP: {
	  Xv_Drawable_info *info;
	  
	  DRAWABLE_INFO_MACRO(server_image_public, info);
	  rv = (Xv_opaque)xv_get(xv_cms(info), CMS_NAME);
      }
      break;
	
      case SERVER_IMAGE_CMS: {
	  Xv_Drawable_info *info;
	  
	  DRAWABLE_INFO_MACRO(server_image_public, info);
	  rv = (Xv_opaque)xv_cms(info);
      }
      break;
	
      default:
	if (xv_check_bad_attr(SERVER_IMAGE, attr) == XV_ERROR) {
	    *status = XV_ERROR;
	}
	rv = (Xv_opaque) NULL;
	break;
    }
    return rv;
}

Pkg_private int
server_image_destroy_internal(server_image_public, status)
    Server_image    server_image_public;
    Destroy_status  status;
{

    register Server_image_info *image = SERVER_IMAGE_PRIVATE(server_image_public);
    Xv_Drawable_info *info;

    if ((status == DESTROY_CHECKING) || (status == DESTROY_PROCESS_DEATH) ||
	(status == DESTROY_SAVE_YOURSELF))
	return XV_OK;

    DRAWABLE_INFO_MACRO(server_image_public, info);
    XFreePixmap(xv_display(info), xv_xid(info));
    free((char *) image);
    return XV_OK;
}

Pkg_private int
server_image_destroy(pr)
    Pixrect        *pr;
{
    Xv_opaque       pr_object;

    XV_OBJECT_TO_STANDARD((Xv_opaque) pr, "server_image_destroy",
			  pr_object);
    xv_destroy(pr_object);
    return XV_OK;
}
