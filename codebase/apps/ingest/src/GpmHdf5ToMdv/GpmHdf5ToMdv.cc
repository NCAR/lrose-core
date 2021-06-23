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
// GpmHdf5ToMdv.cc
//
// GpmHdf5ToMdv object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2008
//
///////////////////////////////////////////////////////////////
//
// GpmHdf5ToMdv reads McIdas data in NetCDF format, and
// converts to MDV
//
////////////////////////////////////////////////////////////////

#include <algorithm>
#include <toolsa/toolsa_macros.h>
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <toolsa/DateTime.hh>
#include <toolsa/Path.hh>
#include <toolsa/TaArray.hh>
#include <Mdv/MdvxField.hh>
#include <dsserver/DsLdataInfo.hh>
#include "GpmHdf5ToMdv.hh"
using namespace std;

const fl32 GpmHdf5ToMdv::_missingFloat = -9999.0;

// Constructor

GpmHdf5ToMdv::GpmHdf5ToMdv(int argc, char **argv)

{

  isOK = true;
  _input = NULL;

  // set programe name

  _progName = "GpmHdf5ToMdv";
  ucopyright((char *) _progName.c_str());

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

  // check that file list set in archive mode
  
  if (_params.mode == Params::FILELIST && _args.inputFileList.size() == 0) {
    cerr << "ERROR: GpmHdf5ToMdv" << endl;
    cerr << "  Mode is ARCHIVE."; 
    cerr << "  You must use -f to specify files on the command line."
	 << endl;
    _args.usage(_progName, cerr);
    isOK = false;
  }

  // init process mapper registration

  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		_params.reg_interval);

  // initialize the data input object
  
  if (_params.mode == Params::REALTIME) {
    _input = new DsInputPath(_progName,
			     _params.debug >= Params::DEBUG_VERBOSE,
			     _params.input_dir,
			     _params.max_realtime_age,
			     PMU_auto_register,
			     _params.use_ldata_info_file,
			     _params.process_latest_file_only);
  } else if (_params.mode == Params::ARCHIVE) {
    _input = new DsInputPath(_progName,
			     _params.debug >= Params::DEBUG_VERBOSE,
			     _params.input_dir,
			     _args.startTime, _args.endTime);
  } else if (_params.mode == Params::FILELIST) {
    if (_params.debug) {
      for (int ii = 0; ii < (int) _args.inputFileList.size(); ii++) {
        cerr << "Adding path: " << _args.inputFileList[ii] << endl;
      }
    }
    _input = new DsInputPath(_progName,
			     _params.debug >= Params::DEBUG_VERBOSE,
			     _args.inputFileList);
  }

  if (strlen(_params.file_name_ext) > 0) {
    _input->setSearchExt(_params.file_name_ext);
  }

  if (strlen(_params.file_name_substr) > 0) {
    _input->setSubString(_params.file_name_substr);
  }

}

// destructor

GpmHdf5ToMdv::~GpmHdf5ToMdv()

{

  // unregister process

  PMU_auto_unregister();

  // free up

  if (_input) {
    delete _input;
  }

}

//////////////////////////////////////////////////
// Run

int GpmHdf5ToMdv::Run ()
{
  
  int iret = 0;
  PMU_auto_register("Run");
  
  // loop until end of data
  
  char *inputPath;
  while ((inputPath = _input->next()) != NULL) {
    
    PMU_auto_register("Reading file");
    ta_file_uncompress(inputPath);
    if (_processFile(inputPath)) {
      cerr << "ERROR = GpmHdf5ToMdv::Run" << endl;
      cerr << "  Processing file: " << inputPath << endl;
      iret = -1;
    }
    
  }
  
  return iret;

}

///////////////////////////////
// process file

int GpmHdf5ToMdv::_processFile(const char *input_path)

{

  PMU_auto_register("Processing file");
  
  if (_params.debug) {
    cerr << "Processing file: " << input_path << endl;
  }

  // use try block to catch any exceptions
  
  try {
    
    // open file
    
    H5File file(input_path, H5F_ACC_RDONLY);
    if (_params.debug) {
      cerr << "  file size: " << file.getFileSize() << endl;
    }
    
    // get the root group
    
    Group root(file.openGroup("/"));

    // root attributes

    _fileHeader = _readStringAttribute(root, "FileHeader", "RootAttr");
    _fileInfo = _readStringAttribute(root, "FileInfo", "RootAttr");
    _inputRecord = _readStringAttribute(root, "InputRecord", "RootAttr");
    _jaxaInfo = _readStringAttribute(root, "JAXAInfo", "RootAttr");
    _navigationRecord = _readStringAttribute(root, "NavigationRecord", "RootAttr");
    _history = _readStringAttribute(root, "history", "RootAttr");
    
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "FileHeader: " << endl << "===================" << endl
           << _fileHeader << "===================" << endl;
      cerr << "FileInfo: " << endl << "===================" << endl
           << _fileInfo << "===================" << endl;
      cerr << "InputRecord: " << endl << "===================" << endl
           << _inputRecord << "===================" << endl;
      cerr << "JAXAInfo: " << endl << "===================" << endl
           << _jaxaInfo << "===================" << endl;
      cerr << "NavigationRecord: " << endl << "===================" << endl
           << _navigationRecord << "===================" << endl;
      cerr << "history: " << endl << "===================" << endl
           << _history << "===================" << endl;
    }

    // open the NS group
    
    Group ns(root.openGroup("NS"));
    if (_readGroupNs(ns)) {
      return -1;
    }

  } // try
  
  catch (H5x::Exception &e) {
    // _addErrStr("ERROR - reading GPM HDF5 file");
    // _addErrStr(e.getDetailMsg());
    return -1;
  }

