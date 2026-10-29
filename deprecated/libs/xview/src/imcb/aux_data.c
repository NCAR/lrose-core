#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)aux.c 50.34 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */



#include <xview_private/i18n_impl.h>
#include <xview/base.h>
#include <xview/frame.h>
#include <xview/panel.h>
#include <xview/notice.h>
#include <xview_private/panel_impl.h>
#include <xview_private/aux_data.h>

#define GET_EVENT
#define	ITERM_BUFSIZE	1024

void   			aux_unpin_proc();

static Panel_setting	aux_post_event();
static Bool		aux_start_called = FALSE;
static Bool		aux_draw_called = FALSE;

static AuxFrameInfo	*aux_frame_info = NULL;
static AuxFrameInfo     *aux_get_frame();
static Frame            aux_get_p_frame();
static void             aux_get_win_position();

#ifdef  GET_EVENT
static    XKeyEvent     e;
#endif

static void             aux_get_defaults();
static AuxDefaults	aux_defaults;

/*#define	HACK	/* 馬鹿もーん　誰がひよわだと？ */



void
aux_start (ic, client_data, callback_data)
    XIC				ic;
    XPointer			client_data;
    XIMAuxStartCallbackStruct 	*callback_data;
{
    Panel_info	*panel;
    wchar_t	*aux_label = (wchar_t *)NULL;
    wchar_t	*aux_text = (wchar_t *)NULL;
    AuxInfo     *info;
    Item_info	*item_private;
    XKeyEvent   *event = (XKeyEvent *)callback_data->event;

    /* 
     * Create aux info structure 
     */
    if( !aux_info_create(client_data) ) {
	xv_error( NULL,
		ERROR_STRING,
		XV_MSG("Aux callback: out of memory"),
		0 );
    }
    info = (AuxInfo *)AUX_UDATA_INFO(client_data);

    /* 
     * Initialize aux info structure 
     */
    info->ic = ic;
    info->dpy = (Display *)XDisplayOfIM((XIM)XIMOfIC(ic));
    info->pe_cache = (XIMPreeditDrawCallbackStruct *) 
		     xv_alloc(XIMPreeditDrawCallbackStruct);
    info->pe_cache->text = (XIMText *)xv_alloc(XIMText);

    /* 
     * Auxiliary Frame creation: 
     * 
     * Make sure to reuse the same aux command frame. Traverse back to 
     * find the base window associated with the client window. Also make 
     * sure aux region frame always comes up in the same x,y location 
     * (ie. if user moves aux frame, remember those coordinates).
     */
    info->frame_info = aux_get_frame(info->dpy, callback_data);
    info->frame = info->frame_info->frame;
    if( !info->frame )
        goto err_return;

    /* 
     * Disable input to the Auxialiary panel
     */
    info->panel = (Panel)xv_get(info->frame ,FRAME_CMD_PANEL, NULL);
    if( !info->panel )
        goto err_return;

    xv_set(info->panel,
            WIN_IGNORE_X_EVENT_MASK,
	    (KeyPress|KeyRelease|ButtonPress|ButtonRelease), NULL);

    /*
     * Initialize Aux region panel items 
     */
    if( callback_data->label->length > 0 ) {
    	if( callback_data->label->encoding_is_wchar )
    		aux_label = callback_data->label->string.wide_char;
    	else {
       		int	len_plus_one;
		len_plus_one = 1 + callback_data->label->length;
		aux_label = (wchar_t *)malloc( len_plus_one * sizeof(wchar_t) );
		mbstowcs(aux_label, callback_data->label->string.multi_byte, 
				len_plus_one);
   	 }
    }

    if( callback_data->text->length > 0 ) {
    	if( callback_data->text->encoding_is_wchar )
    		aux_text = callback_data->text->string.wide_char;
    	else {
        	int	len_plus_one;
		len_plus_one = 1 + callback_data->text->length;
		aux_text = (wchar_t *)malloc( len_plus_one * sizeof(wchar_t) );
		mbstowcs(aux_text, callback_data->text->string.multi_byte, 
				len_plus_one);
    	}
    }

    if( aux_label  && aux_text ) {
    	info->item = (Panel_item)xv_create( info->panel,	PANEL_TEXT,
			XV_X,				10,
			XV_Y,				10,
			PANEL_LABEL_STRING_WCS,		aux_label,
			PANEL_VALUE_STORED_LENGTH,	128,
			PANEL_VALUE_WCS,		aux_text,
			PANEL_NOTIFY_LEVEL,		PANEL_ALL,
			PANEL_NOTIFY_PROC,		aux_post_event,
			XV_KEY_DATA,	AUX_KEY_DATA,	info,
			NULL );
    }
    else if( aux_label ) {
	info->item = (Panel_item)xv_create( info->panel,        PANEL_TEXT,
                        XV_X,                           10,
                        XV_Y,                           10,
                        PANEL_LABEL_STRING_WCS,         aux_label,
			PANEL_VALUE_STORED_LENGTH,	128,
                        PANEL_NOTIFY_LEVEL,             PANEL_ALL,
                        PANEL_NOTIFY_PROC,              aux_post_event,
                        XV_KEY_DATA,    AUX_KEY_DATA,   info,
                        NULL );
    }
    else if ( aux_text ) {
	info->item = (Panel_item)xv_create( info->panel,        PANEL_TEXT,
                        XV_X,                           10,
                        XV_Y,                           10,
                        PANEL_VALUE_WCS,                aux_text,
			PANEL_VALUE_STORED_LENGTH,	128,
                        PANEL_NOTIFY_LEVEL,             PANEL_ALL,
                        PANEL_NOTIFY_PROC,              aux_post_event,
                        XV_KEY_DATA,    AUX_KEY_DATA,   info,
                        NULL );
    }
    else {
	info->item = (Panel_item)xv_create( info->panel,        PANEL_TEXT,
                        XV_X,                           10,
                        XV_Y,                           10,
                        PANEL_NOTIFY_LEVEL,             PANEL_ALL,
                        PANEL_NOTIFY_PROC,              aux_post_event,
                        XV_KEY_DATA,    AUX_KEY_DATA,   info,
                        NULL );
    }


    if( !callback_data->label->encoding_is_wchar
		&& callback_data->label->length > 0 )
	free(aux_label);
    if( !callback_data->text->encoding_is_wchar 
		&& callback_data->text->length > 0 )
	free(aux_text);

    if( !info->item )
	goto err_return; 

    /*
     * Initialize preedit text for panel item
     */
/* get the private handle, allocate buffer for intermediate text and
 * attach it to the panel.
 */
    panel = PANEL_PRIVATE(info->panel);
    panel->preedit->text->string.wide_char = 
	(wchar_t *) malloc(ITERM_BUFSIZE*sizeof(wchar_t));
    panel->preedit->text->feedback =
	(XIMFeedback *) malloc(ITERM_BUFSIZE* sizeof (XIMFeedback));

    /* store the current_caret_offset */
    item_private = ITEM_PRIVATE( info->item );
    panel->preedit_item = panel->kbd_focus_item = item_private;
    /*panel->kbd_focus_item->item_type = PANEL_TEXT_ITEM;*/
    ml_panel_saved_caret(item_private);


    /*
     * We should fit the panel and frame as long as the user has not
     * resized the Aux region.
     */
    if (info->frame_info->f_width == 0 && info->frame_info->f_height == 0) {

	Rect	*rect;

    	window_fit(info->panel);
    	window_fit(info->frame);
	rect = (Rect *)xv_get(info->frame, WIN_RECT);	
/*
	info->frame_info->f_x = rect->r_left;
	info->frame_info->f_y = rect->r_top;
*/
	info->frame_info->f_width = rect->r_width;
	info->frame_info->f_height = rect->r_height;
    }

    xv_set(info->frame ,XV_SHOW, TRUE, NULL );
    
    /*
    return XIMCB_Success;
    */

    aux_start_called = TRUE;
#ifdef  GET_EVENT
    e = *(callback_data->event);
#endif
    return;

    /*
     * Error handling for auxiliary region creation failure.
     */

err_return:     /* xv-object creation failure */
    xv_error( NULL,
	ERROR_STRING,
	XV_MSG("Aux callback: failed to create xv_object "),
	0 );
    /*
    return XIMCB_FatalError;
    */
}


