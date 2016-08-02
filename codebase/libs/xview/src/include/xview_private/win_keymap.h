/*	@(#)win_keymap.h 20.12 93/06/28 SMI	*/

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * SunView related input definitions.
 */
#ifndef	win_keymap_DEFINED
#define	win_keymap_DEFINED

/* 
 * This is for building the xv_map and sem_map table,
 * and look up the convert from X keycode to SunView event.
 */
#define		MAX_KEYCODE_TABLE		2048

#define MAX_MASK			8
#define No_Index_Mask			((unsigned short)0)
#define Shift_Index_Mask		((unsigned short)(1<<8))
#define Control_Index_Mask		((unsigned short)(1<<9))
#define Meta_Index_Mask			((unsigned short)(1<<10))
#define Shift_and_Control_Mask		(Shift_Index_Mask | Control_Index_Mask)
#define Shift_and_Meta_Mask		(Shift_Index_Mask | Meta_Index_Mask)
#define Control_and_Meta_Mask		(Control_Index_Mask | Meta_Index_Mask)
#define All_Index_Masks		(Shift_Index_Mask | Control_Index_Mask | Meta_Index_Mask)


extern unsigned short	state_to_index_mask[];

#define KEYCODE_TO_KEY_INDEX(_keycode, _index_mask) \
	((unsigned short)((_index_mask ) |  ((unsigned short)(_keycode))))

#define KEY_INDEX_TO_MASKS(_index) \
	((unsigned short)((_index) &  (All_Index_Masks)))
	
#define KEY_INDEX_TO_KEYCODE(_index) \
	((unsigned short)((_index) &  ~(All_Index_Masks)))

#define KEYCODE_STATE_TO_KEY_INDEX(_keycode, _state) \
	KEYCODE_TO_KEY_INDEX(_keycode, state_to_index_mask[(_state & 0xf)])	


typedef enum {
    KEYMAP_FUNCT_KEYS,
    KEYMAP_EDIT_KEYS,
    KEYMAP_MOTION_KEYS,
    KEYMAP_TEXT_KEYS
} Event_class;



#endif /*	win_keymap_DEFINED */
