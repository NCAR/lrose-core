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
// RhiOrient.hh
//
// RhiOrient class.
// Compute echo orientation in a pseudo RHI
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2020
//
///////////////////////////////////////////////////////////////

#ifndef RhiOrient_HH
#define RhiOrient_HH

#include <Radx/Radx.hh>
#include <Radx/PseudoRhi.hh>
#include "Params.hh"

class RhiOrient

{

public:

  // constructor

  RhiOrient(const Params &params,
            PseudoRhi *rhi,
            size_t maxNGates,
            double startRangeKm,
            double gateSpacingKm,
            double radarAltKm,
            const vector<double> &gridZLevels);
            
  // destructor

  ~RhiOrient();

  // compute the echo orientation

  void computeEchoOrientation();

private:

  const Params &_params;
  PseudoRhi *_rhi;
  size_t _maxNGates;
  double _startRangeKm;
  double _gateSpacingKm;
  double _radarAltKm;
  vector<double> _gridZLevels;

  vector< vector<Radx::fl32> > _dbz; 
  vector< vector<Radx::fl32> > _gridError; 

  void _free();

};

#endif