#ifdef JUNK

  // open file
  
  if (_openNc3File(input_path)) {
    cerr << "ERROR - GpmHdf5ToMdv::_processFile" << endl;
    cerr << "  File path: " << input_path << endl;
    return -1;
  }
  
  if (_params.debug >= Params::DEBUG_EXTRA) {
    _printFile(*_ncFile);
  }

  // check that this is a valid file

  if (_loadMetaData()) {
    cerr << "ERROR - GpmHdf5ToMdv::_processFile" << endl;
    cerr << "  File has invalid data" << endl;
    cerr << "  File: " << input_path << endl;
    return -1;
  }

  // loop through times

  for (int itime = 0; itime < _nTimes; itime++) {

    // create output Mdvx file object
    
    DsMdvx mdvx;
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      mdvx.setDebug(true);
    }
    
    // set master header
    
    if (_setMasterHeader(mdvx, itime)) {
      return -1;
    }

    // add the data fields

    if (_addDataFields(mdvx, itime)) {
      return -1;
    }
    
    // remap if required
    
    if (_params.remap_output_projection) {
      _remapOutput(mdvx);
    }
    
    // write output file
    
    if (_params.debug) {
      cerr << "Writing file to url: " << _params.output_url << endl;
    }

    if (mdvx.writeToDir(_params.output_url)) {
      cerr << "ERROR - GpmHdf5ToMdv" << endl;
      cerr << "  Cannot write file to url: " << _params.output_url << endl;
      cerr << mdvx.getErrStr() << endl;
      return -1;
    }
    
    if (_params.debug) {
      cerr << "  Wrote output file: " << mdvx.getPathInUse() << endl;
    }

  } // itime

#endif

  return 0;

}

/////////////////////////////////////////////
// read string attribute

string GpmHdf5ToMdv::_readStringAttribute(Group &group,
                                          const string &attrName,
                                          const string &context)
  
{
  Hdf5xx::DecodedAttr decodedAttr;
  _utils.loadAttribute(group, attrName, context, decodedAttr);
  string attr(decodedAttr.getAsString());
  return attr;
}

//////////////////////////////////////////////
// read the NS group

int GpmHdf5ToMdv::_readGroupNs(Group &ns)
  
{

  string swathHeader = _readStringAttribute(ns, "SwathHeader", "ns-attr");
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "SwathHeader: " << endl << "===================" << endl
         << swathHeader << "===================" << endl;
  }

  if (_readTimes(ns)) {
    return -1;
  }

  if (_readLatLon(ns)) {
    return -1;
  }
  
  if (_readQcFlags(ns)) {
    return -1;
  }
  
  if (_readReflectivity(ns)) {
    return -1;
  }
  
  return 0;

#ifdef JUNK
  Hdf5xx::DecodedAttr decodedAttr;

  if (_utils.loadAttribute(what, "object", "root-what", decodedAttr)) {
    _addErrStr(_utils.getErrStr());
    return -1;
  }
  _objectStr = decodedAttr.getAsString();

  if (_objectStr != "PVOL" && _objectStr != "SCAN" &&
      _objectStr != "AZIM" && _objectStr != "ELEV") {
    _addErrStr("Bad object type: ", _objectStr);
    _addErrStr("  Must be 'PVOL','SCAN','AZIM'or'ELEV'");
    return -1;
  }

  _utils.loadAttribute(what, "version", "root-what", decodedAttr);
  _version = decodedAttr.getAsString();
  
  _utils.loadAttribute(what, "date", "root-what", decodedAttr);
  _dateStr = decodedAttr.getAsString();
  
  _utils.loadAttribute(what, "time", "root-what", decodedAttr);
  _timeStr = decodedAttr.getAsString();
  
  _utils.loadAttribute(what, "source", "root-what", decodedAttr);
  _source = decodedAttr.getAsString();
  
  if (_debug) {
    cerr  << "  root what _objectStr: " << _objectStr << endl;
    cerr  << "  root what _version: " << _version << endl;
    cerr  << "  root what _dateStr: " << _dateStr << endl;
    cerr  << "  root what _timeStr: " << _timeStr << endl;
    cerr  << "  root what _source: " << _source << endl;
  }

  return 0;

#endif

}

//////////////////////////////////////////////
// read the scan times

int GpmHdf5ToMdv::_readTimes(Group &ns)
  
{

  Group scanTime(ns.openGroup("ScanTime"));
  vector<size_t> dims;
  string units;
  NcxxPort::si32 missingVal;
  
  vector<NcxxPort::si32> years, months, days, hours, mins, secs, msecs;
  Hdf5xx hdf5;
  if (hdf5.readSi32Array(scanTime, "Year", "ScanTime",
                         dims, missingVal, years, units)) {
    cerr << "ERROR - GpmHdf5ToMdv::_readTimes()" << endl;
    cerr << "  Cannot read Year variable" << endl;
    return -1;
  }
  if (hdf5.readSi32Array(scanTime, "Month", "ScanTime",
                         dims, missingVal, months, units)) {
    cerr << "ERROR - GpmHdf5ToMdv::_readTimes()" << endl;
    cerr << "  Cannot read Month variable" << endl;
    return -1;
  }
  if (hdf5.readSi32Array(scanTime, "DayOfMonth", "ScanTime",
                         dims, missingVal, days, units)) {
    cerr << "ERROR - GpmHdf5ToMdv::_readTimes()" << endl;
    cerr << "  Cannot read DayOfMonth variable" << endl;
    return -1;
  }
  if (hdf5.readSi32Array(scanTime, "Hour", "ScanTime",
                         dims, missingVal, hours, units)) {
    cerr << "ERROR - GpmHdf5ToMdv::_readTimes()" << endl;
    cerr << "  Cannot read Hour variable" << endl;
    return -1;
  }
  if (hdf5.readSi32Array(scanTime, "Minute", "ScanTime",
                         dims, missingVal, mins, units)) {
    cerr << "ERROR - GpmHdf5ToMdv::_readTimes()" << endl;
    cerr << "  Cannot read Minute variable" << endl;
    return -1;
  }
  if (hdf5.readSi32Array(scanTime, "Second", "ScanTime",
                         dims, missingVal, secs, units)) {
    cerr << "ERROR - GpmHdf5ToMdv::_readTimes()" << endl;
    cerr << "  Cannot read Second variable" << endl;
    return -1;
  }
  if (hdf5.readSi32Array(scanTime, "MilliSecond", "ScanTime",
                         dims, missingVal, msecs, units)) {
    cerr << "ERROR - GpmHdf5ToMdv::_readTimes()" << endl;
    cerr << "  Cannot read MilliSecond variable" << endl;
    return -1;
  }

  _times.clear();
  for (size_t ii = 0; ii < years.size(); ii++) {
    DateTime dtime(years[ii], months[ii], days[ii],
                   hours[ii], mins[ii], secs[ii], msecs[ii] / 1000.0);
    _times.push_back(dtime);
  }
  if (_times.size() < 1) {
    cerr << "ERROR - GpmHdf5ToMdv::_readTimes()" << endl;
    cerr << "  No scan times found" << endl;
    return -1;
  }
  
  _startTime = _times[0];
  _endTime = _times[_times.size()-1];

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "====>> Reading scan times <<====" << endl;
    cerr << "  nTimes: " << _times.size() << endl;
    cerr << "  startTime: " << _startTime.asString(3) << endl;
    cerr << "  endTime: " << _endTime.asString(3) << endl;
    if (_params.debug >= Params::DEBUG_EXTRA) {
      for (size_t ii = 0; ii < _times.size(); ii++) {
        cerr << "  ii, time: " << ii << ", " << _times[ii].asString(3) << endl;
      }
    }
  }
  
  return 0;

}

