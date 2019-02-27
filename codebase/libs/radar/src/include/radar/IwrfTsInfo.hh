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
/////////////////////////////////////////////////////////////
// IwrfTsInfo.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2007
//
///////////////////////////////////////////////////////////////
//
// Stores current radar ops info
//
////////////////////////////////////////////////////////////////

#ifndef _IwrfTsInfo_hh
#define _IwrfTsInfo_hh

#include <string>
#include <deque>
#include <dataport/port_types.h>
#include <toolsa/MemBuf.hh>
#include <radar/iwrf_functions.hh>
#include <radar/IwrfCalib.hh>
#include <didss/DsMessage.hh>
using namespace std;

////////////////////////
// This class

class IwrfTsInfo {
  
public:

  // constructor
  
  IwrfTsInfo(IwrfDebug_t debug = IWRF_DEBUG_OFF);

  // destructor
  
  ~IwrfTsInfo();

  // clear all data

  void clear();

  // debugging
  
  void setDebug(IwrfDebug_t debug) { _debug = debug; }

  // set structs from a generic buffer
  // by checking for the id
  // swaps as required
  
  int setFromBuffer(const void *buf, int len);
  
  // override radar name and/or site name
  
  void overrideRadarName(const string &radarName);
  void overrideSiteName(const string &siteName);
  
  // override radar location
  
  void overrideRadarLocation(double altitudeMeters,
                             double latitudeDeg,
                             double longitudeDeg);
  // override gate geometry
  
  void overrideGateGeometry(double startRangeMeters,
                            double gateSpacingMeters);
  // override wavelength

  void overrideWavelength(double wavelengthCm);
  
  // printing everything
  
  void print(FILE *out) const;
  
  // print only what is in the id queue
  // clear the queue if clearQueue is set to true
  
  void printMetaQueue(FILE *out, bool clearQueue) const;

  // read in info from RVP8 file
  
  int readFromRvp8File(FILE *in);

  // Was this derived from real RVP8 data?

  bool isDerivedFromRvp8() const { return (_rvp8.i_version != 0); }
  
  // find search string in rvp8 data
  // Returns 0 on succes, -1 on failure (EOF)
  
  static int findNextRvp8Str(FILE *in, const char *searchStr);
  
  // write in tsarchive format
  
  int writeToTsarchiveFile(FILE *out) const;

  // write a sync packet to file in IWRF format
  // returns 0 on success, -1 on failure

  int writeSyncToFile(FILE *out) const;
  
  // write meta-data to file in IWRF format
  // Writes out any meta-data which has a sequence number
  // later than the previous pulse written.
  // To force a write, set the prevPulseSeqNum to 0.
  // Returns 0 on success, -1 on failure

  int writeMetaToFile(FILE *out, si64 prevPulseSeqNum) const;

  // write meta-data to file in IWRF format
  // Writes out any meta-data for which the id is
  // queued in the _idQueue.
  // If clearQueue is true, the queue will be cleared
  // after the write
  // Returns 0 on success, -1 on failure

  int writeMetaQueueToFile(FILE *out, bool clearQueue) const;

  // write meta-data to DsMessage in IWRF format
  // Loads up any meta-data for which the id is
  // queued in the _idQueue.
  // If clearQueue is true, the queue will be cleared
  // after the write
  
  void addMetaQueueToMsg(DsMessage &msg, bool clearQueue) const;

  // clear the metadata queue
  
  void clearMetadataQueue();

  // clear the event flags

  void clearEventFlags();

  // set sweep and volume flags
  
  void setStartOfSweepFlag() { _startOfSweepFlag = true; }
  void setEndOfSweepFlag() { _endOfSweepFlag = true; }
  void setStartOfVolumeFlag() { _startOfVolumeFlag = true; }
  void setEndOfVolumeFlag() { _endOfVolumeFlag = true; }

  ////////////////////////////////////////////////////////////
  // set info at the struct level - does not set time

  void setRadarInfo(const iwrf_radar_info_t &info,
                    bool addToMetaQueue = true);
  void setScanSegment(const iwrf_scan_segment_t &seg,
                      bool addToMetaQueue = true);
  void setAntennaCorrection(const iwrf_antenna_correction_t &corr,
                            bool addToMetaQueue = true);
  void setTsProcessing(const iwrf_ts_processing_t &proc,
                       bool addToMetaQueue = true);
  void setXmitPower(const iwrf_xmit_power_t &power,
                    bool addToMetaQueue = true);
  void setRxPower(const iwrf_rx_power_t &power,
                  bool addToMetaQueue = true);
  void setXmitSample(const iwrf_xmit_sample_t &sample,
                     bool addToMetaQueue = true);
  void setStatusXml(const iwrf_status_xml_t &hdr,
                    const string &xmlStr,
                    bool addToMetaQueue = true);
  void setCalibration(const iwrf_calibration_t &calib,
                      bool addToMetaQueue = true);
  void setEventNotice(const iwrf_event_notice_t &enotice,
                      bool addToMetaQueue = true);
  void setPhasecode(const iwrf_phasecode_t &phasecode,
                    bool addToMetaQueue = true);
  void setXmitInfo(const iwrf_xmit_info_t &info,
                   bool addToMetaQueue = true);
  void setRvp8Info(const iwrf_rvp8_ops_info_t &info,
                   bool addToMetaQueue = true);
  void setPlatformGeoref(const iwrf_platform_georef_t &georef,
                         bool addToMetaQueue = true);
  
  ////////////////////////////////////////////////////////////
  // set sequence number for each packet
  
  void setRadarInfoPktSeqNum(si64 pkt_seq_num);
  void setScanSegmentPktSeqNum(si64 pkt_seq_num);
  void setAntennaCorrectionPktSeqNum(si64 pkt_seq_num);
  void setTsProcessingPktSeqNum(si64 pkt_seq_num);
  void setXmitPowerPktSeqNum(si64 pkt_seq_num);
  void setRxPowerPktSeqNum(si64 pkt_seq_num);
  void setXmitSamplePktSeqNum(si64 pkt_seq_num);
  void setStatusXmlPktSeqNum(si64 pkt_seq_num);
  void setCalibrationPktSeqNum(si64 pkt_seq_num);
  void setEventNoticePktSeqNum(si64 pkt_seq_num);
  void setPhasecodePktSeqNum(si64 pkt_seq_num);
  void setXmitInfoPktSeqNum(si64 pkt_seq_num);
  void setRvp8InfoPktSeqNum(si64 pkt_seq_num);
  void setPlatformGeorefPktSeqNum(si64 pkt_seq_num);
  void setPlatformGeoref1PktSeqNum(si64 pkt_seq_num);

  ////////////////////////////////////////////////////////////
  // set time on all packets
  
  void setTime(time_t secs, int nano_secs);

  // set time to now on all packets
  
  void setTimeToNow();

  // set time for each packet
  
