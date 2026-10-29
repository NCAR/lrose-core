#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)hist_menu.c 1.9 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1992, 1993 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE file
 *	for terms of the license.
 */


#include <stdio.h>
#include <xview/xview.h>
#include <xview/panel.h>
#include <xview_private/i18n_impl.h>
#include <xview_private/hist_impl.h>

static Attr_attribute HIST_PRIVATE_KEY = 0;

static void	hist_menu_notify_proc();
static Menu	hist_menu_gen_proc();
static void	hist_menu_done_proc();



/*
 * xv_create() method
 */
Pkg_private int
hist_menu_init( owner, public, avlist )
     Xv_opaque owner;
     History_menu_public *public;
     Attr_avlist avlist;
{
    History_menu_private *private = xv_alloc( History_menu_private );

    if ( !HIST_PRIVATE_KEY )
	HIST_PRIVATE_KEY = xv_unique_key();


    public->private_data = (Xv_opaque) private;
    private->public_self = (Xv_opaque) public;

    /*
     * Initialize le' menu
     */
    private->menu 
	= xv_create( owner, MENU_COMMAND_MENU,
		    MENU_NOTIFY_PROC, 	hist_menu_notify_proc,
		    MENU_GEN_PROC,	hist_menu_gen_proc,
		    MENU_DONE_PROC,	hist_menu_done_proc,
		    MENU_DEFAULT,	1,
		    XV_KEY_DATA,	HIST_PRIVATE_KEY, private,
		    NULL );

    return XV_OK;
} /* hist_menu_init() */




/*
 * xv_set() method
 */
Pkg_private Xv_opaque
hist_menu_set( public, avlist )
     History_menu public;
     Attr_avlist avlist;
{
    History_menu_private *private = HIST_MENU_PRIVATE(public);
    Attr_avlist attrs;

    for (attrs=avlist; *attrs; attrs=attr_next(attrs)) {
	switch ( (int) attrs[0] ) {
	case HISTORY_MENU_OBJECT:
	    xv_error( public,
		     ERROR_CANNOT_SET,	attrs[0],
		     ERROR_PKG,		HISTORY_MENU,
		     NULL );
	    break;

	case HISTORY_NOTIFY_PROC:
	    ATTR_CONSUME(attrs[0]);
	    private->notify_proc = (void (*)()) attrs[1];
	    break;

#ifdef OW_I18N
	case HISTORY_NOTIFY_PROC_WCS:
	    ATTR_CONSUME(attrs[0]);
	    private->notify_proc_wcs = (void (*)()) attrs[1];
	    break;
#endif

	case HISTORY_MENU_HISTORY_LIST: {
	    ATTR_CONSUME(attrs[0]);

	    /* reference counting, yea! */
	    if ( private->list )
		xv_set(private->list, XV_DECREMENT_REF_COUNT, NULL);
	    private->list = (History_list) attrs[1];
	    if ( private->list )
		xv_set(private->list, XV_INCREMENT_REF_COUNT, NULL);
	    break;
	}

	case XV_END_CREATE:
	    break;

	default:
	    xv_check_bad_attr(HISTORY_MENU, attrs[0]);
	    break;
	} /* switch() */
    } /* for() */

    return XV_OK;
} /* hist_menu_set() */



/*
 * xv_get() method
 */
Pkg_private Xv_opaque
hist_menu_get( public, status, attr, args )
     History_menu_public   *public; 
     int             *status;
     Attr_attribute  attr;
     Attr_avlist     args;
{
    History_menu_private *private = HIST_MENU_PRIVATE(public);

    switch ( (int) attr ) {
    case HISTORY_MENU_OBJECT:
	return (Xv_opaque) private->menu;

    case HISTORY_MENU_HISTORY_LIST:
	return (Xv_opaque) private->list;

    case HISTORY_NOTIFY_PROC:
	return (Xv_opaque) private->notify_proc;

#ifdef OW_I18N
    case HISTORY_NOTIFY_PROC_WCS:
	return (Xv_opaque) private->notify_proc_wcs;
#endif

    default :
	*status = xv_check_bad_attr(HISTORY_MENU, attr);
	return (Xv_opaque)XV_OK;
    } /* switch */

} /* hist_menu_get() */



