#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)dnd.c 1.30 93/06/28";
#endif
#endif

/*
 *      (c) Copyright 1990 Sun Microsystems, Inc. Sun design patents 
 *      pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *      file for terms of the license.
 */

#include <stdio.h>
#include <errno.h>
#include <sys/time.h>
#include <xview/xview.h>
#include <xview/cursor.h>
#include <xview/dragdrop.h>
#include <xview_private/dndimpl.h>
#include <xview_private/windowimpl.h>

static int  SendDndEvent(),
	    Verification(),
	    ConstructSiteList(),
	    FindDropSite();

static int SendTrigger(Dnd_info *dnd, Xv_Drawable_info *info,
                       XButtonEvent *buttonEvent, int local);

static int
  SendOldDndEvent(Dnd_info *dnd, XButtonEvent *buttonEvent);

static int
  WaitForAck(Dnd_info *dnd, Xv_Drawable_info *info);

static int
  IsV2App(Display *dpy, Window window, Dnd_info *dnd, XButtonEvent *ev);

/* DND_HACK begin */
/* The code highlighted by the words DND_HACK is here to support dropping
 * on V2 clients.  The V3 drop protocol is not compatibile with the V2.
 * If we detect a V2 application, by a property on the its frame, we try
 * to send an V2 style drop event.   This code can be removed once we decide
 * not to support running V2 apps with the latest release.
 */
static Window FindLeafWindow();
/* DND_HACK end */

static void UpdateGrabCursor();
extern int  DndContactDSDM();
extern int  DndFindSite();
extern XID  DndGetCursor();

Xv_public int
dnd_send_drop(dnd_public)
    Xv_object	 dnd_public;
{
    Dnd_info    	*dnd = DND_PRIVATE(dnd_public);
    int		 	 status = DND_SUCCEEDED,
    			 dsdm_present = False,
			 button_released = False,
			 win_was_deaf = False,
			 stop = False;  /* Stop key pressed */
    Display		*dpy;
    XEvent       	*ev, xevent;
    Event		 event;
    Xv_Drawable_info	*info;
    Window_info		*win_info;

    DRAWABLE_INFO_MACRO(dnd->parent, info);
    dpy = xv_display(info);

				 /* Assure that we have a selection to use. */
    if (DndGetSelection(dnd, dpy) == DND_SELECTION)
	return(DND_SELECTION);

	/* Need to grab the keyboard to get STOP key events. */
#ifdef OW_I18N
    if (window_set_xgrabkeyboard(dnd->parent, dpy, xv_xid(info), FALSE, 
			     GrabModeAsync, GrabModeAsync, 
			     CurrentTime) != GrabSuccess) {
        status = DND_ERROR;
	goto BreakOut;
    }
#else
    if (XGrabKeyboard(dpy, xv_xid(info), FALSE, GrabModeAsync,
		      GrabModeAsync, CurrentTime) != GrabSuccess) {
        status = DND_ERROR;
	goto BreakOut;
    }
#endif 

    if (XGrabPointer(dpy, xv_xid(info), FALSE,
                     ButtonMotionMask|ButtonReleaseMask,
		     GrabModeAsync, GrabModeAsync, None, DndGetCursor(dnd),
		     CurrentTime) != GrabSuccess) {
        status = DND_ERROR;
#ifdef OW_I18N
	window_set_xungrabkeyboard(dnd->parent, dpy, CurrentTime);
#else
        XUngrabKeyboard(dpy, CurrentTime);
#endif
	goto BreakOut;
    }

				/* Contact DSDM. */
    if (XGetSelectionOwner(dpy, dnd->atom[DSDM]) != None) {
	if (DndContactDSDM(dnd)) 
	    dsdm_present = True;  /* XXX: sort list */
    }

    ev = &xevent;

    /* If the app is deaf (BUSY), set this window as un-deaf so that
     * xevent_to_event() will pass us the keyboard and mouse events.
     */
    win_info = WIN_PRIVATE(dnd->parent);
    if (WIN_IS_DEAF(win_info)) {
	WIN_SET_DEAF(win_info, False);
	win_was_deaf = True;
    }
    do {
	(void)xview_x_input_readevent(dpy, &event, dnd->parent, True, True,
				      ButtonMotionMask|ButtonReleaseMask|
				      KeyReleaseMask, ev);
	switch(event_action(&event)) {
	  case ACTION_SELECT:
	  case ACTION_ADJUST:
	  case ACTION_MENU:
	      if (event_is_up(&event)) {
		  unsigned int state = ev->xbutton.state;

		  /* Remove the button that was just released from the
		   * state field, then check and see if any other buttons
		   * are press.  If none are, then the drop happened.
		   */
		  state &= ~(1<<(ev->xbutton.button + 7));
		  if (!(state & (Button1Mask|Button2Mask|Button3Mask|
				 Button4Mask|Button5Mask)))
		      button_released = True;
	      }
	      break;
	  case LOC_DRAG:
	      if (dsdm_present) (void)DndFindSite(dnd, (XButtonEvent*)ev);
	      break;
	  case ACTION_STOP:
	      /* Send LeaveNotify if necessary */
	      if (dsdm_present) 
		  (void)DndSendPreviewEvent(dnd, DND_NO_SITE, ev);
	      stop = True;
	      break;
	  default:
	      break;
	}
    } while (!button_released && !stop);

    if (win_was_deaf)
	WIN_SET_DEAF(win_info, True);
    
    XUngrabPointer(dpy, CurrentTime);
#ifdef OW_I18N
    window_set_xungrabkeyboard(dnd->parent, dpy, CurrentTime);
#else
    XUngrabKeyboard(dpy, CurrentTime);
#endif

    if (stop) {
	status = DND_ABORTED;
	goto BreakOut;
    }

    if ((status = Verification(ev, dnd)) == DND_SUCCEEDED) {
	                    /* If drop site is within same process, optimize! */
	status = SendTrigger(dnd, info, (XButtonEvent *) ev,
                             win_data(dpy,dnd->dropSite.window));
    }

BreakOut:
    /* If the dnd pkg created a transient selection and the dnd operation
     * failed or was aborted, free up the transient selection.
     */
    if (status != DND_SUCCEEDED && dnd->transientSel)
	xv_set(dnd_public, SEL_OWN, False, NULL);

    if (dnd->siteRects) {
	xv_free(dnd->siteRects);
	dnd->siteRects = NULL;
    }

    return(status);
}

