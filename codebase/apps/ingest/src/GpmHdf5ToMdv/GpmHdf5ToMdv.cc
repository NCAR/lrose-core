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

    string fileHeader = _readStringAttribute(root, "FileHeader", "RootAttr");
    string fileInfo = _readStringAttribute(root, "FileInfo", "RootAttr");
    string inputRecord = _readStringAttribute(root, "InputRecord", "RootAttr");
    string jaxaInfo = _readStringAttribute(root, "JAXAInfo", "RootAttr");
    string navigationRecord = _readStringAttribute(root, "NavigationRecord", "RootAttr");
    string history = _readStringAttribute(root, "history", "RootAttr");
    
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "FileHeader: " << endl << "===================" << endl
           << fileHeader << "===================" << endl;
      cerr << "FileInfo: " << endl << "===================" << endl
           << fileInfo << "===================" << endl;
      cerr << "InputRecord: " << endl << "===================" << endl
           << inputRecord << "===================" << endl;
      cerr << "JAXAInfo: " << endl << "===================" << endl
           << jaxaInfo << "===================" << endl;
      cerr << "NavigationRecord: " << endl << "===================" << endl
           << navigationRecord << "===================" << endl;
      cerr << "history: " << endl << "===================" << endl
           << history << "===================" << endl;
    }

    // open the NS group
    
    Group ns(root.openGroup("NS"));
    if (_readGroupNs(ns)) {
      return -1;
    }

    // set the number of sweeps

    // if (_getNSweeps(root)) {
    //   _addErrStr("ERROR - OdimHdf5RadxFile::readFromPath");
    //   _addErrStr("  path: ", path);
    //   return -1;
    // }

    // read the root how, what and where groups
    
    // if (_readRootSubGroups(root)) {
    //   _addErrStr("ERROR - OdimHdf5RadxFile::readFromPath");
    //   _addErrStr("  path: ", path);
    //   return -1;
    // }

    // read the sweeps
    
    // for (int isweep = 0; isweep < _nSweeps; isweep++) {
    //   if (_readSweep(root, isweep)) {
    //     return -1;
    //   }
    //   _statusXml += _sweepStatusXml;
    // }

  } // try
  
  catch (H5x::Exception &e) {
    // _addErrStr("ERROR - reading GPM HDF5 file");
    // _addErrStr(e.getDetailMsg());
    return -1;
  }

  // finalize status xml

  // _setStatusXml();
  // _statusXml += RadxXml::writeEndTag("Status", 0);

  // load the data into the read volume
  
  // if (_finalizeReadVolume()) {
  //   return -1;
  // }
  
  // set format as read

  // _fileFormat = FILE_FORMAT_ODIM_HDF5;

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

  if (_readTime(ns)) {
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
// read the scan time

int GpmHdf5ToMdv::_readTime(Group &ns)
  
{

  Group scanTime(ns.openGroup("ScanTime"));
  vector<size_t> dims;
  string units;
  NcxxPort::si32 missingVal;
  
  vector<NcxxPort::si32> years, months, days, hours, mins, secs, msecs;
  Hdf5xx hdf5;
  if (hdf5.readSi32Array(scanTime, "Year", "ScanTime",
                         dims, missingVal, years, units)) {
    cerr << "ERROR - GpmHdf5ToMdv::_readTime()" << endl;
    cerr << "  Cannot read Year variable" << endl;
    return -1;
  }
  if (hdf5.readSi32Array(scanTime, "Month", "ScanTime",
                         dims, missingVal, months, units)) {
    cerr << "ERROR - GpmHdf5ToMdv::_readTime()" << endl;
    cerr << "  Cannot read Month variable" << endl;
    return -1;
  }
  if (hdf5.readSi32Array(scanTime, "DayOfMonth", "ScanTime",
                         dims, missingVal, days, units)) {
    cerr << "ERROR - GpmHdf5ToMdv::_readTime()" << endl;
    cerr << "  Cannot read DayOfMonth variable" << endl;
    return -1;
  }
  if (hdf5.readSi32Array(scanTime, "Hour", "ScanTime",
                         dims, missingVal, hours, units)) {
    cerr << "ERROR - GpmHdf5ToMdv::_readTime()" << endl;
    cerr << "  Cannot read Hour variable" << endl;
    return -1;
  }
  if (hdf5.readSi32Array(scanTime, "Minute", "ScanTime",
                         dims, missingVal, mins, units)) {
    cerr << "ERROR - GpmHdf5ToMdv::_readTime()" << endl;
    cerr << "  Cannot read Minute variable" << endl;
    return -1;
  }
  if (hdf5.readSi32Array(scanTime, "Second", "ScanTime",
                         dims, missingVal, secs, units)) {
    cerr << "ERROR - GpmHdf5ToMdv::_readTime()" << endl;
    cerr << "  Cannot read Second variable" << endl;
    return -1;
  }
  if (hdf5.readSi32Array(scanTime, "MilliSecond", "ScanTime",
                         dims, missingVal, msecs, units)) {
    cerr << "ERROR - GpmHdf5ToMdv::_readTime()" << endl;
    cerr << "  Cannot read MilliSecond variable" << endl;
    return -1;
  }

  _times.clear();
  for (size_t ii = 0; ii < years.size(); ii++) {
    DateTime dtime(years[ii], months[ii], days[ii],
                   hours[ii], mins[ii], secs[ii], msecs[ii] / 1000.0);
    _times.push_back(dtime);
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "====>> Reading scan times <<====" << endl;
    cerr << "nTimes: " << _times.size() << endl;
    for (size_t ii = 0; ii < days.size(); ii++) {
      cerr << "  ii, time: " << ii << ", " << _times[ii].asString(3) << endl;
    }
  }
  
  return 0;

}

/////////////////////////////////////////////
// initialize the input projection

void GpmHdf5ToMdv::_initInputProjection()

{
  
  if (_params.input_projection == Params::PROJ_LATLON) {
    double midLon = _minx + _nx * _dx / 2.0;
    _inputProj.initLatlon(midLon);
  } else if (_params.input_projection == Params::PROJ_FLAT) {
    _inputProj.initFlat(_params.input_proj_origin_lat,
                        _params.input_proj_origin_lon,
                        _params.input_proj_rotation);
  } else if (_params.input_projection == Params::PROJ_LAMBERT_CONF) {
    _inputProj.initLambertConf(_params.input_proj_origin_lat,
                               _params.input_proj_origin_lon,
                               _params.input_proj_lat1,
                               _params.input_proj_lat2);
  } else if (_params.input_projection == Params::PROJ_POLAR_STEREO) {
    Mdvx::pole_type_t poleType = Mdvx::POLE_NORTH;
    if (!_params.input_proj_pole_is_north) {
      poleType = Mdvx::POLE_SOUTH;
    }
    _inputProj.initPolarStereo(_params.input_proj_origin_lat,
                               _params.input_proj_origin_lon,
                               _params.input_proj_tangent_lon,
                               poleType,
                               _params.input_proj_central_scale);
  } else if (_params.input_projection == Params::PROJ_OBLIQUE_STEREO) {
    _inputProj.initObliqueStereo(_params.input_proj_origin_lat,
                                 _params.input_proj_origin_lon,
                                 _params.input_proj_tangent_lat,
                                 _params.input_proj_tangent_lon);
  } else if (_params.input_projection == Params::PROJ_MERCATOR) {
    _inputProj.initMercator(_params.input_proj_origin_lat,
                            _params.input_proj_origin_lon);
  } else if (_params.input_projection == Params::PROJ_TRANS_MERCATOR) {
    _inputProj.initTransMercator(_params.input_proj_origin_lat,
                                 _params.input_proj_origin_lon,
                                 _params.input_proj_central_scale);
  } else if (_params.input_projection == Params::PROJ_ALBERS) {
    _inputProj.initAlbers(_params.input_proj_origin_lat,
                          _params.input_proj_origin_lon,
                          _params.input_proj_lat1,
                          _params.input_proj_lat2);
  } else if (_params.input_projection == Params::PROJ_LAMBERT_AZIM) {
    _inputProj.initLambertAzim(_params.input_proj_origin_lat,
                               _params.input_proj_origin_lon);
  } else if (_params.input_projection == Params::PROJ_VERT_PERSP) {
    _inputProj.initVertPersp(_params.input_proj_origin_lat,
                             _params.input_proj_origin_lon,
                             _params.input_proj_persp_radius);
  }

  if (_params.debug) {
    cerr << "Input projection:" << endl;
    _inputProj.print(cerr);
  }
  
  return;

}

//////////////////////////////////////
// open netcdf file
//
// Returns 0 on success, -1 on failure

int GpmHdf5ToMdv::_openNc3File(const string &path)
  
{
  
  if (_ncFile) {
    _ncFile->close();
    delete _ncFile;
  }

  _ncFile = new Nc3File(path.c_str(), Nc3File::ReadOnly);

  // Check that constructor succeeded

  if (!_ncFile->is_valid()) {
    cerr << "ERROR - GpmHdf5ToMdv::_openNc3File" << endl;
    cerr << "  Opening file, path: " << path << endl;
    return 1;
  }
  
  // Change the error behavior of the netCDF C++ API by creating an
  // Nc3Error object. Until it is destroyed, this Nc3Error object will
  // ensure that the netCDF C++ API silently returns error codes
  // on any failure, and leaves any other error handling to the
  // calling program.
  
  _ncErr = new Nc3Error(Nc3Error::silent_nonfatal);

  if (_params.debug) {
    cerr << "Opened input file: " << path << endl;
  }
 
  return 0;


}

//////////////////////////////////////
// close netcdf file if open
// remove error object if it exists

void GpmHdf5ToMdv::_closeNc3File()
  
{
  
  // close file if open, delete ncFile
  
  if (_ncFile) {
    _ncFile->close();
    delete _ncFile;
    _ncFile = NULL;
  }

  if (_ncErr) {
    delete _ncErr;
    _ncErr = NULL;
  }

}

//////////////////////////////////
// Check that this is a valid file
//
// Returns 0 on success, -1 on failure

int GpmHdf5ToMdv::_loadMetaData()

{

  // dimensions

  _timeDim = _ncFile->get_dim(_params.netcdf_dim_time);
  if (_timeDim == NULL) {
    cerr << "ERROR - GpmHdf5ToMdv::_loadMetaData" << endl;
    cerr << "  time dimension missing: " << _params.netcdf_dim_time << endl;
    return -1;
  }
  _nTimes = _timeDim->size();

  if (strcmp(_params.netcdf_dim_z, "none") == 0) {
    _zDim = NULL;
    _nz = 1;
  } else {
    _zDim = _ncFile->get_dim(_params.netcdf_dim_z);
    if (_zDim == NULL) {
      cerr << "ERROR - GpmHdf5ToMdv::_loadMetaData" << endl;
      cerr << "  Z dimension missing: " << _params.netcdf_dim_z << endl;
      return -1;
    }
    _nz = _zDim->size();
  }
  
  _yDim = _ncFile->get_dim(_params.netcdf_dim_y);
  if (_yDim == NULL) {
    cerr << "ERROR - GpmHdf5ToMdv::_loadMetaData" << endl;
    cerr << "  Y dimension missing: " << _params.netcdf_dim_y << endl;
    return -1;
  }
  _ny = _yDim->size();
  
  _xDim = _ncFile->get_dim(_params.netcdf_dim_x);
  if (_xDim == NULL) {
    cerr << "ERROR - GpmHdf5ToMdv::_loadMetaData" << endl;
    cerr << "  Z dimension missing: " << _params.netcdf_dim_x << endl;
    return -1;
  }
  _nx = _xDim->size();

  // variables
  
  if (strcmp(_params.netcdf_var_base_time, "none") == 0) {
    _baseTimeVar = NULL;
  } else {
    _baseTimeVar = _ncFile->get_var(_params.netcdf_var_base_time);
    if (_baseTimeVar == NULL) {
      cerr << "ERROR - GpmHdf5ToMdv::_loadMetaData" << endl;
      cerr << "  base time var variable missing: " << _params.netcdf_var_base_time << endl;
      return -1;
    }
  }
  
  _timeOffsetVar = _ncFile->get_var(_params.netcdf_var_time_offset);
  if (_timeOffsetVar == NULL) {
    cerr << "ERROR - GpmHdf5ToMdv::_loadMetaData" << endl;
    cerr << "  time offset variable missing: " << _params.netcdf_var_time_offset << endl;
    return -1;
  }

  if (_zDim == NULL) {
    _zVar = NULL;
  } else {
    _zVar = _ncFile->get_var(_params.netcdf_var_z);
    if (_zVar == NULL) {
      cerr << "ERROR - GpmHdf5ToMdv::_loadMetaData" << endl;
      cerr << "  z variable missing: " << _params.netcdf_var_z << endl;
    return -1;
    }
  }

  _yVar = _ncFile->get_var(_params.netcdf_var_y);
  if (_yVar == NULL) {
    cerr << "ERROR - GpmHdf5ToMdv::_loadMetaData" << endl;
    cerr << "  y variable missing: " << _params.netcdf_var_y << endl;
    return -1;
  }

  _xVar = _ncFile->get_var(_params.netcdf_var_x);
  if (_xVar == NULL) {
    cerr << "ERROR - GpmHdf5ToMdv::_loadMetaData" << endl;
    cerr << "  x variable missing: " << _params.netcdf_var_x << endl;
    return -1;
  }

  // Z coord values

  _zArray = (float *) _zArray_.alloc(_nz);
  if (_zVar == NULL) {
    
    _zArray[0] = 0.0;
    
  } else {
    
    if (_zVar->get(_zArray, _nz) == 0) {
      cerr << "ERROR - GpmHdf5ToMdv::_loadMetaData" << endl;
      cerr << "  Cannot get z coords from var: " << _params.netcdf_var_z << endl;
      return -1;
    }
  
    // convert to km
    
    double zScale = 1.0;
    Nc3Att *zUnits = _zVar->get_att("units");
    if (zUnits != NULL) {
      string units = zUnits->as_string(0);
      if (units == "m" || units == "meters") {
        zScale = 0.001;
      }
      delete zUnits;
    }
    if (zScale != 1.0) {
      for (int ii = 0; ii < _zDim->size(); ii++) {
        _zArray[ii] *= zScale;
      }
    }

  }

  // Y coord values

  _yArray = (float *) _yArray_.alloc(_yDim->size());
  if (_yVar->get(_yArray, _yDim->size()) == 0) {
    cerr << "ERROR - GpmHdf5ToMdv::_loadMetaData" << endl;
    cerr << "  Cannot get y coords from var: " << _params.netcdf_var_y << endl;
    return -1;
  }

  // convert to km

  double yScale = 1.0;
  Nc3Att *yUnits = _yVar->get_att("units");
  if (yUnits != NULL) {
    string units = yUnits->as_string(0);
    if (units == "m" || units == "meters") {
      yScale = 0.001;
    }
    delete yUnits;
  }
  if (yScale != 1.0) {
    for (int ii = 0; ii < _yDim->size(); ii++) {
      _yArray[ii] *= yScale;
    }
  }

  // X coord values

  _xArray = (float *) _xArray_.alloc(_xDim->size());
  if (_xVar->get(_xArray, _xDim->size()) == 0) {
    cerr << "ERROR - GpmHdf5ToMdv::_loadMetaData" << endl;
    cerr << "  Cannot get x coords from var: " << _params.netcdf_var_x << endl;
    return -1;
  }

  // convert to km

  double xScale = 1.0;
  Nc3Att *xUnits = _xVar->get_att("units");
  if (xUnits != NULL) {
    string units = xUnits->as_string(0);
    if (units == "m" || units == "meters") {
      xScale = 0.001;
    }
    delete xUnits;
  }
  if (xScale != 1.0) {
    for (int ii = 0; ii < _xDim->size(); ii++) {
      _xArray[ii] *= xScale;
    }
  }
  
  // attributes
  
  Nc3Att *source = _ncFile->get_att("source");
  if (source != NULL) {
    _source = source->as_string(0);
    delete source;
  } else {
    _source = _params.data_set_source;
  }

  // data set info

  Nc3Att *history = _ncFile->get_att("history");
  if (history != NULL) {
    _history = history->as_string(0);
    delete history;
  } else {
    _history = _params.data_set_info;
  }

  // reverse the Y array?

  _yIsReversed = false;
  if (_ny >= 2) {
    int midNy = _ny / 2;
    double dyOrig = (_yArray[midNy+1] - _yArray[midNy]);
    if (dyOrig < 0) {
      _yIsReversed = true;
    }
  }

  if (_yIsReversed) {
    TaArray<float> tmpArray_;
    float *tmpArray = tmpArray_.alloc(_ny);
    memcpy(tmpArray, _yArray, _ny * sizeof(float));
    for (int ii = 0, jj = _ny - 1; ii < _ny; ii++, jj--) {
      _yArray[ii] = tmpArray[jj];
    }
  }

  // set up geometry, for now assuming constant dx and dy

  _minz = _zArray[0];
  _miny = _yArray[0];
  _minx = _xArray[0];

  _maxz = _zArray[_nz-1];
  _maxy = _yArray[_ny-1];
  _maxx = _xArray[_nx-1];

  if (_nz < 2) {
    _dz = 1.0;
  } else {
    _dz = (_zArray[_nz - 1] - _zArray[0]) / (_nz - 1.0);
  }
  
  if (_ny < 2) {
    _dy = 1.0;
  } else {
    _dy = (_yArray[_ny - 1] - _yArray[0]) / (_ny - 1.0);
  }
  
  if (_nx < 2) {
    _dx = 1.0;
  } else {
    _dx = (_xArray[_nx - 1] - _xArray[0]) / (_nx - 1.0);
  }

  _nxValid = _nx;
  _nyValid = _ny;
  _ixValidStart = 0;
  _ixValidEnd = _nx - 1;
  _iyValidStart = 0;
  _iyValidEnd = _ny - 1;

  // check we have a constant deltax and y

  _dxIsConstant = _checkDxIsConstant();
  _dyIsConstant = _checkDyIsConstant();
  
  // for file with latlon coords, find the valid end indices for the lat/lon coords
  
  if (_params.input_xy_is_latlon) {
    if (_findValidLatLonLimits()) {
      cerr << "ERROR - GpmHdf5ToMdv::_loadMetaData" << endl;
      cerr << "  Bad lat/lon values, cannot process" << endl;
      return -1;
    }
  }
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Input geometry:" << endl;
    cerr << "  nz: " << _nz << endl;
    cerr << "  ny: " << _ny << endl;
    cerr << "  nx: " << _nx << endl;
    cerr << "  minz: " << _minz << endl;
    cerr << "  miny: " << _miny << endl;
    cerr << "  minx: " << _minx << endl;
    cerr << "  dz: " << _dz << endl;
    cerr << "  dy: " << _dy << endl;
    cerr << "  yIsReversed: " << _yIsReversed << endl;
    cerr << "  dx: " << _dx << endl;
    cerr << "  dxIsConstant: " << (_dxIsConstant?"Y":"N") << endl;
    cerr << "  dyIsConstant: " << (_dyIsConstant?"Y":"N") << endl;
  }
  
  _initInputProjection();

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

  time_t baseTimeUtc = 0;
  if (_baseTimeVar) {
    baseTimeUtc = _baseTimeVar->as_int(0);
  } else {
    DateTime btime(_params.base_time_string);
    baseTimeUtc = btime.utime();
  }

  // check time units
  
  double offsetMult = 1.0; // secs
  Nc3Att *timeUnits = _timeOffsetVar->get_att("units");
  if (timeUnits != NULL) {
    string unitsStr = timeUnits->as_string(0);
    if (unitsStr.find("day") != string::npos) {
      offsetMult = 86400.0;
    } else if (unitsStr.find("hour") != string::npos) {
      offsetMult = 3600.0;
    } else if (unitsStr.find("min") != string::npos) {
      offsetMult = 60.0;
    }
    DateTime refTime;
    if (refTime.setFromW3c(unitsStr.c_str()) == 0) {
      baseTimeUtc = refTime.utime();
    }
    delete timeUnits;
  }
  
  double timeOffsetSecs = 0;
  if (offsetMult == 1.0) {
    timeOffsetSecs = _timeOffsetVar->as_double(itime);
  } else {
    double timeOffsetDays = _timeOffsetVar->as_double(itime);
    timeOffsetSecs = timeOffsetDays * offsetMult;
  }
  _validTime = baseTimeUtc + (time_t) (timeOffsetSecs + 0.5);
  mdvx.setValidTime(_validTime);

  if (_params.debug) {
    cerr << "===========================================" << endl;
    cerr << "Found data set at time: " << DateTime::strm(_validTime) << endl;
  }
  
  // data collection type
  
  mdvx.setDataCollectionType(Mdvx::DATA_MEASURED);

  // data set name, source and info

  mdvx.setDataSetName(_params.data_set_name);
  mdvx.setDataSetSource(_source.c_str());
  mdvx.setDataSetInfo(_history.c_str());

  return 0;

}

