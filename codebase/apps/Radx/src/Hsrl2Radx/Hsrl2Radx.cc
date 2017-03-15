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
// Hsrl2Radx.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2015
//
///////////////////////////////////////////////////////////////

#include "Hsrl2Radx.hh"
#include <Radx/Radx.hh>
#include <Radx/RadxVol.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxField.hh>
#include <Radx/RadxGeoref.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxTimeList.hh>
#include <Radx/RadxPath.hh>
#include <Radx/RadxArray.hh>
#include <Radx/NcfRadxFile.hh>
#include <dsserver/DsLdataInfo.hh>
#include <didss/DsInputPath.hh>
#include <toolsa/TaXml.hh>
#include <toolsa/pmu.h>
#include <physics/IcaoStdAtmos.hh>
#include "MslFile.hh"
#include "RawFile.hh"
#include "CalReader.hh"
#include "FullCals.hh"
#include <math.h>  

#include <fstream>
#include <sstream>
#include <string>

using namespace std;

// Constructor

Hsrl2Radx::Hsrl2Radx(int argc, char **argv)
  
{

  OK = TRUE;

  // set programe name

  _progName = "Hsrl2Radx";
  
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

  if (_params.override_instrument_location) {
    if (_params.instrument_latitude_deg < -900 ||
        _params.instrument_longitude_deg < -900 ||
        _params.instrument_altitude_meters < -900) {
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

Hsrl2Radx::~Hsrl2Radx()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int Hsrl2Radx::Run()
{
  // reading in calibration files here ----- 
  
  _cals.setDeadTimeHi(_params.calvals_gvhsrl_path, 
			  _params.combined_hi_dead_time_name);
  _cals.setDeadTimeLo(_params.calvals_gvhsrl_path, 
			  _params.combined_hi_dead_time_name);
  _cals.setDeadTimeCross(_params.calvals_gvhsrl_path, 
			  _params.combined_hi_dead_time_name);
  _cals.setDeadTimeMol(_params.calvals_gvhsrl_path, 
			  _params.combined_hi_dead_time_name);
  _cals.setBinWidth(_params.calvals_gvhsrl_path, 
			  _params.combined_hi_dead_time_name); 

  _cals.setBLCor(_params.baseline_calibration_path);
  _cals.setDiffDGeoCor(_params.diff_default_geofile_path);
  _cals.setGeoDefCor(_params.geofile_default_path);
  _cals.setAfPulCor(_params.afterpulse_default_path);
    
  //done reading calibration files. 

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

int Hsrl2Radx::_runFilelist()
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

int Hsrl2Radx::_runArchive()
{

  // get the files to be processed

  RadxTimeList tlist;
  tlist.setDir(_params.input_dir);
  tlist.setModeInterval(_args.startTime, _args.endTime);
  if (tlist.compile()) {
    cerr << "ERROR - Hsrl2Radx::_runArchive()" << endl;
    cerr << "  Cannot compile time list, dir: " << _params.input_dir << endl;
    cerr << "  Start time: " << RadxTime::strm(_args.startTime) << endl;
    cerr << "  End time: " << RadxTime::strm(_args.endTime) << endl;
    cerr << tlist.getErrStr() << endl;
    return -1;
  }

  const vector<string> &paths = tlist.getPathList();
  if (paths.size() < 1) {
    cerr << "ERROR - Hsrl2Radx::_runArchive()" << endl;
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

int Hsrl2Radx::_runRealtimeWithLdata()
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

int Hsrl2Radx::_runRealtimeNoLdata()
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

int Hsrl2Radx::_processFile(const string &readPath)
{


  PMU_auto_register("Processing file");
  
  if (_params.debug) {
    cerr << "INFO - Hsrl2Radx::_processFile" << endl;
    cerr << "  Input path: " << readPath << endl;
  }

  // check if this is a CfRadial file

  NcfRadxFile file;
  if (file.isCfRadial(readPath)) {
    return _processUwCfRadialFile(readPath);
  } else {
    return _processUwRawFile(readPath);
  }

}

//////////////////////////////////////////////////
// Process a UofWisc pseudo CfRadial file
// Returns 0 on success, -1 on failure

int Hsrl2Radx::_processUwCfRadialFile(const string &readPath)
{

  PMU_auto_register("Processing UW CfRadial file");

  if (_params.debug) {
    cerr << "INFO - Hsrl2Radx::_processUwCfRadialFile" << endl;
    cerr << "  UW CfRadial file: " << readPath << endl;
  }

  MslFile inFile(_params);
  _setupRead(inFile);
  
  // read in file

  RadxVol vol;
  if (inFile.readFromPath(readPath, vol)) {
    cerr << "ERROR - Hsrl2Radx::Run" << endl;
    cerr << inFile.getErrStr() << endl;
    return -1;
  }
  _readPaths = inFile.getReadPaths();
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    for (size_t ii = 0; ii < _readPaths.size(); ii++) {
      cerr << "  ==>> read in file: " << _readPaths[ii] << endl;
    }
  }

  // remove unwanted fields

  if (_params.exclude_specified_fields) {
    for (int ii = 0; ii < _params.excluded_fields_n; ii++) {
      if (_params.debug) {
        cerr << "Removing field name: " << _params._excluded_fields[ii] << endl;
      }
      vol.removeField(_params._excluded_fields[ii]);
    }
  }

  // convert to floats

  vol.convertToFl32();

  // override radar location if requested
  
  if (_params.override_instrument_location) {
    vol.overrideLocation(_params.instrument_latitude_deg,
                         _params.instrument_longitude_deg,
                         _params.instrument_altitude_meters / 1000.0);
  }
    
  // override radar name and site name if requested
  
  if (_params.override_instrument_name) {
    vol.setInstrumentName(_params.instrument_name);
  }
  if (_params.override_site_name) {
    vol.setSiteName(_params.site_name);
  }
    
  // apply angle offsets

  if (_params.apply_azimuth_offset) {
    if (_params.debug) {
      cerr << "NOTE: applying azimuth offset (deg): " 
           << _params.azimuth_offset << endl;
    }
    vol.applyAzimuthOffset(_params.azimuth_offset);
  }
  if (_params.apply_elevation_offset) {
    if (_params.debug) {
      cerr << "NOTE: applying elevation offset (deg): " 
           << _params.elevation_offset << endl;
    }
    vol.applyElevationOffset(_params.elevation_offset);
  }

  // override start range and/or gate spacing

  if (_params.override_start_range || _params.override_gate_spacing) {
    _overrideGateGeometry(vol);
  }

  // set the range relative to the instrument location

  if (_params.set_range_relative_to_instrument) {
    _setRangeRelToInstrument(inFile, vol);
  }
  
  // set vol values

  vol.loadSweepInfoFromRays();
  vol.loadVolumeInfoFromRays();

  // vol.printWithRayMetaData(cerr);

  // set field type, names, units etc
  
  _convertFields(vol);

  // set global attributes as needed

  _setGlobalAttr(vol);

  // write the file

  if (_writeVol(vol)) {
    cerr << "ERROR - Hsrl2Radx::_processFile" << endl;
    cerr << "  Cannot write volume to file" << endl;
    return -1;
  }

  return 0;

}

//////////////////////////////////////////////////
// set up read

void Hsrl2Radx::_setupRead(MslFile &file)
{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    file.setDebug(true);
  }
  if (_params.debug >= Params::DEBUG_EXTRA) {
    file.setVerbose(true);
  }

  if (!_params.write_other_fields_unchanged) {

    if (_params.set_output_fields) {
      for (int ii = 0; ii < _params.output_fields_n; ii++) {
        file.addReadField(_params._output_fields[ii].input_field_name);
      }
    }

  }

  if (_params.debug >= Params::DEBUG_EXTRA) {
    file.printReadRequest(cerr);
  }

}

//////////////////////////////////////////////////
// override the gate geometry

void Hsrl2Radx::_overrideGateGeometry(RadxVol &vol)

{

  vector<RadxRay *> &rays = vol.getRays();
  
  for (size_t iray = 0; iray < rays.size(); iray++) {
    
    RadxRay *ray = rays[iray];

    double startRange = ray->getStartRangeKm();
    double gateSpacing = ray->getGateSpacingKm();

    if (_params.override_start_range) {
      startRange = _params.start_range_km;
    }
  
    if (_params.override_gate_spacing) {
      gateSpacing = _params.gate_spacing_km;
    }

    ray->setRangeGeom(startRange, gateSpacing);

  } // iray

}

//////////////////////////////////////////////////
// set the range relative to the instrument location

void Hsrl2Radx::_setRangeRelToInstrument(MslFile &file,
                                         RadxVol &vol)

{

  // telescope direction - 0 is down, 1 is up

  const vector<int> &telescopeDirection = file.getTelescopeDirection();

  vector<RadxRay *> &rays = vol.getRays();
  
  for (size_t iray = 0; iray < rays.size(); iray++) {
    
    const RadxRay *ray = rays[iray];
    const RadxGeoref *georef = ray->getGeoreference();
    double altitudeKm = georef->getAltitudeKmMsl();

    int nGatesIn = ray->getNGates();
    double gateSpacingKm = ray->getGateSpacingKm();
    int altitudeInGates = (int) floor(altitudeKm / gateSpacingKm + 0.5);
    if (altitudeInGates > nGatesIn - 1) {
      altitudeInGates = nGatesIn - 1;
    }

    int telDirn = telescopeDirection[iray];
    bool isUp = (telDirn == _params.telescope_direction_is_up);

    int nGatesValid = nGatesIn;
    if (isUp) {
      nGatesValid = nGatesIn - altitudeInGates;
    } else {
      nGatesValid = altitudeInGates + 1;
    }
    if (nGatesValid < 0) {
      nGatesValid = 0;
    }
    if (nGatesValid > nGatesIn) {
      nGatesValid = nGatesIn;
    }

    vector<RadxField *> fields = ray->getFields();
    for (size_t ifield = 0; ifield < fields.size(); ifield++) {
      RadxField *field = fields[ifield];
      const Radx::fl32 *dataIn = field->getDataFl32();
      RadxArray<Radx::fl32> dataOut_;
      Radx::fl32 *dataOut = dataOut_.alloc(nGatesIn);
      for (int ii = 0; ii < nGatesIn; ii++) {
        dataOut[ii] = field->getMissingFl32();
      }
      if (isUp) {
        for (int ii = 0; ii < nGatesValid; ii++) {
          dataOut[ii] = dataIn[altitudeInGates + ii];
        }
      } else {
        for (int ii = 0; ii < nGatesValid; ii++) {
          dataOut[ii] = dataIn[altitudeInGates - ii];
        }
      }
      field->setDataFl32(nGatesIn, dataOut, true);
    } // ifield
    
  } // iray

}
  
//////////////////////////////////////////////////
// rename fields as required

void Hsrl2Radx::_convertFields(RadxVol &vol)
{

  if (!_params.set_output_fields) {
    return;
  }

  for (int ii = 0; ii < _params.output_fields_n; ii++) {

    const Params::output_field_t &ofld = _params._output_fields[ii];
    
    string iname = ofld.input_field_name;
    string oname = ofld.output_field_name;
    string lname = ofld.long_name;
    string sname = ofld.standard_name;
    string ounits = ofld.output_units;
    
  }

}

//////////////////////////////////////////////////
// set up write

void Hsrl2Radx::_setupWrite(RadxFile &file)
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
  file.setCompressionLevel(_params.compression_level);

  // set output format

  file.setFileFormat(RadxFile::FILE_FORMAT_CFRADIAL);

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

  if (strlen(_params.output_filename_prefix) > 0) {
    file.setWriteFileNamePrefix(_params.output_filename_prefix);
  }

  file.setWriteInstrNameInFileName(_params.include_instrument_name_in_file_name);
  file.setWriteSubsecsInFileName(_params.include_subsecs_in_file_name);
  file.setWriteHyphenInDateTime(_params.use_hyphen_in_file_name_datetime_part);

}

//////////////////////////////////////////////////
// set selected global attributes

void Hsrl2Radx::_setGlobalAttr(RadxVol &vol)
{

  vol.setDriver("Hsrl2Radx(NCAR)");

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

}

//////////////////////////////////////////////////
// write out the volume

int Hsrl2Radx::_writeVol(RadxVol &vol)
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
      cerr << "ERROR - Hsrl2Radx::_writeVol" << endl;
      cerr << "  Cannot write file to path: " << outPath << endl;
      cerr << outFile.getErrStr() << endl;
      return -1;
    }
      
  } else {

    // write to dir
  
    if (outFile.writeToDir(vol, _params.output_dir,
                           _params.append_day_dir_to_output_dir,
                           _params.append_year_dir_to_output_dir)) {
      cerr << "ERROR - Hsrl2Radx::_writeVol" << endl;
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
      cerr << "WARNING - Hsrl2Radx::_writeVol" << endl;
      cerr << "  Cannot write latest data info file to dir: "
           << _params.output_dir << endl;
    }
  }

  return 0;

}

