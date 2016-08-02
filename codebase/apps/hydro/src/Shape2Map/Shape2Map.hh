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
 *   $Date: 2016/03/07 18:36:50 $
 *   $Id: Shape2Map.hh,v 1.8 2016/03/07 18:36:50 dixon Exp $
 *   $Revision: 1.8 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * Shape2Map.hh : header file for the Shape2Map program class.
 *
 * RAP, NCAR, Boulder CO
 *
 * June 2000
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef Shape2Map_HH
#define Shape2Map_HH


#include <rapformats/MapIconDef.hh>
#include <rapformats/MapObject.hh>
#include <shapelib/shapefil.h>
#include <toolsa/str.h>

#include "Args.hh"
#include "Params.hh"
using namespace std;


/*
 **************************** includes **********************************
 */


/*
 ******************************* defines ********************************
 */


/*
 ******************************* structures *****************************
 */

/*
 ************************* global variables *****************************
 */

/*
 ***************************** function prototypes **********************
 */

/*
 ************************* class definitions ****************************
 */

class Shape2Map
{
 public:

  // Destructor

  ~Shape2Map(void);
  
  // Get Shape2Map singleton instance

  static Shape2Map *Inst(int argc, char **argv);
  static Shape2Map *Inst();
  
  // Run the program.

  void run();
  
  // Flag indicating whether the program status is currently okay.

  bool okay;
  
 private:

  // Singleton instance pointer

  static Shape2Map *_instance;
  
  // Program parameters.

  char *_progName;
  Args *_args;
  Params *_params;
  
  // Handles for the different Shape files

  SHPHandle _shapeHandle;
  DBFHandle _databaseHandle;
  

  /////////////////////
  // Private methods //
  /////////////////////

  // Constructor -- private because this is a singleton object

  Shape2Map(int argc, char **argv);
  
  // Create an icon map object from a point shape object.

  MapObject *_createIconObject(const SHPObject &shape,
			       MapIconDef *icon_def,
			       const int shape_num) const;
  
  // Create a polygon map object from a polygon shape object.

  MapObject *_createPolygonObject(const SHPObject &shape) const;

  
};


#endif
