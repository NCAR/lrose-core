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
// PidZdrStats.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2018
//
///////////////////////////////////////////////////////////////

#include "PidZdrStats.hh"
#include <Radx/RadxSweep.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxField.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxTimeList.hh>
#include <Radx/RadxPath.hh>
#include <Ncxx/Nc3xFile.hh>
#include <Mdv/GenericRadxFile.hh>
#include <didss/LdataInfo.hh>
#include <toolsa/pmu.h>
#include <toolsa/toolsa_macros.h>
#include <toolsa/sincos.h>
#include <toolsa/TaArray.hh>
#include <toolsa/file_io.h>
#include <rapmath/RapComplex.hh>
#include <physics/IcaoStdAtmos.hh>
#include <physics/thermo.h>
#include <Spdb/SoundingPut.hh>
#include <algorithm>
using namespace std;

// Constructor

PidZdrStats::PidZdrStats(int argc, char **argv)
  
{

  OK = TRUE;
  _outFilesOpen = false;

  // set programe name

  _progName = "PidZdrStats";
  
  // parse command line args
  
  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  Problem with command line args." << endl;
    OK = FALSE;
    return;
  }
  
  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv,
			   _args.override.list, &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  Problem with TDRP parameters." << endl;
    OK = FALSE;
    return;
  }

  // init process mapper registration

  if (_params.mode == Params::REALTIME) {
    PMU_auto_init( _progName.c_str(),
                  _params.instance,
                  PROCMAP_REGISTER_INTERVAL);
  }

  // check the params

  for (int ii = 0; ii < _params.pid_regions_n; ii++) {
    if (_params._pid_regions[ii].pid < NcarParticleId::CLOUD ||
        _params._pid_regions[ii].pid > NcarParticleId::MISC) {
      cerr << "ERROR: " << _progName << endl;
      cerr << "  Problem with TDRP parameters." << endl;
      cerr << "  PID value out of bounds: " << _params._pid_regions[ii].pid << endl;
      cerr << "    for label: " << _params._pid_regions[ii].label << endl;
    }
  }

  // compute lookup table for PID processing

  for (int ii = 0; ii <= NcarParticleId::MISC; ii++) {
    _pidIndex[ii] = -1;
  } 
  for (int ii = 0; ii < _params.pid_regions_n; ii++) {
    _pidIndex[_params._pid_regions[ii].pid] = ii;
  }
  
}

// destructor

PidZdrStats::~PidZdrStats()

