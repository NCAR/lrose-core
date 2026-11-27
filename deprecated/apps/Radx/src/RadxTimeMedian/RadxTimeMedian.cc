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
 * @file RadxTimeMedian.cc
 */

#include "RadxTimeMedian.hh"
#include "Volume.hh"

#include <Radx/RadxRay.hh>
#include <toolsa/LogStream.hh>
#include <toolsa/TaThreadSimple.hh>

//------------------------------------------------------------------
RadxTimeMedian::RadxTimeMedian(const Parms &parms) :
  _parms(parms), _first(true)
{
  _rayMap = RayxMapping(_parms.fixedElevations_n, _parms._fixedElevations,
                        _parms.azToleranceDegrees,
                        _parms.elevToleranceDegrees);
  return;
}

//------------------------------------------------------------------
RadxTimeMedian::~RadxTimeMedian()
{
  RadxApp::algFinish();
}

//------------------------------------------------------------------
void RadxTimeMedian::initFirstTime(Volume *volume)
{
  if (_first)
  {
    const vector<RadxRay *> &rays = volume->getVolRef().getRays();
    LOG(DEBUG_VERBOSE) << "Nrays=" << rays.size();
    volume->copy(_templateVolume);
    LOG(DEBUG_VERBOSE) << "First: Nrays=" << rays.size();
    for (size_t ii = 0; ii < rays.size(); ii++)
    {
      RadxRay *ray = rays[ii];
      _filter_first(*ray);
    }
    _first = false;
  }
}

//------------------------------------------------------------------
void RadxTimeMedian::processLast(Volume *volume)
{
  // // replace vol with the first template volume
  volume->setFrom(_templateVolume);

  const vector<RadxRay *> &rays = volume->getVolRef().getRays();
  LOG(DEBUG_VERBOSE) << "Nrays=" << rays.size();

  for (size_t ii = 0; ii < rays.size(); ii++)
  {
      RadxRay *ray = rays[ii];
      _filter_last(*ray);
  }
}

//-----------------------------------------------------------------
RayHisto *RadxTimeMedian::initRay(const RadxRay &ray, RayxData &r)
{
  if (!RadxApp::retrieveRay(_parms.input_field, ray, r))
  {
    return NULL;
  }
  
  double az = ray.getAzimuthDeg();
  double elev = ray.getElevationDeg();
  RayHisto *h = matchingRayHisto(az, elev);
  if (h == NULL)
  {
    LOG(WARNING) << "No histo match for az=" << az << " elev=" << elev;
  }
  return h;
}

//-----------------------------------------------------------------
bool RadxTimeMedian::processRay(const RayxData &r, RayHisto *h) const
{
  h->update(r);
  return true;
}  

//------------------------------------------------------------------
RayHisto *RadxTimeMedian::matchingRayHisto(const double az,
					   const double elev)
{
  RadxAzElev ae = _rayMap.match(az, elev);
  if (ae.ok())
  {
    try
    {
      return &_store.at(ae);
    }
    catch (std::out_of_range &err)
    {
      printf("%s is out of range of mappings\n", ae.sprint().c_str());
      return NULL;
    }
  }
  return NULL;
}

//------------------------------------------------------------------
bool RadxTimeMedian::_filter_first(const RadxRay &ray)
{
  double az = ray.getAzimuthDeg();
  double elev = ray.getElevationDeg();
  double x0 = ray.getStartRangeKm();
  double dx = ray.getGateSpacingKm();
  int nx = ray.getNGates();

  if (_rayMap.add(ray))
  {
    RadxAzElev ae = _rayMap.match(az, elev);
    if (_rayMap.isMulti(ae))
    {
      LOG(WARNING) << "Multiple az,elev found in volume " << 
	az << " " << elev << "maps to " <<  ae.sprint();
    }
    else
    {
      RayHisto h(ae.getAz(), ae.getElev(), x0, dx, nx,
		 _parms.min_bin, _parms.delta_bin, _parms.max_bin);
      _store[ae] = h;
    }
    return true;
  }
  else
  {
    LOG(ERROR) << "Az/Elev not configured within tolerance" << az
	       << " " << elev;
    return false;
  }
}

//------------------------------------------------------------------
bool RadxTimeMedian::_filter_last(RadxRay &ray)
{
  double az = ray.getAzimuthDeg();
  double elev = ray.getElevationDeg();
  RayHisto *h = matchingRayHisto(az, elev);
  if (h == NULL)
  {
    LOG(ERROR) << "No last";
    return false;
  }

  RayxData r;
  if (!RadxApp::retrieveRay("DBZ_F", ray, r))
  {
    return false;
  }
  // put the medians into r
  bool stat = h->computeMedian(r);
  if (stat)
  {
    LOG(DEBUG_VERBOSE) << "Writing medians";
    RadxApp::modifyRayForOutput(r, _parms.output_field);
    RadxApp::updateRay(r, ray);
  }
  return stat;
}

