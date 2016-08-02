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
 *   $Date: 2016/03/06 23:53:43 $
 *   $Id: Wsim2mdv.hh,v 1.4 2016/03/06 23:53:43 dixon Exp $
 *   $Revision: 1.4 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * Wsim2mdv.hh : header file for the Wsim2mdv program class.
 *
 * RAP, NCAR, Boulder CO
 *
 * October 1998
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef Wsim2mdv_HH
#define Wsim2mdv_HH

/*
 **************************** includes **********************************
 */

#include <sys/time.h>

#include <didss/DsInputPath.hh>
#include <toolsa/InputDir.hh>
#include <toolsa/ldata_info.h>

#include "Args.hh"
#include "Params.hh"
#include "WsimFile.hh"
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

class Wsim2mdv
{
 public:

  // Destructor

  ~Wsim2mdv(void);
  
  // Get Wsim2mdv singleton instance

  static Wsim2mdv *Inst(int argc, char **argv);
  static Wsim2mdv *Inst();
  
  // Run the program.

  void run();
  
  // Flag indicating whether the program status is currently okay.

  bool okay;
  
  // Retrieves the program parameters

  Params *getParams(void)
  {
    return(_params);
  }
  
 private:

  // Constructor -- private because this is a singleton object

  Wsim2mdv(int argc, char **argv);
  
  // Singleton instance pointer

  static Wsim2mdv *_instance;
  
  // Program parameters.

  char *_progName;
  Args *_args;
  Params *_params;
  
  // The object used to handle the WSI mosaic file

  WsimFile *_wsimFile;
  
  // The object used to get file names for processing

  DsInputPath     *_inputPath;
  
  InputDir        *_inputDir;
  LDATA_handle_t   _ldataHandle;
  time_t           _ldataPrevTime;
  
  // Convert the filter type parameter read from the parameter file
  // into the value needed in the WsimFile class.

  WsimFileFilterType_t _convertParamFilterType(int filter_type);
  
  // Process the given file

  void _processFile(char *file_name);
  
  // Return the class name for error messages.

  static const char *_className(void)
  {
    return("Wsim2mdv");
  }
  
};


#endif
