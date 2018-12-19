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
// RadxGeoref.hh
//
// Correction factors for Radx data
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2010
//
///////////////////////////////////////////////////////////////

#ifndef RadxGeoref_HH
#define RadxGeoref_HH

#include <iostream>
#include <Radx/RadxMsg.hh>
class RadxTime;
using namespace std;

/////////////////Ra//////////////////////////////////////////////
/// PLATFORM GEOREFERENCE CLASS
///
/// For moving platforms, a RadxGeoref object is attached to each
/// RadxRay object.
///
/// This object provides the full-motion georeference information for
/// the platform in 3-D space.
///
/// This georeference information is used to convert the
/// radar-measured ray angles to azimuth and elevation in
/// earth-centric coordinates.
///
/// NOTE: the default copy constructor and assignment methods work for
/// objects in this class.

class RadxGeoref {

public:

  /// Constructor
  
  RadxGeoref();
  
  /// Destructor

  ~RadxGeoref();

  /// Clear the data in the object - set all to missing
  
  void clear();

  /// \name Set methods:
  //@{

  void setRadxTime(const RadxTime &rtime);
  inline void setTimeSecs(time_t val) { _timeSecs = val; }
  inline void setNanoSecs(double val) { _nanoSecs = val; }
  inline void setUnitNum(Radx::si64 val) { _unitNum = val; }
  inline void setUnitId(Radx::si64 val) { _unitId = val; }
  inline void setLongitude(double val) { _longitude = val; }
  inline void setLatitude(double val) { _latitude = val; }
  inline void setAltitudeKmMsl(double val) { _altitudeKmMsl = val; }
  inline void setAltitudeKmAgl(double val) { _altitudeKmAgl = val; }
  inline void setEwVelocity(double val) { _ewVelocity = val; }
  inline void setNsVelocity(double val) { _nsVelocity = val; }
  inline void setVertVelocity(double val) { _vertVelocity = val; }
  inline void setHeading(double val) { _heading = val; }
  inline void setTrack(double val) { _track = val; }
  inline void setRoll(double val) { _roll = val; }
  inline void setPitch(double val) { _pitch = val; }
  inline void setDrift(double val) { _drift = val; }
  inline void setRotation(double val) { _rotation = val; }
  inline void setTilt(double val) { _tilt = val; }
  inline void setEwWind(double val) { _ewWind = val; }
  inline void setNsWind(double val) { _nsWind = val; }
  inline void setVertWind(double val) { _vertWind = val; }
  inline void setHeadingRate(double val) { _headingRate = val; }
  inline void setPitchRate(double val) { _pitchRate = val; }
  inline void setRollRate(double val) { _rollRate = val; }
  inline void setDriveAngle1(double val) { _driveAngle1 = val; }
  inline void setDriveAngle2(double val) { _driveAngle2 = val; }

  //@}
  /// \name Get methods:
  //@{

  RadxTime getRadxTime() const;
  inline time_t getTimeSecs() const { return _timeSecs; }
  inline double getNanoSecs() const { return _nanoSecs; }
  inline Radx::si64 getUnitNum() const { return _unitNum; }
  inline Radx::si64 getUnitId() const { return _unitId; }
  inline double getLongitude() const { return _longitude; }
  inline double getLatitude() const { return _latitude; }
  inline double getAltitudeKmMsl() const { return _altitudeKmMsl; }
  inline double getAltitudeKmAgl() const { return _altitudeKmAgl; }
  inline double getEwVelocity() const { return _ewVelocity; }
  inline double getNsVelocity() const { return _nsVelocity; }
  inline double getVertVelocity() const { return _vertVelocity; }
  inline double getHeading() const { return _heading; }
  inline double getTrack() const { return _track; }
  inline double getRoll() const { return _roll; }
  inline double getPitch() const { return _pitch; }
  inline double getDrift() const { return _drift; }
  inline double getRotation() const { return _rotation; }
  inline double getTilt() const { return _tilt; }
  inline double getEwWind() const { return _ewWind; }
  inline double getNsWind() const { return _nsWind; }
  inline double getVertWind() const { return _vertWind; }
  inline double getHeadingRate() const { return _headingRate; }
  inline double getPitchRate() const { return _pitchRate; }
  inline double getRollRate() const { return _rollRate; }
  inline double getDriveAngle1() const { return _driveAngle1; }
  inline double getDriveAngle2() const { return _driveAngle2; }

  //@}

  /// set all elements to 0

  void setToZero();

  /// increment count if elements are not missing
  ///
  /// Goes through the object looking for non-missing elements and
  /// increments the count accordingly
  
