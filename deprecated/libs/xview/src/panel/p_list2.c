#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)p_list2.c 1.2 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1993 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE 
 *	file for terms of the license.
 */

#include <xview/panel.h>

/*
 * These 2 functions were introduced purely for the MIT build
 * In p_list.c, the statement:
 *	node->f.row_inactive = entry->inactive;
 * was giving errors because of the inactive() macro defined
 * in item_impl.h.
 * So, instead of accessing the inactive field directly in p_list.c,
 * the 2 functions below are used.
 */
Pkg_private void
panel_list_row_inactive_set(entry, value)
Panel_list_row_values	*entry;
int			value;		
{
    entry->inactive = value;
}

Pkg_private int
panel_list_row_inactive_get(entry)
Panel_list_row_values	*entry;
{
    return(entry->inactive);
}
