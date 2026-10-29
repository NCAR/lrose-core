#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)cms.c 1.17 91/03/18";
#endif
#endif

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <xview_private/i18n_impl.h>
#include <xview/cms.h>
#include <xview_private/cms_impl.h>
#include <xview/server.h>

/*
 *      (c) Copyright 1989 Sun Microsystems, Inc. Sun design patents
 *      pending in the U.S. and foreign countries. See LEGAL_NOTICE
 *      file for terms of the license.
 */

static Xv_Colormap *cms_allocate_colormap();

/*
 *	cms_free_colors() frees all the colors in the colormap that 
 *	have been allocated for this colormap segment.
 */
Pkg_private void
cms_free_colors(display, cms)
    Display	*display;
    Cms_info	*cms;
{
    register int	 i;

    if ((cms->index_table == NULL) || (cms->cmap == NULL)) {
	return;
    }

    for (i = 0; i <= cms->size - 1; i++) {
        if (cms->index_table[i] != XV_INVALID_PIXEL) {
            XFreeColors(display, cms->cmap->id, &cms->index_table[i], 1, 0);
 	    cms->index_table[i] = XV_INVALID_PIXEL;
        }
    }
}

/*
 *	cms_set_name() sets the secified name for the cms.
 */
Pkg_private void
cms_set_name(cms, name)
    Cms_info    *cms;
    char	*name;
{
   if (cms->name != NULL) {
	xv_free(cms->name);
       	cms->name = NULL;
   }

   cms->name = (char *)xv_malloc(strlen(name) + 1);
   strcpy(cms->name, name);
}


/* 
 *	cms_set_unique_name() generates & sets a unique name for the cms.
 */
Pkg_private void
cms_set_unique_name(cms)
    Cms_info    *cms;
{
    char	buf[20];

    if (cms->name != NULL) {
	xv_free(cms->name);
    }

    sprintf(buf, "%x", cms);
    cms->name = (char *)xv_malloc(strlen("xv_cms_") + strlen(buf) + 1);
    sprintf(cms->name, "xv_cms_%s", buf);
}

/* 
 * cms_get_colors() returns the colors either as an array of Xv_Singlecolor,
 * as an array of XColors, or as an array of red, green and blue colors.
 */
Pkg_private int
cms_get_colors(cms, cms_index, cms_count, cms_colors, cms_x_colors, 
	red, green, blue)
    Cms_info            *cms;
    unsigned long       cms_index, cms_count;
    Xv_Singlecolor      *cms_colors;
    XColor		*cms_x_colors;
    unsigned char	*red, *green, *blue;
{
    register int        i;
    XColor              *xcolors = NULL;
    Xv_opaque           server;
    Display             *display;
    unsigned long	valid_pixel = XV_INVALID_PIXEL;

    /* 
     * Check to see if atleast one cell has been allocated among the 
     * ones being retrieved.
     */
    for (i = 0; i <= cms_count - 1; i++) {
	if (cms->index_table[cms_index + i] != XV_INVALID_PIXEL) {
	    valid_pixel = cms->index_table[cms_index + i];
	    break;
	}
    }

    /* none of the pixels being retrieved have been allocated */
    if (valid_pixel == XV_INVALID_PIXEL) {
	return(XV_ERROR);
    }

    server = xv_get(cms->screen, SCREEN_SERVER);
    display = (Display *)xv_get(server, XV_DISPLAY);

    if (!cms_x_colors) {
	if ((xcolors = (XColor *)xv_malloc(cms_count * sizeof(XColor))) == NULL)
	    return(XV_ERROR);
    } else {
	xcolors = cms_x_colors;
    }

    for (i = 0; i <= cms_count - 1; i++) {
	if (cms->index_table[cms_index + i] != XV_INVALID_PIXEL)
  	    xcolors[i].pixel = cms->index_table[cms_index + i];
	else
	    xcolors[i].pixel = valid_pixel;
    }
    XQueryColors(display, cms->cmap->id, xcolors, cms_count);
    
    if (cms_colors) {
	for (i = 0; i <= cms_count - 1; i++) {
	    cms_colors[i].red = xcolors[i].red >> 8;
	    cms_colors[i].green = xcolors[i].green >> 8;
	    cms_colors[i].blue = xcolors[i].blue >> 8;
	}
    } else if (!cms_x_colors) {
	for (i = 0; i <= cms_count - 1; i++) {
	    red[i] = xcolors[i].red >> 8;
	    green[i]  = xcolors[i].green >> 8;
	    blue[i] = xcolors[i].blue >> 8;
	}
    }

    if (xcolors  && (xcolors != cms_x_colors))
        xv_free(xcolors);

    return(XV_OK);
}