{

  _closeOutputFiles();

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int PidZdrStats::Run()
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

int PidZdrStats::_runFilelist()
{

  // loop through the input file list

  int iret = 0;

  for (int ii = 0; ii < (int) _args.inputFileList.size(); ii++) {

    string inputPath = _args.inputFileList[ii];
    if (_processFile(inputPath)) {
      return -1;
    }

  }

  return iret;

}

//////////////////////////////////////////////////
// Run in archive mode

int PidZdrStats::_runArchive()
{

  // get the files to be processed

  RadxTimeList tlist;
  tlist.setDir(_params.input_dir);
  tlist.setModeInterval(_args.startTime, _args.endTime);
  if (tlist.compile()) {
    cerr << "ERROR - PidZdrStats::_runFilelist()" << endl;
    cerr << "  Cannot compile time list, dir: " << _params.input_dir << endl;
    cerr << "  Start time: " << RadxTime::strm(_args.startTime) << endl;
    cerr << "  End time: " << RadxTime::strm(_args.endTime) << endl;
    cerr << tlist.getErrStr() << endl;
    return -1;
  }

  const vector<string> &paths = tlist.getPathList();
  if (paths.size() < 1) {
    cerr << "ERROR - PidZdrStats::_runFilelist()" << endl;
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
// Run in realtime mode

int PidZdrStats::_runRealtime()
{

  // init process mapper registration

  PMU_auto_init(_progName.c_str(), _params.instance,
                PROCMAP_REGISTER_INTERVAL);

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

int PidZdrStats::_processFile(const string &filePath)
{

  // check we have not already processed this file
  // in the file aggregation step

  if (_params.debug) {
    cerr << "INFO - PidZdrStats::Run" << endl;
    cerr << "  Input path: " << filePath << endl;
  }
  
  GenericRadxFile inFile;
  _setupRead(inFile);
  
  // read in file
  
  _readVol.clear();
  if (inFile.readFromPath(filePath, _readVol)) {
    cerr << "ERROR - PidZdrStats::Run" << endl;
    cerr << inFile.getErrStr() << endl;
    return -1;
  }

  // process this data set
  
  if (_processVol()) {
    cerr << "ERROR - PidZdrStats::Run" << endl;
    cerr << "  Cannot process data in file: " << filePath << endl;
    return -1;
  }

  return 0;

}

//////////////////////////////////////////////////
// set up read

void PidZdrStats::_setupRead(RadxFile &file)
{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    file.setDebug(true);
  }
  if (_params.debug >= Params::DEBUG_EXTRA) {
    file.setVerbose(true);
  }

  file.addReadField(_params.PID_field_name);
  file.addReadField(_params.ZDR_field_name);
  if (strlen(_params.RHOHV_field_name) > 0) {
    file.addReadField(_params.RHOHV_field_name);
  }
  if (strlen(_params.TEMP_field_name) > 0) {
    file.addReadField(_params.TEMP_field_name);
  }

  if (_params.set_max_range) {
    file.setReadMaxRangeKm(_params.max_range_km);
  }

  if (_params.debug >= Params::DEBUG_EXTRA) {
    file.printReadRequest(cerr);
  }

}

///////////////////////////////////////////
// process this data set

int PidZdrStats::_processVol()
  
{

  if (_params.debug) {
    cerr << "Processing volume ..." << endl;
  }

  // open output files as needed

  if (_openOutputFiles()) {
    cerr << "ERROR - PidZdrStats::_processVol()" << endl;
    cerr << "  Cannot open output files" << endl;
    return -1;
  }

  // set up geom

  _nGates = _readVol.getMaxNGates();
  _startRangeKm = _readVol.getStartRangeKm();
  _gateSpacingKm = _readVol.getGateSpacingKm();

  _radarName = _readVol.getInstrumentName();
  _radarLatitude = _readVol.getLatitudeDeg();
  _radarLongitude = _readVol.getLongitudeDeg();
  _radarAltitude = _readVol.getAltitudeKm();

  // loop through the rays

  const vector<RadxRay *> rays = _readVol.getRays();
  for (size_t iray = 0; iray < rays.size(); iray++) {

    RadxRay *ray = rays[iray];

    // PID field

    RadxField *pidField = ray->getField(_params.PID_field_name);
    if (pidField == NULL) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "WARNING - ray does not have PID field: "
             << _params.PID_field_name << endl;
      }
      continue;
    }
    // convert to ints
    pidField->convertToSi32();
    _pid = pidField->getDataSi32();
    _pidMiss = pidField->getMissingSi32();

    // ZDR field

    RadxField *zdrField = ray->getField(_params.ZDR_field_name);
    if (zdrField == NULL) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "WARNING - ray does not have ZDR field: "
             << _params.ZDR_field_name << endl;
      }
      continue;
    }
    // convert to ints
    zdrField->convertToFl32();
    _zdr = zdrField->getDataFl32();
    _zdrMiss = zdrField->getMissingFl32();

    // RHOHV field

    _rhohv = NULL;
    if (strlen(_params.RHOHV_field_name) > 0) {
      RadxField *rhohvField = ray->getField(_params.RHOHV_field_name);
      if (rhohvField == NULL) {
        if (_params.debug >= Params::DEBUG_VERBOSE) {
          cerr << "WARNING - ray does not have RHOHV field: "
               << _params.RHOHV_field_name << endl;
        }
      } else {
        // convert to ints
        rhohvField->convertToFl32();
        _rhohv = rhohvField->getDataFl32();
        _rhohvMiss = rhohvField->getMissingFl32();
      }
    }

    // TEMP field

    _temp = NULL;
    if (strlen(_params.TEMP_field_name) > 0) {
      RadxField *tempField = ray->getField(_params.TEMP_field_name);
      if (tempField == NULL) {
        if (_params.debug >= Params::DEBUG_VERBOSE) {
          cerr << "WARNING - ray does not have TEMP field: "
               << _params.TEMP_field_name << endl;
        }
      } else {
        // convert to ints
        tempField->convertToFl32();
        _temp = tempField->getDataFl32();
        _tempMiss = tempField->getMissingFl32();
      }
    }

    // process this ray

    _processRay(ray);

  } // iray
    
  return 0;

}

