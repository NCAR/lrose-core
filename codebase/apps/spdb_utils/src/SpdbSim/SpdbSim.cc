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
//
// SpdbSim.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 2026
//
///////////////////////////////////////////////////////////////
//
// SpdbSim is intended to simulate Spdb-type data in a realtime
// scenario.
//
// SpdbSim loops through a set of specified times in Spdb data,
// modifies the times to wall-clock time, and writes the
// modified chunks to an spdb output dir.
//
///////////////////////////////////////////////////////////////

#include "SpdbSim.hh"
#include "PayloadRetimer.hh"

#include <toolsa/DateTime.hh>
#include <toolsa/pmu.h>

#include <algorithm>
#include <cerrno>
#include <climits>
#include <cmath>
#include <cstring>
#include <iostream>
#include <sstream>
#include <time.h>

using namespace std;

//////////////////////////////////////////////////////////////////////
// constructor

SpdbSim::SpdbSim(int argc, char **argv) :
        OK(TRUE),
        _progName("SpdbSim"),
        _paramsPath((char *) "unknown"),
        _stopRequested(0),
        _procmapRegistered(false),
        _productId(0),
        _archiveStart(0),
        _archiveEnd(0)
{

  // parse command line args
  
  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR - " << _progName << ": invalid command line" << endl;
    OK = FALSE;
    return;
  }

  // load parameter file
  
  if (_params.loadFromArgs(argc, argv, _args.override.list, &_paramsPath)) {
    cerr << "ERROR - " << _progName << ": cannot load TDRP parameters"
         << endl;
    OK = FALSE;
  }
}

//////////////////////////////////////////////////////////////////////
// destructor

SpdbSim::~SpdbSim()
{
  if (_procmapRegistered) {
    PMU_auto_unregister();
  }
}

/////////////////////////////////////////////////////////////////////
// main run method

int SpdbSim::Run()
{
  if (_validateParams() || _readAndPreflight()) {
    return -1;
  }

  PMU_auto_init(_progName.c_str(), _params.instance,
                PROCMAP_REGISTER_INTERVAL);
  _procmapRegistered = true;
  PMU_auto_register("Starting SPDB replay");
  return _runCycles();
}

////////////////////////////////////////////////////////////////////////
// check the parameter states

int SpdbSim::_validateParams() const
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
  if (strlen(_params.input_url) == 0 || strlen(_params.output_url) == 0) {
    cerr << "ERROR - input_url and output_url may not be empty" << endl;
    valid = false;
  }
  if (!strcmp(_params.input_url, _params.output_url)) {
    cerr << "ERROR - input_url and output_url must differ" << endl;
    valid = false;
  }
  return valid ? 0 : -1;
}

//////////////////////////////////////////////////////////////////////////
// pre-check input data

int SpdbSim::_readAndPreflight()
{
  time_t startTime = _args.startTimeSet ?
                     _args.startTime :
                     DateTime::parseDateTime(_params.archive_start_time);
  time_t endTime = _args.endTimeSet ?
                   _args.endTime :
                   DateTime::parseDateTime(_params.archive_end_time);
  if (startTime == DateTime::NEVER || endTime == DateTime::NEVER ||
      endTime < startTime) {
    cerr << "ERROR - invalid inclusive archive time range" << endl;
    return -1;
  }

  if (_params.debug) {
    _input.setDebug(true);
  }
  if (_input.getInterval(_params.input_url, startTime, endTime,
                         _params.data_type, _params.data_type2,
                         false, _params.respect_zero_types)) {
    cerr << "ERROR - cannot read SPDB interval from: "
         << _params.input_url << endl;
    cerr << _input.getErrStr() << endl;
    return -1;
  }

  _productId = _input.getProdId();
  _productLabel = _input.getProdLabel();
  const vector<Spdb::chunk_t> &inputChunks = _input.getChunks();
  int nErrors = 0;
  for (size_t ii = 0; ii < inputChunks.size(); ++ii) {
    if (_preflightChunk(inputChunks[ii])) {
      ++nErrors;
      if (!_params.continue_on_chunk_error) {
        return -1;
      }
      continue;
    }
    ReplayChunk replay;
    replay.chunk = inputChunks[ii];
    _chunks.push_back(replay);
  }
  if (_chunks.empty()) {
    cerr << "ERROR - no usable SPDB chunks were found in the interval"
         << endl;
    return -1;
  }

  stable_sort(_chunks.begin(), _chunks.end(), _chunkLess);
  _archiveStart = _chunks.front().chunk.valid_time;
  _archiveEnd = _chunks.back().chunk.valid_time;

  if (_params.debug) {
    cerr << "SPDB product: " << _productId << " ("
         << _productLabel << ")" << endl;
    cerr << "Resolved " << _chunks.size() << " chunks from "
         << DateTime::str(_archiveStart) << " to "
         << DateTime::str(_archiveEnd) << endl;
    if (nErrors > 0) {
      cerr << "Skipped " << nErrors << " malformed chunks" << endl;
    }
  }
  return 0;
}

