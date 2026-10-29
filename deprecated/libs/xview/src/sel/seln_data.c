#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)seln_data.c 1.12 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#include <xview_private/seln_impl.h>


Seln_function_buffer seln_null_function = {
    SELN_FN_ERROR, SELN_UNKNOWN,
    SELN_NULL_HOLDER, SELN_NULL_HOLDER,
    SELN_NULL_HOLDER, SELN_NULL_HOLDER
};
Seln_holder     seln_null_holder = SELN_NULL_HOLDER;
Seln_request    seln_null_request = {
    0, {0, 0}, 0, SELN_UNKNOWN, SELN_FAILED, 0, 0
};
