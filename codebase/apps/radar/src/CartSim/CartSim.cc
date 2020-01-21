// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// © University Corporation for Atmospheric Research (UCAR) 2009-2010. 
// All rights reserved.  The Government's right to use this data and/or 
// software (the "Work") is restricted, per the terms of Cooperative 
// Agreement (ATM (AGS)-0753581 10/1/08) between UCAR and the National 
// Science Foundation, to a "nonexclusive, nontransferable, irrevocable, 
// royalty-free license to exercise or have exercised for or on behalf of 
// the U.S. throughout the world all the exclusive rights provided by 
// copyrights.  Such license, however, does not include the right to sell 
// copies or phonorecords of the copyrighted works to the public."   The 
// Work is provided "AS IS" and without warranty of any kind.  UCAR 
// EXPRESSLY DISCLAIMS ALL OTHER WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
// ANY IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
// PURPOSE.  
//  
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
#include <toolsa/copyright.h>
/**
 * @file CartSim.cc
 */

#include "CartSim.hh"
#include "DataGrid.hh"
#include <toolsa/DateTime.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxProj.hh>

//----------------------------------------------------------------
CartSim::CartSim(const Params &parms) : _parms(parms),_ambient(parms)
{
  // Set time to start time of simulation
  DateTime dt(parms._start_time[0], parms._start_time[1], parms._start_time[2], 
	      parms._start_time[3], parms._start_time[4], parms._start_time[5]);
  _t0 = dt.utime();

  for (int i=0; i<_parms.cartesian_speck_n; ++i)
  {
    _cspeck.push_back(CartesianSpeck(_parms, _parms._cartesian_speck[i]));
  }
  for (int i=0; i<_parms.deviant_ray_n; ++i)
  {
    _dray.push_back(DeviantRay(_parms, _parms._deviant_ray[i]));
  }
  for (int i=0; i<_parms.gust_front_n; ++i)
  {
    _gf.push_back(GustFront(_parms, _parms._gust_front[i]));
  }
  for (int i=0; i<_parms.microburst_n; ++i)
  {
    _mb.push_back(Microburst(_parms, _parms._microburst[i]));
  }
  for (int i=0; i<_parms.polar_speck_n; ++i)
  {
    _pspeck.push_back(PolarSpeck(_parms, _parms._polar_speck[i]));
  }
  for (int i=0; i<_parms.storm_n; ++i)
  {
    _storm.push_back(Storm(_parms, _parms._storm[i]));
  }
  for (int i=0; i<_parms.turbulence_n; ++i)
  {
    _turb.push_back(Turbulence(_parms, _parms._turbulence[i]));
  }
  for (int i=0; i<_parms.vel_circle_n; ++i)
  {
    _vcircle.push_back(VelCircle(_parms, _parms._vel_circle[i]));
  }
  for (int i=0; i<_parms.clutter_region_n; ++i)
  {
    _clutter.push_back(Clutter(_parms, _parms._clutter_region[i]));
  }
}

//----------------------------------------------------------------
CartSim::~CartSim()
{
}

// //----------------------------------------------------------------
// void CartSim::updateInfo(const int dt, time_t &currentTime) const
// {
//   currentTime = _t0 + dt;
//   printf("Processing time %s\n", DateTime::strn(currentTime).c_str());
// }

//----------------------------------------------------------------
void CartSim::setTime(const int dt)
{
  _time = _t0 + dt;
  printf("Processing time %s\n", DateTime::strn(_time).c_str());
}

//----------------------------------------------------------------
void CartSim::process(const Xyz &loc, Data &data)
{
  int seconds = _time - _t0;

  // start with ambient values
  data = Data(_ambient);

  // add in objects in this order
  _addMb(seconds, loc, data);
  _addGf(seconds, loc, data);
  _addStorm(seconds, loc, data);
  _addTurb(seconds, loc, data);

  // add noise after main objects
  data.addNoise();

  // these weird non-weather things go in last
  _addDeviantRay(seconds, loc, data);
  _addSpecks(seconds, loc, data);
  _addVelCircle(seconds, loc, data);
  _addClutter(seconds, loc, data);
}

//----------------------------------------------------------------
void CartSim::_addDeviantRay(int seconds, const Xyz &loc, Data &data)
{
  for (size_t i=0; i<_dray.size(); ++i)
  {
    _dray[i].addToData(seconds, loc, data);
  }
}

//----------------------------------------------------------------
void CartSim::_addGf(int seconds, const Xyz &loc, Data &data)
{
  for (size_t i=0; i<_gf.size(); ++i)
  {
    _gf[i].addToData(seconds, loc, data);
  }
}

//----------------------------------------------------------------
void CartSim::_addMb(int seconds, const Xyz &loc, Data &data)
{
  for (size_t i=0; i<_mb.size(); ++i)
  {
    _mb[i].addToData(seconds, loc, data);
  }
}

//----------------------------------------------------------------
void CartSim::_addSpecks(int seconds, const Xyz &loc, Data &data)
{
  for (size_t i=0; i<_cspeck.size(); ++i)
  {
    _cspeck[i].addToData(seconds, loc, data);
  }
  for (size_t i=0; i<_pspeck.size(); ++i)
  {
    _pspeck[i].addToData(seconds, loc, data);
  }
}

//----------------------------------------------------------------
void CartSim::_addStorm(int seconds, const Xyz &loc, Data &data)
{
  for (size_t i=0; i<_storm.size(); ++i)
  {
    _storm[i].addToData(seconds, loc, data);
  }
}

//----------------------------------------------------------------
void CartSim::_addTurb(int seconds, const Xyz &loc, Data &data)
{
  for (size_t i=0; i<_turb.size(); ++i)
  {
    _turb[i].addToData(seconds, loc, data);
  }
}

//----------------------------------------------------------------
void CartSim::_addVelCircle(int seconds, const Xyz &loc, Data &data)
{
  for (size_t i=0; i<_vcircle.size(); ++i)
  {
    _vcircle[i].addToData(seconds, loc, data);
  }
}

//----------------------------------------------------------------
void CartSim::_addClutter(int seconds, const Xyz &loc, Data &data)
{
  for (size_t i=0; i<_clutter.size(); ++i)
  {
    _clutter[i].addToData(seconds, loc, data);
  }
}
