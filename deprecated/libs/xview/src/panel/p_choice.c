#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)p_choice.c 20.140 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#include <xview_private/panel_impl.h>
#include <pixrect/pr_line.h>
#include <xview/openmenu.h>
#include <xview/font.h>
#include <X11/Xlib.h>
#include <xview_private/draw_impl.h>
#ifdef SVR4 
#include <stdlib.h> 
#endif /* SVR4 */


/*
 * Definitions
 */
#define CHOICE_PRIVATE(item)	\
	XV_PRIVATE(Choice_info, Xv_panel_choice, item)
#define CHOICE_PUBLIC(item)	XV_PUBLIC(item)
#define	CHOICE_FROM_ITEM(ip)	CHOICE_PRIVATE(ITEM_PUBLIC(ip))
#define ITEM_FROM_CHOICE(dp)	ITEM_PRIVATE(CHOICE_PUBLIC(dp))

#define ABSTK_X_GAP	4	/* white space between glyph and text */
#define DATA_HANDLE	1	/* unique key for private data handle */
#define	CHECKBOX_XOFFSET 3	/* # of pixels to leave after a checkbox */
#define NULL_CHOICE	-1
#define	TOGGLE_X_GAP	11	/* # of x pixels between toggle rectangles */
#define	TOGGLE_Y_GAP	6	/* # of y pixels between toggle rectangles */

/*
 * Assume 8 bits per byte, so byte for nth element is n/8, bit within that
 * byte is defined by the loworder three bits of n.
 */
#define WORD(n)         (n >> 5)/* word for element n */
#define BIT(n)          (n & 0x1F)	/* bit in word for element n */

/*
 * Create a set with n elements. Clear a set with n elements. Copy n elements
 * from one set to another
 */
#define	CREATE_SET(n)		\
    ((unsigned int *) xv_calloc((unsigned int) (WORD(n) + 1), sizeof(unsigned int)))

#define	CLEAR_SET(set, n)	\
    (XV_BZERO((char *) (set), (int) (WORD(n) + 1) * sizeof(unsigned int)))

#define	COPY_SET(from_set, to_set, n)	\
    (XV_BCOPY((char *) (from_set), (char *) (to_set), (int) ((WORD(n) + 1) * sizeof(unsigned int))))

/*
 * Add a choice by or-ing in the correct bit. Remove a choice by and-ing out
 * the correct bit.
 */
#define ADD_CHOICE(set, n)	((set)[WORD(n)] |= (1 << BIT(n)))
#define REMOVE_CHOICE(set, n)	((set)[WORD(n)] &= ~(1 << BIT(n)))

/* See if the nth bit is on */
#define IN(set, n)		(((set)[WORD(n)] >> BIT(n)) & 01 ? TRUE : FALSE)

#define	EACH_CHOICE(set, last_element, n)	\
   for ((n) = 0; (n) <= (last_element); (n)++) \
      if (IN(set, n))




/*
 * External functions and data
 */
#ifdef OW_I18N
extern struct pr_size   xv_pf_textwidth_wc();
extern wchar_t         _xv_null_string_wc[];
#else
extern struct pr_size   xv_pf_textwidth();
#endif /* OW_I18N */
extern void             screen_adjust_gc_color();
extern int		panel_item_destroy_flag;


/*
 * Package private functions
 */
Pkg_private int choice_init();
Pkg_private Xv_opaque choice_set_avlist();
Pkg_private Xv_opaque choice_get_attr();
Pkg_private int choice_destroy();


/*
 * Static functions
 */
static void     choice_begin_preview(), choice_cancel_preview(),
		choice_accept_preview(), choice_accept_menu(), choice_remove(),
		choice_accept_key(), choice_paint(), choice_layout(),
		choice_accept_kbd_focus(), choice_yield_kbd_focus();
static int      choice_number();
static int	choice_x_gap();
static int	choice_y_gap();
static unsigned int choice_value();
static void     choice_create_menu();
static void	choice_images_to_menu_items();
static void     choice_menu_busy_proc();
static void     choice_menu_done_proc();
static void	choice_update_focus_win();
static void	compute_nrows_ncols();
static void     layout_choices();
static void     paint_choice();
static void     preview_choice();
static void     update_display();
static void     update_item_rect();
static void     update_value_rect();
static Xv_opaque choice_do_menu_item();

static int
  find_choice(Item_info *ip, Event *event);

static int
modify_choice(register Item_info * ip, int type,
              int which_choice, Xv_opaque choice_info);

static int
  move_specified(register Item_info *ip, register Attr_avlist avlist);

static int
  re_alloc_choices(register Item_info *ip, int type, Xv_opaque choices[]);


static Panel_ops ops = {
    panel_default_handle_event,		/* handle_event() */
    choice_begin_preview,		/* begin_preview() */
    choice_begin_preview,		/* update_preview() */
    choice_cancel_preview,		/* cancel_preview() */
    choice_accept_preview,		/* accept_preview() */
    choice_accept_menu,			/* accept_menu() */
    choice_accept_key,			/* accept_key() */
    panel_default_clear_item,		/* clear() */
    choice_paint,			/* paint() */
    NULL,				/* resize() */
    choice_remove,			/* remove() */
    NULL,				/* restore() */
    choice_layout,			/* layout() */
    choice_accept_kbd_focus,		/* accept_kbd_focus() */
    choice_yield_kbd_focus,		/* yield_kbd_focus() */
    NULL				/* extension: reserved for future use */
};

static struct pr_size image_size();

typedef struct {		/* data for a choice item */
    Panel_item      public_self;/* back pointer to object */
    int             actual;	/* actual value of current */
    Panel_image    *choices;	/* each choice */
    Rect           *choice_rects;	/* each choice rect */
    int		    choose_none; /* exclusive settings:
				  * no choices selected is valid */
    int             choose_one;	/* only one choice allowed */
    int             current;	/* current choice */
    unsigned int   *default_value;	/* default settings (bit field of
					 * choices) */
    Panel_setting   display_level;	/* NONE, CURRENT, ALL */
    Panel_setting   feedback;	/* MARKED, INVERTED, NONE */
    int		    focus_choice; /* which choice the Location Cursor
				   * (Focus Window) points to */
    int             last;	/* last possible choice */
    Menu_item	   *mitems;	/* menu item for each choice */
    int		    ncols;	/* number of columns */
    int             nrows;	/* number of rows */
    unsigned int   *saved_value;/* save value set during preview of defaults */
    unsigned int   *value;	/* bit field of choices */

    int             choices_bold:1;	/* bold/not choices strings (MARKED
					 * feedback only) */
    struct {
	unsigned        choice_fixed:1;
	unsigned	display_updated:1;
	unsigned        feedback_set:1;
	unsigned        gap_set:1;
    }               status;
}               Choice_info;



/* ========================================================================= */

/* -------------------- XView Functions  -------------------- */
/*ARGSUSED*/
Pkg_private int
choice_init(panel_public, item_public, avlist)
    Panel           panel_public;
    Panel_item      item_public;
    Attr_avlist     avlist;
{
    Panel_info     *panel = PANEL_PRIVATE(panel_public);
    register Item_info *ip = ITEM_PRIVATE(item_public);
    Xv_panel_choice *item_object = (Xv_panel_choice *) item_public;
    register Choice_info *dp;

    dp = xv_alloc(Choice_info);

    /* link to object */
    item_object->private_data = (Xv_opaque) dp;
    dp->public_self = item_public;

    ip->ops = ops;
    if (panel->event_proc)
	ip->ops.panel_op_handle_event = (void (*) ()) panel->event_proc;
    ip->item_type = PANEL_CHOICE_ITEM;
    panel_set_bold_label_font(ip);

    if (panel->status.mouseless)
	ip->flags |= WANTS_KEY;

    /* Initialize non-zero Choice_info variables */
    dp->value = CREATE_SET(1);	/* set with one choice */
    dp->default_value = CREATE_SET(1);	/* (default) set with one choice */
    ADD_CHOICE(dp->default_value, 0);	/* default default is choice 0
					 * selected */
    dp->current = NULL_CHOICE;	/* no current choice */
    dp->actual = FALSE;		/* current choice is off */
    dp->display_level = PANEL_ALL;	/* default is display all choices */
    dp->choose_one = TRUE;	/* exclusive choices */
    dp->feedback = PANEL_INVERTED;	/* inverted feedback */

    /*
     * Initialize choices to a single string choice of "". Note that we do
     * not call layout_choices() here. The default layout is now to not have
     * the label and single mark baselines align.  If we lower the label at
     * this point, and the client has fixed its position, the baseline will
     * not be realigned when choices are added. So we settle for a bug in the
     * initial state: label baseline does not line up with single mark
     * baseline.  This restores release 3.0 behavior.
     */
    dp->choices = xv_alloc(struct panel_image);
#ifdef OW_I18N
    panel_make_image(ip->value_font, &dp->choices[0],
                     PIT_STRING, panel_strsave_wc(_xv_null_string_wc), 
		     FALSE, FALSE);
#else
    panel_make_image(ip->value_font, &dp->choices[0],
		     PIT_STRING, (Xv_opaque) "", FALSE, FALSE);
#endif /* OW_I18N */
    dp->choices[0].color = -1;	/* use foreground color */
    dp->choice_rects = xv_alloc(Rect);
    dp->choice_rects[0] = ip->value_rect;
    dp->mitems = xv_alloc(Menu_item);

    /* update the value and item rect */
    update_item_rect(ip);

    return XV_OK;
}


