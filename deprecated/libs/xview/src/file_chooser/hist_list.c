#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)hist_list.c 1.10 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE
 *	file for terms of the license.
 */


#include <stdio.h>
#include <xview/xview.h>
#include <xview/panel.h>
#include <xview/server.h>
#include <xview_private/hist_impl.h>
#include <xview_private/i18n_impl.h>


static void	populate_menu();
static void	add_fixed_entry();
static void	add_rolling_entry();
static void	remove_last_entry();
static int	is_duplicate_entry();


/*
 * Used with xv_find().
 */
static struct history_list *	global_list = (struct history_list *)NULL;



/*
 * xv_create() method
 */
Pkg_private int
hist_list_init( owner, public, avlist )
     Xv_opaque owner;
     History_list_public *public;
     Attr_avlist avlist;
{
    History_list_private *private = xv_alloc( History_list_private );

    public->private_data = (Xv_opaque) private;
    private->public_self = (Xv_opaque) public;

    /*
     * History Lists are parented to Server objects.  In reality,
     * they have no need of the Server (Menu_item's without Server
     * _images are not server-specific); but in some cases (e.g.
     * the File Chooser Go To Menu), it makes sense to be server
     * specific because each server probably indicates a different
     * user.
     *
     * In reality this is only used by xv_find.  The client may get
     * around this by parenting all History_list's to XV_NULL and
     * associating them with History_menu's on different Servers.
     * though anyone so daring will be in for big trouble if this
     * object ever gets augmented to use glyphs...
     */
    private->server = (owner) ? owner : xv_default_server;

    private->roll_max = DEFAULT_ROLL_MAX;
    private->duplicate_labels = TRUE;
    private->duplicate_values = TRUE;

    /*
     * for xv_find, keep a global linked list of the
     * history_list structures.
     */
    private->next = global_list;
    global_list = private;

    return XV_OK;
} /* hist_list_init() */




/*
 * xv_set() method
 */
