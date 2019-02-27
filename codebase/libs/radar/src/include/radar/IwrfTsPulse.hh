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
// IwrfTsPulse.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2006
//
///////////////////////////////////////////////////////////////

#ifndef IwrfTsPulse_hh
#define IwrfTsPulse_hh

#include <string>
#include <vector>
#include <deque>
#include <pthread.h>
#include <toolsa/MemBuf.hh>
#include <dataport/port_types.h>
#include <radar/IwrfTsInfo.hh>
#include <radar/RadarComplex.hh>
class OpsInfo;

using namespace std;

////////////////////////
// This class

class IwrfTsPulse {
  
public:

  // constructor

  IwrfTsPulse(IwrfTsInfo &info,
	      IwrfDebug_t debug = IWRF_DEBUG_OFF);

  // copy constructor
  
  IwrfTsPulse(const IwrfTsPulse &rhs);

  // destructor

  virtual ~IwrfTsPulse();

  // assignment
  
  IwrfTsPulse & operator=(const IwrfTsPulse &rhs);

  // clear data

  void clear();

  // set the ops info

  void setOpsInfo(IwrfTsInfo &info) {
    if (&_info != &info) _info = info;
  }
  
  // debugging

  void setDebug(IwrfDebug_t debug) { _debug = debug; }
  
  // invert HV flag?
  // The HV flag is defined as 1 for H, 0 for V.
  // If the sense of the HV flag in the header is incorrect,
  // you can invert the flag when the pulse header is loaded.
  
  void setInvertHvFlag(bool invert = true) const { _invertHvFlag = invert; }
  
  // set time on all packets
  
  void setTime(time_t secs, int nano_secs);
  void setTimeToNow();

  // set the platform georeference for this pulse

  void setPlatformGeoref(const iwrf_platform_georef_t &georef) {
    _georef = georef;
    _georefActive = true;
  }

  void setGeorefCorr(const iwrf_georef_correction_t &georefCorr) {
    _georefCorr = georefCorr;
    _georefCorrActive = true;
  }

  // compute az and el angles by applying the georef data

  void computeElAzFromGeoref();

  // set from pulse buffer, swapping as required
  // optionally convert iq data to floats
  // Returns 0 on success, -1 on failure
  
  int setFromBuffer(const void *buf, int len, bool convertToFloat);

  // set IQ data as floats
  
  void setIqFloats(int nGates, int nChannels, const fl32 *iq);

  // set IQ data as floats, converting from ScaledSi32
  
  void setIqFromScaledSi32(int nGates,
                           int nChannels,
                           const si32 *siq);
    
  // set IQ data packed
  
  void setIqPacked(int nGates, int nChannels, 
		   iwrf_iq_encoding_t encoding,
		   const si16 *packed,
		   double scale /* = 1.0 */,
		   double offset /* = 0.0 */);

  // convert packed data to float32
  
  void convertToFL32();
    
  // convert to specified packing
  
  void convertToPacked(iwrf_iq_encoding_t encoding);

  // swaps I and Q, because they are stored in the incorrect order
  // in the data arrays
  
  void swapIQ();

  // invert Q values in the data arrays
  
  void invertQ();

  // cohere IQ values to the burst phase
  // by subtracting the burst phase from the IQ phase

  void cohereIqToBurstPhase();

  // copy the pulse width from the ts_proc in the info
  
  void copyPulseWidthFromTsProc();

  // assemble header and data into a single buffer

  void assemble(MemBuf &buf) const;

  // set RVP8-specific fields from pulse header
  
  void setRvp8Hdr(const iwrf_pulse_header_t &hdr);
  
  // set RVP8 legacy unpacking
  // uses 11-bit mantissa instead of the later 12-bit mantissa
  
  void setSigmetLegacyUnpacking(bool state);

  // read pulse data from RVP8 file
  // Returns 0 on success, -1 on failure

  int readFromRvp8File(FILE *in);

  // set headers directly

  void setHeader(const iwrf_pulse_header_t &hdr);
  void setRvp8Header(const iwrf_rvp8_pulse_header_t &hdr);

  // set radar ID
  
  void setRadarId(int id);

  // activate RVP8 header
  
  void activateRvp8Header();

  // set packet sequence numbers for each header

  void setPktSeqNum(si64 pkt_seq_num);
  void setRvp8PktSeqNum(si64 pkt_seq_num);