  void incrementIfNotMissing(RadxGeoref &count);
  
  /// Print the georeference values.
  
  void print(ostream &out) const;
  
  /// convert to XML

  void convert2Xml(string &xml, int level = 0) const;
  
  /// \name Serialization:
  //@{

  // serialize into a RadxMsg
  
  void serialize(RadxMsg &msg);
  
  // deserialize from a RadxMsg
  // return 0 on success, -1 on failure

  int deserialize(const RadxMsg &msg);

  //@}

protected:
private:

  // data
  
  time_t _timeSecs;
  double _nanoSecs;

  Radx::si64 _unitNum;
  Radx::si64 _unitId;
  
  double _longitude;	 /* Antenna longitude (Eastern
                          * Hemisphere is positive, West
                          * negative) in degrees */
  double _latitude;	 /* Antenna Latitude (Northern
                          * Hemisphere is positive, South
                          * Negative) in degrees */
  double _altitudeKmMsl; /* Antenna Altitude above mean sea
                          * level (MSL) in m */
  double _altitudeKmAgl; /* Antenna Altitude above ground level
                          * (AGL) in m */
  double _ewVelocity;	 /* Antenna east-west ground speed
                          * (towards East is positive) in m/sec */
  double _nsVelocity;	 /* Antenna north-south ground speed
                          * (towards North is positive) in m/sec */
  double _vertVelocity;	 /* Antenna vertical velocity (Up is
                          * positive) in m/sec */
  double _heading;	 /* Antenna heading (angle between
                          * rotodome rotational axis and true
                          * North, clockwise (looking down)
                          * positive) in degrees */
  double _track;	 /* Platform track over the ground (TN)
                          * positive) in degrees */
  double _roll;		 /* R oll angle of aircraft tail section
                          * (Horizontal zero, Positive left wing up)
                          * in degrees */
  double _pitch;	 /* Pitch angle of rotodome (Horizontal
                          * is zero positive front up) in degrees*/
  double _drift;	 /* Antenna drift Angle. (angle between
                          * platform true velocity and heading,
                          * positive is drift more clockwise
                          * looking down) in degrees */
  double _rotation;      /* Angle of the radar beam with
                          * respect to the airframe (zero is
                          * along vertical stabilizer, positive
                          * is clockwise) in deg */
  double _tilt;		 /* Angle of radar beam and line normal
                          * to longitudinal axis of aircraft,
                          * positive is towards nose of
                          * aircraft) in degrees */
  double _ewWind;	 /* east - west wind velocity at the
                          * platform (towards East is positive)
                          * in m/sec */
  double _nsWind;	 /* North - South wind velocity at the
                          * platform (towards North is 
                          * positive) in m/sec */
  double _vertWind;	 /* Vertical wind velocity at the
                          * platform (up is positive) in m/sec */
  double _headingRate;   /* Heading rate rate in degrees/second. */
  double _pitchRate;	 /* Pitch rate rate in degrees/second. */
  double _rollRate;	 /* Roll rate rate in degrees/second. */

  double _driveAngle1;   /* angle of antenna drive 1, if applicable */
  double _driveAngle2;   /* angle of antenna drive 2, if applicable */

  // methods
  
  void _init();

  /////////////////////////////////////////////////
  // serialization
  /////////////////////////////////////////////////

  static const int _metaNumbersPartId = 2;
  
  // struct for metadata numbers in messages
  // strings not included - they are passed as XML
  
  typedef struct {
    
    Radx::si64 timeSecs;
    Radx::si64 nanoSecs;
    Radx::si64 unitNum;
    Radx::si64 unitId;
    Radx::fl64 longitude;
    Radx::fl64 latitude;
    Radx::fl64 altitudeKmMsl;
    Radx::fl64 altitudeKmAgl;
    Radx::fl64 ewVelocity;
    Radx::fl64 nsVelocity;
    Radx::fl64 vertVelocity;
    Radx::fl64 heading;
    Radx::fl64 track;
    Radx::fl64 roll;
    Radx::fl64 pitch;
    Radx::fl64 drift;
    Radx::fl64 rotation;
    Radx::fl64 tilt;
    Radx::fl64 ewWind;
    Radx::fl64 nsWind;
    Radx::fl64 vertWind;
    Radx::fl64 headingRate;
    Radx::fl64 pitchRate;
    Radx::fl64 rollRate;
    Radx::fl64 driveAngle1;
    Radx::fl64 driveAngle2;
    Radx::fl64 spareFl64[8];
  
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

