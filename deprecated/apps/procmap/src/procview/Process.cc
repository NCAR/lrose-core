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
// Process.cc
//////////////////////////////////////////////////////////

#include <MotifApp/Application.h>
#include "Schematic.h"
#include "Process.h"
#include <MotifApp/ColorChooser.h>
#include <MotifApp/InfoDialogManager.h>
#include "libc.h"
#define SIZE 25

Process::Process ( const char *colorName, 
		   Schematic * schematic ) : Symbol ( schematic )
{

  // Get an initial velocity at random
    
  _delta.x = ( int ) ( (SIZE * drand48()) / 3 );
  _delta.y = ( int ) ( (SIZE * drand48()) / 3 );
    
  // Initialize the current location and bounding box of the object
    
  _bounds.width = _bounds.height = SIZE;
  _bounds.x     = _bounds.width  + 1;
  _bounds.y     = _bounds.height + 1;
  _gc = NULL;
    
    // If a color name has been specified, try to allocate a color
    // Otherwise, use the ColorChooser to let the user pick a color
    
  if ( colorName ) {
    
    XGCValues  gcv;
    Display   *dpy  = Application::Inst()->display();
    int        scr  = DefaultScreen ( dpy );
    Colormap   cmap = DefaultColormap ( dpy, scr );
    XColor     color, ignore;
    
    // If color allocation fails, use the default black pixel
    
    if ( XAllocNamedColor ( dpy, cmap, colorName, 
			    &color, &ignore ) )
      gcv.foreground = color.pixel;
    else
      gcv.foreground = BlackPixel ( dpy, scr );
    
    // Create a graphics context used to draw this object
    _gc = XtGetGC ( _schematic->baseWidget(),  GCForeground,  &gcv ); 
  
  } else {

    ColorChooser *colorChooser = 
      new ColorChooser ( "colorChooser",
			 Application::Inst()->baseWidget() );
	
    colorChooser->pickColor ( &Process::colorSelectedCallback, 
			      &Process::canceledCallback, 
			      ( void * ) this );
  }

}

void Process::render ( Drawable  d, 
		       Dimension width, 
		       Dimension height )
{

  if ( !_gc)   // Return if no color has been chosen yet
    return;
    
  // 	Draw the object at the new location

  XDrawRectangles ( Application::Inst()->display(), d, _gc, 
		    &_bounds, 1);
  
}

void Process::colorSelectedCallback (int   red, 
				     int   green, 
				     int   blue, 
				     void *clientData )

{

  Process *obj = ( Process * ) clientData;
  obj->colorSelected ( red, green, blue );
}

void Process::colorSelected ( int red, int green, int blue )

{

  XGCValues  gcv;
  Display   *dpy  = Application::Inst()->display();
  int        scr  = DefaultScreen ( dpy );
  Colormap   cmap = DefaultColormap ( dpy, scr );
  XColor     color;           
  color.red   = red   * 256;
  color.green = green * 256;
  color.blue  = blue  * 256 ;
    
  if ( XAllocColor ( dpy, cmap, &color ) )
    gcv.foreground = color.pixel;
  else
    gcv.foreground = BlackPixel ( dpy, scr );
    
  _gc = XtGetGC ( _schematic->baseWidget(), GCForeground,  &gcv ); 

}

void Process::canceledCallback ( void *clientData )
{

  Process *obj = ( Process * ) clientData;
  InfoDialogManager::Inst()->post ( "Using Black as the default color" );
  obj->colorSelected ( 0, 0, 0 );

}
