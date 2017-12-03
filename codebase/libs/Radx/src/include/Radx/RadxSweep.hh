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
// RadxSweep.hh
//
// Field object for Radx data
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2009
//
///////////////////////////////////////////////////////////////

#ifndef RadxSweep_HH
#define RadxSweep_HH

#include <iostream>
#include <Radx/Radx.hh>
#include <Radx/RadxMsg.hh>
using namespace std;

///////////////////////////////////////////////////////////////
/// SWEEP CLASS
///
/// The sweep class provides the information required to group rays
/// into sweeps.
///
/// A volume comprises a number of rays, from one or more sweeps.
///
/// The sweep class does not hold field data itself. Rather it
/// provides the information required to group rays into sweeps.

class RadxSweep {

public:

  /// Constructor
  
  RadxSweep();
  
  /// Copy constructor
  
  RadxSweep(const RadxSweep &rhs);
  
  /// Destructor

  ~RadxSweep();

  /// Assignment
  
  RadxSweep& operator=(const RadxSweep &rhs);
  
  /// Clear the metadata in the object
  
  void clear();

  //////////////////////////////////////////////////////////////
  // Set methods

  /// \name Set methods:
  //@{

  /// Set the volume number.

  inline void setVolumeNumber(int val) { _volNum = val; }

  /// Set the sweep number.

  inline void setSweepNumber(int val) { _sweepNum = val; }

  /// Set the index of the first ray in the sweep.
  ///
  /// This is the position of the first ray in the sweep, in the
  /// vector of sweeps held by RadxVol.

  inline void setStartRayIndex(size_t val) { _startRayIndex = val; }

  /// Set the index of the last ray in the sweep.
  ///
  /// This is the position of the last ray in the sweep, in the
  /// vector of sweeps held by RadxVol.
  ///
  /// The sweep comprised the first ray to the last ray, inclusive.

  inline void setEndRayIndex(size_t val) { _endRayIndex = val; }

  /// Set the scanning mode for the sweep.

  inline void setSweepMode(Radx::SweepMode_t val) {
    _sweepMode = val;
  }
  
  /// Set the polarization mode for the sweep.

  inline void setPolarizationMode(Radx::PolarizationMode_t val) {
    _polarizationMode = val;
  }

  /// Set the PRT mode for the sweep.

  inline void setPrtMode(Radx::PrtMode_t val) {
    _prtMode = val;
  }

  /// Set the target following mode for the sweep, if applicable.

  inline void setFollowMode(Radx::FollowMode_t val) {
    _followMode = val;
  }

  /// Set the fixed angle for the sweep.
  ///
  /// This will be azimuth in RHI mode and ELEVATION_SURVEILLANCE
  /// mode, elevation for all other scans.

  inline void setFixedAngleDeg(double val) { _fixedAngle = val; }

  /// Set the intended scan rate for the sweep.
  /// Units: deg/s

  inline void setTargetScanRateDegPerSec(double val) {
    _targetScanRate = val;
  }

  /// Set the measured scan rate for the sweep.
  /// Units: deg/s

  inline void setMeasuredScanRateDegPerSec(double val) {
    _measuredScanRate = val;
  }

  /// Set to true if the rays in the sweep are indexed.

  inline void setRaysAreIndexed(bool val) { _raysAreIndexed = val; }

  /// If rays are indexed, set to the angular resolution in degrees.

  inline void setAngleResDeg(double val) { _angleRes = val; }

  /// Set the intermediate frequency, in hz, if appropriate
  /// Generally this will only be used for Gematronik data

  inline void setIntermedFreqHz(double val) { _intermedFreqHz = val; }

  // set that this is a long-range sweep
  
  void setIsLongRange(bool val) { _isLongRange = val; }

  //@}
  //////////////////////////////////////////////////////////////
  // Get methods

  /// \name Get methods:
  //@{

  /// Get the volume number.
  
  inline int getVolumeNumber() const { return _volNum; }

  /// Get the sweep number.
  
  inline int getSweepNumber() const { return _sweepNum; }

  /// Get the index of the first ray in the sweep.
  ///
  /// This is the position of the first ray in the sweep, in the
  /// vector of sweeps held by RadxVol.
  
  inline size_t getStartRayIndex() const { return _startRayIndex; }

  /// Get the index of the last ray in the sweep.
  ///
  /// This is the position of the last ray in the sweep, in the
  /// vector of sweeps held by RadxVol.
  ///
  /// The sweep comprised the first ray to the last ray, inclusive.
  
  inline size_t getEndRayIndex() const { return _endRayIndex; }

  /// get number of rays in sweep