aux_info_create( client_data )
    XPointer        client_data;
{
    AuxInfo     *data;
    Xv_opaque   *dum;
	 
    dum = (Xv_opaque *)client_data;
    data = (AuxInfo *)malloc( sizeof( AuxInfo ) );
    if( !data )
     	return 0;
    *dum = (Xv_opaque)data;
     	return 1;
}


void
aux_draw(ic, client_data, callback_data)
    XIC					ic;
    XPointer				client_data;
    XIMPreeditDrawCallbackStruct        *callback_data;
{
    Panel_info 		*panel;
    Item_info 		*ip, *item_private;
    AuxInfo    		*info;
    AuxFrameInfo	*afi;
    int 		i, length;
    XIMPreeditDrawCallbackStruct *preedit_changes;

#ifdef	HACK
    if( !aux_start_called ){
	aux_start(ic, client_data, callback_data);
	return;
    }
#endif

    info = (AuxInfo *)AUX_UDATA_INFO(client_data);
    panel = PANEL_PRIVATE(info->panel);

    /* 
     *  Figure out location and size of the aux frame window.
     *  based on aux defaults, and can be reset by user.
     */

    if (aux_get_draw_info(info) < 0)
	return;
    afi = info->frame_info;

    if (aux_defaults.window_should_fit) {

       xv_set(afi->frame,
		XV_X, afi->f_x,
		XV_Y, afi->f_y,
		XV_WIDTH, afi->f_width,
		XV_HEIGHT, afi->f_height,
		NULL);

       if(xv_get(info->frame, XV_SHOW) == FALSE)
         	xv_set(info->frame, XV_SHOW, TRUE, NULL);
    }

    ip = ITEM_PRIVATE( info->item );

    preedit_changes = (XIMPreeditDrawCallbackStruct *)callback_data;

    if (!aux_draw_called) {
	/*
	 *  If this is the first time that aux_draw is called 
	 *  we need to copy ip->panel->preedit with a copy of
	 *  the preedit info from the input window.
	 */
	ip->panel->preedit->caret = preedit_changes->caret;
	ip->panel->preedit->chg_first = preedit_changes->chg_first;
	ip->panel->preedit->chg_length = preedit_changes->chg_length;

	if (preedit_changes->text) {
	   ip->panel->preedit->text->length = preedit_changes->text->length;
	   ip->panel->preedit->text->encoding_is_wchar = 
		preedit_changes->text->encoding_is_wchar;
	   if (ip->panel->preedit->text->encoding_is_wchar) {
	      for (i = 0; i < preedit_changes->text->length; i++) {
		   ip->panel->preedit->text->feedback[i] = 
		     preedit_changes->text->feedback[i];
		   ip->panel->preedit->text->string.wide_char[i] =
		     preedit_changes->text->string.wide_char[i];
	      }
	   } else {
	      for (i = 0; i < preedit_changes->text->length; i++) {
		ip->panel->preedit->text->feedback[i] = 
		     preedit_changes->text->feedback[i];
		ip->panel->preedit->text->string.multi_byte[i] =
		     preedit_changes->text->string.multi_byte[i];
	      }
	   }	   
	}

    }
    else
    	cache_text_state(preedit_changes, ip->panel->preedit);

    /* Make sure we null terminate the string */
    length = ip->panel->preedit->text->length;
    ip->panel->preedit->text->string.wide_char[length] = 0;

    if (preedit_changes->text) {
       if (!preedit_changes->text->feedback)
          XV_BZERO(ip->panel->preedit,
          sizeof(XIMFeedback)*ip->panel->preedit->text->length);
    }

/* if (ip->ignore_im != TRUE) ml_panel_display_interm(ip);*/
    ml_panel_display_interm(ip);

    aux_draw_called = TRUE;

    /*
    return XIMCB_Success;
    */
    
    return;
}

