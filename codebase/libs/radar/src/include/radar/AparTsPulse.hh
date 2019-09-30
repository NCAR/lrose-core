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
// AparTsPulse.hh
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Aug 2019
//
///////////////////////////////////////////////////////////////

#ifndef AparTsPulse_hh
#define AparTsPulse_hh

#include <string>
#include <vector>
#include <deque>
#include <pthread.h>
#include <toolsa/MemBuf.hh>
#include <dataport/port_types.h>
#include <radar/AparTsInfo.hh>
#include <radar/RadarComplex.hh>
class OpsInfo;

using namespace std;

////////////////////////
// This class

class AparTsPulse {
  
public:

  // constructor

  AparTsPulse(AparTsInfo &info,
	      AparTsDebug_t debug = APAR_TS_DEBUG_OFF);

  // copy constructor
  
  AparTsPulse(const AparTsPulse &rhs);

  // destructor

  virtual ~AparTsPulse();

  // assignment
  
  AparTsPulse & operator=(const AparTsPulse &rhs);

  // clear data

  void clear();

  // set the ops info

  void setOpsInfo(AparTsInfo &info) {
    if (&_info != &info) _info = info;
  }
  
  // debugging

  void setDebug(AparTsDebug_t debug) { _debug = debug; }
  
  // invert HV flag?
  // The HV flag is defined as 1 for H, 0 for V.
  // If the sense of the HV flag in the header is incorrect,
  // you can invert the flag when the pulse header is loaded.
  
  void setInvertHvFlag(bool invert = true) const { _invertHvFlag = invert; }
  
  // set time on all packets
  
  void setTime(time_t secs, int nano_secs);
  void setTimeToNow();

  // set the platform georeference for this pulse

  void setPlatformGeoref(const apar_ts_platform_georef_t &georef) {
    _georef = georef;
    _georefActive = true;
  }

  void setGeorefCorr(const apar_ts_georef_correction_t &georefCorr) {
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
		   apar_ts_iq_encoding_t encoding,
		   const si16 *packed,
		   double scale /* = 1.0 */,
		   double offset /* = 0.0 */);

  // convert packed data to float32
  
  void convertToFL32();
    
  // convert to specified packing
  
  void convertToPacked(apar_ts_iq_encoding_t encoding);

  // convert to scaled si16 packing
  
  void convertToScaledSi16(double scale, double offset);
  
  // set the scale and offset values for scaled si16 packing
  // does not change the data, only the metadata
  
  void setScaleAndOffsetForSi16(double scale, double offset);
  
  // swaps I and Q, because they are stored in the incorrect order
  // in the data arrays
  
  void swapIQ();

  // invert Q values in the data arrays
  
  void invertQ();

  // copy the pulse width from the ts_proc in the info
  
  void copyPulseWidthFromTsProc();

  // assemble header and data into a single buffer

  void assemble(MemBuf &buf) const;

  // set headers directly

  void setHeader(const apar_ts_pulse_header_t &hdr);

  // set radar ID
  
  void setRadarId(int id);

  // set packet sequence numbers for each header

  void setPktSeqNum(si64 pkt_seq_num);

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
  
  int computePhaseDiffs(const deque<AparTsPulse *> &pulseQueue,
			int maxTrips) const;

  int computePhaseDiffs(int qSize,
                        const AparTsPulse **pulseQueue,
			int maxTrips) const;

  // write to file in APAR time series format
  // returns 0 on success, -1 on failure

  int writeToFile(FILE *out) const;

  // printing
  
  void printHeader(FILE *out) const;
  void printData(FILE *out) const;
  void printData(FILE *out, int startGate, int endGate) const;

  // get methods

  inline AparTsInfo &getTsInfo() { return _info; }
  inline const AparTsInfo &getTsInfo() const { return _info; }

  inline const apar_ts_pulse_header_t &getHdr() const { return _hdr; }

  inline int getRadarId() const { return _hdr.packet.radar_id; }
  inline si64 getSeqNum() const { return _hdr.pulse_seq_num; }
  inline double getFTime() const { return _ftime; }
  inline time_t getTime() const { return _hdr.packet.time_secs_utc; }
  inline int getNanoSecs() const { return _hdr.packet.time_nano_secs; }
  inline double getPrt() const { return _hdr.prt; }
  inline double getPrf() const { return _prf; }
  
