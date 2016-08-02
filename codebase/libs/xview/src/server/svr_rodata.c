/*      @(#)svr_rodata.c  1.7  93/06/28  SMI   */

/*
 *      (c) Copyright 1990 Sun Microsystems, Inc. Sun design patents
 *      pending in the U.S. and foreign countries. See LEGAL_NOTICE
 *      file for terms of the license.
 */

/* This file is compiled with the -R flag.
 * So all the data in this file is moved to the text segment
 * of the shared library. This data is strictly read-only.
 */

#ifndef svr_keymap_data_DEFINED
#define svr_keymap_data_DEFINED

#include <X11/keysym.h>
#include <xview/win_input.h>
#include <xview_private/portable.h>



/***************************************************************************
 *
 * Keysym -> Event id (ie_code) translation
 *
 ***************************************************************************/
const unsigned int win_keymap[] = {
	0, 0, 0, 0, 0, 0, 0, 0,
/*
 * TTY Functions, cleverly chosen to map to ascii, for convenience of
 * programming, but could have been arbitrary (at the cost of lookup
 * tables in client code.
 */

	XK_BackSpace,
	XK_Tab,
	XK_Linefeed,
	XK_Clear,
	0,
	XK_Return,

	0, 0,

	/* BUG: On X11/NeWS, Keysym F36 and F37 happen to fall into a couple
		of holes in the win_keymap table.  We will use them for
		now, but this needs to be fixed before MIT decides to put
		real keysyms here.
	*/

	KEY_TOP(11),					/* XK_SunF36 */
	KEY_TOP(12),					/* XK_SunF37 */

	0,

	XK_Pause,
#ifndef XK_Scroll_Lock
        0,
#else    
        /* BUG: Only in R4. */
        XK_Scroll_Lock,                 		/* XK_Scroll_Lock */
#endif /* XK_Scroll_Lock */
	0, 0, 0, 0, 0, 0,

	XK_Escape,

	0, 0, 0, 0,

	/* International & multi-key character composition */

	XK_Multi_key,
	XK_Kanji,

	0, 0, 0,                         0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	/* Cursor control & motion */

	XK_Home,
	KEY_RIGHT(10),					/* XK_Left  */
	KEY_RIGHT(8),					/* XK_Up    */
	KEY_RIGHT(12),					/* XK_Right */
	KEY_RIGHT(14),					/* XK_Down  */
	XK_Prior,
	XK_Next,
	XK_End,
	XK_Begin,

	0, 0, 0, 0, 0, 0, 0,

	/* Misc Functions */
 
	XK_Select,
	XK_Print,
	XK_Execute,
	XK_Insert,
	0,
	XK_Undo,
	XK_Redo,
	XK_Menu,
	XK_Find,
	XK_Cancel,
	XK_Help,
	SHIFT_BREAK,					/* XK_Break */
	
	0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0,

	SHIFT_ALTG,					/* XK_script_switch */
	SHIFT_NUMLOCK,					/* XK_Num_Lock      */

 	/* Keypad Functions, keypad numbers cleverly chosen to map to ascii */

	XK_KP_Space,

	0, 0, 0, 0, 0, 0, 0, 0,

	XK_KP_Tab,

	0, 0, 0,

	XK_KP_Enter,

	0, 0, 0,

	XK_KP_F1,
	XK_KP_F2,
	XK_KP_F3,
	XK_KP_F4,

	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0,

	XK_KP_Multiply,
	XK_KP_Add,
	XK_KP_Separator,
	XK_KP_Subtract,
	XK_KP_Decimal,
	XK_KP_Divide,
	XK_KP_0,
	XK_KP_1,
	XK_KP_2,
	XK_KP_3,
	XK_KP_4,
	XK_KP_5,
	XK_KP_6,
	XK_KP_7,
	XK_KP_8,
	XK_KP_9,

	0, 0, 0,

	XK_KP_Equal,

  /*
   * Auxilliary Functions; note the duplicate definitions for left and right
   * function keys;  Sun keyboards and a few other manufactures have such
   * function key groups on the left and/or right sides of the keyboard.
   * We've not found a keyboard with more than 35 function keys total.
   */
	KEY_TOP(1),					/* XK_F1  */
	KEY_TOP(2),		        		/* XK_F2  */
	KEY_TOP(3),					/* XK_F3  */
	KEY_TOP(4),					/* XK_F4  */
	KEY_TOP(5),					/* XK_F5  */
	KEY_TOP(6),					/* XK_F6  */
	KEY_TOP(7),					/* XK_F7  */
	KEY_TOP(8),					/* XK_F8  */
	KEY_TOP(9),					/* XK_F9  */
	KEY_TOP(10),					/* XK_F10 */
	KEY_LEFT(1),					/* XK_L1  */
	KEY_LEFT(2),					/* XK_L2  */
	KEY_LEFT(3),					/* XK_L3  */
	KEY_LEFT(4),					/* XK_L4  */
	KEY_LEFT(5),					/* XK_L5  */
	KEY_LEFT(6),					/* XK_L6  */
	KEY_LEFT(7),					/* XK_L7  */
	KEY_LEFT(8),					/* XK_L8  */
	KEY_LEFT(9),					/* XK_L9  */
	KEY_LEFT(10),					/* XK_L10 */
	KEY_RIGHT(1),					/* XK_R1  */
	KEY_RIGHT(2),					/* XK_R2  */
	KEY_RIGHT(3),					/* XK_R3  */
	KEY_RIGHT(4),					/* XK_R4  */
	KEY_RIGHT(5),					/* XK_R5  */
	KEY_RIGHT(6),					/* XK_R6  */
	KEY_RIGHT(7),					/* XK_R7  */
	KEY_RIGHT(8),					/* XK_R8  */
	KEY_RIGHT(9),					/* XK_R9  */
	KEY_RIGHT(10),					/* XK_R10 */
	KEY_RIGHT(11),					/* XK_R11 */
	KEY_RIGHT(12),					/* XK_R12 */
	KEY_RIGHT(13),					/* XK_R13 */
	KEY_RIGHT(14),					/* XK_R14 */
	KEY_RIGHT(15),					/* XK_R15 */

	/* Modifiers */
 
	SHIFT_LEFT,					/* XK_Shift_L    */
	SHIFT_RIGHT,					/* XK_Shift_R    */
	SHIFT_CTRL,					/* XK_Control_L  */
	SHIFT_CTRL,					/* XK_Control_R  */
	SHIFT_CAPSLOCK,					/* XK_Caps_Lock  */
	SHIFT_LOCK,					/* XK_Shift_Lock */
	SHIFT_META,					/* XK_Meta_L     */
	SHIFT_META,					/* XK_Meta_R     */
	SHIFT_ALT,					/* XK_Alt_L	 */
	SHIFT_ALTG,					/* XK_Alt_R	 */
	XK_Super_L,
	XK_Super_R,
	XK_Hyper_L,
	XK_Hyper_R,

	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0,

	XK_Delete
};

#endif /* ~svr_keymap_data_DEFINED */

