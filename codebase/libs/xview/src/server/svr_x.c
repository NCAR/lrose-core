#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)svr_x.c 20.57 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE 
 *	file for terms of the license.
 */

#include <stdio.h>
#include <xview/pkg.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/time.h>
#ifdef _XV_DEBUG
#include <xview_private/xv_debug.h>
#endif /* _XV_DEBUG */
#include <xview_private/i18n_impl.h>
#include <xview/win_event.h>
#include <X11/Xlib.h>
#include <xview/defaults.h>
#include <xview/sel_svc.h>
#include <xview/server.h>
#include <xview_private/svr_impl.h>
#include <X11/keysym.h>

extern Display *XOpenDisplay();
#ifndef __linux
Xv_private_data Defaults_pairs xv_kbd_cmds_value_pairs[4];
#else
/* Global already defined and initialized in in server/server.c */
extern Defaults_pairs xv_kbd_cmds_value_pairs[4];
#endif

/*
 * The following table describes the default key mappings for special 
 * XView keys. The first default key mapping is attempted. If this 
 * fails, then the machine we're on probably doesn't
 * have sufficient function keys, so we try the second mapping, and
 * so on. Right now, the rest of XView only supports the default mapping
 * so we set NUM_KEYSYM_SETS to 1 and only supply one set of keys to use.
 * In the future, this will go away and we'll provide a more elegant
 * and flexible way for the user to map function keys to XView functions.
 */

#define NUM_KEYSYM_SETS	1

	/* XXX: XK_F13 is left here to be compatible with V2.  V2 XView
		clients use XK_F13 in the modmap as a trigger that
		tells them the function keys have already been installed.
		In V3, we had changed it such that only F18 and F20 were
		installed.  See Bug 1060242 for more details.
	 */
static KeySym	default_fkey_keysyms[NUM_KEYSYM_SETS][SELN_FN_NUM] = {
		{			/* default keysyms */
			XK_F13,
			XK_F18,
			XK_F20
		}
};

#define MAX_RETRIES	10	/* max number of mapping retries */

static int
my_sync(display)
    Display        *display;
{
    XSync(display, 0);
}

Pkg_private     Xv_opaque
server_init_x(server_name)
    char           *server_name;
{
    register Display *display;

    if (!(display = XOpenDisplay(server_name)))
    	return ((Xv_opaque) NULL);

    if (defaults_get_boolean("window.synchronous", "Window.Synchronous", FALSE)
		 			        && !XSynchronize(display, TRUE))
	(void) XSetAfterFunction(display, my_sync);

    return ((Xv_opaque) display);
}

/*
 * keycode_in_map(map, keycode)
 *
 * returns the associated modifier index if the specified keycode is in the 
 * given modifier map. Otherwise, returns -1.
 */

static int
keycode_in_map(map, keycode)
	XModifierKeymap *map;
	KeyCode keycode;
{
	register int i, max;

	if (!keycode) return(0);

	max = 8 * map->max_keypermod;
	for (i = 0; i < max; i++) {
		if (map->modifiermap[i] == keycode) {
			return (i / map->max_keypermod);
		}
	}
	return -1;
}

static int
find_free_row(map)
	XModifierKeymap *map;
{
	int row, offset, base;

	/*
	 * Find the first unused row in the modifier map.
	 * An unused row will have all zeros in it.
	 */
	for (row = Mod1MapIndex; row <= Mod5MapIndex; row++) {
		base = row * map->max_keypermod;
		for (offset = 0; (offset < map->max_keypermod)  &&
			(map->modifiermap[base + offset] == 0); 
			offset++);
		if (offset == map->max_keypermod) {
			return(row);
		}
	}
	return(-1);
}

/*
 * server_refresh_modifiers(server, update_map)
 *
 * 1) Designates the meta keys as a modifier keys.
 * 2) Inserts all the keys in the array default_fkey_keysyms[] into
 * 	the server's modifier map (all under the same modifier; any
 *	of the modifiers Mod2-Mod5 may be used). This function then
 *	sets server->sel_modmask to be the appropriate mask for whatever
 *      modifier the keys were designated as.
 * 3) If update_map is false, do not try to insert new mappings into the
 *    modifier map.  Get the current mapping and update our internal
 *    understanding only.  update_map is false when a user runs xmodmap
 *    and changes the modifier map.  We don't want to override what the
 *    user just changed, so we try to live with it.
 */
