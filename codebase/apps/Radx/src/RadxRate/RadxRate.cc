// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2018                                         
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
// RadxRate.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2018
//
///////////////////////////////////////////////////////////////
//
// RadxRate reads moments from Radx-supported format files, 
// computes the precip rate and writes out the results 
// to Radx-supported format files
//
///////////////////////////////////////////////////////////////

#include <cerrno>
#include <algorithm>
#include <toolsa/pmu.h>
#include <toolsa/toolsa_macros.h>
#include <toolsa/file_io.h>
#include <toolsa/TaFile.hh>
#include <toolsa/TaArray.hh>
#include <dsserver/DsLdataInfo.hh>
#include <Mdv/GenericRadxFile.hh>
#include <radar/BeamHeight.hh>
#include <Radx/RadxVol.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxField.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxTimeList.hh>
#include <Radx/RadxPath.hh>

#include "RadxRate.hh"
#include "Worker.hh"
#include "WorkerThread.hh"

using namespace std;

// initialize extra field names

string RadxRate::smoothedDbzFieldName = "DBZ_SMOOTHED";
string RadxRate::smoothedRhohvFieldName = "RHOHV_SMOOTHED";
string RadxRate::elevationFieldName = "ELEV";
string RadxRate::rangeFieldName = "RANGE";
string RadxRate::beamHtFieldName = "BEAM_HT";
string RadxRate::tempFieldName = "TEMPC";
string RadxRate::pidFieldName = "PID";
string RadxRate::pidInterestFieldName = "PID_INTEREST";
string RadxRate::mlFieldName = "MELTING_LAYER";
string RadxRate::mlExtendedFieldName = "ML_EXTENDED";
string RadxRate::convFlagFieldName = "CONV_FLAG";

// Constructor

RadxRate::RadxRate(int argc, char **argv)
  
{

  OK = TRUE;

  // set programe name

  _progName = "RadxRate";
  
  // parse command line args
  
  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args." << endl;
    OK = FALSE;
    return;
  }
  
  // read TDRP params for this app

  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv,
			   _args.override.list, &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters." << endl;
    OK = FALSE;
    return;
  }

  // if requested, print params for RATE then exit
  
  if (_args.printParamsRate) {
    _printParamsRate();
    exit(0);
  }

  // if requested, print params for PID then exit
  
  if (_args.printParamsPid) {
    _printParamsPid();
    exit(0);
  }

  // if requested, print params for KDP then exit
  
  if (_args.printParamsKdp) {
    _printParamsKdp();
    exit(0);
  }

  // read params for Ncar PID

  if (strstr(_params.PID_params_file_path, "use-defaults") == NULL) {
    // not using defaults
    if (_ncarPidParams.load(_params.PID_params_file_path,
                            NULL, true, _args.tdrpDebug)) {
      cerr << "ERROR: " << _progName << endl;
      cerr << "Cannot read params file for NcarPid: "
           << _params.PID_params_file_path << endl;
      OK = FALSE;
      return;
    }
  }

  // read params for KdpFilt

  if (strstr(_params.KDP_params_file_path, "use-defaults") == NULL) {
    // not using defaults
    if (_kdpFiltParams.load(_params.KDP_params_file_path,
                            NULL, true, _args.tdrpDebug)) {
      cerr << "ERROR: " << _progName << endl;
      cerr << "Cannot read params file for KdpFilt: "
           << _params.KDP_params_file_path << endl;
      OK = FALSE;
      return;
    }
  }

  // initialize compute object

  pthread_mutex_init(&_debugPrintMutex, NULL);
  
  // set up compute thread pool
  
  for (int ii = 0; ii < _params.n_compute_threads; ii++) {
    WorkerThread *thread =
      new WorkerThread(this, _params, 
                       _kdpFiltParams,
                       _ncarPidParams,
                       _precipRateParams,
                       ii);
    if (!thread->OK) {
      delete thread;
      OK = FALSE;
      return;
    }
    _threadPool.addThreadToMain(thread);
    _workers.push_back(thread->getWorker());
  }
  
}

//////////////////////////////////////
// destructor

RadxRate::~RadxRate()

