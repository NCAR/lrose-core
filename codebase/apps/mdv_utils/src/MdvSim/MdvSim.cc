// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1990 - 2026
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Boulder, Colorado, USA
// ** BSD license applies - redistribution and use in source and binary
// ** forms, with or without modification, are permitted provided that the
// ** copyright notice, conditions, and disclaimer are retained.
// ** THIS SOFTWARE IS PROVIDED "AS IS" WITHOUT EXPRESS OR IMPLIED WARRANTY.
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
///////////////////////////////////////////////////////////////
//
// MdvSim.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 2026
//
///////////////////////////////////////////////////////////////
//
// MdvSim is intended to simulate MDV files in a realtime
// scenario.
//
// MdvSim loops through a set of MDV files,
// modifies the header times to wall-clock time, and writes the
// modified files to an output dir.
//
///////////////////////////////////////////////////////////////

#include "MdvSim.hh"

#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Radx/RadxPath.hh>
#include <dsserver/DsLdataInfo.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/pmu.h>

#include <algorithm>
#include <cerrno>
#include <cstdint>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits.h>
#include <set>
#include <sstream>
#include <time.h>
#include <unistd.h>

using namespace std;

//////////////////////////////////////////////////
// constructor

MdvSim::MdvSim(int argc, char **argv) :
        OK(TRUE),
        _progName("MdvSim"),
        _paramsPath((char *) "unknown"),
        _stopRequested(0),
        _procmapRegistered(false),
        _archiveStart(0),
        _archiveEnd(0)
{

  // read command line args
  
  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR - " << _progName << ": invalid command line" << endl;
    OK = FALSE;
    return;
  }

  // load parameters
  
  if (_params.loadFromArgs(argc, argv, _args.override.list, &_paramsPath)) {
    cerr << "ERROR - " << _progName << ": cannot load TDRP parameters"
         << endl;
    OK = FALSE;
  }

}

//////////////////////////////////////////////////
// destructor

MdvSim::~MdvSim()
{
  // unregister in procmap
  if (_procmapRegistered) {
    PMU_auto_unregister();
  }
}

//////////////////////////
// main run method

int MdvSim::Run()
{

  // check params and input files
  
  if (_validateParams() || _resolveInputs()) {
    return -1;
  }

  // initialize procmap registration
  
  PMU_auto_init(_progName.c_str(), _params.instance,
                PROCMAP_REGISTER_INTERVAL);
  _procmapRegistered = true;
  
  PMU_auto_register("Starting replay");

  // run the simulation
  
  return _runCycles();

}

///////////////////////////////////////////////////////////////////////
// check the parameters make sense

int MdvSim::_validateParams() const
{
  bool valid = true;
  if (!std::isfinite(_params.replay_rate) || _params.replay_rate <= 0.0) {
    cerr << "ERROR - replay_rate must be finite and greater than zero"
         << endl;
    valid = false;
  }
  if (!std::isfinite(_params.delivery_delay_secs)) {
    cerr << "ERROR - delivery_delay_secs must be finite" << endl;
    valid = false;
  }
  if (_params.repeat_replay && _params.inter_cycle_gap_secs < 1) {
    cerr << "ERROR - inter_cycle_gap_secs must be at least one when "
         << "repeat_replay is enabled" << endl;
    valid = false;
  }
  if (strlen(_params.output_dir) == 0) {
    cerr << "ERROR - output_dir may not be empty" << endl;
    valid = false;
  }
  const string output(_params.output_dir);
  if (output.find("::") != string::npos ||
      output.find("://") != string::npos) {
    cerr << "ERROR - phase one requires a local output directory: "
         << output << endl;
    valid = false;
  }
  return valid ? 0 : -1;
}

////////////////////////////////////////////////////////////////////
// get input file list

