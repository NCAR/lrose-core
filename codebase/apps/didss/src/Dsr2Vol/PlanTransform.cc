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
////////////////////////////////////////////////////////////////////////
// PlanTransform.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 2001
//
///////////////////////////////////////////////////////////////////////

#include <toolsa/mem.h>
#include <toolsa/pmu.h>
#include "PlanTransform.hh"
using namespace std;

const double PlanTransform::_smallDiff = 0.00001;

const int PlanTransform::_azSign[8] = { 1, -1, 1, -1, 1, -1, 1, -1 };
const int PlanTransform::_xSign[8] = { 1, 1, 1, 1, -1, -1, -1, -1 };
const int PlanTransform::_ySign[8] = { 1, 1, -1, -1, -1, -1, 1, 1 };
const bool PlanTransform::_swapXy[8] = { false, true, true, false,
					 false, true, true, false };

//////////////////////
// Abstract Base class

PlanTransform::PlanTransform(const Params &params,
			     const string &mdv_url,
                             const vector<FieldInfo> &fields) :
  Transform(params, mdv_url, fields)

{
  _naz = 0;
  _nazPer45 = 0;
  _deltaAz = 0;
}

PlanTransform::~PlanTransform()

{
  freeLut();
}

void PlanTransform::freeLut()

{
  _lut.clear();
}


////////////////////////////////////////////////////////////////
// set regular beam array
// this should be done before setGeom so that the polar
// transform can compute the azimuith limits

void PlanTransform::setRegularBeamArray(const Beam ***regular_array)

{
  _regularBeamArray = regular_array;
}

////////////////////////////////////////////////////////////////
// set the radar and beam geometry
// returns true if geometry has changed, false otherwise

bool PlanTransform::setGeom(const vector<double> elevations,
                            int n_gates,
                            double start_range,
                            double gate_spacing,
                            int n_az,
                            int n_az_per_45,
                            double delta_az,
                            double beam_width,
                            double radar_lat,
                            double radar_lon,
                            double radar_alt)

{

  bool geomChanged = false;
  
  if (_setAzGrid(n_az, n_az_per_45, delta_az)) {
    geomChanged = true;
  }

  if (_setGateGeom(n_gates,
                   start_range,
                   gate_spacing)) {
    geomChanged = true;
  }

  if (_setRadarParams(beam_width,
                      radar_lat,
                      radar_lon,
                      radar_alt)) {
    geomChanged = true;
  }

  if (_setElevations(elevations)) {
    geomChanged = true;
  }

  return geomChanged;

}

////////////////////////////////////////////////////////////////
// write the output volume

int PlanTransform::writeVol(const DsRadarParams &radarParams,
                            const DsRadarCalib *radarCalib,
                            const string &statusXml,
			    int volNum,
                            time_t startTime, time_t endTime,
                            scan_mode_t scanMode)

