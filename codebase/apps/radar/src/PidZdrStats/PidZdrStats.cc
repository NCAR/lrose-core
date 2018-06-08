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

  // set programe name

  _progName = "PidZdrStats";
  
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

  // init process mapper registration

  if (_params.mode == Params::REALTIME) {
    PMU_auto_init( _progName.c_str(),
                  _params.instance,
                  PROCMAP_REGISTER_INTERVAL);
  }

}

// destructor

PidZdrStats::~PidZdrStats()

{

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
      iret = -1;
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

  RadxPath thisPath(filePath);
  for (size_t ii = 0; ii < _readPaths.size(); ii++) {
    RadxPath rpath(_readPaths[ii]);
    if (thisPath.getFile() == rpath.getFile()) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "Skipping file: " << filePath << endl;
        cerr << "  Previously processed in aggregation step" << endl;
      }
      return 0;
    }
  }
  
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
  _readPaths = inFile.getReadPaths();

  // process this data set
  
  if (_processVol()) {
    cerr << "ERROR - PidZdrStats::Run" << endl;
    cerr << "  Cannot process data in file: " << filePath << endl;
    return -1;
  }

  // write output
  
  if (_writeResults()) {
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

  // set up geom

  _nGates = _readVol.getMaxNGates();
  _radxStartRange = _readVol.getStartRangeKm();
  _radxGateSpacing = _readVol.getGateSpacingKm();

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
  
  return 0;

}

//////////////////////////////
// write out results

int PidZdrStats::_writeResults()
  
{

  // compute output file name
  
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
  
  // compute file name
  
  char fileName[BUFSIZ];
  sprintf(fileName,
          "zdr_stats.%.4d%.2d%.2d_%.2d%.2d%.2d.%s.txt",
          fileTime.getYear(), fileTime.getMonth(), fileTime.getDay(),
          fileTime.getHour(), fileTime.getMin(), fileTime.getSec(),
          _radarName.c_str());
  
  char outPath[BUFSIZ];
  sprintf(outPath, "%s%s%s",
          outDir.c_str(), PATH_DELIM,  fileName);

  if (_params.debug) {
    cerr << "Wrote file: " << outPath << endl;
  }

  return 0;

}
  
