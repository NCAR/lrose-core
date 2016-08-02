#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)om_get.c 20.51 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE 
 *	file for terms of the license.
 */

/* ------------------------------------------------------------------------- */

#include <sys/types.h>
#include <stdio.h>
#include <xview/server.h>
#include <xview_private/om_impl.h>
#include <xview_private/portable.h>

/* ------------------------------------------------------------------------- */

extern Xv_object xv_default_server;

/*
 * Package private
 */
Pkg_private Xv_opaque menu_gets();
Pkg_private Xv_opaque menu_item_gets();
Pkg_private Xv_opaque menu_pullright_return_result();

/* ------------------------------------------------------------------------- */


Pkg_private     Xv_opaque
menu_gets(menu_public, status, attr, args)
    Menu            menu_public;
    int            *status;
    Attr_attribute  attr;
    va_list         args;
{
    Xv_opaque       v = XV_ZERO;
    int             value;
    register Xv_menu_info *m = MENU_PRIVATE(menu_public);

    switch (attr) {

      case MENU_NITEMS:
	v = (Xv_opaque) m->nitems;
	break;

      case MENU_NTH_ITEM:
	value = va_arg(args, int);
	if ((int) value > 0 && (int) value <= m->nitems)
	    v = MENU_ITEM_PUBLIC(m->item_list[(int) value - 1]);
	break;

      case MENU_CLIENT_DATA:
	v = (Xv_opaque) m->client_data;
	break;

      case MENU_PIN:
	v = (Xv_opaque) m->pin;
	break;

      case MENU_GEN_PROC:
	v = (Xv_opaque) (m->gen_proc);
	break;

      case MENU_PARENT:
	v = XV_ZERO;
	if (m->parent != NULL) {
	    v = MENU_ITEM_PUBLIC(m->parent);
	}
	break;

      case MENU_DEFAULT_ITEM:
	if (range(m->default_position, 1, m->nitems))
	    v = MENU_ITEM_PUBLIC(m->item_list[m->default_position - 1]);
	break;

      case MENU_NOTIFY_STATUS:
	/* Get notify status of this menu */
	v = (Xv_opaque) m->notify_status;
	break;

      case MENU_CLASS:
	v = (Xv_opaque) m->class;
	break;

      case MENU_BUSY_PROC:
	v = (Xv_opaque) m->busy_proc;
	break;

      case MENU_COLOR:
	v = (Xv_opaque) m->color_index;
	break;

      case MENU_COL_MAJOR:
	v = (Xv_opaque) m->column_major;
	break;

      case MENU_DEFAULT:
	v = (Xv_opaque) m->default_position;
	break;

      case MENU_DONE_PROC:
	v = (Xv_opaque) m->done_proc;
	break;

      case MENU_FIRST_EVENT:
	v = m->group_info ? (Xv_opaque) & m->group_info->first_event : XV_ZERO;
	break;

      case MENU_LAST_EVENT:
	v = m->group_info ? (Xv_opaque) & m->group_info->last_event : XV_ZERO;
	break;

      case MENU_NOTIFY_PROC:
	v = (Xv_opaque) (m->notify_proc);
	break;

      case MENU_NCOLS:
	v = (Xv_opaque) m->ncols;
	break;

      case MENU_NROWS:
	v = (Xv_opaque) m->nrows;
	break;

      case MENU_PIN_PROC:
	v = (Xv_opaque) m->pin_proc;
	break;

      case MENU_PIN_WINDOW:
	v = (Xv_opaque) m->pin_window;
	break;

      case MENU_SELECTED:
	v = (Xv_opaque) m->selected_position;
	break;
    
      case MENU_SELECTED_ITEM:
	if (range(m->selected_position, 1, m->nitems))
	    v = (Xv_opaque) MENU_ITEM_PUBLIC(
		m->item_list[m->selected_position-1]);
	break;

      case MENU_TYPE:
	v = (Xv_opaque) MENU_MENU;
	break;

      case MENU_VALID_RESULT:
	v = (Xv_opaque) m->valid_result;
	break;

      case XV_VISUAL:
	if (m->group_info)
	  v = (Xv_opaque) m->group_info->vinfo->visual;
	else 
	  v = XV_ERROR;
	break;
	
      case XV_VISUAL_CLASS:
	if (m->group_info)
	  v = (Xv_opaque) m->group_info->vinfo->class;
	else if (m->vinfo_mask & VisualClassMask)
	  v = (Xv_opaque) m->vinfo_template.class;
	else	
	  v = XV_ERROR;
	break;
	
      case XV_DEPTH:
	if (m->group_info)
	  v = (Xv_opaque) m->group_info->vinfo->depth;
	else if (m->vinfo_mask & VisualDepthMask)
	  v = (Xv_opaque) m->vinfo_template.depth;
	else	
	  v = XV_ERROR;
	break;

      default:
	if (xv_check_bad_attr(MENU, attr) == XV_ERROR) {
	    *status = XV_ERROR;
	}
	break;

    }
    return v;
}


