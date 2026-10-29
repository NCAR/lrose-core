#ifndef lint
#ifdef SCCS
static char     sccsid[] = "@(#)sel_req.c 1.43 93/06/29";
#endif
#endif

/*
 *	(c) Copyright 1990 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE 
 *	file for terms of the license.
 */

#include <xview_private/sel_impl.h>
#include <xview_private/svr_impl.h>
#include <xview/server.h>
#include <xview/window.h>
#include <xview/notify.h>
#include <sys/time.h>

#if defined(SVR4) || defined(__linux)
#include <stdlib.h> 
#endif /* SVR4 */


#define ITIMER_NULL  ((struct itimercal *)0)


Pkg_private char *xv_sel_atom_to_str(/* display, atom */);
/* Pkg_private XID SelGetOwnerXID(); */
Pkg_private Atom xv_sel_str_to_atom(/* display, string */);
Pkg_private int xv_sel_add_prop_notify_mask();
Pkg_private Atom xv_sel_get_property();
Pkg_private void xv_sel_free_property();
Pkg_private int WaitForReadableTimeout();
Pkg_private int xv_sel_predicate();
Pkg_private int xv_sel_check_selnotify();
Pkg_private int xv_sel_block_for_event();
Pkg_private int xv_sel_check_property_event();
Xv_private Time xv_sel_get_last_event_time();
Pkg_private Sel_owner_info  *xv_sel_find_selection_data();
Pkg_private Notify_value xv_sel_handle_sel_timeout();

static void XvGetSeln();
static void SetExtendedData();
static void SetupMultipleRequest();
static void SelSaveData();
static Xv_opaque SelBlockReq();
static int CheckSelectionNotify();
static int HandleLocalProcess();
static int LocalMultipleTrans();
static int ProcessIncr();
static int TransferData();
static int HandleLocalIncr();
static int ProcessMultiple();
static int GetSelection();
static int ProcessReply();
static Requestor *SelGetReq();

static int
  XvGetRequestedValue( Sel_req_info *selReq, XSelectionEvent *ev,
                       Sel_reply_info *replyInfo,
                       Atom property, Atom target, int blocking );

static XID 
  SelGetOwnerXID( Sel_req_info  *selReq );

static int
  ProcessNonBlkIncr(Sel_req_info *selReq, Sel_reply_info *reply,
                    XSelectionEvent *ev, Atom target);

static int
  ProcessReq( Requestor  *req, XPropertyEvent  *ev );

static int
  OldPkgIsOwner( Display *dpy, Atom selection, Window xid,
                 Sel_reply_info *reply, Sel_req_info *selReq );

/*ARGSUSED*/
Pkg_private int
sel_req_init(parent, sel_req_public, avlist)
Xv_Window	    parent;
Selection_requestor sel_req_public;
Attr_avlist	    avlist;
{
    Sel_req_info     *sel_req;
    Xv_sel_requestor *sel_req_object = (Xv_sel_requestor *) sel_req_public;
    Display          *display;
    Xv_window        win;
    XID              xid=0;

    win = (Xv_window) xv_get( sel_req_public, XV_OWNER );
    xid = (XID) xv_get( win, XV_XID );

    /* Allocate and clear private data */
    sel_req = xv_alloc(Sel_req_info);

    /* Link private and public data */
    sel_req_object->private_data = (Xv_opaque) sel_req;
    sel_req->public_self = sel_req_public;

    /* get the display */
    display = XV_DISPLAY_FROM_WINDOW( parent );

    /* Initialize private data */
    sel_req->nbr_types = 1;
    sel_req->typeIndex = 0;
    sel_req->typeTbl = xv_alloc( Sel_type_tbl );
    sel_req->typeTbl->type = XA_STRING;
    sel_req->typeTbl->status = 0;
    sel_req->typeTbl->type_name = xv_sel_atom_to_str( display, sel_req->typeTbl->type, xid );

    return XV_OK;
}