  void setRadarInfoTime(time_t secs, int nano_secs);
  void setScanSegmentTime(time_t secs, int nano_secs);
  void setAntennaCorrectionTime(time_t secs, int nano_secs);
  void setTsProcessingTime(time_t secs, int nano_secs);
  void setXmitPowerTime(time_t secs, int nano_secs);
  void setRxPowerTime(time_t secs, int nano_secs);
  void setXmitSampleTime(time_t secs, int nano_secs);
  void setStatusXmlTime(time_t secs, int nano_secs);
  void setCalibrationTime(time_t secs, int nano_secs);
  void setEventNoticeTime(time_t secs, int nano_secs);
  void setPhasecodeTime(time_t secs, int nano_secs);
  void setXmitInfoTime(time_t secs, int nano_secs);
  void setRvp8InfoTime(time_t secs, int nano_secs);
  void setPlatformGeorefTime(time_t secs, int nano_secs);
  void setPlatformGeoref1Time(time_t secs, int nano_secs);

  // set time for each packet to now

  void setRadarInfoTimeToNow();
  void setScanSegmentTimeToNow();
  void setAntennaCorrectionTimeToNow();
  void setTsProcessingTimeToNow();
  void setXmitPowerTimeToNow();
  void setRxPowerTimeToNow();
  void setXmitSampleTimeToNow();
  void setStatusXmlTimeToNow();
  void setCalibrationTimeToNow();
  void setEventNoticeTimeToNow();
  void setPhasecodeTimeToNow();
  void setXmitInfoTimeToNow();
  void setRvp8InfoTimeToNow();
  void setPlatformGeorefTimeToNow();
  void setPlatformGeoref1TimeToNow();

  ////////////////////////////////////////////////////////////
  // activate structs individually

  void setRadarInfoActive(bool state);
  void setScanSegmentActive(bool state);
  void setAntennaCorrectionActive(bool state);
  void setTsProcessingActive(bool state);
  void setXmitPowerActive(bool state);
  void setRxPowerActive(bool state);
  void setXmitSampleActive(bool state);
  void setStatusXmlActive(bool state);
  void setCalibrationActive(bool state);
  void setEventNoticeActive(bool state);
  void setPhasecodeActive(bool state);
  void setXmitInfoActive(bool state);
  void setRvp8InfoActive(bool state);
  void setPlatformGeorefActive(bool state);
  void setPlatformGeoref1Active(bool state);

  ////////////////////////////////////////////////////////////
  // set radar id on all structs

  void setRadarId(int id);

  ////////////////////////////////////////////////////////////
  // set IwrfCalib object from _calib, and vice versa
  
  void setIwrfCalib(IwrfCalib &iwrfCalib) const;
  void setFromIwrfCalib(const IwrfCalib &iwrfCalib);

  ////////////////////////////////////////////////////////////
  // set DsRadarCalib object from _calib, and vice versa
  
  void setDsRadarCalib(DsRadarCalib &dsCalib) const;
  void setFromDsRadarCalib(const DsRadarCalib &dsCalib);

  ///////////////////////////////////////////////
  // set RVP8-specific fields from other structs
  
  void setRvp8Info(const iwrf_radar_info_t &radar,
		   const iwrf_calibration_t &calib,
		   const iwrf_ts_processing_t &proc,
		   const iwrf_pulse_header_t &pulse);

  ////////////////////////////////////////////////////////////
  // get info at the struct level

  inline const iwrf_radar_info_t &getRadarInfo() const { return _radar_info; }
  inline const iwrf_scan_segment_t &getScanSegment() const { return _scan_seg; }
  inline const iwrf_antenna_correction_t &getAntennaCorrection() const {
    return _ant_corr;
  }
  inline const iwrf_ts_processing_t &getTsProcessing() const { return _proc; }
  inline const iwrf_xmit_power_t &getXmitPower() const { return _xmit_power; }
  inline const iwrf_rx_power_t &getRxPower() const { return _rx_power; }
  inline const iwrf_xmit_sample_t &getXmitSample() const { 
    return _xmit_sample; 
  }
  inline const iwrf_status_xml_t &getStatusXmlHdr() const { 
    return _status_xml_hdr; 
  }
  inline const string &getStatusXmlStr() const { return _status_xml_str; }
  inline const iwrf_calibration_t &getCalibration() const { return _calib; }
  inline const iwrf_event_notice_t &getEventNotice() const { return _enotice; }
  inline const iwrf_phasecode_t &getPhasecode() const { return _phasecode; }
  inline const iwrf_xmit_info_t &getXmitInfo() const { return _xmit_info; }
  inline const iwrf_rvp8_ops_info_t &getRvp8Info() const { return _rvp8; }
  inline double getRvp8SaturationMult() const { return _rvp8SaturationMult; }
  inline const iwrf_platform_georef_t &getPlatformGeoref() const { 
    return _platform_georef0; 
  }
  inline const iwrf_platform_georef_t &getPlatformGeoref1() const { 
    return _platform_georef1; 
  }

  ////////////////////////////////////////////////////////////
  // check which structs are active

  bool isRadarInfoActive() const { return _radar_info_active; }
  bool isScanSegmentActive() const { return _scan_seg_active; }
  bool isAntennaCorrectionActive() const { return _ant_corr_active; }
  bool isTsProcessingActive() const { return _proc_active; }
  bool isXmitPowerActive() const { return _xmit_power_active; }
  bool isRxPowerActive() const { return _rx_power_active; }
  bool isXmitSampleActive() const { return _xmit_sample_active; }
  bool isStatusXmlActive() const { return _status_xml_active; }
  bool isCalibrationActive() const { return _calib_active; }
  bool isEventNoticeActive() const { return _enotice_active; }
  bool isPhasecodeActive() const { return _phasecode_active; }
  bool isXmitInfoActive() const { return _xmit_info_active; }
  bool isRvp8InfoActive() const { return _rvp8_active; }
  bool isPlatformGeorefActive() const { return _platform_georef0_active; }
  bool isPlatformGeoref1Active() const { return _platform_georef1_active; }

  ////////////////////////////////////////////////////////////
  // is essential info ready to be used?
  // will return true if essential info has been set
  // i.e if radar_info, scan_segment, ts_processing and calibration
  // are all active
  
  bool isEssentialInfoReady() const;

  // get time for each packet
  
  double getRadarInfoTime() const;
  double getScanSegmentTime() const;
  double getAntennaCorrectionTime() const;
  double getTsProcessingTime() const;
  double getXmitPowerTime() const;
  double getRxPowerTime() const;
  double getXmitSampleTime() const;
  double getStatusXmlTime() const;
  double getCalibrationTime() const;
  double getEventNoticeTime() const;
  double getPhasecodeTime() const;
  double getXmitInfoTime() const;
  double getRvp8InfoTime() const;
  double getPlatformGeorefTime() const;
  double getPlatformGeoref1Time() const;

  // get packet sequence numbers