int MdvSim::_collectCandidates(vector<InputFile> &candidates) const
{
  if (_params.input_mode == Params::FILE_LIST) {
    vector<string> paths;
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
    for (size_t ii = 0; ii < paths.size(); ++ii) {
      InputFile input = {};
      input.selector = InputFile::PATH;
      input.path = _canonicalPath(paths[ii]);
      input.tieKey = input.path;
      candidates.push_back(input);
    }
    return 0;
  }

  time_t startTime = _args.startTime;
  time_t endTime = _args.endTime;
  if (!_args.startTimeSet) {
    startTime = DateTime::parseDateTime(_params.archive_start_time);
  }
  if (!_args.endTimeSet) {
    endTime = DateTime::parseDateTime(_params.archive_end_time);
  }
  if (startTime == DateTime::NEVER || endTime == DateTime::NEVER ||
      endTime < startTime) {
    cerr << "ERROR - invalid inclusive archive time range" << endl;
    return -1;
  }

  DsMdvx timeList;
  if (_params.input_data_type == Params::FORECAST) {
    timeList.setTimeListModeGenPlusForecasts(_params.input_url,
                                             startTime, endTime);
  } else {
    timeList.setTimeListModeValid(_params.input_url, startTime, endTime);
  }
  if (timeList.compileTimeList()) {
    cerr << "ERROR - cannot compile MDV archive time list: "
         << _params.input_url << endl;
    cerr << timeList.getErrStr() << endl;
    return -1;
  }

  if (_params.input_data_type == Params::FORECAST) {
    const vector<time_t> &genTimes = timeList.getGenTimes();
    const vector<vector<time_t> > &forecastTimes =
        timeList.getForecastTimesArray();
    if (genTimes.size() != forecastTimes.size()) {
      cerr << "ERROR - forecast generation and valid-time arrays differ"
           << endl;
      return -1;
    }
    for (size_t igen = 0; igen < genTimes.size(); ++igen) {
      for (size_t ival = 0; ival < forecastTimes[igen].size(); ++ival) {
        InputFile input = {};
        input.selector = InputFile::FORECAST_TIME;
        input.genTime = genTimes[igen];
        input.validTime = forecastTimes[igen][ival];
        input.leadTime = (int) (input.validTime - input.genTime);
        ostringstream key;
        key << input.genTime << ":" << input.leadTime;
        input.tieKey = key.str();
        candidates.push_back(input);
      }
    }
  } else {
    const vector<time_t> &validTimes = timeList.getTimeList();
    for (size_t ii = 0; ii < validTimes.size(); ++ii) {
      InputFile input = {};
      input.selector = InputFile::VALID_TIME;
      input.validTime = validTimes[ii];
      ostringstream key;
      key << input.validTime;
      input.tieKey = key.str();
      candidates.push_back(input);
    }
  }

  if (candidates.empty()) {
    cerr << "ERROR - no MDV inputs found in the archive range" << endl;
    return -1;
  }
  return 0;
}

//////////////////////////////////////////////////////////
// check input files are valid

