/*
 *#ident "@(#)olgx_impl.h	1.20 93/06/28 SMI"
 */

/* 
 * Copyright 1990 Sun Microsystems
 */

/*
 * OPEN LOOK object drawing package
 */

#ifndef OL_PRIVATE_DEFINED
#define OL_PRIVATE_DEFINED

#ifdef OW_I18N
/*
 * I18N_Portability: May need to change the following #include to
 * pickup the wchar_t and X11R5(-ish) Xlib functions definitions.
 */
#include <widec.h>
#include <X11/Xlib.h>
#if XlibSpecificationRelease != 5
#include <X11/XlibR5.h>
#endif /* XlibSpecificationRelease != 5 */


#endif
#include <olgx/olgx.h>

#define STRING_SIZE 		128     /* max size of a glyph font string */

#define VARHEIGHT_BUTTON_CORNER_DIMEN  7


#define False                   0
#define True                    1


/*
 * OPEN LOOK constant definitions
 */


/*
 * Macro definitions
 */
#define VARIABLE_LENGTH_MACRO(start_pos, offset)		\
	for (i = 0; i < num_add; i++) {				\
		string[start_pos+i] = offset + add_ins[i];	\
	}

#ifdef OW_I18N
#define	textfontset	utextfont.fontset
#define	textfont	utextfont.fontstruct
#endif

typedef struct _per_disp_res_rec {
  Display * dpy;
  int screen;
  GC_rec * gc_list_ptr;
  Pixmap   busy_stipple;
  Pixmap   grey_stipple;
  struct _per_disp_res_rec * next;
} per_disp_res_rec, *per_disp_res_ptr;

/*
 * Definitions used by the color calculation code 
 */
#define	XRGB	0xffff
#define	MAXRGB	0xff
#define	MAXH	360
#define	MAXSV	1000

#define VMUL		12	/* brighten by 20% (12 = 1.2*10) */
#define SDIV		2	/* unsaturate by 50% (divide by 2) */
#define VMIN		400	/* minimum highlight brightness of 40% */

typedef struct {
    int         r,
                g,
                b;
}           RGB;

typedef struct {
    int         h,
                s,
                v;
}           HSV;

/*
 * Private function declarations
 */

int	           calc_add_ins();
char             * olgx_malloc();
void               olgx_update_horizontal_slider();
void               olgx_update_vertical_slider();
void               olgx_update_vertical_gauge();
void               olgx_update_horiz_gauge();
void               olgx_free();
void               olgx_destroy_gcrec();
void               olgx_total_gcs();
void               olgx_initialise_gcrec();
void               olgx_draw_elevator();
void               olgx_error();
void               olgx_draw_pixmap_label();
void               olgx_draw_varheight_button();
Pixmap             olgx_get_busy_stipple();
Pixmap             olgx_get_grey_stipple();
int                gc_matches();
int                olgx_cmp_fonts();
GC_rec           * olgx_get_gcrec();
GC_rec           * olgx_gcrec_available();
GC_rec           * olgx_set_color_smart();
Graphics_info    * olgx_create_ginfo();
per_disp_res_ptr   olgx_get_perdisplay_list();

/* ol_color.c */
void               hsv_to_rgb();
void               rgb_to_hsv();
void               rgb_to_xcolor();
void               hsv_to_xcolor();
void               xcolor_to_hsv();
void               olgx_hsv_to_3D();

#endif	/* !OL_PRIVATE_DEFINED */





