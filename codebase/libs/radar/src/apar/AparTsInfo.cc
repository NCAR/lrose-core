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
// AparTsInfo.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Aug 2019
//
///////////////////////////////////////////////////////////////
//
// Stores current radar ops info
//
////////////////////////////////////////////////////////////////

#include <iostream>
#include <cerrno>
#include <cmath>
#include <sys/time.h>
#include <toolsa/str.h>
#include <toolsa/mem.h>
#include <toolsa/TaArray.hh>
#include <radar/AparTsInfo.hh>
#include <radar/RadarCalib.hh>
#include <rapformats/DsRadarCalib.hh>
using namespace std;

////////////////////////////////////////////////////
// Constructor

AparTsInfo::AparTsInfo(AparTsDebug_t debug) :
        _debug(debug)

{
  clear();
  _debugPrintCount = 0;
}

/////////////////////////////
// Copy constructor

AparTsInfo::AparTsInfo(const AparTsInfo &rhs)

{
  if (this != &rhs) {
    _copy(rhs);
  }
}

//////////////////////////////////////////////////////////////////
// destructor

AparTsInfo::~AparTsInfo()

{
  _clearMetaQueue();
}

/////////////////////////////
// copy

AparTsInfo &AparTsInfo::_copy(const AparTsInfo &rhs)

{

  if (&rhs == this) {
    return *this;
  }

  // copy members

  _debug = rhs._debug;
  _debugPrintCount = 0;

  _radar_info = rhs._radar_info;
  _scan_seg = rhs._scan_seg;
  _proc = rhs._proc;
  _status_xml_hdr = rhs._status_xml_hdr;
  _status_xml_str = rhs._status_xml_str;
  _calib = rhs._calib;
  _enotice = rhs._enotice;
  _platform_georef0 = rhs._platform_georef0;
  _platform_georef1 = rhs._platform_georef1;

  _radar_info_active = rhs._radar_info_active;
  _scan_seg_active = rhs._scan_seg_active;
  _proc_active = rhs._proc_active;
  _status_xml_active = rhs._status_xml_active;
  _calib_active = rhs._calib_active;
  _enotice_active = rhs._enotice_active;
  _platform_georef0_active = rhs._platform_georef0_active;
  _platform_georef1_active = rhs._platform_georef1_active;

  // free up existing meta-data queue entries

  for (size_t ii = 0; ii < _metaQueue.size(); ii++) {
    delete _metaQueue[ii];
  }
  _metaQueue.clear();
  
  // copy over meta-data queue entries
  for (size_t ii = 0; ii < rhs._metaQueue.size(); ii++) {
    MemBuf *mcopy = new MemBuf(*rhs._metaQueue[ii]);
    _metaQueue.push_back(mcopy);
  }
  
  _startOfSweepFlag = rhs._startOfSweepFlag;
  _endOfSweepFlag = rhs._endOfSweepFlag;
  _startOfVolumeFlag = rhs._startOfVolumeFlag;
  _endOfVolumeFlag = rhs._endOfVolumeFlag;

  return *this;

}

/////////////////////////////
// Assignment
//

AparTsInfo &AparTsInfo::operator=(const AparTsInfo &rhs)

{
  return _copy(rhs);
}

//////////////////////////////////////////////////////////////////
// clear all

void AparTsInfo::clear()

{
  
  apar_ts_radar_info_init(_radar_info);
  apar_ts_scan_segment_init(_scan_seg);
  apar_ts_processing_init(_proc);
  apar_ts_status_xml_init(_status_xml_hdr);
  apar_ts_calibration_init(_calib);
  apar_ts_event_notice_init(_enotice);
  apar_ts_platform_georef_init(_platform_georef0);
  apar_ts_platform_georef_init(_platform_georef1);

  _radar_info_active = false;
  _scan_seg_active = false;
  _proc_active = false;
  _status_xml_active = false;
  _calib_active = false;
  _enotice_active = false;
  _platform_georef0_active = false;
  _platform_georef1_active = false;

  clearEventFlags();

}

///////////////////////////////////////////////////////////
// set structs from a generic buffer
// by checking for the id
// swaps as required

int AparTsInfo::setFromBuffer(const void *buf, int len)
  
