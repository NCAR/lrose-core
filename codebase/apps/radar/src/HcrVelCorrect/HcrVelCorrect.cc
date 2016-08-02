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
// HcrVelCorrect.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2015
//
//////////////////////////////////////////////////////////////////////////
//
// HcrVelCorrect reads in HCR moments, computes the apparent velocity
// of the ground echo, filters the apparent velocity in time to remove
// spurious spikes, and then corrects the weather echo velocity using
// the filtered ground velocity as the correction to be applied.
//
//////////////////////////////////////////////////////////////////////////

#include "HcrVelCorrect.hh"
#include <Radx/RadxRay.hh>
#include <Mdv/GenericRadxFile.hh>
#include <Radx/RadxGeoref.hh>
#include <Radx/RadxTimeList.hh>
#include <Radx/RadxPath.hh>
#include <dsserver/DsLdataInfo.hh>
#include <didss/DsInputPath.hh>
#include <toolsa/TaXml.hh>
#include <toolsa/pmu.h>
using namespace std;

// Constructor

HcrVelCorrect::HcrVelCorrect(int argc, char **argv)
  
{

  OK = TRUE;
  _dbzMax = NULL;
  _rangeToSurface = NULL;
  _surfaceVel = NULL;
  _filteredStage1 = NULL;
  _filteredSpike = NULL;
  _filteredCond = NULL;
  _filteredFinal = NULL;
  _nValid = 0;
  _firstInputFile = true;

  // set programe name

  _progName = "HcrVelCorrect";
  
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
  
  // set up the filters
  
  _initFilters();

  // init process mapper registration

  if (_params.register_with_procmap) {
    PMU_auto_init((char *) _progName.c_str(),
                  _params.instance,
                  PROCMAP_REGISTER_INTERVAL);
  }

}

// destructor

HcrVelCorrect::~HcrVelCorrect()

