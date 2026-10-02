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
//////////////////////////////////////////////////////////////////////////
// Sounding.cc: helper objects for moments computations
//
// RAP, NCAR, Boulder CO
//
// April 2010
//
// Mike Dixon
//
//////////////////////////////////////////////////////////////////////////
//
// This is implemented as a singleton.
//
//////////////////////////////////////////////////////////////////////////


#include "Sounding.hh"
#include <cstdio>
#include <cassert>
#include <iostream>
#include <toolsa/DateTime.hh>

using namespace std;

// Global variables - instance

Sounding *Sounding::_instance = nullptr;

//////////////////////////////////////////////////////////////////////////
// Constructor - private, called by inst()

Sounding::Sounding()
{
  _params = nullptr;
  _tempProfile.clear();
  _radarMdvx = nullptr;
  _radarTime = 0;
  _modelTime = 0;
  _interpProjSet = false;
}

//////////////////////////////////////////////////////////////////////////
// Destructor

Sounding::~Sounding()
{

}

//////////////////////////////////////////////////////////////////////////
// Inst() - Retrieve the singleton instance of this class.

Sounding &Sounding::inst()
{

  if (_instance == (Sounding *)nullptr) {
    _instance = new Sounding();
  }

  return *_instance;

}

///////////////////////////////////////////////////
// set the parameters
  
void Sounding::setParams(const Params *params)
{
  
  _params = params;
  _setFromParams();
  
}

///////////////////////////////////////////////////
// retrieve temperature profile for specified time
// returns 0 on success, -1 on failure
  
int Sounding::retrieveTempProfile(const DsMdvx &radarMdvx)
  
{

  assert(_params != nullptr);
  time_t scanTime = radarMdvx.getMasterHeader().time_centroid;
  assert(_params != nullptr);
  if (_params->debug >= Params::DEBUG_VERBOSE) {
    cerr << "Getting temp profile for time: " 
         << DateTime::strm(scanTime) << endl;
  }
  
  if (_params->sounding_mode == Params::READ_SOUNDING_FROM_SPDB) {
    if (_readSpdb(radarMdvx) == 0) {
      _tempProfile = _spdbProfile;
    } else {
      cerr << "WARNING - cannot get sounding from SPDB" << endl;
      cerr << "  Creating profile from parameter file" << endl;
      _tempProfile = _paramsProfile;
    }
  } else if (_params->sounding_mode == Params::READ_SOUNDING_FROM_MODEL) {
    if (_readModel(radarMdvx) == 0) {
      _tempProfile = _modelProfile;
    } else {
      cerr << "WARNING - cannot get sounding from model" << endl;
      cerr << "  Creating profile from parameter file" << endl;
      _tempProfile = _paramsProfile;
    }
  } else {
    _tempProfile = _paramsProfile;
  }

  _tempProfile.prepareForUse();
  
  if (_params->debug) {
    cerr << "=====================================" << endl;
    cerr << "Overriding temperature profile" << endl;
    cerr << "  scanTime: " << DateTime::strm(scanTime) << endl;
    cerr << "  freezingLevel: " << _tempProfile.getFreezingLevel() << endl;
    cerr << "  htOfMinus20C: " << _tempProfile.getHtKmForTempC(-20.0) << endl;
    cerr << "=====================================" << endl;
  }
  
  if (_params->debug >= Params::DEBUG_VERBOSE) {
    _tempProfile.print(cerr);
  }
  
  return 0;

}

///////////////////////////////////////////////////
// set from parameter file
// returns 0 on success, -1 on failure
  
void Sounding::_setFromParams()
  
{
  
  // set profile from param file
  
  _paramsProfile.clear();
  for (int ii = 0; ii < _params->specified_sounding_n; ii++) {
    const Params::sounding_entry_t &entry = 
      _params->_specified_sounding[ii];
    TempProfile::PointVal point;
    point.setHtKm(entry.height_m / 1000.0);
    point.setTmpC(entry.temp_c);
    if (entry.pressure_hpa >= 0) {
      point.setPressHpa(entry.pressure_hpa);
    }
    if (entry.rh_percent >= 0) {
      point.setRhPercent(entry.rh_percent);
    }
    _paramsProfile.addPoint(point);
  } // ii
  
}

  
///////////////////////////////////////////////////
// retrieve temperature profile from SPDB
// returns 0 on success, -1 on failure
  
int Sounding::_readSpdb(const DsMdvx &radarMdvx)
  