  inline size_t getNRays() const { return _endRayIndex - _startRayIndex + 1; }

  /// Get the scanning mode for the sweep.
  
  inline Radx::SweepMode_t getSweepMode() const { return _sweepMode; }
  
  /// Get the polarization mode for the sweep.
  
  inline Radx::PolarizationMode_t getPolarizationMode() const {
    return _polarizationMode;
  }

  /// Get the PRT mode for the sweep.
  
  inline Radx::PrtMode_t getPrtMode() const { return _prtMode; }

  /// Get the target following mode for the sweep, if applicable.

  inline Radx::FollowMode_t getFollowMode() const { return _followMode; }

  /// Get the fixed angle for the sweep.
  ///
  /// This will be azimuth in RHI mode and ELEVATION_SURVEILLANCE
  /// mode, elevation for all other scans.
  
  inline double getFixedAngleDeg() const { return _fixedAngle; }
  
  /// Get the intended scan rate for the sweep.
  /// Units: deg/s

  inline double getTargetScanRateDegPerSec() const { return _targetScanRate; }
  
  /// Get the measured scan rate for the sweep.
  /// Units: deg/s

  inline double getMeasuredScanRateDegPerSec() const { return _measuredScanRate; }
  
  /// Check whether the rays in the sweep are indexed in angle.

  inline bool getRaysAreIndexed() const { return _raysAreIndexed; }
  
  /// If rays are indexed, get the angular resolution in degrees.

  inline double getAngleResDeg() const { return _angleRes; }
  
  /// Get the intermediate frequency, in hz, if appropriate
  /// Generally this will only be used for Gematronik data

  inline double getIntermedFreqHz() const { return _intermedFreqHz; }

  /// is this a long-range ray?
  
  bool getIsLongRange() const { return _isLongRange; }

  //@}
  
  /// \name Print methods:
  //@{

  /// Print metadata.
  
  void print(ostream &out) const;
  
  //@}
  
  /// \name Serialization:
  //@{

  // serialize into a RadxMsg
  
  void serialize(RadxMsg &msg,
                 RadxMsg::RadxMsg_t msgType = RadxMsg::RadxSweepMsg);

  // deserialize from a RadxMsg
  // return 0 on success, -1 on failure

  int deserialize(const RadxMsg &msg);

  //@}

protected:
private:

  // data

  int _volNum;
  int _sweepNum;
  
  size_t _startRayIndex; // in volume file
  size_t _endRayIndex;   // in volume file

  Radx::SweepMode_t _sweepMode;
  Radx::PolarizationMode_t _polarizationMode;
  Radx::PrtMode_t _prtMode;
  Radx::FollowMode_t _followMode;

  double _fixedAngle;      // deg
  double _targetScanRate;  // deg/s
  double _measuredScanRate;  // deg/s

  bool _raysAreIndexed;  // rays are on regular angular intervals
  double _angleRes;      // angular resolution if indexed (deg)

  double _intermedFreqHz; // intermediate frequency in hz
  
  // is this a long-range sweep?

  bool _isLongRange;

  // methods
  
  void _init();
  RadxSweep & _copy(const RadxSweep &rhs);

  /////////////////////////////////////////////////
  // serialization
  /////////////////////////////////////////////////

  static const int _metaNumbersPartId = 2;
  
  // struct for metadata numbers in messages
  // strings not included - they are passed as XML
  
  typedef struct {
    
    Radx::fl64 fixedAngle;
    Radx::fl64 targetScanRate;
    Radx::fl64 measuredScanRate;
    Radx::fl64 angleRes;
    Radx::fl64 intermedFreqHz;

    Radx::si64 startRayIndex;
    Radx::si64 endRayIndex;

    Radx::si64 spareSi64[9];

    Radx::si32 volNum;
    Radx::si32 sweepNum;

    Radx::si32 sweepMode;
    Radx::si32 polarizationMode;
    Radx::si32 prtMode;
    Radx::si32 followMode;

    Radx::si32 raysAreIndexed;
    Radx::si32 isLongRange;

    Radx::si32 spareSi32[8];

  } msgMetaNumbers_t;

  msgMetaNumbers_t _metaNumbers;
  
  /// load meta numbers to message struct
  
  void _loadMetaNumbersToMsg();
  
  /// set the meta number data from the message struct
  /// returns 0 on success, -1 on failure
  
  int _setMetaNumbersFromMsg(const msgMetaNumbers_t *metaNumbers,
                             size_t bufLen,
                             bool swap);
  
  /// swap meta numbers
  
  static void _swapMetaNumbers(msgMetaNumbers_t &msgMetaNumbers);
          
};

#endif