  si64 getRadarInfoPktSeqNum() const { return _radar_info.packet.seq_num; }
  si64 getScanSegmentPktSeqNum() const { return _scan_seg.packet.seq_num; }
  si64 getAntennaCorrectionPktSeqNum() const { return _ant_corr.packet.seq_num; }
  si64 getTsProcessingPktSeqNum() const { return _proc.packet.seq_num; }
  si64 getXmitPowerPktSeqNum() const { return _xmit_power.packet.seq_num; }
  si64 getRxPowerPktSeqNum() const { return _rx_power.packet.seq_num; }
  si64 getXmitSamplePktSeqNum() const { return _xmit_sample.packet.seq_num; }
  si64 getStatusXmlPktSeqNum() const { return _status_xml_hdr.packet.seq_num; }
  si64 getCalibrationPktSeqNum() const { return _calib.packet.seq_num; }
  si64 getEventNoticePktSeqNum() const { return _enotice.packet.seq_num; }
  si64 getPhasecodePktSeqNum() const { return _phasecode.packet.seq_num; }
  si64 getXmitInfoPktSeqNum() const { return _xmit_info.packet.seq_num; }
  si64 getRvp8InfoPktSeqNum() const { return _rvp8.packet.seq_num; }
  si64 getPlatformGeorefPktSeqNum() const { 
    return _platform_georef0.packet.seq_num; 
  }
  si64 getPlatformGeoref1PktSeqNum() const { 
    return _platform_georef1.packet.seq_num; 
  }

  // is id for an info packet? Check the id
  
  static bool isInfo(int id);
  
  // get sweep and volume flags
  
  bool isStartOfSweep() const { return _startOfSweepFlag; }
  bool isEndOfSweep() const { return _endOfSweepFlag; }
  bool isStartOfVolume() const { return _startOfVolumeFlag; }
  bool isEndOfVolume() const { return _endOfVolumeFlag; }

  ////////////////////////////////////////////////////////////
  // set individual fields

  // set radar_info fields
  
  inline void set_radar_latitude_deg(fl32 x) { _radar_info.latitude_deg = x; }
  inline void set_radar_longitude_deg(fl32 x) { _radar_info.longitude_deg = x; }
  inline void set_radar_altitude_m(fl32 x) { _radar_info.altitude_m = x; }
  inline void set_radar_platform_type(int x) { _radar_info.platform_type = x; }
  inline void set_radar_beamwidth_deg_h(fl32 x) { _radar_info.beamwidth_deg_h = x; }
  inline void set_radar_beamwidth_deg_v(fl32 x) { _radar_info.beamwidth_deg_v = x; }
  inline void set_radar_wavelength_cm(fl32 x) { _radar_info.wavelength_cm = x; }
  inline void set_radar_nominal_gain_ant_db_h(fl32 x) { _radar_info.nominal_gain_ant_db_h = x; }
  inline void set_radar_nominal_gain_ant_db_v(fl32 x) { _radar_info.nominal_gain_ant_db_v = x; }
  void set_radar_name(const string &name);
  void set_radar_site_name(const string &name);

  // set scan_segment fields

  inline void set_scan_mode(si32 x) { _scan_seg.scan_mode = x; }
  inline void set_scan_follow_mode(si32 x) { _scan_seg.follow_mode = x; }
  inline void set_scan_volume_num(si32 x) { _scan_seg.volume_num = x; }
  inline void set_scan_sweep_num(si32 x) { _scan_seg.sweep_num = x; }
  inline void set_scan_time_limit(si32 x) { _scan_seg.time_limit = x; }
  inline void set_scan_az_manual(fl32 x) { _scan_seg.az_manual = x; }
  inline void set_scan_el_manual(fl32 x) { _scan_seg.el_manual = x; }
  inline void set_scan_az_start(fl32 x) { _scan_seg.az_start = x; }
  inline void set_scan_el_start(fl32 x) { _scan_seg.el_start = x; }
  inline void set_scan_rate(fl32 x) { _scan_seg.scan_rate = x; }
  inline void set_scan_left_limit(fl32 x) { _scan_seg.left_limit = x; }
  inline void set_scan_right_limit(fl32 x) { _scan_seg.right_limit = x; }
  inline void set_scan_up_limit(fl32 x) { _scan_seg.up_limit = x; }
  inline void set_scan_down_limit(fl32 x) { _scan_seg.down_limit = x; }
  inline void set_scan_step(fl32 x) { _scan_seg.step = x; }
  inline void set_scan_current_fixed_angle(fl32 x) { _scan_seg.current_fixed_angle = x; }
  inline void set_scan_init_direction_cw(si32 x) { _scan_seg.init_direction_cw = x; }
  inline void set_scan_init_direction_up(si32 x) { _scan_seg.init_direction_up = x; }
  inline void set_scan_n_sweeps(si32 x) { _scan_seg.n_sweeps = x; }
  void set_scan_fixed_angle(int i, fl32 x);
  inline void set_scan_optimizer_rmax_km(fl32 x) { _scan_seg.optimizer_rmax_km = x; }
  inline void set_scan_optimizer_htmax_km(fl32 x) { _scan_seg.optimizer_htmax_km = x; }
  inline void set_scan_optimizer_res_m(fl32 x) { _scan_seg.optimizer_res_m = x; }
  void set_scan_segment_name(const string &x);
  void set_scan_project_name(const string &x);

  // set antenna correction fields

  inline void set_az_correction(fl32 x) { _ant_corr.az_correction = x; }
  inline void set_el_correction(fl32 x) { _ant_corr.el_correction = x; }
  
  // set time-series processing fields

  inline void set_proc_xmit_rcv_mode(si32 x) { _proc.xmit_rcv_mode = x; }
  inline void set_proc_xmit_phase_mode(si32 x) { _proc.xmit_phase_mode = x; }
  inline void set_proc_prf_mode(si32 x) { _proc.prf_mode = x; }
  inline void set_proc_pulse_type(si32 x) { _proc.pulse_type = x; }
  inline void set_proc_prt_usec(fl32 x) { _proc.prt_usec = x; }
  inline void set_proc_prt2_usec(fl32 x) { _proc.prt2_usec = x; }
  inline void set_proc_cal_type(si32 x) { _proc.cal_type = x; }
  inline void set_proc_burst_range_offset_m(fl32 x) {
    _proc.burst_range_offset_m = x;
  }
  inline void set_proc_pulse_width_us(fl32 x) { _proc.pulse_width_us = x; }
  inline void set_proc_start_range_m(fl32 x) { _proc.start_range_m = x; }
  inline void set_proc_gate_spacing_m(fl32 x) { _proc.gate_spacing_m = x; }
  inline void set_proc_integration_cycle_pulses(si32 x) {
    _proc.integration_cycle_pulses = x;
  }
  inline void set_proc_clutter_filter_number(si32 x) {
    _proc.clutter_filter_number = x;
  }
  inline void set_proc_range_gate_averaging(si32 x) { _proc.range_gate_averaging = x; }
  inline void set_proc_max_gate(si32 x) { _proc.max_gate = x; }
  inline void set_proc_test_power_dbm(fl32 x) { _proc.test_power_dbm = x; }
  inline void set_proc_test_pulse_range_km(fl32 x) { _proc.test_pulse_range_km = x; }
  inline void set_proc_test_pulse_length_usec(fl32 x) { _proc.test_pulse_length_usec = x; }
  inline void set_proc_pol_mode(si32 x) { _proc.pol_mode = x; }
  inline void set_proc_xmit_flag0(si32 x) { _proc.xmit_flag[0] = x; }
  inline void set_proc_xmit_flag1(si32 x) { _proc.xmit_flag[1] = x; }
  inline void set_proc_beams_are_indexed(si32 x) { _proc.beams_are_indexed = x; }
  inline void set_proc_specify_dwell_width(si32 x) { _proc.specify_dwell_width = x; }
  inline void set_proc_indexed_beam_width_deg(fl32 x) { _proc.indexed_beam_width_deg = x; }
  inline void set_proc_indexed_beam_spacing_deg(fl32 x) { _proc.indexed_beam_spacing_deg = x; }

