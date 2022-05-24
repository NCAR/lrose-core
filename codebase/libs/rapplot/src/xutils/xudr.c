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
/****************************************************************************
 * XUDR.C: Useful routines for drawing under X
 *
 * Keyword: X circles, marker, axis routines
 *
 * For the AWPG Displays 
 * Frank Hage   1991,1992 NCAR, Research Applications Program
 */

#include <X11/Xlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <rapplot/xudr.h>

#define irint(x) ((int)rint((x)))

#define DEG_PER_RAD 57.29577951 /* degrees per radian */

#ifndef M_PI_2
#define M_PI_2          1.57079632679489661923
#endif

/****************************************************************************
 * X_AXIS : Draw a plain axes with tic marks at each column
 */

void XUDRx_axis(int x1,int y1,int x2,int y2,int ncols,Display *dpy,Drawable xid, GC gc)
{
    int i;
    int x3,y3;
    double xtemp;
    double ytemp;

    xtemp = fabs((x2 -x1) / (double) ncols);
    ytemp = fabs((y2 -y1) / (double) ncols);

    XDrawLine(dpy,xid,gc,x1,y1,x2,y2);

    /* add tic marks to axis */
    for( i = 0; i <= ncols ; i ++ ) {
        x3 = (xtemp * (double)i +0.5) + x1;
        y3 = (ytemp * (double)i +0.5) + y1;
        XDrawLine(dpy,xid,gc,x3,y3-3,x3,y3+3);
    }
}

/****************************************************************************
 * Y_AXIS : Draw a plain axes with tic marks at each row
 */

void XUDRy_axis(int x1,int y1,int x2,int y2,int nrows,Display *dpy,Drawable xid, GC gc)
{
    int x3,y3;
    int i;
    double xtemp;
    double ytemp;

    xtemp = ((x2 - x1) / (double) nrows);
    ytemp = ((y2 - y1) / (double) nrows);

    XDrawLine(dpy,xid,gc,x1,y1,x2,y2);

    /* add tic marks to axis */
    for( i = 0; i <= nrows ; i ++ ) {
        x3 = (xtemp * (double)i +0.5) + x1;
        y3 = (ytemp * (double)i +0.5) + y1;
        XDrawLine(dpy,xid,gc,x3-3,y3,x3+3,y3);
    }
}
 
/*****************************************************************************
 * CROSS :  Place Cross at  coordinates x,y
 *  size1 = max pixels from  center
 *  size2 = beginning at pixels from  center
 */
 
void XUDRcross(int x,int y,int size1,int size2, Display *dpy,Drawable xid, GC gc)
{
    int x1,y1,x2,y2;    /* temp coordinates store */
 
    /* right section of cross */
    x1 = x + size2;
    y1 = y2 = y;
    x2 = x + size1;
    XDrawLine(dpy,xid,gc,x1,y1,x2,y2);
 
    /* top section of cross */
    y1 = y + size2;
    x1 = x2 = x;
    y2 = y + size1;
    XDrawLine(dpy,xid,gc,x1,y1,x2,y2);
 
    /* left section of cross */
    x1 = x - size2;
    y1 = y2 = y;
    x2 = x - size1;
    XDrawLine(dpy,xid,gc,x1,y1,x2,y2);
 
    /* bottom section of cross */

    y1 = y - size2;
    x1 = x2 = x;
    y2 = y - size1;
    XDrawLine(dpy,xid,gc,x1,y1,x2,y2);
}

/*****************************************************************************
 * CIRCLE :  Place Circle of radius rad,  at coordinates x,y
 */
 
void XUDRcircle(int x,int y,int rad,Display *dpy, Drawable xid, GC gc)
{
    XDrawArc(dpy,xid,gc,x-rad,y-rad,rad*2,rad*2,0,23040);
}

/******************************************************************
 * LINE_CLIP: Clip and draw line
 * Based on code from J.Jing
 */