//////////////////////////////////////////////////
// Process a UofWisc raw netcdf file
// Returns 0 on success, -1 on failure

int Hsrl2Radx::_processUwRawFile(const string &readPath)
{

  PMU_auto_register("Processing UW Raw file");
  
  if (_params.debug) {
    cerr << "INFO - Hsrl2Radx::_processUwRawFile" << endl;
    cerr << "  UW raw file: " << readPath << endl;
  }

  RawFile inFile(_params);
  
  // read in file
    RadxVol vol;
  if (inFile.readFromPath(readPath, vol)) {
    cerr << "ERROR - Hsrl2Radx::Run" << endl;
    cerr << inFile.getErrStr() << endl;
    return -1;
  }
  
  // override radar name and site name if requested
    if (_params.override_instrument_name) {
    vol.setInstrumentName(_params.instrument_name);
  }
  if (_params.override_site_name) {
    vol.setSiteName(_params.site_name);
  }
    
  // set global attributes as needed
    _setGlobalAttr(vol);

  // add in height, temperature and pressure fields
  _addEnvFields(vol);

  //add corrections and derived data outputs  
  _addDerivedFields(vol);

  // write the file
  if (_writeVol(vol)) {
    cerr << "ERROR - Hsrl2Radx::_processFile" << endl;
    cerr << "  Cannot write volume to file" << endl;
    return -1;
  }

  return 0;

}