  inline void set_proc_num_prts(si32 x) { _proc.num_prts = x; }
  inline void set_proc_prt3_usec(fl32 x) { _proc.prt3_usec = x; }
  inline void set_proc_prt4_usec(fl32 x) { _proc.prt4_usec = x; }
  
  inline void set_proc_block_mode_prt2_pulses(si32 x) { _proc.block_mode_prt2_pulses = x; }
  inline void set_proc_block_mode_prt3_pulses(si32 x) { _proc.block_mode_prt3_pulses = x; }
  inline void set_proc_block_mode_prt4_pulses(si32 x) { _proc.block_mode_prt4_pulses = x; }

  // set xmit_power fields

  inline void set_xmit_power_dbm_h(fl32 x) { _xmit_power.power_dbm_h = x; }
  inline void set_xmit_power_dbm_v(fl32 x) { _xmit_power.power_dbm_v = x; }
  
  // set rx_power fields

  inline void set_rx_max_power_dbm_hc(fl32 x) {
    _rx_power.max_power_dbm_hc = x;
  }
  inline void set_rx_max_power_dbm_vc(fl32 x) {
    _rx_power.max_power_dbm_vc = x;
  }
  inline void set_rx_max_power_dbm_hx(fl32 x) {
    _rx_power.max_power_dbm_hx = x;
  }
  inline void set_rx_max_power_dbm_vx(fl32 x) {
    _rx_power.max_power_dbm_vx = x;
  }
  
  // set xmit_sample fields

  inline void set_xmit_sample_dbm_h(fl32 x) { _xmit_sample.power_dbm_h = x; }
  inline void set_xmit_sample_dbm_v(fl32 x) { _xmit_sample.power_dbm_v = x; }
  inline void set_xmit_sample_offset(si32 x) { _xmit_sample.offset = x; }
  inline void set_xmit_n_samples(si32 x) { _xmit_sample.n_samples = x; }
  inline void set_xmit_sampling_freq(fl32 x) { _xmit_sample.sampling_freq = x; }
  inline void set_xmit_sample_scale_h(fl32 x) { _xmit_sample.scale_h = x; }
  inline void set_xmit_sample_offset_h(fl32 x) { _xmit_sample.offset_h = x; }
  inline void set_xmit_sample_scale_v(fl32 x) { _xmit_sample.scale_v = x; }
  inline void set_xmit_sample_offset_v(fl32 x) { _xmit_sample.offset_v = x; }
  void set_xmit_sample_h(int i, si32 val);
  void set_xmit_sample_v(int i, si32 val);

  // set calibration fields

  inline void set_calib_wavelength_cm(fl32 x) { _calib.wavelength_cm = x; }
  inline void set_calib_beamwidth_deg_h(fl32 x) { _calib.beamwidth_deg_h = x; }
  inline void set_calib_beamwidth_deg_v(fl32 x) { _calib.beamwidth_deg_v = x; }
  inline void set_calib_gain_ant_db_h(fl32 x) { _calib.gain_ant_db_h = x; }
  inline void set_calib_gain_ant_db_v(fl32 x) { _calib.gain_ant_db_v = x; }
  inline void set_calib_pulse_width_us(fl32 x) { _calib.pulse_width_us = x; }
  inline void set_calib_xmit_power_dbm_h(fl32 x) { _calib.xmit_power_dbm_h = x; }
  inline void set_calib_xmit_power_dbm_v(fl32 x) { _calib.xmit_power_dbm_v = x; }
  inline void set_calib_two_way_waveguide_loss_db_h(fl32 x) {
    _calib.two_way_waveguide_loss_db_h = x;
  }
  inline void set_calib_two_way_waveguide_loss_db_v(fl32 x) {
    _calib.two_way_waveguide_loss_db_v = x;
  }
  inline void set_calib_two_way_radome_loss_db_h(fl32 x) { _calib.two_way_radome_loss_db_h = x; }
  inline void set_calib_two_way_radome_loss_db_v(fl32 x) { _calib.two_way_radome_loss_db_v = x; }
  inline void set_calib_receiver_mismatch_loss_db(fl32 x) {
    _calib.receiver_mismatch_loss_db = x;
  }
  inline void set_calib_radar_constant_h(fl32 x) { _calib.radar_constant_h = x; }
  inline void set_calib_radar_constant_v(fl32 x) { _calib.radar_constant_v = x; }
  inline void set_calib_noise_dbm_hc(fl32 x) { _calib.noise_dbm_hc = x; }
  inline void set_calib_noise_dbm_hx(fl32 x) { _calib.noise_dbm_hx = x; }
  inline void set_calib_noise_dbm_vc(fl32 x) { _calib.noise_dbm_vc = x; }
  inline void set_calib_noise_dbm_vx(fl32 x) { _calib.noise_dbm_vx = x; }
  inline void set_calib_receiver_gain_db_hc(fl32 x) { _calib.receiver_gain_db_hc = x; }
  inline void set_calib_receiver_gain_db_hx(fl32 x) { _calib.receiver_gain_db_hx = x; }
  inline void set_calib_receiver_gain_db_vc(fl32 x) { _calib.receiver_gain_db_vc = x; }
  inline void set_calib_receiver_gain_db_vx(fl32 x) { _calib.receiver_gain_db_vx = x; }
  inline void set_calib_receiver_slope_hc(fl32 x) { _calib.receiver_slope_hc = x; }
  inline void set_calib_receiver_slope_hx(fl32 x) { _calib.receiver_slope_hx = x; }
  inline void set_calib_receiver_slope_vc(fl32 x) { _calib.receiver_slope_vc = x; }
  inline void set_calib_receiver_slope_vx(fl32 x) { _calib.receiver_slope_vx = x; }
  inline void set_calib_base_dbz_1km_hc(fl32 x) { _calib.base_dbz_1km_hc = x; }
  inline void set_calib_base_dbz_1km_hx(fl32 x) { _calib.base_dbz_1km_hx = x; }
  inline void set_calib_base_dbz_1km_vc(fl32 x) { _calib.base_dbz_1km_vc = x; }
  inline void set_calib_base_dbz_1km_vx(fl32 x) { _calib.base_dbz_1km_vx = x; }
  inline void set_calib_sun_power_dbm_hc(fl32 x) { _calib.sun_power_dbm_hc = x; }
  inline void set_calib_sun_power_dbm_hx(fl32 x) { _calib.sun_power_dbm_hx = x; }
  inline void set_calib_sun_power_dbm_vc(fl32 x) { _calib.sun_power_dbm_vc = x; }
  inline void set_calib_sun_power_dbm_vx(fl32 x) { _calib.sun_power_dbm_vx = x; }
  inline void set_calib_noise_source_power_dbm_h(fl32 x) { _calib.noise_source_power_dbm_h = x; }
  inline void set_calib_noise_source_power_dbm_v(fl32 x) { _calib.noise_source_power_dbm_v = x; }
  inline void set_calib_power_meas_loss_db_h(fl32 x) { _calib.power_meas_loss_db_h = x; }
  inline void set_calib_power_meas_loss_db_v(fl32 x) { _calib.power_meas_loss_db_v = x; }
  inline void set_calib_coupler_forward_loss_db_h(fl32 x) {
    _calib.coupler_forward_loss_db_h = x;
  }
  inline void set_calib_coupler_forward_loss_db_v(fl32 x) {
    _calib.coupler_forward_loss_db_v = x;
  }
  inline void set_calib_test_power_dbm_h(fl32 x) { _calib.test_power_dbm_h = x; }
  inline void set_calib_test_power_dbm_v(fl32 x) { _calib.test_power_dbm_v = x; }
  inline void set_calib_zdr_correction_db(fl32 x) { _calib.zdr_correction_db = x; }
  inline void set_calib_ldr_correction_db_h(fl32 x) { _calib.ldr_correction_db_h = x; }
  inline void set_calib_ldr_correction_db_v(fl32 x) { _calib.ldr_correction_db_v = x; }
  inline void set_calib_phidp_rot_deg(fl32 x) { _calib.phidp_rot_deg = x; }
  
