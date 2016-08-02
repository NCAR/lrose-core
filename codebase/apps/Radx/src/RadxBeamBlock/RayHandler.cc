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
 * @file RayHandler.cc
 */
#include "RayHandler.hh"
#include "Parms.hh"


//----------------------------------------------------------------
RayHandler::RayHandler(double azimuth, double elevation, const Parms &params) :
  _azimuth(azimuth),
  _elevation(elevation)
{
  for (int i=0; i<params.ngates(); ++i)
  {
    _gate.push_back(GateHandler(params.ithGate(i)*1000.0));
  }
}

//----------------------------------------------------------------
RayHandler::~RayHandler(void)
{
}

//----------------------------------------------------------------
Radx::fl32 *RayHandler::createData(Params::output_data_t type) const
{
  if (type == Params::EXTENDED_BLOCKAGE)
  {
    return createMaxData(Params::LINEAR_BLOCKAGE);
  }
  else
  {
    int npt = static_cast<int>(_gate.size());
    Radx::fl32 *ret = new Radx::fl32[npt];

    for (int i=0; i<npt; ++i)
    {
      ret[i] = _gate[i].getData(type);
    }
    return ret;
  }
}

//----------------------------------------------------------------
Radx::fl32 *RayHandler::createMaxData(Params::output_data_t type) const
{
  int npt = static_cast<int>(_gate.size());
  Radx::fl32 *ret = new Radx::fl32[npt];
  Radx::fl32 max = _gate[0].getData(type);
  for (int i=0; i<npt; ++i)
  {
    Radx::fl32 di = _gate[i].getData(type);
    if (di <= max)
    {
      ret[i] = max;
    }
    else
    {
      max = di;
      ret[i] = di;
    }
  }
  return ret;
}

//----------------------------------------------------------------
void RayHandler::finish(void)
{
  for (auto &gate : _gate)
  {
    gate.finish();
  }
}
