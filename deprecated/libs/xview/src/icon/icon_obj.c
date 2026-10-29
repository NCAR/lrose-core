#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)icon_obj.c 20.33 90/02/26";
#endif
#endif

/***********************************************************************/
/* icon_obj.c                               */
/*	
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE 
 *	file for terms of the license. 
 */
/***********************************************************************/

#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <pixrect/pixrect.h>
#include <xview_private/i18n_impl.h>
#include <xview_private/portable.h>
#include <xview/rect.h>
#include <xview/rectlist.h>
#include <xview/win_input.h>
#include <xview_private/icon_impl.h>
#include <xview/screen.h>
#include <xview/wmgr.h>
#include <xview/notify.h>
#include <xview_private/draw_impl.h>
#include <xview/defaults.h>

/*
 * Public
 */
extern Notify_value icon_input();


/*
 * private to module 
 */
static void icon_set_wrk_space_color();


/*****************************************************************************/
/* icon_create                                                               */
/*****************************************************************************/

Icon
#ifdef ANSI_FUNC_PROTO
icon_create(Attr_attribute attr1, ...)
#else
icon_create(attr1, va_alist)
    Attr_attribute attr1;
va_dcl
#endif
{
    Attr_attribute  avlist[ATTR_STANDARD_SIZE];
    va_list         valist;

    if( attr1 )
    {
        VA_START(valist, attr1);
        copy_va_to_av( valist, avlist, attr1 );
        va_end(valist);
    }
    else
        avlist[0] = XV_ZERO;

    return (Icon) xv_create_avlist(XV_ZERO, ICON, avlist);
}

/*ARGSUSED*/
int
icon_init(parent, object, avlist)
    Xv_opaque       parent;
    Xv_opaque       object;
    Xv_opaque      *avlist;
{
    register Xv_icon_info *icon;
    Rect            recticon;

    ((Xv_icon *) (object))->private_data = (Xv_opaque) xv_alloc(Xv_icon_info);
    if (!(icon = ICON_PRIVATE(object))) {
	xv_error(object,
		 ERROR_LAYER, ERROR_SYSTEM,
		 ERROR_STRING, 
		    XV_MSG("Can't allocate icon structure"),
		 ERROR_PKG, ICON,
		 NULL);
	return XV_ERROR;
    }
    icon->public_self = object;
    icon->ic_gfxrect.r_width = 64;
    icon->ic_gfxrect.r_height = 64;
    rect_construct(&recticon, 0, 0, 64, 64);
    icon->workspace_color = (char *)malloc (sizeof(char)*12);
    xv_set(object,
	   XV_SHOW, FALSE,
	   WIN_CONSUME_EVENT, WIN_REPAINT,
	   WIN_NOTIFY_SAFE_EVENT_PROC, icon_input,
	   WIN_NOTIFY_IMMEDIATE_EVENT_PROC, icon_input,
	   WIN_RECT, &recticon,
	   NULL);
    return XV_OK;
}

/*****************************************************************************/
/* icon_destroy	                                                             */
/*****************************************************************************/
icon_destroy(icon_public)
    Icon            icon_public;
{
    xv_destroy(icon_public);
}


/*****************************************************************************/
/* icon_destroy_internal						     */
/*****************************************************************************/

int
icon_destroy_internal(icon_public, status)
    Icon            icon_public;
    Destroy_status  status;
{

    Xv_icon_info   *icon = ICON_PRIVATE(icon_public);

    if ((status == DESTROY_CHECKING) || (status == DESTROY_SAVE_YOURSELF))
	return XV_OK;

    free((char *) (icon->workspace_color));
    free((char *) icon);

    return XV_OK;
}

/*****************************************************************************/
/* icon_set                                                                  */
/*****************************************************************************/

int
#ifdef ANSI_FUNC_PROTO
icon_set(Icon icon_public, ...)
#else
icon_set(icon_public, va_alist)
    Icon            icon_public;
va_dcl
#endif
{
    AVLIST_DECL;
    va_list         valist;

    VA_START(valist, icon_public);
    MAKE_AVLIST( valist, avlist );
    va_end(valist);
    return (int) xv_set_avlist(icon_public, avlist);
}

