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
/////////////////////////////////////////////////////////////
// RefractCalib Main
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2026
//
/////////////////////////////////////////////////////////////
//
// RefractCalib:
//    (a) reads radar scan files, in polar coordinates
//    (b) identifies suitable clutter targets
//    (c) computes the mean phase of those targets for a baseline calibration
//    (d) writes the calibration details to a file.
// Typically we use 6 hours of scans for this purpose.
// Ideally the moisture field should be uniform for this procedure to work well.
//
//////////////////////////////////////////////////////////////

#ifndef RefractCalib_HH
#define RefractCalib_HH

#include "Args.hh"
#include "Params.hh"
#include "Calib.hh"
#include <Refract/RefParms.hh>

#include <string>
#include <vector>
#include <sys/stat.h>

/** 
 * @class RefractCalib
 * @brief the Algorithm class
 */
class RefractCalib
{
  
public:

  // constructor

  RefractCalib(int argc, char **argv);
  
  /**
   * Flag indicating whether the program status is currently okay.
   */

  bool okay;
  
  /**
   * @brief Destructor
   */
  virtual ~RefractCalib();
  
  /**
   * @brief Run the program.
   */

  int run();

 private:

  string _progName; /**< program name */
  char *_paramsPath;
  Args _args;
  Params _params; /**< Parameter file paramters. */
  RefParms _refparms;
  Calib *_calib; /**< Data processing object. */

  // analysis time limits
  
  time_t _startTime, _endTime;
  
};


#endif
