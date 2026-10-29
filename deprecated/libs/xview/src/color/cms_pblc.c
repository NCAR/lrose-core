#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)cms_pblc.c 1.21 91/03/21";
#endif
#endif

#include <xview/cms.h>
#include <xview/notify.h>
#ifdef OW_I18N
#include <xview/xv_i18n.h>
#endif /* OW_I18N */
#include <xview_private/cms_impl.h>
#include <xview_private/i18n_impl.h>
#include <xview_private/portable.h>
#include <olgx/olgx.h>

/*
 *      (c) Copyright 1989 Sun Microsystems, Inc. Sun design patents
 *      pending in the U.S. and foreign countries. See LEGAL NOTICE
 *      file for terms of the license.
 */

Pkg_private int
cms_init(parent, cms_public, avlist)
    Xv_Screen		parent;
    Cms			cms_public;
    Attr_avlist     	avlist;
{
    Cms_info			*cms = NULL;
    Xv_cms_struct		*cms_object;
    register Attr_avlist 	attrs;
    register int 		i;
    XVisualInfo		template;
    long		mask = 0;

    cms = (Cms_info *)xv_alloc(Cms_info);
    cms->public_self = cms_public;
    cms_object = (Xv_cms_struct *)cms_public;
    cms_object->private_data = (Xv_opaque)cms;

    cms->size = 0;
    cms->screen = parent ? parent : xv_default_screen;
    cms->type = XV_STATIC_CMS;
    cms->visual = (Screen_visual *)xv_get(cms->screen, SCREEN_DEFAULT_VISUAL);

    for (attrs = avlist; *attrs; attrs = attr_next(attrs)) {
	switch ((int)attrs[0]) {
	  case CMS_TYPE:
	    cms->type = (Cms_type)attrs[1];
	    ATTR_CONSUME(attrs[0]);
	    break;
	    
	  case CMS_DEFAULT_CMS:
	    if (attrs[1]) {
              STATUS_SET(cms, default_cms);
	    } else {
              STATUS_RESET(cms, default_cms);
	    }
	    ATTR_CONSUME(attrs[0]);
	    break;
	    
	  case CMS_CONTROL_CMS:
	    if (attrs[1]) {
		STATUS_SET(cms, control_cms);
	    } else {
		STATUS_RESET(cms, control_cms);
	    }
	    ATTR_CONSUME(attrs[0]);
	    break;
	    
	  case CMS_SIZE:
	    if (attrs[1] != 0) {
	      cms->size = (unsigned long)attrs[1];
            }
	    ATTR_CONSUME(attrs[0]);
	    break;
	    
	  case XV_VISUAL_CLASS:
	    template.class = (int)attrs[1];
	    mask |= VisualClassMask;
	    ATTR_CONSUME(attrs[0]);
	    break;
	    
	  case XV_DEPTH:
	    template.depth = (int)attrs[1];
	    mask |= VisualDepthMask;
	    ATTR_CONSUME(attrs[0]);
	    break;

	  case XV_VISUAL:
	    if (attrs[1]) {
		template.visualid = XVisualIDFromVisual((Visual *)attrs[1]);
		mask |= VisualIDMask;
	    }
	    break;

	  default:
	    break;
	}
    }

    if (mask) {
	Screen_visual *visual;
	
	visual = (Screen_visual *)xv_get(cms->screen, SCREEN_VISUAL, mask, &template);
	if (visual)
	  cms->visual = visual;
    }
    
    /* 
     * Check to see if they are trying to create a dynamic cms from a 
     * static visual.
     */
    if (cms->type == XV_DYNAMIC_CMS && !XV_DYNAMIC_VISUAL(cms->visual->vinfo->class)) {
	xv_error(XV_ZERO, 
		 ERROR_STRING,
		 XV_MSG("Can not allocate a read/write cms from a static visual"),
		 ERROR_PKG, CMS,
		 NULL);
	return(XV_ERROR);
    } else {
	if (cms->size == 0)
	  cms->size = STATUS(cms, control_cms) ? CMS_CONTROL_COLORS : XV_DEFAULT_CMS_SIZE;
	cms->index_table =
	  xv_alloc_n(unsigned long, cms->size);
	if (cms->type == XV_STATIC_CMS) {
	    for (i = 0; i <= cms->size - 1; i++)
	      cms->index_table[i] = XV_INVALID_PIXEL;
	}
    }
    return(XV_OK);
}