{

  // check validity of packet
  
  int packet_id;
  if (apar_ts_get_packet_id(buf, len, packet_id)) {
    cerr << "ERROR - AparTsInfo::setFromBuffer" << endl;
    fprintf(stderr, "  Bad packet, id: 0x%x\n", packet_id);
    cerr << "             len: " << len << endl;
    cerr << "            type: " << apar_ts_packet_id_to_str(packet_id) << endl;
    return -1;
  }

  // swap packet as required, using a copy

  char *copy = new char[len + 1];
  memcpy(copy, buf, len);
  apar_ts_packet_swap(copy, len);

  switch (packet_id) {
    case APAR_TS_RADAR_INFO_ID: {
      setRadarInfo(*(apar_ts_radar_info_t *) copy);
      setRadarInfoActive(true);
    } break;
    case APAR_TS_SCAN_SEGMENT_ID: {
      setScanSegment(*(apar_ts_scan_segment_t *) copy);
      setScanSegmentActive(true);
    } break;
    case APAR_TS_PROCESSING_ID: {
      setTsProcessing(*(apar_ts_processing_t *) copy);
      setTsProcessingActive(true);
    } break;
    case APAR_TS_STATUS_XML_ID: {
      setStatusXml(*(apar_ts_status_xml_t *) copy,
                   copy + sizeof(apar_ts_status_xml_t));
      setStatusXmlActive(true);
    } break;
    case APAR_TS_CALIBRATION_ID: {
      setCalibration(*(apar_ts_calibration_t *) copy);
      setCalibrationActive(true);
    } break;
    case APAR_TS_EVENT_NOTICE_ID: {
      setEventNotice(*(apar_ts_event_notice_t *) copy);
      setEventNoticeActive(true);
    } break;
    case APAR_TS_PLATFORM_GEOREF_ID: {
      apar_ts_platform_georef_t georef;
      memcpy(&georef, copy, sizeof(apar_ts_platform_georef_t));
      apar_ts_platform_georef_swap(georef);
      setPlatformGeoref(georef);
    } break;
    case APAR_TS_SYNC_ID:
    case APAR_TS_PULSE_HEADER_ID:
    case APAR_TS_GEOREF_CORRECTION_ID:
      break;
    default: {
      delete[] copy;
      return -1;
    }
  }

  delete[] copy;
  return 0;

}

///////////////////////////////////////////////////////////
// override radar name and site name

void AparTsInfo::overrideRadarName(const string &radarName)
{
  STRncopy(_radar_info.radar_name, radarName.c_str(), APAR_TS_MAX_RADAR_NAME);
}

void AparTsInfo::overrideSiteName(const string &siteName)
{
  STRncopy(_radar_info.site_name, siteName.c_str(), APAR_TS_MAX_SITE_NAME);
}

///////////////////////////////////////////////////////////
// override radar location

void AparTsInfo::overrideRadarLocation(double altitudeMeters,
				       double latitudeDeg,
				       double longitudeDeg)
  
{
  
  _radar_info.altitude_m = altitudeMeters;
  _radar_info.latitude_deg = latitudeDeg;
  _radar_info.longitude_deg = longitudeDeg;

}

///////////////////////////////////////////////////////////
// override gate geometry

void AparTsInfo::overrideGateGeometry(double startRangeMeters,
				      double gateSpacingMeters)
  
{
  
  _proc.start_range_m = startRangeMeters;
  _proc.gate_spacing_m = gateSpacingMeters; 

}

///////////////////////////////////////////////////////////
// override wavelength

void AparTsInfo::overrideWavelength(double wavelengthCm)
  
{
  
  _radar_info.wavelength_cm = wavelengthCm;
  _calib.wavelength_cm = wavelengthCm;

}

///////////////////////////////////////////////////////////
// is info ready to be used?
 
bool AparTsInfo::isEssentialInfoReady() const
{

  if (_radar_info_active && _proc_active) {
    return true;
  }

  if (_debug != AparTsDebug_t::OFF) {
    if (_debugPrintCount % 2500 == 0) {
      if (!_radar_info_active) {
        cerr << "INFO - AparTsInfo::isEssentialInfoReady()" << endl;
        cerr << "  Waiting for APAR_TS_RADAR_INFO packet" << endl;
      }
      if (!_proc_active) {
        cerr << "INFO - AparTsInfo::isEssentialInfoReady()" << endl;
        cerr << "  Waiting for APAR_TS_PROCESSING packet" << endl;
      }
    }
    _debugPrintCount++;
  }
   
  return false;
   
}

///////////////////////////////////////////////////////////
// is this an info packet? Check the id

bool AparTsInfo::isInfo(int id)
  
{

  switch (id) {
    case APAR_TS_SYNC_ID:
    case APAR_TS_RADAR_INFO_ID:
    case APAR_TS_SCAN_SEGMENT_ID:
    case APAR_TS_PROCESSING_ID:
    case APAR_TS_STATUS_XML_ID:
    case APAR_TS_CALIBRATION_ID:
    case APAR_TS_EVENT_NOTICE_ID:
    case APAR_TS_PLATFORM_GEOREF_ID:
      return true;
  }
  return false;

}

