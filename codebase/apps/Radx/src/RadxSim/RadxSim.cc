// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2026                                         
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
//
// RadxSim.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 2026
//
///////////////////////////////////////////////////////////////
//
// RadxSim is intended to simulate Radx-type files in a realtime
// scenario.
//
// RadxSim loops through a set of polar CfRadial files,
// modifies the ray times to wall-clock time, and writes the
// modified files to an output dir.
//
///////////////////////////////////////////////////////////////

#include "RadxSim.hh"

#include <Mdv/GenericRadxFile.hh>
#include <Radx/Radx.hh>
#include <Radx/RadxPath.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxTimeList.hh>
#include <Radx/RadxVol.hh>
#include <dsserver/DsLdataInfo.hh>
#include <toolsa/pmu.h>

#include <algorithm>
#include <cerrno>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits.h>
#include <set>
#include <sstream>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

using namespace std;

RadxSim::RadxSim(int argc, char **argv) :
        OK(TRUE),
        _progName("RadxSim"),
        _paramsPath((char *) "unknown"),
        _stopRequested(0),
        _procmapRegistered(false),
        _archiveStart(0.0),
        _archiveEnd(0.0),
        _publicationSequence(0)
{
  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR - " << _progName << ": invalid command line" << endl;
    OK = FALSE;
    return;
  }
  if (_params.loadFromArgs(argc, argv, _args.override.list, &_paramsPath)) {
    cerr << "ERROR - " << _progName << ": cannot load TDRP parameters"
         << endl;
    OK = FALSE;
  }
}

RadxSim::~RadxSim()
{
  if (_procmapRegistered) {
    PMU_auto_unregister();
  }
}

int RadxSim::Run()
{
  if (_validateParams() || _resolveInputs()) {
    return -1;
  }

  PMU_auto_init(_progName.c_str(), _params.instance,
                PROCMAP_REGISTER_INTERVAL);
  _procmapRegistered = true;
  PMU_auto_register("Starting replay");

  return _runCycles();
}

int RadxSim::_validateParams() const
{
  bool valid = true;
  if (!std::isfinite(_params.replay_rate) || _params.replay_rate <= 0.0) {
    cerr << "ERROR - replay_rate must be finite and greater than zero" << endl;
    valid = false;
  }
  if (!std::isfinite(_params.time_offset_secs)) {
    cerr << "ERROR - time_offset_secs must be finite" << endl;
    valid = false;
  }
  if (_params.repeat_replay &&
      (!std::isfinite(_params.inter_cycle_gap_secs) ||
       _params.inter_cycle_gap_secs <= 0.0)) {
    cerr << "ERROR - inter_cycle_gap_secs must be finite and greater than "
         << "zero when repeat is enabled" << endl;
    valid = false;
  }
  if (_params.compression_level < 0 || _params.compression_level > 9) {
    cerr << "ERROR - compression_level must be between 0 and 9" << endl;
    valid = false;
  }
  if (strlen(_params.output_dir) == 0) {
    cerr << "ERROR - output_dir may not be empty" << endl;
    valid = false;
  }
  return valid ? 0 : -1;
}

int RadxSim::_collectCandidatePaths(vector<string> &paths) const
{
  if (_params.input_mode == Params::FILE_LIST) {
    for (int ii = 0; ii < _params.input_file_paths_n; ++ii) {
      paths.push_back(_params._input_file_paths[ii]);
    }
    paths.insert(paths.end(), _args.inputFileList.begin(),
                 _args.inputFileList.end());
    if (paths.empty()) {
      cerr << "ERROR - FILE_LIST mode requires input_file_paths or -f"
           << endl;
      return -1;
    }
    return 0;
  }

  time_t startTime = _args.startTime;
  time_t endTime = _args.endTime;
  if (!_args.startTimeSet) {
    startTime = RadxTime::parseDateTime(_params.archive_start_time);
  }
  if (!_args.endTimeSet) {
    endTime = RadxTime::parseDateTime(_params.archive_end_time);
  }
  if (startTime == RadxTime::NEVER || endTime == RadxTime::NEVER ||
      endTime < startTime) {
    cerr << "ERROR - invalid inclusive archive time range" << endl;
    return -1;
  }

  RadxTimeList timeList;
  timeList.setDir(_params.input_dir);
  timeList.setModeInterval(startTime, endTime);
  if (timeList.compile()) {
    cerr << "ERROR - cannot compile archive input list from: "
         << _params.input_dir << endl;
    cerr << timeList.getErrStr() << endl;
    return -1;
  }
  paths = timeList.getPathList();
  if (paths.empty()) {
    cerr << "ERROR - no archive inputs found in: " << _params.input_dir
         << endl;
    return -1;
  }
  return 0;
}