/////////////////////////////////////////////////
// Add the data fields
//
// Returns 0 on success, -1 on failure

int GpmHdf5ToMdv::_addDataFields(DsMdvx &mdvx, int itime)

{
  
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
  
  return 0;

}

/////////////////////////////////////////////////
// Add the data field
//
// Returns 0 on success, -1 on failure

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

#ifdef JUNK
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
#endif

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
  
  _inputProj.syncToFieldHdr(fhdr);

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
    ((Mdvx::encoding_type_t) _params.output_encoding_type,
     (Mdvx::compression_type_t) _params.output_compression_type);

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
    ((Mdvx::encoding_type_t) _params.output_encoding_type,
     (Mdvx::compression_type_t) _params.output_compression_type);

  // set names etc
  
  field->setFieldName(fieldName.c_str());
  field->setFieldNameLong(longName.c_str());
  field->setUnits(units.c_str());
  field->setTransform("");

  return field;

}
  
///////////////////////////////
// print data in file

void GpmHdf5ToMdv::_printFile(Nc3File &ncf)

{

  cout << "ndims: " << ncf.num_dims() << endl;
  cout << "nvars: " << ncf.num_vars() << endl;
  cout << "ngatts: " << ncf.num_atts() << endl;
  Nc3Dim *unlimd = ncf.rec_dim();
  if (unlimd != NULL) {
    cout << "unlimd: " << unlimd->size() << endl;
  }
  
  // dimensions

  TaArray<Nc3Dim *> dims_;
  Nc3Dim **dims = dims_.alloc(ncf.num_dims());
  for (int idim = 0; idim < ncf.num_dims(); idim++) {
    dims[idim] = ncf.get_dim(idim);

    cout << endl;
    cout << "Dim #: " << idim << endl;
    cout << "  Name: " << dims[idim]->name() << endl;
    cout << "  Length: " << dims[idim]->size() << endl;
    cout << "  Is valid: " << dims[idim]->is_valid() << endl;
    cout << "  Is unlimited: " << dims[idim]->is_unlimited() << endl;
    
  } // idim
  
  cout << endl;

  // global attributes

  cout << "Global attributes:" << endl;

  for (int iatt = 0; iatt < ncf.num_atts(); iatt++) {
    cout << "  Att num: " << iatt << endl;
    Nc3Att *att = ncf.get_att(iatt);
    _printAtt(att);
    delete att;
  }

  // loop through variables

  TaArray<Nc3Var *> vars_;
  Nc3Var **vars = vars_.alloc(ncf.num_vars());
  for (int ivar = 0; ivar < ncf.num_vars(); ivar++) {

    vars[ivar] = ncf.get_var(ivar);
    cout << endl;
    cout << "Var #: " << ivar << endl;
    cout << "  Name: " << vars[ivar]->name() << endl;
    cout << "  Is valid: " << vars[ivar]->is_valid() << endl;
    cout << "  N dims: " << vars[ivar]->num_dims();
    TaArray<Nc3Dim *> vdims_;
    Nc3Dim **vdims = vdims_.alloc(vars[ivar]->num_dims());
    if (vars[ivar]->num_dims() > 0) {
      cout << ": (";
      for (int ii = 0; ii < vars[ivar]->num_dims(); ii++) {
	vdims[ii] = vars[ivar]->get_dim(ii);
	cout << " " << vdims[ii]->name();
	if (ii != vars[ivar]->num_dims() - 1) {
	  cout << ", ";
	}
      }
      cout << " )";
    }
    cout << endl;
    cout << "  N atts: " << vars[ivar]->num_atts() << endl;
    
    for (int iatt = 0; iatt < vars[ivar]->num_atts(); iatt++) {

      cout << "  Att num: " << iatt << endl;
      Nc3Att *att = vars[ivar]->get_att(iatt);
      _printAtt(att);
      delete att;

    } // iatt

    cout << endl;
    _printVarVals(vars[ivar]);
    
  } // ivar
  
}