///////////////////////////////////////////
// process a ray

int PidZdrStats::_processRay(RadxRay *ray)
  
{

  double elev = ray->getElevationDeg();

  double rangeKm = _startRangeKm;
  for (size_t igate = 0; igate < ray->getNGates();
       igate++, rangeKm += _gateSpacingKm) {

    // get PID and region
    int pid = _pid[igate];
    if (pid == _pidMiss) {
      continue;
    }

    int pidIndex = _pidIndex[pid];
    const Params::pid_region_t &region =
      _params._pid_regions[pidIndex];
    
    // check for elevation angle

    if (elev < region.min_elev_deg || elev > region.max_elev_deg) {
      continue;
    }

    // check RHOHV
    
    double rhohv = _rhohvMiss;
    if (_rhohv != NULL) {
      rhohv = _rhohv[igate];
    }
    if (rhohv != _rhohvMiss &&
        (rhohv < region.min_rhohv || rhohv > region.max_rhohv)) {
      continue;
    }
    
    // check TEMP
    
    double temp = _tempMiss;
    if (_temp != NULL) {
      temp = _temp[igate];
    }
    if (temp != _tempMiss &&
        (temp < region.min_temp_c || temp > region.max_temp_c)) {
      continue;
    }
    
    double zdr = _zdr[igate];
    if (zdr == _zdrMiss) {
      continue;
    }

    // print out

    fprintf(_outFilePtrs[pidIndex], "%8.2f %8.2f %4d %8.2f %8.2f %8.2f\n",
            elev, rangeKm, pid, temp, rhohv, zdr);

  } // igate

  return 0;

}

//////////////////////////////
// open the output files

int PidZdrStats::_openOutputFiles()
  
{

  if (_outFilesOpen) {
    return 0;
  }

  // compute output dir
  
  RadxTime fileTime(_readVol.getStartTimeSecs());

  string outDir(_params.output_dir);
  char dayStr[BUFSIZ];
  sprintf(dayStr, "%s%.4d%.2d%.2d", PATH_DELIM,
          fileTime.getYear(), fileTime.getMonth(), fileTime.getDay());
  outDir += dayStr;

  // make sure output subdir exists
  
  if (ta_makedir_recurse(outDir.c_str())) {
    cerr << "ERROR - PidZdrStats::_writeResults" << endl;
    cerr << "  Cannot create output dir: " << outDir << endl;
    return -1;
  }

  // open files for each pid regiod

  for (int ii = 0; ii < _params.pid_regions_n; ii++) {

    // compute file name
    
    char fileName[BUFSIZ];
    sprintf(fileName,
            "%s_%.4d%.2d%.2d_%.2d%.2d%.2d.zdr_stats.txt",
            _params._pid_regions[ii].label,
            fileTime.getYear(), fileTime.getMonth(), fileTime.getDay(),
            fileTime.getHour(), fileTime.getMin(), fileTime.getSec());
    
    char outPath[BUFSIZ];
    sprintf(outPath, "%s%s%s",
            outDir.c_str(), PATH_DELIM,  fileName);

    // open file

    FILE *out = fopen(outPath, "w");
    if (out == NULL) {
      cerr << "ERROR - PidZdrStats::_writeResults" << endl;
      cerr << "  Cannot open output file: " << outPath << endl;
      return -1;
    }

    _outFilePtrs.push_back(out);
    _outFilePaths.push_back(outPath);
    
    if (_params.debug) {
      cerr << "Opened output file: " << outPath << endl;
    }

    // write header

    fprintf(out, "# elev range pid temp rhohv zdr\n");

  } // ii

  _outFilesOpen = true;

  return 0;

}
  
//////////////////////////////
// close the output files

void PidZdrStats::_closeOutputFiles()
  
{

  for (size_t ii = 0; ii < _outFilePtrs.size(); ii++) {
    fclose(_outFilePtrs[ii]);
  } // ii

}