static int
SendTrigger(Dnd_info *dnd, Xv_Drawable_info *info,
            XButtonEvent *buttonEvent, int local)
{
    if (local) {
        int		 value;
        Xv_Server	 server = XV_SERVER_FROM_WINDOW(dnd->parent);
        Attr_attribute	 dndKey = xv_get(server, SERVER_DND_ACK_KEY);

    	xv_set(server, XV_KEY_DATA, dndKey, False, NULL);

    	if ((value = SendDndEvent(dnd, Dnd_Trigger_Local, 0, buttonEvent))
	     == DND_SUCCEEDED) {
            if ((int)xv_get(server, XV_KEY_DATA, dndKey))
	        value = DND_SUCCEEDED;
	    else
	        value = DND_TIMEOUT;
	}
	return (value);
    } else {
        if (SendDndEvent(dnd, Dnd_Trigger_Remote, 0, buttonEvent)
	    == DND_SUCCEEDED) {
            int	 value;
	    /* DND_HACK begin */
	    /*
	    return(WaitForAck(dnd, info));
 	    */
	    if ((value = WaitForAck(dnd, info)) == DND_TIMEOUT)
		if (dnd->is_old)
		    return(SendOldDndEvent(dnd, buttonEvent));
	    return(value);
	    /* DND_HACK end */
        } else
	    return(DND_ERROR);
    }
}

