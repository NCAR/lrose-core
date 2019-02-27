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
// IwrfTsInfo.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2007
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
#include <radar/IwrfTsInfo.hh>
#include <radar/RadarCalib.hh>
#include <rapformats/DsRadarCalib.hh>
using namespace std;

////////////////////////////////////////////////////
// Constructor

IwrfTsInfo::IwrfTsInfo(IwrfDebug_t debug) :
        _debug(debug)

{
  clear();
  _debugPrintCount = 0;
}

//////////////////////////////////////////////////////////////////
// destructor

IwrfTsInfo::~IwrfTsInfo()

{
  _clearMetaQueue();
}

//////////////////////////////////////////////////////////////////
// clear all

void IwrfTsInfo::clear()

{
  
  iwrf_radar_info_init(_radar_info);
  iwrf_scan_segment_init(_scan_seg);
  iwrf_antenna_correction_init(_ant_corr);
  iwrf_ts_processing_init(_proc);
  iwrf_xmit_power_init(_xmit_power);
  iwrf_rx_power_init(_rx_power);
  iwrf_xmit_sample_init(_xmit_sample);
  iwrf_status_xml_init(_status_xml_hdr);
  iwrf_calibration_init(_calib);
  iwrf_event_notice_init(_enotice);
  iwrf_phasecode_init(_phasecode);
  iwrf_xmit_info_init(_xmit_info);
  iwrf_rvp8_ops_info_init(_rvp8);
  _rvp8SaturationMult = pow(10.0, _rvp8.f_saturation_dbm / 20.0);
  iwrf_platform_georef_init(_platform_georef0);
  iwrf_platform_georef_init(_platform_georef1);

  _radar_info_active = false;
  _scan_seg_active = false;
  _ant_corr_active = false;
  _proc_active = false;
  _xmit_power_active = false;
  _rx_power_active = false;
  _xmit_sample_active = false;
  _status_xml_active = false;
  _calib_active = false;
  _enotice_active = false;
  _phasecode_active = false;
  _xmit_info_active = false;
  _platform_georef0_active = false;
  _platform_georef1_active = false;

  clearEventFlags();

}

///////////////////////////////////////////////////////////
// set structs from a generic buffer
// by checking for the id
// swaps as required

int IwrfTsInfo::setFromBuffer(const void *buf, int len)
  
