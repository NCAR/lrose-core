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
/**
 * @file RadxPersistentClutter.cc
 */

#include "RadxPersistentClutter.hh"
#include "Info.hh"
#include <Radx/RadxRay.hh>
#include <Radx/RadxVol.hh>
#include <Radx/RadxTimeList.hh>
#include <Radx/RadxPath.hh>
#include <Radx/NcfRadxFile.hh>
#include <toolsa/pmu.h>
#include <toolsa/TaThreadSimple.hh>
#include <toolsa/LogStream.hh>
#include <toolsa/LogMsgStreamInit.hh>
#include <didss/DataFileNames.hh>
#include <algorithm>

//------------------------------------------------------------------
TaThread *RadxPersistentClutter::ComputeThread::clone(int index)
{
  TaThreadSimple *t = new TaThreadSimple(index);
  t->setThreadContext(this);
  t->setThreadMethod(RadxPersistentClutter::compute);
  return dynamic_cast<TaThread *>(t);
}

//------------------------------------------------------------------
RadxPersistentClutter::RadxPersistentClutter(int argc, char **argv)
{

  OK = TRUE;

  // set programe name

  _progName = "RadxPersistentClutter";
  ucopyright((char *) _progName.c_str());
  
  // parse command line args
  
  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args." << endl;
    OK = FALSE;
    return;
  }
  
  _start = _args.startTime;
  _end = _args.endTime;
  _fileList = _args.inputFileList;

  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv,
			   _args.override.list, &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters." << endl;
    OK = FALSE;
    return;
  }

  // set up debugging state for logging  using params
  LogMsgStreamInit::init(_params.debug_mode == Params::DEBUG ||
			 _params.debug_mode == Params::DEBUG_VERBOSE,
			 _params.debug_mode == Params::DEBUG_VERBOSE,
			 true, true);
  if (_params.debug_triggering) {
    LogMsgStreamInit::setThreading(true);
  }

  // initialize the derived paramaters

  if (_initDerivedParams()) {
    OK = false;
    return;
  }
  
  _rayMap = RayxMapping(_params.fixedElevations_n, _params._fixedElevations,
                        _params.azToleranceDegrees,
                        _params.elevToleranceDegrees);
  
  _thread.init(_params.num_threads, _params.thread_debug);

}

//------------------------------------------------------------------
RadxPersistentClutter::~RadxPersistentClutter(void)
{
  PMU_auto_unregister();
}

//------------------------------------------------------------------

//------------------------------------------------------------------
// Initialize the derived parameters from the main params class

