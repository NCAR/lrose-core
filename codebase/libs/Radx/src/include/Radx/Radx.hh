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
// Radx.hh
//
// Definitions for Radx
//
// NetCDF file data for radar radial data in CF-compliant file
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2009
//
///////////////////////////////////////////////////////////////

#ifndef Radx_HH
#define Radx_HH

#include <string>
#include <sys/types.h>
using namespace std;

//////////////////////////////////////////////////////////////////////
/// \mainpage RADX C++ LIBRARY
///
/// The Radx library facilitates the reading, writing and manipulation
/// of radial-coordinate RADAR and LIDAR data.
/// 
/// .
///
/// \section formats File formats
///
/// The following file formats are supported:
///
/// - NetCDF CF/Radial
/// - NCAR/EOL DORADE
/// - NCAR/EOL FORAY
/// - Universal Format (UF)
/// - NEXRAD archive level 2
/// - SIGMET (vaisala) raw volume (read only)
/// - Leosphere lidar raw volume (read only)
/// 
/// \section classes Main classes
///
/// RadxVol is the primary class for storing data from RADAR and LIDAR
/// volumes.
///
/// A volume is made up of a number of sweeps (RadxSweep), each of
/// which contain a number of rays (RadxRay).
///
/// The information held by the volume object is references in a
/// number of ways. There are 3 primary vectors on a volume, and
/// all are inter-related:
///
///  - rays
///  - sweeps
///  - fields 
///
/// A volume may contain one or more sweeps.
///
/// The RAYS are stored as a vector of RadxRay objects. Each RadxRay
/// owns a vector of RadxField objects - see RadxRay for more information.
///
/// A SWEEP is formed from a sequence of RAYS. RadxSweep is a
/// light-weight object which merely keeps information about how the
/// rays are grouped into sweeps. The sweep objects do not hold data,
/// they just keep track of the ray indices, and sweep properties.
///
/// \section fields Memory management for fields
///
/// Each RadxVol contains a vector of RadxField objects. After a
/// volume is read, these fields hold contiguous arrays for the field
/// data.
///
/// If, on the other hand, the ray data is being read in on a
/// ray-by-ray basis, the RadxField objects on the rays will own their
/// data locally, as these rays are added to the volume. The fields on
/// the volume will not yet exist while this is occurring. Once all of
/// the rays have been added, a call to the loadFieldsFromRays()
/// method will create fields on the volume, copy the data from the
/// rays into contiguous arrays on these new fields, and then modify
/// the ray fields so that they point to the volume fields rather than
/// manage their own data.
///
/// The RadxVol class, and associated classes, holds data in memory
/// in a generic manner, unrelated to specific file formats.
///
/// \section file_io Classes for file I/O
///
/// The following classes are used for reading and writing RadxVol
/// data:
///
/// - RadxFile: base class
/// - DoradeRadxFile: DORADE files
/// - ForayNcRadxFile: FORAY files
/// - NcfRadxFile: NetCDF CfRadial files
/// - NexradRadxFile: NEXRAD message 1 and 31 files
/// - SigmetRadxFile: SIGMET RAW files
/// - UfRadxFile: UF files
///
/// \section Code_reading Code example - reading
///
/// The following code snippet illustrates how you might read a
/// volume from a file, and print it.
///
/// Note: this code will not compile, it is meant for illustration only.
///
/// \code
///
///   string filePath = "/tmp/test.nc";
///   RadxFile file;
///   RadxVol vol;
///   if (file.readFromPath(filePath, vol)) {
///     cerr << "ERROR - reading file: " << filePath << endl;
///     cerr << file.getErrStr() << endl;
///     return -1;
///   }
/// 
///   if (print_data) {
///     vol.printWithFieldData(cout);
///   } else if (print_rays) {
///     vol.printWithRayMetaData(cout);
///   } else {
///     vol.printWithMetaData(cout);
///   }
/// 
/// \endcode
///
/// \section Code_writing Code example - writing
///
/// The following code snippet illustrates how you might take
/// ray-by-ray data, store it in a volume, and then write it out.
///
/// Note: this code will not compile, it is meant for illustration only.
///
/// \code
///
///   RadxVol vol;
///   while (rays_available) {
///     RadxRay *ray = new RadxRay();
///     // Set ray properties
///     ray->setTime(time);
///     ray->setVolumeNumber(volNum);
///     ray->setSweepNumber(sweepNum);
///     // etc
///     // Add ray to volume
///     // add field data to rays
///     Radx::fl32 *fdata = getDataMethod(); // whatever makes sense here
///     for (int ifield = 0; ifield < nFields; ifield++) {
///       // add field to ray, ray fields managed data array
///       RadxField *fld = ray->addField(name, units,
///                                      ngates, missingVal,
///                                      data, true);
///       // set other field properties
///       fld->setLongName(longname);
///       fld->setStandardName(standardName);
///       // possibly convert data type
///       ray->getField(name)->convertToSi16();
///    } // ifield
///    // add ray to volume
///    vol.addRay(ray);
///  } // while
///  // load up volume fields from ray data
///  // this changes field data array ownership from ray fields to vol fields
///  vol.loadFieldsFromRays(true);
///  // set volume properties
///  vol.setTitle("Test");
///  vol.setStartTime(startSecs, startNano);
///  vol.setEndTime(endSecs, endNano);
///  vol.setLatitudeDeg(latitude);
///  // etc.
///  //
///  // write out as Dorade
///  DoradeRadxFile file;
///  string dir = "/tmp/dorade";
///  if (file.writeToDir(vol, dir, true, false)) {
///     cerr << "ERROR - writing file to dir: " << dir << endl;
///     cerr << file.getErrStr() << endl;
///     return -1;
///  }
/// 
/// \endcode


