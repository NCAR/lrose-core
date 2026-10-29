#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)font_get.c 20.31 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#include <stdio.h>
#include <X11/Xlib.h>
#include <xview/attr.h>
#include <xview_private/font_impl.h>
#include <xview_private/portable.h>

/*
 * Externed functions
 */

extern struct pr_size xv_pf_textwidth();
#ifdef OW_I18N
extern struct pr_size xv_pf_textwidth_wc();
#endif /*OW_I18N*/

/*
 * Private
 */

Pkg_private     Xv_opaque
font_get_attr(font_public, status, attr, args)
    Xv_font_struct *font_public;
    int            *status;	/* initialized by caller */
    Font_attribute  attr;
    va_list         args;
{
    register Font_info *font = FONT_PRIVATE(font_public);
#ifdef OW_I18N
    XFontStruct         *font_struct;
#endif /*OW_I18N*/
    XFontStruct		*x_font_info = (XFontStruct *)font->x_font_info;
    Xv_opaque       v;
    int             attr_is_char_width = 0;
/* Alpha compatibility, mbuck@debian.org */
#if 0
    Attr_avlist     avlist = (Attr_avlist) args;
#endif

#ifdef OW_I18N
    if (font->type == FONT_TYPE_TEXT)  {
        font_struct = (XFontStruct *)font->font_structs[0];
    }
#endif /*OW_I18N*/

    switch (attr) {
#ifdef OW_I18N
      case FONT_SET_ID:
        v = (Xv_opaque)font->set_id;
        break;
#endif /*OW_I18N*/

      case FONT_INFO:
#ifdef OW_I18N
        if (font->type == FONT_TYPE_TEXT)  {
            if (font_struct != NULL) {
                v = (Xv_opaque)font_struct;
	        break;
	    } else {		/* BUG: could query X font property?? */
	        *status = XV_ERROR;
	        v = (Xv_opaque) 0;
	        break;
	    }
	}
	else {
	    if (x_font_info) {
	        v = (Xv_opaque) x_font_info;
	        break;
	    } else {		/* BUG: could query X font property?? */
	        *status = XV_ERROR;
	        v = (Xv_opaque) 0;
	        break;
	    }
	}
#else
	if (x_font_info) {
	    v = (Xv_opaque) x_font_info;
	    break;
	} else {		/* BUG: could query X font property?? */
	    *status = XV_ERROR;
	    v = (Xv_opaque) 0;
	    break;
	}
#endif

      case FONT_DEFAULT_CHAR_HEIGHT:
	/*
	 * Default char width is max ascent + max descent;
	 */
	v = (Xv_opaque) font->def_char_height;
	break;

      case FONT_SCALE:
	if (font->scale != FONT_NO_SCALE) {
	    v = (Xv_opaque) font->scale;
	    break;
	} else {
	    *status = XV_ERROR;
	    v = (Xv_opaque) 0;
	    break;
	}

#ifdef OW_I18N
      case FONT_CHAR_WIDTH_WC:
        attr_is_char_width = TRUE;
      case FONT_CHAR_HEIGHT_WC:{
            wchar_t         wc = (wchar_t) va_arg(args, int);
            wchar_t         wstr[2];
            struct pr_size  my_pf_size;

            wstr[0] = wc;
            wstr[1] = (wchar_t) 0;
            my_pf_size = xv_pf_textwidth_wc(1, font_public, wstr);
            if (attr_is_char_width) {
                v = (Xv_opaque) my_pf_size.x;
            } else
                v = (Xv_opaque) my_pf_size.y;
            break;
        }
#endif /*OW_I18N*/

      case FONT_SIZE:
	if (font->size) {
	    v = (Xv_opaque) font->size;
	    break;
	} else {		/* BUG: could query X font property?? */
	    *status = XV_ERROR;
	    v = (Xv_opaque) 0;
	    break;
	}

      case FONT_STRING_DIMS:{
/* Alpha compatibility, mbuck@debian.org */
#if 1
	    char           *string = (char *) va_arg(args, char *);
	    Font_string_dims *size = (Font_string_dims *) va_arg(args, Font_string_dims *);
#else
	    char           *string = (char *) avlist[0];
	    Font_string_dims *size = (Font_string_dims *) avlist[1];
#endif
	    struct pr_size  my_pf_size;
	    if (string) {
		my_pf_size = xv_pf_textwidth(strlen(string), font_public, string);
		size->width = my_pf_size.x;
		size->height = my_pf_size.y;
		v = (Xv_opaque) size;
		break;
	    } else {
		*status = XV_ERROR;
		v = (Xv_opaque) size;
		break;
	    }
	}

      case FONT_FAMILY:
	if (font->family) {
	    v = (Xv_opaque) font->family;
	    break;
	} else {
	    *status = XV_ERROR;
	    v = (Xv_opaque) 0;
	    break;
	}

      case FONT_PIXFONT:
	if (!font->pixfont)  {
            font_init_pixfont(font_public);
	}
	v = (Xv_opaque) font->pixfont;
	break;

      case XV_XID:
#ifdef OW_I18N
	if (font->type == FONT_TYPE_TEXT)  {
            v = (Xv_opaque)font_struct->fid;
	}
	else  {
	    v = (Xv_opaque) font->xid;
	}
#else
	v = (Xv_opaque) font->xid;
#endif /*OW_I18N*/
	break;

      case FONT_NAME:
#ifdef OW_I18N
	if (font->type == FONT_TYPE_TEXT)  {
	    v = (Xv_opaque) font->names[0];
	}
	else  {
	    v = (Xv_opaque) font->name;
	}
#else  /*OW_I18N*/    
	v = (Xv_opaque) font->name;
#endif  /*OW_I18N*/	
	break;
	
#ifdef OW_I18N	
      case FONT_NAMES:
	v = (Xv_opaque) font->names;
	break;
#endif  /*OW_I18N*/
	      
      case FONT_STYLE:
	if (font->style) {
	    v = (Xv_opaque) font->style;
	    break;
	} else {
	    *status = XV_ERROR;
	    v = (Xv_opaque) 0;
	    break;
	}

#ifdef OW_I18N
      case FONT_LOCALE:
        if (font->locale_info->locale) {
            v = (Xv_opaque) font->locale_info->locale;
        } else {
            *status = XV_ERROR;
	    v = (Xv_opaque) 0;
        }
	break;
#endif
#ifdef OW_I18N
      case FONT_SET_SPECIFIER:
        if (font->specifier) {
            v = (Xv_opaque) font->specifier;
        } else {
            *status = XV_ERROR;
	    v = (Xv_opaque) 0;
        }
	break;
#endif
      case FONT_TYPE:
	v = (Xv_opaque) font->type;
	break;

      case FONT_DEFAULT_CHAR_WIDTH:
	/*
	 * Default char height is max
	 */
	v = (Xv_opaque) font->def_char_width;
	break;

      case FONT_CHAR_WIDTH:
	attr_is_char_width = TRUE;
      case FONT_CHAR_HEIGHT:{
	    char            font_char = (char) va_arg(args, int);
	    char            font_char_array[2];
	    struct pr_size  my_pf_size;

	    font_char_array[0] = font_char;
	    font_char_array[1] = (char) 0;
	    my_pf_size = xv_pf_textwidth(1, font_public, font_char_array);
	    if (attr_is_char_width) {
		v = (Xv_opaque) my_pf_size.x;
	    } else
		v = (Xv_opaque) my_pf_size.y;
	    break;
	}


#ifdef OW_I18N
      case FONT_STRING_DIMS_WC:{
/* Alpha compatibility, mbuck@debian.org */
#if 1
            wchar_t             *ws = (wchar_t *) va_arg(args, wchar_t *);
            Font_string_dims    *size = (Font_string_dims *) va_arg(args, Font_string_dims *);
#else
            wchar_t             *ws = (wchar_t *) avlist[0];
            Font_string_dims    *size = (Font_string_dims *) avlist[1];
#endif
            struct pr_size      my_pf_size;

            if (ws) {
                my_pf_size = xv_pf_textwidth_wc(wslen(ws), font_public, ws);
                size->width = my_pf_size.x;
                size->height = my_pf_size.y;
                v = (Xv_opaque) size;
                break;
            } else {
                *status = XV_ERROR;
                v = (Xv_opaque) size;
                break;
            }
        }    
#endif /*OW_I18N*/

#ifdef OW_I18N
      case FONT_COLUMN_WIDTH:
        v = (Xv_opaque) font->column_width;
	break;
#endif /* OW_I18N */
		
      default:
	if (xv_check_bad_attr(&xv_font_pkg, attr) == XV_ERROR) {
	    *status = XV_ERROR;
	}
	v = (Xv_opaque) 0;
	break;

    }
    return (Xv_opaque) v;
}
