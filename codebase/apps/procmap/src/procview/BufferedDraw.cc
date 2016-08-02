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
// BufferedDraw.cc
//
// DrawingArea widget with double buffering
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000,
// Boulder, CO, 80307-3000, USA
//
// December 1997
//
//////////////////////////////////////////////////////////

#include "BufferedDraw.h"

BufferedDraw::BufferedDraw ( const char *name ) :
  UIComponent( name )

{
  // Empty
}

BufferedDraw::~BufferedDraw()
  
{
  
  // Free the pixmaps and GC, if they still exist
  
  if ( _front )       
    XFreePixmap ( XtDisplay ( _w ), _front );
    
  if ( _back )        
    XFreePixmap ( XtDisplay ( _w ), _back );
    
  if ( _w && _gc )
    XtReleaseGC ( _w, _gc );
}

void BufferedDraw::resizeCallback ( Widget, 
				    XtPointer clientData, 
				    XtPointer )
{
  BufferedDraw *obj = ( BufferedDraw * ) clientData;
  obj->resize();
}    

void BufferedDraw::resize()

{
    
  // Get the current size of the drawing area
  
  XtVaGetValues ( _w, XmNwidth,  &_width,
		  XmNheight, &_height,
		  NULL );
  
  if(!_width || !_height)
    return;
  
  // Pixmaps can't be resized, so just destroy the old ones
  
  if ( _front )       
    XFreePixmap ( XtDisplay ( _w ), _front );
  
  if ( _back )        
    XFreePixmap ( XtDisplay ( _w ), _back );
  
  // Create new pixmaps to match the new size of the window
  
  _back = XCreatePixmap ( XtDisplay ( _w ),
			  DefaultRootWindow ( XtDisplay ( _w ) ),
			  _width, _height, 
			  DefaultDepthOfScreen ( XtScreen ( _w ) ) );
    
  _front = XCreatePixmap ( XtDisplay ( _w ),
			   DefaultRootWindow ( XtDisplay ( _w ) ),
			   _width, _height, 
			   DefaultDepthOfScreen ( XtScreen ( _w ) ) );
    
  // Erase both pixmaps
  
  XFillRectangle ( XtDisplay ( _w ), _back, 
		   _gc, 0, 0, _width, _height );
  
  XFillRectangle ( XtDisplay ( _w ), _front, 
		   _gc, 0, 0, _width, _height );

}

void BufferedDraw::swapBuffers()
{
  // Switch the front and back buffers
  
  if ( XtIsRealized ( _w ) )
    {
      Pixmap tmp;
	
      // Do the swap
	
      tmp    = _front;
      _front = _back;
      _back  = tmp;
	
      // Copy the new front buffer to the drawing area
	
      XCopyArea ( XtDisplay ( _w ), _front, XtWindow ( _w ),
		  _gc, 0, 0, _width, _height, 0, 0 );
	
      // Erase the new back buffer to get ready for the next scene
      
      XFillRectangle ( XtDisplay ( _w ), _back, 
		       _gc, 0, 0, _width, _height );
    }
}

void BufferedDraw::redisplayCallback ( Widget, 
				       XtPointer clientData, 
				       XtPointer )
{
  BufferedDraw *obj = ( BufferedDraw * ) clientData;
  obj->redisplay();
}    

void BufferedDraw::redisplay ( )
{
  // Copy the contents of the front pixmap
  // to restore the window
  
  XCopyArea ( XtDisplay ( _w), _front, 
	      XtWindow ( _w ), _gc, 0, 0, 
	      _width, _height, 0, 0 );
}    

