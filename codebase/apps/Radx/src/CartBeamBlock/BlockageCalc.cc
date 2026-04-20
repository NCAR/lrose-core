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
///////////////////////////////////////////////////////////////
//
// BlockageCalc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2026
//
///////////////////////////////////////////////////////////////
//
// Compute the blockage up to a Cartesian (x,y) grid point
// for each of the Cartesian levels.
//
///////////////////////////////////////////////////////////////

#include "BlockageCalc.hh"
#include "DemProvider.hh"

///////////////////////////////////////////////////////////////////////////
// constructor

BlockageCalc::BlockageCalc(const Params &params,
                           const DemProvider &dem) :
        _params(params),
        _dem(dem)
{
  
}

///////////////////////////////////////////////////////////////////////////
// destructor


BlockageCalc::~BlockageCalc(void)
{

}

// initialize geometry for computations

void BlockageCalc::initGeom(double maxRangeKm,
                            double rangeResKm,
                            const vector<double> &zCartKm,
                            int nBeamPatternEl,
                            int nBeamPatternAz)

{

  // initialize values
  
  _maxRangeKm = maxRangeKm;
  _rangeResKm = rangeResKm;

  _nRange = (int) ((_maxRangeKm / _rangeResKm) + 1);

  _zCartKm = zCartKm;
  _nBeamPatternEl = nBeamPatternEl;
  _nBeamPatternAz = nBeamPatternAz;
  
  // create 2D points array
  
  _points.clear();
  _points.resize(_nRange);
  for (size_t irange = 0; irange < _points.size(); irange++) {
    vector<AzRangePoint> &azPts = _points[irange];
    azPts.resize(_nBeamPatternAz);
    for (size_t iaz = 0; iaz < azPts.size(); iaz++) {
      AzRangePoint &pt = azPts[iaz];
      pt.cartEl.resize(_zCartKm.size());
      pt.fracBlocked.resize(_zCartKm.size());
    } // iaz
  } // irange
  
}

  