int RadxPersistentClutter::_initDerivedParams()
{

  bool status = true;
  if (_params.mode != Params::REALTIME) {
    _params.max_wait_minutes = 0;
  }
  
  // Build up the single primary URL and the secondary URLs
  int primaryIndex = -1;
  for (int i=0; i<_params.input_n; ++i) {
    if (i == 0) {
      primaryIndex = _params._input[i].index;
      _primaryGroup.index = _params._input[i].index;
      _primaryGroup.dir = _params._input[i].path;
      _primaryGroup.fileTimeOffset = _params._input[i].file_match_time_offset_sec;
      _primaryGroup.fileTimeTolerance =
	_params._input[i].file_match_time_tolerance_sec;
      _primaryGroup.rayElevTolerance = 
	_params._input[i].ray_match_elevation_tolerance_deg;
      _primaryGroup.rayAzTolerance =
	_params._input[i].ray_match_azimuth_tolerance_deg;
      _primaryGroup.rayTimeTolerance =
	_params._input[i].ray_match_time_tolerance_sec;
    } else {
      Group G;
      G.dir = _params._input[i].path;
      G.index = _params._input[i].index;
      G.fileTimeOffset = _params._input[i].file_match_time_offset_sec;
      G.fileTimeTolerance =
	_params._input[i].file_match_time_tolerance_sec;
      G.rayElevTolerance = 
	_params._input[i].ray_match_elevation_tolerance_deg;
      G.rayAzTolerance =
	_params._input[i].ray_match_azimuth_tolerance_deg;
      G.rayTimeTolerance =
	_params._input[i].ray_match_time_tolerance_sec;
      _secondaryGroups.push_back(G);
    }
  }

  for (int i=0; i<_params.field_mapping_n; ++i) {
    if (_params._field_mapping[i].index == primaryIndex) {
      _primaryGroup.names.push_back(_params._field_mapping[i].field);
    } else {
      bool found = false;
      for (int j=0; j<(int)_secondaryGroups.size(); ++j) {
	if (_secondaryGroups[j].index == _params._field_mapping[i].index) {
	  _secondaryGroups[j].names.push_back(_params._field_mapping[i].field);
	  found = true;
	  break;
	}
      }
      if (!found) {
	LOG(ERROR) << "Never found index " << _params._field_mapping[i].index
		   << " in mappings, not used";
	status = false;
      }
    }
  }

  if (_primaryGroup.names.empty())
  {
    LOG(ERROR) << "Primary URL not used in a mapping";
    status = false;
  }

  if (!status) {
    return -1;
  }
  
  if (_params.mode == Params::ARCHIVE) {
    RadxTimeList tlist;
    tlist.setDir(_primaryGroup.dir);
    tlist.setModeInterval(_start, _end);
    if (tlist.compile()) {
      LOG(ERROR) << "Cannot compile time list, dir: " 
		 << _primaryGroup.dir;
      LOG(ERROR) << "   Start time: " << RadxTime::strm(_start);
      LOG(ERROR) << "   End time: " << RadxTime::strm(_end);
      LOG(ERROR) << tlist.getErrStr();
      return -1;
    }
    _paths = tlist.getPathList();
    _pathIndex = 0;
    if (_paths.size() < 1) {
      LOG(ERROR) << "No files found, dir: " << _primaryGroup.dir;
      return -1;
    }
  } else if (_params.mode == Params::FILELIST) {
    // file list is in the args
    _paths = _fileList;
    _pathIndex = 0;
  } else {
    // set the LdataInfo object for REALTIME
    _ldata = LdataInfo(_primaryGroup.dir, _params.debug_triggering);
    PMU_auto_init(_progName.c_str(), _params.instance,
                  PROCMAP_REGISTER_INTERVAL);
  }
  
  // check the inputs to make sure on the list somewhere
  vector<string> input;
  input.push_back(_params.input_field);
  if (!inputsAccountedFor(input)) {
    return -1;
  }

  return 0;

}

//------------------------------------------------------------------
bool RadxPersistentClutter::run(void)
{

  RadxVol vol;
  time_t t;
  bool last;

  bool first = true;

  // trigger at time
  while (trigger(vol, t, last)) {

    if (first) {
      // virtual method
      initFirstTime(t, vol);
      _processFirst(t, vol);
      first = false;
    }
    
    // process the vol
    bool done = _process(t, vol);

    if (done) {
      // it has converged
      _thread.waitForThreads();
      // virtual method
      finishLastTimeGood(t, vol);
      return true;
    }
  }
  // virtual method
  finishBad();
  return false;
}

//----------------------------------------------------------------
void RadxPersistentClutter::compute(void *ti)
{
  Info *info = static_cast<Info *>(ti);
  RadxPersistentClutter *alg = info->_alg;

  if (info->_time == 0 || info->_ray == NULL || info->_alg == NULL) {
    LOG(ERROR) << "Values not set on entry";
    return;
  }

  RayxData r;
  RayClutterInfo *h = alg->_initRayThreaded(*info->_ray, r);
  if (h != NULL) {
    // call a virtual method
    alg->processRay(r, h);
  }
  delete info;
}    

