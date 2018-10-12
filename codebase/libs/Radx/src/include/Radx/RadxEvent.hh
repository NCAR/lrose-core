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
// RadxEvent.hh
//
// Event object for Radx data
//
// e.g. end of sweep, end of volume
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2012
//
///////////////////////////////////////////////////////////////

#ifndef RadxEvent_HH
#define RadxEvent_HH

#include <string>
#include <Radx/Radx.hh>
class RadxRay;
using namespace std;

///////////////////////////////////////////////////////////////
/// EVENT NOTICE CLASS
///
/// This class holds data on events that occur during scan:
///
///   start of sweep
///   end   of sweep
///   start of volume
///   end   of volume
///
/// It also stores information about the scan when the event occurred,
/// for example the sweep mode, fixed angle.

class RadxEvent {

public:

  /// Constructor.
  
  RadxEvent();

  /// Copy constructor.
  
  RadxEvent(const RadxEvent &rhs);

  /// Destructor.
  
  virtual ~RadxEvent();
    
  /// Assignment.
  
  RadxEvent& operator=(const RadxEvent &rhs);
  
  /////////////////////////////////////////////////////////////
  /// \name Set methods:
  //@{

  /// Set the time for this event, in seconds and nanoseconds.

  inline void setTime(time_t secs, double nanoSecs) {
    _timeSecs = secs;
    _nanoSecs = nanoSecs;
  }
  
  /// Set the time for this event, given time as a double

  inline void setTime(double secs) {
    _timeSecs = (time_t ) secs;
    _nanoSecs = (secs - _timeSecs) * 1.0e9;
  }

  /// Set the flags

  inline void setStartOfSweep(bool state) { _startOfSweep = state; }
  inline void setEndOfSweep(bool state) { _endOfSweep = state; }
  inline void setStartOfVolume(bool state) { _startOfVolume = state; }
  inline void setEndOfVolume(bool state) { _endOfVolume = state; }

  /// Set the sweep mode which was active when this event was gathered.

  inline void setSweepMode(Radx::SweepMode_t val) {
    _sweepMode = val;
  }

  /// Set the follow mode which was active when this event was
  /// gathered.

  inline void setFollowMode(Radx::FollowMode_t val) {
    _followMode = val;
  }

  /// Set the volume number.
  
  inline void setVolumeNumber(int val) { _volumeNum = val; }

  /// Set the sweep number.

  inline void setSweepNumber(int val) { _sweepNum = val; }

  /// Set the event cause

  inline void setCause(Radx::EventCause_t val) { _cause = val; }

  /// Set the current sweep fixed angle for this event, in degrees. The fixed
  /// angle is the target angle for the sweep that contains this
  /// event. For RHI scans the fixed angle will be an azimuth angle. For
  /// all other scans this is an elevation angle.
  
  inline void setCurrentFixedAngleDeg(double val) { _currentFixedAngle = val; }

  /// set events from ray flags
  
  void setFromRayFlags(const RadxRay &ray);
  
  /// clear the event flags

  void clearFlags();
  
  //@}

  /////////////////////////////////////////////////////////////
  /// \name Get methods:
  //@{
  
  /// Get the time for this event, in seconds UTC since Jan 1 1970.
  ///
  /// Combine with getNanoSecs() for high-precision time.
  ///
  /// \code
  /// double time = (time in secs) + (nano seconds) / 1.0e9.
  /// \endcode
  
  inline time_t getTimeSecs() const { return _timeSecs; }

  /// Get the time for this event in nanoseconds.
  ///
  /// Combine with getTimeSecs() for high-precision time.
  ///
  /// \code
  /// double time = (time in secs) + (nano seconds) / 1.0e9.
  /// \endcode

  inline double getNanoSecs() const { return _nanoSecs; }

  /// Get the time for this event, as a double,
  /// in seconds UTC since Jan 1 1970.
  ///
  
  inline double getTimeDouble() const { 
    return ((double) _timeSecs + _nanoSecs / 1.0e9);
  }

  /// get the flags
  
  inline bool getStartOfSweep() const { return _startOfSweep; }
  inline bool getEndOfSweep() const { return _endOfSweep; }
  inline bool getStartOfVolume() const { return _startOfVolume; }
  inline bool getEndOfVolume() const { return _endOfVolume; }

  /// Get the sweep mode for this event.

  inline Radx::SweepMode_t getSweepMode() const { return _sweepMode; }

  /// Get the follow mode for this event.

  inline Radx::FollowMode_t getFollowMode() const { return _followMode; }

  /// Get the volume number for this event.

  inline int getVolumeNumber() const { return _volumeNum; }

  /// Get the sweep number for this event.

  inline int getSweepNumber() const { return _sweepNum; }

  /// Get the event cause for this event.

  inline Radx::EventCause_t getCause() const { return _cause; }
  
  /// Get the current fixed angle for this event, in degrees.

  inline double getCurrentFixedAngleDeg() const { return _currentFixedAngle; }

  //@}

  ///////////////////////////////////////////////
  /// \name Printing:
  //@{
  
  /// Print event metadata.
  
  void print(ostream &out) const;
 
  //@}

protected:
private:

  // data
  
  time_t _timeSecs;
  double _nanoSecs;

  bool _startOfSweep;
  bool _endOfSweep;
  
  bool _startOfVolume;
  bool _endOfVolume;
  
  Radx::SweepMode_t _sweepMode;
  Radx::FollowMode_t _followMode;
  
  int _volumeNum;
  int _sweepNum;

  Radx::EventCause_t _cause;

  double _currentFixedAngle;  // deg

  // methods
  
  void _init();
  RadxEvent & _copy(const RadxEvent &rhs);
  
};

#endif