Pkg_private     Xv_opaque
choice_set_avlist(item_public, avlist)
    Panel_item      item_public;
    register Attr_avlist avlist;
{
    register Item_info *ip = ITEM_PRIVATE(item_public);
    register Choice_info *dp = CHOICE_PRIVATE(item_public);
    register Attr_attribute attr;

    short           choices_changed = FALSE;
    short           choices_moved = FALSE;
    int             choices_type = -1;	/* PIT_STRING or PIT_SVRIM */
    Xv_opaque      *choices;	/* choices array */
    Xv_Drawable_info *info;
    Attr_avlist     orig_avlist = avlist;	/* original avlist */
    Panel	    panel_public;
    Xv_opaque       result;
    int		    svalue;	/* signed value */
    unsigned int    value;	/* unsigned value */
    int		    value_set = FALSE;
    int             which_choice;
#ifdef OW_I18N
    int             count;         /* loop counter */
    int             num_choices;   /* number of choices */
    Xv_opaque      *temp_choices = 0; /* wide char version of choices array */
    wchar_t        *temp_choice = 0; /* address of wide char choice string */
#endif /* OW_I18N */

    /* if a client has called panel_item_parent this item may not */
    /* have a parent -- do nothing in this case */
    if (ip->panel == NULL) {
	return ((Xv_opaque) XV_ERROR);
    }
    /*
     * we need to have the item layed out before doing any of this, so call
     * the generic item set code now.
     */

    /* XV_END_CREATE is not used here, so return. */
    if (*avlist == XV_END_CREATE)
	return XV_OK;

    /* Parse Panel Item Generic attributes before Choice attributes.
     * panel_redisplay_item will be called in item_set_avlist.
     * This routine returns XV_SET_DONE, which prevents item_set_avlist
     * from being called again from xv_set_pkg_avlist.
     * Prevent panel_redisplay_item from being called in item_set_avlist.
     */
    ip->panel->no_redisplay_item = TRUE;
    result = xv_super_set_avlist(item_public, &xv_panel_choice_pkg, avlist);
    ip->panel->no_redisplay_item = FALSE;
    if (result != XV_OK)
	return result;

    while (attr = *avlist++) {
	switch (attr) {
#ifdef OW_I18N
          case PANEL_CHOICE_STRINGS:
            choices_type = PIT_STRING;
            /*
             *  Panel will only process wchar_t strings.  Make temp space for
             *  the choice strings and free it at the end of this routine.
             *  re_alloc_choices will do the real malloc for this string.
             */
            choices = avlist;  /* base of the array */
 
            /*  count the number of choices */
            for (num_choices = 0; choices[num_choices]; num_choices++);
        
            /* convert each string into wide char, do it the easy way now,
             *  will need to improve the method of malloc string arrays
             */
            temp_choices = (Xv_opaque *)xv_alloc_n(Xv_opaque, (num_choices+1));
            for (count = 0; count < num_choices; count++) {
                temp_choices[count] = (Xv_opaque)_xv_mbstowcsdup((char*)choices[count]);
            }
            temp_choices[num_choices] = 0;
	    choices = temp_choices;
	    while (*avlist++);
            break;

          case PANEL_CHOICE_STRINGS_WCS:
            choices_type = PIT_STRING;
            /*
             *  Panel will only process wchar_t strings.  Make temp space for
             *  the choice strings and free it at the end of this routine.
             *  re_alloc_choices will do the real malloc for this string.
             */
            choices = avlist;  /* base of the array */
 
            /*  count the number of choices */
            for (num_choices = 0; choices[num_choices]; num_choices++);
        
            /* convert each string into wide char, do it the easy way now,
             *  will need to improve the method of malloc string arrays
             */
            temp_choices = (Xv_opaque *)xv_alloc_n(Xv_opaque, (num_choices+1));
            for (count = 0; count < num_choices; count++) {
                temp_choices[count] = (Xv_opaque)panel_strsave_wc((char *)choices[count]);
            }
            temp_choices[num_choices] = 0;
	    choices = temp_choices;
	    while (*avlist++);
            break;

          case PANEL_CHOICE_IMAGES:
            choices_type = PIT_SVRIM;
            choices = avlist;  /* base of the array */
	    while (*avlist++); /* skip past the list */
            break;
#else
	  case PANEL_CHOICE_STRINGS:
	  case PANEL_CHOICE_IMAGES:
	    choices_type = attr ==
		PANEL_CHOICE_STRINGS ? PIT_STRING : PIT_SVRIM;
	    choices = avlist;	/* base of the array */
	    while (*avlist++);	/* skip past the list */
	    break;
#endif /* OW_I18N */

#ifdef OW_I18N
	  case PANEL_CHOICE_STRING:
	    which_choice = (int) *avlist++;
	    temp_choice = (wchar_t *)_xv_mbstowcsdup((char *)*avlist++);
	    if (!modify_choice(ip, PIT_STRING, which_choice, 
			   (Xv_opaque) temp_choice))
		return (Xv_opaque) attr;
	    choices_changed = TRUE;
	    break;

	  case PANEL_CHOICE_STRING_WCS:
	    which_choice = (int) *avlist++;
	    temp_choice = (wchar_t *)panel_strsave_wc(*avlist++);
	    if (!modify_choice(ip, PIT_STRING, which_choice, (Xv_opaque)temp_choice))
		return (Xv_opaque) attr;
	    choices_changed = TRUE;
	    break;

	  case PANEL_CHOICE_IMAGE:
            which_choice = (int) *avlist++;
            if (!modify_choice(ip, PIT_SVRIM, which_choice, *avlist++))
                return (Xv_opaque) attr;
            choices_changed = TRUE;
	    break;
#else
	  case PANEL_CHOICE_STRING:
	  case PANEL_CHOICE_IMAGE:
	    which_choice = (int) *avlist++;
	    if (!modify_choice(ip, attr == PANEL_CHOICE_STRING ?
			   PIT_STRING : PIT_SVRIM, which_choice, *avlist++))
		return (Xv_opaque) attr;
	    choices_changed = TRUE;
	    break;
#endif /* OW_I18N */

	  case PANEL_CHOICE_FONTS:
	    /* Sunview1 compatibility attribute: not used */
	    while (*avlist++);	/* skip past the list */
	    break;

	  case PANEL_CHOICES_BOLD:
	    /* Note: dp->choices_bold is only used if feedback is MARKED */
	    dp->choices_bold = (*avlist++ != 0);
	    for (which_choice = 0; which_choice <= dp->last; which_choice++)
		if (is_string(&dp->choices[which_choice]))
		    image_bold(&dp->choices[which_choice]) = dp->choices_bold;
	    choices_changed = TRUE;
	    break;

	  case PANEL_CHOICE_OFFSET:
	    /* Sunview 1 compatibility attribute: not used */
	    avlist++;
	    break;

	  case PANEL_CHOOSE_NONE:
	    dp->choose_none = (int) *avlist++;
	    break;

	  case PANEL_CHOOSE_ONE:
	    dp->choose_one = (int) *avlist++;
	    CLEAR_SET(dp->value, dp->last);
	    if (!dp->choose_one) {
		ip->item_type = PANEL_TOGGLE_ITEM;
		/*
		 * Note that this depends on the fact that PANEL_CHOOSE_ONE
		 * can only be specified at create time, as part of the
		 * PANEL_TOGGLE macro.  So no choices have been set yet.
		 */
		/* force re-layout and resize of rect */
		choices_changed = TRUE;
	    } else
		ip->item_type = PANEL_CHOICE_ITEM;
	    break;

	  case PANEL_LAYOUT:
	    avlist++;
	    choices_changed = TRUE;
	    break;

	  case PANEL_FEEDBACK:
	    dp->feedback = (Panel_setting) * avlist++;
	    dp->status.feedback_set = TRUE;
	    choices_changed = TRUE;
	    break;

	  case PANEL_MARK_IMAGES:
	  case PANEL_NOMARK_IMAGES:
	    /* SunView1 compatibility attributes: not used */
	    while (*avlist++);
	    break;

	  case PANEL_MARK_IMAGE:
	  case PANEL_NOMARK_IMAGE:
	    /* SunView1 compatibility attributes: not used */
	    avlist++;
	    avlist++;
	    break;

	  case PANEL_DISPLAY_LEVEL:
	    dp->display_level = (Panel_setting) * avlist++;
	    /* ignore PANEL_NONE and PANEL_CURRENT for toggles */
	    if ((dp->display_level == PANEL_NONE ||
		 dp->display_level == PANEL_CURRENT) && !dp->choose_one)
		dp->display_level = PANEL_ALL;
	    choices_changed = TRUE;
	    break;

	  case PANEL_CHOICE_NROWS:
	      dp->ncols = 0;  /* ignore other dimension */
	      dp->nrows = (int) *avlist++;
	      choices_changed = TRUE;
	      break;

	  case PANEL_CHOICE_NCOLS:
	      dp->nrows = 0;  /* ignore other dimension */
	      dp->ncols = (int) *avlist++;
	      choices_changed = TRUE;
	      break;

	  default:
	    /* skip past what we don't care about */
	    avlist = attr_skip(attr, avlist);
	    break;
	}
    }

    if (!dp->status.feedback_set)
	/* set the default feedback */
	switch (dp->display_level) {
	  case PANEL_NONE:
	  case PANEL_CURRENT:
	    dp->feedback = PANEL_NONE;
	    break;

	  default:
	    dp->feedback = PANEL_INVERTED;
	}

    if (set(choices_type)) {
	if (!re_alloc_choices(ip, choices_type, choices))
	    return (Xv_opaque) XV_ERROR;
	choices_changed = TRUE;
    }

    /*
     * now set things that depend on the new list of choices or the
     * attributes that were set above.
     */
    avlist = orig_avlist;
    while (attr = *avlist++) {
	switch (attr) {
	  case PANEL_CHOICE_COLOR:
	    which_choice = (int) *avlist++;
	    if (which_choice < 0 || which_choice > dp->last)
		return (Xv_opaque) attr;
	    panel_public = PANEL_PUBLIC(ip->panel);
	    DRAWABLE_INFO_MACRO(panel_public, info);
	    if (xv_depth(info) > 1) {
		dp->choices[which_choice].color = (int) *avlist++;
		if (!choices_changed && ip->menu) {
		    xv_set(dp->mitems[which_choice],
			   MENU_COLOR, dp->choices[which_choice].color,
			   NULL);
		}
	    } else
		avlist++;
	    break;

	  case PANEL_MARK_IMAGE:
	  case PANEL_NOMARK_IMAGE:
	    /* SunView1 compatibility attributes: not used */
	    avlist++;
	    avlist++;
	    break;

	  case PANEL_CHOICE_FONT:
	    /* SunView1 compatibility attribute: not used */
	    break;

	  case PANEL_VALUE:
	    if (dp->choose_one) {
		svalue = (int) *avlist++;
		if (svalue <= dp->last) {
		    CLEAR_SET(dp->value, dp->last);
		    if (!dp->choose_none && svalue < 0)
			svalue = 0;
		    if (svalue >= 0)
			ADD_CHOICE(dp->value, svalue);
		}
	    } else
		dp->value[0] = (unsigned int) *avlist++;
	    value_set = TRUE;
	    break;

	  case PANEL_DEFAULT_VALUE:
	    value = (unsigned int) *avlist++;
	    /* Value contains a choice (bit) number */
	    if (value <= dp->last) {
		CLEAR_SET(dp->default_value, dp->last);
		ADD_CHOICE(dp->default_value, value);
	    }
	    if (!choices_changed && ip->menu)
		xv_set(ip->menu,
		       MENU_DEFAULT,
			   choice_number(dp->default_value, dp->last) + 1,
		       NULL);
	    break;

	  case PANEL_TOGGLE_VALUE:
	    which_choice = (int) *avlist++;
	    if (which_choice < 0 || which_choice > dp->last)
		return (Xv_opaque) attr;
	    if (*avlist++)
		ADD_CHOICE(dp->value, which_choice);
	    else
		REMOVE_CHOICE(dp->value, which_choice);
	    value_set = TRUE;
	    break;

	  default:
	    /* skip past what we don't care about */
	    avlist = attr_skip(attr, avlist);
	    break;
	}
    }

    if (dp->choose_one && !dp->choose_none) {
	/* Choice item: insure at least one choice is selected */
	value = choice_number(dp->value, dp->last);
	if (!IN(dp->value, value))
	    ADD_CHOICE(dp->value, 0);
    }

    if ((dp->display_level == PANEL_NONE ||
	 dp->display_level == PANEL_CURRENT) &&
	!created(ip) && !value_set)
	COPY_SET(dp->default_value, dp->value, dp->last);

    if (choices_changed) {
	/* Layout the choices if no choice or mark has a fixed position. */
	if (!dp->status.choice_fixed)
	    layout_choices(ip);
	if (dp->display_level == PANEL_NONE ||
	    dp->display_level == PANEL_CURRENT) {
	    /*
	     * Create the menu. laf note:  This was originally at the end
	     * of choice_init.  However, since we want different menus depending
	     * on whether the user specifies choice or toggle (via
	     * PANEL_CHOOSE_ONE), it was moved to here.  We must first make sure
	     * that we destroy any menu that was previously created (in case
	     * choice_set_avlist was called via xv_set instead of xv_create).
	     */
	    choice_create_menu(ip);
	}
    }

    /* Move any specified choices */
    choices_moved = move_specified(ip, orig_avlist);

    if (choices_changed || choices_moved)
	update_item_rect(ip);

    return XV_OK;
}


