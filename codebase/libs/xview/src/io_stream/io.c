#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)io.c 20.19 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#include <xview_private/i18n_impl.h>
#include <xview/xv_error.h>
#include <xview_private/io_stream.h>

/* GENERIC FUNCTIONS THAT APPLY TO BOTH INPUT AND OUTPUT */

void
stream_close(stream)
    STREAM         *stream;
{
    switch (stream->stream_type) {
      case Input:{
	    struct input_ops_vector *ops = stream->ops.input_ops;
	    (*(ops->close)) (stream);
	    goto out;
	}

      case Output:{
	    struct output_ops_vector *ops = stream->ops.output_ops;
	    (*(ops->close)) (stream);
	    goto out;
	}

      default:
	xv_error((Xv_opaque)stream,
		 ERROR_SEVERITY, ERROR_NON_RECOVERABLE,
		 ERROR_STRING, 
		 XV_MSG("invalid stream type"),
		 NULL);
    }
out:free((char *) stream);	/* client should have freed the client data */
}

struct posrec
stream_get_pos(stream)
    STREAM         *stream;
{
    switch (stream->stream_type) {
      case Input:{
	    struct input_ops_vector *ops = stream->ops.input_ops;
	    return ((*ops->get_pos) (stream));
	}
      case Output:{
	    struct output_ops_vector *ops = stream->ops.output_ops;
	    return ((*ops->get_pos) (stream));
	}
      default:{
	  struct posrec null_posrec;
	  null_posrec.lineno = -1;
	  null_posrec.charpos = -1;
	  xv_error((Xv_object)stream,
		   ERROR_SEVERITY, ERROR_NON_RECOVERABLE,
		   ERROR_STRING, 
		   XV_MSG("invalid stream type"), 
		   NULL);
	  return (null_posrec);
        }
    }
}