  // set event notice fields

  inline void set_enotice_start_of_sweep(si32 x) { _enotice.start_of_sweep = x; }
  inline void set_enotice_end_of_sweep(si32 x) { _enotice.end_of_sweep = x; }
  inline void set_enotice_start_of_volume(si32 x) { _enotice.start_of_volume = x; }
  inline void set_enotice_end_of_volume(si32 x) { _enotice.end_of_volume = x; }
  inline void set_enotice_scan_mode(si32 x) { _enotice.scan_mode = x; }
  inline void set_enotice_follow_mode(si32 x) { _enotice.follow_mode = x; }
  inline void set_enotice_volume_num(si32 x) { _enotice.volume_num = x; }
  inline void set_enotice_sweep_num(si32 x) { _enotice.sweep_num = x; }
  inline void set_enotice_cause(si32 x) { _enotice.cause = x; }
  
  // set phasecode fields

  inline void set_phasecode_seq_length(int x) { _phasecode.seq_length = x; }
  void set_phasecode_deg_h(int i, fl32 x);
  void set_phasecode_deg_v(int i, fl32 x);

  // xmit_info fields

  inline void set_xmit_info_xmit_0_enabled(si32 x) { _xmit_info.xmit_0_enabled = x; }
  inline void set_xmit_info_xmit_1_enabled(si32 x) { _xmit_info.xmit_1_enabled = x; }
  inline void set_xmit_info_xmit_rcv_mode(si32 x) { _xmit_info.xmit_rcv_mode = x; }
  inline void set_xmit_info_xmit_phase_mode(si32 x) { _xmit_info.xmit_phase_mode = x; }
  inline void set_xmit_info_prf_mode(si32 x) { _xmit_info.prf_mode = x; }
  inline void set_xmit_info_pulse_type(si32 x) { _xmit_info.pulse_type = x; }
  inline void set_xmit_info_prt_usec(fl32 x) { _xmit_info.prt_usec = x; }
  inline void set_xmit_info_prt2_usec(fl32 x) { _xmit_info.prt2_usec = x; }
  
  // rvp8 fields

  inline void set_rvp8_i_version(si32 x) { _rvp8.i_version = x; }
  inline void set_rvp8_i_major_mode(ui32 x) { _rvp8.i_major_mode = x; }
  inline void set_rvp8_i_polarization(ui32 x) { _rvp8.i_polarization = x; }
  inline void set_rvp8_i_phase_mode_seq(ui32 x) { _rvp8.i_phase_mode_seq = x; }
  inline void set_rvp8_i_task_sweep(ui16 x) { _rvp8.i_task_sweep = x; }
  inline void set_rvp8_i_task_aux_num(ui16 x) { _rvp8.i_task_aux_num = x; }
  inline void set_rvp8_i_task_scan_type(si32 x) { _rvp8.i_task_scan_type = x; }
  inline void set_rvp8_s_task_name(string x) { iwrf_safe_str(_rvp8.s_task_name, 32) = x; }
  inline void set_rvp8_s_site_name(string x) { iwrf_safe_str(_rvp8.s_site_name, 32) = x; }
  inline void set_rvp8_i_aq_mode(ui32 x) { _rvp8.i_aq_mode = x; }
  inline void set_rvp8_i_unfold_mode(ui32 x) { _rvp8.i_unfold_mode = x; }
  inline void set_rvp8_i_pwidth_code(ui32 x) { _rvp8.i_pwidth_code = x; }
  inline void set_rvp8_f_pwidth_usec(fl32 x) { _rvp8.f_pwidth_usec = x; }
  inline void set_rvp8_f_dbz_calib(fl32 x) { _rvp8.f_dbz_calib = x; }
  inline void set_rvp8_i_sample_size(si32 x) { _rvp8.i_sample_size = x; }
  inline void set_rvp8_i_mean_angle_sync(ui32 x) { _rvp8.i_mean_angle_sync = x; }
  inline void set_rvp8_i_flags(ui32 x) { _rvp8.i_flags = x; }
  inline void set_rvp8_i_playback_version(si32 x) { _rvp8.i_playback_version = x; }
  inline void set_rvp8_f_sy_clk_mhz(fl32 x) { _rvp8.f_sy_clk_mhz = x; }
  inline void set_rvp8_f_wavelength_cm(fl32 x) { _rvp8.f_wavelength_cm = x; }
  inline void set_rvp8_f_saturation_dbm(fl32 x) {
    _rvp8.f_saturation_dbm = x;
    _rvp8SaturationMult = pow(10.0, x / 20.0);
  }
  inline void set_rvp8_f_range_mask_res(fl32 x) { _rvp8.f_range_mask_res = x; }
  void set_rvp8_i_range_mask(int i, ui16 x);
  void set_rvp8_f_noise_dbm(int chan, fl32 x);
  void set_rvp8_f_noise_stdv_db(int chan, fl32 x);
  inline void set_rvp8_f_noise_range_km(fl32 x) { _rvp8.f_noise_range_km = x; }
  inline void set_rvp8_f_noise_prf_hz(fl32 x) { _rvp8.f_noise_prf_hz = x; }
  void set_rvp8_i_gparm_latch_sts(int i, ui16 x);
  void set_rvp8_i_gparm_immed_sts(int i, ui16 x);
  void set_rvp8_i_gparm_diag_bits(int i, ui16 x);
  void set_rvp8_s_version_string(const string &x);