/*VARARGS*/
Pkg_private     Xv_opaque
choice_get_attr(item_public, status, which_attr, valist)
    Panel_item      item_public;
    int            *status;
    register Attr_attribute which_attr;
    va_list	    valist;
{
    register Choice_info *dp = CHOICE_PRIVATE(item_public);
    register int    bad_arg;
    Item_info      *ip = ITEM_PRIVATE(item_public);
    int             nrows, ncols;
    int             arg;

    switch (which_attr) {
      case PANEL_DEFAULT_VALUE:	/* ordinal value */
	return (Xv_opaque) choice_number(dp->default_value, dp->last);

      case PANEL_VALUE:	/* ordinal value or set of values */
	return (Xv_opaque) choice_value(dp->choose_one, dp->value, dp->last);

      case PANEL_TOGGLE_VALUE:	/* on/off value of arg'th choice */
	arg = va_arg(valist, int);
	bad_arg = (arg < 0 || arg > dp->last);
	if (bad_arg)
	    return (Xv_opaque) 0;
	return (Xv_opaque) IN(dp->value, arg);

      case PANEL_DISPLAY_LEVEL:
	return (Xv_opaque) dp->display_level;

      case PANEL_FEEDBACK:
	return (Xv_opaque) dp->feedback;

#ifdef OW_I18N
      case PANEL_CHOICE_STRINGS_WCS:
#endif /* OW_I18N */
      case PANEL_CHOICE_STRINGS:
      case PANEL_CHOICE_IMAGES:
	return (Xv_opaque) NULL;

      case PANEL_CHOOSE_NONE:
	return (Xv_opaque) dp->choose_none;

      case PANEL_CHOOSE_ONE:
	return (Xv_opaque) dp->choose_one;

      case PANEL_CHOICE_FONT:
	arg = va_arg(valist, int);
	bad_arg = (arg < 0 || arg > dp->last);
	if (bad_arg || !is_string(&dp->choices[arg]))
	    return (Xv_opaque) 0;
	return (Xv_opaque) image_font(&dp->choices[arg]);

      case PANEL_CHOICE_FONTS:
	return (Xv_opaque) NULL;

      case PANEL_CHOICE_COLOR:
	arg = va_arg(valist, int);
	bad_arg = (arg < 0 || arg > dp->last);
	if (bad_arg)
	    return (Xv_opaque) 0;
	return (Xv_opaque) image_color(&dp->choices[arg]);

      case PANEL_CHOICE_RECT:
	arg = va_arg(valist, int);
	bad_arg = (arg < 0 || arg > dp->last);
	if (bad_arg)
	    return (Xv_opaque) 0;
	return (Xv_opaque) &dp->choice_rects[arg];

#ifdef OW_I18N
      case PANEL_CHOICE_STRING_WCS:
	arg = va_arg(valist, int);
	bad_arg = (arg < 0 || arg > dp->last);
        if (bad_arg || !is_string(&dp->choices[arg]))
            return (Xv_opaque) NULL;
        return (Xv_opaque) image_string_wc(&dp->choices[arg]);
 
      case PANEL_CHOICE_STRING:
	arg = va_arg(valist, int);
	bad_arg = (arg < 0 || arg > dp->last);
        if (bad_arg || !is_string(&dp->choices[arg]))
            return (Xv_opaque) NULL;
        image_string(&dp->choices[arg]) =
            (char *) _xv_wcstombsdup(image_string_wc(&dp->choices[arg]));
        return (Xv_opaque) image_string(&dp->choices[arg]);
#else
      case PANEL_CHOICE_STRING:
	arg = va_arg(valist, int);
	bad_arg = (arg < 0 || arg > dp->last);
	if (bad_arg || !is_string(&dp->choices[arg]))
	    return (Xv_opaque) 0;
	return (Xv_opaque) image_string(&dp->choices[arg]);
#endif /* OW_I18N */

      case PANEL_CHOICE_IMAGE:
	arg = va_arg(valist, int);
	bad_arg = (arg < 0 || arg > dp->last);
	if (bad_arg || !is_svrim(&dp->choices[arg]))
	    return (Xv_opaque) 0;
	return (Xv_opaque) image_svrim(&dp->choices[arg]);

      case PANEL_CHOICE_X:
	arg = va_arg(valist, int);
	bad_arg = (arg < 0 || arg > dp->last);
	if (bad_arg)
	    return (Xv_opaque) 0;
	return (Xv_opaque) dp->choice_rects[arg].r_left;

      case PANEL_CHOICE_Y:
	arg = va_arg(valist, int);
	bad_arg = (arg < 0 || arg > dp->last);
	if (bad_arg)
	    return (Xv_opaque) 0;
	return (Xv_opaque) dp->choice_rects[arg].r_top;

      case PANEL_MARK_X:
	/* SunView1 compatibility attribute */
	arg = va_arg(valist, int);
	bad_arg = (arg < 0 || arg > dp->last);
	if (bad_arg)
	    return (Xv_opaque) 0;
	return (Xv_opaque) dp->choice_rects[arg].r_left;

      case PANEL_MARK_Y:
	/* SunView1 compatibility attribute */
	arg = va_arg(valist, int);
	bad_arg = (arg < 0 || arg > dp->last);
	if (bad_arg)
	    return (Xv_opaque) 0;
	return (Xv_opaque) dp->choice_rects[arg].r_top;

      case PANEL_MARK_IMAGE:
      case PANEL_NOMARK_IMAGE:
	/* SunView1 compatibility attributes */
	return (Xv_opaque) 0;

      case PANEL_CHOICE_NROWS:
      case PANEL_CHOICE_NCOLS:
	/*
	 *  We should give the user back exactly what we are using
	 *  (which isn't exactly what they specified).
	 */
	compute_nrows_ncols (ip, &nrows, &ncols);
	return (Xv_opaque) which_attr == PANEL_CHOICE_NROWS ? nrows : ncols;

      case PANEL_NCHOICES:
	return (Xv_opaque) dp->last + 1;  /* number of choices */

      default:
	*status = XV_ERROR;
	return (Xv_opaque) 0;
    }
}


Pkg_private int
choice_destroy(item_public, status)
    Panel_item      item_public;
    Destroy_status  status;
{
    register Choice_info *dp = CHOICE_PRIVATE(item_public);

    if ((status == DESTROY_CHECKING) || (status == DESTROY_SAVE_YOURSELF))
	return XV_OK;

    choice_remove(item_public);
    (void) panel_free_choices(dp->choices, 0, (int) dp->last);
    free ((char *) dp->mitems);
    free((char *) dp->choice_rects);
    free((char *) dp->value);
    free((char *) dp->default_value);
    free((char *) dp);

    return XV_OK;
}



/* --------------------  Panel Item Operations  -------------------- */
static void
choice_begin_preview(item_public, event)
    Panel_item	    item_public;
    Event          *event;
{
    Item_info      *ip = ITEM_PRIVATE(item_public);

    /* Ignore drags of the ADJUST mouse button */
    if (event_action(event) == LOC_DRAG && !action_select_is_down(event))
	return;
    preview_choice(ip, find_choice(ip, event), event, TRUE);
}


/* ARGSUSED */
static void
choice_cancel_preview(item_public, event)
    Panel_item	    item_public;
    Event          *event;
{
    int		    choice_nbr;
    Choice_info	   *dp = CHOICE_PRIVATE(item_public);
    Item_info      *ip = ITEM_PRIVATE(item_public);

    /* restore the current choice */
    update_display(ip, dp->current, dp->actual, FALSE);

    /* restore the value if modified */
    choice_nbr = choice_number(dp->value, dp->last);
    if (dp->choose_one && dp->current != NULL_CHOICE &&
    	(choice_nbr || IN(dp->value, 0)))
	update_display(ip, choice_nbr, TRUE, FALSE);

    dp->current = NULL_CHOICE;
}


static void
choice_accept_preview(item_public, event)
    Panel_item	    item_public;
    Event          *event;
{
    int		    choice_nbr;
    Choice_info	   *dp = CHOICE_PRIVATE(item_public);
    Item_info      *ip = ITEM_PRIVATE(item_public);
    int		    on;
    int		    value;

    /* nothing to accept if no current choice */
    if (dp->current == NULL_CHOICE)
	return;

    /*
     * remove current choice if only one choice allowed modify the value if
     * current is non-null
     */
    if (dp->choose_one) {
	if (dp->choose_none) {
	    choice_nbr = choice_number(dp->value, dp->last);
	    if (choice_nbr == 0 && !IN(dp->value, 0)) {
		/* No choice selected: Turn on current choice */
		ADD_CHOICE(dp->value, dp->current);
		on = TRUE;
	    } else if (choice_nbr == dp->current) {
		/* Turn off current choice */
		REMOVE_CHOICE(dp->value, dp->current);
		on = FALSE;
	    } else {
		/* Turn off previous choice and turn on current choice. */
		CLEAR_SET(dp->value, dp->last);
		ADD_CHOICE(dp->value, dp->current);
		on = TRUE;
	    }
	} else {
	    CLEAR_SET(dp->value, dp->last);
	    ADD_CHOICE(dp->value, dp->current);
	    on = TRUE;
	}
    } else if (!dp->actual) {
	ADD_CHOICE(dp->value, dp->current);
	on = TRUE;
    } else {
	REMOVE_CHOICE(dp->value, dp->current);
	on = FALSE;
    }

    dp->focus_choice=dp->current;
    choice_update_focus_win(ip);


    if (dp->feedback == PANEL_MARKED ||
	dp->display_level == PANEL_NONE ||
	dp->display_level == PANEL_CURRENT)
	update_display(ip, dp->current, on, FALSE);

    /* notify the client */
    value = choice_value(dp->choose_one, dp->value, dp->last);
    panel_item_destroy_flag = 1;
    (*ip->notify) (ITEM_PUBLIC(ip), value, event);
    dp->current = NULL_CHOICE;
}


