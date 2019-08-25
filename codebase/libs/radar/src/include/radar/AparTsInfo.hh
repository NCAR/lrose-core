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
// AparTsInfo.hh
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Aug 2019
//
///////////////////////////////////////////////////////////////
//
// Stores current radar ops info
//
////////////////////////////////////////////////////////////////

#ifndef _AparTsInfo_hh
#define _AparTsInfo_hh

#include <string>
#include <deque>
#include <dataport/port_types.h>
#include <toolsa/MemBuf.hh>
#include <radar/apar_ts_functions.hh>
#include <radar/AparTsCalib.hh>
#include <didss/DsMessage.hh>
using namespace std;

////////////////////////
// This class

class AparTsInfo {
  
public:

  // constructor
  
  AparTsInfo(AparTsDebug_t debug = APAR_TS_DEBUG_OFF);

  // copy constructor
  
  AparTsInfo(const AparTsInfo &rhs);

  // assignment
  
  AparTsInfo & operator=(const AparTsInfo &rhs);

  // destructor
  
  ~AparTsInfo();

  // clear all data

  void clear();

  // debugging
  
  void setDebug(AparTsDebug_t debug) { _debug = debug; }

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

  // write in tsarchive format
  
  int writeToTsarchiveFile(FILE *out) const;

  // write a sync packet to file in APAR format
  // returns 0 on success, -1 on failure

  int writeSyncToFile(FILE *out) const;
  
  // write meta-data to file in APAR format
  // Writes out any meta-data which has a sequence number
  // later than the previous pulse written.
  // To force a write, set the prevPulseSeqNum to 0.
  // Returns 0 on success, -1 on failure

  int writeMetaToFile(FILE *out, si64 prevPulseSeqNum) const;

  // write meta-data to file in APAR format
  // Writes out any meta-data for which the id is
  // queued in the _idQueue.
  // If clearQueue is true, the queue will be cleared
  // after the write
  // Returns 0 on success, -1 on failure

  int writeMetaQueueToFile(FILE *out, bool clearQueue) const;

  // write meta-data to DsMessage in APAR format
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

  void setRadarInfo(const apar_ts_radar_info_t &info,
                    bool addToMetaQueue = true);
  void setScanSegment(const apar_ts_scan_segment_t &seg,
                      bool addToMetaQueue = true);
  void setTsProcessing(const apar_ts_processing_t &proc,
                       bool addToMetaQueue = true);
  void setStatusXml(const apar_ts_status_xml_t &hdr,
                    const string &xmlStr,
                    bool addToMetaQueue = true);
  void setCalibration(const apar_ts_calibration_t &calib,
                      bool addToMetaQueue = true);
  void setEventNotice(const apar_ts_event_notice_t &enotice,
                      bool addToMetaQueue = true);
  void setPlatformGeoref(const apar_ts_platform_georef_t &georef,
                         bool addToMetaQueue = true);
  
  ////////////////////////////////////////////////////////////
  // set sequence number for each packet
  
  void setRadarInfoPktSeqNum(si64 pkt_seq_num);
  void setScanSegmentPktSeqNum(si64 pkt_seq_num);
  void setTsProcessingPktSeqNum(si64 pkt_seq_num);
  void setStatusXmlPktSeqNum(si64 pkt_seq_num);
  void setCalibrationPktSeqNum(si64 pkt_seq_num);
  void setEventNoticePktSeqNum(si64 pkt_seq_num);
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
  void setTsProcessingTime(time_t secs, int nano_secs);
  void setStatusXmlTime(time_t secs, int nano_secs);
  void setCalibrationTime(time_t secs, int nano_secs);
  void setEventNoticeTime(time_t secs, int nano_secs);
  void setPlatformGeorefTime(time_t secs, int nano_secs);
  void setPlatformGeoref1Time(time_t secs, int nano_secs);

  // set time for each packet to now

