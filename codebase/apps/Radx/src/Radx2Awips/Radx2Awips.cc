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
// Radx2Awips.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// August 2025
//
///////////////////////////////////////////////////////////////
//
// Converts Radx-style files to AWIPS NetCDF polar files"
//
////////////////////////////////////////////////////////////////

#include "Radx2Awips.hh"
#include <Radx/Radx.hh>
#include <Radx/RadxVol.hh>
#include <Radx/RadxSweep.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxField.hh>
#include <Mdv/GenericRadxFile.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxTimeList.hh>
#include <Radx/RadxPath.hh>
#include <dsserver/DsLdataInfo.hh>
#include <didss/DsInputPath.hh>
#include <toolsa/TaXml.hh>
#include <toolsa/TaStr.hh>
#include <toolsa/TaFile.hh>
#include <toolsa/pmu.h>
#include <toolsa/umisc.h>
#include <cerrno>
#include <set>
using namespace std;

// Constructor

Radx2Awips::Radx2Awips(int argc, char **argv)
  
{

  OK = TRUE;

  // set programe name

  _progName = "Radx2Awips";
  ucopyright((char *) _progName.c_str());
  
  // parse command line args
  
  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args." << endl;
    OK = FALSE;
    return;
  }
  
  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv,
			   _args.override.list, &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters." << endl;
    OK = FALSE;
    return;
  }

  // check on overriding radar location

  if (_params.override_radar_location) {
    if (_params.radar_latitude_deg < -900 ||
        _params.radar_longitude_deg < -900 ||
        _params.radar_altitude_meters < -900) {
      cerr << "ERROR: " << _progName << endl;
      cerr << "  Problem with command line or TDRP parameters." << endl;
      cerr << "  You have chosen to override radar location" << endl;
      cerr << "  You must override latitude, longitude and altitude" << endl;
      cerr << "  You must override all 3 values." << endl;
      OK = FALSE;
    }
  }

}

// destructor

Radx2Awips::~Radx2Awips()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int Radx2Awips::Run()
{

  if (_params.mode == Params::ARCHIVE) {
    return _runArchive();
  } else if (_params.mode == Params::FILELIST) {
    return _runFilelist();
  } else {
    if (_params.latest_data_info_avail) {
      return _runRealtimeWithLdata();
    } else {
      return _runRealtimeNoLdata();
    }
  }
}

//////////////////////////////////////////////////
// Run in filelist mode

int Radx2Awips::_runFilelist()
{
  
  int iret = 0;

  if (_params.debug) {
    cerr << "Running Radx2Awips" << endl;
    cerr << "  n input files: " << _args.inputFileList.size() << endl;
  }

  int nGood = 0;
  int nError = 0;
  
  // loop through the input file list
  
  RadxVol vol;
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    vol.setDebug(true);
  }
  for (int ii = 0; ii < (int) _args.inputFileList.size(); ii++) {
    string inputPath = _args.inputFileList[ii];
    // read input file
    int jret = _readFile(inputPath, vol);
    if (jret == 0) {
      // finalize the volume
      _finalizeVol(vol);
      // write the volume out
      if (_writeVol(vol)) {
        cerr << "ERROR - Radx2Awips::_runFileList" << endl;
        cerr << "  Cannot write volume to file" << endl;
        iret = -1;
        nError++;
        if (_params.debug) {
          cerr << "  ====>> n errors so far: " << nError << endl;
        }
      } else {
        nGood++;
        if (_params.debug) {
          cerr << "  ====>> n good files so far: " << nGood << endl;
          cerr << "  ====>> n errors     so far: " << nError << endl;
          cerr << "  ====>> sum          so far: " << nGood + nError << endl;
        }
      }
    } else if (jret < 0) {
      iret = -1;
      nError++;
      if (_params.debug) {
        cerr << "  ====>> n errors so far: " << nError << endl;
      }
    }
    // free up
    vol.clear();
  }
  
  if (_params.debug) {
    cerr << "Radx2Awips done" << endl;
    cerr << "====>> n good files processed: " << nGood << endl;
  }

  return iret;

}

//////////////////////////////////////////////////
// Run in archive mode

