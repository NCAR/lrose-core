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
 *
 * @file LinearInterpArgs.hh
 *
 * @class LinearInterpArgs
 *
 * Class representing a target data point.
 *  
 * @date 3/31/2010
 *
 */

#ifndef LinearInterpArgs_H
#define LinearInterpArgs_H

/** 
 * @class LinearInterpArgs
 */

class LinearInterpArgs
{
  
public:

  inline LinearInterpArgs(int r, int smoothSideLen, double azimSpacing,
			  double gateSpacing, int numBeams,
			  int twoSmoothRange, double minConsistency,
			  double minAbsConsistency,
			  float range_slope, float init_slope)
  {
    _r = r;
    _smooth_azim = (int)(smoothSideLen*360.0/
			 (azimSpacing*r*gateSpacing*4.0*M_PI));
    if (_smooth_azim >= numBeams/8) _smooth_azim = numBeams/8 - 1;
    if (_smooth_azim <= 0) _smooth_azim = 1;
    _two_smooth_azim = 2*_smooth_azim;
    _minconsistency = (twoSmoothRange+1)*(_two_smooth_azim+1)*minConsistency;
    if (_minconsistency < minAbsConsistency)
      _minconsistency = minAbsConsistency;
    _range_slope = range_slope;
    _init_slope = init_slope;
  }

  inline ~LinearInterpArgs() {}

  inline void initForAz(int azn, int azjump, int rjump)
  {
    _azn = azn;
    _azjump = azjump;
    _rjump = rjump;
  }

  inline int offsetAzimuthIndex(int j) const
  {
    return _azn + j + _smooth_azim;
  }
  
  inline int offsetAzimuthIndexNeg(int j) const
  {
    return _azn + j - _smooth_azim;
  }
  
  int _r;
  int _azn;
  int _azjump;
  int _smooth_azim;
  int _two_smooth_azim;
  float _minconsistency;
  int _rjump;
  float _range_slope;
  float _init_slope;
  
};

#endif