{

  // mutex

  pthread_mutex_destroy(&_debugPrintMutex);

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Print params for RATE

void RadxRate::_printParamsRate()
{

  if (_params.debug) {
    cerr << "Reading RATE params from file: "
         << _params.RATE_params_file_path << endl;
  }
  
  // do we need to expand environment variables?

  bool expandEnvVars = false;
  if (_args.printParamsRateMode.find("expand") != string::npos) {
    expandEnvVars = true;
  }

  // read in RATE params if applicable
  
  if (strstr(_params.RATE_params_file_path, "use-defaults") == NULL) {
    // not using defaults
    if (_precipRateParams.load(_params.RATE_params_file_path,
                               NULL, expandEnvVars, _args.tdrpDebug)) {
      cerr << "ERROR: " << _progName << endl;
      cerr << "Cannot read params file for PrecipFilt: "
           << _params.RATE_params_file_path << endl;
      OK = FALSE;
      return;
    }
  }

  // set print mode

  tdrp_print_mode_t printMode = PRINT_LONG;
  if (_args.printParamsRateMode.find("short") == 0) {
    printMode = PRINT_SHORT;
  } else if (_args.printParamsRateMode.find("norm") == 0) {
    printMode = PRINT_NORM;
  } else if (_args.printParamsRateMode.find("verbose") == 0) {
    printMode = PRINT_VERBOSE;
  }

  // do the print to stdout

  _precipRateParams.print(stdout, printMode);

}

//////////////////////////////////////////////////
// Print params for PID

void RadxRate::_printParamsPid()
{

  if (_params.debug) {
    cerr << "Reading PID params from file: " << _params.PID_params_file_path << endl;
  }

  // do we need to expand environment variables?

  bool expandEnvVars = false;
  if (_args.printParamsPidMode.find("expand") != string::npos) {
    expandEnvVars = true;
  }

  // read in PID params if applicable

  if (strstr(_params.PID_params_file_path, "use-defaults") == NULL) {
    // not using defaults
    if (_ncarPidParams.load(_params.PID_params_file_path,
                            NULL, expandEnvVars, _args.tdrpDebug)) {
      cerr << "ERROR: " << _progName << endl;
      cerr << "Cannot read params file for PidFilt: "
           << _params.PID_params_file_path << endl;
      OK = FALSE;
      return;
    }
  }

  // set print mode

  tdrp_print_mode_t printMode = PRINT_LONG;
  if (_args.printParamsPidMode.find("short") == 0) {
    printMode = PRINT_SHORT;
  } else if (_args.printParamsPidMode.find("norm") == 0) {
    printMode = PRINT_NORM;
  } else if (_args.printParamsPidMode.find("verbose") == 0) {
    printMode = PRINT_VERBOSE;
  }

  // do the print to stdout

  _ncarPidParams.print(stdout, printMode);

}

//////////////////////////////////////////////////
// Print params for KDP

void RadxRate::_printParamsKdp()
{

  if (_params.debug) {
    cerr << "Reading KDP params from file: " << _params.KDP_params_file_path << endl;
  }

  // do we need to expand environment variables?

  bool expandEnvVars = false;
  if (_args.printParamsKdpMode.find("expand") != string::npos) {
    expandEnvVars = true;
  }

  // read in KDP params if applicable

  if (strstr(_params.KDP_params_file_path, "use-defaults") == NULL) {
    // not using defaults
    if (_kdpFiltParams.load(_params.KDP_params_file_path,
                            NULL, expandEnvVars, _args.tdrpDebug)) {
      cerr << "ERROR: " << _progName << endl;
      cerr << "Cannot read params file for KdpFilt: "
           << _params.KDP_params_file_path << endl;
      OK = FALSE;
      return;
    }
  }

  // set print mode

  tdrp_print_mode_t printMode = PRINT_LONG;
  if (_args.printParamsKdpMode.find("short") == 0) {
    printMode = PRINT_SHORT;
  } else if (_args.printParamsKdpMode.find("norm") == 0) {
    printMode = PRINT_NORM;
  } else if (_args.printParamsKdpMode.find("verbose") == 0) {
    printMode = PRINT_VERBOSE;
  }

  // do the print to stdout

  _kdpFiltParams.print(stdout, printMode);

}

//////////////////////////////////////////////////
// Run

int RadxRate::Run()
{

  if (_params.mode == Params::ARCHIVE) {
    return _runArchive();
  } else if (_params.mode == Params::FILELIST) {
    return _runFilelist();
  } else {
    return _runRealtime();
  }
}

//////////////////////////////////////////////////
// Run in filelist mode

int RadxRate::_runFilelist()
{

  // loop through the input file list

  int iret = 0;
  
  for (int ifile = 0; ifile < (int) _args.inputFileList.size(); ifile++) {

    string inputPath = _args.inputFileList[ifile];
    if (_processFile(inputPath)) {
      iret = -1;
    }

  }

  return iret;

}

//////////////////////////////////////////////////
// Run in archive mode

int RadxRate::_runArchive()
{

  // get start and end times

  time_t startTime = RadxTime::parseDateTime(_params.start_time);
  if (startTime == RadxTime::NEVER) {
    cerr << "ERROR - Radx2Grid::_runArchive()" << endl;
    cerr << "  Start time format incorrect: " << _params.start_time << endl;
    if (_args.startTimeSet) {
      cerr << "  Check command line" << endl;
    } else {
      cerr << "  Check params file: " << _paramsPath << endl;
    }
    return -1;
  }

  time_t endTime = RadxTime::parseDateTime(_params.end_time);
  if (endTime == RadxTime::NEVER) {
    cerr << "ERROR - Radx2Grid::_runArchive()" << endl;
    cerr << "  End time format incorrect: " << _params.end_time << endl;
    if (_args.endTimeSet) {
      cerr << "  Check command line" << endl;
    } else {
      cerr << "  Check params file: " << _paramsPath << endl;
    }
    return -1;
  }

  if (_params.debug) {
    cerr << "RadxRate::_runArchive" << endl;
    cerr << "  Input dir: " << _params.input_dir << endl;
    cerr << "  Start time: " << RadxTime::strm(startTime) << endl;
    cerr << "  End time: " << RadxTime::strm(endTime) << endl;
  }

  // get the files to be processed

  RadxTimeList tlist;
  tlist.setDir(_params.input_dir);
  tlist.setModeInterval(startTime, endTime);
  if (tlist.compile()) {
    cerr << "ERROR - RadxRate::_runFilelist()" << endl;
    cerr << "  Cannot compile time list, dir: " << _params.input_dir << endl;
    cerr << "  Start time: " << RadxTime::strm(startTime) << endl;
    cerr << "  End time: " << RadxTime::strm(endTime) << endl;
    cerr << tlist.getErrStr() << endl;
    return -1;
  }

  const vector<string> &paths = tlist.getPathList();
  if (paths.size() < 1) {
    cerr << "ERROR - RadxRate::_runFilelist()" << endl;
    cerr << "  No files found, dir: " << _params.input_dir << endl;
    return -1;
  }
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "n files found: " << paths.size() << endl;
    for (size_t ipath = 0; ipath < paths.size(); ipath++) {
      cerr << "    " << paths[ipath] << endl;
    }
  }

  // loop through the input file list
  
  int iret = 0;
  for (size_t ipath = 0; ipath < paths.size(); ipath++) {
    if (_processFile(paths[ipath])) {
      iret = -1;
    }
  }

  return iret;

}

