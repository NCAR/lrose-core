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
 * @file FiltFIR.cc
 */
#include "FiltFIR.hh"
#include <toolsa/LogMsg.hh>

//------------------------------------------------------------------
FiltFIR::FiltFIR(const Params::data_filter_t f,
		   const Params &P) : Filter(f, P)
{
  if (!ok())
  {
    return;
  }
  _outputNoiseField = P._parm_fir[f.filter_index].noise_field_name;
  switch (P._parm_fir[f.filter_index].edge_compute)
  {
  case Params::USE_FIRST_DATA:
    _edge = RayxData::FIR_EDGE_CLOSEST;
    break;
  case Params::MIRROR:
    _edge = RayxData::FIR_EDGE_MIRROR;
    break;
  case Params::MEAN:
    _edge = RayxData::FIR_EDGE_MEAN;
    break;
  case Params::INTERP:
    _edge = RayxData::FIR_EDGE_INTERP;
    break;
  default:
    LOG(LogMsg::FATAL, "edge compute type out of range");
    _ok = false;
  }    
  
  if (_ok)
  {
    _ok = _coeffSet(P._parm_fir[f.filter_index].coeff_index, P);
  }
}

//------------------------------------------------------------------
FiltFIR::~FiltFIR()
{
}
//------------------------------------------------------------------
void FiltFIR::filter_print(void) const
{
  LOG(LogMsg::DEBUG_VERBOSE, "filtering");
}

//------------------------------------------------------------------
bool FiltFIR::canThread(void) const
{
  return true;
}

//------------------------------------------------------------------
bool FiltFIR::filter(const time_t &t, const RadxRay *ray0, const RadxRay &ray,
		     const RadxRay *ray1, std::vector<RayxData> &data) const
{
  RayxData r;
  if (!RadxApp::retrieveRay(_f.input_field, ray, data, r))
  {
    return false;
  }

  RayxData quality = r;

  bool debug=false; // (r.getAzimuth() == 158);
  r.setDebug(debug);
  r.FIRfilter(_coeff, _edge, quality);
  RadxApp::modifyRayForOutput(r, _f.output_field, _f.output_units,
                              _f.output_missing);
  string out = _f.output_field;
  out += "_qual";
  RadxApp::modifyRayForOutput(quality, out, "none", -1.0);
  data.push_back(r);
  data.push_back(quality);
  return true;
}

//------------------------------------------------------------------
void FiltFIR::filterVolume(const RadxVol &vol)
{
}


//------------------------------------------------------------------
void FiltFIR::finish(void)
{
}


//------------------------------------------------------------------
bool FiltFIR::_coeffSet(int index, const Params &P)
{
  switch (index)
  {
  case 0:
    return _coeffSet(P.coeff0_n, P._coeff0);
  case 1:
    return _coeffSet(P.coeff1_n, P._coeff1);
  case 2:
    return _coeffSet(P.coeff2_n, P._coeff2);
  default:
    LOGF(LogMsg::ERROR, "coefficient index %d out of range 0-2", index);
    return false;
  }
}

//------------------------------------------------------------------
bool FiltFIR::_coeffSet(int n, double *v)
{
  _coeff.clear();
  for (int i=0; i<n; ++i)
  {
    _coeff.push_back(v[i]);
  }
  return n > 0;
}