  void setRadarInfoTimeToNow();
  void setScanSegmentTimeToNow();
  void setTsProcessingTimeToNow();
  void setStatusXmlTimeToNow();
  void setCalibrationTimeToNow();
  void setEventNoticeTimeToNow();
  void setPlatformGeorefTimeToNow();
  void setPlatformGeoref1TimeToNow();

  ////////////////////////////////////////////////////////////
  // activate structs individually

  void setRadarInfoActive(bool state);
  void setScanSegmentActive(bool state);
  void setTsProcessingActive(bool state);
  void setStatusXmlActive(bool state);
  void setCalibrationActive(bool state);
  void setEventNoticeActive(bool state);
  void setPlatformGeorefActive(bool state);
  void setPlatformGeoref1Active(bool state);

  ////////////////////////////////////////////////////////////
  // set radar id on all structs

  void setRadarId(int id);

  ////////////////////////////////////////////////////////////
  // set AparTsCalib object from _calib, and vice versa
  
  void setAparTsCalib(AparTsCalib &aparCalib) const;
  void setFromAparTsCalib(const AparTsCalib &aparCalib);

  ////////////////////////////////////////////////////////////
  // get info at the struct level

  inline const apar_ts_radar_info_t &getRadarInfo() const { return _radar_info; }
  inline const apar_ts_scan_segment_t &getScanSegment() const { return _scan_seg; }
  inline const apar_ts_processing_t &getTsProcessing() const { return _proc; }
  inline const apar_ts_status_xml_t &getStatusXmlHdr() const { 
    return _status_xml_hdr; 
  }
  inline const string &getStatusXmlStr() const { return _status_xml_str; }
  inline const apar_ts_calibration_t &getCalibration() const { return _calib; }
  inline const apar_ts_event_notice_t &getEventNotice() const { return _enotice; }
  inline const apar_ts_platform_georef_t &getPlatformGeoref() const { 
    return _platform_georef0; 
  }
  inline const apar_ts_platform_georef_t &getPlatformGeoref1() const { 
    return _platform_georef1; 
  }

  ////////////////////////////////////////////////////////////
  // check which structs are active

  bool isRadarInfoActive() const { return _radar_info_active; }
  bool isScanSegmentActive() const { return _scan_seg_active; }
  bool isTsProcessingActive() const { return _proc_active; }
  bool isStatusXmlActive() const { return _status_xml_active; }
  bool isCalibrationActive() const { return _calib_active; }
  bool isEventNoticeActive() const { return _enotice_active; }
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
  double getTsProcessingTime() const;
  double getStatusXmlTime() const;
  double getCalibrationTime() const;
  double getEventNoticeTime() const;
  double getPlatformGeorefTime() const;
  double getPlatformGeoref1Time() const;

  // get packet sequence numbers

  si64 getRadarInfoPktSeqNum() const { return _radar_info.packet.seq_num; }
  si64 getScanSegmentPktSeqNum() const { return _scan_seg.packet.seq_num; }
  si64 getTsProcessingPktSeqNum() const { return _proc.packet.seq_num; }
  si64 getStatusXmlPktSeqNum() const { return _status_xml_hdr.packet.seq_num; }
  si64 getCalibrationPktSeqNum() const { return _calib.packet.seq_num; }
  si64 getEventNoticePktSeqNum() const { return _enotice.packet.seq_num; }
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
  
  inline void setRadarLatitudeDeg(fl32 x) { _radar_info.latitude_deg = x; }
  inline void setRadarLongitudeDeg(fl32 x) { _radar_info.longitude_deg = x; }
  inline void setRadarAltitudeM(fl32 x) { _radar_info.altitude_m = x; }
  inline void setRadarPlatformType(int x) { _radar_info.platform_type = x; }
  inline void setRadarBeamwidthDeg_h(fl32 x) { _radar_info.beamwidth_deg_h = x; }
  inline void setRadarBeamwidthDeg_v(fl32 x) { _radar_info.beamwidth_deg_v = x; }
  inline void setRadarWavelengthCm(fl32 x) { _radar_info.wavelength_cm = x; }
  inline void setRadarNominalGain_ant_db_h(fl32 x) { _radar_info.nominal_gain_ant_db_h = x; }
  inline void setRadarNominalGain_ant_db_v(fl32 x) { _radar_info.nominal_gain_ant_db_v = x; }
  void setRadarName(const string &name);
  void setSiteName(const string &name);

