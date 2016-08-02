#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)win_bldkmp.c 20.26 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#ifdef BUILD_TABLE
#include <stdio.h>
#endif
#include <xview/base.h>
#include <xview/win_input.h>
#include <X11/Xlib.h>
#include <xview_private/win_keymap.h>
#include <X11/keysym.h>



/*******************************************************************************
 *  win_keymap_build.c -- build the keycode to SunView event mapping tables    *
 *******************************************************************************
 */


#define ASCII_UNSHIFTED_MASK	0x20
#define ASCII_CONTROL_MASK	0x40
#define META_MASK		0x80

#define META_A			0x000000e1
#define META_C			0x000000e3
#define META_D			0x000000e4
#define META_E			0x000000e5
#define META_F			0x000000e6
#define META_I			0x000000e9
#define META_L			0x000000ec
#define META_P			0x000000f0
#define META_Q			0x000000f1
#define META_S			0x000000f3
#define META_U			0x000000f5
#define META_V			0x000000f6
#define META_X			0x000000f8

/* Backspace */
#define ASCII_DELETE			0x7f



#define ADD_SELN_KEYCODE(_seln_key_list, _max_seln_keys, _index, _key_index) \
	if ((_index) < (_max_seln_keys))					\
	    _seln_key_list[_index++] = KEY_INDEX_TO_KEYCODE(_key_index)


/*
 * This assumes the following encoding for the X11 mask bits:
 * 
 * #define ShiftMask		(1<<0) #define LockMask		(1<<1)
 * #define ControlMask		(1<<2) #define Mod1Mask		(1<<3)  This
 * is meta
 * 
 * This table maps a key code event state mask to the upper three bits of a
 * key_index.
 * 
 */

#define		MAX_INDEX_MASK		16
unsigned short  state_to_index_mask[] = {
    No_Index_Mask,
    Shift_Index_Mask,
    No_Index_Mask,
    Shift_Index_Mask,
    Control_Index_Mask,
    Control_Index_Mask | Shift_Index_Mask,
    Control_Index_Mask,
    Control_Index_Mask | Shift_Index_Mask,
    Meta_Index_Mask,
    Meta_Index_Mask | Shift_Index_Mask,
    Meta_Index_Mask,
    Meta_Index_Mask | Shift_Index_Mask,
    Meta_Index_Mask | Control_Index_Mask,
    Meta_Index_Mask | Control_Index_Mask | Shift_Index_Mask,
    Meta_Index_Mask | Control_Index_Mask,
    Meta_Index_Mask | Control_Index_Mask | Shift_Index_Mask
};


static void
set_xv_id(xv_table, keycode, xv_id)
    unsigned short  xv_table[];
    register int    keycode;
    register int    xv_id;
{
    register int    i;

    for (i = 0; i < MAX_INDEX_MASK; i++)
	xv_table[KEYCODE_TO_KEY_INDEX(keycode,
				      state_to_index_mask[i])] = xv_id;
}

#ifdef BUILD_TABLE
void
pad_zero(str)
    char           *str;
{
    int             i = 0;
    while (str[i] != '\0') {
	if (str[i] == ' ')
	    str[i] = '0';
	i++;
    }
}

#endif


/*
 * Initialize lookup tables to map keycodes to xview events and ascii codes
 * to keycodes.  The first table xv_map maps all the functions keys to the
 * keycodes (indexed by keycodes). The second table ascii_map maps ascii
 * codes to keycodes. (This is used by win_get_vuid_event
 */

unsigned short *
win_build_xv_map(display, count)
    Display        *display;
    int             count;