int Radx2Awips::_runArchive()
{

  // get the files to be processed

  RadxTimeList tlist;
  tlist.setDir(_params.input_dir);
  tlist.setModeInterval(_args.startTime, _args.endTime);
  if (tlist.compile()) {
    cerr << "ERROR - Radx2Awips::_runArchive()" << endl;
    cerr << "  Cannot compile time list, dir: " << _params.input_dir << endl;
    cerr << "  Start time: " << RadxTime::strm(_args.startTime) << endl;
    cerr << "  End time: " << RadxTime::strm(_args.endTime) << endl;
    cerr << tlist.getErrStr() << endl;
    return -1;
  }

  const vector<string> &paths = tlist.getPathList();
  if (paths.size() < 1) {
    cerr << "ERROR - Radx2Awips::_runArchive()" << endl;
    cerr << "  No files found, dir: " << _params.input_dir << endl;
    return -1;
  }
  
  // loop through the input file list

  RadxVol vol;
  int iret = 0;
  for (size_t ii = 0; ii < paths.size(); ii++) {
    // read input file
    int jret = _readFile(paths[ii], vol);
    if (jret == 0) {
      // finalize the volume
      _finalizeVol(vol);
      // write the volume out
      if (_writeVol(vol)) {
        cerr << "ERROR - Radx2Awips::_runArchive" << endl;
        cerr << "  Cannot write volume to file" << endl;
        return -1;
      }
    } else if (jret < 0) {
      iret = -1;
    }
    // free up
    vol.clear();
  }

  return iret;
  
}

//////////////////////////////////////////////////
// Run in realtime mode with latest data info

int Radx2Awips::_runRealtimeWithLdata()
{

  // init process mapper registration

  PMU_auto_init(_progName.c_str(), _params.instance,
                PROCMAP_REGISTER_INTERVAL);

  // watch for new data to arrive

  LdataInfo ldata(_params.input_dir,
                  _params.debug >= Params::DEBUG_VERBOSE);
  if (strlen(_params.search_ext) > 0) {
    ldata.setDataFileExt(_params.search_ext);
  }

  RadxVol vol;
  int iret = 0;
  int msecsWait = _params.wait_between_checks * 1000;
  while (true) {
    ldata.readBlocking(_params.max_realtime_data_age_secs,
                       msecsWait, PMU_auto_register);
    const string path = ldata.getDataPath();
    // read input file
    int jret = _readFile(path, vol);
    if (jret == 0) {
      // finalize the volume
      _finalizeVol(vol);
      // write the volume out
      if (_writeVol(vol)) {
        cerr << "ERROR - Radx2Awips::_runRealtimeWithLdata" << endl;
        cerr << "  Cannot write volume to file" << endl;
        return -1;
      }
    } else if (jret < 0) {
      iret = -1;
    }
    // free up
    vol.clear();
  }

  return iret;

}

//////////////////////////////////////////////////
// Run in realtime mode without latest data info

int Radx2Awips::_runRealtimeNoLdata()
{

  // init process mapper registration

  PMU_auto_init(_progName.c_str(), _params.instance,
                PROCMAP_REGISTER_INTERVAL);
  
  // Set up input path

  DsInputPath input(_progName,
		    _params.debug >= Params::DEBUG_VERBOSE,
		    _params.input_dir,
		    _params.max_realtime_data_age_secs,
		    PMU_auto_register,
		    _params.latest_data_info_avail,
		    false);

  input.setFileQuiescence(_params.file_quiescence);
  input.setSearchExt(_params.search_ext);
  input.setRecursion(_params.search_recursively);
  input.setMaxRecursionDepth(_params.max_recursion_depth);
  input.setMaxDirAge(_params.max_realtime_data_age_secs);

  int iret = 0;
  RadxVol vol;

  while(true) {

    // check for new data
    
    char *path = input.next(false);
    
    if (path == NULL) {
      
      // sleep a bit
      
      PMU_auto_register("Waiting for data");
      umsleep(_params.wait_between_checks * 1000);

    } else {

      // read the input file
      
      int jret = _readFile(path, vol);
      if (jret == 0) {
        // finalize the volume
        _finalizeVol(vol);
        // write the volume out
        if (_writeVol(vol)) {
          cerr << "ERROR - Radx2Awips::_runRealtimeNoLdata" << endl;
          cerr << "  Cannot write volume to file" << endl;
          return -1;
        }
      } else if (jret < 0) {
        iret = -1;
      }

      // free up
      vol.clear();
  
    }

  } // while

  return iret;

}