static void
choice_accept_menu(item_public, event)
    Panel_item	    item_public;
    Event          *event;
{
    Choice_info	   *dp = CHOICE_PRIVATE(item_public);
    int             i;
    Xv_Drawable_info *info;
    Item_info      *ip = ITEM_PRIVATE(item_public);
    Menu_item       menu_item;
    Xv_Window       paint_window = event_window(event);
    Rect            position_rect;

    if (dp->display_level != PANEL_NONE &&
	dp->display_level != PANEL_CURRENT)
	return;

    if (ip->menu == XV_ZERO || paint_window == XV_ZERO)
	return;

    /* Invert the abbreviated menu button */
    DRAWABLE_INFO_MACRO(paint_window, info);
    olgx_draw_abbrev_button(ip->value_ginfo, xv_xid(info),
	dp->choice_rects[0].r_left,
	dp->choice_rects[0].r_top +
	    (dp->choice_rects[0].r_height -
	    Abbrev_MenuButton_Height(ip->value_ginfo)) / 2,
	OLGX_INVOKED);
    dp->status.display_updated = FALSE;

    /* Select the current choice; deselect all others */
    for (i = 0; i <= dp->last; i++) {
	menu_item = menu_get(ip->menu, MENU_NTH_ITEM, i + 1);
	xv_set(menu_item, MENU_SELECTED, IN(dp->value, i), NULL);
    }

    /*
     * Save public panel handle and current menu done proc. Switch to
     * choice's menu done proc.
     */
    xv_set(ip->menu,
	   XV_KEY_DATA, DATA_HANDLE, dp,
	   XV_KEY_DATA, MENU_DONE_PROC, xv_get(ip->menu, MENU_DONE_PROC),
	   MENU_BUSY_PROC, choice_menu_busy_proc,
	   MENU_DONE_PROC, choice_menu_done_proc,
	   NULL);

    /* Show the menu */
    position_rect = ip->value_rect;
    position_rect.r_top += (position_rect.r_height -
	Abbrev_MenuButton_Height(ip->value_ginfo)) / 2;
    position_rect.r_width = 0;	/* paint menu flush left */
    position_rect.r_height = Abbrev_MenuButton_Height(ip->value_ginfo);

    ip->panel->status.current_item_active = TRUE;

    menu_show(ip->menu, paint_window, event,
	      MENU_POSITION_RECT, &position_rect,
	      MENU_PULLDOWN, TRUE,
	      NULL);
}


static void
choice_accept_key(item_public, event)
    Panel_item	    item_public;
    Event          *event;
{
    Choice_info	   *dp = CHOICE_PRIVATE(item_public);
    Item_info      *ip = ITEM_PRIVATE(item_public);
    int		    ncols;	/* number of columns */
    int		    nrows;	/* number of rows */

    if (event_is_up(event))
	return;
    if (dp->ncols > 1) {
	ncols = dp->ncols;
	nrows = (dp->last+1) / dp->ncols;
	if ((dp->last+1) % dp->ncols > 0)
	    nrows++;
    } else if (dp->nrows > 1) {
	ncols = (dp->last+1) / dp->nrows;
	if ((dp->last+1) % dp->nrows > 0)
	    ncols++;
	nrows = dp->nrows;
    } else {
	ncols = 1;
	nrows = 1;
    }
    if (ip->layout == PANEL_VERTICAL) {
	switch (event_action(event)) {
	  case ACTION_UP:
	    if (--dp->focus_choice < 0)
		dp->focus_choice = dp->last;
	    break;
	  case ACTION_DOWN:
	    if (++dp->focus_choice > dp->last)
		dp->focus_choice = 0;
	    break;
	  case ACTION_LEFT:
	    if (ncols <= 1)
		return;
	    if (dp->focus_choice == 0 && dp->last == ncols*nrows - 1) {
		dp->focus_choice = dp->last;
		break;
	    }
	    dp->focus_choice -= nrows;
	    if (dp->focus_choice < 0)
		/* Wrap to end of previous row */
		dp->focus_choice += ncols*nrows - 1;
	    break;
	  case ACTION_RIGHT:
	    if (ncols <= 1)
		return;
	    if (dp->focus_choice == dp->last && dp->last == ncols*nrows - 1) {
		dp->focus_choice = 0;
		break;
	    }
	    dp->focus_choice += nrows;
	    if (dp->focus_choice > dp->last)
		/* Wrap to start of next row */
		dp->focus_choice -= ncols*nrows - 1;
	    break;
	  default:
	    return;
	}
    } else {
	switch (event_action(event)) {
	  case ACTION_UP:
	    if (nrows <= 1)
		return;
	    if (dp->focus_choice == 0 && dp->last == ncols*nrows - 1) {
		dp->focus_choice = dp->last;
		break;
	    }
	    dp->focus_choice -= ncols;
	    if (dp->focus_choice < 0)
		/* Wrap to bottom of previous column */
		dp->focus_choice += ncols*nrows - 1;
	    break;
	  case ACTION_DOWN:
	    if (nrows <= 1)
		return;
	    if (dp->focus_choice == dp->last && dp->last == ncols*nrows - 1) {
		dp->focus_choice = 0;
		break;
	    }
	    dp->focus_choice += ncols;
	    if (dp->focus_choice > dp->last)
		/* Wrap to top of next column */
		dp->focus_choice -= ncols*nrows - 1;
	    break;
	  case ACTION_LEFT:
	    if (--dp->focus_choice < 0)
		dp->focus_choice = dp->last;
	    break;
	  case ACTION_RIGHT:
	    if (++dp->focus_choice > dp->last)
		dp->focus_choice = 0;
	    break;
	  default:
	    return;
	}
    }
    choice_update_focus_win(ip);
}


static void
choice_paint(item_public)
    Panel_item	    item_public;
{
    Choice_info	   *dp = CHOICE_PRIVATE(item_public);
    int		    i;
    Item_info      *ip = ITEM_PRIVATE(item_public);
    int             which_choice;
    Rect            rect;
    int             nrows, ncols;

    /* paint the label, centered vertically with first row of choices */
    rect.r_left = ip->label_rect.r_left;
    rect.r_width = ip->label_rect.r_width;
    if (ip->layout == PANEL_HORIZONTAL) {
	rect.r_top = ip->value_rect.r_top;

	/* get the actual number of rows and columns used */
	compute_nrows_ncols (ip, &nrows, &ncols);

	if (nrows > 1) 
	    rect.r_height = dp->choice_rects[0].r_height;
		     
	else 
	    rect.r_height = ip->value_rect.r_height;

    } else {
	rect.r_top = ip->label_rect.r_top;
	rect.r_height = ip->label_rect.r_height;
    }
    panel_paint_image(ip->panel, &ip->label, &rect, inactive(ip),
		      ip->color_index);

    /* paint the choices */
    switch (dp->display_level) {
      case PANEL_CURRENT:	/* draw the current choice */
	rect = ip->value_rect;
	rect.r_left += Abbrev_MenuButton_Width(ip->value_ginfo) + ABSTK_X_GAP;
	rect.r_width -= Abbrev_MenuButton_Width(ip->value_ginfo) + ABSTK_X_GAP;
	panel_clear_rect(ip->panel, rect); /* clear the current choice */
	/* fall through to PANEL_NONE */
      case PANEL_NONE:
	which_choice = choice_number(dp->value, dp->last);
	paint_choice(ip->panel, ip, dp, which_choice, FALSE);
	break;

      case PANEL_ALL:		/* draw all the choices */
	/* Draw each choice */
	for (i = 0; i <= dp->last; i++)
	    paint_choice(ip->panel, ip, dp, i, IN(dp->value, i));
	break;
    }
}				/* paint */


static void
choice_remove(item_public)
    Panel_item	    item_public;
{
    Item_info      *ip = ITEM_PRIVATE(item_public);
    Panel_info	   *panel = ip->panel;

    /*
     * Only reassign the keyboard focus to another item if the panel isn't
     * being destroyed.
     */
    if (!panel->status.destroying && panel->kbd_focus_item == ip) {
	panel->kbd_focus_item = panel_next_kbd_focus(panel, TRUE);
	panel_accept_kbd_focus(panel);
    }

    return;
}


static void
choice_layout(item_public, deltas)
    Panel_item	    item_public;
    Rect           *deltas;
{
    Choice_info	   *dp = CHOICE_PRIVATE(item_public);
    int		    i;

    /* bump each choice rect */
    for (i = 0; i <= dp->last; i++) {
	dp->choice_rects[i].r_left += deltas->r_left;
	dp->choice_rects[i].r_top += deltas->r_top;
    }
}				/* layout */


static void
choice_accept_kbd_focus(item_public)
    Panel_item	    item_public;
{
    Choice_info	   *dp = CHOICE_PRIVATE(item_public);
    Item_info      *ip = ITEM_PRIVATE(item_public);

/*
    dp->focus_choice = choice_number(dp->value, dp->last);
*/
    choice_update_focus_win(ip);
}


static void
choice_yield_kbd_focus(item_public)
    Panel_item	    item_public;
{
    Xv_Window	    focus_win;
    Frame	    frame;
    Item_info      *ip = ITEM_PRIVATE(item_public);

    frame = xv_get(PANEL_PUBLIC(ip->panel), WIN_FRAME);
    focus_win = xv_get(frame, FRAME_FOCUS_WIN);
    xv_set(focus_win,
	   XV_SHOW, FALSE,
	   NULL);
}



/* --------------------  Local Routines  -------------------- */

/* Create or modify the menu for this choice item. */
static void
choice_create_menu(ip)
    Item_info      *ip;
{
    Choice_info    *dp = CHOICE_FROM_ITEM(ip);
    int             i;
    int             nbr_items;

    if (ip->menu) {
	Menu_item mi;

	/*
	 * Destroy and remove all current items. Note: We can't destroy the
	 * menu because PANEL_CHOICE_IMAGES or PANEL_CHOICE_STRINGS can be
	 * set from within a choice stack menu notify proc, in which case the
	 * menu is still in use.
	 */
	nbr_items = (int) xv_get(ip->menu, MENU_NITEMS);
	for (i = nbr_items; i >= 1; i--) {
	    mi = xv_get(ip->menu, MENU_NTH_ITEM, i);
	    xv_set(ip->menu, MENU_REMOVE, i, NULL);
	    xv_destroy(mi);
	}
    } else {
	/* Create choice menu */
	ip->menu = xv_create(XV_SERVER_FROM_WINDOW(PANEL_PUBLIC(ip->panel)),
			     MENU_CHOICE_MENU,
			     MENU_NOTIFY_PROC, choice_do_menu_item,
			     MENU_CLIENT_DATA, ITEM_PUBLIC(ip),
			     XV_INCREMENT_REF_COUNT,
			     NULL);
    }
    if (dp->nrows)
	xv_set(ip->menu, MENU_NROWS, dp->nrows, NULL);
    else if (dp->ncols)
	xv_set(ip->menu, MENU_NCOLS, dp->ncols, NULL);
    choice_images_to_menu_items(ip, dp->choices, dp->mitems, dp->last);
    xv_set(ip->menu,
	   MENU_DEFAULT, choice_number(dp->default_value, dp->last) + 1,
	   NULL);
}


/*
 * Respond to the selected menu item.
 */
static          Xv_opaque
choice_do_menu_item(menu, menu_item)
    Menu            menu;
    Menu_item       menu_item;
{
    Panel_item      item_public = xv_get(menu, MENU_CLIENT_DATA);
    int             menu_value = xv_get(menu_item, MENU_VALUE);
    Event          *event = (Event *) xv_get(menu, MENU_LAST_EVENT);
    Item_info      *ip;

    if (!item_public)
	return 0;

    ip = ITEM_PRIVATE(item_public);

    /* Preview and accept the new choice */
    preview_choice(ip, menu_value, event, FALSE);
    choice_accept_preview(item_public, event);

    return menu_item;
}