/* DND_HACK begin */
static int
SendOldDndEvent(Dnd_info *dnd, XButtonEvent *buttonEvent)
{
    Selection_requestor	 req;
    unsigned long	 length;
    int			 format;
    char		*data; 
    int			 i = 0;
    long		 msg[5];

    req = xv_create(dnd->parent, SELECTION_REQUESTOR,
			      SEL_RANK, (Atom)xv_get(DND_PUBLIC(dnd), SEL_RANK),
			      SEL_OWN,	True,
			      SEL_TYPE_NAME,	"FILE_NAME",
			      NULL);

    do {
        data = (char *)xv_get(req, SEL_DATA, &length, &format);
        if (length != SEL_ERROR)
	    break;
        else {
	    i++;
	    if (i == 1)
	        xv_set(req, SEL_TYPE, XA_STRING, NULL);
	    else if (i == 2)
	        xv_set(req, SEL_TYPE_NAME, "TEXT", NULL);
	    else
		return(DND_ERROR);
	}
	/* SUPPRESS 558 */
    } while(True);

    msg[0] = XV_POINTER_WINDOW;
    msg[1] = buttonEvent->x;
    msg[2] = buttonEvent->y;
    msg[3] = (long)xv_get(dnd->parent, XV_XID);
    msg[4] = (long)xv_get(XV_SERVER_FROM_WINDOW(dnd->parent),
				SERVER_ATOM, "DRAG_DROP");

    XChangeProperty(XV_DISPLAY_FROM_WINDOW(dnd->parent), msg[3],
                    msg[4], XA_STRING, 8, PropModeReplace,
                    (unsigned char *)data, strlen(data)+1);

    if (i == 0)
        xv_send_message(dnd->parent, dnd->dropSite.window, "XV_DO_DRAG_LOAD",
			32, (Xv_opaque *)msg, sizeof(msg));
    else if (dnd->type == DND_COPY)
        xv_send_message(dnd->parent, dnd->dropSite.window, "XV_DO_DRAG_COPY",
			32, (Xv_opaque *)msg, sizeof(msg));
    else
        xv_send_message(dnd->parent, dnd->dropSite.window, "XV_DO_DRAG_MOVE",
			32, (Xv_opaque *)msg, sizeof(msg));
    return(DND_SUCCEEDED);
}
/* DND_HACK end */

Xv_private int
DndSendPreviewEvent(dnd, site, e)
    Dnd_info 		*dnd;
    int			 site;
    XButtonEvent	*e;
{
    int i = dnd->eventSiteIndex;

		/* No Site yet */
    if (i == DND_NO_SITE) {
	dnd->eventSiteIndex = site;
		/* Moved into a new site */
	if (site != DND_NO_SITE) {
	    if (dnd->siteRects[site].flags & DND_ENTERLEAVE) {
	        if (SendDndEvent(dnd, Dnd_Preview, EnterNotify, e)
	            != DND_SUCCEEDED)
	            return (DND_ERROR); 
	    }
	    UpdateGrabCursor(dnd, EnterNotify);
	}
		/* Moved out of the event site */
    } else if (i != site) {
		/* Tell the old site goodbye */
	if (dnd->siteRects[i].flags & DND_ENTERLEAVE) {
	    if (SendDndEvent(dnd, Dnd_Preview, LeaveNotify, e)
		!= DND_SUCCEEDED)
		return (DND_ERROR);
	}
	UpdateGrabCursor(dnd, LeaveNotify);
	dnd->eventSiteIndex = site;
		/* Say hi to the new site */
	if (site != DND_NO_SITE) {
	    if (dnd->siteRects[site].flags & DND_ENTERLEAVE) {
	        if (SendDndEvent(dnd, Dnd_Preview, EnterNotify, e)
	            != DND_SUCCEEDED)
	            return (DND_ERROR); 
	    }
	    UpdateGrabCursor(dnd, EnterNotify);
	}
		/* Moving through the current event site */ 
    } else if (i == site) {
	if (dnd->siteRects[i].flags & DND_MOTION) {
	    if (SendDndEvent(dnd, Dnd_Preview, MotionNotify, e)
		!= DND_SUCCEEDED)
		return (DND_ERROR);
	}
    }
    return(DND_SUCCEEDED);
}

static void
UpdateGrabCursor(dnd, type)
    Dnd_info	 *dnd;
    int		  type;
{
    Xv_Drawable_info	*info;
    Cursor		 cursor;

    DRAWABLE_INFO_MACRO(dnd->parent, info);

	/* If no cursor, then jump out */
    if (dnd->affCursor)
	cursor = xv_get(dnd->affCursor, XV_XID);
    else if (dnd->affXCursor)
	cursor = dnd->affXCursor;
    else
	return;

    if (type == EnterNotify) 
	XChangeActivePointerGrab(xv_display(info),
				 ButtonMotionMask|ButtonReleaseMask,
				 cursor, CurrentTime);
    else
	XChangeActivePointerGrab(xv_display(info),
				 ButtonMotionMask|ButtonReleaseMask,
				 DndGetCursor(dnd), CurrentTime);
}