//////////////////////////////////////////////////
// Run in realtime mode

int RadxRate::_runRealtime()
{

  // init process mapper registration
  
  PMU_auto_init(_progName.c_str(),
                _params.instance,
                _params.procmap_register_interval);
  PMU_auto_register("Init realtime mode");

  // watch for new data to arrive

  LdataInfo ldata(_params.input_dir,
                  _params.debug >= Params::DEBUG_VERBOSE);

  int iret = 0;

  while (true) {
    ldata.readBlocking(_params.max_realtime_data_age_secs,
                       1000, PMU_auto_register);
    
    const string path = ldata.getDataPath();
    if (_processFile(path)) {
      iret = -1;
    }
  }

  return iret;

}

//////////////////////////////////////////////////
// Process a file
// Returns 0 on success, -1 on failure

int RadxRate::_processFile(const string &filePath)
{

  PMU_auto_register("Processing file");
  
  // check file name
  
  if (strlen(_params.input_file_search_ext) > 0) {
    RadxPath rpath(filePath);
    if (strcmp(rpath.getExt().c_str(), _params.input_file_search_ext)) {
      if (_params.debug) {
        cerr << "WARNING - ignoring file: " << filePath << endl;
        cerr << "  Does not have correct extension: "
             << _params.input_file_search_ext << endl;
      }
      return 0;
    }
  }
  
  if (strlen(_params.input_file_search_substr) > 0) {
    RadxPath rpath(filePath);
    if (rpath.getFile().find(_params.input_file_search_substr)
        == string::npos) {
      if (_params.debug) {
        cerr << "WARNING - ignoring file: " << filePath << endl;
        cerr << "  Does not contain required substr: "
             << _params.input_file_search_substr << endl;
      }
      return 0;
    }
  }

  // check we have not already processed this file
  // in the file aggregation step

  RadxPath thisPath(filePath);
  for (size_t ipath = 0; ipath < _readPaths.size(); ipath++) {
    RadxPath rpath(_readPaths[ipath]);
    if (thisPath.getFile() == rpath.getFile()) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "Skipping file: " << filePath << endl;
        cerr << "  Previously processed in aggregation step" << endl;
      }
      return 0;
    }
  }
  
  if (_params.debug) {
    cerr << "INFO - RadxRate::Run" << endl;
    cerr << "  Input path: " << filePath << endl;
  }
  
  GenericRadxFile inFile;
  _setupRead(inFile);
  
  // read in file
  
  if (inFile.readFromPath(filePath, _vol)) {
    cerr << "ERROR - RadxRate::Run" << endl;
    cerr << inFile.getErrStr() << endl;
    return -1;
  }
  _readPaths = inFile.getReadPaths();

  // convert to fl32
  
  _vol.convertToFl32();

  // set radar properties

  _radarHtKm = _vol.getAltitudeKm();
  _wavelengthM = _vol.getWavelengthM();

  // read the temperature profile into the threads

  if (_workers.size() > 0) {
    // thread 0
    Worker *worker0 = _workers[0];
    if (_params.debug) {
      cerr << "Thread #: " << 0 << endl;
      cerr << "  Loading temp profile for time: "
           << RadxTime::strm(_vol.getStartTimeSecs()) << endl;
    }
    worker0->loadTempProfile(_vol.getStartTimeSecs());
    // copy to other threads
    for (size_t ii = 1; ii < _workers.size(); ii++) {
      _workers[ii]->setTempProfile(worker0->getTempProfile());
    }
  }
  
  // add geometry and pid fields to the volume
  
  _addExtraFieldsToInput();

  // compute the derived fields
  
  if (_compute()) {
    cerr << "ERROR - RadxRate::Run" << endl;
    cerr << "  Cannot compute derived fields" << endl;
    return -1;
  }

  // add extra fields to output

  if (_params.PID_write_debug_fields) {
    _addExtraFieldsToOutput();
  }

  // write results to output file

  if (_writeVol()) {
    return -1;
  }

  return 0;

}

