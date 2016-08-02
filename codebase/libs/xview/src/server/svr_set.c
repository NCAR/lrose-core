#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)svr_set.c 20.56 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE 
 *	file for terms of the license.
 */

#include <sys/types.h>
#include <sys/time.h>
#include <xview/win_event.h>
#include <xview_private/svr_impl.h>

Xv_private Notify_value	xv_input_pending();
Xv_private void 	server_journal_sync_event();
/* ACC_XVIEW */
Xv_private int		server_parse_keystr();
Xv_private void		server_semantic_map_offset();
Xv_private char		*xv_strtok();
/* ACC_XVIEW */
static int server_add_xevent_proc ();
static int server_add_xevent_mask ();


Pkg_private     Xv_opaque
server_set_avlist(server_public, avlist)
    Xv_Server       server_public;
    Attr_attribute  avlist[];
{
    Attr_avlist     attrs;
    Server_info    *server = SERVER_PRIVATE(server_public);
    short	    error = XV_OK;

    for (attrs = avlist; *attrs; attrs = attr_next(attrs)) {
	switch (attrs[0]) {
	  case SERVER_NTH_SCREEN:{
		int             number = (int) attrs[1];
		Xv_Screen       screen = (Xv_Screen) attrs[2];

		if ((number < 0) || (number >= MAX_SCREENS)) {
		    error = (Xv_opaque) attrs[0];
		    break;   /* parse the other attributes */
	        }

		/*
		 * destroy the old screen if overwriting, unless new screen
		 * is null, in which case caller must already have destroyed
		 * old.
		 */
		if (server->screens[number] != screen) {
		    if (screen && server->screens[number])
			(void) xv_destroy(server->screens[number]);
		    server->screens[number] = screen;
		}
	    }
	    break;
	  case SERVER_SYNC:
	    XSync((Display *) server->xdisplay, (int) attrs[1]);
	    break;
	  case SERVER_SYNC_AND_PROCESS_EVENTS:
	    {
		/*
		 * sync with the server to make sure we have all outstanding
		 * events in the queue. Then process the events.
		 */
		Display        *display = (Display *) server->xdisplay;

		XSync(display, 0);
		xv_input_pending(display, 0);/* process pending queued events */
	    }
	    break;
	  case SERVER_JOURNAL_SYNC_ATOM:
	    server->journalling_atom = (Xv_opaque) attrs[1];
	    break;
	  case SERVER_JOURNAL_SYNC_EVENT:
	    if (server->journalling)
		server_journal_sync_event(server_public, (int) attrs[1]);
	    break;
	  case SERVER_JOURNALLING:
	    server->journalling = (int) attrs[1];
	    break;
	  case SERVER_MOUSE_BUTTONS:
	    server->nbuttons = (short) attrs[1];
	    break;
	  case SERVER_BUTTON2_MOD:
	    server->but_two_mod = (unsigned int) attrs[1];
	    break;
	  case SERVER_BUTTON3_MOD:
	    server->but_three_mod = (unsigned int) attrs[1];
	    break;
	  case SERVER_CHORD_MENU:
	    server->chord_menu = (unsigned int) attrs[1];
	    break;
	  case SERVER_CHORDING_TIMEOUT:
	    server->chording_timeout = (int) attrs[1];
	    break;
	  case SERVER_EXTENSION_PROC: 
	    server->extensionProc = (void (*) ()) attrs[1];
	    break;
	  case XV_NAME: 
	    server->display_name = (char *) attrs[1];
	    break;
	  case SERVER_DND_ACK_KEY: 
	    server->dnd_ack_key = (int) attrs[1];
	    break;
	  case SERVER_ATOM_DATA:
	    error = server_set_atom_data(server, (Atom)attrs[1],
					 (Xv_opaque)attrs[2]);
	    break;
	  case SERVER_FOCUS_TIMESTAMP:
            server->focus_timestamp = (unsigned long) attrs[1];
	    break;
	  case SERVER_EXTERNAL_XEVENT_PROC:
	    error = server_add_xevent_proc (server, attrs[1], attrs[2], TRUE);
	    break;
	  case SERVER_PRIVATE_XEVENT_PROC:
	    error = server_add_xevent_proc (server, attrs[1], attrs[2], FALSE);
	    break;
	  case SERVER_EXTERNAL_XEVENT_MASK:
	    error = server_add_xevent_mask (server, attrs[1], attrs[2], 
					    attrs[3], TRUE);
	    break;
	  case SERVER_PRIVATE_XEVENT_MASK:
	    error = server_add_xevent_mask (server, attrs[1], attrs[2], 
					    attrs[3], FALSE);
	    break;
#ifdef OW_I18N
	  case XV_APP_NAME:
            _xv_set_mbs_attr_dup(&server->app_name_string, (char *) attrs[1]);
	    break;
	  case XV_APP_NAME_WCS:
            _xv_set_wcs_attr_dup(&server->app_name_string,
                                   (wchar_t *) attrs[1]);
	    break;
#else
	  case XV_APP_NAME:
	    server->app_name_string = (char *) attrs[1];
	    break;
#endif /*OW_I18N*/

          /* ACC_XVIEW */
	  case SERVER_ADD_ACCELERATOR_MAP:{
	    KeySym		keysym = (KeySym)attrs[1];
	    unsigned int	modifiers = (unsigned int)attrs[2];
	    int			offset;

                /*
                 * If accelerator map not created yet, do it now
                 */
                if (!server->acc_map)  {
                    server->acc_map = 
                        (unsigned char *)xv_calloc(0x1600,sizeof(unsigned char));
                }

		/*
		 * Determine offsets into the semantic mapping tables.
		 */
		server_semantic_map_offset(server_public, modifiers, &offset);

                /*
                 * Increment ref count in accelerator map
                 */
                ++(server->acc_map[(keysym & 0xFF) + offset]);
	    break;
	    }

	  case SERVER_REMOVE_ACCELERATOR_MAP:{
	    KeySym		keysym = (KeySym)attrs[1];
	    unsigned int	modifiers = (unsigned int)attrs[2];
	    int			offset;

	    /*
	     * Break if no accelerator map
	     */
	    if (!server->acc_map)  {
	        break;
	    }

	    /*
	     * Determine offsets into the semantic mapping tables.
	     */
	    server_semantic_map_offset(server_public, modifiers, &offset);

            /*
             * Remove/decrement entry in server accelerator map
             * if it is not zero
             */
            if (server->acc_map && 
                server->acc_map[(keysym & 0xFF) + offset])  {
                --(server->acc_map[(keysym & 0xFF) + offset]);
            }

	    break;
	    }
          /* ACC_XVIEW */

	  default:
	    (void) xv_check_bad_attr(&xv_server_pkg, (Attr_attribute)attrs[0]);
	    break;
	}
    }

    return (Xv_opaque) error;
}