{

  // close queues

  _inputFmq.closeMsgQueue();
  _outputFmq.closeMsgQueue();

  // free memory

  for (size_t iray = 0; iray < _filtRays.size(); iray++) {
    RadxRay::deleteIfUnused(_filtRays[iray]);
  }

  if (_dbzMax) {
    delete[] _dbzMax;
  }
  if (_rangeToSurface) {
    delete[] _rangeToSurface;
  }
  if (_surfaceVel) {
    delete[] _surfaceVel;
  }
  if (_filteredStage1) {
    delete[] _filteredStage1;
  }
  if (_filteredSpike) {
    delete[] _filteredSpike;
  }
  if (_filteredCond) {
    delete[] _filteredCond;
  }
  if (_filteredFinal) {
    delete[] _filteredFinal;
  }

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int HcrVelCorrect::Run()
{

  switch (_params.mode) {
    case Params::FMQ:
      return _runFmq();
    case Params::ARCHIVE:
      return _runArchive();
    case Params::REALTIME:
      if (_params.latest_data_info_avail) {
        return _runRealtimeWithLdata();
      } else {
        return _runRealtimeNoLdata();
      }
    case Params::FILELIST:
    default:
      return _runFilelist();
  } // switch
}

//////////////////////////////////////////////////
// Run in filelist mode

int HcrVelCorrect::_runFilelist()
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

int HcrVelCorrect::_runArchive()
{

  // get the files to be processed

  RadxTimeList tlist;
  tlist.setDir(_params.input_dir);
  tlist.setModeInterval(_args.startTime, _args.endTime);
  if (tlist.compile()) {
    cerr << "ERROR - HcrVelCorrect::_runFilelist()" << endl;
    cerr << "  Cannot compile time list, dir: " << _params.input_dir << endl;
    cerr << "  Start time: " << RadxTime::strm(_args.startTime) << endl;
    cerr << "  End time: " << RadxTime::strm(_args.endTime) << endl;
    cerr << tlist.getErrStr() << endl;
    return -1;
  }

  const vector<string> &paths = tlist.getPathList();
  if (paths.size() < 1) {
    cerr << "ERROR - HcrVelCorrect::_runFilelist()" << endl;
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

int HcrVelCorrect::_runRealtimeWithLdata()
{

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

int HcrVelCorrect::_runRealtimeNoLdata()
{

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

  while(true) {

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

int HcrVelCorrect::_processFile(const string &readPath)
{

  PMU_auto_register("Processing file");

  // check we have not already processed this file
  // in the file aggregation step
  
  for (size_t ii = 0; ii < _readPaths.size(); ii++) {
    RadxPath rpath(_readPaths[ii]);
    if (readPath.find(rpath.getFile()) != string::npos) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "Skipping file: " << readPath << endl;
        cerr << "  Previously processed in aggregation step" << endl;
      }
      return 0;
    }
  }
  
  if (_params.debug) {
    cerr << "INFO - HcrVelCorrect::Run" << endl;
    cerr << "  Input path: " << readPath << endl;
  }
  
  GenericRadxFile inFile;
  _setupRead(inFile);
  
  // read in file

  if (inFile.readFromPath(readPath, _inVol)) {
    cerr << "ERROR - HcrVelCorrect::Run" << endl;
    cerr << inFile.getErrStr() << endl;
    return -1;
  }
  _readPaths = inFile.getReadPaths();
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    for (size_t ii = 0; ii < _readPaths.size(); ii++) {
      cerr << "  ==>> read in file: " << _readPaths[ii] << endl;
    }
  }
  if (_firstInputFile) {
    _inEndTime.set(_inVol.getEndTimeSecs(), _inVol.getEndNanoSecs() / 1.0e9);
    _filtVol.copyMeta(_inVol);
    _firstInputFile = false;
  }
  
  vector<RadxRay *> rays = _inVol.getRays();
  for (size_t iray = 0; iray < rays.size(); iray++) {

    RadxRay *ray = new RadxRay(*rays[iray]);
    
  // process each ray in the volume
    
    _processRay(ray);
    
    if (_filtRays.size() > _finalIndex) {

      RadxRay *outRay = _filtRays[_finalIndex];
      RadxTime filtRayTime = outRay->getRadxTime();

      // write vol when done
      
      if (filtRayTime > _inEndTime) {
        _writeFiltVol();
        _inEndTime.set(_inVol.getEndTimeSecs(), _inVol.getEndNanoSecs() / 1.0e9);
      }
      
      // add to output vol
      
      _filtVol.addRay(outRay);

    }

  } // iray
  
  return 0;

}

//////////////////////////////////////////////////
// set up read

void HcrVelCorrect::_setupRead(RadxFile &file)
{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    file.setDebug(true);
  }
  if (_params.debug >= Params::DEBUG_EXTRA) {
    file.setVerbose(true);
  }

  if (_params.debug >= Params::DEBUG_EXTRA) {
    file.printReadRequest(cerr);
  }

}

/////////////////////////////////////////////////////////////////////////
// Process an incoming ray

int HcrVelCorrect::_processRay(RadxRay *ray)

{

  if (_filtRays.size() == 0) {
    _timeFirstRay = ray->getRadxTime();
  }

  // convert to floats

  ray->convertToFl32();

  // trim ray deque as needed

  if (_filtRays.size() > _finalIndex) {
    RadxRay *toBeDeleted = _filtRays.back();
    RadxRay::deleteIfUnused(toBeDeleted);
    _filtRays.pop_back();
  }

  // shift the data arrays by 1

  _shiftArrays();

  // add to ray deque
  
  ray->addClient();
  _filtRays.push_front(ray);
  
  // compute surface vel, store at end of array
  
  _computeSurfaceVel(ray);
  
  // apply the spike and stage1 filters

  _applyStage1Filter();
  _applySpikeFilter();

  // compute the conditioned velocity, based on the
  // difference between the results of the spike and stage1 filter
  
  _computeConditionedValue();
  
  // apply the final filter
  
  _applyFinalFilter();

  // was this a valid observation? - i.e. can we see the surface

  if (_rangeToSurface[0] > 0) {
    _nValid++;
  } else {
    _nValid = 0;
  }
  
  // print out?
  
  if (_params.write_results_to_stdout &&
      (_filtRays.size() > _finalIndex)) {
    RadxRay *ray = _filtRays[_finalIndex];
    if (_rangeToSurface[_finalIndex] > 0) {
      double deltaTime = ray->getRadxTime() - _timeFirstRay;
      cout << ray->getRadxTime().asString(3) << " "
           << deltaTime << " "
           << _rangeToSurface[_finalIndex] << " "
           << _dbzMax[_finalIndex] << " "
           << _surfaceVel[_finalIndex] << " "
           << _filteredStage1[_finalIndex] << " "
           << _filteredSpike[_finalIndex] << " "
           << _filteredCond[_finalIndex] << " "
           << _filteredFinal[_finalIndex] << endl;
    }
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    if (_filtRays.size() > _finalIndex) {
      RadxRay *ray = _filtRays[_finalIndex];
      if (_rangeToSurface[_finalIndex] > 0) {
        cerr << "Time gndRange dbzMax surfVel filtStage1 filtSpike filtCond filtFinal "
             << ray->getRadxTime().asString() << " "
             << _rangeToSurface[_finalIndex] << " "
             << _dbzMax[_finalIndex] << " "
             << _surfaceVel[_finalIndex] << " "
             << _filteredStage1[_finalIndex] <<  ""
             << _filteredSpike[_finalIndex] << " "
             << _filteredCond[_finalIndex] << " "
             << _filteredFinal[_finalIndex] << endl;
      } else {
        cerr << "Surface NOT found, time: "
             << ray->getRadxTime().asString() << endl;
      }
    }
  }

  // add corrected velocity to ray
  
  if (_filtRays.size() > _finalIndex) {
    RadxRay *ray = _filtRays[_finalIndex];
    if (_nValid > _finalIndex) {
      _correctVelForRay(ray);
    } else {
      _copyVelForRay(ray);
    }
  }

  return 0;

}

/////////////////////////////////////////////////////////////////////////
// compute surface velocity
//
// Sets vel to (0.0) if cannot determine valocity

void HcrVelCorrect::_computeSurfaceVel(RadxRay *ray)
  
{

  // init
  
  _dbzMax[0] = -9999.0;
  _rangeToSurface[0] = -9999.0;
  _surfaceVel[0] = 0.0;

  // check elevation
  // cannot compute gnd vel if not pointing down
  
  double elev = ray->getElevationDeg();
  if (elev > -85 || elev < -95) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Bad elevation for finding surface, time, elev(deg): "
           << ray->getRadxTime().asString() << ", "
           << elev << endl;
    }
    return;
  }

  // get dbz field

  RadxField *dbzField = ray->getField(_params.dbz_field_name);
  if (dbzField == NULL) {
    cerr << "ERROR - HcrVelCorrect::_computeSurfaceVel" << endl;
    cerr << "  No dbz field found, field name: " << _params.dbz_field_name << endl;
    return;
  }
  const Radx::fl32 *dbzArray = dbzField->getDataFl32();
  Radx::fl32 dbzMiss = dbzField->getMissingFl32();

  // get vel field

  RadxField *velField = ray->getField(_params.vel_field_name);
  if (velField == NULL) {
    cerr << "ERROR - HcrVelCorrect::_computeSurfaceVel" << endl;
    cerr << "  No vel field found, field name: " << _params.vel_field_name << endl;
    return;
  }
  const Radx::fl32 *velArray = velField->getDataFl32();
  Radx::fl32 velMiss = velField->getMissingFl32();
  
  // get gate at which max dbz occurs

  double range = dbzField->getStartRangeKm();
  double drange = dbzField->getGateSpacingKm();
  double dbzMax = -9999.0;
  int gateForMax = -1;
  double rangeToSurface = 0;
  double foundSurface = false;
  for (size_t igate = 0; igate < dbzField->getNPoints(); igate++, range += drange) {
    if (range < _params.min_range_to_surface_km) {
      continue;
    }
    Radx::fl32 dbz = dbzArray[igate];
    if (dbz != dbzMiss) {
      if (dbz > dbzMax) {
        dbzMax = dbz;
        gateForMax = igate;
        rangeToSurface = range;
        foundSurface = true;
      }
    }
  }
  
  // check for sufficient power

  if (foundSurface) {
    if (dbzMax < _params.min_dbz_for_surface_echo) {
      foundSurface = false;
      if (_params.debug) {
        cerr << "WARNING - HcrVelCorrect::_computeSurfaceVel" << endl;
        cerr << "  Ray at time: " << ray->getRadxTime().asString() << endl;
        cerr << "  Dbz max not high enough for surface detection: " << dbzMax << endl;
        cerr << "  Range to max dbz: " << rangeToSurface << endl;
      }
    }
  }

  size_t nEachSide = _params.ngates_for_surface_echo / 2;
  if (foundSurface) {
    for (size_t igate = gateForMax - nEachSide; igate <= gateForMax + nEachSide; igate++) {
      Radx::fl32 dbz = dbzArray[igate];
      if (dbz == dbzMiss) {
        foundSurface = false;
      }
      if (dbz < _params.min_dbz_for_surface_echo) {
        foundSurface = false;
      }
    }
  }
  
  // compute surface vel
  
  if (foundSurface) {
    double sum = 0.0;
    double count = 0.0;
    for (size_t igate = gateForMax - nEachSide; igate <= gateForMax + nEachSide; igate++) {
      Radx::fl32 vel = velArray[igate];
      if (vel == velMiss) {
        foundSurface = false;
      }
      sum += vel;
      count++;
    }
    _surfaceVel[0] = sum / count;
    _dbzMax[0] = dbzMax;
    _rangeToSurface[0] = rangeToSurface;
  }

}

