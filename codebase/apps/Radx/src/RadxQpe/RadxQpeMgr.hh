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
 * @file RadxQpeMgr.hh
 * @brief RadxQpeMgr object
 * @class RadxQpeMgr
 * @brief RadxQpeMgr object
 *
 * Dave Albo, RAP, NCAR
 *
 * P.O.Box 3000, Boulder, CO, 80307-3000, USA
 *
 * March 2014
 *
 * RadxQpeMgr reads in a parameter file and uses that to create the
 * RadxQpe algorithm object, and the data handling objects,
 * then runs the algorithm
 */

#ifndef RADX_PRECIP_MGR_HH
#define RADX_PRECIP_MGR_HH

#include "Args.hh"
#include "Parms.hh"
#include "Interp.hh"
#include <string>
#include <vector>
class RadxQpe;
class InputData;
class OutputData;
class BeamBlock;

class RadxQpeMgr
{
public:
  /**
   * Constructor
   * @param[in] argc  Number of command line args
   * @param[in] argv  Command line args
   */
  RadxQpeMgr (int argc, char **argv, void tidyAndExit(int));

  /**
   * Destructor
   */
  virtual ~RadxQpeMgr(void);

  /**
   * Run the algorithm
   * @return 1 for failure, 0 for success
   */
  int Run(void);

  bool _isOK;  /**< Object status */

protected:
private:

  Parms *_params;          /**< App parameters pointer */
  RadxQpe *_alg;           /**< Algorithm object pointer */
  InputData *_data;        /**< Input/output data */
  OutputData *_out;        /**< Input/output data */
  BeamBlock *_beamBlock;   /**< Beam block input data */
  vector<Interp::Field> _interpFields;
  vector<Interp::Ray *> _interpRays;

  void _interpolate(const RadxVol &vol);
  void _initInterpFields(const RadxVol &vol);
  void _loadInterpRays(const RadxVol &vol);
  void _freeInterpRays();
};

#endif

