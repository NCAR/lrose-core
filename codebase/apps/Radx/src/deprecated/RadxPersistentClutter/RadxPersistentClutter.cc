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

//////////////////////////////////////////////////////////////////////////
// Based on following paper:
// Lakshmanan V., J. Zhang, K. Hondl and C. Langston.
// A Statistical Approach to Mitigating Persistent Clutter in
// Radar Reflectivity Data.
// IEEE Journal of Selected Topics in Applied Earth Observations
// and Remote Sensing, Vol. 5, No. 2, April 2012.
///////////////////////////////////////////////////////////////////////////

#include "RadxPersistentClutter.hh"
#include "RadxPersistentClutterFirstPass.hh"
#include "RadxPersistentClutterSecondPass.hh"
#include "Volume.hh"
#include "Info.hh"
#include <radar/RadxApp.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxVol.hh>
#include <toolsa/LogStream.hh>
#include <toolsa/TaThreadSimple.hh>
#include <algorithm>

//------------------------------------------------------------------
RadxPersistentClutter::RadxPersistentClutter(const Parms &parms, Alg_t type) :
  _parms(parms) , _type(type)
{
  _rayMap = RayxMapping(parms.fixedElevations_n, parms._fixedElevations,
                        parms.azToleranceDegrees,
                        parms.elevToleranceDegrees);
}

//------------------------------------------------------------------
RadxPersistentClutter::~RadxPersistentClutter(void)
{
  RadxApp::algFinish();
}

//------------------------------------------------------------------
bool RadxPersistentClutter::getFloat(double &v) const
{
  return 0.0;
}

//------------------------------------------------------------------
void RadxPersistentClutter::_processForOutput(Volume *vol)
{
  // break the volume into rays and process each one
  const vector<RadxRay *> &rays = vol->getVolRef().getRays();
  for (size_t ii = 0; ii < rays.size(); ii++)
  {
    RadxRay *ray = rays[ii];
    _processRayForOutput(*ray);
  }
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
  if (!RadxApp::retrieveRay(_parms.input_field, ray, r))
  {
    return NULL;
  }

  // Point to the matching clutter info for this ray
  double az = ray.getAzimuthDeg();
  double elev = ray.getElevationDeg();
  RayClutterInfo *h = matchingClutterInfo(az, elev);
  if (h == NULL)
  {
    LOG(DEBUG_VERBOSE) << "No histo match for az,elev=" << az << "," << elev;
  }
  else
  {
    LOG(DEBUG_VERBOSE) << "Updating ray az,elev=" << az << "," << elev;
  }
  return h;
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
RayClutterInfo *RadxPersistentClutter::initRay(const RadxRay &ray,
					       RayxData &r)
{
  if (!RadxApp::retrieveRay(_parms.input_field, ray, r))
  {
    return NULL;
  }

  // Point to the matching clutter info for this ray
  double az = ray.getAzimuthDeg();
  double elev = ray.getElevationDeg();
  RayClutterInfo *h = matchingClutterInfo(az, elev);
  if (h == NULL)
  {
    LOG(DEBUG_VERBOSE) << "No histo match for az,elev= " << az << "," << elev;
  }
  else
  {
    LOG(DEBUG_VERBOSE) << "Updating ray az,elev= " << az << "," << elev;
  }
  return h;
}

