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
// Mpd2Radx.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
// Nov 2021
// 
///////////////////////////////////////////////////////////////

#include "Mpd2Radx.hh"
#include <Radx/Radx.hh>
#include <Radx/RadxSweep.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxField.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxTimeList.hh>
#include <Radx/RadxPath.hh>
#include <Radx/RadxArray.hh>
#include <Radx/NcfRadxFile.hh>
#include <dsserver/DsLdataInfo.hh>
#include <didss/DsInputPath.hh>
#include <toolsa/TaXml.hh>
#include <toolsa/toolsa_macros.h>
#include <toolsa/pmu.h>

#include <cmath>  
#include <fstream>
#include <sstream>
#include <string>

#include "MpdNcFile.hh"
#include "Names.hh"

using namespace std;

// Constructor

Mpd2Radx::Mpd2Radx(int argc, char **argv)

{

  OK = TRUE;

  _nGates = 0;
  _gateSpacingKm = 0;
  _startRangeKm = 0;

  // set programe name

  _progName = "Mpd2Radx";
  
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
  
}

// destructor

Mpd2Radx::~Mpd2Radx()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int Mpd2Radx::Run()
{

  int iret = 0;
  if (_params.mode == Params::ARCHIVE) {
    iret = _runArchive();
  } else if (_params.mode == Params::FILELIST) {
    iret = _runFilelist();
  } else if (_params.mode == Params::REALTIME) {
    if (_params.latest_data_info_avail) {
      iret = _runRealtimeWithLdata();
    } else {
      iret = _runRealtimeNoLdata();
    }
  }

  return iret;

}

//////////////////////////////////////////////////
// Run in filelist mode

int Mpd2Radx::_runFilelist()
{

  // loop through the input file list

  int iret = 0;

  for (int ii = 0; ii < (int) _args.inputFileList.size(); ii++) {

    string inputPath = _args.inputFileList[ii];
    if (_processFile(inputPath)) {
      iret = -1;
    }

  }

  return iret;

}

//////////////////////////////////////////////////
// Run in archive mode

int Mpd2Radx::_runArchive()
{

  // get the files to be processed

  RadxTimeList tlist;
  tlist.setDir(_params.input_dir);
  tlist.setModeInterval(_args.startTime, _args.endTime);
  if (tlist.compile()) {
    cerr << "ERROR - Mpd2Radx::_runArchive()" << endl;
    cerr << "  Cannot compile time list, dir: " << _params.input_dir << endl;
    cerr << "  Start time: " << RadxTime::strm(_args.startTime) << endl;
    cerr << "  End time: " << RadxTime::strm(_args.endTime) << endl;
    cerr << tlist.getErrStr() << endl;
    return -1;
  }

  const vector<string> &paths = tlist.getPathList();
  if (paths.size() < 1) {
    cerr << "ERROR - Mpd2Radx::_runArchive()" << endl;
    cerr << "  No files found, dir: " << _params.input_dir << endl;
    return -1;
  }

  // loop through the input file list

  int iret = 0;
  for (size_t ii = 0; ii < paths.size(); ii++) {
    if (_processFile(paths[ii])) {
      iret = -1;
    }
  }

  return iret;

}

//////////////////////////////////////////////////
// Run in realtime mode with latest data info

int Mpd2Radx::_runRealtimeWithLdata()
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

  int iret = 0;
  int msecsWait = _params.wait_between_checks * 1000;
  while (true) {
    ldata.readBlocking(_params.max_realtime_data_age_secs,
                       msecsWait, PMU_auto_register);
    const string path = ldata.getDataPath();
    if (_processFile(path)) {
      iret = -1;
    }
  }

  return iret;

}

//////////////////////////////////////////////////
// Run in realtime mode without latest data info

