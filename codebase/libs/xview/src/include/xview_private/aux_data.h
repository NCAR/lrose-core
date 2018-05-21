/*      "@(#)aux.h 50.9 93/06/28"		 */

/*
 *      (c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *      pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *      file for terms of the license.
 */

#ifndef	aux_DEFINED
#define	aux_DEFINED

#define	AUX_UDATA_INFO(udata)	((AuxInfo *)(*(Xv_opaque *)udata))
#define	AUX_KEY_DATA		101

/*
 * Frame info for the auxiliary region, aux will share the frame per base 
 * frame basis.
 */
typedef struct _aux_frame_info {
    Display                *dpy;
    int                    sc_width;      /* screen width */
    int                    sc_height;     /* screen hight */
 
    Xv_object              p_obj;
    Frame                  p_frame;       /* Parent frame */
 
    Frame                  frame;
    XID                    f_xwin;
    int                    f_x;           /* frame position */
    int                    f_y;
    int                    f_width;       /* frame size */
    int                    f_height;
 
    Xv_Font                xv_font;       /* font in AUX */
    XID                    afont;         /* ASCII font */
    int                    achar_width;
 
    XFontSet               font_set;
    int                    char_ascent;
    int                    char_descent;
    int                    char_width;
    int                    char_height;
 
    Panel                  panel;
    Panel_item             panel_item;
 
    struct _aux_frame_info *next;
} AuxFrameInfo;

typedef struct _aux_default {
    Bool                 has_been_initialized;
 
    Bool                 keygrab;
    Bool                 threed;
    char                *font_name;
 
    int                  window_margin_x;
    int                  window_margin_y;
    Bool                 window_should_fit;     /*
                                                 * aux window will fit
                                                 * within the screen.
                                                 */
    int                  window_off_x;  /* Offset from parent window */
    int                  window_off_y;
    int                  screen_margin_x;
    int                  screen_margin_y;
 
} AuxDefaults;
	
typedef	struct _aux_info {
	Display				*dpy;
	XIC				ic;
	Bool				state;
	Frame				frame;
	Panel				panel;
	Panel_item			item;
	Xv_object			p_obj;
        Frame				p_frame;

	XIMPreeditDrawCallbackStruct	*pe_cache;
	AuxFrameInfo			*frame_info;
	int				f_x;
	int				f_y;
	int				f_width;
	int				f_height;
}  AuxInfo;

#endif /*		~aux_DEFINED */
