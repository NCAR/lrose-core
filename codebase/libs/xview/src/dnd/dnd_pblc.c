#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)dnd_pblc.c 1.17 93/06/28";
#endif
#endif

/*
 *      (c) Copyright 1990 Sun Microsystems, Inc. Sun design patents
 *      pending in the U.S. and foreign countries. See LEGAL NOTICE
 *      file for terms of the license.
 */

#include <X11/Xatom.h>
#include <xview/xview.h>
#include <xview/notify.h>
#include <xview/dragdrop.h>
#include <xview_private/dndimpl.h>
#include <xview_private/draw_impl.h>
#include <xview_private/portable.h>
#ifdef SVR4 
#include <stdlib.h> 
#endif /* SVR4 */

static void BuildDefaults();

/* ARGSUSED */
Pkg_private int
dnd_init(parent, dnd_public, avlist)
    Xv_Window		parent;
    Xv_drag_drop	dnd_public;
    Attr_avlist		avlist;
{
    Dnd_info			*dnd = NULL;
    Xv_dnd_struct		*dnd_object;

    dnd = (Dnd_info *)xv_alloc(Dnd_info);
    dnd->public_self = dnd_public;
    dnd_object = (Xv_dnd_struct *)dnd_public;
    dnd_object->private_data = (Xv_opaque)dnd;

    dnd->parent = parent ? parent : xv_get(xv_default_screen, XV_ROOT);

    BuildDefaults(dnd);

    return(XV_OK);
}

Pkg_private Xv_opaque
dnd_set_avlist(dnd_public, avlist)
    Dnd			dnd_public;
    Attr_attribute	avlist[];
{
    register Dnd_info		*dnd = DND_PRIVATE(dnd_public);
    register Attr_avlist	 attrs;

    for (attrs = avlist; *attrs; attrs = attr_next(attrs)) {
        switch (attrs[0]) {
            case DND_TYPE:
               dnd->type = (DndDragType) attrs[1];
               break;
            case DND_CURSOR:
               dnd->cursor = (Xv_opaque) attrs[1];
               break;
            case DND_X_CURSOR:
               dnd->xCursor = (Cursor) attrs[1];
               break;
            case DND_ACCEPT_CURSOR:
               dnd->affCursor = (Xv_opaque) attrs[1];
               break;
            case DND_ACCEPT_X_CURSOR:
               dnd->affXCursor = (Cursor) attrs[1];
               break;
            case DND_TIMEOUT_VALUE:
	       XV_BCOPY((struct timeval *)attrs[1], &(dnd->timeout),
	    	      sizeof(struct timeval));
               break;
	    case XV_END_CREATE:
               break;
            default:
               (void)xv_check_bad_attr(&xv_dnd_pkg, attrs[0]);
	       break;
        }
    }

    return ((Xv_opaque)XV_OK);
}

/* ARGSUSED */
Pkg_private Xv_opaque
dnd_get_attr(dnd_public, status, attr, args)
    Dnd			 dnd_public;
    int			*status;
    Dnd_attribute	 attr;
    va_list		 args;
{
    Dnd_info		*dnd = DND_PRIVATE(dnd_public);
    Xv_opaque		 value = 0;

    switch (attr) {
        case DND_TYPE:
           value = (Xv_opaque)dnd->type;
           break;
        case DND_CURSOR:
           value = (Xv_opaque)dnd->cursor;
           break;
        case DND_X_CURSOR:
           value = (Xv_opaque)dnd->xCursor;
           break;
        case DND_ACCEPT_CURSOR:
           value = (Xv_opaque)dnd->affCursor;
           break;
        case DND_ACCEPT_X_CURSOR:
           value = (Xv_opaque)dnd->affXCursor;
           break;
        case DND_TIMEOUT_VALUE:
           value = (Xv_opaque)&dnd->timeout;
           break;
        default:
           if (xv_check_bad_attr(&xv_dnd_pkg, attr) == XV_ERROR)
		*status = XV_ERROR;
	   break;
    }

    return(value);
}

Pkg_private int
dnd_destroy(dnd_public, status)
    Dnd			dnd_public;
    Destroy_status	status;
{
    Dnd_info		*dnd = DND_PRIVATE(dnd_public);

    if (status == DESTROY_CLEANUP) {
	if (dnd->sel)
	   xv_destroy(dnd->sel);
	if (dnd->window)
	   xv_destroy(dnd->window);
	if (dnd->siteRects) {
	   xv_free(dnd->siteRects);
	   /* It is possible that the dnd object will be destroyed before
	    * dnd_send_drop() returns.
	    */
	   dnd->siteRects = NULL;
	}
	xv_free(dnd);
    }

    return(XV_OK);
}

static void
BuildDefaults(dnd)
    Dnd_info     *dnd;
{
    Xv_opaque server = XV_SERVER_FROM_WINDOW(dnd->parent);

    dnd->atom[TRIGGER] = (Atom) xv_get(server,
				        SERVER_ATOM, "_SUN_DRAGDROP_TRIGGER");
    dnd->atom[PREVIEW] = (Atom) xv_get(server, 
					SERVER_ATOM, "_SUN_DRAGDROP_PREVIEW");
    dnd->atom[ACK] =     (Atom) xv_get(server,
					SERVER_ATOM, "_SUN_DRAGDROP_ACK");
    dnd->atom[DONE] =    (Atom) xv_get(server,
					SERVER_ATOM, "_SUN_DRAGDROP_DONE");
    dnd->atom[WMSTATE] = (Atom) xv_get(server, SERVER_ATOM, "WM_STATE");
    dnd->atom[INTEREST] =(Atom) xv_get(server,
					SERVER_ATOM, "_SUN_DRAGDROP_INTEREST");
    dnd->atom[DSDM] =    (Atom) xv_get(server,
					SERVER_ATOM, "_SUN_DRAGDROP_DSDM");
    dnd->type = DND_MOVE;
    dnd->sel = XV_ZERO;
    dnd->siteRects = NULL;
    dnd->affXCursor = (Cursor)NULL; 
    dnd->xCursor = (Cursor)NULL;
    dnd->transientSel = False;

    dnd->timeout.tv_sec = xv_get(DND_PUBLIC(dnd), SEL_TIMEOUT_VALUE);
    dnd->timeout.tv_usec = 0;

    dnd->incr_size = 0;
    dnd->incr_mode = 0;
    dnd->lastRootWindow = 0;
    dnd->screenNumber = 0;
}
