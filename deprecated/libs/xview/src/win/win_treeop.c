#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)win_treeop.c 20.55 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * win_treeop.c: Implement window tree operations. 
 */

#include <stdio.h>
#include <xview_private/i18n_impl.h>
#include <xview_private/portable.h>
#include <xview/pkg.h>
#include <xview/server.h>
#include <xview/window.h>
#include <xview_private/draw_impl.h>
#include <xview/win_struct.h>
#include <xview_private/win_info.h>
#include <xview/rect.h>
#include <X11/Xutil.h>
#include <X11/X.h>
#include <xview/screen.h>

Xv_object       win_data();

Xv_private void win_set_parent();

/*
 * Tree operations.
 * 
 * In SunView 1.X, a link was a window number, namely the trailing numeric part
 * of the window device name. In XView, a link is an XID for a window,
 * which need not exist as a SunView object in this process.
 */
Xv_public       XID
win_getlink(window, linkname)
    Xv_object       window;
    int             linkname;
{
    register Xv_Drawable_info *info;
    XID             window_xid;
    XID             root, parent;
    XID            *children = NULL;
    XID             return_val = WIN_NULLLINK;
    unsigned int    nchildren;

    if (!window)
	return WIN_NULLLINK;

    DRAWABLE_INFO_MACRO(window, info);
    window_xid = xv_xid(info);
    /* first get the parent */
    if (XQueryTree(xv_display(info), window_xid,
		   &root, &parent, &children, &nchildren) == 0) {
	fprintf(stderr, 
		XV_MSG("win_getlink: XQueryTree failed!\n"));
	goto deallocate;
    }
    switch (linkname) {
      case WL_ENCLOSING:
	return_val = parent;
	break;
      case WL_BOTTOMCHILD:
	if (nchildren) {
	    return_val = *children;
	}
	break;
      case WL_TOPCHILD:
	if (nchildren) {
	    return_val = children[nchildren - 1];
	}
	break;
      case WL_COVERED:		/* looking for the older sibling */
      case WL_COVERING:	/* looking for the younger sibling */
	/* Free children of window, get children of parent */
	if (children)
	    xv_free(children);
	if (XQueryTree(xv_display(info), parent,
		       &root, &parent, &children, &nchildren) == 0) {
	    fprintf(stderr, 
		XV_MSG("win_getlink: XQueryTree failed!\n"));
	    goto deallocate;
	}
	if (nchildren) {
	    register XID   *c;

	    for (c = children; nchildren && (*c != window_xid);
		 nchildren--, c++) {
		return_val = *c;
	    }
	    if (*c != window_xid) {
		fprintf(stderr, 
		    XV_MSG("win_getlink(sibling): window not in tree\n"));
		return_val = WIN_NULLLINK;
		goto deallocate;
	    }
	    if (linkname != WL_COVERED) {
		if (nchildren > 1) {
		    return_val = c[1];
		}
	    }
	}
	break;
      default:
	fprintf(stderr, 
		XV_MSG("win_getlink: unknown linkname: %d\n"), linkname);
	break;
    }

deallocate:
    if (children)
	xv_free(children);

    return return_val;
}

Xv_public void
win_setlink(window, linkname, number)
    Xv_object       window;
    int             linkname;
    XID             number;
{
    register Xv_Drawable_info *info;
    Display        *display;
    XWindowChanges  link_info;
    XID             sub_root;

    DRAWABLE_INFO_MACRO(window, info);
    display = xv_display(info);
    switch (linkname) {
      case WL_ENCLOSING:	/* set parent */
	win_set_parent(window, number, 0, 0);
	return;
      case WL_COVERED:		/* set older sibling */
      case WL_COVERING:	/* younger       */
	sub_root = xv_xid(info);
	link_info.sibling = number;
	link_info.stack_mode = (linkname == WL_COVERED) ? Above : Below;
	break;
      case WL_TOPCHILD:
      case WL_BOTTOMCHILD:
	/*
	 * BUG: deal with this. SunWindows allows clients to orphan a child,
	 * X11 does not.
	 */
	if (number == 0) {
	    xv_error(window,
		     ERROR_STRING,
		 XV_MSG("Call to win_setlink() with ZERO link value would orphan a child. Ignored. (Win package)"),
		     NULL);
	    return;
	}
	sub_root = number;
	link_info.sibling = win_getlink(window, linkname);
	link_info.stack_mode = (linkname == WL_TOPCHILD) ? Above : Below;
	break;
      default:
	fprintf(stderr, 
		XV_MSG("win_setlink: unknown linkname: %d\n"), linkname);
	return;
    }
    XConfigureWindow(display, sub_root, link_info.sibling ?
		     CWSibling | CWStackMode : CWStackMode, &link_info);
}

Xv_private void
win_set_parent(window, parent, x, y)
    Xv_object       window, parent;
    int             x, y;
{
    register Xv_Drawable_info *info;
    register Xv_Drawable_info *parent_info;
    register Display          *display;
    
    DRAWABLE_INFO_MACRO(window, info);
    DRAWABLE_INFO_MACRO(parent, parent_info);
    display = xv_display(info);
               
/*
    if (parent == (Xv_object)xv_root(info)) {
        Rect	   	rect;
        	
        (void) win_getrect(xv_get(window, WIN_PARENT), &rect);
    	x += rect.r_left;
    	y += rect.r_top;
    }
*/
    
    window_set_parent(window, parent);	                      
    XReparentWindow(display, xv_xid(info), xv_xid(parent_info), x, y);
}