Pkg_private Xv_opaque
cms_set_avlist(cms_public, avlist)
    Cms       	    cms_public;
    Attr_attribute  avlist[];
{
    register Cms_info	    *cms = CMS_PRIVATE(cms_public);
    register Attr_avlist    attrs;
    unsigned long	    cms_index, cms_count;
    Xv_Singlecolor	    *colors = NULL;
    XColor	    	    *xcolors = NULL, *cms_parse_named_colors();
    char		    **named_colors = NULL;

    /* defaults */
    if (STATUS(cms, control_cms)) {
	cms_index = CMS_CONTROL_COLORS;
	cms_count = cms->size - CMS_CONTROL_COLORS;
    } else {
	cms_index = 0;
	cms_count = cms->size;
    }

    for (attrs = avlist; *attrs; attrs = attr_next(attrs)) {
	switch (attrs[0]) {
	    case CMS_NAME:
		cms_set_name(cms, (char *)attrs[1]);
		ATTR_CONSUME(attrs[0]);
		break;

	    case CMS_COLORS:
		colors = (Xv_Singlecolor *)attrs[1];
		ATTR_CONSUME(attrs[0]);
		break;

	    case CMS_X_COLORS:
		xcolors = (XColor *)attrs[1];
		ATTR_CONSUME(attrs[0]);
		break;

	    case CMS_NAMED_COLORS:
		named_colors = (char **)&attrs[1];
		break;

	    case CMS_INDEX:
		cms_index = (int)attrs[1];
		ATTR_CONSUME(attrs[0]);
		break;

	    case CMS_COLOR_COUNT:
		cms_count = (int)attrs[1];
		ATTR_CONSUME(attrs[0]);
		break;

	    case CMS_TYPE:
		xv_error(XV_ZERO,
		         ERROR_STRING, 
				XV_MSG("CMS_TYPE is a create-only attribute"),
		    	 ERROR_PKG, CMS,
		    	 NULL);
		return((Xv_opaque)XV_ERROR);

	    case CMS_FRAME_CMS:
		if (attrs[1]) {
		    STATUS_SET(cms, frame_cms);
		} else {
		    STATUS_RESET(cms, frame_cms);
		}
		ATTR_CONSUME(attrs[0]);
		break;

	    case XV_END_CREATE:
		if (cms->name == NULL) {
		    cms_set_unique_name(cms);
	 	}

		if (STATUS(cms, control_cms)) {
		    register int    i;
		    char	    *control_color, *defaults_get_string();
    		    XColor	    *xcolors = NULL;
		    Display         *display;
		    Colormap	     cmap;

		    /*
              	     * Get the control color from OpenWindows.WindowColor.
              	     * Index   color   
              	     *   0     control color         (BG1 80% grey by default)
              	     *   1     90% of control color. (BG2)
              	     *   2     50% of control color  (BG3)
              	     *   3     120% of control color (WHITE)
		     *        Note: the white is not the white defined
		     *              by Open Look, but it looks better,
		     *              and is consistent with the way olwm
		     *              handles it.
              	     */
 
            	    xcolors = xv_alloc_n(XColor, 4 );
            	    control_color = (char *)
			defaults_get_string("openWindows.windowColor",
                        	"OpenWindows.WindowColor", "#cccccc");

		    display = (Display *) 
		      xv_get(xv_get(cms->screen, SCREEN_SERVER, NULL), XV_DISPLAY, NULL);

		    /* Can't use the cms->cmap, because it may not be allocated */
		    cmap = DefaultColormap(display,
					   (int)xv_get(cms->screen, SCREEN_NUMBER, NULL));
		    if (!XParseColor(display, cmap, 
				     control_color, &(xcolors[0]))) {
			xv_error(XV_ZERO, 
				 ERROR_STRING,
				 "Unable to parse window color",
				 ERROR_PKG, CMS,
				 NULL);
			xcolors[CMS_CONTROL_BG1].red = 0xcccc;
			xcolors[CMS_CONTROL_BG1].green = 0xcccc;
			xcolors[CMS_CONTROL_BG1].blue = 0xcccc;
		    }			

		    olgx_calculate_3Dcolors((XColor *)NULL, 
					    &(xcolors[CMS_CONTROL_BG1]),
					    &(xcolors[CMS_CONTROL_BG2]),
					    &(xcolors[CMS_CONTROL_BG3]), 
					    &(xcolors[CMS_CONTROL_HIGHLIGHT]));
	     
            	    for (i = CMS_CONTROL_BG1; i <= CMS_CONTROL_HIGHLIGHT; i++)
		      xcolors[i].flags = (DoRed | DoGreen | DoBlue);

		    if (cms_set_colors(cms, (Xv_Singlecolor *)NULL, xcolors, 
			    (unsigned long)0, (unsigned long)CMS_CONTROL_COLORS) == XV_ERROR) {
			xv_error(XV_ZERO,
			    ERROR_STRING,
			    XV_MSG("Unable to allocate control colors for colormap segment"),
			    ERROR_PKG, CMS,
			    NULL);
			xv_free(xcolors);
	    		return((Xv_opaque)XV_ERROR);
		    }
		    xv_free(xcolors);
		    xcolors = NULL;
		}

		if (cms->cmap == NULL) {
		    /*
		     * Colors for this cms were not specified as part 
		     * of the create. Allocate enough cells for this 
		     * colormap segment however and bind it to a 
		     * X11 colormap.
		     */
		    cms_set_colors(cms, NULL, NULL, 0, cms->size);
		}
		break;

	    default:
		xv_check_bad_attr(&xv_cms_pkg, attrs[0]);
		break;
	}
    }

    if (named_colors) {
	xcolors = cms_parse_named_colors(cms, named_colors);
    }

    if (colors || xcolors) {
	if (cms_set_colors(cms, colors, xcolors, cms_index, cms_count)
	    == XV_ERROR) {
	    return((Xv_opaque)XV_ERROR);
	}
    }

    if (named_colors && xcolors) {
    	/* free up memory that was allocated for ASCII to RGB conversion */
	xv_free(xcolors);
    }

    return((Xv_opaque)XV_OK);
}


