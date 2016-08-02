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
/////////////////////////////////////////////////////////////
// Beam.cc
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2002
//
///////////////////////////////////////////////////////////////

#include "Beam.hh"
#include <toolsa/mem.h>
#include <toolsa/str.h>
#include <toolsa/DateTime.hh>
#include <dataport/bigend.h>
#include <cmath>
using namespace std;

// constructor

Beam::Beam(const string &prog_name,
	   const Params &params) :
  _progName(prog_name),
  _params(params)
{

  _tiltIndex = -1;
  _tiltNum = -99;
  _targetElev = 0;
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    record.setDebug(true);
  }

}

Beam::~Beam()
{

}

////////////////////////////////////
// load UF structs and derived data

int Beam::load(const void *buf, const int &nbytes)

{

  if (record.disassemble(buf, nbytes)) {
    if (_params.override_decode_errors_on_read) {
      cerr << "-->> WARNING - overriding decode error <<--" << endl;
    } else {
      cerr << "ERROR = Beam::load" << endl;
      cerr << "  Cannot load beam from raw UF data." << endl;
      return -1;
    }
  }

  if (_params.set_time_to_current) {
    record.beamTime = std::time(NULL);
  } else {
    int year = record.manHdr.year;
    if (year < 1900) {
      if (year < 70) {
	year += 2000;
      } else {
	year += 1900;
      }
    }
    DateTime beamTime(year,
		      record.manHdr.month,
		      record.manHdr.day,
		      record.manHdr.hour,
		      record.manHdr.minute,
		      record.manHdr.second);
    record.beamTime = beamTime.utime();
  }
  
  if (_params.check_gate_geom) {
    if (fabs(record.startRange - _params.target_start_range) > 0.001) {
      return -1;
    }
    if (fabs(record.gateSpacing - _params.target_gate_spacing) > 0.001) {
      return -1;
    }
  }
  
  return 0;

}

////////////////////
// load radar params

void Beam::loadRadarParams(DsRadarParams &rParams)

{

  double missing = -9999.0;

  rParams.radarId = _params.radar.radar_id;
  rParams.radarType = DS_RADAR_GROUND_TYPE;
  rParams.numFields = _params.output_fields_n;
  if (_params.set_ngates_out) {
    rParams.numGates = _params.ngates_out;
  } else {
    rParams.numGates = record.maxGates;
  }
  rParams.samplesPerBeam = record.numSamples;
  rParams.scanType = _params.scan_type_id;
  rParams.scanMode = record.manHdr.sweep_mode;
  
  if (_params.radar.polarization == Params::USE_FILE_VALUE) {
    rParams.polarization = record.polarizationCode;
  } else {
    rParams.polarization = _params.radar.polarization;
  }
  
  if (_params.radar.altitude == missing) {
    rParams.altitude = record.manHdr.antenna_height / 1000.0;
  } else {
    rParams.altitude = _params.radar.altitude;
  }

  if (_params.radar.latitude == missing) {
    rParams.latitude =
      (double) record.manHdr.lat_degrees +
      (record.manHdr.lat_minutes / 60.0) +
      (record.manHdr.lat_seconds / (64.0 * 3600.0));
  } else {
    rParams.latitude = _params.radar.latitude;
  }

  if (_params.radar.longitude == missing) {
    rParams.longitude =
      (double) record.manHdr.lon_degrees +
      (record.manHdr.lon_minutes / 60.0) +
      (record.manHdr.lon_seconds / (64.0 * 3600.0));
  } else {
    rParams.longitude = _params.radar.longitude;
  }

  if (_params.radar.start_range == missing) {
    rParams.startRange = record.startRange;
  } else {
    rParams.startRange = _params.radar.start_range;
  }
  
  if (_params.radar.gate_spacing == missing) {
    rParams.gateSpacing = record.gateSpacing;
  } else {
    rParams.gateSpacing = _params.radar.gate_spacing;
  }

  if (_params.radar.beam_width == missing) {
    rParams.horizBeamWidth = record.horizBeamWidth;
  } else {
    rParams.horizBeamWidth = _params.radar.beam_width;
  }

  if (_params.radar.beam_width == missing) {
    rParams.vertBeamWidth = record.vertBeamWidth;
  } else {
    rParams.vertBeamWidth = _params.radar.beam_width;
  }

  if (_params.radar.radar_constant == missing) {
    rParams.radarConstant =record. radarConstant;
  } else {
    rParams.radarConstant = _params.radar.radar_constant;
  }

  if (_params.radar.xmit_peak_pwr == missing) {
    rParams.xmitPeakPower = record.peakPower;
  } else {
    rParams.xmitPeakPower = _params.radar.xmit_peak_pwr;
  }

  rParams.receiverMds = _params.radar.receiver_mds;

  if (_params.radar.receiver_gain == missing) {
    rParams.receiverGain = record.receiverGain;
  } else {
    rParams.receiverGain = _params.radar.receiver_gain;
  }

  if (_params.radar.antenna_gain == missing) {
    rParams.antennaGain = record.antennaGain;
  } else {
    rParams.antennaGain = _params.radar.antenna_gain;
  }

  rParams.systemGain = _params.radar.system_gain;

  if (_params.radar.pulse_width == missing) {
    rParams.pulseWidth = record.pulseWidth;
  } else {
    rParams.pulseWidth = _params.radar.pulse_width;
  }

  if (_params.radar.wavelength == missing) {
    rParams.wavelength = record.wavelength;
  } else {
    rParams.wavelength = _params.radar.wavelength;
  }

  if (_params.radar.prf == missing) {
    rParams.pulseRepFreq = record.prf;
  } else {
    rParams.pulseRepFreq = _params.radar.prf;
  }

  if (_params.radar.unambig_velocity == missing) {
    rParams.unambigVelocity = record.nyquistVel;
  } else {
    rParams.unambigVelocity = _params.radar.unambig_velocity;
  }

  rParams.unambigRange = _params.radar.unambig_range;

  if (strlen(_params.radar.radar_name) == 0) {
    rParams.radarName = UfRadar::label(record.manHdr.radar_name, 8) + ":" +
      UfRadar::label(record.manHdr.site_name, 8);
  } else {
    rParams.radarName = _params.radar.radar_name;
  }
  rParams.scanTypeName = _params.scan_type_name;

}