  // Swap the PRT values in the pulse header.
  // STAGGERED PRT mode only.
  // There are two prt values in the pulse header, prt and prt next.
  // prt is the time SINCE the PREVIOUS pulse
  // prt_next is the time TO to the NEXT next pulse
  // If the incoming data uses a different convention, this call
  // can be used to swap the values
  
  void swapPrtValues();
  
  // compute phase differences between this pulse and previous ones
  // to be able to cohere to multiple trips
  //
  // Before this method is called, this pulse should be added to
  // the queue.
  
  int computePhaseDiffs(const deque<IwrfTsPulse *> &pulseQueue,
			int maxTrips) const;

  int computePhaseDiffs(int qSize,
                        const IwrfTsPulse **pulseQueue,
			int maxTrips) const;

  // typedef for burst phase for phase (de)coding

  typedef struct {
    RadarComplex_t trip1;
    RadarComplex_t trip2;
    RadarComplex_t trip3;
    RadarComplex_t trip4;
  } burst_phase_t;
  
  // set burst phase for various trips
  // for trip1, use tripNum = 0
  // for trip2, use tripNum = 1, etc
  
  inline void setBurstPhases(const burst_phase_t &phases) {
    _burstPhases = phases;
  }
  inline void setBurstPhaseDegTrip1(fl32 phase) {
    RadarComplex::setFromDegrees(phase, _burstPhases.trip1);
  }
  inline void setBurstPhaseDegTrip2(fl32 phase) {
    RadarComplex::setFromDegrees(phase, _burstPhases.trip2);
  }
  inline void setBurstPhaseDegTrip3(fl32 phase) {
    RadarComplex::setFromDegrees(phase, _burstPhases.trip3);
  }
  inline void setBurstPhaseDegTrip4(fl32 phase) {
    RadarComplex::setFromDegrees(phase, _burstPhases.trip4);
  }

  // get burst phase for various trips
  // for trip1, use tripNum = 0
  // for trip2, use tripNum = 1, etc
  
  const burst_phase_t &getBurstPhases() const {
    return _burstPhases;
  }

  inline const RadarComplex_t& getBurstPhaseTrip1() const {
    return _burstPhases.trip1;
  }
  inline const RadarComplex_t& getBurstPhaseTrip2() const {
    return _burstPhases.trip2;
  }
  inline const RadarComplex_t& getBurstPhaseTrip3() const {
    return _burstPhases.trip3;
  }
  inline const RadarComplex_t& getBurstPhaseTrip4() const {
    return _burstPhases.trip4;
  }

  ///////////////////////////////////////////////////////////
  // write to tsarchive file
  // Returns 0 on success, -1 on failure
  
  int writeToTsarchiveFile(FILE *out) const;
    
  // write to file in IWRF format
  // returns 0 on success, -1 on failure

  int writeToFile(FILE *out) const;

  // printing
  
  void printHeader(FILE *out) const;
  void printData(FILE *out) const;
  void printData(FILE *out, int startGate, int endGate) const;

  // get methods - commonly used

  inline IwrfTsInfo &getTsInfo() { return _info; }
  inline const IwrfTsInfo &getTsInfo() const { return _info; }

  inline const iwrf_pulse_header_t &getHdr() const { return _hdr; }
  inline const iwrf_rvp8_pulse_header_t &getRvp8Hdr() const { return _rvp8_hdr; }

  inline int getNGates() const { return _hdr.n_gates; }
  inline int getNChannels() const { return _hdr.n_channels; }
  inline int getRadarId() const { return _hdr.packet.radar_id; }
  inline si64 getSeqNum() const { return _hdr.pulse_seq_num; }
  inline double getFTime() const { return _ftime; }
  inline time_t getTime() const { return _hdr.packet.time_secs_utc; }
  inline int getNanoSecs() const { return _hdr.packet.time_nano_secs; }
  inline double getPrt() const { return _hdr.prt; }
  inline double getPrf() const { return _prf; }
  inline double getPulseWidthUs() const {
    return _hdr.pulse_width_us;  // microsecs
  }
  inline double getMeasXmitPowerDbmH() const { return _info.getXmitPower().power_dbm_h; }
  inline double getMeasXmitPowerDbmV() const { return _info.getXmitPower().power_dbm_v; }
  
