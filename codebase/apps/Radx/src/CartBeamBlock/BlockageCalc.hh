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

#ifndef BLOCKAGE_CALC_HH
#define BLOCKAGE_CALC_HH

#include "Params.hh"
#include <vector>
class DemProvider;

class BlockageCalc
{
public:

  // constructor
  
  BlockageCalc(const Params &params,
               const DemProvider &dem);
  
  // destructor
  
  virtual ~BlockageCalc();

  // initialize geometry for computations

  void initGeom(double maxRangeKm,
                double rangeResKm,
                const vector<double> &zCartKm,
                int nBeamPatternEl,
                int nBeamPatternAz);

protected:
  
private:
  
  const Params &_params;
  const DemProvider &_dem;

  double _maxRangeKm;
  double _rangeResKm;
  int _nRange;
  
  vector<double> _zCartKm;
  int _nBeamPatternAz, _nBeamPatternEl;

  class AzRangePoint {

  public:
    
    double lat;
    double lon;
    double rangeKm;
    double terrainHtKm;

    vector<double> cartEl;
    vector<double> fracBlocked;

  };

  // the dimensions will be [_Range][_nBeamPatternAz]
  
  vector< vector< AzRangePoint > > _points;
  
};

#endif