///////////////////////////////////////////////////////////////
/// CLASS Radx
///
/// Utility class, type definitions.

class Radx {

public:

  ///////////////
  // enumerations

  /// data encoding type

  typedef enum {

    SI08, ///< signed 8-bit int
    SI16, ///< signed 16-bit int
    SI32, ///< signed 32-bit int
    UI08, ///< unsigned 8-bit int
    UI16, ///< unsigned 16-bit int
    UI32, ///< unsigned 32-bit int
    FL32, ///< 32-bit IEEE float
    FL64, ///< 64-bit IEEE float
    ASIS  ///< leave it as it is
    
  } DataType_t;

  // portable data types
  
  typedef char si08; ///< portable unsigned 8-bit integer
  typedef unsigned char ui08; ///< portable unsigned 8-bit integer
  typedef int16_t si16; ///< portable signed 16-bit integer
  typedef u_int16_t ui16; ///< portable unsigned 16-bit integer
  typedef int32_t si32; ///< portable signed 32-bit integer
  typedef u_int32_t ui32; ///< portable unsigned 32-bit integer
  typedef int64_t si64; ///< portable signed 32-bit integer
  typedef u_int64_t ui64; ///< portable unsigned 32-bit integer
  typedef float fl32; ///< portable 32-bit IEEE float
  typedef double fl64; ///< portable 64-bit IEEE float

  /// instrument type

  typedef enum {

    INSTRUMENT_TYPE_RADAR = 0, ///< all RADARs
    INSTRUMENT_TYPE_LIDAR = 1, ///< all LIDARs
    INSTRUMENT_TYPE_LAST
    
  } InstrumentType_t;
  
  /// platform type

  typedef enum {

    PLATFORM_TYPE_NOT_SET = 0, ///< Initialized but not yet set
    PLATFORM_TYPE_FIXED = 1, ///< Radar is in a fixed location
    PLATFORM_TYPE_VEHICLE = 2, ///< Radar is mounted on a land vehicle
    PLATFORM_TYPE_SHIP = 3, ///< Radar is mounted on a ship
    PLATFORM_TYPE_AIRCRAFT = 4, ///< Foreward looking on aircraft
    PLATFORM_TYPE_AIRCRAFT_FORE = 5, ///< Foreward looking on aircraft
    PLATFORM_TYPE_AIRCRAFT_AFT = 6, ///< Backward looking on aircraft
    PLATFORM_TYPE_AIRCRAFT_TAIL = 7, ///< Tail - e.g. ELDORA
    PLATFORM_TYPE_AIRCRAFT_BELLY = 8, ///< Belly radar on aircraft
    PLATFORM_TYPE_AIRCRAFT_ROOF = 9, ///< Roof radar on aircraft
    PLATFORM_TYPE_AIRCRAFT_NOSE = 10, ///< radar in nose radome on aircraft
    PLATFORM_TYPE_SATELLITE_ORBIT = 11, ///< orbiting satellite
    PLATFORM_TYPE_SATELLITE_GEOSTAT = 12, ///< geostationary satellite
    PLATFORM_TYPE_LAST
    
  } PlatformType_t;
  