int MdvSim::_resolveInputs()
{
  
  vector<InputFile> candidates;
  if (_collectCandidates(candidates)) {
    return -1;
  }

  if (RadxPath::makeDirRecurse(_params.output_dir)) {
    cerr << "ERROR - cannot create output directory: "
         << _params.output_dir << ": " << strerror(errno) << endl;
    return -1;
  }
  const string canonicalOutput = _canonicalPath(_params.output_dir);
  if (_params.input_mode == Params::ARCHIVE) {
    const string inputUrl(_params.input_url);
    if (inputUrl.find("::") == string::npos &&
        inputUrl.find("://") == string::npos) {
      const string canonicalInput = _canonicalPath(inputUrl);
      if (canonicalInput == canonicalOutput ||
          canonicalInput.find(canonicalOutput + "/") == 0 ||
          canonicalOutput.find(canonicalInput + "/") == 0) {
        cerr << "ERROR - archive input and output trees must be separate: "
             << canonicalInput << " and " << canonicalOutput << endl;
        return -1;
      }
    }
  }

  set<string> seen;
  int nErrors = 0;
  for (size_t ii = 0; ii < candidates.size(); ++ii) {
    InputFile input = candidates[ii];
    if (_inspectInput(input)) {
      ++nErrors;
      if (!_params.continue_on_input_error) {
        return -1;
      }
      continue;
    }
    if (!seen.insert(input.tieKey).second) {
      if (_params.debug) {
        cerr << "WARNING - ignoring duplicate input: "
             << input.tieKey << endl;
      }
      continue;
    }
    if (input.selector == InputFile::PATH) {
      const string inputDir =
          _canonicalPath(RadxPath(input.path).getDirectory());
      if (inputDir == canonicalOutput ||
          inputDir.find(canonicalOutput + "/") == 0 ||
          canonicalOutput.find(inputDir + "/") == 0) {
        cerr << "ERROR - input and output directories must be separate: "
             << inputDir << " and " << canonicalOutput << endl;
        return -1;
      }
    }
    _inputs.push_back(input);
  }

  if (_inputs.empty()) {
    cerr << "ERROR - no valid MDV inputs were resolved" << endl;
    return -1;
  }

  sort(_inputs.begin(), _inputs.end(), _inputLess);
  _archiveStart = _inputs.front().scheduleTime;
  _archiveEnd = _inputs.front().scheduleTime;
  for (size_t ii = 1; ii < _inputs.size(); ++ii) {
    _archiveStart = min(_archiveStart, _inputs[ii].scheduleTime);
    _archiveEnd = max(_archiveEnd, _inputs[ii].scheduleTime);
  }

  if (_params.debug) {
    cerr << "Resolved " << _inputs.size() << " MDV inputs" << endl;
    if (nErrors > 0) {
      cerr << "Skipped " << nErrors << " invalid inputs" << endl;
    }
    cerr << "Replay span: " << DateTime::str(_archiveStart)
         << " to " << DateTime::str(_archiveEnd) << endl;
  }
  return 0;
}

////////////////////////////////////////////
// check input files

int MdvSim::_inspectInput(InputFile &input) const
{

  DsMdvx mdvx;
  if (_readInput(input, mdvx)) {
    return -1;
  }

  const Mdvx::master_header_t &mhdr = mdvx.getMasterHeader();
  if (mhdr.n_fields < 1 || mdvx.getFieldByNum(0) == NULL) {
    cerr << "ERROR - MDV input contains no fields: "
         << mdvx.getPathInUse() << endl;
    return -1;
  }

  const bool headerForecast =
      mhdr.data_collection_type == Mdvx::DATA_FORECAST ||
      mhdr.data_collection_type == Mdvx::DATA_EXTRAPOLATED;
  const bool expectForecast =
      _params.input_data_type == Params::FORECAST;
  if (headerForecast != expectForecast) {
    cerr << "ERROR - input data type does not match input_data_type: "
         << mdvx.getPathInUse() << endl;
    return -1;
  }

  input.scheduleTime = expectForecast ? mhdr.time_gen : mhdr.time_centroid;
  if (input.scheduleTime <= 0) {
    cerr << "ERROR - input has no usable scheduling time: "
         << mdvx.getPathInUse() << endl;
    return -1;
  }
  input.forecastDelta = mdvx.getForecastLeadSecs();
  // NCF inputs are normally converted to the MDV in-memory representation on
  // read, so the internal format cannot identify their on-disk format.
  input.sourceFormat = Mdvx::isNcfFile(mdvx.getPathInUse()) ?
                       (int) Mdvx::FORMAT_NCF :
                       (int) Mdvx::FORMAT_MDV;
  if (_params.debug) {
    cerr << "Input format: "
         << Mdvx::format2Str((Mdvx::mdv_format_t) input.sourceFormat)
         << ", path: " << mdvx.getPathInUse() << endl;
  }
  if (_params.output_format == Params::PRESERVE_INPUT &&
      input.sourceFormat != (int) Mdvx::FORMAT_MDV &&
      input.sourceFormat != (int) Mdvx::FORMAT_NCF) {
    cerr << "ERROR - PRESERVE_INPUT supports MDV and NCF inputs only: "
         << mdvx.getPathInUse() << endl;
    return -1;
  }

  const string pathInUse = mdvx.getPathInUse();
  if (!pathInUse.empty()) {
    input.path = _canonicalPath(pathInUse);
    input.tieKey = input.path;
  }
  if (input.tieKey.empty()) {
    ostringstream key;
    key << input.scheduleTime << ":" << input.forecastDelta;
    input.tieKey = key.str();
  }

  if (expectForecast) {
    const Mdvx::field_header_t &fhdr =
        mdvx.getFieldByNum(0)->getFieldHeader();
    if (fhdr.forecast_time <= 0) {
      cerr << "ERROR - forecast input has no field forecast_time: "
           << mdvx.getPathInUse() << endl;
      return -1;
    }
    if (fhdr.forecast_delta != fhdr.forecast_time - mhdr.time_gen &&
        _params.debug) {
      cerr << "WARNING - forecast_delta differs from forecast_time - "
           << "time_gen in: " << mdvx.getPathInUse() << endl;
    }
  }
  return 0;
}