/////////////
// print beam
//

void Beam::print(bool print_headers, bool print_data)

{
  
  record.print(cout, print_headers, print_data);

}

/////////////////////
// print derived data
//

void Beam::_printDerived()

{

  record.printDerived(cout);

}


//////////////////////////////
// compute the tilt number etc
//
// Returns 0 on success, -1 on invalid beam

int Beam::computeTiltNum()

{

  double elev = record.manHdr.elevation / 64.0;

  bool use_tilt_table = false;
  bool search_by_tilt_num = false;

  if (record.manHdr.sweep_mode == UF_SWEEP_PPI)   {
    
    switch(_params.tilt_table_flag) {
    case Params::GET_TARGET_FROM_TABLE :
      use_tilt_table = true;
      search_by_tilt_num = true;
      break;
    case Params::COMPUTE_TILT_NUM_FROM_TABLE :
      use_tilt_table = true;
      search_by_tilt_num = false;
      break;
    case Params::DONT_USE_TABLE :
      use_tilt_table = false;
      search_by_tilt_num = false;
      break;
    } // switch

  } // if (record.manHdr.sweep_mode == UF_SWEEP_PPI)
  
  if (use_tilt_table) {
    
    if (search_by_tilt_num &&
	(_tiltIndex < 0 || record.manHdr.sweep_num != _tiltIndex)) {
      
      // Find the tilt information.
      
      _tiltIndex = -1;
      for (int i = 0; i < _params.tilt_table_n; i++) {
	if (_params._tilt_table[i].tilt_num == record.manHdr.sweep_num) {
	  _tiltIndex = i;
	  break;
	}
      } // i

      if (_tiltIndex < 0) {
	if (_params.debug >= Params::DEBUG_VERBOSE) {
	  cerr << "WARNING - Beam::_computeTileNum" << endl;
	  cerr << "  Tilt in primary file but not in tilt table: "
	       << record.manHdr.sweep_num << endl;
	  cerr << "  Skipping beam" << endl;
	}
	return -1;
      }
      
    } else {

      // Find the tilt with the closest target elevation.
      
      if (_params.tilt_table_n < 1) {
	cerr << "ERROR - Beam::_computeTileNum" << endl;
	cerr << "  Must have at least one entry in tilt table" << endl;
	return -1;
      }
      
      _tiltIndex = 0;
      double minDiff = fabs(_targetElev - elev);
      
      for (int i = 1; i < _params.tilt_table_n; i++) {
	double target = _params._tilt_table[_tiltIndex].target_elevation;
	double diff = fabs(elev - target);
	if (diff < minDiff) {
	  minDiff = diff;
	  _tiltIndex = i;
	}
      } // i
      
      _tiltNum = _params._tilt_table[_tiltIndex].tilt_num;
      _targetElev = _params._tilt_table[_tiltIndex].target_elevation;

    } // if (search_by_tilt_num ...

  } else {

    _tiltIndex = -1;
    _targetElev = elev;
    _tiltNum = record.manHdr.sweep_num;

  } // if (use_tilt_table)

  return 0;

}

  
  