Pkg_private Xv_opaque
cms_get_attr(cms_public, status, attr, args)
    Cms			cms_public;
    int			*status;
    Cms_attribute 	attr;
    va_list         	args;
{
    Cms_info		*cms = CMS_PRIVATE(cms_public);
    Xv_opaque		value;
/* Alpha compatibility, mbuck@debian.org */
#if 0
    Attr_avlist     	avlist = (Attr_avlist) args;
#endif
    int			cms_status = 0;

    switch (attr) {
      case CMS_PIXEL: {
	  unsigned long    index;
	  
/* Alpha compatibility, mbuck@debian.org */
#if 1
	  index = va_arg(args, unsigned long);
#else
	  index = (unsigned long)avlist[0];
#endif
	  if (index >= cms->size) {
	      index = cms->size - 1;
	  } else if (index < 0) {
	      index = 0;
	  }
	  value = (Xv_opaque)cms->index_table[index];
	  break;
      }

      case CMS_SIZE:
	value = (Xv_opaque)cms->size;
	break;
	
      case CMS_STATUS_BITS:
	if (STATUS(cms, default_cms)) {
	    cms_status = STATUS(cms, default_cms) << CMS_STATUS_DEFAULT;
	} else if (STATUS(cms, control_cms)) {
	    cms_status = STATUS(cms, control_cms) << CMS_STATUS_CONTROL;
	} else if (STATUS(cms, frame_cms)) {
	    cms_status = STATUS(cms, frame_cms)   << CMS_STATUS_FRAME;
	}
	value = (Xv_opaque)cms_status;
	break;
	
      case CMS_INDEX_TABLE:
	value = (Xv_opaque)cms->index_table;
	break;
	
      case CMS_CONTROL_CMS:
	value = STATUS(cms, control_cms);
	break;

      case CMS_DEFAULT_CMS:
	value = STATUS(cms, default_cms);
	break;
	
      case CMS_NAME:
	value = (Xv_opaque)cms->name;
	break;
	
      case CMS_TYPE:
	value = (Xv_opaque)cms->type;
	break;

      case CMS_FOREGROUND_PIXEL:
	value = (Xv_opaque)cms->index_table[cms->size - 1];
	break;
	
      case CMS_BACKGROUND_PIXEL:
	value = (Xv_opaque)cms->index_table[0];
	break;
	
      case CMS_SCREEN:
	value = (Xv_opaque)cms->screen;
	break;
	
      case XV_VISUAL:
	value = (Xv_opaque)cms->visual->vinfo->visual;
	break;
	
      case XV_VISUAL_CLASS:
	value = (Xv_opaque)cms->visual->vinfo->class;
	break;
	
      case XV_DEPTH:
	value = (Xv_opaque)cms->visual->vinfo->depth;
	break;
	
      case XV_XID:	/* public */
      case CMS_CMAP_ID: /* private that should be removed */
	value = (Xv_opaque)cms->cmap->id;
	break;
	
      case CMS_COLORS:  
/* Alpha compatibility, mbuck@debian.org */
#if 1
	{
	    Xv_singlecolor *v = va_arg(args, Xv_singlecolor *);
	
	    if (cms_get_colors(cms, (unsigned long)0, cms->size, 
			       v, (XColor *)NULL,
			       (unsigned char *)NULL, (unsigned char *)NULL,
			       (unsigned char *)NULL) == XV_OK) {
		value = (Xv_opaque)v;
	    } else {
		value = XV_ZERO;
	    }
	}
#else
	if (cms_get_colors(cms, (unsigned long)0, cms->size, 
			   (Xv_singlecolor *)avlist[0], (XColor *)NULL,
			   (unsigned char *)NULL, (unsigned char *)NULL,
			   (unsigned char *)NULL) == XV_OK) {
	    value = (Xv_opaque)avlist[0];
	} else {
	    value = NULL;
	}
#endif
	break;
	
      case CMS_X_COLORS:  
/* Alpha compatibility, mbuck@debian.org */
#if 1
	{
	    XColor *v = va_arg(args, XColor *);
	    
	    if (cms_get_colors(cms, (unsigned long)0, cms->size, 
			       (Xv_singlecolor *)NULL, v,
			       (unsigned char *)NULL, (unsigned char *)NULL,
			       (unsigned char *)NULL) == XV_OK) {
		value = (Xv_opaque)v;
	    } else {
		value = XV_ZERO;
	    }
	}
#else
	if (cms_get_colors(cms, (unsigned long)0, cms->size, 
			   (Xv_singlecolor *)NULL, (XColor *)avlist[0],
			   (unsigned char *)NULL, (unsigned char *)NULL,
			   (unsigned char *)NULL) == XV_OK) {
	    value = (Xv_opaque)avlist[0];
	} else {
	    value = NULL;
	}
#endif
	break;
	
      case CMS_CMS_DATA: {
/* Alpha compatibility, mbuck@debian.org */
#if 1
	  Xv_cmsdata	    *cms_data = va_arg(args, Xv_cmsdata *);
#else
	  Xv_cmsdata	    *cms_data = (Xv_cmsdata *)avlist[0];
#endif
	  
	  cms_data->type = cms->type;
	  cms_data->size = cms->size;
	  cms_data->index = 0;
	  cms_data->rgb_count = cms->size;
	  cms_get_colors(cms, (unsigned long)0, cms->size, 
			 (Xv_singlecolor *)NULL, (XColor *)NULL, cms_data->red,
			 cms_data->green, cms_data->blue);
	  value = (Xv_opaque)cms_data;
	  break;
      }
	
      case CMS_FRAME_CMS:
	value = STATUS(cms, frame_cms);
	break;
	
      case CMS_CMAP_TYPE:
	value = (Xv_opaque)cms->cmap->type;
	break;
	
      default:
	if (xv_check_bad_attr(&xv_cms_pkg, attr) == XV_ERROR) {
	    *status = XV_ERROR;
	}
    }
    
    return(value);
}