  double getEl() const;
  double getAz() const;
  double getFixedAngle() const; // Depends on scan mode
  double getFixedEl() const; // PPI mode
  double getFixedAz() const; // RHI mode

  inline int getVolNum() const { return _hdr.volume_num; }
  inline int getSweepNum() const { return _hdr.sweep_num; }
  inline double getPhaseDiff0() const { return _phaseDiff[0]; }
  inline double getPhaseDiff1() const { return _phaseDiff[1]; }
  bool isHoriz() const; // is horizontally polarized
  inline bool antennaTransition() const { return (bool) _hdr.antenna_transition; }

  iwrf_scan_mode_t getScanMode() const {
    return ( iwrf_scan_mode_t) _hdr.scan_mode;
  }

  // get the platform georeference for this pulse

  const iwrf_platform_georef_t &getPlatformGeoref() const {
    return _georef;
  }
  bool getGeorefActive() const { return _georefActive; }

  const iwrf_georef_correction_t &getGeorefCorrection() const {
    return _georefCorr;
  }
  bool getGeorefCorrActive() const { return _georefCorrActive; }

  // get IQ data arrays
  
  inline const fl32 *getIq0() const { return _chanIq[0]; }
  inline const fl32 *getIq1() const { return _chanIq[1]; }
  inline const fl32 *getIq2() const { return _chanIq[2]; }
  inline const fl32 *getIq3() const { return _chanIq[3]; }
  inline const fl32 *getBurstIq0() const { return _burstIq[0]; }
  inline const fl32 *getBurstIq1() const { return _burstIq[1]; }
  inline const fl32 *getBurstIq2() const { return _burstIq[2]; }
  inline const fl32 *getBurstIq3() const { return _burstIq[3]; }

  inline fl32 **getIqArray() { return _chanIq; }
  inline fl32 **getBurstIqArray() { return _burstIq; }

  // get IQ data at a gate
  // returns IWRF_MISSING_FLOAT if not available at that gate for that channel
  
  void getIq0(int gateNum, fl32 &ival, fl32 &qval) const;
  void getIq1(int gateNum, fl32 &ival, fl32 &qval) const;
  void getIq2(int gateNum, fl32 &ival, fl32 &qval) const;
  void getIq3(int gateNum, fl32 &ival, fl32 &qval) const;
  void getIq(int chanNum, int gateNum, fl32 &ival, fl32 &qval) const;

  // get packed data

  iwrf_iq_encoding_t getPackedEncoding() const { return _packedEncoding; }
  double getPackedScale() const { return _packedScale; }
  double getPackedOffset() const { return _packedOffset; }
  const void *getPackedData() const;

  // get packet sequence numbers

  si64 getPulseSeqNum() const { return _hdr.pulse_seq_num; }
  si64 getPktSeqNum() const { return _hdr.packet.seq_num; }
  si64 getRvp8PktSeqNum() const { return _rvp8_hdr.packet.seq_num; }

  // is RVP8 header active

  bool isRvp8HeaderActive() const { return _rvp8_header_active; }

  // set individual fields

  inline void set_pulse_seq_num(si64 x) { _hdr.pulse_seq_num = x; }
  inline void set_scan_mode(si32 x) { _hdr.scan_mode = x; }
  inline void set_follow_mode(si32 x) { _hdr.follow_mode = x; }
  inline void set_sweep_num(si32 x) { _hdr.sweep_num = x; }
  inline void set_volume_num(si32 x) { _hdr.volume_num = x; }
  inline void set_fixed_el(fl32 x) { _hdr.fixed_el = x; }
  inline void set_fixed_az(fl32 x) { _hdr.fixed_az = x; }
  inline void set_elevation(fl32 x) { _hdr.elevation = x; }
  inline void set_azimuth(fl32 x) { _hdr.azimuth = x; }
  inline void set_prt(fl32 x) { _hdr.prt = x; }
  inline void set_prt_next(fl32 x) { _hdr.prt_next = x; }
  inline void set_pulse_width_us(fl32 x) { _hdr.pulse_width_us = x; }
  inline void set_n_gates(si32 x) { _hdr.n_gates = x; }
  inline void set_n_channels(si32 x) { _hdr.n_channels = x; }
  inline void set_iq_encoding(si32 x) { _hdr.iq_encoding = x; }
  inline void set_hv_flag(si32 x) { _hdr.hv_flag = x; }
  inline void set_antenna_transition(si32 x) { _hdr.antenna_transition = x; }
  inline void set_phase_cohered(si32 x) { _hdr.phase_cohered = x; }
  inline void set_status(si32 x) { _hdr.status = x; }
  inline void set_n_data(si32 x) { _hdr.n_data = x; }
  inline void set_iq_offset(int chan, si32 x) { _hdr.iq_offset[chan] = x; }
  inline void set_burst_mag(int chan, fl32 x) { _hdr.burst_mag[chan] = x; }
  inline void set_burst_arg(int chan, fl32 x) { _hdr.burst_arg[chan] = x; }
  inline void set_burst_arg_diff(int chan, fl32 x) { _hdr.burst_arg_diff[chan] = x; }
  inline void set_scale(fl32 x) { _hdr.scale = x; }
  inline void set_offset(fl32 x) { _hdr.offset = x; }
  inline void set_n_gates_burst(si32 x) { _hdr.n_gates_burst = x; }
  inline void set_start_range_m(fl32 x) { _hdr.start_range_m = x; }
  inline void set_gate_spacing_m(fl32 x) { _hdr.gate_spacing_m = x; }

