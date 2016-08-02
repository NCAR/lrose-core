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

/**
 * @file RadxBeamBlockMgr.hh
 * @brief RadxBeamBlockMgr object
 * @class RadxBeamBlockMgr
 * @brief RadxBeamBlockMgr object
 *
 * Dave Albo, RAP, NCAR
 *
 * P.O.Box 3000, Boulder, CO, 80307-3000, USA
 *
 * March 2014
 *
 * RadxBeamBlockMgr reads in a parameter file and uses that to create the
 * RadxBeamBlock algorithm object, and runs the algorithm
 */

#ifndef RADXBEAMBLOCKMGR_HH
#define RADXBEAMBLOCKMGR_HH

#include "Args.hh"
#include "Parms.hh"
#include <string>
#include <vector>

class RadxBeamBlock;

class RadxBeamBlockMgr
{
public:
  /**
   * Constructor
   * @param[in] argc  Number of command line args
   * @param[in] argv  Command line args
   */
  RadxBeamBlockMgr (int argc, char **argv);

  /**
   * Destructor
   */
  virtual ~RadxBeamBlockMgr(void);

  /**
   * Run the algorithm
   * @return 1 for failure, 0 for success
   */
  int Run(void);

  bool isOK;  /**< Object status */

protected:
private:

  string _progName;   /**< Name of app */
  char *_paramsPath;  /**< Name of parm file */
  Args _args;         /**< Argument handler */
  Parms *_params;     /**< App parameters pointer */
  RadxBeamBlock *_alg;/**< Algorithm object pointer */
};

#endif