Xv_private void
server_set_timestamp(server_public, ev_time, xtime)
    Xv_Server       server_public;
    struct timeval *ev_time;
    unsigned long   xtime;
{
    Server_info    *server = SERVER_PRIVATE(server_public);

    server->xtime = (Xv_opaque) xtime;

    /* Set the time stamp in the event struct */
    if (ev_time) {
	ev_time->tv_sec = ((unsigned long)xtime)/1000;
	ev_time->tv_usec = (((unsigned long) xtime) % 1000) * 1000;
    }
}

Xv_private      void
server_set_fullscreen(server_public, in_fullscreen)
    Xv_Server       server_public;
    int             in_fullscreen;
{
    Server_info    *server = SERVER_PRIVATE(server_public);
    server->in_fullscreen = in_fullscreen;
}

/* The set of below named functions:
 *
 *        server_*node_from_*,
 * 	  server_add_xevent_*,
 *	  server_do_xevent_callback,
 *       
 * together implement the support X event callback mechanism.
 *
 * Server_info *server->idproclist points to a linked list of entries
 * 	each entry containing an id (=0 for app, =a unique handle otherwise)
 *      and a pointer to a callback function.
 * 
 * Server_info *server->xidlist points a linked list of entries
 * 	each entry containing a window xid, and a pointer (->idmasklist) to a
 *	to linked list of (Server_mask_list *) described next.  
 * 
 * Server_info *server->xidlist->idmasklist  points a linked list of entries
 * 	each entry containing a id (=0 for app, =a unique value otherwise),
 *      an event mask, and a pointer to an item in the first linked list,
 *	namely, server->idproclist.
 *
 * All linked lists are created and used using XV_SL_* macros.  
 * Important note: There is no explicit head for each list.  The first entry
 * in the list, if one exists, is the head.
 *  
 */