////////////////////////////////////////////////////////////
// set info at the struct level

void AparTsInfo::setRadarInfo(const apar_ts_radar_info_t &info,
                              bool addToMetaQueue /* = true */) {
  _radar_info = info;
  _radar_info.packet.id = APAR_TS_RADAR_INFO_ID;
  _radar_info.packet.len_bytes = sizeof(apar_ts_radar_info_t);
  _radar_info.packet.version_num = 1;
  if (addToMetaQueue) {
    // _addIdToQueue(APAR_TS_RADAR_INFO_ID);
    _addMetaToQueue(sizeof(_radar_info), &_radar_info);
  }
}

void AparTsInfo::setScanSegment(const apar_ts_scan_segment_t &seg,
                                bool addToMetaQueue /* = true */) {
  _scan_seg = seg;
  _scan_seg.packet.id = APAR_TS_SCAN_SEGMENT_ID;
  _scan_seg.packet.len_bytes = sizeof(apar_ts_scan_segment_t);
  _scan_seg.packet.version_num = 1;
  if (addToMetaQueue) {
    // _addIdToQueue(APAR_TS_SCAN_SEGMENT_ID);
    _addMetaToQueue(sizeof(_scan_seg), &_scan_seg);
  }
}

void AparTsInfo::setTsProcessing(const apar_ts_processing_t &proc,
                                 bool addToMetaQueue /* = true */) {
  _proc = proc;
  if (isnan(_proc.start_range_m)) {
    _proc.start_range_m = 0.0;
  }
  _proc.packet.id = APAR_TS_PROCESSING_ID;
  _proc.packet.len_bytes = sizeof(apar_ts_processing_t);
  _proc.packet.version_num = 1;
  if (addToMetaQueue) {
    // _addIdToQueue(APAR_TS_PROCESSING_ID);
    _addMetaToQueue(sizeof(_proc), &_proc);
  }
}

void AparTsInfo::setStatusXml(const apar_ts_status_xml_t &hdr,
                              const string &xmlStr,
                              bool addToMetaQueue /* = true */) {
  _status_xml_hdr = hdr;
  _status_xml_str = xmlStr;
  _status_xml_hdr.packet.id = APAR_TS_STATUS_XML_ID;
  _status_xml_hdr.packet.len_bytes =
    sizeof(apar_ts_status_xml_t) + xmlStr.size() + 1;
  _status_xml_hdr.packet.version_num = 1;
  if (addToMetaQueue) {
    // _addIdToQueue(APAR_TS_STATUS_XML_ID);
    MemBuf buf;
    buf.add(&_status_xml_hdr, sizeof(_status_xml_hdr));
    buf.add(xmlStr.c_str(), xmlStr.size() + 1);
    _addMetaToQueue(buf.getLen(), buf.getPtr());
  }
}

void AparTsInfo::setCalibration(const apar_ts_calibration_t &calib,
                                bool addToMetaQueue /* = true */) {
  _calib = calib;
  _calib.packet.id = APAR_TS_CALIBRATION_ID;
  _calib.packet.len_bytes = sizeof(apar_ts_calibration_t);
  _calib.packet.version_num = 1;
  if (addToMetaQueue) {
    // _addIdToQueue(APAR_TS_CALIBRATION_ID);
    _addMetaToQueue(sizeof(_calib), &_calib);
  }
}

void AparTsInfo::setEventNotice(const apar_ts_event_notice_t &enotice,
                                bool addToMetaQueue /* = true */) {
  _enotice = enotice;
  _enotice.packet.id = APAR_TS_EVENT_NOTICE_ID;
  _enotice.packet.len_bytes = sizeof(apar_ts_event_notice_t);
  _enotice.packet.version_num = 1;
  if (addToMetaQueue) {
    // _addIdToQueue(APAR_TS_EVENT_NOTICE_ID);
    _addMetaToQueue(sizeof(_enotice), &_enotice);
  }
  if (_enotice.start_of_sweep) {
    setStartOfSweepFlag();
  }
  if (_enotice.end_of_sweep) {
    setEndOfSweepFlag();
  }
  if (_enotice.start_of_volume) {
    setStartOfVolumeFlag();
  }
  if (_enotice.end_of_volume) {
    setEndOfVolumeFlag();
  }
}