{
    register int    keycode;
    unsigned short *xv_map;
    KeySym         *ksym;
    int             keysyms_per_keycode;
    register int    i;

#ifdef BUILD_TABLE
    FILE           *fp, *dfp, *fopen();
    int             entry_count = 0;
    char            str[10];
    fp = fopen("Default_keysym_table", "w");
    dfp = fopen("XV_debug_table", "w");
#endif


    xv_map = (unsigned short *) xv_calloc(MAX_KEYCODE_TABLE, sizeof(unsigned short));

    for (i = 0; i < MAX_KEYCODE_TABLE; i++)
	xv_map[i] = ACTION_NULL_EVENT;


    keysyms_per_keycode = display->keysyms_per_keycode;
    /*
     * BUG:  Should really loop through all keysyms for each keycode
     */
    for (keycode = display->min_keycode;
	 keycode < (display->min_keycode + count);
	 keycode++) {
	/* Looking for the first keysym; there are more than one per keycode */
	ksym = &display->keysyms[(keycode - display->min_keycode)
				 * keysyms_per_keycode];
#ifdef BUILD_TABLE
	sprintf(str, "0x%4x,", *ksym);
	pad_zero(str);
	fprintf(fp, "%7s", str);
	if (((entry_count + 1) % 8) == 0)
	    fprintf(fp, "\n");
	else
	    fprintf(fp, " ");
	entry_count++;
#endif


	if ((*ksym >= XK_L1) && (*ksym <= XK_L10)) {
	    set_xv_id(xv_map, keycode, KEY_LEFT(*ksym - XK_L1 + 1));
	    continue;
	} else if ((*ksym >= XK_R1) && (*ksym <= XK_R15)) {
	    set_xv_id(xv_map, keycode, KEY_RIGHT(*ksym - XK_R1 + 1));
	    continue;
	} else if ((*ksym >= XK_F1) && (*ksym <= XK_F9)) {
	    set_xv_id(xv_map, keycode, KEY_TOP(*ksym - XK_F1 + 1));
	    continue;
	}
	switch (*ksym) {
	  case XK_Up:
	    set_xv_id(xv_map, keycode, KEY_RIGHT(8));
	    break;
	  case XK_Left:
	    set_xv_id(xv_map, keycode, KEY_RIGHT(10));
	    break;
	  case XK_Right:
	    set_xv_id(xv_map, keycode, KEY_RIGHT(12));
	    break;
	  case XK_Down:
	    set_xv_id(xv_map, keycode, KEY_RIGHT(14));
	    break;
	  case XK_Shift_L:	/* Left Shift */
	    set_xv_id(xv_map, keycode, SHIFT_LEFT);
	    break;
	  case XK_Shift_R:	/* Right Shift */
	    set_xv_id(xv_map, keycode, SHIFT_RIGHT);
	    break;
	  case XK_Control_R:	/* Right Control */
	  case XK_Control_L:	/* Left Control */
	    set_xv_id(xv_map, keycode, SHIFT_CTRL);
	    break;
	  case XK_Caps_Lock:	/* Caps Lock */
	    set_xv_id(xv_map, keycode, SHIFT_CAPSLOCK);
	    break;
	  case XK_Shift_Lock:	/* Shift Lock */
	    set_xv_id(xv_map, keycode, SHIFT_LOCK);
	    break;
	  case XK_Meta_L:
	  case XK_Meta_R:	/* Meta */
	    set_xv_id(xv_map, keycode, SHIFT_META);
	    break;
	  case XK_Return:	/* CR */
	  case XK_Linefeed:	/* Line feed */
	  case XK_Tab:		/* Tab */
	  case XK_Escape:	/* Escape */
	  case XK_BackSpace:	/* Backspace */
	    set_xv_id(xv_map, keycode, (*ksym & 0x7f));
	    xv_map[KEYCODE_TO_KEY_INDEX(keycode, Meta_Index_Mask)] =
		xv_map[KEYCODE_TO_KEY_INDEX(keycode, Shift_and_Meta_Mask)] = 
		xv_map[KEYCODE_TO_KEY_INDEX(keycode, Control_and_Meta_Mask)] =
		xv_map[KEYCODE_TO_KEY_INDEX(keycode, All_Index_Masks)] = 
		    (xv_map[KEYCODE_TO_KEY_INDEX(keycode, No_Index_Mask)] | META_MASK);
	    break;
	  case XK_Delete:	/* Delete */
	    set_xv_id(xv_map, keycode, ASCII_DELETE);
	    xv_map[KEYCODE_TO_KEY_INDEX(keycode, Meta_Index_Mask)] =
		xv_map[KEYCODE_TO_KEY_INDEX(keycode, Shift_and_Meta_Mask)] =
		xv_map[KEYCODE_TO_KEY_INDEX(keycode, Shift_and_Control_Mask)] = 
		xv_map[KEYCODE_TO_KEY_INDEX(keycode, Control_and_Meta_Mask)] =
		xv_map[KEYCODE_TO_KEY_INDEX(keycode, All_Index_Masks)] = (ASCII_DELETE | META_MASK);
	    break;
	  case XK_space:	/* Space */
	    xv_map[KEYCODE_TO_KEY_INDEX(keycode, No_Index_Mask)] = 
		    xv_map[KEYCODE_TO_KEY_INDEX(keycode, Shift_Index_Mask)] = XK_space;
	    xv_map[KEYCODE_TO_KEY_INDEX(keycode, Control_Index_Mask)] =
		xv_map[KEYCODE_TO_KEY_INDEX(keycode, Shift_and_Control_Mask)] =  0;
	    xv_map[KEYCODE_TO_KEY_INDEX(keycode, Meta_Index_Mask)] =
		xv_map[KEYCODE_TO_KEY_INDEX(keycode, Shift_and_Meta_Mask)] = (XK_space | META_MASK);
	    xv_map[KEYCODE_TO_KEY_INDEX(keycode, Control_and_Meta_Mask)] = 
		xv_map[KEYCODE_TO_KEY_INDEX(keycode, All_Index_Masks)] = META_MASK;
	    break;


	  case XK_Help:
	    set_xv_id(xv_map, keycode, XK_Help);
	    break;

	  default:{
		register unsigned char unshifted_char, shifted_char;
#ifndef SVR4
		/* If not ASCII then map to nothing */
		if (!((*ksym >= XK_space) && (*ksym <= XK_asciitilde)))
		    break;

		unshifted_char = *ksym & 0x7f;
		shifted_char = *(ksym + 1) & 0x7f;
#else SVR4
                /* all other keysyms are masked to 8 bits */

		unshifted_char = *ksym;
		shifted_char = *(ksym + 1);
#endif SVR4

#ifdef notdef   /* This test seems obsolete */
		if (unshifted_char & ASCII_UNSHIFTED_MASK) {
		    /*
		     * Use the conventions assuming the first keysym
		     * represents the unshifted char and the next keysym is
		     * the shifted char.
		     */
		    shifted_char = *(ksym + 1) & 0x7f;
		} else {	/* Char is shifted */
		    /*
		     * Keysym does not correspond to conventions, so use
		     * standard ASCII shift.
		     */
		    shifted_char = unshifted_char;
		    unshifted_char = shifted_char | Shift_Index_Mask;
		}
		
#endif

		if ((unshifted_char >= 'a') && (unshifted_char <= 'z') ||
		    (unshifted_char == XK_backslash) || (unshifted_char == XK_bracketleft) ||
		    (unshifted_char == XK_bracketright)) {
		    /* Set entry for different modifier masks */
		    xv_map[KEYCODE_TO_KEY_INDEX(keycode, No_Index_Mask)] = unshifted_char;
		    xv_map[KEYCODE_TO_KEY_INDEX(keycode, Shift_Index_Mask)] = shifted_char;

		    xv_map[KEYCODE_TO_KEY_INDEX(keycode, Control_Index_Mask)] =
			xv_map[KEYCODE_TO_KEY_INDEX(keycode, Shift_and_Control_Mask)] =
			(unshifted_char &
			 ~(ASCII_UNSHIFTED_MASK | ASCII_CONTROL_MASK));

		    xv_map[KEYCODE_TO_KEY_INDEX(keycode, Meta_Index_Mask)] =
			(unshifted_char | META_MASK);
		    xv_map[KEYCODE_TO_KEY_INDEX(keycode, Shift_and_Meta_Mask)] =
		        (shifted_char | META_MASK);
		    xv_map[KEYCODE_TO_KEY_INDEX(keycode, Control_and_Meta_Mask)] =
		        xv_map[KEYCODE_TO_KEY_INDEX(keycode, All_Index_Masks)] =
		        (xv_map[KEYCODE_TO_KEY_INDEX(keycode, Control_Index_Mask)] |
		        META_MASK);
			
		} else {
		    xv_map[KEYCODE_TO_KEY_INDEX(keycode, No_Index_Mask)] =
			xv_map[KEYCODE_TO_KEY_INDEX(keycode, Control_Index_Mask)] =
			xv_map[KEYCODE_TO_KEY_INDEX(keycode, Shift_and_Control_Mask)] =
			unshifted_char;

		    xv_map[KEYCODE_TO_KEY_INDEX(keycode, Shift_Index_Mask)] =
			shifted_char;

		    xv_map[KEYCODE_TO_KEY_INDEX(keycode, Meta_Index_Mask)] =
		        xv_map[KEYCODE_TO_KEY_INDEX(keycode, Control_and_Meta_Mask)] =
		        xv_map[KEYCODE_TO_KEY_INDEX(keycode, All_Index_Masks)] =
		        (unshifted_char | META_MASK);
		    xv_map[KEYCODE_TO_KEY_INDEX(keycode, Shift_and_Meta_Mask)] =
		        (shifted_char | META_MASK);
		    /* Special case characters */
		    switch (unshifted_char) {
		      case XK_space:
			xv_map[KEYCODE_TO_KEY_INDEX(keycode, Shift_Index_Mask)] = unshifted_char;
			break;
		      case XK_2:
		      case XK_6:
		      	xv_map[KEYCODE_TO_KEY_INDEX(keycode, Control_Index_Mask)] =
			    xv_map[KEYCODE_TO_KEY_INDEX(keycode, Shift_and_Control_Mask)] = (shifted_char & ~ASCII_CONTROL_MASK);
			xv_map[KEYCODE_TO_KEY_INDEX(keycode, Control_and_Meta_Mask)] =
			    xv_map[KEYCODE_TO_KEY_INDEX(keycode, All_Index_Masks)] = 
			    (xv_map[KEYCODE_TO_KEY_INDEX(keycode, Control_Index_Mask)] | META_MASK);
			break;
		      case XK_quoteleft:
			xv_map[KEYCODE_TO_KEY_INDEX(keycode, Control_and_Meta_Mask)] =
			    xv_map[KEYCODE_TO_KEY_INDEX(keycode, All_Index_Masks)] = 0236;
			break;
		      case XK_slash:
		      case XK_minus:
			xv_map[KEYCODE_TO_KEY_INDEX(keycode, Control_Index_Mask)] =
			    xv_map[KEYCODE_TO_KEY_INDEX(keycode, Shift_and_Control_Mask)] = '\037';
			xv_map[KEYCODE_TO_KEY_INDEX(keycode, Control_and_Meta_Mask)] =
			    xv_map[KEYCODE_TO_KEY_INDEX(keycode, All_Index_Masks)] = 
			    (xv_map[KEYCODE_TO_KEY_INDEX(keycode, Control_Index_Mask)] | META_MASK);			    
			break;

		    }
		}

	    }
	    break;
	}
#ifdef BUILD_TABLE
	if ((xv_map[KEYCODE_TO_KEY_INDEX(keycode, No_Index_Mask)] > 0) && 
	    (xv_map[KEYCODE_TO_KEY_INDEX(keycode, No_Index_Mask)] < 256))
	    fprintf(dfp, "%c(0%3o), %c(0%3o) C(0%3o) S-C(0%3o) M(0%3o) S-M(0%3o) C-M(0%3o) S-C-M(0%3o)\n", 
	    	    xv_map[KEYCODE_TO_KEY_INDEX(keycode, No_Index_Mask)], 
	    	    xv_map[KEYCODE_TO_KEY_INDEX(keycode, No_Index_Mask)], 
	    	    xv_map[KEYCODE_TO_KEY_INDEX(keycode, Shift_Index_Mask)], 
	    	    xv_map[KEYCODE_TO_KEY_INDEX(keycode, Shift_Index_Mask)],
	            xv_map[KEYCODE_TO_KEY_INDEX(keycode, Control_Index_Mask)],
	            xv_map[KEYCODE_TO_KEY_INDEX(keycode, Shift_and_Control_Mask)],
	            xv_map[KEYCODE_TO_KEY_INDEX(keycode, Meta_Index_Mask)],
	            xv_map[KEYCODE_TO_KEY_INDEX(keycode, Shift_and_Meta_Mask)],
	            xv_map[KEYCODE_TO_KEY_INDEX(keycode, Control_and_Meta_Mask)],
	            xv_map[KEYCODE_TO_KEY_INDEX(keycode, All_Index_Masks)]	            
	            );
	
#endif
    }
    

#ifdef BUILD_TABLE
    fclose(fp);
    fclose(dfp);

    fp = fopen("Default_XV_table", "w");

    for (entry_count = 0; entry_count < MAX_KEYCODE_TABLE; entry_count++) {
	sprintf(str, "0x%4x,", xv_map[entry_count]);
	pad_zero(str);
	fprintf(fp, "%7s", str);
	if (((entry_count + 1) % 8) == 0)
	    fprintf(fp, "\n");
	else
	    fprintf(fp, " ");

    }
    fclose(fp);
#endif
    /*
     * display->keysym is used to initialize the ascii_map table. Deallocate
     * it.
     */

    XFree(display->keysyms);
    display->keysyms = (KeySym *) 0;
    return (xv_map);
}