Pkg_private Xv_opaque
hist_list_set( public, avlist )
     History_list public;
     Attr_avlist avlist;
{
    History_list_private *private = HIST_LIST_PRIVATE(public);
    Attr_avlist attrs;

    for (attrs=avlist; *attrs; attrs=attr_next(attrs)) {
	switch ( (int) attrs[0] ) {

#ifdef OW_I18N
	case HISTORY_ADD_ROLLING_ENTRY_WCS: 
#endif	    
	case HISTORY_ADD_ROLLING_ENTRY: {
	    char *label = (char *) attrs[1];
	    char *value = (char *) attrs[2];

#ifdef OW_I18N
	    if ( (Attr_attribute)attrs[0] == HISTORY_ADD_ROLLING_ENTRY_WCS ) {
		label = _xv_wcstombsdup( (wchar_t *)attrs[1] );
		value = _xv_wcstombsdup( (wchar_t *)attrs[2] );
	    }
#endif  /* OW_I18N */	    

	    ATTR_CONSUME(attrs[0]);

	    if ( private->roll_max == 0 )
		break;

	    /* check for dupes */
	    if ( (!private->duplicate_labels && is_duplicate_entry(private, label))
		|| (!private->duplicate_values && is_duplicate_entry(private, value))
		)
		/*
		 * since the client may not really know if this string is
		 * already in the list, we cannot reasonably say it is an
		 * error.  hence, don't beep or xv_error -- just punt.
		 */
		break;

	    add_rolling_entry(private, label, value);
#ifdef OW_I18N
	    if ( (Attr_attribute)attrs[0] == HISTORY_ADD_ROLLING_ENTRY_WCS ) {
		xv_free_ref( label );
		xv_free_ref( value );
	    }
#endif /* OW_I18N */

	    if ( private->roll_count < private->roll_max )
		private->roll_count++;
	    else
		remove_last_entry( &private->roll_last );
	    break;
	}


#ifdef OW_I18N
	case HISTORY_ADD_FIXED_ENTRY_WCS:
#endif	    
	case HISTORY_ADD_FIXED_ENTRY: {
	    char *label = (char *) attrs[1];
	    char *value = (char *) attrs[2];

#ifdef OW_I18N
	    if ( (Attr_attribute)attrs[0] == HISTORY_ADD_FIXED_ENTRY_WCS ) {
		label = _xv_wcstombsdup( (wchar_t *)attrs[1] );
		value = _xv_wcstombsdup( (wchar_t *)attrs[2] );
	    }
#endif  /* OW_I18N */	    

	    add_fixed_entry(private, label, value);
#ifdef OW_I18N
	    if ( (Attr_attribute)attrs[0] == HISTORY_ADD_FIXED_ENTRY_WCS ) {
		xv_free_ref( label );
		xv_free_ref( value );
	    }
#endif /* OW_I18N */

	    private->fixed_count++;
	    ATTR_CONSUME(attrs[0]);
	    break;
	}

	case HISTORY_ROLLING_MAXIMUM:
	    ATTR_CONSUME(attrs[0]);
	    if ( (int) attrs[1] < private->roll_count ) {
		int ii;

		/* remove the extra entries */
		for (ii=0; ii< (int) attrs[1]; ++ii)
		    remove_last_entry( &private->roll_last );
	    }
	    private->roll_max = (int) attrs[1];
	    break;

	case HISTORY_DUPLICATE_LABELS:
	    ATTR_CONSUME(attrs[0]);
	    private->duplicate_labels = (int) attrs[1];
	    break;

	case HISTORY_DUPLICATE_VALUES:
	    ATTR_CONSUME(attrs[0]);
	    private->duplicate_values = (int) attrs[1];
	    break;

	case HISTORY_INACTIVE: {
	    struct hist_entry *entry;
	    History_space space = (History_space) attrs[1];
	    int row = (int) attrs[2];
	    int val = (int) attrs[3];
	    int ii;
	    
	    if ( space == HISTORY_FIXED ) {
		entry = private->fixed_first;
		if ( row >= private->fixed_count )
		    break;
	    } else {
		entry = private->roll_first;
		if ( row >= private->roll_count )
		    break;
	    }
	    
	    /* find the row */
	    for ( ii=0; ii<row; ++ii )
		entry = entry->next;

	    xv_set(entry->mi, MENU_INACTIVE, val, NULL);

	    ATTR_CONSUME(attrs[0]);
	    break;
	}

	case HISTORY_POPULATE_MENU:		/* private attr */
	    ATTR_CONSUME(attrs[0]);
	    populate_menu( private, (Menu) attrs[1] );
	    break;

	case HISTORY_LABEL:
	case HISTORY_VALUE:
	case HISTORY_VALUE_FROM_MENUITEM: 	/* private attr */
	    xv_error( public,
		     ERROR_PKG,		HISTORY_LIST,
		     ERROR_CANNOT_SET,	attrs[0],
		     NULL );
	    break;

	case XV_END_CREATE:
	    break;

	default:
	    xv_check_bad_attr(HISTORY_LIST, attrs[0]);
	    break;
	} /* switch() */
    } /* for() */

    return XV_OK;
} /* hist_list_set() */




/*
 * xv_get() method
 */
