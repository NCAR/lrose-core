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
// RadarTsPulse.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2006
//
///////////////////////////////////////////////////////////////
//
// Class has been deprecated.
// Use IwrfTsPulse instead.
//
///////////////////////////////////////////////////////////////

#ifndef RadarTsPulse_hh
#define RadarTsPulse_hh

#include <string>
#include <vector>
#include <deque>
#include <pthread.h>
#include <dataport/port_types.h>
#include <rapformats/ds_radar_ts.h>
#include <radar/RadarTsInfo.hh>
class OpsInfo;

using namespace std;

////////////////////////
// This class

class RadarTsPulse {
  
public:

  RadarTsPulse(const RadarTsInfo &info,
               RadarTsDebug_t debug = RTS_DEBUG_OFF);

  ~RadarTsPulse();
  
  // debugging

  void setDebug(RadarTsDebug_t debug) { _debug = debug; }

  // invert HV flag?
  
  void setInvertHvFlag(bool invert = true) const { _invertHvFlag = invert; }
  
  // set from TS pulse buffer
  // Returns 0 on success, -1 on failure
  
  int setFromTsBuffer(const void *buf, int len);

  // set RVP8-specific fields
  
  void setRvp8Hdr(const ts_pulse_hdr_t &hdr);
  
  // read pulse data from RVP8 file
  // Returns 0 on success, -1 on failure

  int readFromRvp8File(FILE *in);

  // write to tsarchive file
  // Returns 0 on success, -1 on failure
  
  int write2TsarchiveFile(FILE *out);
    
  // compute phase differences between this pulse and previous ones
  // to be able to cohere to multiple trips
  //
  // Before this method is called, this pulse should be added to
  // the queue.
  
  int computePhaseDiffs(const deque<const RadarTsPulse *> &pulseQueue,
			int maxTrips) const;
  
  // printing
  
  void print(ostream &out) const;

  // get methods

  const ts_pulse_hdr_t &getHdr() const { return _hdr; }

  int getNGates() const { return _hdr.nGates; }
  int getNChannels() const { return _hdr.nChannels; }
  int getRadarId() const { return _hdr.radarId; }
  long getSeqNum() const { return _hdr.pulseSeqNum; }
  double getFTime() const { return _ftime; }
  time_t getTime() const { return _hdr.timeSecsUTC; }
  double getPrt() const { return _hdr.prt; }
  double getPrf() const { return _prf; }
  double getPulseWidth() const { return _hdr.pulseWidth; }
  double getMeasXmitPowerDbmH() const { return _hdr.measXmitPowerDbmH; }
  double getMeasXmitPowerDbmV() const { return _hdr.measXmitPowerDbmV; }

  double getEl() const;
  double getAz() const;
  double getTargetEl() const;
  double getTargetAz() const; // RHI mode

  int getVolNum() const { return _hdr.volNum; }
  int getTiltNum() const { return _hdr.tiltNum; }
  double getPhaseDiff0() const { return _phaseDiff[0]; }
  double getPhaseDiff1() const { return _phaseDiff[1]; }
  double getPulseWidthUs() const { return _hdr.pulseWidth; }
  bool isHoriz() const; // is horizontally polarized
  bool antennaTransition() const { return (bool) _hdr.antennaTransition; }

  ts_scan_mode_t getScanMode() const {
    return ( ts_scan_mode_t) _hdr.scanMode;
  }

  // get IQ data
  
  const fl32 *getIq0() const { return _iq0; }
  const fl32 *getIq1() const { return _iq1; }
  const fl32 *getBurstIq0() const { return _burstIq0; }
  const fl32 *getBurstIq1() const { return _burstIq1; }

  // get packed data

  const ui16 *getPacked() const;

  // Memory management.
  // This class uses the notion of clients to decide when it should be deleted.
  // If removeClient() returns 0, the object should be deleted.
  
  int addClient(const string &clientName) const; 
  int removeClient(const string &clientName) const;

  // SIGMET ui16 pack/unpack routines

  static void vecFloatIQFromPackIQ
    (volatile fl32 fIQVals_a[], volatile const ui16 iCodes_a[],
     si32 iCount_a);
  
  static void vecPackIQFromFloatIQ
    (volatile ui16 iCodes_a[], volatile const fl32 fIQVals_a[],
     si32 iCount_a);

protected:
private:

  const RadarTsInfo &_info;

  // most of the metadata is in the header

  ts_pulse_hdr_t _hdr;

  // invcert HV flag

  mutable bool _invertHvFlag;

  // debugging

  RadarTsDebug_t _debug;

  // derived from pulse header

  double _ftime;
  double _prf;
  double _phaseDiff[2]; // phase difference from previous pulse
  mutable vector<double> _phaseDiffs;

  // floating point IQ data

  fl32 *_iq;  // data is stored here
  fl32 *_iq0; // channel 0 - points into _iq
  fl32 *_iq1; // channel 1 if applicable - points into _iq
  fl32 *_burstIq0; // burst channel 0 - points into _iq
  fl32 *_burstIq1; // burst channel 1 if applicable - points into _iq

  // packed 16-bit data

  mutable ui16 *_packed;
  
  // lookup table for converting packed 16-bit floats to 32-bit floats

  static bool _floatLutReady;
  static fl32 _floatLut[65536];

  // memory handling

  mutable int _nClients;
  mutable pthread_mutex_t _nClientsMutex;

  // functions
  
  void _checkScanModeForVert();
  int _readRvp8Header(FILE *in);
  void _deriveFromRvp8Header();
  int _readRvp8Data(FILE *in);
  void _loadIqFromPacked();
  
  void _computeFloatLut();

};

#endif