//////////////////////////////////////////////////
// Add in the height, pressure and temperature
// environmental fields

void Hsrl2Radx::_addEnvFields(RadxVol &vol)
{
  
  IcaoStdAtmos stdAtmos;

  // loop through the rays

  vector<RadxRay *> rays = vol.getRays();
  
  //cout<<"rays.size()="<<rays.size()<<'\n';
  
  for(size_t iray = 0; iray < rays.size(); iray++) {

    RadxRay *ray = rays[iray];
    const RadxGeoref *geo = ray->getGeoreference();
    if (!geo) {
      continue;
    }

    double altitudeKm = geo->getAltitudeKmMsl();
    double elevDeg = ray->getElevationDeg();
    
    size_t nGates = ray->getNGates();

    //cout<<"nGates="<<nGates<<'\n';
    
    RadxArray<Radx::fl32> htKm_, tempK_, presHpa_,testThing_;
    Radx::fl32 *htKm = htKm_.alloc(nGates);
    Radx::fl32 *tempK = tempK_.alloc(nGates);
    Radx::fl32 *presHpa = presHpa_.alloc(nGates);

    double sinEl = sin(elevDeg * Radx::DegToRad);
    double startRangeKm = ray->getStartRangeKm();
    double gateSpacingKm = ray->getGateSpacingKm();

    double rangeKm = startRangeKm;
    for (size_t igate = 0; igate < nGates; igate++, rangeKm += gateSpacingKm) {
      htKm[igate] = altitudeKm + rangeKm * sinEl;
      double htM = htKm[igate] * 1000.0;
      tempK[igate] = stdAtmos.ht2temp(htM);
      presHpa[igate] = stdAtmos.ht2pres(htM);
    }

    RadxField *htField =
      ray->addField("height", "km", nGates, Radx::missingFl32, htKm, true);
    htField->setLongName("height_MSL");
    htField->setRangeGeom(startRangeKm, gateSpacingKm);

    RadxField *tempField =
      ray->addField("temperature", "K", nGates, Radx::missingFl32, tempK, true);
    tempField->setLongName("temperaure_from_std_atmos");
    tempField->setRangeGeom(startRangeKm, gateSpacingKm);

    RadxField *presField =
      ray->addField("pressure", "HPa", nGates, Radx::missingFl32, presHpa, true);
    presField->setLongName("pressure_from_std_atmos");
    presField->setRangeGeom(startRangeKm, gateSpacingKm);
    
    
  } // iray
  

}