/////////////////////
// print an attribute

void GpmHdf5ToMdv::_printAtt(Nc3Att *att)

{

  cout << "    Name: " << att->name() << endl;
  cout << "    Num vals: " << att->num_vals() << endl;
  cout << "    Type: ";
  
  Nc3Values *values = att->values();

  switch(att->type()) {
    
  case nc3NoType: {
    cout << "No type: ";
  }
  break;
  
  case nc3Byte: {
    cout << "BYTE: ";
    unsigned char *vals = (unsigned char *) values->base();
    for (long ii = 0; ii < att->num_vals(); ii++) {
      cout << " " << vals[ii];
    }
  }
  break;
  
  case nc3Char: {
    cout << "CHAR: ";
    TaArray<char> vals_;
    char *vals = vals_.alloc(att->num_vals() + 1);
    memset(vals, 0, att->num_vals() + 1);
    memcpy(vals, values->base(), att->num_vals());
    cout << vals;
  }
  break;
  
  case nc3Short: {
    cout << "SHORT: ";
    short *vals = (short *) values->base();
    for (long ii = 0; ii < att->num_vals(); ii++) {
      cout << " " << vals[ii];
    }
  }
  break;
  
  case nc3Int: {
    cout << "INT: ";
    int *vals = (int *) values->base();
    for (long ii = 0; ii < att->num_vals(); ii++) {
      cout << " " << vals[ii];
    }
  }
  break;
  
  case nc3Float: {
    cout << "FLOAT: ";
    float *vals = (float *) values->base();
    for (long ii = 0; ii < att->num_vals(); ii++) {
      cout << " " << vals[ii];
    }
  }
  break;
  
  case nc3Double: {
    cout << "DOUBLE: ";
    double *vals = (double *) values->base();
    for (long ii = 0; ii < att->num_vals(); ii++) {
      cout << " " << vals[ii];
    }
  }
  break;

    default: {}
  
  }
  
  cout << endl;

  delete values;

}

    
///////////////////////////////
// print variable values