/////////////////////////////////////////////////////////////////////////
// apply the stage1 filter
// this is applied to the tail end of the incoming data

void HcrVelCorrect::_applyStage1Filter()

{
  
  double sum = 0.0;
  double sumWts = 0.0;
  for (size_t ii = 0; ii < _lenStage1; ii++) {
    double vel = _surfaceVel[ii];
    double wt = _filtCoeffStage1[ii];
    sum += wt * vel;
    sumWts += wt;
  }

  double filtVal = _surfaceVel[_lenStage1Half];
  if (sumWts > 0) {
    filtVal = sum / sumWts;
  }
  
  _filteredStage1[_lenStage1Half] = filtVal;

}

/////////////////////////////////////////////////////////////////////////
// apply the spike filter

void HcrVelCorrect::_applySpikeFilter()

{

  double sum = 0.0;
  double sumWts = 0.0;
  for (size_t ii = 0; ii < _lenSpike; ii++) {
    double vel = _surfaceVel[ii];
    double wt = _filtCoeffSpike[ii];
    sum += wt * vel;
    sumWts += wt;
  }

  double filtVal = _surfaceVel[_lenSpikeHalf];
  if (sumWts > 0) {
    filtVal = sum / sumWts;
  }

  _filteredSpike[_lenSpikeHalf] = filtVal;

}

/////////////////////////////////////////////////////////////////////////
// compute the conditioned value

void HcrVelCorrect::_computeConditionedValue()

{
  
  // we compute the conditioned value

  double filtStage1 = _filteredStage1[_condIndex];
  double filtSpike = _filteredSpike[_condIndex];
  double surfaceVel = _surfaceVel[_condIndex];
  double conditionedVel = surfaceVel;

  double absDiff = fabs(surfaceVel - filtStage1);
  if (absDiff > _params.spike_filter_difference_threshold) {
    conditionedVel = filtSpike;
  }

  _filteredCond[_condIndex] = conditionedVel;
  
}

/////////////////////////////////////////////////////////////////////////
// apply the final filter to compute the final filtered velocity

void HcrVelCorrect::_applyFinalFilter()

{
  
  size_t istart = _finalIndex - _lenFinalHalf;
  
  double sum = 0.0;
  double sumWts = 0.0;
  for (size_t ii = 0; ii < _lenFinal; ii++) {
    double vel = _filteredCond[ii + istart];
    double wt = _filtCoeffFinal[ii];
    sum += wt * vel;
    sumWts += wt;
  }
  
  double finalVal = _filteredCond[_finalIndex];
  if (sumWts > 0) {
    finalVal = sum / sumWts;
  }
  
  _filteredFinal[_finalIndex] = finalVal;

}

//////////////////////////////////////////////////
// convert all fields to specified output encoding

void HcrVelCorrect::_convertFieldsForOutput(RadxVol &vol)
{

  switch(_params.output_encoding) {
    case Params::OUTPUT_ENCODING_INT32:
      vol.convertToSi32();
      return;
    case Params::OUTPUT_ENCODING_INT16:
      vol.convertToSi16();
      return;
    case Params::OUTPUT_ENCODING_INT08:
      vol.convertToSi08();
      return;
    case Params::OUTPUT_ENCODING_FLOAT32:
    default:
      vol.convertToFl32();
      return;
  }

}

//////////////////////////////////////////////////
// write out the filtered volume

int HcrVelCorrect::_writeFiltVol()
{

  // set output encoding on all fields
  
  _convertFieldsForOutput(_filtVol);
  
  // set global attributes

  _setGlobalAttr(_filtVol);

  // set properties
  
  _filtVol.loadVolumeInfoFromRays();
  _filtVol.loadSweepInfoFromRays();

  // create output file
  
  GenericRadxFile outFile;
  _setupWrite(outFile);
  
  if (_params.output_filename_mode == Params::SPECIFY_FILE_NAME) {
    
    string outPath = _params.output_dir;
    outPath += PATH_DELIM;
    outPath += _params.output_filename;

    // write to path
    
    if (outFile.writeToPath(_filtVol, outPath)) {
      cerr << "ERROR - HcrVelCorrect::_writeFiltVol" << endl;
      cerr << "  Cannot write file to path: " << outPath << endl;
      cerr << outFile.getErrStr() << endl;
      return -1;
    }
      
  } else {

    // write to dir
  
    if (outFile.writeToDir(_filtVol, _params.output_dir,
                           _params.append_day_dir_to_output_dir,
                           _params.append_year_dir_to_output_dir)) {
      cerr << "ERROR - HcrVelCorrect::_writeFiltVol" << endl;
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
    if (ldata.write(_filtVol.getEndTimeSecs())) {
      cerr << "WARNING - HcrVelCorrect::_writeFiltVol" << endl;
      cerr << "  Cannot write latest data info file to dir: "
           << _params.output_dir << endl;
    }
  }

  // free up rays on vol

  _filtVol.clearRays();
  _filtVol.clearSweeps();
  _filtVol.clearRcalibs();
  _filtVol.clearCfactors();

  // copy the metadata from the current input volume

  _filtVol.copyMeta(_inVol);

  return 0;

}

//////////////////////////////////////////////////
// set up write

void HcrVelCorrect::_setupWrite(RadxFile &file)
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

  // set output format to CfRadial

  file.setFileFormat(RadxFile::FILE_FORMAT_CFRADIAL);
  file.setWriteCompressed(true);
  file.setCompressionLevel(_params.compression_level);

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

}

//////////////////////////////////////////////////
// set selected global attributes

void HcrVelCorrect::_setGlobalAttr(RadxVol &vol)
{

  vol.setDriver("HcrVelCorrect(NCAR)");
  
  string history(vol.getHistory());
  history += "\n";
  history += "Velocity filtering applied using HcrVelCorrect\n";
  vol.setHistory(history);
  
  RadxTime now(RadxTime::NOW);
  vol.setCreated(now.asString());

}

//////////////////////////////////////////////////
// Run in FMQ mode