void Hsrl2Radx::_addDerivedFields(RadxVol &vol)
{

  bool debug=false;
  
  IcaoStdAtmos stdAtmos;

  // loop through the rays
  vector<RadxRay *> rays = vol.getRays();
  
  for(size_t iray = 0; iray < rays.size(); iray++) {
    
    RadxRay *ray = rays[iray];
    const RadxGeoref *geo = ray->getGeoreference();
    if (!geo) {
      continue;
    }

    double altitudeKm = geo->getAltitudeKmMsl();
    double elevDeg = ray->getElevationDeg();
    
    size_t nGates = ray->getNGates();
        
    //get more info from ray here
    
    double power = ray->getMeasXmitPowerDbmH();
    
    double shotCount=ray->getNSamples();
    
    RadxArray<Radx::fl32> htKm_, tempK_, presHpa_,testThing_;
    Radx::fl32 *htKm = htKm_.alloc(nGates);
    Radx::fl32 *tempK = tempK_.alloc(nGates);
    Radx::fl32 *presHpa = presHpa_.alloc(nGates);
    Radx::fl32 *testThing = testThing_.alloc(nGates);
    
    double sinEl = sin(elevDeg * Radx::DegToRad);
    double startRangeKm = ray->getStartRangeKm();
    double gateSpacingKm = ray->getGateSpacingKm();

    double rangeKm = startRangeKm;
    for (size_t igate = 0; igate < nGates; igate++, rangeKm += gateSpacingKm) {
      htKm[igate] = altitudeKm + rangeKm * sinEl;
      double htM = htKm[igate] * 1000.0;
      tempK[igate] = stdAtmos.ht2temp(htM);
      presHpa[igate] = stdAtmos.ht2pres(htM);
      testThing[igate] = igate;
    }
    /*
    RadxField *htField =
      ray->addField("height", "km", nGates, Radx::missingFl32, htKm, true);
    htField->setLongName("height_MSL");
    htField->setRangeGeom(startRangeKm, gateSpacingKm);

    RadxField *tempField =
      ray->addField("temperature", "K", nGates, Radx::missingFl32, tempK, true);
    tempField->setLongName("temperaure_from_std_atmos");
    tempField->setRangeGeom(startRangeKm, gateSpacingKm);

    RadxField *presField =
      ray->addField("pressure", "HPa", nGates, Radx::missingFl32, presHpa, true);
    presField->setLongName("pressure_from_std_atmos");
    presField->setRangeGeom(startRangeKm, gateSpacingKm);
    */      
   
    //get raw data fields
    const RadxField *hiField = ray->getField(_params.combined_hi_field_name);
    const RadxField *loField = ray->getField(_params.combined_lo_field_name);
    const RadxField *crossField = ray->getField(_params.cross_field_name);
    const RadxField *molField = ray->getField(_params.molecular_field_name);

    //holds derived data fields
    RadxArray<Radx::fl32> volDepol_, backscatRatio_, partDepol_, 
      backscatCoeff_, extinction_;
    Radx::fl32 *volDepol = volDepol_.alloc(nGates);
    Radx::fl32 *backscatRatio = backscatRatio_.alloc(nGates);
    Radx::fl32 *partDepol = partDepol_.alloc(nGates);
    Radx::fl32 *backscatCoeff = backscatCoeff_.alloc(nGates);
    Radx::fl32 *extinction = extinction_.alloc(nGates);
    
    if(hiField != NULL && loField != NULL && molField != NULL && crossField != NULL)
      {
	const Radx::fl32 *hiData = hiField->getDataFl32();
	const Radx::fl32 *loData = loField->getDataFl32();
	const Radx::fl32 *crossData = crossField->getDataFl32();
	const Radx::fl32 *molData = molField->getDataFl32();
	
	//apply correction and calibration functions here 

	vector<double> hiDataRate;
	vector<double> loDataRate;
	vector<double> crossDataRate;
	vector<double> molDataRate;
	
	CalReader dt_hi=(_cals.getDeadTimeHi());
	CalReader dt_lo=(_cals.getDeadTimeLo());
	CalReader dt_cross=(_cals.getDeadTimeCross());
	CalReader dt_mol=(_cals.getDeadTimeMol());
	CalReader binwid=(_cals.getBinWidth());
	
	for(unsigned int igate=0;igate<nGates;igate++)
	  {
	    if(dt_hi.dataTypeisNum() && 
	       ( ( dt_hi.getDataNum() ).at(_cals.getHiPos()) ).size()==1 && 
	         dt_lo.dataTypeisNum() && 
	       ( ( dt_lo.getDataNum() ).at(_cals.getLoPos()) ).size()==1 && 
	       dt_cross.dataTypeisNum() && 
	       ( ( dt_cross.getDataNum() ).at(_cals.getCrossPos()) ).size()==1 && 
	       dt_mol.dataTypeisNum() &&   
	       ( ( dt_mol.getDataNum() ).at(_cals.getMolPos()) ).size()==1 &&
	       binwid.dataTypeisNum() && 
	       ( ( binwid.getDataNum() ).at(_cals.getBinPos()) ).size()==1 )
	      {
	    	
		double hiDeadTime=((dt_hi.getDataNum()).at(_cals.getHiPos())).at(0);
		double loDeadTime=((dt_lo.getDataNum()).at(_cals.getLoPos())).at(0);
		double crossDeadTime=((dt_cross.getDataNum()).at(_cals.getCrossPos())).at(0);
		double molDeadTime=((dt_mol.getDataNum()).at(_cals.getMolPos())).at(0);
		double binW= ((binwid.getDataNum()).at(_cals.getBinPos())).at(0);
		hiDataRate.push_back(_nonLinCountCor(hiData[igate], hiDeadTime, binW, shotCount)); 
		loDataRate.push_back(_nonLinCountCor(loData[igate], loDeadTime, binW, shotCount)); 
		crossDataRate.push_back(_nonLinCountCor(crossData[igate], 
							crossDeadTime, binW, shotCount)); 
		molDataRate.push_back(_nonLinCountCor(molData[igate], 
						      molDeadTime, binW, shotCount)); 
	      }
	    
	  }
 
	if(debug)
	  for(unsigned int igate=0;igate<1;igate++)
	    {
	      cout<<"hiDataRate["<<igate<<"]="<<hiDataRate[igate]<<'\n';
	      cout<<"loDataRate["<<igate<<"]="<<loDataRate[igate]<<'\n';
	      cout<<"crossDataRate["<<igate<<"]="<<crossDataRate[igate]<<'\n';
	      cout<<"molDataRate["<<igate<<"]="<<molDataRate[igate]<<'\n';
	    }
	if(debug)
	  cout<<"baselineSubtract^^^^^"<<'\n';

	vector< vector<double> > blCor=(_cals.getBLCor());
	int blCorSize=(blCor.at(0)).size();
	for(unsigned int igate=0;igate<nGates;igate++)
	  {
	    //need pol baseline from file to replace 1.0
	    //_baselineSubtract is a passthrough function for now anyway

	    int calGate= blCorSize/nGates * igate +  0.5 * blCorSize/nGates;
	    
	    hiDataRate.at(igate)=_baselineSubtract(hiDataRate.at(igate),
						   (blCor.at(1)).at(calGate),1.0);
	    loDataRate.at(igate)=_baselineSubtract(loDataRate.at(igate),
						   (blCor.at(2)).at(calGate),1.0);
	    crossDataRate.at(igate)=_baselineSubtract(crossDataRate.at(igate),
						      (blCor.at(3)).at(calGate),1.0);
	    molDataRate.at(igate)=_baselineSubtract(molDataRate.at(igate),
						    (blCor.at(4)).at(calGate),1.0);
	  }
	
	if(debug)
	  for(unsigned int igate=0;igate<1;igate++)
	    {
	      cout<<"hiDataRate["<<igate<<"]="<<hiDataRate[igate]<<'\n';
	      cout<<"loDataRate["<<igate<<"]="<<loDataRate[igate]<<'\n';
	      cout<<"crossDataRate["<<igate<<"]="<<crossDataRate[igate]<<'\n';
	      cout<<"molDataRate["<<igate<<"]="<<molDataRate[igate]<<'\n';
	    }
	
	double hibackgroundRate=0;
	double lobackgroundRate=0;
	double crossbackgroundRate=0;
	double molbackgroundRate=0;
	if(debug)
	  cout<<"backgroundSub^^^^^"<<'\n';

	//grabs last 100 out of 4000 bins for background, or fractionally adjusts for less bins
	int bgBinCount=0;
	for(unsigned int cgate=nGates-1;cgate>=nGates*0.975;cgate--)
	  {
	    hibackgroundRate=hibackgroundRate+hiDataRate.at(cgate);
	    lobackgroundRate=lobackgroundRate+loDataRate.at(cgate);
	    crossbackgroundRate=crossbackgroundRate+crossDataRate.at(cgate);
	    molbackgroundRate=molbackgroundRate+molDataRate.at(cgate);
	    bgBinCount++;	    
	  }
	hibackgroundRate/=bgBinCount;
	lobackgroundRate/=bgBinCount;
	crossbackgroundRate/=bgBinCount;
	molbackgroundRate/=bgBinCount;

	for(unsigned int igate=0;igate<nGates;igate++)
	  {
	    hiDataRate.at(igate)=_backgroundSub(hiDataRate.at(igate),hibackgroundRate);
	    loDataRate.at(igate)=_backgroundSub(loDataRate.at(igate),lobackgroundRate);
	    crossDataRate.at(igate)=_backgroundSub(crossDataRate.at(igate),crossbackgroundRate);
	    molDataRate.at(igate)=_backgroundSub(molDataRate.at(igate),molbackgroundRate);
	  }
	if(debug)
	  {
	    cout<<"hibackgroundRate="<<hibackgroundRate<<'\n';
	    cout<<"lobackgroundRate="<<lobackgroundRate<<'\n';
	    cout<<"crossbackgroundRate="<<crossbackgroundRate<<'\n';
	    cout<<"molbackgroundRate="<<molbackgroundRate<<'\n';
	    
	    for(unsigned int igate=0;igate<1;igate++)
	      {
		cout<<"hiDataRate["<<igate<<"]="<<hiDataRate[igate]<<'\n';
		cout<<"loDataRate["<<igate<<"]="<<loDataRate[igate]<<'\n';
		cout<<"crossDataRate["<<igate<<"]="<<crossDataRate[igate]<<'\n';
		cout<<"molDataRate["<<igate<<"]="<<molDataRate[igate]<<'\n';
	      }
	    
	    cout<<"energyNorm^^^^^"<<'\n';
	  }

	for(unsigned int igate=0;igate<nGates;igate++)
	  {
	    hiDataRate.at(igate)=_energyNorm(hiDataRate.at(igate), power);
	    loDataRate.at(igate)=_energyNorm(loDataRate.at(igate), power);
	    crossDataRate.at(igate)=_energyNorm(crossDataRate.at(igate), power);
	    molDataRate.at(igate)=_energyNorm(molDataRate.at(igate), power);
	  }
	
	if(debug)
	  for(unsigned int igate=0;igate<1;igate++)
	    {
	      cout<<"hiDataRate["<<igate<<"]="<<hiDataRate[igate]<<'\n';
	      cout<<"loDataRate["<<igate<<"]="<<loDataRate[igate]<<'\n';
	      cout<<"crossDataRate["<<igate<<"]="<<crossDataRate[igate]<<'\n';
	      cout<<"molDataRate["<<igate<<"]="<<molDataRate[igate]<<'\n';
	    }
	if(debug)
	  cout<<"diffOverlapCor^^^^^"<<'\n';
	
	vector< vector<double> > diffDGeo=(_cals.getDiffDGeoCor());
	int diffDGeoSize=(diffDGeo.at(1)).size();	
	
	for(unsigned int igate=0;igate<nGates;igate++)
	  {

	    int calGate= diffDGeoSize/nGates * igate + 0.5 * diffDGeoSize/nGates;

	    vector<double> rates;
	    vector<double> diffOverlap;
	    
	    rates.push_back(hiDataRate.at(igate));
	    diffOverlap.push_back( (diffDGeo.at(1)).at(calGate) );
	    rates.push_back(loDataRate.at(igate));
	    diffOverlap.push_back( (diffDGeo.at(2)).at(calGate) );
	    rates.push_back(crossDataRate.at(igate));
	    diffOverlap.push_back( 1.0 ); 
	    // ***** need cross overlap correction from another file
	    rates.push_back(molDataRate.at(igate));
	    diffOverlap.push_back( 1.0 );
	    
	    rates=_diffOverlapCor(rates, diffOverlap);
	    
	    hiDataRate.at(igate)=rates.at(0);
	    loDataRate.at(igate)=rates.at(1);
	    crossDataRate.at(igate)=rates.at(2);
	    molDataRate.at(igate)=rates.at(3);
	  }
	
	if(debug)
	  for(unsigned int igate=0;igate<1;igate++)
	    {
	      cout<<"hiDataRate["<<igate<<"]="<<hiDataRate[igate]<<'\n';
	      cout<<"loDataRate["<<igate<<"]="<<loDataRate[igate]<<'\n';
	      cout<<"crossDataRate["<<igate<<"]="<<crossDataRate[igate]<<'\n';
	      cout<<"molDataRate["<<igate<<"]="<<molDataRate[igate]<<'\n';
	    }
	if(debug)
	  cout<<"processQWPRotation^^^^^"<<'\n';
	
	for(unsigned int igate=0;igate<nGates;igate++)
	  {
	    vector<double> rates;
	    vector<double> polCal;
	    rates.push_back(hiDataRate.at(igate));
	    polCal.push_back( 1.0 );
	    rates.push_back(loDataRate.at(igate));
	    polCal.push_back( 1.0 );
	    rates.push_back(crossDataRate.at(igate));
	    polCal.push_back( 1.0 );
	    rates.push_back(molDataRate.at(igate));
	    polCal.push_back( 1.0 );
	    // need to replace 1.0 with polarization calibration info, 
	    // _processQWPRotation is passthrough for now though 
	    rates=_processQWPRotation(rates, polCal);
	  
	    hiDataRate.at(igate)=rates.at(0);
	    loDataRate.at(igate)=rates.at(1);
	    crossDataRate.at(igate)=rates.at(2);
	    molDataRate.at(igate)=rates.at(3);
	  
	  }
	
	if(debug)
	  for(unsigned int igate=0;igate<1;igate++)
	    {
	      cout<<"hiDataRate["<<igate<<"]="<<hiDataRate[igate]<<'\n';
	      cout<<"loDataRate["<<igate<<"]="<<loDataRate[igate]<<'\n';
	      cout<<"crossDataRate["<<igate<<"]="<<crossDataRate[igate]<<'\n';
	      cout<<"molDataRate["<<igate<<"]="<<molDataRate[igate]<<'\n';
	    }
	if(debug)
	  cout<<"hiAndloMerge^^^^^"<<'\n';
	
	vector<double> combineRate;
	
	for(unsigned int igate=0;igate<nGates;igate++)
	  {
	    combineRate.push_back(_hiAndloMerge(hiDataRate.at(igate), loDataRate.at(igate)));
	  }
	
	if(debug)
	  for(unsigned int igate=0;igate<1;igate++)
	    {
	      cout<<"hiDataRate["<<igate<<"]="<<hiDataRate[igate]<<'\n';
	      cout<<"loDataRate["<<igate<<"]="<<loDataRate[igate]<<'\n';
	      cout<<"crossDataRate["<<igate<<"]="<<crossDataRate[igate]<<'\n';
	      cout<<"molDataRate["<<igate<<"]="<<molDataRate[igate]<<'\n';
	      cout<<"combineRate["<<igate<<"]="<<combineRate[igate]<<'\n';
	    }
	if(debug)
	  cout<<"geoOverlapCor^^^^^"<<'\n';
	
	vector< vector<double> > geoDef=(_cals.getGeoDefCor());
	int geoDefSize=(geoDef.at(1)).size();	
	
	for(unsigned int igate=0;igate<nGates;igate++)
	  {
	    int calGate= geoDefSize/nGates * igate + 0.5*geoDefSize/nGates;
	    
	    double calibr=(geoDef.at(1)).at(calGate);

	    combineRate.at(igate)=_geoOverlapCor(combineRate.at(igate), calibr);
	    crossDataRate.at(igate)=_geoOverlapCor(crossDataRate.at(igate), calibr);
	    molDataRate.at(igate)=_geoOverlapCor(molDataRate.at(igate), calibr);
	  }
	
	if(debug)
	  for(unsigned int igate=0;igate<1;igate++)
	    {
	      cout<<"hiDataRate["<<igate<<"]="<<hiDataRate[igate]<<'\n';
	      cout<<"loDataRate["<<igate<<"]="<<loDataRate[igate]<<'\n';
	      cout<<"crossDataRate["<<igate<<"]="<<crossDataRate[igate]<<'\n';
	      cout<<"molDataRate["<<igate<<"]="<<molDataRate[igate]<<'\n';
	      cout<<"combineRate["<<igate<<"]="<<combineRate[igate]<<'\n';
	    }
	if(debug)
	  cout<<"volDepol^^^^^"<<'\n';
	
	//done with corrections, time for derived fields. 
	
	for(unsigned int igate=0;igate<nGates;igate++)
	  {
	    volDepol[igate] = _volDepol( crossDataRate.at(igate), combineRate.at(igate));
	  }
	
	if(debug)
	  for(unsigned int igate=0;igate<1;igate++)
	    cout<<"volDepol["<<igate<<"]="<<volDepol[igate]<<'\n';
	
	if(debug)
	  cout<<"backscatRatio^^^^^"<<'\n';

	for(unsigned int igate=0;igate<nGates;igate++)
	  {
	    backscatRatio[igate] = _backscatRatio(combineRate.at(igate), 
						  molDataRate.at(igate) );
	  }

	if(debug)
	  for(unsigned int igate=0;igate<1;igate++)
	    cout<<"backscatRatio["<<igate<<"]="<<backscatRatio[igate]<<'\n';
	if(debug)
	  cout<<"partDepol^^^^^"<<'\n';

	for(unsigned int igate=0;igate<nGates;igate++)
	  {
	    partDepol[igate] = _partDepol(volDepol[igate], backscatRatio[igate] );
	  }
	
	if(debug)
	  for(unsigned int igate=0;igate<1;igate++)
	    cout<<"partDepol["<<igate<<"]="<<partDepol[igate]<<'\n';
	if(debug)
	  cout<<"backscatCo^^^^^"<<'\n';

	for(unsigned int igate=0;igate<nGates;igate++)
	  {
	    backscatCoeff[igate] = _backscatCo(presHpa[igate], tempK[igate], 
					       backscatRatio[igate]);
	  }
	
	if(debug)
	  for(unsigned int igate=0;igate<1;igate++)
	    cout<<"backscatCoeff["<<igate<<"]="<<backscatCoeff[igate]<<'\n';
	if(debug)
	  cout<<"extinction^^^^^"<<'\n';

	for(unsigned int igate=0;igate<nGates;igate++)
	  {
	    extinction[igate] = 1.0;
	    //there is no extinction calculation, so have placeholder here for now. 
	  }
	if(debug)
	  for(unsigned int igate=0;igate<1;igate++)
	    cout<<"extinction["<<igate<<"]="<<extinction[igate]<<'\n';
	
      }
       
    RadxField *volDepolField =
      ray->addField("volDepol", "unit", nGates, Radx::missingFl32, volDepol, true);
    volDepolField->setLongName("volume_depolarization");
    volDepolField->setRangeGeom(startRangeKm, gateSpacingKm);
        
    RadxField *backscatRatioField =
      ray->addField("backscatRatio", "unit", nGates, Radx::missingFl32, backscatRatio, true);
    backscatRatioField->setLongName("backscatter_ratio");
    backscatRatioField->setRangeGeom(startRangeKm, gateSpacingKm);
    
    RadxField *partDepolField =
      ray->addField("partDepol", "unit", nGates, Radx::missingFl32, partDepol, true);
    partDepolField->setLongName("particle_depolarization");
    partDepolField->setRangeGeom(startRangeKm, gateSpacingKm);
    
    RadxField *backscatCoeffField =
      ray->addField("backscatCoeff", "unit", nGates, Radx::missingFl32, backscatCoeff, true);
    backscatCoeffField->setLongName("backscatter_coefficient");
    backscatCoeffField->setRangeGeom(startRangeKm, gateSpacingKm);
   
    RadxField *extinctionField =
      ray->addField("extinction", "unit", nGates, Radx::missingFl32, extinction, true);
    extinctionField->setLongName("extinction");
    extinctionField->setRangeGeom(startRangeKm, gateSpacingKm);
       
  }
}

