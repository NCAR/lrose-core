#ifndef lint
#ifdef SCCS
static char     sccsid[] = "@(#)sel_own.c 1.50 93/06/29";
#endif
#endif

/*
 *	(c) Copyright 1990 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */


#include <xview_private/i18n_impl.h>
#include <xview_private/sel_impl.h>
#include <xview/window.h>
#include <X11/Xproto.h>
#ifdef SVR4 
#include <stdlib.h> 
#endif /* SVR4 */

Pkg_private char *xv_sel_atom_to_str(/* display, atom */);
Pkg_private int xv_sel_add_prop_notify_mask();
Pkg_private int xv_sel_block_for_event();
Pkg_private int xv_sel_predicate();
Pkg_private Sel_atom_list *xv_sel_find_atom_list();
Pkg_private Sel_prop_list *xv_sel_get_prop_list();
Xv_private Time xv_sel_get_last_event_time();
Pkg_private Sel_owner_info  *xv_sel_find_selection_data();
Pkg_private Sel_owner_info  *xv_sel_set_selection_data();

static int DoConversion();
static int OwnerHandleReply();
static int SelLoseOwnership();
static int sel_set_ownership(/* sel_owner */);
static int (*OldErrorHandler)();
static int SelOwnerErrorHandler();
static int ValidatePropertyEvent();

static void SelClean();
static void HandleMultipleReply();
static void ReplyTimestamp();
static void SendIncrMessage();
static void SetupPropInfo();
static void OwnerProcessIncr();
static void RegisterSelClient();

static int
  SendIncr( Sel_owner_info  *seln );

extern XContext  selCtx;
extern XContext  propCtx;
extern XContext  reqCtx;
extern XContext  targetCtx;


/*ARGSUSED*/
Pkg_private int
sel_owner_init(parent, sel_owner_public, avlist)
Xv_Window	    parent;
Selection_owner sel_owner_public;
Attr_avlist	    avlist;
{
	Sel_owner_info *sel_owner;
	Xv_sel_owner   *sel_owner_object = (Xv_sel_owner *) sel_owner_public;

	/* Allocate and clear private data */
	sel_owner = xv_alloc(Sel_owner_info);

	/* Link private and public data */
	sel_owner_object->private_data = (Xv_opaque) sel_owner;
	sel_owner->public_self = sel_owner_public;

	/* Initialize private data */
	sel_owner->convert_proc = sel_convert_proc;
	sel_owner->dpy = (Display *) XV_DISPLAY_FROM_WINDOW( parent );
	sel_owner->xid = (XID) xv_get( parent, XV_XID );

	return XV_OK;
}


/*ARGSUSED*/
Pkg_private Xv_opaque
sel_owner_set_avlist(sel_owner_public, avlist)
Selection_owner sel_owner_public;
Attr_avlist	    avlist;
{
    Attr_avlist	    attrs;
    Sel_owner_info *sel_owner = SEL_OWNER_PRIVATE(sel_owner_public);
    int   owner=FALSE;
    int   result;

    /*
     * Parse Selection attributes before Selection_owner attributes.
     * The call to xv_super_set_avlist is very inefficient. This call needs
     * to be changed to a more efficient way of processing the object parent 
     * attrs before the object itself.
     */    
    result = xv_super_set_avlist( sel_owner_public, &xv_sel_owner_pkg, avlist );
    if ( result !=  XV_OK )
        return result;
    
    for (attrs = avlist; *attrs; attrs = attr_next(attrs)) {
        switch (attrs[0]) {
	  case SEL_CONVERT_PROC:
	    sel_owner->convert_proc = (Bool (*) ()) attrs[1];

	    /* Shouldn't let clients set convert_proc to NULL */
	    if ( sel_owner->convert_proc == NULL )
	        sel_owner->convert_proc = sel_convert_proc;
	    break;
	  case SEL_DONE_PROC:
	    sel_owner->done_proc = (void (*) ()) attrs[1];
	    break;
	  case SEL_LOSE_PROC:
	    sel_owner->lose_proc = (void (*) ()) attrs[1];
	    break;
	  case SEL_OWN:
	    if (sel_owner->own != (Bool) attrs[1]  ) {
	        if ( !(Bool) attrs[1] )
		    SelLoseOwnership( sel_owner );
		else
		    owner=TRUE;
	    }
	    break;
        }
    }
    if ( owner )
        return sel_set_ownership(sel_owner);

    return XV_OK;
}