//////////////////////////////////////////////////
// Read in a file
// accounting for special cases such as gematronik
// Returns 0 on success
//         1 if already read,
//         -1 on failure

int Radx2Awips::_readFile(const string &readPath,
                           RadxVol &vol)
{
  
  PMU_auto_register("Processing file");

  // clear all data on volume object

  vol.clear();

  // check file name
  
  if (strlen(_params.search_ext) > 0) {
    RadxPath rpath(readPath);
    if (strcmp(rpath.getExt().c_str(), _params.search_ext)) {
      if (_params.debug) {
        cerr << "WARNING - ignoring file: " << readPath << endl;
        cerr << "  Does not have correct extension: "
             << _params.search_ext << endl;
      }
      return -1;
    }
  }
  
  if (strlen(_params.search_substr) > 0) {
    RadxPath rpath(readPath);
    if (rpath.getFile().find(_params.search_substr)
        == string::npos) {
      if (_params.debug) {
        cerr << "WARNING - ignoring file: " << readPath << endl;
        cerr << "  Does not contain required substr: "
             << _params.search_substr << endl;
      }
      return -1;
    }
  }

  // check we have not already processed this file
  // in the file aggregation step
  // if not clear _readPaths

  RadxPath rpath(readPath);
  for (auto ii = _readPaths.begin(); ii != _readPaths.end(); ii++) {
    RadxPath tpath(*ii);
    if (rpath.getFile() == tpath.getFile()) {
      if (_params.debug >= Params::DEBUG_EXTRA) {
        cerr << "Skipping file: " << readPath << endl;
        cerr << "  Previously processed" << endl;
      }
      return 1;
    }
  }
  
  _readPaths.clear();

  if (_params.debug) {
    cerr << "INFO - Radx2Awips::Run" << endl;
    cerr << "  Input path: " << readPath << endl;
  }
  
  GenericRadxFile inFile;
  _setupRead(inFile);
  
  // read in file

  int iret = 0;
  if (inFile.readFromPath(readPath, vol)) {
    cerr << "ERROR - Radx2Awips::_readFile" << endl;
    cerr << "  path: " << readPath << endl;
    cerr << inFile.getErrStr() << endl;
    iret = -1;
  }
  
  // save read paths used

  vector<string> rpaths = inFile.getReadPaths();
  for (size_t ii = 0; ii < rpaths.size(); ii++) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "  ==>> used file: " << rpaths[ii] << endl;
    }
    _readPaths.insert(rpaths[ii]);
  }

  if (iret) {
    return -1;
  }

  return 0;

}

//////////////////////////////////////////////////
// Finalize the volume based on parameters
// Returns 0 on success, -1 on failure

void Radx2Awips::_finalizeVol(RadxVol &vol)
  
{

  vector<string> uniqueFields = vol.getUniqueFieldNameList();
  for (size_t jj = 0; jj < uniqueFields.size(); jj++) {
    string fname = uniqueFields[jj]; 
    bool keep = false;
    for (int ii = 0; ii < _params.output_fields_n; ii++) {
      if (fname == _params._output_fields[ii].input_field_name) {
        keep = true;
        break;
      }
    } // ii
    if (!keep) {
      vol.removeField(fname);
    }
  } // jj

  // set number of gates constant
  
  vol.setNGatesConstant();

  // override radar location if requested
  
  if (_params.override_radar_location) {
    vol.overrideLocation(_params.radar_latitude_deg,
                         _params.radar_longitude_deg,
                         _params.radar_altitude_meters / 1000.0);
  }
    
  // trim to 360s if requested

  if (_params.trim_surveillance_sweeps_to_360deg) {
    if (_params.debug) {
      cerr << "DEBUG - trimming surveillance sweeps to 360 deg" << endl;
    }
    vol.trimSurveillanceSweepsTo360Deg();
  }

  // constrain by elevation
  
  vol.constrainByElevation(_params.min_elevation_deg,
                           _params.max_elevation_deg);
  
  // set field type, names, units etc
  
  _convertFields(vol);

  // reload sweep and/or volume info from rays

  vol.loadSweepInfoFromRays();
  vol.loadVolumeInfoFromRays();

  // set global attributes
  
  _setGlobalAttr(vol);

}