int Mpd2Radx::_runRealtimeNoLdata()
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

  while(true) { // how does this loop end? --Brad

    // check for new data

    char *path = input.next(false);

    if (path == NULL) {

      // sleep a bit

      PMU_auto_register("Waiting for data");
      umsleep(_params.wait_between_checks * 1000);

    } else {

      // process the file

      if (_processFile(path)) {
        iret = -1;
      }

    }

  } // while

  return iret;

}

//////////////////////////////////////////////////
// Process a file
// Returns 0 on success, -1 on failure

int Mpd2Radx::_processFile(const string &readPath)
{


  PMU_auto_register("Processing file");

  if (_params.debug) {
    cerr << "INFO - Mpd2Radx::_processFile" << endl;
    cerr << "  Input path: " << readPath << endl;
  }

  // check if this is an MPD file

  MpdNcFile file(_params);
  if (file.isMpdNcFile(readPath)) {
    return _processMpdNcFile(readPath);
  }
   
  // error

  cerr << "ERROR - Mpd2Radx::_processFile" << endl;
  cerr << "  File format not supported: " << readPath << endl;
  return -1;

}

//////////////////////////////////////////////////
// set up write

void Mpd2Radx::_setupWrite(RadxFile &file)
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

  file.setWriteCompressed(true);
  file.setCompressionLevel(4);
  file.setWriteLdataInfo(_params.write_latest_data_info);

  // set output format

  file.setFileFormat(RadxFile::FILE_FORMAT_CFRADIAL);
  file.setNcFormat(RadxFile::NETCDF4);

  if (strlen(_params.output_filename_prefix) > 0) {
    file.setWriteFileNamePrefix(_params.output_filename_prefix);
  }

  file.setWriteInstrNameInFileName
    (_params.include_instrument_name_in_file_name);
  
  file.setWriteSiteNameInFileName(false);
  file.setWriteScanTypeInFileName(false);
  file.setWriteVolNumInFileName(false);

  file.setWriteSubsecsInFileName(_params.include_subsecs_in_file_name);

  file.setWriteHyphenInDateTime(_params.use_hyphen_in_file_name_datetime_part);

}

//////////////////////////////////////////////////
// finalize the volume

void Mpd2Radx::_finalizeVol(RadxVol &vol)
{

  if (strlen(_params.instrument_name) > 0) {
    vol.setInstrumentName(_params.instrument_name);
  }

  if (strlen(_params.site_name) > 0) {
    vol.setSiteName(_params.site_name);
  }

}

//////////////////////////////////////////////////
// write out the volume

int Mpd2Radx::_writeVol(RadxVol &vol)
{

  // output file

  NcfRadxFile outFile;
  _setupWrite(outFile);

  if (_params.output_filename_mode == Params::SPECIFY_FILE_NAME) {

    string outPath = _params.output_dir;
    outPath += PATH_DELIM;
    outPath += _params.output_filename;

    // write to path

    if (outFile.writeToPath(vol, outPath)) {
      cerr << "ERROR - Mpd2Radx::_writeVol" << endl;
      cerr << "  Cannot write file to path: " << outPath << endl;
      cerr << outFile.getErrStr() << endl;
      return -1;
    }

  } else {

    // write to dir

    if (outFile.writeToDir(vol, _params.output_dir,
                           _params.append_day_dir_to_output_dir,
                           _params.append_year_dir_to_output_dir)) {
      cerr << "ERROR - Mpd2Radx::_writeVol" << endl;
      cerr << "  Cannot write file to dir: " << _params.output_dir << endl;
      cerr << outFile.getErrStr() << endl;
      return -1;
    }

  }

  string outputPath = outFile.getPathInUse();

  // write latest data info file if requested 
  if (_params.write_latest_data_info) {
    DsLdataInfo ldata(_params.output_dir);
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      ldata.setDebug(true);
    }
    string relPath;
    RadxPath::stripDir(_params.output_dir, outputPath, relPath);
    ldata.setRelDataPath(relPath);
    ldata.setWriter(_progName);
    if (ldata.write(vol.getEndTimeSecs())) {
      cerr << "WARNING - Mpd2Radx::_writeVol" << endl;
      cerr << "  Cannot write latest data info file to dir: "
           << _params.output_dir << endl;
    }
  }

  return 0;

}

