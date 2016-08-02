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
 * @file DigitalElevationHandler.cc
 */
#include "DigitalElevationHandler.hh"
#include <BeamBlock/spheroid.h>
#include <toolsa/LogMsg.hh>
using namespace rainfields;
using namespace rainfields::ancilla;


//------------------------------------------------------------------
static spheroid::standard _toSphereStandard(Params::DigitalElevationModel_t t)
{
  std::string s="";
  switch (t)
  {
  case Params::ESRI_I65:
    s = "i65";
    break;
  case Params::ESRI_ANS:
    s = "ans";
    break;
  case Params::ESRI_CLARKE1858:
    s = "clarke1858";
    break;
  case Params::ESRI_GRS80:
    s = "grs80";
    break;
  case Params::ESRI_WGS84:
    s = "wgs84";
    break;
  case Params::ESRI_WGS72:
    s = "wgs72";
    break;
  case Params::INTERNATIONAL1924:
    s = "international924";
    break;
  case Params::AUSTRALIAN_NATIONAL:
    s = "australian_national";
    break;
  default:
    s = "?";
    break;
  }

  return from_string<spheroid::standard>(s);
}

//----------------------------------------------------------------
DigitalElevationHandler::DigitalElevationHandler(const Parms &params) :
  _params(params)
{
}

//----------------------------------------------------------------
DigitalElevationHandler::~DigitalElevationHandler(void)
{
}

//----------------------------------------------------------------
bool DigitalElevationHandler::set(const std::pair<double,double> &sw,
				  const std::pair<double,double> &ne)
{
  spheroid::standard which;

  switch (_params.input_data_format)
  {
  case Params::SHUTTLE_RADAR_TOPOGRAPHY:
    _dem.reset(new digital_elevation_srtm3(_params.input_dem_path));
    break;
  case Params::ESRI_I65:
  case Params::ESRI_ANS:
  case Params::ESRI_CLARKE1858:
  case Params::ESRI_GRS80:
  case Params::ESRI_WGS84:
  case Params::ESRI_WGS72:
  case Params::INTERNATIONAL1924:
  case Params::AUSTRALIAN_NATIONAL:
    which = _toSphereStandard(_params.input_data_format);
    _set(sw, ne, which);
    break;
  default:
    LOG(LogMsg::ERROR, "format Unknown");
    exit(1);
  }

  return true;
}

//----------------------------------------------------------------
latlonalt DigitalElevationHandler::radarOrigin(const latlonalt &radar) const
{
  spheroid wgs84{spheroid::standard::wgs84};
  latlonalt ret =
    _dem->reference_spheroid().ecefxyz_to_latlon(wgs84.latlon_to_ecefxyz(radar));

  // warn if the stated site location is less than 3 meters above the DEM location of that site
  LOGF(LogMsg::DEBUG, "Altitude (native spheroid): %lf",
       _params.radar_location.altitudeKm*1000.0);
  LOGF(LogMsg::DEBUG, "Altitude (DEM spheroid): %lf", ret.alt);
  LOGF(LogMsg::DEBUG, "DEM:  %lf", _dem->lookup(ret));
  if (ret.alt < _dem->lookup(ret) + 3.0)
  {
    LOG(LogMsg::WARNING, 
	"site altitude is less than 3 meters above DEM altitude");
  }
  return ret;
}

//----------------------------------------------------------------
void
DigitalElevationHandler::determine_dem_segment_peak(const latlon& origin,
						    angle bearing,
						    real min_range,
						    real max_range,
						    real& peak_ground_range,
						    real& peak_altitude,
						    size_t bin_samples
						    ) const
{
  const auto delta_range = (max_range - min_range) / bin_samples;
  const auto& sphere = _dem->reference_spheroid();

  peak_altitude = -10000.0_r;

  // loop over our ground range 
  for (size_t i = 0; i < bin_samples; ++i)
  {
    // determine range at this step
    auto range = min_range + i * delta_range;

    // convert range bearing into latlon
    auto loc = sphere.bearing_range_to_latlon(origin, bearing, range);

    // get height from DEM
    auto altitude = _dem->lookup(loc);

    // are we a new peak? (nan's will always fail this test)
    if (altitude > peak_altitude)
    {
      peak_altitude = altitude;
      peak_ground_range = range;
    }
  }
}

//----------------------------------------------------------------
double
DigitalElevationHandler::getElevation(const rainfields::latlon& loc) const
{
  return _dem->lookup(loc);
}

//----------------------------------------------------------------
void DigitalElevationHandler::_set(const std::pair<double,double> &sw,
				   const std::pair<double,double> &ne,
				   spheroid::standard which)
{
  angle a0, a1;
  a0.set_degrees(sw.first);
  a1.set_degrees(sw.second);
  latlon lsw(a0, a1);

  a0.set_degrees(ne.first);
  a1.set_degrees(ne.second);
  latlon lne(a0, a1);

  _dem.reset(new digital_elevation_esri(_params.input_dem_path, lsw, lne,
					spheroid(which)));
}
