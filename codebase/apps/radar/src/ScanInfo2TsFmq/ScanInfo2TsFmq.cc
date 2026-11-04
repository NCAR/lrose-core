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
// ScanInfo2TsFmq.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 2026
//
///////////////////////////////////////////////////////////////
//
// ScanInfo2Fmq reads scan information via UDP.
// It then writes the appropriate packets to a time series FMQ.
//
////////////////////////////////////////////////////////////////

#include "ScanInfo2TsFmq.hh"

#include <arpa/inet.h>
#include <cerrno>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
#include <map>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

#include <radar/iwrf_functions.hh>
#include <toolsa/MemBuf.hh>
#include <toolsa/pmu.h>

using namespace std;

// Constructor

ScanInfo2TsFmq::ScanInfo2TsFmq(int argc, char **argv) :
        isOK(true),
        _paramsPath((char *) "unknown"),
        _udpFd(-1),
        _packetSeqNum(0),
        _pulseSeqNum(0),
        _lastReceiveTime(0),
        _staleEventWritten(false),
        _haveScanState(false),
        _outputVolumeNum(-1),
        _outputSweepNum(-1)
{
  _progName = "ScanInfo2TsFmq";

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR - " << _progName << ": invalid command line arguments"
         << endl;
    isOK = false;
    return;
  }

  if (_params.loadFromArgs(argc, argv, _args.override.list, &_paramsPath)) {
    cerr << "ERROR - " << _progName << ": cannot load TDRP parameters"
         << endl;
    isOK = false;
    return;
  }

  if (_openOutputFmq() || _openUdpSocket()) {
    isOK = false;
    return;
  }

  if (_params.register_with_procmap) {
    PMU_auto_init((char *) _progName.c_str(), _params.instance,
                  PROCMAP_REGISTER_INTERVAL);
  }
}

// Destructor

ScanInfo2TsFmq::~ScanInfo2TsFmq()
{

  if (_udpFd >= 0) {
    close(_udpFd);
    _udpFd = -1;
  }

  _outputFmq.closeMsgQueue();
  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int ScanInfo2TsFmq::Run()
{

  if (_params.debug) {
    cerr << "Listening for scan telemetry on " << _params.udp_bind_address
         << ":" << _params.udp_ascii_port << endl;
    cerr << "Writing IWRF scan information to "
         << _params.output_fmq_path << endl;
  }

  while (true) {

    PMU_auto_register("Waiting for UDP scan telemetry");

    string datagram;
    bool timedOut = false;

    if (_readDatagram(datagram, timedOut)) {
      cerr << "ERROR - " << _progName << ": UDP receive failed: "
           << strerror(errno) << endl;
      return -1;
    }

    if (timedOut) {
      if (_params.write_end_of_vol_when_data_stops &&
          !_staleEventWritten && _lastReceiveTime > 0 &&
          time(NULL) - _lastReceiveTime >=
              _params.nsecs_no_data_for_end_of_vol) {
        if (_writeEndOfVol()) {
          return -1;
        }
        _staleEventWritten = true;
      }
      continue;
    }

    if (datagram.empty()) {
      continue;
    }

    ScanTelemetry telemetry;
    string error;
    if (!_decodeTelemetry(datagram, telemetry, error) ||
        !_validateTelemetry(telemetry, error)) {
      cerr << "WARNING - " << _progName
           << ": ignoring malformed telemetry: " << error << endl;
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "  datagram: " << datagram << endl;
      }
      continue;
    }

    // Time interpolation in TsScanInfoMerge requires a strictly ordered
    // master stream. UDP duplicates or reordering must not move it backward.

    if (_prevTelemetryTime.isValid() &&
        telemetry.time - _prevTelemetryTime <= 0.0) {
      if (_params.debug) {
        cerr << "WARNING - " << _progName
             << ": dropping non-increasing telemetry time "
             << telemetry.time.asString(9) << ", previous time "
             << _prevTelemetryTime.asString(9) << endl;
      }
      continue;
    }

    _prevTelemetryTime = telemetry.time;
    _lastReceiveTime = time(NULL);
    _staleEventWritten = false;

    if (_processTelemetry(telemetry)) {
      return -1;
    }

  } // while (true)

  return 0;
}