unsigned short *
win_build_semantic_map(display, xv_map, seln_key_list, max_seln_keys)
    Display        *display;
    register unsigned short xv_map[];
    register unsigned short seln_key_list[];
    register int    max_seln_keys;

{
    register int    key_index;
    register unsigned short *sem_map;
    register int    index = 0;
    register unsigned short modifier_masks;
    int             f1_key_index = 0;	/* 0 = undefined */
    char            help_key_defined = FALSE;

#ifdef BUILD_TABLE
    FILE           *fp, *fopen();
    int             entry_count = 0;
    char            str[10];
    fp = fopen("Default_Sem_table", "w");
#endif
    sem_map = (unsigned short *) xv_calloc(MAX_KEYCODE_TABLE, sizeof(unsigned short));

    for (key_index = 0; key_index < MAX_KEYCODE_TABLE; key_index++) {

	sem_map[key_index] = ACTION_NULL_EVENT;

	modifier_masks = KEY_INDEX_TO_MASKS(key_index);
	switch (xv_map[key_index]) {
	  case KEY_LEFT(1):	/* L1 */
	    if (modifier_masks == No_Index_Mask)
		sem_map[key_index] = ACTION_STOP;
	    break;
	  case KEY_LEFT(2):	/* L2 */
	    if (modifier_masks == No_Index_Mask)
		sem_map[key_index] = ACTION_AGAIN;
	    break;
	  case KEY_LEFT(3):	/* L3 */
	    if (modifier_masks == No_Index_Mask) {
		sem_map[key_index] = ACTION_PROPS;
		ADD_SELN_KEYCODE(seln_key_list, max_seln_keys, index, key_index);
	    }
	    break;
	  case KEY_LEFT(4):	/* L4 */
	    if (modifier_masks == No_Index_Mask)
		sem_map[key_index] = ACTION_UNDO;
	    break;
	  case KEY_LEFT(5):	/* L5 */
	    if (modifier_masks == No_Index_Mask)
		sem_map[key_index] = ACTION_FRONT;
	    else if (modifier_masks == Shift_Index_Mask)
		sem_map[key_index] = ACTION_BACK;
	    break;
	  case KEY_LEFT(6):	/* L6 */
	    if (modifier_masks == No_Index_Mask) {
		sem_map[key_index] = ACTION_COPY;
		ADD_SELN_KEYCODE(seln_key_list, max_seln_keys, index, key_index);
	    }
	    break;
	  case KEY_LEFT(7):	/* L7 */
	    if (modifier_masks == No_Index_Mask)
		sem_map[key_index] = ACTION_OPEN;
	    else if (modifier_masks == Shift_Index_Mask)
		sem_map[key_index] = ACTION_CLOSE;
	    break;
	  case KEY_LEFT(8):	/* L8 */
	    if (modifier_masks == No_Index_Mask) {
		sem_map[key_index] = ACTION_PASTE;
		ADD_SELN_KEYCODE(seln_key_list, max_seln_keys, index, key_index);
	    }
	    break;
	  case KEY_LEFT(9):	/* L9 */
	    switch (modifier_masks) {
	      case No_Index_Mask:
		sem_map[key_index] = ACTION_FIND_FORWARD;
		ADD_SELN_KEYCODE(seln_key_list, max_seln_keys, index, key_index);
		break;
	      case Shift_Index_Mask:
		sem_map[key_index] = ACTION_FIND_BACKWARD;
		break;
	      case Control_Index_Mask:
		sem_map[key_index] = ACTION_REPLACE;
		break;
	      default:
		break;
	    }
	    break;
	  case KEY_LEFT(10):	/* L10 */
	    if (modifier_masks == No_Index_Mask) {
		sem_map[key_index] = ACTION_CUT;
		ADD_SELN_KEYCODE(seln_key_list, max_seln_keys, index, key_index);
	    }
	    break;
	  case KEY_RIGHT(7):	/* R7 */
	    if (modifier_masks == No_Index_Mask)
		sem_map[key_index] = ACTION_GO_DOCUMENT_START;
	    break;
	  case KEY_RIGHT(8):	/* R8 */
	    if (modifier_masks == No_Index_Mask)
		sem_map[key_index] = ACTION_GO_COLUMN_BACKWARD;
	    break;

	  case KEY_RIGHT(10):	/* R10 */
	    if (modifier_masks == No_Index_Mask)
		sem_map[key_index] = ACTION_GO_CHAR_BACKWARD;
	    break;
	  case KEY_RIGHT(11):	/* R11 */
	    if (modifier_masks == No_Index_Mask)
		sem_map[key_index] = ACTION_GO_LINE_FORWARD;
	    break;
	  case KEY_RIGHT(12):	/* R12 */
	    if (modifier_masks == No_Index_Mask)
		sem_map[key_index] = ACTION_GO_CHAR_FORWARD;
	    break;
	  case KEY_RIGHT(13):	/* R13 */
	    if (modifier_masks == No_Index_Mask)
		sem_map[key_index] = ACTION_GO_DOCUMENT_END;
	    break;
	  case KEY_RIGHT(14):	/* R14 */
	    if (modifier_masks == No_Index_Mask)
		sem_map[key_index] = ACTION_GO_COLUMN_FORWARD;
	    break;
	  case KEY_TOP(1):	/* F1 */
	    if (modifier_masks == No_Index_Mask)
		f1_key_index = key_index;
	    break;
	  case '\001':		/* control a */
	    switch (modifier_masks) {
	      case Control_Index_Mask:
		sem_map[key_index] = ACTION_GO_LINE_BACKWARD;
		break;
	      case Shift_and_Control_Mask:
		sem_map[key_index] = ACTION_GO_LINE_END;
		break;
	      case Meta_Index_Mask:
		sem_map[key_index] = ACTION_AGAIN;
		break;
	      default:
		break;
	    }
	    break;
	  case META_A:		/* meta a */
	    if (modifier_masks == Meta_Index_Mask) {
		sem_map[key_index] = ACTION_AGAIN;
	    }
	    break;
	  case '\002':		/* control b */
	    switch (modifier_masks) {
	      case Control_Index_Mask:
		sem_map[key_index] = ACTION_GO_CHAR_BACKWARD;
		break;
	      case Shift_and_Control_Mask:
		sem_map[key_index] = ACTION_GO_CHAR_FORWARD;
		break;
	      default:
		break;
	    }
	    break;
	  case META_C:		/* meta c */
	    if (modifier_masks == Meta_Index_Mask)
		sem_map[key_index] = ACTION_COPY;
	    break;
	  case META_D:		/* meta d */
	    if (modifier_masks == Meta_Index_Mask)
		sem_map[key_index] = ACTION_MATCH_DELIMITER;
	    break;
	  case '\005':		/* control e */
	    switch (modifier_masks) {
	      case Control_Index_Mask:
		sem_map[key_index] = ACTION_GO_LINE_END;
		break;
	      case Shift_and_Control_Mask:
		sem_map[key_index] = ACTION_GO_LINE_BACKWARD;
		break;
	      case Meta_Index_Mask:
		sem_map[key_index] = ACTION_EMPTY;
		break;
	      default:
		break;
	    }
	    break;

	  case META_E:		/* meta e */
	    if (modifier_masks == Meta_Index_Mask) {
		sem_map[key_index] = ACTION_EMPTY;
	    }
	    break;

	  case '\006':		/* control f */
	    switch (modifier_masks) {
	      case Control_Index_Mask:
		sem_map[key_index] = ACTION_GO_CHAR_FORWARD;
		break;
	      case Shift_and_Control_Mask:
		sem_map[key_index] = ACTION_GO_CHAR_BACKWARD;
		break;
	      default:
		break;
	    }
	    break;

	  case META_F:		/* meta f */
	    switch (modifier_masks) {
	      case Meta_Index_Mask:
		sem_map[key_index] = ACTION_FIND_FORWARD;
		break;
	      case Shift_and_Meta_Mask:
		sem_map[key_index] = ACTION_FIND_BACKWARD;
		break;
	      default:
		break;
	    }
	    break;
	  case META_I:		/* meta I */
	    if (modifier_masks == Meta_Index_Mask)
		sem_map[key_index] = ACTION_INCLUDE_FILE;
	    break;
	  case META_L:		/* meta l */
	    if (modifier_masks == Meta_Index_Mask)
		sem_map[key_index] = ACTION_LOAD;
	    break;

	  case '\016':		/* control n */
	    switch (modifier_masks) {
	      case Control_Index_Mask:
		sem_map[key_index] = ACTION_GO_COLUMN_FORWARD;
		break;
	      case Shift_and_Control_Mask:
		sem_map[key_index] = ACTION_GO_COLUMN_BACKWARD;
		break;
	      default:
		break;
	    }
	    break;

	  case '\020':		/* control p */
	    switch (modifier_masks) {
	      case Control_Index_Mask:
		sem_map[key_index] = ACTION_GO_COLUMN_BACKWARD;
		break;
	      case Shift_and_Control_Mask:
		sem_map[key_index] = ACTION_GO_COLUMN_FORWARD;
		break;
	      default:
		break;
	    }
	    break;

	  case META_P:		/* meta p */
	    if (modifier_masks == Meta_Index_Mask)
		sem_map[key_index] = ACTION_COPY_THEN_PASTE;
	    break;

	  case META_Q:		/* meta q */
	    if (modifier_masks == Meta_Index_Mask)
		sem_map[key_index] = ACTION_QUOTE;
	    break;

	  case META_S:		/* meta s */
	    if (modifier_masks == Meta_Index_Mask)
		sem_map[key_index] = ACTION_STORE;
	    break;

	  case '\025':		/* control u */
	    switch (modifier_masks) {
	      case Control_Index_Mask:
		sem_map[key_index] = ACTION_ERASE_LINE_BACKWARD;
		break;
	      case Shift_and_Control_Mask:
		sem_map[key_index] = ACTION_ERASE_LINE_END;
		break;
	      default:
		break;
	    }
	    break;

	  case META_U:		/* meta u */
	    if (modifier_masks == Meta_Index_Mask)
		sem_map[key_index] = ACTION_UNDO;
	    break;

	  case META_V:		/* meta v */
	    if (modifier_masks == Meta_Index_Mask)
		sem_map[key_index] = ACTION_PASTE;
	    break;

	  case '\027':		/* control w */
	    switch (modifier_masks) {
	      case Control_Index_Mask:
		sem_map[key_index] = ACTION_ERASE_WORD_BACKWARD;
		break;
	      case Shift_and_Control_Mask:
		sem_map[key_index] = ACTION_ERASE_WORD_FORWARD;
		break;
	      default:
		break;
	    }
	    break;

	  case META_X:		/* meta x */
	    if (modifier_masks == Meta_Index_Mask)
		sem_map[key_index] = ACTION_CUT;
	    break;

	  case ',':		/* , */
	    switch (modifier_masks) {
	      case Control_Index_Mask:
		sem_map[key_index] = ACTION_GO_WORD_BACKWARD;
		break;
	      case Shift_and_Control_Mask:
		sem_map[key_index] = ACTION_GO_WORD_FORWARD;
		break;
	      default:
		break;
	    }
	    break;
	  case '.':		/* . */
	    switch (modifier_masks) {
	      case Control_Index_Mask:
		sem_map[key_index] = ACTION_GO_WORD_END;
		break;
	      case Shift_and_Control_Mask:
		sem_map[key_index] = ACTION_GO_WORD_BACKWARD;
		break;
	      default:
		break;
	    }
	    break;
	  case '/':		/* / */
	    switch (modifier_masks) {
	      case Control_Index_Mask:
		sem_map[key_index] = ACTION_GO_WORD_FORWARD;
		break;
	      case Shift_and_Control_Mask:
		sem_map[key_index] = ACTION_GO_WORD_BACKWARD;
		break;
	      case Shift_and_Meta_Mask:
		sem_map[key_index] = ACTION_HELP;
		break;
	      default:
		break;
	    }
	    break;
	  case XK_Help:
	  case XK_F1:
	    sem_map[key_index] = ACTION_HELP;
	    help_key_defined = TRUE;
	    break;
	  case ';':
	    if (modifier_masks == Control_Index_Mask)
		sem_map[key_index] = ACTION_GO_LINE_FORWARD;
	    break;
	  case '\r':		/* CR */
	    /*
	     * This is one way to tell the difference between ^M and CR. But
	     * we might what to do it in a better way.
	     */
	    if (xv_map[key_index & ~(All_Index_Masks)] != xv_map[key_index])
		break;

	    switch (modifier_masks) {
	      case Control_Index_Mask:
		sem_map[key_index] = ACTION_GO_DOCUMENT_END;
		break;
	      case Meta_Index_Mask:
		sem_map[key_index] = ACTION_DO_IT;
		break;
	      case Shift_and_Control_Mask:
		sem_map[key_index] = ACTION_GO_DOCUMENT_START;
		break;
	      default:
		break;
	    }
	    break;
	  case '\t':		/* Tab */
	    /*
	     * This is one way to tell the difference between ^I and Tab. But
	     * we might what to do it in a better way.
	     */
	    if (xv_map[key_index & ~(All_Index_Masks)] != xv_map[key_index])
		break;

	    switch (modifier_masks) {
	      case Control_Index_Mask:
		sem_map[key_index] = ACTION_SELECT_FIELD_FORWARD;
		break;
	      case Shift_and_Control_Mask:
		sem_map[key_index] = ACTION_SELECT_FIELD_BACKWARD;
		break;
	      default:
		break;
	    }
	    break;

	  case ASCII_DELETE:	/* Delete for backspace */
	  case '\b':		/* Delete for backspace */

	    if (modifier_masks == No_Index_Mask)
		sem_map[key_index] = ACTION_ERASE_CHAR_BACKWARD;
	    else if (modifier_masks == Shift_Index_Mask)
		sem_map[key_index] = ACTION_ERASE_CHAR_FORWARD;
	    break;
	}

    }

    /*
     * *** BUG ALERT:  This is an interim solution.  By the time a XView
     * application starts up, a Help keysym should have been defined, either
     * by the Window Manager or the Session Manager.  Otherwise, help will
     * not following the pointer, but instead be delivered to the input
     * focus.
     */
    if (!help_key_defined && f1_key_index) {
	sem_map[f1_key_index] = ACTION_HELP;
    }
#ifdef BUILD_TABLE
    for (entry_count = 0; entry_count < MAX_KEYCODE_TABLE; entry_count++) {
	sprintf(str, "0x%4x,", sem_map[entry_count]);
	pad_zero(str);
	fprintf(fp, "%7s", str);
	if (((entry_count + 1) % 8) == 0)
	    fprintf(fp, "\n");
	else
	    fprintf(fp, " ");
    }
    fclose(fp);
#endif
    return (sem_map);
}