//////////////////////////////////////////////
// read the quality flags

int GpmHdf5ToMdv::_readQcFlags(Group &ns)
  
{

  Group scanStatus(ns.openGroup("scanStatus"));
  vector<size_t> dims;
  string units;
  NcxxPort::si32 missingVal;

  _dataQuality.resize(_times.size());
  _dataWarning.resize(_times.size());
  _geoError.resize(_times.size());
  _geoWarning.resize(_times.size());
  _limitErrorFlag.resize(_times.size());
  _missingScan.resize(_times.size());
  
  Hdf5xx hdf5;
  if (hdf5.readSi32Array(scanStatus, "dataQuality", "scanStatus",
                         dims, missingVal, _dataQuality, units)) {
    cerr << "WARNING - GpmHdf5ToMdv::_readQcFlags()" << endl;
    cerr << "  Cannot read dataQuality flag variable" << endl;
  }
  if (hdf5.readSi32Array(scanStatus, "dataWarning", "scanStatus",
                         dims, missingVal, _dataWarning, units)) {
    cerr << "WARNING - GpmHdf5ToMdv::_readQcFlags()" << endl;
    cerr << "  Cannot read dataWarning flag variable" << endl;
  }
  if (hdf5.readSi32Array(scanStatus, "geoError", "scanStatus",
                         dims, missingVal, _geoError, units)) {
    cerr << "WARNING - GpmHdf5ToMdv::_readQcFlags()" << endl;
    cerr << "  Cannot read geoError flag variable" << endl;
  }
  if (hdf5.readSi32Array(scanStatus, "geoWarning", "scanStatus",
                         dims, missingVal, _geoWarning, units)) {
    cerr << "WARNING - GpmHdf5ToMdv::_readQcFlags()" << endl;
    cerr << "  Cannot read geoWarning flag variable" << endl;
  }
  if (hdf5.readSi32Array(scanStatus, "limitErrorFlag", "scanStatus",
                         dims, missingVal, _limitErrorFlag, units)) {
    cerr << "WARNING - GpmHdf5ToMdv::_readQcFlags()" << endl;
    cerr << "  Cannot read limitErrorFlag variable" << endl;
    return -1;
  }
  if (hdf5.readSi32Array(scanStatus, "missing", "scanStatus",
                         dims, missingVal, _missingScan, units)) {
    cerr << "WARNING - GpmHdf5ToMdv::_readQcFlags()" << endl;
    cerr << "  Cannot read missing flag variable" << endl;
    return -1;
  }

  
  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "====>> Reading QC flags <<====" << endl;
    for (size_t ii = 0; ii < _times.size(); ii++) {
      cerr << "  ii, dQ, dW, geoE, geoW, limE, miss: "
           << ii << ", "
           << _dataQuality[ii] << ", "
           << _dataWarning[ii] << ", "
           << _geoError[ii] << ", "
           << _geoWarning[ii] << ", "
           << _limitErrorFlag[ii] << ", "
           << _missingScan[ii] << endl;
    }
  }
  
  return 0;

}

//////////////////////////////////////////////
// read the lat/lon arrays

int GpmHdf5ToMdv::_readLatLon(Group &ns)
  
