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
// RadxPlatform.hh
//
// RadxPlatform object
//
// Parameters for the platform
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2012
//
///////////////////////////////////////////////////////////////

#ifndef RadxPlatform_HH
#define RadxPlatform_HH

#include <string>
#include <vector>
#include <Radx/Radx.hh>
class RadxMsg;
using namespace std;

//////////////////////////////////////////////////////////////////////
/// CLASS FOR STORING RADAR/LIDAR PLATFORM PARAMETERS
/// 

class RadxPlatform {

public:

  /// Constructor
  
  RadxPlatform();
  
  /// Copy constructor
  
  RadxPlatform(const RadxPlatform &rhs);

  /// Destructor
  
  virtual ~RadxPlatform();

  /// Assignment
  
  RadxPlatform& operator=(const RadxPlatform &rhs);
  
  //////////////////////////////////////////////////////////////////
  /// \name Set methods:
  //@{

  /// Set the instrument name, if available.

  void setInstrumentName(const string &val);

  /// Set the site name, if available.

  void setSiteName(const string &val);

  /// Set the instrument type. Default is RADAR.

  inline void setInstrumentType(Radx::InstrumentType_t val) {
    _instrumentType = val;
  }

  /// Set the platform type. Default is FIXED.

  inline void setPlatformType(Radx::PlatformType_t val) {
    _platformType = val;
  }

  /// Set the primary rotation axis. Default is AXIS_Z.

  inline void setPrimaryAxis(Radx::PrimaryAxis_t val) {
    _primaryAxis = val;
  }

  /// Set the latitude of the platform in degrees.
  ///
  /// Used for non-mobile platforms.

  void setLatitudeDeg(double val) { _latitudeDeg = val; }

  /// Set the longitude of the platform in degrees.
  ///
  /// Used for non-mobile platforms.

  void setLongitudeDeg(double val) { _longitudeDeg = val; }

  /// Set the altitude of the platform in km.
  ///
  /// Used for non-mobile platforms.

  void setAltitudeKm(double val) { _altitudeKm = val; }

  /// Set the sensor ht above the surface

  void setSensorHtAglM(double val) { _sensorHtAglM = val; }

  /// Set up the list of frequencies, adding them one at a time.
  /// Normally there is only one frequency, but multiple are supported.
  
  /// The set methods clear the list first, and then add the value
  /// The add methods do not clear the list first
  
  void setFrequencyHz(double val);
  void setWavelengthM(double val);
  void setWavelengthCm(double val);
  
  void addFrequencyHz(double val);
  void addWavelengthM(double val);
  void addWavelengthCm(double val);
  
  /// Set the RADAR beam width, horizontal, in degrees.

  void setRadarBeamWidthDegH(double val);

  /// Set the RADAR beam width, vertical, in degrees.

  void setRadarBeamWidthDegV(double val);

  /// Set the RADAR antenna gain, horizontal, in dB.

  void setRadarAntennaGainDbH(double val);

  /// Set the RADAR antenna gain, vertical, in dB.

  void setRadarAntennaGainDbV(double val);

  /// Set the RADAR receiver bandwidth, in MHz.

  void setRadarReceiverBandwidthMhz(double val);

  /// Set the instrument type to LIDAR. Default is RADAR.

  void setIsLidar(bool val);

  /// Set the LIDAR constant.

  void setLidarConstant(double val);

  /// Set the LIDAR pulse energy, in Joules.

  void setLidarPulseEnergyJ(double val);

  /// Set the LIDAR peak power, in Watts.

  void setLidarPeakPowerW(double val);

  /// Set the LIDAR aperture diameter, in cm.

  void setLidarApertureDiamCm(double val);

  /// Set the LIDAR aperture efficiency, in percent.

  void setLidarApertureEfficiency(double val);

  /// Set the LIDAR field of view, in milli-radians.

  void setLidarFieldOfViewMrad(double val);
  
  /// Set the LIDAR beam divergence, in milli-radians.

  void setLidarBeamDivergenceMrad(double val);

  //@}

  //////////////////////////////////////////////////////////////////
  /// \name Get methods:
  //@{

  /// Get instrument name.

  inline const string &getInstrumentName() const { return _instrumentName; }

  /// Get site name.

  inline const string &getSiteName() const { return _siteName; }

  /// Get instrument type

  inline Radx::InstrumentType_t getInstrumentType() const {
    return _instrumentType;
  }

  /// Get platform type

  inline Radx::PlatformType_t getPlatformType() const {
    return _platformType;
  }

  /// Get primary axis

  inline Radx::PrimaryAxis_t getPrimaryAxis() const {
    return _primaryAxis;
  }

  /// Get latitude in degrees. Applies to FIXED platform.

  inline double getLatitudeDeg() const { return _latitudeDeg; }

  /// Get longitude in degrees. Applies to FIXED platform.

  inline double getLongitudeDeg() const { return _longitudeDeg; }

  /// Get altitude in km. Applies to FIXED platform.

  inline double getAltitudeKm() const { return _altitudeKm; }

  /// Get the sensor ht above the surface in meters

  inline double getSensorHtAglM() const { return _sensorHtAglM; }

  /// Get vector of frequencies - normally only 1 entry

  inline const vector<double> &getFrequencyHz() const { return _frequencyHz; }
  double getWavelengthM() const;
  double getWavelengthCm() const;