//------------------------------------------------------------------
RayClutterInfo *RadxPersistentClutter::_initRayThreaded(const RadxRay &ray,
							RayxData &r)
{
  // lock because the method can change ray, in spite of the const!
  _thread.lockForIO();
  if (!retrieveRay(_params.input_field, ray, r))
  {
    _thread.unlockAfterIO();
    return NULL;
  }
  _thread.unlockAfterIO();

  // Point to the matching clutter info for this ray
  double az = ray.getAzimuthDeg();
  double elev = ray.getElevationDeg();
  RayClutterInfo *h = matchingClutterInfo(az, elev);
  if (h == NULL) {
    LOG(WARNING) << "No histo match for az=" << az << " elev=" << elev;
  } else {
    LOG(DEBUG_VERBOSE) << "Updating ray az=" << az << " elev=" << elev;
  }
  return h;
}

//------------------------------------------------------------------
void RadxPersistentClutter::_processForOutput(RadxVol &vol)
{
  // break the volume into rays and process each one
  const vector<RadxRay *> &rays = vol.getRays();
  for (size_t ii = 0; ii < rays.size(); ii++) {
    RadxRay *ray = rays[ii];
    _processRayForOutput(*ray);
  }
}

//------------------------------------------------------------------
double RadxPersistentClutter::_countOfScans(const int number) const
{
  double ret = 0.0;
  for (std::map<RadxAzElev, RayClutterInfo>::const_iterator ii = _store.begin();
       ii!=_store.end(); ++ii) {
    ret += ii->second.numWithMatchingCount(number);
  }
  return ret;
}

//------------------------------------------------------------------
int RadxPersistentClutter::_updateClutterState(const int kstar,
                                               FrequencyCount &F)
{
  int nchange = 0;
  int nclutter = 0;
  // count up changes in clutter value, and update _store internal state
  for (std::map<RadxAzElev, RayClutterInfo>::iterator ii = _store.begin();
       ii!=_store.end(); ++ii) {
    nchange += ii->second.updateClutter(kstar, nclutter, F);
  }
  return nchange;
}

//------------------------------------------------------------------
bool RadxPersistentClutter::_process(const time_t t, RadxVol &vol)
{

  // break the volume into rays and process each one
  const vector<RadxRay *> &rays = vol.getRays();
  LOG(DEBUG_VERBOSE) << "Nrays=" << rays.size();
  for (size_t ii = 0; ii < rays.size(); ii++) {
    RadxRay *ray = rays[ii];
    _processRay(t, ray);
  }
  _thread.waitForThreads();

  // virtual method at the end of processing all rays
  return processFinishVolume(t, vol);
}

//------------------------------------------------------------------
void RadxPersistentClutter::_processFirst(const time_t t, const RadxVol &vol)
{

  // break the volume into rays and process each one
  const vector<RadxRay *> &rays = vol.getRays();
  LOG(DEBUG_VERBOSE) << "Nrays=" << rays.size();
  for (size_t ii = 0; ii < rays.size(); ii++) {
    RadxRay *ray = rays[ii];
    // virtual method to pre-process each ray, done first volume only
    preProcessRay(*ray);
  }
}

//------------------------------------------------------------------
void RadxPersistentClutter::_processRay(const time_t &t, const RadxRay *ray)
{

  // create an info pointer
  Info *info = new Info();

  // set the info values
  info->set(t, ray, this);

  int index = 0;
  _thread.thread(index, info);
}

//------------------------------------------------------------------
bool RadxPersistentClutter::_processRayForOutput(RadxRay &ray)
{
  RayxData r;

  // initialization
  RayClutterInfo *h = _initRay(ray, r);
  if (h != NULL) {
    // call a virtual method
    return setRayForOutput(h, r, ray);
  } else {
    return false;
  }
}


//------------------------------------------------------------------
RayClutterInfo *RadxPersistentClutter::_initRay(const RadxRay &ray,
						RayxData &r)
{
  if (!retrieveRay(_params.input_field, ray, r)) {
    return NULL;
  }

  // Point to the matching clutter info for this ray
  double az = ray.getAzimuthDeg();
  double elev = ray.getElevationDeg();
  RayClutterInfo *h = matchingClutterInfo(az, elev);
  if (h == NULL) {
    LOG(WARNING) << "No histo match for az=" << az << " elev=" << elev;
  } else {
    LOG(DEBUG_VERBOSE) << "Updating ray az:" << az << " elev:" << elev;
  }
  return h;
}