/*argsused*/
Pkg_private Xv_opaque
sel_req_set_avlist(sel_req_public, avlist)
Selection_requestor sel_req_public;
Attr_avlist	    avlist;
{
    Sel_req_info   *sel_req = SEL_REQUESTOR_PRIVATE(sel_req_public);
    int		    type_set = FALSE;
    int		    types_set = FALSE;
    int		    type_name_set = FALSE;
    int		    apndTypeSet = FALSE;
    int		    apndTypeNameSet = FALSE;
    int		    type_names_set = FALSE;
    Sel_info        *sel_info;
    Sel_prop_info   *propInfo;
    Attr_avlist	    attrs;
    int		    i;
    int             numApndTypes, numTypes;
    Xv_window        win;
    XID              xid=0;

    win = (Xv_window) xv_get( sel_req_public, XV_OWNER );
    xid = (XID) xv_get( win, XV_XID );

    sel_info = SEL_PRIVATE( sel_req->public_self );

    for (attrs = avlist; *attrs; attrs = attr_next(attrs)) {
	switch (attrs[0]) {
	  case SEL_REPLY_PROC:
	    sel_req->reply_proc = (void (*) ()) attrs[1];
	    break;
	  case SEL_TYPE:
	    sel_req->nbr_types = 1;
	    sel_req->typeTbl[0].type = (Atom) attrs[1];
	    sel_req->typeTbl[0].status = 0;
	    type_set = TRUE;
	    break;
	  case SEL_TYPES:
	    free(sel_req->typeTbl);
	    for (i=1; attrs[i]; i++);
	    sel_req->nbr_types = i - 1;  /* don't count NULL terminator */

	    sel_req->typeTbl = (Sel_type_tbl *) xv_calloc(sel_req->nbr_types , 
						       sizeof(Sel_type_tbl));

	    for (i=1; i <= sel_req->nbr_types; i++ )   {
		sel_req->typeTbl[i-1].type = (Atom) attrs[i];
		sel_req->typeTbl[i-1].status = 0;
	    }
	    types_set = TRUE;
	    break;
	  case SEL_APPEND_TYPES:
	    for (i=1; attrs[i]; i++);
	    numApndTypes = i - 1;  /* don't count NULL terminator */
	    numTypes = sel_req->nbr_types;
	    
	    sel_req->typeTbl = (Sel_type_tbl *) xv_realloc( (char *) sel_req->typeTbl, 
				(unsigned) ((numTypes+numApndTypes) * 
				sizeof(Sel_type_tbl)) );

	    for (i=1; i <= numApndTypes; i++ )   {
		sel_req->typeTbl[numTypes-1+i].type = (Atom) attrs[i];
		sel_req->typeTbl[numTypes-1+i].status = 0;
	    }
	    sel_req->nbr_types += numApndTypes;
	    apndTypeSet = TRUE;
	    break;
	  case SEL_APPEND_TYPE_NAMES:
	    for (i=1; attrs[i]; i++);
	    numApndTypes = i - 1;  /* don't count NULL terminator */
	    numTypes = sel_req->nbr_types;

	    sel_req->typeTbl = (Sel_type_tbl *) xv_realloc( (char *) sel_req->typeTbl, 
				(unsigned) ((numTypes+numApndTypes) * 
				sizeof(Sel_type_tbl)) );

	    for (i=1; i <= numApndTypes; i++ )   {
		sel_req->typeTbl[numTypes-1+i].type_name=(char *) attrs[i];
		sel_req->typeTbl[numTypes-1+i].status = 0;
	    }
	    sel_req->nbr_types += numApndTypes;
	    apndTypeNameSet = TRUE;
	    break;
	  case SEL_TYPE_NAME:
	    sel_req->nbr_types = 1;
	    sel_req->typeTbl->type_name = (char *) attrs[1];
	    sel_req->typeTbl->status = 0;
	    type_name_set = TRUE;
	    break;
	  case SEL_TYPE_NAMES:
	    free(sel_req->typeTbl);
	    for (i=1; attrs[i]; i++);
	    sel_req->nbr_types = i - 1;  /* don't count NULL terminator */

	    sel_req->typeTbl = (Sel_type_tbl *) xv_calloc(sel_req->nbr_types, 
						       sizeof(Sel_type_tbl));

	    for (i=1; i <= sel_req->nbr_types; i++ )   {
		sel_req->typeTbl[i-1].type_name = (char *) attrs[i];
		sel_req->typeTbl[i-1].status = 0;
	    }
	    type_names_set = TRUE;
	    break;
	  case SEL_TYPE_INDEX:
	    sel_req->typeIndex = (int) attrs[1];
	    sel_req->typeTbl[sel_req->typeIndex].propInfo = xv_alloc( Sel_prop_info);
	    propInfo = sel_req->typeTbl[sel_req->typeIndex].propInfo;
	    propInfo->data = XV_ZERO;
	    propInfo->format = 8;
	    propInfo->length = 0;
	    propInfo->type = XA_STRING;
	    propInfo->typeName = (char *) NULL;
	    break;
	  case SEL_PROP_DATA:
	    i = sel_req->typeIndex;
	    sel_req->typeTbl[i].propInfo->data = (Xv_opaque) attrs[1];
	    sel_req->typeTbl[i].status |= SEL_PROPERTY_DATA;
	    break;
	  case SEL_PROP_LENGTH:
	    i = sel_req->typeIndex;
	    sel_req->typeTbl[i].propInfo->length = (long) attrs[1];
	    break;
	  case SEL_PROP_FORMAT:
	    i = sel_req->typeIndex;
	    sel_req->typeTbl[i].propInfo->format = (int) attrs[1];
	    break;
	  case SEL_PROP_TYPE:
	    i = sel_req->typeIndex;
	    sel_req->typeTbl[i].propInfo->type = (Atom) attrs[1];
	    break;
	  case SEL_PROP_TYPE_NAME:
	    i = sel_req->typeIndex;
	    sel_req->typeTbl[i].propInfo->typeName = (char *) attrs[1];
	    break;
        }
    }

    if (type_set && !type_name_set)
	sel_req->typeTbl[0].type_name = xv_sel_atom_to_str(sel_info->dpy, 
						     sel_req->typeTbl[0].type, xid );
    else if (type_name_set && !type_set)
	sel_req->typeTbl[0].type = xv_sel_str_to_atom( sel_info->dpy,
						sel_req->typeTbl[0].type_name, xid );
    else if (types_set && !type_names_set)   {
	for (i = 1; i <= sel_req->nbr_types; i++) {
	    sel_req->typeTbl[i-1].type_name = xv_sel_atom_to_str( sel_info->dpy, 
						   sel_req->typeTbl[i-1].type, xid );
	}
    }
    else if (type_names_set && !types_set)   {
	for (i = 1; i <= sel_req->nbr_types; i++)
	    sel_req->typeTbl[i-1].type = xv_sel_str_to_atom(sel_info->dpy, 
					      sel_req->typeTbl[i-1].type_name, xid );
    }

    if (apndTypeSet && !apndTypeNameSet)   {
	for (i = numTypes; i < sel_req->nbr_types; i++) {
	    sel_req->typeTbl[i].type_name = xv_sel_atom_to_str( sel_info->dpy, 
						 sel_req->typeTbl[i].type, xid );
	}
    }
    else if (apndTypeNameSet && !apndTypeSet)   {
	for (i = numTypes; i < sel_req->nbr_types; i++)
	    sel_req->typeTbl[i].type = xv_sel_str_to_atom(sel_info->dpy, 
					       sel_req->typeTbl[i].type_name, xid );
    }

    for ( i=0; i< sel_req->nbr_types; i++ )  {
	if ( sel_req->typeTbl[i].status & SEL_PROPERTY_DATA ) {
	    propInfo = sel_req->typeTbl[i].propInfo;
	    if ( propInfo->typeName == NULL )
	        propInfo->typeName = xv_sel_atom_to_str(sel_info->dpy,propInfo->type, xid );
	    else
	        propInfo->type = xv_sel_str_to_atom(sel_info->dpy, propInfo->typeName, xid );
	}
    }
    return XV_OK;
}


/*ARGSUSED*/
Pkg_private Xv_opaque
sel_req_get_attr(sel_req_public, status, attr, valist)
Selection_requestor sel_req_public;
int		   *status;
Attr_attribute  attr;
va_list	    valist;
{
    Sel_req_info *sel_req = SEL_REQUESTOR_PRIVATE(sel_req_public);
    int		 arg;
    int          i;
    static Atom         *types=NULL;
    static char         **typeNames=NULL;

    switch (attr) {
      case SEL_DATA:
	/* Initiate a blocking request */
        arg = va_arg(valist, int);
	return (Xv_opaque) SelBlockReq( sel_req, arg, va_arg(valist, int));
      case SEL_REPLY_PROC:
	return (Xv_opaque) sel_req->reply_proc;
      case XV_XID:
	return (Xv_opaque) SelGetOwnerXID(sel_req);
      case SEL_TYPE:
	return (Xv_opaque)  sel_req->typeTbl[0].type;
      case SEL_TYPES:
	if ( types != NULL )
	    XFree( (char *)types );
	types = (Atom *) xv_calloc( sel_req->nbr_types + 1, sizeof( Atom  ) );
	for (i = 0; i < sel_req->nbr_types; i++) 
	    types[i] = sel_req->typeTbl[i].type;
	types[i] = 0 ; /* NULL terminate the list */
	return (Xv_opaque) types;
      case SEL_TYPE_NAME:
	return (Xv_opaque) sel_req->typeTbl[0].type_name;
      case SEL_TYPE_NAMES:
	if ( typeNames != NULL )
	    XFree( (char *)typeNames );
	typeNames = (char **) xv_malloc( (sel_req->nbr_types + 1) * sizeof (char *) );
	for (i = 0; i < sel_req->nbr_types; i++)
	    typeNames[i] = (char *) sel_req->typeTbl[i].type_name;
        typeNames[i] = 0; /* Null terminate the list */
	return (Xv_opaque) typeNames;
      default:
	*status = XV_ERROR;
	return (Xv_opaque) 0;
    }
}