////////////////////////////////////////////////////////////////
// read MDV file

int MdvSim::_readInput(const InputFile &input, DsMdvx &mdvx) const
{
  if (_params.debug) {
    mdvx.setDebug(true);
  }
  if (input.selector == InputFile::PATH) {
    mdvx.setReadPath(input.path);
  } else if (input.selector == InputFile::FORECAST_TIME) {
    mdvx.setReadTime(Mdvx::READ_SPECIFIED_FORECAST,
                     _params.input_url, 0,
                     input.genTime, input.leadTime);
  } else {
    mdvx.setReadTime(Mdvx::READ_CLOSEST,
                     _params.input_url, 0, input.validTime);
  }

  if (mdvx.readVolume()) {
    cerr << "ERROR - cannot read MDV input: " << input.tieKey << endl;
    cerr << mdvx.getErrStr() << endl;
    return -1;
  }
  return 0;
}

///////////////////////////////////////////////////////////
// get scheduled time for file in cycle

time_t MdvSim::_mappedSchedule(const InputFile &input,
                               time_t dataCycleStart) const
{
  const double scaled =
      (input.scheduleTime - _archiveStart) / _params.replay_rate;
  return dataCycleStart + (time_t) llround(scaled);
}

//////////////////////////////////////////////////////////////////
// check next cycle

int MdvSim::_preflightCycle(time_t dataCycleStart) const
{
  set<string> outputKeys;
  for (size_t ii = 0; ii < _inputs.size(); ++ii) {
    const InputFile &input = _inputs[ii];
    const time_t mapped = _mappedSchedule(input, dataCycleStart);
    ostringstream key;
    if (_params.input_data_type == Params::FORECAST) {
      key << "F:" << mapped << ":" << input.forecastDelta;
    } else {
      key << "V:" << mapped;
    }
    if (!outputKeys.insert(key.str()).second) {
      cerr << "ERROR - replay_rate maps multiple inputs to MDV output key "
           << key.str() << endl;
      cerr << "  Reduce replay_rate to preserve one-second path uniqueness."
           << endl;
      return -1;
    }
  }
  return 0;
}

////////////////////////////////////////////////////////////
// run through file cycles, repeatedly

int MdvSim::_runCycles()
{
  time_t dataCycleStart = time(NULL) + _params.time_offset_secs;
  double deadlineCycleStart =
      _monotonicNow() + _params.delivery_delay_secs;

  do {
    if (_preflightCycle(dataCycleStart)) {
      return -1;
    }
    if (_runCycle(dataCycleStart, deadlineCycleStart)) {
      return _stopRequested ? 0 : -1;
    }
    if (!_params.repeat_replay || _stopRequested) {
      break;
    }

    const double scaledDuration =
        (_archiveEnd - _archiveStart) / _params.replay_rate;
    const time_t finalMapped = dataCycleStart +
        (time_t) llround(scaledDuration);
    dataCycleStart = finalMapped + _params.inter_cycle_gap_secs;
    const double finalDeadline = deadlineCycleStart + scaledDuration;
    deadlineCycleStart = max(finalDeadline, _monotonicNow()) +
                         _params.inter_cycle_gap_secs;
  } while (!_stopRequested);

  return 0;
}

