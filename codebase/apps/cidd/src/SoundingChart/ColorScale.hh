// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
/**********************************************************************
 * COLORSCALE.HH:  COlorscale Support Class
 */

#ifndef COLORSCALE_HH
#define COLORSCALE_HH

using namespace std;

#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

#include <toolsa/str.h>
#include <toolsa/http.h>

#include <string>
#include <vector>

#define COLOR_NAME_LENGTH 64
#define COLOR_LABEL_LENGTH 16

typedef struct    {    /* data value to color mappings */
    double  min,max;   /* data Range to map onto color */
    long    pixval;    /* X value to use to draw in this color */
    unsigned short    r,g,b,a;    /* Red Green Blue Alpha values */
    char    cname[COLOR_NAME_LENGTH];   /* color name    */
    char    label[COLOR_LABEL_LENGTH];  /* label to use (use min value if empty) */
}Val_color_t;

///////////////////////////////////////////////////////////////
// Class Definition

class ColorScale
{
public:

  ColorScale(Display *dpy,
             const char *color_file_subdir,
             const char *Color_fname,
             int  debug_level = 0,
             const char *out_of_range_color = "magenta");

   ~ColorScale();

   void setDebug(int level);

   long val2color(double val);

   void draw_colorbar(Display *dpy, Drawable xid, GC gc, XFontStruct *fst,  int x1,  int y1,  int width,  int height);

   // Return the scale minima and maxima
   double get_min_value(void);
   double get_max_value(void);


protected:

private:
   vector <Val_color_t>vc;

   Val_color_t oor_color; // Out of Range Color

   int debug;

   Display *display;
};

#endif