///////////////////////////////////////
// Open and bind the UDP input socket

int ScanInfo2TsFmq::_openUdpSocket()
{

  if (_params.udp_ascii_port < 1 || _params.udp_ascii_port > 65535) {
    cerr << "ERROR - " << _progName << ": invalid UDP port "
         << _params.udp_ascii_port << endl;
    return -1;
  }

  struct sockaddr_in address;
  memset(&address, 0, sizeof(address));
  address.sin_family = AF_INET;
  address.sin_port = htons(static_cast<unsigned short>(_params.udp_ascii_port));
  if (inet_pton(AF_INET, _params.udp_bind_address, &address.sin_addr) != 1) {
    cerr << "ERROR - " << _progName << ": invalid IPv4 bind address "
         << _params.udp_bind_address << endl;
    return -1;
  }

  _udpFd = socket(AF_INET, SOCK_DGRAM, 0);
  if (_udpFd < 0) {
    cerr << "ERROR - " << _progName << ": cannot create UDP socket: "
         << strerror(errno) << endl;
    return -1;
  }

  const int reuseAddress = 1;
  if (setsockopt(_udpFd, SOL_SOCKET, SO_REUSEADDR, &reuseAddress,
                 sizeof(reuseAddress)) != 0) {
    cerr << "ERROR - " << _progName << ": cannot set SO_REUSEADDR: "
         << strerror(errno) << endl;
    return -1;
  }

  if (bind(_udpFd, reinterpret_cast<struct sockaddr *>(&address),
           sizeof(address)) != 0) {
    cerr << "ERROR - " << _progName << ": cannot bind "
         << _params.udp_bind_address << ":" << _params.udp_ascii_port
         << ": " << strerror(errno) << endl;
    return -1;
  }

  return 0;
}

/////////////////////////////////////////////////////////
// Read one UDP datagram, waiting for up to one second

int ScanInfo2TsFmq::_readDatagram(string &datagram, bool &timedOut)
{

  static const size_t maxDatagramBytes = 4096;

  timedOut = false;

  fd_set readSet;
  FD_ZERO(&readSet);
  FD_SET(_udpFd, &readSet);
  struct timeval timeout;
  timeout.tv_sec = 1;
  timeout.tv_usec = 0;

  const int ready = select(_udpFd + 1, &readSet, NULL, NULL, &timeout);
  if (ready < 0) {
    if (errno == EINTR) {
      timedOut = true;
      return 0;
    }
    return -1;
  }
  if (ready == 0) {
    timedOut = true;
    return 0;
  }

  char buffer[maxDatagramBytes];
  const ssize_t received = recvfrom(_udpFd, buffer, sizeof(buffer),
                                    MSG_TRUNC, NULL, NULL);
  if (received < 0) {
    if (errno == EINTR) {
      timedOut = true;
      return 0;
    }
    return -1;
  }
  if (static_cast<size_t>(received) > sizeof(buffer)) {
    cerr << "WARNING - " << _progName << ": discarding UDP datagram larger "
         << "than " << sizeof(buffer) << " bytes" << endl;
    datagram.clear();
    return 0;
  }

  datagram.assign(buffer, static_cast<size_t>(received));

  return 0;

}

////////////////////////////////////////////////////
// Decode a comma-delimited ASCII telemetry record

