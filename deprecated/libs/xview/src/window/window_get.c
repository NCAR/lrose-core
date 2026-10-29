#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)window_get.c 20.109 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE 
 *	file for terms of the license.
 */

#include <stdio.h>
#include <xview_private/windowimpl.h>
#include <xview_private/i18n_impl.h>
#include <pixrect/pixrect.h>
#include <pixrect/pixfont.h>
#include <xview/notify.h>
#include <xview/win_screen.h>
#include <xview/rectlist.h>
#include <xview/xv_xrect.h>
#include <xview/cms.h>
#ifdef OW_I18N
#ifdef FULL_R5
#include <xview/frame.h>
#include <xview_private/fm_impl.h>
#include <xview_private/svr_impl.h>
#endif /* FULL_R5 */
#endif /* OW_I18N */

/*
 * Private
 */

static int      get_mask_bit();
static Xv_opaque  window_empty_event_proc();
static struct timeval alarmdata;
#ifdef OW_I18N
static XIC xv_window_create_ic();
#ifdef FULL_R5
#define XV_SUPPORTED_STYLE_COUNT               12
#else /*FULL_R5 */
#define XV_SUPPORTED_STYLE_COUNT               15
#endif /* FULL_R5 */
#endif /* OW_I18N */


/* BUG: should be in hdr file */
extern void		win_xmask_to_im(); /* defined in win_input.c */
extern unsigned int	win_im_to_xmask(); /* defined in win_input.c */
Xv_private Xv_font	xv_find_olglyph_font(); /* defined in font.c */


/* VARARGS2 */
Xv_public       Xv_opaque
#ifdef ANSI_FUNC_PROTO
window_get(Xv_Window win_public, Window_attribute attr, ...)
#else
window_get(win_public, attr, va_alist)
    Xv_Window       win_public;
    Window_attribute attr;
va_dcl
#endif
{
    va_list valist;
    Xv_opaque getvalue;

    VA_START( valist, attr );
    getvalue =  xv_get_varargs(win_public, attr, valist);
    va_end( valist );
    return getvalue;
}