int SpdbSim::_preflightChunk(const Spdb::chunk_t &chunk) const
{
  if (!_fitsSpdbTime(chunk.valid_time) ||
      (chunk.expire_time != 0 && !_fitsSpdbTime(chunk.expire_time))) {
    cerr << "ERROR - source chunk time is outside the SPDB 32-bit range"
         << endl;
    return -1;
  }
  if (chunk.len <= 0 || chunk.data == NULL) {
    cerr << "ERROR - source chunk has an empty payload at "
         << DateTime::str(chunk.valid_time) << endl;
    return -1;
  }

  if (!PayloadRetimer::supports(_productId, chunk.data_type2)) {
    if (_params.unsupported_payload_policy == Params::COPY_UNSUPPORTED) {
      return 0;
    }
    cerr << "ERROR - no payload retimer for SPDB product ID "
         << _productId << endl;
    return -1;
  }

  vector<unsigned char> output;
  string error;
  if (PayloadRetimer::retime(_productId, chunk.data_type2,
                             chunk.data, chunk.len, 0,
                             output, error)) {
    cerr << "ERROR - invalid SPDB payload at "
         << DateTime::str(chunk.valid_time) << ": " << error << endl;
    return -1;
  }
  return 0;
}

int SpdbSim::_preflightCycle(time_t dataCycleStart) const
{
  for (size_t ii = 0; ii < _chunks.size(); ++ii) {
    const Spdb::chunk_t &chunk = _chunks[ii].chunk;
    const time_t mapped = _mappedValid(chunk, dataCycleStart);
    const int64_t offset = (int64_t) mapped - chunk.valid_time;
    const int64_t mappedExpire = chunk.expire_time == 0 ? 0 :
                                 (int64_t) chunk.expire_time + offset;
    if (!_fitsSpdbTime(mapped) ||
        (mappedExpire != 0 && !_fitsSpdbTime(mappedExpire))) {
      cerr << "ERROR - mapped chunk time exceeds the SPDB 32-bit range"
           << endl;
      return -1;
    }
    if (PayloadRetimer::supports(_productId, chunk.data_type2)) {
      vector<unsigned char> output;
      string error;
      if (PayloadRetimer::retime(_productId, chunk.data_type2,
                                 chunk.data, chunk.len, offset,
                                 output, error)) {
        if (_params.continue_on_chunk_error) {
          cerr << "WARNING - mapped payload cannot be retimed: "
               << error << endl;
          continue;
        }
        cerr << "ERROR - mapped payload cannot be retimed: "
             << error << endl;
        return -1;
      }
    }
  }
  return 0;
}

///////////////////////////////////////////////////////////////////////////
// run the repeated simulation through the input data