/* ARGSUSED */
Pkg_private Xv_object
cms_find_cms(screen_public, pkg, avlist)
    Xv_opaque       screen_public;
    Xv_pkg         *pkg;
    Attr_avlist     avlist;
{
    Xv_Screen screen;
    register Attr_avlist    attrs;
    Xv_Colormap             *cmap;
    Cms_info                *cms;
    Screen_visual	    *visual;

    if (!screen_public)
      screen = xv_default_screen;
    else
      screen = screen_public;
    
    for (attrs = avlist; *attrs; attrs = attr_next(attrs)) {
	switch ((int)attrs[0]) {
	  case CMS_NAME:
	    visual = (Screen_visual *)xv_get(screen, SCREEN_DEFAULT_VISUAL);
	    while (visual != (Screen_visual *)NULL) {
                for (cmap = (Xv_Colormap *)visual->colormaps;
		     cmap != NULL; 
		     cmap = cmap->next) {
                    for (cms = cmap->cms_list; cms != NULL; cms = cms->next) {
                        if (!strcmp(cms->name, (char *) attrs[1])) 
			    return(CMS_PUBLIC(cms));
                    }
                }
		visual = visual->next;
	    }
	    break;
		
	  default:
	    break;
	}
    }
    return(XV_ZERO);
}