void GpmHdf5ToMdv::_printVarVals(Nc3Var *var)

{

  int nprint = var->num_vals();
  if (nprint > 100) {
    nprint = 100;
  }

  Nc3Values *values = var->values();

  cout << "  Variable vals:";
  
  switch(var->type()) {
    
  case nc3NoType: {
  }
  break;
  
  case nc3Byte: {
    cout << "(byte)";
    unsigned char *vals = (unsigned char *) values->base();
    for (long ii = 0; ii < nprint; ii++) {
      cout << " " << vals[ii];
    }
  }
  break;
  
  case nc3Char: {
    cout << "(char)";
    TaArray<char> str_;
    char *str = str_.alloc(nprint + 1);
    memset(str, 0, nprint + 1);
    memcpy(str, values->base(), nprint);
    cout << " " << str;
  }
  break;
  
  case nc3Short: {
    cout << "(short)";
    short *vals = (short *) values->base();
    for (long ii = 0; ii < nprint; ii++) {
      cout << " " << vals[ii];
    }
  }
  break;
  
  case nc3Int: {
    cout << "(int)";
    int *vals = (int *) values->base();
    for (long ii = 0; ii < nprint; ii++) {
      cout << " " << vals[ii];
    }
  }
  break;
  
  case nc3Float: {
    cout << "(float)";
    float *vals = (float *) values->base();
    for (long ii = 0; ii < nprint; ii++) {
      cout << " " << vals[ii];
    }
  }
  break;
  
  case nc3Double: {
    cout << "(double)";
    double *vals = (double *) values->base();
    for (long ii = 0; ii < nprint; ii++) {
      cout << " " << vals[ii];
    }
  }
  break;

    default: {}
  
  }
  
  cout << endl;

  delete values;

}