/*Argsused*/
Pkg_private Xv_opaque
sel_owner_get_attr(sel_owner_public, status, attr, valist)
Selection_owner sel_owner_public;
int		   *status;
Attr_attribute  attr;
va_list	    valist;
{
    Sel_owner_info *sel_owner = SEL_OWNER_PRIVATE(sel_owner_public);
    int		    arg;
    Sel_item_info  *ip;

    switch (attr) {
      case SEL_CONVERT_PROC:
	return (Xv_opaque) sel_owner->convert_proc;
      case SEL_DONE_PROC:
	return (Xv_opaque) sel_owner->done_proc;
      case SEL_FIRST_ITEM:
        if (sel_owner->first_item) 
	   return (Xv_opaque) SEL_ITEM_PUBLIC(sel_owner->first_item);
        else
           return (Xv_opaque) 0;
      case SEL_LOSE_PROC:
	return (Xv_opaque) sel_owner->lose_proc;
      case SEL_NEXT_ITEM:
        arg = va_arg(valist, int);
	ip = SEL_ITEM_PRIVATE((Selection_item)arg);
	if (ip->next) {
	    return (Xv_opaque) SEL_ITEM_PUBLIC(ip->next);
	} else {
	    return XV_ZERO;
	}
      case SEL_OWN:
	return (Xv_opaque) sel_owner->own;
      case SEL_PROP_INFO:	
	SetupPropInfo( sel_owner );
	return (Xv_opaque) sel_owner->propInfo;
      default:
	*status = XV_ERROR;
	return (Xv_opaque) 0;
    }
}


Pkg_private int
sel_owner_destroy(sel_owner_public, status)
Selection_owner sel_owner_public;
Destroy_status  status;
{
    Sel_owner_info *sel_owner = SEL_OWNER_PRIVATE(sel_owner_public);
    Sel_atom_list  *list=NULL;
    Sel_prop_list  *pList;
    
    if (status == DESTROY_CHECKING || status == DESTROY_SAVE_YOURSELF
         || status == DESTROY_PROCESS_DEATH)
	return XV_OK;

    if ( sel_owner->own )
        SelLoseOwnership( sel_owner );

    XDeleteContext( sel_owner->dpy, (Window) sel_owner->selection, selCtx );
 

/*
    if (!XFindContext( sel_owner->dpy, DefaultRootWindow(sel_owner->dpy), targetCtx, 
		      (caddr_t *)&list ) ) 
        XFree( (char *) list );
    XDeleteContext( sel_owner->dpy, DefaultRootWindow(sel_owner->dpy), targetCtx );
    targetCtx = 0;

    if ( !XFindContext( sel_owner->dpy, DefaultRootWindow(sel_owner->dpy), propCtx, 
						(caddr_t *)&pList ) ) 
        XFree( (char *) pList );

    XDeleteContext( sel_owner->dpy, DefaultRootWindow(sel_owner->dpy), propCtx );
    propCtx = 0;
*/
    

    /* Free up malloc'ed storage */

    RegisterSelClient( sel_owner, SEL_DELETE_CLIENT );
    
    XFree( (char *) sel_owner );

    return XV_OK;
}


Xv_public Bool	/* True = success, False = failure */
sel_convert_proc(sel_owner_public, type, data, length, format )
Selection_owner sel_owner_public;
Atom	        *type;
Xv_opaque	*data;
unsigned long	*length;
int		*format;
{
    Sel_item_info  *ip;
    Sel_owner_info *sel_owner = SEL_OWNER_PRIVATE(sel_owner_public);

    for (ip = sel_owner->first_item; ip; ip = ip->next) {
	if (ip->type == *type) {
	    *data = ip->data;
	    *length = ip->length;
	    *format = ip->format;
	    return TRUE;
	}
    }

    /*
     * If type is TARGETS, we should return a list of atoms representing
     * the targets for which an attempt to convert the current selection
     * will succeed.
     */
    if ( *type == sel_owner->atomList->targets )  {
	Atom  *atomList = xv_alloc( Atom );
	int   i;
	
	for (i=0,ip = sel_owner->first_item; ip; ip = ip->next, i++) {
	    atomList[i] = ip->type;
	    atomList = (Atom *) xv_realloc( (char *) atomList, 
					(unsigned) ((i+2) *sizeof( Atom ) ) );
	}
	/* Now, add in the TARGETS and TIMESTAMP */
	atomList[i] = sel_owner->atomList->targets;
	atomList = (Atom *) xv_realloc( (char *) atomList, 
				    (unsigned) ((i+2) *sizeof( Atom ) ) );
	i++;
	atomList[i] = sel_owner->atomList->timestamp;
	atomList = (Atom *) xv_realloc( (char *) atomList, 
				    (unsigned) ((i+2) *sizeof( Atom ) ) );

	i++;
	*format = 32;
	*data = (Xv_opaque) atomList;
	*length = i;
	return TRUE;
    }

    return FALSE;
}