Pkg_private     Xv_opaque
window_get_attr(win_public, status, attr, valist)
    Xv_Window       win_public;
    int            *status;
    Window_attribute attr;
    va_list         valist;
{
    register Window_info *win = WIN_PRIVATE(win_public);
    register Xv_Drawable_info *info;
    register Xv_opaque v = 0;
    int             vp;

    DRAWABLE_INFO_MACRO(win_public, info);
    switch (attr) {

      case XV_SCREEN:
	v = xv_screen(info);
	break;

      case WIN_FRAME:
	for (v = win_public; v; v = (Xv_opaque) WIN_PRIVATE(v)->parent) {
	    if (WIN_PRIVATE(v)->top_level)
		break;
	}
	break;

      case WIN_TOP_LEVEL:
	v = (Xv_opaque) win->top_level;
	break;

      case WIN_RECT_INFO:
	v = (Xv_opaque) win->rect_info;
	break;

      case WIN_BORDER:
	v = (Xv_opaque) win->has_border;
	break;

      case WIN_IS_IN_FULLSCREEN_MODE:
	v = (Xv_opaque) win->in_fullscreen_mode;
	break;

      case WIN_NO_CLIPPING:
	v = (Xv_opaque) win->no_clipping;
	break;

      case WIN_PARENT:
	v = win->parent;
	break;

      case WIN_CLIENT_DATA:
	v = (Xv_opaque) win->client_data;
	break;
      case WIN_X_PAINT_WINDOW:
	v = (Xv_opaque) win->x_paint_window;
	break;

      case WIN_TOP_LEVEL_NO_DECOR:
	v = (Xv_opaque) win->top_level_no_decor;
	break;

      case WIN_IS_ROOT:
	if (win->owner == NULL)
	    v = (Xv_opaque) TRUE;
	break;

      case WIN_CMS_NAME:
	v = (Xv_opaque) xv_get(info->cms, CMS_NAME);
	break;

      case WIN_FOREGROUND_COLOR:
	v = (Xv_opaque) xv_cms_fg(info);
	break;

      case WIN_BACKGROUND_COLOR:
	v = (Xv_opaque) xv_cms_bg(info);
	break;

      case WIN_GLYPH_FONT:
	if (!win->glyph_font)
	    win->glyph_font = xv_find_olglyph_font(win->font);
	v = (Xv_opaque) win->glyph_font;
	break;

      case WIN_LAYOUT_PROC:
	v = (Xv_opaque) (win->layout_proc);
	break;

      case WIN_KBD_FOCUS:
	v = (Xv_opaque) win->has_kbd;
	break;

      case WIN_INPUT_MASK:{
	    static Inputmask im;

	    (void) win_xmask_to_im(win->xmask, &im);
	    v = (Xv_opaque) & im;
	    break;
	}

      case WIN_DESIRED_WIDTH:
	v = (Xv_opaque) win->desired_width;
	break;

      case WIN_TYPE:
	/* SunView1.X compatibility */
	v = (Xv_opaque) WINDOW_TYPE;
	break;

      case WIN_IS_CLIENT_PANE:
	v = (Xv_opaque) win->is_client_pane;
	break;

      case XV_SHOW:
      case WIN_MAP:
	v = (Xv_opaque) win->map;
	break;


      case WIN_OUTER_RECT:
	{
	    static Rect     rec;
	    (void) win_appeal_to_owner(FALSE, win, (Xv_opaque) WIN_GET_RECT, (Xv_opaque) & rec);
	    if (win->has_border)
		rect_borderadjust(&rec, WIN_DEFAULT_BORDER_WIDTH);
	    v = (Xv_opaque) & rec;
	}
	break;

      case WIN_X_CLIP_RECTS:{
	    static Xv_xrectlist xrects;
	    Rectlist       *rl;
	    extern Rectlist *win_get_damage();

	    if ((rl = win_get_damage(win_public)) != RECTLIST_NULL) {
		xrects.count = win_convert_to_x_rectlist(rl, xrects.rect_array,
							 XV_MAX_XRECTS);
		v = (Xv_opaque) & xrects;
	    } else {
		v = (Xv_opaque) NULL;
	    }
	    break;
	}

      case WIN_BIT_GRAVITY:
	{
	    XWindowAttributes win_attrs;
	    
	    (void) XGetWindowAttributes(xv_display(info), xv_xid(info), &win_attrs);
	    v = (Xv_opaque) win_attrs.bit_gravity;
        }
	break;
	
      case WIN_CMD_LINE:
	v = (Xv_opaque) win->cmdline;
	break;

      case WIN_INPUT_ONLY:
	v = (Xv_opaque) win->input_only;
	break;

      case WIN_FINDINTERSECT:{
	    /* return win number of child underlying cursor */
	    short           x, y;

	    x = va_arg(valist, int);
	    y = va_arg(valist, int);
	    v = (Xv_opaque) win_findintersect(win_public, x, y);
	}
	break;

      case WIN_ALARM_DATA:
	/* return pointer to static that has alarm info from environment */
	/* (someday this will be more than just time values) */
	alarmdata.tv_sec = 0;
	alarmdata.tv_usec = 100000;

	v = (Xv_opaque) & alarmdata;
	break;

      case WIN_DESIRED_HEIGHT:
	v = (Xv_opaque) win->desired_height;
	break;

      case WIN_COLUMNS:
	(void) win_appeal_to_owner(FALSE, win, (Xv_opaque) WIN_GET_WIDTH,
				   (Xv_opaque) & vp);
	vp -= win->left_margin + win->right_margin;
	v = (Xv_opaque) (vp / (actual_column_width(win) + win->column_gap));
	break;

      case WIN_CURSOR:
	v = win->cursor;
	break;

      case WIN_EVENT_PROC:
	v = (Xv_opaque) (win->event_proc);
	if (!v)
	    v = (Xv_opaque) window_empty_event_proc;
	break;

      case WIN_FONT:
	v = (Xv_opaque) win->font;
	break;

      case WIN_HEIGHT:
	(void) win_appeal_to_owner(FALSE, win, (Xv_opaque) WIN_GET_HEIGHT,
				   (Xv_opaque) & vp);
	v = (Xv_opaque) vp;	/* Move to register */
	break;

      case WIN_MENU:
	v = win->menu;
	break;

      case WIN_NOTIFY_SAFE_EVENT_PROC:
	v = (Xv_opaque) win->notify_safe_event_proc;
	break;

      case WIN_NOTIFY_IMMEDIATE_EVENT_PROC:
	v = (Xv_opaque) win->notify_immediate_event_proc;
	break;

      case WIN_OWNER:
	if (win->owner)
	    v = WIN_PUBLIC(win->owner);
	break;

      case WIN_PERCENT_HEIGHT:
	/* % height needs owner */
	if (!win->owner)
	    break;

	v = (Xv_opaque)
	    ((xv_get(win_public, WIN_GET_HEIGHT) + win->owner->column_gap)*100 /
	     (xv_get(WIN_PUBLIC(win->owner), WIN_HEIGHT)
	      - win->owner->top_margin - win->owner->bottom_margin));
	break;

      case WIN_PERCENT_WIDTH:
	/* % width needs owner */
	if (!win->owner)
	    break;

	v = (Xv_opaque)
	    ((xv_get(win_public, WIN_GET_WIDTH) + win->owner->row_gap) * 100 /
	     (xv_get(WIN_PUBLIC(win->owner), WIN_WIDTH)
	      - win->owner->left_margin - win->owner->right_margin));
	break;

      case WIN_RECT:
	{
	    static Rect     rec;
	    (void) win_appeal_to_owner(FALSE, win, (Xv_opaque) WIN_GET_RECT, (Xv_opaque) & rec);
	    v = (Xv_opaque) & rec;
	}
	break;

      case WIN_ROWS:
	(void) win_appeal_to_owner(FALSE, win, (Xv_opaque) WIN_GET_HEIGHT, (Xv_opaque) & vp);
	vp -= win->top_margin + win->bottom_margin;
	v = (Xv_opaque) (vp / (actual_row_height(win) + win->row_gap));
	break;

      case WIN_SCREEN_RECT:
	{
	    static Rect     rect;

	    rect = *(Rect *) xv_get(xv_root(info), WIN_RECT);

	    v = (Xv_opaque) & rect;
	}
	break;

      case XV_DEPTH:
      case WIN_DEPTH:
      case WIN_DEPTH_V2:
	v = (Xv_opaque) xv_depth(info);
	break;

      case WIN_RETAINED:
	v = (Xv_opaque) win_get_retained(win_public);
	break;

      case WIN_WIDTH:
	(void) win_appeal_to_owner(FALSE, win, (Xv_opaque) WIN_GET_WIDTH, (Xv_opaque) & vp);
	v = (Xv_opaque) vp;	/* Move to register */
	break;

      case WIN_WINDOW_GRAVITY:
	{
	    XWindowAttributes win_attrs;
	    
	    (void) XGetWindowAttributes(xv_display(info), xv_xid(info), &win_attrs);
	    v = (Xv_opaque) win_attrs.win_gravity;
        }
	break;
	
      case WIN_X:
	(void) win_appeal_to_owner(FALSE, win, (Xv_opaque) WIN_GET_X, (Xv_opaque) & vp);
	v = (Xv_opaque) vp;	/* Move to register */
	break;

      case WIN_Y:
	(void) win_appeal_to_owner(FALSE, win, (Xv_opaque) WIN_GET_Y, (Xv_opaque) & vp);
	v = (Xv_opaque) vp;	/* Move to register */
	break;

      case WIN_TOP_MARGIN:
	v = (Xv_opaque) win->top_margin;
	break;

      case WIN_BOTTOM_MARGIN:
	v = (Xv_opaque) win->bottom_margin;
	break;

      case WIN_LEFT_MARGIN:
	v = (Xv_opaque) win->left_margin;
	break;

      case WIN_RIGHT_MARGIN:
	v = (Xv_opaque) win->right_margin;
	break;

      case WIN_ROW_HEIGHT:
	v = (Xv_opaque) actual_row_height(win);
	break;

      case WIN_COLUMN_WIDTH:
	v = (Xv_opaque) actual_column_width(win);
	break;

      case WIN_ROW_GAP:
	v = (Xv_opaque) win->row_gap;
	break;

      case WIN_COLUMN_GAP:
	v = (Xv_opaque) win->column_gap;
	break;

      case WIN_CONSUME_EVENT:{
	    Inputmask       im;

	    (void) win_xmask_to_im(win->xmask, &im);
	    v = (Xv_opaque) get_mask_bit(&im, va_arg(valist, Window_input_event), win_public);
	    break;
	}
      case WIN_CONSUME_X_EVENT_MASK:
      case WIN_X_EVENT_MASK:
	v = (Xv_opaque) win->xmask;
	break;

      case WIN_MOUSE_XY:{
	    static Rect     rect;
	    /* Rect struct is used to get mouse x and y */
	    win_getmouseposition(win_public, &rect.r_left, &rect.r_top);
	    v = (Xv_opaque) & rect;
	    break;
	}

      case WIN_MESSAGE_TYPE:
	return ((Xv_opaque) win->client_message.type);

      case WIN_MESSAGE_FORMAT:
	return ((Xv_opaque) win->client_message.format);

      case WIN_MESSAGE_DATA:
	return ((Xv_opaque) win->client_message.data.b);

      case WIN_FIT_HEIGHT:
      case WIN_FIT_WIDTH:
      case WIN_VERTICAL_SCROLLBAR:
      case WIN_HORIZONTAL_SCROLLBAR:	/* package will handle these */
	*status = XV_ERROR;
	break;

      case XV_ROOT:
	v = (Xv_opaque) xv_root(info);
	break;

      case WIN_INHERIT_COLORS:
	v = (Xv_opaque) win->inherit_colors;
	break;

      case XV_VISUAL_CLASS:
	v = (Xv_opaque)(xv_visual(info)->vinfo->class);
	break;

      case XV_VISUAL:
	v = (Xv_opaque)(xv_visual(info)->vinfo->visual);
	break;
	
      case WIN_BACKGROUND_PIXMAP:
	v = (Xv_opaque) win->background_pixmap;
	break;

      case WIN_COLLAPSE_EXPOSURES:
	v = (Xv_opaque) win->collapse_exposures;
	break;

      case WIN_COLLAPSE_MOTION_EVENTS:
	v = (Xv_opaque) win->collapse_motion_events;
	break;

      case WIN_CMS:
	v = (Xv_opaque) info->cms;
	break;

      case WIN_COLOR_INFO: {
	static Xv_Color_info	xv_color_info;

	xv_color_info.cms    = xv_cms(info);
	xv_color_info.cms_fg = xv_cms_fg(info);
	xv_color_info.cms_bg = xv_cms_bg(info);
	v = (Xv_opaque) &xv_color_info;
	break;
      }

      case WIN_X_COLOR_INDICES: {
	  v = (Xv_opaque)xv_get(xv_cms(info), CMS_INDEX_TABLE);
	break;
      }

      case WIN_ADD_DROP_ITEM: {
	  v = (Xv_opaque)win->dropSites;
	break;
      }

      case WIN_CMS_DATA: {
	  static Xv_cmsdata	*xv_cms_data = NULL;
	  static short		 xv_rgb_size = 0;
	  unsigned long		 cms_size;

	  if (!xv_cms_data) {
	      xv_cms_data = xv_alloc(Xv_cmsdata);
	  }

	  cms_size = (unsigned long)xv_get(xv_cms(info), CMS_SIZE);
	  if (xv_rgb_size < cms_size) {
	      if (xv_rgb_size) {
		  free(xv_cms_data->red);
		  free(xv_cms_data->green);
		  free(xv_cms_data->blue);
	      }

	      xv_cms_data->red = xv_alloc_n(unsigned char, cms_size );
	      xv_cms_data->green = xv_alloc_n(unsigned char, cms_size );
	      xv_cms_data->blue = xv_alloc_n(unsigned char, cms_size );
	      xv_rgb_size = cms_size;
	  }
	
	  xv_cms_data = (Xv_cmsdata *)
			  xv_get(xv_cms(info), CMS_CMS_DATA, xv_cms_data);
	  v = (Xv_opaque)xv_cms_data;
	  break;
     }	

#ifdef OW_I18N
      case WIN_IC: {
          if (win->win_use_im && !win->xic) {
 
                XIM              im;
                XIMStyles        *style = NULL;
                XIMStyle 	 supported_styles[XV_SUPPORTED_STYLE_COUNT];
                int              i,j;
                Xv_Drawable_info *info;
                Window           window;
                Xv_Server        server;
#ifdef FULL_R5
		Server_info	*server_private;
#endif

                server = XV_SERVER_FROM_WINDOW(win_public);

                /* Get the im from server in order to pass to XCreateIC */
                im = (XIM)xv_get(server, XV_IM);
                if (!im)
                    break;
#ifdef FULL_R5
		/*
		 *  Determine IM style is based on:
		 *	1. User specified input style
		 *	   a. command line option
		 *	   b. ~/.Xdefaults
		 *	   c. System Xdefaults 
		 *	2. Default supported locale-specific input style
		 *	3. Default supported input style
		 *
		 *       Currently the im style is associated on a
		 *	 per server basis.  This should be changed
		 *	 into a private attribute, but for now
		 *	 we are grabbing server_private->determined_style
		 *	 directly.
		 *  
		 */
		server_private = SERVER_PRIVATE(server);

                DRAWABLE_INFO_MACRO(xv_get(win_public, WIN_FRAME), info);
                window = xv_xid(info);
                   
                /* Create the IC with the determined style */
		if (win->x_im_style_mask == NULL)
		    win->x_im_style_mask = server_private->determined_im_style;	
		win->xic = xv_window_create_ic(win_public, im, window,
				     	       win->x_im_style_mask);

                if (win->xic) 
		    win->ic_created = TRUE;
#else /* FULL_R5 */
                XGetIMValues(im, XNQueryInputStyle, &style, NULL);
                   
                if (style) {
                   /*  Make a list of the styles we currently support */
                   supported_styles[0] = (XIMPreeditCallbacks | XIMStatusCallbacks |
                                           XIMLookupCallbacks | XIMAuxCallbacks);
                   supported_styles[1] = (XIMPreeditCallbacks | XIMStatusCallbacks |
                                           XIMLookupCallbacks);
                   supported_styles[2] = (XIMPreeditCallbacks | XIMStatusCallbacks |
                                           XIMAuxCallbacks);
                   supported_styles[3] = (XIMPreeditCallbacks | XIMStatusCallbacks);
                   supported_styles[4] = (XIMPreeditCallbacks | XIMStatusArea);
              	   supported_styles[5] = (XIMPreeditCallbacks | XIMStatusNothing);
                   
                   supported_styles[6] = (XIMPreeditPosition | XIMStatusArea);
                   supported_styles[7] = (XIMPreeditPosition | XIMStatusNothing);
                   
                   supported_styles[8] = (XIMPreeditArea | XIMStatusArea);
                   supported_styles[9] = (XIMPreeditArea | XIMStatusNothing);
                   
                   supported_styles[10] = (XIMPreeditNothing | XIMStatusArea);
                   supported_styles[11] = (XIMPreeditNothing | XIMStatusNothing);
                   
                   supported_styles[12] = (XIMPreeditCallbacks | XIMStatusNone);
                   supported_styles[13] = (XIMPreeditPosition | XIMStatusNone);
          	   supported_styles[14] = (XIMPreeditArea | XIMStatusNone);
                   
                   /* Find the matching style */
                   for (j = 0; j < XV_SUPPORTED_STYLE_COUNT; j++) {
                       for (i = 0; i < (int)style->count_styles; i++)
                           if (style->supported_styles[i] == supported_styles[j])
                                   goto Found_style;
                   }
Found_style:          
                   
                   DRAWABLE_INFO_MACRO(xv_get(win_public, WIN_FRAME), info);
                   window = xv_xid(info);
                   
                   /* Create the IC and register the callbacks */
                   win->xic = xv_window_create_ic(win_public, im, window,
                                     style->supported_styles[i]);
                        
                   if (win->xic) 
			win->ic_created = TRUE;
                   
                   if (style) 
			XFree(style);
                }
#endif /* FULL_R5 */
          }        
          v = (Xv_opaque)win->xic;
          break;
      } 

      case WIN_IC_CONVERSION:
	  if (win->win_use_im && win->xic) {

	     int     conv_on;
 
             if (!XGetICValues(win->xic, XNExtXimp_Conversion, &conv_on, 
		 NULL)){
                  /* This IM supports Sun Extensions */
		  if (conv_on == XIMEnable)
			win->ic_conversion = TRUE;
		  else
			win->ic_conversion = FALSE;
	     } 
          }
	  v = (Xv_opaque)win->ic_conversion;
	  break;

      case WIN_IC_COMMIT_STRING:
	  v = (Xv_opaque)win->win_ic_committed;
	  break;

      case WIN_IC_COMMIT_STRING_WCS:
	  v = (Xv_opaque)win->win_ic_committed_wcs;
	  break;

      case WIN_USE_IM:
          v = (Xv_opaque)win->win_use_im;
          break;

      case WIN_IC_ACTIVE:
          v = (Xv_opaque)win->ic_active;
          break;

#ifdef FULL_R5
      case WIN_X_IM_STYLE_MASK:
	  v = (Xv_opaque)win->x_im_style_mask;
	  break;

      case XV_IM_STYLES:
          v = (Xv_opaque) xv_get((Xv_opaque)XV_SERVER_FROM_WINDOW(win_public),
                                                XV_IM_STYLES);
          break;
#endif /* FULL_R5 */
#endif /* OW_I18N */

      case XV_LC_BASIC_LOCALE:
	return (Xv_opaque) xv_get((Xv_opaque)XV_SERVER_FROM_WINDOW(win_public),
						XV_LC_BASIC_LOCALE);

      case XV_LC_DISPLAY_LANG:
	return (Xv_opaque) xv_get((Xv_opaque)XV_SERVER_FROM_WINDOW(win_public),
						XV_LC_DISPLAY_LANG);

      case XV_LC_INPUT_LANG:
	return (Xv_opaque) xv_get((Xv_opaque)XV_SERVER_FROM_WINDOW(win_public),
						XV_LC_INPUT_LANG);

      case XV_LC_NUMERIC:
	return (Xv_opaque) xv_get((Xv_opaque)XV_SERVER_FROM_WINDOW(win_public),
						XV_LC_NUMERIC);

      case XV_LC_TIME_FORMAT:
	return (Xv_opaque) xv_get((Xv_opaque)XV_SERVER_FROM_WINDOW(win_public),
						XV_LC_TIME_FORMAT);
	      
      default:
	if (xv_check_bad_attr(&xv_window_pkg, attr) == XV_ERROR) {
	    *status = XV_ERROR;
	}
	break;

    }
    return v;
}


