/*
 *      (c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *      pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *      file for terms of the license.
 */

#include <X11/Xlib.h>
#include <xview/xview.h>
#include <xview/seln.h>
#include <xview/dragdrop.h>
#include <xview_private/portable.h>
#ifdef SVR4 
#include <stdlib.h> 
#endif /* SVR4 */

#define DD_FAILED  -1

static int HandleNewDrop();

int
xv_decode_drop(ev, buffer, bsize)
    Event 	*ev;
    char	*buffer;
    unsigned int bsize;
{
    unsigned long	 nitems, bytes_after; 
    XClientMessageEvent *cM;
    Seln_holder	 	 seln_holder;
    Atom	         actual_type;
    int	                 actual_format,
			 data_length,
			 return_value,
			 NoDelete = False;
    char		*data;
    Xv_window		 window;


    if ((event_action(ev) != ACTION_DRAG_COPY) &&
	(event_action(ev) != ACTION_DRAG_MOVE) &&
	(event_action(ev) != ACTION_DRAG_LOAD))
	return (DD_FAILED);
				       	/* Dig out the ClientMessage event. */
    cM = (XClientMessageEvent *)event_xevent(ev);

    if ((window = win_data(cM->display, cM->window)) == XV_ZERO)
	return (DD_FAILED);

    if (cM->message_type == xv_get(XV_SERVER_FROM_WINDOW(window), SERVER_ATOM,
				   "_SUN_DRAGDROP_TRIGGER"))
	return(HandleNewDrop(ev, cM, window, buffer, bsize));


				        /* Find out who owns the drag. */
    seln_holder = seln_inquire(SELN_PRIMARY);
    if (seln_holder.state == SELN_NONE)
        return(DD_FAILED);
    			             	/* Use client set property. */
    if ((cM->data.l[4]) && (XGetWindowProperty(cM->display, cM->data.l[3],
					cM->data.l[4], 0L,(long)((bsize + 3)/4),
					True, AnyPropertyType, &actual_type,
					&actual_format, &nitems, &bytes_after,
			                (unsigned char **)&data) == Success)) {
	    data_length = strlen(data);
	    return_value = data_length + bytes_after;
	    if (data_length >= bsize) {
		data_length = bsize-1;
		NoDelete = True;
	    }
	    XV_BCOPY(data, buffer, data_length);
	    buffer[data_length] = '\0';
	    XFree(data);
    } else {
	Seln_request	*seln_buffer;
					/* Ask for the ascii contents of the */
					/* selection.			     */
	seln_buffer = seln_ask(&seln_holder,
				     SELN_REQ_CONTENTS_ASCII, 0,
				     NULL);

	if (seln_buffer->status == SELN_FAILED)
	    return(DD_FAILED);

	if (*((Seln_attribute *) seln_buffer->data) != SELN_REQ_CONTENTS_ASCII)
	    return(DD_FAILED);

	data = (char *) seln_buffer->data;
	data += sizeof(SELN_REQ_CONTENTS_ASCII);

	return_value = data_length = strlen(data);
	if (data_length >= bsize) {
		data_length = bsize-1;
		NoDelete = True;
	}
	XV_BCOPY(data, buffer, data_length);
        /* Insure the buffer ends with a NULL.*/
	buffer[data_length] = '\0';
    }
					/* If this is a move, then ask */
					/* the owner to delete the selection. */
    if ((event_action(ev) == ACTION_DRAG_MOVE) && (!NoDelete))
	seln_ask(&seln_holder, SELN_REQ_DELETE, 0, NULL);

    return(return_value);
}

static int
HandleNewDrop(ev, cM, window, buffer, bsize)
    Event 		*ev;
    XClientMessageEvent *cM;
    Xv_window		 window;
    char		*buffer;
    unsigned int 	 bsize;
{
    Selection_requestor  sel;
    Xv_drop_site	 ds;
    char 		*buf;
    int			 length,
			 format,
			 NoDelete = False;

    sel = xv_create(window, SELECTION_REQUESTOR,
				SEL_TYPE,	XA_STRING,
				SEL_TIME,	&event_time(ev),
				NULL);

    (void)dnd_decode_drop(sel, ev);

    buf = (char *)xv_get(sel, SEL_DATA, &length, &format);
    if (length == SEL_ERROR)
	return(DD_FAILED);

    if (length >= bsize) {
	bsize -= 1;
	NoDelete = True;
    }
    XV_BCOPY(buf, buffer, bsize);
    buffer[bsize] = '\0';
    free(buf);
                                /* If this is a move operation, we must ask
                                 * the source to delete the selection object.
                                 * We should only do this if the transfer of
                                 * data was successful.
                                 */
    if (event_action(ev) == ACTION_DRAG_MOVE && !NoDelete) {
        int length, format;
        xv_set(sel, SEL_TYPE_NAME, "DELETE", NULL);
        (void)xv_get(sel, SEL_DATA, &length, &format);
    }
    dnd_done(sel);
}