  // set scan_segment fields

  inline void setScanMode(si32 x) { _scan_seg.scan_mode = x; }
  inline void setScanVolumeNum(si32 x) { _scan_seg.volume_num = x; }
  inline void setScanSweepNum(si32 x) { _scan_seg.sweep_num = x; }
  inline void setScanAzStart(fl32 x) { _scan_seg.az_start = x; }
  inline void setScanElStart(fl32 x) { _scan_seg.el_start = x; }
  inline void setScanRate(fl32 x) { _scan_seg.scan_rate = x; }
  inline void setScanLeftLimit(fl32 x) { _scan_seg.left_limit = x; }
  inline void setScanRightLimit(fl32 x) { _scan_seg.right_limit = x; }
  inline void setScanUpLimit(fl32 x) { _scan_seg.up_limit = x; }
  inline void setScanDownLimit(fl32 x) { _scan_seg.down_limit = x; }
  inline void setScanStep(fl32 x) { _scan_seg.step = x; }
  inline void setScanCurrentFixedAngle(fl32 x) { _scan_seg.current_fixed_angle = x; }
  inline void setScanNSweeps(si32 x) { _scan_seg.n_sweeps = x; }
  void setScanFixedAngle(int i, fl32 x);
  void setScanSegmentName(const string &x);
  void setScanProjectName(const string &x);

  // set time-series processing fields

  inline void setProcPolMode(apar_ts_pol_mode_t x) {
    _proc.pol_mode = (si32) x;
  }
  inline void setProcPrfMode(apar_ts_prf_mode_t x) {
    _proc.prf_mode = (si32) x;
  }
  inline void setProcPulseShape(apar_ts_pulse_shape_t x) {
    _proc.pulse_shape = (si32) x; 
  }

  inline void setProcPulseWidthUs(fl32 x) { _proc.pulse_width_us = x; }

  inline void setProcStartRangeM(fl32 x) { _proc.start_range_m = x; }
  inline void setProcGateSpacingM(fl32 x) { _proc.gate_spacing_m = x; }

  inline void setProcTestPulseRangeKm(fl32 x) { _proc.test_pulse_range_km = x; }
  inline void setProcTestPulseLengthUs(fl32 x) { _proc.test_pulse_length_us = x; }

  inline void setProcNumPrts(si32 x) { _proc.num_prts = x; }
  inline void setProcPrtUs(int index, fl32 x) {
    if (index >= 0 && index < 4) {
      _proc.prt_us[index] = x;
    }
  }
  
  // set calibration fields

