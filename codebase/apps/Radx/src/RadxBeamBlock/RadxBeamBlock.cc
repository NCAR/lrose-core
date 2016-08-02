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
 * @file RadxBeamBlock.cc
 */

#include "RadxBeamBlock.hh"
#include <euclid/Grid2d.hh>
#include <euclid/GridAlgs.hh>
#include <toolsa/LogMsg.hh>

using namespace rainfields;
using namespace ancilla;

//------------------------------------------------------------------
RadxBeamBlock::RadxBeamBlock(const Parms &params) : _params(params),
						    _data(params),
						    _dem(params)
{

}

//------------------------------------------------------------------
RadxBeamBlock::~RadxBeamBlock()
{
}

//------------------------------------------------------------------
int RadxBeamBlock::Run(void)
{

  // pull out extremes in latitude/longitude, which are needed for setup
  // in some cases
  pair<double, double> sw, ne;
  _params.latlonExtrema(sw, ne);

  // set up the digital elevation object
  if (!_dem.set(sw, ne))
  {
    return 1;
  }

  angle a0, a1;
  a0.set_degrees(_params.radar_location.latitudeDeg);
  a1.set_degrees(_params.radar_location.longitudeDeg);

  double elevation;
  if (_params.do_lookup_radar_altitude)
  {
    latlon ll(a0, a1);
    elevation = _dem.getElevation(ll);
  }
  else
  {
    elevation = _params.radar_location.altitudeKm*1000.0;
  }
  // latlonalt origin(a0, a1, _params.radar_location.altitudeKm*1000.0);
  latlonalt origin(a0, a1, elevation);

  // convert the site location of the volume into the native spheroid of the DEM
  origin = _dem.radarOrigin(origin); 

  // setup our beam power models
  angle height, width;
  height.set_degrees(_params.vert_beam_width_deg);
  width.set_degrees(_params.horiz_beam_width_deg);
  beam_power power_model(width, height);

  // process each scan
  bool short_circuit = false;
  for (auto &scan : _data)
  {
    _processScan(scan, power_model, origin, short_circuit);
  }
  _data.finish();
  return 0;
}

//------------------------------------------------------------------
bool RadxBeamBlock::_processScan(ScanHandler &scan,
				 const beam_power &power_model,
				 latlonalt origin, bool &short_circuit)
{
  angle elevAngle = scan.elevation();
  LOGF(LogMsg::DEBUG, "  occluding tilt: %lf", scan.elevDegrees());

  beam_propagation bProp(elevAngle, _params.radar_location.altitudeKm*1000.0);
  angle beam_width_v, beam_width_h, angle_width;
  beam_width_v.set_degrees(3*_params.vert_beam_width_deg);
  beam_width_h.set_degrees(3*_params.horiz_beam_width_deg +
			   _params.azimuths.delta);
  angle_width.set_degrees(_params.azimuths.delta);
  beam_power_cross_section csec{power_model, angle_width,
      static_cast<size_t>(_params.num_elev_subsample),
      static_cast<size_t>(_params.num_gate_subsample),
      beam_width_v, beam_width_h};
  csec.make_vertical_integration();

  // // shortcut if we've already found a tilt with no occlusion...
  // if (short_circuit)
  // {
  //   _data.insert(scan);
  //   return true;
  // }

  for (auto &ray : scan)
  {
    _processBeam(ray, origin, bProp, csec);
  }

// #if 0
//   // if this tilt was not occluded at all, tell remaining ones to short-circuit
//   short_circuit = true;
//   for (size_t i = 0; i < outb.size(); ++i)
//   {
//     if (outb.data()[i] > 0.0_r)
//     {
//       short_circuit = false;
//       break;
//     }
//   }
//   if (short_circuit)
//     trace::log() << "  no occlusion detected - shortcut remaining tilts";
// #endif

  return true;
}


//------------------------------------------------------------------
void RadxBeamBlock::_processBeam(RayHandler &ray, latlonalt origin, 
				 const beam_propagation &bProp,
				 const beam_power_cross_section &csec)

