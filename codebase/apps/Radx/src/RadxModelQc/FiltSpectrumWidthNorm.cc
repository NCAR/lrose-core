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
 * @file FiltSpectrumWidthNorm.cc
 */
#include "FiltSpectrumWidthNorm.hh"
#include <Radx/RadxVol.hh>
#include <toolsa/LogStream.hh>
#include <toolsa/globals.h>
#include <cmath>
//------------------------------------------------------------------
FiltSpectrumWidthNorm::FiltSpectrumWidthNorm(const Params::data_filter_t f,
					     const Params &P) : Filter(f, P)
{
  if (!ok())
  {
    return;
  }

  _dbz = P._parm_sw_norm[f.filter_index].dbz;
  _lambda = P._parm_sw_norm[f.filter_index].lambda;
  _meanValuesSet = false;
}

//------------------------------------------------------------------
FiltSpectrumWidthNorm::~FiltSpectrumWidthNorm()
{
}

//------------------------------------------------------------------
void FiltSpectrumWidthNorm::filter_print(void) const
{
  LOG(DEBUG_VERBOSE) << "filtering";
}

//------------------------------------------------------------------
bool FiltSpectrumWidthNorm::canThread(void) const
{
  return true;
}

//------------------------------------------------------------------
bool FiltSpectrumWidthNorm::filter(const time_t &t, const RadxRay *ray0,
				   const RadxRay &ray, const RadxRay *ray1,
				   std::vector<RayxData> &data) const
{
  if (!_meanValuesSet)
  {
    LOG(ERROR) << "Mean values not set";
    return false;
  }


  RayxData r;
  if (!RadxApp::retrieveRay(_f.input_field, ray, data, r))
  {
    return false;
  }


  double dwell = _meanPrt*_meanNsamples;

  // replace negative input values with missing value
  r.maskWhenLessThanOrEqual(0.0, 0.0, true);

  if (_dbz)
  {
    double f = 4.0*sqrt(PI)*dwell/_lambda;
    r.multiply(f);
    // now want to take this and set to 10*log10(1+sqrt(1.0/r)
    r.invert();
    r.squareRoot();
    r.inc(1.0);
    r.logBase10();
    r.multiply(10.0);
  }
  else
  {
    double f = _lambda/(8.0*dwell*sqrt(PI));
    r.multiply(f);
  }

  RadxApp::modifyRayForOutput(r, _f.output_field, _f.output_units,
                              _f.output_missing);
  data.push_back(r);
  return true;
}

//------------------------------------------------------------------
void FiltSpectrumWidthNorm::filterVolume(const RadxVol &vol)
{
  const vector<RadxRay *> &rays = vol.getRays();

  // take mean of prt values
  _meanPrt = 0.0;
  _meanNsamples = 0.0;

  for (size_t i=0; i<rays.size(); ++i)
  {
    _meanPrt += rays[i]->getPrtSec();
    _meanNsamples += rays[i]->getNSamples();
  }

  if (!rays.empty())
  {
    _meanPrt /= static_cast<double>(rays.size());
    _meanNsamples /= static_cast<double>(rays.size());
    _meanValuesSet = true;
  }
}

//------------------------------------------------------------------
void FiltSpectrumWidthNorm::finish(void)
{
  _meanValuesSet = false;
}