  inline void setCalibWavelengthCm(fl32 x) { _calib.wavelength_cm = x; }
  inline void setCalibBeamwidthDegH(fl32 x) { _calib.beamwidth_deg_h = x; }
  inline void setCalibBeamwidthDegV(fl32 x) { _calib.beamwidth_deg_v = x; }
  inline void setCalibGainAntDbH(fl32 x) { _calib.gain_ant_db_h = x; }
  inline void setCalibGainAntDbV(fl32 x) { _calib.gain_ant_db_v = x; }
  inline void setCalibPulseWidthUs(fl32 x) { _calib.pulse_width_us = x; }
  inline void setCalibXmitPowerDbmH(fl32 x) { _calib.xmit_power_dbm_h = x; }
  inline void setCalibXmitPowerDbmV(fl32 x) { _calib.xmit_power_dbm_v = x; }
  inline void setCalibTwoWayWaveguideLossDbH(fl32 x) {
    _calib.two_way_waveguide_loss_db_h = x;
  }
  inline void setCalibTwoWayWaveguideLossDbV(fl32 x) {
    _calib.two_way_waveguide_loss_db_v = x;
  }
  inline void setCalibTwoWayRadomeLossDbH(fl32 x) { _calib.two_way_radome_loss_db_h = x; }
  inline void setCalibTwoWayRadomeLossDbV(fl32 x) { _calib.two_way_radome_loss_db_v = x; }
  inline void setCalibReceiverMismatchLossDb(fl32 x) {
    _calib.receiver_mismatch_loss_db = x;
  }
  inline void setCalibRadarConstantH(fl32 x) { _calib.radar_constant_h = x; }
  inline void setCalibRadarConstantV(fl32 x) { _calib.radar_constant_v = x; }
  inline void setCalibNoiseDbmHc(fl32 x) { _calib.noise_dbm_hc = x; }
  inline void setCalibNoiseDbmHx(fl32 x) { _calib.noise_dbm_hx = x; }
  inline void setCalibNoiseDbmVc(fl32 x) { _calib.noise_dbm_vc = x; }
  inline void setCalibNoiseDbmVx(fl32 x) { _calib.noise_dbm_vx = x; }
  inline void setCalibReceiverGainDbHc(fl32 x) { _calib.receiver_gain_db_hc = x; }
  inline void setCalibReceiverGainDbHx(fl32 x) { _calib.receiver_gain_db_hx = x; }
  inline void setCalibReceiverGainDbVc(fl32 x) { _calib.receiver_gain_db_vc = x; }
  inline void setCalibReceiverGainDbVx(fl32 x) { _calib.receiver_gain_db_vx = x; }
  inline void setCalibReceiverSlope_hc(fl32 x) { _calib.receiver_slope_hc = x; }
  inline void setCalibReceiverSlope_hx(fl32 x) { _calib.receiver_slope_hx = x; }
  inline void setCalibReceiverSlope_vc(fl32 x) { _calib.receiver_slope_vc = x; }
  inline void setCalibReceiverSlope_vx(fl32 x) { _calib.receiver_slope_vx = x; }
  inline void setCalibBaseDbz1km_hc(fl32 x) { _calib.base_dbz_1km_hc = x; }
  inline void setCalibBaseDbz1km_hx(fl32 x) { _calib.base_dbz_1km_hx = x; }
  inline void setCalibBaseDbz1km_vc(fl32 x) { _calib.base_dbz_1km_vc = x; }
  inline void setCalibBaseDbz1km_vx(fl32 x) { _calib.base_dbz_1km_vx = x; }
  inline void setCalibSunPowerDbmHc(fl32 x) { _calib.sun_power_dbm_hc = x; }
  inline void setCalibSunPowerDbmHx(fl32 x) { _calib.sun_power_dbm_hx = x; }
  inline void setCalibSunPowerDbmVc(fl32 x) { _calib.sun_power_dbm_vc = x; }
  inline void setCalibSunPowerDbmVx(fl32 x) { _calib.sun_power_dbm_vx = x; }
  inline void setCalibNoiseSourcePowerDbmH(fl32 x) { _calib.noise_source_power_dbm_h = x; }
  inline void setCalibNoiseSourcePowerDbmV(fl32 x) { _calib.noise_source_power_dbm_v = x; }
  inline void setCalibPowerMeasLossDbH(fl32 x) { _calib.power_meas_loss_db_h = x; }
  inline void setCalibPowerMeasLossDbV(fl32 x) { _calib.power_meas_loss_db_v = x; }
  inline void setCalibCouplerForwardLossDbH(fl32 x) {
    _calib.coupler_forward_loss_db_h = x;
  }
  inline void setCalibCouplerForwardLossDbV(fl32 x) {
    _calib.coupler_forward_loss_db_v = x;
  }
  inline void setCalibTestPowerDbmH(fl32 x) { _calib.test_power_dbm_h = x; }
  inline void setCalibTestPowerDbmV(fl32 x) { _calib.test_power_dbm_v = x; }
  inline void setCalibZdrCorrection_db(fl32 x) { _calib.zdr_correction_db = x; }
  inline void setCalibLdrCorrectionDbH(fl32 x) { _calib.ldr_correction_db_h = x; }
  inline void setCalibLdrCorrectionDbV(fl32 x) { _calib.ldr_correction_db_v = x; }
  inline void setCalibPhidpRotDeg(fl32 x) { _calib.phidp_rot_deg = x; }
  