void XUDRline_clip(Display *dpy,Drawable xid,GC gc,int x1,int y1,int x2,int y2,
        int xmin,int ymin,int xmax,int ymax)
{
int dx,dy;
int tt;

    /* Check to see if line is totally out of bounds */
    if(x1<xmin && x2<xmin) return;
    if(x1>xmax && x2>xmax) return;
    if(y1<ymin && y2<ymin) return;
    if(y1>ymax && y2>ymax) return;
    
    /* Order the coordinates */
    if(x1>x2) {
        tt=x2;
        x2=x1;
        x1=tt;
        tt=y2;
        y2=y1;
        y1=tt;    
    }

    dx=x2-x1;
    dy=y2-y1;
     
    /* Clip  in X direction */
    if(x1<xmin) {
      y1=y1+dy*(xmin-x1)/dx;
      x1=xmin;
    }
    if(x2>xmax) {
      y2=y1+dy*(xmax-x1)/dx;
      x2=xmax;
    }

    /* Order the coordinates */
    if(y1>y2) {
        tt=x2;
        x2=x1;
        x1=tt;
        tt=y2;
        y2=y1;
        y1=tt;    
    }

    /* Clip in Y direction  */
    if(y1<ymin) {
      x1=x1+dx*(ymin-y1)/dy;
      y1=ymin;
      if(x1<xmin) x1=xmin;
      if(x1>xmax) x1=xmax;
    }
    if(y2>ymax) {
      x2=x1+dx*(ymax-y1)/dy;
      y2=ymax;
      if(x2<xmin) x2=xmin;
      if(x2>xmax) x2=xmax;
    }


    XDrawLine(dpy,xid,gc,x1,y1,x2,y2);
} 
 
/****************************************************************
 * CIRCLE_SEGMENT: Draw a circle in straight line segments 
 *
 */

#define N_SEGMENTS 64
void XUDRcircle_segment(Display *dpy,Drawable xid,GC gc,int x,int y,int r,
    int xmin,int ymin,int xmax,int ymax)
/*    int    x,y,r;            center coords, radius in pixel coordinates */
/*    int    xmin,ymin,xmax,ymax;    Clip boundries */
{
    static int init=0;
    int    x1,y1,x2,y2;
    static double si[N_SEGMENTS+1],co[N_SEGMENTS+1];
    double alfa;
    int i;

    if(init==0){    /* set up sin/cos lookup table */
        init=1;
        for(i=0;i<N_SEGMENTS;i++){
            alfa=(double)i * 3.1415926536/(N_SEGMENTS /2.0);
            si[i]=sin(alfa);
            co[i]=cos(alfa);
        }
        si[N_SEGMENTS]=si[0];
        co[N_SEGMENTS]=co[0];
    }

    x1 =x+irint((double)r*si[0]);
    y1 =y+irint((double)r*co[0]);

    for(i=1;i<N_SEGMENTS+1;i++){
        x2=x+irint((double)r*si[i]);
        y2=y+irint((double)r*co[i]);

        XUDRline_clip(dpy,xid,gc,x1,y1,x2,y2,xmin,ymin,xmax,ymax);
        x1 = x2;
        y1 = y2;
    }
}

#define CLOCK_TICKS 360 
struct Xpoint { short    x,y;};
/****************************************************************
 * DRAW_CLOCK: Draw a clock face 
 *
 */