#ifdef _XV_DEBUG

static print_struct(s)
Server_info *s;
{
    Server_xid_list  *xid_node;
    Server_mask_list *mask_node;
    Server_proc_list *proc_node;

    printf ("idproclist=%d, xidlist=%x\n", s->idproclist, s->xidlist);

    XV_SL_TYPED_FOR_ALL(s->xidlist, xid_node, Server_xid_list *) {
	printf ("xidnode=%d ", xid_node);
	XV_SL_TYPED_FOR_ALL(xid_node->masklist, mask_node, Server_mask_list *)
	    printf ("masknode=%d ", mask_node);
	printf ("\n");
    }
}

#endif /* _XV_DEBUG */

Pkg_private 	Server_xid_list *
server_xidnode_from_xid (server, xid)
    Server_info    *server;
    Xv_opaque      xid;
{
    Server_xid_list *node = 0;

    XV_SL_TYPED_FOR_ALL(server->xidlist, node, Server_xid_list *)
	{
	    if (node->xid == xid)
		break;
	}

    return node;
}

Pkg_private  Server_proc_list *
server_procnode_from_id (server, pkg_id)
    Server_info    *server;
    Xv_opaque      pkg_id;
{
    Server_proc_list *node = 0;

    XV_SL_TYPED_FOR_ALL(server->idproclist, node, Server_proc_list *)
	{
	    if (node->id == pkg_id)
		break;
	}

    return node;
}

Pkg_private Server_mask_list *
server_masknode_from_xidid (server, xid, pkg_id)
    Server_info		*server;
    Xv_opaque      	xid;
    Xv_opaque         	pkg_id;
{
    Server_xid_list *xid_node = 0;
    Server_mask_list *node = 0;

    if (xid_node = server_xidnode_from_xid(server, xid))
	XV_SL_TYPED_FOR_ALL(xid_node->masklist, node, Server_mask_list *)
	{
	    if (node->id == pkg_id)
		break;
	}

    return node;
}

static 		int
server_add_xevent_proc (server, func, pkg_id, external)
    Server_info    *server;
    Xv_opaque	func;
    int         pkg_id;
    int         external;
{
    int  error = XV_OK;
    Server_proc_list *node = 0;
    Server_xid_list  *xid_node;
    Server_mask_list *mask_node;

    node = (Server_proc_list *)server_procnode_from_id(server, pkg_id);

    if (!node) {
	node = (Server_proc_list *)xv_alloc(Server_proc_list);
	node->id = pkg_id;

	/* add entry at the beginning of the list */
	server->idproclist = (Server_proc_list *)
	    XV_SL_ADD_AFTER(server->idproclist, XV_SL_NULL, node);

	/*if mask node exists, link them back to this node */
    
	XV_SL_TYPED_FOR_ALL(server->xidlist, xid_node, Server_xid_list *) {
	    XV_SL_TYPED_FOR_ALL(xid_node->masklist, mask_node, Server_mask_list *) {
		if (mask_node->id == pkg_id)
		    mask_node->proc = node;
	    }
	}
    }
    if (external)
	node->extXeventProc = (void (*) ())func;
    else
	node->pvtXeventProc = (void (*) ())func;

#ifdef _XV_DEBUG
    p(server);
#endif /* _XV_DEBUG */

    return error;
}

