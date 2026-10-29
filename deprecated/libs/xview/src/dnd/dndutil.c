#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)dndutil.c 1.16 93/06/28";
#endif
#endif

/*
 *      (c) Copyright 1990 Sun Microsystems, Inc. Sun design patents 
 *      pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *      file for terms of the license.
 */

#include <X11/Xproto.h>
#include <xview/xview.h>
#include <xview/cursor.h>
#include <xview/server.h>
#include <xview/dragdrop.h>
#include <xview_private/dndimpl.h>

static Atom InternSelection();

/* 
 * Determine what cursor to use, create one if none defined.  Return the XID.
 */
Xv_private XID
DndGetCursor(dnd)
    Dnd_info 	*dnd;
{
    if (!dnd->xCursor && !dnd->cursor) {
        dnd->cursor = xv_find(dnd->parent, CURSOR,
			      CURSOR_SRC_CHAR,
		  	     (dnd->type == DND_MOVE) ?
			                    OLC_MOVE_PTR : OLC_COPY_PTR,
			      CURSOR_MASK_CHAR,
		  	     (dnd->type == DND_MOVE) ?
			                    OLC_MOVE_MASK_PTR :
						      OLC_COPY_MASK_PTR,
					    NULL);
	return((XID)xv_get(dnd->cursor, XV_XID));
    } else if (dnd->cursor)
	return((XID)xv_get(dnd->cursor, XV_XID));
    else
	return((XID)dnd->xCursor);
}

Xv_private int
DndGetSelection(dnd, dpy) 
    Dnd_info    *dnd;
    Display 	*dpy;
{
    int 	 i = 0;
    Atom	 seln;
    Xv_Server	 server = XV_SERVER_FROM_WINDOW(dnd->parent);

				/* Application defined selection. */
    if (xv_get(DND_PUBLIC(dnd), SEL_OWN)) return(DND_SUCCEEDED);

				/* Create our own transient selection. */
				/* Look for a selection no one else is using. */

				/* XXX: This will become very slow if the app
				 * has > 100 selections in use.  We will go
				 * through > 100 XGetSelectionOwner() requests
				 * looking for a free selection.
				 */
    for (i = 0; ; i++) {
	seln = InternSelection(server, i, (XID)xv_get(dnd->parent, XV_XID));
	if (XGetSelectionOwner(dpy, seln) == None) {
	    dnd->transientSel = True;
	    xv_set(DND_PUBLIC(dnd), SEL_RANK, seln,
	    			    SEL_OWN, True, NULL);
	    break;
        }
    }
    return(DND_SUCCEEDED);
}

static Atom
InternSelection(server, n, xid)
    Xv_server 	server;
    int 	n;
    XID		xid;
{
    char buf[60]; /* Generous. SUN_DND_TRANSIENT_TEMPLATE is defined in dndimpl.h */
		  /* If it changes, the size of buf would have to change also */
		  /* Cannot use sizeof(SUN_DND_TRANSIENT_TEMPLATE) as it will */
		  /* return wrong value (4) if xstr is used */

    sprintf(buf, SUN_DND_TRANSIENT_TEMPLATE, xid, n);
    return xv_get(server, SERVER_ATOM, buf);
}

static int sendEventError;
static (*old_handler)();

static int
sendEventErrorHandler(dpy, error)
    Display *dpy;
    XErrorEvent *error;
{
    if (error->request_code == X_SendEvent)
        sendEventError = True;
    else
        (*old_handler)(dpy, error);
}

Pkg_private int
DndSendEvent(dpy, event)
    Display *dpy;
    XAnyEvent *event;
{
    Status status;

    sendEventError = False;
    old_handler = XSetErrorHandler(sendEventErrorHandler);

    status = XSendEvent(dpy, event->window, False, NoEventMask,
			(XEvent *) event);
    XSync(dpy, False);
    (void) XSetErrorHandler(old_handler);

    if (status && ! sendEventError)
	return(DND_SUCCEEDED);
    
    return(DND_ERROR);
}

/*ARGSUSED*/
Pkg_private int
DndMatchProp(dpy, event, wE)
    Display             *dpy;
    XEvent              *event;
    DnDWaitEvent        *wE;
{
    if ((event->type == wE->eventType) &&
                           (((XPropertyEvent*)event)->atom == (Atom)wE->window))
        return(True);
    else
        return(False);
}