static int
WaitForAck(Dnd_info *dnd, Xv_Drawable_info *info)
{
    Display		*dpy = xv_display(info);
    XEvent	 	 event;
    XSelectionEvent	 selNotifyEvent;
    Atom		 property;
    int			 status = DND_SUCCEEDED,
    			 DndMatchProp(),
    			 DndMatchEvent();

			/* Wait for the dest to respond with an
			 * _SUN_DRAGDROP_ACK.
			 * XXX: Should check timestamp.  Sec 2.2 ICCCM
			 */
    if ((status = DndWaitForEvent(dpy, xv_xid(info), SelectionRequest,
			          dnd->atom[ACK], &dnd->timeout, &event,
				  DndMatchEvent))
				  != DND_SUCCEEDED)
	goto BailOut;

		/* Select for PropertyNotify events on requestor's window */
    XSelectInput(dpy, event.xselectionrequest.requestor, PropertyChangeMask);

		/* If the property field is None, the requestor is an obsolete
		 * client.   Sec. 2.2 ICCCM
		 */
    if (event.xselectionrequest.property == None)
       property = event.xselectionrequest.target;
    else
       property = event.xselectionrequest.property;

		/* If the destination ACK'ed us, send it back a NULL reply. */
		/* XXX: Should be prepared to handle bad alloc errors from the
		 * server.  Sec 2.2 ICCCM
		 */
    XChangeProperty(dpy, event.xselectionrequest.requestor,
		    property, event.xselectionrequest.target, 32,
		    PropModeReplace, (unsigned char *)NULL, 0);

    selNotifyEvent.type = SelectionNotify;
    selNotifyEvent.display = dpy;
    selNotifyEvent.requestor = event.xselectionrequest.requestor;
    selNotifyEvent.selection = event.xselectionrequest.selection;
    selNotifyEvent.target = event.xselectionrequest.target;
    selNotifyEvent.property = property;
    selNotifyEvent.time = event.xselectionrequest.time;

    /* SUPPRESS 68 */
    if (DndSendEvent(dpy, &selNotifyEvent) != DND_SUCCEEDED) {
        status = DND_ERROR;
        goto BailOut;
    }

    status = DndWaitForEvent(dpy, property,
			     PropertyNotify, NULL, &dnd->timeout, &event,
			     DndMatchProp);

	/* XXX: This will kill any events someone else has selected for. */
    XSelectInput(dpy, event.xproperty.window, NoEventMask); 
    XFlush(dpy);

BailOut:
    return(status);
}

Pkg_private int
DndWaitForEvent(dpy, window, eventType, target, timeout, event, MatchFunc)
    Display 		*dpy;
    Window		 window;
    int		 	 eventType;
    Atom		 target;
    struct timeval 	*timeout;
    XEvent		*event;
    int		       (*MatchFunc)();
{
    fd_set 	 	 xFd;
    int		  	 nFd;
    DnDWaitEvent 	 wE;

    wE.window = window;
    wE.eventType = eventType;
    wE.target = target;

    FD_ZERO(&xFd);

    XFlush(dpy);
    do {
	FD_SET(XConnectionNumber(dpy), &xFd);
	if (!(nFd = select(XConnectionNumber(dpy)+1, &xFd, NULL, NULL,timeout)))
	    return(DND_TIMEOUT);
	else if (nFd == -1) {
	    if (errno != EINTR)
	        return(DND_ERROR);
	} else {
	    if (XCheckIfEvent(dpy, event, MatchFunc, (char *)&wE))
	        return(DND_SUCCEEDED);
	}
	/* SUPPRESS 558 */
    } while (True);

    /*NOTREACHED*/
}