void AparTsInfo::setPlatformGeoref
  (const apar_ts_platform_georef_t &platform_georef,
   bool addToMetaQueue /* = true */) {
  if (platform_georef.unit_num == 1) {
    _platform_georef1 = platform_georef;
    _platform_georef1.packet.id = APAR_TS_PLATFORM_GEOREF_ID;
    _platform_georef1.packet.len_bytes = sizeof(apar_ts_platform_georef_t);
    _platform_georef1.packet.version_num = 1;
    setPlatformGeoref1Active(true);
    if (addToMetaQueue) {
      _addMetaToQueue(sizeof(_platform_georef1), &_platform_georef1);
    }
  } else {
    _platform_georef0 = platform_georef;
    _platform_georef0.packet.id = APAR_TS_PLATFORM_GEOREF_ID;
    _platform_georef0.packet.len_bytes = sizeof(apar_ts_platform_georef_t);
    _platform_georef0.packet.version_num = 1;
    setPlatformGeorefActive(true);
    if (addToMetaQueue) {
      _addMetaToQueue(sizeof(_platform_georef0), &_platform_georef0);
    }
  }
}

////////////////////////////////////////////////////////////
// activate structs

void AparTsInfo::setRadarInfoActive(bool state) {
  _radar_info_active = state;
}

void AparTsInfo::setScanSegmentActive(bool state) {
  _scan_seg_active = state;
}

void AparTsInfo::setTsProcessingActive(bool state) {
  _proc_active = state;
}

void AparTsInfo::setStatusXmlActive(bool state) {
  _status_xml_active = state;
}

void AparTsInfo::setCalibrationActive(bool state) {
  _calib_active = state;
}

void AparTsInfo::setEventNoticeActive(bool state) {
  _enotice_active = state;
}

void AparTsInfo::setPlatformGeorefActive(bool state) {
  _platform_georef0_active = state;
}

void AparTsInfo::setPlatformGeoref1Active(bool state) {
  _platform_georef1_active = state;
}

////////////////////////////////////////////////////////////
// set sequence number for each packet type

void AparTsInfo::setRadarInfoPktSeqNum(si64 pkt_seq_num) {
  _radar_info.packet.seq_num = pkt_seq_num;
}

void AparTsInfo::setScanSegmentPktSeqNum(si64 pkt_seq_num) {
  _scan_seg.packet.seq_num = pkt_seq_num;
}

void AparTsInfo::setTsProcessingPktSeqNum(si64 pkt_seq_num) {
  _proc.packet.seq_num = pkt_seq_num;
}

void AparTsInfo::setStatusXmlPktSeqNum(si64 pkt_seq_num) {
  _status_xml_hdr.packet.seq_num = pkt_seq_num;
}

void AparTsInfo::setCalibrationPktSeqNum(si64 pkt_seq_num) {
  _calib.packet.seq_num = pkt_seq_num;
}

void AparTsInfo::setEventNoticePktSeqNum(si64 pkt_seq_num) {
  _enotice.packet.seq_num = pkt_seq_num;
}

void AparTsInfo::setPlatformGeorefPktSeqNum(si64 pkt_seq_num) {
  _platform_georef0.packet.seq_num = pkt_seq_num;
}

void AparTsInfo::setPlatformGeoref1PktSeqNum(si64 pkt_seq_num) {
  _platform_georef1.packet.seq_num = pkt_seq_num;
}

////////////////////////////////////////////////////////////
// set time on all packets

void AparTsInfo::setTime(time_t secs, int nano_secs) {
  apar_ts_set_packet_time(_radar_info.packet, secs, nano_secs);
  apar_ts_set_packet_time(_scan_seg.packet, secs, nano_secs);
  apar_ts_set_packet_time(_proc.packet, secs, nano_secs);
  apar_ts_set_packet_time(_status_xml_hdr.packet, secs, nano_secs);
  apar_ts_set_packet_time(_calib.packet, secs, nano_secs);
  apar_ts_set_packet_time(_enotice.packet, secs, nano_secs);
  if (_platform_georef0_active) {
    apar_ts_set_packet_time(_platform_georef0.packet, secs, nano_secs);
  }
  if (_platform_georef1_active) {
    apar_ts_set_packet_time(_platform_georef1.packet, secs, nano_secs);
  }
}

////////////////////////////////////////////////////////////
// set time to now on all packets

void AparTsInfo::setTimeToNow() {
  struct timeval time;
  gettimeofday(&time, NULL);
  setTime(time.tv_sec, time.tv_usec * 1000);
}

////////////////////////////////////////////////////////////
// set time for each packet

