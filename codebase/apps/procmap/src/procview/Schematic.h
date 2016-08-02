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
// Schematic.h
/////////////////////////////////////////////////

#ifndef SCHEMATIC_H
#define SCHEMATIC_H

#include <MotifApp/SimpleList.h>
#include "BufferedDraw.h"
#include "Symbol.h"

class Schematic : public BufferedDraw {
    
public:
    
  Schematic ( const char *, Widget );    
  virtual ~Schematic ();    
    
  virtual void refresh();   // Refresh all symbols
    
  void addSymbol ( Symbol * );       
  void removeSymbol ( Symbol * );
    
  virtual const char *const className() { return ( "Schematic" ); }

  // mode - run/edit

  enum Mode { Run, Edit, Add };
  void setMode (Mode);

  void prepareSymbolAdd(Symbol::SymbolType symbolType);
  void cancelSymbolAdd();
  
protected:
    
  GC          _draw_gc;          // Used to draw

  SimpleList<Symbol*> _symbols;  // List of Symbol objects on the Schematic
    
private:

  // current schematic mode

  Mode _mode;

  // current symbol type

  Symbol::SymbolType _symbolType;

  // callbacks
    
  static void trap(Widget widget, XEvent *event,
		   String *args, unsigned int *num_args);
  
  static XEvent *Schematic::discardMultipleMotion();

};

#endif
