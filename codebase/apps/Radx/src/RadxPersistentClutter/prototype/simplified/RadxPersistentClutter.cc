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
 * @file RadxPersistentClutter.cc
 */


#include "RadxPersistentClutter.hh"
#include "Info.hh"
#include "AppTemplate.hh"
#include <Radx/RadxRay.hh>
#include <Radx/RadxVol.hh>
#include <toolsa/LogMsg.hh>
#include <toolsa/TaThreadSimple.hh>
#include <algorithm>

//------------------------------------------------------------------
TaThread *RadxPersistentClutter::ComputeThread::clone(int index)
{
  TaThreadSimple *t = new TaThreadSimple(index);
  t->setThreadContext(this);
  t->setThreadMethod(RadxPersistentClutter::compute);
  return dynamic_cast<TaThread *>(t);
}

//------------------------------------------------------------------
RadxPersistentClutter::RadxPersistentClutter(int argc, char **argv,
					     void cleanup(int),
					     void outOfStore(void))
{
  // call the library initializer
  OK = parmAppInit(_params, _alg, argc, argv);

  // and the algorithm initializer
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
}

//------------------------------------------------------------------
RadxPersistentClutter::~RadxPersistentClutter(void)
{
  _alg.finish();
}

//------------------------------------------------------------------
bool RadxPersistentClutter::run(void)
{
  RadxVol vol;
  time_t t;
  bool last;

  bool first = true;

  // trigger at time
  while (_alg.trigger(vol, t, last))
  {
    if (first)
    {
      // virtual method
      initFirstTime(t, vol);
      _processFirst(t, vol);
      first = false;
    }

    // process the vol
    bool done = _process(t, vol);

    if (done)
    {
      // it has converged
      _thread.waitForThreads();

      // virtual method
      finishLastTimeGood(t, vol);
      return true;
    }
  }
  // virtual method
  finishBad();
  return false;
}

//----------------------------------------------------------------
void RadxPersistentClutter::compute(void *ti)
{
  Info *info = static_cast<Info *>(ti);
  RadxPersistentClutter *alg = info->_alg;

  if (info->_time == 0 || info->_ray == NULL || info->_alg == NULL)
  {
    LOG(LogMsg::ERROR, "Values not set on entry");
    return;
  }

  RayxData r;
  RayClutterInfo *h = alg->_initRayThreaded(*info->_ray, r);
  if (h != NULL)
  {
    // call a virtual method
    alg->processRay(r, h);
  }
  delete info;
}    

//------------------------------------------------------------------
RayClutterInfo *RadxPersistentClutter::_initRayThreaded(const RadxRay &ray,
							RayxData &r)
{
  // lock because the method can change ray, in spite of the const!
  _thread.lockForIO();
  if (!App::retrieveRay(_params.input_field, ray, r))
  {
    _thread.unlockAfterIO();
    return NULL;
  }
  _thread.unlockAfterIO();

  // Point to the matching clutter info for this ray
  double az = ray.getAzimuthDeg();
  double elev = ray.getElevationDeg();
  RayClutterInfo *h = matchingClutterInfo(az, elev);
  if (h == NULL)
  {
    LOGF(LogMsg::WARNING, "No histo match for az=%lf elev=%lf",
	    az, elev);
  }
  else
  {
    LOGF(LogMsg::DEBUG_VERBOSE, "Updating ray az:%lf  elev:%lf", az, elev);
  }
  return h;
}

//------------------------------------------------------------------
void RadxPersistentClutter::_processForOutput(RadxVol &vol)
{
  // break the volume into rays and process each one
  const vector<RadxRay *> &rays = vol.getRays();
  for (size_t ii = 0; ii < rays.size(); ii++)
  {
    RadxRay *ray = rays[ii];
    _processRayForOutput(*ray);
  }
}

//------------------------------------------------------------------
double RadxPersistentClutter::_countOfScans(const int number) const
{
  double ret = 0.0;
  for (std::map<RadxAzElev, RayClutterInfo>::const_iterator ii = _store.begin();
       ii!=_store.end(); ++ii)
  {
    ret += ii->second.numWithMatchingCount(number);
  }
  return ret;
}

//------------------------------------------------------------------
int RadxPersistentClutter::_updateClutterState(const int kstar,
                                               FrequencyCount &F)
{
  int nchange = 0;
  int nclutter = 0;

  // count up changes in clutter value, and update _store internal state
  for (std::map<RadxAzElev, RayClutterInfo>::iterator ii = _store.begin();
       ii!=_store.end(); ++ii)
  {
    nchange += ii->second.updateClutter(kstar, nclutter, F);
  }
  return nchange;
}

//------------------------------------------------------------------
bool RadxPersistentClutter::_process(const time_t t, RadxVol &vol)
{

  // break the volume into rays and process each one
  const vector<RadxRay *> &rays = vol.getRays();
  LOGF(LogMsg::DEBUG_VERBOSE, "Nrays=%d", static_cast<int>(rays.size()));
  for (size_t ii = 0; ii < rays.size(); ii++)
  {
    RadxRay *ray = rays[ii];
    _processRay(t, ray);
  }

  _thread.waitForThreads();

  // virtual method at the end of processing all rays
  return processFinishVolume(t, vol);
}

//------------------------------------------------------------------
void RadxPersistentClutter::_processFirst(const time_t t, const RadxVol &vol)
{

  // break the volume into rays and process each one
  const vector<RadxRay *> &rays = vol.getRays();
  LOGF(LogMsg::DEBUG_VERBOSE, "Nrays=%d", static_cast<int>(rays.size()));
  for (size_t ii = 0; ii < rays.size(); ii++)
  {
    RadxRay *ray = rays[ii];

    // virtual method to pre-process each ray, done first volume only
    preProcessRay(*ray);
  }
}

//------------------------------------------------------------------
void RadxPersistentClutter::_processRay(const time_t &t, const RadxRay *ray)
{

  // create an info pointer
  Info *info = new Info();

  // set the info values
  info->set(t, ray, this);

  int index = 0;
  _thread.thread(index, info);
}

//------------------------------------------------------------------
bool RadxPersistentClutter::_processRayForOutput(RadxRay &ray)
{
  RayxData r;

  // initialization
  RayClutterInfo *h = _initRay(ray, r);
  if (h != NULL)
  {
    // call a virtual method
    return setRayForOutput(h, r, ray);
  }
  else
  {
    return false;
  }
}


//------------------------------------------------------------------
RayClutterInfo *RadxPersistentClutter::_initRay(const RadxRay &ray,
						RayxData &r)
{
  if (!App::retrieveRay(_params.input_field, ray, r))
  {
    return NULL;
  }

  // Point to the matching clutter info for this ray
  double az = ray.getAzimuthDeg();
  double elev = ray.getElevationDeg();
  RayClutterInfo *h = matchingClutterInfo(az, elev);
  if (h == NULL)
  {
    LOGF(LogMsg::WARNING, "No histo match for az=%lf elev=%lf",
	    az, elev);
  }
  else
  {
    LOGF(LogMsg::DEBUG_VERBOSE, "Updating ray az:%lf  elev:%lf", az, elev);
  }
  return h;
}

