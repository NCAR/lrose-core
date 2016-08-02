/*      @(#)filter.h 20.18 93/06/28 SMI      */

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#ifndef xview_filter_DEFINED
#define xview_filter_DEFINED

#include <xview/xv_c_types.h>
#include <xview_private/io_stream.h>

/*
 ***********************************************************************
 *		Typedefs, enumerations, and structs
 ***********************************************************************
 */

struct filter_rec {
	char           *key_name;
	int             key_num;
	char           *class;
	char          **call;
};

/*
 ***********************************************************************
 *			Globals
 ***********************************************************************
 */

/*
 * Private Functions 
 */
EXTERN_FUNCTION (struct filter_rec **xv_parse_filter_table, (STREAM *in, char *filename));

EXTERN_FUNCTION (void 	xv_free_filter_table, (struct filter_rec **table));

#endif /* ~xview_filter_DEFINED */
