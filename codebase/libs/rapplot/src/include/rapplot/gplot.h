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

/******************************************************************
 * gplot.h: header file for graphics utility routines
 ******************************************************************/
  
#ifndef gplot_h
#define gplot_h

#ifdef __cplusplus
extern "C" {
#endif

#include <toolsa/umisc.h>
#include <toolsa/toolsa_macros.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

#include <stdio.h>

  /*
   * max no of chars in default string
   */

#define MAXDEFLEN 80

  /*
   * device numbers for Xwindows and Postscript driver routines
   */

#define XDEV 0
#define PSDEV 1

  /*
   * text justification
   */

#define XJ_LEFT 1
#define XJ_CENTER 2
#define XJ_RIGHT 3
#define YJ_ABOVE 1
#define YJ_CENTER 2
#define YJ_BELOW 3

  /*
   * postscript constants
   */

#define BUTTCAP (long) 0
#define ROUNDCAP (long) 1
#define LINECAP (long) 2
#define MITERJOIN (long) 0
#define ROUNDJOIN (long) 1
#define LINEJOIN (long) 2

  /*
   * postscript constants
   */

#define PS_UNITSCALE (float) 72.0
#define PS_PAGEWIDTH (float) 8.5 /* printer page size */
#define PS_PAGELENGTH (float) 11.0
#define PS_LENGTHMARGIN (float) 0.5 /* page margins */
#define PS_WIDTHMARGIN (float) 0.5

  /*
   * image data start points
   */

#define IMAGE_START_TOPLEFT (long) 1
#define IMAGE_START_BOTLEFT (long) 2

  /*
   * colors
   */

#define DATA_COLOR_MAX 255	/* max value a color can have as data */
#define X_COLOR_MAX 0xffff	/* X 16-bit Hex value for max color */
#define PS_GRAY_MAX 0xf		/* PS 4-bit Hex value for max gray level */
#define PS_GRAY_BITS 4		/* # of bits in ps gray data */
#define PS_BACKGROUND_HEXSTRING "f" /* background gray value for image */

  /*
   * structure for world co-ordinate windows - PSREF;
   */

  typedef struct {
    int xmin, ymin;
    size_t width, height;
    double xscale, yscale;
  } psref_t;

  /*
   * structure for PostScript Graphics Context
   */

  typedef struct {
    FILE *file;
    int current_x, current_y;
    char *fontname;
    double fontsize;
    double line_width;
    double dash_length;
    double space_length;
    double graylevel;
    char hexstring[4];
  } psgc_t;

  /*
   * struct for point in postscript - as in XPoint, GPoint
   */

  typedef struct {
    int x, y;
  } PsPoint;

  /*
   * structure for x frame attributes - XREF
   */

  typedef struct {
    Display *display;
    Drawable drawable;
    Pixmap pixmap;
    size_t width, height;
    double xscale, yscale;
    GC gc;
    XFontStruct *font;
  } xref_t;

  /*
   * structures for color list
   */

  typedef struct x_color_list_s {
    GC gc;
    XColor x_color;
    char *colorname;
    int duplicate;
    struct x_color_list_s *next;
  } x_color_list_t;

  typedef struct {
    x_color_list_t *first_entry;
    long n_entries;
  } x_color_list_index_t;

  /*
   * frame structure for world co-ords in X and PostScript
   */

  typedef struct {
    double w_xmin, w_ymin, w_xmax, w_ymax;
    xref_t *x;
    psref_t *ps;
    psgc_t *psgc;
  } gframe_t;

  /*
   * structure for point in world coords
   */

  typedef struct {
    double x, y;
  } GPoint;

  /*
   * structure for rectangle in world coords
   */

  typedef struct {
    double x, y, width, height;
  } GRectangle;

  /*
   * g_color_scale_level_t -  structure for data level color
   */

  typedef struct{
    double start, end;
    XColor x_color;
    char *colorname;
    GC gc;
    psgc_t *psgc;
  } g_color_scale_level_t;

  /*
   * g_color_scale_t - struct to hold the color scale
   */

  typedef struct{
    long nvalues;
    long nlevels;
    double scale, bias;
    g_color_scale_level_t *level;
    GC *gc;
    psgc_t **psgc;
  } g_color_scale_t;

  /*
   * g_contour_t - struct for contour levels
   */

  typedef struct {
    long n;
    double *value;
  } g_contour_t;

  /*
   * postscript function prototypes
   */

  extern void PsGrestore(FILE *file);

  extern void PsGsave(FILE *file);

  extern void PsPageSetup();

  extern void PsReadColors();

  extern void PsRotate(FILE *file,
		       double angle);

  extern void PsScale(FILE *file,
		      double xscale,
		      double yscale);

  extern void PsSetFont(FILE *file,
			const char *fontname,
			double fontsize);

  extern void PsSetGray(FILE *file,
			double graylevel);

  extern void PsSetLineDash(FILE *file,
			    double dash_length,
			    double space_length);

  extern void PsSetLineStyle(FILE *file,
			     const psgc_t *psgc);

  extern void PsSetLineWidth(FILE *file,
			     double line_width);

  extern void PsShowPage(FILE *file);

  extern void PsTranslate(FILE *file,
			  double wx,
			  double wy);


  /*
   * X function prototypes
   */

  extern GC xGetColorGC(Display *display,
			Colormap cmap,
			x_color_list_index_t *list_index,
			const char *colorname);

  extern XColor *xGetXColor(Display *display,
			    Colormap cmap,
			    x_color_list_index_t *list_index,
			    const char *colorname);

  extern XFontStruct *xLoadFont(Display *display,
				const char *fontname);

  extern char *xGetResString(Display *display,
			     const char *name,
			     const char *res_string,
			     const char *hard_def);

  extern double xGetResDouble(Display *display,
			      const char *name,
			      const char *res_string,
			      double hard_def);

  extern float xGetResFloat(Display *display,
			    const char *name,
			    const char *res_string,
			    float hard_def);

  extern long xGetResLong(Display *display,
			  const char *name,
			  const char *res_string,
			  long hard_def);

  extern void xFreeColorList(Display *display,
			     Colormap cmap,
			     x_color_list_index_t *list_index);

  extern void xFreeFont(Display *display,
			XFontStruct *font);

  extern void xFreeGC(Display *display,
		      GC gc);

  /*
   * general function prototypes
   */

  extern double GXWorldx(const gframe_t *frame, int windowx);

  extern double GXWorldy(const gframe_t *frame, int windowy);

  extern double GPsWorldx(const gframe_t *frame, int windowx);

  extern double GPsWorldy(const gframe_t *frame, int windowy);

  extern gframe_t *GCreateFrame(double w_xmin,
				double w_ymin,
				double w_xmax,
				double w_ymax);

  extern void GLinearTicks(double min,
			   double max,
			   long approx_nticks,
			   long *nticks,
			   double *tickmin,
			   double *delta_tick);

  extern int GLoadGCScale(g_color_scale_t *color_scale,
			  Display *display,
			  Colormap cmap,
			  x_color_list_index_t *color_index,
			  double scale,
			  double bias);

  extern int GLoadPsgcScale(g_color_scale_t *color_scale,
			    Display *display,
			    Colormap cmap,
			    double scale,
			    double bias);

  extern int GReadColorScale(const char *file_name,
			     g_color_scale_t **color_scale_ptr);

  extern int GXWindowx(const gframe_t *frame,
		       double worldx);

  extern int GXWindowy(const gframe_t *frame,
		       double worldy);

  extern int GPsWindowx(const gframe_t *frame, double worldx);
    
  extern int GPsWindowy(const gframe_t *frame, double worldy);

  extern void GAdjustGCScale(g_color_scale_t *color_scale,
			     double scale,
			     double bias);

  extern void GAdjustPsgcScale(g_color_scale_t *color_scale,
			       double scale,
			       double bias);

  extern void GDrawArc(int dev,
		       const gframe_t *frame,
		       GC xgc,
		       const psgc_t *psgc,
		       double wx,
		       double wy,
		       double wradiusx,
		       double wradiusy,
		       double wangle1,
		       double wangle2,
		       double axis_rotation,
		       int nsegments);

  extern void GDrawArrow(int dev,
			 const gframe_t *frame,
			 GC xgc,
			 const psgc_t *psgc,
			 double wx1,
			 double wy1,
			 double wx2,
			 double wy2,
			 double head_angle,
			 double head_length);

  extern void GDrawContours(int dev,
			    const gframe_t *frame,
			    GC pos_gc,
			    GC zero_gc,
			    GC neg_gc,
			    psgc_t *pos_psgc,
			    psgc_t *zero_psgc,
			    psgc_t *neg_psgc,
			    double wx,
			    double wy,
			    double wwidth,
			    double wheight,
			    long nsamples,
			    long nlines,
			    long startpoint,
			    unsigned char *data_array,
			    double scale,
			    double bias,
			    g_contour_t *contours,
			    long missing_val);

  extern void GDrawImage(int dev,
			 const gframe_t *frame,
			 g_color_scale_t *colors,
			 double wx,
			 double wy,
			 double wwidth,
			 double wheight,
			 long nsamples,
			 long nlines,
			 long startpoint,
			 unsigned char *data_array,
			 long missing_val);

  extern void GDrawImageProgDetail(int dev,
				   const gframe_t *frame, 
				   g_color_scale_t *colors, 
				   double wx, 
				   double wy, 
				   double wwidth, 
				   double wheight, 
				   long nsamples, 
				   long nlines, 
				   long startpoint, 
				   unsigned char *data_array, 
				   long missing_val,
				   int min_pix_per_grid);

  extern void GDrawImageString(int dev,
			       const gframe_t *frame,
			       GC xgc,
			       XFontStruct *font,
			       const psgc_t *psgc,
			       int xjust, /* XJ_LEFT, XJ_CENTER or XJ_RIGHT */
			       int yjust, /* YJ_ABOVE, YJ_CENTER or YJ_BELOW */
			       double wx,
			       double wy,
			       const char *text);

  extern void GDrawImageStringOffset(int dev, 
				     const gframe_t *frame, 
				     GC xgc, 
				     XFontStruct *font, 
				     const psgc_t *psgc, 
				     int xjust, 
				     int yjust, 
				     double wx, 
				     double wy, 
				     int x_offset,
				     int y_offset,
				     const char *text);
  
  extern void GDrawLine(int dev,
			const gframe_t *frame,
			GC xgc,
			const psgc_t *psgc,
			double wx1,
			double wy1,
			double wx2,
			double wy2);

  extern void GDrawLines(int dev,
			 const gframe_t *frame,
			 GC xgc,
			 const psgc_t *psgc,
			 GPoint *gpoints,
			 int npoints,
			 int mode);

  extern void GDrawPoints(int dev,
			  const gframe_t *frame,
			  GC xgc,
			  const psgc_t *psgc,
			  double wx,
			  double wy,
			  XPoint *offsets,
			  int num_offsets);
  
  extern void GDrawPsFrameBorder(const gframe_t *frame,
				 double border_line_width);

  extern void GDrawRectangle(int dev,
			     const gframe_t *frame,
			     GC xgc,
			     const psgc_t *psgc,
			     double wx,
			     double wy,
			     double wwidth,
			     double wheight);

  extern void GDrawString(int dev,
			  const gframe_t *frame,
			  GC xgc,
			  XFontStruct *font,
			  const psgc_t *psgc,
			  int xjust,
			  int yjust,
			  double wx,
			  double wy,
			  const char *text);

  extern void GDrawStringOffset(int dev,
				const gframe_t *frame,
				GC xgc,
				XFontStruct *font,
				const psgc_t *psgc,
				int xjust,
				int yjust,
				double wx,
				double wy,
				int x_offset,
				int y_offset,
				const char *text);

  extern void GDrawWindBarb(int dev, 
			    const gframe_t *frame, 
			    GC xgc, 
			    const psgc_t *psgc, 
			    double world_x,
			    double world_y,
			    double u_wind,
			    double v_wind,
			    int barb_shaft_len);

  extern void GFillArc(int dev, 
		       const gframe_t *frame, 
		       GC xgc, 
		       const psgc_t *psgc, 
		       double wx, 
		       double wy, 
		       double wradiusx, 
		       double wradiusy, 
		       double wangle1, 
		       double wangle2, 
		       double axis_rotation, 
		       int nsegments);

  extern void GFillPolygon(int dev, 
			   const gframe_t *frame, 
			   GC xgc, 
			   const psgc_t *psgc, 
			   GPoint *gpoints, 
			   int npoints, 
			   int mode);

  extern void GFillRectangle(int dev,
			     const gframe_t *frame,
			     GC xgc,
			     const psgc_t *psgc,
			     double wx,
			     double wy,
			     double wwidth,
			     double wheight);

  extern void GFreeFrame(gframe_t *frame);

  extern void GMoveFrameWorld(gframe_t *frame,
			      double w_xmin,
			      double w_ymin,
			      double w_xmax,
			      double w_ymax);

  extern void GPageSetup(int dev,
			 const gframe_t *vert_page_frame,
			 const gframe_t *horiz_page_frame,
			 double print_width,
			 double ps_total_width,
			 double ps_total_height,
			 double page_width,
			 double page_length,
			 double width_margin,
			 double length_margin,
			 FILE *ps_file);

  extern void GPsInitFrame(psgc_t *psgc,
			   FILE *file,
			   const char *fontname,
			   double fontsize,
			   long line_width);

  extern void GPsSetGeomFrame(gframe_t *frame,
			      int xmin,
			      int ymin,
			      size_t width,
			      size_t height);

  extern void GScale(int dev,
		     const gframe_t *frame,
		     double wwidth,
		     double wheight);

  extern void GSetClipRectangles(int dev,
				 const gframe_t *frame,
				 GC xgc,
				 const psgc_t *psgc,
				 GRectangle *grectangles,
				 int n);

  extern void GTranslate(int dev,
			 const gframe_t *frame,
			 double wx,
			 double wy);

  extern void GXInitFrame(gframe_t *frame,
			  Display *display,
			  Drawable drawable,
			  XFontStruct *font);

  extern void GXSetGeomFrame(gframe_t *frame,
			     size_t width,
			     size_t height);


  extern void GXCreateFrame(gframe_t *frame,
			    Display *display,
			    Window parent,
			    int x, int y,
			    int width, int height,
			    int border_width,
			    int foreground, int background,
			    int border_color,
			    XFontStruct *font);

  extern void GXResetFrame(gframe_t *frame, 
			   int x, int y,
			   size_t width, 
			   size_t height);

  extern void GSetPsGC(psgc_t *psgc, 
		       FILE *file, 
		       const char *fontname, 
		       double fontsize, 
		       long line_width);

  extern void GSetPsRef(gframe_t *frame, 
			int xmin, 
			int ymin, 
			size_t width, 
			size_t height);

  extern void GSetXRef(gframe_t *frame, 
		       Display *display, 
		       Drawable drawable, 
		       size_t width, 
		       size_t height, 
		       XFontStruct *font);

#ifdef __cplusplus
}
#endif

#endif