bool ScanInfo2TsFmq::_decodeTelemetry(const string &datagram,
                                      ScanTelemetry &telemetry,
                                      string &error) const
{

  map<string, string> fields;
  string::size_type begin = 0;
  while (begin <= datagram.size()) {
    const string::size_type comma = datagram.find(',', begin);
    const string part = _trim(datagram.substr(
        begin, comma == string::npos ? string::npos : comma - begin));
    const string::size_type equals = part.find('=');
    if (equals == string::npos) {
      error = "field lacks '='";
      return false;
    }
    const string key = _trim(part.substr(0, equals));
    const string value = _trim(part.substr(equals + 1));
    if (key.empty() || fields.find(key) != fields.end()) {
      error = key.empty() ? "empty field name" : "duplicate field '" + key + "'";
      return false;
    }
    fields[key] = value;
    if (comma == string::npos) {
      break;
    }
    begin = comma + 1;
  }

  const char *required[] = {
    "t", "az", "el", "type", "vol", "sweep", "fixed", "tr"
  };
  for (size_t ii = 0; ii < sizeof(required) / sizeof(required[0]); ++ii) {
    if (fields.find(required[ii]) == fields.end()) {
      error = string("missing field '") + required[ii] + "'";
      return false;
    }
  }

  int transition = 0;
  if (!_parseTime(fields["t"], telemetry.time, error)) {
    return false;
  }
  if (!_parseDouble(fields["az"], telemetry.azimuth) ||
      !_parseDouble(fields["el"], telemetry.elevation) ||
      !_parseInt(fields["type"], telemetry.scanType) ||
      !_parseInt(fields["vol"], telemetry.volumeNum) ||
      !_parseInt(fields["sweep"], telemetry.sweepNum) ||
      !_parseDouble(fields["fixed"], telemetry.fixedAngle) ||
      !_parseInt(fields["tr"], transition)) {
    error = "one or more fields have invalid numeric values";
    return false;
  }
  if (transition != 0 && transition != 1) {
    error = "transition field must be 0 or 1";
    return false;
  }

  telemetry.inTransition = transition != 0;
  telemetry.scanName = fields.find("scan") == fields.end() ?
    string() : fields["scan"];

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Telemetry entry:" << endl;
    cerr << "  time: " << telemetry.time.asString(3) << endl;
    cerr << "  el, az: " << telemetry.elevation << ", " << telemetry.azimuth << endl;
    cerr << "  scanType: " << telemetry.scanType << endl;
    cerr << "  volumeNum: " << telemetry.volumeNum << endl;
    cerr << "  sweepNum: " << telemetry.sweepNum << endl;
    cerr << "  fixedAngle: " << telemetry.fixedAngle << endl;
    cerr << "  inTransition: " << (telemetry.inTransition?"Y":"N") << endl;
    cerr << "  scanName: " << telemetry.scanName << endl;
  }

  return true;
  
}

///////////////////////////////////////////////
// Check decoded values against wire and IWRF constraints

bool ScanInfo2TsFmq::_validateTelemetry(const ScanTelemetry &telemetry,
                                        string &error) const
{
  if (!telemetry.time.isValid() || telemetry.time.utime() < 0) {
    error = "timestamp is invalid or negative";
    return false;
  }
  if (telemetry.scanName.size() >= IWRF_MAX_SEGMENT_NAME) {
    error = "scan name exceeds the IWRF segment-name limit";
    return false;
  }
  if (telemetry.scanName != "idle" &&
      (telemetry.scanType < 1 || telemetry.scanType > 7)) {
    error = "unknown scan type";
    return false;
  }
  return true;
}

////////////////////////////
// Trim surrounding white space

string ScanInfo2TsFmq::_trim(const string &input) const
{
  const string whitespace(" \t\r\n");
  const string::size_type first = input.find_first_not_of(whitespace);
  if (first == string::npos) {
    return string();
  }

  const string::size_type last = input.find_last_not_of(whitespace);
  return input.substr(first, last - first + 1);
}

/////////////////////////////////////////////////////////
// Parse epoch seconds without converting the complete
// timestamp through a double. The fractional field is
// retained to the nanosecond precision supported by IWRF.