/*
 * add items to a menu from a list of images.
 */
static void
choice_images_to_menu_items(ip, images, mitems, last)
    Item_info      *ip;
    Panel_image     images[];
    Menu_item	    mitems[];
    int             last;
{
    register int    i;		/* counter */
    register Panel_image *image;
    int		    color_index;

    for (i = 0; i <= last; i++) {
	image = &(images[i]);
	if (image->color >= 0)
		color_index = image->color;
	else
	    color_index = ip->color_index;
	switch (image_type(image)) {
	  case PIT_STRING:
#ifdef OW_I18N
            mitems[i] = xv_create(NULL, MENUITEM,
                                  MENU_STRING_ITEM_WCS, image_string_wc(image), i,
                                  MENU_COLOR, color_index,
				  MENU_RELEASE,
                                  NULL);
#else
	    mitems[i] = xv_create(XV_ZERO, MENUITEM,
				  MENU_STRING_ITEM, image_string(image), i,
		    		  MENU_COLOR, color_index,
				  MENU_RELEASE,
				  NULL);
#endif /* OW_I18N */
	    xv_set(ip->menu,
		   MENU_APPEND_ITEM, mitems[i],
		   NULL);
	    break;

	  case PIT_SVRIM:
	    mitems[i] = xv_create(XV_ZERO, MENUITEM,
		    		  MENU_IMAGE_ITEM, image_svrim(image), i,
		    		  MENU_COLOR, color_index,
				  MENU_RELEASE,
		    		  NULL);
	    xv_set(ip->menu,
		   MENU_APPEND_ITEM, mitems[i],
		   NULL);
	    break;
	}
    }
}


static void
choice_menu_busy_proc(menu)
    Menu	    menu;
{
    Choice_info    *dp = (Choice_info *) xv_get(menu, XV_KEY_DATA, DATA_HANDLE);
    Xv_Drawable_info *info;
    Item_info	   *ip = ITEM_FROM_CHOICE(dp);
    Xv_Window	    pw;

    PANEL_EACH_PAINT_WINDOW(ip->panel, pw)
	DRAWABLE_INFO_MACRO(pw, info);
	olgx_draw_abbrev_button(ip->value_ginfo, xv_xid(info),
	    dp->choice_rects[0].r_left,
	    dp->choice_rects[0].r_top +
		(dp->choice_rects[0].r_height -
		Abbrev_MenuButton_Height(ip->value_ginfo)) / 2,
	    OLGX_BUSY);
    PANEL_END_EACH_PAINT_WINDOW
}


static void
choice_menu_done_proc(menu, result)
    Menu            menu;
    Xv_opaque       result;
{
    int             old_default_choice, new_default_choice;
    void            (*orig_done_proc) ();	/* original menu-done
						 * procedure */
    Choice_info    *dp = (Choice_info *) xv_get(menu, XV_KEY_DATA, DATA_HANDLE);
    Xv_Drawable_info *info;
    Item_info	   *ip = ITEM_FROM_CHOICE(dp);
    Xv_Window	    pw;

    /* Update the default, if necessary */
    old_default_choice = choice_number(dp->default_value, dp->last);
    new_default_choice = (int) xv_get(menu, MENU_DEFAULT) - 1;
    if (new_default_choice != old_default_choice) {
	REMOVE_CHOICE(dp->default_value, old_default_choice);
	ADD_CHOICE(dp->default_value, new_default_choice);
    }
    if ((dp->display_level == PANEL_NONE ||
	 dp->display_level == PANEL_CURRENT) && !dp->status.display_updated) {
	/* choice_do_menu_item not called: unhighlight the abbreviated
	 * menu button.
	 */
	PANEL_EACH_PAINT_WINDOW(ip->panel, pw)
	    DRAWABLE_INFO_MACRO(pw, info);
	    olgx_draw_abbrev_button(ip->value_ginfo, xv_xid(info),
		dp->choice_rects[0].r_left,
		dp->choice_rects[0].r_top +
		    (dp->choice_rects[0].r_height -
		    Abbrev_MenuButton_Height(ip->value_ginfo)) / 2,
		ip->panel->status.three_d ? OLGX_NORMAL :
		OLGX_ERASE | OLGX_NORMAL);
	PANEL_END_EACH_PAINT_WINDOW
    }

    /* Restore original menu done proc. */
    orig_done_proc = (void (*) ()) xv_get(menu, XV_KEY_DATA, MENU_DONE_PROC);
    xv_set(menu,
	MENU_DONE_PROC, orig_done_proc,
	NULL);

    /* Invoke original menu done proc (if any) */
    if (orig_done_proc)
	(orig_done_proc) (menu, result);

    ip->panel->status.current_item_active = FALSE;
    ip->panel->current=NULL;
}


/* Return the index of the first set bit in value_set */
static int
choice_number(value_set, last_element)
    register unsigned int *value_set;
    register int    last_element;
{
    register int    i;

    EACH_CHOICE(value_set, last_element, i)
	return i;
    return 0;
}


static void
choice_update_focus_win(ip)
    Item_info	   *ip;
{
    Choice_info	   *dp = CHOICE_FROM_ITEM(ip);
    Frame	    frame;
    Rect	    rect;
    int		    x;
    int		    y;

    frame = xv_get(PANEL_PUBLIC(ip->panel), WIN_FRAME);
    rect = dp->choice_rects[dp->focus_choice];
    if (ip->layout == PANEL_HORIZONTAL) {
	xv_set(frame, FRAME_FOCUS_DIRECTION, FRAME_FOCUS_UP, NULL);
	if (dp->display_level == PANEL_CURRENT) {
	    rect.r_width = Abbrev_MenuButton_Width(ip->value_ginfo);
	    y = rect.r_top + (rect.r_height +
		Abbrev_MenuButton_Height(ip->value_ginfo)) / 2;
	} else if (dp->feedback == PANEL_MARKED) {
	    rect.r_width = CheckBox_Width(ip->value_ginfo);
	    y = rect.r_top + (rect.r_height +
		CheckBox_Height(ip->value_ginfo)) / 2;
	} else
	    y = rect.r_top + rect.r_height;
	x = rect.r_left + (rect.r_width - FRAME_FOCUS_UP_WIDTH)/2;
	if (dp->display_level == PANEL_ALL &&
	    dp->feedback != PANEL_MARKED)
	    y -= FRAME_FOCUS_UP_HEIGHT/2;
    } else {
	xv_set(frame, FRAME_FOCUS_DIRECTION, FRAME_FOCUS_RIGHT, NULL);
	x = rect.r_left - FRAME_FOCUS_RIGHT_WIDTH;
	if (dp->display_level == PANEL_ALL &&
	    dp->feedback != PANEL_MARKED)
	    x += FRAME_FOCUS_RIGHT_WIDTH/2;
	y = rect.r_top + (rect.r_height - FRAME_FOCUS_RIGHT_HEIGHT)/2;
    }
    if (x < 0)
	x = 0;
    if (y < 0)
	y = 0;
    panel_show_focus_win(ITEM_PUBLIC(ip), frame, x, y);
}


static unsigned int
choice_value(choose_one, value_set, last_element)
    int             choose_one;
    unsigned int   *value_set;
    int             last_element;
{
    int		    value;

    if (choose_one) {
	value = (choice_number(value_set, last_element));
	if (!IN(value_set, value))
	    return -1;	/* Choggle with no choice set */
	return value;	/* Choice or choggle with a choice set */
    } else
	return value_set[0];  /* Toggle */
}


static int
choice_x_gap(three_d, choose_one)
    int		    three_d;
    int             choose_one;
{
    if (choose_one)
	return (three_d ? 0 : -1);
    else
	return (TOGGLE_X_GAP);
}


static int
choice_y_gap(three_d, choose_one)
    int		    three_d;
    int             choose_one;
{
    if (choose_one)
	return (three_d ? 0 : -1);
    else
	return (TOGGLE_Y_GAP);
}


static
void
compute_nrows_ncols (ip, rows, cols)
    Item_info   *ip;
    int         *rows, *cols;  /* out parameters */
{
    Choice_info *dp = CHOICE_FROM_ITEM (ip);
    /*
	compute the number of rows and columns
    */
    if (!dp->nrows && !dp->ncols)
	if (ip->layout == PANEL_HORIZONTAL) {
	    *rows = 1;
	    *cols = dp->last + 1;

	} else {  /* PANEL_VERTICAL */
	    *cols = 1;
	    *rows = dp->last + 1;
	}

    else if (dp->nrows) {
	*rows = dp->nrows > dp->last + 1 ? dp->last + 1 : dp->nrows;
	*cols = (dp->last + *rows) / *rows;

    } else {  /* user specified number of columns */
	*cols = dp->ncols > dp->last + 1 ? dp->last + 1 : dp->ncols;
	*rows = (dp->last + *cols) / *cols;
    }
}


static int
  find_choice(Item_info *ip, Event *event)
{
    register int    x = event_x(event);	/* locator x */
    register int    y = event_y(event);	/* locator y */
    register Choice_info *dp = CHOICE_FROM_ITEM(ip);
    register int    i;		/* counter */
    Rect            rect;

    switch (dp->display_level) {
      case PANEL_NONE:
      case PANEL_CURRENT:
	/* Select default if pointing to abbreviated button stack image */
	rect = ip->value_rect;
	rect.r_width = Abbrev_MenuButton_Width(ip->value_ginfo);
	if (!rect_includespoint(&rect, x, y))
	    return (NULL_CHOICE);
	return (choice_number(dp->default_value, dp->last));

      case PANEL_ALL:
	/* If not a mouse event, then return the focus choice.
	 * Otherwise, look for choice based on x and y.
	 */
	if (event_id(event) != MS_LEFT && event_id(event) != LOC_DRAG)
	    return dp->focus_choice;
	for (i = 0; i <= dp->last; i++)
	    if (rect_includespoint(&dp->choice_rects[i], x, y)) {
		if (!dp->choose_one && dp->feedback == PANEL_MARKED) {
		    /* Check box: cursor must be over check box */
		    rect.r_left = dp->choice_rects[i].r_left;
		    rect.r_top = dp->choice_rects[i].r_top +
			(dp->choice_rects[i].r_height -
			CheckBox_Height(ip->value_ginfo))/2;
		    rect.r_width = CheckBox_Width(ip->value_ginfo);
		    rect.r_height = CheckBox_Height(ip->value_ginfo);
		    if (rect_includespoint(&rect, x, y))
			return (i);
		    else
			return (NULL_CHOICE);
		} else
		    return (i);
	    }

	return (NULL_CHOICE);

      default:			/* invalid display level */
	return (NULL_CHOICE);
    }
}


/*
 * image_size returns the size of image.  The amount of image above the
 * baseline is also returned.
 */