void AparTsInfo::setRadarInfoTime(time_t secs, int nano_secs) {
  apar_ts_set_packet_time(_radar_info.packet, secs, nano_secs);
}

void AparTsInfo::setScanSegmentTime(time_t secs, int nano_secs) {
  apar_ts_set_packet_time(_scan_seg.packet, secs, nano_secs);
}

void AparTsInfo::setTsProcessingTime(time_t secs, int nano_secs) {
  apar_ts_set_packet_time(_proc.packet, secs, nano_secs);
}

void AparTsInfo::setStatusXmlTime(time_t secs, int nano_secs) {
  apar_ts_set_packet_time(_status_xml_hdr.packet, secs, nano_secs);
}

void AparTsInfo::setCalibrationTime(time_t secs, int nano_secs) {
  apar_ts_set_packet_time(_calib.packet, secs, nano_secs);
}

void AparTsInfo::setEventNoticeTime(time_t secs, int nano_secs) {
  apar_ts_set_packet_time(_enotice.packet, secs, nano_secs);
}

void AparTsInfo::setPlatformGeorefTime(time_t secs, int nano_secs) {
  apar_ts_set_packet_time(_platform_georef0.packet, secs, nano_secs);
}

void AparTsInfo::setPlatformGeoref1Time(time_t secs, int nano_secs) {
  apar_ts_set_packet_time(_platform_georef1.packet, secs, nano_secs);
}

////////////////////////////////////////////////////////////
// set time to now for each packet

void AparTsInfo::setRadarInfoTimeToNow() {
  apar_ts_set_packet_time_to_now(_radar_info.packet);
}

void AparTsInfo::setScanSegmentTimeToNow() {
  apar_ts_set_packet_time_to_now(_scan_seg.packet);
}

void AparTsInfo::setTsProcessingTimeToNow() {
  apar_ts_set_packet_time_to_now(_proc.packet);
}

void AparTsInfo::setStatusXmlTimeToNow() {
  apar_ts_set_packet_time_to_now(_status_xml_hdr.packet);
}

void AparTsInfo::setCalibrationTimeToNow() {
  apar_ts_set_packet_time_to_now(_calib.packet);
}

void AparTsInfo::setEventNoticeTimeToNow() {
  apar_ts_set_packet_time_to_now(_enotice.packet);
}

void AparTsInfo::setPlatformGeorefTimeToNow() {
  apar_ts_set_packet_time_to_now(_platform_georef0.packet);
}

void AparTsInfo::setPlatformGeoref1TimeToNow() {
  apar_ts_set_packet_time_to_now(_platform_georef1.packet);
}

////////////////////////////////////////////////////////////
// get time for each packet

double AparTsInfo::getRadarInfoTime() const {
  return apar_ts_get_packet_time_as_double(_radar_info.packet);
}

double AparTsInfo::getScanSegmentTime() const {
  return apar_ts_get_packet_time_as_double(_scan_seg.packet);
}

double AparTsInfo::getTsProcessingTime() const {
  return apar_ts_get_packet_time_as_double(_proc.packet);
}

double AparTsInfo::getStatusXmlTime() const {
  return apar_ts_get_packet_time_as_double(_status_xml_hdr.packet);
}

double AparTsInfo::getCalibrationTime() const {
  return apar_ts_get_packet_time_as_double(_calib.packet);
}

double AparTsInfo::getEventNoticeTime() const {
  return apar_ts_get_packet_time_as_double(_enotice.packet);
}

double AparTsInfo::getPlatformGeorefTime() const {
  return apar_ts_get_packet_time_as_double(_platform_georef0.packet);
}

double AparTsInfo::getPlatformGeoref1Time() const {
  return apar_ts_get_packet_time_as_double(_platform_georef1.packet);
}

////////////////////////////////////////////////////////////
// set radar ID on all packet types

void AparTsInfo::setRadarId(int id) {

  _radar_info.packet.radar_id = id;
  _scan_seg.packet.radar_id = id;
  _proc.packet.radar_id = id;
  _status_xml_hdr.packet.radar_id = id;
  _calib.packet.radar_id = id;
  _enotice.packet.radar_id = id;
  _platform_georef0.packet.radar_id = id;
  _platform_georef1.packet.radar_id = id;

}

///////////////////////////////////////////////////////////////
// set AparTsCalib object from _calib

void AparTsInfo::setAparTsCalib(AparTsCalib &aparCalib) const

{
  if (_calib_active) {
    // only set it if _calib has been set
    aparCalib.set(_calib);
  }
}

///////////////////////////////////////////////////////////////
// set _calib from AparTsCalib object