Pkg_private int
sel_req_destroy(sel_req_public, status)
Selection_requestor sel_req_public;
Destroy_status      status;
{
    Sel_req_info   *selReq = SEL_REQUESTOR_PRIVATE(sel_req_public);
    Sel_info       *sel_info;


    if (status == DESTROY_CHECKING || status == DESTROY_SAVE_YOURSELF
        || status == DESTROY_PROCESS_DEATH)
	return XV_OK;

    sel_info = SEL_PRIVATE( selReq->public_self );

    /* Free up malloc'ed storage */
    XFree( (char *) selReq->typeTbl );
    XFree( (char *) selReq );

    return XV_OK;
}



/*
* Create a reply struct.
*/
static Sel_reply_info *
NewReplyInfo( req, win, selection, time, selReq )
Selection_requestor  req;
Window               win;
Sel_owner_info       *selection;
Time                 time;
Sel_req_info         *selReq;
{
    int   numTarget;
    Sel_reply_info  *replyInfo;

    replyInfo = xv_alloc( Sel_reply_info );

    selection->req_info = SEL_REQUESTOR_PRIVATE(req);
    numTarget = selReq->nbr_types;
    replyInfo->seln = selection;

    replyInfo->target = ( Atom *) xv_calloc( numTarget+1, sizeof( Atom  ));

    if ( numTarget > 1 )  {
	Atom *target;
    
        target = (Atom *) xv_get( req, SEL_TYPES );
        *replyInfo->target = selection->atomList->multiple;
	XV_BCOPY( (char *) target, (char *) replyInfo->target+sizeof(Atom), 
	      numTarget*sizeof(Atom));
    }
    else {
	Atom  target;
	
	target = (Atom ) xv_get( req, SEL_TYPE );
	XV_BCOPY( (char *) &target, replyInfo->target, numTarget * sizeof(Atom));
    }

    replyInfo->property = xv_sel_get_property( selection->dpy );
    replyInfo->requestor = win;
    if ( time == XV_ZERO )
	replyInfo->time = xv_sel_get_last_event_time( selection->dpy, win );
    else
    	replyInfo->time = time;

    replyInfo->timeout = xv_get( req, SEL_TIMEOUT_VALUE );
    replyInfo->multiple = 0;
    replyInfo->data = (Xv_opaque) NULL;
    replyInfo->format = XV_ZERO;
    replyInfo->incr = 0;
    replyInfo->status = 0;
    replyInfo->length = (long) NULL;
    replyInfo->req_info = selReq;

	if ( numTarget == 1 )
		SetExtendedData( replyInfo, replyInfo->property, 0 );
		 
    return( replyInfo );
}


static int
CheckSelectionNotify( selReq, replyInfo, ev, blocking ) 
Sel_req_info    *selReq;
Sel_reply_info  *replyInfo;
XSelectionEvent *ev;
{
    if (ev->time != replyInfo->time) {
        xv_sel_handle_error( SEL_BAD_TIME, selReq, replyInfo, *replyInfo->target );
	return( FALSE );
    }
    if (ev->requestor != replyInfo->requestor) {
        xv_sel_handle_error( SEL_BAD_WIN_ID, selReq, replyInfo, *replyInfo->target );
	return( FALSE );
    }
    /*
     * If the "property" field is None, the conversion has been refused. 
     * This can mean that there is no owner for the selection, that the owner 
     * does not support the conversion impiled by "target", or that the 
     * server did not have sufficient space.
     */
    if (ev->property == None)  {
        xv_sel_handle_error( SEL_BAD_CONVERSION, selReq, replyInfo, 
		    *replyInfo->target );
	if ( !blocking && !replyInfo->multiple ) 
	    xv_sel_end_request( replyInfo );
	return( FALSE );
    }	

    return( TRUE );
}


/*
 * If this process is both the selection owner and the selection requestor, 
 * don't transfer the data through X.
 */
static int
HandleLocalProcess( selReq, reply, blocking )
Sel_req_info   *selReq;
Sel_reply_info *reply;
int            blocking;
{	
    if ( selReq->nbr_types > 1 )  {
    	if ( blocking && (selReq->reply_proc == NULL ) )  
	    return FALSE;
	return LocalMultipleTrans( reply, selReq, blocking );
    }

    return TransferData(  selReq, reply, *reply->target, blocking, 0 );
}


static int
LocalMultipleTrans( reply, selReq, blocking )
Sel_reply_info *reply;
Sel_req_info   *selReq;
int            blocking;
{
    unsigned long  length;
    unsigned long  bytesafter;
    atom_pair      *atomPair;
    unsigned char  *prop;
    int            format;
    Atom           target;
    int            byteLen, i;
    int            multipleIndex=SEL_MULTIPLE, firstTarget=1;

    /*
     * The contents of the property named in the request is a list
     * of atom pairs, the first atom naming a target, and the second
     * naming a property. Do not delete the data. The requestor needs to
     * access this data.
     */
     if ( XGetWindowProperty( reply->seln->dpy, reply->requestor,
			     reply->property, 0L, 1000000,
			     False,(Atom)AnyPropertyType, &target, &format, 
			     &length, &bytesafter, &prop) != Success )  {
         xv_error( selReq->public_self,
                   ERROR_STRING, XV_MSG("XGetWindowProperty Failed"), 
                   ERROR_PKG,SELECTION,
                   0 );
	 xv_sel_handle_error( SEL_BAD_CONVERSION, selReq, reply, 
		     reply->seln->atomList->multiple );
     }
    
    byteLen = BYTE_SIZE( length, format ) / sizeof( atom_pair );

    for ( i=0, atomPair = ( atom_pair *)prop; byteLen; i++,atomPair++,byteLen--) {

         reply->req_info->typeIndex = i;


	if ( firstTarget ) {
	    multipleIndex = SEL_BEGIN_MULTIPLE;	    
	    firstTarget = 0;
	}
	
	if ( !(byteLen - 1 ) )
	    multipleIndex = SEL_END_MULTIPLE;

	TransferData(  selReq, reply, atomPair->target, blocking, multipleIndex );
	xv_sel_free_property( reply->seln->dpy, atomPair->property ); 
	multipleIndex = SEL_MULTIPLE;
    }

    xv_sel_free_property( reply->seln->dpy, reply->property );
    XFree( (char *)prop );
    return TRUE;
}