int RadxSim::_resolveInputs()
{
  vector<string> candidates;
  if (_collectCandidatePaths(candidates)) {
    return -1;
  }

  if (RadxPath::makeDirRecurse(_params.output_dir)) {
    cerr << "ERROR - cannot create output directory: "
         << _params.output_dir << ": " << strerror(errno) << endl;
    return -1;
  }
  const string canonicalOutput = _canonicalPath(_params.output_dir);
  if (_params.input_mode == Params::ARCHIVE) {
    const string canonicalInputRoot = _canonicalPath(_params.input_dir);
    if (canonicalInputRoot == canonicalOutput ||
        canonicalInputRoot.find(canonicalOutput + "/") == 0 ||
        canonicalOutput.find(canonicalInputRoot + "/") == 0) {
      cerr << "ERROR - archive input and output trees must be separate: "
           << canonicalInputRoot << " and " << canonicalOutput << endl;
      return -1;
    }
  }

  set<string> seen;
  int inputErrors = 0;
  for (size_t ii = 0; ii < candidates.size(); ++ii) {
    const string path = _canonicalPath(candidates[ii]);
    if (!seen.insert(path).second) {
      if (_params.debug) {
        cerr << "WARNING - ignoring duplicate input path: " << path << endl;
      }
      continue;
    }

    const string inputDir = _canonicalPath(RadxPath(path).getDirectory());
    if (inputDir == canonicalOutput ||
        inputDir.find(canonicalOutput + "/") == 0 ||
        canonicalOutput.find(inputDir + "/") == 0) {
      cerr << "ERROR - input and output directories must be separate: "
           << inputDir << " and " << canonicalOutput << endl;
      return -1;
    }

    InputFile input;
    if (_inspectInput(path, input)) {
      ++inputErrors;
      if (!_params.continue_on_input_error) {
        return -1;
      }
      continue;
    }
    _inputs.push_back(input);
  }

  if (_inputs.empty()) {
    cerr << "ERROR - no valid input volumes were resolved" << endl;
    return -1;
  }

  sort(_inputs.begin(), _inputs.end(), _inputLess);
  _archiveStart = _inputs.front().startTime;
  _archiveEnd = _inputs.front().endTime;
  for (size_t ii = 1; ii < _inputs.size(); ++ii) {
    _archiveStart = min(_archiveStart, _inputs[ii].startTime);
    _archiveEnd = max(_archiveEnd, _inputs[ii].endTime);
  }

  if (_params.debug) {
    cerr << "Resolved " << _inputs.size() << " input volumes" << endl;
    if (inputErrors > 0) {
      cerr << "Skipped " << inputErrors << " invalid inputs" << endl;
    }
    const time_t startSecs = (time_t) floor(_archiveStart);
    const time_t endSecs = (time_t) floor(_archiveEnd);
    const RadxTime startTime(startSecs, _archiveStart - startSecs);
    const RadxTime endTime(endSecs, _archiveEnd - endSecs);
    cerr << "Archive span: " << startTime.asString(6)
         << " to " << endTime.asString(6) << endl;
  }
  return 0;
}