void XUDRdraw_clock(Display *dpy,Drawable xid,GC gc,int x,int y,int r,time_t ti, int local_flag)
/*    int    x,y,r;      center coords, radius in pixel coordinates */
/*    time_t ti;	 Unix Time to display */
/*    int local_flag;	 Set to true to display local time -otherwize GMT */
{
    int i;
    int    x1,y1,x2,y2;
    double alfa;
    struct Xpoint pt[16];
    struct tm *tm;
    static int init=0;
    static double si[CLOCK_TICKS],co[CLOCK_TICKS];

    if(init==0){    /* set up sin/cos lookup table */
        init=1;
        for(i=0;i<CLOCK_TICKS;i++){
            alfa=(double)i * 3.1415926536/(CLOCK_TICKS /2.0);
            si[i] = sin(alfa);
            co[i] = cos(alfa);
	}

    }

    if(ti ==0) return;   /* Don't render anything if time is not set */

    /* Draw the clock face */
    for(i=0;i<CLOCK_TICKS;i+=(CLOCK_TICKS/12)) {
        x1 = x + irint((double)r * si[i] * 0.95);
        y1 = y + irint((double)r * co[i] * 0.95);
	
        x2 =  x + irint((double)r * si[i]);
        y2 =  y + irint((double)r * co[i]);

	XDrawLine(dpy,xid,gc,x1,y1,x2,y2);
    }

    if(local_flag) {
	tm = localtime(&ti);
    } else {
	tm = gmtime(&ti);
    }

    /* position the tip of the hour hand */
    i = (tm->tm_hour * (CLOCK_TICKS/12) + tm->tm_min / 2 ) % CLOCK_TICKS;
    pt[0].x = x+irint((double)r * si[i] * 0.6);
    pt[0].y = y-irint((double)r * co[i] * 0.6);

    /* position the back side of the hand */
    i = (i + CLOCK_TICKS/3) % CLOCK_TICKS;
    pt[1].x = x+irint((double)r * si[i] * 0.05);
    pt[1].y = y-irint((double)r * co[i] * 0.05);

    i = (i + CLOCK_TICKS/3) % CLOCK_TICKS;
    pt[2].x = x+irint((double)r * si[i] * 0.05);
    pt[2].y = y-irint((double)r * co[i] * 0.05);

    XFillPolygon(dpy,xid,gc,(XPoint *)pt,3,Convex,CoordModeOrigin);

    /* position the tip of the minute hand */
    i = (tm->tm_min * CLOCK_TICKS/60) % CLOCK_TICKS;
    pt[0].x = x+irint((double)r * si[i] * 0.9);
    pt[0].y = y-irint((double)r * co[i] * 0.9);

    /* position the back side of the hand */
    i = (i + CLOCK_TICKS/3) % CLOCK_TICKS;
    pt[1].x = x+irint((double)r * si[i] * 0.05);
    pt[1].y = y-irint((double)r * co[i] * 0.05);

    i = (i + CLOCK_TICKS/3) % CLOCK_TICKS;
    pt[2].x = x+irint((double)r * si[i] * 0.05);
    pt[2].y = y-irint((double)r * co[i] * 0.05);

    XFillPolygon(dpy,xid,gc,(XPoint *) pt,3,Convex,CoordModeOrigin);
}

/*************************************************************************
 * ARROW: Draw an arrow between x1,y1 and x2,y2, with the head at
 *  the second point.
 *
 */

void XUDRarrow(Display *dpy,Drawable xid,GC gc,int x1,int y1,int x2,int y2,int head_size,double head_angle)
/*    int x1, y1;         Tail position (pixels) */
/*    int x2, y2;         Head position (pixels) */
/*    int    head_size;    Size of head in pixels */
/*    double    head_angle;    Angle in Radians between shaft and head elements */
{
    int    x3,y3;
    double    slope;
    double    head_slope;

    if(x1 == x2 && y1 == y2) {
         slope = 0.0;
    } else {
         slope = atan2(((double)(y1-y2)),((double)(x1-x2)));
    }

    /* shaft segment */
    XDrawLine(dpy,xid,gc,x1,y1,x2,y2);
     
    /* First segment */
    head_slope = slope + head_angle;
    x3 = x2 + irint(head_size * cos(head_slope));
    y3 = y2 + irint(head_size * sin(head_slope));
    XDrawLine(dpy,xid,gc,x2,y2,x3,y3);

    /* Second segment */
    head_slope = slope - head_angle;
    x3 = x2 + irint(head_size * cos(head_slope));
    y3 = y2 + irint(head_size * sin(head_slope));
    XDrawLine(dpy,xid,gc,x2,y2,x3,y3);
}