int HcrVelCorrect::_runFmq()
{

  // Instantiate and initialize the input DsRadar queue and message
  
  if (_params.seek_to_end_of_input_fmq) {
    if (_inputFmq.init(_params.input_fmq_url, _progName.c_str(),
                       _params.debug >= Params::DEBUG_VERBOSE,
                       DsFmq::BLOCKING_READ_WRITE, DsFmq::END )) {
      fprintf(stderr, "ERROR - %s:Dsr2Radx::_run\n", _progName.c_str());
      fprintf(stderr, "Could not initialize radar queue '%s'\n",
	      _params.input_fmq_url);
      return -1;
    }
  } else {
    if (_inputFmq.init(_params.input_fmq_url, _progName.c_str(),
                       _params.debug >= Params::DEBUG_VERBOSE,
                       DsFmq::BLOCKING_READ_WRITE, DsFmq::START )) {
      fprintf(stderr, "ERROR - %s:Dsr2Radx::_run\n", _progName.c_str());
      fprintf(stderr, "Could not initialize radar queue '%s'\n",
	      _params.input_fmq_url);
      return -1;
    }
  }

  // create the output FMQ
  
  if (_outputFmq.init(_params.output_fmq_url,
                      _progName.c_str(),
                      _params.debug >= Params::DEBUG_VERBOSE,
                      DsFmq::READ_WRITE, DsFmq::END,
                      _params.output_fmq_compress,
                      _params.output_fmq_n_slots,
                      _params.output_fmq_buf_size)) {
    cerr << "WARNING - trying to create new fmq" << endl;
    if (_outputFmq.init(_params.output_fmq_url,
                        _progName.c_str(),
                        _params.debug >= Params::DEBUG_VERBOSE,
                        DsFmq::CREATE, DsFmq::START,
                        _params.output_fmq_compress,
                        _params.output_fmq_n_slots,
                        _params.output_fmq_buf_size)) {
      cerr << "ERROR - Radx2Dsr" << endl;
      cerr << "  Cannot open fmq, URL: " << _params.output_fmq_url << endl;
      return -1;
    }
  }
  
  if (_params.output_fmq_compress) {
    _outputFmq.setCompressionMethod(TA_COMPRESSION_GZIP);
  }
  
  if (_params.output_fmq_write_blocking) {
    _outputFmq.setBlockingWrite();
  }
  if (_params.output_fmq_data_mapper_report_interval > 0) {
    _outputFmq.setRegisterWithDmap
      (true, _params.output_fmq_data_mapper_report_interval);
  }

  // read messages from the queue and process them
  
  _nRaysRead = 0;
  _nRaysWritten = 0;
  _needWriteParams = false;
  RadxTime prevDwellRayTime;
  int iret = 0;
  while (true) {

    PMU_auto_register("Reading FMQ");

    RadxRay *ray = _readFmqRay();
    if (ray == NULL) {
      umsleep(100);
      continue;
    }

    // process this ray

    _processRay(ray);
    
    // write params if needed
    
    if (_needWriteParams) {
      if (_writeParams(ray)) {
        return -1; 
      }
      _needWriteParams = false;
    }
    
    // write out ray
    
    _writeRay(ray);

#ifdef JUNK

    // combine rays if combined time exceeds specified dwell

    const vector<RadxRay *> &raysDwell = _filtVol.getRays();
    size_t nRaysDwell = raysDwell.size();
    if (nRaysDwell > 1) {
      
      _dwellStartTime = raysDwell[0]->getRadxTime();
      _dwellEndTime = raysDwell[nRaysDwell-1]->getRadxTime();
      double dwellSecs = (_dwellEndTime - _dwellStartTime);
      dwellSecs *= ((double) nRaysDwell / (nRaysDwell - 1.0));
      
      if (dwellSecs >= _filtBufLen) {

        // dwell time exceeded, so compute dwell ray and add to volume

        if (_params.debug >= Params::DEBUG_VERBOSE) {
          cerr << "INFO: _runFmq, using nrays: " << nRaysDwell << endl;
        }
        RadxRay *dwellRay = _filtVol.computeFieldStats(_dwellStatsMethod);

        RadxTime dwellRayTime(dwellRay->getRadxTime());
        //double deltaSecs = dwellRayTime - prevDwellRayTime;

        prevDwellRayTime = dwellRayTime;
        
        // write params if needed

        if (_needWriteParams) {
          if (_writeParams(dwellRay)) {
            return -1; 
          }
          _needWriteParams = false;
        }

        // write out ray

        _writeRay(dwellRay);

        // clean up

        delete ray; // SHOULD CHANGE
        _georefs.clear();

      }

    } // if (nRaysDwell > 1)

#endif

  } // while (true)
  
  return iret;

}

////////////////////////////////////////////////////////////////////
// _readFmqRay()
//
// Read a ray from the FMQ, setting the flags about ray_data
// and _endOfVolume appropriately.
//
// Returns the ray on success, NULL on failure
// ray must be freed by caller.

RadxRay *HcrVelCorrect::_readFmqRay() 
  
{
  
  while (true) {

    PMU_auto_register("Reading radar queue");

    bool gotMsg = false;
    _inputContents = 0;
    if (_inputFmq.getDsMsg(_inputMsg, &_inputContents, &gotMsg)) {
      return NULL;
    }
    if (!gotMsg) {
      return NULL;
    }
    
    // set radar parameters if avaliable
    
    if (_inputContents & DsRadarMsg::RADAR_PARAMS) {
      _loadRadarParams();
      _needWriteParams = true;
    }
    
    // pass message through if not related to beam
    
    if (!(_inputContents & DsRadarMsg::RADAR_BEAM) &&
        !(_inputContents & DsRadarMsg::RADAR_PARAMS) &&
        !(_inputContents & DsRadarMsg::PLATFORM_GEOREF)) {
      if(_outputFmq.putDsMsg(_inputMsg, _inputContents)) {
        cerr << "ERROR - HcrVelCorrect::_runFmq()" << endl;
        cerr << "  Cannot copy message to output queue" << endl;
        cerr << "  URL: " << _params.output_fmq_url << endl;
        return NULL;
      }
    }

    // If we have radar and field params, and there is good ray data,
    
    if ((_inputContents & DsRadarMsg::RADAR_BEAM) && _inputMsg.allParamsSet()) {
        
      _nRaysRead++;
      
      // crete ray from ray message
      
      RadxRay *ray = _createInputRay();
      
      // debug print
      
      if (_params.debug) {
        if ((_nRaysRead > 0) && (_nRaysRead % 360 == 0)) {
          cerr << "==>>    read nRays, latest time, el, az: "
               << _nRaysRead << ", "
               << utimstr(ray->getTimeSecs()) << ", "
               << ray->getElevationDeg() << ", "
               << ray->getAzimuthDeg() << endl;
        }
      }
      
      // process that ray
      
      return ray;
      
    } // if (_inputContents ...

  } // while
  
  return NULL;

}

////////////////////////////////////////////////////////////////
// load radar params