  ////////////////////////////////////////////////////////////
  // get individual fields

  // get radar_info fields

  inline fl32 get_radar_latitude_deg() const { return _radar_info.latitude_deg; }
  inline fl32 get_radar_longitude_deg() const { return _radar_info.longitude_deg; }
  inline fl32 get_radar_altitude_m() const { return _radar_info.altitude_m; }
  inline si32 get_radar_platform_type() const { return _radar_info.platform_type; }
  inline fl32 get_radar_beamwidth_deg_h() const { return _radar_info.beamwidth_deg_h; }
  inline fl32 get_radar_beamwidth_deg_v() const { return _radar_info.beamwidth_deg_v; }
  inline fl32 get_radar_wavelength_cm() const { return _radar_info.wavelength_cm; }
  inline fl32 get_radar_nominal_gain_ant_db_h() const { return _radar_info.nominal_gain_ant_db_h; }
  inline fl32 get_radar_nominal_gain_ant_db_v() const { return _radar_info.nominal_gain_ant_db_v; }
  string get_radar_name() const;
  string get_radar_site_name() const;

  // get scan_segment fields

  inline si32 get_scan_mode() const { return _scan_seg.scan_mode; }
  inline si32 get_scan_follow_mode() const { return _scan_seg.follow_mode; }
  inline si32 get_scan_volume_num() const { return _scan_seg.volume_num; }
  inline si32 get_scan_sweep_num() const { return _scan_seg.sweep_num; }
  inline si32 get_scan_time_limit() const { return _scan_seg.time_limit; }
  inline fl32 get_scan_az_manual() const { return _scan_seg.az_manual; }
  inline fl32 get_scan_el_manual() const { return _scan_seg.el_manual; }
  inline fl32 get_scan_az_start() const { return _scan_seg.az_start; }
  inline fl32 get_scan_el_start() const { return _scan_seg.el_start; }
  inline fl32 get_scan_rate() const { return _scan_seg.scan_rate; }
  inline fl32 get_scan_left_limit() const { return _scan_seg.left_limit; }
  inline fl32 get_scan_right_limit() const { return _scan_seg.right_limit; }
  inline fl32 get_scan_up_limit() const { return _scan_seg.up_limit; }
  inline fl32 get_scan_down_limit() const { return _scan_seg.down_limit; }
  inline fl32 get_scan_step() const { return _scan_seg.step; }
  inline fl32 get_scan_current_fixed_angle() const { return _scan_seg.current_fixed_angle; }
  inline si32 get_scan_init_direction_cw() const { return _scan_seg.init_direction_cw; }
  inline si32 get_scan_init_direction_up() const { return _scan_seg.init_direction_up; }
  inline si32 get_scan_n_sweeps() const { return _scan_seg.n_sweeps; }
  fl32 get_scan_fixed_angle(int i) const;
  inline fl32 get_scan_optimizer_rmax_km() const { return _scan_seg.optimizer_rmax_km; }
  inline fl32 get_scan_optimizer_htmax_km() const { return _scan_seg.optimizer_htmax_km; }
  inline fl32 get_scan_optimizer_res_m() const { return _scan_seg.optimizer_res_m; }
  string get_scan_segment_name() const;
  string get_scan_project_name() const;

  // get antenna correction fields

  inline fl32 get_az_correction() const { return _ant_corr.az_correction; }
  inline fl32 get_el_correction() const { return _ant_corr.el_correction; }

  // get time-series processing fields

  inline si32 get_proc_xmit_rcv_mode() const { return _proc.xmit_rcv_mode; }
  inline si32 get_proc_xmit_phase_mode() const { return _proc.xmit_phase_mode; }
  inline si32 get_proc_prf_mode() const { return _proc.prf_mode; }
  inline si32 get_proc_pulse_type() const { return _proc.pulse_type; }
  inline fl32 get_proc_prt_usec() const { return _proc.prt_usec; }
  inline fl32 get_proc_prt2_usec() const { return _proc.prt2_usec; }
  inline si32 get_proc_cal_type() const { return _proc.cal_type; }
  
  inline fl32 get_proc_burst_range_offset_m() const {
    return _proc.burst_range_offset_m;
  }
  
  inline fl32 get_proc_pulse_width_us() const { return _proc.pulse_width_us; }

  inline fl32 get_proc_start_range_m() const { return _proc.start_range_m; }
  inline fl32 get_proc_start_range_km() const {
    return _proc.start_range_m / 1000.0;
  }

  inline fl32 get_proc_gate_spacing_m() const { return _proc.gate_spacing_m; }
  inline fl32 get_proc_gate_spacing_km() const {
    return _proc.gate_spacing_m / 1000.0;
  }

  inline si32 get_proc_integration_cycle_pulses() const {
    return _proc.integration_cycle_pulses;
  }
  inline si32 get_proc_clutter_filter_number() const {
    return _proc.clutter_filter_number;
  }
  inline si32 get_proc_range_gate_averaging() const { return _proc.range_gate_averaging; }
  inline si32 get_proc_max_gate() const { return _proc.max_gate; }
  inline fl32 get_proc_test_power_dbm() const { return _proc.test_power_dbm; }
  inline fl32 get_proc_test_pulse_range_km() const { return _proc.test_pulse_range_km; }
  inline fl32 get_proc_test_pulse_length_usec() const { return _proc.test_pulse_length_usec; }
  inline si32 get_proc_pol_mode() const { return _proc.pol_mode; }
  inline si32 get_proc_xmit_flag0() const { return _proc.xmit_flag[0]; }
  inline si32 get_proc_xmit_flag1() const { return _proc.xmit_flag[1]; }
  inline si32 get_proc_beams_are_indexed() const { return _proc.beams_are_indexed; }
  inline si32 get_proc_specify_dwell_width() const { return _proc.specify_dwell_width; }
  inline fl32 get_proc_indexed_beam_width_deg() const { return _proc.indexed_beam_width_deg; }
  inline fl32 get_proc_indexed_beam_spacing_deg() const { return _proc.indexed_beam_spacing_deg; }

  inline si32 get_proc_num_prts() const { return _proc.num_prts; }
  inline fl32 get_proc_prt3_usec() const { return _proc.prt3_usec; }
  inline fl32 get_proc_prt4_usec() const { return _proc.prt4_usec; }

  inline si32 get_proc_block_mode_prt2_pulses() const { return _proc.block_mode_prt2_pulses; }
  inline si32 get_proc_block_mode_prt3_pulses() const { return _proc.block_mode_prt3_pulses; }
  inline si32 get_proc_block_mode_prt4_pulses() const { return _proc.block_mode_prt4_pulses; }

  // get xmit_power fields

