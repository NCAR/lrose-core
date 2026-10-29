#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)luc.c 50.49 93/06/28";
#endif
#endif

/*
 *      (c) Copyright 1990 Sun Microsystems, Inc. Sun design patents
 *      pending in the U.S. and foreign countries. See LEGAL NOTICE
 *      file for terms of the license.
 */
/*LINTLIBRARY*/

/*
 * Using CANVAS to drew the contents of the lookup choice window.
 * This is much faster than using panelsw.
 */
#define USE_CANVAS

#ifndef USE_CANVAS
/*
 * When waiting for the {Map, Unmap}Notify event after issuing the map
 * request, this code will put the {Map, Unmap}Notify event to the top
 * of event queue.  This will elimate the problem with waiting in the
 * XPeekIfEvent forever.  But this code manipulating the event queue,
 * therefor there are might be side effect on this code.
 */
#define	PUT_BACK_EVENT
#endif /* ! USE_CANVAS */

#ifdef DEBUG
#    define	dprintf(a)	fprintf a
#else
#    define	dprintf(a)
#endif

#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#include <xview_private/i18n_impl.h>
#include <xview_private/portable.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

#include <xview/xview.h>
#include <xview/frame.h>
#ifdef USE_CANVAS
#include <xview/canvas.h>
#else
#include <xview/panel.h>
#endif
#include <xview/xv_xrect.h>
#include <xview/font.h>
#include <xview/defaults.h>
#ifdef USE_CANVAS
#include <olgx/olgx.h>
#endif

#include <xview_private/luc_impl.h>


static void		 luc_arrow_handle();
static void		 luc_do_negotiate();
static void		 luc_get_defaults();
static			 luc_get_draw_info();
static LucFrameInfo	*luc_get_frame();
static Frame		 luc_get_p_frame();
static void		 luc_get_win_position();
static LucInfo		*luc_info_create();
static			 luc_match_label();
static			 luc_redraw();
static void		 luc_repaint_proc();
static void		 luc_win_event_proc();
#ifndef USE_CANVAS
static			 luc_draw_candidates();
static Bool		 luc_check_map();
static Bool		 luc_check_unmap();
static Bool		 luc_syncup();
#ifdef MOUSE_SUPPORT
static void		 ignore_mouse_handler();  /* ignore mouse selection handler */
static void		 luc_mouse_handler(); /* mouse selection handler */
#endif
#endif /* ! USE_CANVAS */

static LucDefaults	 luc_defaults;
static LucFrameInfo	*luc_frame_info = NULL;
static Bool		 luc_start_is_called = FALSE;

static const wchar_t		 alphabet[] =
{ ' ', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l',
  'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y',
  'z', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L',
  'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'};
