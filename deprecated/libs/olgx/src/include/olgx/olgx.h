/* 
 * @(#) olgx.h 1.48 93/06/28 
 */

/*
 * OPEN LOOK object drawing package
 * Sun Microsystems, Inc.,
 */

#ifndef OL_PUBLIC_DEFINED
#define OL_PUBLIC_DEFINED

#include <X11/Xlib.h>
#ifdef OW_I18N
#if ! defined(XlibSpecificationRelease) || XlibSpecificationRelease < 5
#include <X11/XlibR5.h>          
#endif /* XlibSpecificationRelease != 5 */
#endif /* OW_I18N */

/*
 * Package constant definitions
 */


/*
 * Monitor dependent  definitons  
 */

#define OLGX_2D                0
#define OLGX_3D_COLOR          1
#define OLGX_3D_MONO           2




/* 	GC number definitions 	*/

#define NONE		       -1	/* means "don't draw in this color" */
#define OLGX_WHITE		0
#define OLGX_BLACK		1
#define OLGX_BG1		2
#define OLGX_BG2		3
#define OLGX_BG3		4
#define OLGX_NUM_COLORS		5	/* 1 more than last *color* GC */

#define OLGX_TEXTGC		5
#define OLGX_GREY_OUT    	6       /* Inactive GC  */
#define OLGX_BUSYGC		7
#define OLGX_SCROLL_GREY_GC     8       /* Special GC for 50% black cable */
#define OLGX_NUM_GCS		9	/* always 1 more than last GC */

/* 2D gc definitions */

#define OLGX_TEXTGC_REV         2      /* Is used only for 2D hence
					  substituted for OLGX_BG1 
					*/
/*
 * State Flag Definitions
 */
#define OLGX_NORMAL		0x0000	/* normal button */
#define OLGX_INVOKED		0x0001	/* invoked button */
#define OLGX_MENU_ITEM		0x0002	/* menu item */
#define	OLGX_ERASE		0x0004	/* erase first (only for menu items) */
#define OLGX_BUSY		0x0008	/* busy item */
#define OLGX_DEFAULT		0x0010	/* default item */
#define OLGX_INACTIVE		0x0020	/* inactive item */
#define OLGX_VERT_MENU_MARK	0x0040	/* include a vertical menu mark */
#define OLGX_HORIZ_MENU_MARK	0x0080	/* include a horizontal menu mark */
#define OLGX_VERT_BACK_MENU_MARK 0x2000	/* include a vertical menu mark */
#define OLGX_HORIZ_BACK_MENU_MARK 0x4000/* include a horizontal menu mark */
#define OLGX_DIAMOND_MARK       0x8000  /* include a diamond mark */
#define OLGX_MENU_MARK		0x00c0	/* VERT_MENU_MARK | HORIZ_MENU_MARK */
#define OLGX_LABEL_IS_PIXMAP	0x0200	/* button label is a pixmap */
#define OLGX_LABEL_IS_XIMAGE	0x2000	/* button label is an ximage */
#define OLGX_LABEL_HAS_UNDERLINE 0x0800	/* button label has underline */
#define OLGX_LABEL_HAS_HIGHLIGHT 0x1000	/* button label has highlight */
#ifdef OW_I18N
#define OLGX_LABEL_IS_WCS      0x10000 /* button label is a wide char str */
#endif
#define OLGX_VERTICAL 		0x0800	/* orientation is vertical */
#define OLGX_HORIZONTAL 	0x1000	/* orientation is horizontal */
#define OLGX_PUSHPIN_OUT	0x2000	/* pushpin is in */
#define OLGX_PUSHPIN_IN		0x4000	/* pushpin is out */
#define OLGX_UPDATE		0x8000	/* object is to be updated */
#define OLGX_ABBREV		0x0100	/* object is abbreviated */
#define OLGX_CHECKED		0x0002	/* object is checked (check boxes) */
#define OLGX_MORE_ARROW         0x0400  /* more arrow added for truncated text */

/* scrollbar -2d stateflag defns */

#define OLGX_SCROLL_FORWARD		0x0002	/*scroll forward */
#define OLGX_SCROLL_BACKWARD		0x2000	/*scroll backward */
#define OLGX_SCROLL_ABSOLUTE		0x4000	/*scroll absolute-
						  center darkened  */