{

  // check validity of packet
  
  int packet_id;
  if (iwrf_get_packet_id(buf, len, packet_id)) {
    cerr << "ERROR - IwrfTsInfo::setFromBuffer" << endl;
    fprintf(stderr, "  Bad packet, id: 0x%x\n", packet_id);
    cerr << "             len: " << len << endl;
    cerr << "            type: " << iwrf_packet_id_to_str(packet_id) << endl;
   return -1;
  }

  // swap packet as required, using a copy

  char *copy = new char[len];
  memcpy(copy, buf, len);
  iwrf_packet_swap(copy, len);

  switch (packet_id) {
    case IWRF_RADAR_INFO_ID: {
      setRadarInfo(*(iwrf_radar_info_t *) copy);
      setRadarInfoActive(true);
    } break;
    case IWRF_SCAN_SEGMENT_ID: {
      setScanSegment(*(iwrf_scan_segment_t *) copy);
      setScanSegmentActive(true);
    } break;
    case IWRF_ANTENNA_CORRECTION_ID: {
      setAntennaCorrection(*(iwrf_antenna_correction_t *) copy);
      setAntennaCorrectionActive(true);
    } break;
    case IWRF_TS_PROCESSING_ID: {
      setTsProcessing(*(iwrf_ts_processing_t *) copy);
      setTsProcessingActive(true);
    } break;
    case IWRF_XMIT_POWER_ID: {
      setXmitPower(*(iwrf_xmit_power_t *) copy);
      setXmitPowerActive(true);
    } break;
    case IWRF_RX_POWER_ID: {
      setRxPower(*(iwrf_rx_power_t *) copy);
      setRxPowerActive(true);
    } break;
    case IWRF_XMIT_SAMPLE_ID: {
      setXmitSample(*(iwrf_xmit_sample_t *) copy);
      setXmitSampleActive(true);
    } break;
    case IWRF_STATUS_XML_ID: {
      setStatusXml(*(iwrf_status_xml_t *) copy,
                   copy + sizeof(iwrf_status_xml_t));
      setStatusXmlActive(true);
    } break;
    case IWRF_CALIBRATION_ID: {
      setCalibration(*(iwrf_calibration_t *) copy);
      setCalibrationActive(true);
    } break;
    case IWRF_EVENT_NOTICE_ID: {
      setEventNotice(*(iwrf_event_notice_t *) copy);
      setEventNoticeActive(true);
    } break;
    case IWRF_PHASECODE_ID: {
      setPhasecode(*(iwrf_phasecode_t *) copy);
      setPhasecodeActive(true);
    } break;
    case IWRF_XMIT_INFO_ID: {
      setXmitInfo(*(iwrf_xmit_info_t *) copy);
      setXmitInfoActive(true);
    } break;
    case IWRF_RVP8_OPS_INFO_ID: {
      setRvp8Info(*(iwrf_rvp8_ops_info_t *) copy);
      setRvp8InfoActive(true);
    } break;
    case IWRF_PLATFORM_GEOREF_ID: {
      iwrf_platform_georef_t georef;
      memcpy(&georef, copy, sizeof(iwrf_platform_georef_t));
      iwrf_platform_georef_swap(georef);
      setPlatformGeoref(georef);
    } break;
    case IWRF_SYNC_ID:
    case IWRF_PULSE_HEADER_ID:
    case IWRF_RVP8_PULSE_HEADER_ID:
    case IWRF_MOMENTS_FIELD_HEADER_ID:
    case IWRF_MOMENTS_RAY_HEADER_ID:
    case IWRF_MOMENTS_FIELD_INDEX_ID:
    case IWRF_GEOREF_CORRECTION_ID:
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

void IwrfTsInfo::overrideRadarName(const string &radarName)
{
  STRncopy(_radar_info.radar_name, radarName.c_str(), IWRF_MAX_RADAR_NAME);
}

void IwrfTsInfo::overrideSiteName(const string &siteName)
{
  STRncopy(_radar_info.site_name, siteName.c_str(), IWRF_MAX_SITE_NAME);
}

///////////////////////////////////////////////////////////
// override radar location

void IwrfTsInfo::overrideRadarLocation(double altitudeMeters,
				       double latitudeDeg,
				       double longitudeDeg)
  
{
  
  _radar_info.altitude_m = altitudeMeters;
  _radar_info.latitude_deg = latitudeDeg;
  _radar_info.longitude_deg = longitudeDeg;

}

///////////////////////////////////////////////////////////
// override gate geometry

void IwrfTsInfo::overrideGateGeometry(double startRangeMeters,
				      double gateSpacingMeters)
  
{
  
  _proc.start_range_m = startRangeMeters;
  _proc.gate_spacing_m = gateSpacingMeters; 

}

///////////////////////////////////////////////////////////
// override wavelength

void IwrfTsInfo::overrideWavelength(double wavelengthCm)
  
{
  
  _radar_info.wavelength_cm = wavelengthCm;
  _calib.wavelength_cm = wavelengthCm;

}

///////////////////////////////////////////////////////////
// is info ready to be used?
 
 bool IwrfTsInfo::isEssentialInfoReady() const
 {

   if (_radar_info_active && _proc_active) {
     return true;
   }

   if (_debug) {
     if (_debugPrintCount % 2500 == 0) {
       if (!_radar_info_active) {
         cerr << "INFO - IwrfTsInfo::isEssentialInfoReady()" << endl;
         cerr << "  Waiting for IWRF_RADAR_INFO packet" << endl;
       }
       if (!_proc_active) {
         cerr << "INFO - IwrfTsInfo::isEssentialInfoReady()" << endl;
         cerr << "  Waiting for IWRF_TS_PROCESSING packet" << endl;
       }
     }
     _debugPrintCount++;
   }
   
   return false;
   
 }

///////////////////////////////////////////////////////////
// is this an info packet? Check the id

bool IwrfTsInfo::isInfo(int id)
  
{

  switch (id) {
    case IWRF_SYNC_ID:
    case IWRF_RADAR_INFO_ID:
    case IWRF_SCAN_SEGMENT_ID:
    case IWRF_ANTENNA_CORRECTION_ID:
    case IWRF_TS_PROCESSING_ID:
    case IWRF_XMIT_POWER_ID:
    case IWRF_RX_POWER_ID:
    case IWRF_XMIT_SAMPLE_ID:
    case IWRF_STATUS_XML_ID:
    case IWRF_CALIBRATION_ID:
    case IWRF_EVENT_NOTICE_ID:
    case IWRF_PHASECODE_ID:
    case IWRF_XMIT_INFO_ID:
    case IWRF_RVP8_OPS_INFO_ID:
    case IWRF_PLATFORM_GEOREF_ID:
      return true;
  }
  return false;

}

////////////////////////////////////////////////////////////
// set info at the struct level

void IwrfTsInfo::setRadarInfo(const iwrf_radar_info_t &info,
                              bool addToMetaQueue /* = true */) {
  _radar_info = info;
  _radar_info.packet.id = IWRF_RADAR_INFO_ID;
  _radar_info.packet.len_bytes = sizeof(iwrf_radar_info_t);
  _radar_info.packet.version_num = 1;
  if (addToMetaQueue) {
    // _addIdToQueue(IWRF_RADAR_INFO_ID);
    _addMetaToQueue(sizeof(_radar_info), &_radar_info);
  }
}

void IwrfTsInfo::setScanSegment(const iwrf_scan_segment_t &seg,
                                bool addToMetaQueue /* = true */) {
  _scan_seg = seg;
  _scan_seg.packet.id = IWRF_SCAN_SEGMENT_ID;
  _scan_seg.packet.len_bytes = sizeof(iwrf_scan_segment_t);
  _scan_seg.packet.version_num = 1;
  if (addToMetaQueue) {
    // _addIdToQueue(IWRF_SCAN_SEGMENT_ID);
    _addMetaToQueue(sizeof(_scan_seg), &_scan_seg);
  }
}

void IwrfTsInfo::setAntennaCorrection(const iwrf_antenna_correction_t &corr,
                                      bool addToMetaQueue /* = true */) {
  _ant_corr = corr;
  _ant_corr.packet.id = IWRF_ANTENNA_CORRECTION_ID;
  _ant_corr.packet.len_bytes = sizeof(iwrf_antenna_correction_t);
  _ant_corr.packet.version_num = 1;
  if (addToMetaQueue) {
    // _addIdToQueue(IWRF_ANTENNA_CORRECTION_ID);
    _addMetaToQueue(sizeof(_ant_corr), &_ant_corr);
  }
}

void IwrfTsInfo::setTsProcessing(const iwrf_ts_processing_t &proc,
                                 bool addToMetaQueue /* = true */) {
  _proc = proc;
  if (isnan(_proc.start_range_m)) {
    _proc.start_range_m = 0.0;
  }
  _proc.packet.id = IWRF_TS_PROCESSING_ID;
  _proc.packet.len_bytes = sizeof(iwrf_ts_processing_t);
  _proc.packet.version_num = 1;
  if (addToMetaQueue) {
    // _addIdToQueue(IWRF_TS_PROCESSING_ID);
    _addMetaToQueue(sizeof(_proc), &_proc);
  }
}

void IwrfTsInfo::setXmitPower(const iwrf_xmit_power_t &power,
                              bool addToMetaQueue /* = true */) {
  _xmit_power = power;
  _xmit_power.packet.id = IWRF_XMIT_POWER_ID;
  _xmit_power.packet.len_bytes = sizeof(iwrf_xmit_power_t);
  _xmit_power.packet.version_num = 1;
  if (addToMetaQueue) {
    // _addIdToQueue(IWRF_XMIT_POWER_ID);
    _addMetaToQueue(sizeof(_xmit_power), &_xmit_power);
  }
}

void IwrfTsInfo::setRxPower(const iwrf_rx_power_t &power,
                              bool addToMetaQueue /* = true */) {
  _rx_power = power;
  _rx_power.packet.id = IWRF_RX_POWER_ID;
  _rx_power.packet.len_bytes = sizeof(iwrf_rx_power_t);
  _rx_power.packet.version_num = 1;
  if (addToMetaQueue) {
    // _addIdToQueue(IWRF_RX_POWER_ID);
    _addMetaToQueue(sizeof(_rx_power), &_rx_power);
  }
}

void IwrfTsInfo::setXmitSample(const iwrf_xmit_sample_t &sample,
                               bool addToMetaQueue /* = true */) {
  _xmit_sample = sample;
  _xmit_sample.packet.id = IWRF_XMIT_SAMPLE_ID;
  _xmit_sample.packet.len_bytes = sizeof(iwrf_xmit_sample_t);
  _xmit_sample.packet.version_num = 1;
  if (addToMetaQueue) {
    // _addIdToQueue(IWRF_XMIT_SAMPLE_ID);
    _addMetaToQueue(sizeof(_xmit_sample), &_xmit_sample);
  }
}

void IwrfTsInfo::setStatusXml(const iwrf_status_xml_t &hdr,
                              const string &xmlStr,
                              bool addToMetaQueue /* = true */) {
  _status_xml_hdr = hdr;
  _status_xml_str = xmlStr;
  _status_xml_hdr.packet.id = IWRF_STATUS_XML_ID;
  _status_xml_hdr.packet.len_bytes =
    sizeof(iwrf_status_xml_t) + xmlStr.size() + 1;
  _status_xml_hdr.packet.version_num = 1;
  if (addToMetaQueue) {
    // _addIdToQueue(IWRF_STATUS_XML_ID);
    MemBuf buf;
    buf.add(&_status_xml_hdr, sizeof(_status_xml_hdr));
    buf.add(xmlStr.c_str(), xmlStr.size() + 1);
    _addMetaToQueue(buf.getLen(), buf.getPtr());
  }
}

void IwrfTsInfo::setCalibration(const iwrf_calibration_t &calib,
                                bool addToMetaQueue /* = true */) {
  _calib = calib;
  _calib.packet.id = IWRF_CALIBRATION_ID;
  _calib.packet.len_bytes = sizeof(iwrf_calibration_t);
  _calib.packet.version_num = 1;
  if (addToMetaQueue) {
    // _addIdToQueue(IWRF_CALIBRATION_ID);
    _addMetaToQueue(sizeof(_calib), &_calib);
  }
}

void IwrfTsInfo::setEventNotice(const iwrf_event_notice_t &enotice,
                                bool addToMetaQueue /* = true */) {
  _enotice = enotice;
  _enotice.packet.id = IWRF_EVENT_NOTICE_ID;
  _enotice.packet.len_bytes = sizeof(iwrf_event_notice_t);
  _enotice.packet.version_num = 1;
  if (addToMetaQueue) {
    // _addIdToQueue(IWRF_EVENT_NOTICE_ID);
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

void IwrfTsInfo::setPhasecode(const iwrf_phasecode_t &phasecode,
                              bool addToMetaQueue /* = true */) {
  _phasecode = phasecode;
  _phasecode.packet.id = IWRF_PHASECODE_ID;
  _phasecode.packet.len_bytes = sizeof(iwrf_phasecode_t);
  _phasecode.packet.version_num = 1;
  if (addToMetaQueue) {
    // _addIdToQueue(IWRF_PHASECODE_ID);
    _addMetaToQueue(sizeof(_phasecode), &_phasecode);
  }
}

void IwrfTsInfo::setXmitInfo(const iwrf_xmit_info_t &info,
                             bool addToMetaQueue /* = true */) {
  _xmit_info = info;
  _xmit_info.packet.id = IWRF_XMIT_INFO_ID;
  _xmit_info.packet.len_bytes = sizeof(iwrf_xmit_info_t);
  _xmit_info.packet.version_num = 1;
  if (addToMetaQueue) {
    // _addIdToQueue(IWRF_XMIT_INFO_ID);
    _addMetaToQueue(sizeof(info), &info);
  }
}

void IwrfTsInfo::setRvp8Info(const iwrf_rvp8_ops_info_t &info,
                             bool addToMetaQueue /* = true */) { 
  _rvp8 = info;
  _rvp8.packet.id = IWRF_RVP8_OPS_INFO_ID;
  _rvp8.packet.len_bytes = sizeof(iwrf_rvp8_ops_info_t);
  _rvp8.packet.version_num = 1;
  if (addToMetaQueue) {
    // _addIdToQueue(IWRF_RVP8_OPS_INFO_ID);
    _addMetaToQueue(sizeof(info), &info);
  }
}

void IwrfTsInfo::setPlatformGeoref
  (const iwrf_platform_georef_t &platform_georef,
   bool addToMetaQueue /* = true */) {
  if (platform_georef.unit_num == 1) {
    _platform_georef1 = platform_georef;
    _platform_georef1.packet.id = IWRF_PLATFORM_GEOREF_ID;
    _platform_georef1.packet.len_bytes = sizeof(iwrf_platform_georef_t);
    _platform_georef1.packet.version_num = 1;
    setPlatformGeoref1Active(true);
    if (addToMetaQueue) {
      _addMetaToQueue(sizeof(_platform_georef1), &_platform_georef1);
    }
  } else {
    _platform_georef0 = platform_georef;
    _platform_georef0.packet.id = IWRF_PLATFORM_GEOREF_ID;
    _platform_georef0.packet.len_bytes = sizeof(iwrf_platform_georef_t);
    _platform_georef0.packet.version_num = 1;
    setPlatformGeorefActive(true);
    if (addToMetaQueue) {
      _addMetaToQueue(sizeof(_platform_georef0), &_platform_georef0);
    }
  }
}

////////////////////////////////////////////////////////////
// activate structs

void IwrfTsInfo::setRadarInfoActive(bool state) {
  _radar_info_active = state;
}

void IwrfTsInfo::setScanSegmentActive(bool state) {
  _scan_seg_active = state;
}

void IwrfTsInfo::setAntennaCorrectionActive(bool state) {
  _ant_corr_active = state;
}

void IwrfTsInfo::setTsProcessingActive(bool state) {
  _proc_active = state;
}

void IwrfTsInfo::setXmitPowerActive(bool state) {
  _xmit_power_active = state;
}

void IwrfTsInfo::setRxPowerActive(bool state) {
  _rx_power_active = state;
}

void IwrfTsInfo::setXmitSampleActive(bool state) {
  _xmit_sample_active = state;
}

void IwrfTsInfo::setStatusXmlActive(bool state) {
  _status_xml_active = state;
}

void IwrfTsInfo::setCalibrationActive(bool state) {
  _calib_active = state;
}

void IwrfTsInfo::setEventNoticeActive(bool state) {
  _enotice_active = state;
}

void IwrfTsInfo::setPhasecodeActive(bool state) {
  _phasecode_active = state;
}

void IwrfTsInfo::setXmitInfoActive(bool state) {
  _xmit_info_active = state;
}

void IwrfTsInfo::setRvp8InfoActive(bool state) {
  _rvp8_active = state;
}

void IwrfTsInfo::setPlatformGeorefActive(bool state) {
  _platform_georef0_active = state;
}

void IwrfTsInfo::setPlatformGeoref1Active(bool state) {
  _platform_georef1_active = state;
}

////////////////////////////////////////////////////////////
// set sequence number for each packet type

void IwrfTsInfo::setRadarInfoPktSeqNum(si64 pkt_seq_num) {
  _radar_info.packet.seq_num = pkt_seq_num;
}

void IwrfTsInfo::setScanSegmentPktSeqNum(si64 pkt_seq_num) {
  _scan_seg.packet.seq_num = pkt_seq_num;
}

void IwrfTsInfo::setAntennaCorrectionPktSeqNum(si64 pkt_seq_num) {
  _ant_corr.packet.seq_num = pkt_seq_num;
}

void IwrfTsInfo::setTsProcessingPktSeqNum(si64 pkt_seq_num) {
  _proc.packet.seq_num = pkt_seq_num;
}

void IwrfTsInfo::setXmitPowerPktSeqNum(si64 pkt_seq_num) {
  _xmit_power.packet.seq_num = pkt_seq_num;
}

void IwrfTsInfo::setRxPowerPktSeqNum(si64 pkt_seq_num) {
  _rx_power.packet.seq_num = pkt_seq_num;
}

void IwrfTsInfo::setXmitSamplePktSeqNum(si64 pkt_seq_num) {
  _xmit_sample.packet.seq_num = pkt_seq_num;
}

void IwrfTsInfo::setStatusXmlPktSeqNum(si64 pkt_seq_num) {
  _status_xml_hdr.packet.seq_num = pkt_seq_num;
}

void IwrfTsInfo::setCalibrationPktSeqNum(si64 pkt_seq_num) {
  _calib.packet.seq_num = pkt_seq_num;
}

void IwrfTsInfo::setEventNoticePktSeqNum(si64 pkt_seq_num) {
  _enotice.packet.seq_num = pkt_seq_num;
}

void IwrfTsInfo::setPhasecodePktSeqNum(si64 pkt_seq_num) {
  _phasecode.packet.seq_num = pkt_seq_num;
}

void IwrfTsInfo::setXmitInfoPktSeqNum(si64 pkt_seq_num) {
  _xmit_info.packet.seq_num = pkt_seq_num;
}

void IwrfTsInfo::setRvp8InfoPktSeqNum(si64 pkt_seq_num) {
  _rvp8.packet.seq_num = pkt_seq_num;
}

void IwrfTsInfo::setPlatformGeorefPktSeqNum(si64 pkt_seq_num) {
  _platform_georef0.packet.seq_num = pkt_seq_num;
}

void IwrfTsInfo::setPlatformGeoref1PktSeqNum(si64 pkt_seq_num) {
  _platform_georef1.packet.seq_num = pkt_seq_num;
}

////////////////////////////////////////////////////////////
// set time on all packets

void IwrfTsInfo::setTime(time_t secs, int nano_secs) {
  iwrf_set_packet_time(_radar_info.packet, secs, nano_secs);
  iwrf_set_packet_time(_scan_seg.packet, secs, nano_secs);
  iwrf_set_packet_time(_ant_corr.packet, secs, nano_secs);
  iwrf_set_packet_time(_proc.packet, secs, nano_secs);
  iwrf_set_packet_time(_xmit_power.packet, secs, nano_secs);
  iwrf_set_packet_time(_rx_power.packet, secs, nano_secs);
  iwrf_set_packet_time(_xmit_sample.packet, secs, nano_secs);
  iwrf_set_packet_time(_status_xml_hdr.packet, secs, nano_secs);
  iwrf_set_packet_time(_calib.packet, secs, nano_secs);
  iwrf_set_packet_time(_enotice.packet, secs, nano_secs);
  iwrf_set_packet_time(_phasecode.packet, secs, nano_secs);
  iwrf_set_packet_time(_xmit_info.packet, secs, nano_secs);
  iwrf_set_packet_time(_rvp8.packet, secs, nano_secs);
  if (_platform_georef0_active) {
    iwrf_set_packet_time(_platform_georef0.packet, secs, nano_secs);
  }
  if (_platform_georef1_active) {
    iwrf_set_packet_time(_platform_georef1.packet, secs, nano_secs);
  }
}

////////////////////////////////////////////////////////////
// set time to now on all packets

void IwrfTsInfo::setTimeToNow() {
  struct timeval time;
  gettimeofday(&time, NULL);
  setTime(time.tv_sec, time.tv_usec * 1000);
}

////////////////////////////////////////////////////////////
// set time for each packet

void IwrfTsInfo::setRadarInfoTime(time_t secs, int nano_secs) {
  iwrf_set_packet_time(_radar_info.packet, secs, nano_secs);
}

void IwrfTsInfo::setScanSegmentTime(time_t secs, int nano_secs) {
  iwrf_set_packet_time(_scan_seg.packet, secs, nano_secs);
}

void IwrfTsInfo::setAntennaCorrectionTime(time_t secs, int nano_secs) {
  iwrf_set_packet_time(_ant_corr.packet, secs, nano_secs);
}

void IwrfTsInfo::setTsProcessingTime(time_t secs, int nano_secs) {
  iwrf_set_packet_time(_proc.packet, secs, nano_secs);
}

void IwrfTsInfo::setXmitPowerTime(time_t secs, int nano_secs) {
  iwrf_set_packet_time(_xmit_power.packet, secs, nano_secs);
}

void IwrfTsInfo::setRxPowerTime(time_t secs, int nano_secs) {
  iwrf_set_packet_time(_rx_power.packet, secs, nano_secs);
}

void IwrfTsInfo::setXmitSampleTime(time_t secs, int nano_secs) {
  iwrf_set_packet_time(_xmit_sample.packet, secs, nano_secs);
}

void IwrfTsInfo::setStatusXmlTime(time_t secs, int nano_secs) {
  iwrf_set_packet_time(_status_xml_hdr.packet, secs, nano_secs);
}

void IwrfTsInfo::setCalibrationTime(time_t secs, int nano_secs) {
  iwrf_set_packet_time(_calib.packet, secs, nano_secs);
}

void IwrfTsInfo::setEventNoticeTime(time_t secs, int nano_secs) {
  iwrf_set_packet_time(_enotice.packet, secs, nano_secs);
}

void IwrfTsInfo::setPhasecodeTime(time_t secs, int nano_secs) {
  iwrf_set_packet_time(_phasecode.packet, secs, nano_secs);
}

void IwrfTsInfo::setXmitInfoTime(time_t secs, int nano_secs) {
  iwrf_set_packet_time(_xmit_info.packet, secs, nano_secs);
}

void IwrfTsInfo::setRvp8InfoTime(time_t secs, int nano_secs) {
  iwrf_set_packet_time(_rvp8.packet, secs, nano_secs);
}

void IwrfTsInfo::setPlatformGeorefTime(time_t secs, int nano_secs) {
  iwrf_set_packet_time(_platform_georef0.packet, secs, nano_secs);
}

void IwrfTsInfo::setPlatformGeoref1Time(time_t secs, int nano_secs) {
  iwrf_set_packet_time(_platform_georef1.packet, secs, nano_secs);
}

////////////////////////////////////////////////////////////
// set time to now for each packet

void IwrfTsInfo::setRadarInfoTimeToNow() {
  iwrf_set_packet_time_to_now(_radar_info.packet);
}

void IwrfTsInfo::setScanSegmentTimeToNow() {
  iwrf_set_packet_time_to_now(_scan_seg.packet);
}

void IwrfTsInfo::setAntennaCorrectionTimeToNow() {
  iwrf_set_packet_time_to_now(_ant_corr.packet);
}

void IwrfTsInfo::setTsProcessingTimeToNow() {
  iwrf_set_packet_time_to_now(_proc.packet);
}

void IwrfTsInfo::setXmitPowerTimeToNow() {
  iwrf_set_packet_time_to_now(_xmit_power.packet);
}

void IwrfTsInfo::setRxPowerTimeToNow() {
  iwrf_set_packet_time_to_now(_rx_power.packet);
}

void IwrfTsInfo::setXmitSampleTimeToNow() {
  iwrf_set_packet_time_to_now(_xmit_sample.packet);
}

void IwrfTsInfo::setStatusXmlTimeToNow() {
  iwrf_set_packet_time_to_now(_status_xml_hdr.packet);
}

void IwrfTsInfo::setCalibrationTimeToNow() {
  iwrf_set_packet_time_to_now(_calib.packet);
}

void IwrfTsInfo::setEventNoticeTimeToNow() {
  iwrf_set_packet_time_to_now(_enotice.packet);
}

void IwrfTsInfo::setPhasecodeTimeToNow() {
  iwrf_set_packet_time_to_now(_phasecode.packet);
}

void IwrfTsInfo::setXmitInfoTimeToNow() {
  iwrf_set_packet_time_to_now(_xmit_info.packet);
}

void IwrfTsInfo::setRvp8InfoTimeToNow() {
  iwrf_set_packet_time_to_now(_rvp8.packet);
}

void IwrfTsInfo::setPlatformGeorefTimeToNow() {
  iwrf_set_packet_time_to_now(_platform_georef0.packet);
}

void IwrfTsInfo::setPlatformGeoref1TimeToNow() {
  iwrf_set_packet_time_to_now(_platform_georef1.packet);
}

////////////////////////////////////////////////////////////
// get time for each packet

double IwrfTsInfo::getRadarInfoTime() const {
  return iwrf_get_packet_time_as_double(_radar_info.packet);
}

double IwrfTsInfo::getScanSegmentTime() const {
  return iwrf_get_packet_time_as_double(_scan_seg.packet);
}

double IwrfTsInfo::getAntennaCorrectionTime() const {
  return iwrf_get_packet_time_as_double(_ant_corr.packet);
}

double IwrfTsInfo::getTsProcessingTime() const {
  return iwrf_get_packet_time_as_double(_proc.packet);
}

double IwrfTsInfo::getXmitPowerTime() const {
  return iwrf_get_packet_time_as_double(_xmit_power.packet);
}

double IwrfTsInfo::getRxPowerTime() const {
  return iwrf_get_packet_time_as_double(_rx_power.packet);
}

double IwrfTsInfo::getXmitSampleTime() const {
  return iwrf_get_packet_time_as_double(_xmit_sample.packet);
}

double IwrfTsInfo::getStatusXmlTime() const {
  return iwrf_get_packet_time_as_double(_status_xml_hdr.packet);
}

double IwrfTsInfo::getCalibrationTime() const {
  return iwrf_get_packet_time_as_double(_calib.packet);
}

double IwrfTsInfo::getEventNoticeTime() const {
  return iwrf_get_packet_time_as_double(_enotice.packet);
}

double IwrfTsInfo::getPhasecodeTime() const {
  return iwrf_get_packet_time_as_double(_phasecode.packet);
}

double IwrfTsInfo::getXmitInfoTime() const {
  return iwrf_get_packet_time_as_double(_xmit_info.packet);
}

double IwrfTsInfo::getRvp8InfoTime() const {
  return iwrf_get_packet_time_as_double(_rvp8.packet);
}

double IwrfTsInfo::getPlatformGeorefTime() const {
  return iwrf_get_packet_time_as_double(_platform_georef0.packet);
}

double IwrfTsInfo::getPlatformGeoref1Time() const {
  return iwrf_get_packet_time_as_double(_platform_georef1.packet);
}

////////////////////////////////////////////////////////////
// set radar ID on all packet types

void IwrfTsInfo::setRadarId(int id) {

  _radar_info.packet.radar_id = id;
  _scan_seg.packet.radar_id = id;
  _ant_corr.packet.radar_id = id;
  _proc.packet.radar_id = id;
  _xmit_power.packet.radar_id = id;
  _xmit_sample.packet.radar_id = id;
  _status_xml_hdr.packet.radar_id = id;
  _calib.packet.radar_id = id;
  _enotice.packet.radar_id = id;
  _phasecode.packet.radar_id = id;
  _xmit_info.packet.radar_id = id;
  _rvp8.packet.radar_id = id;
  _platform_georef0.packet.radar_id = id;
  _platform_georef1.packet.radar_id = id;

}

///////////////////////////////////////////////////////////////
// set IwrfCalib object from _calib

void IwrfTsInfo::setIwrfCalib(IwrfCalib &iwrfCalib) const

{
  if (_calib_active) {
    // only set it if _calib has been set
    iwrfCalib.set(_calib);
  }
}

///////////////////////////////////////////////////////////////
// set _calib from IwrfCalib object

void IwrfTsInfo::setFromIwrfCalib(const IwrfCalib &iwrfCalib)

{
  _calib = iwrfCalib.getStruct();
}

///////////////////////////////////////////////////////////////
// set DsRadarCalib object from _calib

void IwrfTsInfo::setDsRadarCalib(DsRadarCalib &dsCalib) const

{
  IwrfCalib iwrfCalib;
  iwrfCalib.set(_calib);
  RadarCalib::copyIwrfToDsRadar(iwrfCalib, dsCalib);
  dsCalib.setRadarName(_radar_info.radar_name);
}

///////////////////////////////////////////////////////////////
// set _calib from DsRadarCalib object

void IwrfTsInfo::setFromDsRadarCalib(const DsRadarCalib &dsCalib)

{
  set_radar_name(dsCalib.getRadarName());
  IwrfCalib iwrfCalib;
  RadarCalib::copyDsRadarToIwrf(dsCalib, iwrfCalib);
  _calib = iwrfCalib.getStruct();
}

////////////////////////////////////////////////////////////
// set methods for individual fields

void IwrfTsInfo::set_radar_name(const string &x) {
  STRncopy(_radar_info.radar_name, x.c_str(), IWRF_MAX_RADAR_NAME);
}

void IwrfTsInfo::set_radar_site_name(const string &x) { 
  STRncopy(_radar_info.site_name, x.c_str(), IWRF_MAX_SITE_NAME);
}

void IwrfTsInfo::set_scan_segment_name(const string &x) {
  STRncopy(_scan_seg.segment_name, x.c_str(), IWRF_MAX_SEGMENT_NAME);
}

void IwrfTsInfo::set_scan_project_name(const string &x) {
  STRncopy(_scan_seg.project_name, x.c_str(), IWRF_MAX_PROJECT_NAME);
}

void IwrfTsInfo::set_rvp8_s_version_string(const string &x) {
  STRncopy(_rvp8.s_version_string, x.c_str(), 12);
}

void IwrfTsInfo::set_scan_fixed_angle(int i, fl32 x) {
  if (i < IWRF_MAX_FIXED_ANGLES) {
    _scan_seg.fixed_angles[i] = x;
  }
}

void IwrfTsInfo::set_xmit_sample_h(int i, si32 val) {
  if (i < IWRF_N_TXSAMP) {
    _xmit_sample.samples_h[i] = val;
  }
}

void IwrfTsInfo::set_xmit_sample_v(int i, si32 val) {
  if (i < IWRF_N_TXSAMP) {
    _xmit_sample.samples_v[i] = val;
  }
}

void IwrfTsInfo::set_phasecode_deg_h(int i, fl32 x) {
  if (i < IWRF_MAX_PHASE_SEQ_LEN) {
    _phasecode.phase[i].phase_deg_h = x;
  }
}

void IwrfTsInfo::set_phasecode_deg_v(int i, fl32 x) {
  if (i < IWRF_MAX_PHASE_SEQ_LEN) {
    _phasecode.phase[i].phase_deg_v = x;
  }
}

void IwrfTsInfo::set_rvp8_i_range_mask(int i, ui16 x) {
  if (i < IWRF_RVP8_GATE_MASK_LEN) {
    _rvp8.i_range_mask[i] = x;
  }
}

void IwrfTsInfo::set_rvp8_f_noise_dbm(int chan, fl32 x) {
  if (chan < IWRF_MAX_CHAN) {
    _rvp8.f_noise_dbm[chan] = x;
  }
}

void IwrfTsInfo::set_rvp8_f_noise_stdv_db(int chan, fl32 x) {
  if (chan < IWRF_MAX_CHAN) {
    _rvp8.f_noise_stdv_db[chan] = x;
  }
}

void IwrfTsInfo::set_rvp8_i_gparm_latch_sts(int i, ui16 x) {
  if (i < 2) {
    _rvp8.i_gparm_latch_sts[i] = x;
  }
}

void IwrfTsInfo::set_rvp8_i_gparm_immed_sts(int i, ui16 x) {
  if (i < 2) {
    _rvp8.i_gparm_immed_sts[i] = x;
  }
}

void IwrfTsInfo::set_rvp8_i_gparm_diag_bits(int i, ui16 x) {
  if (i < 2) {
    _rvp8.i_gparm_diag_bits[i] = x;
  }
}

////////////////////////////////////////////////////////////
// get methods for individual fields

string IwrfTsInfo::get_radar_name() const {
  return iwrf_safe_str(_radar_info.radar_name, IWRF_MAX_RADAR_NAME);
}

string IwrfTsInfo::get_radar_site_name() const { 
  return iwrf_safe_str(_radar_info.site_name, IWRF_MAX_SITE_NAME);
}

string IwrfTsInfo::get_scan_segment_name() const {
  return iwrf_safe_str(_scan_seg.segment_name, IWRF_MAX_SEGMENT_NAME);
}

string IwrfTsInfo::get_scan_project_name() const {
  return iwrf_safe_str(_scan_seg.project_name, IWRF_MAX_PROJECT_NAME);
}

fl32 IwrfTsInfo::get_scan_fixed_angle(int i) const {
  if (i < IWRF_MAX_FIXED_ANGLES) {
    return _scan_seg.fixed_angles[i];
  } else {
    return -1;
  }
}

si32 IwrfTsInfo::get_xmit_sample_h(int i) const {
  if (i < IWRF_N_TXSAMP) {
    return _xmit_sample.samples_h[i];
  } else {
    return 0;
  }
}

si32 IwrfTsInfo::get_xmit_sample_v(int i) const {
  if (i < IWRF_N_TXSAMP) {
    return _xmit_sample.samples_v[i];
  } else {
    return 0;
  }
}

fl32 IwrfTsInfo::get_phasecode_deg_h(int i) const {
  if (i < IWRF_MAX_PHASE_SEQ_LEN) {
    return _phasecode.phase[i].phase_deg_h;
  } else {
    return -1;
  }
}

fl32 IwrfTsInfo::get_phasecode_deg_v(int i) const {
  if (i < IWRF_MAX_PHASE_SEQ_LEN) {
    return _phasecode.phase[i].phase_deg_v;
  } else {
    return -1;
  }
}

ui16 IwrfTsInfo::get_rvp8_i_range_mask(int i) const {
  if (i < IWRF_RVP8_GATE_MASK_LEN) {
    return _rvp8.i_range_mask[i];
  } else {
    return 0;
  }
}

fl32 IwrfTsInfo::get_rvp8_f_noise_dbm(int chan) const {
  if (chan < IWRF_MAX_CHAN) {
    return _rvp8.f_noise_dbm[chan];
  } else {
    return -1;
  }
}

fl32 IwrfTsInfo::get_rvp8_f_noise_stdv_db(int chan) const {
  if (chan < IWRF_MAX_CHAN) {
    return _rvp8.f_noise_stdv_db[chan];
  } else {
    return -1;
  }
}

ui16 IwrfTsInfo::get_rvp8_i_gparm_latch_sts(int i) const {
  if (i < 2) {
    return _rvp8.i_gparm_latch_sts[i];
  } else {
    return 0;
  }
}

ui16 IwrfTsInfo::get_rvp8_i_gparm_immed_sts(int i) const {
  if (i < 2) {
    return _rvp8.i_gparm_immed_sts[i];
  } else {
    return 0;
  }
}

ui16 IwrfTsInfo::get_rvp8_i_gparm_diag_bits(int i) const {
  if (i < 2) {
    return _rvp8.i_gparm_diag_bits[i];
  } else {
    return 0;
  }
}

string IwrfTsInfo::get_rvp8_s_version_string() const {
  return iwrf_safe_str(_rvp8.s_version_string, 12);
}

///////////////////////////////////////////////////////////
// set RVP8-specific info from other structs

void IwrfTsInfo::setRvp8Info(const iwrf_radar_info_t &radar,
			     const iwrf_calibration_t &calib,
			     const iwrf_ts_processing_t &proc,
			     const iwrf_pulse_header_t &pulse)
  
{
  
  if (_rvp8.i_version != 0) {
    // rvp8 info has been set from real RVP8 data, therefore don't set it here
    return;
  }
  
  _rvp8.i_task_sweep = pulse.sweep_num;
  
  STRncopy(_rvp8.s_site_name, radar.site_name, 32);
  _rvp8.f_pwidth_usec = pulse.pulse_width_us;
  _rvp8.f_dbz_calib = calib.base_dbz_1km_hc;
  _rvp8.f_wavelength_cm = radar.wavelength_cm;
  _rvp8.f_range_mask_res = proc.gate_spacing_m;
  
  /* iRangeMask is bit mask of bins that have
   * actually been selected at the
   * above spacing. */
  
  memset(_rvp8.i_range_mask, 0, IWRF_RVP8_GATE_MASK_LEN * sizeof(ui16));
  int one = 1;
  int count = 0;
  int nGates = pulse.n_gates;
  for (int ii = 0; ii < IWRF_RVP8_GATE_MASK_LEN; ii++) {
    for (int iBit = 0; iBit < 16; iBit++) {
      if (count >= nGates) {
	break;
      }
      int mask = one << iBit;
      _rvp8.i_range_mask[ii] |= mask;
      count++;
    }
    if (count >= nGates) {
      break;
    }
  }
  
  _rvp8.f_noise_dbm[0] = calib.noise_source_power_dbm_h;
  _rvp8.f_noise_dbm[1] = calib.noise_source_power_dbm_v;

}
  
///////////////////////////////////////////////////////////////
// read in pulse info from RVP8 file
//
// Returns 0 on success, -1 on failure

int IwrfTsInfo::readFromRvp8File(FILE *in)

{

  fseek(in, 0L, SEEK_SET);

  if (_readRvp8Info(in)) {
    return -1;
  }

  return 0;

}
  
///////////////////////////////////////////////////////////////
// find search string in rvp8 data
//
// Returns 0 on succes, -1 on failure (EOF)

int IwrfTsInfo::findNextRvp8Str(FILE *in,
				const char *searchStr)
  
{
  
  int searchLen = strlen(searchStr);

  // create buffer for incoming characters
  
  TaArray<char> cbuf_;
  char *cbuf = cbuf_.alloc(searchLen + 1);
  memset(cbuf, 0, searchLen + 1);
  
  while (!feof(in)) {
  
    // read in a character

    int cc = fgetc(in);
    if (cc == EOF) {
      return -1;
    }
    
    // move the characters along in the buffer

    memmove(cbuf + 1, cbuf, searchLen - 1);

    // store the latest character

    cbuf[0] = cc;
    
    // test for the search string

    bool match = true;
    for (int ii = 0, jj = searchLen - 1; ii < searchLen; ii++, jj--) {
      if (searchStr[ii] != cbuf[jj]) {
        match = false;
        break;
      }
    }

    if (match) {
      return 0;
    }

  } // while

  return -1;

}

///////////////////////////////////////////////////////////
// print

// print everything

void IwrfTsInfo::print(FILE *out) const
{
  
  fprintf(out, "******************** Start IwrfTsInfo ***************************\n");

  if (_radar_info_active) {
    iwrf_radar_info_print(out, _radar_info);
  }
  if (_scan_seg_active) {
    iwrf_scan_segment_print(out, _scan_seg);
  }
  if (_ant_corr_active) {
    iwrf_antenna_correction_print(out, _ant_corr);
  }
  if (_proc_active) {
    iwrf_ts_processing_print(out, _proc);
  }
  if (_xmit_power_active) {
    iwrf_xmit_power_print(out, _xmit_power);
  }
  if (_xmit_sample_active) {
    iwrf_xmit_sample_print(out, _xmit_sample);
  }
  if (_status_xml_active) {
    iwrf_status_xml_print(out, _status_xml_hdr, _status_xml_str);
  }
  if (_calib_active) {
    iwrf_calibration_print(out, _calib);
  }
  if (_enotice_active) {
    iwrf_event_notice_print(out, _enotice);
  }
  if (_phasecode_active) {
    iwrf_phasecode_print(out, _phasecode);
  }
  if (_xmit_info_active) {
    iwrf_xmit_info_print(out, _xmit_info);
  }
  if (_rvp8_active) {
    iwrf_rvp8_ops_info_print(out, _rvp8);
  }
  if (_platform_georef0_active) {
    iwrf_platform_georef_print(out, _platform_georef0);
  }
  if (_platform_georef1_active) {
    iwrf_platform_georef_print(out, _platform_georef1);
  }

  fprintf(out, "********************* End IwrfTsInfo ****************************\n");

}

// print only what is in the id queue
// clear the queue if clearQueue is set to true

void IwrfTsInfo::printMetaQueue(FILE *out, bool clearQueue) const
{
  
  if (_metaQueue.size() == 0) {
    return;
  }

  if (_debug >= IWRF_DEBUG_VERBOSE) {
    cerr << "DEBUG - IwrfTsInfo::printMetaQueue()" << endl;
    cerr << "  _metaQueue.size(): " << _metaQueue.size() << endl;
  }

  fprintf(out, "******************** Start IwrfTsInfo ***************************\n");
  
  for (size_t ii = 0; ii < _metaQueue.size(); ii++) {
    MemBuf *buf = _metaQueue[ii];
    iwrf_packet_print(out, buf->getPtr(), buf->getLen());
  } // ii

  fprintf(out, "********************* End IwrfTsInfo ****************************\n");
  
  if (clearQueue) {
    _clearMetaQueue();
  }

}

///////////////////////////////////////////////////
// write a sync packet to file in IWRF format
//
// returns 0 on success, -1 on failure

int IwrfTsInfo::writeSyncToFile(FILE *out) const

{
  
  if (out == NULL) {
    return -1;
  }
  
  iwrf_sync_t sync;
  iwrf_sync_init(sync);
  if (fwrite(&sync, sizeof(sync), 1, out) != 1) {
    int errNum = errno;
    cerr << "ERROR - IwrfTsInfo::writeSync2File" << endl;
    cerr << "  Cannot write sync packet" << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  
  return 0;

}

///////////////////////////////////////////////////
// write meta-data to file in IWRF format
//
// Writes out any meta-data which has a sequence number
// later than the previous pulse written.
// To force a write, set the prevPulseSeqNum to 0.
//
// returns 0 on success, -1 on failure

int IwrfTsInfo::writeMetaToFile(FILE *out, si64 prevPulseSeqNum) const

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
	cerr << "ERROR - IwrfTsInfo::write2File" << endl;
	cerr << "  Cannot write _radar_info packet" << endl;
	cerr << "  " << strerror(errNum) << endl;
	return -1;
      }
    }

  }

  if (_scan_seg_active && _scan_seg.packet.seq_num > prevPulseSeqNum) {
      if (fwrite(&_scan_seg, sizeof(_scan_seg), 1, out) != 1) {
	int errNum = errno;
	cerr << "ERROR - IwrfTsInfo::write2File" << endl;
	cerr << "  Cannot write _scan_seg packet" << endl;
	cerr << "  " << strerror(errNum) << endl;
	return -1;
      }
    }

  if (_ant_corr_active && _ant_corr.packet.seq_num > prevPulseSeqNum) {
    if (fwrite(&_ant_corr, sizeof(_ant_corr), 1, out) != 1) {
      int errNum = errno;
      cerr << "ERROR - IwrfTsInfo::write2File" << endl;
      cerr << "  Cannot write _ant_corr packet" << endl;
      cerr << "  " << strerror(errNum) << endl;
      return -1;
    }
  }

  if (_proc_active && _proc.packet.seq_num > prevPulseSeqNum) {
    if (fwrite(&_proc, sizeof(_proc), 1, out) != 1) {
      int errNum = errno;
      cerr << "ERROR - IwrfTsInfo::write2File" << endl;
      cerr << "  Cannot write _proc packet" << endl;
      cerr << "  " << strerror(errNum) << endl;
      return -1;
    }
  }

  if (_xmit_power_active && _xmit_power.packet.seq_num > prevPulseSeqNum) {
    if (fwrite(&_xmit_power, sizeof(_xmit_power), 1, out) != 1) {
      int errNum = errno;
      cerr << "ERROR - IwrfTsInfo::write2File" << endl;
      cerr << "  Cannot write _xmit_power packet" << endl;
      cerr << "  " << strerror(errNum) << endl;
      return -1;
    }
  }

  if (_xmit_sample_active && _xmit_sample.packet.seq_num > prevPulseSeqNum) {
    if (fwrite(&_xmit_sample, sizeof(_xmit_sample), 1, out) != 1) {
      int errNum = errno;
      cerr << "ERROR - IwrfTsInfo::write2File" << endl;
      cerr << "  Cannot write _xmit_sample packet" << endl;
      cerr << "  " << strerror(errNum) << endl;
      return -1;
    }
  }

  if (_status_xml_active &&
      _status_xml_hdr.packet.seq_num > prevPulseSeqNum) {
    if (fwrite(&_status_xml_hdr, sizeof(_status_xml_hdr), 1, out) != 1) {
      int errNum = errno;
      cerr << "ERROR - IwrfTsInfo::write2File" << endl;
      cerr << "  Cannot write _status_xml header" << endl;
      cerr << "  " << strerror(errNum) << endl;
      return -1;
    }
    if (fwrite(_status_xml_str.c_str(),
               _status_xml_str.size() + 1, 1, out) != 1) {
      int errNum = errno;
      cerr << "ERROR - IwrfTsInfo::write2File" << endl;
      cerr << "  Cannot write _status_xml string" << endl;
      cerr << "  " << strerror(errNum) << endl;
      return -1;
    }
  }

  if (_calib_active && _calib.packet.seq_num > prevPulseSeqNum) {
    if (fwrite(&_calib, sizeof(_calib), 1, out) != 1) {
      int errNum = errno;
      cerr << "ERROR - IwrfTsInfo::write2File" << endl;
      cerr << "  Cannot write _calib packet" << endl;
      cerr << "  " << strerror(errNum) << endl;
      return -1;
    }
  }

  if (_enotice_active && _enotice.packet.seq_num > prevPulseSeqNum) {
    if (fwrite(&_enotice, sizeof(_enotice), 1, out) != 1) {
      int errNum = errno;
      cerr << "ERROR - IwrfTsInfo::write2File" << endl;
      cerr << "  Cannot write _enotice packet" << endl;
      cerr << "  " << strerror(errNum) << endl;
      return -1;
    }
  }

  if (_phasecode_active && _phasecode.packet.seq_num > prevPulseSeqNum) {
    if (fwrite(&_phasecode, sizeof(_phasecode), 1, out) != 1) {
      int errNum = errno;
      cerr << "ERROR - IwrfTsInfo::write2File" << endl;
      cerr << "  Cannot write _phasecode packet" << endl;
      cerr << "  " << strerror(errNum) << endl;
      return -1;
    }
  }

  if (_xmit_info_active && _xmit_info.packet.seq_num > prevPulseSeqNum) {
    if (fwrite(&_xmit_info, sizeof(_xmit_info), 1, out) != 1) {
      int errNum = errno;
      cerr << "ERROR - IwrfTsInfo::write2File" << endl;
      cerr << "  Cannot write _xmit_info packet" << endl;
      cerr << "  " << strerror(errNum) << endl;
      return -1;
    }
  }

  if (_rvp8_active && _rvp8.packet.seq_num > prevPulseSeqNum) {
    if (fwrite(&_rvp8, sizeof(_rvp8), 1, out) != 1) {
      int errNum = errno;
      cerr << "ERROR - IwrfTsInfo::write2File" << endl;
      cerr << "  Cannot write _rvp8 packet" << endl;
      cerr << "  " << strerror(errNum) << endl;
      return -1;
    }
  }

  if (_platform_georef0_active &&
      _platform_georef0.packet.seq_num > prevPulseSeqNum) {
    if (fwrite(&_platform_georef0, sizeof(_platform_georef0), 1, out) != 1) {
      int errNum = errno;
      cerr << "ERROR - IwrfTsInfo::write2File" << endl;
      cerr << "  Cannot write _platform_georef0 packet" << endl;
      cerr << "  " << strerror(errNum) << endl;
      return -1;
    }
  }

  if (_platform_georef1_active &&
      _platform_georef1.packet.seq_num > prevPulseSeqNum) {
    if (fwrite(&_platform_georef1, sizeof(_platform_georef1), 1, out) != 1) {
      int errNum = errno;
      cerr << "ERROR - IwrfTsInfo::write2File" << endl;
      cerr << "  Cannot write _platform_georef1 packet" << endl;
      cerr << "  " << strerror(errNum) << endl;
      return -1;
    }
  }

  return 0;

}

///////////////////////////////////////////////////
// write meta-data to file in IWRF format
//
// Writes out any meta-data in the queue
// If clearQueue is true, the queue will be cleared.
//
// Returns 0 on success, -1 on failure

int IwrfTsInfo::writeMetaQueueToFile(FILE *out, bool clearQueue) const

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

  if (_debug >= IWRF_DEBUG_VERBOSE) {
    cerr << "DEBUG - IwrfTsInfo::writeMetaQueueToFile()" << endl;
    cerr << "  _metaQueue.size(): " << _metaQueue.size() << endl;
  }

  // write out the queue of metadata packets

  int iret = 0;
  for (size_t ii = 0; ii < _metaQueue.size(); ii++) {
    MemBuf *buf = _metaQueue[ii];
    if (fwrite(buf->getPtr(), buf->getLen(), 1, out) != 1) {
      int errNum = errno;
      cerr << "ERROR - IwrfTsInfo::write2File" << endl;
      cerr << "  Cannot write metaData packet" << endl;
      cerr << "  " << strerror(errNum) << endl;
      iwrf_packet_print(stderr, buf->getPtr(), buf->getLen());
      iret = -1;
    }
  } // ii

  if (clearQueue) {
    _clearMetaQueue();
  }

  return iret;

}