int RadxSim::_inspectInput(const string &path, InputFile &input) const
{
  RadxVol vol;
  if (_readVolume(path, vol)) {
    return -1;
  }

  const vector<RadxRay *> &rays = vol.getRays();
  if (rays.empty()) {
    cerr << "ERROR - input volume contains no rays: " << path << endl;
    return -1;
  }
  if (vol.getPlatformType() != Radx::PLATFORM_TYPE_NOT_SET &&
      vol.getPlatformType() != Radx::PLATFORM_TYPE_FIXED) {
    cerr << "ERROR - phase one supports only fixed-platform input: "
         << path << endl;
    return -1;
  }
  for (size_t ii = 0; ii < rays.size(); ++ii) {
    if (rays[ii]->getGeoreference() != NULL) {
      cerr << "ERROR - phase one does not retime ray georeference metadata: "
           << path << endl;
      return -1;
    }
    if (ii > 0 && rays[ii]->getTimeDouble() <
                    rays[ii - 1]->getTimeDouble()) {
      cerr << "ERROR - ray times decrease in input volume: " << path << endl;
      return -1;
    }
  }

  input.path = path;
  input.startTime = rays.front()->getTimeDouble();
  input.endTime = rays.back()->getTimeDouble();
  return 0;
}

int RadxSim::_readVolume(const string &path, RadxVol &vol) const
{
  GenericRadxFile inFile;
  if (_params.debug) {
    inFile.setDebug(true);
  }
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    inFile.setVerbose(true);
  }
  if (inFile.readFromPath(path, vol)) {
    cerr << "ERROR - cannot read input volume: " << path << endl;
    cerr << inFile.getErrStr() << endl;
    return -1;
  }
  return 0;
}

int RadxSim::_runCycles()
{
  double dataCycleStart = _wallClockNow() + _params.time_offset_secs;
  double monotonicCycleStart = _monotonicNow();

  do {
    if (_runCycle(dataCycleStart, monotonicCycleStart)) {
      return _stopRequested ? 0 : -1;
    }
    if (!_params.repeat_replay || _stopRequested) {
      break;
    }

    const double scaledDuration =
        (_archiveEnd - _archiveStart) / _params.replay_rate;
    dataCycleStart += scaledDuration + _params.inter_cycle_gap_secs;
    // Preserve a real gap after a late cycle rather than trying to catch up.
    monotonicCycleStart = max(monotonicCycleStart + scaledDuration,
                              _monotonicNow()) +
                          _params.inter_cycle_gap_secs;
  } while (!_stopRequested);

  return 0;
}

int RadxSim::_runCycle(double dataCycleStart, double monotonicCycleStart)
{
  for (size_t ii = 0; ii < _inputs.size() && !_stopRequested; ++ii) {
    const InputFile &input = _inputs[ii];
    RadxVol vol;
    PMU_auto_register("Reading input volume");
    if (_readVolume(input.path, vol)) {
      if (_params.continue_on_input_error) {
        continue;
      }
      return -1;
    }
    if (_retimeVolume(vol, dataCycleStart)) {
      return -1;
    }

    const double deadline = monotonicCycleStart +
        (input.endTime - _archiveStart) / _params.replay_rate;
    if (_waitUntil(deadline, "Waiting for volume completion")) {
      return -1;
    }
    if (_stopRequested) {
      return 0;
    }
    if (_publishVolume(vol)) {
      return -1;
    }
  }
  return 0;
}

int RadxSim::_retimeVolume(RadxVol &vol, double dataCycleStart) const
{
  vector<RadxRay *> &rays = vol.getRays();
  if (rays.empty()) {
    return -1;
  }

  for (size_t ii = 0; ii < rays.size(); ++ii) {
    const double mappedTime = dataCycleStart +
        (rays[ii]->getTimeDouble() - _archiveStart) / _params.replay_rate;
    rays[ii]->setTime(mappedTime);
  }

  // Ray time is authoritative; rebuild every summary and boundary flag after
  // the complete immutable mapping has been applied.
  vol.loadSweepInfoFromRays();
  vol.loadVolumeInfoFromRays();

  ostringstream history;
  history << vol.getHistory();
  if (!vol.getHistory().empty() &&
      vol.getHistory()[vol.getHistory().size() - 1] != '\n') {
    history << '\n';
  }
  history << "RadxSim replay rate: " << _params.replay_rate
          << ", data-time offset seconds: " << _params.time_offset_secs
          << '\n';
  vol.setHistory(history.str());
  return 0;
}

