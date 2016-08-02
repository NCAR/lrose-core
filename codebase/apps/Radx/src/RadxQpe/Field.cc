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
 * @file Field.cc
 */

#include "Field.hh"
#include "Geom.hh"
#include "AzMapper.hh"
#include "GateMapper.hh"
#include "BadValue.hh"
#include <Radx/RadxField.hh>
#include <Radx/RadxSweep.hh>
#include <Radx/RadxRay.hh>
#include <cmath>
#include <toolsa/LogMsg.hh>

#define KM_TO_METERS 1000.0

//----------------------------------------------------------------
Field::Field(void) : Grid2d("bad", 1, 1, 0), _isOK(false), _units("bad")
{
}

//----------------------------------------------------------------
Field::Field(const std::string &name, const RadxSweep &sweep,
	     const std::vector<RadxRay *> &rays, const Geom &geom) :
  Grid2d(name, geom.nGate(), geom.nOutputAz(), 0), // missing changed later
  _isOK(true),
  _units("?")
{

  // create mapping from azimuth to ray index
  AzMapper map(sweep, rays);

  // look through fixed output azimuths
  bool first = true;
  for (int ia=0; ia<geom.nOutputAz(); ++ia)
  {
    double a = geom.ithOutputAz(ia);

    // find closest ray
    int ir = map.closestRayIndex(a, geom.deltaOutputAz());
    if (ir < 0)
    {
      // this azimuth is all missing in output
      continue;
    }      

    // point to that ray and pull out geometry stuff
    const RadxRay *ray = rays[ir];
    double r0 = ray->getStartRangeKm()*KM_TO_METERS;
    int nr = ray->getNGates();
    double dr = ray->getGateSpacingKm()*KM_TO_METERS;

    // compare ray geometry to grid geometry and set up the mapper
    GateMapper map(r0, nr, dr, geom);
    if (!map._isOK)
    {
      // bad
      _isOK = false;
    }
    else
    {
      const RadxField *field = ray->getField(_name);
      if (field == NULL)
      {
	_isOK = false;
      }
      else
      {
	if (first)
	{
	  first = false;
	  _units = field->getUnits();
	  double missing = field->getMissingFl32();
	  Grid2d::changeMissingAndData(missing);
	}
	const Radx::fl32 *d = field->getDataFl32();
	addDataAtAzimuth(map, ia, d);
      }
    }
  }
}

//----------------------------------------------------------------
Field::Field(const Params::output_field_t &field, const Geom &geom) :
  Grid2d(field.name, geom.nGate(), geom.nOutputAz(), BadValue::value()), 
  _isOK(true),  _units(field.units)
{
}


//----------------------------------------------------------------
Field::Field(const Params::rainrate_field_t &field, const Geom &geom) :
  Grid2d(field.output_rainrate_name, geom.nGate(), geom.nOutputAz(),
	 BadValue::value()),
  _isOK(true),  _units(field.units)
{
}

//----------------------------------------------------------------
Field::~Field()
{
}

//----------------------------------------------------------------
void Field::print(void) const
{
  Grid2d::print();
}

//----------------------------------------------------------------
void Field::addDataAtAzimuth(const GateMapper &map, int iaz, const fl32 *data)
{
#ifdef CPP11
  for (auto & mi : map)
  {
#else
  for (size_t i=0; i< map.size(); ++i)
  {
    const GateMapper1 &mi = map[i];
#endif
    Grid2d::setValue(mi._grid2dIndex, iaz, data[mi._radxIndex]);
  }
}

//----------------------------------------------------------------
Radx::fl32 *Field::createData(int iaz, int nr) const
{
  if (nr != Grid2d::getNx())
  {
    LOGF(LogMsg::ERROR, "Inconsistent radial dimension %d %d", nr,
	 Grid2d::getNx());
    return NULL;
  }

  Radx::fl32 *ret = new Radx::fl32[nr];
  for (int i=0; i<nr; ++i)
  {
    ret[i] = Grid2d::getValue(i, iaz);
  }
  return ret;
}

//----------------------------------------------------------------
bool Field::isBad(const Field &f)
{
  return (f.nameEquals("bad") && f._units == "bad" &&
	  f.getNx() == 1 && f.getNy() == 1);
}