int SpdbSim::_runCycles()
{
  // Data time follows the wall clock, while waits use a monotonic clock so a
  // system-clock correction cannot stretch or collapse the replay schedule.
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

///////////////////////////////////////////////////////////////////////////
// single run through the input data

int SpdbSim::_runCycle(time_t dataCycleStart, double deadlineCycleStart)
{
  size_t begin = 0;
  while (begin < _chunks.size() && !_stopRequested) {
    size_t end = begin + 1;
    const time_t sourceTime = _chunks[begin].chunk.valid_time;
    while (end < _chunks.size() &&
           _chunks[end].chunk.valid_time == sourceTime) {
      ++end;
    }

    const double deadline = deadlineCycleStart +
        (sourceTime - _archiveStart) / _params.replay_rate;
    if (_waitUntil(deadline, "Waiting for SPDB chunk time")) {
      return -1;
    }
    if (_stopRequested) {
      return 0;
    }
    if (_publishGroup(begin, end, dataCycleStart)) {
      return -1;
    }
    begin = end;
  }
  return 0;
}

//////////////////////////////////////////
// write out modified data

int SpdbSim::_publishGroup(size_t begin, size_t end,
                           time_t dataCycleStart)
{
  // Chunks sharing a source valid time are submitted in one put.  Consumers
  // therefore see the same grouping and latest-data update as in the archive.
  DsSpdb output;
  output.setAppName(_progName);
  output.setPutMode(Spdb::putModeAdd);
  if (_params.debug) {
    output.setDebug(true);
  }
  if (_params.lead_time_storage == Params::LEAD_TIME_DATA_TYPE) {
    output.setLeadTimeStorage(Spdb::LEAD_TIME_IN_DATA_TYPE);
  } else if (_params.lead_time_storage == Params::LEAD_TIME_DATA_TYPE2) {
    output.setLeadTimeStorage(Spdb::LEAD_TIME_IN_DATA_TYPE2);
  }
  output.clearPutChunks();

  int nAdded = 0;
  for (size_t ii = begin; ii < end; ++ii) {
    const Spdb::chunk_t &chunk = _chunks[ii].chunk;
    const time_t mapped = _mappedValid(chunk, dataCycleStart);
    const int64_t offset = (int64_t) mapped - chunk.valid_time;
    const time_t mappedExpire = chunk.expire_time == 0 ? 0 :
        (time_t) ((int64_t) chunk.expire_time + offset);

    vector<unsigned char> payload;
    string error;
    if (PayloadRetimer::supports(_productId, chunk.data_type2)) {
      if (PayloadRetimer::retime(_productId, chunk.data_type2,
                                 chunk.data, chunk.len, offset,
                                 payload, error)) {
        cerr << "ERROR - cannot retime SPDB payload: " << error << endl;
        if (_params.continue_on_chunk_error) {
          continue;
        }
        return -1;
      }
    } else {
      const unsigned char *bytes =
          (const unsigned char *) chunk.data;
      payload.assign(bytes, bytes + chunk.len);
    }

    // The current SPDB wire API takes signed 32-bit payload lengths.  Reject
    // oversized reassembled products instead of truncating the size at put.
    if (payload.size() > INT_MAX) {
      cerr << "ERROR - retimed SPDB payload exceeds the 32-bit size limit"
           << endl;
      if (_params.continue_on_chunk_error) {
        continue;
      }
      return -1;
    }

    output.setChunkCompressOnPut(_outputCompression(chunk));
    const char *tag = chunk.tag.empty() ? NULL : chunk.tag.c_str();
    output.addPutChunk(chunk.data_type, mapped, mappedExpire,
                       (int) payload.size(), payload.data(),
                       chunk.data_type2, tag);
    ++nAdded;
  }

  if (nAdded == 0) {
    return 0;
  }
  PMU_auto_register("Publishing SPDB chunks");
  if (output.put(_params.output_url, _productId, _productLabel)) {
    cerr << "ERROR - cannot publish SPDB chunks to: "
         << _params.output_url << endl;
    cerr << output.getErrStr() << endl;
    return -1;
  }
  if (_params.debug) {
    cerr << "Published " << nAdded << " chunks at "
         << DateTime::str(_mappedValid(_chunks[begin].chunk,
                                      dataCycleStart)) << endl;
  }
  return 0;
}

////////////////////////////////////////////////////////////////////////
// wait

int SpdbSim::_waitUntil(double deadline, const string &activity)
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

/////////////////////////////////////////////////////////////////////
// check input

time_t SpdbSim::_mappedValid(const Spdb::chunk_t &chunk,
                             time_t dataCycleStart) const
{
  const double scaled =
      (chunk.valid_time - _archiveStart) / _params.replay_rate;
  return dataCycleStart + (time_t) llround(scaled);
}

/////////////////////////////////////////////////////////////////////
// set compression

Spdb::compression_t SpdbSim::_outputCompression(
    const Spdb::chunk_t &chunk) const
{
  if (_params.output_compression == Params::GZIP_COMPRESSION) {
    return Spdb::COMPRESSION_GZIP;
  }
  if (_params.output_compression == Params::BZIP2_COMPRESSION) {
    return Spdb::COMPRESSION_BZIP2;
  }
  if (_params.output_compression == Params::NO_COMPRESSION) {
    return Spdb::COMPRESSION_NONE;
  }
  if (chunk.stored_compression == Spdb::COMPRESSION_GZIP ||
      chunk.stored_compression == Spdb::COMPRESSION_BZIP2) {
    return chunk.stored_compression;
  }
  return Spdb::COMPRESSION_NONE;
}

/////////////////////////////////////////////////////////////////////
// sorting

bool SpdbSim::_chunkLess(const ReplayChunk &lhs, const ReplayChunk &rhs)
{
  if (lhs.chunk.valid_time != rhs.chunk.valid_time) {
    return lhs.chunk.valid_time < rhs.chunk.valid_time;
  }
  if (lhs.chunk.data_type != rhs.chunk.data_type) {
    return lhs.chunk.data_type < rhs.chunk.data_type;
  }
  return lhs.chunk.data_type2 < rhs.chunk.data_type2;
}

bool SpdbSim::_fitsSpdbTime(int64_t value)
{
  return value >= 0 && value <= INT32_MAX;
}

/////////////////////////////////////////////////////////////////
// wall time monotic

double SpdbSim::_monotonicNow()
{
  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC, &now);
  return now.tv_sec + now.tv_nsec / 1.0e9;
}