static int
sel_set_ownership( sel_owner )
Sel_owner_info *sel_owner;
{
    Atom             seln;
    Sel_owner_info   *owner;
    Selection_owner  sel_owner_public;
    struct  timeval  *time;
    Time             lastEventTime;

    sel_owner_public = SEL_OWNER_PUBLIC( sel_owner );

    /* Get the selection rank  */
    seln = xv_get( sel_owner_public, SEL_RANK );

    owner = xv_sel_set_selection_data( sel_owner->dpy, seln, sel_owner );

    /* get the time;if it hasn't been set, set it */
    time = (struct timeval *) xv_get( sel_owner_public, SEL_TIME );

    owner->time = xv_sel_cvt_timeval_to_xtime( time );
    lastEventTime = xv_sel_get_last_event_time( sel_owner->dpy, sel_owner->xid );
    
    if ( owner->time == XV_ZERO || owner->time < lastEventTime ) { 
        owner->time = lastEventTime;
	xv_set( sel_owner_public, 
	       SEL_TIME, xv_sel_cvt_xtime_to_timeval( owner->time ), 
	       NULL );
    }
    
    if ( seln != None ) {
	/*
	 * Compatibility routine between the old and the new selection packages.
	 * When the textsw is converted to use the new selection package, this
	 * routine can be deleted.
	 */
	xv_sel_send_old_pkg_sel_clear( owner->dpy, owner->selection, owner->xid, owner->time );
	/* End compatibility routines */

	/* Become the selection owner */
	XSetSelectionOwner( owner->dpy, seln, owner->xid, owner->time );
	if ( XGetSelectionOwner( owner->dpy, seln ) != owner->xid )  {

	    xv_error( (Xv_opaque)owner,
                       ERROR_STRING, XV_MSG("Selection ownership failed"),
                       ERROR_PKG, SELECTION,
                       NULL);

	    XDeleteContext( owner->dpy,seln, selCtx );
	    owner->own = FALSE;

	    return XV_ERROR;
	}
	owner->own = TRUE;

	RegisterSelClient( owner, SEL_ADD_CLIENT );
	/*
	 * Compatibility routine between the old and the new selection packages.
	 * When the textsw is converted to use the new selection package, this
	 * routine can be deleted.
	 */
	xv_sel_set_compat_data( owner->dpy, owner->selection, owner->xid, NEW_SEL_CLIENT );
	/* End compatibility routines */
    }

    return XV_OK;
}


static int
SelLoseOwnership( sel_owner )
Sel_owner_info  *sel_owner;
{
    struct  timeval  time;

    /*
     * If we are processing a transaction now, don't lose the ownership
     * until we are done.
     */
    if ( sel_owner->status & SEL_BUSY )  {
	sel_owner->status |= SEL_LOSE;
	return FALSE;
    }

    /*
     * Compatibility routine between the old and the new selection packages.
     * When the textsw is converted to use the new selection package, this
     * routine can be deleted.
     */
    xv_sel_free_compat_data( sel_owner->dpy, sel_owner->selection );    

    XSetSelectionOwner( sel_owner->dpy, sel_owner->selection, 
		       None, sel_owner->time );

    if ( sel_owner->lose_proc != NULL )
        (* sel_owner->lose_proc)( sel_owner->public_self );

    /*
     * Set the time to zero so that when we become the selection owner
     * again we get the correct time.
     */    
    time.tv_sec = 0;
    time.tv_usec = 0;
    xv_set( sel_owner->public_self, SEL_TIME, &time, 0 );

    sel_owner->time = 0;
    sel_owner->own = FALSE;

    XDeleteContext( sel_owner->dpy, sel_owner->xid, selCtx );
    return TRUE;
}

static void
SetupReplyEvent( replyEvent, reqEvent )
XSelectionEvent *replyEvent;
XSelectionRequestEvent  *reqEvent;
{
    replyEvent->type = SelectionNotify;
    replyEvent->display = reqEvent->display;
    replyEvent->requestor = reqEvent->requestor;
    replyEvent->selection = reqEvent->selection;
    replyEvent->property = reqEvent->property;
    replyEvent->target = reqEvent->target;
    replyEvent->time = reqEvent->time;
}