//////////////////////////////////////////////////
// set up read

void RadxRate::_setupRead(RadxFile &file)
{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    file.setDebug(true);
  }
  if (_params.debug >= Params::DEBUG_EXTRA) {
    file.setVerbose(true);
  }

  // field names

  if (_params.SNR_available) {
    file.addReadField(_params.SNR_field_name);
  }
  file.addReadField(_params.DBZ_field_name);
  file.addReadField(_params.ZDR_field_name);
  file.addReadField(_params.PHIDP_field_name);
  file.addReadField(_params.RHOHV_field_name);
  if (_params.LDR_available) {
    file.addReadField(_params.LDR_field_name);
  }
  if (_params.copy_selected_input_fields_to_output) {
    for (int ii = 0; ii < _params.copy_fields_n; ii++) {
      file.addReadField(_params._copy_fields[ii].input_name);
    }
  }

  if (_params.debug >= Params::DEBUG_EXTRA) {
    file.printReadRequest(cerr);
  }
  
}

//////////////////////////////////////////////////
// encode fields for output

void RadxRate::_encodeFieldsForOutput()
{
  
  if (_params.output_encoding == Params::OUTPUT_ENCODING_INT16) {
    _vol.convertToSi16();
  } else {
    _vol.convertToFl32();
  }

}

//////////////////////////////////////////////////
// set up write

void RadxRate::_setupWrite(RadxFile &file)
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

  // set output format
  
  switch (_params.output_format) {
    case Params::OUTPUT_FORMAT_UF:
      file.setFileFormat(RadxFile::FILE_FORMAT_UF);
      break;
    case Params::OUTPUT_FORMAT_DORADE:
      file.setFileFormat(RadxFile::FILE_FORMAT_DORADE);
      break;
    default:
    case Params::OUTPUT_FORMAT_CFRADIAL:
      file.setFileFormat(RadxFile::FILE_FORMAT_CFRADIAL);
  }

  // set netcdf format - used for CfRadial

  file.setNcFormat(RadxFile::NETCDF4);

  file.setWriteForceNgatesVary(true);

}