static int
TransferData(  selReq, reply, target, blocking, multipleIndex )
Sel_req_info   *selReq;
Sel_reply_info *reply;
Atom           target;
int            blocking;
int            multipleIndex;
{
    Atom   replyType;
    char           *replyBuff;
    unsigned long  svr_max_req_size;
    
    if ( target == reply->seln->atomList->timestamp )  {
	reply->data = (Xv_opaque) &reply->seln->time;
	reply->length = 1;
	reply->format = 32;
	if ( selReq->reply_proc ) 
	    (*selReq->reply_proc)( SEL_REQUESTOR_PUBLIC(selReq), target, 
			     reply->seln->atomList->integer,
			     reply->data, reply->length, reply->format );
	xv_sel_free_property( reply->seln->dpy, reply->property );
	return TRUE;
    }

    replyType = target;
    /*
     * set the length to the max_server-buffer size; set the format to
     * indicate the beginning or end of a multiple conversion.
     */    

    svr_max_req_size = (MAX_SEL_BUFF_SIZE(reply->seln->dpy) << 2) - 100;
    reply->length = svr_max_req_size;
    reply->format = multipleIndex;

    if ( !(*reply->seln->convert_proc)( reply->seln->public_self, &replyType, 
			     &replyBuff, &reply->length, &reply->format ))   {

	if ( selReq->reply_proc != NULL )
	    xv_sel_handle_error( SEL_BAD_CONVERSION, selReq, reply, target );
        goto Error;    
    }

    /*
     * Note: The user done_proc should free "replyBuff".
     *       The user reply_proc should free "reply->data".
     */
    SelSaveData( replyBuff, reply, BYTE_SIZE(reply->length, reply->format) );
    
    if ( ( (Atom) replyType == reply->seln->atomList->incr )
	|| (BYTE_SIZE(reply->length,reply->format) >= svr_max_req_size) )   {

	reply->incr = TRUE;
	
	/*
	 * If we are processing a blocking request and the selection owner is 
	 * transferring the data in increments or the size of selection data 
	 * is larger than the max selection size and the requestor hasn't 
	 * registered a reply_proc, reject the data transfer.
	 */
	if ( blocking && (selReq->reply_proc == NULL) )
            goto Error;    
	return HandleLocalIncr( selReq, replyBuff, reply, target, replyType );
    }
    

    if ( selReq->reply_proc ) 
	(*selReq->reply_proc)( SEL_REQUESTOR_PUBLIC(selReq), target, replyType,
			     reply->data,  reply->length, reply->format );

    if ( reply->seln->done_proc )
        (*reply->seln->done_proc)( reply->seln->public_self, replyBuff, target );

    xv_sel_free_property( reply->seln->dpy, reply->property );
    return TRUE;

Error:
    xv_sel_free_property( reply->seln->dpy, reply->property );
    return FALSE;
}


/*
 * This routine transfers the local process data in increments.
*/
static int
HandleLocalIncr( selReq, replyBuf, reply, target, retType )
Sel_req_info   *selReq;
char           *replyBuf;
Sel_reply_info *reply;
Atom           target;
Atom           retType;
{
    Atom  replyType;
    int   byteLength;
    int   offset=0;
    int   size;
    unsigned long svr_max_req_size;

    
    
    replyType = target;
    if ( selReq->reply_proc == NULL ) 
	return FALSE;

    /*
     * We are setting the size to a lower bound on the number of bytes of
     * data in the selection.
     * Clients who wish to send the selection data incrementally by setting
     * the type to INCR, should set the reply buffer to a lower bound on the 
     * number of bytes of
     */
    if ( retType != reply->seln->atomList->incr )   {	
	size = BYTE_SIZE( reply->length, reply->format );
	SelSaveData( &size, reply, sizeof( int ) );
    }

    /*
     * Call the user defined reply_proc with replyType set to INCR and
     * reply->data set to the a lower bound of the number of bytes of data
     * in the selection.
     */
    (*selReq->reply_proc)( SEL_REQUESTOR_PUBLIC(selReq), target,
			 reply->seln->atomList->incr,  reply->data, 
			 1, 32 );

    /*
     * If this is INCR from a multiple request; we need to call the
     * user defined convert_proc with format set to SEL_MULTIPLE.
     */
    if ( reply->req_info->nbr_types > 1 ) 
	reply->format = SEL_MULTIPLE;


    svr_max_req_size = (MAX_SEL_BUFF_SIZE(reply->seln->dpy) << 2) - 100;

    /*
     * If the user has decided to send the data in increments,  call it's
     * convert_porc the second time to get the actual data (the first time
     * it told us that the data is being transferred in increments.
     */
    if ( retType == reply->seln->atomList->incr )  {
	reply->length = svr_max_req_size;
        if ( !(*reply->seln->convert_proc)( reply->seln->public_self, 
					   &replyType, &replyBuf, 
					   &reply->length, &reply->format ))
	    goto CvtFailed;
    }

    byteLength = BYTE_SIZE( reply->length, reply->format );
    offset = 0;
    
    do 
    {
	size = ( (byteLength - offset) > svr_max_req_size) ?
	svr_max_req_size : (byteLength - offset);

	SelSaveData( replyBuf+offset, reply, size );

	/* We have the first batch of data */
	(*selReq->reply_proc)( SEL_REQUESTOR_PUBLIC(selReq), target, 
			       replyType, reply->data, size, 
			       reply->format );

	offset += size;

	/*
	 * If the selection owner has explicitly asked for incremental data
	 * transfer; execute the passed-in convert routine again.
	 */
	if ( retType == reply->seln->atomList->incr )  {
	    /*
	     * Set the length to max buffer size and Run the convert proc.
	     */
	    reply->length = svr_max_req_size;

	    /*
 	     * If this is INCR from a multiple request; we need to call the
     	     * user defined convert_proc with format set to SEL_MULTIPLE.
     	     */
    	    if ( reply->req_info->nbr_types > 1 ) 
		    reply->format = SEL_MULTIPLE;

	    /* Get the next batch */
	    replyType = target;
	    if ( !(*reply->seln->convert_proc)( reply->seln->public_self, 
					       &replyType, &replyBuf, 
					       &reply->length, &reply->format ))   
	        goto CvtFailed;
	    
	    byteLength = BYTE_SIZE( reply->length, reply->format );
	    offset = 0;
	}
	if ( !(byteLength - offset) )
	    break;
    } while( 1 );

    /*
     * The owner calls us with a zero length data to indicate the end of data
     * transfer. We call the user defined reply_proc with
     * the zero length data  to indicate to the client the end of 
     * incremental data transfer.
     */
    (*selReq->reply_proc)( SEL_REQUESTOR_PUBLIC(selReq), target, replyType, 
			 reply->data, 0, reply->format );    

    if ( reply->seln->done_proc )
        (*reply->seln->done_proc)( reply->seln->public_self, replyBuf, target );

    return TRUE;
    
CvtFailed:
    xv_sel_handle_error( SEL_BAD_CONVERSION, selReq, reply, target );
    reply->format = 0;
    reply->length = 0;
    XFree( (char *)(reply->data) );
    reply->data = (Xv_opaque) NULL;
    return FALSE;    
}    
    


/*
* This function is called by the selection requestor. It sets the MULTIPLE 
* atom pairs on the requestor window.
*/
static void
SetupMultipleRequest( reply, numTarget )
Sel_reply_info *reply;
int            numTarget;
{
    atom_pair   *pairPtr;
    int   i, count;
    
    reply->atomPair = (atom_pair *) xv_calloc( numTarget+1, sizeof( atom_pair ) );

    count = numTarget;
    
    for ( i = 1, pairPtr = reply->atomPair; count > 0 ; pairPtr++, count--, i++ )  {
	pairPtr->target = *(reply->target+i);
	pairPtr->property = xv_sel_get_property( reply->seln->dpy );
	SetExtendedData( reply, pairPtr->property, i-1 );
    }

    XChangeProperty( reply->seln->dpy, reply->requestor, reply->property, 
		    reply->property, 32, PropModeReplace, 
		    (unsigned char *) reply->atomPair, numTarget * 2  );

#ifdef SEL_DEBUG
    printf("SetUpMultipleRequest: win=%d  prop=%d  type=%d  length=%d  format=%d\n", 
       reply->requestor, reply->property, reply->property,numTarget * 2 , 32);
#endif

    reply->multiple = numTarget;
}

	