Xv_private int
xv_sel_handle_selection_request( reqEvent )
XSelectionRequestEvent  *reqEvent;
{
    Sel_owner_info   *owner;
    Requestor        *req;
    XSelectionEvent  replyEvent;

    owner = xv_sel_find_selection_data( reqEvent->display, reqEvent->selection, 
					reqEvent->owner );


#ifdef SEL_DEBUG1
	printf("SelectionREquest recieved  selection = %s property = %s target = %s \n",XGetAtomName(reqEvent->display,reqEvent->selection),XGetAtomName(reqEvent->display,reqEvent->property),XGetAtomName(reqEvent->display,reqEvent->target));
#endif

    /* Is this event for this window? */
    if ( owner->xid != reqEvent->owner ) 
        return  FALSE;

    SetupReplyEvent( &replyEvent, reqEvent );

    /*
     * Compare the timestamp with the period we owned the selection, if the
     * time is outside, refuse the SelectionRequest.
     */
    if (( owner->time >= reqEvent->time) && (reqEvent->time!=CurrentTime) )  {
        goto Refuse;
    }
    
    /* Is this event for the selection that we register for? */
    if ( owner->selection != reqEvent->selection ) 
        goto Refuse;


    if ( reqEvent->property == None )
    {
	/* An obsolete client; use the "target" atom as the property name for
	 * the reply.
	 */
	if ( reqEvent->target == owner->atomList->multiple )
	    /* The MULTIPLE target atom is valid only when a property is 
	     * specified.
	     */
            goto Refuse;	    
        else
	    replyEvent.property = reqEvent->target;
    }
    owner->status = 0;

    (void)XSaveContext( reqEvent->display, DefaultRootWindow(reqEvent->display), 
		       selCtx, (caddr_t)&owner->status );

    owner->property = reqEvent->property;
    req = xv_alloc( Requestor );
    req->requestor = reqEvent->requestor;
    req->target = reqEvent->target;
    req->property = reqEvent->property;
    req->time = reqEvent->time;
    req->multiple = FALSE;
    req->timeout = xv_get( owner->public_self, SEL_TIMEOUT_VALUE );
    req->incr = FALSE;
    req->numIncr = 0;
    owner->req = req;
    req->owner = owner;

    /*
     * Save the client's error handler.
     */
    if ( OldErrorHandler == NULL )  
        OldErrorHandler = XSetErrorHandler( SelOwnerErrorHandler );
    
    if ( OwnerHandleReply( owner, &replyEvent ) )    
        if ( replyEvent.property == None )  {
	    /*
	     * We have finished transferring INCR. DO not need to send 
	     * PropertyNotify.
	     */
    	    XFree( (char *) owner->req );
            owner->req = NULL;
	    return( TRUE );
	}

    /*
     * Send the requestor window a SelectionNotify event specifying the
     * the end of transaction.
     */
    (void) XSendEvent(owner->dpy, replyEvent.requestor, False,(unsigned long)NULL,
		      (XEvent *) &replyEvent );
    
#ifdef SEL_DEBUG1     
    printf("Sending SelectionNotify Event win = %d \n",replyEvent.requestor);
#endif

    OwnerProcessIncr( owner );
    
    SelClean( owner );
    return( TRUE );

Refuse:
    replyEvent.property = None;
    (void) XSendEvent(owner->dpy, replyEvent.requestor, False, (unsigned long)NULL,
		  (XEvent *) &replyEvent );
    SelClean( owner );
    return( TRUE );
}


static void
OwnerProcessIncr( sel )
Sel_owner_info   *sel;
{
    int  i;
    Requestor  *req;
	int  numIncr = sel->req->numIncr;

    for ( i=0; i< numIncr; i++ )   {
	if ( sel->req->incrPropList[i] != 0 )    {
	    if ( XFindContext( sel->dpy, sel->req->incrPropList[i], 
			      reqCtx, (caddr_t *)&req )) 
	        continue;

		req->incrPropList = sel->req->incrPropList;
	    req->owner->req = req;
	    xv_sel_handle_incr( req->owner );	
	}
    }
}


static int 
SelOwnerErrorHandler( dpy, error )
Display      *dpy;
XErrorEvent  *error;
{
    if ( error->request_code == X_GetProperty ||
	error->request_code == X_ChangeProperty )  {
	int   *status;
	
	if (XFindContext( dpy, DefaultRootWindow(dpy), selCtx, (caddr_t *)&status)) 
	    return FALSE;
	*status |= SEL_INTERNAL_ERROR;
        return TRUE;
    }
	else  {	    
        (*OldErrorHandler) (dpy, error );
        return TRUE;
    }
}


    

static void
SelClean( owner )
Sel_owner_info   *owner;
{
    XWindowAttributes  winAttr;    
    Requestor	      *req;
    /*
     * If we were asked to lose the selection ownership while
     * we were processing, lose the selection ownership.
     */
    if ( owner->status & SEL_LOSE )  {
	owner->status = 0;
        SelLoseOwnership( owner );
    }
    if (owner->req == NULL) 
       return;
    /* 
     * If the requestor is dead, free and exit.
     */
    if ( !(owner->status & SEL_INTERNAL_ERROR) )  {	
	/* 
	 * If we have added PropertyChangeMask to the win, reset the mask to
	 * it's original state.
	 */
	if ( owner->status & SEL_ADD_PROP_NOTIFY ) {
	    unsigned  int  mask;

	    mask = PropertyChangeMask;
	    XGetWindowAttributes( owner->dpy, owner->req->requestor, 
				 &winAttr );  
	    
	    XSelectInput( owner->dpy, owner->req->requestor, 
			 winAttr.your_event_mask & (~mask) );
	}
    }
    
    owner->status = 0;    

    if (XFindContext(owner->dpy, owner->req->property, reqCtx,
		     (caddr_t *)&req) != XCNOENT)
	XDeleteContext(owner->dpy, owner->req->property, reqCtx);

    XFree( (char *) owner->req );
    owner->req = NULL;
}    