//////////////////////////////////////////////////
// Process a Mpd Hayman file
// Returns 0 on success, -1 on failure

int Mpd2Radx::_processMpdNcFile(const string &readPath)
{
   
  PMU_auto_register("Processing Mpd netcdf file");

  if (_params.debug) {
    cerr << "INFO - Mpd2Radx::_processMpdNcFile" << endl;
    cerr << "  Mpd file: " << readPath << endl;
  }

  MpdNcFile inFile(_params);
  
  // read in file
  RadxVol vol;
  if (inFile.readFromPath(readPath, vol)) {
    cerr << "ERROR - Mpd2Radx::_processMpdNcFile" << endl;
    cerr << inFile.getErrStr() << endl;
    return -1;
  }
  
  // vector<RadxRay *> rays = vol.getRays();

  // finalize the volume before writing
  
  _finalizeVol(vol);
  
  // write the file
  if (_writeVol(vol)) {
    cerr << "ERROR - Mpd2Radx::_processMpdNcFile" << endl;
    cerr << "  Cannot write volume to file" << endl;
    return -1;
  }

  return 0;

}


//////////////////////////////////////////////////
// Add in the height, pressure and temperature
// environmental fields

void Mpd2Radx::_addEnvFields(const RadxVol &vol, RadxRay *ray)
{

  double altitudeKm = vol.getAltitudeKm();

  double elevDeg = ray->getElevationDeg();
  size_t nGates = ray->getNGates();

  RadxArray<Radx::fl32> htMeters_, tempK_, presHpa_;
  Radx::fl32 *htMeters = htMeters_.alloc(nGates);
  Radx::fl32 *tempK = tempK_.alloc(nGates);
  Radx::fl32 *presHpa = presHpa_.alloc(nGates);

  double sinEl = sin(elevDeg * Radx::DegToRad);
  double startRangeKm = ray->getStartRangeKm();
  double gateSpacingKm = ray->getGateSpacingKm();

  double rangeKm = startRangeKm;
  for (size_t igate = 0; igate < nGates; igate++, rangeKm += gateSpacingKm) {
    double htm = (altitudeKm + rangeKm * sinEl) * 1000.0;
    htMeters[igate] = htm;
    tempK[igate] = _stdAtmos.ht2temp(htm);
    presHpa[igate] = _stdAtmos.ht2pres(htm);
  }

  if (_params.read_temp_and_pressure_profile_from_model_files) {
    if (_getModelData(ray->getTimeSecs()) == 0) {
      _setProfileFromModel(vol, ray, htMeters, tempK, presHpa);
    }
  }

  RadxField *htField =
    ray->addField(Names::Height, "m", nGates, Radx::missingFl32, htMeters, true);
  htField->setStandardName(Names::height_above_mean_sea_level);
  htField->setLongName(Names::height_above_mean_sea_level);
  htField->setRangeGeom(startRangeKm, gateSpacingKm);

  RadxField *tempField =
    ray->addField(Names::Temperature, "K", nGates, Radx::missingFl32, tempK, true);
  tempField->setStandardName(Names::air_temperature);
  tempField->setLongName(Names::air_temperature);
  tempField->setRangeGeom(startRangeKm, gateSpacingKm);

  RadxField *presField =
    ray->addField(Names::Pressure, "HPa", nGates, Radx::missingFl32, presHpa, true);
  presField->setStandardName(Names::air_pressure);
  presField->setLongName(Names::air_pressure);
  presField->setRangeGeom(startRangeKm, gateSpacingKm);

}

///////////////////////////////////////////////////////
// read in temp and pressure profile from model

int Mpd2Radx::_getModelData(time_t rayTime)