//////////////////////////////////////////////////
// set up read

void Radx2Awips::_setupRead(RadxFile &file)
{
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    file.setDebug(true);
  }
  if (_params.debug >= Params::DEBUG_EXTRA) {
    file.setVerbose(true);
  }

  // set input field names
  
  for (int ii = 0; ii < _params.output_fields_n; ii++) {
    file.addReadField(_params._output_fields[ii].input_field_name);
  }
  
  if (_params.set_max_range) {
    file.setReadMaxRangeKm(_params.max_range_km);
  }
  
  if (_params.debug >= Params::DEBUG_EXTRA) {
    file.printReadRequest(cerr);
  }

}

//////////////////////////////////////////////////
// rename fields as required

void Radx2Awips::_convertFields(RadxVol &vol)
{

  for (int ii = 0; ii < _params.output_fields_n; ii++) {
    
    const Params::output_field_t &ofld = _params._output_fields[ii];

    
    
    string iname = ofld.input_field_name;
    string oname = ofld.output_field_name;
    string ounits = ofld.output_units;
    
    Radx::DataType_t dtype = Radx::ASIS;
    switch(ofld.encoding) {
      case Params::OUTPUT_ENCODING_INT32:
        dtype = Radx::SI32;
        break;
      case Params::OUTPUT_ENCODING_INT16:
        dtype = Radx::SI16;
        break;
      case Params::OUTPUT_ENCODING_FLOAT32:
      default:
        dtype = Radx::FL32;
        break;
    }

    vol.convertField(iname, OUTPUT_ENCODING_FLOAT32, 
                     oname, ounits, oname, oname);
    
  }

}

//////////////////////////////////////////////////
// set up write

void Radx2Awips::_setupWrite(RadxFile &file)
{

  if (_params.debug) {
    file.setDebug(true);
  }
  if (_params.debug >= Params::DEBUG_EXTRA) {
    file.setVerbose(true);
  }

  if (_params.output_filename_mode == Params::START_TIME_ONLY) {
    file.setWriteFileNameMode(RadxFile::FILENAME_WITH_START_TIME_ONLY);
  } else if (_params.output_filename_mode == Params::END_TIME_ONLY) {
    file.setWriteFileNameMode(RadxFile::FILENAME_WITH_END_TIME_ONLY);
  } else {
    file.setWriteFileNameMode(RadxFile::FILENAME_WITH_START_AND_END_TIMES);
  }

  if (_params.output_compressed) {
    file.setWriteCompressed(true);
    file.setCompressionLevel(_params.compression_level);
  } else {
    file.setWriteCompressed(false);
  }

  if (_params.output_native_byte_order) {
    file.setWriteNativeByteOrder(true);
  } else {
    file.setWriteNativeByteOrder(false);
  }

  // set output format

  switch (_params.output_format) {
    case Params::OUTPUT_FORMAT_UF:
      file.setFileFormat(RadxFile::FILE_FORMAT_UF);
      break;
    case Params::OUTPUT_FORMAT_DORADE:
      file.setFileFormat(RadxFile::FILE_FORMAT_DORADE);
      break;
    case Params::OUTPUT_FORMAT_FORAY:
      file.setFileFormat(RadxFile::FILE_FORMAT_FORAY_NC);
      break;
    case Params::OUTPUT_FORMAT_NEXRAD:
      file.setFileFormat(RadxFile::FILE_FORMAT_NEXRAD_AR2);
      break;
    case Params::OUTPUT_FORMAT_MDV_RADIAL:
      file.setFileFormat(RadxFile::FILE_FORMAT_MDV_RADIAL);
      break;
    case Params::OUTPUT_FORMAT_NSSL_MRD:
      file.setFileFormat(RadxFile::FILE_FORMAT_NSSL_MRD);
      break;
    case Params::OUTPUT_FORMAT_ODIM_HDF5:
      file.setFileFormat(RadxFile::FILE_FORMAT_ODIM_HDF5);
      break;
    // case Params::OUTPUT_FORMAT_NCXX:
    //   file.setFileFormat(RadxFile::FILE_FORMAT_NCXX);
    //   break;
    case Params::OUTPUT_FORMAT_CFRADIAL2:
      file.setFileFormat(RadxFile::FILE_FORMAT_CFRADIAL2);
      break;
    default:
    case Params::OUTPUT_FORMAT_CFRADIAL:
      file.setFileFormat(RadxFile::FILE_FORMAT_CFRADIAL);
  }

  // set netcdf format - used for CfRadial

  switch (_params.netcdf_style) {
    case Params::NETCDF4_CLASSIC:
      file.setNcFormat(RadxFile::NETCDF4_CLASSIC);
      break;
    case Params::NC64BIT:
      file.setNcFormat(RadxFile::NETCDF_OFFSET_64BIT);
      break;
    case Params::NETCDF4:
      file.setNcFormat(RadxFile::NETCDF4);
      break;
    default:
      file.setNcFormat(RadxFile::NETCDF_CLASSIC);
  }

  if (_params.write_individual_sweeps) {
    file.setWriteIndividualSweeps(true);
  }

  if (_params.output_force_ngates_vary) {
    file.setWriteForceNgatesVary(true);
  }

  if (strlen(_params.output_filename_prefix) > 0) {
    file.setWriteFileNamePrefix(_params.output_filename_prefix);
  }
  if (strlen(_params.output_filename_suffix) > 0) {
    file.setWriteFileNameSuffix(_params.output_filename_suffix);
  }

  file.setWriteInstrNameInFileName(_params.include_instrument_name_in_file_name);
  file.setWriteSiteNameInFileName(_params.include_site_name_in_file_name);
  file.setWriteSubsecsInFileName(_params.include_subsecs_in_file_name);
  file.setWriteScanTypeInFileName(_params.include_scan_type_in_file_name);
  file.setWriteScanNameInFileName(_params.include_scan_name_in_file_name);
  file.setWriteScanIdInFileName(_params.include_scan_id_in_file_name);
  file.setWriteRangeResolutionInFileName(_params.include_range_resolution_in_file_name);
  file.setWriteVolNumInFileName(_params.include_vol_num_in_file_name);
  file.setWriteHyphenInDateTime(_params.use_hyphen_in_file_name_datetime_part);

  if (_params.write_using_proposed_standard_name_attr) {
    file.setWriteProposedStdNameInNcf(true);
  }

}