///////////////////////////////////////////////////
// add meta-data to DsMessage in IWRF format
//
// Loads up any meta-data in the queue.
// If clearQueue is true, the queue will be cleared.

void IwrfTsInfo::addMetaQueueToMsg(DsMessage &msg, bool clearQueue) const
  
{

  if (_debug >= IWRF_DEBUG_VERBOSE) {
    cerr << "DEBUG - IwrfTsInfo::addMetaQueueToMsg()" << endl;
    cerr << "  _metaQueue.size(): " << _metaQueue.size() << endl;
  }

  for (size_t ii = 0; ii < _metaQueue.size(); ii++) {
    MemBuf *buf = _metaQueue[ii];
    int packetId = 0;
    if (iwrf_get_packet_id(buf->getPtr(), buf->getLen(), packetId) == 0) {
      msg.addPart(packetId, buf->getLen(), buf->getPtr());
    }
  } // ii

  if (clearQueue) {
    _clearMetaQueue();
  }

}

///////////////////////////////////////////////////
// write in rvp8 tsarchive format

int IwrfTsInfo::writeToTsarchiveFile(FILE *out) const

{

  if (out == NULL) {
    return -1;
  }

  fprintf(out, "rvptsPulseInfo start\n");
  fprintf(out, "iVersion=%d\n", _rvp8.i_version);
  fprintf(out, "iMajorMode=%d\n", _rvp8.i_major_mode);
  fprintf(out, "iPolarization=%d\n", _rvp8.i_polarization);
  fprintf(out, "iPhaseModSeq=%d\n", _rvp8.i_phase_mode_seq);
  fprintf(out, "taskID.iSweep=%d\n", _rvp8.i_task_sweep);
  fprintf(out, "taskID.iAuxNum=%d\n", _rvp8.i_task_aux_num);
  fprintf(out, "taskID.sTaskName=%s\n", _rvp8.s_task_name);
  fprintf(out, "sSiteName=%s\n", _rvp8.s_site_name);
  fprintf(out, "iAqMode=%d\n", _rvp8.i_aq_mode);
  fprintf(out, "iUnfoldMode=%d\n", _rvp8.i_unfold_mode);
  fprintf(out, "iPWidthCode=%d\n", _rvp8.i_pwidth_code);
  fprintf(out, "fPWidthUSec=%g\n", _rvp8.f_pwidth_usec);
  fprintf(out, "fDBzCalib=%g\n", _rvp8.f_dbz_calib);
  fprintf(out, "iSampleSize=%d\n", _rvp8.i_sample_size);
  fprintf(out, "iMeanAngleSync=%d\n", _rvp8.i_mean_angle_sync);
  fprintf(out, "iFlags=%d\n", _rvp8.i_flags);
  fprintf(out, "iPlaybackVersion=%d\n", _rvp8.i_playback_version);
  fprintf(out, "fSyClkMHz=%g\n", _rvp8.f_sy_clk_mhz);
  fprintf(out, "fWavelengthCM=%g\n", _rvp8.f_wavelength_cm);
  fprintf(out, "fSaturationDBM=%g\n", _rvp8.f_saturation_dbm);
  fprintf(out, "fRangeMaskRes=%g\n", _rvp8.f_range_mask_res);

  fprintf(out, "iRangeMask=");
  for (int ii = 0; ii < IWRF_RVP8_GATE_MASK_LEN; ii++) {
    fprintf(out, "%d", _rvp8.i_range_mask[ii]);
    if (ii != IWRF_RVP8_GATE_MASK_LEN - 1) {
      fprintf(out, " ");
    }
  }
  fprintf(out, "\n");
  
  fprintf(out, "fNoiseDBm=%g %g\n",
          _rvp8.f_noise_dbm[0],
          _rvp8.f_noise_dbm[1]);
  fprintf(out, "fNoiseStdvDB=%g %g\n",
          _rvp8.f_noise_stdv_db[0],
          _rvp8.f_noise_stdv_db[1]);
  
  fprintf(out, "fNoiseRangeKM=%g\n", _rvp8.f_noise_range_km);
  fprintf(out, "fNoisePRFHz=%g\n", _rvp8.f_noise_prf_hz);

  fprintf(out, "iGparmLatchSts=%d %d\n",
          _rvp8.i_gparm_latch_sts[0],
          _rvp8.i_gparm_latch_sts[1]);

  fprintf(out, "iGparmImmedSts=%d %d %d %d %d %d\n",
          _rvp8.i_gparm_immed_sts[0],
          _rvp8.i_gparm_immed_sts[1],
          _rvp8.i_gparm_immed_sts[2],
          _rvp8.i_gparm_immed_sts[3],
          _rvp8.i_gparm_immed_sts[4],
          _rvp8.i_gparm_immed_sts[5]);

  fprintf(out, "iGparmDiagBits=%d %d %d %d\n",
          _rvp8.i_gparm_diag_bits[0],
          _rvp8.i_gparm_diag_bits[1],
          _rvp8.i_gparm_diag_bits[2],
          _rvp8.i_gparm_diag_bits[3]);

  fprintf(out, "sVersionString=%s\n", _rvp8.s_version_string);

  fprintf(out, "rvptsPulseInfo end\n");

  return 0;

}

