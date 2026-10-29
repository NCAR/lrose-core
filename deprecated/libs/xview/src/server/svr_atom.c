#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)svr_atom.c 1.7 93/06/28";
#endif
#endif


/*      
 *      (c) Copyright 1990 Sun Microsystems, Inc. Sun design patents 
 *      pending in the U.S. and foreign countries. See LEGAL_NOTICE 
 *      file for terms of the license. 
 */

#include <xview/xview.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#include <xview_private/svr_impl.h>
#include <xview/server.h>

static void update_atom_list();

Xv_private Atom 
server_intern_atom(server, atomName)
    Server_info	*server;
    char	*atomName;
{
    XrmQuark     	 quark;
    Atom	 	 atom;

		/* Convert the string into a quark.
		 * The atomName is copied and never freed.  This is acceptable
		 * since the quark can be shared between server objects.
		 */
    quark = XrmStringToQuark(atomName);

		/* See if we have an atom for this quark already */
    if (XFindContext(server->xdisplay, server->atom_mgr[ATOM],
		     (XContext)quark, (caddr_t *)&atom) == XCNOENT) {

	atom = XInternAtom(server->xdisplay, atomName, False);

			/* We don't care if SaveContext fails (no mem).  It
			 * just means that FindContext will return XCNOENT and
			 * the atom will need to be interned again.
			 */
			/* Support lookup by atom name */
	(void)XSaveContext(server->xdisplay, server->atom_mgr[ATOM],
			   (XContext)quark, (caddr_t) atom);

			/* Support lookup by atom value */
	(void)XSaveContext(server->xdisplay, server->atom_mgr[NAME],
			   (XContext)atom, (caddr_t)strdup(atomName));

	update_atom_list(server, atom);

    }
    return ((Atom)atom);
}

Xv_private char *
server_get_atom_name(server, atom)
    Server_info *server;
    Atom	 atom;
{
    XrmQuark     quark; 
    char	*atomName;

    if (XFindContext(server->xdisplay, server->atom_mgr[NAME], (XContext)atom,
		     &atomName) == XCNOENT) {
	if (!(atomName = XGetAtomName(server->xdisplay, atom)))
	    return((char *)NULL);

				/* Convert the string into a quark */
        quark = XrmStringToQuark(atomName);

				/* Support lookup by atom name */ 
	(void)XSaveContext(server->xdisplay, server->atom_mgr[ATOM],
			   (XContext)quark, (caddr_t) atom);

			        /* Support lookup by atom value */
        (void)XSaveContext(server->xdisplay, server->atom_mgr[NAME],
			   (XContext)atom, (caddr_t)atomName);

	update_atom_list(server, atom);
    }
    return((char *)atomName);
}

static void
update_atom_list(server, atom)
    Server_info *server;
    Atom 	 atom;
{
    unsigned int	 slot;
    Server_atom_list 	*atom_list_tail,
			*atom_list_head;

			/* Our list is made up of blocks of atoms.  When 
			 * a block is full, we allocate a new block.  These
			 * blocks of data are needed when the server is 
			 * destroyed.  We use them to free up all the
			 * XContext manager stuff.
			 */

			/* Get the tail of our list. */
	atom_list_tail = (Server_atom_list *)xv_get(SERVER_PUBLIC(server),
				       XV_KEY_DATA, server->atom_list_tail_key);

			/* Figure out what slot in this block is empty. */
	slot = server->atom_list_number % SERVER_LIST_SIZE;

	                /* If this is slot 0, we create a new block because we
			 * know we filled up the old one.
			 */
	if (slot == 0 && (server->atom_list_number/SERVER_LIST_SIZE != 0)) {
	    Server_atom_list *atom_list = xv_alloc(Server_atom_list);

	    atom_list->list[0] = atom;

	    atom_list_head = (Server_atom_list *)xv_get(SERVER_PUBLIC(server),
				       XV_KEY_DATA, server->atom_list_head_key);
	    XV_SL_ADD_AFTER(atom_list_head, atom_list_tail, atom_list);

	    xv_set(SERVER_PUBLIC(server), XV_KEY_DATA,
				      server->atom_list_tail_key, atom_list, NULL);
	} else
	    atom_list_tail->list[slot] = atom;

	server->atom_list_number++;
}

Xv_private int
server_set_atom_data(server, atom, data)
    Server_info	*server;
    Atom	 atom;
    Xv_opaque    data;
{
    if (XSaveContext(server->xdisplay, server->atom_mgr[DATA],
			   (XContext)atom, (caddr_t) data) == XCNOMEM)
	return(XV_ERROR);
    else
	return(XV_OK);
}


Xv_private Xv_opaque
server_get_atom_data(server, atom, status)
    Server_info	*server;
    Atom	 atom;
    int		*status;
{
    Xv_opaque data;

    if (XFindContext(server->xdisplay, server->atom_mgr[DATA], (XContext)atom,
		     (caddr_t *)&data) == XCNOENT)
	*status = XV_ERROR;
    else
	*status = XV_OK;

    return(data);
}