//////////////////////////////////////////////////
// set selected global attributes

void Radx2Awips::_setGlobalAttr(RadxVol &vol)
{

  vol.setDriver("Radx2Awips(NCAR)");

  if (strlen(_params.convention_override) > 0) {
    vol.setConvention(_params.convention_override);
  }

  if (strlen(_params.subconvention_override) > 0) {
    vol.setSubConventions(_params.subconvention_override);
  }

  if (strlen(_params.version_override) > 0) {
    vol.setVersion(_params.version_override);
  }

  if (strlen(_params.title_override) > 0) {
    vol.setTitle(_params.title_override);
  }

  if (strlen(_params.institution_override) > 0) {
    vol.setInstitution(_params.institution_override);
  }

  if (strlen(_params.references_override) > 0) {
    vol.setReferences(_params.references_override);
  }

  if (strlen(_params.source_override) > 0) {
    vol.setSource(_params.source_override);
  }

  if (strlen(_params.history_override) > 0) {
    vol.setHistory(_params.history_override);
  }

  if (strlen(_params.author_override) > 0) {
    vol.setAuthor(_params.author_override);
  }

  if (strlen(_params.comment_override) > 0) {
    vol.setComment(_params.comment_override);
  }

  RadxTime now(RadxTime::NOW);
  vol.setCreated(now.asString());

  if (_params.add_user_specified_global_attributes) {
    for (int ii = 0; ii < _params.user_defined_global_attributes_n; ii++) {
      Params::attr_t attr = _params._user_defined_global_attributes[ii];
      RadxVol::UserGlobAttr::attr_type_t attrType = 
        RadxVol::UserGlobAttr::ATTR_STRING;
      switch (attr.attrType) {
        case Params::ATTR_STRING:
          attrType = RadxVol::UserGlobAttr::ATTR_STRING;
          break;
        case Params::ATTR_INT:
          attrType = RadxVol::UserGlobAttr::ATTR_INT;
          break;
        case Params::ATTR_DOUBLE:
          attrType = RadxVol::UserGlobAttr::ATTR_DOUBLE;
          break;
        case Params::ATTR_INT_ARRAY:
          attrType = RadxVol::UserGlobAttr::ATTR_INT_ARRAY;
          break;
        case Params::ATTR_DOUBLE_ARRAY:
          attrType = RadxVol::UserGlobAttr::ATTR_DOUBLE_ARRAY;
          break;
      } // switch
      vol.addUserGlobAttr(attr.name, attrType, attr.val);
    } // ii
  } // if (_params.add_user_specified_global_attributes) {

}