static int
get_mask_bit(mask, code, win_public)
    Inputmask      *mask;
    Window_input_event code;
    Xv_Window       win_public;
{
    int             v, i;

    switch (code) {
      case WIN_NO_EVENTS:
	v = (mask == 0);
	break;

      case WIN_ASCII_EVENTS:
	v = mask->im_flags & (IM_ASCII | IM_META);
	break;

      case WIN_UP_ASCII_EVENTS:
	v = mask->im_flags & (IM_NEGASCII | IM_NEGMETA);
	break;

      case WIN_UP_EVENTS:
	v = mask->im_flags & IM_NEGEVENT;
	break;

      case WIN_MOUSE_BUTTONS:
	v = (win_getinputcodebit(mask, MS_LEFT) &&
	     win_getinputcodebit(mask, MS_MIDDLE) &&
	     win_getinputcodebit(mask, MS_RIGHT)
	    );
	break;

      case WIN_LEFT_KEYS:
	v = 1;
	for (i = 1; i <= 15; i++)
	    v &= win_getinputcodebit(mask, KEY_LEFT(i));
	break;

      case WIN_RIGHT_KEYS:
	v = 1;
	for (i = 1; i <= 15; i++)
	    v &= win_getinputcodebit(mask, KEY_RIGHT(i));
	break;

      case WIN_META_EVENTS:
	v = mask->im_flags & IM_META;
	break;

      case WIN_UP_META_EVENTS:
	v = mask->im_flags & IM_NEGMETA;
	break;

      case WIN_TOP_KEYS:
	v = 1;
	for (i = 1; i <= 15; i++)
	    v &= win_getinputcodebit(mask, KEY_TOP(i));
	break;

      case WIN_IN_TRANSIT_EVENTS:
	v = mask->im_flags & IM_INTRANSIT;
	break;

      default:
	if (isworkstationdevid((int) code))
	    v = win_getinputcodebit(mask, (int) code);
	else {
	    /* see if key press events are enabled. */
	    unsigned int    xmask = win_im_to_xmask(win_public, mask);
	    Inputmask       im;

	    /*
	     * translating between im to xmask sets KeyPressMask if any key
	     * press events are enabled.  Translating xmask back to im will
	     * enable all key press events.
	     */
	    win_xmask_to_im(xmask, &im);
	    win_getinputcodebit(&im, KEY_LEFT(1));
	}
	break;

    }
    return (v != 0);
}