  inline double getPhaseDiff0() const { return _phaseDiff[0]; }
  inline double getPhaseDiff1() const { return _phaseDiff[1]; }
  bool isHoriz() const; // is horizontally polarized

  apar_ts_scan_mode_t getScanMode() const {
    return ( apar_ts_scan_mode_t) _hdr.scan_mode;
  }

  inline si64 getPktSeqNum() const { return _hdr.packet.seq_num; }
  inline si64 getPulseSeqNum() const { return _hdr.pulse_seq_num; }
  inline si64 getDwellSeqNum() const { return _hdr.dwell_seq_num; }

  inline si32 getBeamNumInDwell() const { return _hdr.beam_num_in_dwell; }
  inline si32 getVisitNumInBeam() const { return _hdr.visit_num_in_beam; }

  inline si32 getSweepNum() const { return _hdr.sweep_num; }
  inline si32 getVolumeNum() const { return _hdr.volume_num; }
  double getElevation() const;
  double getAzimuth() const;
  double getFixedAngle() const;
  inline double getPrtNext() const { return _hdr.prt_next; }
  inline double getPulseWidthUs() const { return _hdr.pulse_width_us; }
  inline si32 getNGates() const { return _hdr.n_gates; }
  inline si32 getNChannels() const { return _hdr.n_channels; }
  inline si32 getIqEncoding() const { return _hdr.iq_encoding; }
  inline si32 getHvFlag() const { return _hdr.hv_flag; }
  inline si32 getPhaseCohered() const { return _hdr.phase_cohered; }
  inline si32 getStatus() const { return _hdr.status; }
  inline si32 getNData() const { return _hdr.n_data; }
  inline double getScale() const { return _hdr.scale; }
  inline double getOffset() const { return _hdr.offset; }
  inline double getStartRangeM() const { return _hdr.start_range_m; }
  inline double getStartRangeKm() const {
    return _hdr.start_range_m / 1000.0;
  }
  inline double getGateSpacingM() const { return _hdr.gate_spacing_m; }
  inline double getGateSpacingKm() const {
    return _hdr.gate_spacing_m / 1000.0;
  }
  
  inline bool getStartOfSweep() {
    return (_hdr.event_flags & APAR_TS_START_OF_SWEEP);
  }
  inline bool getStartOfVolume() {
    return (_hdr.event_flags & APAR_TS_START_OF_VOLUME);
  }
  inline bool getEndOfSweep() {
    return (_hdr.event_flags & APAR_TS_END_OF_SWEEP);
  }
  inline bool getEndOfVolume() {
    return (_hdr.event_flags & APAR_TS_END_OF_VOLUME);
  }

  inline bool getChanIsCopol(int ii) const {
    if (ii >= 0 && ii < APAR_TS_MAX_CHAN) {
      return _hdr.chan_is_copol[ii];
    } else {
      return -1;
    }
  }

  // get the platform georeference for this pulse

  const apar_ts_platform_georef_t &getPlatformGeoref() const {
    return _georef;
  }
  bool getGeorefActive() const { return _georefActive; }

  const apar_ts_georef_correction_t &getGeorefCorrection() const {
    return _georefCorr;
  }
  bool getGeorefCorrActive() const { return _georefCorrActive; }

  // get IQ data arrays
  
  inline const fl32 *getIq0() const { return _chanIq[0]; }
  inline const fl32 *getIq1() const { return _chanIq[1]; }
  inline const fl32 *getIq2() const { return _chanIq[2]; }
  inline const fl32 *getIq3() const { return _chanIq[3]; }

  inline fl32 **getIqArray() { return _chanIq; }

  // get IQ data at a gate
  // returns APAR_TS_MISSING_FLOAT if not available at that gate for that channel
  
  void getIq0(int gateNum, fl32 &ival, fl32 &qval) const;
  void getIq1(int gateNum, fl32 &ival, fl32 &qval) const;
  void getIq2(int gateNum, fl32 &ival, fl32 &qval) const;
  void getIq3(int gateNum, fl32 &ival, fl32 &qval) const;
  void getIq(int chanNum, int gateNum, fl32 &ival, fl32 &qval) const;

  // get packed data

  apar_ts_iq_encoding_t getPackedEncoding() const { return _packedEncoding; }
  double getPackedScale() const { return _packedScale; }
  double getPackedOffset() const { return _packedOffset; }
  const void *getPackedData() const;

  // set individual fields