static
server_add_xevent_mask (server, xid, mask, pkg_id, external)
    Server_info    *server;
    Xv_opaque	xid;
    Xv_opaque	mask;
    int         pkg_id;
    int         external; /* called by app or xview pkg */
{
    int  error = XV_OK;
    Server_xid_list *xid_node = 0;
    Server_mask_list *mask_node = 0, *link;

    if (xid_node = server_xidnode_from_xid(server, xid))
	mask_node = server_masknode_from_xidid (server, xid, pkg_id);

    if (!mask) {  /* mask is null.  remove node is necessary */
	
	if (!mask_node)
	    return error;

	    if (external)
		mask_node->extmask = mask;
	    else
		mask_node->pvtmask = mask;

	if (!(mask_node->pvtmask|mask_node->extmask)) {
	    /* both masks are null, remove the node */

	    if (xid_node->masklist == mask_node)   	/* first node */
		xid_node->masklist = (Server_mask_list *)mask_node->next;
	    else
		XV_SL_REMOVE(xid_node->masklist, mask_node);

	    xv_free(mask_node);
	}

	/* compute new mask */
	XV_SL_TYPED_FOR_ALL(xid_node->masklist, link, Server_mask_list *)
	    {
		mask |= link->extmask|link->pvtmask;
	    }
	
	/* send new mask to server */
	XSelectInput ((Display *) server->xdisplay, xid, mask);

	/* get rid of xid node */

	if (!xid_node->masklist) {

	    if (server->xidlist == xid_node)  	/* first node */
		server->xidlist = (Server_xid_list *)xid_node->next;
	    else
		XV_SL_REMOVE(server->xidlist, xid_node);

	    xv_free(xid_node);
	}

    } else {

	if (!xid_node) { 	/* create an xid entry */
	    xid_node = (Server_xid_list *)xv_alloc(Server_xid_list);
	    xid_node->xid = xid;
	    server->xidlist = (Server_xid_list *)
		XV_SL_ADD_AFTER(server->xidlist, XV_SL_NULL, xid_node);
	}

	if (!mask_node) {	/* create a mask entry */
	    mask_node = (Server_mask_list *)xv_alloc(Server_mask_list);
	    mask_node->id = pkg_id;
	    mask_node->proc = server_procnode_from_id(server, pkg_id);
	    xid_node->masklist = (Server_mask_list *)
		XV_SL_ADD_AFTER(xid_node->masklist, XV_SL_NULL, mask_node);
	}

	if ((external & (mask_node->extmask != mask)) |
	    (!external & (mask_node->pvtmask != mask))) {

	    if (external)
		mask_node->extmask = mask;
	    else
		mask_node->pvtmask = mask;

	    /* compute new mask */
	    XV_SL_TYPED_FOR_ALL(xid_node->masklist, link, Server_mask_list *)
		{
		    mask |= link->extmask|link->pvtmask;
		}
	
	    /* send new mask to server */
	    XSelectInput ((Display *) server->xdisplay, xid, mask);
	}
    }

#ifdef _XV_DEBUG
    p(server);
#endif /* _XV_DEBUG */

    return error;
}

Xv_private 	void
server_do_xevent_callback (server, display, xevent)
    Server_info *server;
    Display   	*display;
    XEvent	*xevent;
{
    XAnyEvent      *any = (XAnyEvent *) xevent;
    Server_xid_list  *xid_node;
    Server_mask_list *mask_node;
    Server_proc_list *proc_node;
    
    /* get the server object */

#ifdef _XV_DEBUG
    printf("callback:\n");
    p(server);
#endif /* _XV_DEBUG */

    XV_SL_TYPED_FOR_ALL(server->xidlist, xid_node, Server_xid_list *) {
	
	if (xid_node->xid == any->window) {

	    XV_SL_TYPED_FOR_ALL(xid_node->masklist, mask_node, Server_mask_list *) {
		    proc_node = mask_node->proc;
		    if (proc_node && proc_node->extXeventProc)
			proc_node->extXeventProc (SERVER_PUBLIC(server), display, xevent, proc_node->id);
		    if (proc_node && proc_node->pvtXeventProc)
			proc_node->pvtXeventProc (SERVER_PUBLIC(server), display, xevent, proc_node->id);
		}
	    break;  /* found a matching xid, get out of the first loop */
	}
    }
}

/* ACC_XVIEW */
Xv_private void
server_semantic_map_offset(server_public, modifiers, offset)
Xv_server	server_public;
unsigned int	modifiers;
int		*offset;
{
    unsigned int	alt_modmask, meta_modmask;

    /*
     * Get Meta, Alt masks
     */
    meta_modmask = (unsigned int)xv_get(server_public, 
                        SERVER_META_MOD_MASK);
    alt_modmask = (unsigned int)xv_get(server_public, 
                        SERVER_ALT_MOD_MASK);

    *offset = 0;

    /*
     * Determine offsets into the semantic mapping tables.
     */
    if (modifiers & ControlMask)
        *offset += 0x100;
    if (modifiers & meta_modmask)
        *offset += 0x200;
    if (modifiers & alt_modmask)
        *offset += 0x400;
    if (modifiers & ShiftMask)
        *offset += 0x800;
}
/* ACC_XVIEW */