{
  
  Hdf5xx hdf5;

  // read Latitude
  
  vector<size_t> latDims;
  string latUnits;
  if (hdf5.readFl64Array(ns, "Latitude", "NS",
                         latDims, _missingLat, _lats, latUnits)) {
    cerr << "ERROR - GpmHdf5ToMdv::_readLatLon()" << endl;
    cerr << "  Cannot read Latitude variable" << endl;
    return -1;
  }

  // read Longitude
  
  vector<size_t> lonDims;
  string lonUnits;
  if (hdf5.readFl64Array(ns, "Longitude", "NS",
                         lonDims, _missingLon, _lons, lonUnits)) {
    cerr << "ERROR - GpmHdf5ToMdv::_readLatLon()" << endl;
    cerr << "  Cannot read Longitude variable" << endl;
    return -1;
  }

  // check dimensions for consistency
  
  if (latDims.size() != 2 || lonDims.size() != 2) {
    cerr << "ERROR - GpmHdf5ToMdv::_readLatLon()" << endl;
    cerr << "  Latitude/Longitude must have 2 dimensions" << endl;
    cerr << "  latDims.size(): " << latDims.size() << endl;
    cerr << "  lonDims.size(): " << lonDims.size() << endl;
    return -1;
  }
  if (latDims[0] != lonDims[0] || latDims[1] != lonDims[1]) {
    cerr << "ERROR - GpmHdf5ToMdv::_readLatLon()" << endl;
    cerr << "  Latitude/Longitude must have same dimensions" << endl;
    cerr << "  latDims[0]: " << latDims[0] << endl;
    cerr << "  lonDims[0]: " << lonDims[0] << endl;
    cerr << "  latDims[1]: " << latDims[1] << endl;
    cerr << "  lonDims[1]: " << lonDims[1] << endl;
    return -1;
  }
  _nScans = latDims[0];
  _nRays = latDims[1];

  _latLons.clear();
  _minLat = 90.0;
  _maxLat = -90.0;
  _minLon = 360.0;
  _maxLon = -360.0;
  for (size_t iscan = 0; iscan < _nScans; iscan++) {
    vector<LatLonPt_t> pts;
    for (size_t iray = 0; iray < _nRays; iray++) {
      size_t ipt = iscan * _nRays + iray;
      LatLonPt_t pt;
      pt.lat = _lats[ipt];
      pt.lon = _lons[ipt];
      pts.push_back(pt);
      if (pt.lat != _missingLat) {
        _minLat = min(_minLat, pt.lat);
        _maxLat = max(_maxLat, pt.lat);
      }
      if (pt.lon != _missingLon) {
        _minLon = min(_minLon, pt.lon);
        _maxLon = max(_maxLon, pt.lon);
      }
    } // iray
    _latLons.push_back(pts);
  } // iscan

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "====>> Reading lat/lon <<====" << endl;
    cerr << "  missingLat: " << _missingLat << endl;
    cerr << "  missingLon: " << _missingLon << endl;
    cerr << "  minLat, maxLat: " << _minLat << ", " << _maxLat << endl;
    cerr << "  minLon, maxLon: " << _minLon << ", " << _maxLon << endl;
    cerr << "  nScans, nRays: " << _nScans << ", " << _nRays << endl;
    if (_params.debug >= Params::DEBUG_EXTRA) {
      for (size_t iscan = 0; iscan < _nScans; iscan++) {
        for (size_t iray = 0; iray < _nRays; iray++) {
          cerr << "  iscan, iray, lat, lon: "
               << iscan << ", "
               << iray << ", "
               << _latLons[iscan][iray].lat << ", "
               << _latLons[iscan][iray].lon << endl;
        } // iray
      } // iscan
    }
  }

  // set up (x, y) grid details

  _dx = _params.output_grid.dLon;
  _dy = _params.output_grid.dLat;

  if (_params.set_output_grid_limits_from_data) {
    _minx = ((int) floor(_minLon / _dx) - 5) * _dx;
    _miny = ((int) floor(_minLat / _dy) - 5) * _dy;
    _nx = (_maxLon - _minLon) / _dx + 10;
    _ny = (_maxLat - _minLat) / _dy + 10;
  } else {
    _nx = _params.output_grid.nLon;
    _ny = _params.output_grid.nLat;
    _minx = _params.output_grid.minLon;
    _miny = _params.output_grid.minLat;
  }

  _maxx = _minx + _nx * _dx;
  _maxy = _miny + _ny * _dy;
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "====>> Output grid details <<====" << endl;
    cerr << "  _nx, _ny: " << _nx << ", " << _ny << endl;
    cerr << "  _dx, _dy: " << _dx << ", " << _dy << endl;
    cerr << "  _minx, _miny: " << _minx << ", " << _miny << endl;
    cerr << "  _maxx, _maxy: " << _maxx << ", " << _maxy << endl;
  }

  return 0;

}

//////////////////////////////////////////////
// read the refectivity

int GpmHdf5ToMdv::_readReflectivity(Group &ns)
  
{
  
  Hdf5xx hdf5;

  // read Latitude
  
  vector<size_t> dbzDims;
  
  Group slv(ns.openGroup("SLV"));
  if (hdf5.readFl32Array(slv, "zFactorCorrected", "NS/SLV",
                         dbzDims, _missingDbz, _dbzVals, _dbzUnits)) {
    cerr << "ERROR - GpmHdf5ToMdv::_readReflectivity()" << endl;
    cerr << "  Cannot read zFactorCorrected variable" << endl;
    return -1;
  }

  // check dimensions for consistency
  
  if (dbzDims.size() != 3) {
    cerr << "ERROR - GpmHdf5ToMdv::_readReflectivity()" << endl;
    cerr << "  zFactorCorrected must have 3 dimensions" << endl;
    cerr << "  dbzDims.size(): " << dbzDims.size() << endl;
    return -1;
  }
  if (dbzDims[0] != _nScans || dbzDims[1] != _nRays) {
    cerr << "ERROR - GpmHdf5ToMdv::_readReflectivity()" << endl;
    cerr << "  DBZ dimensions must match nScans and nRays" << endl;
    cerr << "  dbzDims[0]: " << dbzDims[0] << endl;
    cerr << "  dbzDims[1]: " << dbzDims[1] << endl;
    cerr << "  _nScans: " << _nScans << endl;
    cerr << "  _nRays: " << _nRays << endl;
    return -1;
  }

  _nGates = dbzDims[2];

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "====>> Read DBZ <<====" << endl;
    cerr << "  nScans, nRays, nGates: " 
         << _nScans << ", " << _nRays << ", " << _nGates << endl;
    cerr << "  missingDbz: " << _missingDbz << endl;
    if (_params.debug >= Params::DEBUG_EXTRA) {
      for (size_t iscan = 0; iscan < _nScans; iscan++) {
        for (size_t iray = 0; iray < _nRays; iray++) {
          for (size_t igate = 0; igate < _nGates; igate++) {
            size_t ipt = iscan * _nRays * _nGates + iray * _nGates + igate;
            if (_dbzVals[ipt] != _missingDbz) {
              cerr << "  iscan, iray, igate, dbz: "
                   << iscan << ", "
                   << iray << ", "
                   << igate << ", "
                   << _dbzVals[ipt] << endl;
            }
          } // igate
        } // iray
      } // iscan
    } // extra
  } // verbose
  
  return 0;

}