//////////////////////////////////////////////////
// add geometry and pid fields to the volume

void RadxRate::_addExtraFieldsToInput()

{

  vector<RadxRay *> &rays = _vol.getRays();
  for (size_t iray = 0; iray < rays.size(); iray++) {

    RadxRay *ray = rays[iray];

    RadxField *smoothDbzFld = new RadxField(smoothedDbzFieldName, "dBZ");
    RadxField *smoothRhohvFld = new RadxField(smoothedRhohvFieldName, "");
    RadxField *elevFld = new RadxField(elevationFieldName, "deg");
    RadxField *rangeFld = new RadxField(rangeFieldName, "km");
    RadxField *beamHtFld = new RadxField(beamHtFieldName, "km");
    RadxField *tempFld = new RadxField(tempFieldName, "C");
    RadxField *pidFld = new RadxField(pidFieldName, "");
    RadxField *pidIntrFld = new RadxField(pidInterestFieldName, "");
    RadxField *mlFld = new RadxField(mlFieldName, "");

    size_t nGates = ray->getNGates();

    TaArray<Radx::fl32> elev_, rng_, ht_, temp_;
    Radx::fl32 *elev = elev_.alloc(nGates);
    Radx::fl32 *rng = rng_.alloc(nGates);
    Radx::fl32 *ht = ht_.alloc(nGates);
    Radx::fl32 *temp = temp_.alloc(nGates);
    
    double startRangeKm = ray->getStartRangeKm();
    double gateSpacingKm = ray->getGateSpacingKm();

    BeamHeight beamHt;
    beamHt.setInstrumentHtKm(_vol.getAltitudeKm());
    if (_params.override_standard_pseudo_earth_radius) {
      beamHt.setPseudoRadiusRatio(_params.pseudo_earth_radius_ratio);
    }

    // loop through the gates
    
    double rangeKm = startRangeKm;
    double elevationDeg = ray->getElevationDeg();
    const TempProfile &profile = _workers[0]->getTempProfile();
    for (size_t igate = 0; igate < nGates; igate++, rangeKm += gateSpacingKm) {
      double htKm = beamHt.computeHtKm(elevationDeg, rangeKm);
      double tempC = profile.getTempForHtKm(htKm);
      elev[igate] = elevationDeg;
      rng[igate] = rangeKm;
      ht[igate] = htKm;
      temp[igate] = tempC;
    } // igate

    smoothDbzFld->setTypeFl32(-9999.0);
    smoothRhohvFld->setTypeFl32(-9999.0);
    elevFld->setTypeFl32(-9999.0);
    rangeFld->setTypeFl32(-9999.0);
    beamHtFld->setTypeFl32(-9999.0);
    tempFld->setTypeFl32(-9999.0);
    pidFld->setTypeSi32(-9999, 1.0, 0.0);
    pidIntrFld->setTypeFl32(-9999.0);
    mlFld->setTypeSi32(-9999, 1.0, 0.0);
    
    smoothDbzFld->addDataMissing(nGates);
    smoothRhohvFld->addDataMissing(nGates);
    elevFld->addDataFl32(nGates, elev);
    rangeFld->addDataFl32(nGates, rng);
    beamHtFld->addDataFl32(nGates, ht);
    tempFld->addDataFl32(nGates, temp);
    pidFld->addDataMissing(nGates);
    pidIntrFld->addDataMissing(nGates);
    mlFld->addDataMissing(nGates);

    ray->addField(smoothDbzFld);
    ray->addField(smoothRhohvFld);
    ray->addField(elevFld);
    ray->addField(rangeFld);
    ray->addField(beamHtFld);
    ray->addField(tempFld);
    ray->addField(pidFld);
    ray->addField(pidIntrFld);
    ray->addField(mlFld);

  } // iray

}

//////////////////////////////////////////////////
// add extra output fields

void RadxRate::_addExtraFieldsToOutput()