///////////////////////////////////////////////////////////////
// read in pulse info from RVP8 file
//
// Returns 0 on success, -1 on failure

int IwrfTsInfo::_readRvp8Info(FILE *in)

{

  // initialize to missing values

  MEM_zero(_rvp8);

  // find the start of the info header
  
  if (findNextRvp8Str(in, "PulseInfo start")) {
    return -1;
  }

  // prepare structs

  iwrf_radar_info_t radarInfo;
  iwrf_ts_processing_t tsProc;
  iwrf_calibration_t cal;

  iwrf_radar_info_init(radarInfo);
  iwrf_ts_processing_init(tsProc);
  iwrf_calibration_init(cal);

  // read in info header

  char line[8192];
  bool taskNameFound = false;
  bool siteNameFound = false;
  bool versionStringFound = false;
  bool rangeMaskFound = false;
  bool iVersionFound = false;
  bool iMajorModeFound = false;
  bool iPolarizationFound = false;
  bool iPhaseModSeqFound = false;
  bool iTaskSweepFound = false;
  bool iTaskAuxNumFound = false;
  bool iAqModeFound = false;
  bool iUnfoldModeFound = false;
  bool iPWidthCodeFound = false;
  bool fPWidthUSecFound = false;
  bool fDBzCalibFound = false;
  bool iSampleSizeFound = false;
  bool iMeanAngleSyncFound = false;
  bool iFlagsFound = false;
  bool iPlaybackVersionFound = false;
  bool fSyClkMHzFound = false;
  bool fWavelengthCMFound = false;
  bool fSaturationDBMFound = false;
  bool fRangeMaskResFound = false;
  bool fNoiseDBmFound = false;
  bool fNoiseStdvDBFound = false;
  bool fNoiseRangeKMFound = false;
  bool fNoisePRFHzFound = false;
  bool iGparmLatchStsFound = false;
  bool iGparmImmedStsFound = false;
  bool iGparmDiagBitsFound = false;

  while (!feof(in)) {

    if (fgets(line, BUFSIZ, in) == NULL) {
      return -1;
    }
    
    if (strstr(line, "PulseInfo end")) {
      break;
    }

    char name[80];
    int ival, jval[6];
    double dval, eval[2];


    if (!iVersionFound && sscanf(line, "iVersion=%d", &ival) == 1) {
      _rvp8.i_version = ival;
      iVersionFound = true;
      continue;
    }

    if (!iMajorModeFound && sscanf(line, "iMajorMode=%d", &ival) == 1) {
      _rvp8.i_major_mode = ival;
      iMajorModeFound = true;
      continue;
    }

    if (!iPolarizationFound && sscanf(line, "iPolarization=%d", &ival) == 1) {
      _rvp8.i_polarization = ival;
      iPolarizationFound = true;
      continue;
    }

    if (!iPhaseModSeqFound && sscanf(line, "iPhaseModSeq=%d", &ival) == 1) {
      _rvp8.i_phase_mode_seq = ival;
      iPhaseModSeqFound = true;
      continue;
    }

    if (!iTaskSweepFound && sscanf(line, "taskID.iSweep=%d", &ival) == 1) {
      _rvp8.i_task_sweep = ival;
      iTaskSweepFound = true;
      continue;
    }

    if (!iTaskAuxNumFound && sscanf(line, "taskID.iAuxNum=%d", &ival) == 1) {
      _rvp8.i_task_aux_num = ival;
      iTaskAuxNumFound = true;
      continue;
    }
    if (!taskNameFound && strlen(line) < 80 &&
	sscanf(line, "taskID.sTaskName=%s", name) == 1) {
      STRncopy(_rvp8.s_task_name, name, sizeof(_rvp8.s_task_name));
      taskNameFound = true;
      continue;
    }
    if (!siteNameFound && strlen(line) < 80 &&
	sscanf(line, "sSiteName=%s", name) == 1) {
      STRncopy(_rvp8.s_site_name, name, sizeof(_rvp8.s_site_name));
      STRncopy(radarInfo.radar_name, name, IWRF_MAX_RADAR_NAME);
      STRncopy(radarInfo.site_name, name, IWRF_MAX_SITE_NAME);
      siteNameFound = true;
    }

    if (!iAqModeFound && sscanf(line, "iAqMode=%d", &ival) == 1) {
      _rvp8.i_aq_mode = ival;
      iAqModeFound = true;
      continue;
    }
    
    if (!iUnfoldModeFound && sscanf(line, "iUnfoldMode=%d", &ival) == 1) {
      _rvp8.i_unfold_mode = ival;
      iUnfoldModeFound = true;
      continue;
    }

    if (!iPWidthCodeFound && sscanf(line, "iPWidthCode=%d", &ival) == 1) {
      _rvp8.i_pwidth_code = ival;
      iPWidthCodeFound = true;
      continue;
    }

    if (!fPWidthUSecFound && sscanf(line, "fPWidthUSec=%lg", &dval) == 1) {
      _rvp8.f_pwidth_usec = dval;
      fPWidthUSecFound = true;
      cal.pulse_width_us = dval;
      continue;
    }

    if (!fDBzCalibFound && sscanf(line, "fDBzCalib=%lg", &dval) == 1) {
      _rvp8.f_dbz_calib = dval;
      fDBzCalibFound = true;
      cal.base_dbz_1km_hc = dval;
      cal.base_dbz_1km_vc = dval;
      continue;
    }

    if (!iSampleSizeFound && sscanf(line, "iSampleSize=%d", &ival) == 1) {
      _rvp8.i_sample_size = ival;
      iSampleSizeFound = true;
      continue;
    }

    if (!iMeanAngleSyncFound && sscanf(line, "iMeanAngleSync=%d", &ival) == 1) {
      _rvp8.i_mean_angle_sync = ival;
      iMeanAngleSyncFound = true;
      continue;
    }

    if (!iFlagsFound && sscanf(line, "iFlags=%d", &ival) == 1) {
      _rvp8.i_flags = ival;
      iFlagsFound = true;
      continue;
    }

    if (!iPlaybackVersionFound && sscanf(line, "iPlaybackVersion=%d", &ival) == 1) {
      _rvp8.i_playback_version = ival;
      iPlaybackVersionFound = true;
      continue;
    }

    if (!fSyClkMHzFound && sscanf(line, "fSyClkMHz=%lg", &dval) == 1) {
      _rvp8.f_sy_clk_mhz = dval;
      fSyClkMHzFound = true;
      continue;
    }

    if (!fWavelengthCMFound && sscanf(line, "fWavelengthCM=%lg", &dval) == 1) {
      _rvp8.f_wavelength_cm = dval;
      _radar_info.wavelength_cm = dval;
      fWavelengthCMFound = true;
      radarInfo.wavelength_cm = dval;
      cal.wavelength_cm = dval;
      continue;
    }

    if (!fSaturationDBMFound && sscanf(line, "fSaturationDBM=%lg", &dval) == 1) {
      set_rvp8_f_saturation_dbm(dval);
      fSaturationDBMFound = true;
      continue;
    }
    
    if (!fRangeMaskResFound && sscanf(line, "fRangeMaskRes=%lg", &dval) == 1) {
      _rvp8.f_range_mask_res = dval;
      fRangeMaskResFound = true;
      continue;
    }

    if (!rangeMaskFound && strstr(line, "iRangeMask=") != NULL) {
      char *toks = strtok(line, "= ");
      int count = 0;
      while (toks != NULL && count < IWRF_RVP8_GATE_MASK_LEN) {
        toks = strtok(NULL, "= ");
        if (toks != NULL) {
          if (sscanf(toks, "%d", &ival) == 1) {
            _rvp8.i_range_mask[count] = ival;
          }
          count++;
        }
      }
      rangeMaskFound = true;
      continue;
    }


    if (!fNoiseDBmFound &&
        sscanf(line, "fNoiseDBm=%lg %lg", &eval[0], &eval[1]) == 2) {
      _rvp8.f_noise_dbm[0] = eval[0];
      _rvp8.f_noise_dbm[1] = eval[1];
      fNoiseDBmFound = true;
      cal.noise_dbm_hc = eval[0];
      cal.noise_dbm_vc = eval[1];
      continue;
    }


    if (!fNoiseStdvDBFound &&
        sscanf(line, "fNoiseStdvDB=%lg %lg", &eval[0], &eval[1]) == 2) {
      _rvp8.f_noise_stdv_db[0] = eval[0];
      _rvp8.f_noise_stdv_db[1] = eval[1];
      fNoiseStdvDBFound = true;
      continue;
    }


    if (!fNoiseRangeKMFound && sscanf(line, "fNoiseRangeKM=%lg", &dval) == 1) {
      _rvp8.f_noise_range_km = dval;
      fNoiseRangeKMFound = true;
      continue;
    }


    if (!fNoisePRFHzFound && sscanf(line, "fNoisePRFHz=%lg", &dval) == 1) {
      _rvp8.f_noise_prf_hz = dval;
      fNoisePRFHzFound = true;
      continue;
    }
    

    if (!iGparmLatchStsFound &&
        sscanf(line, "iGparmLatchSts=%d %d", &jval[0], &jval[1]) == 2) {
      _rvp8.i_gparm_latch_sts[0] = jval[0];
      _rvp8.i_gparm_latch_sts[1] = jval[1];
      iGparmLatchStsFound = true;
      continue;
    }


    if (!iGparmImmedStsFound &&
        sscanf(line, "iGparmImmedSts=%d %d %d %d %d %d",
               &jval[0], &jval[1], &jval[2],
               &jval[3], &jval[4], &jval[5]) == 6) {
      _rvp8.i_gparm_immed_sts[0] = jval[0];
      _rvp8.i_gparm_immed_sts[1] = jval[1];
      _rvp8.i_gparm_immed_sts[2] = jval[2];
      _rvp8.i_gparm_immed_sts[3] = jval[3];
      _rvp8.i_gparm_immed_sts[4] = jval[4];
      _rvp8.i_gparm_immed_sts[5] = jval[5];
      iGparmImmedStsFound = true;
      continue;
    }


    if (!iGparmDiagBitsFound &&
        sscanf(line, "iGparmDiagBits=%d %d %d %d",
               &jval[0], &jval[1],
               &jval[2], &jval[3]) == 4) {
      _rvp8.i_gparm_diag_bits[0] = jval[0];
      _rvp8.i_gparm_diag_bits[1] = jval[1];
      _rvp8.i_gparm_diag_bits[2] = jval[2];
      _rvp8.i_gparm_diag_bits[3] = jval[3];
      iGparmDiagBitsFound = true;
      continue;
    }

    if (!versionStringFound && strlen(line) < 80 &&
	sscanf(line, "sVersionString=%s", name) == 1) {
      STRncopy(_rvp8.s_version_string, name, sizeof(_rvp8.s_version_string));
      continue;
    }
    
  } // while

  // derive range info

  _deriveRangeFromRvp8Info(tsProc);

  // set the individual info structs

  setRadarInfo(radarInfo);
  setTsProcessing(tsProc);
  setCalibration(cal);

  setRadarInfoActive(true);
  setTsProcessingActive(true);
  setCalibrationActive(true);

  return 0;

}


