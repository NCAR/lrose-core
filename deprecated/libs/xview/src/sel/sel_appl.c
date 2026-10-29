#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)sel_appl.c 20.30 93/06/29";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#include <xview_private/i18n_impl.h>
#include <xview_private/portable.h>
#include <xview_private/seln_impl.h>
#include <xview/attr.h>
#include <xview/rect.h>
#include <xview/server.h>
#include <xview/sel_compat.h>

static void     seln_init_request_buffer();

/*
 * Generic request to another holder
 */
Xv_public Seln_request *
#ifdef ANSI_FUNC_PROTO
seln_ask(Seln_holder *holder, ...)
#else
seln_ask(holder, va_alist)
    Seln_holder    *holder;
va_dcl
#endif
{
    AVLIST_DECL;
    va_list         args;

    VA_START(args, holder);
    MAKE_AVLIST( args, avlist );
    va_end(args);
    return (selection_ask(xv_default_server, holder, ATTR_LIST, avlist, NULL));
}

/*ARGSUSED*/
Xv_public Seln_request *
#ifdef ANSI_FUNC_PROTO
selection_ask(Xv_Server server, Seln_holder *holder, ...)
#else
selection_ask(server, holder, va_alist)
    Xv_Server       server;
    Seln_holder    *holder;
va_dcl
#endif
{
    static Seln_request *buffer;
    va_list         valist;

    if (buffer == (Seln_request *) NULL) {
	buffer = (Seln_request *)
	    xv_malloc((unsigned) (sizeof(Seln_request)));
	if (buffer == (Seln_request *) NULL) {
	    (void) fprintf(stderr,
		       XV_MSG("Couldn't malloc request buffer (no swap space?)\n"));
	    return &seln_null_request;
	}
    }

    if (holder->state == SELN_NONE) {
	return &seln_null_request;
    }
    VA_START(valist, holder);
    /* WARNING: A previous call to attr_make here performed a test to
       make sure that buffer->data could hold the valist. Since
       buffer->data holds almost 500 attributes and the attribute
       package can only deal with 250 attributes, the test was removed
       as unnecessary. Also selection_ask is never expected to have
       more than a dozen parameters.
    */
    copy_va_to_av( valist, (Attr_avlist) buffer->data, 0 );

    va_end(valist);
    seln_init_request_buffer(buffer, holder);
    if (selection_request(server, holder, buffer) == SELN_SUCCESS) {
	return buffer;
    } else {
	return &seln_null_request;
    }
}


Xv_public void
#ifdef ANSI_FUNC_PROTO
seln_init_request(Seln_request *buffer, Seln_holder *holder, ...)
#else
seln_init_request(buffer, holder, va_alist)
    Seln_request   *buffer;
    Seln_holder    *holder;
va_dcl
#endif
{
    AVLIST_DECL;
    va_list         args;

    VA_START(args, holder);
    MAKE_AVLIST( args, avlist );
    va_end(args);
    selection_init_request(xv_default_server, buffer, holder, 
			   ATTR_LIST, avlist, NULL);
}

/*ARGSUSED*/
Xv_public void
#ifdef ANSI_FUNC_PROTO
selection_init_request(Xv_Server server, Seln_request *buffer,
                       Seln_holder *holder, ...)
#else
selection_init_request(server, buffer, holder, va_alist)
    Xv_Server	    server;
    Seln_request   *buffer;
    Seln_holder    *holder;
va_dcl
#endif
{

    va_list         valist;

    VA_START(valist, holder);
    /* WARNING: A previous call to attr_make here performed a test to
       make sure that buffer->data could hold the valist. Since
       buffer->data holds almost 500 attributes and the attribute
       package can only deal with 250 attributes, the test was removed
       as unnecessary. Also selection_ask is never expected to have
       more than a dozen parameters.
    */
    copy_va_to_av( valist, (Attr_avlist) buffer->data, 0 );

    va_end(valist);
    seln_init_request_buffer(buffer, holder);
}

Xv_public       Seln_result
#ifdef ANSI_FUNC_PROTO
seln_query(Seln_holder *holder, Seln_result (*reader) (Seln_request *),
           char *context, ...)
#else
seln_query(holder, reader, context, va_alist)
    Seln_holder    *holder;
    Seln_result(*reader) ();
    char           *context;
va_dcl
#endif
{
    AVLIST_DECL;
    va_list         args;

    VA_START(args, context);
    MAKE_AVLIST( args, avlist ); 
    va_end(args);
    return (selection_query(xv_default_server, holder, reader, context,
			    ATTR_LIST, avlist, NULL));
}



/*ARGSUSED*/
Xv_public       Seln_result
#ifdef ANSI_FUNC_PROTO
selection_query(Xv_Server server, Seln_holder *holder,
           Seln_result (*reader) (Seln_request *), char *context, ...)
#else
selection_query(server, holder, reader, context, va_alist)
    Xv_Server       server;
    Seln_holder    *holder;
    Seln_result	  (*reader) ();
    char           *context;
va_dcl
#endif
{
    static Seln_request *buffer;
    va_list         valist;

    if (buffer == (Seln_request *) NULL) {
	buffer = (Seln_request *)
	    xv_malloc((unsigned) (sizeof(Seln_request)));
	if (buffer == (Seln_request *) NULL) {
	    (void) fprintf(stderr,
		       XV_MSG("Couldn't malloc request buffer (no swap space?)\n"));
	    return SELN_FAILED;
	}
    }
    if (holder->state == SELN_NONE) {
	return SELN_FAILED;
    }

    VA_START(valist, context);
    /* WARNING: A previous call to attr_make here performed a test to
       make sure that buffer->data could hold the valist. Since
       buffer->data holds almost 500 attributes and the attribute
       package can only deal with 250 attributes, the test was removed
       as unnecessary. Also selection_ask is never expected to have
       more than a dozen parameters.
    */
    copy_va_to_av( valist, (Attr_avlist) buffer->data, 0 );
    va_end(valist);

    seln_init_request_buffer(buffer, holder);
    buffer->requester.consume = reader;
    buffer->requester.context = context;
    return selection_request(server, holder, buffer);
}

static void
seln_init_request_buffer(buffer, holder)
    Seln_request   *buffer;
    Seln_holder    *holder;
{
    buffer->buf_size = attr_count((Attr_avlist) buffer->data) *
	sizeof(char *);
    buffer->rank = holder->rank;
    buffer->addressee = holder->access.client;
    buffer->replier = 0;
    buffer->requester.consume = 0;
    buffer->requester.context = 0;
}