////////////////////////////////////////////
// correct a field for sun angle

void GpmHdf5ToMdv::_correctForSunAngle(MdvxField *field)

{

  if (_params.debug) {
    cerr << "Correcting field for sun angle: " << field->getFieldName() << endl;
  }

  Mdvx::field_header_t fhdr = field->getFieldHeader();
  Mdvx::encoding_type_t encoding = (Mdvx::encoding_type_t) fhdr.encoding_type;
  Mdvx::compression_type_t compression = (Mdvx::compression_type_t) fhdr.compression_type;

  if (fhdr.nz != 1) {
    cerr << "GpmHdf5ToMdv::_correctForSunAngle()" << endl;
    cerr << "  Field name: " << field->getFieldName() << endl;
    cerr << "  Is not a 2D field, nz: " << fhdr.nz << endl;
    cerr << "  Sun angle correction will not be applied" << endl;
    return;
  }

  // convert to uncompressed floats

  field->convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_NONE);

  // get the projection object

  MdvxProj proj(fhdr);

  // get the data

  fl32 *data = (fl32 *) field->getVol();

  // correct for each location

  double minVal = 0.0;
  double maxVal = 1.0;
  // double minVal = _params.corrected_field_min_value;
  // double maxVal = _params.corrected_field_max_value;

  int pos = 0;
  for (int iy = 0; iy < fhdr.ny; iy++) {
    for (int ix = 0; ix < fhdr.nx; ix++, pos++) {
      fl32 val = data[pos];
      if (val != fhdr.missing_data_value) {
        // not missing
        double lat, lon;
        proj.xyIndex2latlon(ix, iy, lat, lon);
        // double sinAlt = _sunAngle.computeSinAlt(lat, lon);
        double sinAlt = 1.0;
        double correctedVal = val / sinAlt;
        if (correctedVal < minVal) {
          correctedVal = minVal;
        } else if (correctedVal > maxVal) {
          correctedVal = maxVal;
        }
        data[pos] = correctedVal;
      }
    } // ix
  } // iy


  // convert back to original encoding and compression

  field->convertType(encoding, compression);

}