  /// primary axis of rotation
  /// Z/Y/Z is a right handed coordinate system, where
  ///  Z: vertical
  ///  Y: longitudinal axis of platform (e.g. forward along aircraft fuselage)
  ///  X: lateral axis of platform (e.g. along aircraft right wing)

  typedef enum {

    PRIMARY_AXIS_Z = 0, ///< vertical
    PRIMARY_AXIS_Y = 1, ///< longitudinal axis of platform
    PRIMARY_AXIS_X = 2, ///< lateral axis of platform
    PRIMARY_AXIS_Z_PRIME = 3, ///< inverted vertical
    PRIMARY_AXIS_Y_PRIME = 4, ///< ELDORA, HRD tail
    PRIMARY_AXIS_X_PRIME = 5  ///< translated lateral
    
  } PrimaryAxis_t;
  
  /// sweep mode

  typedef enum {

    SWEEP_MODE_NOT_SET = -1, ///< Initialized but not yet set
    SWEEP_MODE_CALIBRATION = 0, ///< pointing for calibration
    SWEEP_MODE_SECTOR = 1,     ///< sector scan mode
    SWEEP_MODE_COPLANE = 2,    ///< co-plane dual doppler mode
    SWEEP_MODE_RHI = 3,        ///< range height vertical scanning mode
    SWEEP_MODE_VERTICAL_POINTING = 4, ///< vertical pointing for calibration
    SWEEP_MODE_IDLE = 7,       ///< between scans
    SWEEP_MODE_AZIMUTH_SURVEILLANCE = 8, /**< 360-degree azimuth mode,
                                          * (surveillance) */
    SWEEP_MODE_ELEVATION_SURVEILLANCE = 9, /**< 360-degree elevation
                                            * mode (Eldora) */
    SWEEP_MODE_SUNSCAN = 11,   ///< scanning the sun for calibrations
    SWEEP_MODE_POINTING = 12,  ///< fixed pointing
    SWEEP_MODE_FOLLOW_VEHICLE = 13, ///< follow target vehicle
    SWEEP_MODE_EL_SURV = 14, ///< elevation surveillance (ELDORA)
    SWEEP_MODE_MANUAL_PPI = 15, /**< Manual PPI mode - elevation does
                                 * not step automatically */
    SWEEP_MODE_MANUAL_RHI = 16, /**< Manual RHI mode - azimuth does
                                 * not step automatically */
    SWEEP_MODE_SUNSCAN_RHI = 17,  ///< scanning the sun in RHI mode
    SWEEP_MODE_DOPPLER_BEAM_SWINGING = 18, ///< as in profiler or lidar
    SWEEP_MODE_COMPLEX_TRAJECTORY = 19,  ///< any sequential angle sequence
    SWEEP_MODE_ELECTRONIC_STEERING = 20,  ///< as in phased array
    SWEEP_MODE_LAST

  } SweepMode_t;

  /// is the instrument following an object?

  typedef enum {

    FOLLOW_MODE_NOT_SET = 0, ///< Initialized but not yet set
    FOLLOW_MODE_NONE = 1, /**< Radar is not tracking any object */
    FOLLOW_MODE_SUN = 2, /**< Radar is tracking the sun */
    FOLLOW_MODE_VEHICLE = 3, /**< Radar is tracking a vehicle */
    FOLLOW_MODE_AIRCRAFT = 4, /**< Radar is tracking an aircraft */
    FOLLOW_MODE_TARGET = 5, /**< Radar is tracking a target - e.g. sphere */
    FOLLOW_MODE_MANUAL = 6, /**< Radar is under manual tracking mode */
    FOLLOW_MODE_LAST
    
  } FollowMode_t;

  /// polarization mode

  typedef enum {
    
    POL_MODE_NOT_SET = 0, ///< Initialized but not yet set
    POL_MODE_HORIZONTAL = 1, /**< Horizontal */
    POL_MODE_VERTICAL = 2, /**< Vertical */
    POL_MODE_HV_ALT = 3, /**< Fast alternating */
    POL_MODE_HV_SIM = 4, /**< Simultaneous Slant-45 */
    POL_MODE_CIRCULAR = 5, /**< Circular */
    POL_MODE_HV_H_XMIT = 6, /**< transmit H, receive both */
    POL_MODE_HV_V_XMIT = 7, /**< transmit V, receive both */
    POL_MODE_LAST
    
  } PolarizationMode_t;

  /// pulsing mode

