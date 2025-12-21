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
 * @file Out.cc
 */
#include "Out.hh"
#include "BadValue.hh"
#include "VertData.hh"
#include "VertPrecipData.hh"
#include "Sweep.hh"
#include "Pid.hh"
#include "Parms.hh"

//----------------------------------------------------------------
Out::Out(const VertData &vert, const Parms &parms) :
        _index(-1),
        _elevDeg(BadValue::value()),
        _heightKm(BadValue::value()),
        _rangeKm(BadValue::value()),
        _pid(BadValue::value()),
        _nblock(0),
	_pcappi(0),
        _nlow_snr(0),
        _nclutter(0),
	_peak(0)
{
  for (size_t i=0; i<vert.size(); ++i)
  {
    if (_evaluate(vert[i], parms))
    {
      _index = i;
      break;
    }
  }
}

//----------------------------------------------------------------
Out::~Out(void)
{
}

//----------------------------------------------------------------
void Out::store(int igt, int iaz, const Parms &parms, Sweep &out) const
{
  string name;

  if (parms.matchingOutput(Params::HEIGHT, name))
  {
    out.setValue(name, igt, iaz, _heightKm);
  }
  if (parms.matchingOutput(Params::ELEVATION, name))
  {
    out.setValue(name, igt, iaz, _elevDeg);
  }
  if (parms.matchingOutput(Params::RANGE, name))
  {
    out.setValue(name, igt, iaz, _rangeKm);
  }
  if (parms.matchingOutput(Params::PID, name))
  {
    out.setValue(name, igt, iaz, _pid);
  }
  if (parms.matchingOutput(Params::NUM_BLOCKED, name))
  {
    out.setValue(name, igt, iaz, _nblock);
  }
  if (parms.matchingOutput(Params::NUM_LOW_SNR, name))
  {
    out.setValue(name, igt, iaz, _nlow_snr);
  }
  if (parms.matchingOutput(Params::NUM_CLUTTER, name))
  {
    out.setValue(name, igt, iaz, _nclutter);
  }
  if (parms.matchingOutput(Params::MASK, name))
  {
    out.setValue(name, igt, iaz, 1.0);
  }
  if (parms.matchingOutput(Params::PCAPPI, name))
  {
    out.setValue(name, igt, iaz, _pcappi);
  }
}

//----------------------------------------------------------------
void Out::storePrecip(const std::string &name, const VertPrecipData &pdata,
		      int igt, int iaz, const Parms &parms, Sweep &out) const
{
  int pindex = pdata.getIndex();
  double v = BadValue::value();
  if (_index >= 0)
  {
    //printf("Range of Value - %f, Max Rake - %f\n",_rangeKm,parms.max_range_km);
    if (_rangeKm <=  parms.max_range_km) 
    {
      v = pdata[_index];
      if (v < parms.min_valid_precip_rate)
      {
	v = BadValue::value();
      }
    }
  }
    
  out.setValue(parms.ithOutputRateName(pindex), igt, iaz, v);
}

//----------------------------------------------------------------
bool Out::_evaluate(const VertData1 &v, const Parms &parms)
{

  // always save out range
  _rangeKm = v._rangeKm;
  if (v._peak < 0)
    _peak = 0.0;
  else
    _peak = v._peak/1000.0;

  if (v._hasSnr)
  {
    if (v._snr == BadValue::value())
    {
      // move up
      ++_nlow_snr;
      return false;
    }
    if (v._snr < parms.min_SNR)
    {
      // move up
      ++_nlow_snr;
      return false;
    }
  }
  if (v._beamE > parms.max_beam_block_fraction)
  {
    // blocked, move up
    ++_nblock;
    return false;
  }
  if ( v._heightKm < (_peak + parms.min_beam_height_km_msl)) 	
  {
    //#printf("Beam Fraction - %f, Terrain Height - %f, Min PCAPPI height - %f, Beam Height - %f\n", v._beamE,v._peak, parms.min_beam_height_km_msl,v._heightKm);
    ++_pcappi;
    return false;
  }
  if (v._heightKm > (_peak + parms.max_beam_height_km_msl))
  {
    //printf("Beam Fraction - %f, Terrain Height - %f, Max PCAPPI height - %f, Beam Height - %f\n", v._beamE,v._peak, parms.max_beam_height_km_msl,v._heightKm);
    ++_pcappi;
    return false;
  }
  
  if (v._pid == BadValue::value())
  {
    // move up
    return false;
  }	  
  switch ((int)v._pid)
  {
  case Ground_Clutter:
  case Second_trip:
  case Flying_Insects:
    ++_nclutter;
    return false;
  case Saturated_receiver:
  case Drizzle:
  case Light_Rain:
  case Moderate_Rain:
  case Heavy_Rain:
  case Hail:
  case Rain_Hail_Mixture:
  case Graupel_Small_Hail:
  case Graupel_Rain:
  case Cloud:
  case Dry_Snow:
  case Wet_Snow:
  case Ice_Crystals:
  case Irreg_Ice_Crystals:
  case Supercooled_Liquid_Droplets:
    _heightKm = v._heightKm;
    _elevDeg = v._elevDeg;
    _pid = v._pid;
    return true;
  default:
    break;
  }
  return false;
}
  