/*ARGSUSED*/
Pkg_private int
DndMatchEvent(dpy, event, wE)
    Display		*dpy;
    XEvent		*event;
    DnDWaitEvent	*wE;
{
    Atom	target = 0;

    if (event->type == SelectionNotify)
	target = event->xselection.target;
    else if (event->type == SelectionRequest)
	target = event->xselectionrequest.target;

    if ((event->type == wE->eventType) &&
	(event->xany.window == wE->window) &&
	(target == wE->target))
	return(True);
    else
	return(False);
}

static int
SendDndEvent(dnd, type, subtype, ev)
    Dnd_info 		*dnd;
    DndMsgType		 type;
    long		 subtype;
    XButtonEvent 	*ev;
{
    XClientMessageEvent  cM;
    Event		 event;

    cM.type = ClientMessage;
    cM.display = ev->display;
    cM.format = 32;

    switch(type) {
      case Dnd_Trigger_Remote: {
	  long flags = DND_ACK_FLAG;

	  if (dnd->type == DND_MOVE)
	      flags |= DND_MOVE_FLAG;
	  if (dnd->transientSel)
	      flags |= DND_TRANSIENT_FLAG;
	  if (dnd->dropSite.flags & DND_FORWARDED_FLAG)
	      flags |= DND_FORWARDED_FLAG;

          cM.message_type = dnd->atom[TRIGGER];
    	  cM.window = dnd->dropSite.window;
    	  cM.data.l[0] = (Atom)xv_get(DND_PUBLIC(dnd), SEL_RANK);
    	  cM.data.l[1] = ev->time;
    	  cM.data.l[2] = (ev->x_root << 16) | ev->y_root;
    	  cM.data.l[3] = dnd->dropSite.site_id;  /* Site ID */
    	  cM.data.l[4] = flags;
	  }
	break;
      case Dnd_Trigger_Local: {
	  int	 	x, y;
	  Window 	child;
	  Xv_Window 	dropObject = win_data(ev->display,
					      dnd->dropSite.window);
	  long 		flags = DND_ACK_FLAG;
				/* Indicate that this is a local event */
	  long 		local_flags = DND_LOCAL;

	  if (dnd->type == DND_MOVE)
	      flags |= DND_MOVE_FLAG;
	  if (dnd->transientSel)
	      flags |= DND_TRANSIENT_FLAG;
	  if (dnd->dropSite.flags & DND_FORWARDED_FLAG) {
	      flags |= DND_FORWARDED_FLAG;
	      local_flags |= DND_FORWARDED;
	  }

          cM.message_type = dnd->atom[TRIGGER];
	  cM.serial = 0L; 		/* XXX: This is incorrect. */
	  cM.send_event = True;
    	  cM.window = dnd->dropSite.window;
    	  cM.data.l[0] = (Atom)xv_get(DND_PUBLIC(dnd), SEL_RANK);
    	  cM.data.l[1] = ev->time;
    	  cM.data.l[2] = (ev->x_root << 16) | ev->y_root;
    	  cM.data.l[3] = dnd->dropSite.site_id;  /* Site ID */
    	  cM.data.l[4] = flags;

    	  event_init(&event);
    	  event_set_window(&event, dropObject);
    	  event_set_action(&event,
			  (dnd->type == DND_MOVE ? ACTION_DRAG_MOVE :
							     ACTION_DRAG_COPY));

	  if (!XTranslateCoordinates(ev->display, ev->window,
				      dnd->dropSite.window, ev->x,
				      ev->y, &x, &y, &child)) {
	      /* Different Screens */
	      return(DND_ERROR);
	  }

	  /* Set the time of the trigger event */
	  event.ie_time.tv_sec = ((unsigned long) ev->time) / 1000;
	  event.ie_time.tv_usec = (((unsigned long) ev->time) % 1000) * 1000;

    	  event_set_x(&event, x);
    	  event_set_y(&event, y);
				/* Indicate that this is a local event */
    	  event_set_flags(&event, local_flags);
          event_set_xevent(&event, (XEvent *)&cM);

          if (win_post_event(dropObject, &event, NOTIFY_IMMEDIATE) != NOTIFY_OK)
	  	return(DND_ERROR);

	  return(DND_SUCCEEDED);
	}
        /*NOTREACHED*/
	break;
      case Dnd_Preview: {
	  Xv_Window 	eventObject = win_data(ev->display,
				    dnd->siteRects[dnd->eventSiteIndex].window);

          cM.message_type = dnd->atom[PREVIEW];
    	  cM.window = dnd->siteRects[dnd->eventSiteIndex].window;
    	  cM.data.l[0] = subtype;
    	  cM.data.l[1] = ev->time;
    	  cM.data.l[2] = (ev->x_root << 16) | ev->y_root;
    	  cM.data.l[3] = dnd->siteRects[dnd->eventSiteIndex].site_id;
	  if (dnd->siteRects[dnd->eventSiteIndex].flags & DND_FORWARDED_FLAG)
    	      cM.data.l[4] = DND_FORWARDED_FLAG;
	  else
    	      cM.data.l[4] = 0;

	  if (eventObject) {
	      int	x, y;
	      Window	child;
	      long	local_flags = DND_LOCAL;

    	      event_init(&event);
    	      event_set_window(&event, eventObject);
	      switch (subtype) {
                case EnterNotify:
                  event_set_id(&event, LOC_WINENTER);
                  break;
                case LeaveNotify:
                  event_set_id(&event, LOC_WINEXIT);
                  break;
                case MotionNotify:
                  event_set_id(&event, LOC_DRAG);
                  break;
	      }
    	      event_set_action(&event, ACTION_DRAG_PREVIEW);

	      /* XXX: This can be improved.  Roundtrip for local preview 
	       * events is not neccessary.
	       */
	      if (!XTranslateCoordinates(ev->display, ev->root,
				    dnd->siteRects[dnd->eventSiteIndex].window,
				    ev->x_root, ev->y_root, &x, &y, &child)) {
	          /* XXX: Different Screens */
		  return(DND_ERROR);
	      }

    	      event_set_x(&event, x);
    	      event_set_y(&event, y);

	      event.ie_time.tv_sec = ((unsigned long) ev->time) / 1000;
	      event.ie_time.tv_usec = (((unsigned long) ev->time) % 1000)*1000;

	      if (dnd->siteRects[dnd->eventSiteIndex].flags &
							     DND_FORWARDED_FLAG)
	          local_flags |= DND_FORWARDED;

    	      event_set_flags(&event, local_flags);
              event_set_xevent(&event, (XEvent *)&cM);

              if (win_post_event(eventObject, &event, NOTIFY_IMMEDIATE)
		  != NOTIFY_OK)
	          return(DND_ERROR);

	      return(DND_SUCCEEDED);
	  }
        }
        break;
      default:
	  return(DND_ERROR);
    }
    /* SUPPRESS 68 */
    return(DndSendEvent(ev->display, &cM));
}