void HcrVelCorrect::_loadRadarParams()

{
  
  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "=========>> got RADAR_PARAMS" << endl;
  }

  _rparams = _inputMsg.getRadarParams();
  
  _filtVol.setInstrumentName(_rparams.radarName);
  _filtVol.setScanName(_rparams.scanTypeName);
  
  switch (_rparams.radarType) {
    case DS_RADAR_AIRBORNE_FORE_TYPE: {
      _filtVol.setPlatformType(Radx::PLATFORM_TYPE_FIXED);
      break;
    }
    case DS_RADAR_AIRBORNE_AFT_TYPE: {
      _filtVol.setPlatformType(Radx::PLATFORM_TYPE_AIRCRAFT_FORE);
      break;
    }
    case DS_RADAR_AIRBORNE_TAIL_TYPE: {
      _filtVol.setPlatformType(Radx::PLATFORM_TYPE_AIRCRAFT_TAIL);
      break;
    }
    case DS_RADAR_AIRBORNE_LOWER_TYPE: {
      _filtVol.setPlatformType(Radx::PLATFORM_TYPE_AIRCRAFT_BELLY);
      break;
    }
    case DS_RADAR_AIRBORNE_UPPER_TYPE: {
      _filtVol.setPlatformType(Radx::PLATFORM_TYPE_AIRCRAFT_ROOF);
      break;
    }
    case DS_RADAR_SHIPBORNE_TYPE: {
      _filtVol.setPlatformType(Radx::PLATFORM_TYPE_SHIP);
      break;
    }
    case DS_RADAR_VEHICLE_TYPE: {
      _filtVol.setPlatformType(Radx::PLATFORM_TYPE_VEHICLE);
      break;
    }
    case DS_RADAR_GROUND_TYPE:
    default:{
      _filtVol.setPlatformType(Radx::PLATFORM_TYPE_FIXED);
    }
  }
  
  _filtVol.setLocation(_rparams.latitude,
                        _rparams.longitude,
                        _rparams.altitude);

  _filtVol.addWavelengthCm(_rparams.wavelength);
  _filtVol.setRadarBeamWidthDegH(_rparams.horizBeamWidth);
  _filtVol.setRadarBeamWidthDegV(_rparams.vertBeamWidth);

}

////////////////////////////////////////////////////////////////////
// add an input ray from an incoming message

RadxRay *HcrVelCorrect::_createInputRay()

{

  // input data

  const DsRadarBeam &rbeam = _inputMsg.getRadarBeam();
  const DsRadarParams &rparams = _inputMsg.getRadarParams();
  const vector<DsFieldParams *> &fparamsVec = _inputMsg.getFieldParams();

  // create new ray

  RadxRay *ray = new RadxRay;

  // set ray properties

  ray->setTime(rbeam.dataTime, rbeam.nanoSecs);
  ray->setVolumeNumber(rbeam.volumeNum);
  ray->setSweepNumber(rbeam.tiltNum);

  int scanMode = rparams.scanMode;
  if (rbeam.scanMode > 0) {
    scanMode = rbeam.scanMode;
  } else {
    scanMode = rparams.scanMode;
  }

  ray->setSweepMode(_getRadxSweepMode(scanMode));
  ray->setPolarizationMode(_getRadxPolarizationMode(rparams.polarization));
  ray->setPrtMode(_getRadxPrtMode(rparams.prfMode));
  ray->setFollowMode(_getRadxFollowMode(rparams.followMode));

  double elev = rbeam.elevation;
  if (elev > 180) {
    elev -= 360.0;
  }
  ray->setElevationDeg(elev);

  double az = rbeam.azimuth;
  if (az < 0) {
    az += 360.0;
  }
  ray->setAzimuthDeg(az);

  // range geometry

  int nGates = rparams.numGates;
  ray->setRangeGeom(rparams.startRange, rparams.gateSpacing);

  if (scanMode == DS_RADAR_RHI_MODE ||
      scanMode == DS_RADAR_EL_SURV_MODE) {
    ray->setFixedAngleDeg(rbeam.targetAz);
  } else {
    ray->setFixedAngleDeg(rbeam.targetElev);
  }

  ray->setIsIndexed(rbeam.beamIsIndexed);
  ray->setAngleResDeg(rbeam.angularResolution);
  ray->setAntennaTransition(rbeam.antennaTransition);
  ray->setNSamples(rparams.samplesPerBeam);
  
  ray->setPulseWidthUsec(rparams.pulseWidth);
  double prt = 1.0 / rparams.pulseRepFreq;
  ray->setPrtSec(prt);
  ray->setPrtRatio(1.0);
  ray->setNyquistMps(rparams.unambigVelocity);

  ray->setUnambigRangeKm(Radx::missingMetaDouble);
  ray->setUnambigRange();

  ray->setMeasXmitPowerDbmH(rbeam.measXmitPowerDbmH);
  ray->setMeasXmitPowerDbmV(rbeam.measXmitPowerDbmV);

  // platform georeference
  
  if (_inputContents & DsRadarMsg::PLATFORM_GEOREF) {
    const DsPlatformGeoref &platformGeoref = _inputMsg.getPlatformGeoref();
    const ds_iwrf_platform_georef_t &dsGeoref = platformGeoref.getGeoref();
    _georefs.push_back(platformGeoref);
    RadxGeoref georef;
    georef.setTimeSecs(dsGeoref.packet.time_secs_utc);
    georef.setNanoSecs(dsGeoref.packet.time_nano_secs);
    georef.setLongitude(dsGeoref.longitude);
    georef.setLatitude(dsGeoref.latitude);
    georef.setAltitudeKmMsl(dsGeoref.altitude_msl_km);
    georef.setAltitudeKmAgl(dsGeoref.altitude_agl_km);
    georef.setEwVelocity(dsGeoref.ew_velocity_mps);
    georef.setNsVelocity(dsGeoref.ns_velocity_mps);
    georef.setVertVelocity(dsGeoref.vert_velocity_mps);
    georef.setHeading(dsGeoref.heading_deg);
    georef.setRoll(dsGeoref.roll_deg);
    georef.setPitch(dsGeoref.pitch_deg);
    georef.setDrift(dsGeoref.drift_angle_deg);
    georef.setRotation(dsGeoref.rotation_angle_deg);
    georef.setTilt(dsGeoref.tilt_angle_deg);
    georef.setEwWind(dsGeoref.ew_horiz_wind_mps);
    georef.setNsWind(dsGeoref.ns_horiz_wind_mps);
    georef.setVertWind(dsGeoref.vert_wind_mps);
    georef.setHeadingRate(dsGeoref.heading_rate_dps);
    georef.setPitchRate(dsGeoref.pitch_rate_dps);
    georef.setDriveAngle1(dsGeoref.drive_angle_1_deg);
    georef.setDriveAngle2(dsGeoref.drive_angle_2_deg);
    ray->clearGeoref();
    ray->setGeoref(georef);
  }

  // load up fields

  int byteWidth = rbeam.byteWidth;
  
  for (size_t iparam = 0; iparam < fparamsVec.size(); iparam++) {

    // is this an output field or censoring field?

    const DsFieldParams &fparams = *fparamsVec[iparam];
    string fieldName = fparams.name;
    // if (_params.set_output_fields && !_isOutputField(fieldName)) {
    //   continue;
    // }

    // convert to floats
    
    Radx::fl32 *fdata = new Radx::fl32[nGates];

    if (byteWidth == sizeof(fl32)) {

      fl32 *inData = (fl32 *) rbeam.data() + iparam;
      fl32 inMissing = (fl32) fparams.missingDataValue;
      Radx::fl32 *outData = fdata;
      
      for (int igate = 0; igate < nGates;
           igate++, inData += fparamsVec.size(), outData++) {
        fl32 inVal = *inData;
        if (inVal == inMissing) {
          *outData = Radx::missingFl32;
        } else {
          *outData = *inData;
        }
      } // igate

    } else if (byteWidth == sizeof(ui16)) {

      ui16 *inData = (ui16 *) rbeam.data() + iparam;
      ui16 inMissing = (ui16) fparams.missingDataValue;
      double scale = fparams.scale;
      double bias = fparams.bias;
      Radx::fl32 *outData = fdata;
      
      for (int igate = 0; igate < nGates;
           igate++, inData += fparamsVec.size(), outData++) {
        ui16 inVal = *inData;
        if (inVal == inMissing) {
          *outData = Radx::missingFl32;
        } else {
          *outData = *inData * scale + bias;
        }
      } // igate

    } else {

      // byte width 1

      ui08 *inData = (ui08 *) rbeam.data() + iparam;
      ui08 inMissing = (ui08) fparams.missingDataValue;
      double scale = fparams.scale;
      double bias = fparams.bias;
      Radx::fl32 *outData = fdata;
      
      for (int igate = 0; igate < nGates;
           igate++, inData += fparamsVec.size(), outData++) {
        ui08 inVal = *inData;
        if (inVal == inMissing) {
          *outData = Radx::missingFl32;
        } else {
          *outData = *inData * scale + bias;
        }
      } // igate

    } // if (byteWidth == 4)

    RadxField *field = new RadxField(fparams.name, fparams.units);
    field->copyRangeGeom(*ray);
    field->setTypeFl32(Radx::missingFl32);
    field->addDataFl32(nGates, fdata);

    ray->addField(field);

    delete[] fdata;

  } // iparam

  return ray;

}