//------------------------------------------------------------------
bool RadxPersistentClutter::trigger(RadxVol &v, time_t &t, bool &last)
{

  LOG(DEBUG) << "------before trigger-----";

  if (_params.mode == Params::ARCHIVE ||
      _params.mode == Params::FILELIST) {
    if (_pathIndex >= (int)_paths.size()) {
      LOG(DEBUG) << "---No more files to process--";
      return false;
    }
    string inputPath = _paths[_pathIndex++];
    last = _pathIndex == (int)_paths.size();
    return _processFile(inputPath, v, t);
  } else {
    _ldata.readBlocking(_params.max_realtime_data_age_secs,  1000,
			PMU_auto_register);
    string inputPath = _ldata.getDataPath();
    last = false;
    return _processFile(inputPath, v, t);
  }
}

//---------------------------------------------------------------
bool RadxPersistentClutter::rewind(void)
{
  bool ret = true;
  if (_params.mode == Params::ARCHIVE) {
    _pathIndex = 0;
  } else if (_params.mode == Params::FILELIST) {
    _pathIndex = 0;
  } else {
    LOG(ERROR) << "Cannot rewind in real time mode";
    ret = false;
  }
  return ret;
}

//---------------------------------------------------------------
bool RadxPersistentClutter::write(RadxVol &vol, const time_t &t,
                                  const std::string &url)
{
  vol.loadVolumeInfoFromRays();
  vol.loadSweepInfoFromRays();
  vol.setPackingFromRays();

  RadxFile *outFile;
  NcfRadxFile *ncfFile = new NcfRadxFile();
  outFile = ncfFile;

  _setupWrite(*outFile);

  if (outFile->writeToDir(vol, url, true, false)) {
    LOG(ERROR) << "Cannot write file to dir: " <<  url;
    LOG(ERROR) << outFile->getErrStr();
    delete outFile;
    return false;
  }
  string outputPath = outFile->getPathInUse();
  delete outFile;

  string name = nameWithoutPath(outputPath);
  LOG(DEBUG) << "Wrote output file: " << name;

  // in realtime mode, write latest data info file

  if (_params.mode == Params::REALTIME) {
    LdataInfo ldata(url);
    if (LOG_STREAM_IS_ENABLED(LogStream::DEBUG_VERBOSE)) {
      ldata.setDebug(true);
    }
    RadxPath rpath(outputPath);
    ldata.setRelDataPath(rpath.getFile());
    ldata.setWriter(_progName);
    if (ldata.write(vol.getEndTimeSecs())) {
      LOG(WARNING) << "Cannot write latest data info file to dir: " << url;
    }
  }
  return true;
}

//---------------------------------------------------------------
bool RadxPersistentClutter::write(RadxVol &vol, const time_t &t)
{
  return write(vol, t, _params.output_url);
}

//------------------------------------------------------------------
string RadxPersistentClutter::nameWithoutPath(const string &name)
{
  string::size_type i = name.find_last_of("/");
  string ret = name.substr(i+1);
  return ret;
}

//---------------------------------------------------------------
bool RadxPersistentClutter::retrieveRay(const std::string &name,
                                        const RadxRay &ray,
                                        std::vector<RayxData> &data,
                                        RayxData &r)
{
  // try to find the field in the ray first
  if (retrieveRay(name, ray, r, false)) {
    return true;
  }

  // try to find the field in the data vector
  for (size_t i=0; i<data.size(); ++i){
    if (data[i].getName() == name){
      // return a copy of that ray
      r = data[i];
      return true;
    }
  }
    
  LOG(ERROR) << "Field " << name << "never found";
  return false;
}  

