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
 *   $Date: 2016/03/04 02:22:13 $
 *   $Id: UpdateMdvFieldNames.hh,v 1.5 2016/03/04 02:22:13 dixon Exp $
 *   $Revision: 1.5 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * UpdateMdvFieldNames.hh : header file for the UpdateMdvFieldNames
 *                          program class.
 *
 * RAP, NCAR, Boulder CO
 *
 * October 1998
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef UpdateMdvFieldNames_HH
#define UpdateMdvFieldNames_HH

/*
 **************************** includes **********************************
 */

#include <sys/time.h>

#include <didss/DsInputPath.hh>

#include "Args.hh"
#include "Params.hh"
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

class UpdateMdvFieldNames
{
 public:

  // Destructor

  ~UpdateMdvFieldNames(void);
  
  // Get UpdateMdvFieldNames singleton instance

  static UpdateMdvFieldNames *Inst(int argc, char **argv);
  static UpdateMdvFieldNames *Inst();
  
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

  UpdateMdvFieldNames(int argc, char **argv);
  
  // Singleton instance pointer

  static UpdateMdvFieldNames *_instance;
  
  // Program parameters.

  char *_progName;
  Args *_args;
  Params *_params;
  
  // Input path object

  DsInputPath *_inputPath;
  
  // Construct the file name

  static char *_constructFileName(char *directory, time_t data_time);
  
};


#endif