  inline void set_start_of_sweep() {
    _hdr.event_flags |= IWRF_START_OF_SWEEP;
  }
  inline void set_start_of_volume() {
    _hdr.event_flags |= IWRF_START_OF_VOLUME;
  }
  inline void set_end_of_sweep() {
    _hdr.event_flags |= IWRF_END_OF_SWEEP;
  }
  inline void set_end_of_volume() {
    _hdr.event_flags |= IWRF_END_OF_VOLUME;
  }

  inline void set_rvp8_i_version(si32 x) { _rvp8_hdr.i_version = x; }
  inline void set_rvp8_i_flags(ui08 x) { _rvp8_hdr.i_flags = x; }
  inline void set_rvp8_i_aq_mode(ui08 x) { _rvp8_hdr.i_aq_mode = x; }
  inline void set_rvp8_i_polar_bits(ui08 x) { _rvp8_hdr.i_polar_bits = x; }
  inline void set_rvp8_i_viq_per_bin(ui08 x) { _rvp8_hdr.i_viq_per_bin = x; }
  inline void set_rvp8_i_tg_bank(ui08 x) { _rvp8_hdr.i_tg_bank = x; }
  inline void set_rvp8_i_tx_phase(ui16 x) { _rvp8_hdr.i_tx_phase = x; }
  inline void set_rvp8_i_az(ui16 x) { _rvp8_hdr.i_az = x; }
  inline void set_rvp8_i_el(ui16 x) { _rvp8_hdr.i_el = x; }
  inline void set_rvp8_i_num_vecs(si16 x) { _rvp8_hdr.i_num_vecs = x; }
  inline void set_rvp8_i_max_vecs(si16 x) { _rvp8_hdr.i_max_vecs = x; }
  inline void set_rvp8_i_tg_wave(ui16 x) { _rvp8_hdr.i_tg_wave = x; }
  inline void set_rvp8_i_btime_api(ui32 x) { _rvp8_hdr.i_btime_api = x; }
  inline void set_rvp8_i_sys_time(ui32 x) { _rvp8_hdr.i_sys_time = x; }
  inline void set_rvp8_i_prev_prt(ui32 x) { _rvp8_hdr.i_prev_prt = x; }
  inline void set_rvp8_i_next_prt(ui32 x) { _rvp8_hdr.i_next_prt = x; }
  inline void set_rvp8_uiq_perm(int i, ui32 x) { _rvp8_hdr.uiq_perm[i] = x; }
  inline void set_rvp8_uiq_once(int i, ui32 x) { _rvp8_hdr.uiq_once[i] = x; }
  inline void set_rvp8_i_data_off(int chan, si32 x) { _rvp8_hdr.i_data_off[chan] = x; }
  inline void set_rvp8_f_burst_mag(int chan, fl32 x) { _rvp8_hdr.f_burst_mag[chan] = x; }
  inline void set_rvp8_i_burst_arg(int chan, ui16 x) { _rvp8_hdr.i_burst_arg[chan] = x; }
  inline void set_rvp8_i_wrap_iq(int chan, ui16 x) { _rvp8_hdr.i_wrap_iq[chan] = x; }

  // get all individual fields from header