Pkg_private int
cms_set_colors(cms, cms_colors, cms_x_colors, cms_index, cms_count)
    Cms_info       	*cms;
    Xv_Singlecolor	*cms_colors;
    XColor		*cms_x_colors;
    unsigned long	cms_index, cms_count;
{
    register int    i;
    XColor	    *xcolors = NULL;
    Xv_opaque	    server;
    Display    	    *display;
    int		    status;

    if (cms->index_table == NULL) {
	return(XV_ERROR);
    }

    server = xv_get(cms->screen, SCREEN_SERVER);
    display = (Display *)xv_get(server, XV_DISPLAY);

    if (cms_colors) {
        xcolors = xv_alloc_n(XColor, cms_count);
        for (i = 0; i <= cms_count - 1; i++) {
            (xcolors + i)->red = (unsigned short)(cms_colors + i)->red << 8;
            (xcolors + i)->green = (unsigned short)(cms_colors + i)->green << 8;
            (xcolors + i)->blue = (unsigned short)(cms_colors + i)->blue << 8;
            (xcolors + i)->flags = DoRed | DoGreen | DoBlue;
        } 
    } else if (cms_x_colors) {
	xcolors = cms_x_colors;
    }

    if (cms->type == XV_STATIC_CMS) {
	status =
	    cms_set_static_colors(display, cms, xcolors, cms_index, cms_count);
    } else {
	status =
	    cms_set_dynamic_colors(display, cms, xcolors, cms_index, cms_count);
    }

    if (xcolors != cms_x_colors) {
	xv_free(xcolors);
    }

    return(status);
}


Pkg_private int 
cms_set_static_colors(display, cms, xcolors, cms_index, cms_count)
    Display		*display;
    Cms_info            *cms;
    XColor              *xcolors;
    unsigned long       cms_index, cms_count;
{
    int			status;
    Xv_Colormap		*cmap_list, *cmap = NULL;

    /* 
     * Always attempt to allocate read-only colors from the default 
     * colormap. If the allocation fails, try any other colormaps 
     * that have been previously created. If that fails allocate a 
     * new colormap and allocate the read-only cells from it.
     */
    if (cms->cmap == NULL) {
	cmap_list = (Xv_Colormap *)cms->visual->colormaps;

	for (cmap = cmap_list; cmap != NULL; cmap = cmap->next) {
	    status = cms_alloc_static_colors(display, cms, cmap, xcolors,
					     cms_index, cms_count);
	    if (status == XV_ERROR) {
		cms->cmap = cmap;
		cms_free_colors(display, cms);
		cms->cmap = NULL;
	    } else {
		cms->cmap = cmap;
		cms->next = cmap->cms_list;
		cmap->cms_list = cms;
		break;
	    }
	}
	
	if (cmap == NULL) {
	    /* could not use any of the currently available colormaps */
	    cmap = cms_allocate_colormap(display, cms);
	    cms->cmap = cmap;
	    
	    /*
             * Add colormap to the screen visual's colormap list. The default 
             * colormap is always the first element in the colormap list.
             */
	    cmap->next = cmap_list->next;
	    cmap_list->next = cmap;
	    
    	    status = cms_alloc_static_colors(display, cms, cmap, xcolors,
					     cms_index, cms_count);
	    if (status == XV_ERROR) {
 		cms_free_colors(display, cms);
		cms->cmap = NULL;
	    }
	}
    } else {
	status = cms_alloc_static_colors(display, cms, cms->cmap,
			xcolors, cms_index, cms_count);
    }

    return(status);
}

Pkg_private int
cms_alloc_static_colors(display, cms, cmap, xcolors, cms_index, cms_count)
    Display             *display;
    Cms_info            *cms;
    Xv_Colormap		*cmap;
    XColor              *xcolors;
    unsigned long       cms_index, cms_count;
{
    unsigned long       *pixel;
    register int        i;

    if (xcolors == NULL)
	return(XV_OK);

    for (i = 0; i <= cms_count - 1; i++) {
	pixel = &cms->index_table[cms_index + i];

	/* static cms pixels are write-once only */
	if (*pixel == XV_INVALID_PIXEL) {
	    if (!XAllocColor(display, cmap->id, xcolors + i)) {
		return(XV_ERROR);
	    }
	    *pixel = (xcolors + i)->pixel;
	}
    }

    return(XV_OK);
}

