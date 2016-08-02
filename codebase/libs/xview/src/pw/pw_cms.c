#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)pw_cms.c 20.31 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE 
 *	file for terms of the license.
 */

/*
 * Implements the color interface of the pixwin layer.
 */
#include <stdio.h>
#include <xview/window.h>
#include <xview_private/draw_impl.h>
#include <xview/cms.h>

extern Attr_attribute xv_cms_name_key;

Xv_public int
pw_getcmsname(pw, name)
    Xv_opaque       pw;
    char           *name;
{
    char           *temp_name;
    Xv_Drawable_info *info;

    if (temp_name = (char *) xv_get(pw, XV_KEY_DATA, xv_cms_name_key)) {
	strcpy(name, temp_name);
    } else {
	DRAWABLE_INFO_MACRO(pw, info);
	strcpy(name, (char *)xv_get(xv_cms(info), CMS_NAME));
    }
}

Xv_public int
pw_putcolormap(pw, index, count, red, green, blue)
    Xv_opaque       pw;
    int             index, count;
    unsigned char   red[], green[], blue[];
{
    Xv_cmsdata      cms_data;

    cms_data.type = XV_DYNAMIC_CMS;
    cms_data.index = index;
    cms_data.size = count;
    cms_data.rgb_count = count;
    cms_data.red = red;
    cms_data.green = green;
    cms_data.blue = blue;

    window_set_cms_data(pw, &cms_data);
}

Xv_public int
pw_getcolormap(pw, index, count, red, green, blue)
    Xv_opaque       pw;
    int             index, count;
    unsigned char   red[], green[], blue[];
{
    Xv_cmsdata	    *cms_data;
    register int    i;

    cms_data = (Xv_cmsdata *)xv_get(pw, WIN_CMS_DATA);
    for (i = 0; i <  count; i++) {
      red[i] = cms_data->red[index + i];
      green[i] = cms_data->green[index + i];
      blue[i] = cms_data->blue[index + i];
    }
}

Xv_public int
pw_putattributes(pw, planes)
    Xv_opaque       pw;
    register int   *planes;
{
    Xv_Drawable_info *info;
    DRAWABLE_INFO_MACRO(pw, info);

    xv_plane_mask(info) = *planes;
}

Xv_public int
pw_getattributes(pw, planes)
    Xv_opaque       pw;
    int            *planes;
{
    Xv_Drawable_info *info;
    DRAWABLE_INFO_MACRO(pw, info);

    *planes = xv_plane_mask(info);
}