  inline fl32 get_xmit_power_dbm_h() const { return _xmit_power.power_dbm_h; }
  inline fl32 get_xmit_power_dbm_v() const { return _xmit_power.power_dbm_v; }

  // get rx_power fields

  inline fl32 get_rx_max_power_dbm_hc() const {
    return _rx_power.max_power_dbm_hc;
  }
  inline fl32 get_rx_max_power_dbm_vc() const {
    return _rx_power.max_power_dbm_vc;
  }
  inline fl32 get_rx_max_power_dbm_hx() const {
    return _rx_power.max_power_dbm_hx;
  }
  inline fl32 get_rx_max_power_dbm_vx() const {
    return _rx_power.max_power_dbm_vx;
  }

  // get xmit_sample fields

  inline fl32 get_xmit_sample_dbm_h() const { return _xmit_sample.power_dbm_h; }
  inline fl32 get_xmit_sample_dbm_v() const { return _xmit_sample.power_dbm_v; }
  inline si32 get_xmit_sample_offset() const { return _xmit_sample.offset; }
  inline si32 get_xmit_n_samples() const { return _xmit_sample.n_samples; }
  inline fl32 get_xmit_sampling_freq() const { return _xmit_sample.sampling_freq; }
  inline fl32 get_xmit_sample_scale_h() const { return _xmit_sample.scale_h; }
  inline fl32 get_xmit_sample_offset_h() const { return _xmit_sample.offset_h; }
  inline fl32 get_xmit_sample_scale_v() const { return _xmit_sample.scale_v; }
  inline fl32 get_xmit_sample_offset_v() const { return _xmit_sample.offset_v; }
  si32 get_xmit_sample_h(int i) const;
  si32 get_xmit_sample_v(int i) const;

  // get calibration fields

  inline fl32 get_calib_wavelength_cm() const { return _calib.wavelength_cm; }
  inline fl32 get_calib_beamwidth_deg_h() const { return _calib.beamwidth_deg_h; }
  inline fl32 get_calib_beamwidth_deg_v() const { return _calib.beamwidth_deg_v; }
  inline fl32 get_calib_gain_ant_db_h() const { return _calib.gain_ant_db_h; }
  inline fl32 get_calib_gain_ant_db_v() const { return _calib.gain_ant_db_v; }
  inline fl32 get_calib_pulse_width_us() const { return _calib.pulse_width_us; }
  inline fl32 get_calib_xmit_power_dbm_h() const { return _calib.xmit_power_dbm_h; }
  inline fl32 get_calib_xmit_power_dbm_v() const { return _calib.xmit_power_dbm_v; }
  inline fl32 get_calib_two_way_waveguide_loss_db_h() const {
    return _calib.two_way_waveguide_loss_db_h;
  }
  inline fl32 get_calib_two_way_waveguide_loss_db_v() const {
    return _calib.two_way_waveguide_loss_db_v;
  }
  inline fl32 get_calib_two_way_radome_loss_db_h() const { return _calib.two_way_radome_loss_db_h; }
  inline fl32 get_calib_two_way_radome_loss_db_v() const { return _calib.two_way_radome_loss_db_v; }
  inline fl32 get_calib_receiver_mismatch_loss_db() const {
    return _calib.receiver_mismatch_loss_db;
  }
  inline fl32 get_calib_radar_constant_h() const { return _calib.radar_constant_h; }
  inline fl32 get_calib_radar_constant_v() const { return _calib.radar_constant_v; }
  inline fl32 get_calib_noise_dbm_hc() const { return _calib.noise_dbm_hc; }
  inline fl32 get_calib_noise_dbm_hx() const { return _calib.noise_dbm_hx; }
  inline fl32 get_calib_noise_dbm_vc() const { return _calib.noise_dbm_vc; }
  inline fl32 get_calib_noise_dbm_vx() const { return _calib.noise_dbm_vx; }
  inline fl32 get_calib_receiver_gain_db_hc() const { return _calib.receiver_gain_db_hc; }
  inline fl32 get_calib_receiver_gain_db_hx() const { return _calib.receiver_gain_db_hx; }
  inline fl32 get_calib_receiver_gain_db_vc() const { return _calib.receiver_gain_db_vc; }
  inline fl32 get_calib_receiver_gain_db_vx() const { return _calib.receiver_gain_db_vx; }
  inline fl32 get_calib_receiver_slope_hc() const { return _calib.receiver_slope_hc; }
  inline fl32 get_calib_receiver_slope_hx() const { return _calib.receiver_slope_hx; }
  inline fl32 get_calib_receiver_slope_vc() const { return _calib.receiver_slope_vc; }
  inline fl32 get_calib_receiver_slope_vx() const { return _calib.receiver_slope_vx; }
  inline fl32 get_calib_base_dbz_1km_hc() const { return _calib.base_dbz_1km_hc; }
  inline fl32 get_calib_base_dbz_1km_hx() const { return _calib.base_dbz_1km_hx; }
  inline fl32 get_calib_base_dbz_1km_vc() const { return _calib.base_dbz_1km_vc; }
  inline fl32 get_calib_base_dbz_1km_vx() const { return _calib.base_dbz_1km_vx; }
  inline fl32 get_calib_sun_power_dbm_hc() const { return _calib.sun_power_dbm_hc; }
  inline fl32 get_calib_sun_power_dbm_hx() const { return _calib.sun_power_dbm_hx; }
  inline fl32 get_calib_sun_power_dbm_vc() const { return _calib.sun_power_dbm_vc; }
  inline fl32 get_calib_sun_power_dbm_vx() const { return _calib.sun_power_dbm_vx; }
  inline fl32 get_calib_noise_source_power_dbm_h() const { return _calib.noise_source_power_dbm_h; }
  inline fl32 get_calib_noise_source_power_dbm_v() const { return _calib.noise_source_power_dbm_v; }
  inline fl32 get_calib_power_meas_loss_db_h() const { return _calib.power_meas_loss_db_h; }
  inline fl32 get_calib_power_meas_loss_db_v() const { return _calib.power_meas_loss_db_v; }
  inline fl32 get_calib_coupler_forward_loss_db_h() const {
    return _calib.coupler_forward_loss_db_h;
  }
  inline fl32 get_calib_coupler_forward_loss_db_v() const {
    return _calib.coupler_forward_loss_db_v;
  }
  inline fl32 get_calib_test_power_dbm_h() const { return _calib.test_power_dbm_h; }
  inline fl32 get_calib_test_power_dbm_v() const { return _calib.test_power_dbm_v; }
  inline fl32 get_calib_zdr_correction_db() const { return _calib.zdr_correction_db; }
  inline fl32 get_calib_ldr_correction_db_h() const { return _calib.ldr_correction_db_h; }
  inline fl32 get_calib_ldr_correction_db_v() const { return _calib.ldr_correction_db_v; }
  inline fl32 get_calib_phidp_rot_deg() const { return _calib.phidp_rot_deg; }
  
  // get event notice fields