//do calculations for derived fields

//nonlinear count corrections
double Hsrl2Radx::_nonLinCountCor(Radx::fl32 count, double deadtime, 
				      double binWid, double shotCount)
{
  if(shotCount==0)
    return count;
  double photonRate=count/shotCount/binWid;
  double corrFactor=photonRate*deadtime;
  if(corrFactor > 0.99)
    corrFactor=0.95;
  return count/(1-corrFactor);
}


//baseline subtraction
double Hsrl2Radx::_baselineSubtract(double arrivalRate, double profile, 
					double polarization)
{
  //pass through for now, not expected to significantly impact displays
  return arrivalRate;
}


//background subtraction
double Hsrl2Radx::_backgroundSub(double arrivalRate, double backgroundBins)
{
  //background bins is average of the last 100 bins, this can be negative
  return arrivalRate-backgroundBins;
}


//energy normalization
double Hsrl2Radx::_energyNorm(double arrivalRate, double totalEnergy)
{
  if(totalEnergy==0)
    return arrivalRate;
  return arrivalRate/totalEnergy * pow(10,6);
}


//differential overlap correction the vector coresponds to hi, lo, cross, mol
vector<double> Hsrl2Radx::_diffOverlapCor(vector<double> arrivalRate, vector<double> diffOverlap)
{
  assert(arrivalRate.size()==4);
  assert(diffOverlap.size()==4);
  arrivalRate.at(0) = arrivalRate.at(0)/diffOverlap.at(0);
  arrivalRate.at(1) = arrivalRate.at(1)/diffOverlap.at(1);
  arrivalRate.at(2) = arrivalRate.at(2)/(diffOverlap.at(3)*diffOverlap.at(0));
  // arrivalRate.at(3) is unchanged

  return arrivalRate;//note the passthrough functionality for now
}

