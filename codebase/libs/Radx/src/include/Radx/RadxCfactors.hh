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
// RadxCfactors.hh
//
// Correction factors for Radx data
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2010
//
///////////////////////////////////////////////////////////////

#ifndef RadxCfactors_HH
#define RadxCfactors_HH

#include <iostream>
#include <Radx/RadxMsg.hh>
using namespace std;

///////////////////////////////////////////////////////////////
/// CORRECTION FACTORS CLASS
///
/// The correction factors are scalars which apply to the entire
/// volume.
///
/// They are used to correct the values in a RadxGeoref object.
///
/// NOTE: the default copy constructor and assignment methods work for
/// objects in this class.

class RadxCfactors {

public:

  /// Constructor
  
  RadxCfactors();
  
  /// Destructor

  ~RadxCfactors();

  /// Clear the data in the object.
  
  void clear();

  /// \name Set methods:
  //@{

  inline void setAzimuthCorr(double val) { _azimuthCorr = val; }
  inline void setElevationCorr(double val) { _elevationCorr = val; }
  inline void setRangeCorr(double val) { _rangeCorr = val; }
  inline void setLongitudeCorr(double val) { _longitudeCorr = val; }
  inline void setLatitudeCorr(double val) { _latitudeCorr = val; }
  inline void setPressureAltCorr(double val) { _pressureAltCorr = val; }
  inline void setAltitudeCorr(double val) { _altitudeCorr = val; }
  inline void setEwVelCorr(double val) { _ewVelCorr = val; }
  inline void setNsVelCorr(double val) { _nsVelCorr = val; }
  inline void setVertVelCorr(double val) { _vertVelCorr = val; }
  inline void setHeadingCorr(double val) { _headingCorr = val; }
  inline void setRollCorr(double val) { _rollCorr = val; }
  inline void setPitchCorr(double val) { _pitchCorr = val; }
  inline void setDriftCorr(double val) { _driftCorr = val; }
  inline void setRotationCorr(double val) { _rotationCorr = val; }
  inline void setTiltCorr(double val) { _tiltCorr = val; }

  //@}
  /// \name Get methods:
  //@{

  inline double getAzimuthCorr() const { return _azimuthCorr; }
  inline double getElevationCorr() const { return _elevationCorr; }
  inline double getRangeCorr() const { return _rangeCorr; }
  inline double getLongitudeCorr() const { return _longitudeCorr; }
  inline double getLatitudeCorr() const { return _latitudeCorr; }
  inline double getPressureAltCorr() const { return _pressureAltCorr; }
  inline double getAltitudeCorr() const { return _altitudeCorr; }
  inline double getEwVelCorr() const { return _ewVelCorr; }
  inline double getNsVelCorr() const { return _nsVelCorr; }
  inline double getVertVelCorr() const { return _vertVelCorr; }
  inline double getHeadingCorr() const { return _headingCorr; }
  inline double getRollCorr() const { return _rollCorr; }
  inline double getPitchCorr() const { return _pitchCorr; }
  inline double getDriftCorr() const { return _driftCorr; }
  inline double getRotationCorr() const { return _rotationCorr; }
  inline double getTiltCorr() const { return _tiltCorr; }

  //@}

  /// Print the correction factors.
  
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
  
  double _azimuthCorr;     /* Correction added to azimuth[deg] */
  double _elevationCorr;   /* Correction added to elevation[deg] */
  double _rangeCorr;       /* Correction used for range [km] */
  double _longitudeCorr;   /* Correction added to radar longitude */
  double _latitudeCorr;    /* Correction added to radar latitude */
  double _pressureAltCorr; /* Correction added to pressure altitude
                            * (msl) [km] */
  double _altitudeCorr;    /* Correction added to radar altitude above
                            * ground level(agl) [km] */
  double _ewVelCorr;       /* Correction added to radar platform
                            * ground speed(E-W) [m/s] */
  double _nsVelCorr;       /* Correction added to radar platform
                            * ground speed(N-S) [m/s] */
  double _vertVelCorr;     /* Correction added to radar platform
                            * vertical velocity [m/s] */
  double _headingCorr;     /* Correction added to radar platform 
                            * heading [deg]) */
  double _rollCorr;        /* Correction added to radar platform roll  [deg] */
  double _pitchCorr;       /* Correction added to radar platform pitch [deg] */
  double _driftCorr;       /* Correction added to radar platform drift [deg] */
  double _rotationCorr;    /* Corrrection add to radar rotation angle  [deg] */
  double _tiltCorr;        /* Correction added to radar tilt angle [deg] */
  
  // methods
  
  void _init();

  /////////////////////////////////////////////////
  // serialization
  /////////////////////////////////////////////////

  static const int _metaNumbersPartId = 2;
  
  // struct for metadata numbers in messages
  // strings not included - they are passed as XML
  
  typedef struct {
    
    Radx::fl64 azimuthCorr;
    Radx::fl64 elevationCorr;
    Radx::fl64 rangeCorr;
    Radx::fl64 longitudeCorr;
    Radx::fl64 latitudeCorr;
    Radx::fl64 pressureAltCorr;
    Radx::fl64 altitudeCorr;
    Radx::fl64 ewVelCorr;
    Radx::fl64 nsVelCorr;
    Radx::fl64 vertVelCorr;
    Radx::fl64 headingCorr;
    Radx::fl64 rollCorr;
    Radx::fl64 pitchCorr;
    Radx::fl64 driftCorr;
    Radx::fl64 rotationCorr;
    Radx::fl64 tiltCorr;

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