static int
OwnerHandleReply( owner, replyEvent )
Sel_owner_info      *owner;
XSelectionEvent   *replyEvent;
{
    /* 
     * We are going to a busy state.
     */
    owner->status |= SEL_BUSY;
    
    /* Handle MULTIPLE target */
    if ( owner->req->target == owner->atomList->multiple )  {
	owner->req->multiple = TRUE;
        HandleMultipleReply( owner );
    }
    else {   /* Handle normal */
	if (DoConversion(owner,owner->req->target,owner->req->property,0 )) {
	    replyEvent->property = owner->req->property;
	    return TRUE;
	}
	else {
	    replyEvent->property = None;
	    return FALSE;
	}
	
    }
    return TRUE;
}



static void
HandleMultipleReply( seln )
Sel_owner_info      *seln;
{
    atom_pair      *atomPair;
    unsigned long  bytesafter;
    unsigned char  *prop;
    unsigned long  length;
    int            format;
    Atom           target;
    int            byteLen, set=0;
    int            ret, multipleIndex=SEL_MULTIPLE, firstTarget=1;
    

    /*
     * The contents of the property named in the request is a list
     * of atom pairs, the first atom naming a target, and the second
     * naming a property. Do not delete the data. The requestor needs to
     * access this data.
     */

    if ( XGetWindowProperty( seln->dpy, seln->req->requestor,
			    seln->req->property, 0L, 1000000,
			    False,(Atom)AnyPropertyType, &target, &format, 
			    &length, &bytesafter, &prop) != Success )
        xv_error( seln->public_self, 
                  ERROR_STRING, XV_MSG("XGetWindowProperty Failed"),
                  ERROR_PKG, SELECTION,
                  0 );

    byteLen = BYTE_SIZE( length, format ) / sizeof( atom_pair );

        
    for ( atomPair = ( atom_pair *)prop; byteLen; atomPair++, byteLen--) {

	if ( firstTarget ) {
	    multipleIndex = SEL_BEGIN_MULTIPLE;	    
	    firstTarget = 0;
	}
	
	if ( !(byteLen - 1 ) )
	    multipleIndex = SEL_END_MULTIPLE;

#ifdef SEL_DEBUG	
	printf("HandleMultipleReply: property=%d  target=%d\n", 
	       atomPair->property, atomPair->target );
#endif
	

	ret =  DoConversion(seln,atomPair->target,atomPair->property,multipleIndex);
	if (!ret ) {
	    /*
	     * Replace in the MULTIPLE property any property atoms for targets
	     * it failed to convert with None.
	     */
	    atomPair->property = None;
	    set = TRUE;	    
	}
	multipleIndex = SEL_MULTIPLE;
    }


    /*
     * We have replaced in the MULTIPLE property any property atoms for
     * targets it failed to convert with None.
     */
    if ( set )
        XChangeProperty( seln->dpy, seln->req->requestor, seln->property, 
			target,	format, PropModeReplace, prop, (int)length);

    XFree( (char *)prop );
}


static int
DoConversion( selection, target, property, multipleIndex )
Sel_owner_info  *selection;
Atom          target;
Atom          property;
int           multipleIndex;
{
    Atom           replyType;
    char           *replyBuff;
    unsigned long  length;
    int            format=0;  
    unsigned long  svr_max_req_size;  


    selection->req->property = property;
    
    if ( target == selection->atomList->timestamp )  {
	ReplyTimestamp( selection, &replyType, &replyBuff, &length, &format );
	selection->req->type = replyType;
	selection->req->target = target;
	selection->req->property = property;
	goto Done;
    }

    replyType = target;

    /*
     * set the length to the max_server-buffer size; set the format to
     * indicate the beginning or end of a multiple conversion.
     * MAX_SEL_BUF_SIZE returns the size in long words. hence the
     * multiplication by 4 to convert the size to bytes.
     */

    svr_max_req_size = (MAX_SEL_BUFF_SIZE(selection->dpy) << 2) - 100;
    
    length = svr_max_req_size ; 
    format = multipleIndex;
    
    
    /*
     * Run the user defined convert proc.
     */
    if ( !(*selection->convert_proc)( selection->public_self, &replyType, 
			     &replyBuff, &length, &format ))   {

        return( FALSE );
    }

    /*
     * If the user defined convert proc has set the incr to TRUE,
     * send data in increments.
     */
    if ( replyType == selection->atomList->incr )
        selection->req->incr = TRUE;
    
    selection->req->target = target;
    selection->req->bytelength = BYTE_SIZE( length, format );
    selection->req->offset = 0;
    selection->req->format = format;
    selection->req->type = replyType;
    selection->req->data = (char *) replyBuff;
    /*
     * If the data size is bigger than the server's maximum_request_size
     * send the data in increments or if the selection owner has explicitly
     * asked for incremental transfer; do incr
     */


    if ((selection->req->bytelength  > svr_max_req_size) ||
	selection->req->incr ) {
	XWindowAttributes  winAttr;    
	int   status;
	

	status = xv_sel_add_prop_notify_mask( selection->dpy, selection->req->requestor, 
				   &winAttr );

	if ( status )
		selection->status |= SEL_ADD_PROP_NOTIFY;

	SendIncrMessage( selection );
        selection->req->incr = FALSE;

	return SEL_INCREMENT;
    }
    /*
     * Successfully converted none incremental data. 
     * Cleanup!!
     * NOTE: The application is responsible for deallocating replyBuff.
     */
    if ( target == selection->atomList->timestamp )
        XFree( (char * ) replyBuff );
    goto Done;

Done:
    
    /*
     * Place the data resulting from the selection conversion into the
     * specified property on the requestor window.
     */
    XChangeProperty( selection->dpy, selection->req->requestor, 
		    selection->req->property, selection->req->type,
		    format, PropModeReplace,  (unsigned char *) replyBuff, 
		    length  );

#ifdef SEL_DEBUG	
    printf("DoConversion: win=%d  prop=%d  type=%d  length=%d  format=%d\n", 
       selection->req->requestor, selection->req->property, 
       selection->req->type, length, format );
#endif
    
    XFlush( selection->dpy );

    if ( selection->done_proc )
        (*selection->done_proc)( selection->public_self, 
				selection->req->data, target );

    return TRUE;
}


