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
///////////////////////////////////////////////////////////////
// Mdv2SoundingSpdb.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2014
//
///////////////////////////////////////////////////////////////
//
// Mdv2SoundingSpdb reads MDV Cartesian grid files, samples the data
// at an array of specified points, loads the data as a sounding and
// writes the soundings to SPDB.
//
///////////////////////////////////////////////////////////////

#include "Mdv2SoundingSpdb.hh"
#include <toolsa/str.h>
#include <toolsa/pmu.h>
#include <toolsa/TaArray.hh>
#include <toolsa/toolsa_macros.h>
#include <didss/DsInputPath.hh>
#include <Spdb/SoundingPut.hh>
#include <physics/thermo.h>
#include <Mdv/MdvxField.hh>
using namespace std;
const double Mdv2SoundingSpdb::_missingVal = -9999.0;

// Constructor

Mdv2SoundingSpdb::Mdv2SoundingSpdb(int argc, char **argv)

{

  isOK = true;
  _isLatLon = false;
  
  // set programe name

  _progName = "Mdv2SoundingSpdb";
  ucopyright(_progName.c_str());

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = false;
    return;
  }

  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = false;
  }

  // check that start and end time is set in archive mode

  if (_params.mode == Params::ARCHIVE) {
    if (_args.startTime == 0 || _args.endTime == 0) {
      cerr << "ERROR - must specify start and end dates." << endl << endl;
      _args.usage(_progName, cerr);
      isOK = false;
    }
  }
  
  if (!isOK) {
    return;
  }

  // input file object

  if (_params.mode == Params::FILELIST) {

    if (_args.inputFileList.size() > 0) {
      _input = new DsInputPath(_progName,
			       _params.debug >= Params::DEBUG_VERBOSE,
			       _args.inputFileList);
    } else {
      cerr << "ERROR: " << _progName << endl;
      cerr << "  In FILELIST mode you must specify the files using -f arg." << endl;
      _args.usage(_progName, cerr);
      isOK = false;
      return;
    }

  } else if (_params.mode == Params::ARCHIVE) {
      
    if (_args.startTime != 0 && _args.endTime != 0) {
      _input = new DsInputPath(_progName,
			       _params.debug >= Params::DEBUG_VERBOSE,
			       _params.input_dir,
			       _args.startTime,
			       _args.endTime);
    } else {
      cerr << "ERROR: " << _progName << endl;
      cerr << "  In ARCHIVE mode you must specify start and end dates." << endl << endl;
      _args.usage(_progName, cerr);
      isOK = false;
      return;
    }
      
  } else {

    // realtime mode
    
    _input = new DsInputPath(_progName,
			     _params.debug >= Params::DEBUG_VERBOSE,
			     _params.input_dir,
			     _params.max_realtime_valid_age,
			     PMU_auto_register,
                             _params.latest_data_info_avail);
    
  } // if (_params.mode == Params::FILELIST)

  // auto register with procmap
  
  PMU_auto_init(_progName.c_str(),
                _params.instance,
                _params.procmap_register_interval);
  
}

// destructor

Mdv2SoundingSpdb::~Mdv2SoundingSpdb()

{

  // unregister process

  PMU_auto_unregister();
  
  // free up
  
  delete _input;

}

//////////////////////////////////////////////////
// Run

int Mdv2SoundingSpdb::Run ()
{

  PMU_auto_register("Mdv2SoundingSpdb::Run");

  int iret = 0;
  const char *filePath;
  while ((filePath = _input->next()) != NULL) {
    if (_processFile(filePath)) {
      iret = -1;
    }
  }

  return iret;

}

////////////////////////////
// Process the file
//
// Returns 0 on success, -1 on failure

int Mdv2SoundingSpdb::_processFile(const char *filePath)

{

  if (_params.debug) {
    fprintf(stderr, "Processing file: %s\n", filePath);
  }
  PMU_auto_register("Processing file");

  // set up MDV object for reading
  
  DsMdvx mdvx;
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    mdvx.setDebug(true);
  }
  _setupRead(mdvx);
  mdvx.setReadPath(filePath);

  // loop through the sampling points

  int iret = 0;
  for (int ii = 0; ii < _params.station_locations_n; ii++) {
    if (_processStation(mdvx,
                        _params._station_locations[ii].name,
                        _params._station_locations[ii].latitude,
                        _params._station_locations[ii].longitude,
                        _params._station_locations[ii].altitudeKm)) {
      iret = -1;
    }
  }
  
  return iret;

}