  typedef enum {
    
    PRT_MODE_NOT_SET = 0, ///< Initialized but not yet set
    PRT_MODE_FIXED = 1, /**< fixed pulsing mode */
    PRT_MODE_STAGGERED = 2, /**< staggered PRT - alternate pulses */
    PRT_MODE_DUAL = 3, /**< dual PRT, alternate rays */
    PRT_MODE_LAST
    
  } PrtMode_t;

  /// event cause

  typedef enum {
    
    EVENT_CAUSE_NOT_SET = 0,
    EVENT_CAUSE_DONE = 1, /**< Scan completed normally */
    EVENT_CAUSE_TIMEOUT = 2, /**< Scan has timed out */
    EVENT_CAUSE_TIMER = 3, /**< Timer caused this scan to abort */
    EVENT_CAUSE_ABORT = 4, /**< Operator issued an abort */
    EVENT_CAUSE_SCAN_ABORT = 5, /**< Scan Controller detected error */
    EVENT_CAUSE_RESTART = 6, /**< communication fault was recovered,
                                  *   restarting scan */
    EVENT_CAUSE_SCAN_STATE_TIMEOUT = 7, /**< Scan Controller state machine timeout */
    EVENT_CAUSE_LAST /**< not used */

  } EventCause_t;

  /// \name Missing values for enums, by type:
  //@{

  static const InstrumentType_t missingInstrumentType;
  static const PlatformType_t missingPlatformType;
  static const SweepMode_t missingSweepMode;
  static const FollowMode_t missingFollowMode;
  static const PolarizationMode_t missingPolarizationMode;
  static const PrtMode_t missingPrtMode;
  static const EventCause_t missingEventCause;

  //@}

  /// \name Missing values for metadata:
  //@{

  static double missingMetaDouble;
  static float missingMetaFloat;
  static int missingMetaInt;
  static char missingMetaChar;

  //@}

  /// \name Missing values for field data, by type:
  //@{

  static fl64 missingFl64;
  static fl32 missingFl32;
  static si32 missingSi32;
  static si16 missingSi16;
  static si08 missingSi08;
  
  //@}

  /// speed of light

  static const double LIGHT_SPEED;

  /// angle conversion

  static const double DegToRad;
  static const double RadToDeg;

  /// \name Standard strings for use in files:
  //@{

  const static char* AIRCRAFT;
  const static char* AIRCRAFT_AFT;
  const static char* AIRCRAFT_BELLY;
  const static char* AIRCRAFT_FORE;
  const static char* AIRCRAFT_NOSE;
  const static char* AIRCRAFT_ROOF;
  const static char* AIRCRAFT_TAIL;
  const static char* AXIS_X;
  const static char* AXIS_X_PRIME;
  const static char* AXIS_Y;
  const static char* AXIS_Y_PRIME;
  const static char* AXIS_Z;
  const static char* AXIS_Z_PRIME;
  const static char* AZIMUTH_SURVEILLANCE;
  const static char* CALIBRATION;
  const static char* CIRCULAR;
  const static char* COMPLEX_TRAJECTORY;
  const static char* COPLANE;
  const static char* DOPPLER_BEAM_SWINGING;
  const static char* DUAL;
  const static char* ELECTRONIC_STEERING;
  const static char* ELEVATION_SURVEILLANCE;
  const static char* FIXED;
  const static char* HORIZONTAL;
  const static char* HV_ALT;
  const static char* HV_SIM;
  const static char* HV_H_XMIT;
  const static char* HV_V_XMIT;
  const static char* IDLE;
  const static char* LIDAR;
  const static char* MANUAL;
  const static char* MANUAL_PPI;
  const static char* MANUAL_RHI;
  const static char* NONE;
  const static char* NOT_SET;
  const static char* POINTING;
  const static char* RADAR;
  const static char* RHI;
  const static char* SATELLITE_GEOSTAT;
  const static char* SATELLITE_ORBIT;
  const static char* SECTOR;
  const static char* SHIP;
  const static char* STAGGERED;
  const static char* SUN;
  const static char* SUNSCAN;
  const static char* SUNSCAN_RHI;
  const static char* TARGET;
  const static char* UNKNOWN;
  const static char* VEHICLE;
  const static char* VERTICAL;
  const static char* VERTICAL_POINTING;

  const static char* EVENT_DONE;
  const static char* EVENT_TIMEOUT;
  const static char* EVENT_TIMER;
  const static char* EVENT_ABORT;
  const static char* EVENT_SCAN_ABORT;
  const static char* EVENT_RESTART;
  const static char* EVENT_SCAN_STATE_TIMEOUT;

