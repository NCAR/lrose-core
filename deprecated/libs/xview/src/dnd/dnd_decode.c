#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)dnd_decode.c 1.15 93/06/28";
#endif
#endif

/*
 *      (c) Copyright 1990 Sun Microsystems, Inc. Sun design patents
 *      pending in the U.S. and foreign countries. See LEGAL NOTICE
 *      file for terms of the license.
 */

#include <sys/time.h>
#include <X11/Xatom.h>
#include <xview/xview.h>
#include <xview/server.h>
#include <xview/window.h>
#include <xview/sel_pkg.h>
#include <xview/dragdrop.h>
#include <xview_private/dndimpl.h>
#include <xview_private/xv_list.h>

static int SendACK(),
	   MakeSelRequest(); 
Pkg_private int	DndMatchEvent(),
		DndMatchProp();

typedef struct dnd_drop_site {
    Xv_sl_link           next;
    Xv_drop_site         drop_item;
} Dnd_drop_site;

static int	dnd_transient_key = 0;

Xv_public Xv_opaque
dnd_decode_drop(sel_obj, event)
    Xv_object	 sel_obj;
    Event 	*event;
{
    XClientMessageEvent *cM;
    Dnd_drop_site	*site;


    if (!(event_action(event) == ACTION_DRAG_COPY ||
	  event_action(event) == ACTION_DRAG_MOVE))
	return(DND_ERROR);

    if (!dnd_transient_key)
	dnd_transient_key = xv_unique_key();

    cM = (XClientMessageEvent *)event_xevent(event);

    if (cM->message_type != (Atom)xv_get(
		XV_SERVER_FROM_WINDOW(xv_get(sel_obj, XV_OWNER)), SERVER_ATOM,
                "_SUN_DRAGDROP_TRIGGER"))
	return(DND_ERROR);

		/* Remind ourself to send _SUN_DRAGDROP_DONE in dnd_done() */
    if (DND_IS_TRANSIENT(event))
	xv_set(sel_obj, XV_KEY_DATA, dnd_transient_key, True, NULL);

		/* Set the rank of the selection to the rank being used in
		 * the drag and drop transaction.
		 */
    xv_set(sel_obj, SEL_RANK, cM->data.l[0], NULL); 

		/* If the acknowledgement flag is set, send an ack. */
    if (cM->data.l[4] & DND_ACK_FLAG) {
	if (SendACK(sel_obj, event, cM) == DND_ERROR)
	   return(DND_ERROR);
    }

		/* Find the drop site that was dropped on. */
    site = (Dnd_drop_site *)xv_get(event_window(event), WIN_ADD_DROP_ITEM);

    /* SUPPRESS 560 */
    while(site = (Dnd_drop_site *) (XV_SL_SAFE_NEXT(site))) {
        if ((long)xv_get(site->drop_item, DROP_SITE_ID) == (long)cM->data.l[3])
	    return(site->drop_item);
    }

    return(DND_ERROR);
}

static int
SendACK(sel, ev, cM)
    Xv_object		 sel;
    Event    		*ev;
    XClientMessageEvent	*cM;
{
    Xv_Server            server = XV_SERVER_FROM_WINDOW(event_window(ev));

    if (dnd_is_local(ev)) {          /* flag set to True in local case */
        Attr_attribute       dndKey = xv_get(server, SERVER_DND_ACK_KEY);

    	xv_set(server, XV_KEY_DATA, dndKey, True, NULL);
        return (DND_SUCCEEDED);
    } else { /* Remote case */
	return(MakeSelRequest(cM->display, cM->data.l[0], sel,
		      (Atom)xv_get(server, SERVER_ATOM, "_SUN_DRAGDROP_ACK"),
		      cM->window, cM->data.l[1])); 
    }
}

static int
MakeSelRequest(dpy, selection, sel_obj, target, window, time) 
    Display		*dpy;
    Atom 		 selection;
    Xv_object		 sel_obj;
    Atom 		 target;
    Window		 window;
    Time		 time;
{
    struct timeval	 timeout;
    char	        *data;
    int		 	 DndMatchEvent();
    int			 format;
    unsigned long        nitems,
			 remain;
    Atom		 type;
    XEvent		 xEvent;

    timeout.tv_sec = (int)xv_get(sel_obj, SEL_TIMEOUT_VALUE);
    timeout.tv_usec = 0;

		/* REMIND: Using the target atom as the name of the
		 * property the owner should use.  Is this safe?
		 */
    XConvertSelection(dpy, selection, target, target, window, time);

			                  /* Wait for a SelectionNotify event */
    if (DndWaitForEvent(dpy, window, SelectionNotify, target, &timeout,
			               &xEvent, DndMatchEvent) != DND_SUCCEEDED)
	goto BailOut;

		/* The request has been rejected. */
    if (xEvent.xselection.property == None)
	goto BailOut;

			                    /* Get the data from the property */
				       /* BUG: 1000L is an arbritrary size. */
    if (XGetWindowProperty(dpy, window, xEvent.xselection.property, 0L,
			   1000L, False, AnyPropertyType, &type, &format,
			   &nitems, &remain, (unsigned char **)&data) != Success)
	goto BailOut;

    if (data) XFree(data);

    XDeleteProperty(dpy, window, xEvent.xselection.property);
		    
    return(DND_SUCCEEDED);

BailOut:
    return(DND_ERROR);
}

Xv_public void
dnd_done(sel)
    Selection_requestor	sel;
{
    if (xv_get(sel, XV_KEY_DATA, dnd_transient_key)) {
	int  format;
	long length;
	void (*reply_proc)();

	/* SUPPRESS 560 */
        if ((reply_proc = (void (*)())xv_get(sel, SEL_REPLY_PROC)))
	    xv_set(sel, SEL_REPLY_PROC, (void (*)())NULL, NULL);

	xv_set(sel, XV_KEY_DATA, dnd_transient_key, False, NULL);
	xv_set(sel, SEL_TYPE_NAME, "_SUN_DRAGDROP_DONE", NULL);

	(void)xv_get(sel, SEL_DATA, &length, &format);
	
        if (reply_proc)
	    xv_set(sel, SEL_REPLY_PROC, reply_proc, NULL);
    }
}