void
aux_done(ic, client_data, callback_data)
    XIC		ic;
    XPointer	client_data;
    XPointer	*callback_data;
{
    Panel_info *panel;
    AuxInfo    *info;
    Item_info  *item_private;

    info = (AuxInfo *)AUX_UDATA_INFO(client_data);
    panel = PANEL_PRIVATE(info->panel);
    item_private = ITEM_PRIVATE( info->item );
	

    /* saved the caret offset */
    ml_panel_saved_caret(item_private);

    /*
    free ((char *)item_private->panel->preedit);
    free ((char *)item_private->panel->preedit->text->string.wide_char); 
    item_private->panel->preedit->text->string.wide_char[0] = 0;
    item_private->panel->preedit->text->length = 0;
    */

    xv_set(info->frame, XV_SHOW, FALSE, NULL);
    xv_destroy(info->item); 
    free( info );
    
    aux_start_called = FALSE;
    aux_draw_called = FALSE;

}	

static Panel_setting	aux_post_event(item, event)
    Panel_item	item;
    Event	*event;
{

/*
 * Can not use Event proc. JOWN201  1/15/91 
 */
    
}

void
aux_resize_handler(frame, event, arg, type)
        Window                  frame;
        Event                   *event;
        Notify_arg              arg;
        Notify_event_type       type;
{


	/*
	 *  If we get a resize event we want to resize the panel
	 *  text item appropriately. (Ex. Should have PANEL_DISPLAY_LENGTH
	 *  adjust appropriately, depending on size or width of the panel.)
	 */
        /* 
	 * Make sure this is not a synthetic resize event.  
	 * Synthetic resize events generated when window is moved.
         */
        if (event_action(event) == WIN_RESIZE && 
            event_xevent(event)->xconfigure.send_event == 0) {

	    Panel 	panel;
	    Panel_item 	item;
     	    Panel_info  *pprivate;
            int         width, n, stored_length, display_length;

	    panel = (Panel) xv_get(frame, FRAME_CMD_PANEL, NULL);
            pprivate = PANEL_PRIVATE(panel);
	    item = ITEM_PUBLIC(pprivate->preedit_item);

	     (void)notify_next_event_func(frame, event, arg, type);

            /*
             * Set the display width of the fillin field to extend to the
             * right edge of the panel.  This will vary depending on how the
             * user resizes the tool
             */
             width = (int)xv_get(panel, XV_WIDTH) -
                        (int)xv_get(item, PANEL_VALUE_X) - 5;
 
             n = width / (int)xv_get(xv_get(panel, XV_FONT), FONT_COLUMN_WIDTH);
 
	     stored_length = xv_get(item, PANEL_VALUE_STORED_LENGTH);
	     display_length = xv_get(item, PANEL_VALUE_DISPLAY_LENGTH);
             if (n < 5)
                n = 5;
             else if (n > stored_length) 
                n = stored_length;
 
/*
	     printf("width: %d, PANEL_VALUE_STORED_LENGTH: %d, PANEL_VALUE_DISPLAY_LENGTH: %d\n", width, stored_length, display_length);
*/
             xv_set(item, PANEL_VALUE_DISPLAY_LENGTH, n, NULL);

	     /* need to reset the rect information for frame info*/

        } else {
		(void)notify_next_event_func(frame, event, arg, type);
	}
}