  inline si64 get_pulse_seq_num() const { return _hdr.pulse_seq_num; }
  inline si32 get_scan_mode() const { return _hdr.scan_mode; }
  inline si32 get_follow_mode() const { return _hdr.follow_mode; }
  inline si32 get_sweep_num() const { return _hdr.sweep_num; }
  inline si32 get_volume_num() const { return _hdr.volume_num; }
  inline fl32 get_fixed_el() const { return _hdr.fixed_el; }
  inline fl32 get_fixed_az() const { return _hdr.fixed_az; }
  inline fl32 get_elevation() const { return _hdr.elevation; }
  inline fl32 get_azimuth() const { return _hdr.azimuth; }
  inline fl32 get_prt() const { return _hdr.prt; }
  inline fl32 get_prt_next() const { return _hdr.prt_next; }
  inline fl32 get_pulse_width_us() const { return _hdr.pulse_width_us; }
  inline si32 get_n_gates() const { return _hdr.n_gates; }
  inline si32 get_n_channels() const { return _hdr.n_channels; }
  inline si32 get_iq_encoding() const { return _hdr.iq_encoding; }
  inline si32 get_hv_flag() const { return _hdr.hv_flag; }
  inline si32 get_antenna_transition() const { return _hdr.antenna_transition; }
  inline si32 get_phase_cohered() const { return _hdr.phase_cohered; }
  inline si32 get_status() const { return _hdr.status; }
  inline si32 get_n_data() const { return _hdr.n_data; }
  inline si32 get_iq_offset(int chan) const { return _hdr.iq_offset[chan]; }
  inline fl32 get_burst_mag(int chan) const { return _hdr.burst_mag[chan]; }
  inline fl32 get_burst_arg(int chan) const { return _hdr.burst_arg[chan]; }
  inline fl32 get_burst_arg_diff(int chan) const {
    return _hdr.burst_arg_diff[chan];
  }
  inline fl32 get_scale() const { return _hdr.scale; }
  inline fl32 get_offset() const { return _hdr.offset; }
  inline si32 get_n_gates_burst() const { return _hdr.n_gates_burst; }
  inline fl32 get_start_range_m() const { return _hdr.start_range_m; }
  inline fl32 get_start_range_km() const {
    return _hdr.start_range_m / 1000.0;
  }
  inline fl32 get_gate_spacing_m() const { return _hdr.gate_spacing_m; }
  inline fl32 get_gate_spacing_km() const {
    return _hdr.gate_spacing_m / 1000.0;
  }
  
  inline bool get_start_of_sweep() {
    return (_hdr.event_flags & IWRF_START_OF_SWEEP);
  }
  inline bool get_start_of_volume() {
    return (_hdr.event_flags & IWRF_START_OF_VOLUME);
  }
  inline bool get_end_of_sweep() {
    return (_hdr.event_flags & IWRF_END_OF_SWEEP);
  }
  inline bool get_end_of_volume() {
    return (_hdr.event_flags & IWRF_END_OF_VOLUME);
  }

  inline si32 get_rvp8_i_version() const { return _rvp8_hdr.i_version; }
  inline ui08 get_rvp8_i_flags() const { return _rvp8_hdr.i_flags; }
  inline ui08 get_rvp8_i_aq_mode() const { return _rvp8_hdr.i_aq_mode; }
  inline ui08 get_rvp8_i_polar_bits() const { return _rvp8_hdr.i_polar_bits; }
  inline ui08 get_rvp8_i_viq_per_bin() const { return _rvp8_hdr.i_viq_per_bin; }
  inline ui08 get_rvp8_i_tg_bank() const { return _rvp8_hdr.i_tg_bank; }
  inline ui16 get_rvp8_i_tx_phase() const { return _rvp8_hdr.i_tx_phase; }
  inline ui16 get_rvp8_i_az() const { return _rvp8_hdr.i_az; }
  inline ui16 get_rvp8_i_el() const { return _rvp8_hdr.i_el; }
  inline si16 get_rvp8_i_num_vecs() const { return _rvp8_hdr.i_num_vecs; }
  inline si16 get_rvp8_i_max_vecs() const { return _rvp8_hdr.i_max_vecs; }
  inline ui16 get_rvp8_i_tg_wave() const { return _rvp8_hdr.i_tg_wave; }
  inline ui32 get_rvp8_i_btime_api() const { return _rvp8_hdr.i_btime_api; }
  inline ui32 get_rvp8_i_sys_time() const { return _rvp8_hdr.i_sys_time; }
  inline ui32 get_rvp8_i_prev_prt() const { return _rvp8_hdr.i_prev_prt; }
  inline ui32 get_rvp8_i_next_prt() const { return _rvp8_hdr.i_next_prt; }
  inline ui32 get_rvp8_uiq_perm(int i) const { return _rvp8_hdr.uiq_perm[i]; }
  inline ui32 get_rvp8_uiq_once(int i) const { return _rvp8_hdr.uiq_once[i]; }
  inline si32 get_rvp8_i_data_off(int chan) const { return _rvp8_hdr.i_data_off[chan]; }
  inline fl32 get_rvp8_f_burst_mag(int chan) const { return _rvp8_hdr.f_burst_mag[chan]; }
  inline ui16 get_rvp8_i_burst_arg(int chan) const { return _rvp8_hdr.i_burst_arg[chan]; }
  inline ui16 get_rvp8_i_wrap_iq(int chan) const { return _rvp8_hdr.i_wrap_iq[chan]; }