  //@}

  /// Get byte width for given data type.

  static int getByteWidth(DataType_t dtype);

  /// \name Convert enums to strings.
  //@{

  static string dataTypeToStr(DataType_t dtype);
  static string instrumentTypeToStr(InstrumentType_t itype);
  static string platformTypeToStr(PlatformType_t ptype);
  static string sweepModeToStr(SweepMode_t mode);
  static string sweepModeToShortStr(SweepMode_t mode);
  static string followModeToStr(FollowMode_t mode);
  static string polarizationModeToStr(PolarizationMode_t mode);
  static string prtModeToStr(PrtMode_t mode);
  static string primaryAxisToStr(PrimaryAxis_t ptype);
  static string eventCauseToStr(EventCause_t cause);

  //@}
  /// \name Convert strings to enums.
  //@{

  static DataType_t dataTypeFromStr(const string &str);
  static InstrumentType_t instrumentTypeFromStr(const string &str);
  static PlatformType_t platformTypeFromStr(const string &str);
  static SweepMode_t sweepModeFromStr(const string &str);
  static FollowMode_t followModeFromStr(const string &str);
  static PolarizationMode_t polarizationModeFromStr(const string &str);
  static PrtMode_t prtModeFromStr(const string &str);
  static PrimaryAxis_t primaryAxisFromStr(const string &str);
  static EventCause_t eventCauseFromStr(const string &str);

  //@}
  /// \name Get options string for enums.
  //@{

  static string instrumentTypeOptions();
  static string platformTypeOptions();
  static string sweepModeOptions();
  static string followModeOptions();
  static string polarizationModeOptions();
  static string prtModeOptions();
  static string primaryAxisOptions();
  static string eventCauseOptions();

  //@}

  /// Add integer to error string.

  static void addErrInt(string &errStr,
                        string label, int iarg,
                        bool cr = true);

  /// Add double to error string.

  static void addErrDbl(string &errStr,
                        string label, double darg,
                        string format, bool cr = true);

  /// Add string to error string.

  static void addErrStr(string &errStr,
                        string label, string strarg = "",
                        bool cr = true);

  /// Safely make string from char text.
  /// Ensure null termination.
  
  static string makeString(const char *text, int len);
  
  /// Safely print char text.
  /// Ensure null termination.
  
  static void printString(const string &label, const char *text,
                          int len, ostream &out);

  /// replace spaces in a string with underscores

  static void replaceSpacesWithUnderscores(string &str);

  /// compute sin and cos together

  static void sincos(double radians, double &sinVal, double &cosVal);

  /// condition az angle, to between 0 and 360

  static double conditionAz(double az);

  /// condition el to between -180 and 180
  
  static double conditionEl(double el);

  /// condition angle delta to between -180 and 180
  
  static double conditionAngleDelta(double delta);

  /// compute diff between 2 angles: (ang1 - ang2)
  
  static double computeAngleDiff(double ang1, double ang2);
  
  /// compute mean of 2 angles - ang1 + ((ang2 - ang1)/2)

  static double computeAngleMean(double ang1, double ang2);

  /// \name set missing values
  //@{

  static void setMissingMetaDouble(double val) { missingMetaDouble = val; }
  static void setMissingMetaFloat(float val) { missingMetaFloat = val; }
  static void setMissingMetaInt(int val) { missingMetaInt = val; }
  static void setMissingMetaChar(char val) { missingMetaChar = val; }
  static void setMissingFl64(fl64 val) { missingFl64 = val; }
  static void setMissingFl32(fl32 val) { missingFl32 = val; }
  static void setMissingSi32(si32 val) { missingSi32 = val; }
  static void setMissingSi16(si16 val) { missingSi16 = val; }
  static void setMissingSi08(si08 val) { missingSi08 = val; }

  //@}

  /// \name get missing values
  //@{

  static double getMissingMetaDouble() { return missingMetaDouble; }
  static float getMissingMetaFloat() { return missingMetaFloat; }
  static int getMissingMetaInt() { return missingMetaInt; }
  static char getMissingMetaChar() { return missingMetaChar; }
  static fl64 getMissingFl64() { return missingFl64; }
  static fl32 getMissingFl32() { return missingFl32; }
  static si32 getMissingSi32() { return missingSi32; }
  static si16 getMissingSi16() { return missingSi16; }
  static si08 getMissingSi08() { return missingSi08; }

  //@}

};

#endif