//---------------------------------------------------------------
bool RadxPersistentClutter::retrieveRay(const std::string &name,
                                        const RadxRay &ray,
                                        RayxData &r,
                                        const bool showError)
{
  // try to find the field in the ray
  const vector<RadxField *> &fields = ray.getFields();
  for (size_t ifield = 0; ifield < fields.size(); ifield++) {
    if (fields[ifield]->getName() == name) {
      Radx::DataType_t t = fields[ifield]->getDataType();
      if (t != Radx::FL32) {
	// need this to pull out values
	fields[ifield]->convertToFl32();
      }
      r = RayxData(name, fields[ifield]->getUnits(),
                   fields[ifield]->getNPoints(), fields[ifield]->getMissing(),
                   ray.getAzimuthDeg(), ray.getElevationDeg(),
                   ray.getGateSpacingKm(), ray.getStartRangeKm(),
                   *fields[ifield]);
      return true;
    }
  }
  if (showError) {
    LOG(ERROR) << "Field " << name << "never found";
  }
  return false;
}

//---------------------------------------------------------------
void RadxPersistentClutter::modifyRayForOutput(RayxData &r,
                                               const std::string &name,
                                               const std::string &units,
                                               const double missing)
{
  r.changeName(name);
  if (units.empty()) {
    return;
  }
  r.changeUnits(units);
  r.changeMissing(missing);
}

//---------------------------------------------------------------
void RadxPersistentClutter::updateRay(const RayxData &r, RadxRay &ray)
{
  // add in the one RayxData, then clear out everything else
    
  int nGatesPrimary = ray.getNGates();
  Radx::fl32 *data = new Radx::fl32[nGatesPrimary];

  string name = r.getName();
  r.retrieveData(data, nGatesPrimary);
  ray.addField(name, r.getUnits(), r.getNpoints(), r.getMissing(), data, true);
  vector<string> wanted;
  wanted.push_back(name);
  ray.trimToWantedFields(wanted);
  delete [] data;
}

//---------------------------------------------------------------
void RadxPersistentClutter::updateRay(const vector<RayxData> &raydata,
                                      RadxRay &ray)
{
  // take all the data and add to the ray.
  // note that here should make sure not deleting a result (check names)
  if (raydata.empty()) {
    return;
  }

  int nGatesPrimary = ray.getNGates();
  Radx::fl32 *data = new Radx::fl32[nGatesPrimary];

  string name = raydata[0].getName();
  raydata[0].retrieveData(data, nGatesPrimary);
  ray.addField(name, raydata[0].getUnits(), raydata[0].getNpoints(),
	       raydata[0].getMissing(), data, true);
  vector<string> wanted;
  wanted.push_back(name);
  ray.trimToWantedFields(wanted);

  // now add in all the other ones
  for (int i=1; i<(int)raydata.size(); ++i) {
    raydata[i].retrieveData(data, nGatesPrimary);
    ray.addField(raydata[i].getName(), raydata[i].getUnits(),
		 raydata[i].getNpoints(),
		 raydata[i].getMissing(), data, true);
  }
  delete [] data;
}