//////////////////////////////////////////////////
// write out the volume

int Radx2Awips::_writeVol(RadxVol &vol)
{

  // output file

  GenericRadxFile outFile;
  _setupWrite(outFile);

  string outputDir = _params.output_dir;

  if (_params.separate_output_dirs_by_scan_type) {
    outputDir += PATH_DELIM;
    Radx::SweepMode_t sweepMode = vol.getPredomSweepMode();
    switch (sweepMode) {
      case Radx::SWEEP_MODE_RHI:
        outputDir += _params.rhi_subdir;
        break;
      case Radx::SWEEP_MODE_SECTOR:
        outputDir += _params.sector_subdir;
        break;
      case Radx::SWEEP_MODE_VERTICAL_POINTING:
        outputDir += _params.vert_subdir;
        break;
      case Radx::SWEEP_MODE_SUNSCAN:
      case Radx::SWEEP_MODE_SUNSCAN_RHI:
        outputDir += _params.sun_subdir;
        break;
      default:
        outputDir += _params.surveillance_subdir;
    }
  }
    
  if (_params.output_filename_mode == Params::SPECIFY_FILE_NAME) {
    
    string outPath = outputDir;
    outPath += PATH_DELIM;
    outPath += _params.output_filename;

    // write to path
  
    if (outFile.writeToPath(vol, outPath)) {
      cerr << "ERROR - Radx2Awips::_writeVol" << endl;
      cerr << "  Cannot write file to path: " << outPath << endl;
      cerr << outFile.getErrStr() << endl;
      return -1;
    }
    
  } else {

    // write to dir
  
    if (outFile.writeToDir(vol, outputDir,
                           _params.append_day_dir_to_output_dir,
                           _params.append_year_dir_to_output_dir)) {
      cerr << "ERROR - Radx2Awips::_writeVol" << endl;
      cerr << "  Cannot write file to dir: " << outputDir << endl;
      cerr << outFile.getErrStr() << endl;
      return -1;
    }

  }

  string outputPath = outFile.getPathInUse();

  // write latest data info file if requested 
  
  if (_params.write_latest_data_info) {
    DsLdataInfo ldata(outputDir);
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      ldata.setDebug(true);
    }
    string relPath;
    RadxPath::stripDir(outputDir, outputPath, relPath);
    ldata.setRelDataPath(relPath);
    ldata.setWriter(_progName);
    if (ldata.write(vol.getStartTimeSecs())) {
      cerr << "WARNING - Radx2Awips::_writeVol" << endl;
      cerr << "  Cannot write latest data info file to dir: "
           << outputDir << endl;
    }
  }

  return 0;

}

////////////////////////////////////////////////////////////////////
// censor fields in vol

void Radx2Awips::_censorFields(RadxVol &vol)

{

  vector<RadxRay *> &rays = vol.getRays();
  for (size_t ii = 0; ii < rays.size(); ii++) {
    _censorRay(rays[ii]);
  }

}

////////////////////////////////////////////////////////////////////
// censor fields in a ray

void Radx2Awips::_censorRay(RadxRay *ray)