//////////////////////////////////////////////////
// write radar and field parameters

int HcrVelCorrect::_writeParams(const RadxRay *ray)

{

  // radar parameters
  
  DsRadarMsg msg;
  DsRadarParams &rparams = msg.getRadarParams();
  rparams = _rparams;
  rparams.numFields = ray->getNFields();
  
  // field parameters - all fields are fl32
  
  vector<DsFieldParams* > &fieldParams = msg.getFieldParams();
  
  for (int ifield = 0; ifield < (int) ray->getNFields(); ifield++) {
    
    const RadxField &fld = *ray->getFields()[ifield];
    double dsScale = 1.0, dsBias = 0.0;
    int dsMissing = (int) floor(fld.getMissingFl32() + 0.5);
    
    DsFieldParams *fParams = new DsFieldParams(fld.getName().c_str(),
                                               fld.getUnits().c_str(),
                                               dsScale, dsBias, sizeof(fl32),
                                               dsMissing);
    fieldParams.push_back(fParams);
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      fParams->print(cerr);
    }

  } // ifield

  // put params
  
  int content = DsRadarMsg::FIELD_PARAMS | DsRadarMsg::RADAR_PARAMS;
  if(_outputFmq.putDsMsg(msg, content)) {
    cerr << "ERROR - HcrVelCorrect::_writeParams()" << endl;
    cerr << "  Cannot write field params to FMQ" << endl;
    cerr << "  URL: " << _params.output_fmq_url << endl;
    return -1;
  }
        
  return 0;

}

//////////////////////////////////////////////////
// write ray

int HcrVelCorrect::_writeRay(const RadxRay *ray)
  
{

  // write params if needed

  int nGates = ray->getNGates();
  const vector<RadxField *> &fields = ray->getFields();
  int nFields = ray->getNFields();
  int nPoints = nGates * nFields;
  
  // meta-data
  
  DsRadarMsg msg;
  DsRadarBeam &beam = msg.getRadarBeam();

  beam.dataTime = ray->getTimeSecs();
  beam.nanoSecs = (int) (ray->getNanoSecs() + 0.5);
  beam.referenceTime = 0;

  beam.byteWidth = sizeof(fl32); // fl32

  beam.volumeNum = ray->getVolumeNumber();
  beam.tiltNum = ray->getSweepNumber();
  Radx::SweepMode_t sweepMode = ray->getSweepMode();
  beam.scanMode = _getDsScanMode(sweepMode);
  beam.antennaTransition = ray->getAntennaTransition();
  
  beam.azimuth = ray->getAzimuthDeg();
  beam.elevation = ray->getElevationDeg();
  
  if (sweepMode == Radx::SWEEP_MODE_RHI ||
      sweepMode == Radx::SWEEP_MODE_MANUAL_RHI) {
    beam.targetAz = ray->getFixedAngleDeg();
  } else {
    beam.targetElev = ray->getFixedAngleDeg();
  }

  beam.beamIsIndexed = ray->getIsIndexed();
  beam.angularResolution = ray->getAngleResDeg();
  beam.nSamples = ray->getNSamples();

  beam.measXmitPowerDbmH = ray->getMeasXmitPowerDbmH();
  beam.measXmitPowerDbmV = ray->getMeasXmitPowerDbmV();

  // 4-byte floats
  
  fl32 *data = new fl32[nPoints];
  for (int ifield = 0; ifield < nFields; ifield++) {
    fl32 *dd = data + ifield;
    const RadxField *fld = fields[ifield];
    const Radx::fl32 *fd = (Radx::fl32 *) fld->getData();
    if (fd == NULL) {
      cerr << "ERROR - Radx2Dsr::_writeBeam" << endl;
      cerr << "  NULL data pointer, field name, elev, az: "
           << fld->getName() << ", "
           << ray->getElevationDeg() << ", "
           << ray->getAzimuthDeg() << endl;
      delete[] data;
      return -1;
    }
    for (int igate = 0; igate < nGates; igate++, dd += nFields, fd++) {
      *dd = *fd;
    }
  }
  beam.loadData(data, nPoints * sizeof(fl32), sizeof(fl32));
  delete[] data;
    
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    beam.print(cerr);
  }
  
  // add georeference if applicable

  int contents = (int) DsRadarMsg::RADAR_BEAM;
  if (_georefs.size() > 0) {
    DsPlatformGeoref &georef = msg.getPlatformGeoref();
    // use mid georef
    georef = _georefs[_georefs.size() / 2];
    contents |= DsRadarMsg::PLATFORM_GEOREF;
  }
    
  // put beam
  
  if(_outputFmq.putDsMsg(msg, contents)) {
    cerr << "ERROR - HcrVelCorrect::_writeBeam()" << endl;
    cerr << "  Cannot write beam to FMQ" << endl;
    cerr << "  URL: " << _params.output_fmq_url << endl;
    return -1;
  }

  // debug print
  
  _nRaysWritten++;
  if (_params.debug) {
    if (_nRaysWritten % 100 == 0) {
      cerr << "====>> wrote nRays, latest time, el, az: "
           << _nRaysWritten << ", "
           << utimstr(ray->getTimeSecs()) << ", "
           << ray->getElevationDeg() << ", "
           << ray->getAzimuthDeg() << endl;
    }
  }
  
  return 0;

}