static const wchar_t		 number[] =
{ '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};

typedef struct _luc_label_table {
	int	 ndecimal;
	const wchar_t	*label;
	int	 alphabet_adjust;
	int	 numeric_adjust;
} LucLabelTable;
/*
 * Warning: I cann't put "const" to following structure, since it will
 * require the relocated information (which can not handled by shared
 * library.
 */
static LucLabelTable	 luc_label_table [] = {
{ /* LUC_ALPHABETIC */		52,	alphabet,	1,	0},
{ /* LUC_NUMERIC */		10,	number,		0,	1},
{ /* LUC_ALPHABETIC_LOWER */	26,	alphabet,	1,	0},
{ /* LUC_ALPHABETIC_UPPER */	26,	&alphabet[26],	1,	0}
};


/*
 * This is a LookupChoice callback package.
 */

void
lookup_choices_start(ic, client_data, cb_data)
    XIC					 ic;
    XPointer				 client_data;
    XIMLookupStartCallbackStruct	*cb_data;
{

/* Caution!;
 *    callback_data->WhoIsMaster must be XIMIsMaster at the 2nd call
 *    Also, if callback_data->WhoIsMaster was XIMIsMaster at 1st call 
 *    XIM canot call lookup_choices_start in the same session ,because 
 *    in this case CB has to obey XIM , there remains no room
 *    for further negotiation.
 */

    register LucInfo    *info;
    Display		*dpy;
    LucFrameInfo	*lfi;


#ifdef DEBUG1
    malloc_debug(2);
#endif
	
    dprintf((stderr, "luc_start\n"));
    /*
     * check to see if this is the 2nd call (before calling 'done'
     * proc -- negotiateion only.
     */
    if (luc_start_is_called) {
	info = LUC_UDATA_INFO(client_data);
	luc_do_negotiate(info, cb_data);
	return;
    }

    if ((info = luc_info_create(client_data)) == NULL) {
	xv_error(XV_ZERO,
		 ERROR_STRING,
		 XV_MSG("luc: out of memory"),
		 NULL);
	return;
    }

    info->ic = ic;
    dpy = (Display *) XDisplayOfIM((XIM) XIMOfIC(ic));
    info->start = cb_data;
    info->keybuf_counter = 0;

    /* create xview objects */
    info->frame_info = lfi = luc_get_frame(dpy, cb_data);
    if (lfi == NULL)
        goto err_return;

    luc_do_negotiate(info, cb_data);

#ifdef USE_CANVAS
    if (lfi->canvas == NULL)
	goto err_return;
    (void) xv_set(lfi->canvas,
			XV_KEY_DATA, LUC_KEY,	info,
			NULL);
#else /* USE_CANVAS */
    if (lfi->panel == NULL)
	goto err_return;

    /*
     * Disable mouse event in lookup choice region.  mouse selection
     * of the kana -> kanji conversion will be supported in the future
     * At that time replace PANEL_EVENT_PROC with PANEL_NOTIFY_PROC,
     * and replace ignore_mouse_handler with luc_mouse_handler.  So
     * capture events here with ignore_mouse_handler
     */

    lfi->panel_item = (Panel_item) xv_create(lfi->panel, PANEL_CHOICE,
				PANEL_CHOOSE_ONE,       TRUE,
				PANEL_EVENT_PROC,	ignore_mouse_handler,
#ifdef MOUSE_SUPPORT
				XV_KEY_DATA,            LUC_KEY, info,
#endif
				NULL);
    if (lfi->panel_item == NULL)
	    goto err_return;
#endif /* USE_CANVAS */
 
    /* set up some initial values */
    info->state = LUC_START_CALLED;
    info->first = 0;    /* index of the 1st candidate */

    info->WhoIsMaster = cb_data->WhoIsMaster;

    luc_start_is_called = TRUE;

    return;


err_return:     /* xv-object creation failure */
    xv_error( NULL,
	ERROR_STRING,
	XV_MSG("luc: failed to create xv_object"),
	0 );
    return;
}


void
lookup_choices_draw(ic, client_data, cb_data)
    XIC					 ic;
    XPointer				 client_data;
    XIMLookupDrawCallbackStruct		*cb_data;
{
    register LucInfo    *info;
    Window		 xwin;
    LucFrameInfo	*lfi;


    dprintf((stderr, "luc_draw\n"));
    info = LUC_UDATA_INFO(client_data);
    info->draw = cb_data;
    lfi = info->frame_info;

#ifndef USE_CANVAS
    if (info->state  == LUC_DRAW_CALLED) {
	(void) xv_set(lfi->panel, XV_SHOW, FALSE, NULL);
        xv_destroy(lfi->panel_item);
        lfi->panel_item = (Panel_item) xv_create(lfi->panel, PANEL_CHOICE,
                        PANEL_CHOOSE_ONE,       TRUE,
                        PANEL_EVENT_PROC,       ignore_mouse_handler,
#ifdef MOUSE_SUPPORT
                        XV_KEY_DATA,            LUC_KEY, info,

#endif
                        NULL);
    }
#endif /* ! USE_CANVAS */

    if (luc_get_draw_info(info) < 0)
	return;

#ifdef USE_CANVAS
    /*
     * Setting the size and mapping has to be separate xv_set call,
     * otherwise, we'll see the screen flush.
     */
    if (lfi->f_width != info->f_width || lfi->f_height != info->f_height) {
	lfi->f_should_resize = TRUE;
	lfi->f_width = info->f_width;
	lfi->f_height = info->f_height;
    }
    if (luc_defaults.window_should_fit == TRUE
     && (lfi->f_x != info->f_x || lfi->f_y != info->f_y)) {
        lfi->f_should_resize = TRUE;
	lfi->f_x = info->f_x;
	lfi->f_y = info->f_y;
    }
    if (info->state  == LUC_DRAW_CALLED) {
	XClearWindow(lfi->dpy, lfi->canvas_xwin);
	luc_repaint_proc(lfi->canvas, (Xv_Window)NULL,
			 lfi->dpy, lfi->canvas_xwin, (Xv_xrectlist *)NULL);
    } else {
	switch (lfi->lfs) {
	   default:
		if (lfi->f_should_resize == TRUE) {
		    dprintf((stderr, "draw: being unmapped, req to resize/map\n"));
		    lfi->lfs = LFS_RESIZE_REQUESTED;
		} else {
		    dprintf((stderr, "draw: being unmapped, req to map\n"));
		    lfi->lfs = LFS_MAP_REQUESTED;
		}
		break;

	    case LFS_UNMAPPED:
		if (lfi->f_should_resize == TRUE) {
			xv_set (lfi->frame,
				XV_WIDTH,	lfi->f_width,
				XV_HEIGHT,	lfi->f_height,
				XV_X,	lfi->f_x,
				XV_Y,	lfi->f_y,
				NULL);
			lfi->f_should_resize = FALSE;
			dprintf((stderr, "draw: being resized, req to map\n"));
		}
		(void) xv_set(lfi->frame,
			      XV_SHOW,	TRUE,
			      NULL);
		lfi->lfs = LFS_BEING_MAPPED;
		dprintf((stderr, "draw: being mapped\n"));
		break;
	}
    }

#else /* USE_CANVAS */

    luc_draw_candidates(info);          /* draw candidates     */

    window_fit(lfi->panel);
/*
    window_fit( info->frame );
*/

    if (info->state  == LUC_DRAW_CALLED) {
	(void) xv_set(lfi->panel, XV_SHOW, TRUE, NULL);
        window_fit(lfi->frame);
    } else {
        window_fit(lfi->frame);
	(void) xv_set(lfi->frame,
		      XV_SHOW,		TRUE,
		      NULL);
    }

    (void) xv_set(info->frame_info->panel_item, PANEL_VALUE, 0, NULL);
#endif /* USE_CANVAS */

    info->current = 1;   /* current choice nember */
    info->state = LUC_DRAW_CALLED;

    if (luc_defaults.keygrab) {
    	xwin = (Window) xv_get(lfi->p_obj, XV_XID);
    	XGrabKeyboard(lfi->dpy, xwin, True,
		      GrabModeAsync, GrabModeAsync, CurrentTime);
    }


    return;
}


void
lookup_choices_process(ic, client_data, cb_data)
    XIC                     ic;
    XPointer                client_data;
    XIMLookupProcessCallbackStruct  *cb_data;
{
    LucInfo			*info;
    XKeyEvent			*event;
    char			 buf[LUC_LOOKUPSTRING_BUFLEN + 1];
    int				 len;
    KeySym			 keysym;
    XComposeStatus 		 status;
    int				 find;
    int				 i;


    dprintf((stderr, "luc_process\n"));
    info = LUC_UDATA_INFO(client_data);
    event = (XKeyEvent *)cb_data->event;

    if (info->keybuf_counter <= 0
     || info->keybuf_counter > LUC_LOOKUPSTRING_BUFLEN) {
	XV_BZERO((char *)info->keybuf, sizeof (info->keybuf));
	info->keybuf_counter = 0;
    }
	
    find = 0;
    len = XLookupString(event, buf, LUC_LOOKUPSTRING_BUFLEN, &keysym, &status);
    switch (keysym) {
	case XK_Left:
	case XK_Right:
	case XK_Up:
	case XK_Down:
	    luc_arrow_handle(info, keysym);
	    find = XIM_UNDETERMINED;
	    info->keybuf_counter = 0;
	    break;

	default:
	    if (event->state & ControlMask) {
		if ((find = luc_redraw(info, keysym)) != 0) {
		    info->keybuf_counter = 0;
		    break;
		}
	    }
	    if (len <= 0)
		break;
	    buf[len] = 0;
	    len = mbstowcs(&info->keybuf[info->keybuf_counter], buf,
			   sizeof (info->keybuf) - info->keybuf_counter);
	    if (len <= 0)
		break;
	    i = info->keybuf_counter;
	    info->keybuf_counter += len;
	    info->keybuf[info->keybuf_counter] = 0;
	    dprintf((stderr, "process try [%ws]: ", info->keybuf));
	    find = luc_match_label(info, 0);
	    if (find == XIM_UNKNOWN_KEYSYM && i > 0) {
		dprintf((stderr, "Unknown, try on firstfit: "));
		info->keybuf_counter = i;
		find = luc_match_label(info, 1);
	    }
	    dprintf((stderr, "%d\n", find));
    }

    if (find == 0) {
	cb_data->index_of_choice_selected = XIM_UNKNOWN_KEYSYM;
	return;
    }

    if (find != XIM_UNDETERMINED && find != XIM_UNKNOWN_KEYSYM)
        cb_data->index_of_choice_selected = find - 1 + info->first;
    else
	cb_data->index_of_choice_selected = find;
/* if selection complete , free keyboard grab 
    if (find != XIM_UNDETERMINED && find != XIM_UNKNOWN_KEYSYM  && info->keygrab) {
        XUngrabKeyboard(info->dpy, CurrentTime);
    } */


    return;
}


void
lookup_choices_done(ic, client_data, cb_data)
    XIC					 ic;
    XPointer				 client_data;
    XIMLookupStartCallbackStruct	*cb_data;
{
    LucInfo		*info;
    LucFrameInfo	*lfi;
    

    dprintf((stderr, "luc_done\n"));
    info = LUC_UDATA_INFO(client_data);
    lfi = info->frame_info;
    
    if (luc_defaults.keygrab) {
    	XUngrabKeyboard(lfi->dpy, CurrentTime);
    }

    (void) xv_set(lfi->canvas,
		  XV_KEY_DATA, LUC_KEY,	NULL,
		  NULL);
    switch (lfi->lfs) {
	default:
	    dprintf((stderr, "done: being mapped, request to unmap\n"));
	    lfi->lfs = LFS_UNMAP_REQUESTED;
	    break;

	case LFS_MAPPED:
	    (void) xv_set(lfi->frame,
			  XV_SHOW,		FALSE,
			  NULL);
	    dprintf((stderr, "done: being unmapped\n"));
	    lfi->lfs = LFS_BEING_UNMAPPED;
	    break;
    }

#ifndef USE_CANVAS
    (void) xv_destroy(info->frame_info->panel_item);
#endif
    
    xv_free (info);
    luc_start_is_called = FALSE;
}


/*
 *   Do negotiation with XIM.
 */
static void
luc_do_negotiate(info, cb_data)
    register LucInfo			*info;
    XIMLookupStartCallbackStruct        *cb_data;
{

    if (cb_data->WhoIsMaster == XIMIsMaster) {
	info->WhoOwnsLabel = cb_data->XIMPreference->WhoOwnsLabel;
	info->max_can = cb_data->XIMPreference->choice_per_window;
	info->d_rows = cb_data->XIMPreference->nrows;
	info->d_columns = cb_data->XIMPreference->ncolumns;
	info->dir = cb_data->XIMPreference->DrawUpDirection;
    } else {
	info->WhoOwnsLabel = CBOwnsLabel;
	cb_data->CBPreference->choice_per_window
				= info->max_can = luc_defaults.max_can;
	cb_data->CBPreference->nrows
				= info->d_rows = luc_defaults.rows;
	cb_data->CBPreference->ncolumns
				= info->d_columns = luc_defaults.columns;
	cb_data->CBPreference->DrawUpDirection
				= info->dir = luc_defaults.dir;
	cb_data->CBPreference->WhoOwnsLabel = CBOwnsLabel;
	cb_data->WhoIsMaster = CBIsMaster;
    }
}


/*
 *	 This function allocates appropriate memory for LucInfo and
 *	connects it to client_data so that every function in the package can
 *	reference LucInfo.
 */
static LucInfo *
luc_info_create(client_data)
    XPointer		 client_data;
{
    LucInfo		*data;
    Xv_opaque		*dum;

    dum = (Xv_opaque *) client_data;
    if ((data = (LucInfo *) xv_alloc(LucInfo)) == NULL)
	return NULL;

    *dum = (Xv_opaque) data;

    return data;
}


/*
 * This function retrieves user-preference setings from resource
 * database.
 */
static void
luc_get_defaults(locale)
    Xv_generic_attr	locale;
{
    int			i, j, k, l, m;
    int			nchar;
    wchar_t		*p;
    wchar_t		*label_storage;
    wchar_t		 label[LUC_LOOKUPSTRING_BUFLEN + 1];
    LucLabelTable	*llt;
    static Defaults_pairs keykind_pairs[] = {
	{"Numeric",		(int) LUC_NUMERIC},
	{"N",			(int) LUC_NUMERIC},
	{"AlphabeticLower",	(int) LUC_ALPHABETIC_LOWER},
	{"AL",			(int) LUC_ALPHABETIC_LOWER},
	{"AlphabeticUpper",	(int) LUC_ALPHABETIC_UPPER},
	{"AU",			(int) LUC_ALPHABETIC_UPPER},
	{"Alphabetic",		(int) LUC_ALPHABETIC},
	{"A",			(int) LUC_ALPHABETIC},
	{NULL,			(int) LUC_ALPHABETIC}};
    static Defaults_pairs dir_pairs[] = {
	{"Verticalyl",		(int) DrawUpVertically},
	{"Vertical",		(int) DrawUpVertically},
	{"V",			(int) DrawUpVertically},
	{"Horizontal",		(int) DrawUpHorizontally},
	{"Horizontalyl",	(int) DrawUpHorizontally},
	{"H",			(int) DrawUpHorizontally},
	{NULL,			(int) DrawUpHorizontally}};
	

    defaults_set_locale(NULL, locale);

    luc_defaults.keygrab = defaults_get_boolean("luc.keygrab",
						"Luc.Keygrab",
						TRUE);

    luc_defaults.threed = defaults_get_boolean("OpenWindows.3DLook.Color",
					       "OpenWindows.3DLook.Color",
					       TRUE);

    if ((luc_defaults.font_name = defaults_get_string("luc.font",
						      "Luc.Font",
						      NULL)) != NULL)
        luc_defaults.font_name = xv_strsave(luc_defaults.font_name);

    luc_defaults.key_kind = (LucKeyKind) defaults_get_enum("luc.keykind",
					      "Luc.Keykind",
					      keykind_pairs);
 
    luc_defaults.window_margin_x
    		= defaults_get_integer_check("luc.window.margin.x",
					     "Luc.Window.Margin.X",
					     15, 0, 100000);
    luc_defaults.window_margin_y
		= defaults_get_integer_check("luc.window.margin.y",
					     "Luc.Window.Margin.Y",
					     15, 0, 100000);
    luc_defaults.window_off_x
    		= defaults_get_integer_check("luc.window.off.x",
					     "Luc.Window.Off.X",
					     20, 0, 100000);
    luc_defaults.window_off_y
		= defaults_get_integer_check("luc.window.off.y",
					     "Luc.Window.Off.Y",
					     10, 0, 100000);
    luc_defaults.window_should_fit
		= defaults_get_boolean("luc.window.fit",
				       "Luc.Window.Fit", TRUE);
    luc_defaults.screen_margin_x
		= defaults_get_integer_check("luc.screen.margin.x",
					     "Luc.Screen.Margin.X",
					     20, 0, 100000);
    luc_defaults.screen_margin_y
		= defaults_get_integer_check("luc.screen.margin.y",
					     "Luc.Screen.Margin.Y",
					     20, 0, 100000);

    luc_defaults.dir = (DrawUpDirection) defaults_get_enum("luc.ddir",
					 "Luc.Ddir",
					 dir_pairs);
    	
    luc_defaults.max_can = defaults_get_integer_check("luc.max",
						      "Luc.Max",
						      26, 1, 10000);

    luc_defaults.rows = defaults_get_integer_check("luc.nrows",
						   "Luc.Nrows",
						   6, 1, 10000);

    luc_defaults.columns = defaults_get_integer_check("luc.ncolumns",
						      "Luc.Ncolumns",
						      6, 1, 10000);

    luc_defaults.label_next = mbstowcsdup(XV_MSG("^N: NEXT"));

    luc_defaults.label_previous = mbstowcsdup(XV_MSG("^P: PREVIOUS"));

    defaults_set_locale(NULL, XV_NULL);


    /*
     * Constructs label.
     */
    llt = &luc_label_table[(int) luc_defaults.key_kind];
    /* Figure out how many charcaters may need for each label. */
    nchar = luc_defaults.max_can / llt->ndecimal + 1 + 1 /* for NULL */;
    label_storage = xv_alloc_n(wchar_t, (luc_defaults.max_can * nchar));
    luc_defaults.labels = xv_alloc_n(wchar_t *, luc_defaults.max_can);
    label[sizeof(label) / sizeof(wchar_t) - 1] = 0;

    for (i = 0; i < luc_defaults.max_can; i++) {
	p = &label[sizeof (label) / sizeof (wchar_t) - 2];
	j = i + llt->numeric_adjust;
	l = llt->ndecimal;
	m = 1;
	do {
	    k = j % l + (m == 1 ? llt->alphabet_adjust : 0);
	    *p-- = llt->label[k];
	    j /= l;
	    m++;
	} while (j > 0);
	(void) wscpy(label_storage, p + 1);
	luc_defaults.labels[i] = label_storage;
	label_storage += nchar;
    }
}


/*
 *	 This function sets geometry of LUC window.
 */
static
luc_get_draw_info(info)
    register LucInfo	*info;
{
    int			 i;
    int			 max_col;
    int			 max_lin;
    register XIMLookupDrawCallbackStruct  *draw = info->draw;
    int			 last_candidate;
    LucFrameInfo	*lfi;
    int			 max_label_len;


    lfi = info->frame_info;

    info->rows = info->d_rows;
    info->columns = info->d_columns;

    /*
     * number of candidates to be drawn
     */
    if (info->WhoIsMaster == XIMIsMaster) {
    	info->first = draw->index_of_first_candidate;
	last_candidate = draw->index_of_last_candidate;
    } else {
	/*
	 * This setting is done in lookup_choices_start() for the 1st
	 * time, and in luc_redraw() for the 2nd call and
	 * afterwards.
	 */
	 last_candidate = draw->n_choices - 1;
    }
    info->num = last_candidate - info->first + 1;
    if (info->num > info->max_can) {
	info->num = info->max_can;
	last_candidate = info->first + info->num - 1;
    }

    /*
     * cell-size etc
     */
    if (info->WhoOwnsLabel == CBOwnsLabel) {
	i = luc_label_table[(int) luc_defaults.key_kind].ndecimal;
	for (max_label_len = 1; info->num >= i; i *= i)
	    max_label_len++;
	info->label_width = (max_label_len + 1) * lfi->achar_width;
	info->item_width = info->label_width
			+ (draw->max_len * lfi->char_width)
			+ lfi->gap_x;
    } else {
        register int	lab;

	max_label_len = 0;
	for (lab = draw->index_of_first_candidate; 
		lab <= last_candidate ; lab++) {
		if ((unsigned int)max_label_len < draw->choices->label->length)
			max_label_len = draw->choices->label->length;
	}
	info->label_width = max_label_len * lfi->char_width;
	info->item_width = info->label_width
			+ (draw->max_len * lfi->char_width)
			+ lfi->gap_x;
    }

    info->item_height = lfi->char_height + lfi->gap_y;

    /*
     * max number of cells
     */
    max_col = (lfi->sc_width - luc_defaults.window_margin_x * 2)
			/ info->item_width;
    max_lin = (lfi->sc_height - luc_defaults.window_margin_y * 2)
			/ info->item_height;

    /*
     * number of lines and columns
     */
    if (info->dir == DrawUpVertically) { /* use specified nrows first*/
	if (info->rows > max_lin)
	    info->rows = max_lin;
	info->columns = (info->num - 1) / info->rows + 1;
	if (info->columns > max_col) {
	    info->rows = max_lin;
	    info->columns = (info->num - 1) / info->rows + 1;
	    if (info->columns > max_col) {
		info->columns = max_col;
	    	info->num = max_lin * max_col;
		last_candidate = info->first + info->num - 1;
		/*
		 * BUG!! To determine cols & rows, we used info->num
		 * and candidate, which affect the max-len of the
		 * candidates. Here, we reduce those numbers
		 * considering ths restriction of screen size. So it
		 * may happen that, this reduction (in turn) reduces
		 * the required area per candidate. As a result, we
		 * might be able to display more candidates if we make
		 * a feedback.But this seems to be an exceptional case.
		 */
	    }
	}
	if (info->rows > info->num)
	    info->rows = info->num;
    } else if (info->dir == DrawUpHorizontally) { /* use specified ncolumns */
	if (info->columns > max_col)
	    info->columns = max_col;
    	info->rows = (info->num - 1) / info->columns + 1;
	if (info->rows > max_lin) {
	    info->columns = max_col;
	    info->rows = (info->num -1) / info->columns + 1;
	    if (info->rows > max_lin) {
		info->rows = max_lin;
	    	info->num = max_lin * max_col;
		last_candidate = info->first + info->num -1;
	    }
        }
	if (info->columns > info->num)
	    info->columns = info->num;
    }

    /*
     * Calculate the size of window (frame).
     */
    info->f_width = (info->columns * info->item_width)
    		    + (luc_defaults.window_margin_x * 2) - lfi->gap_x;
    info->f_height = (info->rows * info->item_height)
    		    + (luc_defaults.window_margin_y * 2) - lfi->gap_y;
    if (info->draw->n_choices > info->num) {
	/*
	 * Need "Next"/"Previous" button.
	 */
	if (info->f_width < lfi->label_np_width)
	    info->f_width = lfi->label_np_width;
	    info->f_height += info->item_height + lfi->gap_y;
    }
#ifdef DEBUG1
    fprintf(stderr, "width %d, height %d, fx %d, fy %d -> ",
	info->f_width, info->f_height, lfi->f_x, lfi->f_y);
#endif

    /*
     * Now, make sure new window fit into the screen.
     */
    if ((info->f_width + lfi->f_x)
		> (lfi->sc_width - luc_defaults.screen_margin_x))
	info->f_x = lfi->sc_width - info->f_width
			- luc_defaults.screen_margin_x;
    else
	info->f_x = lfi->f_x;
    if (info->f_x < luc_defaults.screen_margin_x)
	info->f_x = luc_defaults.screen_margin_x;

    if ((info->f_height + lfi->f_y)
		> (lfi->sc_height - luc_defaults.screen_margin_y))
	info->f_y = lfi->sc_height - info->f_height
			- luc_defaults.screen_margin_y;
    else
	info->f_y = lfi->f_y;
    if (info->f_y < luc_defaults.screen_margin_y)
	info->f_y = luc_defaults.screen_margin_y;
#ifdef DEBUG1
    fprintf(stderr, " fx %d, fy %d\n", info->f_x, info->f_y);
#endif

    return 0;
}


#ifndef CANVAS
/*    
 *      This function puts candidates on the panel.
 */
static
luc_draw_candidates(info)
   LucInfo  		*info;
{
    int			 count_minus_1 = 0;
    int			 max_len;
    register XIMChoiceObject	
			*choice  = info->draw->choices;
    wchar_t		 buf[256];
    static wchar_t	 blank[2]= {(wchar_t)' ' , (wchar_t)'\0'};
    wchar_t		 wchar_buf[256];
    char		*format1 = "% d:";
    char		*format2 = "  %c:";
    char		 char_num[4];
    int			 number, offset, u_or_l;

    (void) xv_set(info->frame_info->panel_item,
			      XV_X,			info->window_margin_x,
			      XV_Y, 			info->window_margin_y,
			      PANEL_CHOICE_NROWS,	info->rows,
			      PANEL_CHOICE_NCOLS,	info->columns,
			      NULL );

    choice += info->first;

    if (info->dir == DrawUpVertically)
	(void) xv_set(info->frame_info->panel_item,
		PANEL_LAYOUT,	PANEL_VERTICAL,
		NULL);
    else
	(void) xv_set(info->frame_info->panel_item,
		PANEL_LAYOUT,	PANEL_HORIZONTAL,
		NULL);

    if (info->WhoOwnsLabel == CBOwnsLabel
     && luc_defaults.key_kind == LUC_NUMERIC){
	/*
	 * setup character format for labels( numeric case )
	 */
	(void) sprintf( char_num, "%1d" ,LUC_MAX_LABEL_LEN);
	format1[1] = char_num[0];
    }
	
    while (count_minus_1 < info->num) {
    	*buf = 0;

	/** make label first **/
       	if (info->WhoOwnsLabel == XIMOwnsLabel) {
		if (choice->label->encoding_is_wchar)
			wscpy(buf, choice->label->string.wide_char);
		else
			mbstowcs(buf, choice->label->string.multi_byte ,256);
	} else switch (luc_defaults.key_kind) {
	    case LUC_NUMERIC:
	        (void) wsprintf(buf, format1, count_minus_1 + 1);
		break;

	    case LUC_ALPHABETIC:
		number = count_minus_1 / 52;
		offset = count_minus_1 % 26 ;
		u_or_l = ( count_minus_1 / 26 ) / 2;

		if (number != 0) {
			(void) sprintf(char_num, "2d", number);
			bcopy(char_num, format2, 2);
		}
		
		if (u_or_l == 0)
			(void) wsprintf(buf, format2, offset + 'a');
		else
			(void) wsprintf(buf, format2, offset + 'A');
		break;

	    case LUC_ALPHABETIC_UPPER:
                number = 0;
                offset = count_minus_1 % 26;

                if (number != 0) {
                        (void) sprintf(char_num, "2d", number);
                        bcopy(char_num, format2, 2);
                }
                (void) wsprintf(buf, format2, offset + 'A');
		break;

	    case LUC_ALPHABETIC_LOWER:
                number = 0;
                offset = count_minus_1 % 26;

                if (number != 0) {
                        (void) sprintf(char_num, "2d", number);
                        bcopy(char_num, format2, 2);
                }
                (void) wsprintf(buf, format2, offset + 'a');
		break;
        }

	/** now , append the candidate string **/
	if (choice->value->encoding_is_wchar) {
  		wscat(buf, choice->value->string.wide_char);
	} else {
 		mbstowcs(wchar_buf, choice->value->string.multi_byte, 256);
		wscat(buf, wchar_buf);
	}
/*
	pad_len = ( max_len - choice->value->length ) * info->char_ratio;
	for (pad = 0 ; pad < pad_len ; pad++)
		wscat(buf, blank);
*/
	(void) xv_set(info->frame_info->panel_item,
		PANEL_CHOICE_STRING_WCS,	count_minus_1, buf,
		NULL);
	count_minus_1++;
    	choice ++;
    }

    if (info->draw->n_choices > info->num) {
    	(void) xv_set(info->frame_info->panel_item,
                PANEL_CHOICE_STRING, count_minus_1++ ,
	 	XV_MSG("^N: NEXT"),
                NULL);
    	(void) xv_set(info->frame_info->panel_item,
                PANEL_CHOICE_STRING, count_minus_1++ , 
		XV_MSG("^P: PREVIOUS"),
                NULL);
    }
}
#endif /* CANVAS */


static
luc_match_label(info, firstfit)
    LucInfo		*info;
    Bool		 firstfit;
{
    wchar_t	       **l;
    int			 i;
    int			 find;
    int			 count;


#ifdef XIMOWNSLABEL
    /*
     *  It is redundant that CB compares labels where as labels were
     *  created in XIM. The following if block supports this case,
     *  but it is very slow.
     */
    if (info->WhoOwnsLabel == XIMOwnsLabel) {
        char			 labelbuf[LUC_MAX_LABEL_LEN];
        char			*lab;
        XIMChoiceObject		*choice = info->draw->choices + info->first;
        int			 match = 0;
        int	    		 find_mark;

	for (find = 1 ; find <= info->num ;  find++) {
	    if ((unsigned) count > choice->label->length)
		    continue;
	    if (choice->label->encoding_is_wchar) {
		lab = labelbuf;
	        if (count != wcstombs(lab, choice->label->string.wide_char,
				LUC_MAX_LABEL_LEN + 1 ))
			/*
			 *  only ascii characters can be used in labels
			 */
			return 0;  /* Fatal error (wrong char in labels)*/
	    } else {
		lab = choice->label->string.multi_byte;
	    }
	    if (! strncmp(lab, info->keybuf, count)) {
	        find_mark = find;
	        match ++;
	        if (flag == 0) {    
		    /* 
		     *	There must be a label that matches exactly. If not,
		     *  XIM_UNKNOWN_KEYSYM is returned.
		     */
	            if (!strcmp(lab, keybuf)) 
			return find;
		    else
			match = 0;
		}
	    }
	    choice++;
	}
	if (match == 1)    /* Just one candidate matched */
		return  find_mark;
	else if (match == 0)
		return  XIM_UNKNOWN_KEYSYM;
	else
		return  XIM_UNDETERMINED;
    }
#endif /* XIMOWNSLABEL */

    count = 0;
    for (i = 1, l = luc_defaults.labels;
	 l < &luc_defaults.labels[info->num]; l++, i++) {
	if (info->keybuf[0] == l[0][0]) {
	    if (wsncmp(info->keybuf, *l, info->keybuf_counter) == 0) {
		count++;
		find = i;
		if (count == 1)
		    info->current = find;
		if (firstfit == True)
		    break;
	    }
	}
    }
    if (count == 1)
        return find;
    if (count > 1)
        return XIM_UNDETERMINED;

    return XIM_UNKNOWN_KEYSYM;
}


/*
 *   This function is a handler of arrow keys. When user inputs arrow keys,
 *   the only thing that lookup_choices_process() does is change the current
 *   candidate.
 */
static void
luc_arrow_handle(info, keysym)
register LucInfo	*info;
KeySym			keysym;
{
    int		deltax,deltay;

    if (info->dir == DrawUpHorizontally) {
	deltax = 1;
	deltay = info->columns;
    } else {
	deltax = info->rows;
	deltay = 1;
    }
	
    switch (keysym) {
      case XK_Left:
	if (info->current > deltax)
		info->current -= deltax;
	else
		return;
	break;

      case XK_Right:
	if (info->current + deltax <= info->num)
		info->current += deltax;
	else
		return;
	break;

      case XK_Up:
	if (info->current > deltay)
		info->current -= deltay;
	else
		return;
	break;

      case XK_Down:
	if (info->current + deltay <= info->num)
		info->current += deltay;
	else
		return;
	break;
    }

#ifndef USE_CANVAS
    (void) xv_set(info->frame_info->panel_item, PANEL_VALUE, info->current-1, NULL);
#endif

}


/*  !!Caution.
 *  This function assumes that WhoIsMaster is CBIsMaster
 *
 *  This function is used to redraw lookup choice window in case that
 *  user specifies next ( or previous ) sets of candidates to be drawn.
 */
static
luc_redraw(info, keysym)
    register LucInfo    *info;
    KeySym		keysym;
{
    int		n_set;
    register XIMLookupDrawCallbackStruct  *draw = info->draw;
    LucFrameInfo *lfi;


    lfi = info->frame_info;

    switch (keysym) {
	case XK_p:
	    info->first -= info->max_can;
	    if (info->first < 0) {
		n_set = (draw->n_choices - 1) / info->max_can + 1;
		info->first = (n_set - 1) * info->max_can;
	    }
	    break;

	case XK_n:
	    info->first += info->num;
	    if (info->first >= draw->n_choices) {
		info->first = 0;
	    }
	    break;

	default:
	    return 0;
    }

#ifdef USE_CANVAS
    XClearWindow(lfi->dpy, lfi->canvas_xwin);
    if (luc_get_draw_info(info) < 0)
	return 0;
    luc_repaint_proc(lfi->canvas, (Xv_Window) NULL,
		     lfi->dpy, lfi->canvas_xwin, (Xv_xrectlist *)NULL);

#else /* !USE_CANVAS */

    (void) xv_set(info->frame_info->frame , XV_SHOW , FALSE , NULL);
    luc_syncup(check_unmap, lfi);

    if (luc_get_draw_info(info) < 0)
	return 0;

    xv_destroy(lfi->panel_item);

    /*
     * Disable mouse event in lookup choice region.  mouse selection
     * of the kana -> kanji conversion will be supported in the future.
     * At that time replace PANEL_EVENT_PROC with PANEL_NOTIFY_PROC,
     * and replace ignore_mouse_handler with luc_mouse_handler.  So
     * capture events here with ignore_mouse_handler
     */

    lfi->panel_item = (Panel_item) xv_create(lfi->panel, PANEL_CHOICE,
                PANEL_CHOOSE_ONE,       TRUE,
		PANEL_EVENT_PROC,	ignore_mouse_handler,
#ifdef MOUSE_SUPPORT
		XV_KEY_DATA,            LUC_KEY, info,
#endif
                NULL);

    luc_draw_candidates(info);          /* draw candidates     */

    window_fit(lfi->panel);
    window_fit(lfi->frame);

    (void) xv_set(lfi->frame,
		XV_SHOW,        TRUE,
		NULL);
    luc_syncup(luc_check_map, lfi);

    (void) xv_set(lfi->panel_item, PANEL_VALUE, 0, NULL);
#endif /* USE_CANVAS */

    info->current = 1;   /* current choice nember */
    info->state = LUC_DRAW_CALLED;

    return XIM_UNKNOWN_KEYSYM;
}
    

#ifdef MOUSE_SUPPORT
static void
luc_mouse_handler(item, value, event)
    Panel_item	item;
    int		value;
    Event	*event;
{
    register LucInfo	*info;

    info = (LucInfo *) xv_get(item, XV_KEY_DATA, LUC_KEY);

    info->current = info->first + value + 1;
    dprintf((stderr , "Mouse selection was %d\n" , info->current));
}
#endif /* MOUSE_SUPPORT */


#ifndef USE_CANVAS
void static
ignore_mouse_handler(item, event)
    Panel_item	item;
    Event		*event;
{
    /*
     * Only pass on non-mouse events to default event handler
     * ACTION_{SELECT,ADJUST,MENU} and PANEL_EVENT_CANCEL,
     * PANEL_EVENT_DRAG_IN have all been replaced with event_is_button
     * at Mitch's suggestion.  Because PANEL_EVENT_DRAG_IN is no
     * longer supported.  However, initial testing shows this doesn't
     * quite work.  We need to refix this code.
     */

    if ( (event_action(event) != LOC_DRAG) &&
       !(event_is_button(event)) )
	panel_default_handle_event(item, event);
}

/*
 * Because of the asynchronous nature of X, requesting an XV_SHOW,
 * FALSE, of the frame then immediately request an XV_SHOW, TRUE, of
 * the frame causes the caching mechanism to go out of sync.
 * Therefore, after we do XV_SHOW, FALSE, we sit and wait until we see
 * the unmap event.
 */

static void
luc_syncup(what, lfi)
    Bool		(*what)();
    LucFrameInfo	 *lfi;
{
    XEvent	event;

    XSync(lfi->dpy, False);
#ifdef PUT_BACK_EVENT
    XIfEvent(lfi->dpy, &event, what, (char *) lfi->f_xwin);
    XPutBackEvent(lfi->dpy, &event);
#else
    XPeekIfEvent(lfi->dpy, &event, what, (char *) lfi->f_xwin);
#endif
}


static Bool
luc_check_map(display, event, xid)
    Display *display;
    XEvent  *event;
    char    *xid;
{
    if ((event->type == MapNotify)
     && (((XMapEvent *) event)->window == (XID)xid))
            return True;
    else
            return False;
}


static Bool
luc_check_unmap(display, event, xid)
    Display *display;
    XEvent  *event;
    char    *xid;
{
    if ((event->type == UnmapNotify)
     && (((XUnmapEvent *) event)->window == (XID)xid))
            return True;
    else
            return False;
}
#endif /* ! USE_CANVAS */


/*
 * Frame handler, it will cache the frame per base frame basis.
 */
static LucFrameInfo *
luc_get_frame(dpy, cb_data)
    Display				*dpy;
    XIMLookupStartCallbackStruct	*cb_data;
{
    LucFrameInfo	*lfi;
    LucFrameInfo	*plfi;
    XFontSetExtents	*font_set_extents;
    Xv_object		 p_obj;
    Frame		 p_frame;
    Rect		*rect;
#ifdef USE_CANVAS
    Xv_Window		 paint_window;
    unsigned long	 event_mask;
#ifdef notdef
    Xv_Screen		 screen;
    GC			*gc_list;
#endif
    XFontStruct		*font_struct;
#endif

    p_obj = (Xv_object) win_data(dpy, cb_data->event->window);
    p_frame = luc_get_p_frame(p_obj);
    for (plfi = lfi = luc_frame_info; lfi != NULL; plfi = lfi, lfi = lfi->next)
	if (lfi->dpy == dpy && lfi->p_frame == p_frame)
	    break;

    if (lfi == NULL) {
#ifdef DEBUG1
	fprintf(stderr, "Creating frame and panel for luc [%x/%x].\n",
		dpy, p_frame);
#endif
	if (luc_defaults.has_been_initialized == FALSE) {
	    luc_get_defaults(XV_LC_INPUT_LANG);
	    luc_defaults.has_been_initialized = TRUE;
	}

	if ((lfi = (LucFrameInfo *) xv_alloc(LucFrameInfo)) == NULL)
	    return NULL;
	if (luc_frame_info == NULL)
	    luc_frame_info = lfi;
	else
	    plfi->next = lfi;

	lfi->dpy = dpy;
	lfi->p_obj = p_obj;
	lfi->p_frame = p_frame;
	luc_get_win_position(lfi);
	lfi->frame = (Frame) xv_create(lfi->p_frame, FRAME_CMD,
		WIN_USE_IM,			FALSE,
		XV_X,				lfi->f_x,
		XV_Y,				lfi->f_y,
		XV_SHOW,			FALSE,
		OPENWIN_NO_MARGIN,		TRUE,
		FRAME_SHOW_HEADER,		FALSE,
		FRAME_SHOW_RESIZE_CORNER,	FALSE,
		FRAME_CLOSED,			FALSE,
		FRAME_CMD_POINTER_WARP,		FALSE,
		WIN_SAVE_UNDER,			TRUE,
		WIN_CONSUME_EVENTS,		    ACTION_OPEN,
						    ACTION_CLOSE,
						    ACTION_RESCALE,
						    WIN_RESIZE,
						    WIN_MAP_NOTIFY,
						    WIN_UNMAP_NOTIFY,
						NULL,
		WIN_CONSUME_X_EVENT_MASK,	StructureNotifyMask,
		WIN_EVENT_PROC,			luc_win_event_proc,
		XV_KEY_DATA, LUC_WIN_KEY,	lfi,
		NULL);
	if (lfi->frame == NULL)
	    return NULL;
	lfi->lfs = LFS_UNMAPPED;

	lfi->f_xwin = xv_get(lfi->frame, XV_XID);
	if (luc_defaults.font_name != NULL) {
		if ((lfi->xv_font = xv_find(lfi->frame, FONT,
				FONT_SET_SPECIFIER,	luc_defaults.font_name,
				NULL)) == NULL) {
		    char	 buf[100];

		    (void) sprintf(buf,
		        XV_MSG("luc: font(%s) couldn't find, use default"),
			luc_defaults.font_name);
		    xv_error(XV_ZERO, ERROR_STRING, buf, NULL);
		}
	}
	if (lfi->xv_font == NULL)
	    lfi->xv_font = xv_get(lfi->frame, XV_FONT);

	lfi->afont = (XID) xv_get(lfi->xv_font, XV_XID);
	if ((font_struct = XQueryFont(dpy, lfi->afont)) == NULL) {
		/*
		 * This should not happen, but if its ever happen.  We
		 * should get around the problem, instead of just
		 * telling the failar.
		 */
		return NULL;
	}
	lfi->achar_width = font_struct->max_bounds.width;
	/*
	 * Should not free the font_struct here.  At least current
	 * Sun's implementation of mltext refers to the same physical
	 * structure by the Xlib(mltext) itself.
	 */
	lfi->font_set = (XFontSet) xv_get(lfi->xv_font, FONT_SET_ID);
	font_set_extents = XExtentsOfFontSet(lfi->font_set);
	lfi->char_ascent = -font_set_extents->max_logical_extent.y;
	lfi->char_descent = font_set_extents->max_logical_extent.height
			  + font_set_extents->max_logical_extent.y;
	/*
	 * FIX_ME:
	 * Waiting for the BugID 1062587 (Synopsis:  XExtentsOfFontSet
	 * does not return right value for the
	 * max_logical_extent.width) to be fixed.
	 *
	 * lfi->char_width = font_set_extents->max_logical_extent.width;
	 */
	lfi->char_width = font_set_extents->max_ink_extent.width;
	lfi->char_height = font_set_extents->max_logical_extent.height;

#ifdef USE_CANVAS
	lfi->canvas = (Canvas) xv_create(lfi->frame, CANVAS,
				 XV_X,			0,
				 XV_Y,			0,
				 WIN_USE_IM,		FALSE,
				 CANVAS_X_PAINT_WINDOW,	TRUE,
				 CANVAS_REPAINT_PROC,	luc_repaint_proc,
				 CANVAS_RETAINED,	FALSE,
				 NULL);
	if (lfi->canvas == NULL)
	    return NULL;
	lfi->canvas_pw = paint_window =
		(Xv_opaque) xv_get(lfi->canvas,
					  CANVAS_NTH_PAINT_WINDOW, NULL);
	lfi->canvas_xwin = xv_get(lfi->canvas_pw, XV_XID);
	/*
	 * Make sure, canvas does not get the any keyboard inputs.
	 */
	event_mask = xv_get(paint_window, WIN_X_EVENT_MASK);
	event_mask &= ~(KeyPressMask | KeyReleaseMask
			| EnterWindowMask | LeaveWindowMask
			| FocusChangeMask);
	(void) xv_set(paint_window, WIN_X_EVENT_MASK, event_mask, NULL);
	lfi->ginfo = (Graphics_info *) xv_init_olgx(paint_window,
						    &luc_defaults.threed,
						    lfi->xv_font);
	lfi->gc_text = lfi->ginfo->gc_rec[OLGX_BLACK]->gc;
/*	lfi->gc_clear = lfi->ginfo->gc_rec[OLGX_BG1]->gc;*/

	lfi->gap_x = lfi->char_width;
	lfi->gap_y = lfi->char_height / 2;

	lfi->label_next_width = XwcTextEscapement(lfi->font_set,
					  luc_defaults.label_next,
					  wslen(luc_defaults.label_next));
	lfi->label_np_width = lfi->label_next_width + lfi->gap_x
		      + XwcTextEscapement(lfi->font_set,
					  luc_defaults.label_previous,
					  wslen(luc_defaults.label_previous));
#else /* ! USE_CANVAS */
	lfi->panel = (Panel) xv_get(lfi->frame, FRAME_CMD_PANEL,
				    NULL);
#endif /* USE_CANVAS */
    } else {
	rect = (Rect *) xv_get(lfi->frame, WIN_RECT);
	lfi->f_x = rect->r_left;
	lfi->f_y = rect->r_top;
    }

    return lfi;
}


/*
 * This function gets handlers of pre-edit windows.( Frame and the
 * associated object ) to enable Luc frame a command-frame, and also,
 * to make keyboard grab possible.
 */
static Frame
luc_get_p_frame(p_obj)
    Xv_object	p_obj;
{
    int         is_subframe;
    Frame	p_frame;

    p_frame = (Frame) xv_get(p_obj, WIN_FRAME);
    is_subframe = (int) xv_get(xv_get(p_frame, WIN_OWNER),
			       XV_IS_SUBTYPE_OF, FRAME_CLASS);
    if (is_subframe) 
	p_frame = (Frame) xv_get(p_frame, WIN_OWNER);

    return p_frame;
}


static void
luc_get_win_position(lfi)
    register LucFrameInfo		*lfi;
{
    Display	*dpy;
    Rect	*rect;

    
 /** overall window geometry **/
    dpy = lfi->dpy;

    lfi->sc_width = DisplayWidth(dpy, DefaultScreen(dpy));
    lfi->sc_height = DisplayHeight(dpy, DefaultScreen(dpy));

    rect = (Rect *) xv_get(lfi->p_frame, WIN_RECT);
    lfi->f_x = rect->r_left + luc_defaults.window_off_x;
    lfi->f_y = rect->r_top + luc_defaults.window_off_y;
}


static void
luc_repaint_proc(canvas, pw, dpy, xw, xrects)
    Canvas		 canvas;
    Xv_Window		 pw;
    Display		*dpy;
    Window		 xw;
    Xv_xrectlist	*xrects;
{
    LucInfo		*info;
    LucFrameInfo	*lfi;
    XIMChoiceObject	*choice;
    XIMChoiceObject	*last_choice;
    int			 num;
    int			 x, y;
    int			 i;
    wchar_t		 buf[10];
    wchar_t		*value;


    dprintf((stderr, "luc_repaint\n"));
    if ((info = (LucInfo *) xv_get(canvas, XV_KEY_DATA, LUC_KEY)) == NULL) {
	/*
	 * Has been destroyed before actually shown to the screen.
	 */
	return;
    }
    lfi = info->frame_info;
    if (lfi->lfs != LFS_MAPPED) {
	/*
	 * Well, it still doing something, let's not draw to the
	 * screen yet.
	 */
	return;
    }

    choice = &(info->draw->choices[info->first]);
    last_choice = &choice[info->num];

    for (num = 0; choice < last_choice; choice++, num++) {
	if (info->dir == DrawUpVertically) {
		x = luc_defaults.window_margin_x
		    + (num / info->rows) * info->item_width;
		y = luc_defaults.window_margin_y + lfi->char_ascent
		    + (num % info->rows) * info->item_height;
	} else {
		x = luc_defaults.window_margin_x
		    + (num % info->columns) * info->item_width;
		y = luc_defaults.window_margin_y + lfi->char_ascent
		    + (num / info->columns) * info->item_height;
	}
	i = wsprintf(buf, "%ws:", luc_defaults.labels[num]);
	XwcDrawString(dpy, xw, lfi->font_set, lfi->gc_text,
		      x, y, buf, i);
	x += info->label_width;
	value = choice->value->string.wide_char;
	XwcDrawString(dpy, xw, lfi->font_set, lfi->gc_text,
		      x, y, value, wslen(value));
    }
    if (info->draw->n_choices > info->num) {
	/*
	 * Need "Next"/"Previous" button.
	 */
	x = (lfi->f_width - lfi->label_np_width) / 2;
	y = lfi->f_height - luc_defaults.window_margin_y - lfi->char_descent;
	XwcDrawString(dpy, xw, lfi->font_set, lfi->gc_text,
		      x, y, luc_defaults.label_next,
		      wslen(luc_defaults.label_next));
	x += lfi->label_next_width + lfi->gap_x;
	XwcDrawString(dpy, xw, lfi->font_set, lfi->gc_text,
		      x, y, luc_defaults.label_previous,
		      wslen(luc_defaults.label_previous));
    }
}


static void
luc_win_event_proc(window, event, arg)
    Xv_Window	window;
    Event	*event;
    Notify_arg	arg;
{
    LucFrameInfo	*lfi;
    LucFrameState	 new;


    lfi = (LucFrameInfo *) xv_get(window, XV_KEY_DATA, LUC_WIN_KEY);
    dprintf((stderr, "event_proc: action %d, lfs = %d, Xevent = %d\n",
	     event_action(event), lfi->lfs, event_xevent(event)->type));
    new = lfi->lfs;
    switch (event_action(event)) {
	case ACTION_OPEN:
	case WIN_MAP_NOTIFY:
	    dprintf((stderr, "event_proc: mapped\n"));
	    lfi->mapunmap = LFS_NOP;
	    switch (lfi->lfs) {
	        case LFS_UNMAP_REQUESTED:
		    dprintf((stderr, "	being un-mapped upon req\n"));
		    xv_set(lfi->frame, XV_SHOW,	FALSE, NULL);
		    new = LFS_BEING_UNMAPPED;
		    break;

		case LFS_RESIZE_REQUESTED:
		    dprintf((stderr, "	being un-mapped upon resize req\n"));
		    xv_set(lfi->frame, XV_SHOW,	FALSE, NULL);
		    break;

		default:
		    new = LFS_MAPPED;
		    break;
	    }
	    break;

	case ACTION_CLOSE:
	case WIN_UNMAP_NOTIFY:
	    dprintf((stderr, "event_proc: unmapped\n"));
	    /*
	     * FIX_ME: Due to the bug in olwm, we have to wait for the
	     * reparent notify, instead of just unmapnotify.
	     */
	    lfi->mapunmap = LFS_UNMAPPED;
	    break;

unmapped:
	    dprintf((stderr, "	Unmap is actually done\n"));
	    lfi->mapunmap = LFS_NOP;
	    switch (lfi->lfs) {
		case LFS_RESIZE_REQUESTED:
		    dprintf((stderr, "	being resized...\n"));
		    xv_set (lfi->frame,
			    XV_WIDTH,	lfi->f_width,
			    XV_HEIGHT,	lfi->f_height,
			    XV_X,	lfi->f_x,
			    XV_Y,	lfi->f_y,
			    NULL);
		    lfi->f_should_resize = FALSE;
		    /*
		     * Follow thru to the LFS_MAP_REQUESTED.
		     */
		    /*FALLTHROUGH*/

		case LFS_MAP_REQUESTED:
		    dprintf((stderr, "	being mapped upon request\n"));
		    xv_set(lfi->frame, XV_SHOW, TRUE, NULL);
		    new = LFS_BEING_MAPPED;
		    break;

		default:
		    new = LFS_UNMAPPED;
		    break;
	    }
	    break;

	case WIN_REPARENT_NOTIFY:
	    dprintf((stderr, "event_proc: reparent\n"));
	    switch (lfi->mapunmap) {
		case LFS_UNMAPPED:
		    goto unmapped;
	    }
	    break;

	default:
	    break;
    }
    lfi->lfs = new;
}
