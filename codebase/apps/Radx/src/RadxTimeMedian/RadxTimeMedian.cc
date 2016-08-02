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
#include "Params.hh"
#include "Info.hh"
#include <radar/RadxAppTemplate.hh>
#include <Radx/RayxData.hh>
#include <Radx/RadxRay.hh>
#include <toolsa/LogMsg.hh>
#include <algorithm>
#include <stdexcept>
#include <toolsa/TaThreadSimple.hh>

//------------------------------------------------------------------
TaThread *RadxTimeMedian::RadxThreads::clone(int index)
{
  TaThreadSimple *t = new TaThreadSimple(index);
  t->setThreadContext(this);
  t->setThreadMethod(RadxTimeMedian::compute);
  return dynamic_cast<TaThread *>(t);
}

//------------------------------------------------------------------
RadxTimeMedian::RadxTimeMedian(int argc, char **argv,void cleanup(int),
			       void outOfStore(void))
{
  _first = true;
  OK = parmAppInit(_params, _alg, argc, argv);
  vector<string> input;
  input.push_back(_params.input_field);
  if (!_alg.init(cleanup, outOfStore, input))
  {
    OK = false;
  }

  _rayMap = RayxMapping(_params.fixedElevations_n, _params._fixedElevations,
                        _params.azToleranceDegrees,
                        _params.elevToleranceDegrees);

  _thread.init(_alg.parmRef().num_threads, _alg.parmRef().thread_debug);
  return;
}

//------------------------------------------------------------------
// destructor

RadxTimeMedian::~RadxTimeMedian()
{
  _alg.finish();
}

//------------------------------------------------------------------
int RadxTimeMedian::Run()
{
  RadxVol vol;
  time_t t;
  bool last;
  while (_alg.trigger(vol, t, last))
  {
    _process(t, vol, last);
    _thread.waitForThreads();
    if (last)
    {
      _alg.write(vol, t);
    }
  }
  return 0;
}

//------------------------------------------------------------------
void RadxTimeMedian::compute(void *ti)
{
  Info *info = static_cast<Info *>(ti);
  RadxTimeMedian *alg = info->_alg;//static_cast<RadxTimeMedian *>(ai);

  // perform computations
  double az = info->_ray->getAzimuthDeg();
  double elev = info->_ray->getElevationDeg();

  RayHisto *h = alg->matchingRayHisto(az, elev);
  if (h == NULL)
  {
    LOGF(LogMsg::WARNING, "No histo match for az=%lf elev=%lf",
	    az, elev);
    return;
  }
  RayxData r;
  if (!RadxApp::retrieveRay(info->_input_field, *info->_ray, r))
  {
    return;
  }
  bool multi = alg->isMulti(az, elev);
  if (multi)
  {
    alg->_thread.lockForIO();
  }
  h->update(r);
  if (multi)
  {
    alg->_thread.unlockAfterIO();
  }

  delete info;
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
    catch (std::out_of_range err)
    {
      printf("%s is out of range of mappings\n", ae.sprint().c_str());
      return NULL;
    }
  }
  return NULL;
}

//------------------------------------------------------------------
void RadxTimeMedian::_process(const time_t t, RadxVol &vol, const bool last)
{
  const vector<RadxRay *> &rays = vol.getRays();

  if (_first)
  {
    _templateVol = vol;
    LOGF(LogMsg::DEBUG_VERBOSE, "First: Nrays=%d",
	 static_cast<int>(rays.size()));
    for (size_t ii = 0; ii < rays.size(); ii++)
    {
      RadxRay *ray = rays[ii];
      _filter_first(t, *ray);
    }
    _first = false;
  }

  // break the vol into rays and process each one, using threads if 
  // configured for threads
  LOGF(LogMsg::DEBUG_VERBOSE, "Nrays=%d", static_cast<int>(rays.size()));
  for (size_t ii = 0; ii < rays.size(); ii++)
  {
    const RadxRay *ray = rays[ii];
    _filter(t, ray);
  }
  _thread.waitForThreads();

  if (last)
  {
    // replace vol with the first template volume
    vol = _templateVol;
    for (size_t ii = 0; ii < rays.size(); ii++)
    {
      RadxRay *ray = rays[ii];
      _filter_last(t, *ray);
    }
  }    
}

//------------------------------------------------------------------
bool RadxTimeMedian::_filter_first(const time_t &t, const RadxRay &ray)
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
      LOGF(LogMsg::WARNING,
	      "Multiple az,elev (%f,%f) found in volume maps to %s", 
	      az, elev, ae.sprint().c_str());
    }
    else
    {
      RayHisto h(ae.getAz(), ae.getElev(), x0, dx, nx,
		 _params.min_bin, _params.delta_bin, _params.max_bin);
      _store[ae] = h;
    }
    return true;
  }
  else
  {
    LOGF(LogMsg::ERROR, "Az/Elev %lf,%f not configured within tolerance\n",
	    az, elev);
    return false;
  }
}

//------------------------------------------------------------------
void RadxTimeMedian::_filter(const time_t &t, const RadxRay *ray)
{
  Info *info = new Info(_params.input_field, ray, this);
  int index = 0;
  _thread.thread(index, info);
}

//------------------------------------------------------------------
bool RadxTimeMedian::_filter_last(const time_t &t, RadxRay &ray)
{
  double az = ray.getAzimuthDeg();
  double elev = ray.getElevationDeg();
  RayHisto *h = matchingRayHisto(az, elev);
  if (h == NULL)
  {
    return false;
  }

  RayxData r;
  if (!RadxApp::retrieveRay(_params.input_field, ray, r))
  {
    return false;
  }
  // put the medians into r
  bool stat = h->computeMedian(r);
  if (stat)
  {
    RadxApp::modifyRayForOutput(r, _params.output_field);
    RadxApp::updateRay(r, ray);
  }
  return stat;
}