/////////////////////////////////////////////////////////////////////
// run through one cycle of files

int MdvSim::_runCycle(time_t dataCycleStart, double deadlineCycleStart)
{
  for (size_t ii = 0; ii < _inputs.size() && !_stopRequested; ++ii) {
    const InputFile &input = _inputs[ii];
    DsMdvx mdvx;
    PMU_auto_register("Reading input product");
    if (_readInput(input, mdvx)) {
      if (_params.continue_on_input_error) {
        continue;
      }
      return -1;
    }

    const time_t mapped = _mappedSchedule(input, dataCycleStart);
    if (_retime(mdvx, mapped)) {
      return -1;
    }
    const double deadline = deadlineCycleStart +
        (input.scheduleTime - _archiveStart) / _params.replay_rate;
    if (_waitUntil(deadline, "Waiting for product time")) {
      return -1;
    }
    if (_stopRequested) {
      return 0;
    }
    if (_publish(mdvx, input)) {
      return -1;
    }
  }
  return 0;
}

////////////////////////////////////////////////////////////////
// modify times in MDV object

int MdvSim::_retime(DsMdvx &mdvx, time_t mappedSchedule) const
{
  Mdvx::master_header_t mhdr = mdvx.getMasterHeader();
  const time_t sourceSchedule =
      _params.input_data_type == Params::FORECAST ?
      mhdr.time_gen : mhdr.time_centroid;
  const int64_t offset = (int64_t) mappedSchedule - sourceSchedule;

  if (mhdr.time_gen != 0) mhdr.time_gen += offset;
  if (mhdr.time_begin != 0) mhdr.time_begin += offset;
  if (mhdr.time_end != 0) mhdr.time_end += offset;
  if (mhdr.time_centroid != 0) mhdr.time_centroid += offset;
  if (mhdr.time_expire != 0) mhdr.time_expire += offset;
  if (mhdr.forecast_time != 0) mhdr.forecast_time += offset;

  Mdvx::field_header_t firstField = {};
  bool haveFirstField = false;
  for (int ifld = 0; ifld < mhdr.n_fields; ++ifld) {
    MdvxField *field = mdvx.getFieldByNum(ifld);
    if (field == NULL) {
      cerr << "ERROR - missing field " << ifld << " during retime" << endl;
      return -1;
    }
    Mdvx::field_header_t fhdr = field->getFieldHeader();
    if (fhdr.forecast_time != 0) {
      fhdr.forecast_time += offset;
    }
    field->setFieldHeader(fhdr);
    if (!haveFirstField) {
      firstField = fhdr;
      haveFirstField = true;
    }
  }

  // Mdvx caches the first field's forecast values in the master header.
  // Update both copies so updateMasterHeader() cannot retain stale input time.
  if (haveFirstField && firstField.forecast_time != 0) {
    mhdr.forecast_time = firstField.forecast_time;
    mhdr.forecast_delta = firstField.forecast_delta;
  }
  mdvx.setMasterHeader(mhdr);
  return 0;
}

////////////////////////////////////////////////////////////
// wait before writing next file

int MdvSim::_waitUntil(double deadline, const string &activity)
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
      // Resume only the unslept portion; the outer loop owns the deadline.
    }
  }
  return -1;
}

////////////////////////////////////////////////////////////////
// write the files