  inline void setBeamNumInDwell(si32 x) { _hdr.beam_num_in_dwell = x; }
  inline void setVisitNumInBeam(si32 x) { _hdr.visit_num_in_beam = x; }

  inline void setPulseSeqNum(si64 x) { _hdr.pulse_seq_num = x; }
  inline void setDwellSeqNum(si64 x) { _hdr.dwell_seq_num = x; }

  inline void setScanMode(si32 x) { _hdr.scan_mode = x; }
  inline void setSweepNum(si32 x) { _hdr.sweep_num = x; }
  inline void setVolumeNum(si32 x) { _hdr.volume_num = x; }
  inline void setElevation(fl32 x) { _hdr.elevation = x; }
  inline void setAzimuth(fl32 x) { _hdr.azimuth = x; }
  inline void setFixedAngle(fl32 x) { _hdr.fixed_angle = x; }
  inline void setPrt(fl32 x) { _hdr.prt = x; }
  inline void setPrtNext(fl32 x) { _hdr.prt_next = x; }
  inline void setPulseWidthUs(fl32 x) { _hdr.pulse_width_us = x; }
  inline void setNGates(si32 x) { _hdr.n_gates = x; }
  inline void setNChannels(si32 x) { _hdr.n_channels = x; }
  inline void setIqEncoding(si32 x) { _hdr.iq_encoding = x; }
  inline void setHvFlag(si32 x) { _hdr.hv_flag = x; }
  inline void setPhaseCohered(si32 x) { _hdr.phase_cohered = x; }
  inline void setStatus(si32 x) { _hdr.status = x; }
  inline void setNData(si32 x) { _hdr.n_data = x; }
  inline void setScale(fl32 x) { _hdr.scale = x; }
  inline void setOffset(fl32 x) { _hdr.offset = x; }
  inline void setStartRangeM(fl32 x) { _hdr.start_range_m = x; }
  inline void setGateGSpacineM(fl32 x) { _hdr.gate_spacing_m = x; }

  inline void setStartOfSweep() {
    _hdr.event_flags |= APAR_TS_START_OF_SWEEP;
  }
  inline void setStartOfVolume() {
    _hdr.event_flags |= APAR_TS_START_OF_VOLUME;
  }
  inline void setEndOfSweep() {
    _hdr.event_flags |= APAR_TS_END_OF_SWEEP;
  }
  inline void setEndOfVolume() {
    _hdr.event_flags |= APAR_TS_END_OF_VOLUME;
  }

  // Memory management.
  // This class uses the notion of clients to decide when it should be deleted.
  // If removeClient() returns 0, the object should be deleted.
  
  int addClient() const; 
  int removeClient() const;
  static void deleteIfUnused(AparTsPulse *pulse);
  int getNClients() const;

protected:

  // copy
  
  virtual AparTsPulse &_copy(const AparTsPulse &rhs);
  
private:

  // constants

  static const double PHASE_MULT;

  // info

  AparTsInfo &_info;
  
  // meta-data information
  
  apar_ts_pulse_header_t _hdr;

  // platform georeference for this pulse

  apar_ts_platform_georef_t _georef;
  bool _georefActive;

  apar_ts_georef_correction_t _georefCorr;
  bool _georefCorrActive;

  // invert HV flag

  mutable bool _invertHvFlag;

  // debugging

  AparTsDebug_t _debug;

  // derived from pulse header

  double _ftime;
  double _prf;
  double _phaseDiff[2]; // phase difference from previous pulse
  mutable vector<double> _phaseDiffs;

  // floating point IQ data

  fl32 *_iqData;  // pointer to float data
  fl32 *_chanIq[APAR_TS_MAX_CHAN]; // array of channel data pointers
  MemBuf _iqBuf;  // float data is stored here

  // packed 16-bit data

  apar_ts_iq_encoding_t _packedEncoding;
  double _packedScale, _packedOffset;
  si16 *_packed; // pointer to packed data
  MemBuf _packedBuf; // packed data is stored here
  
  // memory handling

  mutable int _nClients;
  mutable pthread_mutex_t _nClientsMutex;

  // functions
  
  void _checkRangeMembers();
  void _setDataPointers();
  void _fixZeroPower();

  void _clearIq();
  void _clearPacked();

  void _doPrintData(FILE *out, int channel, int startGate, int endGate);
  void _printFl32Data(FILE *out, int channel, int startGate, int endGate);

};

#endif

