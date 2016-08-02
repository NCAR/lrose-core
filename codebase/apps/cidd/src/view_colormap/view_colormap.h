/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/* ** Copyright UCAR (c) 1990 - 2016                                         */
/* ** University Corporation for Atmospheric Research (UCAR)                 */
/* ** National Center for Atmospheric Research (NCAR)                        */
/* ** Boulder, Colorado, USA                                                 */
/* ** BSD licence applies - redistribution and use in source and binary      */
/* ** forms, with or without modification, are permitted provided that       */
/* ** the following conditions are met:                                      */
/* ** 1) If the software is modified to produce derivative works,            */
/* ** such modified software should be clearly marked, so as not             */
/* ** to confuse it with the version available from UCAR.                    */
/* ** 2) Redistributions of source code must retain the above copyright      */
/* ** notice, this list of conditions and the following disclaimer.          */
/* ** 3) Redistributions in binary form must reproduce the above copyright   */
/* ** notice, this list of conditions and the following disclaimer in the    */
/* ** documentation and/or other materials provided with the distribution.   */
/* ** 4) Neither the name of UCAR nor the names of its contributors,         */
/* ** if any, may be used to endorse or promote products derived from        */
/* ** this software without specific prior written permission.               */
/* ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  */
/* ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      */
/* ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    */
/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
#ifdef __cplusplus
 extern "C" {
#endif
/************************************************************************
 * view_colormap.h : header file for view_colormap program
 *
 * RAP, NCAR, Boulder CO
 *
 * march 1991
 *
 * Mike Dixon
 ************************************************************************/

/*
 **************************** includes *********************************
 */

#include <toolsa/umisc.h>
#include <rapplot/xplot.h>
#include <rapplot/gplot.h>

/*
 ******************************** defines ********************************
 */

/*
 * interval (secs) for redraw
 */

#define UPDATE_INTERVAL 5

/*
 * x fonts
 */

#define X_BUTTON_FONT "6x10"
#define X_LABEL_FONT "6x10"

/*
 * global struct
 */

#ifdef MAIN
  
char *Prog_name = NULL;                        /* program name */
char *Env_path_name = NULL;                    /* env file path name */
char *Bif_path_name = NULL;                    /* bif file path name */
char *Rdata_path_name = NULL;                  /* radar data path name */
unsigned char **Rdata = NULL;                  /* radar data array */
int Sem_id = 0;                                /* semaphore id */
unsigned long Ndata = 0;                       /* no of points in Rdata array */
unsigned long Nfields = 0;                     /* no of data fields */
CART_FIELD_PARAMS *Cfparams = NULL;            /* cartesian fiels params */
ZOOM_AREA *Zoom_area = NULL;                   /* zoom area data structure */
VERT_SECTION *Vert_section = NULL;             /* vertical section data struct */

/*
 * command line option args
 */

long View_mode = 0;                            /* mode, cappi or vert */
long Plane = 0;                                /* plane number for cappi */
long Datatype = 0;                             /* input data type */
long Fieldnum = 0;                             /* data field number */
long Plot_rings = 0;                           /* plot rings flag */
long Copy_only = 0;                            /* copy only flag */
float Altitude_requested = 0;                  /* requested altitude */
float Print_width = 0.0;                       /* width of copy */
char *Display_name = NULL;                     /* X display name */
char *Foregroundstr = NULL;
char *Backgroundstr = NULL;

/*
 * X declarations
 */

int Rscreen = 0;                               /* screen id */
Display *Rdisplay = NULL;                      /* display device */
Window Main_window = 0;                        /* main window */
Pixmap Plot_pixmap = 0;                        /* pixmap for plot */
int Pixmap_current = FALSE;                    /* is pixmap current ? */
GC Title_gc = NULL;                            /* GC for title blocks */
GC Button_gc = NULL;                           /* GC for buttons */
GC Scale_gc = NULL;                            /* GC for scale legend */
GC Tick_gc = NULL;                             /* GC for tick marks */
GC Ring_gc = NULL;                             /* GC for rings */
GC Vsection_pos_gc = NULL;                     /* GC for vsection pos line */
GC Header_gc = NULL;                           /* GC for header in plot frame */
GC Ticklabel_gc = NULL;                        /* GC for axis labels */
GC Crosshair_gc = NULL;                        /* GC for crosshair */
GC Copyarea_gc = NULL;                         /* GC for copying pixmap
						  to screen */
GC Pixmap_gc = NULL;                           /* for initializing pixmap */
GC Xor_gc = NULL;                              /* for rubberband lines */

/*
 * main window geometry
 */

int Mainx = 0;
int Mainy = 0;
int Mainx_sign = 0;
int Mainy_sign = 0;
unsigned int Mainwidth = 0;
unsigned int Mainheight = 0;

/*
 * postscript page geometry
 */

float Ps_total_width = 0;
float Ps_total_height = 0;

/*
 * fonts
 */

XFontStruct *X_title_font = NULL;
XFontStruct *X_button_font = NULL;
XFontStruct *X_scale_font = NULL;
XFontStruct *X_ringlabel_font = NULL;
XFontStruct *X_ticklabel_font = NULL;
XFontStruct *X_header_font = NULL;

float Ps_title_fontsize = 0;
float Ps_scale_fontsize = 0;
float Ps_ringlabel_fontsize = 0;
float Ps_ticklabel_fontsize = 0;
float Ps_header_fontsize = 0;

/*
 * colors
 */

unsigned long Foreground = 1;
unsigned long Background = 0;
unsigned long Hlight_background = 0;
unsigned long Border_color = 1;
COLORS **Xcolors = NULL;                        /* x color structure */
COLORS **Pscolors = NULL;                       /* ps grey scale structure */

/*
 * frames for graphics
 */

GFRAME *Title_frame = NULL;
GFRAME **Button_frame = NULL;
GFRAME *Plot_frame = NULL;
GFRAME *Scale_frame = NULL;
GFRAME *Ps_plot_frame = NULL;
GFRAME *Vert_page_frame = NULL;
GFRAME *Horiz_page_frame = NULL;

/*
 * **************** if not main, declare globale as externs **************
 */

#else

extern char *Prog_name;                        /* program name */
extern char *Env_path_name;                    /* env file path name */
extern char *Bif_path_name;                    /* bif file path name */
extern char *Rdata_path_name;                  /* radar data path name */
extern unsigned char **Rdata;                  /* radar data array */
extern int Sem_id;                             /* semaphore id */
extern unsigned long Ndata;                    /* no of points in Rdata array */
extern unsigned long Nfields;                  /* no of data fields */
extern CART_FIELD_PARAMS *Cfparams;            /* cartesian fiels params */
extern ZOOM_AREA *Zoom_area;                   /* zoom area data structure */
extern VERT_SECTION *Vert_section;             /* vertical section data struct */

/*
 * command line option args
 */

extern long View_mode;                          /* mode, cappi or vert */
extern long Plane;                              /* plane number for cappi */
extern long Datatype;                           /* input data type */
extern long Fieldnum;                           /* data field number */
extern long Plot_rings;                         /* plot rings flag */
extern long Copy_only;                          /* copy only flag */
extern float Altitude_requested;                /* requested altitude */
extern float Print_width;                       /* width of copy */
extern char *Display_name;                      /* X display name */
extern char *Foregroundstr;
extern char *Backgroundstr;

/*
 * X declarations
 */

extern int Rscreen;                            /* screen id */
extern Display *Rdisplay;                      /* display device */
extern Window Main_window;                     /* main window */
extern Pixmap Plot_pixmap;                     /* pixmap for plot */
extern int Pixmap_current;                     /* is pixmap current ? */
extern GC Title_gc;                            /* GC for title blocks */
extern GC Button_gc;                           /* GC for buttons */
extern GC Scale_gc;                            /* GC for scale legend */
extern GC Tick_gc;                             /* GC for tick marks */
extern GC Vsection_pos_gc;                     /* GC for vsection pos line */
extern GC Ring_gc;                             /* GC for rings */
extern GC Header_gc;                           /* GC for header in plot frame */
extern GC Ticklabel_gc;                        /* GC for axis labels */
extern GC Crosshair_gc;                        /* GC for crosshair */
extern GC Copyarea_gc;                         /* GC for copying pixmap
						  to screen */
extern GC Pixmap_gc;                           /* for initializing pixmap */
extern GC Xor_gc;                              /* for rubberband lines */

/*
 * main window geometry
 */

extern int Mainx;
extern int Mainy;
extern int Mainx_sign;
extern int Mainy_sign;
extern unsigned int Mainwidth;
extern unsigned int Mainheight;

/*
 * postscript page geometry
 */

extern float Ps_total_width;
extern float Ps_total_height;

/*
 * fonts
 */

extern XFontStruct *X_title_font;
extern XFontStruct *X_button_font;
extern XFontStruct *X_scale_font;
extern XFontStruct *X_ringlabel_font;
extern XFontStruct *X_ticklabel_font;
extern XFontStruct *X_header_font;

extern float Ps_title_fontsize;
extern float Ps_scale_fontsize;
extern float Ps_ringlabel_fontsize;
extern float Ps_ticklabel_fontsize;
extern float Ps_header_fontsize;

/*
 * colors
 */

extern unsigned long Foreground;
extern unsigned long Background;
extern unsigned long Hlight_background;
extern unsigned long Border_color;
extern COLORS **Xcolors;                     /* x color structure */
extern COLORS **Pscolors;                    /* ps grey scale structure */

/*
 * frames for graphics
 */

extern GFRAME *Title_frame;
extern GFRAME **Button_frame;
extern GFRAME *Plot_frame;
extern GFRAME *Scale_frame;
extern GFRAME *Ps_plot_frame;
extern GFRAME *Vert_page_frame;
extern GFRAME *Horiz_page_frame;

#endif

/*
 *********************** function definitions ****************************
 */

extern char *getenv();
extern double strtod();
extern long strtol();

extern int update_display();
extern short swap_bytes();
extern void create_frames();
extern void draw_button();
extern void draw_cappi();
extern void draw_plot();
extern void draw_scale();
extern void draw_title();
extern void draw_vsection();
extern void event_loop();
extern void get_data();
extern void parse_args();
extern void pscolors_read();
extern void redraw_plot();
extern void redraw_windows();
extern void view_colormap_copy();
extern void set_itimer();
extern void setup_page();
extern void setup_windows();
extern void setup_x();
extern void vsection_calc();
extern void xborders_set();
extern void xcolors_read();
extern void xfonts_set();
extern void xgcs_set();
extern void xsens_set();
extern void zoom_calc();
#ifdef __cplusplus
}
#endif