/*
* This procedure is called by the requestor, after the owner has set 
* the selection data on the requestor window. The target is MULTIPLE so
* the passed in user proc is call "numTarget" of times.
*/
static int
ProcessMultiple( selReq, reply, ev, blocking )
Sel_req_info    *selReq;
Sel_reply_info  *reply;
XSelectionEvent *ev;
int             blocking;
{
    atom_pair      *pPtr;
    int            format;
    Atom           type;
    unsigned char  *pair;
    unsigned long  length;
    unsigned long  bytesafter;
    int            byteLen;
    
    /*
     * Get the atom pair data that we set on our window. We can delete 
     * now since the owners has already read this.
     */

    if ( XGetWindowProperty( ev->display, ev->requestor, 
			    reply->property, 0L,  1000000, False, 
			    (Atom)AnyPropertyType,  &type, &format,  
			    &length, &bytesafter,
			    (unsigned char **) &pair ) != Success ) {
	xv_error( selReq->public_self, 
		  ERROR_STRING,XV_MSG("XGetWindowProperty Failed"),
                  ERROR_PKG,SELECTION,
		  0 );
	 xv_sel_handle_error( SEL_BAD_CONVERSION, selReq, reply, 
		     reply->seln->atomList->multiple );
	 return FALSE;
    }

    byteLen = BYTE_SIZE( length, format ) / sizeof( atom_pair );

    for ( pPtr = (atom_pair *) pair; byteLen;  byteLen--, pPtr++ ) {
	/*
	 * The owner replace the property to None, if it failes to convert .
	 */
	if ( pPtr->property == None ) 
            xv_sel_handle_error( SEL_BAD_CONVERSION, selReq, reply, pPtr->target );
	else 
	    XvGetRequestedValue( selReq, ev, reply, pPtr->property,
				     pPtr->target, blocking );

	/* 
	 * The properties used for multiple request are freed in
	 * xv_sel_end_request proc.
	 */
    }  /* for loop */
    XFree( (char *)pair );
    return TRUE;
}



static int
ProcessIncr( selReq, reply, target, ev )
Sel_req_info    *selReq;
Sel_reply_info  *reply;
Atom            target;
XSelectionEvent *ev;
{
    int               format;
    Atom              type;
    unsigned long     length;
    unsigned long     bytesafter;
    unsigned char     *propValue;
    XEvent            event;
    XPropertyEvent    *propEv;
    XWindowAttributes winAttr;
    int   status = xv_sel_add_prop_notify_mask( ev->display, reply->requestor, 
				     &winAttr );

    /*
     * We should start the transfer process by deleting the (type==INCR) 
     * property and by calling the requestor reply_proc with "type" set to
     * INCR and replyBuf set to a lower bound on the number of bytes of
     * data in the selection.
     */
    if ( XGetWindowProperty( ev->display, reply->requestor, 
			    reply->property, 0L,1000000,True, AnyPropertyType,  
			    &type, &format,  &length, &bytesafter,
			    (unsigned char **) &propValue) != Success ) {
	xv_error( selReq->public_self, 
		 ERROR_STRING,XV_MSG( "XGetWindowProperty Failed"),
                 ERROR_PKG, SELECTION,
		 0 );
	xv_sel_handle_error( SEL_BAD_CONVERSION, selReq, reply, target );
	return FALSE;
    }

    /*
     * Note: The user reply_proc should free "propValue".
     */
    (*selReq->reply_proc)( SEL_REQUESTOR_PUBLIC(selReq), target, type, 
			   propValue,  length, format );

    do {
	/* Wait for PropertyNotify with stat==NewValue */
	if ( !xv_sel_block_for_event( ev->display, &event, reply->timeout, 
			    xv_sel_check_property_event, (char *) reply ) ) {
	    if ( status )
	        XSelectInput( ev->display, reply->requestor,
		     winAttr.your_event_mask );

	    xv_sel_handle_error( SEL_TIMEDOUT, selReq, reply, target );
	    return FALSE;
	}

	propEv = (XPropertyEvent *) &event;

	/* retrieve the data and delete */
	if ( XGetWindowProperty( propEv->display, propEv->window, 
				propEv->atom,0L,10000000,True,AnyPropertyType,
  				&type, &format,  &length, &bytesafter,
				(unsigned char **) &propValue) != Success ) {
	    xv_error( selReq->public_self, 
                      ERROR_STRING, XV_MSG("XGetWindowProperty Failed"),
                      ERROR_PKG,SELECTION,
                      0 );
	    xv_sel_handle_error( SEL_BAD_CONVERSION, selReq, reply, target );

	    if ( status )
	        XSelectInput( ev->display, reply->requestor,
		     winAttr.your_event_mask );

	    return FALSE;
	}

	if ( !type/* != *(reply->target)*/ ) {
	    /* reset length */
	    length=1;
	    continue;
	}

	if ( length == 0 )
	    propValue = (unsigned char *) NULL;

	/*
	 * The owner calls us with a zero length data to indicate the end of 
	 * data transfer. We call the user defined reply_proc with
	 * the zero length data  to indicate to the client the end of 
	 * incremental data transfer.
	 */
	(*selReq->reply_proc)( SEL_REQUESTOR_PUBLIC(selReq), target, type, 
			     propValue, length, format );
    } while ( length );

    /* 
     * If we have added PropertyChangeMask to the win, reset the mask to
     * it's original state.
     */
    if ( status )
        XSelectInput( ev->display, reply->requestor,
		     winAttr.your_event_mask );

    XDeleteProperty( ev->display, reply->requestor, reply->property );
    return TRUE;
}



static int
  XvGetRequestedValue( Sel_req_info *selReq, XSelectionEvent *ev,
                       Sel_reply_info *replyInfo,
                       Atom property, Atom target, int blocking )
{
    int            format;
    Atom           type;
    unsigned long  length;
    unsigned long  bytesafter;
    unsigned char  *propValue;

    
    if ( XGetWindowProperty( ev->display, ev->requestor, 
			    property, 0L,  10000000, False, 
			    AnyPropertyType,  &type, &format,  &length, 
			    &bytesafter, (unsigned char **) &propValue) != 
	Success ) {
	xv_error( selReq->public_self, 
    		  ERROR_STRING, XV_MSG("XGetWindowProperty Failed"),
                  ERROR_PKG,SELECTION,
		  0 );

	xv_sel_handle_error( SEL_BAD_CONVERSION, selReq, replyInfo, target );
	return FALSE;
    }

    /*
     * If we are processing a blocking request and there is no reply_proc
     * registered with package and the selection owner is 
     * transferring the data in increments, reject the data transfer.
     */
    if ( ( selReq->reply_proc == NULL ) 
	&& (type == replyInfo->seln->atomList->incr) )  
	return FALSE;

    if ( type == replyInfo->seln->atomList->incr )  {
	replyInfo->property = property;


	/*
	 * The following lines of code were used to process the INCR target
	 * without blocking the thread of execution. It could be used again
	 * if we decide to go back to a non-blocking schem.

	if ( !blocking ) {
	    replyInfo->incr++;
            ProcessNonBlkIncr( selReq, replyInfo, ev, target );
	    return TRUE;
	    
	}
	*/


	ProcessIncr( selReq, replyInfo, target, ev );
	return( SEL_INCREMENT );
    }    
    replyInfo->data = (Xv_opaque ) propValue;
    replyInfo->length = length;
    replyInfo->format = format;

    /*
     * Note: The user reply_proc should free "propValue".
     */
    if ( selReq->reply_proc )
        (*selReq->reply_proc)( SEL_REQUESTOR_PUBLIC(selReq), target, type, 
			     (Xv_opaque) propValue, length, format );
    
    XDeleteProperty( ev->display, replyInfo->requestor, property );
    return( TRUE );
}