/*
 * See if the window has set the  _SUN_DRAGDROP_INTEREST atom.  If so, see if
 * the point at which the drop occured happed within a registered drop site.
 */
static int
Verification(ev, dnd)
    XButtonEvent *ev;
    Dnd_info	 *dnd;	
{
    Window	  child;
    Display	 *dpy = ev->display;
    Window	  srcXid = ev->root,
		  dstXid = ev->root;
    int		  srcX = ev->x_root,
		  srcY = ev->y_root,
		  dstX, dstY,
		  root_window = False,
		  window_count = 0;
    long	 *interest_prop = NULL;

    /* DND_HACK begin */
    dnd->is_old = False;
    /* DND_HACK end */

    do {
	if (!XTranslateCoordinates(dpy, srcXid, dstXid, srcX, srcY, &dstX,
				   &dstY, &child))
	    return(DND_ERROR); /* XXX: Different Screens !! */

	srcXid = dstXid; dstXid = child;
	srcX = dstX;     srcY = dstY;

	/* If this is the first time through the loop and child == NULL
	 * meaning there are no mapped childern of the root, assume it
	 * is a root window drop.  This of course won't work if someone
	 * is using a virtual window manager.
	 */
	if (!window_count && !child)
	    root_window = True;

	window_count++;

	if (child) {
	    Atom  type;
	    int   format;
	    unsigned long   nitems, remain;
	    unsigned char *data;

				/* Look for the interest property set on
				 * the icon, wm frame, or an application's
				 * top level window.
				 */
            if (XGetWindowProperty(dpy, child, dnd->atom[INTEREST], 0L, 65535L,
		       	           False, AnyPropertyType, &type, &format,
			           &nitems, &remain, &data)
			           != Success) {
	        return(DND_ERROR);
	    } else if (type != None) {
    		Window	  ignore;
				/* Remember the last interest property we
				 * see on our way out to the WM_STATE prop.
				 */
		if (interest_prop)
		    XFree((char *)interest_prop);
		interest_prop = (long *)data;

		(void)XTranslateCoordinates(dpy, srcXid, dstXid, srcX, srcY,
					    &dstX, &dstY, &ignore);
				/* Save x,y coords of top level window
				 * of drop site.  Will use later.
				 */
		dnd->drop_target_x = dstX;  
		dnd->drop_target_y = dstY;
	    }
				/* Look for the WM_STATE property set on a
				 * window as we traverse out to the leaf
				 * windows.  If we see WM_STATE, then the user
				 * dropped over an application, not the root.
				 */
	    if (XGetWindowProperty(dpy, child, dnd->atom[WMSTATE], 0L, 2L,
				   False, AnyPropertyType, &type, &format,
				   &nitems, &remain, &data) != Success)
	        return(DND_ERROR);
	    else if (type != None) {
		XFree((char *)data);
				/* If we saw the WM_STATE but didn't see an
				 * interest property, then this could be a
				 * V2 application.
				 */
		if (!interest_prop) {
                    return(IsV2App(dpy, child, dnd, ev));
	        } else {
				/* If we saw an interest property and saw
				 * the WM_STATE property, then it's safe
				 * to jump out of this loop.
				 */
		    break;
		}
	    }
	}
    } while(child);

    /* XXX: Revisit once we work out root window drops */
    /* The idea here is that if we only do one xtranslate before we hit the
     * root window, then the user dropped on the root.
     */
    if (root_window) {
	return(DND_ROOT);
    } else if (interest_prop) {
	Dnd_site_desc *drop_site;
	unsigned int   nsites;
				/* This might be a legal drop site. We will
				 * need to dig out the _SUN_DRAGDROP_INTEREST
				 * property, construct the rects and determine
				 * if the drop happened within a drop region.
				 */
	(void)ConstructSiteList(dpy, NULL, interest_prop, &drop_site,
				&nsites);
	XFree((char *)interest_prop);
	if (!FindDropSite(dnd, drop_site, nsites, &dnd->dropSite)) { 
	    if (nsites) {  /* The site may not have any regions. */
	        XFree((char *)drop_site->rect);
	        XFree((char *)drop_site);
	    }
            return(IsV2App(dpy, child, dnd, ev));
	} else {
	    XFree((char *)drop_site->rect);
	    XFree((char *)drop_site);
	    return(DND_SUCCEEDED);
	}
    }

    return(DND_ILLEGAL_TARGET);
}