  /// For RADAR, get horizontal beam width, in degrees.

  inline double getRadarBeamWidthDegH() const { return _radarBeamWidthDegH; }

  /// For RADAR, get vertical beam width, in degrees.

  inline double getRadarBeamWidthDegV() const { return _radarBeamWidthDegV; }

  /// For RADAR, get horizontal antenna gain, in dB.

  inline double getRadarAntennaGainDbH() const { return _radarAntGainDbH; }

  /// For RADAR, get horizontal antrenna gain, in dB.

  inline double getRadarAntennaGainDbV() const { return _radarAntGainDbV; }

  /// For RADAR, get receiver band width, in Mhz.

  inline double getRadarReceiverBandwidthMhz() const {
    return _radarReceiverBandwidthMhz;
  }

  /// For LIDAR, get lidar constant.

  inline double getLidarConstant() const { return _lidarConstant; }

  /// For LIDAR, get pulse energy, in Joules.

  inline double getLidarPulseEnergyJ() const { return _lidarPulseEnergyJ; }

  /// For LIDAR, get peak power, in watts.

  inline double getLidarPeakPowerW() const { return _lidarPeakPowerW; }

  /// For LIDAR, get aperture diameter, in cm.

  inline double getLidarApertureDiamCm() const { return _lidarApertureDiamCm; }

  /// For LIDAR, get aperture efficiency, in percent.

  inline double getLidarApertureEfficiency() const {
    return _lidarApertureEfficiency;
  }

  /// For LIDAR, get field of view, in milli-radians.

  inline double getLidarFieldOfViewMrad() const { 
    return _lidarFieldOfViewMrad; 
  }

  /// For LIDAR, get beam divergence, in milli-radians.

  inline double getLidarBeamDivergenceMrad() const {
    return _lidarBeamDivergenceMrad;
  }

  //@}

  //////////////////////////////////////////////////////////////////
  /// \name Clearing data:
  //@{

  /// Clear all of the data in the object.
  
  void clear();
  
  /// Clear the frequency list.
  
  void clearFrequency();

  //@}

  //////////////////////////////////////////////////////////////////
  /// \name Printing:
  //@{

  /// Print metadata.
  
  void print(ostream &out) const;
  
  //@}

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

  // platform details

  string _instrumentName;
  string _siteName;
  Radx::InstrumentType_t _instrumentType;
  Radx::PlatformType_t _platformType;
  Radx::PrimaryAxis_t _primaryAxis;

  // platform location if fixed

  double _latitudeDeg;
  double _longitudeDeg;
  double _altitudeKm; // MSL of sensor
  double _sensorHtAglM; // sensor ht above surface in meters

  // Frequency list - normally there is only one entry

  vector<double> _frequencyHz;
  
  // radar parameters

  double _radarBeamWidthDegH;
  double _radarBeamWidthDegV;
  double _radarAntGainDbH;
  double _radarAntGainDbV;
  double _radarReceiverBandwidthMhz;
  
  // lidar parameters

  double _lidarConstant;
  double _lidarPulseEnergyJ; // Joules
  double _lidarPeakPowerW; // Watts
  double _lidarApertureDiamCm; // centimeters
  double _lidarApertureEfficiency; // percent
  double _lidarFieldOfViewMrad; // milliradians
  double _lidarBeamDivergenceMrad; // milliradians
  
  // private methods
  
  void _init();
  RadxPlatform & _copy(const RadxPlatform &rhs);

  /////////////////////////////////////////////////
  // serialization
  /////////////////////////////////////////////////
  
  static const int _metaStringsPartId = 1;
  static const int _metaNumbersPartId = 2;
  static const int _frequencyPartId = 3;
  
  // struct for metadata numbers in messages
  // strings not included - they are passed as XML
  
  typedef struct {
    
    Radx::fl64 latitudeDeg;
    Radx::fl64 longitudeDeg;
    Radx::fl64 altitudeKm;
    Radx::fl64 sensorHtAglM;
    
    Radx::fl64 radarBeamWidthDegH;
    Radx::fl64 radarBeamWidthDegV;
    Radx::fl64 radarAntGainDbH;
    Radx::fl64 radarAntGainDbV;
    Radx::fl64 radarReceiverBandwidthMhz;

    Radx::fl64 lidarConstant;
    Radx::fl64 lidarPulseEnergyJ;
    Radx::fl64 lidarPeakPowerW;
    Radx::fl64 lidarApertureDiamCm;
    Radx::fl64 lidarApertureEfficiency;
    Radx::fl64 lidarFieldOfViewMrad;
    Radx::fl64 lidarBeamDivergenceMrad;

    Radx::fl64 spareFl64[8];

    Radx::si32 instrumentType;
    Radx::si32 platformType;
    Radx::si32 primaryAxis;

    Radx::si32 spareSi64[13];
    
  } msgMetaNumbers_t;
  
  msgMetaNumbers_t _metaNumbers;
    
  /// convert metadata to XML
  
  void _loadMetaStringsToXml(string &xml, int level = 0) const;
  
  /// set metadata from XML
  /// returns 0 on success, -1 on failure
  
  int _setMetaStringsFromXml(const char *xml, 
                             size_t bufLen);
  
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