#define OLGX_SCROLL_NO_FORWARD          0x0040  /* no scroll forward */
#define OLGX_SCROLL_NO_BACKWARD         0x0080  /* no scroll backward */

/*
 * corner definitions
 */

#define OLGX_UPPER_LEFT		0
#define OLGX_UPPER_RIGHT	1
#define OLGX_LOWER_RIGHT	2
#define OLGX_LOWER_LEFT		3

/*
 * Special Definition !!
 *
 */

#define OLGX_SPECIAL            1

/*
 * character definitions
 */
#define OLG_VSB_ELEVATOR                        1
#define OLG_VSB_ELEVATOR_LINE_BACKWARD          2
#define OLG_VSB_ELEVATOR_ABSOLUTE               3
#define OLG_VSB_ELEVATOR_LINE_FORWARD           4
#define OLG_VSB_REDUCED_ELEVATOR                5
#define OLG_VSB_REDUCED_ELEVATOR_LINE_BACKWARD  6
#define OLG_VSB_REDUCED_ELEVATOR_LINE_FORWARD   7
#define OLG_VSB_ANCHOR                          8
#define OLG_VSB_ANCHOR_INVERTED                 9
#define OLG_HSB_ELEVATOR                        10
#define OLG_HSB_ELEVATOR_LINE_BACKWARD          11
#define OLG_HSB_ELEVATOR_ABSOLUTE               12
#define OLG_HSB_ELEVATOR_LINE_FORWARD           13
#define OLG_HSB_REDUCED_ELEVATOR                14
#define OLG_HSB_REDUCED_ELEVATOR_LINE_BACKWARD  15
#define OLG_HSB_REDUCED_ELEVATOR_LINE_FORWARD   16
#define OLG_HSB_ANCHOR                          17
#define OLG_HSB_ANCHOR_INVERTED                 18
#define OLG_MENU_PIN_OUT                        19
#define OLG_MENU_PIN_IN                         20
#define OLG_MENU_DEFAULT_PIN_OUT                21
#define OLG_ABBREV_MENU_BUTTON                  22
#define OLG_ABBREV_MENU_BUTTON_INVERTED         23
/* new extension */
#define BUTTON_UL				24
#define BUTTON_LL				25
#define BUTTON_LEFT_ENDCAP_FILL			26
#define BUTTON_LR				27
#define BUTTON_UR				28
#define BUTTON_RIGHT_ENDCAP_FILL		29
#define BUTTON_TOP_1				30
#define BUTTON_TOP_2				31
#define BUTTON_TOP_4				32
#define BUTTON_TOP_8				33
#define BUTTON_TOP_16				34
#define BUTTON_BOTTOM_1				35
#define BUTTON_BOTTOM_2				36
#define BUTTON_BOTTOM_4				37
#define BUTTON_BOTTOM_8				38
#define BUTTON_BOTTOM_16			39
#define BUTTON_FILL_1				40
#define BUTTON_FILL_2				41
#define BUTTON_FILL_4				42
#define BUTTON_FILL_8				43
#define BUTTON_FILL_16				44
#define VERT_MENU_MARK_UL			45
#define VERT_MENU_MARK_LR			46
#define VERT_MENU_MARK_FILL			47
#define HORIZ_MENU_MARK_UL			48
#define HORIZ_MENU_MARK_LR			49
#define HORIZ_MENU_MARK_FILL			50
#define ABBREV_MENU_UL				51
#define ABBREV_MENU_LR				52
#define ABBREV_MENU_FILL			53
#define VERT_SB_UL				54
#define VERT_SB_LR				55
#define VERT_SB_TOPBOX_FILL			56
#define HORIZ_SB_UL				57
#define HORIZ_SB_LR				58
#define VERT_SB_BOTBOX_FILL			59
#define HORIZ_SLIDER_CONTROL_UL			60
#define HORIZ_SLIDER_CONTROL_LR			61
#define HORIZ_SLIDER_CONTROL_FILL		62
#define HORIZ_SLIDER_UL				63
#define HORIZ_SLIDER_LL				64
#define HORIZ_SLIDER_UR				65
#define HORIZ_SLIDER_LR				66
#define HORIZ_SLIDER_BOTTOM_1			67
#define HORIZ_SLIDER_BOTTOM_2			68
#define HORIZ_SLIDER_BOTTOM_4			69
#define HORIZ_SLIDER_BOTTOM_8			70
#define HORIZ_SLIDER_BOTTOM_16			71
#define HORIZ_SLIDER_FILL_1			72
#define HORIZ_SLIDER_FILL_2			73
#define HORIZ_SLIDER_FILL_4			74
#define HORIZ_SLIDER_FILL_8			75
#define HORIZ_SLIDER_FILL_16			76
#define HORIZ_SLIDER_LEFT_ENDCAP_FILL		77
#define HORIZ_SLIDER_RIGHT_ENDCAP_FILL		78
#define VERT_SLIDER_UL				79
#define VERT_SLIDER_UR				80
#define VERT_SLIDER_TOP_ENDCAP_FILL		81
#define VERT_SLIDER_LL				82
#define VERT_SLIDER_LR				83
#define VERT_SLIDER_BOTTOM_ENDCAP_FILL		84
#define VERT_SLIDER_CONTROL_UL			85
#define VERT_SLIDER_CONTROL_LR			86
#define VERT_SLIDER_CONTROL_FILL		87
#define UL_RESIZE_UL				88
#define UL_RESIZE_LR				89
#define UL_RESIZE_FILL				90
#define UR_RESIZE_UL				91
#define UR_RESIZE_LR				92
#define UR_RESIZE_FILL				93
#define LR_RESIZE_UL				94
#define LR_RESIZE_LR				95
#define LR_RESIZE_FILL				96
#define LL_RESIZE_UL				97
#define LL_RESIZE_LR				98
#define LL_RESIZE_FILL				99
#define PUSHPIN_OUT_TOP				100
#define PUSHPIN_OUT_BOTTOM			101
#define PUSHPIN_OUT_MIDDLE			102
#define PUSHPIN_IN_TOP				103
#define PUSHPIN_IN_BOTTOM			104
#define PUSHPIN_IN_MIDDLE			105
#define DFLT_BUTTON_LEFT_ENDCAP			106
#define DFLT_BUTTON_RIGHT_ENDCAP		107
#define DFLT_BUTTON_MIDDLE_1			108
#define DFLT_BUTTON_MIDDLE_2			109
#define DFLT_BUTTON_MIDDLE_4			110
#define DFLT_BUTTON_MIDDLE_8			111
#define DFLT_BUTTON_MIDDLE_16			112
#define BASE_OFF_SPECIALCHAR			113 /*special char */
#define UNCHECKED_BOX_UL			114
#define UNCHECKED_BOX_LR			115
#define UNCHECKED_BOX_FILL			116
#define CHECK_MARK				117
#define CHECKED_BOX_FILL			118
#define UNCHECKED_BOX_OUTLINE			119
#define HORIZ_GAUGE_UL				120
#define HORIZ_GAUGE_LL				121
#define HORIZ_GAUGE_UR				122
#define HORIZ_GAUGE_LR				123
#define HORIZ_GAUGE_BOTTOM_1			124
#define HORIZ_GAUGE_BOTTOM_2			125
#define HORIZ_GAUGE_BOTTOM_4			126
#define HORIZ_GAUGE_BOTTOM_8			127
#define HORIZ_GAUGE_BOTTOM_16			128
#define VERT_GAUGE_UL				129
#define VERT_GAUGE_UR				130
#define VERT_GAUGE_LL				131
#define VERT_GAUGE_LR				132
#define VERT_ABBREV_SB_UL			133
#define VERT_ABBREV_SB_LR			134
#define HORIZ_SB_RIGHTBOX_FILL			135
#define HORIZ_ABBREV_SB_UL			136
#define HORIZ_ABBREV_SB_LR			137
#define HORIZ_SB_LEFTBOX_FILL			138
#define BUTTON_OUTLINE_LEFT_ENDCAP		139
#define BUTTON_OUTLINE_RIGHT_ENDCAP		140
#define BUTTON_OUTLINE_MIDDLE_1			141
#define BUTTON_OUTLINE_MIDDLE_2			142
#define BUTTON_OUTLINE_MIDDLE_4			143
#define BUTTON_OUTLINE_MIDDLE_8			144
#define BUTTON_OUTLINE_MIDDLE_16		145
#define BUTTON_FILL_2D_LEFTENDCAP		146
#define BUTTON_FILL_2D_RIGHTENDCAP      	147
#define BUTTON_FILL_2D_MIDDLE_1     		148
#define BUTTON_FILL_2D_MIDDLE_2 		149
#define BUTTON_FILL_2D_MIDDLE_4 		150
#define BUTTON_FILL_2D_MIDDLE_8 		151
#define BUTTON_FILL_2D_MIDDLE_16		152
#define MENU_DFLT_OUTLINE_LEFT_ENDCAP           153
#define MENU_DFLT_OUTLINE_RIGHT_ENDCAP          154
#define MENU_DFLT_OUTLINE_MIDDLE_1              155
#define MENU_DFLT_OUTLINE_MIDDLE_2              156
#define MENU_DFLT_OUTLINE_MIDDLE_4              157
#define MENU_DFLT_OUTLINE_MIDDLE_8              158
#define MENU_DFLT_OUTLINE_MIDDLE_16             159
#define PIXLABEL_BUTTON_UL			160 
#define PIXLABEL_BUTTON_LL			161
#define UL_RESIZE_OUTLINE			162
#define UR_RESIZE_OUTLINE			163
#define LR_RESIZE_OUTLINE			164
#define LL_RESIZE_OUTLINE			165
#define VERT_SB_NO_BACK_OUTLINE                 166
#define VERT_SB_NO_FWD_OUTLINE                  167
#define VERT_SB_INACTIVE_OUTLINE                168
#define HORIZ_SB_NO_BACK_OUTLINE                169
#define HORIZ_SB_NO_FWD_OUTLINE                 170
#define HORIZ_SB_INACTIVE_OUTLINE               171
#define HORIZ_SLIDER_CONTROL_OUTLINE		172
#define HORIZ_SLIDER_LEFT_ENDCAP_OUTLINE	173
#define	HORIZ_SLIDER_RIGHT_ENDCAP_OUTLINE	174
#define HORIZ_SLIDER_OUTLINE_1			175
#define HORIZ_SLIDER_OUTLINE_2			176
#define HORIZ_SLIDER_OUTLINE_4			177
#define HORIZ_SLIDER_OUTLINE_8			178
#define HORIZ_SLIDER_OUTLINE_16			179
#define VERT_SLIDER_TOP_ENDCAP_OUTLINE		180
#define VERT_SLIDER_BOTTOM_ENDCAP_OUTLINE	181
#define VERT_SLIDER_CONTROL_OUTLINE		182
#define PUSHPIN_OUT_DEFAULT_TOP 		183
#define PUSHPIN_OUT_DEFAULT_BOTTOM 		184
#define PUSHPIN_OUT_DEFAULT_MIDDLE 		185
#define HORIZ_GAUGE_LEFT_ENDCAP_OUTLINE		186
#define HORIZ_GAUGE_RIGHT_ENDCAP_OUTLINE	187
#define HORIZ_GAUGE_OUTLINE_MIDDLE_1		188
#define HORIZ_GAUGE_OUTLINE_MIDDLE_2		189
#define HORIZ_GAUGE_OUTLINE_MIDDLE_4		190
#define HORIZ_GAUGE_OUTLINE_MIDDLE_8		191
#define HORIZ_GAUGE_OUTLINE_MIDDLE_16		192
#define CHECK_BOX_CLEAR_FILL			193
#define VERT_SB_BOX_UL 				194
#define VERT_SB_BOX_LR 				195
#define DIMPLE_UL				196
#define DIMPLE_LR				197
#define DIMPLE_FILL				198
#define SLIDER_CHANNEL_OFFSET			199 /* special char */
#define HORIZ_SB_BOX_UL				200
#define HORIZ_SB_BOX_LR				201
#define VERT_BACK_MENU_MARK_UL			202
#define VERT_BACK_MENU_MARK_LR			203
#define VERT_BACK_MENU_MARK_FILL		204
#define HORIZ_BACK_MENU_MARK_UL			205
#define HORIZ_BACK_MENU_MARK_LR			206
#define HORIZ_BACK_MENU_MARK_FILL		207
#define	OLGX_ACTIVE_CARET			208
#define OLGX_INACTIVE_CARET			209
#define VERT_GAUGE_TOPENDCAP   			210
#define VERT_GAUGE_BOTENDCAP   			211
#define PIXLABEL_BUTTON_UR   			212
#define PIXLABEL_BUTTON_LR   			213
#define PIXLABEL_BUTTON_2D_LR 			214
#define PIXLABEL_DEF_BUTTON_UL 			215
#define PIXLABEL_DEF_BUTTON_LL 			216
#define PIXLABEL_DEF_BUTTON_UR 			217
#define PIXLABEL_DEF_BUTTON_LR 			218
#define HORIZ_GAUGE_LEFT_ENDFILL                219
#define HORIZ_GAUGE_MIDDLE_FILL_1               220
#define HORIZ_GAUGE_MIDDLE_FILL_2               221
#define HORIZ_GAUGE_MIDDLE_FILL_4               222
#define HORIZ_GAUGE_MIDDLE_FILL_8               223
#define HORIZ_GAUGE_MIDDLE_FILL_16              224
#define HORIZ_GAUGE_RIGHT_ENDFILL               225
#define VERT_GAUGE_TOP_FILL                     226
#define VERT_GAUGE_BOT_FILL                     227
#define TEXTSCROLLBUTTON_LEFT                   228
#define TEXTSCROLLBUTTON_RIGHT                  229
#define TEXTSCROLLBUTTON_LEFT_INV               230
#define TEXTSCROLLBUTTON_RIGHT_INV              231
#define NUMERIC_SCROLL_BUTTON_NORMAL            232
#define NUMERIC_SCROLL_BUTTON_LEFT_INV          233
#define NUMERIC_SCROLL_BUTTON_RIGHT_INV         234


