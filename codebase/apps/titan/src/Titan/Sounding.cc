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
// retrieve temperature profile for specified time
// returns 0 on success, -1 on failure
  
int Sounding::retrieveTempProfile(time_t profileTime)
  
{

  if (_params->debug >= Params::DEBUG_VERBOSE) {
    cerr << "Getting temp profile for time: " 
         << DateTime::strm(profileTime) << endl;
  }

  assert(_params != nullptr);
  
  time_t retrievedTime = profileTime;
  _tempProfile.clear();

  if (_params->sounding_mode == Params::READ_SOUNDING_FROM_SPDB) {
    
    _tempProfile.setSoundingLocationName
      (_params->sounding_location_name);
    _tempProfile.setSoundingSearchTimeMarginSecs
      (_params->sounding_search_time_margin_secs);
    
    _tempProfile.setCheckPressureRange
      (_params->sounding_check_pressure_range);
    _tempProfile.setSoundingRequiredMinPressureHpa
      (_params->sounding_required_pressure_range_hpa.min_val);
    _tempProfile.setSoundingRequiredMaxPressureHpa
      (_params->sounding_required_pressure_range_hpa.max_val);
    
    _tempProfile.setCheckHeightRange
      (_params->sounding_check_height_range);
    _tempProfile.setSoundingRequiredMinHeightM
      (_params->sounding_required_height_range_m.min_val);
    _tempProfile.setSoundingRequiredMaxHeightM
      (_params->sounding_required_height_range_m.max_val);
    
    _tempProfile.setCheckPressureMonotonicallyDecreasing
      (_params->sounding_check_pressure_monotonically_decreasing);
    
    if (_params->debug >= Params::DEBUG_EXTRA) {
      _tempProfile.setVerbose();
    }
    if (_params->debug >= Params::DEBUG_VERBOSE) {
      _tempProfile.setDebug();
    }
  
    if (_tempProfile.loadFromSpdb(_params->sounding_spdb_url,
                                  profileTime,
                                  retrievedTime)) {
      cerr << "ERROR - Sounding::retrieveTempProfile" << endl;
      cerr << "  Cannot retrive profile for time: "
           << DateTime::strm(profileTime) << endl;
      cerr << "  url: " << _params->sounding_spdb_url << endl;
      cerr << "  station name: " << _params->sounding_location_name << endl;
      cerr << "  time margin secs: "
           << _params->sounding_search_time_margin_secs << endl;
      return -1;
    }
    
    if (_params->debug) {
      cerr << "=====================================" << endl;
      cerr << "Got temp profile, URL: " << _params->sounding_spdb_url << endl;
      cerr << "Overriding temperature profile" << endl;
      cerr << "  vol time: " << DateTime::strm(profileTime) << endl;
      cerr << "  retrievedTime: " << DateTime::strm(retrievedTime) << endl;
      cerr << "  freezingLevel: " << _tempProfile.getFreezingLevel() << endl;
    }

  } else {
    
    // set profile from param file

    _tempProfile.clear();
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
      _tempProfile.addPoint(point);
    } // ii
    _tempProfile.prepareForUse();

  }

  if (_params->debug >= Params::DEBUG_VERBOSE) {
    _tempProfile.print(cerr);
  }
  
  return 0;

}

/////////////////////////////////////////////////////////
// read in model data
//
// Returns 0 on success, -1 on failure.

int Sounding::_readModel(const DsMdvx &radarMdvx)

{

  _radarMdvx = &radarMdvx;
  _radarTime = _radarMdvx->getMasterHeader().time_centroid;
  _modelRawMdvx.clearRead();
  _modelRawMdvx.setReadTime(Mdvx::READ_CLOSEST,
                            _params->model_input_url,
                            _params->model_search_margin_secs,
                            _radarTime);

  for (int ii = 0; ii < _params->model_fields_n; ii++) {
    if (_params->_model_fields[ii].is_available) {
      continue;
    }
    _modelRawMdvx.addReadField(_params->_model_fields[ii].field_name);
  }
  
  if (_modelRawMdvx.readVolume()) {
    cerr << "ERROR - Sounding::_readModel" << endl;
    cerr << "  Cannot read model data" << endl;
    cerr << "  URL: " << _params->model_input_url << endl;
    cerr << "  Search time: " << DateTime::strm(_radarTime) << endl;
    cerr << "  Search margin (secs): " << _params->model_search_margin_secs << endl;
    cerr << _modelRawMdvx.getErrStr() << endl;
    return -1;
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

  _tempProfile.clear();

  MdvxField *tempFld =
    _modelInterpMdvx.getField(getModelInputName(Params::TEMP).c_str());
  if (tempFld == nullptr) {
    cerr << "ERROR - Sounding::_computeModelTempProfile" << endl;
    cerr << "  Cannot find temp field in model, time: "
         << DateTime::strm(_radarTime) << endl;
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
    if (nn > 0) {
      double meanTemp = sum / nn;
      TempProfile::PointVal val(htKm, meanTemp);
      _tempProfile.addPoint(val);
    }
    
  } // iz

  if (_tempProfile.getProfile().size() < 2) {
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
  } // ifield
  
}

//////////////////////////////////////////////////
// get model field name from type

string Sounding::getModelInputName(Params::model_field_type_t mftype)
{
  for (int ii = 0; ii < _params->model_fields_n; ii++) {
    if (_params->_model_fields[ii].field_type == mftype) {
      return _params->_model_fields[ii].field_name;
    }
  }
  // not found
  return "";
}

//////////////////////////////////////////////////
// get model type from input name

Params::model_field_type_t Sounding::getModelTypeFromInputName(const string name)
{
  for (int ii = 0; ii < _params->model_fields_n; ii++) {
    string inputName = _params->_model_fields[ii].field_name;
    if (name == inputName) {
      return _params->_model_fields[ii].field_type;
    }
  }
  // not found, assume temp
  return Params::MODEL_NOT_SET;
}