  // Memory management.
  // This class uses the notion of clients to decide when it should be deleted.
  // If removeClient() returns 0, the object should be deleted.
  
  int addClient() const; 
  int removeClient() const;
  static void deleteIfUnused(IwrfTsPulse *pulse);
  int getNClients() const;

  // SIGMET ui16 pack/unpack routines

  static void vecFloatIQFromPackIQ
    (volatile fl32 fIQVals_a[], volatile const ui16 iCodes_a[],
     si32 iCount_a);
  
  static void vecFloatIQFromPackIQLegacy
  ( volatile fl32 fIQVals_a[], volatile const ui16 iCodes_a[],
    si32 iCount_a);

  static void vecPackIQFromFloatIQ
    (volatile ui16 iCodes_a[], volatile const fl32 fIQVals_a[],
     si32 iCount_a);

protected:

  // copy
  
  virtual IwrfTsPulse &_copy(const IwrfTsPulse &rhs);
  
private:

  // constants

  static const double PHASE_MULT;

  // info

  IwrfTsInfo &_info;
  
  // meta-data information
  
  iwrf_pulse_header_t _hdr;
  iwrf_rvp8_pulse_header_t _rvp8_hdr;
  bool _rvp8_header_active;

  // platform georeference for this pulse

  iwrf_platform_georef_t _georef;
  bool _georefActive;

  iwrf_georef_correction_t _georefCorr;
  bool _georefCorrActive;

  // invert HV flag

  mutable bool _invertHvFlag;

  // debugging

  IwrfDebug_t _debug;

  // derived from pulse header

  double _ftime;
  double _prf;
  double _phaseDiff[2]; // phase difference from previous pulse
  mutable vector<double> _phaseDiffs;

  // floating point IQ data

  fl32 *_iqData;  // pointer to float data
  fl32 *_chanIq[IWRF_MAX_CHAN]; // array of channel data pointers
  fl32 *_burstIq[IWRF_MAX_CHAN]; // array of pointers to burst channels
  MemBuf _iqBuf;  // float data is stored here

  // burst phase for phase (de)coding

  burst_phase_t _burstPhases;

  // packed 16-bit data

  iwrf_iq_encoding_t _packedEncoding;
  double _packedScale, _packedOffset;
  si16 *_packed; // pointer to packed data
  MemBuf _packedBuf; // packed data is stored here
  
  // memory handling

  mutable int _nClients;
  mutable pthread_mutex_t _nClientsMutex;

  // lookup table for converting packed 16-bit floats to 32-bit floats

  static bool _sigmetFloatLutReady;
  static fl32 _sigmetFloatLut[65536];
  static bool _sigmetLegacyUnpacking;
  static bool _sigmetLegacyUnpackingActive;
  static pthread_mutex_t _sigmetLutMutex;

  // functions
  
  void _checkScanModeForVert();
  void _checkRangeMembers();
  int _readRvp8Header(FILE *in);
  void _deriveFromRvp8Header();
  int _readRvp8Data(FILE *in);
  void _loadIqFromSigmetFL16();
  void _setDataPointers();
  void _fixZeroPower();
  void _computeSigmetFloatLut() const;

  void _clearIq();
  void _clearPacked();

  void _doPrintData(FILE *out, int channel, int startGate, int endGate);

};

#endif