  inline si32 get_enotice_start_of_sweep() const { return _enotice.start_of_sweep; }
  inline si32 get_enotice_end_of_sweep() const { return _enotice.end_of_sweep; }
  inline si32 get_enotice_start_of_volume() const { return _enotice.start_of_volume; }
  inline si32 get_enotice_end_of_volume() const { return _enotice.end_of_volume; }
  inline si32 get_enotice_scan_mode() const { return _enotice.scan_mode; }
  inline si32 get_enotice_follow_mode() const { return _enotice.follow_mode; }
  inline si32 get_enotice_volume_num() const { return _enotice.volume_num; }
  inline si32 get_enotice_sweep_num() const { return _enotice.sweep_num; }
  inline si32 get_enotice_cause() const { return _enotice.cause; }

  // get phasecode fields

  inline int get_phasecode_seq_length() const { return _phasecode.seq_length; }
  fl32 get_phasecode_deg_h(int i) const;
  fl32 get_phasecode_deg_v(int i) const;

  // xmit_info fields

  inline si32 get_xmit_info_xmit_0_enabled() const { return _xmit_info.xmit_0_enabled; }
  inline si32 get_xmit_info_xmit_1_enabled() const { return _xmit_info.xmit_1_enabled; }
  inline si32 get_xmit_info_xmit_rcv_mode() const { return _xmit_info.xmit_rcv_mode; }
  inline si32 get_xmit_info_xmit_phase_mode() const { return _xmit_info.xmit_phase_mode; }
  inline si32 get_xmit_info_prf_mode() const { return _xmit_info.prf_mode; }
  inline si32 get_xmit_info_pulse_type() const { return _xmit_info.pulse_type; }
  inline fl32 get_xmit_info_prt_usec() const { return _xmit_info.prt_usec; }
  inline fl32 get_xmit_info_prt2_usec() const { return _xmit_info.prt2_usec; }

  // rvp8 fields

  inline si32 get_rvp8_i_version() const { return _rvp8.i_version; }
  inline ui32 get_rvp8_i_major_mode() const { return _rvp8.i_major_mode; }
  inline ui32 get_rvp8_i_polarization() const { return _rvp8.i_polarization; }
  inline ui32 get_rvp8_i_phase_mode_seq() const { return _rvp8.i_phase_mode_seq; }
  inline ui16 get_rvp8_i_task_sweep() const { return _rvp8.i_task_sweep; }
  inline ui16 get_rvp8_i_task_aux_num() const { return _rvp8.i_task_aux_num; }
  inline si32 get_rvp8_i_task_scan_type() const { return _rvp8.i_task_scan_type; }
  inline string get_rvp8_s_task_name() const { return iwrf_safe_str(_rvp8.s_task_name, 32); }
  inline string get_rvp8_s_site_name() const { return iwrf_safe_str(_rvp8.s_site_name, 32); }
  inline ui32 get_rvp8_i_aq_mode() const { return _rvp8.i_aq_mode; }
  inline ui32 get_rvp8_i_unfold_mode() const { return _rvp8.i_unfold_mode; }
  inline ui32 get_rvp8_i_pwidth_code() const { return _rvp8.i_pwidth_code; }
  inline fl32 get_rvp8_f_pwidth_usec() const { return _rvp8.f_pwidth_usec; }
  inline fl32 get_rvp8_f_dbz_calib() const { return _rvp8.f_dbz_calib; }
  inline si32 get_rvp8_i_sample_size() const { return _rvp8.i_sample_size; }
  inline ui32 get_rvp8_i_mean_angle_sync() const { return _rvp8.i_mean_angle_sync; }
  inline ui32 get_rvp8_i_flags() const { return _rvp8.i_flags; }
  inline si32 get_rvp8_i_playback_version() const { return _rvp8.i_playback_version; }
  inline fl32 get_rvp8_f_sy_clk_mhz() const { return _rvp8.f_sy_clk_mhz; }
  inline fl32 get_rvp8_f_wavelength_cm() const { return _rvp8.f_wavelength_cm; }
  inline fl32 get_rvp8_f_saturation_dbm() const { return _rvp8.f_saturation_dbm; }
  inline fl32 get_rvp8_f_range_mask_res() const { return _rvp8.f_range_mask_res; }
  ui16 get_rvp8_i_range_mask(int i) const;
  fl32 get_rvp8_f_noise_dbm(int chan) const;
  fl32 get_rvp8_f_noise_stdv_db(int chan) const;
  inline fl32 get_rvp8_f_noise_range_km() const { return _rvp8.f_noise_range_km; }
  inline fl32 get_rvp8_f_noise_prf_hz() const { return _rvp8.f_noise_prf_hz; }
  ui16 get_rvp8_i_gparm_latch_sts(int i) const;
  ui16 get_rvp8_i_gparm_immed_sts(int i) const;
  ui16 get_rvp8_i_gparm_diag_bits(int i) const;
  string get_rvp8_s_version_string() const;

protected:
  
private:

  IwrfDebug_t _debug;
  mutable int _debugPrintCount;

  // meta-data information

  iwrf_radar_info_t _radar_info;
  iwrf_scan_segment_t _scan_seg;
  iwrf_antenna_correction_t _ant_corr;
  iwrf_ts_processing_t _proc;
  iwrf_xmit_power_t _xmit_power;
  iwrf_rx_power_t _rx_power;
  iwrf_xmit_sample_t _xmit_sample;
  iwrf_status_xml_t _status_xml_hdr;
  string _status_xml_str;
  iwrf_calibration_t _calib;
  iwrf_event_notice_t _enotice;
  iwrf_phasecode_t _phasecode;
  iwrf_xmit_info_t _xmit_info;
  iwrf_rvp8_ops_info_t _rvp8;
  double _rvp8SaturationMult;
  iwrf_platform_georef_t _platform_georef0;
  iwrf_platform_georef_t _platform_georef1;

  bool _radar_info_active;
  bool _scan_seg_active;
  bool _ant_corr_active;
  bool _proc_active;
  bool _xmit_power_active;
  bool _rx_power_active;
  bool _xmit_sample_active;
  bool _status_xml_active;
  bool _calib_active;
  bool _enotice_active;
  bool _phasecode_active;
  bool _xmit_info_active;
  bool _rvp8_active;
  bool _platform_georef0_active;
  bool _platform_georef1_active;

  // metadataqueue - so writes occur in the correct order

  mutable deque<MemBuf *> _metaQueue;
  void _addMetaToQueue(size_t len, const void *packet);
  MemBuf *_popFrontFromMetaQueue();
  void _clearMetaQueue() const;

  // mutable deque<si32> _idQueue;
  // void _addIdToQueue(si32 id);

  // sweep and volume flags
  
  bool _startOfSweepFlag;
  bool _endOfSweepFlag;
  bool _startOfVolumeFlag;
  bool _endOfVolumeFlag;

  // private rvp8 functions
  
  int _readRvp8Info(FILE *in);
  void _deriveRangeFromRvp8Info(iwrf_ts_processing_t &tsProc);

};

#endif