  // set event notice fields
  
  inline void setEnoticeStartOfSweep(si32 x) { _enotice.start_of_sweep = x; }
  inline void setEnoticeEndOfSweep(si32 x) { _enotice.end_of_sweep = x; }
  inline void setEnoticeStartOfVolume(si32 x) { _enotice.start_of_volume = x; }
  inline void setEnoticeEndOfVolume(si32 x) { _enotice.end_of_volume = x; }
  inline void setEnoticeScanMode(si32 x) { _enotice.scan_mode = x; }
  inline void setEnoticeVolumeNum(si32 x) { _enotice.volume_num = x; }
  inline void setEnoticeSweepNum(si32 x) { _enotice.sweep_num = x; }
  
  ////////////////////////////////////////////////////////////
  // get individual fields

  // get radar_info fields

  inline fl32 getRadarLatitudeDeg() const { return _radar_info.latitude_deg; }
  inline fl32 getRadarLongitudeDeg() const { return _radar_info.longitude_deg; }
  inline fl32 getRadarAltitudeM() const { return _radar_info.altitude_m; }
  inline si32 getRadarPlatformType() const { return _radar_info.platform_type; }
  inline fl32 getRadarBeamwidthDegH() const { return _radar_info.beamwidth_deg_h; }
  inline fl32 getRadarBeamwidthDegV() const { return _radar_info.beamwidth_deg_v; }
  inline fl32 getRadarWavelengthCm() const { return _radar_info.wavelength_cm; }
  inline fl32 getRadarNominalGainAntDbH() const { return _radar_info.nominal_gain_ant_db_h; }
  inline fl32 getRadarNominalGainAntDbV() const { return _radar_info.nominal_gain_ant_db_v; }
  string getRadarName() const;
  string getSiteName() const;

  // get scan_segment fields

  inline si32 getScanMode() const { return _scan_seg.scan_mode; }
  inline si32 getScanVolumeNum() const { return _scan_seg.volume_num; }
  inline si32 getScanSweepNum() const { return _scan_seg.sweep_num; }
  inline fl32 getScanAzStart() const { return _scan_seg.az_start; }
  inline fl32 getScanElStart() const { return _scan_seg.el_start; }
  inline fl32 getScanRate() const { return _scan_seg.scan_rate; }
  inline fl32 getScanLeftLimit() const { return _scan_seg.left_limit; }
  inline fl32 getScanRightLimit() const { return _scan_seg.right_limit; }
  inline fl32 getScanUpLimit() const { return _scan_seg.up_limit; }
  inline fl32 getScanDownLimit() const { return _scan_seg.down_limit; }
  inline fl32 getScanStep() const { return _scan_seg.step; }
  inline fl32 getScanCurrentFixedAngle() const { return _scan_seg.current_fixed_angle; }
  inline si32 getScanNSweeps() const { return _scan_seg.n_sweeps; }
  fl32 getScanFixedAngle(int i) const;
  string getScanSegmentName() const;
  string getScanProjectName() const;

  // get time-series processing fields

  inline apar_ts_pol_mode_t getProcPolMode() const {
    return (apar_ts_pol_mode_t) _proc.pol_mode;
  }
  inline apar_ts_prf_mode_t getProcPrfMode() const {
    return (apar_ts_prf_mode_t) _proc.prf_mode;
  }
  inline apar_ts_pulse_shape_t getProcPulseShape() const {
    return (apar_ts_pulse_shape_t) _proc.pulse_shape; 
  }