win_insert(window)
    Xv_object       window;
{
    register Xv_Drawable_info *info;

    DRAWABLE_INFO_MACRO(window, info);
    XMapWindow(xv_display(info), xv_xid(info));
}

win_insert_in_front(window)
    Xv_object       window;
{
    register Xv_Drawable_info *info;

    DRAWABLE_INFO_MACRO(window, info);
    XMapRaised(xv_display(info), xv_xid(info));
}

win_remove(window)
    Xv_object       window;
{
    register Xv_Drawable_info *info;

    DRAWABLE_INFO_MACRO(window, info);

    if (xv_get (window, WIN_TOP_LEVEL))
        XWithdrawWindow(xv_display(info), xv_xid(info),
				  (int) xv_get(xv_screen(info), SCREEN_NUMBER));
    else
	XUnmapWindow (xv_display(info), xv_xid(info));
}


/*
 * Utilities
 */
Xv_private char *
win_name_for_qualified_xid(name, display, xid)
    char           *name;
    Display        *display;
    XID             xid;
{
    (void) sprintf(name, "%s:%d", XDisplayString(display), xid);
    return (name);
}
 
/*
 * win_number_to_object is used to convert the results of low level calls
 * such as win_getlink or win_findintersect back into window objects if such
 * objects exist in this process for the specified number.
 */
Xv_public       Xv_object
win_number_to_object(window, number)
    Xv_object       window;
    XID             number;
{
    Xv_object       result;
    register Xv_Drawable_info *info;
 
    DRAWABLE_INFO_MACRO(window, info);
    result = win_data(xv_display(info), number);
    return (result);
}

Xv_public XID
win_nametonumber(name)
    char           *name;
{
    char           *number_part;
 
    number_part = XV_RINDEX(name, ':');
    if (number_part == 0)
        return (-1);
    return (atoi(number_part));
}
 
Xv_public void
win_numbertoname(winnumber, name)
    int             winnumber;
    char           *name;
{
    extern Xv_object xv_default_display;

    (void) win_name_for_qualified_xid(name,
                xv_get(xv_default_display, XV_DISPLAY), winnumber);
}
 
Xv_public char *
win_fdtoname(window, name)
    Xv_object       window;
    char           *name;
{
    register Xv_Drawable_info *info;
 
    DRAWABLE_INFO_MACRO(window, info);
    return win_name_for_qualified_xid(name, xv_display(info), xv_xid(info));
}

Xv_public       XID
win_fdtonumber(window)
    Xv_object       window;
{
    register Xv_Drawable_info *info;
 
    DRAWABLE_INFO_MACRO(window, info);
    return xv_xid(info);
}

Xv_private void
win_free(window)
    Xv_object       window;
{
    register Xv_Drawable_info *info;
    Display        *display;

    DRAWABLE_INFO_MACRO(window, info);
    display = xv_display(info);
    XDeleteContext(display, xv_xid(info), CONTEXT);
    XDestroyWindow(display, xv_xid(info));
}

Xv_private      Xv_object
win_data(display, xid)
    Display        *display;
    XID             xid;
{
    Xv_object       result;

    if (XFindContext(display, xid, CONTEXT, (caddr_t *)&result)) {
	result = 0;
    }
    return (result);
}

Xv_private int
win_is_mapped(window)
    Xv_object       window;
{
    register Xv_Drawable_info *info;
    XWindowAttributes xattr;

    DRAWABLE_INFO_MACRO(window, info);
    if (XGetWindowAttributes(xv_display(info), xv_xid(info), &xattr)) {
	return (xattr.map_state == IsViewable ? 1 : 0);
    } else {
	return (0);
    }
}

Xv_private int
win_view_state(display, xid)
    Display        *display;
    XID             xid;
{
    XWindowAttributes xattr;

    if (XGetWindowAttributes(display, xid, &xattr)) {
	return (xattr.map_state);
    } else {
	return (0);
    }
}

Xv_private void
win_change_property(window, property_name, property_type, data_size,
		    property_data, data_count)
    Xv_object       window;
    Server_attr     property_name;
    Atom            property_type;
    int             data_size;
    unsigned char  *property_data;
    int             data_count;
{
    register Xv_Drawable_info *info;
    Atom            property;

    DRAWABLE_INFO_MACRO(window, info);
    property = (Atom) xv_get(xv_server(info), property_name);
    XChangeProperty(xv_display(info), xv_xid(info), property, property_type,
		    data_size, PropModeReplace, property_data, data_count);
}


Xv_private void
win_get_property(window, property_name, data_offset, data_count,
	     property_type, returned_data_count, bytes_after, property_data)
    Xv_object       window;
    Server_attr     property_name;
    long            data_offset, data_count;
    Atom            property_type;
    unsigned long  *returned_data_count;
    unsigned long  *bytes_after;
    unsigned char **property_data;
{
    register Xv_Drawable_info *info;
    Atom            property, returned_type;
    int             returned_format, result;

    DRAWABLE_INFO_MACRO(window, info);
    property = (Atom) xv_get(xv_server(info), property_name);
    result = XGetWindowProperty(xv_display(info), xv_xid(info),
		    property, data_offset, data_count, FALSE, property_type,
	 &returned_type, &returned_format, returned_data_count, bytes_after,
				property_data);
    /* BUG: should check returned values */
}
