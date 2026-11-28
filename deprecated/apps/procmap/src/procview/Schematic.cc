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
// Schematic.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000,
// Boulder, CO, 80307-3000, USA
//
// December 1997
//
//////////////////////////////////////////////////////////

#include <MotifApp/Application.h>
#include <Xm/DrawingA.h>
#include <stream.h>
#include "Schematic.h"

Schematic::Schematic ( const char *name, Widget parent ) :
  BufferedDraw ( name )

{
  
  XGCValues  gcv;
  XGCValues  draw_gcv;
    
  // Initialize all data members 
    
  _front   = 0;
  _back    = 0;
    
  // Translations for the DrawingArea widget
  // ManagerGadget* functions are necessary for DrawingArea widgets
  // that steal away button events from the normal translation tables.

  String translations =
    "<Btn1Down>:    trap(down) ManagerGadgetArm()  \n"
    "<Btn1Up>:      trap(up)   ManagerGadgetActivate()  \n"
    "<Btn1Motion>:  trap(motion) ManagerGadgetButtonMotion()";

  XtActionsRec actions;
  actions.string = "trap";
  actions.proc = trap;
  XtAppAddActions (Application::Inst()->appContext(), &actions, 3);

  // Create the visible drawing canvas, and set 
  // up the destruction handler
    
  _w =  XtVaCreateWidget (_name,
			  xmDrawingAreaWidgetClass,
			  parent,
			  XmNtranslations,
			  XtParseTranslationTable (translations),
			  NULL);

  installDestroyHandler();
    
  // Add callbacks to handle resizing and exposures
    
  XtAddCallback ( _w, XmNresizeCallback, 
		  &Schematic::resizeCallback, ( XtPointer ) this );
  XtAddCallback ( _w, XmNexposeCallback, 
		  &Schematic::redisplayCallback, ( XtPointer ) this );

  // A graphics context is needed for copying the pixmap buffers and
  // erasing the back pixmap. Use the background color of 
  // the base widget for the fill color.
    
  XtVaGetValues ( _w, XmNbackground, &gcv.foreground, NULL );
    
  _gc = XtGetGC ( _w, GCForeground, &gcv ); 

  _draw_gc = XtGetGC ( _w, GCForeground, &draw_gcv ); 

  XSetForeground(XtDisplay(_w), _draw_gc,
		 BlackPixel(XtDisplay(_w),0));
  XtVaSetValues ( _w, XmNuserData, _draw_gc, NULL);

  // initialize mode

  setMode(Run);
    
  // Call resize to create the first pixmaps
    
  resize();
}

Schematic::~Schematic()

{
  // Empty
}

void Schematic::setMode(Mode mode)

{
  _mode = mode;
}

void Schematic::prepareSymbolAdd(Symbol::SymbolType symbolType)

{
  _symbolType = symbolType;
  _mode = Add;
}

void Schematic::cancelSymbolAdd()

{
  _mode = Edit;
}

void  Schematic::refresh()
{
  // Have each Symbol object draw itself into the back buffer
  // and then swap the buffers
    
  for ( int i = 0; i < _symbols.size(); i++)
    _symbols[i]->render ( _back, _width, _height );

  swapBuffers();
  
}

void Schematic::addSymbol ( Symbol *newSymbol )
{
  _symbols.add ( newSymbol );
}

void Schematic::removeSymbol ( Symbol *oldSymbol )
{
  _symbols.remove ( oldSymbol );   
}

//////////////////////////////////////////////////////
// trap()
//
// Action procedure to trap any of the events from the
// translation table.
//
//////////////////////////////////////////////////////

void Schematic::trap(Widget widget, XEvent *event, String *args,
		     unsigned int *num_args)

{

  XButtonEvent *bevent = (XButtonEvent *) event;

  if (*num_args != 1)
    XtError ("Schematic::trap - wrong number of args!");

  if (!strcmp (args[0], "down")) {

    cerr << " down(), x, y: " << bevent->x << ", " <<  bevent->y << "\n";

  } else if (!strcmp (args[0], "up")) {

    cerr << " up(), x, y: " << bevent->x << ", " <<  bevent->y << "\n";

  } else if (!strcmp (args[0], "motion")) {

    // Discard multiple motion events, keeping only the last one.
    // This helps speed up rubber-banding etc.

    XEvent *multipleMotionEvent = discardMultipleMotion();
    
    if (multipleMotionEvent != NULL) {
      bevent = (XButtonEvent *) multipleMotionEvent;
    }

    cerr << " motion(), x, y: " << bevent->x << ", " <<  bevent->y << "\n";

  }

  // keep compiler quiet about unused args

  Widget w;
  w = widget;
  
}

////////////////////////////////////////////////////////////////
// discardMultipleMotion()
//
// Discard multiple motion discards multiple motion events,
// returning a pointer to the last motion event, or NULL if
// no multiple motion events were found
//
////////////////////////////////////////////////////////////////

XEvent *Schematic::discardMultipleMotion()

{

  XEvent nextEvent;
  XEvent *motionEvent = NULL;
  
  int motionEventFound = TRUE;

  do { // while (motionEventFound)
      
    motionEventFound = FALSE;
    
    // Check to see if there are any events pending
    
    XtInputMask imask = XtAppPending(Application::Inst()->appContext());
    
    if (imask & XtIMXEvent) {
      
      // Peek at event
      
      XEvent peekEvent;
      
      if (XtAppPeekEvent(Application::Inst()->appContext(),
			 &peekEvent)) {
	
	// We have an X event - check for motion type
	
	if (peekEvent.xany.type == MotionNotify) {
	  motionEventFound = TRUE;
	  XtAppNextEvent(Application::Inst()->appContext(),
			 &nextEvent);

	  motionEvent = &nextEvent;
	}
	
      } // if (XtAppPeekEvent( ... 
      
    } // if (imask & XTIMXEvent)
    
  } while (motionEventFound);

  return (motionEvent);

}