static struct pr_size
image_size(image, above_baseline, max_width)
    register Panel_image *image;
    register int   *above_baseline;
{
#ifdef OW_I18N
    XFontSet            font_set;
    XFontSetExtents     *font_set_extents;
#else
    XFontStruct		*x_font_info;
#endif /* OW_I18N */
    struct pr_size  size;	/* full size */
    register CHAR  *sp;		/* string version */
    Xv_Font	    font;	/* font for string */
    int			chrwth, chrht;

    switch (image->im_type) {
      case PIT_STRING:
	font = image_font(image);
#ifdef OW_I18N
	sp = image_string_wc(image);
        size = xv_pf_textwidth_wc(wslen(sp), font, sp);
	chrwth = xv_get(font, FONT_COLUMN_WIDTH);
#else
	sp = image_string(image);
	size = xv_pf_textwidth(strlen(sp), font, sp);
	chrwth = xv_get(font, FONT_DEFAULT_CHAR_WIDTH);
#endif /* OW_I18N */
	chrht = xv_get(font, FONT_DEFAULT_CHAR_HEIGHT);
	if (max_width)
	    size.x = max_width + 2 * chrwth;
	else
	{
#ifdef OW_I18N
	    if (wslen(sp))
#else
	    if (strlen(sp))
#endif /* OW_I18N */
	        size.x += 2 * chrwth;
	}
	/*
	 * Use max height of font (ascent + descent).  Leave some vertical
	 * space between bounding box and string.
	 */
	size.y = chrht + OLGX_CHOICE_MARGIN;

	if (*sp)  {
#ifdef OW_I18N
            font_set = (XFontSet)xv_get(font, FONT_SET_ID);
            font_set_extents = XExtentsOfFontSet(font_set);
            *above_baseline = 1 - font_set_extents->max_logical_extent.y;
        }   
        else
            *above_baseline = 0;
        break;
#else
	    x_font_info = (XFontStruct *)xv_get(font, FONT_INFO);
	    *above_baseline = x_font_info->ascent + 1;
	}
	else
	    *above_baseline = 0;
	break;
#endif /* OW_I18N */

      case PIT_SVRIM:
	size = ((Pixrect *)image_svrim(image))->pr_size;
	if (max_width)
	    size.x = max_width + 4;  /* should have OLGX constant when 
					Nataraj provides one ???
					sri - 2/12/91 */	
	*above_baseline = size.y;
	/* Leave space between bounding box and image */
	size.x += OLGX_CHOICE_MARGIN;
	size.y += OLGX_CHOICE_MARGIN;
	break;
    }
    return (size);
}


/*
 * layout_choices lays out the choices in ip.
 */
static void
layout_choices(ip)
    register Item_info *ip;	/* the item */
{
    register Choice_info *dp;	/* choice data */
    register int    i, j, k;    /* counters */
    Panel_image    *image;	/* each choice image */
    struct pr_size  size;	/* each choice size */
    Rect           *rect;	/* each choice rect */
    int             left, top;	/* corner of each choice */
    int             above_baseline;	/* amount above baseline */
    int		    max_height;
    int             max_stringwidth;	/* max width of choice items */
    int             nrows, ncols;
    int             x_gap;
    int             y_gap;

    dp = CHOICE_FROM_ITEM(ip);
    x_gap = choice_x_gap(ip->panel->status.three_d ? TRUE : FALSE,
			 ip->item_type == PANEL_CHOICE_ITEM);
    y_gap = choice_y_gap(ip->panel->status.three_d ? TRUE : FALSE,
			 ip->item_type == PANEL_CHOICE_ITEM);

    compute_nrows_ncols(ip, &nrows, &ncols);

    /* If more than one row, figure out longest choice item */
    max_stringwidth = 0;

    if (nrows > 1 || dp->display_level == PANEL_CURRENT)
	for (i = 0; i <= dp->last; i++) {
	    image = &(dp->choices[i]);
	    switch (image->im_type) {
	      case PIT_STRING:
#ifdef OW_I18N
		size = xv_pf_textwidth_wc(wslen(image->im_value.t.text_wc),
			    image->im_value.t.font, image->im_value.t.text_wc);
#else
		size = xv_pf_textwidth(strlen(image->im_value.t.text),
			    image->im_value.t.font, image->im_value.t.text);
#endif /* OW_I18N */
		if (size.x > max_stringwidth)
		    max_stringwidth = size.x;
		break;

	      case PIT_SVRIM:
		size = ((Pixrect *)image_svrim(image))->pr_size;
		if (size.x > max_stringwidth)
		    max_stringwidth = size.x;
		break;

	    }
	}

    /*
     * Make sure the label is in the right place.  Account for the baseline
     * label adjustment made below.
     */
    panel_fix_label_position(ip);

    /* Initalize the value width & height */
    ip->value_rect.r_width = 0;
    ip->value_rect.r_height = 0;

    if (dp->display_level == PANEL_NONE)
	return;

    top = ip->value_rect.r_top;
    left = ip->value_rect.r_left;

    /* Layout each choice a row or column at a time */
    i = 0;
    while (i <= dp->last) {
	max_height = 0;

	for (j = 0;
	     i <= dp->last &&
	     j < (ip->layout == PANEL_HORIZONTAL ? ncols: nrows);
	     i++, j++) {
	    image = &(dp->choices[i]);
	    rect = &(dp->choice_rects[i]);
	    size = image_size(image, &above_baseline, max_stringwidth);

	    if (dp->display_level == PANEL_CURRENT) {
		size.x += Abbrev_MenuButton_Width(ip->value_ginfo) +
		    ABSTK_X_GAP;
		size.y = MAX(size.y,
			     Abbrev_MenuButton_Height(ip->value_ginfo));
	    }
	    max_height = MAX(max_height, size.y);

	    rect_construct(rect, left, top, size.x, size.y);

	    /* Adjust for the check box, if any */
	    if (dp->feedback == PANEL_MARKED) {
		rect->r_width += CheckBox_Width(ip->value_ginfo) +
		    CHECKBOX_XOFFSET;

		rect->r_height = MAX(rect->r_height,
		    CheckBox_Height(ip->value_ginfo));

		max_height = MAX(max_height, CheckBox_Height(ip->value_ginfo));
	    }

	    if (dp->display_level == PANEL_ALL) {
		if (ip->layout == PANEL_HORIZONTAL) {
		    /* Move over one column */
		    left = rect_right(rect) + 1 + x_gap;
		} else {
		    /* Move down one row */
		    top = rect_bottom(rect) + 1 + y_gap;
		}
	    }

	}

	if (ip->layout == PANEL_HORIZONTAL) {
	    /*
	     * Compute the top of each choice rect before we move
	     * on to the next row
	     */
	    for (k = i - j; k < i; k++) {
		rect = &dp->choice_rects[k];
		rect->r_top = top + max_height - rect->r_height;
	    }

	    /* Move down one row */
	    if (dp->display_level == PANEL_ALL)
		top = rect_bottom(rect) + 1 + y_gap;
	    left = ip->value_rect.r_left;
	} else {
	    /* Move over one column */
	    if (dp->display_level == PANEL_ALL)
		left = rect_right(rect) + 1 + x_gap;
	    top = ip->value_rect.r_top;
	}
	    
    }
}				/* layout_choices */


/*
 * modify_choice modifies the specified choice string or image. If the
 * specified choice does not already exist, the list of choices is extended
 * by adding empty choice strings.
 */

/*      register Item_info *ip;	 the item *\/ */
/*      int             type;    PIT_STRING or PIT_SVRIM *\/ */
/*      int             which_choice; choice to change *\/ */
/*      Xv_opaque       choice_info; new choice string or pixrect *\/ */

static int
modify_choice(register Item_info * ip, int type,
              int which_choice, Xv_opaque choice_info)

{
    register Choice_info *dp = CHOICE_FROM_ITEM(ip);
    Panel_image    *old_choices = dp->choices;
    int             old_last = dp->last;
    Menu_item	   *old_mitems = dp->mitems;
    unsigned int   *old_value = dp->value;
    Rect           *old_choice_rects = dp->choice_rects;
    register int    num_choices, i, old_slot;	/* counters */
    struct pr_size  size;
    int             choice_is_bold = FALSE;	/* boolean */

    /* expand the list if not big enough */
    if (which_choice > dp->last) {
	num_choices = which_choice + 1;
	/* allocate the panel_image[] storage for the choices */
	if ((dp->choices = (Panel_image *)
	   xv_calloc((unsigned int) num_choices, sizeof(Panel_image))) == NULL)
	    return 0;

	/* Allocate the Menu_item storage for the choices */
	if ((dp->mitems = (Menu_item *)
	   xv_calloc((unsigned int) num_choices, sizeof(Menu_item))) == NULL)
	    return 0;

	/* allocate the enclosing rectangles for each choice */
	if ((dp->choice_rects = (Rect *)
	     xv_calloc((unsigned int) num_choices, sizeof(Rect))) == NULL)
	    return 0;

	dp->last = num_choices - 1;	/* last slot used in choices[] */

	/* allocate the value set */
	if ((dp->value = CREATE_SET(dp->last)) == NULL)
	    return 0;

	/* copy the old values */
	COPY_SET(old_value, dp->value, MIN(dp->last, old_last));

	/*
	 * Copy the choices to the allocated storage.
	 */
	for (i = 0; i <= dp->last; i++) {
	    old_slot = (i <= old_last) ? i : old_last;
	    dp->choice_rects[i] = old_choice_rects[old_slot];
	    if (i <= old_last) {
		dp->choices[i] = old_choices[old_slot];
		dp->mitems[i] = old_mitems[old_slot];
	    } else {
		if (dp->feedback == PANEL_MARKED)
		    choice_is_bold = dp->choices_bold;
		dp->choices[i].color = old_choices[old_slot].color;
#ifdef OW_I18N
                size = panel_make_image(is_string(&old_choices[old_slot]) ?
                    image_font(&old_choices[old_slot]) : ip->panel->std_font,
                    &dp->choices[i], PIT_STRING,
		    panel_strsave_wc(_xv_null_string_wc),
                    choice_is_bold, FALSE);
#else
		size = panel_make_image(ip->value_font, &dp->choices[i],
					PIT_STRING, (Xv_opaque) "",
		    			choice_is_bold, FALSE);
#endif /* OW_I18N */
		if (dp->display_level == PANEL_NONE) {
		    dp->choice_rects[i].r_width =
			Abbrev_MenuButton_Width(ip->value_ginfo);
		    dp->choice_rects[i].r_height =
			Abbrev_MenuButton_Height(ip->value_ginfo);
		} else {
		    dp->choice_rects[i].r_width = size.x;
		    dp->choice_rects[i].r_height = size.y+OLGX_CHOICE_MARGIN;
		}
	    }
	}

    }
    if (dp->feedback == PANEL_MARKED)
	choice_is_bold = dp->choices_bold;
    dp->choices[which_choice].color = -1;  /* use foreground color */
    size = panel_make_image(ip->value_font, &dp->choices[which_choice],
			    type, choice_info, choice_is_bold, FALSE);
    if (dp->display_level == PANEL_NONE) {
	dp->choice_rects[i].r_width =
	    Abbrev_MenuButton_Width(ip->value_ginfo);
	dp->choice_rects[i].r_height =
	    Abbrev_MenuButton_Height(ip->value_ginfo);
    } else {
	dp->choice_rects[which_choice].r_width = size.x;
	dp->choice_rects[which_choice].r_height = size.y + OLGX_CHOICE_MARGIN;
    }

    if (dp->last != old_last) {
	/* now free the old info */
	/* if new array is smaller, free the unused strings */
	/* in any case, free old_choices */
	(void) panel_free_choices(old_choices, (int) (dp->last + 1), old_last);
	free((char *) old_mitems);
	free((char *) old_choice_rects);
	free((char *) old_value);
    }
    return 1;
}				/* modify_choice */