/*
 * Definitions needed for XView 
 */


#define OLGX_VAR_HEIGHT_BTN_MARGIN              10
#define OLGX_CHOICE_MARGIN                      10
#define OLGX_VAR_HEIGHT_BTN_ENDCAP_WIDTH        5


#ifdef OW_I18N

/*
 * Definision for the Graphics Info flag (Olgx_Flags)
 */

#define	OLGX_FONTSET	(1<<0)

#endif /* OW_I18N */

 /*
 * Type and Structure Definitions
 */


typedef struct _GC_rec {
  GC gc;
  short ref_count;
  int   num_cliprects;          /* Information to set clip rectangles
  XID   clipmask;                * or clipmasks on the GCs
  int   clip_flag;               */
  unsigned long valuemask;      /* what fields are being used */
  XGCValues values;             /* Values stored  */
  struct _GC_rec * next;	/* Pointer to the next gc_info in the list */
  int      depth;
} GC_rec;

typedef struct pixlabel {
  Pixmap pixmap;
  int width,height;
} Pixlabel;

typedef struct ximlabel {
  XImage *ximage;
  int width,height;
} Ximlabel;

typedef struct underlinelabel {
  void *label;
  int position;
} Underlinelabel;

#ifdef OW_I18N
typedef union olgx_font_or_fs{
    XFontSet     fontset;		/* ptr to text font set info */
    XFontStruct *fontstruct;		/* ptr to text font info */
} Olgx_font_or_fs;
#endif