Pkg_private int
cms_destroy(cms_public, status)
    Cms 		cms_public;
    Destroy_status  	status;
{
    Cms_info		*cms = CMS_PRIVATE(cms_public);
    Cms_info		*cms_list;
    Xv_Colormap		*cmap_list;

    /* default cms cannot be freed until the application is terminated */
    if (STATUS(cms, default_cms))
      return(XV_OK);

    if (status == DESTROY_CLEANUP) {
        Xv_opaque		server;
        Display		*display;

        server = xv_get(cms->screen, SCREEN_SERVER);
        display = (Display *)xv_get(server, XV_DISPLAY);

	xv_free(cms->name);
	cms_free_colors(display, cms);
	xv_free(cms->index_table);

	/* remove from colormap's cms list */
	if (cms == cms->cmap->cms_list) {
	    cms->cmap->cms_list = cms->next;
	} else {
	    for (cms_list = cms->cmap->cms_list; cms_list->next != NULL; 
		 cms_list = cms_list->next) {
		if (cms_list->next == cms) {
		    cms_list->next = cms_list->next->next;
		    break;
		}
	    }
	}
	
	/* check if colormap is to be freed */
	if (cms->cmap->cms_list == NULL) {
	    XFreeColormap(display, cms->cmap->id);
	    
	    cmap_list = (Xv_Colormap *)cms->visual->colormaps;
	    
	    if (cms->cmap == cmap_list) {
		cms->visual->colormaps = (Xv_opaque)cms->cmap->next;
	    } else {
		for ( ; cmap_list->next != NULL ; cmap_list = cmap_list->next) {
		    if (cmap_list->next == cms->cmap) {
			cmap_list->next = cmap_list->next->next;
			break;
		    }
		}
	    }
	    xv_free(cms->cmap);
	}
	
	xv_free(cms);
    }
    
    return(XV_OK);
}