  inline fl32 getProcPulseWidthUs() const { return _proc.pulse_width_us; }

  inline fl32 getProcStartRangeM() const { return _proc.start_range_m; }
  inline fl32 getProcStartRangeKm() const {
    return _proc.start_range_m / 1000.0;
  }
  inline fl32 getProcGateSpacingM() const { return _proc.gate_spacing_m; }
  inline fl32 getProcGateSpacingKm() const {
    return _proc.gate_spacing_m / 1000.0;
  }

  inline fl32 getProcTestPulseRangeKm() const { return _proc.test_pulse_range_km; }
  inline fl32 getProcTestPulseLengthUs() const { return _proc.test_pulse_length_us; }
  
  inline si32 getProcNumPrts() const { return _proc.num_prts; }
  inline fl32 getProcPrtUs(int index) const {
    if (index >= 0 && index < 4) {
      return _proc.prt_us[index]; 
    } else {
      return 9999.0;
    }
  }

  // get calibration fields

  inline fl32 getCalibWavelengthCm() const { return _calib.wavelength_cm; }
  inline fl32 getCalibBeamwidthDegH() const { return _calib.beamwidth_deg_h; }
  inline fl32 getCalibBeamwidthDegV() const { return _calib.beamwidth_deg_v; }
  inline fl32 getCalibGainAntDbH() const { return _calib.gain_ant_db_h; }
  inline fl32 getCalibGainAntDbV() const { return _calib.gain_ant_db_v; }
  inline fl32 getCalibPulseWidthUs() const { return _calib.pulse_width_us; }
  inline fl32 getCalibXmitPowerDbmH() const { return _calib.xmit_power_dbm_h; }
  inline fl32 getCalibXmitPowerDbmV() const { return _calib.xmit_power_dbm_v; }
  inline fl32 getCalibTwoWayWaveguideLossDbH() const {
    return _calib.two_way_waveguide_loss_db_h;
  }
  inline fl32 getCalibTwoWayWaveguideLossDbV() const {
    return _calib.two_way_waveguide_loss_db_v;
  }
  inline fl32 getCalibTwoWayRadomeLossDbH() const { return _calib.two_way_radome_loss_db_h; }
  inline fl32 getCalibTwoWayRadomeLossDbV() const { return _calib.two_way_radome_loss_db_v; }
  inline fl32 getCalibReceiverMismatchLossDb() const {
    return _calib.receiver_mismatch_loss_db;
  }
  inline fl32 getCalibRadarConstantH() const { return _calib.radar_constant_h; }
  inline fl32 getCalibRadarConstantV() const { return _calib.radar_constant_v; }
  inline fl32 getCalibNoiseDbmHc() const { return _calib.noise_dbm_hc; }
  inline fl32 getCalibNoiseDbmHx() const { return _calib.noise_dbm_hx; }
  inline fl32 getCalibNoiseDbmVc() const { return _calib.noise_dbm_vc; }
  inline fl32 getCalibNoiseDbmVx() const { return _calib.noise_dbm_vx; }
  inline fl32 getCalibReceiverGainDbHc() const { return _calib.receiver_gain_db_hc; }
  inline fl32 getCalibReceiverGainDbHx() const { return _calib.receiver_gain_db_hx; }
  inline fl32 getCalibReceiverGainDbVc() const { return _calib.receiver_gain_db_vc; }
  inline fl32 getCalibReceiverGainDbVx() const { return _calib.receiver_gain_db_vx; }
  inline fl32 getCalibReceiverSlopeHc() const { return _calib.receiver_slope_hc; }
  inline fl32 getCalibReceiverSlopeHx() const { return _calib.receiver_slope_hx; }
  inline fl32 getCalibReceiverSlopeVc() const { return _calib.receiver_slope_vc; }
  inline fl32 getCalibReceiverSlopeVx() const { return _calib.receiver_slope_vx; }
  inline fl32 getCalibBaseDbz1KmHc() const { return _calib.base_dbz_1km_hc; }
  inline fl32 getCalibBaseDbz1KmHx() const { return _calib.base_dbz_1km_hx; }
  inline fl32 getCalibBaseDbz1KmVc() const { return _calib.base_dbz_1km_vc; }
  inline fl32 getCalibBaseDbz1KmVx() const { return _calib.base_dbz_1km_vx; }
  inline fl32 getCalibSunPowerDbmHc() const { return _calib.sun_power_dbm_hc; }
  inline fl32 getCalibSunPowerDbmHx() const { return _calib.sun_power_dbm_hx; }
  inline fl32 getCalibSunPowerDbmVc() const { return _calib.sun_power_dbm_vc; }
  inline fl32 getCalibSunPowerDbmVx() const { return _calib.sun_power_dbm_vx; }
  inline fl32 getCalibNoiseSourcePowerDbmH() const { return _calib.noise_source_power_dbm_h; }
  inline fl32 getCalibNoiseSourcePowerDbmV() const { return _calib.noise_source_power_dbm_v; }
  inline fl32 getCalibPowerMeasLossDbH() const { return _calib.power_meas_loss_db_h; }
  inline fl32 getCalibPowerMeasLossDbV() const { return _calib.power_meas_loss_db_v; }
  inline fl32 getCalibCouplerForwardLossDbH() const {
    return _calib.coupler_forward_loss_db_h;
  }
  inline fl32 getCalibCouplerForwardLossDbV() const {
    return _calib.coupler_forward_loss_db_v;
  }
  inline fl32 getCalibTestPowerDbmH() const { return _calib.test_power_dbm_h; }
  inline fl32 getCalibTestPowerDbmV() const { return _calib.test_power_dbm_v; }
  inline fl32 getCalibZdrCorrection_db() const { return _calib.zdr_correction_db; }
  inline fl32 getCalibLdrCorrectionDbH() const { return _calib.ldr_correction_db_h; }
  inline fl32 getCalibLdrCorrectionDbV() const { return _calib.ldr_correction_db_v; }
  inline fl32 getCalibPhidpRotDeg() const { return _calib.phidp_rot_deg; }
  