/////////////////////////////////////////////////
// Set the master header from the NCF file
//
// Returns 0 on success, -1 on failure

int GpmHdf5ToMdv::_setMasterHeader(DsMdvx &mdvx, int itime)

{

  mdvx.clearMasterHeader();

  // time

  if (_params.debug) {
    cerr << "===========================================" << endl;
    cerr << "Found data set at start time: " << _startTime.asString(3) << endl;
  }

  mdvx.setBeginTime(_startTime.utime());
  mdvx.setEndTime(_endTime.utime());
  
  // data collection type
  
  mdvx.setDataCollectionType(Mdvx::DATA_MEASURED);

  // data set name, source and info

  mdvx.setDataSetName(_params.data_set_name);
  mdvx.setDataSetSource("NASA-GPM");
  mdvx.setDataSetInfo(_history.c_str());

  return 0;

}

/////////////////////////////////////////////////
// Add the data fields
//
// Returns 0 on success, -1 on failure

int GpmHdf5ToMdv::_addDataFields(DsMdvx &mdvx, int itime)

{

#ifdef JUNK
  
  for (int ivar = 0; ivar < _ncFile->num_vars(); ivar++) {

    Nc3Var *var = _ncFile->get_var(ivar);
    
    if (var->get_dim(0) != _timeDim) {
      continue;
    }

    bool xySwapped = false;
    if (_zDim) {
      if (var->num_dims() < 4) {
        continue;
      }
      if (var->get_dim(1) != _zDim) {
        continue;
      }
      if (var->get_dim(2) == _yDim && var->get_dim(3) == _xDim) {
        xySwapped = false;
      } else if (var->get_dim(3) == _yDim && var->get_dim(2) == _xDim) {
        xySwapped = true;
      } else {
        continue;
      }
    } else {
      if (var->num_dims() < 3) {
        continue;
      }
      if (var->get_dim(1) == _yDim && var->get_dim(2) == _xDim) {
        xySwapped = false;
      } else if (var->get_dim(2) == _yDim && var->get_dim(1) == _xDim) {
        xySwapped = true;
      } else {
        continue;
      }
    }

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Found var:" << endl;
      cerr << "  Name: " << var->name() << endl;
      cerr << "  Is valid: " << var->is_valid() << endl;
      cerr << "  N dims: " << var->num_dims() << endl;
    }

    _addDataField(var, mdvx, itime, xySwapped);

  } // ivar

#endif
  
  return 0;

}

/////////////////////////////////////////////////
// Add the data field
//
// Returns 0 on success, -1 on failure

#ifdef JUNK

int GpmHdf5ToMdv::_addDataField(Nc3Var *var, DsMdvx &mdvx,
                                int itime, bool xySwapped)