int MdvSim::_publish(DsMdvx &mdvx, const InputFile &input)
{
  PMU_auto_register("Writing output product");
  mdvx.setAppName(_progName);
  mdvx.clearWriteLdataInfo();
  if (_params.input_data_type == Params::FORECAST) {
    mdvx.setWriteAsForecast();
  } else {
    mdvx.clearWriteAsForecast();
  }
  if (_params.write_using_extended_paths) {
    mdvx.setWriteUsingExtendedPath();
  } else {
    mdvx.clearWriteUsingExtendedPath();
  }
  if (_params.append_year_dir_to_output_dir) {
    mdvx.setWriteAddYearSubdir();
  } else {
    mdvx.clearWriteAddYearSubdir();
  }

  Mdvx::mdv_format_t outputFormat =
      (Mdvx::mdv_format_t) input.sourceFormat;
  if (_params.output_format == Params::OUTPUT_MDV) {
    outputFormat = Mdvx::FORMAT_MDV;
  } else if (_params.output_format == Params::OUTPUT_NCF) {
    outputFormat = Mdvx::FORMAT_NCF;
  }
  mdvx.setWriteFormat(outputFormat);

  // Mdvx consults MDV_WRITE_FORMAT immediately before every local write and
  // otherwise resets the format to NCF. MdvSim is deliberately single
  // threaded, so setting this process-wide directive per product safely
  // supports PRESERVE_INPUT even when a file list contains both formats.

  const char *formatEnv = outputFormat == Mdvx::FORMAT_MDV ?
                          "FORMAT_MDV" : "FORMAT_NCF";
  if (setenv("MDV_WRITE_FORMAT", formatEnv, 1)) {
    cerr << "ERROR - cannot select output format: " << strerror(errno)
         << endl;
    return -1;
  }

  if (_params.debug) {
    cerr << "Output format: " << Mdvx::format2Str(mdvx.getWriteFormat())
         << ", uncompressed MDV bytes: " << mdvx.getWriteLen32()
         << endl;
  }

  if (mdvx.writeToDir(_params.output_dir)) {
    cerr << "ERROR - cannot publish MDV product to: "
         << _params.output_dir << endl;
    cerr << mdvx.getErrStr() << endl;
    return -1;
  }

  // writeToDir() has atomically renamed the data file. Publish DsLdataInfo
  // afterward so DataMapper and readers only observe committed products.
  const string outputPath = mdvx.getPathInUse();
  string relativePath;
  RadxPath::stripDir(_params.output_dir, outputPath, relativePath);
  RadxPath path(outputPath);
  DsLdataInfo latest(_params.output_dir);
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    latest.setDebug(true);
  }
  latest.setRelDataPath(relativePath);
  latest.setDataFileExt(path.getExt());
  latest.setDataType(path.getExt());
  latest.setWriter(_progName);

  const Mdvx::master_header_t &mhdr = mdvx.getMasterHeader();
  time_t latestTime = mhdr.time_centroid;
  if (_params.input_data_type == Params::FORECAST) {
    latest.setIsFcast();
    latest.setLeadTime(mdvx.getForecastLeadSecs());
    latestTime = mhdr.time_gen;
  }
  if (latest.write(latestTime)) {
    cerr << "ERROR - data file was published, but _latest_data_info failed: "
         << outputPath << endl;
    return -1;
  }

  if (_params.debug) {
    cerr << "Published: " << outputPath << endl;
  }
  return 0;
}

///////////////////////////////////////////////////////////////
// get monotonic clock time

double MdvSim::_monotonicNow()
{
  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC, &now);
  return now.tv_sec + now.tv_nsec / 1.0e9;
}

///////////////////////////////////////////////////////////
// resolve the path, free of links etc

string MdvSim::_canonicalPath(const string &path)
{
  char resolved[PATH_MAX];
  if (realpath(path.c_str(), resolved) != NULL) {
    return resolved;
  }
  return path;
}

//////////////////////////////////////////////////////////////
// comparison for file path order

bool MdvSim::_inputLess(const InputFile &lhs, const InputFile &rhs)
{
  if (lhs.scheduleTime < rhs.scheduleTime) {
    return true;
  }
  if (lhs.scheduleTime > rhs.scheduleTime) {
    return false;
  }
  if (lhs.forecastDelta < rhs.forecastDelta) {
    return true;
  }
  if (lhs.forecastDelta > rhs.forecastDelta) {
    return false;
  }
  return lhs.tieKey < rhs.tieKey;
}
