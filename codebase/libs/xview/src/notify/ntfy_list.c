#ifndef	lint
#ifdef sccs
static char     sccsid[] = "@(#)ntfy_list.c 20.17 93/06/28 Copyr 1985 Sun Micro";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * Ntfy_list.c - NTFY_CLIENT and NTFY_CONDITION list management. Both
 * structures use NTFY_NODEs and can share the same list management code.
 */

#include <xview_private/i18n_impl.h>
#include <xview_private/ntfy.h>
#include <xview_private/ndis.h>
#include <xview_private/ndet.h>


static NTFY_NODE *node_list_tail[2];

pkg_private void
ntfy_append_node(node_list, new_node)
    register NTFY_NODE **node_list;
    register NTFY_NODE *new_node;
{
    register NTFY_NODE *node;

    ntfy_assert(NTFY_IN_CRITICAL, 29 /* Unprotected list manipulations */);
    new_node->n.next = NTFY_NODE_NULL;

    if( *node_list == NULL ) {
        *node_list = new_node;
         if( node_list == (NTFY_NODE **)&ndet_clients )
             node_list_tail[0] = new_node;
         else if( node_list == (NTFY_NODE **)&ndis_clients )
             node_list_tail[1] = new_node;
    }
             
    else if( node_list == (NTFY_NODE **)&ndet_clients ) {
        node_list_tail[0]->n.next = new_node;
        node_list_tail[0] = new_node;
    }
    else if( node_list == (NTFY_NODE **)&ndis_clients ) {
        node_list_tail[1]->n.next = new_node;
        node_list_tail[1] = new_node;
    } 
    else
        for (node = *node_list; node; node = node->n.next)
	    if (node->n.next == NTFY_NODE_NULL) {
	        node->n.next = new_node;
	        break;
            }
}

pkg_private void
ntfy_remove_node(node_list, node_axe)
    register NTFY_NODE **node_list;
    register NTFY_NODE *node_axe;
{
    register NTFY_NODE **node_ptr;
    register NTFY_NODE **node_ptr_next;
    register NTFY_NODE *node_ptr_prev = NULL;

    ntfy_assert(*node_list != NTFY_NODE_NULL, 30
		/* Tried to remove node from empty list */);
    ntfy_assert(node_axe != NTFY_NODE_NULL, 31 /* Tried to remove null node */);
    ntfy_assert(NTFY_IN_CRITICAL, 32 /* Unprotected list manipulations */);
    for (node_ptr = node_list; *node_ptr; node_ptr = node_ptr_next) {
	node_ptr_next = &((*node_ptr)->n.next);
	if (*node_ptr == node_axe) {
            if(( node_list == (NTFY_NODE **)&ndet_clients ) &&
               ( node_axe == node_list_tail[0] ))
                node_list_tail[0] = node_ptr_prev;
            else if(( node_list == (NTFY_NODE **)&ndis_clients ) &&
                    ( node_axe == node_list_tail[1] ))
                node_list_tail[1] = node_ptr_prev;

	    *node_ptr = *node_ptr_next;
	    ntfy_free_node(node_axe);
	    return;
	}
        node_ptr_prev = *node_ptr;
    }
    ntfy_fatal_error(XV_MSG("Tried to remove node that wasn't on list"));
}