Pkg_private Xv_opaque
hist_list_get( public, status, attr, args )
     History_list_public   *public;
     int             *status;
     Attr_attribute  attr;
     va_list         args;
{
    History_list_private *private = HIST_LIST_PRIVATE(public);

    switch ( (int) attr ) {
    case HISTORY_ROLLING_MAXIMUM:
	return (Xv_opaque) private->roll_max;

    case HISTORY_DUPLICATE_LABELS:
	return (Xv_opaque) private->duplicate_labels;

    case HISTORY_DUPLICATE_VALUES:
	return (Xv_opaque) private->duplicate_values;

    case HISTORY_FIXED_COUNT:
	return (Xv_opaque) private->fixed_count;

    case HISTORY_ROLLING_COUNT:
	return (Xv_opaque) private->roll_count;

    case HISTORY_INACTIVE:
    case HISTORY_LABEL:
    case HISTORY_VALUE: {
	struct hist_entry *entry;
	History_space space = va_arg(args, int);
	int row = va_arg(args, int);
	int ii;

	if ( space == HISTORY_FIXED ) {
	    entry = private->fixed_first;
	    if ( row >= private->fixed_count )
		return (attr == HISTORY_INACTIVE) ? -1 : XV_NULL;
	} else {
	    entry = private->roll_first;
	    if ( row >= private->roll_count )
		return (attr == HISTORY_INACTIVE) ? -1 : XV_NULL;
	}

	/* find the row */
	for ( ii=0; ii<row; ++ii )
	    entry = entry->next;

	if ( attr == HISTORY_LABEL )
	    return (Xv_opaque) ((entry->label) ? entry->label : "");
	else if ( attr == HISTORY_VALUE )
	    return (Xv_opaque) ((entry->value) ? entry->value : "");
	else
	    return xv_get(entry->mi, MENU_INACTIVE);
	break;
    }

#ifdef OW_I18N
    case HISTORY_LABEL_WCS:
    case HISTORY_VALUE_WCS: {
	struct hist_entry *entry;
	History_space space = va_arg(args, int);
	int row = va_arg(args, int);
	int ii;

	if ( space == HISTORY_FIXED ) {
	    entry = private->fixed_first;
	    if ( row >= private->fixed_count )
		return XV_NULL;
	} else {
	    entry = private->roll_first;
	    if ( row >= private->roll_count )
		return XV_NULL;
	}

	/* find the row */
	for ( ii=0; ii<row; ++ii )
	    entry = entry->next;

	if ( attr == HISTORY_LABEL_WCS ) {
	    xv_free_ref( entry->label_wcs );

	    if ( entry->label )
		entry->label_wcs = _xv_mbstowcsdup( entry->label );
	    else
		entry->label_wcs = _xv_mbstowcsdup( "" );
	    return (Xv_opaque) entry->label_wcs;
	} else if ( attr == HISTORY_VALUE_WCS ) {
	    xv_free_ref( entry->value_wcs );

	    if ( entry->value )
		entry->value_wcs = _xv_mbstowcsdup( entry->value );
	    else
		entry->value_wcs = _xv_mbstowcsdup( "" );
	    return (Xv_opaque) entry->value_wcs;
	}
	break;
    }
#endif	/* OW_I18N */


    case HISTORY_VALUE_FROM_MENUITEM:  { 	/* private attr */
	struct hist_entry *entry;
	Menu_item mi = va_arg(args, Menu_item);

	entry = private->fixed_first;
	while ( entry ) {
	    if ( entry->mi == mi )
		return (Xv_opaque) entry->value;
	    entry = entry->next;
	}

	entry = private->roll_first;
	while ( entry ) {
	    if ( entry->mi == mi )
		return (Xv_opaque) entry->value;
	    entry = entry->next;
	}
	return XV_NULL;
    }

    default :
	*status = xv_check_bad_attr(HISTORY_LIST, attr);
	return (Xv_opaque)XV_OK;
    } /* switch */

} /* hist_list_get() */




/*
 * Don't act so surprised!  Yes, this is an xv_find method...
 */
Pkg_private Xv_object
hist_list_find( parent, pkg, avlist )
     Xv_opaque parent;
     Xv_pkg *pkg;
     Attr_avlist avlist;
{
    Attr_avlist attrs;
    struct history_list *node = global_list; /* global list */
    Xv_server server;
    

    server = (parent) ? parent : xv_default_server;

    for (attrs = avlist; *attrs; attrs = attr_next(attrs)) {
	switch ((int)attrs[0]) {
	case XV_NAME:
	    while ( node ) {
		/* BUG:  should cache name? */
		if ( strequal( (char *)xv_get(HIST_LIST_PUBLIC(node), XV_NAME), 
			      (char *)attrs[1]) 
		    && (node->server == server)
		    )
		    return HIST_LIST_PUBLIC(node);
		node = node->next;
	    }
	    break;

	default:
	    break;
	} /* switch */
    } /* for */

    return XV_NULL;
} /* hist_list_find() */



/*
 * xv_destroy() method
 */
Pkg_private int
hist_list_destroy( public, status )
     History_list_public *public;
     Destroy_status status;
{
    History_list_private *private = HIST_LIST_PRIVATE(public);
    struct hist_entry *	node;
    struct hist_entry *	old_node;
    struct history_list *list_node;


    if (status != DESTROY_CLEANUP)
	return XV_OK;

    
    /* first, remove private from global list */
    list_node = global_list;
    if ( list_node == private ) {
	global_list = list_node->next;
    } else {
	while ( list_node->next != private )
	    list_node = list_node->next;
	list_node->next = list_node->next->next;
    }

    if ( private->blank )
	xv_destroy( private->blank );

    /* destroy fixed space items */
    node = private->fixed_last;
    while ( node )
	remove_last_entry( &node );


    /* destroy rolling space items */
    node = private->roll_last;
    while ( node )
	remove_last_entry( &node );


    xv_free( private );

    return XV_OK;
} /* hist_list_destroy() */


/*******************************************************************************/



/*
 * check for duplicate entry.
 */