/* return the timestamp the owner used to acquire ownership. */
static void
ReplyTimestamp( owner, replyType, replyBuff, length, format )
Sel_owner_info         *owner;
Atom                   *replyType;
char                   **replyBuff;
unsigned long          *length;
int                    *format;
{
    *replyBuff = ( char *) xv_malloc( sizeof( Time ) );
    *replyBuff = (char *) &owner->time;
    *replyType = owner->atomList->integer;
    *length = 1;
    *format = 32;
}



/*
 * Handle incremental data transfer.
 */
Xv_private int
xv_sel_handle_incr( selection )
Sel_owner_info     *selection;
{
    XEvent  event;
    int    endTransfer = FALSE;
    unsigned long   length;
    unsigned long   svr_max_req_size;
    Requestor  *req;
    
    req = selection->req;
    req->type = req->target;
    
    /*
     * If the selection owner has choosen to send the data in increments,
     * call it's convert proc to get the data.
     */

    if ( req->incr )  {
        svr_max_req_size = (MAX_SEL_BUFF_SIZE(selection->dpy) << 2) - 100;
	length = svr_max_req_size;
	if ( !(*selection->convert_proc)( selection->public_self, &req->type, 
					 &req->data, &length, &req->format )) 
	    return FALSE;    

	req->bytelength = BYTE_SIZE( length, req->format );
	req->offset = 0;
    }
    
    /*
     * The requestor should start the transfer process by deleting the
     * (type==INCR) property forming the reply to the selection.
     * Here, we block for PropertyNotify event and process the data transfer.
     */
    while ( 1 )  {
	if (xv_sel_block_for_event( selection->dpy, &event, selection->req->timeout, 
			  ValidatePropertyEvent, (char *) req )) {
	    if ( endTransfer ) 
	        break;
	    endTransfer = SendIncr( selection );
	}
	else   {
	    /*
	     * If timedout call the owner done_proc.
	     */
	    
	    if ( selection->done_proc )
	        (*selection->done_proc)( selection->public_self, 
					selection->req->data, req->target );

	    return FALSE;
	}
    }
    
    /*
     * We are now done sending all the data.
     * Finish the transaction by sending zero-length data to the requestor.
     */
    if ( selection->status & SEL_INTERNAL_ERROR )
        return FALSE;
    
    XChangeProperty( selection->dpy, selection->req->requestor, 
		    selection->req->property, selection->req->type,
		    selection->req->format, PropModeReplace,
		    (unsigned char *) NULL, 0);

#ifdef SEL_DEBUG	
    printf("xv_sel_handle_incr: win=%d  prop=%d  type=%d  length=%d  format=%d\n", 
       selection->req->requestor, selection->req->property, 
       selection->req->type, 0, selection->req->format );
#endif

    if ( selection->done_proc )
        (*selection->done_proc)( selection->public_self, selection->req->data,
				req->target );

    return TRUE;    
}