/*
 * Graphics information structure
 */
typedef struct graphics_info {
  Display *dpy;
  int scrn;
  unsigned int depth;	        	/* depth in which we are drawing */
  XFontStruct *glyphfont;		/* ptr to glyph font info*/
#ifdef OW_I18N
  Olgx_font_or_fs utextfont;
#else
  XFontStruct *textfont;		/* ptr to text font info */
#endif
  short  	three_d;		/* Either one of OLGX_2D,
					 * OLGX_3D_COLOR,OLGX_3D_MONO
					 */
						
  GC_rec 	* gc_rec[OLGX_NUM_GCS];

      /* OLGX_3D_MONO has been nuked....
       * info->stipple_pixmaps[3] has been changed to info->drawable[3]
       * and info->drawable[0] is used for storing the drawable associated
       * with the ginfo. In other words, since stipple_pixmaps is no
       * longer going to be used, we are using up this space to
       * avoid binary compatiblity problem between 2.0 and 3.0 
       * info->drawable[1-2] are still blank.
       */
#ifdef OW_I18N
      /*
       * with OW_I18N, info->drawable[1] is redefined to be Olgx_Flags
       * (macro).  This Olgx_Flags being used to distinguish between
       * i18n/fontset and main/fontstruct instance.
       */
#define	_Olgx_Flags	drawable[1]
#endif /* OW_I18N */
  Pixmap        drawable[3];
  unsigned long pixvals[OLGX_NUM_COLORS];
  
  /*
   * important OPEN LOOK values associated with this glyph font
   */

  short	button_height;			/* height of buttons */
  short	endcap_width;			/* size of button endcap */
  short	sb_width, sb_height;		/* scrollbar elevator size */
  short	abbrev_width;         	        /* abbrev menu button size */
  short	slider_width, slider_height;	/* slider control size */
  short	se_width, se_height;		/* slider endcap size */
  short	mm_width, mm_height;		/* menu mark size */
  short base_off           ;            /* Text base_off */
  short slider_offset           ;       /*SliderChanneloffset */
  short	cb_width, cb_height;		/* check box size */
  short	pp_width, pp_height;		/* push pin  size*/
  short gauge_width,gauge_height ;      /* gauge width& height*/
  short textscbu_width ;                /* text scroll button width */
  short gauge_endcapOffset ;            /* Offset between the endcap and gauge
					 * channel
					 */
  short numscbu_width;                  /* Number Scrolling button Width */
  short resize_arm_width;               /* Resize Corner Width  */
  short abbsb_height ;                  /* Abbreviated Scrollbar Height */
  short cable_offset    ;               /* Cable offset distance from
					 * the Scrollbar origin */
  short cable_width  ;                  /* Width of the Cable */
  short point_size   ;		        /* Pixel Point size of the Font */
  short dtarget_height;                 /* Drop Target Height */
  short dtarget_width;                  /* Drop Target Width */
  short dtarget_swidth;                 /* Drop Target Stroke Width */
  short dtarget_ewidth;                 /* Drop Target Edge Width */


} Graphics_info;