int RadxSim::_waitUntil(double deadline, const string &activity)
{
  while (!_stopRequested) {
    const double remaining = deadline - _monotonicNow();
    if (remaining <= 0.0) {
      if (_params.debug >= Params::DEBUG_VERBOSE && remaining < -0.001) {
        cerr << "Replay deadline late by " << -remaining << " seconds"
             << endl;
      }
      return 0;
    }

    PMU_auto_register(activity.c_str());
    const double chunk = min(remaining, 1.0);
    struct timespec request;
    request.tv_sec = (time_t) chunk;
    request.tv_nsec = (long) ((chunk - request.tv_sec) * 1.0e9);
    while (nanosleep(&request, &request) && errno == EINTR &&
           !_stopRequested) {
      // Resume only the unslept portion; the outer loop rechecks the absolute
      // deadline so signal handling and scheduler delays cannot accumulate.
    }
  }
  return -1;
}

int RadxSim::_publishVolume(RadxVol &vol)
{
  PMU_auto_register("Writing output volume");
  GenericRadxFile outFile;
  if (_params.debug) {
    outFile.setDebug(true);
  }
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    outFile.setVerbose(true);
  }
  outFile.setFileFormat(RadxFile::FILE_FORMAT_CFRADIAL);
  outFile.setNcFormat(RadxFile::NETCDF4);
  outFile.setWriteFileNameMode(RadxFile::FILENAME_WITH_START_AND_END_TIMES);
  outFile.setWriteSubsecsInFileName(true);
  char suffix[64];
  snprintf(suffix, sizeof(suffix), "_sim_%08llu", _publicationSequence++);
  outFile.setWriteFileNameSuffix(suffix);
  outFile.setWriteCompressed(_params.compression_level > 0);
  outFile.setCompressionLevel(_params.compression_level);

  if (outFile.writeToDir(vol, _params.output_dir,
                         _params.append_day_dir_to_output_dir,
                         _params.append_year_dir_to_output_dir)) {
    cerr << "ERROR - cannot publish CfRadial volume to: "
         << _params.output_dir << endl;
    cerr << outFile.getErrStr() << endl;
    return -1;
  }

  // The renamed data file is the commit point. Publish latest-data state only
  // afterward so readers can never be directed to an incomplete path.
  const string outputPath = outFile.getPathInUse();
  string relativePath;
  RadxPath::stripDir(_params.output_dir, outputPath, relativePath);
  DsLdataInfo latest(_params.output_dir);
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    latest.setDebug(true);
  }
  latest.setRelDataPath(relativePath);
  latest.setWriter(_progName);
  if (latest.write(vol.getStartTimeSecs())) {
    cerr << "ERROR - data file was published, but _latest_data_info failed: "
         << outputPath << endl;
    return -1;
  }

  if (_params.debug) {
    cerr << "Published: " << outputPath << endl;
  }
  return 0;
}

double RadxSim::_wallClockNow()
{
  struct timeval now;
  gettimeofday(&now, NULL);
  return now.tv_sec + now.tv_usec / 1.0e6;
}

double RadxSim::_monotonicNow()
{
  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC, &now);
  return now.tv_sec + now.tv_nsec / 1.0e9;
}

string RadxSim::_canonicalPath(const string &path)
{
  char resolved[PATH_MAX];
  if (realpath(path.c_str(), resolved) != NULL) {
    return resolved;
  }
  return path;
}

bool RadxSim::_inputLess(const InputFile &lhs, const InputFile &rhs)
{
  if (lhs.startTime < rhs.startTime) {
    return true;
  }
  if (lhs.startTime > rhs.startTime) {
    return false;
  }
  return lhs.path < rhs.path;
}