/*
* This function initiates and processes request. It sends a SelectionRequest
* event to the selection owner and blocks (waits) for the owner 
* SelectionNotify event. If the "numTarget" is greater than one it sends a 
* SelectionRequest to the owner with the target set to MULTIPLE.
*/
static int
GetSelection( dpy, xid, selReq, reply, time )
Display        *dpy;
XID            xid;
Sel_req_info   *selReq;
Sel_reply_info **reply;
Time           time;
{
    Selection_requestor  requestor;
    Sel_reply_info       *replyInfo;
    Atom                 selection;
    Sel_owner_info       *sel;
    XEvent               event;
    int                  arg;
    
    /* get the requestor's xid, event time, selection and the target */
    requestor = SEL_REQUESTOR_PUBLIC( selReq );
    selection = (Atom) xv_get( requestor, SEL_RANK );

    /*
     * Have we saved any data on this window?
     */
    sel = xv_sel_find_selection_data( dpy, selection, xid );

    replyInfo = NewReplyInfo( requestor, xid, sel, time, selReq );
    *reply = replyInfo;
	
	
    if ( selReq->nbr_types > 1 )   /* target is MULTIPLE */
        SetupMultipleRequest( replyInfo, selReq->nbr_types );


    /*
     * Compatibility routine between the old and the new selection packages.
     * When the textsw is converted to use the new selection package, this
     * routine can be deleted.
     */
    if ( OldPkgIsOwner( dpy, selection, xid, replyInfo, selReq ) )
        return TRUE;


    if (sel->xid && (sel->own == TRUE )) {    
	/* Do local process data transfer */
	sel->status |= SEL_LOCAL_PROCESS;
	return HandleLocalProcess( selReq, replyInfo, 1 );
    }
    else  {
	XSelectionEvent  *ev;
	
	/*
	 * Send the SelectionRequest message to the selection owner.
	 */
	XConvertSelection( dpy, selection, *replyInfo->target, 
			  replyInfo->property,  xid, replyInfo->time);

	/*
	 * Wait for the SelectionNotify event.
	 */
	if ( !xv_sel_block_for_event( dpy,&event,replyInfo->timeout, xv_sel_check_selnotify, 
			    (char *) replyInfo )) {
	    xv_sel_handle_error( SEL_TIMEDOUT, selReq,replyInfo,*replyInfo->target);
	    goto Error;
	}
	ev = (XSelectionEvent *) &event;

	if ( !CheckSelectionNotify( selReq, replyInfo, ev, 1 ) ) 
	    goto Error;

	if (ev->target == replyInfo->seln->atomList->multiple || 
	    replyInfo->multiple) {

	    /*
	     * If we are processing a blocking request and the requestor
	     * has requested for MULTIPLE targets with no reply_proc 
	     * registered, reject the data transfer.
	     */
	    if ( selReq->reply_proc == NULL ) 
		goto Error;

	    if ( ProcessMultiple( selReq, replyInfo, ev, 1 ) )  
		return TRUE;

	}
	if ( ev->target == replyInfo->seln->atomList->incr ) {
	    /*
	     * If we are processing a blocking request and the selection 
	     * owner is transferring the data in increments with no reply_proc
	     * registered, reject the data transfer.
	     */
	    if ( selReq->reply_proc == NULL ) 
	        goto Error;
		
	    if ( ProcessIncr( selReq,replyInfo,*replyInfo->target,ev ) )  {
		return TRUE;
	    }
	}
	return XvGetRequestedValue( selReq, ev, replyInfo, ev->property, 
				   *replyInfo->target, 1 );
    }

Error:
    xv_sel_free_property( dpy, replyInfo->property );
    return FALSE;
}


static Xv_opaque 
SelBlockReq( selReq, length, format )
Sel_req_info  *selReq;
unsigned long  *length;
int   *format;
{
    Selection_requestor  requestor;
    Sel_reply_info       *reply;    
    struct timeval       *time;
    Xv_window            win;
    Display              *dpy;
    Time                 XTime;
    XID                  xid;
    
    /* get the requestor's xid, event time, selection and the target */
    requestor = SEL_REQUESTOR_PUBLIC( selReq );
    win = (Xv_window) xv_get( requestor, XV_OWNER );
    xid = (XID) xv_get( win, XV_XID );
    dpy = XV_DISPLAY_FROM_WINDOW( win );

    /* get the time;if it hasn't been set, set it */
    time = (struct timeval *) xv_get( requestor, SEL_TIME );

    XTime = xv_sel_cvt_timeval_to_xtime( time );
    
    if ( XTime == XV_ZERO )  {
        XTime = xv_sel_get_last_event_time( dpy, xid );
	xv_set( requestor, SEL_TIME, xv_sel_cvt_xtime_to_timeval( XTime ), 0 );
    }
    time->tv_sec = 0;
    time->tv_usec = 0;
    if (GetSelection(dpy, xid, selReq, &reply, XTime ) )  {
	if ( reply->incr || reply->multiple ) 	{
	    *length = XV_OK;
	    *format = reply->format;
	    xv_set( requestor, SEL_TIME, time, 0 );
	    return  (Xv_opaque) NULL;
	}
	*length = reply->length;
	*format = reply->format;
	xv_set( requestor, SEL_TIME, time, 0 );
        return  (Xv_opaque) reply->data;
    }
    xv_set( requestor, SEL_TIME, time, 0 );
    *length = SEL_ERROR;
    *format = 0;
    return (Xv_opaque) NULL;
}


int 
sel_post_req( sel )
Selection_requestor  sel;
{
    Sel_req_info   *selReq;
    struct timeval *time;
    Display        *dpy;
    Xv_window      win;
    Time           XTime;
    XID            xid;
    
    /* get the requestor's xid, event time, selection and the target */
    selReq = SEL_REQUESTOR_PRIVATE( sel );
    win = (Xv_window) xv_get( sel, XV_OWNER );
    xid = (XID) xv_get( win, XV_XID );
    dpy = XV_DISPLAY_FROM_WINDOW( win );

    /*
     * If no reply proc is defined, return XV_ERROR.
     */
    if ( selReq->reply_proc == NULL )
        return XV_ERROR;
    
    /* get the time;if it hasn't been set, set it */
    time = (struct timeval *) xv_get( sel, SEL_TIME );

    XTime = xv_sel_cvt_timeval_to_xtime( time );
    
    if ( XTime == XV_ZERO )  {
        XTime = xv_sel_get_last_event_time( dpy, xid );
	xv_set( sel, SEL_TIME, xv_sel_cvt_xtime_to_timeval( XTime ), 0 );
    }
    XvGetSeln( dpy, xid, selReq, XTime, 0 );
    time->tv_sec = 0;
    time->tv_usec = 0;
    xv_set( sel, SEL_TIME, time, 0 );
    return XV_OK;
}