  // get event notice fields

  inline si32 getEnoticeStartOfSweep() const { return _enotice.start_of_sweep; }
  inline si32 getEnoticeEndOfSweep() const { return _enotice.end_of_sweep; }
  inline si32 getEnoticeStartOfVolume() const { return _enotice.start_of_volume; }
  inline si32 getEnoticeEndOfVolume() const { return _enotice.end_of_volume; }
  inline si32 getEnoticeScanMode() const { return _enotice.scan_mode; }
  inline si32 getEnoticeVolumeNum() const { return _enotice.volume_num; }
  inline si32 getEnoticeSweepNum() const { return _enotice.sweep_num; }

protected:
  
  // copy
  
  virtual AparTsInfo &_copy(const AparTsInfo &rhs);
  
private:

  AparTsDebug_t _debug;
  mutable int _debugPrintCount;

  // meta-data information

  apar_ts_radar_info_t _radar_info;
  apar_ts_scan_segment_t _scan_seg;
  apar_ts_processing_t _proc;
  apar_ts_status_xml_t _status_xml_hdr;
  string _status_xml_str;
  apar_ts_calibration_t _calib;
  apar_ts_event_notice_t _enotice;
  apar_ts_platform_georef_t _platform_georef0;
  apar_ts_platform_georef_t _platform_georef1;

  bool _radar_info_active;
  bool _scan_seg_active;
  bool _proc_active;
  bool _status_xml_active;
  bool _calib_active;
  bool _enotice_active;
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

};

#endif