void AparTsInfo::setFromAparTsCalib(const AparTsCalib &aparCalib)

{
  _calib = aparCalib.getStruct();
}

////////////////////////////////////////////////////////////
// set methods for individual fields

void AparTsInfo::setRadarName(const string &x) {
  STRncopy(_radar_info.radar_name, x.c_str(), APAR_TS_MAX_RADAR_NAME);
}

void AparTsInfo::setSiteName(const string &x) { 
  STRncopy(_radar_info.site_name, x.c_str(), APAR_TS_MAX_SITE_NAME);
}

void AparTsInfo::setScanSegmentName(const string &x) {
  STRncopy(_scan_seg.segment_name, x.c_str(), APAR_TS_MAX_SEGMENT_NAME);
}

void AparTsInfo::setScanProjectName(const string &x) {
  STRncopy(_scan_seg.project_name, x.c_str(), APAR_TS_MAX_PROJECT_NAME);
}

void AparTsInfo::setScanFixedAngle(int i, fl32 x) {
  if (i < APAR_TS_MAX_FIXED_ANGLES) {
    _scan_seg.fixed_angles[i] = x;
  }
}

////////////////////////////////////////////////////////////
// get methods for individual fields

string AparTsInfo::getRadarName() const {
  return apar_ts_safe_str(_radar_info.radar_name, APAR_TS_MAX_RADAR_NAME);
}

string AparTsInfo::getSiteName() const { 
  return apar_ts_safe_str(_radar_info.site_name, APAR_TS_MAX_SITE_NAME);
}

string AparTsInfo::getScanSegmentName() const {
  return apar_ts_safe_str(_scan_seg.segment_name, APAR_TS_MAX_SEGMENT_NAME);
}

string AparTsInfo::getScanProjectName() const {
  return apar_ts_safe_str(_scan_seg.project_name, APAR_TS_MAX_PROJECT_NAME);
}

fl32 AparTsInfo::getScanFixedAngle(int i) const {
  if (i < APAR_TS_MAX_FIXED_ANGLES) {
    return _scan_seg.fixed_angles[i];
  } else {
    return -1;
  }
}

///////////////////////////////////////////////////////////
// print

// print everything

void AparTsInfo::print(FILE *out) const
{
  
  fprintf(out, "******************** Start AparTsInfo ***************************\n");

  if (_radar_info_active) {
    apar_ts_radar_info_print(out, _radar_info);
  }
  if (_scan_seg_active) {
    apar_ts_scan_segment_print(out, _scan_seg);
  }
  if (_proc_active) {
    apar_ts_processing_print(out, _proc);
  }
  if (_status_xml_active) {
    apar_ts_status_xml_print(out, _status_xml_hdr, _status_xml_str);
  }
  if (_calib_active) {
    apar_ts_calibration_print(out, _calib);
  }
  if (_enotice_active) {
    apar_ts_event_notice_print(out, _enotice);
  }
  if (_platform_georef0_active) {
    apar_ts_platform_georef_print(out, _platform_georef0);
  }
  if (_platform_georef1_active) {
    apar_ts_platform_georef_print(out, _platform_georef1);
  }

  fprintf(out, "********************* End AparTsInfo ****************************\n");

}

// print only what is in the id queue
// clear the queue if clearQueue is set to true

void AparTsInfo::printMetaQueue(FILE *out, bool clearQueue) const
{
  
  if (_metaQueue.size() == 0) {
    return;
  }

  if (_debug >= AparTsDebug_t::VERBOSE) {
    cerr << "DEBUG - AparTsInfo::printMetaQueue()" << endl;
    cerr << "  _metaQueue.size(): " << _metaQueue.size() << endl;
  }

  fprintf(out, "******************** Start AparTsInfo ***************************\n");
  
  for (size_t ii = 0; ii < _metaQueue.size(); ii++) {
    MemBuf *buf = _metaQueue[ii];
    apar_ts_packet_print(out, buf->getPtr(), buf->getLen());
  } // ii

  fprintf(out, "********************* End AparTsInfo ****************************\n");
  
  if (clearQueue) {
    _clearMetaQueue();
  }

}

///////////////////////////////////////////////////
// write a sync packet to file in APAR format
//
// returns 0 on success, -1 on failure

int AparTsInfo::writeSyncToFile(FILE *out) const