{

  Nc3Att *missingAtt = var->get_att("missing_value");
  if (missingAtt == NULL) {
    missingAtt = var->get_att("_FillValue");
    if (missingAtt == NULL) {
      cerr << "ERROR - GpmHdf5ToMdv::_addDataField" << endl;
      cerr << "  Cannot find missing_value of _FillValue attribute" << endl;
      cerr << "  field name: " << var->name() << endl;
      return -1;
    }
  }

  // set npoints, allocate float values array

  int npts = _nz * _ny * _nx;

  TaArray<float> vals_;
  float *vals = vals_.alloc(npts);

  // set current location based on time

  if (_zDim) {
    var->set_cur(itime, 0, 0, 0);
  } else {
    var->set_cur(itime, 0, 0);
  }

  if (var->type() == NC_FLOAT) {

    TaArray<float> fvals_;
    float *fvals = fvals_.alloc(npts);
    
    // read data

    int iret = 0;
    if (_zDim) {
      if (xySwapped) {
        iret = var->get(fvals, 1, _nz, _nx, _ny);
      } else {
        iret = var->get(fvals, 1, _nz, _ny, _nx);
      }
    } else {
      if (xySwapped) {
        iret = var->get(fvals, 1, _nx, _ny);
      } else {
        iret = var->get(fvals, 1, _ny, _nx);
      }
    }

    if (iret == 0) {
      cerr << "ERROR - GpmHdf5ToMdv::_addDataField" << endl;
      cerr << "  Cannot get data from input netcdf variable" << endl;
      cerr << "  field name: " << var->name() << endl;
      cerr << _ncErr->get_errmsg() << endl;
      return -1;
    }

    // save data
    
    float missing = missingAtt->as_float(0);
    for (int ii = 0; ii < npts; ii++) {
      if (isnan(fvals[ii]) || fvals[ii] == missing) {
        vals[ii] = _missingFloat;
      } else {
        vals[ii] = fvals[ii];
      }
    }

  } else if (var->type() == NC_DOUBLE) {
    
    TaArray<double> dvals_;
    double *dvals = dvals_.alloc(npts);

    // read data

    int iret = 0;
    if (_zDim) {
      if (xySwapped) {
        iret = var->get(dvals, 1, _nz, _nx, _ny);
      } else {
        iret = var->get(dvals, 1, _nz, _ny, _nx);
      }
    } else {
      if (xySwapped) {
        iret = var->get(dvals, 1, _nx, _ny);
      } else {
        iret = var->get(dvals, 1, _ny, _nx);
      }
    }
    if (iret == 0) {
      cerr << "ERROR - GpmHdf5ToMdv::_addDataField" << endl;
      cerr << "  Cannot get data from input netcdf variable" << endl;
      cerr << "  field name: " << var->name() << endl;
      cerr << _ncErr->get_errmsg() << endl;
      return -1;
    }

    // save data
    
    double missing = missingAtt->as_double(0);
    for (int ii = 0; ii < npts; ii++) {
      if (isnan(dvals[ii]) || dvals[ii] == missing) {
        vals[ii] = _missingFloat;
      } else {
        vals[ii] = dvals[ii];
      }
    }

  } else {

    // for int fields, we need scale and offset

    double scale = 1.0;
    Nc3Att *scaleAtt = var->get_att("scale");
    if (scaleAtt == NULL) {
      scaleAtt = var->get_att("scale_factor");
    }
    if (scaleAtt == NULL) {
      cerr << "WARNING - GpmHdf5ToMdv::_addDataField" << endl;
      cerr << "  Cannot get scale for integer variable" << endl;
      cerr << "  field name: " << var->name() << endl;
      cerr << "  Setting scale to 1.0" << endl;
    } else {
      scale = scaleAtt->as_double(0);
      delete scaleAtt;
    }
      
    double offset = 0.0;
    Nc3Att *offsetAtt = var->get_att("offset");
    if (offsetAtt == NULL) {
      if (_params.debug) {
        cerr << "WARNING - GpmHdf5ToMdv::_addDataField" << endl;
        cerr << "  Cannot get offset for integer variable" << endl;
        cerr << "  field name: " << var->name() << endl;
        cerr << "  setting to 0" << endl;
      }
    } else {
      offset = offsetAtt->as_double(0);
      delete offsetAtt;
    }
    
    if (var->type() == NC_INT) {
      
      TaArray<int> ivals_;
      int *ivals = ivals_.alloc(npts);
      
      // read data
      
      int iret = 0;
      if (_zDim) {
        if (xySwapped) {
          iret = var->get(ivals, 1, _nz, _nx, _ny);
        } else {
          iret = var->get(ivals, 1, _nz, _ny, _nx);
        }
      } else {
        if (xySwapped) {
          iret = var->get(ivals, 1, _nx, _ny);
        } else {
          iret = var->get(ivals, 1, _ny, _nx);
        }
      }
      if (iret == 0) {
        cerr << "ERROR - GpmHdf5ToMdv::_addDataField" << endl;
        cerr << "  Cannot get data from input netcdf variable" << endl;
        cerr << "  field name: " << var->name() << endl;
        cerr << _ncErr->get_errmsg() << endl;
        return -1;
      }

      // save data

      int missing = missingAtt->as_int(0);
      for (int ii = 0; ii < npts; ii++) {
        if (ivals[ii] == missing) {
          vals[ii] = _missingFloat;
        } else {
          vals[ii] = ivals[ii] * scale + offset;
        }
      }

    } else if (var->type() == NC_SHORT) {
      
      TaArray<short> svals_;
      short *svals = svals_.alloc(npts);
      
      // read data
      
      int iret = 0;
      if (_zDim) {
        if (xySwapped) {
          iret = var->get(svals, 1, _nz, _nx, _ny);
        } else {
          iret = var->get(svals, 1, _nz, _ny, _nx);
        }
      } else {
        if (xySwapped) {
          iret = var->get(svals, 1, _nx, _ny);
        } else {
          iret = var->get(svals, 1, _ny, _nx);
        }
      }
      if (iret == 0) {
        cerr << "ERROR - GpmHdf5ToMdv::_addDataField" << endl;
        cerr << "  Cannot get data from input netcdf variable" << endl;
        cerr << "  field name: " << var->name() << endl;
        cerr << _ncErr->get_errmsg() << endl;
        return -1;
      }

      // save data

      short missing = missingAtt->as_short(0);
      for (int ii = 0; ii < npts; ii++) {
        if (svals[ii] == missing) {
          vals[ii] = _missingFloat;
        } else {
          vals[ii] = svals[ii] * scale + offset;
        }
      }

    } else if (var->type() == NC_BYTE) {
      

      TaArray<ncbyte> bvals_;
      ncbyte *bvals = bvals_.alloc(npts);
      TaArray<unsigned char> uvals_;
      unsigned char *uvals = uvals_.alloc(npts);
      
      // read data
      
      int iret = 0;
      if (_zDim) {
        if (xySwapped) {
          iret = var->get(bvals, 1, _nz, _nx, _ny);
        } else {
          iret = var->get(bvals, 1, _nz, _ny, _nx);
        }
      } else {
        if (xySwapped) {
          iret = var->get(bvals, 1, _nx, _ny);
        } else {
          iret = var->get(bvals, 1, _ny, _nx);
        }
      }
      if (iret == 0) {
        cerr << "ERROR - GpmHdf5ToMdv::_addDataField" << endl;
        cerr << "  Cannot get data from input netcdf variable" << endl;
        cerr << "  field name: " << var->name() << endl;
        cerr << _ncErr->get_errmsg() << endl;
        return -1;
      }
      memcpy(uvals, bvals, npts);

      // save data
      
      ncbyte missing = missingAtt->as_ncbyte(0);
      for (int ii = 0; ii < npts; ii++) {
        if (bvals[ii] == missing) {
          vals[ii] = _missingFloat;
        } else {
          if (_params.treat_ncbyte_as_unsigned) {
            vals[ii] = (int) uvals[ii] * scale + offset;
          } else {
            vals[ii] = (int) bvals[ii] * scale + offset;
          }
        }
      }

    } // if (var->type() == NC_INT)

  } // if (var->type() == NC_FLOAT)

  // free up attribute

  delete missingAtt;

  // swap (x,y) if required

  if (xySwapped) {

    TaArray<float> tmpVals_;
    float *tmpVals = tmpVals_.alloc(npts);
    memcpy(tmpVals, vals, npts * sizeof(float));

    int nptsPlane = _ny * _nx;

    for (int iz = 0; iz < _nz; iz ++) {
      int planeOffset = iz * nptsPlane;
      for (int iy = 0; iy < _ny; iy ++) {
        for (int ix = 0; ix < _nx; ix ++) {
          float *src = tmpVals + planeOffset + ix * _ny + iy;
          float *dest = vals + planeOffset + iy * _nx + ix;
          *dest = *src;
        } // ix
      } // iy
    } // iz
    
  } // if (xySwapped) 

  // reverse y order if it was in reverse order in the file

  if (_yIsReversed) {

    TaArray<float> tmpVals_;
    float *tmpVals = tmpVals_.alloc(npts);
    memcpy(tmpVals, vals, npts * sizeof(float));

    int nptsPlane = _ny * _nx;

    for (int iz = 0; iz < _nz; iz ++) {
      int planeOffset = iz * nptsPlane;
      for (int iy = 0; iy < _ny; iy ++) {
        float *src = tmpVals + planeOffset + _nx * (_ny - 1 - iy);
        float *dest = vals + planeOffset + _nx * iy;
        memcpy(dest, src, _nx * sizeof(float));
      } // iy
    } // iz
    
  } // if (_yIsReversed) 
  
  // get field name and units

  string fieldName(var->name());

  string units;
  Nc3Att *unitsAtt = var->get_att("units");
  if (unitsAtt != NULL) {
    units = unitsAtt->as_string(0);
    delete unitsAtt;
  }

  string longName(fieldName);
  Nc3Att *longNameAtt = var->get_att("long_name");
  if (longNameAtt != NULL) {
    longName = longNameAtt->as_string(0);
    delete longNameAtt;
  }

  // create fields and add to mdvx object

  if (_params.input_xy_is_latlon &&
      _params.resample_latlon_onto_regular_grid &&
      (!_dxIsConstant || !_dyIsConstant)) {
    
    // create the field from the remapped
    
    MdvxField *field =
      _createRegularLatlonField(fieldName, longName, units, vals);
    // add to Mdvx, which takes over ownership
    mdvx.addField(field);
    
  } else {
  
    // create the field from the netcdf array
    MdvxField *field = _createMdvxField(fieldName, longName, units,
                                        _nx, _ny, _nz,
                                        _minx, _miny, _minz,
                                        _dx, _dy, _dz,
                                        vals);
    // add to Mdvx, which takes over ownership
    mdvx.addField(field);
    
  }
  
  return 0;

}