{

  if (!_params.apply_censoring) {
    return;
  }

  if (_params.censoring_limit_elev_angles) {
    if (ray->getElevationDeg() < _params.censoring_min_elev_deg ||
        ray->getElevationDeg() > _params.censoring_max_elev_deg) {
      return;
    }
  }

  // convert fields to floats

  vector<Radx::DataType_t> fieldTypes;
  vector<RadxField *> fields = ray->getFields();
  for (size_t ii = 0; ii < fields.size(); ii++) {
    RadxField *field = fields[ii];
    Radx::DataType_t dtype = field->getDataType();
    fieldTypes.push_back(dtype);
    if (_checkFieldForCensoring(field)) {
      field->convertToFl32();
    }
  }

  // initialize censoring flags to true to
  // turn censoring ON everywhere
  
  vector<int> censorFlag;
  size_t nGates = ray->getNGates();
  for (size_t igate = 0; igate < nGates; igate++) {
    censorFlag.push_back(1);
  }

  // check OR fields
  // if any of these have VALID data, we turn censoring OFF

  int orFieldCount = 0;

  for (int ifield = 0; ifield < _params.censoring_fields_n; ifield++) {

    const Params::censoring_field_t &cfld = _params._censoring_fields[ifield];
    if (cfld.combination_method != Params::LOGICAL_OR) {
      continue;
    }

    RadxField *field = ray->getField(cfld.name);
    if (field == NULL) {
      // field missing, do not censor
      if (_nWarnCensorPrint % 360 == 0) {
        cerr << "WARNING - censoring field missing: " << cfld.name << endl;
        cerr << "  Censoring will not be applied for this field." << endl;
      }
      _nWarnCensorPrint++;
      for (size_t igate = 0; igate < nGates; igate++) {
        censorFlag[igate] = 0;
      }
      continue;
    }
    
    orFieldCount++;
    
    double minValidVal = cfld.min_valid_value;
    double maxValidVal = cfld.max_valid_value;

    const Radx::fl32 *fdata = (const Radx::fl32 *) field->getData();
    for (size_t igate = 0; igate < nGates; igate++) {
      double val = fdata[igate];
      if (val >= minValidVal && val <= maxValidVal) {
        censorFlag[igate] = 0;
      }
    }
    
  } // ifield

  // if no OR fields were found, turn off ALL censoring at this stage

  if (orFieldCount == 0) {
    for (size_t igate = 0; igate < nGates; igate++) {
      censorFlag[igate] = 0;
    }
  }

  // check AND fields
  // if any of these have INVALID data, we turn censoring ON

  for (int ifield = 0; ifield < _params.censoring_fields_n; ifield++) {
    
    const Params::censoring_field_t &cfld = _params._censoring_fields[ifield];
    if (cfld.combination_method != Params::LOGICAL_AND) {
      continue;
    }

    RadxField *field = ray->getField(cfld.name);
    if (field == NULL) {
      continue;
    }
    
    double minValidVal = cfld.min_valid_value;
    double maxValidVal = cfld.max_valid_value;

    const Radx::fl32 *fdata = (const Radx::fl32 *) field->getData();
    for (size_t igate = 0; igate < nGates; igate++) {
      double val = fdata[igate];
      if (val < minValidVal || val > maxValidVal) {
        censorFlag[igate] = 1;
      }
    }
    
  } // ifield

  // check that uncensored runs meet the minimum length
  // those which do not are censored

  int minValidRun = _params.censoring_min_valid_run;
  if (minValidRun > 1) {
    int runLength = 0;
    bool doCheck = false;
    for (int igate = 0; igate < (int) nGates; igate++) {
      if (censorFlag[igate] == 0) {
        doCheck = false;
        runLength++;
      } else {
        doCheck = true;
      }
      // last gate?
      if (igate == (int) nGates - 1) doCheck = true;
      // check run length
      if (doCheck) {
        if (runLength < minValidRun) {
          // clear the run which is too short
          for (int jgate = igate - runLength; jgate < igate; jgate++) {
            censorFlag[jgate] = 1;
          } // jgate
        }
        runLength = 0;
      } // if (doCheck ...
    } // igate
  }

  // apply censoring by setting censored gates to missing for all fields

  for (size_t ifield = 0; ifield < fields.size(); ifield++) {
    RadxField *field = fields[ifield];
    if (_checkFieldForCensoring(field)) {
      Radx::fl32 *fdata = (Radx::fl32 *) field->getData();
      Radx::fl32 fmiss = field->getMissingFl32();
      for (size_t igate = 0; igate < nGates; igate++) {
        if (censorFlag[igate] == 1) {
          fdata[igate] = fmiss;
        }
      } // igate
    } // if (_checkFieldForCensoring(field))
  } // ifield

  // convert back to original types
  
  for (size_t ii = 0; ii < fields.size(); ii++) {
    RadxField *field = fields[ii];
    if (_checkFieldForCensoring(field)) {
      field->convertToType(fieldTypes[ii]);
    }
  }

}