/* Public macro definitions to get info from the Ginfo struct  */


#define ScrollbarElevator_Height(info)   	\
  (info->three_d)?((info)->sb_height-1):((info)->sb_height)		

#define ScrollbarElevator_Width(info)   	\
  (info->three_d)?((info)->sb_width-1):((info) ->sb_width)		


#define HorizSliderControl_Width(info)  ((info)->slider_width)
#define HorizSliderControl_Height(info) ((info)->slider_height)
#define SliderEndCap_Width(info)   	((info)->se_width)
#define SliderEndCap_Height(info)   	((info)->se_height)
#define CheckBox_Height(info)   	((info)->cb_height)
#define CheckBox_Width(info)    	((info)->cb_width)
#define PushPinIn_Width(info)           ((info)->pp_height)
#define PushPinOut_Width(info)    	((info)->pp_width)
#define PushPinOut_Height(info)    	((info)->pp_height)
#define ButtonEndcap_Width(info) 	((info)->endcap_width)
#define Button_Height(info)   	\
  ((info->three_d)?((info)->button_height-1):((info)->button_height))
#define MenuMark_Width(info) 		((info)->mm_width)
#define MenuMark_Height(info) 		((info)->mm_height)
#define Abbrev_MenuButton_Height(info) 	((info)->abbrev_width -1)
#define Abbrev_MenuButton_Width(info) 	((info)->abbrev_width)
#define Gauge_EndCapWidth(info)	 	((info)->gauge_width)
#define Gauge_EndCapHeight(info)       	((info)->gauge_height)
#define Gauge_EndCapOffset(info)   	((info)->gauge_endcapOffset)
#define TextScrollButton_Width(info)    ((info)->textscbu_width)
#define TextScrollButton_Height(info)   ((info)->textscbu_width)
#define NumScrollButton_Width(info)     ((info)->numscbu_width)
#define NumScrollButton_Height(info)    ((info)->textscbu_width)
#define ResizeArm_Width(info)           ((info)->resize_arm_width)
#define ResizeArm_Height(info)          ((info)->resize_arm_width)
#ifdef OW_I18N
#define Ascent_of_TextFont(info)        ((info)->utextfont.fontstruct->ascent)
#define Descent_of_TextFont(info)       ((info)->utextfont.fontstruct->descent)
#else
#define Ascent_of_TextFont(info)        ((info)->textfont->ascent)
#define Descent_of_TextFont(info)       ((info)->textfont->descent)
#endif
#define Ascent_of_GlyphFont(info)       ((info)->glyphfont->ascent)
#define Descent_of_GlyphFont(info)      ((info)->glyphfont->descent)
#define Pointsize_Glyph(info)           ((info)->point_size)
#define Vertsb_Endbox_Height(info)      (SliderEndCap_Height(info) + 1)
#define Vertsb_Endbox_Width(info)       (ScrollbarElevator_Width(info))
#define Dimension(info)                 ((info)->three_d)
#ifdef OW_I18N
#define TextFont_Struct(info)           ((info)->utextfont.fontstruct)
#define TextFont_Set(info)              ((info)->utextfont.fontset)
#define	Olgx_Flags(info)		((info)->_Olgx_Flags)
#else
#define TextFont_Struct(info)           ((info)->textfont)
#endif
#define GlyphFont_Struct(info)          ((info)->glyphfont)
#define AbbScrollbar_Height(ginfo)      ((info)->abbsb_height)
#define DropTarget_Width(info)          ((info)->dtarget_width)
#define DropTarget_Height(info)         ((info)->dtarget_height)