////////////////////////////////////////////
// set up the read

void Mdv2SoundingSpdb::_setupRead(DsMdvx &mdvx)

{
  
  if (_params.debug) {
    mdvx.setDebug(true);
  }

  // set up the Mdvx read
  
  mdvx.clearRead();
  
  // set the field list

  if (strlen(_params.temp_field_name) > 0) {
    mdvx.addReadField(_params.temp_field_name);
  }

  if (strlen(_params.dewpt_field_name) > 0) {
    mdvx.addReadField(_params.dewpt_field_name);
  }

  if (strlen(_params.rh_field_name) > 0) {
    mdvx.addReadField(_params.rh_field_name);
  }

  if (strlen(_params.u_wind_field_name) > 0) {
    mdvx.addReadField(_params.u_wind_field_name);
  }

  if (strlen(_params.v_wind_field_name) > 0) {
    mdvx.addReadField(_params.v_wind_field_name);
  }

  if (strlen(_params.w_wind_field_name) > 0) {
    mdvx.addReadField(_params.w_wind_field_name);
  }

  if (strlen(_params.pressure_field_name) > 0) {
    mdvx.addReadField(_params.pressure_field_name);
  }

  if (strlen(_params.height_field_name) > 0) {
    mdvx.addReadField(_params.height_field_name);
  }

  mdvx.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  mdvx.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    mdvx.printReadRequest(cerr);
  }

}

////////////////////////////////////////////////////////////
// process data for a station

int Mdv2SoundingSpdb::_processStation(DsMdvx &mdvx,
                                      const string &name,
                                      double latitude,
                                      double longitude,
                                      double altitudeKm)

