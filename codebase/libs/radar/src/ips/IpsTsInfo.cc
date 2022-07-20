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
// IpsTsInfo.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Aug 2019
//
///////////////////////////////////////////////////////////////
//
// Support for Independent Pulse Sampling.
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
#include <radar/IpsTsInfo.hh>
#include <radar/RadarCalib.hh>
#include <rapformats/DsRadarCalib.hh>
using namespace std;

////////////////////////////////////////////////////
// Constructor

IpsTsInfo::IpsTsInfo(IpsTsDebug_t debug) :
        _debug(debug)

{
  clear();
  _debugPrintCount = 0;
}

/////////////////////////////
// Copy constructor

IpsTsInfo::IpsTsInfo(const IpsTsInfo &rhs)

{
  if (this != &rhs) {
    _copy(rhs);
  }
}

//////////////////////////////////////////////////////////////////
// destructor

IpsTsInfo::~IpsTsInfo()

{
  _clearMetaQueue();
}

/////////////////////////////
// copy

IpsTsInfo &IpsTsInfo::_copy(const IpsTsInfo &rhs)

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

IpsTsInfo &IpsTsInfo::operator=(const IpsTsInfo &rhs)

{
  return _copy(rhs);
}

//////////////////////////////////////////////////////////////////
// clear all

void IpsTsInfo::clear()