////////////////////////////////////////////
// remap output data

void GpmHdf5ToMdv::_remapOutput(DsMdvx &mdvx)

{

  if (!_params.remap_output_projection) {
    return;
  }

  for (size_t ifld = 0; ifld < mdvx.getNFields(); ifld++) {
    
    MdvxField *field = mdvx.getField(ifld);
    
    if (field == NULL) {
      cerr << "ERROR - MdvxConvert::_remap" << endl;
      cerr << "  Error remapping field #" << ifld <<
	" in output file" << endl;
      return;
    }
    
    if (_params.remap_projection == Params::PROJ_LATLON) {
      field->remap2Latlon(_remapLut,
			  _params.remap_grid.nx,
			  _params.remap_grid.ny,
			  _params.remap_grid.minx,
			  _params.remap_grid.miny,
			  _params.remap_grid.dx,
			  _params.remap_grid.dy);
    } else if (_params.remap_projection == Params::PROJ_FLAT) {
      field->remap2Flat(_remapLut,
			_params.remap_grid.nx,
			_params.remap_grid.ny,
			_params.remap_grid.minx,
			_params.remap_grid.miny,
			_params.remap_grid.dx,
			_params.remap_grid.dy,
			_params.remap_origin_lat,
			_params.remap_origin_lon,
			_params.remap_rotation,
                        _params.remap_false_northing,
                        _params.remap_false_easting);
    } else if (_params.remap_projection == Params::PROJ_LAMBERT_CONF)	{
      field->remap2LambertConf(_remapLut,
			       _params.remap_grid.nx,
			       _params.remap_grid.ny,
			       _params.remap_grid.minx,
			       _params.remap_grid.miny,
			       _params.remap_grid.dx,
			       _params.remap_grid.dy,
			       _params.remap_origin_lat,
			       _params.remap_origin_lon,
			       _params.remap_lat1,
			       _params.remap_lat2,
                               _params.remap_false_northing,
                               _params.remap_false_easting);
    } else if (_params.remap_projection == Params::PROJ_POLAR_STEREO) {
      Mdvx::pole_type_t poleType = Mdvx::POLE_NORTH;
      if (!_params.remap_pole_is_north) {
	poleType = Mdvx::POLE_SOUTH;
      }
      field->remap2PolarStereo(_remapLut,
			       _params.remap_grid.nx,
			       _params.remap_grid.ny,
			       _params.remap_grid.minx,
			       _params.remap_grid.miny,
			       _params.remap_grid.dx,
			       _params.remap_grid.dy,
			       _params.remap_origin_lat,
			       _params.remap_origin_lon,
			       _params.remap_tangent_lon,
			       poleType,
			       _params.remap_central_scale,
                               _params.remap_false_northing,
                               _params.remap_false_easting);
    } else if (_params.remap_projection == Params::PROJ_OBLIQUE_STEREO) {
      field->remap2ObliqueStereo(_remapLut,
				 _params.remap_grid.nx,
				 _params.remap_grid.ny,
				 _params.remap_grid.minx,
				 _params.remap_grid.miny,
				 _params.remap_grid.dx,
				 _params.remap_grid.dy,
				 _params.remap_origin_lat,
				 _params.remap_origin_lon,
				 _params.remap_tangent_lat,
				 _params.remap_tangent_lon,
                                 _params.remap_false_northing,
                                 _params.remap_false_easting);
    } else if (_params.remap_projection == Params::PROJ_MERCATOR) {
      field->remap2Mercator(_remapLut,
			    _params.remap_grid.nx,
			    _params.remap_grid.ny,
			    _params.remap_grid.minx,
			    _params.remap_grid.miny,
			    _params.remap_grid.dx,
			    _params.remap_grid.dy,
			    _params.remap_origin_lat,
			    _params.remap_origin_lon,
                            _params.remap_false_northing,
                            _params.remap_false_easting);
    } else if (_params.remap_projection == Params::PROJ_TRANS_MERCATOR) {
      field->remap2TransverseMercator(_remapLut,
				      _params.remap_grid.nx,
				      _params.remap_grid.ny,
				      _params.remap_grid.minx,
				      _params.remap_grid.miny,
				      _params.remap_grid.dx,
				      _params.remap_grid.dy,
				      _params.remap_origin_lat,
				      _params.remap_origin_lon,
				      _params.remap_central_scale,
                                      _params.remap_false_northing,
                                      _params.remap_false_easting);
    } else if (_params.remap_projection == Params::PROJ_ALBERS) {
      field->remap2Albers(_remapLut,
			  _params.remap_grid.nx,
			  _params.remap_grid.ny,
			  _params.remap_grid.minx,
			  _params.remap_grid.miny,
			  _params.remap_grid.dx,
			  _params.remap_grid.dy,
			  _params.remap_origin_lat,
			  _params.remap_origin_lon,
			  _params.remap_lat1,
			  _params.remap_lat2,
                          _params.remap_false_northing,
                          _params.remap_false_easting);
    } else if (_params.remap_projection == Params::PROJ_LAMBERT_AZIM) {
      field->remap2LambertAzimuthal(_remapLut,
				    _params.remap_grid.nx,
				    _params.remap_grid.ny,
				    _params.remap_grid.minx,
				    _params.remap_grid.miny,
				    _params.remap_grid.dx,
				    _params.remap_grid.dy,
				    _params.remap_origin_lat,
				    _params.remap_origin_lon,
                                    _params.remap_false_northing,
                                    _params.remap_false_easting);
    } else if (_params.remap_projection == Params::PROJ_VERT_PERSP) {
      field->remap2VertPersp(_remapLut,
                             _params.remap_grid.nx,
                             _params.remap_grid.ny,
                             _params.remap_grid.minx,
                             _params.remap_grid.miny,
                             _params.remap_grid.dx,
                             _params.remap_grid.dy,
                             _params.remap_origin_lat,
                             _params.remap_origin_lon,
                             _params.remap_persp_radius,
                             _params.remap_false_northing,
                             _params.remap_false_easting);
     }
  }
  
}