//process QWP rotation
vector<double> Hsrl2Radx::_processQWPRotation(vector<double> arrivalRate, vector<double> polCal)
{
  return arrivalRate;
}


//merge hi and lo profiles 
double Hsrl2Radx::_hiAndloMerge(double hiRate, double loRate)
{  
  //pass through for now, not expected to significantly impact displays
  return hiRate;
}


//geometric overlap correction
double Hsrl2Radx::_geoOverlapCor(double arrivalRate, double geoOverlap)
{
  return arrivalRate*geoOverlap;
}


//volume depolarization
double Hsrl2Radx::_volDepol(double crossRate, double combineRate)
{
  if(crossRate+combineRate == 0)
    return 0;
  return crossRate/(crossRate+combineRate);
}


//backscatter ratio
double Hsrl2Radx::_backscatRatio(double combineRate, double molRate)
{
  if(molRate==0)
    return 0;
  return combineRate/molRate;
}


//particle depolarization
double Hsrl2Radx::_partDepol(double volDepol, double backscatRatio)
{
  double d_mol=2*0.000365/(1+0.000365);
  double pDepol1=volDepol/(1-1/backscatRatio)-d_mol/(backscatRatio-1);
  //double pDepol2=(-d_mol+backscatRatio*volDepol)/(backscatRatio-1);
  
  //cout<<"pDepol1="<<pDepol1<<'\n';
  //cout<<"pDepol2="<<pDepol2<<'\n';
  
  return pDepol1;
}


//backscatter coefficient
double Hsrl2Radx::_backscatCo(double pressure, double temperature, 
				  double backscatRatio)
{
  //If temp is 0 this causes errors but also there is a case where temp is -1.99384e+34
  if(temperature<=0)
    return 0;
  
  double beta_m_sonde = 5.45*(550/532)*4 * pow(10,-32) * pressure /(temperature*1.3805604 * pow(10,-23));
  double aer_beta_bs = (backscatRatio-1)*beta_m_sonde;
  
  return aer_beta_bs;
}