{
  
  // make a single-point vert section request

  mdvx.clearReadWayPts();
  mdvx.addReadWayPt(latitude, longitude);
  
  // read the column

  if (mdvx.readVsection()) {
    cerr << "ERROR - Mdv2SoundingSpdb::_processStation" << endl;
    cerr << "  Cannot read in column data." << endl;
    cerr << mdvx.getErrStr() << endl;
    return -1;
  }
  
  // get the vertical profile details

  MdvxField *fld0 = mdvx.getField(0);
  if (fld0 == NULL) {
    cerr << "ERROR - Mdv2SoundingSpdb::_processStation" << endl;
    cerr << "  No fields found in file." << endl;
    cerr << "  Path: " << mdvx.getPathInUse() << endl;
    return -1;
  }
  const Mdvx::field_header_t &fhdr0 = fld0->getFieldHeader();
  const Mdvx::vlevel_header_t &vhdr0 = fld0->getVlevelHeader();
  int nz = fhdr0.nz;

  // debug print
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    mdvx.printVol(cerr, &mdvx, false, true);
  }

  // load up profile data

  int nFieldsFound = 0;

  // temp

  TaArray<double> temp_;
  double *temp = (double *) temp_.alloc(nz);
  if (_loadField(mdvx, _params.temp_field_name, temp, nz) == 0) {
    nFieldsFound++;
  }
  if (_params.temp_units_are_kelvin) {
    for (int ii = 0; ii < nz; ii++) {
      if (temp[ii] != _missingVal) {
        temp[ii] = TEMP_K_TO_C(temp[ii]);
      }
    }
  }
  
  // rh
  
  TaArray<double> rh_;
  double *rh = (double *) rh_.alloc(nz);
  
  if (_params.compute_rh_from_dewpt) {
    TaArray<double> dewpt_;
    double *dewpt = (double *) dewpt_.alloc(nz);
    if (_loadField(mdvx, _params.dewpt_field_name, dewpt, nz) == 0) {
      nFieldsFound++;
    }
    for (int ii = 0; ii < nz; ii++) {
      if (temp[ii] != _missingVal && dewpt[ii] != _missingVal) {
        rh[ii] = PHYrelh(temp[ii], dewpt[ii]);
      }
    }
  } else {
    if (_loadField(mdvx, _params.rh_field_name, rh, nz) == 0) {
      nFieldsFound++;
    }
  }

  // pressure

  TaArray<double> pressure_;
  double *pressure = (double *) pressure_.alloc(nz);
  if (fhdr0.vlevel_type == Mdvx::VERT_TYPE_PRESSURE) {
    for (int ii = 0; ii < nz; ii++) {
      pressure[ii] = vhdr0.level[ii];
    }
  } else {
    if (_loadField(mdvx, _params.pressure_field_name, pressure, nz) == 0) {
      nFieldsFound++;
    }
  }

  // height

  TaArray<double> height_;
  double *height = (double *) height_.alloc(nz);
  if (fhdr0.vlevel_type == Mdvx::VERT_TYPE_Z) {
    for (int ii = 0; ii < nz; ii++) {
      height[ii] = vhdr0.level[ii] * 1000.0;
    }
  } else {
    if (_loadField(mdvx, _params.height_field_name, height, nz) == 0) {
      nFieldsFound++;
    }
    if (_params.heights_are_in_km) {
      for (int ii = 0; ii < nz; ii++) {
        if (height[ii] != _missingVal) {
          height[ii] *= 1000.0;
        }
      }
    }
  }

  // winds

  TaArray<double> uu_;
  double *uu = (double *) uu_.alloc(nz);
  if (_loadField(mdvx, _params.u_wind_field_name, uu, nz) == 0) {
    nFieldsFound++;
  }

  TaArray<double> vv_;
  double *vv = (double *) vv_.alloc(nz);
  if (_loadField(mdvx, _params.v_wind_field_name, vv, nz) == 0) {
    nFieldsFound++;
  }

  TaArray<double> ww_;
  double *ww = (double *) ww_.alloc(nz);
  if (_loadField(mdvx, _params.w_wind_field_name, ww, nz) == 0) {
    nFieldsFound++;
  }

  if (nFieldsFound < 1) {
    cerr << "ERROR - Mdv2SoundingSpdb::_processStation" << endl;
    cerr << "  No fields found in file." << endl;
    cerr << "  Path: " << mdvx.getPathInUse() << endl;
    return -1;
  }

  // create sounding

  SoundingPut sndg;
  sndg.init(_params.output_url,
            Sounding::RUC_ID,
            "Mdv2SoundingSpdb",
            Spdb::hash4CharsToInt32(name.c_str()),
            name.c_str(),
            latitude,
            longitude,
            altitudeKm * 1000.0,
            _missingVal);
  
  time_t stime = mdvx.getValidTime();

  if (sndg.set(stime, nz, height, uu, vv, ww, pressure, rh, temp, NULL)) {
    cerr << "ERROR - Mdv2SoundingSpdb::_processStation" << endl;
    cerr << "  Cannot set data in output sounding object" << endl;
    return -1;
  }

  if (sndg.writeSounding(stime, stime + _params.valid_period_secs)) {
    cerr << "ERROR - Mdv2SoundingSpdb::_processStation" << endl;
    cerr << "  Cannot write sounding object" << endl;
    cerr << sndg.getSpdbMgr().getErrStr() << endl;
  }

  if (_params.debug) {
    cerr << "Wrote spdb data, URL: " << _params.output_url << endl;
    cerr << "       Station name : " << name << endl;
    cerr << "       Sounding time: " << DateTime::strm(stime) << endl;
  }

  return 0;

}

////////////////////////////////////////////////////////////
// load field data into array

int Mdv2SoundingSpdb::_loadField(const DsMdvx &mdvx,
                                 const char *fieldName,
                                 double *arrayVals,
                                 int nz)
  
{

  // initialize to missing
  
  for (int ii = 0; ii < nz; ii++) {
    arrayVals[ii] = _missingVal;
  }
 
  if (strlen(fieldName) == 0) {
    return -1;
  }

  MdvxField *field = mdvx.getField(fieldName);
  if (field == NULL) {
    cerr << "ERROR - Mdv2SoundingSpdb::_loadField" << endl;
    cerr << "  Cannot find field name: " << fieldName << endl;
    cerr << "  File path: " << mdvx.getPathInUse() << endl;
    return -1;
  }

  fl32 *fldVals = (fl32*) field->getVol();
  fl32 missing = field->getFieldHeader().missing_data_value;
  int nzCopy = nz;
  if (nzCopy > field->getFieldHeader().nz) {
    nzCopy = field->getFieldHeader().nz;
  }
  for (int iz = 0; iz < nzCopy; iz++) {
    fl32 fldVal = fldVals[iz];
    if (fldVal == missing) {
      arrayVals[iz] = _missingVal;
    } else {
      arrayVals[iz] = fldVal;
    }
  }

  return 0;

}