{
  
  if (out == NULL) {
    return -1;
  }
  
  apar_ts_sync_t sync;
  apar_ts_sync_init(sync);
  if (fwrite(&sync, sizeof(sync), 1, out) != 1) {
    int errNum = errno;
    cerr << "ERROR - AparTsInfo::writeSync2File" << endl;
    cerr << "  Cannot write sync packet" << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  
  return 0;

}

///////////////////////////////////////////////////
// write meta-data to file in APAR format
//
// Writes out any meta-data which has a sequence number
// later than the previous pulse written.
// To force a write, set the prevPulseSeqNum to 0.
//
// returns 0 on success, -1 on failure

int AparTsInfo::writeMetaToFile(FILE *out, si64 prevPulseSeqNum) const

{
  
  if (out == NULL) {
    return -1;
  }
  
  if (_radar_info.packet.seq_num > prevPulseSeqNum) {

    // send sync packet
    
    if (writeSyncToFile(out)) {
      return -1;
    }
    
    if (_radar_info_active) {
      if (fwrite(&_radar_info, sizeof(_radar_info), 1, out) != 1) {
	int errNum = errno;
	cerr << "ERROR - AparTsInfo::write2File" << endl;
	cerr << "  Cannot write _radar_info packet" << endl;
	cerr << "  " << strerror(errNum) << endl;
	return -1;
      }
    }

  }

  if (_scan_seg_active && _scan_seg.packet.seq_num > prevPulseSeqNum) {
    if (fwrite(&_scan_seg, sizeof(_scan_seg), 1, out) != 1) {
      int errNum = errno;
      cerr << "ERROR - AparTsInfo::write2File" << endl;
      cerr << "  Cannot write _scan_seg packet" << endl;
      cerr << "  " << strerror(errNum) << endl;
      return -1;
    }
  }

  if (_proc_active && _proc.packet.seq_num > prevPulseSeqNum) {
    if (fwrite(&_proc, sizeof(_proc), 1, out) != 1) {
      int errNum = errno;
      cerr << "ERROR - AparTsInfo::write2File" << endl;
      cerr << "  Cannot write _proc packet" << endl;
      cerr << "  " << strerror(errNum) << endl;
      return -1;
    }
  }

  if (_status_xml_active &&
      _status_xml_hdr.packet.seq_num > prevPulseSeqNum) {
    if (fwrite(&_status_xml_hdr, sizeof(_status_xml_hdr), 1, out) != 1) {
      int errNum = errno;
      cerr << "ERROR - AparTsInfo::write2File" << endl;
      cerr << "  Cannot write _status_xml header" << endl;
      cerr << "  " << strerror(errNum) << endl;
      return -1;
    }
    if (fwrite(_status_xml_str.c_str(),
               _status_xml_str.size() + 1, 1, out) != 1) {
      int errNum = errno;
      cerr << "ERROR - AparTsInfo::write2File" << endl;
      cerr << "  Cannot write _status_xml string" << endl;
      cerr << "  " << strerror(errNum) << endl;
      return -1;
    }
  }

  if (_calib_active && _calib.packet.seq_num > prevPulseSeqNum) {
    if (fwrite(&_calib, sizeof(_calib), 1, out) != 1) {
      int errNum = errno;
      cerr << "ERROR - AparTsInfo::write2File" << endl;
      cerr << "  Cannot write _calib packet" << endl;
      cerr << "  " << strerror(errNum) << endl;
      return -1;
    }
  }

  if (_enotice_active && _enotice.packet.seq_num > prevPulseSeqNum) {
    if (fwrite(&_enotice, sizeof(_enotice), 1, out) != 1) {
      int errNum = errno;
      cerr << "ERROR - AparTsInfo::write2File" << endl;
      cerr << "  Cannot write _enotice packet" << endl;
      cerr << "  " << strerror(errNum) << endl;
      return -1;
    }
  }

  if (_platform_georef0_active &&
      _platform_georef0.packet.seq_num > prevPulseSeqNum) {
    if (fwrite(&_platform_georef0, sizeof(_platform_georef0), 1, out) != 1) {
      int errNum = errno;
      cerr << "ERROR - AparTsInfo::write2File" << endl;
      cerr << "  Cannot write _platform_georef0 packet" << endl;
      cerr << "  " << strerror(errNum) << endl;
      return -1;
    }
  }

  if (_platform_georef1_active &&
      _platform_georef1.packet.seq_num > prevPulseSeqNum) {
    if (fwrite(&_platform_georef1, sizeof(_platform_georef1), 1, out) != 1) {
      int errNum = errno;
      cerr << "ERROR - AparTsInfo::write2File" << endl;
      cerr << "  Cannot write _platform_georef1 packet" << endl;
      cerr << "  " << strerror(errNum) << endl;
      return -1;
    }
  }

  return 0;

}

///////////////////////////////////////////////////
// write meta-data to file in APAR format
//
// Writes out any meta-data in the queue
// If clearQueue is true, the queue will be cleared.
//
// Returns 0 on success, -1 on failure

int AparTsInfo::writeMetaQueueToFile(FILE *out, bool clearQueue) const

{
  
  if (_metaQueue.size() == 0) {
    return 0;
  }

  if (out == NULL) {
    return -1;
  }
  
  // write sync packet
  
  if (writeSyncToFile(out)) {
    return -1;
  }

  if (_debug >= AparTsDebug_t::VERBOSE) {
    cerr << "DEBUG - AparTsInfo::writeMetaQueueToFile()" << endl;
    cerr << "  _metaQueue.size(): " << _metaQueue.size() << endl;
  }

  // write out the queue of metadata packets

  int iret = 0;
  for (size_t ii = 0; ii < _metaQueue.size(); ii++) {
    MemBuf *buf = _metaQueue[ii];
    if (fwrite(buf->getPtr(), buf->getLen(), 1, out) != 1) {
      int errNum = errno;
      cerr << "ERROR - AparTsInfo::write2File" << endl;
      cerr << "  Cannot write metaData packet" << endl;
      cerr << "  " << strerror(errNum) << endl;
      apar_ts_packet_print(stderr, buf->getPtr(), buf->getLen());
      iret = -1;
    }
  } // ii

  if (clearQueue) {
    _clearMetaQueue();
  }

  return iret;

}

///////////////////////////////////////////////////
// add meta-data to DsMessage in APAR format
//
// Loads up any meta-data in the queue.
// If clearQueue is true, the queue will be cleared.

void AparTsInfo::addMetaQueueToMsg(DsMessage &msg, bool clearQueue) const
  
{

  if (_debug >= AparTsDebug_t::VERBOSE) {
    cerr << "DEBUG - AparTsInfo::addMetaQueueToMsg()" << endl;
    cerr << "  _metaQueue.size(): " << _metaQueue.size() << endl;
  }

  for (size_t ii = 0; ii < _metaQueue.size(); ii++) {
    MemBuf *buf = _metaQueue[ii];
    int packetId = 0;
    if (apar_ts_get_packet_id(buf->getPtr(), buf->getLen(), packetId) == 0) {
      msg.addPart(packetId, buf->getLen(), buf->getPtr());
    }
  } // ii

  if (clearQueue) {
    _clearMetaQueue();
  }

}

///////////////////////////////////////////////////////////////
// add id to queue
// deletes previous entries for the same id

// void AparTsInfo::_addIdToQueue(si32 id)

// {
  
//   bool done = false;
//   while (!done) {
//     done = true;
//     deque<si32>::iterator ii;
//     for (ii = _idQueue.begin(); ii != _idQueue.end(); ii++) {
//       if (*ii == id) {
// 	_idQueue.erase(ii);
// 	done = false;
// 	break;
//       }
//     }
//   }
//   _idQueue.push_back(id);
  
// }

///////////////////////////////////////////////////////////////
// add metadata packet to back of queue

void AparTsInfo::_addMetaToQueue(size_t len, const void *packet)

{

  // prevent queue from getting too large
  // will never need more that 100 metadata packets
  
  if (_metaQueue.size() > 100) {
    MemBuf *front = _popFrontFromMetaQueue();
    delete front;
  }
  
  // add to the back of the queue

  MemBuf *mbuf = new MemBuf;
  mbuf->add(packet, len);
  _metaQueue.push_back(mbuf);
  
}

/////////////////////////////////////////////
// pop an entry from the metadata queue

MemBuf *AparTsInfo::_popFrontFromMetaQueue()
  
{
  MemBuf *front = _metaQueue.front();
  _metaQueue.pop_front();
  return front;
}

/////////////////////////////////////////////
// clear the metadata queue - public

void AparTsInfo::clearMetadataQueue()
  
{
  // _idQueue.clear();
  _clearMetaQueue();
}

/////////////////////////////////////////////
// clear the metadata queue - private

void AparTsInfo::_clearMetaQueue() const
  
{
  for (size_t ii = 0; ii < _metaQueue.size(); ii++) {
    delete _metaQueue[ii];
  }
  _metaQueue.clear();
}

/////////////////////////////////////////////
// clear the event flags

void AparTsInfo::clearEventFlags()
  
{

  _startOfSweepFlag = false;
  _endOfSweepFlag = false;
  _startOfVolumeFlag = false;
  _endOfVolumeFlag = false;

}

