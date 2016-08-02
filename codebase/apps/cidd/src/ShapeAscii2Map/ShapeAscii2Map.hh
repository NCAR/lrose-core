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
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

/* RCS info
 *   $Author: dixon $
 *   $Locker:  $
 *   $Date: 2016/03/07 18:28:25 $
 *   $Id: ShapeAscii2Map.hh,v 1.2 2016/03/07 18:28:25 dixon Exp $
 *   $Revision: 1.2 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * ShapeAscii2Map : ShapeAscii2Map program class.
 *
 * RAP, NCAR, Boulder CO
 *
 * Sept 2006
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef ShapeAscii2Map_HH
#define ShapeAscii2Map_HH

#include <toolsa/str.h>

#include "Args.hh"
#include "Params.hh"

#include "InputProcessor.hh"

using namespace std;

class ShapeAscii2Map
{
 public:

  // Destructor

  ~ShapeAscii2Map(void);
  
  // Get ShapeAscii2Map singleton instance

  static ShapeAscii2Map *Inst(int argc, char **argv);
  static ShapeAscii2Map *Inst();
  
  // Initialize the program.  Must be called before run().

  bool init();
  
  // Run the program.

  void run();
  
  // Flag indicating whether the program status is currently okay.

  bool okay;
  
 private:

  // Singleton instance pointer

  static ShapeAscii2Map *_instance;
  
  // Program parameters.

  char *_progName;
  Args *_args;
  Params *_params;
  
  InputProcessor *_inputProcessor;
  

  /////////////////////
  // Private methods //
  /////////////////////

  // Constructor -- private because this is a singleton object

  ShapeAscii2Map(int argc, char **argv);
  
};


#endif