/*
 * Public macros to calculate the positions for menu accelerators`
 */

#define ButtonSpace_Width(info)         ((info)->mm_width)
#define MainLabel_Pos(info,x)           (x + (info)->endcap_width)
#define ButtonMark_Pos(info,m_pos,max_mlabel_length,max_qlabel_length) \
      (m_pos+ButtonSpace_Width(info)*3+(max_mlabel_length)+(max_qlabel_length))
#define KeyLabel_Pos(info,mark_pos)     (mark_pos + ButtonSpace_Width(info)*2)
#define QualifierLabel_Pos(info,mark_pos,qlabel_length) \
        (mark_pos - ButtonSpace_Width(info) - qlabel_length)
#define Button_Width(info,x,key_pos,max_klabel_length) \
        (key_pos + (info)->endcap_width + max_klabel_length -x)

/*
 * Public function declarations
 */


Graphics_info *   olgx_main_initialize();
#ifdef OW_I18N
Graphics_info *   olgx_i18n_initialize();
#endif
Graphics_info *   olgx_initialize();
void              olgx_calculate_3Dcolors();
unsigned long     olgx_get_single_color();
void          olgx_closedown();
void          olgx_destroy();


#define olgx_draw_accel_text   olgx_draw_accel_label

void	
  olgx_set_text_font(),
#ifdef OW_I18N
  olgx_set_text_fontset(),
#endif
  olgx_set_glyph_font(),
  olgx_set_single_color(),
  olgx_stipple_rect(),
  olgx_scroll_stipple_rects(),
  olgx_draw_accel_button(),
  olgx_draw_accel_choice_item(),
  olgx_draw_accel_label(),
  olgx_draw_elevator(),
  olgx_draw_button(),
  olgx_draw_choice_item(),
  olgx_draw_scrollbar(),
  olgx_draw_menu_mark(),
  olgx_draw_abbrev_button(),
  olgx_draw_slider(),
  olgx_draw_horizontal_slider(),
  olgx_draw_vertical_slider(),
  olgx_draw_resize_corner(),
  olgx_draw_textscroll_button(),
  olgx_draw_numscroll_button(),
  olgx_draw_gauge(),
  olgx_draw_horiz_gauge(),
  olgx_draw_vertical_gauge(),
  olgx_draw_pushpin(),
  olgx_draw_box(),
  olgx_draw_text(),
  olgx_draw_text_ledge(),
  olgx_draw_check_box();


#endif	/* !OL_PUBLIC_DEFINED */