Xv_opaque
icon_set_internal(icon_public, avlist)
    Icon            icon_public;
    register Attr_avlist avlist;
{
    register Xv_icon_info *icon = ICON_PRIVATE(icon_public);
    register Xv_opaque arg1;
    short repaint = FALSE;
    short label_changed = FALSE;
    short change_color = FALSE;
    
    for (; *avlist; avlist = attr_next(avlist)) {
	arg1 = avlist[1];
	switch (*avlist) {

	  case ICON_WIDTH:
	    icon->ic_gfxrect.r_width = (short) arg1;
	    repaint = TRUE;
	    break;

	  case ICON_HEIGHT:
	    icon->ic_gfxrect.r_height = (short) arg1;
	    repaint = TRUE;
	    break;

	  case ICON_IMAGE:
	    icon->ic_mpr = (struct pixrect *) arg1;
	    repaint = TRUE;
	    break;

	  case ICON_IMAGE_RECT:
	    if (arg1) {
		icon->ic_gfxrect = *(Rect *) arg1;
		repaint = TRUE;
	    }
	    break;

	  case ICON_LABEL_RECT:
	    if (arg1) {
		icon->ic_textrect = *(Rect *) arg1;
		repaint = TRUE;
	    }
	    break;

	  case XV_LABEL:
	    /* Consume attribute so that generic handler not also invoked. */
	    *avlist = (Xv_opaque) ATTR_NOP(*avlist);
#ifdef OW_I18N
            if (icon->ic_text_wcs)
                xv_free(icon->ic_text_wcs);
            if (icon->ic_text) {
                xv_free(icon->ic_text);
                icon->ic_text = NULL;
            }
            if ((char *) arg1)
                icon->ic_text_wcs = (wchar_t *)_xv_mbstowcsdup((char *)arg1);
#else
	    if ( icon->ic_text )
	        free( icon->ic_text );
	    if ( (char *) arg1 ) {
		icon->ic_text = (char *) xv_calloc( 1, strlen((char *) arg1) + 1 );
		strcpy( icon->ic_text, (char *) arg1 );
	    }
#endif
	    label_changed = TRUE;
	    repaint = TRUE;
	    icon->ic_flags &= (~ICON_TRANSLABEL);  /* set the flag to 0 */
	    break;

#ifdef OW_I18N
          case XV_LABEL_WCS:
            /* Consume attribute so that generic handler not also invoked. */
            *avlist = (Xv_opaque) ATTR_NOP(*avlist);
            if (icon->ic_text_wcs)
                xv_free (icon->ic_text_wcs);
            if (icon->ic_text) {
                free(icon->ic_text);
                icon->ic_text = NULL;
            }
            if ((wchar_t *) arg1)
                icon->ic_text_wcs = wsdup((wchar_t *)arg1);
            label_changed = TRUE;
            repaint = TRUE;
            icon->ic_flags &= (~ICON_TRANSLABEL);  /* set the flag to 0 */
            break;
#endif /* OW_I18N */
	  case XV_OWNER:{

		/*
		 * Consume attribute so that generic handler not also
		 * invoked.
		 */
		*avlist = (Xv_opaque) ATTR_NOP(*avlist);
		icon->frame = arg1;
		break;
	    }

	  case WIN_CMS_CHANGE:
	    repaint = TRUE;
	    break;

	  case ICON_TRANSPARENT:
	    if (arg1)
	      icon->ic_flags |= ICON_BKGDTRANS;
	    else 
	      icon->ic_flags &= (~ICON_BKGDTRANS);
	    repaint = TRUE;
	    change_color = TRUE;
	    break;
	    
	  case ICON_MASK_IMAGE:
	    icon->ic_mask = (Server_image) arg1;
	    change_color = TRUE;
	    repaint = TRUE;
	    break;

	  case ICON_TRANSPARENT_LABEL:
#ifdef OW_I18N
            if (icon->ic_text_wcs)
                xv_free(icon->ic_text_wcs);
            if (icon->ic_text) {
                xv_free(icon->ic_text);
                icon->ic_text = NULL;
            }
            if ((char *) arg1)
                icon->ic_text_wcs = (wchar_t *)_xv_mbstowcsdup((char *)arg1);
#else 
	    if ( icon->ic_text )
	        free( icon->ic_text );
	    if ( (char *) arg1 ) {
		icon->ic_text = (char *) xv_calloc( 1, strlen((char *) arg1) + 1 );
		strcpy( icon->ic_text, (char *) arg1 );
	    }
#endif
	    label_changed = TRUE;
	    icon->ic_flags |= ICON_TRANSLABEL;
	    repaint = TRUE;
	    break;

#ifdef OW_I18N
          case ICON_TRANSPARENT_LABEL_WCS:
            if (icon->ic_text_wcs)
                xv_free(icon->ic_text_wcs);
            if (icon->ic_text) {
                xv_free(icon->ic_text);
                icon->ic_text = NULL;
            }
            if ((wchar_t *) arg1)
                icon->ic_text_wcs = wsdup((wchar_t *)arg1);
            label_changed = TRUE;
            icon->ic_flags |= ICON_TRANSLABEL;
            repaint = TRUE;
            break;
#endif /* OW_I18N */

	  case XV_END_CREATE:
	    /*
	     * reparent the icon to force it to be child of root.
	     */
	    xv_set(icon_public,WIN_PARENT,xv_get(icon_public,XV_ROOT),
		   NULL);
	    break;
          default:
	    if (xv_check_bad_attr(ICON, *avlist) == XV_OK) {
		return *avlist;
	    }
	    break;
	}
    }

    /*
     * tell the window manager the new icon name, this provides a fall-back
     * for window managers whose ideas about icons differ widely from those
     * of the client.
     */
    if (label_changed && icon->frame) {
	Xv_Drawable_info *info;
	DRAWABLE_INFO_MACRO(icon->frame, info);
	XSetIconName(xv_display(info), xv_xid(info), icon->ic_text);
    }
    
    if ( change_color )
      icon_set_wrk_space_color( icon_public );
    
    if (repaint && icon->ic_flags & ICON_PAINTED)
      icon_display(icon_public, 0, 0);

    return (Xv_opaque)XV_OK;
}