static XID 
  SelGetOwnerXID( Sel_req_info  *selReq )
{
    Sel_info        *sel_info;

    sel_info = SEL_PRIVATE( selReq->public_self );
    return XGetSelectionOwner( sel_info->dpy, sel_info->rank );
}


/*
* This function initiates and processes request. It sends a SelectionRequest
* event to the selection owner.
* If the number of target is greater than one it sends a SelectionRequest 
* to the owner with the target set to MULTIPLE.
*/
static void
  XvGetSeln( dpy, xid, selReq, time, blocking )
Display        *dpy;
XID            xid;
Sel_req_info   *selReq;
Time           time;
int            blocking;
{
    Selection_requestor  requestor;
    Sel_reply_info       *replyInfo;
    Sel_owner_info       *sel;
    Atom                 selection;

    /* get the requestor's xid, event time, selection and the target */
    requestor = SEL_REQUESTOR_PUBLIC( selReq );
    selection = (Atom) xv_get( requestor, SEL_RANK );

    /*
     * Have we saved any data on this window?
     */
    sel = xv_sel_find_selection_data( dpy, selection, xid );

    replyInfo = NewReplyInfo( requestor, xid, sel, time, selReq );
	
    if ( selReq->nbr_types > 1 )   /* target is MULTIPLE */
        SetupMultipleRequest( replyInfo, selReq->nbr_types );



    if ( sel->xid && (sel->own == TRUE )) { 
	sel->status |= SEL_LOCAL_PROCESS;
	HandleLocalProcess( selReq, replyInfo, blocking );
    }
    else  {
	struct  itimerval  timeout;

	xv_sel_set_reply( replyInfo );

	timeout.it_interval.tv_usec = 0;
	timeout.it_interval.tv_sec = 0;

	timeout.it_value.tv_usec = 0;
	timeout.it_value.tv_sec = replyInfo->timeout;


	(void) notify_set_itimer_func((Notify_client) replyInfo,
                 xv_sel_handle_sel_timeout, ITIMER_REAL, &timeout,
                 (struct itimerval *) ITIMER_NULL );
	

	/*
	 * Send the SelectionRequest message to the selection owner.
	 */
	XConvertSelection( dpy, selection, *replyInfo->target, 
			  replyInfo->property,  xid, replyInfo->time);
    }
}


Xv_private int
xv_sel_handle_selection_notify( ev )
XSelectionEvent *ev;
{
    Sel_reply_info   *reply;
    Sel_req_info     *selReq;
    XWindowAttributes  winAttr;

    reply = (Sel_reply_info *) xv_sel_get_reply( ev );

#ifdef SEL_DEBUG1
    printf("Recieved SelectionNotify win = %d\n",ev->requestor);
#endif

    if ( reply == NULL )
        return FALSE;

    if ( !CheckSelectionNotify( reply->req_info, reply, ev, 0  ) )
        return FALSE;

    selReq = reply->req_info;
    
    if ( ev->target == reply->seln->atomList->incr ) {
	reply->incr = TRUE;
	
	reply->status = xv_sel_add_prop_notify_mask( ev->display, reply->requestor, 
					 &winAttr );

	if ( ProcessNonBlkIncr( selReq, reply, ev, *reply->target ) )  
	    return TRUE;

    }
    if (ev->target == reply->seln->atomList->multiple || 
	reply->multiple) {

	reply->status = xv_sel_add_prop_notify_mask( ev->display, reply->requestor, 
					 &winAttr );

	if ( ProcessMultiple( selReq, reply, ev, 0 ) )  {
	    if ( !reply->incr )
	        xv_sel_end_request( reply );
	    return TRUE;
	}
    }
    reply->incr = FALSE;
    if ( XvGetRequestedValue( selReq, ev, reply, ev->property, 
			     *reply->target, 0 ) )  {
	    if ( !reply->incr )
			xv_sel_end_request( reply );
	return TRUE;
    }
    return FALSE;
}




static int
CheckPropertyNotify( ev, reply )
XPropertyEvent *ev;
Sel_reply_info   *reply;
{
    int  i;
    
    if ( !reply->incr )
        return FALSE;

    if ( ev->window != reply->requestor )
        return FALSE;

    if ( ev->state != PropertyNewValue )
        return FALSE;

    if ( reply->multiple ) 
        for ( i=0; i < reply->multiple; i++ )    {
	    if ( ev->atom == reply->atomPair[i].property )
	        return TRUE;
	}
    
    if ( ev->atom != reply->property )
        return FALSE;    
	
    return TRUE;
}



static int
  ProcessNonBlkIncr(Sel_req_info *selReq, Sel_reply_info *reply,
                    XSelectionEvent *ev, Atom target)
{
    unsigned long  length;
    unsigned long  bytesafter;
    unsigned char  *propValue;
    int            format;
    Atom           type;

    /*
     * We should start the transfer process by deleting the (type==INCR) 
     * property and by calling the requestor reply_proc with "type" set to
     * INCR and replyBuf set to a lower bound on the number of bytes of
     * data in the selection.
     */
    if ( XGetWindowProperty( ev->display, reply->requestor, 
			    reply->property, 0L,10000000,True, AnyPropertyType,  
			    &type, &format,  &length, &bytesafter,
			    (unsigned char **) &propValue) != Success ) {
	xv_error( selReq->public_self, 
		  ERROR_STRING, XV_MSG("XGetWindowProperty Failed"),
                  ERROR_PKG,SELECTION,
		  0 );
	xv_sel_handle_error( SEL_BAD_CONVERSION, selReq, reply, *reply->target );
	return FALSE;
    }
    
    (*selReq->reply_proc)( SEL_REQUESTOR_PUBLIC(selReq), target,
			   type, propValue, length, format );


    return TRUE;
}


Xv_private int
xv_sel_handle_property_notify( ev )
XPropertyEvent  *ev;
{
    Sel_reply_info   *reply;
    Requestor        *req;
    
#ifdef SEL_DEBUG1
    printf("Recieved Property Notify win=%d  atom=%s  state=%s\n", 
	   ev->window, XGetAtomName(ev->display,ev->atom), (ev->state== PropertyNewValue)? "NewVALUE":"Deleted" );
#endif
    
    /*
     * If this event is for the selection requestor, process the owner's
     * reply and return.
     * If it is for the selection owner, process the request.
     */
    reply = (Sel_reply_info *) xv_sel_get_reply( ev );
    if ( reply != NULL )
        return ProcessReply( reply, ev );

    req = (Requestor *) SelGetReq( ev );
    if ( req != NULL )
        return ProcessReq( req, ev );
    return FALSE;
}


	
static void
SetExtendedData( reply, property, typeIndex )
Sel_reply_info  *reply;
Atom   property;
int    typeIndex;
{
    Sel_prop_info   *exType;

    if ( reply->req_info->typeTbl[typeIndex].status & SEL_PROPERTY_DATA )  {
	exType = reply->req_info->typeTbl[typeIndex].propInfo;
	
        XChangeProperty( reply->seln->dpy, reply->requestor, property,
			exType->type,exType->format, PropModeReplace,
			(unsigned char *) exType->data, exType->length );

#ifdef SEL_DEBUG
	printf("SetExtendedData: win=%d  prop=%d  type=%d  length=%d  format=%d\n", 
	       reply->requestor, property, exType->type,exType->length ,
	       exType->format );
#endif
    }
    
}


