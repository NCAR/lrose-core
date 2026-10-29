#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)xv_list.c 20.13 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#include <xview_private/xv_list.h>

Xv_private void
xv_sl_init(head)
    Xv_sl_head      head;
{
    head->next = XV_SL_NULL;
}

Xv_private      Xv_sl_link
xv_sl_add_after(head, link, new)
    register Xv_sl_head head;
    register Xv_sl_link link, new;
{
    if (link != XV_SL_NULL) {
	new->next = link->next;
	link->next = new;
    } else {
	new->next = head;
    }
    return (new);
}

Xv_private      Xv_sl_link
xv_sl_remove_after(head, link)
    register Xv_sl_head head;
    register Xv_sl_link link;
{
    register Xv_sl_link result;

    if (link != XV_SL_NULL) {
	result = link->next;
	link->next = result->next;
    } else {
	result = head;
    }
    return (result);
}

Xv_private      Xv_sl_link
xv_sl_remove(head, link)
    register Xv_sl_head head;
    register Xv_sl_link link;
{
    register Xv_sl_link prev;

    if ((head == link) || (link == XV_SL_NULL)) {
	prev = XV_SL_NULL;
    } else {
	XV_SL_FOR_ALL(head, prev) {
	    if (prev->next == link)
		break;
	}
#ifdef _XV_DEBUG
	abort();
#endif
    }
    return (xv_sl_remove_after(head, prev));
}
