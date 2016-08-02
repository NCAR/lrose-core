#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)om_data.c 20.28 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#include <xview/openmenu.h>

Pkg_private int		menu_create_internal();
Pkg_private int		menu_create_item_internal();
Pkg_private int		menu_destroy_internal();
Pkg_private Xv_opaque	menu_gets();
Pkg_private int		menu_item_destroy_internal();
Pkg_private Xv_opaque	menu_item_gets();
Pkg_private Xv_opaque	menu_item_sets();
Pkg_private Xv_opaque	menu_pkg_find();
Pkg_private Xv_opaque	menu_sets();

Xv_pkg          xv_command_menu_pkg = {
    "Command Menu",		/* seal -> package name */
    (Attr_pkg) ATTR_PKG_MENU,	/* menu attr */
    sizeof(Xv_menu),		/* size of the menu public data structure */
    &xv_generic_pkg,		/* pointer to parent */
    menu_create_internal,	/* init routine */
    menu_sets,
    menu_gets,
    menu_destroy_internal,
    NULL			/* no find proc */
};

Xv_pkg          xv_choice_menu_pkg = {
    "Choice Menu",		/* seal -> package name */
    (Attr_pkg) ATTR_PKG_MENU,	/* menu attr */
    sizeof(Xv_menu),		/* size of the menu public data structure */
    &xv_generic_pkg,		/* pointer to parent */
    menu_create_internal,	/* init routine */
    menu_sets,
    menu_gets,
    menu_destroy_internal,
    NULL			/* no find proc */
};

Xv_pkg          xv_toggle_menu_pkg = {
    "Toggle Menu",		/* seal -> package name */
    (Attr_pkg) ATTR_PKG_MENU,	/* menu attr */
    sizeof(Xv_menu),		/* size of the menu public data structure */
    &xv_generic_pkg,		/* pointer to parent */
    menu_create_internal,	/* init routine */
    menu_sets,
    menu_gets,
    menu_destroy_internal,
    NULL			/* no find proc */
};

Xv_pkg          xv_menu_item_pkg = {
    "Menu_item",
    (Attr_pkg) ATTR_PKG_MENU,	/* menu item shares menu attrs */
    sizeof(Xv_menu_item),	/* size of the item public data structure */
    &xv_generic_pkg,
    menu_create_item_internal,
    menu_item_sets,
    menu_item_gets,
    menu_item_destroy_internal,
    menu_pkg_find
};