////////////////////////////////////////////
// auto remap to latlon grid
//
// Automatically picks the grid resolution and extent
// from the existing data.

void GpmHdf5ToMdv::_autoRemapToLatLon(DsMdvx &mdvx)

{
  
  for (size_t ifld = 0; ifld < mdvx.getNFields(); ifld++) {
    
    MdvxField *field = mdvx.getField(ifld);
    
    if (field == NULL) {
      cerr << "ERROR - GpmHdf5ToMdv::_autoRemapToLatLon()" << endl;
      cerr << "  Error remapping field #" << ifld <<
	" in output file" << endl;
      return;
    }
    
    field->autoRemap2Latlon(_remapLut);
  }
  
}

///////////////////////////////////////////
// Check if dx is constant

bool GpmHdf5ToMdv::_checkDxIsConstant()

{

  if (_nx < 3) {
    return true;
  }

  for (int ix = 0; ix < _nx - 1; ix++) {
    double dx = _xArray[ix+1] - _xArray[ix];
    double diff = fabs(dx - _dx);
    double diffFraction = diff / fabs(_dx);
    if (diffFraction > 0.001) {
      return false;
    }
  }

  return true;

}

////////////////////////////////////////////
// Check if dy is constant

bool GpmHdf5ToMdv::_checkDyIsConstant()

{

  if (_ny < 3) {
    return true;
  }

  for (int iy = 0; iy < _ny - 1; iy++) {
    double dy = _yArray[iy+1] - _yArray[iy];
    double diff = fabs(dy - _dy);
    double diffFraction = diff / fabs(_dy);
    if (diffFraction > 0.001) {
      return false;
    }
  }
  
  return true;

}

//////////////////////////////////////////////////////////////////////////
// Initialize the mercator projection from the input file coord variables

void GpmHdf5ToMdv::_initMercatorFromInputCoords()

{

  // sanity check

  if (_nx < 5 || _ny < 5) {
    cerr << "WARNING - GpmHdf5ToMdv::_initMercatorFromInputCoords()" << endl;
    cerr << "  Grid too small to deduce Mercator properties accurately" << endl;
    cerr << "  nx: " << _nx << endl;
    cerr << "  ny: " << _ny << endl;
    return;
  }

  // find mid pt

  int midIx = _nx / 2;
  int midIy = _ny / 2;

  // set origin

  double originLon = _xArray[midIx];
  double originLat = _yArray[midIy];

  if (_params.debug) {
    cerr << "Setting Mercator from input data" << endl;
    cerr << "  Origin lon: " << originLon << endl;
    cerr << "  Origin lat: " << originLat << endl;
  }

  _inputProj.initMercator(originLat, originLon);

}

