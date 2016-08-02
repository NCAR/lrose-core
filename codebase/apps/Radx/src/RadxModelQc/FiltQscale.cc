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
 * @file FiltQscale.cc
 */
#include "FiltQscale.hh"
#include <toolsa/LogStream.hh>

//------------------------------------------------------------------
FiltQscale::FiltQscale(const Params::data_filter_t f,
		       const Params &P) : Filter(f, P)
{
  if (!ok())
  {
    return;
  }

  _scale = P._parm_qscale[f.filter_index].scale;
  _topv = P._parm_qscale[f.filter_index].topv;
  _lowv = P._parm_qscale[f.filter_index].lowv;
  _invert = P._parm_qscale[f.filter_index].invert;
}

//------------------------------------------------------------------
FiltQscale::~FiltQscale()
{
}

//------------------------------------------------------------------
void FiltQscale::filter_print(void) const
{
  LOG(DEBUG_VERBOSE) << "filtering";
}

//------------------------------------------------------------------
bool FiltQscale::canThread(void) const
{
  return true;
}

//------------------------------------------------------------------
bool FiltQscale::filter(const time_t &t, const RadxRay *ray0,
			const RadxRay &ray, const RadxRay *ray1,
			std::vector<RayxData> &data) const
{
  RayxData r;
  if (!RadxApp::retrieveRay(_f.input_field, ray, data, r))
  {
    return false;
  }

  r.qscale(_scale, _topv, _lowv, _invert);

  RadxApp::modifyRayForOutput(r, _f.output_field, _f.output_units,
                              _f.output_missing);
  data.push_back(r);
  return true;
}

//------------------------------------------------------------------
void FiltQscale::filterVolume(const RadxVol &vol)
{
}

//------------------------------------------------------------------
void FiltQscale::finish(void)
{
}