{

  // check if the previously read file will suffice

  time_t prevValidTime = _mdvx.getValidTime();
  double tdiff = fabs((double) prevValidTime - (double) rayTime);
  if (tdiff <= _params.model_profile_search_margin_secs) {
    if (_params.debug >= Params::DEBUG_EXTRA) {
      cerr << "Reusing previously read MDV file for profile" << endl;
      cerr << "  file: " << _mdvx.getPathInUse() << endl;
    }
    return 0;
  }

  // set up the read

  _mdvx.clear();
  _mdvx.setReadTime(Mdvx::READ_CLOSEST,
                    _params.model_profile_mdv_data_url,
                    _params.model_profile_search_margin_secs,
                    rayTime);
  _mdvx.addReadField(_params.model_temperature_field_name);
  _mdvx.addReadField(_params.model_pressure_field_name);
  _mdvx.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  _mdvx.setReadCompressionType(Mdvx::COMPRESSION_NONE);

  // perform the read

  if (_mdvx.readVolume()) {
    cerr << "ERROR - Mpd2Radx::_getModelData()" << endl;
    cerr << "  Cannot read model data for time: "
         << RadxTime::strm(rayTime) << endl;
    cerr << _mdvx.getErrStr() << endl;
    return -1;
  }

  if (_params.debug) {
    cerr << "====>> read model data for temp and pressure profiles" << endl;
    cerr << "  ray time: "
         << RadxTime::strm(rayTime) << endl;
    cerr << "  model valid time: " 
         << RadxTime::strm(_mdvx.getValidTime()) << endl;
    cerr << "  file: " << _mdvx.getPathInUse() << endl;
    cerr << "=====================================================" << endl;
  }

  // get fields

  _modelTempField = _mdvx.getField(_params.model_temperature_field_name);
  if (_modelTempField == NULL) {
    cerr << "ERROR - Mpd2Radx::_getModelData()" << endl;
    cerr << "  No temperaure field found" << endl;
    return -1;
  }
  _modelTempFhdr = _modelTempField->getFieldHeader();
  _modelTempVhdr = _modelTempField->getVlevelHeader();
  _modelTempData = (fl32 *) _modelTempField->getVol();

  _modelPresField = _mdvx.getField(_params.model_pressure_field_name);
  if (_modelPresField == NULL) {
    cerr << "ERROR - Mpd2Radx::_getModelData()" << endl;
    cerr << "  No pressure field found" << endl;
    return -1;
  }
  _modelPresData = (fl32 *) _modelPresField->getVol();

  // set projection

  _modelProj.init(_modelTempFhdr);

  return 0;

}

///////////////////////////////////////////////////////
// set the profile from the model data

int Mpd2Radx::_setProfileFromModel(const RadxVol &vol,
                                   RadxRay *ray,
                                   Radx::fl32 *htMeters,
                                   Radx::fl32 *tempK,
                                   Radx::fl32 *presHpa)

