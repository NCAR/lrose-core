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
//////////////////////////////////////////////////////////
// Legend.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000,
// Boulder, CO, 80307-3000, USA
//
// December 1997
//
//////////////////////////////////////////////////////////

#include <MotifApp/Application.h>
#include <Xm/DrawingA.h>
#include "Legend.h"

Legend::Legend ( const char *name, Widget parent ) :
  BufferedDraw ( name )

{
  
  XGCValues  gcv;
  XGCValues  draw_gcv;
  
  // Initialize all data members 
  
  _front   = 0;
  _back    = 0;
  
  // Create the visible drawing canvas, and set 
  // up the destruction handler
  
  _w =  XtVaCreateWidget (_name,
			  xmDrawingAreaWidgetClass,
			  parent,
			  XmNheight, 50,
			  NULL);

  installDestroyHandler();
    
  // A graphics context is needed for copying the pixmap buffers and
  // erasing the back pixmap. Use the background color of 
  // the base widget for the fill color.
    
  XtVaGetValues ( _w, XmNbackground, &gcv.foreground, NULL );
    
  _gc = XtGetGC ( _w, GCForeground, &gcv ); 

  _draw_gc = XtGetGC ( _w, GCForeground, &draw_gcv ); 

  XSetForeground(XtDisplay(_w), _draw_gc,
		 BlackPixel(XtDisplay(_w),0));
  XtVaSetValues ( _w, XmNuserData, _draw_gc, NULL);

  // Call resize to create the first pixmaps
    
  resize();

}

Legend::~Legend()

{
  // Empty
}

void  Legend::refresh()
{
}