{

  PMU_auto_register("PlanTransform::writeVol");

  if (_params.debug) {
    cerr << "**** Start PlanTransform::writeVol() ****" << endl;
  }
  
  OutputMdv mdv("Dsr2Vol", _params, _geomType);
  
  int volDuration = endTime - startTime;
  if (volDuration > _params.max_vol_duration) {
    if (_params.debug) {
      cerr << "WARNING - PlanTransform::_writeVol" << endl;
      cerr << "  Vol duration exceed max allowable" << endl;
      cerr << "  Vol duration: " << volDuration << " secs" << endl;
      cerr << "  Max duration: " << _params.max_vol_duration
	   << " secs" << endl;
    }
    return -1;
  }

  time_t midTime;
  if (_params.auto_mid_time) {
    midTime = startTime + (endTime - startTime) / 2;
  } else {
    midTime = endTime - _params.age_at_end_of_volume;
  }

  if (_params.debug) {
    cerr << "startTime: " << DateTime::str(startTime) << endl;
    cerr << "endTime: " << DateTime::str(endTime) << endl;
    cerr << "midTime: " << DateTime::str(midTime) << endl;
  }
  
  mdv.setMasterHeader(volNum, startTime, midTime, endTime,
		      _nx, _ny, _nz,
		      _radarLat, _radarLon, _radarAlt,
		      radarParams.radarName.c_str());
  
  for (size_t ifield = 0; ifield < _fields.size(); ifield++) {
    const FieldInfo &fld = _fields[ifield];
    if (fld.isLoaded) {
      mdv.addField(fld.name.c_str(),
		   fld.units.c_str(),
		   fld.isDbz,
		   _nx, _dx, _minx,
		   _ny, _dy, _miny,
		   _nz, _dz, _minz,
		   _elevArray,
		   _radarLat,
		   _radarLon,
		   _radarAlt,
		   fld.byteWidth,
		   fld.scale,
		   fld.bias,
		   fld.encoding,
		   fld.compression,
		   _outputFields[ifield]);
    }
  } // ifield

  // add coverage if requested

  if (_coverage != NULL) {

    Mdvx::compression_type_t compression = Mdvx::COMPRESSION_ZLIB;
    if (_params.output_compression == Params::RLE_COMPRESSION) {
      compression = Mdvx::COMPRESSION_RLE;
    } else if (_params.output_compression == Params::BZIP_COMPRESSION) {
      compression = Mdvx::COMPRESSION_BZIP;
    } else if (_params.output_compression == Params::GZIP_COMPRESSION) {
      compression = Mdvx::COMPRESSION_GZIP;
    } else if (_params.output_compression == Params::NO_COMPRESSION) {
      compression = Mdvx::COMPRESSION_NONE;
    }

    mdv.addField("Coverage", "", false,
		 _nx, _dx, _minx,
		 _ny, _dy, _miny,
		 _nz, _dz, _minz,
		 _elevArray,
		 _radarLat,
		 _radarLon,
		 _radarAlt,
		 4, 1.0, 0.0,
		 Mdvx::ENCODING_INT8,
		 compression,
		 _coverage);
  }
  
  mdv.addChunks(radarParams, radarCalib, statusXml, _elevArray);

  string url = _mdvUrl;
  // bool separateScanTypes = false;
  if (_params.separate_vert_files &&
      scanMode == SCAN_MODE_VERT) {
    url += PATH_DELIM;
    url += _params.vert_subdirectory;
    // separateScanTypes = true;
  } else if (scanMode == SCAN_MODE_RHI) {
    // use path as entered in param file
  } else if (_params.separate_sector_files) {
    url += PATH_DELIM;
    if (scanMode == SCAN_MODE_SURVEILLANCE) {
      url += _params.surveillance_subdirectory;
    } else {
      url += _params.sector_subdirectory;
    }
    // separateScanTypes = true;
  }

  if (mdv.writeVol(url.c_str())) {
    cerr << "ERROR - PlanTransform::writeVol" << endl;
    cerr << "  Cannot write output volume" << endl;
    return -1;
  }

  // if we separate the scan types, also register the
  // master URL with the DataMapper

  if (_params.write_master_ldata_info) {
    mdv.writeMasterLdataInfo();
  }

  if (_params.debug) {
    cerr << "**** End PlanTransform::writeVol() ****" << endl;
  }

  return 0;

}

////////////////////////////////////////////////////////////////
// Set the azimuth grid
//
// Returns true if grid has changed, false otherwise

bool PlanTransform::_setAzGrid(int n_az,
                               int n_az_per_45,
                               double delta_az)

{

  if (n_az != _naz ||
      n_az_per_45 != _nazPer45 ||
      fabs(delta_az - _deltaAz) > _smallDiff) {

    _naz = n_az;
    _nazPer45 = n_az_per_45;
    _deltaAz = delta_az;

    return true;

  }

  return false;

}

////////////////////////////////////////////////////////////////
// Set the gate geometry
//
// Returns true if geometry has changed, false otherwise

bool PlanTransform::_setGateGeom(int n_gates,
                                 double start_range,
                                 double gate_spacing)
  
{

  if (n_gates != _nGates ||
      fabs(start_range - _startRange) > _smallDiff ||
      fabs(gate_spacing - _gateSpacing) > _smallDiff) {

    _nGates = n_gates;
    _startRange = start_range;
    _gateSpacing = gate_spacing;

    return true;

  }

  return false;
}

////////////////////////////////////////////////////////////////
// set the radar params
// returns true if params have changed, false otherwise

bool PlanTransform::_setRadarParams(double beam_width,
                                    double radar_lat,
                                    double radar_lon,
                                    double radar_alt)

{

  if (fabs(beam_width - _beamWidth) > _smallDiff ||
      fabs(radar_lat - _radarLat) > _smallDiff ||
      fabs(radar_lon - _radarLon) > _smallDiff ||
      fabs(radar_alt - _radarAlt) > _smallDiff) {
    
    _beamWidth = beam_width;
    _radarLat = radar_lat;
    _radarLon = radar_lon;
    _radarAlt = radar_alt;

    return true;

  }

  return false;

}

////////////////////////////////////////////////////////////////
// set the radar elevations
// returns true if elevations have changed, false otherwise

bool PlanTransform::_setElevations(const vector<double> elevations)

{

  if (_elevsHaveChanged(elevations)) {

    _elevArray = elevations;
    _nElev = _elevArray.size();
    return true;

  }

  return false;

}

////////////////////////////////////////////////////////////////
// check for change in elevations
// returns true if changed, false otherwise

bool PlanTransform::_elevsHaveChanged(const vector<double> elevations)
  
{
  
  if (_elevArray.size() != elevations.size()) {
    return true;
  }
  
  for (size_t ii = 0; ii < elevations.size(); ii++) {
    if (fabs(_elevArray[ii] -elevations[ii]) > 0.01) {
      return true;
    }
  }

  return false;

}

