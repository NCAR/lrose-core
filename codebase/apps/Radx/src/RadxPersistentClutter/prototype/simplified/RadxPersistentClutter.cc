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
// Based on following paper:
// Lakshmanan V., J. Zhang, K. Hondl and C. Langston.
// A Statistical Approach to Mitigating Persistent Clutter in
// Radar Reflectivity Data.
// IEEE Journal of Selected Topics in Applied Earth Observations
// and Remote Sensing, Vol. 5, No. 2, April 2012.
///////////////////////////////////////////////////////////////////////////

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
#include <toolsa/LogStreamInit.hh>
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
  LogStreamInit::init(false, false, true, true);
  LOG_STREAM_DISABLE(LogStream::WARNING);
  LOG_STREAM_DISABLE(LogStream::DEBUG);
  LOG_STREAM_DISABLE(LogStream::DEBUG_VERBOSE);
  LOG_STREAM_DISABLE(LogStream::DEBUG_EXTRA);
  if (_params.debug_mode >= Params::DEBUG_EXTRA) {
    LOG_STREAM_ENABLE(LogStream::DEBUG_EXTRA);
    LOG_STREAM_ENABLE(LogStream::DEBUG_VERBOSE);
    LOG_STREAM_ENABLE(LogStream::DEBUG);
  } else if (_params.debug_mode >= Params::DEBUG_VERBOSE) {
    LOG_STREAM_ENABLE(LogStream::DEBUG_VERBOSE);
    LOG_STREAM_ENABLE(LogStream::DEBUG);
  } else if (_params.debug_mode >= Params::DEBUG) {
    LOG_STREAM_ENABLE(LogStream::DEBUG);
    LOG_STREAM_ENABLE(LogStream::WARNING);
  }
  if (_params.debug_triggering) {
    LogStreamInit::setThreading(true);
  }

  // initialize the derived paramaters

  if (_initDerivedParams()) {
    OK = false;
    return;
  }

  // initialize threading
  
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

  if (_params.mode == Params::ARCHIVE) {
    RadxTimeList tlist;
    tlist.setDir(_params.input_dir);
    tlist.setModeInterval(_start, _end);
    if (tlist.compile()) {
      LOG(ERROR) << "Cannot compile time list, dir: " 
		 << _params.input_dir;
      LOG(ERROR) << "   Start time: " << RadxTime::strm(_start);
      LOG(ERROR) << "   End time: " << RadxTime::strm(_end);
      LOG(ERROR) << tlist.getErrStr();
      return -1;
    }
    _paths = tlist.getPathList();
    _pathIndex = 0;
    if (_paths.size() < 1) {
      LOG(ERROR) << "No files found, dir: " << _params.input_dir;
      return -1;
    }
  } else if (_params.mode == Params::FILELIST) {
    // file list is in the args
    _paths = _fileList;
    _pathIndex = 0;
  } else {
    // set the LdataInfo object for REALTIME
    _ldata = LdataInfo(_params.input_dir, _params.debug_triggering);
    PMU_auto_init(_progName.c_str(), _params.instance,
                  PROCMAP_REGISTER_INTERVAL);
  }

  // ray maps
  
  _rayMap = RayMapping(_params.sweep_fixed_angles_n, _params._sweep_fixed_angles,
                       _params.az_tolerance_degrees,
                       _params.elev_tolerance_degrees);
  
  // fixed angles

  for (int ii = 0; ii < _params.sweep_fixed_angles_n; ii++) {
    _fixedAngles.push_back(_params._sweep_fixed_angles[ii]);
  }
  sort(_fixedAngles.begin(), _fixedAngles.end());
  
  return 0;

}

//------------------------------------------------------------------
bool RadxPersistentClutter::run(const string &label)
{

  LOG(DEBUG) << "=============>> run() - starting " << label << " <<================";
  
  RadxVol vol;
  time_t t;

  bool first = true;
  bool filesDone = false;
  
  // trigger at time
  while (true) {

    if (!_trigger(vol, t, filesDone)) {
      if (filesDone) {
        return false;
      } else {
        continue;
      }
    }
    
    if (first) {
      // virtual method
      initFirstTime(t, vol);
      _processFirst(t, vol);
      first = false;
    }
    
    // process the vol
    bool algDone = _process(t, vol);

    if (algDone) {
      LOG(DEBUG) << "=========================================================";
      LOG(DEBUG) << label << " has converged <<===============";
      LOG(DEBUG) << "=========================================================";
    } else {
      LOG(DEBUG) << label << " has NOT converged <<===============";
    }
    
    if (algDone) {
      // it has converged
      _thread.waitForThreads();
      // virtual method
      finishLastTimeGood(t, vol);
      return true;
    }
  } // while

  // did not converge
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

  RayData r;
  RayClutterInfo *h = alg->_initRayThreaded(*info->_ray, r);
  if (h != NULL) {
    // call a virtual method
    alg->processRay(r, h);
  }
  delete info;
}    