/* DND_HACK begin */
static int
IsV2App(Display *dpy, Window window, Dnd_info *dnd, XButtonEvent *ev)
{
    Atom  	 type;
    int          format;
    unsigned long   	 nitems, remain;
    unsigned char 	*data;
    Atom 	 v2_app = xv_get(XV_SERVER_FROM_WINDOW(dnd->parent),
				 SERVER_ATOM, "_XVIEW_V2_APP");

    if (!window)
        return(DND_ILLEGAL_TARGET);

    if (XGetWindowProperty(dpy, window, v2_app, 0L, 1L,
			   False, AnyPropertyType, &type, &format,
			   &nitems, &remain, &data) != Success) {
	return(DND_ERROR);
    } else if (type == None) {
        return(DND_ILLEGAL_TARGET);
    } else {
        dnd->dropSite.site_id = 0;
	dnd->dropSite.window = FindLeafWindow(ev);
        dnd->is_old = True;
	XFree((char *)data);
	return(DND_SUCCEEDED);
    }
}
/* DND_HACK end */

/* DND_HACK begin */
static Window
FindLeafWindow(ev)
    XButtonEvent *ev;
{
    Display	*dpy = ev->display;
    Window	 srcXid = ev->window,
		 dstXid = ev->root;
    int		 srcX = ev->x,
		 srcY = ev->y,
		 dstX, dstY;
    Window	 child;
    do {
	if (!XTranslateCoordinates(dpy, srcXid, dstXid, srcX, srcY, &dstX,
				   &dstY, &child))
	    return(DND_ERROR); /* XXX: Different Screens !! */

	if (!child) break;

	srcXid = dstXid; dstXid = child;
	srcX = dstX;     srcY = dstY;
    } while (child);

    return(dstXid);
}
/* DND_HACK end */