/*
 * xv_destroy() method
 */
Pkg_private int
hist_menu_destroy( public, status )
     History_menu public;
     Destroy_status status;
{
    History_menu_private *private = HIST_MENU_PRIVATE(public);
    Xv_opaque owner;


    if (status != DESTROY_CLEANUP)
	return XV_OK;


    /*
     * Make sure menu is cleared out.  Note that the Panel Button
     * doesn't call the MENU_DONE_PROC if it generated a menu to
     * get the default value, but never showed it.  Is this a bug?
     */
    hist_menu_done_proc( private->menu, NULL );


    if ( private->list ) {
	xv_set(private->list, XV_DECREMENT_REF_COUNT, NULL);
	xv_destroy( private->list );
    }

    if ( private->menu )
	xv_destroy( private->menu );

    xv_free ( private );

    return XV_OK;
} /* hist_menu_destroy() */


/*******************************************************************************/



/*
 * MENU_NOTIFY_PROC for history menu.  notify user of selection.
 */
static void
hist_menu_notify_proc( menu, mi )
     Menu menu;
     Menu_item mi;
{
    char *label = (char *)xv_get(mi, MENU_STRING);
    History_menu_private *private 
	= (History_menu_private *)xv_get(menu, XV_KEY_DATA, HIST_PRIVATE_KEY);
    char *value = (char *)xv_get(private->list, HISTORY_VALUE_FROM_MENUITEM, mi);

#ifdef OW_I18N
    if ( private->notify_proc_wcs ) {
	wchar_t *value_wcs = _xv_mbstowcsdup( value );
	wchar_t *label_wcs = _xv_mbstowcsdup( label );

	(* private->notify_proc)( HIST_MENU_PUBLIC(private), label_wcs, value_wcs );
	xv_free( label_wcs );
	xv_free( value_wcs );
    } else
#endif
    if ( private->notify_proc )
	(* private->notify_proc)( HIST_MENU_PUBLIC(private), label, value );

    xv_set(menu, MENU_NOTIFY_STATUS, XV_ERROR, NULL);
} /* hist_menu_notify_proc() */




/*
 * MENU_DONE_PROC, clean up after display of history menu.
 */
static void
hist_menu_done_proc( menu, result )
     Menu menu;
     Xv_opaque result;
{
    int items = (int) xv_get(menu, MENU_NITEMS);
    int ii;


    /*
     * note:  remove items from menu, but don't destroy
     * them, as they may be shared with another menu.
     * destroying the History_list will do this.
     */
    for(ii=items; ii>0; --ii)
	xv_set(menu, MENU_REMOVE, ii, NULL);
} /* hist_menu_done_proc() */




/*
 * MENU_GEN_PROC for history menu.  menu items are installed
 * and destroyed (see history_menu_done_proc) on the fly so as
 * to implement the OL perscribed behavior of the rolling
 * stack of path names.
 */
static Menu
hist_menu_gen_proc( menu, op )
     Menu menu;
     Menu_generate op;
{
    History_menu_private *private 
	= (History_menu_private *) xv_get(menu, XV_KEY_DATA, HIST_PRIVATE_KEY);
    int ii;


    if ( op != MENU_DISPLAY )
	return menu;

    /*
     * Make sure menu is cleared out.  Note that the Panel Button
     * doesn't call the MENU_DONE_PROC if it generated a menu to
     * get the default value, but never showed it.  Is this a bug?
     */
    hist_menu_done_proc( menu, NULL );

    /* tell History_list to put Menu_items into our Menu */
    if ( private->list )
	xv_set( private->list,
	       HISTORY_POPULATE_MENU, private->menu,
	       NULL);

    return menu;
} /* hist_menu_gen_proc() */