//------------------------------------------------------------------
RayClutterInfo *RadxPersistentClutter::_initRayThreaded(const RadxRay &ray,
							RayData &r)
{
  // lock because the method can change ray, in spite of the const!
  _thread.lockForIO();
  if (!retrieveRay(_params.input_field_name, ray, r))
  {
    _thread.unlockAfterIO();
    return NULL;
  }
  _thread.unlockAfterIO();

  // Point to the matching clutter info for this ray
  double az = ray.getAzimuthDeg();
  double elev = ray.getElevationDeg();
  RayClutterInfo *h = matchingClutterInfo(az, elev);
  // if (h == NULL) {
  //   LOG(DEBUG_EXTRA) << "No histo match for az=" << az << " elev=" << elev;
  // } else {
  //   LOG(DEBUG_EXTRA) << "Updating ray az=" << az << " elev=" << elev;
  // }
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
  for (std::map<RayAzElev, RayClutterInfo>::const_iterator ii = _store.begin();
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
  for (std::map<RayAzElev, RayClutterInfo>::iterator ii = _store.begin();
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
  RayData r;

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
						RayData &r)
{
  if (!retrieveRay(_params.input_field_name, ray, r)) {
    return NULL;
  }

  // Point to the matching clutter info for this ray
  double az = ray.getAzimuthDeg();
  double elev = ray.getElevationDeg();
  RayClutterInfo *h = matchingClutterInfo(az, elev);
  // if (h == NULL) {
  //   LOG(DEBUG_EXTRA) << "No histo match for az=" << az << " elev=" << elev;
  // } else {
  //   LOG(DEBUG_EXTRA) << "Updating ray az:" << az << " elev:" << elev;
  // }
  return h;
}

//------------------------------------------------------------------
bool RadxPersistentClutter::_trigger(RadxVol &v, time_t &t, bool &done)
{

  LOG(DEBUG) << "------before trigger-----";

  done = false;
  if (_params.mode == Params::ARCHIVE ||
      _params.mode == Params::FILELIST) {
    if (_pathIndex >= (int)_paths.size()) {
      LOG(DEBUG) << "---No more files to process--";
      done = true;
      return true;
    }
    string inputPath = _paths[_pathIndex++];
    if (_readFile(inputPath, v, t)) {
      return true;
    } else {
      return false;
    }
  } else {
    _ldata.readBlocking(_params.max_realtime_data_age_secs,  1000,
			PMU_auto_register);
    string inputPath = _ldata.getDataPath();
    if (_readFile(inputPath, v, t)) {
      return true;
    } else {
      return false;
    }
  }
  return false;
}

//---------------------------------------------------------------
bool RadxPersistentClutter::_rewind(void)
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
bool RadxPersistentClutter::_write(RadxVol &vol, const time_t &t,
                                   const std::string &dir)
{
  vol.loadVolumeInfoFromRays();
  vol.loadSweepInfoFromRays();
  vol.setPackingFromRays();

  RadxFile *outFile;
  NcfRadxFile *ncfFile = new NcfRadxFile();
  outFile = ncfFile;

  _setupWrite(*outFile);

  if (outFile->writeToDir(vol, dir, true, false)) {
    LOG(ERROR) << "Cannot write file to dir: " <<  dir;
    LOG(ERROR) << outFile->getErrStr();
    delete outFile;
    return false;
  }
  string outputPath = outFile->getPathInUse();
  delete outFile;

  string name = nameWithoutPath(outputPath);
  LOG(DEBUG) << "==>> Wrote output path: " << outputPath << " <<==";

  // in realtime mode, write latest data info file

  if (_params.mode == Params::REALTIME) {
    LdataInfo ldata(dir);
    if (LOG_STREAM_IS_ENABLED(LogStream::DEBUG_VERBOSE)) {
      ldata.setDebug(true);
    }
    RadxPath rpath(outputPath);
    ldata.setRelDataPath(rpath.getFile());
    ldata.setWriter(_progName);
    if (ldata.write(vol.getEndTimeSecs())) {
      LOG(WARNING) << "Cannot write latest data info file to dir: " << dir;
    }
  }
  return true;
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
                                        std::vector<RayData> &data,
                                        RayData &r)
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
                                        RayData &r,
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
      r = RayData(name, fields[ifield]->getUnits(),
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
void RadxPersistentClutter::modifyRayForOutput(RayData &r,
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
void RadxPersistentClutter::updateRay(const RayData &r, RadxRay &ray)
{
  // add in the one RayData, then clear out everything else
    
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
void RadxPersistentClutter::updateRay(const vector<RayData> &raydata,
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
bool RadxPersistentClutter::_readFile(const string &path,
                                      RadxVol &vol, time_t &t)
{
  string name = nameWithoutPath(path);

  LOG(DEBUG) << "Reading file: " <<  path;
  
  RadxFile primaryFile;
  _setupRead(primaryFile);
  
  if (primaryFile.readFromPath(path, vol)) {
    LOG(ERROR) << "Cannot read in file: " << path;
    LOG(ERROR) << primaryFile.getErrStr();
    return false;
  }

  t = vol.getStartTimeSecs();
  // bool dateOnly;
  // if (DataFileNames::getDataTime(path, t, dateOnly)) {
  //   LOG(ERROR) << "Cannot get time from file name: " << name;
  //   return false;
  // }

  LOG(DEBUG) << "Time for file: " << RadxTime::strm(t);

  // check if this is an RHI
  
  _isRhi = vol.checkIsRhi();

  // remove sweeps we do not want, and set the sweep angles
  // to the selected ones

  if (!_isRhi) {
    vol.optimizeSurveillanceTransitions(_params.elev_tolerance_degrees);
  }

  // set each full PPI to 360 deg if appropriate
  
  vol.trimSurveillanceSweepsTo360Deg();
  
  // remove transition rays and optimize the 
  
  vol.removeTransitionRays();

  // remove sweeps we do not want, and set the sweep angles
  // to the selected ones

  if (_isRhi) {
    vol.trimSweepsToSelectedAngles(_fixedAngles, _params.az_tolerance_degrees);
  } else {
    vol.trimSweepsToSelectedAngles(_fixedAngles, _params.elev_tolerance_degrees);
  }

  vector<RadxRay *> &rays = vol.getRays();
  for (size_t ii = 0; ii < rays.size(); ii++) {
    RadxRay *ray = rays[ii];
    if (_isRhi) {
      ray->setAzimuthDeg(ray->getFixedAngleDeg());
    } else {
      ray->setElevationDeg(ray->getFixedAngleDeg());
    }
  }

  
  LOG(DEBUG) << "-------Triggered " << RadxTime::strm(t) << " ----------";
  return true;
}

//------------------------------------------------------------------
void RadxPersistentClutter::_setupRead(RadxFile &file)
{
  if (LOG_STREAM_IS_ENABLED(LogStream::DEBUG_EXTRA)) {
    file.setDebug(true);
    file.printReadRequest(cerr);
  }

  file.addReadField(_params.input_field_name);
  file.setReadIgnoreTransitions(true);
  
  if (_params.set_max_range) {
    file.setReadMaxRangeKm(_params.max_range_km);
  }
  
  LOG(DEBUG_VERBOSE) << "===== SETTING UP READ FOR PRIMARY FILES =====";
  LOG(DEBUG_VERBOSE) << "=============================================";
}



//------------------------------------------------------------------
void RadxPersistentClutter::_setupWrite(RadxFile &file)
{

  if (LOG_STREAM_IS_ENABLED(LogStream::DEBUG_VERBOSE))
  {
    file.setDebug(true);
  }
  
  file.setWriteFileNameMode(RadxFile::FILENAME_WITH_START_AND_END_TIMES);

  file.setWriteCompressed(true);
  file.setCompressionLevel(4);

  file.setFileFormat(RadxFile::FILE_FORMAT_CFRADIAL);

}

