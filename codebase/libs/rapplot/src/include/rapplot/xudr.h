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
#ifndef XUDR_WAS_INCLUDED
#define XUDR_WAS_INCLUDED

#ifdef __cplusplus
 extern "C" {
#endif

/* XUDR.C: Useful routines for drawing under X  */


extern void XUDRx_axis(int x1, int why1, int x2, int y2, int ntics,
	 Display *dpy, Drawable xid, GC gc);
/*  Draw an x axis starting from x1,why1 to x2,y2 (pixel coordinates).
 *  place "ntics" tic marks along the axis
 */


extern void XUDRy_axis(int x1, int why1, int x2, int y2, int ntics,
	 Display *dpy, Drawable xid, GC gc);
/*  Draw a y axis starting from x1,why1 to x2,y2 (pixel coordinates).
 *  place "ntics" tic marks along the axis
 */

 
extern void XUDRcross(int x, int y, int size1, int size2, 
	Display *dpy,Drawable xid, GC gc);
/*  Draw a cross centered at x,y  (pixel coordinates).
 *  size1 = max pixels from center
 *  size2 = beginning at pixels from center
 */


extern void XUDRcircle(int x, int y, int radius,
	Display *dpy, Drawable xid, GC gc);
/* draw a circle centered at x,y of given radius in pixels */


extern void XUDRline_clip( Display *dpy, Drawable xid, GC gc,
		int x1, int why1, int x2, int y2,
		int xmin, int ymin, int xmax, int ymax);
/* draw a line from x1,why1 to x2,y2 clipped within the rectangle defined
   by xmin, ymin, xmax, ymax.  All coordinates in pixels.
 */


extern void XUDRcircle_segment(Display *dpy, Drawable xid, GC gc,
	int x, int y, int radius,
	int xmin,int ymin,int xmax,int ymax);
/*  Draw a circle centered at x,y of given radius using line segments.
    Clip it within the rectangle defined by xmin, ymin, xmax, ymax.  
    All coordinates in pixels.
 */

extern void XUDRarrow(Display *dpy, Drawable xid, GC gc,
	int x1, int why1, int x2, int y2,
	int head_size,double head_angle);
/*  Draw an arrow from x1,why1 to x2,y2 with the arrow head at x2,y2.
    head_size is the size of the arrow head in pixels; head_angle is the 
    angle in radians between shaft and head elements.
 */
  
extern void XUDRtickarrow(Display *dpy, Drawable xid, GC gc,
	int x1, int why1, int x2, int y2,
	int head_size,double head_angle,
	int num_ticks, int tick_size);
/*  Draw an arrow from x1,why1 to x2,y2 with the arrow head at x2,y2.
    head_size is the size of the arrow head in pixels; head_angle is the 
    angle in radians between shaft and head elements.
    Place num_ticks equally spaced cross hatches of length tick_size
    along its shaft.
 */

/*************************************************************************
 * XUDRMET_WIND_BARB: This routine draws a Northern or Southern
 *  Hemisphere wind barb  Given  U and V Componets. It is assumed that U
 * is positive to the east and V is positive to the North. label features
 * is a it wise flag to control the * style of the text label. Latitude
 * determines which side the flags get drawn on.
 */

void XUDRmet_wind_barb(Display *dpy,Drawable xid,GC gc,int x,int y,
           double u,double v,int  shaftlen, int label_features,
           double lat);
/*    int x, y;            canvas coordinates of barb origin in pixels */
/*    double u,v;         Wind Speed in native units */
/*    int    shaftlen;       length in pixels to draw barb shaft */
/*    int label_features; Bitwise flag to add text labels  */
/*    double lat;* Controls which side the barbs are plotted on */

/* Label Feature Bitwise flag */
/* Plot the direction/10 - Near the vertex */
#define ROUND10_LABEL 1

/* Plot the ten's digit off the end of the shaft */
#define TENS_ONLY_LABEL 2


extern void XUDRwind_barb(Display *dpy, Drawable xid, GC gc,
	int x, int y, double u, double v,int shaftlen);
/*  Draw a Northern Hemisphere wind barb.
 *  It is assumed that u is positive to the east and v is positive to the North.
 *	int x, y;	 coordinates of barb origin in pixels
 *	double u,v;      Wind Speed in arbitrary units
 *	int shaftlen;    length in pixels to draw barb shaft
 */


extern void XUDRwind_barb_labeled(Display *dpy, Drawable xid, GC gc,
	int x, int y, double u, double v,int shaftlen);
/*  Draw a Northern Hemisphere wind barb.
 *  It is assumed that u is positive to the east and v is positive to the North.
 *	int x, y;	 coordinates of barb origin in pixels
 *	double u,v;      Wind Speed in arbitrary units
 *	int shaftlen;    length in pixels to draw barb shaft
 */

extern void XUDRdraw_clock(Display *dpy,Drawable xid,GC gc,int x,int y,int r,
     time_t ti,int local_flag);
 /*    int    x,y,r;      center coords, radius in pixel coordinates */
 /*    time_t ti;         Unix Time to display */
 /*    int local_flag;    Set to true to display local time -otherwize GMT */ 


#ifdef __cplusplus
}
#endif
#endif