#endif

///////////////////////////////
// Create an Mdvx field

MdvxField *GpmHdf5ToMdv::_createMdvxField
  (const string &fieldName,
   const string &longName,
   const string &units,
   int nx, int ny, int nz,
   double minx, double miny, double minz,
   double dx, double dy, double dz,
   const float *vals)

{

  // check max levels

  if (nz > MDV_MAX_VLEVELS) {
    nz = MDV_MAX_VLEVELS;
  }

  // set up MdvxField headers

  Mdvx::field_header_t fhdr;
  MEM_zero(fhdr);
  
  // _inputProj.syncToFieldHdr(fhdr);

  fhdr.compression_type = Mdvx::COMPRESSION_NONE;
  fhdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  fhdr.scaling_type = Mdvx::SCALING_NONE;
  
  fhdr.native_vlevel_type = Mdvx::VERT_TYPE_Z;
  fhdr.vlevel_type = Mdvx::VERT_TYPE_Z;
  fhdr.dz_constant = false;
  fhdr.data_dimension = 3;

  fhdr.bad_data_value = _missingFloat;
  fhdr.missing_data_value = _missingFloat;
  
  fhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  fhdr.data_element_nbytes = sizeof(fl32);
  fhdr.volume_size = nx * ny * nz * sizeof(fl32);
  
  fhdr.nx = nx;
  fhdr.ny = ny;
  fhdr.nz = nz;

  fhdr.grid_minx = minx;
  fhdr.grid_miny = miny;
  fhdr.grid_minz = minz;

  fhdr.grid_dx = dx;
  fhdr.grid_dy = dy;
  fhdr.grid_dz = dz;
  
  Mdvx::vlevel_header_t vhdr;
  MEM_zero(vhdr);
  
  for (int ii = 0; ii < nz; ii++) {
    vhdr.type[ii] = Mdvx::VERT_TYPE_Z;
    vhdr.level[ii] = _zArray[ii];
  }

  // create MdvxField object
  // converting data to encoding and compression types

  MdvxField *field = new MdvxField(fhdr, vhdr, vals);
  field->convertType
    ((Mdvx::encoding_type_t) _params.output_encoding_type, Mdvx::COMPRESSION_GZIP);

  // set names etc
  
  field->setFieldName(fieldName.c_str());
  field->setFieldNameLong(longName.c_str());
  field->setUnits(units.c_str());
  field->setTransform("");

  return field;

}
  
////////////////////////////////////////////////////////////////
// Create a field remapped from lat/lon onto regular grid

MdvxField *GpmHdf5ToMdv::_createRegularLatlonField
  (const string &fieldName,
   const string &longName,
   const string &units,
   const float *vals)