/////////////////////////////////////////////
// convert from DsrRadar enums to Radx enums

Radx::SweepMode_t HcrVelCorrect::_getRadxSweepMode(int dsrScanMode)

{

  switch (dsrScanMode) {
    case DS_RADAR_SECTOR_MODE:
    case DS_RADAR_FOLLOW_VEHICLE_MODE:
      return Radx::SWEEP_MODE_SECTOR;
    case DS_RADAR_COPLANE_MODE:
      return Radx::SWEEP_MODE_COPLANE;
    case DS_RADAR_RHI_MODE:
      return Radx::SWEEP_MODE_RHI;
    case DS_RADAR_VERTICAL_POINTING_MODE:
      return Radx::SWEEP_MODE_VERTICAL_POINTING;
    case DS_RADAR_MANUAL_MODE:
      return Radx::SWEEP_MODE_POINTING;
    case DS_RADAR_SURVEILLANCE_MODE:
      return Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE;
    case DS_RADAR_EL_SURV_MODE:
      return Radx::SWEEP_MODE_ELEVATION_SURVEILLANCE;
    case DS_RADAR_SUNSCAN_MODE:
      return Radx::SWEEP_MODE_SUNSCAN;
    case DS_RADAR_SUNSCAN_RHI_MODE:
      return Radx::SWEEP_MODE_SUNSCAN_RHI;
    case DS_RADAR_POINTING_MODE:
      return Radx::SWEEP_MODE_POINTING;
    default:
      return Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE;
  }
}

Radx::PolarizationMode_t HcrVelCorrect::_getRadxPolarizationMode(int dsrPolMode)

{
  switch (dsrPolMode) {
    case DS_POLARIZATION_HORIZ_TYPE:
      return Radx::POL_MODE_HORIZONTAL;
    case DS_POLARIZATION_VERT_TYPE:
      return Radx::POL_MODE_VERTICAL;
    case DS_POLARIZATION_DUAL_TYPE:
      return Radx::POL_MODE_HV_SIM;
    case DS_POLARIZATION_DUAL_HV_ALT:
      return Radx::POL_MODE_HV_ALT;
    case DS_POLARIZATION_DUAL_HV_SIM:
      return Radx::POL_MODE_HV_SIM;
    case DS_POLARIZATION_RIGHT_CIRC_TYPE:
    case DS_POLARIZATION_LEFT_CIRC_TYPE:
      return Radx::POL_MODE_CIRCULAR;
    case DS_POLARIZATION_ELLIPTICAL_TYPE:
    default:
      return Radx::POL_MODE_HORIZONTAL;
  }
}

Radx::FollowMode_t HcrVelCorrect::_getRadxFollowMode(int dsrMode)

{
  switch (dsrMode) {
    case DS_RADAR_FOLLOW_MODE_SUN:
      return Radx::FOLLOW_MODE_SUN;
    case DS_RADAR_FOLLOW_MODE_VEHICLE:
      return Radx::FOLLOW_MODE_VEHICLE;
    case DS_RADAR_FOLLOW_MODE_AIRCRAFT:
      return Radx::FOLLOW_MODE_AIRCRAFT;
    case DS_RADAR_FOLLOW_MODE_TARGET:
      return Radx::FOLLOW_MODE_TARGET;
    case DS_RADAR_FOLLOW_MODE_MANUAL:
      return Radx::FOLLOW_MODE_MANUAL;
    default:
      return Radx::FOLLOW_MODE_NONE;
  }
}

Radx::PrtMode_t HcrVelCorrect::_getRadxPrtMode(int dsrMode)

{
  switch (dsrMode) {
    case DS_RADAR_PRF_MODE_FIXED:
      return Radx::PRT_MODE_FIXED;
    case DS_RADAR_PRF_MODE_STAGGERED_2_3:
    case DS_RADAR_PRF_MODE_STAGGERED_3_4:
    case DS_RADAR_PRF_MODE_STAGGERED_4_5:
      return Radx::PRT_MODE_STAGGERED;
    default:
      return Radx::PRT_MODE_FIXED;
  }
}

//////////////////////////////////////////////////
// get Dsr enums from Radx enums

int HcrVelCorrect::_getDsScanMode(Radx::SweepMode_t mode)

{
  switch (mode) {
    case Radx::SWEEP_MODE_SECTOR:
      return DS_RADAR_SECTOR_MODE;
    case Radx::SWEEP_MODE_COPLANE:
      return DS_RADAR_COPLANE_MODE;
    case Radx::SWEEP_MODE_RHI:
      return DS_RADAR_RHI_MODE;
    case Radx::SWEEP_MODE_VERTICAL_POINTING:
      return DS_RADAR_VERTICAL_POINTING_MODE;
    case Radx::SWEEP_MODE_IDLE:
      return DS_RADAR_IDLE_MODE;
    case Radx::SWEEP_MODE_ELEVATION_SURVEILLANCE:
      return DS_RADAR_SURVEILLANCE_MODE;
    case Radx::SWEEP_MODE_SUNSCAN:
      return DS_RADAR_SUNSCAN_MODE;
    case Radx::SWEEP_MODE_POINTING:
      return DS_RADAR_POINTING_MODE;
    case Radx::SWEEP_MODE_MANUAL_PPI:
      return DS_RADAR_MANUAL_MODE;
    case Radx::SWEEP_MODE_MANUAL_RHI:
      return DS_RADAR_MANUAL_MODE;
    case Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE:
    default:
      return DS_RADAR_SURVEILLANCE_MODE;
  }
}