/* ARGSUSED */
Pkg_private     Xv_opaque
menu_item_gets(menu_item_public, status, attr, args)
    Menu_item       menu_item_public;
    int            *status;
    Attr_attribute  attr;
    va_list         args;
{
    Xv_opaque       v = XV_ZERO;
    Xv_menu_item_info *mi = MENU_ITEM_PRIVATE(menu_item_public);

    switch (attr) {

      case XV_FONT:
	v = (Xv_opaque) mi->image.font;
	break;

      case MENU_CLIENT_DATA:
	v = (Xv_opaque) mi->client_data;
	break;

      case MENU_PARENT:
	v = XV_ZERO;
	if (mi->parent != NULL) {
	    v = MENU_PUBLIC(mi->parent);
	}
	break;

      case MENU_TITLE:
	v = (Xv_opaque) mi->title;
	break;

      case MENU_PULLRIGHT:
	v = (Xv_opaque) (mi->pullright ? mi->value : 0);
	break;

      case MENU_INACTIVE:
	v = (Xv_opaque) mi->inactive;
	break;

      case MENU_ACTION:	/* & case MENU_NOTIFY_PROC: */
	v = (Xv_opaque) (mi->notify_proc);
	break;

      case MENU_COLOR:
	v = (Xv_opaque) mi->color_index;
	break;

      case MENU_FEEDBACK:
	v = (Xv_opaque) ! mi->no_feedback;
	break;

      case MENU_GEN_PROC:
	v = (Xv_opaque) (mi->gen_proc);
	break;

      case MENU_GEN_PULLRIGHT:
	v = (Xv_opaque) (mi->gen_pullright);
	break;

      case MENU_IMAGE:
	v = (Xv_opaque) mi->image.svr_im;
	break;

      case MENU_INVERT:
	v = (Xv_opaque) mi->image.invert;
	break;

      case MENU_RELEASE:
	v = (Xv_opaque) mi->free_item;
	break;

      case MENU_RELEASE_IMAGE:
	v = (Xv_opaque) (mi->image.free_string || mi->image.free_svr_im);
	break;

      case MENU_SELECTED:
	v = (Xv_opaque) mi->selected;
	break;
#ifdef OW_I18N
      case MENU_STRING:
        v = (Xv_opaque) _xv_get_mbs_attr_nodup(&mi->image.string);
        break;

      case MENU_STRING_WCS:
        v = (Xv_opaque) _xv_get_wcs_attr_nodup(&mi->image.string);
        break;
#else
      case MENU_STRING:
	v = (Xv_opaque) mi->image.string;
	break;
#endif /* OW_I18N */
      /* ACC_XVIEW */
      case MENU_ACC_KEY:
#ifdef OW_I18N
        v = (Xv_opaque) _xv_get_mbs_attr_nodup(&mi->key_image.string);
#else
	v = (Xv_opaque) mi->key_image.string;
#endif /* OW_I18N */
	break;

      case MENU_ACC_QUAL:
#ifdef OW_I18N
        v = (Xv_opaque) _xv_get_mbs_attr_nodup(&mi->qual_image.string);
#else
	v = (Xv_opaque) mi->qual_image.string;
#endif /* OW_I18N */
	break;
      /* ACC_XVIEW */

      case MENU_TYPE:
	v = (Xv_opaque) MENU_ITEM;
	break;

      case MENU_VALUE:
	if (mi->pullright && mi->parent && mi->parent->group_info)
	    v = (Xv_opaque) menu_pullright_return_result(
						      MENU_ITEM_PUBLIC(mi));
	else
	    v = (Xv_opaque) mi->value;
	break;
 
      /* ACC_XVIEW */
#ifdef OW_I18N
      case MENU_ACCELERATOR_WCS:
        v = (Xv_opaque) _xv_get_wcs_attr_dup(&mi->menu_acc);
	break;
      case MENU_ACCELERATOR:
        v = (Xv_opaque) _xv_get_mbs_attr_dup(&mi->menu_acc);
	break;
#else
      case MENU_ACCELERATOR:
	v = (Xv_opaque) mi->menu_acc;
	break;
#endif
      /* ACC_XVIEW */

      default:
	if (xv_check_bad_attr(MENUITEM, attr) == XV_ERROR) {
	    *status = XV_ERROR;
	}
	break;

    }
    return v;
}