static Xv_opaque
window_empty_event_proc()
{
	short dummy = 0;
	return((Xv_opaque) dummy);
}

#ifdef OW_I18N
static XIC
xv_window_create_ic(win_public, im, xid, style)
    Xv_Window           win_public;
    XIM                 im;
    Window              xid;
    XIMStyle            style;
{
    Window_info         *win = WIN_PRIVATE(win_public);
    XIC                 xic;
    XVaNestedList       preedit_list, status_list;
#ifdef FULL_R5
    XRectangle          area;
    XPoint              spot_location;
    XFontSet		fontset;
    Cms			cms;
    Colormap            cmap;
    unsigned long       fg, bg;
    Pixmap              bg_pixmap;
    Frame		frame_public;
    Frame_class_info    *frame; 
#endif
    
    preedit_list = status_list = NULL;

#ifdef FULL_R5
    if (style & XIMPreeditCallbacks || style & XIMPreeditPosition ||
	style & XIMPreeditArea || style & XIMStatusArea ) {

	fontset = (XFontSet)xv_get(win->font, FONT_SET_ID);

	frame_public = xv_get(win_public, WIN_FRAME);
        frame = FRAME_PRIVATE(frame_public);
    }
    if (style & XIMPreeditCallbacks || style & XIMPreeditPosition ||
	style & XIMPreeditArea ) {
    	spot_location.x = 0;
    	spot_location.y = xv_get(win->font, FONT_DEFAULT_CHAR_HEIGHT);
    }	
#endif /* FULL_R5 */
    /*
     *  Create Preedit list based on selected style.
     */
    if (style & XIMPreeditCallbacks) {
	 /* On-the-spot input style requested */
         preedit_list = XVaCreateNestedList(NULL,
		       /* Must be specified for on-the-spot */		
                       XNPreeditStartCallback, &win->start_pecb_struct,
                       XNPreeditDrawCallback,  &win->draw_pecb_struct,
                       XNPreeditCaretCallback, &win->caret_pecb_struct,
                       XNPreeditDoneCallback,  &win->done_pecb_struct,
#ifdef FULL_R5
		       /* Specify for luc but may be ignored by im-server */
		       XNSpotLocation,         &spot_location,
                       XNFontSet,              fontset,
#endif
                       NULL);
    } else if (style & XIMPreeditPosition) {
	/* Over-the-spot input style requested */
#ifdef FULL_R5
	cms = xv_get(win_public, WIN_CMS);
	cmap = (Colormap)xv_get(cms, XV_XID);
	fg = (unsigned long)xv_get(cms, CMS_FOREGROUND_PIXEL);
	bg = (unsigned long)xv_get(cms, CMS_BACKGROUND_PIXEL);
	bg_pixmap = (Pixmap)xv_get(win_public, WIN_BACKGROUND_PIXMAP);
#endif /* FULL_R5 */

        preedit_list = XVaCreateNestedList(NULL,
#ifdef FULL_R5
		       /* Must be specified for over-the-spot */
                       XNSpotLocation,  &spot_location,
                       XNFontSet,       fontset,
		       XNColormap,	cmap,
		       XNForeground,	fg,
		       XNBackground,	bg,
		       XNBackgroundPixmap,  bg_pixmap,
#endif
                       NULL);
    } else if (style & XIMPreeditArea) {
	/* Off-the-spot input style requested */
        preedit_list = XVaCreateNestedList(NULL,
#ifdef FULL_R5
		       /* Must be specified for off-the-spot */	
                       XNFontSet,       fontset,
		       /* Specify for luc but may be ignored by im-server */
		       XNSpotLocation,  &spot_location,
		       /* XNArea,	&area,*/
#endif
                       NULL);
    } else if (style & XIMPreeditNothing) {
	/* Root-window input style requested */
        preedit_list = XVaCreateNestedList(NULL,
                       NULL);
    } else if (style & XIMPreeditNone) {
	/* No preedit style requested */
	/* How should we handle this?? */
    }
 
    /*
     *  Create Status list based on selected style.
     */
    if (style & XIMStatusCallbacks) {
	 /* On-the-spot input style requested */
         status_list = XVaCreateNestedList(NULL,
                       XNStatusStartCallback, &win->start_stcb_struct,
                       XNStatusDrawCallback,  &win->draw_stcb_struct,
                       XNStatusDoneCallback,  &win->done_stcb_struct,
                       NULL);
    } else if (style & XIMStatusArea) {
	/* Over-the-spot input style requested */
#ifdef FULL_R5
	Rect 	*status_rect;

	status_rect = (Rect *)xv_get(frame_public, FRAME_IMSTATUS_RECT);	
	area.x = status_rect->r_left;
	area.y = status_rect->r_top;
	area.width = status_rect->r_width;
	area.height = status_rect->r_height;

	cms = xv_get(frame->imstatus, WIN_CMS);
	cmap = (Colormap)xv_get(cms, XV_XID);
	fg = (unsigned long)xv_get(cms, CMS_FOREGROUND_PIXEL);
	bg = (unsigned long)xv_get(cms, CMS_BACKGROUND_PIXEL);
	bg_pixmap = (Pixmap)xv_get(frame->imstatus, WIN_BACKGROUND_PIXMAP);
#endif
        status_list = XVaCreateNestedList(NULL,
#ifdef FULL_R5
                       XNFontSet,     fontset,
		       XNArea,	      &area,	
		       XNColormap,    cmap,
		       XNForeground,  fg,
		       XNBackground,  bg,
		       XNBackgroundPixmap,  bg_pixmap,
#endif
                       NULL);
    } else if (style & XIMStatusNothing) {
        status_list = XVaCreateNestedList(NULL,
                       NULL);
    } else if (style & XIMStatusNone) {
	/* No status style requested */
	/* How should we handle this?? */
    }

    /*  Currently support only preedit and status callback style. */
    xic = (XIC)XCreateIC(im,
                        XNClientWindow, xid,
                        XNInputStyle, style,
                        XNPreeditAttributes, preedit_list,
                        XNStatusAttributes, status_list,
                        NULL);
                        
    if (preedit_list);
        XFree(preedit_list);
    if (status_list);
        XFree(status_list);
         
    return(xic);
}
#endif /* OW_I18N */