//---------------------------------------------------------------
bool RadxPersistentClutter::_processFile(const string &path,
                                         RadxVol &vol, time_t &t)
{
  string name = nameWithoutPath(path);

  LOG(DEBUG) << "Primary file: " <<  name;
  
  RadxFile primaryFile;
  _setupRead(primaryFile);

  if (primaryFile.readFromPath(path, vol)) {
    LOG(ERROR) << "Cannot read in primary file: " << name;
    LOG(ERROR) << primaryFile.getErrStr();
    return false;
  }

  t = vol.getEndTimeSecs();
  bool dateOnly;
  if (DataFileNames::getDataTime(path, t, dateOnly)) {
    LOG(ERROR) << "Cannot get time from file path: " << name;
    return false;
  }

  LOG(DEBUG) << "Time for primary file: " << RadxTime::strm(t);

  // Search for secondary files

  for (size_t igroup = 0; igroup < _secondaryGroups.size(); igroup++) {
    
    _activeGroup = _secondaryGroups[igroup];
    
    string secondaryPath;
    time_t searchTime = t + _activeGroup.fileTimeOffset;
    
    RadxTimeList tlist;
    tlist.setDir(_activeGroup.dir);
    tlist.setModeClosest(searchTime, _activeGroup.fileTimeTolerance);
    
    if (LOG_STREAM_IS_ENABLED(LogStream::DEBUG_VERBOSE)) {
      tlist.printRequest(cerr);
    }
    
    if (tlist.compile()) {
      LOG(ERROR) << "Cannot compile secondary file time list";
      LOG(ERROR) << tlist.getErrStr().c_str();
      return false;
    }
    const vector<string> &pathList = tlist.getPathList();
    if (pathList.size() < 1) {
      LOG(WARNING) << "No suitable secondary file found, " 
                   << "Primary file:" << path << ", Secondary path:"
                   << _activeGroup.dir;
      return false;
    } else {
      secondaryPath = pathList[0];
    }
    
    string secondaryName = nameWithoutPath(secondaryPath);
    LOG(DEBUG) << "Found secondary file: " << secondaryName;

    RadxFile secondaryFile;
    _setupSecondaryRead(secondaryFile);
    
    RadxVol secondaryVol;
    if (secondaryFile.readFromPath(secondaryPath, secondaryVol)) {
      LOG(ERROR) << "Cannot read in secondary file: " << secondaryName;
      LOG(ERROR) << secondaryFile.getErrStr();
      return false;
    }

    // merge the primary and seconday volumes, using the primary
    // volume to hold the merged data
    
    if (!_mergeVol(vol, secondaryVol)) {
      LOG(ERROR) << "Merge failed, primary:" << name
		 << "secondary:" << secondaryName;
      return false;
    }
    
  } // igroup
  
  
  // remove some bad stuff we never will want
  vol.removeTransitionRays();
  vol.trimSurveillanceSweepsTo360Deg();

  LOG(DEBUG) << "-------Triggered " << RadxTime::strm(t) << " ----------";
  return true;
}

//------------------------------------------------------------------
void RadxPersistentClutter::_setupRead(RadxFile &file)
{
  if (LOG_STREAM_IS_ENABLED(LogStream::DEBUG_VERBOSE)) {
    file.setDebug(true);
  }

  if (LOG_STREAM_IS_ENABLED(LogStream::DEBUG_VERBOSE)) {
    file.setVerbose(true);
    file.printReadRequest(cerr);
  }

  if (_params.read_set_fixed_angle_limits) {
    file.setReadFixedAngleLimits(_params.read_lower_fixed_angle,
                                 _params.read_upper_fixed_angle);
  }
  
  for (size_t ii = 0; ii < _primaryGroup.names.size(); ii++) {
    file.addReadField(_primaryGroup.names[ii]);
  }

  if (_params.ignore_antenna_transitions) {
    file.setReadIgnoreTransitions(true);
  }

  file.setReadAggregateSweeps(false);
  
  if (_params.set_max_range) {
    file.setReadMaxRangeKm(_params.max_range_km);
  }
  
  
  LOG(DEBUG_VERBOSE) << "===== SETTING UP READ FOR PRIMARY FILES =====";
  if (LOG_STREAM_IS_ENABLED(LogStream::DEBUG_VERBOSE)) {
    file.printReadRequest(cerr);
  }
  LOG(DEBUG_VERBOSE) << "=============================================";
}