{

  time_t scanTime = radarMdvx.getMasterHeader().time_centroid;
  
  time_t retrievedTime = scanTime;
  _spdbProfile.clear();
  
  _spdbProfile.setSoundingLocationName
    (_params->sounding_location_name);
  _spdbProfile.setSoundingSearchTimeMarginSecs
    (_params->sounding_search_time_margin_secs);
  
  _spdbProfile.setCheckPressureRange
    (_params->sounding_check_pressure_range);
  _spdbProfile.setSoundingRequiredMinPressureHpa
    (_params->sounding_required_pressure_range_hpa.min_val);
  _spdbProfile.setSoundingRequiredMaxPressureHpa
    (_params->sounding_required_pressure_range_hpa.max_val);
  
  _spdbProfile.setCheckHeightRange
    (_params->sounding_check_height_range);
  _spdbProfile.setSoundingRequiredMinHeightM
    (_params->sounding_required_height_range_m.min_val);
  _spdbProfile.setSoundingRequiredMaxHeightM
    (_params->sounding_required_height_range_m.max_val);
  
  _spdbProfile.setCheckPressureMonotonicallyDecreasing
    (_params->sounding_check_pressure_monotonically_decreasing);
  
  if (_params->debug >= Params::DEBUG_EXTRA) {
    _spdbProfile.setVerbose();
  }
  if (_params->debug >= Params::DEBUG_VERBOSE) {
    _spdbProfile.setDebug();
  }
  
  if (_spdbProfile.loadFromSpdb(_params->sounding_spdb_url,
                                scanTime,
                                retrievedTime)) {
    cerr << "ERROR - Sounding::retrieveTempProfile" << endl;
    cerr << "  Cannot retrive profile for time: "
         << DateTime::strm(scanTime) << endl;
    cerr << "  url: " << _params->sounding_spdb_url << endl;
    cerr << "  station name: " << _params->sounding_location_name << endl;
    cerr << "  time margin secs: "
         << _params->sounding_search_time_margin_secs << endl;
    return -1;
  }
  
  if (_params->debug) {
    cerr << "Got SPDB temp profile, URL: " << _params->sounding_spdb_url << endl;
  }

  return 0;

}

/////////////////////////////////////////////////////////
// read in model data
//
// Returns 0 on success, -1 on failure.

int Sounding::_readModel(const DsMdvx &radarMdvx)

{

  // cerr << "111111111111111111111111111111111111" << endl;
  // radarMdvx.printAllHeaders(cerr);
  // cerr << "111111111111111111111111111111111111" << endl;
  
  _radarMdvx = &radarMdvx;
  _radarTime = _radarMdvx->getMasterHeader().time_centroid;
  // cerr << "  Search time: " << DateTime::strm(_radarTime) << endl;
  // cerr << "111111111111111111111111111111111111" << endl;

  // check for relevant model data
  
  MdvxTimeList timeList;
  timeList.setModeClosest(_params->model_input_url, _radarTime,
                          _params->model_search_margin_secs);
  if (timeList.compile() ||
      timeList.getValidTimes().size() < 1) {
    cerr << "ERROR - Sounding::_readModel" << endl;
    cerr << "  Cannot find model data within search time" << endl;
    cerr << "  URL: " << _params->model_input_url << endl;
    cerr << "  Search time: " << DateTime::strm(_radarTime) << endl;
    cerr << "  Search margin (secs): " << _params->model_search_margin_secs << endl;
    cerr << timeList.getErrStr() << endl;
    return -1;
  }
  time_t thisModelTime = timeList.getValidTimes()[0];
  // cerr << "  nTimes: " << timeList.getValidTimes().size() << endl;
  // cerr << "  thisModelTime: " << DateTime::strm(thisModelTime) << endl;
  // cerr << "111111111111111111111111111111111111" << endl;
  if (thisModelTime == _modelTime) {
    // same as previous time, so use previous
    if (_params->debug) {
      cerr << "Using temperature profile from previous model ingest" << endl;
      cerr << "  Model time: " << DateTime::strm(_modelTime) << endl;
    }
    return 0;
  }
  
  // read in model temperature
  
  _modelRawMdvx.clearRead();
  _modelRawMdvx.setReadTime(Mdvx::READ_CLOSEST,
                            _params->model_input_url,
                            _params->model_search_margin_secs,
                            thisModelTime);
  _modelRawMdvx.addReadField(_params->model_temperature_field_name);
  
  if (_modelRawMdvx.readVolume()) {
    cerr << "ERROR - Sounding::_readModel" << endl;
    cerr << "  Cannot read model data" << endl;
    cerr << "  URL: " << _params->model_input_url << endl;
    cerr << "  Search time: " << DateTime::strm(thisModelTime) << endl;
    cerr << "  Search margin (secs): " << _params->model_search_margin_secs << endl;
    cerr << _modelRawMdvx.getErrStr() << endl;
    return -1;
  }

  _modelTime = _modelRawMdvx.getMasterHeader().time_centroid;
  if (_params->debug) {
    cerr << "Success reading model temperature" << endl;
    cerr << "  Model file " << _modelRawMdvx.getPathInUse() << endl;
    cerr << "  Model time: " << DateTime::strm(_modelTime) << endl;
  }

  // interpolate the model data onto the output Cartesian grid

  _interpModelToRadarGrid();

  // compute the temperature profile from the model data

  if (_computeModelTempProfile()) {
    cerr << "ERROR - Sounding::_readModel" << endl;
    cerr << "  Cannot compute temp profile, time: "
         << DateTime::strm(_radarTime) << endl;
    return -1;
  }

  return 0;

}