Xv_private void
server_refresh_modifiers(server_public, update_map)
	Xv_opaque	 server_public;
	Bool		 update_map;   /* Update the server map */
{
	Server_info	*server = SERVER_PRIVATE(server_public);
	Display	*display = (Display *)server->xdisplay;
	XModifierKeymap *map;
	int             i, modifier, func_modifier, updated = False;
	int		keysym_set, result, retry_count;

	for (keysym_set = 0; keysym_set < NUM_KEYSYM_SETS; keysym_set++) {
		if (!(map = XGetModifierMapping(display))) {
			return;
		}

		/* See if META is already installed. */
		if ((modifier = keycode_in_map(map,
		     XKeysymToKeycode(display, XK_Meta_L))) == -1) {
		    /* Find a free row for META */
		    if (update_map && (modifier = find_free_row(map)) != -1) {
			updated = True;
			/* Insert the meta keys as modifiers. */
			map = XInsertModifiermapEntry(map,
			    XKeysymToKeycode(display, (KeySym) XK_Meta_L),
			    modifier);
			map = XInsertModifiermapEntry(map,
			    XKeysymToKeycode(display, (KeySym) XK_Meta_R),
			    modifier);
			}
		}
		if (modifier == -1 || modifier == 0)
		    server->meta_modmask = 0;
		else
		    server->meta_modmask = 1<<modifier;

		/* See if NUM LOCK is already installed. */
		if ((modifier = keycode_in_map(map,
		     XKeysymToKeycode(display, XK_Num_Lock))) == -1) {
		    /* Find a free row for NUM LOCK */
		    if (update_map && (modifier = find_free_row(map)) != -1) {
			updated = True;
			/* Insert the meta keys as modifiers. */
			map = XInsertModifiermapEntry(map,
			    XKeysymToKeycode(display, (KeySym) XK_Num_Lock),
			    modifier);
			}
		}
		if (modifier == -1 || modifier == 0)
		    server->num_lock_modmask = 0;
		else
		    server->num_lock_modmask = 1<<modifier;

		if (defaults_get_enum("openWindows.keyboardCommands",
				      "OpenWindows.KeyboardCommands",
				      xv_kbd_cmds_value_pairs) >=
		    KBD_CMDS_BASIC) {
		    /* See if ALT is already installed. */
		    if ((modifier = keycode_in_map(map,
			 XKeysymToKeycode(display, XK_Alt_L))) == -1) {
			/* Find a free row for ALT */
			if (update_map && (modifier = find_free_row(map))!=-1) {
			    updated = True;
			    /* Insert the alt keys as modifiers. */
			    map = XInsertModifiermapEntry(map,
				XKeysymToKeycode(display, (KeySym) XK_Alt_L),
				modifier);
			    map = XInsertModifiermapEntry(map,
				XKeysymToKeycode(display, (KeySym) XK_Alt_R),
				modifier);
			}
		    }
		    if (modifier == -1 || modifier == 0)
			server->alt_modmask = 0;
		    else
			server->alt_modmask = 1<<modifier;
		}

					       /* See if function keys in map */
		if (((func_modifier = keycode_in_map(map,
			       XKeysymToKeycode(display,
			       default_fkey_keysyms[keysym_set][0]))) == -1) ||
		    ((func_modifier = keycode_in_map(map,
			       XKeysymToKeycode(display,
			       default_fkey_keysyms[keysym_set][1]))) == -1)) {
					                  /* Find a free row. */
			if (update_map &&
				   (func_modifier = find_free_row(map)) != -1) {
			    for (i = 0; i < SELN_FN_NUM; i++) {
				updated = True;
				map = XInsertModifiermapEntry(map,
					XKeysymToKeycode(display,
					   default_fkey_keysyms[keysym_set][i]),
								 func_modifier);
			    }
			    server->sel_modmask = 1 << func_modifier;
			}
		} else
  			server->sel_modmask = 1 << func_modifier;

		if (func_modifier == -1 || func_modifier == 0)/* no free rows */
			server->sel_modmask = 0;

		/*
		 * Attempt to install the modifier mapping.
		 * If successful, exit this function. If not, try another 
		 * set of keysyms.
		 */
		if (updated) {
		    for (retry_count = 0;
			((result = XSetModifierMapping(display, map)) 
				== MappingBusy && retry_count < MAX_RETRIES);
			retry_count++) {
				sleep(1);/* if busy, wait 1 sec and retry */
		    }
  	  	    if (result == Success) {
    			XFreeModifiermap(map);
			return;
		    }
		} else {
    		    XFreeModifiermap(map);
		    return;
		}
	}

	/* all our attempts failed */
	xv_error(XV_ZERO,
		 ERROR_STRING, 
		    XV_MSG("Problems setting default modifier mapping"),
		 ERROR_PKG, SERVER,
		 NULL);

    	XFreeModifiermap(map);
}


Xv_private void
server_set_seln_function_pending(server_public, flag)
    Xv_Server       server_public;
    int             flag;
{
    Server_info    *server = SERVER_PRIVATE(server_public);
    server->seln_function_pending = flag ? TRUE : FALSE;
}

Xv_private int
server_get_seln_function_pending(server_public)
    Xv_Server       server_public;
{
    return (SERVER_PRIVATE(server_public)->seln_function_pending);
}