/*
 * Frame handler, it will cache the frame per base frame basis.
 */
static AuxFrameInfo *
aux_get_frame(dpy, cb_data)
    Display				*dpy;
    XIMAuxStartCallbackStruct		*cb_data;
{
    AuxFrameInfo	*afi;
    AuxFrameInfo	*pafi;
    XFontSetExtents	*font_set_extents;
    Xv_object		 p_obj;
    Frame		 p_frame;
    Rect		*rect;
    XFontStruct         *font_struct;

    p_obj = (Xv_object) win_data(dpy, cb_data->event->window);
    p_frame = aux_get_p_frame(p_obj);
    for (pafi = afi = aux_frame_info; afi != NULL; pafi = afi, afi = afi->next)
	if (afi->dpy == dpy && afi->p_frame == p_frame)
	    break;

    if (afi == NULL) {

	if (aux_defaults.has_been_initialized == FALSE) {
	    aux_get_defaults(XV_LC_INPUT_LANG);
	    aux_defaults.has_been_initialized = TRUE;
	}

	if ((afi = (AuxFrameInfo *)xv_alloc(AuxFrameInfo)) == NULL)
	    return NULL;
	if (aux_frame_info == NULL)
	    aux_frame_info = afi;
	else
	    pafi->next = afi;

	/*
	 * Find aux window position
	 */
	afi->dpy = dpy;
	afi->p_obj = p_obj;
	afi->p_frame = p_frame;
	aux_get_win_position(afi, cb_data);

	/*
	 * Create command frame for Aux region
	 */
	afi->frame = (Frame) xv_create(afi->p_frame, FRAME_CMD,
		XV_X,				afi->f_x,
		XV_Y,				afi->f_y,
		OPENWIN_NO_MARGIN,		TRUE,
		FRAME_SHOW_RESIZE_CORNER,	TRUE,
		FRAME_CLOSED,			FALSE,
		XV_SHOW,			FALSE,
		WIN_USE_IM,			FALSE,
                FRAME_LABEL,
			XV_MSG("Auxiliary Region"),
		FRAME_CMD_POINTER_WARP,		FALSE,
		WIN_SAVE_UNDER,			TRUE,
		WIN_CONSUME_EVENT,		WIN_CONFIGURE_REQUEST,
		FRAME_INHERIT_COLORS,		TRUE,
		FRAME_DONE_PROC,		aux_unpin_proc,
		NULL);
	if (afi->frame == NULL)
	    return NULL;

	afi->f_xwin = xv_get(afi->frame, XV_XID);

	if (aux_defaults.font_name != NULL) {
		if ((afi->xv_font = xv_find(afi->frame, FONT,
				FONT_SET_SPECIFIER,	aux_defaults.font_name,
				NULL)) == NULL) {
		    char	 buf[100];

		    (void) sprintf(buf,
		        XV_MSG("aux: font(%s) couldn't find, use default"),
			aux_defaults.font_name);
		    xv_error(NULL, ERROR_STRING, buf, NULL);
		}
	}

	if (afi->xv_font == NULL)
	    afi->xv_font = xv_get(afi->frame, XV_FONT);

	afi->afont = (XFontSet) xv_get(afi->xv_font, XV_XID);
	if ((font_struct = XQueryFont(dpy, afi->afont)) == NULL) {
		/*
		 * This should not happen, but if its ever happen.  We
		 * should get around the problem, instead of just
		 * telling the failar.
		 */
		return NULL;
	}
	afi->achar_width = font_struct->max_bounds.width;
	/*
	 * Should not free the font_struct here.  At least current
	 * Sun's implementation of mltext refers to the same physical
	 * structure by the Xlib(mltext) itself.
	 */
	afi->font_set = (XFontSet) xv_get(afi->xv_font, FONT_SET_ID);
	font_set_extents = XExtentsOfFontSet(afi->font_set);
	afi->char_ascent = -font_set_extents->max_logical_extent.y;
	afi->char_descent = font_set_extents->max_logical_extent.height
			  + font_set_extents->max_logical_extent.y;
	/*
	 * FIX_ME:
	 * Waiting for the BugID 1062587 (Synopsis:  XExtentsOfFontSet
	 * does not return right value for the
	 * max_logical_extent.width) to be fixed.
	 *
	 * afi->char_width = font_set_extents->max_logical_extent.width;
	 */
	afi->char_width = font_set_extents->max_ink_extent.width;
	afi->char_height = font_set_extents->max_logical_extent.height;

	afi->panel = (Panel) xv_get(afi->frame, FRAME_CMD_PANEL,
				    NULL);

        notify_interpose_event_func(afi->frame, aux_resize_handler, NOTIFY_SAFE);
	afi->f_width = 0;
	afi->f_height = 0;
    } else {
	rect = (Rect *) xv_get(afi->frame, WIN_RECT);
	afi->f_x = rect->r_left;
	afi->f_y = rect->r_top;
	afi->f_width = rect->r_width;
	afi->f_height = rect->r_height;
    }

    return afi;
}