static int
ConstructSiteList(dpy, dest_window, prop, dsite, nsites)
    Display		*dpy;
    Window		 dest_window;
    long 		*prop;
    Dnd_site_desc      **dsite;
    unsigned int	*nsites;
{
    long		*data; 
    unsigned int  	 i, j;
    Dnd_site_desc	*drop_site;

    data = prop;
    (void)*data++;		/* Version Number */	

    *nsites = (int)*data++;

    drop_site = (Dnd_site_desc *)xv_calloc(*nsites, sizeof(Dnd_site_desc));

    for (i = 0; i < *nsites; i++) {
	drop_site[i].window = (Window)*data++;
	drop_site[i].site_id = (long)*data++;
	drop_site[i].flags = (unsigned long)*data++;   /* flags */
	if (*data++ == DND_RECT_SITE) {
	    drop_site[i].nrects = (int)*data++;
	    drop_site[i].rect = (DndRect *)xv_calloc(drop_site[i].nrects,
					   	     sizeof(DndRect));
	    for (j = 0; j < drop_site[i].nrects; j++) {
	        drop_site[i].rect[j].x = (int)*data++;
	        drop_site[i].rect[j].y = (int)*data++;
	        drop_site[i].rect[j].w = (unsigned int)*data++;
	        drop_site[i].rect[j].h = (unsigned int)*data++;
	    }
	} else { /* DND_WINDOW_SITE */
	    drop_site[i].nrects = (int)*data++;
	    drop_site[i].rect = (DndRect *)xv_calloc(drop_site[i].nrects,
						     sizeof(DndRect));
	    for (j = 0; j < drop_site[i].nrects; j++) {
		Window 		root;
		int		x, y;
		unsigned int 	w, h;
		unsigned int 	bw, depth;

		if (!XGetGeometry(dpy, (Window)*data,
				 &root, &x, &y, &w,
				 &h, &bw, &depth)) {
		    /* XXX: Handle Bad Window */
		    (void)*data++;
		    /* XXX: Skip it for now */
	            drop_site[i].rect[j].x = 0;
	            drop_site[i].rect[j].y = 0;
	            drop_site[i].rect[j].w = 0;
	            drop_site[i].rect[j].h = 0;
		} else {
		    int dstX, dstY;
		    Window child;

			/* Window coords must be in coord space of top level
			 * window.
			 */
		    if (!XTranslateCoordinates(dpy, (Window)*data++,
					       dest_window, x, y,
					       &dstX, &dstY, &child)) {
			/* Different Screens */
			break;
		    } else {
	                drop_site[i].rect[j].x = dstX;
	                drop_site[i].rect[j].y = dstY;
	                drop_site[i].rect[j].w = w + 2 * bw; /*Includes border*/
	                drop_site[i].rect[j].h = h + 2 * bw;
		    }
		}
	    }
	}
    }
    *dsite = drop_site; 
    return(DND_SUCCEEDED);
}

static int
FindDropSite(dnd, dsl, nsites, site)
    Dnd_info		*dnd;	
    Dnd_site_desc	*dsl;		/* drop site list */  
    unsigned int 	 nsites;
    Dnd_site_desc	*site;		
{
    int i, j;

    for (i = 0; i < nsites; i++) {
	for (j = 0; j < dsl[i].nrects; j++) {
	    if (DND_POINT_IN_RECT(&dsl[i].rect[j], dnd->drop_target_x,
				  dnd->drop_target_y)) {
		site->window = dsl[i].window;
		site->site_id = dsl[i].site_id;
		site->flags = dsl[i].flags;
		return(True);
	    }
	}
    }
    return(False);
}