bool ScanInfo2TsFmq::_parseTime(const string &text, DateTime &value,
                               string &error) const
{
  const string::size_type decimalPos = text.find('.');
  const string secondsText = text.substr(0, decimalPos);
  if (secondsText.empty()) {
    error = "timestamp has no integer seconds";
    return false;
  }

  char *end = NULL;
  errno = 0;
  const long long parsedSeconds = strtoll(secondsText.c_str(), &end, 10);
  if (errno != 0 || end == secondsText.c_str() || *end != '\0') {
    error = "timestamp has invalid integer seconds";
    return false;
  }

  const time_t seconds = static_cast<time_t>(parsedSeconds);
  if (static_cast<long long>(seconds) != parsedSeconds) {
    error = "timestamp integer seconds are outside the time_t range";
    return false;
  }

  int nanoSecs = 0;
  if (decimalPos != string::npos) {
    const string fractionText = text.substr(decimalPos + 1);
    if (fractionText.empty() || fractionText.size() > 9) {
      error = "timestamp fraction must contain 1 to 9 digits";
      return false;
    }

    for (size_t ii = 0; ii < fractionText.size(); ++ii) {
      if (fractionText[ii] < '0' || fractionText[ii] > '9') {
        error = "timestamp fraction contains a non-digit";
        return false;
      }
      nanoSecs = nanoSecs * 10 + (fractionText[ii] - '0');
    }
    for (size_t ii = fractionText.size(); ii < 9; ++ii) {
      nanoSecs *= 10;
    }
  }

  value.set(seconds, nanoSecs * 1.0e-9);
  return true;
}

////////////////////////////////////
// Parse a finite double-precision value

bool ScanInfo2TsFmq::_parseDouble(const string &text, double &value) const
{
  char *end = NULL;
  errno = 0;
  value = strtod(text.c_str(), &end);
  return errno == 0 && end != text.c_str() && *end == '\0' &&
         std::isfinite(value);
}

////////////////////////////////////
// Parse a value in the range of int

bool ScanInfo2TsFmq::_parseInt(const string &text, int &value) const
{
  char *end = NULL;
  errno = 0;
  const long parsed = strtol(text.c_str(), &end, 10);
  if (errno != 0 || end == text.c_str() || *end != '\0' ||
      parsed < numeric_limits<int>::min() ||
      parsed > numeric_limits<int>::max()) {
    return false;
  }

  value = static_cast<int>(parsed);
  return true;
}

//////////////////////////////////////////
// Copy a string into a fixed IWRF field

void ScanInfo2TsFmq::_copyString(char *dest, size_t destLen,
                                 const string &source) const
{
  if (destLen == 0) {
    return;
  }

  strncpy(dest, source.c_str(), destLen - 1);
  dest[destLen - 1] = '\0';
}

////////////////////////////////////////////////////
// Convert one telemetry record into IWRF output

int ScanInfo2TsFmq::_processTelemetry(ScanTelemetry telemetry)
{

  static const double metadataIntervalSecs = 1.0;

  const bool firstSample = !_haveScanState;
  const bool scanChanged = _scanStateChanged(telemetry);
  const bool metadataDue = firstSample ||
    telemetry.time - _lastMetadataTime >= metadataIntervalSecs;

  // Start metadata and changed scan state in a fresh FMQ message. A reader
  // joining at the tail must never encounter a pulse before its bootstrap
  // metadata, even when an earlier pulse batch is only partially filled.
  if (!firstSample && (metadataDue || scanChanged)) {
    if (_writeToOutputFmq(true)) {
      return -1;
    }
  }

  if (metadataDue) {
    _addReaderMetadata(telemetry);
    _lastMetadataTime = telemetry.time;
  } else if (scanChanged) {
    _addScanSegment(telemetry);
  }

  _addPulse(telemetry);
  _prevScanState = telemetry;
  _haveScanState = true;
  _outputVolumeNum = telemetry.volumeNum;
  _outputSweepNum = telemetry.sweepNum;

  if (metadataDue || scanChanged) {
    return _writeToOutputFmq(true);
  }
  return _writeToOutputFmq();
}

////////////////////////////////////////////
// Map the controller scan type to IWRF

