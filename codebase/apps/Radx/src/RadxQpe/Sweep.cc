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
#include <toolsa/copyright.h>

/**
 * @file Sweep.cc
 */

#include "Sweep.hh"
#include <Radx/RadxSweep.hh>
#include <Radx/RadxField.hh>
#include <Radx/RadxRay.hh>
#include <toolsa/LogMsg.hh>
#include <cmath>

//----------------------------------------------------------------
Sweep::Sweep(double elev) : _isOK(true), _elev(elev), _bad()
{

}

//----------------------------------------------------------------
Sweep::Sweep(double elev, const RadxSweep &sweep,
	     const std::vector<RadxRay *> &rays, const Geom &geom) :
  _elev(elev), _bad()
{
  _isOK = fill(sweep, rays, geom);
}  

//----------------------------------------------------------------
Sweep::Sweep(double elv, const Geom &geom, const Parms &parms) :
  _isOK(true), _elev(elv), _bad()
{
  // setup the grids
  for (int i=0; i<parms.output_fields_n; ++i)
  {
    _grids.push_back(Field(parms._output_fields[i], geom));
  }
  for (int i=0; i<parms.rainrate_fields_n; ++i)
  {
    _grids.push_back(Field(parms._rainrate_fields[i], geom));
  }
}


//----------------------------------------------------------------
Sweep::~Sweep()
{
}

//----------------------------------------------------------------
bool Sweep::fill(const RadxSweep &sweep,
		 const std::vector<RadxRay *> &rays, const Geom &geom)
{
  _grids.clear();
  // check elevation angle 
  if (sweep.getFixedAngleDeg() != _elev)
  {
    LOGF(LogMsg::ERROR, "Incorrect sweep fill elevs disagree %lf %lf",
	 sweep.getFixedAngleDeg(), _elev);
    return false;
  }

  // get the first ray to find field names
  const RadxRay *ray = rays[sweep.getStartRayIndex()];
  const vector<RadxField *> fields = ray->getFields();
  bool ret = true;
  for (size_t i=0; i<fields.size(); ++i)
  {
    Field f(fields[i]->getName(), sweep, rays, geom);
    if (f._isOK)
    {
      _grids.push_back(f);
    }
    else
    {
      ret = false;
    }
  }
  return ret;
}  

//----------------------------------------------------------------
void Sweep::setRadarHtKm(const Params &params, double val)
{
  _beamHt.setInstrumentHtKm(val);
  if (params.override_standard_pseudo_earth_radius) {
    _beamHt.setPseudoRadiusRatio(params.pseudo_earth_radius_ratio);
  }
}

//----------------------------------------------------------------
double Sweep::getBeamHtKmMsl(int igt, const Geom &geom, double &rangeKm) const
{
  double rangeMeters = geom.ithGtMeters(igt);
  rangeKm = rangeMeters / 1000.0;
  return _beamHt.computeHtKm(_elev, rangeKm);
}

//----------------------------------------------------------------
void Sweep::print(void) const
{
  printf("ELEVATION:%.4lf\n", _elev);
  for (size_t i=0; i<_grids.size(); ++i)
  {
    _grids[i].print();
  }
}

//----------------------------------------------------------------
bool Sweep::value(int igt0, int iaz0, const std::string &fieldName,
		  double &v) const
{
  const Field &g = operator[](fieldName);
  if (Field::isBad(g))
  {
    return false;
  }
  else
  {
    return g.getValue(igt0, iaz0, v);
  }
}

//----------------------------------------------------------------
bool Sweep::setValue(const std::string &fieldName, int igt, int iaz, double v)
{
  Field &g = operator[](fieldName);
  if (Field::isBad(g))
  {
    return false;
  }
  else
  {
    g.setValue(igt, iaz, v);
    return true;
  }
}

//----------------------------------------------------------------
Field& Sweep::operator[](const std::string &name)
{
  for (size_t i=0; i<_grids.size(); ++i)
  {
    if (_grids[i].nameEquals(name))
    {
      return _grids[i];
    }
  }
  // LOGF(LogMsg::ERROR, "field %s never found", name.c_str());
  return _bad;
}

//----------------------------------------------------------------
const Field& Sweep::operator[](const std::string &name) const
{
  for (size_t i=0; i<_grids.size(); ++i)
  {
    if (_grids[i].nameEquals(name))
    {
      return _grids[i];
    }
  }
  // LOGF(LogMsg::ERROR, "field %s never found", name.c_str());
  return _bad;
}