void XUDRtickarrow(Display *dpy,Drawable xid,GC gc,int x1,int y1,int x2,int y2,int head_size,double head_angle,int num_ticks, int
tick_size)
/*    int x1, y1;         Tail position (pixels) */
/*    int x2, y2;         Head position (pixels) */
/*    int    head_size;    Size of head in pixels */
/*    double    head_angle;    Angle in Radians between shaft and head elements */
/*    int       num_ticks; Number of ticks to place along its shaft */
/*    int       tick_size; How far they should stick out of its shaft */
{
    int i;
    int    x3,y3,x4,y4,x5,y5;
    double    slope;
    double    head_slope;
    double    tick_slope;
    double n_div;  /* number of equal divisions */

    if(x1 == x2 && y1 == y2) {
         slope = 0.0;
    } else {
         slope = atan2(((double)(y1-y2)),((double)(x1-x2)));
    }

    /* shaft segment */
    XDrawLine(dpy,xid,gc,x1,y1,x2,y2);
     
    /* First segment */
    head_slope = slope + head_angle;
    x3 = x2 + (head_size * cos(head_slope)) + 0.5;
    y3 = y2 + (head_size * sin(head_slope)) + 0.5;
    XDrawLine(dpy,xid,gc,x2,y2,x3,y3);

    /* Second segment */
    head_slope = slope - head_angle;
    x3 = x2 + (head_size * cos(head_slope)) + 0.5;
    y3 = y2 + (head_size * sin(head_slope)) + 0.5;
    XDrawLine(dpy,xid,gc,x2,y2,x3,y3);

    n_div = num_ticks +1;
    for(i= 1; i <= num_ticks; i++) {
       /* find the point along the shaft */
       x3 = x1 + ((x2 - x1) * (i / n_div)) + 0.5;
       y3 = y1 + ((y2 - y1) * (i / n_div)) + 0.5;

       tick_slope = slope - M_PI_2;
       x4 = x3 + (tick_size * cos(tick_slope)) + 0.5;
       y4 = y3 + (tick_size * sin(tick_slope)) + 0.5;

       tick_slope = slope + M_PI_2;
       x5 = x3 + (tick_size * cos(tick_slope)) + 0.5;
       y5 = y3 + (tick_size * sin(tick_slope)) + 0.5;
       XDrawLine(dpy,xid,gc,x4,y4,x5,y5);
    }
}

/*************************************************************************
 * XUDRMET_WIND_BARB: This routine draws a Northern or Southern
 *  Hemisphere wind barb
 * Given  U and V Componets. It is assumed that U is positive to the east
 * and V is positive to the North. label features is a it wise flag to
 * control the style of the text label. Latitude determines which side
 * the flags get drawn on.
 */

void XUDRmet_wind_barb(Display *dpy,Drawable xid,GC gc,int x,int y,
           double u,double v,int  shaftlen, int label_features,
	   double lat)