int ScanInfo2TsFmq::_scanMode(const ScanTelemetry &telemetry) const
{
  if (telemetry.scanName == "idle" || telemetry.fixedAngle == 400.0) {
    return IWRF_SCAN_MODE_IDLE;
  }
  if (telemetry.scanType == 1) {
    return IWRF_SCAN_MODE_AZ_SUR_360;
  }
  if (telemetry.scanType == 2 || telemetry.scanType == 3 ||
      telemetry.scanType == 6) {
    return IWRF_SCAN_MODE_SECTOR;
  }
  if (telemetry.scanType == 4 || telemetry.scanType == 5 ||
      telemetry.scanType == 7) {
    return IWRF_SCAN_MODE_RHI;
  }
  return IWRF_SCAN_MODE_NOT_SET;
}

/////////////////////////////////////////////////////
// Check whether a new scan segment should be emitted

bool ScanInfo2TsFmq::_scanStateChanged(
    const ScanTelemetry &telemetry) const
{
  if (!_haveScanState) {
    return true;
  }
  return _scanMode(telemetry) != _scanMode(_prevScanState) ||
         telemetry.volumeNum != _prevScanState.volumeNum ||
         telemetry.sweepNum != _prevScanState.sweepNum ||
         telemetry.fixedAngle != _prevScanState.fixedAngle ||
         telemetry.scanName != _prevScanState.scanName;
}

////////////////////////////////////////////////////
// Add the complete metadata set required before a reader can return pulses

void ScanInfo2TsFmq::_addReaderMetadata(const ScanTelemetry &telemetry)
{
  iwrf_radar_info_t radarInfo;
  iwrf_radar_info_init(radarInfo);
  _setPacketTimeAndSequence(radarInfo.packet, telemetry.time);
  _copyString(radarInfo.radar_name, sizeof(radarInfo.radar_name),
              "scan-telemetry");
  _outputMsg.addPart(IWRF_RADAR_INFO_ID, sizeof(radarInfo), &radarInfo);

  iwrf_ts_processing_t processing;
  iwrf_ts_processing_init(processing);
  _setPacketTimeAndSequence(processing.packet, telemetry.time);
  _outputMsg.addPart(IWRF_TS_PROCESSING_ID, sizeof(processing), &processing);

  _addScanSegment(telemetry);
}

////////////////////////////////////////
// Add the current IWRF scan segment

void ScanInfo2TsFmq::_addScanSegment(const ScanTelemetry &telemetry)
{
  iwrf_scan_segment_t segment;
  iwrf_scan_segment_init(segment);
  _setPacketTimeAndSequence(segment.packet, telemetry.time);
  segment.scan_mode = _scanMode(telemetry);
  segment.volume_num = telemetry.volumeNum;
  segment.sweep_num = telemetry.sweepNum;
  segment.current_fixed_angle = telemetry.fixedAngle;
  _copyString(segment.segment_name, sizeof(segment.segment_name),
              telemetry.scanName);
  _outputMsg.addPart(IWRF_SCAN_SEGMENT_ID, sizeof(segment), &segment);
}

///////////////////////////////////////////////////////
// Add a synthetic pulse carrying angles and scan state

void ScanInfo2TsFmq::_addPulse(const ScanTelemetry &telemetry)
{
  iwrf_pulse_header_t pulse;
  iwrf_pulse_header_init(pulse);
  _setPacketTimeAndSequence(pulse.packet, telemetry.time);
  pulse.pulse_seq_num = _pulseSeqNum++;
  pulse.scan_mode = _scanMode(telemetry);
  pulse.volume_num = telemetry.volumeNum;
  pulse.sweep_num = telemetry.sweepNum;
  pulse.elevation = telemetry.elevation;
  pulse.azimuth = telemetry.azimuth;
  pulse.antenna_transition = telemetry.inTransition ? 1 : 0;

  if (pulse.scan_mode == IWRF_SCAN_MODE_RHI) {
    pulse.fixed_az = telemetry.fixedAngle;
  } else if (pulse.scan_mode != IWRF_SCAN_MODE_IDLE) {
    pulse.fixed_el = telemetry.fixedAngle;
  }

  // Some general IWRF consumers assume every pulse has gate zero. The scan
  // stream has no measured signal, so attach one zero-valued dummy IQ gate
  // rather than exposing a null data array to those readers.
  const fl32 dummyIq[2] = {0.0, 0.0};
  pulse.n_gates = 1;
  pulse.n_channels = 1;
  pulse.iq_encoding = IWRF_IQ_ENCODING_FL32;
  pulse.n_data = 2;
  pulse.iq_offset[0] = 0;
  pulse.packet.len_bytes = sizeof(pulse) + sizeof(dummyIq);

  MemBuf pulseBuf;
  pulseBuf.add(&pulse, sizeof(pulse));
  pulseBuf.add(dummyIq, sizeof(dummyIq));
  _outputMsg.addPart(IWRF_PULSE_HEADER_ID, pulseBuf.getLen(),
                     pulseBuf.getPtr());

  if (_params.debug >= Params::DEBUG_EXTRA) {
    iwrf_pulse_header_print(stderr, pulse);
  }
}

