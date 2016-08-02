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
 *   $Date: 2016/03/04 02:22:16 $
 *   $Id: UpdateMdvTimes.hh,v 1.6 2016/03/04 02:22:16 dixon Exp $
 *   $Revision: 1.6 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * UpdateMdvTimes.hh : header file for the UpdateMdvTimes program class.
 *
 * RAP, NCAR, Boulder CO
 *
 * October 1998
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef UpdateMdvTimes_HH
#define UpdateMdvTimes_HH

#include <sys/time.h>

#include <Mdv/DsMdvx.hh>
#include <toolsa/DateTime.hh>

#include "Args.hh"
#include "Params.hh"
using namespace std;

class UpdateMdvTimes
{
 public:

  // Destructor

  ~UpdateMdvTimes(void);
  
  // Get UpdateMdvTimes singleton instance

  static UpdateMdvTimes *Inst(int argc, char **argv);
  static UpdateMdvTimes *Inst();
  
  // Run the program.

  int run();
  
  // Flag indicating whether the program status is currently okay.

  bool okay;
  
  // Retrieves the program parameters

  Params *getParams(void)
  {
    return(_params);
  }
  
 private:

  // Constructor -- private because this is a singleton object

  UpdateMdvTimes(int argc, char **argv);
  
  // Singleton instance pointer

  static UpdateMdvTimes *_instance;

  // process file

  int _processFile(const string &file_path);
  int _processFile(const DateTime trigger_time);
  int _processFile(DsMdvx &mdvx);

  // Program parameters.

  char *_progName;
  Args *_args;
  Params *_params;
  
};


#endif