static int
is_duplicate_entry( private, string )
     History_list_private *private;
     char *string;
{
    struct hist_entry *ptr;

    ptr = private->fixed_first;
    while( ptr ) {
	if ( !private->duplicate_labels 
	    && ptr->label && strequal(ptr->label, string) 
	    )
	    return TRUE;
	if ( !private->duplicate_values 
	    && ptr->value && strequal(ptr->value, string) 
	    )
	    return TRUE;
	ptr = ptr->next;
    }

    ptr = private->roll_first;
    while( ptr ) {
	if ( !private->duplicate_labels 
	    && ptr->label && strequal(ptr->label, string) 
	    )
	    return TRUE;
	if ( !private->duplicate_values 
	    && ptr->value && strequal(ptr->value, string) 
	    )
	    return TRUE;
	ptr = ptr->next;
    }

    return FALSE;
} /* duplicate_entry() */





/*
 * Used to add entries to Fixed Space.
 * These are placed at the end of the  list
 */
static void
add_fixed_entry( private, label, value )
     History_list_private *private;
     char *label;
     char *value;
{
    struct hist_entry	*new;


    /*
     * Create new fixed entry.  Note, Fixed Space
     * interperates NULL as an empty entry.
     */
    new = xv_calloc(1, sizeof(struct hist_entry));
    if ( label ) {
	new->label = xv_strcpy(NULL, label);
	new->value = xv_strcpy(NULL, value);
	new->mi = xv_create(XV_NULL, MENUITEM,
			    MENU_STRING,	new->label,
			    NULL);
    } else {
	new->mi = xv_create(XV_NULL, MENUITEM_SPACE, NULL);
    }


    /*
     * Add new entry to end of list.
     */
    if ( !private->fixed_last ) {
	private->fixed_last = private->fixed_first = new;
    } else {
	private->fixed_last->next = new;
	new->prev = private->fixed_last;
	private->fixed_last = new;
    }

} /* add_fixed_entry() */




/*
 * Used to add entries to Rolling Space.
 * These are placed at the start of the list
 */
static void
add_rolling_entry( private, label, value )
     History_list_private *private; 
     char *label;
     char *value;
{
    struct hist_entry	*new;


    /*
     * Create new rolling entry
     */
    new = xv_calloc(1, sizeof(struct hist_entry));
    new->label = xv_strcpy(NULL, label);
    new->value = xv_strcpy(NULL, value);
    new->mi = xv_create(XV_NULL, MENUITEM,
			MENU_STRING,	new->label,
			NULL);


    /*
     * As soon as there is something in Rolling Space,
     * create a blank separator item for between Fixed
     * and Rolling space.
     */
    if ( !private->blank )
	private->blank = xv_create(XV_NULL, MENUITEM_SPACE, NULL);


    /*
     * Add new entry to list.  include forward
     * and backward links, and insert into front
     * of list.
     */
    new->next = private->roll_first;
    if ( new->next )
	new->next->prev = new;
    private->roll_first = new;
    if ( private->roll_count == 0 )
	private->roll_last = new;

} /* add_rolling_entry() */





/*
 * Unlink and destroy last entry in
 * a Histroy entry list.
 */
static void
remove_last_entry( last )
     struct hist_entry **last;
{
    struct hist_entry *node;

    /* unlink the node */
    node = *last;
    *last = node->prev;
    if ( *last )
	(* last)->next = (struct hist_entry *)NULL;


    /* now free it up */
    if ( node->label )
	xv_free( node->label );
    if ( node->value )
	xv_free( node->value );

#ifdef OW_I18N
    if ( node->label_wcs )
	xv_free( node->label_wcs );
    if ( node->value_wcs )
	xv_free( node->value_wcs );
#endif

    if ( node->mi )
	xv_destroy( node->mi );
    xv_free( node );
} /* remove_last_entry() */




/*
 * Make menu current with our history list
 */
static void
populate_menu( private, menu )
     History_list_private *private;
     Menu menu;
{
    int ii;
    struct hist_entry *node;


    /* add Fixed Space to menu */
    node = private->fixed_first;
    while ( node ) {
	xv_set(menu, MENU_APPEND_ITEM, node->mi, NULL);
	node = node->next;
    }


    /*
     * Blank separator between Fixed and Rolling Space
     */
    if ( private->blank )
	xv_set(menu, MENU_APPEND_ITEM, private->blank, NULL);


    /* add Rolling Space to menu */
    node = private->roll_first;
    while ( node ) {
	xv_set(menu, MENU_APPEND_ITEM, node->mi, NULL);
	node = node->next;
    }

} /* populate_menu() */