//////////////////////////////////////////////////////
// Set the common time and sequence fields on a packet

void ScanInfo2TsFmq::_setPacketTimeAndSequence(iwrf_packet_info_t &packet,
                                               const DateTime &time)
{
  time_t seconds = time.utime();
  int nanoSecs = static_cast<int>(time.getSubSec() * 1.0e9 + 0.5);
  if (nanoSecs >= 1000000000) {
    seconds++;
    nanoSecs -= 1000000000;
  }
  iwrf_set_packet_time(packet, seconds, nanoSecs);
  packet.seq_num = _packetSeqNum++;
}

///////////////////////////////////////
// Open the output FMQ

int ScanInfo2TsFmq::_openOutputFmq()
{
  if (_outputFmq.initReadWrite(
          _params.output_fmq_path, _progName.c_str(),
          _params.debug >= Params::DEBUG_EXTRA, Fmq::END, false,
          _params.output_fmq_nslots, _params.output_fmq_size)) {
    cerr << "ERROR - " << _progName << ": cannot initialize FMQ "
         << _params.output_fmq_path << endl;
    cerr << _outputFmq.getErrStr() << endl;
    return -1;
  }
  _outputFmq.setSingleWriter();
  if (_params.data_mapper_report_interval > 0) {
    _outputFmq.setRegisterWithDmap(true, _params.data_mapper_report_interval);
  }
  _outputMsg.clearAll();
  _outputMsg.setType(0);
  return 0;
}

/////////////////////////////////////////////
// Write the accumulated message when ready

int ScanInfo2TsFmq::_writeToOutputFmq(bool force)
{
  const int nParts = _outputMsg.getNParts();
  if (nParts == 0 || (!force && nParts < _params.n_messages_per_write)) {
    return 0;
  }

  PMU_auto_register("Writing output FMQ");
  void *buffer = _outputMsg.assemble();
  const int length = _outputMsg.lengthAssembled();
  if (_outputFmq.writeMsg(0, 0, buffer, length)) {
    cerr << "ERROR - " << _progName << ": cannot write FMQ "
         << _params.output_fmq_path << endl;
    _outputMsg.clearParts();
    return -1;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Wrote " << nParts << " IWRF parts, " << length
         << " bytes" << endl;
  }
  _outputMsg.clearParts();
  return 0;
}

///////////////////////////////////////////////////////
// Write one end-of-volume event after input goes stale

int ScanInfo2TsFmq::_writeEndOfVol()
{

  if (!_haveScanState) {
    return 0;
  }

  iwrf_event_notice_t notice;
  iwrf_event_notice_init(notice);
  _setPacketTimeAndSequence(notice.packet, _prevTelemetryTime);
  notice.end_of_volume = true;
  notice.volume_num = _outputVolumeNum;
  notice.sweep_num = _outputSweepNum;
  _outputMsg.addPart(IWRF_EVENT_NOTICE_ID, sizeof(notice), &notice);

  if (_params.debug) {
    cerr << "Writing end-of-volume event after telemetry timeout" << endl;
  }

  return _writeToOutputFmq(true);

}