///////////////////////////////////////////////////////////////
// derive quantities from RVP8 info

void IwrfTsInfo::_deriveRangeFromRvp8Info(iwrf_ts_processing_t &tsProc)

{

  // Find first, last, and total number of range bins in the mask
  // Based on SIGMET code in tsview.c

  int binCount = 0;
  int binStart = 0;
  int binEnd = 0;

  for (int ii = 0; ii < IWRF_RVP8_GATE_MASK_LEN; ii++) {
    ui16 mask = _rvp8.i_range_mask[ii];
    if (mask) {
      for (int iBit = 0; iBit < 16; iBit++) {
        if (1 & (mask >> iBit)) {
          int iBin = iBit + (16*ii);
          if (binCount == 0) {
            binStart = iBin;
          }
          binEnd = iBin;
          binCount++;
        }
      }
    }
  }
  
  // range computations
  
  double startRangeM = (binStart * _rvp8.f_range_mask_res);
  double maxRangeM = (binEnd * _rvp8.f_range_mask_res);
  double gateSpacingM = (maxRangeM - startRangeM) / (binCount - 1.0);

  // pulse centered on PRT boundary, and first gate holds burst
  
  startRangeM += gateSpacingM; 
  
  tsProc.start_range_m = startRangeM;
  tsProc.gate_spacing_m = gateSpacingM;

}


///////////////////////////////////////////////////////////////
// add id to queue
// deletes previous entries for the same id

// void IwrfTsInfo::_addIdToQueue(si32 id)

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

void IwrfTsInfo::_addMetaToQueue(size_t len, const void *packet)

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

MemBuf *IwrfTsInfo::_popFrontFromMetaQueue()
  
{
  MemBuf *front = _metaQueue.front();
  _metaQueue.pop_front();
  return front;
}

/////////////////////////////////////////////
// clear the metadata queue - public

void IwrfTsInfo::clearMetadataQueue()
  
{
  // _idQueue.clear();
  _clearMetaQueue();
}

/////////////////////////////////////////////
// clear the metadata queue - private

void IwrfTsInfo::_clearMetaQueue() const
  
{
  for (size_t ii = 0; ii < _metaQueue.size(); ii++) {
    delete _metaQueue[ii];
  }
  _metaQueue.clear();
}

/////////////////////////////////////////////
// clear the event flags

void IwrfTsInfo::clearEventFlags()
  
{

  _startOfSweepFlag = false;
  _endOfSweepFlag = false;
  _startOfVolumeFlag = false;
  _endOfVolumeFlag = false;

}