Pkg_private int
cms_set_dynamic_colors(display, cms, xcolors, cms_index, cms_count)
    Display             *display;
    Cms_info            *cms;
    XColor              *xcolors;
    unsigned long       cms_index, cms_count;
{
    Xv_Colormap		*cmap_list, *cmap = NULL;
    register int	i;

    /* Search for an appropriate colormap to allocate colors from */
    if (cms->cmap == NULL) { 
	cmap_list = (Xv_Colormap *)cms->visual->colormaps;
	for (cmap = cmap_list; cmap != NULL; cmap = cmap->next) {
	    if (XAllocColorCells(display, cmap->id, True, NULL,
				 0, cms->index_table, cms->size)) 
	      break;	
	}

	/* Couldn't find one, so create a new colormap for allocation */
	if (cmap == NULL) {
	    cmap = cms_allocate_colormap(display, cms);
	    cms->cmap = cmap;

	    if (!XAllocColorCells(display, cmap->id, True, NULL,
			0, cms->index_table, cms->size)) {
		xv_free(cmap);
		return(XV_ERROR);
	    }

	    /*
	     * Add colormap to the screen's colormap list. The default colormap  
	     * is always the first element in the colormap list.
	     */
	    cmap->next = cmap_list->next;
	    cmap_list->next = cmap;
	} else {
	    cms->cmap = cmap;
	    cms->next = cmap->cms_list;
	    cmap->cms_list = cms;
	}
    }

    if (xcolors != NULL) {
        for (i = 0; i <= cms_count - 1; i++) {
	    (xcolors + i)->pixel = cms->index_table[cms_index + i];
        }
        XStoreColors(display, cms->cmap->id, xcolors, cms_count);
    }

    return(XV_OK);
}

Pkg_private XColor *
cms_parse_named_colors(cms, named_colors)
    Cms_info            *cms;
    char		**named_colors;
{
    XColor		*xcolors;
    int	    		count = 0;
    Display             *display;
    int			screen_num;

    if ((named_colors == NULL) || (*named_colors == NULL))
	return(NULL);

    while (named_colors[count])
	++count;

    xcolors = (XColor *)xv_malloc(count * sizeof(XColor));

    display = (Display *)xv_get(xv_get(cms->screen, SCREEN_SERVER), XV_DISPLAY);
    screen_num = (int)xv_get(cms->screen, SCREEN_NUMBER);

    for (--count; count >= 0; --count) {
	if (!XParseColor(display, DefaultColormap(display, screen_num), named_colors[count],
	    	xcolors+count)) {
	    xv_error(XV_ZERO,
		 ERROR_STRING,
		 XV_MSG("Unable to find RGB values for a named color"),
		 ERROR_PKG, CMS,
		 NULL);
	    return(NULL);
	}
    }

    return(xcolors);
}

static Xv_Colormap *
cms_allocate_colormap(display, cms)
    Display  *display;
    Cms_info *cms;	
{
    Xv_Colormap *cmap;
    int screen_num = (int)xv_get(cms->screen, SCREEN_NUMBER);

    cmap = xv_alloc(Xv_Colormap);
    if (STATUS(cms, default_cms) && 
	(cms->visual->vinfo->visualid == XVisualIDFromVisual(DefaultVisual(display, screen_num))))
      cmap->id = DefaultColormap(display, screen_num);
    else     
      cmap->id = XCreateColormap(display, 
				 RootWindow(display, screen_num),
				 cms->visual->vinfo->visual, AllocNone);
    cmap->type = XV_DYNAMIC_VISUAL(cms->visual->vinfo->class) ? 
      XV_DYNAMIC_CMAP : XV_STATIC_CMAP;
    cmap->cms_list = cms;
    cmap->next = (Xv_Colormap *)NULL;
    return(cmap);
}

Xv_private Xv_opaque
cms_default_colormap(server, display, screen_number, vinfo)
    Xv_Server	 server;
    Display	*display;
    int		 screen_number;
    XVisualInfo *vinfo;
{
    Bool		 found_one = FALSE;
    Xv_Colormap 	*colormap;
    XStandardColormap	*std_colormaps;
    int 		 num_cmaps, c;

    colormap = xv_alloc(Xv_Colormap);
    colormap->type = XV_DYNAMIC_VISUAL(vinfo->class) ? XV_DYNAMIC_CMAP : XV_STATIC_CMAP;
    colormap->cms_list = (Cms_info *)NULL;
    colormap->next = (Xv_Colormap *)NULL;

    /* Check to see if the screen's default visual matches the server's default visual */
    if (vinfo->visualid == XVisualIDFromVisual(DefaultVisual(display, screen_number))) {
	colormap->id = DefaultColormap(display, screen_number);
	found_one = TRUE;
    } else if (colormap->type == XV_DYNAMIC_CMAP) {
	/* Check to see if there is a standard colormap already allocated */   
	if (XGetRGBColormaps(display, RootWindow(display, screen_number),
			     &std_colormaps, &num_cmaps, XA_RGB_DEFAULT_MAP)) {
	    if (num_cmaps) {	    
		c = 0;
		while ((std_colormaps[c].visualid != vinfo->visualid) && (c < num_cmaps))
		  c++;
		if (c < num_cmaps) {
		    colormap->id = std_colormaps[c].colormap;
		    found_one = TRUE;
		}
	    }
	}
    }
    if (!found_one) 
      colormap->id = XCreateColormap(display, RootWindow(display, screen_number), 
				     vinfo->visual, AllocNone);
    
    return((Xv_opaque)colormap);
}    