{
  angle azAngle = ray.azimuth();
  angle elevAngle = ray.elev();

  LOGF(LogMsg::DEBUG, "  processing beam %lf", ray.azDegrees());

  // subsample each azimuth based on the number of horizontal cells in our
  // cross section
  for (size_t iray = 0; iray < csec.cols(); ++iray)
  {
    const angle bearing = azAngle + csec.offset_azimuth(iray);
    angle max_ray_theta = -90.0_deg;  // updated as we go

    // walk out along our ray, keeping track of the total power loss at each bin
    for (auto & gate : ray)
    {
      _processGate(gate, elevAngle, iray, origin, bProp, bearing, csec,
		   max_ray_theta);
    }
  }
}

//------------------------------------------------------------------
void RadxBeamBlock::_processGate(GateHandler &gate, angle elevAngle,
				 size_t iray, latlonalt origin, 
				 const beam_propagation &bProp, angle bearing,
				 const beam_power_cross_section &csec,
				 angle &max_ray_theta)
{
  double gateMeters = gate.meters();

  // get maximum height of DEM along ray in segment bounded by this bin
  real peak_ground_range = 0.0_r;
  real peak_altitude;
  real progressive_loss = 0.0_r;
  _dem.determine_dem_segment_peak(origin, bearing, gateMeters,
				  gateMeters + _params.gates.delta*1000.0,
				  peak_ground_range, peak_altitude,
				  _params.num_bin_subsample);

  // if DEM gave us no valid values we can fail this condition
  // this is usually due to sea regions not being included in the DEM
  if (peak_ground_range > 0.0_r)
  {
    _adjustValues(iray, bProp, peak_ground_range, peak_altitude, elevAngle,
		  csec, max_ray_theta, progressive_loss);

    // update the highest peak observed for the bin
    gate.adjustPeak(peak_altitude);
  }

  // add the loss in this ray,bin to the gate,bin total
  gate.incrementLoss(progressive_loss);
}

//------------------------------------------------------------------
void RadxBeamBlock::_adjustValues(size_t ray, const beam_propagation &bProp,
				  real peak_ground_range, real peak_altitude,
				  angle elevAngle,
				  const beam_power_cross_section &csec,
				  angle &max_ray_theta, real &progressive_loss)
{
  // convert geometric height into an elevation above radar horizon
  angle max_bin_theta = 
    bProp.required_elevation_angle(peak_ground_range, peak_altitude) -
    elevAngle;
	      
  // TODO apply part of fractional loss to this bin based on distance through
  //      bin where peak was found?

  // if we've got a new highest obstruction, then we need to lookup the new 
  // progressive fractional loss for this ray from the cross-section model
  if (max_bin_theta > max_ray_theta)
  {
    max_ray_theta = max_bin_theta;
              
    // what is the closest vertical bin
    const angle csec_delta = csec.height() / csec.rows();
    const real csec_y_offset = csec.rows() / 2.0_r;
    int y = std::floor(max_ray_theta / csec_delta + csec_y_offset);
    if (y >= static_cast<int>(csec.rows()))
    {
      progressive_loss = csec.power(csec.rows() - 1, ray);
    }
    else if (y >= 0)
    {
      progressive_loss = csec.power(y, ray);
    }
    else
    {
      ; // y < 0 - no loss - do nothing
    }

#if 0
    trace::log() 
      << bin 
      << " dir " << bearing
      << " rngl " << scan.bin_ground_range(bin)
      << " rngh " << scan.bin_ground_range(bin + 1)
      << " rng " << peak_ground_range
      << " alt " << peak_altitude
      << " ele " << scan.beam_model().required_elevation_angle(peak_ground_range, peak_altitude)
      << " dlt " << max_bin_theta
      << " max " << max_ray_theta
      << " y " << y
      << " loss " << progressive_loss;
#endif
  }
}

//------------------------------------------------------------------
int RadxBeamBlock::Write(void)
{
  return _data.write();
}


