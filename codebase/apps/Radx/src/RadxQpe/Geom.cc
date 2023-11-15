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
#include <toolsa/copyright.h>
#include <cmath>

/**
 * @file Geom.cc
 */

#include "Geom.hh"
#include <toolsa/LogMsg.hh>
#include <cstdlib>

//----------------------------------------------------------------
Geom::Geom(void) :
  _isIndexed(false),
  _da(0),
  _r0(0),
  _dr(0),
  _nr(0),
  _output_da(0),
  _output_na(0)
{
}

//----------------------------------------------------------------
Geom::Geom(bool isIndexed, double angleResDeg, 
	   const std::vector<double> &az, int nr, double r0, double dr,
	   double outputAngleResDeg) :
  _isIndexed(isIndexed),
  _da(angleResDeg),
  _r0(r0),
  _dr(dr),
  _nr(nr),
  _az(az),
  _output_da(outputAngleResDeg)
{
  if (_isIndexed)
  {
    _az.clear();
    for (double a=0.0; a<360.0; a += _da)
    {
      _az.push_back(a);
    }
  }

  _output_na = (int) ((360.0 / fabs(outputAngleResDeg)) + 0.5);

}

//----------------------------------------------------------------
Geom::~Geom()
{
}

//----------------------------------------------------------------
bool Geom::closestGateIndex(double gate, int &ig) const
{
  ig = (int)((gate - _r0)/_dr);
  if (ig < 0)
  {
    return false;
  }
  if (ig >= _nr)
  {
    return false;
  }
  return true;
}

//----------------------------------------------------------------
bool Geom::closestOutputAzimuthIndex(double az, int &iaz) const
{
  iaz = (int)(az/_output_da);
  if (iaz < 0)
  {
    return false;
  }
  if (iaz >= _output_na)
  {
    return false;
  }
  return true;
}