{

  // get lat and lon

  double lat = vol.getLatitudeDeg();
  double lon = vol.getLongitudeDeg();

  // get grid location index

  int xIndex, yIndex;
  if (_modelProj.latlon2xyIndex(lat, lon, xIndex, yIndex, true)) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "ERROR - Mpd2Radx::_setProfileFromModel()" << endl;
      cerr << "  lat,lon outside model domain: " << lat << ", " << lon << endl;
    }
    return -1;
  }
  int nx = _modelTempFhdr.nx;
  int ny = _modelTempFhdr.ny;
  int nz = _modelTempFhdr.nz;
  int planeOffset = yIndex * nx + xIndex;
  int nPointsPlane = ny * nx;

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "lat, lon, xIndex, yIndex: "
         << lat << ", "
         << lon << ", "
         << xIndex << ", "
         << yIndex << endl;
  }

  // load up temp and pressure column arrays from model

  RadxArray<Radx::fl32> modelTempK_, modelPresHpa_;
  Radx::fl32 *modelTempK = modelTempK_.alloc(nz);
  Radx::fl32 *modelPresHpa = modelPresHpa_.alloc(nz);

  for (int iz = 0; iz < nz; iz++) {
    long int dataOffset = iz * nPointsPlane + planeOffset;
    if (_params.temperature_profile_units == Params::DEGREES_CELCIUS) {
      modelTempK[iz] = TEMP_C_TO_K(_modelTempData[dataOffset]);
    } else {
      modelTempK[iz] = _modelTempData[dataOffset];
    }
    if (_params.pressure_profile_units == Params::PA) {
      modelPresHpa[iz] = _modelPresData[dataOffset] / 100.0;
    } else {
      modelPresHpa[iz] = _modelPresData[dataOffset];
    }
    // cerr << "11111111 iz, ht, temp0, temp, pres0, pres: " << iz << ", "
    //      << vhdr.level[iz] << ", "
    //      << tempData[iz * nPointsPlane] << ", "
    //      << modelTempK[iz] << ", "
    //      << presData[iz * nPointsPlane] << ", "
    //      << modelPresHpa[iz] << endl;
  } // iz

  // interpolate temp and pressure from model onto ray

  size_t nGates = ray->getNGates();

  if (ray->getElevationDeg() > 0) {

    // pointing up

    int startIz = 1;
    for (size_t igate = 0; igate < nGates; igate++) {
      double htKm = htMeters[igate] / 1000.0;
      if (htKm <= _modelTempVhdr.level[0]) {
        tempK[igate] = modelTempK[0];
        presHpa[igate] = modelPresHpa[0];
      } else if (htKm >= _modelTempVhdr.level[nz - 1]) {
        tempK[igate] = modelTempK[nz - 1];
        presHpa[igate] = modelPresHpa[nz - 1];
      } else {
        for (int iz = startIz; iz < nz; iz++) {
          if (htKm > _modelTempVhdr.level[iz - 1] &&
              htKm <= _modelTempVhdr.level[iz]) {
            double wt1 =
              (htKm - _modelTempVhdr.level[iz - 1]) / 
              (_modelTempVhdr.level[iz] - _modelTempVhdr.level[iz - 1]);
            double wt0 = 1.0 - wt1;
            tempK[igate] = modelTempK[iz - 1] * wt0 + modelTempK[iz] * wt1;
            presHpa[igate] = modelPresHpa[iz - 1] * wt0 + modelPresHpa[iz] * wt1;
            startIz = iz;
            break;
          } // if (htKm > ...
        } // iz
      } // if (htKm <= ...
    } // igate

  } else {

    // pointing down

    int startIz = nz - 1;
    for (size_t igate = 0; igate < nGates; igate++) {
      double htKm = htMeters[igate] / 1000.0;
      if (htKm <= _modelTempVhdr.level[0]) {
        tempK[igate] = modelTempK[0];
        presHpa[igate] = modelPresHpa[0];
      } else if (htKm >= _modelTempVhdr.level[nz - 1]) {
        tempK[igate] = modelTempK[nz - 1];
        presHpa[igate] = modelPresHpa[nz - 1];
      } else {
        for (int iz = startIz; iz > 0; iz--) {
          if (htKm > _modelTempVhdr.level[iz - 1] &&
              htKm <= _modelTempVhdr.level[iz]) {
            double wt1 =
              (htKm - _modelTempVhdr.level[iz - 1]) / 
              (_modelTempVhdr.level[iz] - _modelTempVhdr.level[iz - 1]);
            double wt0 = 1.0 - wt1;
            tempK[igate] = modelTempK[iz - 1] * wt0 + modelTempK[iz] * wt1;
            presHpa[igate] = modelPresHpa[iz - 1] * wt0 + modelPresHpa[iz] * wt1;
            startIz = iz;
            break;
          } // if (htKm > ...
        } // iz
      } // if (htKm <= ...
    } // igate

  } // if (ray->getElevationDeg() > 0) 
     
  return 0;
  
}

