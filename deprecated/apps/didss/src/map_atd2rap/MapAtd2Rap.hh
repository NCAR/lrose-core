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
 *   $Date: 2016/03/06 23:53:42 $
 *   $Id: MapAtd2Rap.hh,v 1.3 2016/03/06 23:53:42 dixon Exp $
 *   $Revision: 1.3 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * MapAtd2Rap.hh : header file for the MapAtd2Rap program class.
 *
 * RAP, NCAR, Boulder CO
 *
 * October 1998
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef MapAtd2Rap_HH
#define MapAtd2Rap_HH

/*
 **************************** includes **********************************
 */

#include <sys/time.h>

#include <rapformats/Map.hh>

#include "Args.hh"
using namespace std;

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

class MapAtd2Rap
{
 public:

  // Destructor

  ~MapAtd2Rap(void);
  
  // Get MapAtd2Rap singleton instance

  static MapAtd2Rap *Inst(int argc, char **argv);
  static MapAtd2Rap *Inst();
  
  // Run the program.

  void run();
  
  // Flag indicating whether the program status is currently okay.

  bool okay;
  
 private:

  // Constructor -- private because this is a singleton object

  MapAtd2Rap(int argc, char **argv);
  
  // Singleton instance pointer

  static MapAtd2Rap *_instance;
  
  // Program parameters.

  char *_progName;
  Args *_args;
  
  // Read the ATD map from the indicated file.

  Map *_readAtdMap(string atd_filename);
  
  // Return the class name for error messages.

  static const char *_className(void)
  {
    return("MapAtd2Rap");
  }
  
};


#endif
