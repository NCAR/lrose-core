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
// IwrfTsBurst.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2006
//
///////////////////////////////////////////////////////////////

#ifndef IwrfTsBurst_hh
#define IwrfTsBurst_hh

#include <string>
#include <vector>
#include <toolsa/MemBuf.hh>
#include <dataport/port_types.h>
#include <radar/IwrfTsInfo.hh>
class OpsInfo;

using namespace std;

////////////////////////
// This class

class IwrfTsBurst {
  
public:

  // constructor

  IwrfTsBurst(IwrfDebug_t debug = IWRF_DEBUG_OFF);

  // copy constructor
  
  IwrfTsBurst(const IwrfTsBurst &rhs);

  // destructor

  virtual ~IwrfTsBurst();

  // assignment
  
  IwrfTsBurst & operator=(const IwrfTsBurst &rhs);

  // clear data

  void clear();
  
  // debugging

  void setDebug(IwrfDebug_t debug) { _debug = debug; }
  
  // set time on all packets
  
  void setTime(time_t secs, int nano_secs);
  void setTimeToNow();
  
  // set from burst buffer, swapping as required
  // optionally convert iq data to floats
  // Returns 0 on success, -1 on failure
  
  int setFromBuffer(const void *buf, int len, bool convertToFloat);

  // set IQ data as floats
  
  void setIqFloats(int nSamples, const fl32 *iq);
  
  // set IQ data packed
  
  void setIqPacked(int nSamples,
		   iwrf_iq_encoding_t encoding,
		   const si16 *packed,
		   double scale /* = 1.0 */,
		   double offset /* = 0.0 */);

  // convert packed data to float32
  
  void convertToFL32();
    
  // convert to specified packing
  
  void convertToPacked(iwrf_iq_encoding_t encoding,
		       double scale,
		       double offset);

  // assemble header and data into a single buffer

  void assemble(MemBuf &buf) const;

  // set headers directly

  void setHeader(const iwrf_burst_header_t &hdr);

  // set packet sequence number

  void setPktSeqNum(si64 pkt_seq_num);

  ///////////////////////////////////////////////////////////
  // write to file in IWRF format
  // returns 0 on success, -1 on failure

  int writeToFile(FILE *out) const;

  // printing
  
  void printHeader(FILE *out) const;
  void printData(FILE *out) const;
  void printData(FILE *out, int startSample, int endSample) const;

  // get methods

  inline double getFTime() const { return _ftime; }
  inline time_t getTime() const { return _hdr.packet.time_secs_utc; }
  inline int getNanoSecs() const { return _hdr.packet.time_nano_secs; }

  inline si64 getPktSeqNum() const { return _hdr.packet.seq_num; }
  inline si64 getPulseSeqNum() const { return _hdr.pulse_seq_num; }
  inline int getNSamples() const { return _hdr.n_samples; }
  inline int getChannelsId() const { return _hdr.channel_id; }

  inline fl32 getPowerDbm() const { return _hdr.power_dbm; }
  inline fl32 getPhaseDeg() const { return _hdr.phase_deg; }
  inline fl32 getFreqHz() const { return _hdr.freq_hz; }
  inline fl32 getSamplingFreqHz() const { return _hdr.sampling_freq_hz; }

  // get IQ data
  
  inline const fl32 *getIq() const { return _iq; }

  // get packed data

  iwrf_iq_encoding_t getPackedEncoding() const { return _packedEncoding; }
  double getPackedScale() const { return _packedScale; }
  double getPackedOffset() const { return _packedOffset; }
  const void *getPackedData() const;

  // get packet sequence number

  si64 getPktSeqNum() { return _hdr.packet.seq_num; }
  
protected:

  // copy
  
  virtual IwrfTsBurst &_copy(const IwrfTsBurst &rhs);
  
private:

  // meta-data information
  
  iwrf_burst_header_t _hdr;

  // debugging
  
  IwrfDebug_t _debug;

  // derived from burst header

  double _ftime;

  // floating point IQ data
  
  fl32 *_iq;  // data is stored here
  MemBuf _iqBuf;

  // packed 16-bit data

  iwrf_iq_encoding_t _packedEncoding;
  double _packedScale, _packedOffset;
  si16 *_packed;
  MemBuf _packedBuf;
  
  // functions
  
  void _clearIq();
  void _clearPacked();

};

#endif