//------------------------------------------------------------------
void RadxPersistentClutter::_setupSecondaryRead(RadxFile &file)
{
  if (LOG_STREAM_IS_ENABLED(LogStream::DEBUG_VERBOSE))
  {
    file.setDebug(true);
  }
  if (LOG_STREAM_IS_ENABLED(LogStream::DEBUG_VERBOSE))
  {
    file.setVerbose(true);
  }

  if (_params.read_set_fixed_angle_limits)
  {
    file.setReadFixedAngleLimits(_params.read_lower_fixed_angle,
                                 _params.read_upper_fixed_angle);
  }

  for (size_t ii = 0; ii < _activeGroup.names.size(); ii++)
  {
    file.addReadField(_activeGroup.names[ii]);
  }

  file.setReadAggregateSweeps(false);

  LOG(DEBUG_VERBOSE) << "===== SETTING UP READ FOR SECONDARY FILES =====";
  if (LOG_STREAM_IS_ENABLED(LogStream::DEBUG_VERBOSE))
  {
    file.printReadRequest(cerr);
  }
  LOG(DEBUG_VERBOSE) << "===============================================";
}

//------------------------------------------------------------------
void RadxPersistentClutter::_setupWrite(RadxFile &file)
{
  if (LOG_STREAM_IS_ENABLED(LogStream::DEBUG_VERBOSE))
  {
    file.setDebug(true);
  }

  if (LOG_STREAM_IS_ENABLED(LogStream::DEBUG_VERBOSE))
  {
    file.setVerbose(true);
  }
  
  file.setWriteFileNameMode(RadxFile::FILENAME_WITH_START_AND_END_TIMES);

  file.setWriteCompressed(true);
  file.setCompressionLevel(4);

  file.setFileFormat(RadxFile::FILE_FORMAT_CFRADIAL);

}

//------------------------------------------------------------------
bool RadxPersistentClutter::_mergeVol(RadxVol &primaryVol,
                                      const RadxVol &secondaryVol)
{
  // loop through all rays in primary vol

  const vector<RadxRay *> &pRays = primaryVol.getRays();
  int searchStart = 0;

  for (size_t ii = 0; ii < pRays.size(); ii++) {

    RadxRay *pRay = pRays[ii];
    double pTime = (double) pRay->getTimeSecs() + pRay->getNanoSecs() / 1.0e9;
    double pAz = pRay->getAzimuthDeg();   
    double pEl = pRay->getElevationDeg();

    int pMilli = pRay->getNanoSecs() / 1.0e6;
    char pMStr[16];
    sprintf(pMStr, "%.3d", pMilli);

    // find matching ray in secondary volume

    const vector<RadxRay *> &sRays = secondaryVol.getRays();
    bool found = false;
    for (size_t jj = searchStart; jj < sRays.size(); jj++)
    {
      RadxRay *sRay = sRays[jj];
      double sTime = (double) sRay->getTimeSecs() + sRay->getNanoSecs() / 1.0e9;
      double sAz = sRay->getAzimuthDeg();   
      double sEl = sRay->getElevationDeg();
      
      int sMilli = sRay->getNanoSecs() / 1.0e6;
      char sMStr[16];
      sprintf(sMStr, "%.3d", sMilli);

      double diffTime;
      diffTime = fabs(pTime - sTime);
      double dAz = pAz - sAz;
      if (dAz < -180)
      {
        dAz += 360.0;
      }
      else if (dAz > 180)
      {
        dAz -= 360.0;
      }
      double diffAz = fabs(dAz);
      double diffEl = fabs(pEl - sEl);

      if (diffTime <= _activeGroup.rayTimeTolerance &&
          diffAz <= _activeGroup.rayAzTolerance &&
          diffEl <= _activeGroup.rayElevTolerance)
      {
        // same ray, merge the rays
        _mergeRay(*pRay, *sRay);
        found = true;
	LOG(DEBUG_VERBOSE) << "Matched ray - time "
			   << RadxTime::strm((time_t)pTime) + "." + pMStr
			   << " az,el=" << pAz << "," << pEl 
			   << " az2,el2=" << sAz << "," << sEl
			   << " dEl=" << diffEl
			   << " dAz=" << diffAz
			   << " dTime=" << diffTime;
	break;
      }
    } // jj

    if (!found)
    {
      LOG(DEBUG_VERBOSE) << "===>>> missed merge, time="
			 << RadxTime::strm((time_t) pTime) + "." + pMStr
			 << " az,el:" << pAz << "," << pEl;
    }
  } // ii

  return true;

}

