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
/////////////////////////////////////////////////
// BufferedDraw.h
//
// Drawing area with double buffering
//
/////////////////////////////////////////////////

#ifndef BUFFEREDDRAW_H
#define BUFFEREDDRAW_H

#include <MotifApp/UIComponent.h>

class BufferedDraw : public UIComponent {
    
public:
    
  virtual ~BufferedDraw ();    
    
  virtual void refresh() = 0;   // Refresh
    
  virtual const char *const className() { return ( "BufferedDraw" ); }

protected:
    
  BufferedDraw ( const char * ); // protected constructor

  GC          _gc;               // Used to clear and copy pixmaps
  Dimension   _width, _height;   // Current window/buffer size
  Pixmap      _front, _back;     // Buffers (Always draw to _back)
    
  virtual void resize();
  virtual void redisplay();
  virtual void swapBuffers();
    
  static void resizeCallback ( Widget, XtPointer, XtPointer );
  static void redisplayCallback ( Widget, XtPointer, XtPointer );

private:

};
#endif