static int
  SendIncr( Sel_owner_info  *seln )
{
    Requestor  *req;
    int        size;
    unsigned long svr_max_req_size;
    

    req = seln->req;
    svr_max_req_size = (MAX_SEL_BUFF_SIZE(seln->dpy) << 2)  - 100;



    size = ((req->bytelength - req->offset) > svr_max_req_size) ?
              svr_max_req_size : (req->bytelength - req->offset);


    XChangeProperty( seln->dpy, req->requestor, req->property, req->type,
		    req->format, PropModeReplace, 
		    (unsigned char *) &req->data[req->offset],
		    ( size / (req->format>>3) ));

#ifdef SEL_DEBUG1
    printf("SendIncr : win=%d  prop=%s  type=%d  length=%d  format=%d\n", 
       req->requestor, XGetAtomName(seln->dpy,req->property), req->type,( size / (req->format>>3) ) , req->format );
#endif

    req->offset += size;

    /*
     * If the selection owner has explicitly asked for incremental data
     * transfer; execute the passed-in convert routine again.
     */
    if ( req->incr )  {
	unsigned long length;

	/*
	 * Set the length to max buffer size and Run the convert proc.
	 */
	length = svr_max_req_size;
	
	req->type = req->target;

	/*
	 * If this is INCR from a multiple request; we need to call the
	 * user defined convert_proc with format set to SEL_MULTIPLE.
	 */
	if ( req->multiple ) 
	    req->format = SEL_MULTIPLE;

	if ( !(*seln->convert_proc)( seln->public_self, &req->type, 
				    &req->data, &length, &req->format ))  {

	    /*
	     * REMINDER: The return value needs to be changed!
	     */
	    return TRUE;    
	}

	/* The client selection owners are required to set the length to 
	 * zero, after the completion of data conversion.
	 */
	req->bytelength = BYTE_SIZE( length, req->format );
	req->offset = 0;
    }
    
    if ( !(req->bytelength - req->offset) ) 
        return( TRUE );

    return( FALSE );
}


    
Xv_private int
xv_sel_handle_selection_clear( clrEv )
XSelectionClearEvent  *clrEv;
{
    Sel_owner_info   *owner;

    owner = xv_sel_find_selection_data( clrEv->display, clrEv->selection, 
				clrEv->window );

    /* Is this event for this window? */
    if ( owner->xid != clrEv->window ) 
        return( FALSE );
    /* make sure that the selection was owned before the clear event
       was sent.  If it was owned after the clear event was sent, then
       don't lose the selection. */
    if (owner->own && (owner->time <= clrEv->time)) {
	SelLoseOwnership( owner );
    }
    return TRUE;
}

    
static void
SetupPropInfo( sel )
Sel_owner_info  *sel;
{
    unsigned long  bytesafter;
    unsigned char  *prop;
    unsigned long  length;
    int            format;
    Atom           target;
    Selection      owner_public = SEL_OWNER_PUBLIC( sel);
    
    sel->propInfo = xv_alloc( Sel_prop_info );

    if ( sel->status & SEL_LOCAL_PROCESS )    {
	int  index =  sel->req_info->typeIndex;
	
	*sel->propInfo = *sel->req_info->typeTbl[index].propInfo;
	return;
    }
    

    if ( XGetWindowProperty( sel->dpy, sel->req->requestor,
			    sel->req->property, 0L, 1000000,
			    False,(Atom)AnyPropertyType, &target, &format, 
			    &length, &bytesafter, &prop) != Success )
        xv_error( sel->public_self, 
                  ERROR_STRING, XV_MSG("XGetWindowProperty Failed"),
                  ERROR_PKG,SELECTION, 
                  0 );

    sel->propInfo->data = (Xv_opaque) prop;
    sel->propInfo->format = format;
    sel->propInfo->length = length;
    sel->propInfo->type = target;

    if ( prop != NULL && length != 0 )
        sel->propInfo->typeName = xv_sel_atom_to_str( sel->dpy, target, sel->xid );
}


/*   
 * the owner sends a property of type INCR in response to any target that
 * results in selection data.
 */
static void
SendIncrMessage( sel )
Sel_owner_info *sel;
{
    long    size;
    char    *propData;
    Requestor *incrReq;
    
    /* We are setting the size to a lower bound on the number of bytes of
     * data in the selection.
     */
    if ( sel->req->incr ) 
        /* 
	 * If the user has decided to send the data in increments, the 
	 * selection->req->data should be the size.
	 */
	propData = (char *) sel->req->data;
    else  {
	size = sel->req->bytelength;
	propData =  (char *) &size;
    }


    /*
     * If this is INCR from a multiple request; we need to call the
     * user defined convert_proc with format set to SEL_MULTIPLE.
     */
    if ( sel->req->multiple ) 
	sel->req->format = SEL_MULTIPLE;

    XChangeProperty( sel->dpy, sel->req->requestor, 
		    sel->req->property, sel->atomList->incr,
		    32, PropModeReplace,  (unsigned char *) propData, 1  );

#ifdef SEL_DEBUG1
    printf("SendIncrMessage: win=%d  prop=%s  type=%s  length=%d  format=%d\n", sel->req->requestor, XGetAtomName(sel->dpy,sel->req->property),XGetAtomName(sel->dpy,sel->atomList->incr) ,1 ,32 );
#endif

    sel->req->numIncr++;


    if ( sel->req->numIncr == 1 )
        sel->req->incrPropList = xv_alloc( Atom ); 
    else						  
        sel->req->incrPropList=(Atom *)xv_realloc((char *)sel->req->incrPropList, 
		       (unsigned) ( sel->req->numIncr * sizeof( Atom ) ) );

    sel->req->incrPropList[sel->req->numIncr-1] = sel->req->property;
	
    if ( reqCtx == 0 )
        reqCtx = XUniqueContext();

    incrReq = xv_alloc( Requestor );
    
    XV_BCOPY( (char *) sel->req, (char *) incrReq, sizeof(Requestor) ); 
    
    (void)XSaveContext( sel->dpy,(Window)incrReq->property, reqCtx,(caddr_t)incrReq);
}