{

  // loop through rays

  vector<RadxRay *> &inputRays = _vol.getRays();
  for (size_t iray = 0; iray < inputRays.size(); iray++) {
    
    RadxRay *inputRay = inputRays[iray];
    
    // match output ray to input ray based on time
    
    RadxRay *outputRay = NULL;
    double inTime = inputRay->getTimeDouble();
    for (size_t jj = 0; jj < _derivedRays.size(); jj++) {
      RadxRay *derivedRay = _derivedRays[jj];
      double outTime = derivedRay->getTimeDouble();
      if (fabs(inTime - outTime) < 0.001) {
        outputRay = derivedRay;
        break;
      }
    } // jj

    if (outputRay != NULL) {
      // make a copy of the input fields
      RadxField *mlFld = new RadxField(*inputRay->getField(mlFieldName));
      // add to output
      outputRay->addField(mlFld);
    }

  } // iray

}

/////////////////////////////////////////////////////
// compute the derived fields for all rays in volume

int RadxRate::_compute()
{

  // initialize the volume with ray numbers
  
  _vol.setRayNumbersInOrder();

  // initialize derived

  _derivedRays.clear();
  
  // loop through the input rays,
  // computing the derived fields

  const vector<RadxRay *> &inputRays = _vol.getRays();
  for (size_t iray = 0; iray < inputRays.size(); iray++) {

    // get a thread from the pool
    bool isDone = true;
    WorkerThread *thread = 
      (WorkerThread *) _threadPool.getNextThread(true, isDone);
    if (thread == NULL) {
      break;
    }
    if (isDone) {
      // store the results computed by the thread
      if (_storeDerivedRay(thread)) {
        cerr << "ERROR - RadxRate::_compute()" << endl;
        cerr << "  Cannot compute for ray num: " << iray << endl;
        break;
      }
      // return thread to the available pool
      _threadPool.addThreadToAvail(thread);
      // reduce iray by 1 since we did not actually process this ray
      // we only handled a previously started thread
      iray--;
    } else {
      // got a thread to use, set the input ray
      thread->setInputRay(inputRays[iray]);
      // set it running
      thread->signalRunToStart();
    }

  } // iray
    
  // collect remaining done threads

  _threadPool.setReadyForDoneCheck();
  while (!_threadPool.checkAllDone()) {
    WorkerThread *thread = (WorkerThread *) _threadPool.getNextDoneThread();
    if (thread == NULL) {
      break;
    } else {
      // store the results computed by the thread
      _storeDerivedRay(thread);
      // return thread to the available pool
      _threadPool.addThreadToAvail(thread);
    }
  } // while

  return 0;

}

///////////////////////////////////////////////////////////
// Store the derived ray

int RadxRate::_storeDerivedRay(WorkerThread *thread)

{
  
  RadxRay *derivedRay = thread->getOutputRay();
  if (derivedRay == NULL) {
    // error
    return -1;
  }

  // good return, add to results
  _derivedRays.push_back(derivedRay);
  
  return 0;

}
      
//////////////////////////////////////////////////
// write out the volume

int RadxRate::_writeVol()
{

  // clear the input rays

  _vol.clearRays();
  
  // add the derived rays to the volume
  
  for (size_t iray = 0; iray < _derivedRays.size(); iray++) {
    _vol.addRay(_derivedRays[iray]);
  }
  
  _vol.sortRaysByNumber();
  _vol.loadVolumeInfoFromRays();
  _vol.loadSweepInfoFromRays();
  _vol.setPackingFromRays();

  // create output file object

  GenericRadxFile outFile;
  _setupWrite(outFile);
  
  // write to dir
  
  if (outFile.writeToDir(_vol, _params.output_dir,
                         _params.append_day_dir_to_output_dir,
                         _params.append_year_dir_to_output_dir)) {
    cerr << "ERROR - RadxRate::_writeVol" << endl;
    cerr << "  Cannot write file to dir: " << _params.output_dir << endl;
    cerr << outFile.getErrStr() << endl;
    return -1;
  }

  string outputPath = outFile.getPathInUse();

  // write latest data info file
  
  DsLdataInfo ldata(_params.output_dir);
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    ldata.setDebug(true);
  }
  string relPath;
  RadxPath::stripDir(_params.output_dir, outputPath, relPath);
  ldata.setRelDataPath(relPath);
  ldata.setWriter(_progName);
  if (ldata.write(_vol.getEndTimeSecs())) {
    cerr << "WARNING - RadxRate::_writeVol" << endl;
    cerr << "  Cannot write latest data info file to dir: "
         << _params.output_dir << endl;
  }

  return 0;

}