/*
 * This function gets handlers of pre-edit windows.( Frame and the
 * associated object ) to enable Aux frame a command-frame, and also,
 * to make keyboard grab possible.
 */
static Frame
aux_get_p_frame(p_obj)
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
aux_get_win_position(afi, cb_data)
    register AuxFrameInfo		*afi;
    XIMLookupStartCallbackStruct        *cb_data;
{
    Display	*dpy;
    Rect	*rect;

    
 /** overall window geometry **/
#ifdef  GET_EVENT
    dpy = afi->dpy;

    afi->sc_width = DisplayWidth(dpy, DefaultScreen(dpy));
    afi->sc_height = DisplayHeight(dpy, DefaultScreen(dpy));

    rect = (Rect *) xv_get(afi->p_frame, WIN_RECT);

    /* 
     * Position aux frame relative to the parent frame
     */
    afi->f_x = rect->r_left + aux_defaults.window_off_x;
    afi->f_y = rect->r_top + aux_defaults.window_off_y;

#else
    info->sc_x = DisplayWidth(info->dpy, DefaultScreen(info->dpy));
    info->sc_y = DisplayHeight(info->dpy, DefaultScreen(info->dpy));
    info->f_x = 100;
    info->f_y = 100;
#endif   
}

void
aux_unpin_proc(frame)
   Frame		frame;
{

	notice_prompt(frame, NULL,
        	NOTICE_MESSAGE_STRING,
                XV_MSG("You are in conversion mode.\n"
                       "Auxialiary Region window cannot be dismissed."),
                NOTICE_BUTTON, XV_MSG("Continue"),
                NULL);

}