////////////////////////////////////////////////////////////////////
// check if a field should be censored

bool Radx2Awips::_checkFieldForCensoring(const RadxField *field)

{

  if (!_params.apply_censoring) {
    return false;
  }

  if (!_params.specify_fields_to_be_censored) {
    return true;
  }

  string checkName = field->getName();

  for (int ii = 0; ii < _params.fields_to_be_censored_n; ii++) {
    string specifiedName = _params._fields_to_be_censored[ii];
    if (checkName == specifiedName) {
      return true;
    }
  }
    
  return false;

}

////////////////////////////////////////////////////////////////////
// add SNR field based on DBZ

int Radx2Awips::_addSnrField(RadxVol &vol)

{

  vector<RadxRay *> &rays = vol.getRays();
  for (size_t ii = 0; ii < rays.size(); ii++) {
    if (_addSnrField(rays[ii])) {
      cerr << "WARNING - SNR field will not be added." << endl;
      return -1;
    }
  }

  return 0;
  
}

////////////////////////////////////////////////////////////////////
// add SNR field in ray

int Radx2Awips::_addSnrField(RadxRay *ray)
  
{
  
  // check SNR field does not already exist

  {
    const RadxField *snrField = ray->getField(_params.SNR_field_name);
    if (snrField != NULL) {
      cerr << "WARNING - SNR field already exists." << endl;
      cerr << "  SNR field name: " << _params.SNR_field_name << endl;
      return -1;
    }
  }
  
  // get DBZ field
  
  const RadxField *dbzField = ray->getField(_params.DBZ_field_name);
  if (dbzField == NULL) {
    cerr << "WARNING - DBZ field does not exist." << endl;
    cerr << "  DBZ field name: " << _params.DBZ_field_name << endl;
    return -1;
  }

  // make copy of dbz field, convert to floats

  RadxField *dbzCopy = new RadxField(*dbzField);
  dbzCopy->convertToFl32();
  size_t nGates = dbzCopy->getNPoints();
  Radx::fl32 missingVal = dbzCopy->getMissingFl32();
  
  // create SNR array, fill with missing
  
  vector<Radx::fl32> snr;
  snr.resize(nGates, missingVal);

  // compute SNR
  
  _computeSnrFromDbz(nGates,
                     dbzCopy->getDataFl32(),
                     snr.data(),
                     missingVal,
                     ray->getStartRangeKm(),
                     ray->getGateSpacingKm());
  
  // create SNR field
  
  RadxField *snrField = new RadxField;

  // set properties

  snrField->setName(_params.SNR_field_name);
  snrField->setLongName("signal_to_noise_ratio");
  snrField->setStandardName("signal_to_noise_ratio_co_polar_h");
  snrField->setUnits("dB");
  snrField->setComment("Computed from reflectivity field");
  snrField->setAncillaryVariables(_params.DBZ_field_name);

  // add data
  
  snrField->setTypeFl32(missingVal);
  snrField->addDataFl32(nGates, snr.data());

  // add SNR field to ray - which takes ownership of the memory

  ray->addField(snrField);

  // clean up

  delete dbzCopy;

  // done
  
  return 0;

}

//////////////////////////////////////////////////////////////
// Compute the SNR field from the DBZ field

void Radx2Awips::_computeSnrFromDbz(size_t nGates,
                                     const Radx::fl32 *dbz,
                                     Radx::fl32 *snr,
                                     Radx::fl32 missingVal,
                                     double startRangeKm,
                                     double gateSpacingKm)

{

  // compute noise at each gate
  
  vector<double> noiseDbz(nGates);
  double range = startRangeKm;
  if (range == 0) {
    range = gateSpacingKm / 10.0;
  }
  for (size_t igate = 0; igate < nGates; igate++, range += gateSpacingKm) {
    noiseDbz[igate] = _params.noise_dbz_at_100km +
      20.0 * (log10(range) - log10(100.0));
  }

  // compute snr from dbz
  
  for (size_t igate = 0; igate < nGates; igate++, snr++, dbz++) {
    if (*dbz != missingVal) {
      *snr = *dbz - noiseDbz[igate];
    } else {
      *snr = -20;
    }
  }
  
}