/*    int x, y;            canvas coordinates of barb origin in pixels */
/*    double u,v;         Wind Speed in native units */
/*    int    shaftlen;       length in pixels to draw barb shaft */
/*    int label_features; Bitwise flag to add text labels  */
/*    double lat;  >=0 barb on left. - < 0 Barb on right */
{
    int x1,y1,x2,y2,x3,y3;
    int    flaglen;
    int    flag_space;
    int    radius;
    double    speed;    /* Speed in native units */
    double    speed_temp; 
    double    sign_x,sign_y;
    double    dx,dy;    /* Essentailly cos(angle) and sin(angle) */
    double    ratio;    /* The slope of the barb */
    XPoint    xpt[4];

    int iangle;
    double  angle;
    char    label[32];

    radius = shaftlen / 15;

    /* Draw the Shaft */
    if(u == 0.0 ) { /* avoid div by 0 */
        x1 = x;
        y1 = v > 0.0 ? y + shaftlen:  y - shaftlen;

	/* compute met label position */
        x3 = x;
        y3 = v > 0.0 ? y + (shaftlen * 1.25) :  y - (shaftlen * 1.25);

        speed = fabs(v);
	if(speed >= 0.0) {
            XDrawLine(dpy, xid, gc, x, y, x1, y1 );
            XUDRcircle(x,y,radius,dpy,xid,gc); 
        } else {
            XUDRcircle(x,y,radius,dpy,xid,gc); 
	    return; /* Nothing else gets plotted */
        }
        dx = 0.0;
        dy =  v < 0.0 ? 1.0: -1.0; 
    } else {
        ratio = fabs(v/u);
        sign_x = u < 0.0 ? 1.0: -1.0;
        if(sign_x > 0.0) {
            sign_y = v < 0.0 ? 1.0: -1.0;
        } else {
            sign_y = v < 0.0 ? -1.0: 1.0;
        }

        dx = ((double) sign_x / sqrt(ratio * ratio + 1));
        dy = dx * ratio * sign_y;
         
        x1 =  x + (dx * shaftlen);
        y1 =  y - (dy * shaftlen);

	/* compute met label position */
        x3 =  x + (dx * shaftlen * 1.25);
        y3 =  y - (dy * shaftlen * 1.25);
        speed = sqrt((u * u) + (v * v));
        if(speed >= 0.0) {    /* Draw a shaft */
            XDrawLine(dpy, xid, gc, x, y, x1, y1 );
            XUDRcircle(x,y,radius,dpy,xid,gc); 
        } else {
            XUDRcircle(x,y,radius,dpy,xid,gc); 
	    return; /* Nothing else gets plotted */
        }
    }

    if(speed > 250.0) return;    /* Abort on ridiculously high values */
    flag_space = speed <= 80.0 ? 3: 2;    /* Space higher values more closely */

    speed_temp = speed;

    flaglen = shaftlen / 3;

    /* reverse the barb in the southern hemisphere */
    if(lat < 0.0) flaglen = -flaglen;

    /* Add the speed markings - Rounded to the nearest 5.0 units  */
    while (speed_temp >= 47.5) {    
        xpt[0].x = x + (dx * shaftlen);
        xpt[0].y = y - (dy * shaftlen);
        shaftlen -= flag_space;
        xpt[1].x = (x + (dx * shaftlen)) + (dy * flaglen);
        xpt[1].y = (y - (dy * shaftlen)) + (dx * flaglen);
        shaftlen -= flag_space;
        xpt[2].x = x + (dx * shaftlen); 
        xpt[2].y = y - (dy * shaftlen);
        XFillPolygon(dpy,xid, gc, xpt, 3,Convex,CoordModeOrigin);
        shaftlen -= flag_space;
        speed_temp -= 50.0;
    }

    while (speed_temp >= 7.50 ) {
        x1 = x + (dx * shaftlen);
        y1 = y - (dy * shaftlen);
        x2 = x1 + (dy * flaglen);
        y2 = y1 + (dx * flaglen);
        XDrawLine(dpy, xid, gc, x1, y1, x2, y2);
        shaftlen -= flag_space;
        speed_temp -= 10.0;
    }

    flaglen /= 2;

    if (speed_temp >= 2.5 ) {
        x1 = x + (dx * shaftlen);
        y1 = y - (dy * shaftlen);
        x2 = x1 + (dy * flaglen);
        y2 = y1 + (dx * flaglen);
        XDrawLine(dpy, xid, gc, x1, y1, x2, y2);
    }

    /*  If the speed is lest than 2.5 units/sec, forget the label */
    if(label_features == 0 || speed < 2.5) return;

    /* show angle of the wind */
    angle = atan2( -v, -u) * DEG_PER_RAD;
    angle = 90.0 - angle; /* convert to North = 0 */
     
    if (angle > 360.0) angle -= 360.0;
    if (angle < 0.0) angle += 360.0;

    /* round to nearest 10 degrees */
    if( angle >= 0.0) {
        iangle = (int) (angle+5)/10;
    } else {
        iangle = (int) (angle-5)/10;
    }
     
    if (iangle == 0) iangle = 36;
    if(label_features & ROUND10_LABEL) { 
        sprintf(label,"%02d", iangle);
        XDrawString(dpy,xid, gc, x+2, y+2, label, strlen(label));
    }


    if(label_features & TENS_ONLY_LABEL) { 
        iangle = iangle %10;
        sprintf(label,"%1d", iangle);
        XDrawString(dpy,xid, gc, x3-3, y3+4, label, strlen(label));
    }


}

/*************************************************************************
 * DRAW_WIND_BARB: This routine draws a Northern Hemisphere wind barb
 */
void XUDRwind_barb(Display *dpy,Drawable xid,GC gc,int x,int y,
           double u,double v,int  shaftlen)
{
    XUDRmet_wind_barb(dpy, xid, gc,x,y,u,v,shaftlen,0,1.0);
}

/*************************************************************************
 * DRAW_WIND_BARB_LABELED: This routine draws a N Hemisphere wind barb
 * A label of Heading in degrees / 10 is added 
 */

void XUDRwind_barb_labeled(Display *dpy,Drawable xid,GC gc,int x,int y,
           double u,double v,int  shaftlen)
{
    XUDRmet_wind_barb(dpy, xid, gc,x,y,u,v,shaftlen,TENS_ONLY_LABEL,1.0);
}