////////////////////////////////////////////////////////////////
// compute the temperatude profile from the interpolated model

int Sounding::_computeModelTempProfile()
{

  _modelProfile.clear();
  
  MdvxField *tempFld =
    _modelInterpMdvx.getField(_params->model_temperature_field_name);
  if (_params->model_convert_K_to_C) {
    tempFld->applyLinearTransform(1.0, -273.15, "", "K");
  }
  if (tempFld == nullptr) {
    cerr << "ERROR - Sounding::_computeModelTempProfile" << endl;
    cerr << "  Cannot find temp field in model, name: "
         << _params->model_temperature_field_name << endl;
    cerr << "  Model time: " << DateTime::strm(_modelTime) << endl;
    return -1;
  }

  const Mdvx::field_header_t &fhdr = tempFld->getFieldHeader();
  const Mdvx::vlevel_header_t &vhdr = tempFld->getVlevelHeader();
  fl32 *tempVol = (fl32 *) tempFld->getVol();
  fl32 miss = fhdr.missing_data_value;
  size_t nPtsPlane = fhdr.ny * fhdr.nx;

  for (int iz = 0; iz < fhdr.nz; iz++) {
    double htKm = vhdr.level[iz];
    fl32 *tmpPtr = tempVol + iz * nPtsPlane;
    double sum = 0.0, nn = 0.0;
    for (size_t ii = 0; ii < nPtsPlane; ii++, tmpPtr++) {
      if (*tmpPtr != miss) {
        sum += *tmpPtr;
        nn++;
      }
    } // ii
    // cerr << "tttttttttttttttt iz, htkm, meanTemp: " << iz << ", " << htKm << ", " << sum / nn << endl;
    if (nn > 0) {
      double meanTemp = sum / nn;
      TempProfile::PointVal val(htKm, meanTemp);
      _modelProfile.addPoint(val);
    }
    
  } // iz

  if (_modelProfile.getProfile().size() < 2) {
    cerr << "ERROR - Sounding::_computeModelTempProfile" << endl;
    cerr << "  Not enough temp data for valid profile" << endl;
    cerr << "  Cannot find temp field in model, time: "
         << DateTime::strm(_radarTime) << endl;
    return -1;
  }
  
  return 0;

}

/////////////////////////////////////////////////////////
// interpolate the model data onto the output grid

void Sounding::_interpModelToRadarGrid()
{

  if (!_interpProjSet) {
    _interpProj.init(*_radarMdvx);
    MdvxField *fld0 = _radarMdvx->getField(0);
    Mdvx::field_header_t fhdr0 = fld0->getFieldHeader();
    Mdvx::vlevel_header_t vhdr0 = fld0->getVlevelHeader();
    _interpVlevels.clear();
    for (int ii = 0; ii < fhdr0.nz; ii++) {
      _interpVlevels.push_back(vhdr0.level[ii]);
    }
    _modelRemap.setTargetCoords(_interpProj, _interpVlevels);
    _interpProjSet = true;
  }
  
  _modelInterpMdvx.clear();
  if (_params->debug >= Params::DEBUG_VERBOSE) {
    _modelInterpMdvx.setDebug(true);
  }
  _modelInterpMdvx.setMasterHeader(_radarMdvx->getMasterHeader());
  
  for (size_t ifield = 0; ifield < _modelRawMdvx.getNFields(); ifield++) {
    MdvxField *rawFld = _modelRawMdvx.getField(ifield);
    MdvxField *interpField = _modelRemap.interpField(*rawFld);
    string rawName = rawFld->getFieldName();
    _modelInterpMdvx.addField(interpField);
  }

  if (_params->write_model_temp_files) {
    if (_modelInterpMdvx.writeToDir(_params->model_temp_output_url) == 0) {
      if (_params->debug) {
        cerr << "Wrote model temp file: " << _modelInterpMdvx.getPathInUse() << endl;
      }
    } else {
      cerr << "ERROR writing model temp data to url: "
           << _params->model_temp_output_url << endl;
      cerr << _modelInterpMdvx.getErrStr() << endl;
    }
  }
  
}