//------------------------------------------------------------------
void RadxPersistentClutter::_mergeRay(RadxRay &primaryRay,
                                      const RadxRay &secondaryRay)
{

  primaryRay.loadFieldNameMap();

  // compute lookup in case geometry differs
  RadxRemap remap;
  bool geomDiffers =
    remap.checkGeometryIsDifferent(secondaryRay.getStartRangeKm(),
                                   secondaryRay.getGateSpacingKm(),
                                   primaryRay.getStartRangeKm(),
                                   primaryRay.getGateSpacingKm());
  if (geomDiffers) {
    remap.prepareForInterp(secondaryRay.getNGates(),
                           secondaryRay.getStartRangeKm(),
                           secondaryRay.getGateSpacingKm(),
                           primaryRay.getStartRangeKm(),
                           primaryRay.getGateSpacingKm());
  }
  
  const vector<RadxField *> &sFields = secondaryRay.getFields();
  int nGatesPrimary = primaryRay.getNGates();

  for (size_t ifield = 0; ifield < sFields.size(); ifield++) {
    
    RadxField sField(*sFields[ifield]);

    // get output field name

    string outputName = sField.getName();

    // ensure geometry is correct

    if (geomDiffers)
    {
      sField.remapRayGeom(remap);
    }
    sField.setNGates(nGatesPrimary);

    Radx::DataType_t dType = sField.getDataType();
    switch (dType) {
      case Radx::UI08:
      case Radx::SI08: {
        primaryRay.addField(outputName,
                            sField.getUnits(),
                            sField.getNPoints(),
                            sField.getMissingSi08(),
                            (const Radx::si08 *) sField.getData(),
                            sField.getScale(),
                            sField.getOffset(),
                            true);
        break;
      }
      case Radx::UI32:
      case Radx::SI32: {
        primaryRay.addField(outputName,
                            sField.getUnits(),
                            sField.getNPoints(),
                            sField.getMissingSi32(),
                            (const Radx::si32 *) sField.getData(),
                            sField.getScale(),
                            sField.getOffset(),
                            true);
        break;
      }
      case Radx::FL32: {
        primaryRay.addField(outputName,
                            sField.getUnits(),
                            sField.getNPoints(),
                            sField.getMissingFl32(),
                            (const Radx::fl32 *) sField.getData(),
                            true);
        break;
      }
      case Radx::FL64: {
        primaryRay.addField(outputName,
                            sField.getUnits(),
                            sField.getNPoints(),
                            sField.getMissingFl64(),
                            (const Radx::fl64 *) sField.getData(),
                            true);
        break;
      }
      case Radx::UI16:
      case Radx::SI16:
      default: {
        primaryRay.addField(outputName,
                            sField.getUnits(),
                            sField.getNPoints(),
                            sField.getMissingSi16(),
                            (const Radx::si16 *) sField.getData(),
                            sField.getScale(),
                            sField.getOffset(),
                            true);
      }
    } // switch
  } // ifield

}

//------------------------------------------------------------------
bool RadxPersistentClutter::inputsAccountedFor(const vector<string> &inputs) const
{
  bool status = true;
  for (int i=0; i<(int)inputs.size(); ++i) {
    string name = inputs[i];
    bool found = false;
    for (int j=0; j<(int)_primaryGroup.names.size(); ++j) {
      string n2 = _primaryGroup.names[j];
      if (n2 == name) {
	found = true;
	break;
      }
    }
    if (!found) {
      for (int j=0; j<(int)_secondaryGroups.size(); ++j) {
	for (int k=0; k<(int)_secondaryGroups[j].names.size(); ++k) {
	  string n2 = _secondaryGroups[j].names[k];
	  if (n2 == name) {
	    found = true;
	    break;
	  }
	}
	if (found) {
	  break;
	}
      }
    }
    if (!found) {
      LOG(ERROR) << "Never found input " << name << " in indexing";
      status = false;
    }
  }
  return status;
}