/*
 * move_specified moves the specified choices marks in ip. If any choices are
 * moved, TRUE is returned.
 */
static int
  move_specified(register Item_info *ip, register Attr_avlist avlist)
{
    register Choice_info *dp = CHOICE_FROM_ITEM(ip);
    register Attr_attribute attr;
    register int    which_choice;	/* index of current choice */
    register int    i;		/* counter */
    int            *xs;		/* choice x coordinates */
    int            *ys;		/* choice y coordinates */
    int             moved = FALSE;	/* TRUE if moved */

    while (attr = *avlist++) {
	switch (attr) {
	  case PANEL_CHOICE_X:
	    i = (int) *avlist++;
	    dp->choice_rects[i].r_left = (int) *avlist++;
	    dp->status.choice_fixed = TRUE;
	    ip->flags |= VALUE_X_FIXED;
	    moved = TRUE;
	    break;

	  case PANEL_CHOICE_Y:
	    i = (int) *avlist++;
	    dp->choice_rects[i].r_top = (int) *avlist++;
	    dp->status.choice_fixed = TRUE;
	    ip->flags |= VALUE_Y_FIXED;
	    moved = TRUE;
	    break;

	  case PANEL_MARK_X:
	  case PANEL_MARK_Y:
	    /* SunView1 compatibility attributes: not used */
	    avlist++;
	    avlist++;
	    break;

	  case PANEL_CHOICE_XS:
	    xs = (int *) avlist;
	    dp->status.choice_fixed = TRUE;
	    ip->flags |= VALUE_X_FIXED;
	    if (xs[0])
		for (which_choice = i = 0; which_choice <= dp->last;
		     which_choice++, i += xs[i + 1] ? 1 : 0)
		    dp->choice_rects[which_choice].r_left = xs[i];
	    while (*avlist++);
	    moved = TRUE;
	    break;

	  case PANEL_CHOICE_YS:
	    ys = (int *) avlist;
	    dp->status.choice_fixed = TRUE;
	    ip->flags |= VALUE_Y_FIXED;
	    if (ys[0])
		for (which_choice = i = 0; which_choice <= dp->last;
		     which_choice++, i += ys[i + 1] ? 1 : 0)
		    dp->choice_rects[which_choice].r_top = ys[i];
	    while (*avlist++);
	    moved = TRUE;
	    break;

	  case PANEL_MARK_XS:
	  case PANEL_MARK_YS:
	    /* SunView1 compatibility attributes: not used */
	    while (*avlist++);
	    break;

	  default:
	    /* skip past what we don't care about */
	    avlist = attr_skip(attr, avlist);
	    break;
	}
    }
    return moved;
}				/* move_specified */


/*
 * paint the choice which_choice.  The off mark is drawn if dp->feedback is
 * PANEL_MARKED.
 */
static void
paint_choice(panel, ip, dp, which_choice, selected)
    Panel_info     *panel;
    Item_info      *ip;
    register Choice_info *dp;
    register int    which_choice;
    int             selected;
{
    int		    color_index;
    Display        *display;
    Panel_image    *image;
    Xv_Drawable_info *info;
    Xv_opaque	    label;
    int             left;
    Pixlabel	    pixlabel;
    Xv_Window       pw;
    Rect           *rect;
    int		    save_black;
    int		    state;
    int             top;
    Drawable        xid;


    if (selected) {
	if (dp->feedback == PANEL_MARKED)
	    state = OLGX_CHECKED;
	else
	    state = OLGX_INVOKED;
    } else if (panel->status.three_d)
	state = OLGX_NORMAL;
    else
	state = OLGX_NORMAL | OLGX_ERASE;
    if (inactive(ip))
	state |= OLGX_INACTIVE;

    if (dp->choices[which_choice].color >= 0)
	color_index = dp->choices[which_choice].color;
    else
	color_index = ip->color_index;
    if (color_index >= 0)
	save_black = olgx_get_single_color(ip->value_ginfo, OLGX_BLACK);

    rect = &dp->choice_rects[which_choice];
    left = rect->r_left;

    if (dp->feedback == PANEL_MARKED) {
	top = rect->r_top + (rect->r_height - CheckBox_Height(ip->value_ginfo))
	/ 2;
	PANEL_EACH_PAINT_WINDOW(panel, pw)
	    DRAWABLE_INFO_MACRO(pw, info);
	    if (color_index >= 0) {
		olgx_set_single_color(ip->value_ginfo, OLGX_BLACK,
				      xv_get(xv_cms(info), CMS_PIXEL,
				      color_index), OLGX_SPECIAL);
	    }
	    olgx_draw_check_box(ip->value_ginfo, xv_xid(info), left, top,
				state);
	PANEL_END_EACH_PAINT_WINDOW
	left += CheckBox_Width(ip->value_ginfo) + CHECKBOX_XOFFSET;
    }

    /* draw the choice image */
    image = &dp->choices[which_choice];

    if (dp->display_level == PANEL_NONE ||
	dp->display_level == PANEL_CURRENT) {
	/* Draw the abbreviated button stack */
	PANEL_EACH_PAINT_WINDOW(ip->panel, pw)
	    DRAWABLE_INFO_MACRO(pw, info);
	    olgx_draw_abbrev_button(ip->value_ginfo, xv_xid(info),
		left, rect->r_top + (rect->r_height -
		Abbrev_MenuButton_Height(ip->value_ginfo)) / 2, state);
	PANEL_END_EACH_PAINT_WINDOW
    }
	
    switch (image->im_type) {
      case PIT_STRING:
#ifdef OW_I18N
	state |= OLGX_LABEL_IS_WCS;
	label = (Xv_opaque) image_string_wc(image);
#else
	label = (Xv_opaque) image_string(image);
#endif /* OW_I18N */
	if (dp->feedback == PANEL_MARKED ||
	    dp->display_level == PANEL_CURRENT) {
	    int		chrht, chrwth;
	    top = rect->r_top + panel_fonthome(image_font(image));
	    chrht = xv_get(image_font(image), FONT_DEFAULT_CHAR_HEIGHT);
#ifdef OW_I18N
	    chrwth = xv_get(image_font(image), FONT_COLUMN_WIDTH);
#else
	    chrwth = xv_get(image_font(image), FONT_DEFAULT_CHAR_WIDTH);
#endif /* OW_I18N */
	    top += (rect->r_height - chrht) / 2;
	    left += chrwth;
	    if (dp->display_level == PANEL_CURRENT)
		/* Paint text to the right of the abbreviated button stack
		 * image.
		 */
		left += Abbrev_MenuButton_Width(ip->value_ginfo) +
		    ABSTK_X_GAP;
	    PANEL_EACH_PAINT_WINDOW(panel, pw)
		DRAWABLE_INFO_MACRO(pw, info);
#ifdef OW_I18N
		panel_paint_text(pw, ip->value_fontset_id, color_index,
				 left, top, image_string_wc(image));
#else
		panel_paint_text(pw, ip->value_font_xid, color_index,
				 left, top, image_string(image));
#endif /* OW_I18N */
	    PANEL_END_EACH_PAINT_WINDOW
	}
	break;

      case PIT_SVRIM:
	state |= OLGX_LABEL_IS_PIXMAP;
	pixlabel.pixmap = (XID) xv_get(image_svrim(image), XV_XID);
	pixlabel.width = ((Pixrect *)image_svrim(image))->pr_width;
	pixlabel.height = ((Pixrect *)image_svrim(image))->pr_height;
	label = (Xv_opaque) &pixlabel;
	if (dp->feedback == PANEL_MARKED ||
	    dp->display_level == PANEL_CURRENT) {
	    left += OLGX_CHOICE_MARGIN/2;
	    top = rect->r_top;
	    if (dp->display_level == PANEL_CURRENT) {
		left += Abbrev_MenuButton_Width(ip->value_ginfo) +
		    ABSTK_X_GAP;
		top += (rect->r_height -
		    ((Pixrect *)image_svrim(image))->pr_height)/2;
	    } else
		top += OLGX_CHOICE_MARGIN/2;
	    PANEL_EACH_PAINT_WINDOW(panel, pw)
		panel_paint_svrim(pw, (Pixrect *) image_svrim(image), left, top,
				  color_index, (Pixrect *)NULL);
	    PANEL_END_EACH_PAINT_WINDOW
	}
	break;
    }

    PANEL_EACH_PAINT_WINDOW(panel, pw)
	DRAWABLE_INFO_MACRO(pw, info);
	if (color_index >= 0) {
	    olgx_set_single_color(ip->value_ginfo, OLGX_BLACK,
				  xv_get(xv_cms(info), CMS_PIXEL,
				  color_index), OLGX_SPECIAL);
	}
	if (dp->display_level != PANEL_NONE &&
	    dp->display_level != PANEL_CURRENT &&
	    dp->feedback != PANEL_MARKED) {
	    olgx_draw_choice_item(ip->value_ginfo, xv_xid(info),
		rect->r_left, rect->r_top, rect->r_width, rect->r_height,
		label, state);
	}

	if ((dp->feedback == PANEL_MARKED ||
	     dp->display_level == PANEL_NONE ||
	     dp->display_level == PANEL_CURRENT)
	    && inactive(ip)) {
	    Xv_Screen      screen;
	    GC             *gc_list;

	    screen = xv_screen(info);
	    gc_list = (GC *)xv_get(screen, SCREEN_OLGC_LIST, pw);
	    display = xv_display(info);
	    xid = xv_xid(info);
	    screen_adjust_gc_color(pw, SCREEN_INACTIVE_GC);
	    XFillRectangle(display, xid,
			   gc_list[SCREEN_INACTIVE_GC],
			   rect->r_left, rect->r_top,
			   rect->r_width, rect->r_height);
	}
    PANEL_END_EACH_PAINT_WINDOW

    if (color_index >= 0)
	olgx_set_single_color(ip->value_ginfo, OLGX_BLACK, save_black,
			      OLGX_SPECIAL);
}