/*****************************************************************************/
/* icon_get                                                                  */
/*****************************************************************************/

Xv_opaque
icon_get(icon_public, attr)
    register Icon   icon_public;
    Icon_attribute  attr;
{
    return xv_get(icon_public, attr);
}


/*ARGSUSED*/
Xv_opaque
icon_get_internal(icon_public, status, attr, args)
    Icon            icon_public;
    int            *status;
    Attr_attribute  attr;
    va_list         args;
{
    Xv_icon_info   *icon = ICON_PRIVATE(icon_public);

    switch (attr) {

      case ICON_WIDTH:
	return (Xv_opaque) icon->ic_gfxrect.r_width;

      case ICON_HEIGHT:
	return (Xv_opaque) icon->ic_gfxrect.r_height;

      case ICON_IMAGE:
	return (Xv_opaque) icon->ic_mpr;

      case ICON_IMAGE_RECT:
	return (Xv_opaque) & (icon->ic_gfxrect);

      case ICON_LABEL_RECT:
	return (Xv_opaque) & (icon->ic_textrect);

      case XV_LABEL:
#ifdef OW_I18N
        if (icon->ic_text == NULL && icon->ic_text_wcs != NULL)
            icon->ic_text = (char *)_xv_wcstombsdup((wchar_t *)icon->ic_text_wcs);
#endif
	return (Xv_opaque) icon->ic_text;

#ifdef OW_I18N
      case XV_LABEL_WCS:
        return (Xv_opaque) icon->ic_text_wcs;
#endif

      case XV_OWNER:
	return (Xv_opaque) icon->frame;

      case ICON_TRANSPARENT_LABEL:
#ifdef OW_I18N
        if (icon->ic_text == NULL && icon->ic_text_wcs != NULL)
            icon->ic_text = (char *)_xv_wcstombsdup((wchar_t *)icon->ic_text_wcs);
#endif
	return (Xv_opaque) icon->ic_text;

#ifdef OW_I18N
      case ICON_TRANSPARENT_LABEL_WCS:
        return (Xv_opaque) icon->ic_text_wcs;
#endif

      case ICON_TRANSPARENT:
	return (Xv_opaque) (icon->ic_flags & ICON_BKGDTRANS);

      case ICON_MASK_IMAGE:
	return (Xv_opaque) icon->ic_mask;

      default:
	if (xv_check_bad_attr(ICON, attr) == XV_ERROR) {
	    *status = XV_ERROR;
	}
	return (Xv_opaque) NULL;
    }
    /* NOTREACHED */
}

static void
icon_set_wrk_space_color( icon_public )
Icon            icon_public;
{	
    Xv_icon_info *icon = ICON_PRIVATE( icon_public );
    Display    *display;
    XID        xid;
    Colormap   cmap;
    register Xv_Drawable_info  *info;
    char       *color_name;
    XColor     color;
    int valid_color = FALSE;

    color_name = defaults_get_string( "openWindows.workspaceColor",
				      "OpenWindows.WorkspaceColor", 
				      "#cccccc" );

    if (strcmp(color_name,icon->workspace_color) == 0) /* no change */ 
      return;
    else
      strncpy(icon->workspace_color,color_name,12);
    
    DRAWABLE_INFO_MACRO( icon_public, info );
    display = xv_display( info );
    xid = (XID)xv_xid(info);
    cmap = xv_get( xv_cms(info), XV_XID );

    if ( strlen( color_name ) )
      if ( !XParseColor( display, cmap, color_name, &color ) ) {
	  char msg[100];
	  
	  sprintf( msg,
		  XV_MSG("icon: color name \"%s\" not in database"),
		  color_name );
	  xv_error( XV_ZERO, ERROR_SEVERITY, ERROR_RECOVERABLE,
		   ERROR_STRING, msg,
		   ERROR_PKG, ICON,
		   NULL );
      } else if ( !XAllocColor( display, cmap, &color ) )  {
	  xv_error( XV_ZERO, ERROR_SEVERITY, ERROR_RECOVERABLE,
		   ERROR_STRING, 
		   XV_MSG("icon: all color cells are allocated"),
		   ERROR_PKG, ICON,
		   NULL );	
      } else {
	  valid_color = TRUE;
      }
    
    if ( valid_color )
      icon->workspace_pixel = color.pixel;
    else 
      icon->workspace_pixel = (unsigned long) xv_get( xv_cms( info ), CMS_BACKGROUND_PIXEL );
    XSetWindowBackground( display, xid, icon->workspace_pixel );
}