/*
 * This function retrieves user-preference setings from resource
 * database.
 */
static void
aux_get_defaults(locale)
    Attr_attribute      locale;
{
        
    defaults_set_locale(NULL, locale);
 
    aux_defaults.keygrab = defaults_get_boolean("aux.keygrab",
                                                "Aux.Keygrab",
                                                TRUE);
 
    aux_defaults.threed = defaults_get_boolean("OpenWindows.3DLook.Color",
                                            
                                               "OpenWindows.3DLook.Color",
                                               TRUE);
 
    if ((aux_defaults.font_name = defaults_get_string("aux.font",
                                                      "Aux.Font",
                                                      NULL)) != NULL)
        aux_defaults.font_name = xv_strsave(aux_defaults.font_name);
 
 
/*
    aux_defaults.window_margin_x
                = defaults_get_integer_check("aux.window.margin.x",
                                             "Aux.Window.Margin.X",
                                             15, 0, 100000);
    aux_defaults.window_margin_y
                = defaults_get_integer_check("aux.window.margin.y",
                                             "Aux.Window.Margin.Y",
                                             15, 0, 100000);
*/
    /* 
     * Horizontal and vertical offset of aux region window from
     * base window.
     */
    aux_defaults.window_off_x
                = defaults_get_integer_check("aux.window.off.x",
                                             "Aux.Window.Off.X",
                                             20, 0, 100000);
    aux_defaults.window_off_y
                = defaults_get_integer_check("aux.window.off.y",
                                             "Aux.Window.Off.Y",
                                             10, 0, 100000);
/*
    aux_defaults.window_width
                = defaults_get_integer_check("aux.window.width",
                                             "Aux.Window.width",
                                             20, 0, 100000);
    aux_defaults.window_height
                = defaults_get_integer_check("aux.window.height",
                                             "Aux.Window.height",
                                             10, 0, 100000);
*/
    /* 
     * If true, aux region will try to fit within the screen 
     */
    aux_defaults.window_should_fit
                = defaults_get_boolean("aux.window.fit",
                                       "Aux.Window.Fit", TRUE);

    /* 
     * Horizontal and vertical screen margin used when 
     * aux_defaults.window_should_fit is TRUE.
     */
    aux_defaults.screen_margin_x
                = defaults_get_integer_check("aux.screen.margin.x",
                                             "Aux.Screen.Margin.X",
                                             20, 0, 100000);
    aux_defaults.screen_margin_y
                = defaults_get_integer_check("aux.screen.margin.y",
                                             "Aux.Screen.Margin.Y",
                                             20, 0, 100000);
 
    defaults_set_locale(NULL, XV_NULL);
}

/*
 *       This function sets geometry of AUX window.
 */
static
aux_get_draw_info(info)
    register AuxInfo    *info;
{
    AuxFrameInfo        *afi;
    Rect		*rect;


    afi = info->frame_info;

    rect = (Rect *)xv_get(afi->frame, WIN_RECT);
    afi->f_x = rect->r_left;
    afi->f_y = rect->r_top;
    afi->f_width = rect->r_width;
    afi->f_height = rect->r_height;
    /*   
     * Now, make sure new window fits into the screen.
     */
    if (aux_defaults.window_should_fit) {
    	if ((afi->f_width + afi->f_x)
             > (afi->sc_width - aux_defaults.screen_margin_x))
            afi->f_x = afi->sc_width - afi->f_width
                        - aux_defaults.screen_margin_x;
    	if (afi->f_x < aux_defaults.screen_margin_x)
            afi->f_x = aux_defaults.screen_margin_x;
 
        if ((afi->f_height + afi->f_y)
             > (afi->sc_height - aux_defaults.screen_margin_y))
            afi->f_y = afi->sc_height - afi->f_height
                        - aux_defaults.screen_margin_y;
    	if (afi->f_y < aux_defaults.screen_margin_y)
            afi->f_y = aux_defaults.screen_margin_y;
    }
 
    return 0;
}
