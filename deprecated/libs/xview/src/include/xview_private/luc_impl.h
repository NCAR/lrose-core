/*      "@(#)luc_impl.h 50.15 93/06/28"                 */

/*
 *      (c) Copyright 1990 Sun Microsystems, Inc. Sun design patents 
 *      pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *      file for terms of the license.
 */

#ifndef		luc_impl_DEFINED
#define		luc_impl_DEFINED

/*
 * FIX_ME: This should be get from xv_unique_key().
 */
#define		LUC_KEY			777
#define		LUC_WIN_KEY		778

#define		LUC_MAX_LABEL_LEN	4
#define		LUC_LOOKUPSTRING_BUFLEN 4

#define LUC_UDATA_INFO(udata)   ((LucInfo *)(*(Xv_opaque *)udata))


typedef	enum {
	LUC_ALPHABETIC		= 0,
	LUC_NUMERIC		= 1,
	LUC_ALPHABETIC_LOWER	= 2,
	LUC_ALPHABETIC_UPPER	= 3,
	LUC_KIND_MAX		= 4
} LucKeyKind;


typedef enum {
	LUC_INITIAL,
	LUC_START_CALLED,
	LUC_DRAW_CALLED,
	LUC_DONE_CALLED
}  LucState;


/*
 * In order to sync up with window manager and/or X server, we need
 * keep track of the frame (window) status.  This gives to take
 * correct action at the corrent timming.
 */
typedef enum {
	LFS_NOP,
	LFS_MAP_REQUESTED,
	LFS_BEING_MAPPED,
	LFS_MAPPED,
	LFS_UNMAP_REQUESTED,
	LFS_BEING_UNMAPPED,
	LFS_UNMAPPED,
	LFS_RESIZE_REQUESTED,
	LFS_BEING_RESIZED
} LucFrameState;


/*
 * Frame info for the luc, luc will share the frame per base frame
 * basis.
 */
typedef struct _luc_frame_info {
    /*
     * Per display based informations.
     */
    Display		*dpy;
    int			 sc_width;	/* screen width		*/
    int			 sc_height;	/* screen hight;	*/

    /*
     * Parent of this XView object.
     */
    Xv_object		 p_obj;
    Frame		 p_frame;	/* Parent frame */

    /*
     * Frame (command frame) informations.
     */
    Frame		 frame;
    XID			 f_xwin;
    int			 f_x;		/* frame position 	*/
    int			 f_y;
    int			 f_width;	/* frame size */
    int			 f_height;
    Bool		 f_should_resize;
    LucFrameState	 lfs;
    LucFrameState	 mapunmap;

    /*
     * Font related informations.
     */
    Xv_Font		 xv_font;	/* font in LUC		*/
    XID			 afont;		/* ASCII font */
    int			 achar_width;
    XFontSet		 font_set;
    int			 char_ascent;
    int			 char_descent;
    int			 char_width;
    int			 char_height;
    int			 label_next_width;
    int			 label_np_width;	/* Next/Previous label width */

#ifdef USE_CANVAS
    /*
     * Canvas specific informations.
     */
    Canvas		 canvas;
    Xv_Window		 canvas_pw;
    Window		 canvas_xwin;
    Graphics_info	*ginfo;
    GC			 gc_text;
    GC			 gc_clear;
    int			 gap_x;
    int			 gap_y;
#else
    Panel		 panel;
    Panel_item		 panel_item;
#endif

    struct _luc_frame_info
    			*next;
} LucFrameInfo;


/*
 * Default values which getting from Xrm.
 */
typedef struct _luc_default {
    Bool		 has_been_initialized;

    Bool		 keygrab;
    Bool		 threed;
    char		*font_name;

    int			 window_margin_x;
    int			 window_margin_y;
    Bool		 window_should_fit;	/*
						 * luc window will fit
						 * within the screen.
						 */
    int			 window_off_x;	/* Offset from parent window */
    int			 window_off_y;
    int			 screen_margin_x;
    int			 screen_margin_y;

    LucKeyKind		 key_kind;	/* key label type	*/
    int			 max_can;	/* maximum # of candidates */
    wchar_t	       **labels;	/* item labels */
    DrawUpDirection	 dir;		/* vertical or horizontal */
    int			 rows;		/* number of lines	*/
    int			 columns;	/* number of columns	*/

    wchar_t		*label_next;
    wchar_t		*label_previous;
} LucDefaults;


/*
 * Lokup Choice Sesstion data, this data will be allocate per each sesstion.
 */
typedef	struct _luc_info {
    XIC			 ic;		/* IC 			*/ 

    LucFrameInfo	*frame_info;
    int			 f_x;
    int			 f_y;
    int			 f_width;
    int			 f_height;

    /*
     * Negotiation among callback and XIM.
     */
    WhoOwnsLabel   	 WhoOwnsLabel;   /* Owner of labels      */
    WhoIsMaster		 WhoIsMaster;	

    /*
     * Keeping callback data structure (FIX_ME: this should be copied
     * instead of just pointer).
     */
    XIMLookupStartCallbackStruct
			*start;
    XIMLookupDrawCallbackStruct
			*draw;

    /*
     * Geometorical informations.
     */
    DrawUpDirection	 dir;		/* vertical or horizontal */
    int			 max_can;	/* maximum # of candidates */
    int			 d_rows;	/* number of lines	*/
    int			 d_columns;	/* number of columns	*/
    int			 rows;		/* number of lines	*/
    int			 columns;	/* number of columns	*/

    int			 label_width;
    int			 item_width;	/* width of a cell in PIXEL */
    int			 item_height;	/* height of a cell in PIXEL */

    /*
     * Keep track of what (which part of) is shown on the screen.
     */
    int			 first;		/* first can to be appear*/
    int			 num;		/* number of candidates */
	    				/* to be appear		*/

    /*
     * Input process.
     */
    int			 current;	/* current choice	*/
    wchar_t		 keybuf[LUC_LOOKUPSTRING_BUFLEN + 1];
    int			 keybuf_counter;/* num of char in keybuf */

    LucState		 state;		/* draw() called or not */
} LucInfo;


#endif	/* ~luc_impl_DEFINED */