{
  
  ips_ts_radar_info_init(_radar_info);
  ips_ts_scan_segment_init(_scan_seg);
  ips_ts_processing_init(_proc);
  ips_ts_status_xml_init(_status_xml_hdr);
  ips_ts_calibration_init(_calib);
  ips_ts_event_notice_init(_enotice);
  ips_ts_platform_georef_init(_platform_georef0);
  ips_ts_platform_georef_init(_platform_georef1);

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

int IpsTsInfo::setFromBuffer(const void *buf, int len)
  
{

  // check validity of packet
  
  int packet_id;
  if (ips_ts_get_packet_id(buf, len, packet_id)) {
    cerr << "ERROR - IpsTsInfo::setFromBuffer" << endl;
    fprintf(stderr, "  Bad packet, id: 0x%x\n", packet_id);
    cerr << "             len: " << len << endl;
    cerr << "            type: " << ips_ts_packet_id_to_str(packet_id) << endl;
    return -1;
  }

  // swap packet as required, using a copy

  char *copy = new char[len + 1];
  memcpy(copy, buf, len);
  ips_ts_packet_swap(copy, len);

  switch (packet_id) {
    case IPS_TS_RADAR_INFO_ID: {
      setRadarInfo(*(ips_ts_radar_info_t *) copy);
      setRadarInfoActive(true);
    } break;
    case IPS_TS_SCAN_SEGMENT_ID: {
      setScanSegment(*(ips_ts_scan_segment_t *) copy);
      setScanSegmentActive(true);
    } break;
    case IPS_TS_PROCESSING_ID: {
      setTsProcessing(*(ips_ts_processing_t *) copy);
      setTsProcessingActive(true);
    } break;
    case IPS_TS_STATUS_XML_ID: {
      setStatusXml(*(ips_ts_status_xml_t *) copy,
                   copy + sizeof(ips_ts_status_xml_t));
      setStatusXmlActive(true);
    } break;
    case IPS_TS_CALIBRATION_ID: {
      setCalibration(*(ips_ts_calibration_t *) copy);
      setCalibrationActive(true);
    } break;
    case IPS_TS_EVENT_NOTICE_ID: {
      setEventNotice(*(ips_ts_event_notice_t *) copy);
      setEventNoticeActive(true);
    } break;
    case IPS_TS_PLATFORM_GEOREF_ID: {
      ips_ts_platform_georef_t georef;
      memcpy(&georef, copy, sizeof(ips_ts_platform_georef_t));
      ips_ts_platform_georef_swap(georef);
      setPlatformGeoref(georef);
    } break;
    case IPS_TS_SYNC_ID:
    case IPS_TS_PULSE_HEADER_ID:
    case IPS_TS_GEOREF_CORRECTION_ID:
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

void IpsTsInfo::overrideRadarName(const string &radarName)
{
  STRncopy(_radar_info.radar_name, radarName.c_str(), IPS_TS_MAX_RADAR_NAME);
}

void IpsTsInfo::overrideSiteName(const string &siteName)
{
  STRncopy(_radar_info.site_name, siteName.c_str(), IPS_TS_MAX_SITE_NAME);
}

///////////////////////////////////////////////////////////
// override radar location

void IpsTsInfo::overrideRadarLocation(double altitudeMeters,
                                      double latitudeDeg,
                                      double longitudeDeg)
  
{
  
  _radar_info.altitude_m = altitudeMeters;
  _radar_info.latitude_deg = latitudeDeg;
  _radar_info.longitude_deg = longitudeDeg;

}

///////////////////////////////////////////////////////////
// override gate geometry

void IpsTsInfo::overrideGateGeometry(double startRangeMeters,
                                     double gateSpacingMeters)
  
{
  
  _proc.start_range_m = startRangeMeters;
  _proc.gate_spacing_m = gateSpacingMeters; 

}

///////////////////////////////////////////////////////////
// override wavelength

void IpsTsInfo::overrideWavelength(double wavelengthCm)
  
{
  
  _radar_info.wavelength_cm = wavelengthCm;
  _calib.wavelength_cm = wavelengthCm;

}

///////////////////////////////////////////////////////////
// is info ready to be used?
 
bool IpsTsInfo::isEssentialInfoReady() const
{

  if (_radar_info_active && _proc_active) {
    return true;
  }

  if (_debug != IpsTsDebug_t::OFF) {
    if (_debugPrintCount % 2500 == 0) {
      if (!_radar_info_active) {
        cerr << "INFO - IpsTsInfo::isEssentialInfoReady()" << endl;
        cerr << "  Waiting for IPS_TS_RADAR_INFO packet" << endl;
      }
      if (!_proc_active) {
        cerr << "INFO - IpsTsInfo::isEssentialInfoReady()" << endl;
        cerr << "  Waiting for IPS_TS_PROCESSING packet" << endl;
      }
    }
    _debugPrintCount++;
  }
   
  return false;
   
}

///////////////////////////////////////////////////////////
// is this an info packet? Check the id

bool IpsTsInfo::isInfo(int id)
  
{

  switch (id) {
    case IPS_TS_SYNC_ID:
    case IPS_TS_RADAR_INFO_ID:
    case IPS_TS_SCAN_SEGMENT_ID:
    case IPS_TS_PROCESSING_ID:
    case IPS_TS_STATUS_XML_ID:
    case IPS_TS_CALIBRATION_ID:
    case IPS_TS_EVENT_NOTICE_ID:
    case IPS_TS_PLATFORM_GEOREF_ID:
      return true;
  }
  return false;

}

////////////////////////////////////////////////////////////
// set info at the struct level

void IpsTsInfo::setRadarInfo(const ips_ts_radar_info_t &info,
                             bool addToMetaQueue /* = true */) {
  _radar_info = info;
  _radar_info.packet.id = IPS_TS_RADAR_INFO_ID;
  _radar_info.packet.len_bytes = sizeof(ips_ts_radar_info_t);
  _radar_info.packet.version_num = 1;
  if (addToMetaQueue) {
    // _addIdToQueue(IPS_TS_RADAR_INFO_ID);
    _addMetaToQueue(sizeof(_radar_info), &_radar_info);
  }
}

void IpsTsInfo::setScanSegment(const ips_ts_scan_segment_t &seg,
                               bool addToMetaQueue /* = true */) {
  _scan_seg = seg;
  _scan_seg.packet.id = IPS_TS_SCAN_SEGMENT_ID;
  _scan_seg.packet.len_bytes = sizeof(ips_ts_scan_segment_t);
  _scan_seg.packet.version_num = 1;
  if (addToMetaQueue) {
    // _addIdToQueue(IPS_TS_SCAN_SEGMENT_ID);
    _addMetaToQueue(sizeof(_scan_seg), &_scan_seg);
  }
}

void IpsTsInfo::setTsProcessing(const ips_ts_processing_t &proc,
                                bool addToMetaQueue /* = true */) {
  _proc = proc;
  if (std::isnan(_proc.start_range_m)) {
    _proc.start_range_m = 0.0;
  }
  _proc.packet.id = IPS_TS_PROCESSING_ID;
  _proc.packet.len_bytes = sizeof(ips_ts_processing_t);
  _proc.packet.version_num = 1;
  if (addToMetaQueue) {
    // _addIdToQueue(IPS_TS_PROCESSING_ID);
    _addMetaToQueue(sizeof(_proc), &_proc);
  }
}

void IpsTsInfo::setStatusXml(const ips_ts_status_xml_t &hdr,
                             const string &xmlStr,
                             bool addToMetaQueue /* = true */) {
  _status_xml_hdr = hdr;
  _status_xml_str = xmlStr;
  _status_xml_hdr.packet.id = IPS_TS_STATUS_XML_ID;
  _status_xml_hdr.packet.len_bytes =
    sizeof(ips_ts_status_xml_t) + xmlStr.size() + 1;
  _status_xml_hdr.packet.version_num = 1;
  if (addToMetaQueue) {
    // _addIdToQueue(IPS_TS_STATUS_XML_ID);
    MemBuf buf;
    buf.add(&_status_xml_hdr, sizeof(_status_xml_hdr));
    buf.add(xmlStr.c_str(), xmlStr.size() + 1);
    _addMetaToQueue(buf.getLen(), buf.getPtr());
  }
}

void IpsTsInfo::setCalibration(const ips_ts_calibration_t &calib,
                               bool addToMetaQueue /* = true */) {
  _calib = calib;
  _calib.packet.id = IPS_TS_CALIBRATION_ID;
  _calib.packet.len_bytes = sizeof(ips_ts_calibration_t);
  _calib.packet.version_num = 1;
  if (addToMetaQueue) {
    // _addIdToQueue(IPS_TS_CALIBRATION_ID);
    _addMetaToQueue(sizeof(_calib), &_calib);
  }
}

void IpsTsInfo::setEventNotice(const ips_ts_event_notice_t &enotice,
                               bool addToMetaQueue /* = true */) {
  _enotice = enotice;
  _enotice.packet.id = IPS_TS_EVENT_NOTICE_ID;
  _enotice.packet.len_bytes = sizeof(ips_ts_event_notice_t);
  _enotice.packet.version_num = 1;
  if (addToMetaQueue) {
    // _addIdToQueue(IPS_TS_EVENT_NOTICE_ID);
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

void IpsTsInfo::setPlatformGeoref
  (const ips_ts_platform_georef_t &platform_georef,
   bool addToMetaQueue /* = true */) {
  if (platform_georef.unit_num == 1) {
    _platform_georef1 = platform_georef;
    _platform_georef1.packet.id = IPS_TS_PLATFORM_GEOREF_ID;
    _platform_georef1.packet.len_bytes = sizeof(ips_ts_platform_georef_t);
    _platform_georef1.packet.version_num = 1;
    setPlatformGeoref1Active(true);
    if (addToMetaQueue) {
      _addMetaToQueue(sizeof(_platform_georef1), &_platform_georef1);
    }
  } else {
    _platform_georef0 = platform_georef;
    _platform_georef0.packet.id = IPS_TS_PLATFORM_GEOREF_ID;
    _platform_georef0.packet.len_bytes = sizeof(ips_ts_platform_georef_t);
    _platform_georef0.packet.version_num = 1;
    setPlatformGeorefActive(true);
    if (addToMetaQueue) {
      _addMetaToQueue(sizeof(_platform_georef0), &_platform_georef0);
    }
  }
}

////////////////////////////////////////////////////////////
// activate structs

void IpsTsInfo::setRadarInfoActive(bool state) {
  _radar_info_active = state;
}

void IpsTsInfo::setScanSegmentActive(bool state) {
  _scan_seg_active = state;
}

void IpsTsInfo::setTsProcessingActive(bool state) {
  _proc_active = state;
}

void IpsTsInfo::setStatusXmlActive(bool state) {
  _status_xml_active = state;
}

void IpsTsInfo::setCalibrationActive(bool state) {
  _calib_active = state;
}

void IpsTsInfo::setEventNoticeActive(bool state) {
  _enotice_active = state;
}

void IpsTsInfo::setPlatformGeorefActive(bool state) {
  _platform_georef0_active = state;
}

void IpsTsInfo::setPlatformGeoref1Active(bool state) {
  _platform_georef1_active = state;
}

////////////////////////////////////////////////////////////
// set sequence number for each packet type

void IpsTsInfo::setRadarInfoPktSeqNum(si64 pkt_seq_num) {
  _radar_info.packet.seq_num = pkt_seq_num;
}

void IpsTsInfo::setScanSegmentPktSeqNum(si64 pkt_seq_num) {
  _scan_seg.packet.seq_num = pkt_seq_num;
}

void IpsTsInfo::setTsProcessingPktSeqNum(si64 pkt_seq_num) {
  _proc.packet.seq_num = pkt_seq_num;
}

void IpsTsInfo::setStatusXmlPktSeqNum(si64 pkt_seq_num) {
  _status_xml_hdr.packet.seq_num = pkt_seq_num;
}

void IpsTsInfo::setCalibrationPktSeqNum(si64 pkt_seq_num) {
  _calib.packet.seq_num = pkt_seq_num;
}

void IpsTsInfo::setEventNoticePktSeqNum(si64 pkt_seq_num) {
  _enotice.packet.seq_num = pkt_seq_num;
}

void IpsTsInfo::setPlatformGeorefPktSeqNum(si64 pkt_seq_num) {
  _platform_georef0.packet.seq_num = pkt_seq_num;
}

void IpsTsInfo::setPlatformGeoref1PktSeqNum(si64 pkt_seq_num) {
  _platform_georef1.packet.seq_num = pkt_seq_num;
}

////////////////////////////////////////////////////////////
// set time on all packets

void IpsTsInfo::setTime(time_t secs, int nano_secs) {
  ips_ts_set_packet_time(_radar_info.packet, secs, nano_secs);
  ips_ts_set_packet_time(_scan_seg.packet, secs, nano_secs);
  ips_ts_set_packet_time(_proc.packet, secs, nano_secs);
  ips_ts_set_packet_time(_status_xml_hdr.packet, secs, nano_secs);
  ips_ts_set_packet_time(_calib.packet, secs, nano_secs);
  ips_ts_set_packet_time(_enotice.packet, secs, nano_secs);
  if (_platform_georef0_active) {
    ips_ts_set_packet_time(_platform_georef0.packet, secs, nano_secs);
  }
  if (_platform_georef1_active) {
    ips_ts_set_packet_time(_platform_georef1.packet, secs, nano_secs);
  }
}

////////////////////////////////////////////////////////////
// set time to now on all packets

void IpsTsInfo::setTimeToNow() {
  struct timeval time;
  gettimeofday(&time, NULL);
  setTime(time.tv_sec, time.tv_usec * 1000);
}

////////////////////////////////////////////////////////////
// set time for each packet

void IpsTsInfo::setRadarInfoTime(time_t secs, int nano_secs) {
  ips_ts_set_packet_time(_radar_info.packet, secs, nano_secs);
}

void IpsTsInfo::setScanSegmentTime(time_t secs, int nano_secs) {
  ips_ts_set_packet_time(_scan_seg.packet, secs, nano_secs);
}

void IpsTsInfo::setTsProcessingTime(time_t secs, int nano_secs) {
  ips_ts_set_packet_time(_proc.packet, secs, nano_secs);
}

void IpsTsInfo::setStatusXmlTime(time_t secs, int nano_secs) {
  ips_ts_set_packet_time(_status_xml_hdr.packet, secs, nano_secs);
}

void IpsTsInfo::setCalibrationTime(time_t secs, int nano_secs) {
  ips_ts_set_packet_time(_calib.packet, secs, nano_secs);
}

void IpsTsInfo::setEventNoticeTime(time_t secs, int nano_secs) {
  ips_ts_set_packet_time(_enotice.packet, secs, nano_secs);
}

void IpsTsInfo::setPlatformGeorefTime(time_t secs, int nano_secs) {
  ips_ts_set_packet_time(_platform_georef0.packet, secs, nano_secs);
}

void IpsTsInfo::setPlatformGeoref1Time(time_t secs, int nano_secs) {
  ips_ts_set_packet_time(_platform_georef1.packet, secs, nano_secs);
}

////////////////////////////////////////////////////////////
// set time to now for each packet

void IpsTsInfo::setRadarInfoTimeToNow() {
  ips_ts_set_packet_time_to_now(_radar_info.packet);
}

void IpsTsInfo::setScanSegmentTimeToNow() {
  ips_ts_set_packet_time_to_now(_scan_seg.packet);
}

void IpsTsInfo::setTsProcessingTimeToNow() {
  ips_ts_set_packet_time_to_now(_proc.packet);
}

void IpsTsInfo::setStatusXmlTimeToNow() {
  ips_ts_set_packet_time_to_now(_status_xml_hdr.packet);
}

void IpsTsInfo::setCalibrationTimeToNow() {
  ips_ts_set_packet_time_to_now(_calib.packet);
}

void IpsTsInfo::setEventNoticeTimeToNow() {
  ips_ts_set_packet_time_to_now(_enotice.packet);
}

void IpsTsInfo::setPlatformGeorefTimeToNow() {
  ips_ts_set_packet_time_to_now(_platform_georef0.packet);
}

void IpsTsInfo::setPlatformGeoref1TimeToNow() {
  ips_ts_set_packet_time_to_now(_platform_georef1.packet);
}

////////////////////////////////////////////////////////////
// get time for each packet

double IpsTsInfo::getRadarInfoTime() const {
  return ips_ts_get_packet_time_as_double(_radar_info.packet);
}

double IpsTsInfo::getScanSegmentTime() const {
  return ips_ts_get_packet_time_as_double(_scan_seg.packet);
}

double IpsTsInfo::getTsProcessingTime() const {
  return ips_ts_get_packet_time_as_double(_proc.packet);
}

double IpsTsInfo::getStatusXmlTime() const {
  return ips_ts_get_packet_time_as_double(_status_xml_hdr.packet);
}

double IpsTsInfo::getCalibrationTime() const {
  return ips_ts_get_packet_time_as_double(_calib.packet);
}

double IpsTsInfo::getEventNoticeTime() const {
  return ips_ts_get_packet_time_as_double(_enotice.packet);
}

double IpsTsInfo::getPlatformGeorefTime() const {
  return ips_ts_get_packet_time_as_double(_platform_georef0.packet);
}

double IpsTsInfo::getPlatformGeoref1Time() const {
  return ips_ts_get_packet_time_as_double(_platform_georef1.packet);
}

////////////////////////////////////////////////////////////
// set radar ID on all packet types

void IpsTsInfo::setRadarId(int id) {

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
// set IpsTsCalib object from _calib

void IpsTsInfo::setIpsTsCalib(IpsTsCalib &ipsCalib) const

{
  if (_calib_active) {
    // only set it if _calib has been set
    ipsCalib.set(_calib);
  }
}

///////////////////////////////////////////////////////////////
// set _calib from IpsTsCalib object

void IpsTsInfo::setFromIpsTsCalib(const IpsTsCalib &ipsCalib)

{
  _calib = ipsCalib.getStruct();
}

////////////////////////////////////////////////////////////
// set methods for individual fields

void IpsTsInfo::setRadarName(const string &x) {
  STRncopy(_radar_info.radar_name, x.c_str(), IPS_TS_MAX_RADAR_NAME);
}

void IpsTsInfo::setSiteName(const string &x) { 
  STRncopy(_radar_info.site_name, x.c_str(), IPS_TS_MAX_SITE_NAME);
}

void IpsTsInfo::setScanSegmentName(const string &x) {
  STRncopy(_scan_seg.segment_name, x.c_str(), IPS_TS_MAX_SEGMENT_NAME);
}

void IpsTsInfo::setScanProjectName(const string &x) {
  STRncopy(_scan_seg.project_name, x.c_str(), IPS_TS_MAX_PROJECT_NAME);
}

void IpsTsInfo::setScanFixedAngle(int i, fl32 x) {
  if (i < IPS_TS_MAX_FIXED_ANGLES) {
    _scan_seg.fixed_angles[i] = x;
  }
}

////////////////////////////////////////////////////////////
// get methods for individual fields

string IpsTsInfo::getRadarName() const {
  return ips_ts_safe_str(_radar_info.radar_name, IPS_TS_MAX_RADAR_NAME);
}

string IpsTsInfo::getSiteName() const { 
  return ips_ts_safe_str(_radar_info.site_name, IPS_TS_MAX_SITE_NAME);
}

string IpsTsInfo::getScanSegmentName() const {
  return ips_ts_safe_str(_scan_seg.segment_name, IPS_TS_MAX_SEGMENT_NAME);
}

string IpsTsInfo::getScanProjectName() const {
  return ips_ts_safe_str(_scan_seg.project_name, IPS_TS_MAX_PROJECT_NAME);
}

fl32 IpsTsInfo::getScanFixedAngle(int i) const {
  if (i < IPS_TS_MAX_FIXED_ANGLES) {
    return _scan_seg.fixed_angles[i];
  } else {
    return -1;
  }
}

///////////////////////////////////////////////////////////
// print

// print everything

void IpsTsInfo::print(FILE *out) const
{
  
  fprintf(out, "******************** Start IpsTsInfo ***************************\n");

  if (_radar_info_active) {
    ips_ts_radar_info_print(out, _radar_info);
  }
  if (_scan_seg_active) {
    ips_ts_scan_segment_print(out, _scan_seg);
  }
  if (_proc_active) {
    ips_ts_processing_print(out, _proc);
  }
  if (_status_xml_active) {
    ips_ts_status_xml_print(out, _status_xml_hdr, _status_xml_str);
  }
  if (_calib_active) {
    ips_ts_calibration_print(out, _calib);
  }
  if (_enotice_active) {
    ips_ts_event_notice_print(out, _enotice);
  }
  if (_platform_georef0_active) {
    ips_ts_platform_georef_print(out, _platform_georef0);
  }
  if (_platform_georef1_active) {
    ips_ts_platform_georef_print(out, _platform_georef1);
  }

  fprintf(out, "********************* End IpsTsInfo ****************************\n");

}

// print only what is in the id queue
// clear the queue if clearQueue is set to true

void IpsTsInfo::printMetaQueue(FILE *out, bool clearQueue) const
{
  
  if (_metaQueue.size() == 0) {
    return;
  }

  if (_debug >= IpsTsDebug_t::VERBOSE) {
    cerr << "DEBUG - IpsTsInfo::printMetaQueue()" << endl;
    cerr << "  _metaQueue.size(): " << _metaQueue.size() << endl;
  }

  fprintf(out, "******************** Start IpsTsInfo ***************************\n");
  
  for (size_t ii = 0; ii < _metaQueue.size(); ii++) {
    MemBuf *buf = _metaQueue[ii];
    ips_ts_packet_print(out, buf->getPtr(), buf->getLen());
  } // ii

  fprintf(out, "********************* End IpsTsInfo ****************************\n");
  
  if (clearQueue) {
    _clearMetaQueue();
  }

}

///////////////////////////////////////////////////
// write a sync packet to file in IPS format
//
// returns 0 on success, -1 on failure

int IpsTsInfo::writeSyncToFile(FILE *out) const

{
  
  if (out == NULL) {
    return -1;
  }
  
  ips_ts_sync_t sync;
  ips_ts_sync_init(sync);
  if (fwrite(&sync, sizeof(sync), 1, out) != 1) {
    int errNum = errno;
    cerr << "ERROR - IpsTsInfo::writeSync2File" << endl;
    cerr << "  Cannot write sync packet" << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  
  return 0;

}

///////////////////////////////////////////////////
// write meta-data to file in IPS format
//
// Writes out any meta-data which has a sequence number
// later than the previous pulse written.
// To force a write, set the prevPulseSeqNum to 0.
//
// returns 0 on success, -1 on failure

int IpsTsInfo::writeMetaToFile(FILE *out, si64 prevPulseSeqNum) const

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
	cerr << "ERROR - IpsTsInfo::write2File" << endl;
	cerr << "  Cannot write _radar_info packet" << endl;
	cerr << "  " << strerror(errNum) << endl;
	return -1;
      }
    }

  }

  if (_scan_seg_active && _scan_seg.packet.seq_num > prevPulseSeqNum) {
    if (fwrite(&_scan_seg, sizeof(_scan_seg), 1, out) != 1) {
      int errNum = errno;
      cerr << "ERROR - IpsTsInfo::write2File" << endl;
      cerr << "  Cannot write _scan_seg packet" << endl;
      cerr << "  " << strerror(errNum) << endl;
      return -1;
    }
  }

  if (_proc_active && _proc.packet.seq_num > prevPulseSeqNum) {
    if (fwrite(&_proc, sizeof(_proc), 1, out) != 1) {
      int errNum = errno;
      cerr << "ERROR - IpsTsInfo::write2File" << endl;
      cerr << "  Cannot write _proc packet" << endl;
      cerr << "  " << strerror(errNum) << endl;
      return -1;
    }
  }

  if (_status_xml_active &&
      _status_xml_hdr.packet.seq_num > prevPulseSeqNum) {
    if (fwrite(&_status_xml_hdr, sizeof(_status_xml_hdr), 1, out) != 1) {
      int errNum = errno;
      cerr << "ERROR - IpsTsInfo::write2File" << endl;
      cerr << "  Cannot write _status_xml header" << endl;
      cerr << "  " << strerror(errNum) << endl;
      return -1;
    }
    if (fwrite(_status_xml_str.c_str(),
               _status_xml_str.size() + 1, 1, out) != 1) {
      int errNum = errno;
      cerr << "ERROR - IpsTsInfo::write2File" << endl;
      cerr << "  Cannot write _status_xml string" << endl;
      cerr << "  " << strerror(errNum) << endl;
      return -1;
    }
  }

  if (_calib_active && _calib.packet.seq_num > prevPulseSeqNum) {
    if (fwrite(&_calib, sizeof(_calib), 1, out) != 1) {
      int errNum = errno;
      cerr << "ERROR - IpsTsInfo::write2File" << endl;
      cerr << "  Cannot write _calib packet" << endl;
      cerr << "  " << strerror(errNum) << endl;
      return -1;
    }
  }

  if (_enotice_active && _enotice.packet.seq_num > prevPulseSeqNum) {
    if (fwrite(&_enotice, sizeof(_enotice), 1, out) != 1) {
      int errNum = errno;
      cerr << "ERROR - IpsTsInfo::write2File" << endl;
      cerr << "  Cannot write _enotice packet" << endl;
      cerr << "  " << strerror(errNum) << endl;
      return -1;
    }
  }

  if (_platform_georef0_active &&
      _platform_georef0.packet.seq_num > prevPulseSeqNum) {
    if (fwrite(&_platform_georef0, sizeof(_platform_georef0), 1, out) != 1) {
      int errNum = errno;
      cerr << "ERROR - IpsTsInfo::write2File" << endl;
      cerr << "  Cannot write _platform_georef0 packet" << endl;
      cerr << "  " << strerror(errNum) << endl;
      return -1;
    }
  }

  if (_platform_georef1_active &&
      _platform_georef1.packet.seq_num > prevPulseSeqNum) {
    if (fwrite(&_platform_georef1, sizeof(_platform_georef1), 1, out) != 1) {
      int errNum = errno;
      cerr << "ERROR - IpsTsInfo::write2File" << endl;
      cerr << "  Cannot write _platform_georef1 packet" << endl;
      cerr << "  " << strerror(errNum) << endl;
      return -1;
    }
  }

  return 0;

}

///////////////////////////////////////////////////
// write meta-data to file in IPS format
//
// Writes out any meta-data in the queue
// If clearQueue is true, the queue will be cleared.
//
// Returns 0 on success, -1 on failure

int IpsTsInfo::writeMetaQueueToFile(FILE *out, bool clearQueue) const

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

  if (_debug >= IpsTsDebug_t::VERBOSE) {
    cerr << "DEBUG - IpsTsInfo::writeMetaQueueToFile()" << endl;
    cerr << "  _metaQueue.size(): " << _metaQueue.size() << endl;
  }

  // write out the queue of metadata packets

  int iret = 0;
  for (size_t ii = 0; ii < _metaQueue.size(); ii++) {
    MemBuf *buf = _metaQueue[ii];
    if (fwrite(buf->getPtr(), buf->getLen(), 1, out) != 1) {
      int errNum = errno;
      cerr << "ERROR - IpsTsInfo::write2File" << endl;
      cerr << "  Cannot write metaData packet" << endl;
      cerr << "  " << strerror(errNum) << endl;
      ips_ts_packet_print(stderr, buf->getPtr(), buf->getLen());
      iret = -1;
    }
  } // ii

  if (clearQueue) {
    _clearMetaQueue();
  }

  return iret;

}

///////////////////////////////////////////////////
// add meta-data to DsMessage in IPS format
//
// Loads up any meta-data in the queue.
// If clearQueue is true, the queue will be cleared.

void IpsTsInfo::addMetaQueueToMsg(DsMessage &msg, bool clearQueue) const
  
{

  if (_debug >= IpsTsDebug_t::VERBOSE) {
    cerr << "DEBUG - IpsTsInfo::addMetaQueueToMsg()" << endl;
    cerr << "  _metaQueue.size(): " << _metaQueue.size() << endl;
  }

  for (size_t ii = 0; ii < _metaQueue.size(); ii++) {
    MemBuf *buf = _metaQueue[ii];
    int packetId = 0;
    if (ips_ts_get_packet_id(buf->getPtr(), buf->getLen(), packetId) == 0) {
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

// void IpsTsInfo::_addIdToQueue(si32 id)

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

void IpsTsInfo::_addMetaToQueue(size_t len, const void *packet)

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

MemBuf *IpsTsInfo::_popFrontFromMetaQueue()
  
{
  MemBuf *front = _metaQueue.front();
  _metaQueue.pop_front();
  return front;
}

/////////////////////////////////////////////
// clear the metadata queue - public

void IpsTsInfo::clearMetadataQueue()
  
{
  // _idQueue.clear();
  _clearMetaQueue();
}

/////////////////////////////////////////////
// clear the metadata queue - private

void IpsTsInfo::_clearMetaQueue() const
  
{
  for (size_t ii = 0; ii < _metaQueue.size(); ii++) {
    delete _metaQueue[ii];
  }
  _metaQueue.clear();
}

/////////////////////////////////////////////
// clear the event flags

void IpsTsInfo::clearEventFlags()
  
{

  _startOfSweepFlag = false;
  _endOfSweepFlag = false;
  _startOfVolumeFlag = false;
  _endOfVolumeFlag = false;

}