/*ARGSUSED*/
static int
ValidatePropertyEvent( display, xevent, args )
Display  *display;
XEvent   *xevent;
char	 *args;
{
    Requestor  req;
    

    XV_BCOPY( (char *) args, (char *) &req, sizeof(Requestor) ); 
    /*
     * If some other process wants to become the selection owner, let
     * it do so but continue handling the current transaction.
     */
    if ( ( xevent->type & 0177) == SelectionClear )  {
	xv_sel_handle_selection_clear( (XSelectionClearEvent * ) xevent );
	return FALSE;
    }
    
    if ( ( xevent->type & 0177) == PropertyNotify )  {
	XPropertyEvent *ev = (XPropertyEvent *) xevent;


	if ( ev->state == PropertyDelete && ev->atom == req.property &&
		ev->time > req.time ) {
#ifdef SEL_DEBUG1
  printf("ValProp-Okay Recieved PropertyNotify win =%d atom = %s state = %s\n", ev->window,XGetAtomName(ev->display,ev->atom),(ev->state == PropertyNewValue)? "NewValue":"Deleted");
#endif
	    return ( TRUE );
        }
    }
    return ( FALSE );
}




static void
RegisterSelClient( owner, flag  )
Sel_owner_info   *owner;
int              flag;
{
    Sel_client_info  *clientInfo, *infoPtr;
    Display          *dpy = owner->dpy;
    static XContext  clientCtx = 0;

    if ( clientCtx == 0 )
        clientCtx = XUniqueContext();

#ifdef SEL_DEBUG2
    printf("RegisterSelClient: flag=%s selection=%d  xid=%d own=%d time=%ul dpy=%x\n",
	   (flag)?"SEL_DELETE_CLIENT":"SEL_ADD_CLIENT", owner->selection, owner->xid, owner->own, owner->time, owner->dpy );
#endif

    if (XFindContext(dpy, DefaultRootWindow(dpy), clientCtx, (caddr_t *)&clientInfo)) {

        if (flag == SEL_DELETE_CLIENT)
            return;
	clientInfo = xv_alloc( Sel_client_info );

	if ( clientInfo == NULL ) 
	    return;

	clientInfo->client = owner;
	clientInfo->next = NULL;	

	(void)XSaveContext(dpy, DefaultRootWindow(dpy), clientCtx, (caddr_t)clientInfo);
	return;
    }

    infoPtr = clientInfo;
    
    do {
	if ( infoPtr->client == NULL ) {
	    if ( infoPtr->next == NULL )
	        break;
	    infoPtr = infoPtr->next;	    
	    continue;
	}
	
	/*
	 * If we are deleting a client from the list, set client to NULL.
	 */
	if (( infoPtr->client->xid == owner->xid ) 
              && (infoPtr->client->selection == owner->selection)
              && ( flag == SEL_DELETE_CLIENT )) {
	    infoPtr->client = NULL;
	    return;
	}
#ifdef SEL_DEBUG 
	printf("RegisterSelClient: clntSel=%d  clntXid=%d clntOwn=%d dpy=%x\n",
	       infoPtr->client->selection, infoPtr->client->xid, 
	       infoPtr->client->own, infoPtr->client->dpy );
#endif

	/*
	 * If there is another client in this process that is the selection owner;
	 * make it lose the ownership and occupy the list space. XSelectionClear 
	 * event is distributed per process not per window.
	 */
	if ((infoPtr->client->selection == owner->selection) && 
            (flag == SEL_ADD_CLIENT) ) {
	    if (infoPtr->client->xid != owner->xid ) {
		/* only lose ownership if you actually own the selection */
		if (infoPtr->client->own) {
		    SelLoseOwnership( infoPtr->client );
		}
	    }
	    infoPtr->client = owner;
	    return;
	}
	if ( infoPtr->next == NULL )
	    break;
	infoPtr = infoPtr->next;
    } while ( 1 );

    /*
     * Go over the list one more time to fill-in the empty spaces. This list
     * should stay pretty small, so looping over it should not effect the
     * performance (not too much).
     */
    infoPtr = clientInfo;
    do {
	if ( infoPtr->client == NULL ) {
           if (flag != SEL_DELETE_CLIENT)
	    infoPtr->client = owner;
	    return;
	}
	if ( infoPtr->next == NULL )
	    break;
	infoPtr = infoPtr->next;
    } while ( 1 );

   if (flag == SEL_ADD_CLIENT) { 
    infoPtr->next = xv_alloc( Sel_client_info );

    if ( infoPtr->next == NULL ) 
	return;
    infoPtr = infoPtr->next;
    infoPtr->client = owner;
    infoPtr->next = NULL;
   };
    return;
}

	