static void
preview_choice(ip, new, event, paint)
    Item_info      *ip;
    int             new;	/* new choice # to preview */
    Event          *event;
    int		    paint;	/* TRUE or FALSE.
				 * If FALSE, new must != NULL_CHOICE */
{
    register Choice_info *dp = CHOICE_FROM_ITEM(ip);
    int		    choice_nbr;
    int             new_is_on;

    /* no change */
    if (new == dp->current)
	return;

    /* if no new choice cancel the current choice & restore the value */
    if (new == NULL_CHOICE) {
	choice_cancel_preview(ITEM_PUBLIC(ip), event);
	return;
    }
    new_is_on = IN(dp->value, new);
    if (paint) {
	switch (dp->choose_one) {
	  case TRUE:
	    /* if no current, un-mark the actual marked choice */
	    if (dp->current == NULL_CHOICE) {
		choice_nbr = choice_number(dp->value, dp->last);
		if (choice_nbr || IN(dp->value, 0))
		    /* There is a choice selected */
		    update_display(ip, choice_nbr, FALSE, TRUE);
	    } else
		update_display(ip, dp->current, FALSE, TRUE);

	    /* mark the new choice */
	    update_display(ip, new, dp->choose_none ? !new_is_on : TRUE, TRUE);
	    break;

	  case FALSE:
	    /* restore the current choice */
	    update_display(ip, dp->current, dp->actual,
			   dp->feedback != PANEL_MARKED);

	    /* toggle the mark for new */
	    update_display(ip, new, !new_is_on, TRUE);
	    break;
	}
    }
    dp->current = new;
    dp->actual = new_is_on;
}


/*
 * re_alloc_choices allocates dp->choices from choices.  The old info is
 * reused and then freed.
 */
static int
  re_alloc_choices(register Item_info *ip, int type, Xv_opaque choices[])
     /* register Item_info *ip;	the item */
     /* int             type;	PIT_STRING or PIT_SVRIM */
     /* Xv_opaque       choices[];	each choice */
{
    register Choice_info *dp = CHOICE_FROM_ITEM(ip);
    Panel_image    *old_choices = dp->choices;
    int             old_last = dp->last;
    Menu_item	   *old_mitems = dp->mitems;
    unsigned int   *old_value = dp->value;
    unsigned int   *old_default_value = dp->default_value;
    Rect           *old_choice_rects = dp->choice_rects;
    register int    num_choices, i, old_slot;	/* counters */
    struct pr_size  size;
    int             choice_is_bold = FALSE;	/* boolean */

    /* count the # of choices */
    for (num_choices = 0; choices[num_choices]; num_choices++);

    /* allocate the panel_image[] storage for the choices */
    if ((dp->choices = (Panel_image *)
	 xv_calloc((unsigned int) num_choices, sizeof(Panel_image))) == NULL)
	return 0;

    /* Allocate the Menu_item storage for each choice */
    if ((dp->mitems = (Menu_item *)
	 xv_calloc((unsigned int) num_choices, sizeof(Menu_item))) == NULL)
	return 0;

    /* allocate the enclosing rectangles for each choice */
    if ((dp->choice_rects = (Rect *)
	 xv_calloc((unsigned int) num_choices, sizeof(Rect))) == NULL)
	return 0;

    dp->last = num_choices - 1;	/* last slot used in base[] */

    /* allocate the value set */
    if ((dp->value = CREATE_SET(dp->last)) == NULL)
	return 0;

    /* copy the old values */
    COPY_SET(old_value, dp->value, MIN(dp->last, old_last));

    /* allocate the default value set */
    if ((dp->default_value = CREATE_SET(dp->last)) == NULL)
	return 0;

    /* copy the old default values */
    COPY_SET(old_default_value, dp->default_value, MIN(dp->last, old_last));

    /*
     * Copy the choices to the allocated storage.
     */
    for (i = 0; i <= dp->last; i++) {
	old_slot = (i <= old_last) ? i : old_last;
	dp->choice_rects[i] = old_choice_rects[old_slot];
	dp->choices[i].color = old_choices[old_slot].color;
	dp->mitems[i] = old_mitems[old_slot];
	if (dp->feedback == PANEL_MARKED)
	    choice_is_bold = dp->choices_bold;
	size = panel_make_image(ip->value_font, &dp->choices[i], type,
				choices[i], choice_is_bold, FALSE);
	if (dp->display_level == PANEL_NONE) {
	    dp->choice_rects[i].r_width =
		Abbrev_MenuButton_Width(ip->value_ginfo);
	    dp->choice_rects[i].r_height =
		Abbrev_MenuButton_Height(ip->value_ginfo);
	} else {
	    dp->choice_rects[i].r_width = size.x;
	    dp->choice_rects[i].r_height = size.y + OLGX_CHOICE_MARGIN;
	}
    }

    /*
     * if there are fewer choices now, and this is not a toggle item, make
     * sure the value is <= the number of the last choice.
     */
    if (dp->choose_one && (dp->last < old_last)) {
	if (choice_number(old_value, old_last) > dp->last) {
	    CLEAR_SET(dp->value, dp->last);
	    ADD_CHOICE(dp->value, dp->last);
	}
	if (choice_number(old_default_value, old_last) > dp->last) {
	    CLEAR_SET(dp->default_value, dp->last);
	    ADD_CHOICE(dp->default_value, dp->last);
	}
    }
    /* now free the old info */
    (void) panel_free_choices(old_choices, 0, old_last);
    free((char *) old_mitems);
    free((char *) old_choice_rects);
    free((char *) old_value);
    free((char *) old_default_value);

    return 1;
}				/* re_alloc_choices */


/*
 * update_display updates the display to suggest or un-suggest which_choice
 * depending on the value of on.
 */
static void
update_display(ip, which_choice, on, preview)
    register Item_info *ip;
    register int    which_choice;
    int             on;	/* not used if display level is PANEL_NONE or
			   PANEL_CURRENT */
    int		    preview;
{
    register Choice_info *dp = CHOICE_FROM_ITEM(ip);
    Xv_Drawable_info *info;
    Xv_Window       pw;
    Rect            rect;
    int		    save_black;
    int		    state;
    int		    color_index;

    if (dp->display_level == PANEL_NONE ||
	dp->display_level == PANEL_CURRENT) {
	if (which_choice == NULL_CHOICE) {
	    PANEL_EACH_PAINT_WINDOW(ip->panel, pw)
		DRAWABLE_INFO_MACRO(pw, info);
		olgx_draw_abbrev_button(ip->value_ginfo, xv_xid(info),
		    dp->choice_rects[0].r_left,
		    dp->choice_rects[0].r_top +
			(dp->choice_rects[0].r_height -
			Abbrev_MenuButton_Height(ip->value_ginfo)) / 2,
		    preview ? OLGX_INVOKED :
			ip->panel->status.three_d ? OLGX_NORMAL :
			    OLGX_ERASE | OLGX_NORMAL);
	    PANEL_END_EACH_PAINT_WINDOW
	    return;
	}
	rect = ip->value_rect;
	rect.r_left += Abbrev_MenuButton_Width(ip->value_ginfo) + ABSTK_X_GAP;
	rect.r_width -= Abbrev_MenuButton_Width(ip->value_ginfo) + ABSTK_X_GAP;
	panel_clear_rect(ip->panel, rect);
	paint_choice(ip->panel, ip, dp, which_choice, preview);
	dp->status.display_updated = TRUE;
	return;
    }

    if (dp->display_level == PANEL_NONE || which_choice == NULL_CHOICE)
	return;

    if (dp->choices[which_choice].color >= 0)
         color_index = dp->choices[which_choice].color;
    else
         color_index = ip->color_index;


    if (on) {
	/* turn the choice on */

	switch (dp->feedback) {
	  case PANEL_INVERTED:
	    paint_choice(ip->panel, ip, dp, which_choice, TRUE);
	    break;

	  case PANEL_MARKED:
	    state = OLGX_CHECKED;
	    if (preview)
		state |= OLGX_INVOKED;
	    if (inactive(ip))
		state |= OLGX_INACTIVE;
	    if (color_index >= 0)
		save_black = olgx_get_single_color(ip->value_ginfo,
						   OLGX_BLACK);
	    PANEL_EACH_PAINT_WINDOW(ip->panel, pw)
		DRAWABLE_INFO_MACRO(pw, info);
		if (color_index >= 0) {
		    olgx_set_single_color(ip->value_ginfo, OLGX_BLACK,
					  xv_get(xv_cms(info), CMS_PIXEL,
					  color_index), OLGX_SPECIAL);
		}
		olgx_draw_check_box(ip->value_ginfo, xv_xid(info),
		    dp->choice_rects[which_choice].r_left,
		    dp->choice_rects[which_choice].r_top +
			(dp->choice_rects[which_choice].r_height -
			CheckBox_Height(ip->value_ginfo))/2,
		    state);
	    PANEL_END_EACH_PAINT_WINDOW
	    if (color_index >= 0)
		olgx_set_single_color(ip->value_ginfo, OLGX_BLACK, save_black,
				      OLGX_SPECIAL);
	    break;

	  case PANEL_NONE:
	    break;
	}
    } else {
	/* turn the choice off */

	/* un-mark/invert old */
	switch (dp->feedback) {
	  case PANEL_INVERTED:
	    paint_choice(ip->panel, ip, dp, which_choice, FALSE);
	    break;

	  case PANEL_MARKED:
	    if (preview)
		state = OLGX_INVOKED;
	    else
		state = OLGX_NORMAL;
	    if (inactive(ip))
		state |= OLGX_INACTIVE;
	    if (color_index >= 0)
		save_black = olgx_get_single_color(ip->value_ginfo,
						   OLGX_BLACK);
	    PANEL_EACH_PAINT_WINDOW(ip->panel, pw)
		DRAWABLE_INFO_MACRO(pw, info);
		if (color_index >= 0) {
		    olgx_set_single_color(ip->value_ginfo, OLGX_BLACK,
					  xv_get(xv_cms(info), CMS_PIXEL,
					  color_index), OLGX_SPECIAL);
		}
		olgx_draw_check_box(ip->value_ginfo, xv_xid(info),
		    dp->choice_rects[which_choice].r_left,
		    dp->choice_rects[which_choice].r_top +
			(dp->choice_rects[which_choice].r_height -
			CheckBox_Height(ip->value_ginfo))/2,
		    state);
	    PANEL_END_EACH_PAINT_WINDOW
	    if (color_index >= 0)
		olgx_set_single_color(ip->value_ginfo, OLGX_BLACK, save_black,
				      OLGX_SPECIAL);
	    break;

	  case PANEL_NONE:
	    break;
	}
    }
}


static void
update_item_rect(ip)
    Item_info      *ip;
{
    update_value_rect(ip);
    ip->rect = panel_enclosing_rect(&ip->label_rect, &ip->value_rect);
}


/*
 * compute the width & height of the value rect to enclose all of the choices
 * & marks.
 */
static void
update_value_rect(ip)
    Item_info      *ip;
{
    register Choice_info *dp = CHOICE_FROM_ITEM(ip);
    register int    i;
    Rect            rect;

    if (dp->display_level == PANEL_NONE) {
	ip->value_rect.r_width = Abbrev_MenuButton_Width(ip->value_ginfo);
	ip->value_rect.r_height = Abbrev_MenuButton_Height(ip->value_ginfo);
    } else {
	rect = ip->value_rect;
	rect.r_width = rect.r_height = 0;
	for (i = 0; i <= dp->last; i++)
	    rect = panel_enclosing_rect(&rect, &dp->choice_rects[i]);
	ip->value_rect = rect;
    }
}				/* update_value_rect */
