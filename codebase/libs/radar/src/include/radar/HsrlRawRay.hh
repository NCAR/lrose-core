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

  /// @brief Construct a HsrlRawRay instance with no time or data
  HsrlRawRay();

  /// @brief default destructor
  virtual ~HsrlRawRay();

  // set methods

  /// @brief Set the time of the ray
  /// @param timeSecs time of the ray in integer seconds since
  /// 1970-01-01 00:00:00 UTC
  /// @param subSecs subsecond time to be added to timeSecs to obtain the
  /// complete time of the ray; this should be < 1 second.
  void setTime(time_t timeSecs, double subSecs) {
    _timeSecs = timeSecs;
    _subSecs = subSecs;
  }

  /// @brief Set the "locked" state of the telescope for the ray
  /// @param val true iff the telescope is locked
  void setTelescopeLocked(bool val) { _telescopeLocked = val; }

  /// @brief Set the telescope pointing direction for the ray
  /// @param val 0 = down, 1 = up
  void setTelescopeDirn(int val) { _telescopeDirn = val; }

  /// @brief Set the total energy counts for the ray
  /// @param val the total energy counts for the ray
  void setTotalEnergy(int val) { _totalEnergy = val; }

  /// @brief Set the polarization angle for the ray, in degrees
  /// @param val polarization angle for the ray, in degrees
  void setPolAngle(double val) { _polAngle = val; }

  /// @brief Set the vectors of combinedHi, combinedLo, molecular, and cross-
  /// channel returns for the ray
  /// @param nGates the number of gates recorded in the ray
  /// @param combinedHi pointer to an array containing @p nGates of combinedHi
  /// channel returns for the ray
  /// @param combinedLo pointer to an array containing @p nGates of combinedLo
  /// channel returns for the ray
  /// @param molecular pointer to an array containing @p nGates of molecular
  /// channel returns for the ray
  /// @param cross pointer to an array containing @p nGates of cross channel
  /// returns for the ray
  void setFields(int nGates,
                 const float32 *combinedHi,
                 const float32 *combinedLo,
                 const float32 *molecular,
                 const float32 *cross);

  // get methods

  /// @brief Return the time of the ray in integer seconds since 1970-01-01
  /// 00:00:00 UTC
  time_t getTimeSecs() const { return _timeSecs; }

  /// @brief Return the subsecond time of the ray
  ///
  /// This time should be added to the value from getTimeSecs() to obtain the
  /// complete time of the ray. The returned value will be < 1 second.
  /// @return the subsecond time of the ray
  double getSubSecs() const { return _subSecs; }

  /// @brief Return true iff the telescope was locked for this ray
  /// @return true iff the telescope was locked for this ray
  bool getTelescopeLocked() const { return _telescopeLocked; }

  /// @brief Return the telescope pointing direction for this ray: 0 = down,
  /// 1 = up
  /// @return the telescope pointing direction for this ray: 0 = down, 1 = up
  int getTelescopeDirn() const { return _telescopeDirn; }

  /// @brief Return the total energy counts for this ray
  /// @return the total energy counts for this ray
  int getTotalEnergy() const { return _totalEnergy; }

  /// @brief Return the polarization angle for this ray
  /// @return the polarization angle for this ray
  double getPolAngle() const { return _polAngle; }

  /// @brief Return the number of gates recorded in this ray
  /// @return the number of gates recorded in this ray
  int getNGates() const { return _nGates; }

  /// @brief Return the array of combined high channel counts for all gates of
  /// this ray
  /// @return the array of combined high channel counts for all gates of this ray
  const std::vector<float32> &getCombinedHi() const { return _combinedHi; }

  /// @brief Return the array of combined low channel counts for all gates of
  /// this ray
  /// @return the array of combined low channel counts for all gates of this ray
  const std::vector<float32> &getCombinedLo() const { return _combinedLo; }

  /// @brief Return the array of molecular channel counts for all gates of
  /// this ray
  /// @return the array of molecular channel counts for all gates of this ray
  const std::vector<float32> &getMolecular() const { return _molecular; }

  /// @brief Return the array of cross channel counts for all gates of
  /// this ray
  /// @return the array of cross channel counts for all gates of this ray
  const std::vector<float32> &getCross() const { return _cross; }

  /// @brief Serialize object into a buffer for transmission.
  ///
  /// After calling, use getBufPtr() and getBufLen() to get the contents and
  /// length of the buffer.
  void serialize();

  /// @brief Return a pointer to the buffer populated by serialize()
  /// @return a pointer to the buffer populated by serialize()
  const void *getBufPtr() const { return _packetBuf; }
  
  /// @brief Return the length of the buffer populated by serialize()
  /// @return the length of the buffer populated by serialize()
  int getBufLen() const { return _bufLen; }

  /// @brief Deserialize from the given buffer to populate this object.
  /// @param buffer pointer to the buffer from which to deserialize
  /// @param bufLen the length of the buffer
  /// @return 0 on success or -1 on error
  int deserialize(const void *buffer, int bufLen);

  /// @brief Cookie used to identify if bytes have been swapped in transmission
  static const int64_t COOKIE = 987654321;

  // check if ID needs swapping
  
  static bool idIsSwapped(int64_t id) {
    if (id == HsrlRawRay::COOKIE) {
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

  /// @brief Current version number for the _tcp_hdr_t struct
  // This number should be incremented if the _tcp_hdr_t struct below is altered
  static const int64_t _HEADER_VERSION = 1;

  /// @brief Inner class defining the header structure used when serializing
  /// an instance of HsrlRawRay.
  ///
  /// The length of the struct is 128 bytes
  typedef struct {

    int64_t id;
    int64_t len_bytes; // header plus data fields
    int64_t version_num;

    int64_t time_secs_utc;
    int64_t time_nano_secs;

    int64_t spares64bit[5];

    int32_t telescope_locked;
    int32_t telescope_dirn;

    int32_t total_energy;
    float32 pol_angle;

    int32_t n_gates;
    int32_t spares32bit[7];

  } _tcp_hdr_t;

  // private data

  time_t _timeSecs;
  double _subSecs;
  
  bool _telescopeLocked;
  int _telescopeDirn; // 1 is up, 0 is down

  int _totalEnergy;
  double _polAngle;

  int _nGates;

  static const int _NFIELDS = 4;
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
  
  void _SwapHdr(_tcp_hdr_t *hdr);

  /// Perform an in-place 64-bit word byte swap, if necessary, to produce
  /// BE representation from machine representation, or vice-versa.
  /// 
  /// Array must be aligned on an 8-byte boundary in memory.
  
  void _Swap64(void *array, size_t nbytes);
  
  /// Performs an in-place 32-bit word byte swap, if necessary, to produce
  /// BE representation from machine representation, or vice-versa.
  /// 
  /// Array must be aligned on an 4-byte boundary in memory.
  
  void _Swap32(void *array, size_t nbytes);
  
};

#endif