{

  if (_params.debug) {
    cerr << "==== Creating regular lat/lon grid ====" << endl;
  }

  // compute min dlat and dlon

  double dLon = 1.0e6;
  for (int ix = _ixValidStart; ix < _ixValidEnd; ix++) {
    double lonVal = fabs(_xArray[ix+1] - _xArray[ix]);
    if (lonVal < dLon) {
      dLon = lonVal;
    }
  }

  double dLat = 1.0e6;
  for (int iy = _iyValidStart; iy < _iyValidEnd; iy++) {
    double latVal = fabs(_yArray[iy+1] - _yArray[iy]);
    if (latVal < dLat) {
      dLat = latVal;
    }
  }

  double minLon = _xArray[_ixValidStart];
  double maxLon = _xArray[_ixValidEnd];
  double rangeLon = maxLon - minLon;

  double minLat = _yArray[_iyValidStart];
  double maxLat = _yArray[_iyValidEnd];
  double rangeLat = maxLat - minLat;

  int nLon = (int) (rangeLon / dLon) + 1;
  int nLat = (int) (rangeLat / dLat) + 1;

  if (_params.debug) {
    cerr << "==>> minLon, maxLon, rangeLon, dLon, nLon: "
         << minLon << ", " << maxLon << ", "
         << rangeLon << ", " << dLon << ", " << nLon << endl;
    cerr << "==>> minLat, maxLat, rangeLat, dLat, nLat: "
         << minLat << ", " << maxLat << ", "
         << rangeLat << ", " << dLat << ", " << nLat << endl;
  }

  // create resampled grid

  int nResamp = nLon * nLat;
  TaArray<fl32> resampled_;
  fl32 *resampled = resampled_.alloc(nResamp);

  // compute resampling indices

  vector<int> latIndex;
  double latitude = minLat;
  for (int ilat = 0; ilat < nLat; ilat++, latitude += dLat) {
    int index = _getClosestLatIndex(latitude, dLat / 2.0);
    latIndex.push_back(index);
  }

  vector<int> lonIndex;
  double longitude = minLon;
  for (int ilon = 0; ilon < nLon; ilon++, longitude += dLon) {
    int index = _getClosestLonIndex(longitude, dLon / 2.0);
    lonIndex.push_back(index);
  }
  
  // load the resampled grid

  fl32 *resamp = resampled;
  for (int ilat = 0; ilat < nLat; ilat++) {
    int latIx = latIndex[ilat];
    for (int ilon = 0; ilon < nLon; ilon++, resamp++) {
      int lonIx = lonIndex[ilon];
      if (latIx < 0 || lonIx < 0) {
        *resamp = _missingFloat;
      } else {
        int valIndex = latIx * _nx + lonIx;
        *resamp = vals[valIndex];
      }
    } // ilon
  } // ilat

  // create the field

  // set up MdvxField headers

  Mdvx::field_header_t fhdr;
  MEM_zero(fhdr);

  fhdr.proj_type = Mdvx::PROJ_LATLON;
  fhdr.proj_origin_lat = minLat + rangeLat / 2.0;
  fhdr.proj_origin_lon = minLon + rangeLon / 2.0;

  fhdr.compression_type = Mdvx::COMPRESSION_NONE;
  fhdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  fhdr.scaling_type = Mdvx::SCALING_NONE;
  
  fhdr.native_vlevel_type = Mdvx::VERT_TYPE_Z;
  fhdr.vlevel_type = Mdvx::VERT_TYPE_Z;
  fhdr.dz_constant = false;
  fhdr.data_dimension = 3;

  fhdr.bad_data_value = _missingFloat;
  fhdr.missing_data_value = _missingFloat;
  
  fhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  fhdr.data_element_nbytes = sizeof(fl32);
  fhdr.volume_size = nLon * nLat * _nz * sizeof(fl32);
  
  fhdr.nx = nLon;
  fhdr.ny = nLat;
  fhdr.nz = _nz;

  fhdr.grid_minx = minLon;
  fhdr.grid_miny = minLat;
  fhdr.grid_minz = _minz;

  fhdr.grid_dx = dLon;
  fhdr.grid_dy = dLat;
  fhdr.grid_dz = _dz;
  
  Mdvx::vlevel_header_t vhdr;
  MEM_zero(vhdr);
  
  for (int ii = 0; ii < _nz; ii++) {
    vhdr.type[ii] = Mdvx::VERT_TYPE_Z;
    vhdr.level[ii] = _zArray[ii];
  }

  // create MdvxField object
  // converting data to encoding and compression types
  
  MdvxField *field = new MdvxField(fhdr, vhdr, resampled);
  field->convertType
    ((Mdvx::encoding_type_t) _params.output_encoding_type, Mdvx::COMPRESSION_GZIP);

  // set names etc
  
  field->setFieldName(fieldName.c_str());
  field->setFieldNameLong(longName.c_str());
  field->setUnits(units.c_str());
  field->setTransform("");

  return field;

}
  
//////////////////////////////////////////////////
// get the closest lat index to a given latitude
// within the tolerance
// return -1 if outside grid by more than tolerance

int GpmHdf5ToMdv::_getClosestLatIndex(double latitude, double tolerance)

{

  if (latitude < _yArray[_iyValidStart]) {
    if (fabs(latitude - _yArray[_iyValidStart]) <= tolerance) {
      return _iyValidStart;
    } else {
      return -1;
    }
  }

  if (latitude > _yArray[_iyValidEnd]) {
    if (fabs(latitude - _yArray[_iyValidEnd]) <= tolerance) {
      return _iyValidEnd;
    } else {
      return -1;
    }
  }

  for (int iy = _iyValidStart; iy < _iyValidEnd; iy++) {
    if (latitude >= _yArray[iy] && latitude <= _yArray[iy+1]) {
      if (fabs(latitude - _yArray[iy]) < fabs(latitude - _yArray[iy+1])) {
        return iy;
      } else {
        return iy + 1;
      }
    }
  }

  return -1;

}


//////////////////////////////////////////////////
// get the closest lon index to a given longitude
// within the tolerance
// return -1 if outside grid by more than tolerance

int GpmHdf5ToMdv::_getClosestLonIndex(double longitude, double tolerance)

{

  if (longitude < _xArray[_ixValidStart]) {
    if (fabs(longitude - _xArray[_ixValidStart]) <= tolerance) {
      return _ixValidStart;
    } else {
      return -1;
    }
  }

  if (longitude > _xArray[_ixValidEnd]) {
    if (fabs(longitude - _xArray[_ixValidEnd]) <= tolerance) {
      return _ixValidEnd;
    } else {
      return -1;
    }
  }

  for (int ix = _ixValidStart; ix < _ixValidEnd; ix++) {
    if (longitude >= _xArray[ix] && longitude <= _xArray[ix+1]) {
      if (fabs(longitude - _xArray[ix]) < fabs(longitude - _xArray[ix+1])) {
        return ix;
      } else {
        return ix + 1;
      }
    }
  }

  return -1;

}

