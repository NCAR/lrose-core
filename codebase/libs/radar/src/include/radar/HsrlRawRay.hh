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
// HsrlRawRay
//
// Holds raw data from a single HSRL ray or beam
//
// Mike Dixon, Brad Schoenrock, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2017
//
///////////////////////////////////////////////////////////////

#ifndef HsrlRawRay_hh
#define HsrlRawRay_hh

#include <string>
#include <vector>
#include <iostream>
#include <sys/types.h>

typedef float float32; 
typedef double float64; 

class HsrlRawRay {

public:

  // default constructor
  
  HsrlRawRay();
  
  // destructor
  
  virtual ~HsrlRawRay();

  // set methods

  void setTime(time_t timeSecs, double subSecs) {
    _timeSecs = timeSecs;
    _subSecs = subSecs;
  }

  void setTelescopeLocked(bool val) { _telescopeLocked = val; }
  void setTelescopeDirn(int val) { _telescopeDirn = val; }
  void setTotalEnergy(int val) { _totalEnergy = val; }
  void setPolAngle(double val) { _polAngle = val; }

  void setFields(int nGates,
                 const float32 *combinedHi,
                 const float32 *combinedLo,
                 const float32 *molecular,
                 const float32 *cross);

  // get methods

  time_t getTimeSecs() const { return _timeSecs; }
  double getSubSecs() const { return _subSecs; }

  bool getTelescopeLocked() const { return _telescopeLocked; }
  int getTelescopeDirn() const { return _telescopeDirn; }
  int getTotalEnergy() const { return _totalEnergy; }
  double getPolAngle() const { return _polAngle; }

  int getNGates() const { return _nGates; }
  const std::vector<float32> &getCombinedHi() const { return _combinedHi; }
  const std::vector<float32> &getCombinedLo() const { return _combinedLo; }
  const std::vector<float32> &getMolecular() const { return _molecular; }
  const std::vector<float32> &getCross() const { return _cross; }

  // Serialize object into buffer for transmission.
  // After calling, call getBufPtr() and getBufLen()
  // to get the details of the buffer.

  void serialize();

  // get pointer to buffer from serialization
  
  const void *getBufPtr() const { return _packetBuf; }
  
  // get length of buffer from serialization

  int getBufLen() const { return _bufLen; }

  // dserialize from buffer into the object
  // returns 0 on success, -1 on error

  int dserialize(const char *buffer, int bufLen);

  // cookie to identify raw HSRL ray

  static const int64_t cookie = 987654321;

  // check if ID needs swapping
  
  static bool idIsSwapped(int64_t id) {
    if (id == HsrlRawRay::cookie) {
      // is swapped
      return false;
    }
    return true;
  }
  
  //  Print metadata
  
  void printMetaData(std::ostream &out);

  //  Print tcp message header in the current buffer
 
  void printTcpHdr(std::ostream &out);

protected:
private:

  // typedef for header struct for tcp packet
  // 128 bytes long

  int64_t _seqNum = 0;
  
  typedef struct {

    int64_t id;
    int64_t len_bytes; // header plus data fields
    int64_t seq_num;
    int64_t version_num;

    int64_t time_secs_utc;
    int64_t time_nano_secs;

    int64_t spares1[4];

    int32_t telescope_locked;
    int32_t telescope_dirn;

    int32_t total_energy;
    float32 pol_angle;

    int32_t n_gates;
    int32_t spares2[7];

  } tcp_hdr_t;

  // private data

  time_t _timeSecs;
  double _subSecs;
  
  bool _telescopeLocked;
  int _telescopeDirn; // 1 is up, 0 is down

  int _totalEnergy;
  double _polAngle;

  int _nGates;

  const int _nFields = 4;
  std::vector<float32> _combinedHi;
  std::vector<float32> _combinedLo;
  std::vector<float32> _molecular;
  std::vector<float32> _cross;

  // tcp packet array

  int _fieldLen;
  int _bufLen;
  char *_packetBuf;

  // swapping

  // Swap the TCP header
  
  void _swapHdr(tcp_hdr_t *hdr);

  /// Perform an in-place 64-bit word byte swap, if necessary, to produce
  /// BE representation from machine representation, or vice-versa.
  /// 
  /// Array must be aligned on an 8-byte boundary in memory.
  
  void _swap64(void *array, size_t nbytes);
  
  /// Performs an in-place 32-bit word byte swap, if necessary, to produce
  /// BE representation from machine representation, or vice-versa.
  /// 
  /// Array must be aligned on an 4-byte boundary in memory.
  
  void _swap32(void *array, size_t nbytes);
  
};

#endif
