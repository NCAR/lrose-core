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
 * @file Data.cc
 */
#include "Data.hh"
#include <dsdata/DsUrlTrigger.hh>
#include <Radx/RadxTimeList.hh>
#include <Radx/RadxFile.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxSweep.hh>
#include <didss/DataFileNames.hh>
#include <toolsa/LogMsg.hh>
#include <toolsa/DateTime.hh>
#include <cmath>
#include <algorithm>

#define KM_TO_METERS 1000.0

//------------------------------------------------------------------
static string nameWithoutPath(const string &name)
{
  string::size_type i = name.find_last_of("/");
  string ret = name.substr(i+1);
  return ret;
}

//----------------------------------------------------------------
Data::Data(const Parms &params) : Geom(), _dataIsOK(true), _params(params)
{
}

//----------------------------------------------------------------
Data::~Data(void)
{
}

//----------------------------------------------------------------
bool Data::mismatch(const Data &data) const
{
  bool ret = false;
  // if (fabs(data._lat - _lat) > 1.0e-3 || fabs(data._lon - _lon) > 1.0e-3)
  if (fabs(data._lat - _lat) > 2.0e-3 || fabs(data._lon - _lon) > 2.0e-3)
  {
    LOGF(LogMsg::ERROR,
	 "Mismatch in lat lon (%.4lf,%.4lf) (%.4lf,%.4lf)",
	 _lat, _lon, data._lat, data._lon);
    ret = true;
  }
  return ret;
}

//----------------------------------------------------------------
bool Data::readLite(const std::string &path,
		    const std::vector<std::string> &fields)
{
  _sweeps.clear();

  RadxFile primaryFile;
  primaryFile.clearRead();
  for (size_t i=0; i<fields.size(); ++i)
  {
    primaryFile.addReadField(fields[i]);
  }
  return _readLite(primaryFile, path);
}

//----------------------------------------------------------------
bool Data::read(const std::string &path,
		const std::vector<std::string> &fields)
{
  if (readLite(path, fields))
  {
    const vector<RadxSweep *> sw = _vol.getSweeps();
    const vector<RadxRay *> rays = _vol.getRays();
    // for each sweep
    double ret = true;
    for (size_t i=0; i<sw.size(); ++i)
    {
      // Augment the correct sweep 
      if (!_sweeps[i].fill(*sw[i], rays, *this))
      {
	ret = false;
      }
    }  
    return ret;
  }
  else
  {
    return false;
  }
}

//----------------------------------------------------------------
bool Data::read(const time_t &t,
		const std::vector<std::string> &fields)
{
  RadxTimeList tlist;
  tlist.setDir(_params.data_url);
  tlist.setModeFirstBefore(t, _params.volume_time_margin_seconds);

  if (tlist.compile())
  {
    LOGF(LogMsg::ERROR, "Cannot compile time list, dir: %s", _params.data_url);
    LOG(LogMsg::ERROR, tlist.getErrStr().c_str());
    return false;
  }
  vector<string> paths = tlist.getPathList();
  if (paths.empty())
  {
    LOGF(LogMsg::ERROR,
	 "No input data in time range specified, which is %s to %s",
	 DateTime::strn(t-_params.volume_time_margin_seconds).c_str(),
	 DateTime::strn(t).c_str());
    return false;
  }

  string path = *(paths.rbegin());
  return read(path, fields);
}


//----------------------------------------------------------------
std::vector<double> Data::getElev(void) const
{
  std::vector<double> ret;
  for (size_t i=0; i<_sweeps.size(); ++i)
  {
    ret.push_back(_sweeps[i].elev());
  }
  return ret;
}

//----------------------------------------------------------------
bool Data::_readLite(RadxFile &primaryFile, const std::string &path)
{
  string name = nameWithoutPath(path);
  LOGF(LogMsg::DEBUG, "full name: %s", path.c_str());
  LOGF(LogMsg::DEBUG, "name without path: %s", name.c_str());
  
  // primaryFile.setReadFixedAngleLimits(_minElev, _maxElev);
  if (primaryFile.readFromPath(path, _vol))
  {
    LOGF(LogMsg::ERROR, "Cannot read in file: %s", name.c_str());
    return false;
  }
  time_t tloc = _vol.getEndTimeSecs();
  bool dateOnly;
  if (DataFileNames::getDataTime(path, tloc, dateOnly))
  {
    LOGF(LogMsg::ERROR, "Cannot get time from file path: %s", name.c_str());
    return false;
  }

  LOGF(LogMsg::DEBUG_VERBOSE, "-------Read file %s ----------", 
       RadxTime::strm(tloc).c_str());

  _vol.convertToFl32();
  _sweeps.clear();
  const vector<RadxSweep *> sw = _vol.getSweeps();
  const vector<RadxRay *> rays = _vol.getRays();
  for (size_t i=0; i<sw.size(); ++i)
  {
    double a = sw[i]->getFixedAngleDeg();
    _sweeps.push_back(Sweep(a));
    _sweeps[_sweeps.size()-1].setRadarHtKm(_params, _vol.getAltitudeKm());
  }
  _lat = _vol.getLatitudeDeg();
  _lon = _vol.getLongitudeDeg();

  vector<double> az;
  int nr=0;
  double dr=0, r0=0;
  // build up geometry using lowest sweep
  bool first = true;
  RadxSweep *r = sw[0];
  bool isIndexed = r->getRaysAreIndexed();
  double angleResDeg = r->getAngleResDeg();
  for (int ir = r->getStartRayIndex(); 
       ir <= static_cast<int>(r->getEndRayIndex()); ++ir)
  {
    double a = rays[ir]->getAzimuthDeg();
    az.push_back(a);
    if (first)
    {
      first = false;
      nr = rays[ir]->getNGates();
      dr = rays[ir]->getGateSpacingKm()*KM_TO_METERS;
      r0 = rays[ir]->getStartRangeKm()*KM_TO_METERS;
    }
    else
    {
      int ng = rays[ir]->getNGates();
      if (ng != nr)
      {
    	LOGF(LogMsg::WARNING, "Uneven number of gates %d to %d", nr, ng);
      }
      double r0i = rays[ir]->getStartRangeKm()*KM_TO_METERS;
      if (r0i != r0)
      {
    	LOGF(LogMsg::WARNING, "Uneven starting range %lf to %lf", r0, r0i);
      }
    }
  }

  sort(az.begin(), az.end());
  Geom::operator=(Geom(isIndexed, angleResDeg, az, nr, r0, dr,
		       _params.azimuthal_resolution_degrees));
  return true;
}