static int
ProcessReply( reply, ev )
Sel_reply_info  *reply;
XPropertyEvent  *ev;
{
    struct itimerval timeout;
    unsigned long    length;
    unsigned long    bytesafter;
    unsigned char    *propValue;
    Sel_req_info     *selReq;
    int              format;
    Atom             type;
    Atom             target;
    int              i;
    
    if ( !CheckPropertyNotify( ev, reply ) ) 
        return FALSE;

    selReq = reply->req_info;

    /* retrieve the data and delete */
    if ( XGetWindowProperty( ev->display, ev->window, 
			    ev->atom,0L,10000000,True,AnyPropertyType,
			    &type, &format,  &length, &bytesafter,
			    (unsigned char **) &propValue) != Success ) {
	xv_error( selReq->public_self, 
		 ERROR_STRING,XV_MSG("XGetWindowProperty Failed"),
                 ERROR_PKG,SELECTION,
		 0 );
	xv_sel_handle_error( SEL_BAD_CONVERSION, selReq, reply, *reply->target );
	return FALSE;
    }
#ifdef SEL_DEBUG1      
 printf("GEtting Window Property win = %d prop = %s type = %s length = %d bytesafter = %d\n", ev->window, XGetAtomName(ev->display,ev->atom),XGetAtomName(ev->display,type),length,bytesafter);
#endif

    if( !type )
        return TRUE;

    target = *reply->target;
    if ( reply->multiple ) 
        for ( i=0; i < reply->multiple; i++ )    {
	    if ( ev->atom == reply->atomPair[i].property )
	        target = reply->atomPair[i].target;
	}

    (*selReq->reply_proc)( SEL_REQUESTOR_PUBLIC(selReq), target, 
			   type, propValue, length, format );


    /*
     * For INCR the time out should be the time between each reply.
     * We just got a reply reset the timer.
     */

    timeout.it_interval.tv_usec = 0;
    timeout.it_interval.tv_sec = 0;
    timeout.it_value.tv_usec = 0;
    timeout.it_value.tv_sec = reply->timeout;

    (void) notify_set_itimer_func((Notify_client) reply, xv_sel_handle_sel_timeout, 
               ITIMER_REAL, &timeout, (struct itimerval *) ITIMER_NULL );
	
	/*
	 * If length is set to zero, we have completed an incr transfer.
	 */
	 if ( length == 0 )
	  	reply->incr--;

	/*
	 * We have completed all the incr replies. Finish this request.
	 */
    if ( reply->incr == 0 )
	    xv_sel_end_request (reply );
    
    return TRUE;
}


    
static Requestor *
SelGetReq( ev )
XPropertyEvent  *ev;
{
    Requestor  *req;

    if ( reqCtx == 0 )
        reqCtx = XUniqueContext();

    
    if ( XFindContext( ev->display, ev->atom, reqCtx, (caddr_t *)&req )) 
	return (Requestor *) NULL;
    return  req;
}





static int
  ProcessReq( Requestor  *req, XPropertyEvent  *ev )
{
    if ( ev->window != req->owner->xid ||  ev->atom != req->property ||
	 ev->state != PropertyDelete || ev->time < req->time )
        return FALSE;
    
    xv_sel_handle_incr( req->owner );
    return TRUE;
}



static void
SelSaveData( buffer, reply, size )
char           *buffer;
Sel_reply_info *reply;
int            size;
{
    char  *strPtr;

    
    /*
     * One extra byte is malloced than is needed. This last byte is null 
     * terminated for returning string properties, so the client does't then 
     * have to recopy the string to make it null terminated.
     * This is done to make local process transactions to be exactly like
     * non-local transactions. XGetWindowProperty null terminates the last
     * extra byte.
     */
    reply->data = (Xv_opaque) xv_malloc( size+1 );

    strPtr = (char *) reply->data;
    strPtr[size] = '\0';

    if ( buffer != (char *) NULL )
        XV_BCOPY( (char *)buffer, (char *)reply->data, size );
}




static int
  OldPkgIsOwner( Display *dpy, Atom selection, Window xid,
                 Sel_reply_info *reply, Sel_req_info *selReq )
{
    Sel_cmpat_info      *cmpatInfo, *infoPtr;
			   
    if ( cmpatCtx == 0 )
        cmpatCtx = XUniqueContext();
    if ( XFindContext(dpy, DefaultRootWindow(dpy), cmpatCtx,(caddr_t *)&cmpatInfo))
	return FALSE;
    
    infoPtr = cmpatInfo;
    do {
	if ( (infoPtr->selection == selection) && 
	    (infoPtr->clientType == OLD_SEL_CLIENT) ) {


            Xv_Server        server;
	    Xv_window        xvWin;
	    Seln_holder      holder;
	    Seln_agent_info  *agent;
	    Seln_request     *response;
	    Seln_rank        rank;
	    Atom             type = XA_STRING;


           xvWin = win_data( dpy, xid );
           server = XV_SERVER_FROM_WINDOW( xvWin );

           if (*reply->target == xv_get(server,SERVER_ATOM,"_SUN_SELECTION_END")){
               /* The old Selection package client should reject 
                * SUN_SELECTION_END 
                */
	      reply->data = (Xv_opaque ) NULL;
	      reply->length = SEL_ERROR;
	      reply->format = 0;
              return TRUE;
           }
           agent = (Seln_agent_info *) xv_get(server,
                                              (Attr_attribute)XV_KEY_DATA, 
				              SELN_AGENT_INFO);	    
	   rank = selection_to_rank( selection, agent );
	   holder = selection_inquire( server, rank );

           if (*reply->target == xv_get(server,SERVER_ATOM,"_SUN_SELN_YIELD")){
	      response = selection_ask(server,&holder,SELN_REQ_YIELD,0,NULL);
              return TRUE;
           }

	   response = selection_ask(server,&holder,SELN_REQ_CONTENTS_ASCII,0,NULL);

	   reply->data = (Xv_opaque )response->data+sizeof(SELN_REQ_CONTENTS_ASCII);
	   reply->data = (Xv_opaque) strdup((char *) reply->data);
	   reply->length = strlen( (char *) reply->data );
	   reply->format = 8;

	    /*
	     * Note: The user reply_proc should free "propValue".
	     */
	    if ( selReq->reply_proc )
	    (*selReq->reply_proc)( SEL_REQUESTOR_PUBLIC(selReq), *reply->target, type, 
				  (Xv_opaque) reply->data, reply->length, 
				  reply->format );

	    return TRUE;
	}
	if ( infoPtr->next == NULL )
	    break;
	
	infoPtr = infoPtr->next;
    }while (1);
    return FALSE;
}