//////////////////////////////////////////////////////////////////////////
// FInd the limits of valid lat/lon values if the x,y locations are
// supplied in latitude/longitude coords
//
// Returns 0 on success, -1 on failure

int GpmHdf5ToMdv::_findValidLatLonLimits()

{
  
  // sanity check

  if (_nx < 5 || _ny < 5) {
    cerr << "ERROR - GpmHdf5ToMdv::_findValidLatLonLimits()" << endl;
    cerr << "  Grid too small to deduce limits accurately" << endl;
    cerr << "  nx: " << _nx << endl;
    cerr << "  ny: " << _ny << endl;
    _ixValidStart = 0;
    _ixValidEnd = _nx - 1;
    _iyValidStart = 0;
    _iyValidEnd = _ny - 1;
    return -1;
  }
  
  // find mid pt

  int midIx = _nx / 2;
  int midIy = _ny / 2;

  // set origin

  double centerLon = _xArray[midIx];
  double centerLat = _yArray[midIy];

  if (_params.debug) {
    cerr << "DEBUG - findValidLatLonLimits()" << endl;
    cerr << "  data center lon: " << centerLon << endl;
    cerr << "  data center lat: " << centerLat << endl;
  }

  // get starting deltas at center of grid

  double dLon0 =  fabs(_xArray[midIx+1] - _xArray[midIx-1]) / 2.0;
  double dLat0 =  fabs(_yArray[midIy+1] - _yArray[midIy-1]) / 2.0;

  // move out from the grid center, looking for big jumps in the delta
  // and stop there

  _ixValidStart = 0;
  double dLonPrev = dLon0;
  for (int ix = midIx; ix > 0; ix--) {
    double lon0 =  _xArray[ix];
    double lon1 =  _xArray[ix-1];
    double dLon =  fabs(lon0 - lon1);
    double dd = fabs(dLon - dLonPrev);
    double ddFrac = dd / dLonPrev;
    if (fabs(lon0) > 180.0 || lon0 == 0.0 || ddFrac > 1.0) {
      // bad jump, stop here
      _ixValidStart = ix;
      cerr << "ERROR - GpmHdf5ToMdv::_findValidLatLonLimits()" << endl;
      cerr << "   Bad longitude jump, ix, lon0, lon1: "
           << ix << ", " << lon0 << ", " << lon1 << endl;
      break;
    }
    dLonPrev = dLon;
  } // ix

  _ixValidEnd = _nx - 1;
  dLonPrev = dLon0;
  for (int ix = midIx; ix < _nx - 1; ix++) {
    double lon0 =  _xArray[ix];
    double lon1 =  _xArray[ix+1];
    double dLon =  fabs(_xArray[ix+1] - _xArray[ix]);
    double dd = fabs(dLon - dLonPrev);
    double ddFrac = dd / dLonPrev;
    if (fabs(lon0) > 360.0 || lon0 == 0.0 || ddFrac > 1.0) {
      // bad jump, stop here
      _ixValidEnd = ix;
      cerr << "ERROR - GpmHdf5ToMdv::_findValidLatLonLimits()" << endl;
      cerr << "   Bad longitude jump, ix, lon0, lon1: "
           << ix << ", " << lon0 << ", " << lon1 << endl;
      break;
    }
    dLonPrev = dLon;
  } // ix

  _iyValidStart = 0;
  double dLatPrev = dLat0;
  for (int iy = midIy; iy > 0; iy--) {
    double lat0 =  _yArray[iy];
    double lat1 =  _yArray[iy-1];
    double dLat =  fabs(_yArray[iy] - _yArray[iy-1]);
    double dd = fabs(dLat - dLatPrev);
    double ddFrac = dd / dLatPrev;
    if (fabs(lat0) > 90.0 || lat0 == 0.0 || ddFrac > 1.0) {
      // big jump, stop here
      _iyValidStart = iy;
      cerr << "ERROR - GpmHdf5ToMdv::_findValidLatLonLimits()" << endl;
      cerr << "   Bad latitude jump, iy, lat0, lat1: " 
           << iy << ", " << lat0 << ", " << lat1 << endl;
      break;
    }
    dLatPrev = dLat;
  } // iy

  _iyValidEnd = _ny - 1;
  dLatPrev = dLat0;
  for (int iy = midIy; iy < _ny - 1; iy++) {
    double lat0 =  _yArray[iy];
    double lat1 =  _yArray[iy+1];
    double dLat =  fabs(_yArray[iy+1] - _yArray[iy]);
    double dd = fabs(dLat - dLatPrev);
    double ddFrac = dd / dLatPrev;
    if (fabs(lat0) > 90.0 || lat0 == 0.0 || ddFrac > 1.0) {
      // big jump, stop here
      _iyValidEnd = iy;
      cerr << "ERROR - GpmHdf5ToMdv::_findValidLatLonLimits()" << endl;
      cerr << "   Bad latitude jump, iy, lat0, lat1: " 
           << iy << ", " << lat0 << ", " << lat1 << endl;
      break;
    }
    dLatPrev = dLat;
  } // iy

  _nxValid = _ixValidEnd - _ixValidStart + 1;
  _nyValid = _iyValidEnd - _iyValidStart + 1;

  if (_nxValid < 5 || _nyValid < 5) {
    cerr << "ERROR - GpmHdf5ToMdv::_findValidLatLonLimits()" << endl;
    cerr << "  Valid grid too small to process" << endl;
    cerr << "  nxValid: " << _nxValid << endl;
    cerr << "  nyValid: " << _nyValid << endl;
    return -1;
  }

  return 0;
  
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