//////////////////////////////////////////////////
// initialize the filters

void HcrVelCorrect::_initFilters()
{

  // set up vectors of filter coefficients

  _filtCoeffStage1.clear();
  _filtCoeffSpike.clear();
  _filtCoeffFinal.clear();

  for (int ii = 0; ii < _params.stage1_filter_n; ii++) {
    _filtCoeffStage1.push_back(_params._stage1_filter[ii]);
  }
  
  for (int ii = 0; ii < _params.spike_filter_n; ii++) {
    _filtCoeffSpike.push_back(_params._spike_filter[ii]);
  }

  for (int ii = 0; ii < _params.final_filter_n; ii++) {
    _filtCoeffFinal.push_back(_params._final_filter[ii]);
  }
  
  // compute filter lengths and half-lengths

  _lenStage1 = _filtCoeffStage1.size();
  _lenStage1Half = _lenStage1 / 2;
  
  _lenSpike = _filtCoeffSpike.size();
  _lenSpikeHalf = _lenSpike / 2;

  _lenFinal = _filtCoeffFinal.size();
  _lenFinalHalf = _lenFinal / 2;

  // compute indices for filter results and
  // length of buffers required for filtering
  
  if (_lenSpike > _lenStage1) {
    _condIndex = _lenSpikeHalf;
    _finalIndex = _lenSpikeHalf + _lenFinalHalf;
    _filtBufLen = _lenSpike;
    if (_finalIndex + _lenFinalHalf > _filtBufLen) {
      _filtBufLen = _finalIndex + _lenFinalHalf;
    }
  } else {
    _condIndex = _lenStage1Half;
    _finalIndex = _lenStage1Half + _lenFinalHalf;
    _filtBufLen = _lenStage1;
    if (_finalIndex + _lenFinalHalf > _filtBufLen) {
      _filtBufLen = _finalIndex + _lenFinalHalf;
    }
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Filter summary:" << endl;
    cerr << "  Filter _lenStage1, _lenStage1Half: "
         << _lenStage1 << ", " << _lenStage1Half << endl;
    cerr << "  Filter _lenSpike, _lenSpikeHalf: "
         << _lenSpike << ", " << _lenSpikeHalf << endl;
    cerr << "  Filter _lenFinal, _lenFinalHalf: "
         << _lenFinal << ", " << _lenFinalHalf << endl;
    cerr << "  Conditioned index: " << _condIndex << endl;
    cerr << "  Final index: " << _finalIndex << endl;
    cerr << "  Filter buffer len: " << _filtBufLen << endl;
  }

  // set up arrays

  int nbytes = _filtBufLen * sizeof(double);
  
  _dbzMax = new double[_filtBufLen];
  _rangeToSurface = new double[_filtBufLen];
  _surfaceVel = new double[_filtBufLen];
  _filteredSpike = new double[_filtBufLen];
  _filteredStage1 = new double[_filtBufLen];
  _filteredCond = new double[_filtBufLen];
  _filteredFinal = new double[_filtBufLen];

  memset(_dbzMax, 0, nbytes);
  memset(_rangeToSurface, 0, nbytes);
  memset(_surfaceVel, 0, nbytes);
  memset(_filteredSpike, 0, nbytes);
  memset(_filteredStage1, 0, nbytes);
  memset(_filteredCond, 0, nbytes);
  memset(_filteredFinal, 0, nbytes);

}

//////////////////////////////////////////////////
// shift the arrays by 1

void HcrVelCorrect::_shiftArrays()
{
  
  int nbytesMove = (_filtBufLen - 1) * sizeof(double);
  
  memmove(_dbzMax + 1, _dbzMax, nbytesMove);
  memmove(_rangeToSurface + 1, _rangeToSurface, nbytesMove);
  memmove(_surfaceVel + 1, _surfaceVel, nbytesMove);
  memmove(_filteredSpike + 1, _filteredSpike, nbytesMove);
  memmove(_filteredStage1 + 1, _filteredStage1, nbytesMove);
  memmove(_filteredCond + 1, _filteredCond, nbytesMove);
  memmove(_filteredFinal + 1, _filteredFinal, nbytesMove);

}

//////////////////////////////////////////////////
// correct velocity on ray

void HcrVelCorrect::_correctVelForRay(RadxRay *ray)

{

  double surfVel = _filteredFinal[_finalIndex];
  RadxField *velField = ray->getField(_params.vel_field_name);
  if (velField == NULL) {
    // no vel field, nothing to do
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "WARNING - no vel field found: " << _params.vel_field_name << endl;
    }
    return;
  }

  // create the corrected field
  
  RadxField *correctedField = new RadxField(_params.corrected_vel_field_name,
                                            velField->getUnits());
  correctedField->copyMetaData(*velField);
  correctedField->setName(_params.corrected_vel_field_name);

  // correct the values
  
  const Radx::fl32 *vel = velField->getDataFl32();
  Radx::fl32 miss = velField->getMissingFl32();
  Radx::fl32 *corrected = new Radx::fl32[velField->getNPoints()];
  for (size_t ii = 0; ii < velField->getNPoints(); ii++) {
    if (vel[ii] != miss) {
      corrected[ii] = vel[ii] - surfVel;
    } else {
      corrected[ii] = miss;
    }
  }

  // set data for field

  correctedField->setDataFl32(velField->getNPoints(), corrected, true);
  delete[] corrected;

  // add field to ray

  ray->addField(correctedField);

}

//////////////////////////////////////////////////
// copy velocity across for yay

void HcrVelCorrect::_copyVelForRay(RadxRay *ray)
  
{
  
  RadxField *velField = ray->getField(_params.vel_field_name);
  if (velField == NULL) {
    // no vel field, nothing to do
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "WARNING - no vel field found: " << _params.vel_field_name << endl;
    }
    return;
  }

  // create the field to be copied
  
  RadxField *copyField = new RadxField(_params.corrected_vel_field_name,
                                       velField->getUnits());
  copyField->copyMetaData(*velField);
  copyField->setName(_params.corrected_vel_field_name);
  
  // copy the values
  
  const Radx::fl32 *vel = velField->getDataFl32();
  Radx::fl32 *copy = new Radx::fl32[velField->getNPoints()];
  for (size_t ii = 0; ii < velField->getNPoints(); ii++) {
    copy[ii] = vel[ii];
  }

  // set data for field

  copyField->setDataFl32(velField->getNPoints(), copy, true);
  delete[] copy;

  // add field to ray

  ray->addField(copyField);

}

