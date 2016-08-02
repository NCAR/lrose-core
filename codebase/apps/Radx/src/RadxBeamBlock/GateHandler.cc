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
 * @file GateHandler.cc
 */
#include "GateHandler.hh"
#include <cmath>

//----------------------------------------------------------------
GateHandler::GateHandler(double gateMeters) :
  _gateMeters(gateMeters),
  _beamb(0),
  _beaml(0),
  _peak(0)
{
}

//----------------------------------------------------------------
GateHandler::~GateHandler(void)
{
}

//----------------------------------------------------------------
void GateHandler::finish(void)
{
  _beaml = _beamb;
  if (_beamb < 1.0)
  {
    _beamb = 10.0*log10(1.0/(1.0 - _beamb));
  }
}

//----------------------------------------------------------------
Radx::fl32 GateHandler::getData(Params::output_data_t type) const
{
  Radx::fl32 ret = 0;
  switch (type)
  {
  case Params::BLOCKAGE:
    ret = _beamb;
    break;
  case Params::LINEAR_BLOCKAGE:
    ret = _beaml;
    break;
  case Params::PEAK:
    ret = _peak;
    break;
  default:
    break;
  }
  return ret;
}
