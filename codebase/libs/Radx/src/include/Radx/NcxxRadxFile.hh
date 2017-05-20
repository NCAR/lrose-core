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
// NcxxRadxFile.hh
//
// NcxxRadxFile object
//
// NetCDF data for radar radial data in CF-compliant file
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2009
//
///////////////////////////////////////////////////////////////

#ifndef NcxxRadxFile_HH
#define NcxxRadxFile_HH

#include <string>
#include <vector>

#include <Radx/Radx.hh>
#include <Radx/RadxFile.hh>
#include <Radx/RadxRangeGeom.hh>
#include <Radx/RadxCfactors.hh>
#include <Radx/RadxRemap.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxGeoref.hh>
#include <NcxxUtils/Ncxx.hh>

class RadxField;
class RadxVol;
class RadxRay;
class RadxSweep;
class RadxRcalib;
using namespace std;

///////////////////////////////////////////////////////////////
/// CXX FILE IO CLASS FOR NETCDF CF/RADIAL FILE FORMAT
///
/// This subclass of RadxFile handles I/O for netcdf files.

class NcxxRadxFile : public RadxFile

{
  
public:

  /// Constructor
  
  NcxxRadxFile();
  
  /// Destructor
  
  virtual ~NcxxRadxFile();
  
  /// clear all data
  
  virtual void clear();

  /// Check if specified file is a CfRadialFile
  /// Returns true on success, false on failure
  
  virtual bool isSupported(const string &path);
    
  /// Check if specified file is a CfRadialXx file.
  /// Returns true on success, false on failure
  
  bool isCfRadialXx(const string &path);
    
  //////////////////////////////////////////////////////////////
  /// \name Perform writing:
  //@{

  //////////////////////////////////////////////////////////////
  /// Write data from RadxVol volume to the specified directory.
  ///
  /// If addDaySubDir is true, a subdirectory will be
  /// created with the name dir/yyyymmdd/.
  ///
  /// If addYearSubDir is true, a subdirectory will be
  /// created with the name dir/yyyy/.
  ///
  /// If both addDaySubDir and addYearSubDir are true,
  /// the subdirectory will be dir/yyyy/yyyymmdd/.
  ///
  /// Returns 0 on success, -1 on failure
  ///
  /// Use getErrStr() to get error message if error occurs.
  /// Use getDirInUse() for directory to which the data was written.
  /// Use getPathInUse() for path to which the data was written.
  
  virtual int writeToDir(const RadxVol &vol,
                         const string &dir,
                         bool addDaySubDir,
                         bool addYearSubDir);

  //////////////////////////////////////////////////////////////
  /// Write data from RadxVol volume to the specified path.
  //
  /// Returns 0 on success, -1 on failure
  //
  /// Use getErrStr() to get error message if error occurs.
  /// Use getPathInUse() for path to which the data was written.
  
  virtual int writeToPath(const RadxVol &vol,
                          const string &path);

  //@}

  //////////////////////////////////////////////////////////////
  /// \name Perform the read:
  //@{
  
  /// Read in data file from specified path,
  /// load up volume object.
  /// Returns 0 on success, -1 on failure
  /// Use getErrStr() if error occurs
  
  virtual int readFromPath(const string &path,
                           RadxVol &vol);

  //@}

  ////////////////////////
  /// \name Checks
  //@{
  
  /// Check if number of gates vary in volume
  
  bool getNGatesVary() const { return _nGatesVary; }
    
  //@}

  ////////////////////////
  /// \name Printing:
  //@{
  
  /// Print summary after read.
  
  virtual void print(ostream &out) const;
  
  /// Print data in file, in native format.
  ///
  /// This is not really applicable to netcdf files, and will
  /// return an error.
  ///
  /// For netcdf, use ncdump to inspect file.
  ///
  /// Returns 0 on success, -1 on failure
  /// Use getErrStr() if error occurs
  
  virtual int printNative(const string &path, ostream &out,
                          bool printRays, bool printData);

  /// Get the date and time from a dorade file path.
  /// returns 0 on success, -1 on failure

  int getTimeFromPath(const string &path, RadxTime &rtime);

  //@}

protected:
private:

  // string length

  static const int NCF_STRING_LEN_8 = 8;
  static const int NCF_STRING_LEN_32 = 32;
  typedef char String8_t[NCF_STRING_LEN_8];
  typedef char String32_t[NCF_STRING_LEN_32];

  static const string CfConvention; // base convention
  static const string BaseConvention; // base convention
  static const string CurrentVersion; // current version

  // volume for writing
  
  const RadxVol *_writeVol; ///< volume from which data is written
  
  // format version

  string _conventions;
  string _subconventions;
  string _version;
  
  // netcdf file
  
  NcxxFile _file;
  string _tmpPath;

  // dimensions

  NcxxDim _timeDim;
  NcxxDim _rangeDim;
  NcxxDim _nPointsDim;
  NcxxDim _sweepDim;
  NcxxDim _calDim;
  NcxxDim _stringLen8Dim;
  NcxxDim _stringLen32Dim;
  NcxxDim _statusXmlDim;
  NcxxDim _frequencyDim;

  // coordinate variables

  NcxxVar _timeVar;
  NcxxVar _rangeVar;

  // gate geometry variables
  
  NcxxVar _rayNGatesVar;
  NcxxVar _rayStartIndexVar;

  NcxxVar _rayStartRangeVar;
  NcxxVar _rayGateSpacingVar;
  bool _gateGeomVaries;

  // scalar variables
  
  NcxxVar _volumeNumberVar;
  NcxxVar _instrumentTypeVar;
  NcxxVar _platformTypeVar;
  NcxxVar _primaryAxisVar;
  NcxxVar _statusXmlVar;
  NcxxVar _startTimeVar;
  NcxxVar _endTimeVar;

  // frequencies

  NcxxVar _frequencyVar;

  // projection variables
  
  NcxxVar _projVar;
  NcxxVar _latitudeVar;
  NcxxVar _longitudeVar;
  NcxxVar _altitudeVar;
  NcxxVar _altitudeAglVar;
  
  // operational modes

  NcxxVar _polModeVar;
  NcxxVar _prtModeVar;

  // sweep variables

  NcxxVar _sweepNumberVar;
  NcxxVar _sweepModeVar;
  NcxxVar _sweepFollowModeVar;
  NcxxVar _sweepFixedAngleVar;
  bool _fixedAnglesFound;
  NcxxVar _targetScanRateVar;
  NcxxVar _sweepStartRayIndexVar;
  NcxxVar _sweepEndRayIndexVar;
  NcxxVar _raysAreIndexedVar;
  NcxxVar _rayAngleResVar;
  NcxxVar _intermedFreqHzVar;

  // radar param variables

  NcxxVar _radarAntennaGainHVar;
  NcxxVar _radarAntennaGainVVar;
  NcxxVar _radarBeamWidthHVar;
  NcxxVar _radarBeamWidthVVar;
  NcxxVar _radarRxBandwidthVar;

  // lidar param variables

  NcxxVar _lidarConstantVar;
  NcxxVar _lidarPulseEnergyJVar;
  NcxxVar _lidarPeakPowerWVar;
  NcxxVar _lidarApertureDiamCmVar;
  NcxxVar _lidarApertureEfficiencyVar;
  NcxxVar _lidarFieldOfViewMradVar;
  NcxxVar _lidarBeamDivergenceMradVar;

  // calibration variables

  NcxxVar _rCalTimeVar;
  NcxxVar _rCalPulseWidthVar;
  NcxxVar _rCalXmitPowerHVar;
  NcxxVar _rCalXmitPowerVVar;
  NcxxVar _rCalTwoWayWaveguideLossHVar;
  NcxxVar _rCalTwoWayWaveguideLossVVar;
  NcxxVar _rCalTwoWayRadomeLossHVar;
  NcxxVar _rCalTwoWayRadomeLossVVar;
  NcxxVar _rCalReceiverMismatchLossVar;
  NcxxVar _rCalRadarConstHVar;
  NcxxVar _rCalRadarConstVVar;
  NcxxVar _rCalAntennaGainHVar;
  NcxxVar _rCalAntennaGainVVar;
  NcxxVar _rCalNoiseHcVar;
  NcxxVar _rCalNoiseHxVar;
  NcxxVar _rCalNoiseVcVar;
  NcxxVar _rCalNoiseVxVar;
  NcxxVar _rCalReceiverGainHcVar;
  NcxxVar _rCalReceiverGainHxVar;
  NcxxVar _rCalReceiverGainVcVar;
  NcxxVar _rCalReceiverGainVxVar;
  NcxxVar _rCalBaseDbz1kmHcVar;
  NcxxVar _rCalBaseDbz1kmHxVar;
  NcxxVar _rCalBaseDbz1kmVcVar;
  NcxxVar _rCalBaseDbz1kmVxVar;
  NcxxVar _rCalSunPowerHcVar;
  NcxxVar _rCalSunPowerHxVar;
  NcxxVar _rCalSunPowerVcVar;
  NcxxVar _rCalSunPowerVxVar;
  NcxxVar _rCalNoiseSourcePowerHVar;
  NcxxVar _rCalNoiseSourcePowerVVar;
  NcxxVar _rCalPowerMeasLossHVar;
  NcxxVar _rCalPowerMeasLossVVar;
  NcxxVar _rCalCouplerForwardLossHVar;
  NcxxVar _rCalCouplerForwardLossVVar;
  NcxxVar _rCalDbzCorrectionVar;
  NcxxVar _rCalZdrCorrectionVar;
  NcxxVar _rCalLdrCorrectionHVar;
  NcxxVar _rCalLdrCorrectionVVar;
  NcxxVar _rCalSystemPhidpVar;
  NcxxVar _rCalTestPowerHVar;
  NcxxVar _rCalTestPowerVVar;
  NcxxVar _rCalReceiverSlopeHcVar;
  NcxxVar _rCalReceiverSlopeHxVar;
  NcxxVar _rCalReceiverSlopeVcVar;
  NcxxVar _rCalReceiverSlopeVxVar;

  // ray variables

  NcxxVar _azimuthVar;
  NcxxVar _elevationVar;
  NcxxVar _pulseWidthVar;
  NcxxVar _prtVar;
  NcxxVar _prtRatioVar;
  NcxxVar _nyquistVar;
  NcxxVar _unambigRangeVar;
  NcxxVar _antennaTransitionVar;
  NcxxVar _georefsAppliedVar;
  NcxxVar _georefTimeVar;
  NcxxVar _nSamplesVar;
  NcxxVar _calIndexVar;
  NcxxVar _xmitPowerHVar;
  NcxxVar _xmitPowerVVar;
  NcxxVar _scanRateVar;
  NcxxVar _estNoiseDbmHcVar;
  NcxxVar _estNoiseDbmVcVar;
  NcxxVar _estNoiseDbmHxVar;
  NcxxVar _estNoiseDbmVxVar;

  // georeference variables

  bool _georefsActive;
  bool _georefsApplied;
  NcxxVar _headingVar;
  NcxxVar _trackVar;
  NcxxVar _rollVar;
  NcxxVar _pitchVar;
  NcxxVar _driftVar;
  NcxxVar _rotationVar;
  NcxxVar _tiltVar;
  NcxxVar _ewVelocityVar;
  NcxxVar _nsVelocityVar;
  NcxxVar _vertVelocityVar;
  NcxxVar _ewWindVar;
  NcxxVar _nsWindVar;
  NcxxVar _vertWindVar;
  NcxxVar _headingRateVar;
  NcxxVar _pitchRateVar;
  NcxxVar _rollRateVar;
  NcxxVar _driveAngle1Var;
  NcxxVar _driveAngle2Var;

  // correction factor variables

  bool _correctionsActive;
  NcxxVar _azimuthCorrVar;
  NcxxVar _elevationCorrVar;
  NcxxVar _rangeCorrVar;
  NcxxVar _longitudeCorrVar;
  NcxxVar _latitudeCorrVar;
  NcxxVar _pressureAltCorrVar;
  NcxxVar _altitudeCorrVar;
  NcxxVar _ewVelCorrVar;
  NcxxVar _nsVelCorrVar;
  NcxxVar _vertVelCorrVar;
  NcxxVar _headingCorrVar;
  NcxxVar _rollCorrVar;
  NcxxVar _pitchCorrVar;
  NcxxVar _driftCorrVar;
  NcxxVar _rotationCorrVar;
  NcxxVar _tiltCorrVar;

  // unique field names for writing

  vector<string> _uniqueFieldNames;

  // objects to be set on read

  string _title;
  string _institution;
  string _references;
  string _source;
  string _history;
  string _comment;
  string _author;
  string _origFormat;
  string _driver;
  string _created;
  string _statusXml;

  string _siteName;
  string _scanName;
  int _scanId;
  string _instrumentName;

  vector<double> _dTimes;
  time_t _refTimeSecsFile;
  bool _rayTimesIncrease;

  int _volumeNumber;
  Radx::InstrumentType_t _instrumentType;
  Radx::PlatformType_t _platformType;
  Radx::PrimaryAxis_t _primaryAxis;

  vector<double> _latitude;
  vector<double> _longitude;
  vector<double> _altitude;
  vector<double> _altitudeAgl;

  vector<double> _frequency;

  double _radarBeamWidthDegH;
  double _radarBeamWidthDegV;
  double _radarRxBandwidthHz;
  double _radarAntennaGainDbH;
  double _radarAntennaGainDbV;

  double _lidarConstant;
  double _lidarPulseEnergyJ;
  double _lidarPeakPowerW;
  double _lidarApertureDiamCm;
  double _lidarApertureEfficiency;
  double _lidarFieldOfViewMrad;
  double _lidarBeamDivergenceMrad;

  size_t _nTimesInFile;
  vector<double> _rangeKm;
  size_t _nRangeInFile;

  bool _nGatesVary;
  int _nPoints;
  vector<int> _rayNGates;
  vector<int> _rayStartIndex;
  vector<double> _rayStartRange;
  vector<double> _rayGateSpacing;
  
  RadxRangeGeom _geom;
  RadxRemap _remap;
  bool _gateSpacingIsConstant;
  vector<RadxRay *> _raysVol, _raysFromFile;
  vector<RadxRcalib *> _rCals;
  vector<RadxField *> _fields;
  RadxCfactors _cfactors;

  // storing sweep information

  class SweepInfo {
  public:
    string path;
    int sweepNum;
    size_t indexInFile;
    double fixedAngle;
  };
  vector<RadxSweep *> _sweeps;
  vector<RadxSweep *> _sweepsInFile;
  vector<SweepInfo> _sweepsOrig;
  vector<SweepInfo> _sweepsToRead;

  // storing ray information for reading from file

  class RayInfo {
  public:
    size_t indexInFile;
    const RadxSweep *sweep;
  };
  vector<RayInfo> _raysToRead;

  // ray meta data arrays

  vector<double> _rayAzimuths;
  vector<double> _rayElevations;
  vector<double> _rayPulseWidths;
  vector<double> _rayPrts;
  vector<double> _rayPrtRatios;
  vector<double> _rayNyquists;
  vector<double> _rayUnambigRanges;
  vector<bool> _rayAntennaTransitions;
  vector<bool> _rayGeorefsApplied;
  vector<int> _rayNSamples;
  vector<int> _rayCalNum;
  vector<double> _rayXmitPowerH;
  vector<double> _rayXmitPowerV;
  vector<double> _rayScanRate;
  
  vector<double> _rayEstNoiseDbmHc;
  vector<double> _rayEstNoiseDbmVc;
  vector<double> _rayEstNoiseDbmHx;
  vector<double> _rayEstNoiseDbmVx;

  bool _estNoiseAvailHc;
  bool _estNoiseAvailVc;
  bool _estNoiseAvailHx;
  bool _estNoiseAvailVx;

  // georeference meta data

  RadxGeoref _geoCount;

  vector<double> _geoTime;
  vector<double> _geoLatitude;
  vector<double> _geoLongitude;
  vector<double> _geoAltitudeMsl;
  vector<double> _geoAltitudeAgl;
  vector<double> _geoEwVelocity;
  vector<double> _geoNsVelocity;
  vector<double> _geoVertVelocity;
  vector<double> _geoHeading;
  vector<double> _geoTrack;
  vector<double> _geoRoll;
  vector<double> _geoPitch;
  vector<double> _geoDrift;
  vector<double> _geoRotation;
  vector<double> _geoTilt;
  vector<double> _geoEwWind;
  vector<double> _geoNsWind;
  vector<double> _geoVertWind;
  vector<double> _geoHeadingRate;
  vector<double> _geoPitchRate;
  vector<double> _geoRollRate;
  vector<double> _geoDriveAngle1;
  vector<double> _geoDriveAngle2;

  // string constants

  const static char* ADD_OFFSET;
  const static char* AIRBORNE;
  const static char* ALTITUDE;
  const static char* ALTITUDE_AGL;
  const static char* ALTITUDE_CORRECTION;
  const static char* ALTITUDE_OF_PROJECTION_ORIGIN;
  const static char* ANCILLARY_VARIABLES;
  const static char* ANTENNA_TRANSITION;
  const static char* AXIS;
  const static char* AUTHOR;
  const static char* AZIMUTH;
  const static char* AZIMUTH_CORRECTION;
  const static char* BLOCK_AVG_LENGTH;
  const static char* CALENDAR;
  const static char* CFRADIAL;
  const static char* CM;
  const static char* COMMENT;
  const static char* COMPRESS;
  const static char* CONVENTIONS;
  const static char* COORDINATES;
  const static char* CREATED;
  const static char* DB;
  const static char* DBM;
  const static char* DBZ;
  const static char* DEGREES;
  const static char* DEGREES_EAST;
  const static char* DEGREES_NORTH;
  const static char* DEGREES_PER_SECOND;
  const static char* DORADE;
  const static char* DOWN;
  const static char* DRIFT;
  const static char* DRIFT_CORRECTION;
  const static char* DRIVER;
  const static char* DRIVE_ANGLE_1;
  const static char* DRIVE_ANGLE_2;
  const static char* EASTWARD_VELOCITY;
  const static char* EASTWARD_VELOCITY_CORRECTION;
  const static char* EASTWARD_WIND;
  const static char* ELEVATION;
  const static char* ELEVATION_CORRECTION;
  const static char* END_DATETIME;
  const static char* FALSE_NORTHING;
  const static char* FALSE_EASTING;
  const static char* FFT_LENGTH;
  const static char* FIELD_FOLDS;
  const static char* FILL_VALUE;
  const static char* FIXED_ANGLE;
  const static char* FLAG_MASKS;
  const static char* FLAG_MEANINGS;
  const static char* FLAG_VALUES;
  const static char* FOLD_LIMIT_LOWER;
  const static char* FOLD_LIMIT_UPPER;
  const static char* FOLLOW_MODE;
  const static char* FREQUENCY;
  const static char* GEOMETRY_CORRECTION;
  const static char* GEOREFS_APPLIED;
  const static char* GEOREF_TIME;
  const static char* GREGORIAN;
  const static char* GRID_MAPPING;
  const static char* GRID_MAPPING_NAME;
  const static char* HEADING;
  const static char* HEADING_CHANGE_RATE;
  const static char* HEADING_CORRECTION;
  const static char* HISTORY;
  const static char* HZ;
  const static char* INDEX_VAR_NAME;
  const static char* INSTITUTION;
  const static char* INSTRUMENT_NAME;
  const static char* INSTRUMENT_PARAMETERS;
  const static char* INSTRUMENT_TYPE;
  const static char* IS_DISCRETE;
  const static char* IS_QUALITY;
  const static char* IS_SPECTRUM;
  const static char* INTERMED_FREQ_HZ;
  const static char* JOULES;
  const static char* JULIAN;
  const static char* LATITUDE;
  const static char* LATITUDE_CORRECTION;
  const static char* LATITUDE_OF_PROJECTION_ORIGIN;
  const static char* LEGEND_XML;
  const static char* LIDAR_APERTURE_DIAMETER;
  const static char* LIDAR_APERTURE_EFFICIENCY;
  const static char* LIDAR_BEAM_DIVERGENCE;
  const static char* LIDAR_CALIBRATION;
  const static char* LIDAR_CONSTANT;
  const static char* LIDAR_FIELD_OF_VIEW;
  const static char* LIDAR_PARAMETERS;
  const static char* LIDAR_PEAK_POWER;
  const static char* LIDAR_PULSE_ENERGY;
  const static char* LONGITUDE;
  const static char* LONGITUDE_CORRECTION;
  const static char* LONG_NAME;
  const static char* LONGITUDE_OF_PROJECTION_ORIGIN;
  const static char* META_GROUP;
  const static char* METERS;
  const static char* METERS_BETWEEN_GATES;
  const static char* METERS_PER_SECOND;
  const static char* METERS_TO_CENTER_OF_FIRST_GATE;
  const static char* MISSING_VALUE;
  const static char* MOVING;
  const static char* MRAD;
  const static char* NORTHWARD_VELOCITY;
  const static char* NORTHWARD_VELOCITY_CORRECTION;
  const static char* NORTHWARD_WIND;
  const static char* NYQUIST_VELOCITY;
  const static char* N_GATES_VARY;
  const static char* N_POINTS;
  const static char* N_PRTS;
  const static char* N_SAMPLES;
  const static char* N_SPECTRA;
  const static char* OPTIONS;
  const static char* ORIGINAL_FORMAT;
  const static char* PERCENT;
  const static char* PITCH;
  const static char* PITCH_CHANGE_RATE;
  const static char* PITCH_CORRECTION;
  const static char* PLATFORM_IS_MOBILE;
  const static char* PLATFORM_TYPE;
  const static char* PLATFORM_VELOCITY;
  const static char* POLARIZATION_MODE;
  const static char* POLARIZATION_SEQUENCE;
  const static char* POSITIVE;
  const static char* PRESSURE_ALTITUDE_CORRECTION;
  const static char* PRIMARY_AXIS;
  const static char* PRT;
  const static char* PRT_MODE;
  const static char* PRT_RATIO;
  const static char* PRT_SEQUENCE;
  const static char* PULSE_WIDTH;
  const static char* QUALIFIED_VARIABLES;
  const static char* QC_PROCEDURES;
  const static char* RADAR_ANTENNA_GAIN_H;
  const static char* RADAR_ANTENNA_GAIN_V;
  const static char* RADAR_BEAM_WIDTH_H;
  const static char* RADAR_BEAM_WIDTH_V;
  const static char* RADAR_CALIBRATION;
  const static char* RADAR_ESTIMATED_NOISE_DBM_HC;
  const static char* RADAR_ESTIMATED_NOISE_DBM_HX;
  const static char* RADAR_ESTIMATED_NOISE_DBM_VC;
  const static char* RADAR_ESTIMATED_NOISE_DBM_VX;
  const static char* RADAR_MEASURED_COLD_NOISE;
  const static char* RADAR_MEASURED_HOT_NOISE;
  const static char* RADAR_MEASURED_SKY_NOISE;
  const static char* RADAR_MEASURED_TRANSMIT_POWER_H;
  const static char* RADAR_MEASURED_TRANSMIT_POWER_V;
  const static char* RADAR_PARAMETERS;
  const static char* RADAR_RX_BANDWIDTH;
  const static char* RANGE;
  const static char* RANGE_CORRECTION;
  const static char* RAYS_ARE_INDEXED;
  const static char* RAY_ANGLE_RES;
  const static char* RAY_GATE_SPACING;
  const static char* RAY_N_GATES;
  const static char* RAY_START_INDEX;
  const static char* RAY_START_RANGE;
  const static char* RAY_TIMES_INCREASE;
  const static char* REFERENCES;
  const static char* ROLL;
  const static char* ROLL_CHANGE_RATE;
  const static char* ROLL_CORRECTION;
  const static char* ROTATION;
  const static char* ROTATION_CORRECTION;
  const static char* RX_RANGE_RESOLUTION;
  const static char* R_CALIB;
  const static char* R_CALIB_ANTENNA_GAIN_H;
  const static char* R_CALIB_ANTENNA_GAIN_V;
  const static char* R_CALIB_BASE_DBZ_1KM_HC;
  const static char* R_CALIB_BASE_DBZ_1KM_HX;
  const static char* R_CALIB_BASE_DBZ_1KM_VC;
  const static char* R_CALIB_BASE_DBZ_1KM_VX;
  const static char* R_CALIB_COUPLER_FORWARD_LOSS_H;
  const static char* R_CALIB_COUPLER_FORWARD_LOSS_V;
  const static char* R_CALIB_DBZ_CORRECTION;
  const static char* R_CALIB_DIELECTRIC_FACTOR_USED;
  const static char* R_CALIB_INDEX;
  const static char* R_CALIB_LDR_CORRECTION_H;
  const static char* R_CALIB_LDR_CORRECTION_V;
  const static char* R_CALIB_NOISE_HC;
  const static char* R_CALIB_NOISE_HX;
  const static char* R_CALIB_NOISE_SOURCE_POWER_H;
  const static char* R_CALIB_NOISE_SOURCE_POWER_V;
  const static char* R_CALIB_NOISE_VC;
  const static char* R_CALIB_NOISE_VX;
  const static char* R_CALIB_POWER_MEASURE_LOSS_H;
  const static char* R_CALIB_POWER_MEASURE_LOSS_V;
  const static char* R_CALIB_PROBERT_JONES_CORRECTION;
  const static char* R_CALIB_PULSE_WIDTH;
  const static char* R_CALIB_RADAR_CONSTANT_H;
  const static char* R_CALIB_RADAR_CONSTANT_V;
  const static char* R_CALIB_RECEIVER_GAIN_HC;
  const static char* R_CALIB_RECEIVER_GAIN_HX;
  const static char* R_CALIB_RECEIVER_GAIN_VC;
  const static char* R_CALIB_RECEIVER_GAIN_VX;
  const static char* R_CALIB_RECEIVER_MISMATCH_LOSS;
  const static char* R_CALIB_RECEIVER_MISMATCH_LOSS_H;
  const static char* R_CALIB_RECEIVER_MISMATCH_LOSS_V;
  const static char* R_CALIB_RECEIVER_SLOPE_HC;
  const static char* R_CALIB_RECEIVER_SLOPE_HX;
  const static char* R_CALIB_RECEIVER_SLOPE_VC;
  const static char* R_CALIB_RECEIVER_SLOPE_VX;
  const static char* R_CALIB_SUN_POWER_HC;
  const static char* R_CALIB_SUN_POWER_HX;
  const static char* R_CALIB_SUN_POWER_VC;
  const static char* R_CALIB_SUN_POWER_VX;
  const static char* R_CALIB_SYSTEM_PHIDP;
  const static char* R_CALIB_TEST_POWER_H;
  const static char* R_CALIB_TEST_POWER_V;
  const static char* R_CALIB_TIME;
  const static char* R_CALIB_TIME_W3C_STR;
  const static char* R_CALIB_TWO_WAY_RADOME_LOSS_H;
  const static char* R_CALIB_TWO_WAY_RADOME_LOSS_V;
  const static char* R_CALIB_TWO_WAY_WAVEGUIDE_LOSS_H;
  const static char* R_CALIB_TWO_WAY_WAVEGUIDE_LOSS_V;
  const static char* R_CALIB_XMIT_POWER_H;
  const static char* R_CALIB_XMIT_POWER_V;
  const static char* R_CALIB_ZDR_CORRECTION;
  const static char* SAMPLING_RATIO;
  const static char* SCALE_FACTOR;
  const static char* SCANNING;
  const static char* SCANNING_RADIAL;
  const static char* SCAN_ID;
  const static char* SCAN_NAME;
  const static char* SCAN_RATE;
  const static char* SECONDS;
  const static char* SECS_SINCE_JAN1_1970;
  const static char* SITE_NAME;
  const static char* SOURCE;
  const static char* SPACING_IS_CONSTANT;
  const static char* SPECTRUM_N_SAMPLES;
  const static char* STANDARD;
  const static char* STANDARD_NAME;
  const static char* STARING;
  const static char* START_DATETIME;
  const static char* STATIONARY;
  const static char* STATUS_XML;
  const static char* STATUS_XML_LENGTH;
  const static char* STRING_LENGTH_256;
  const static char* STRING_LENGTH_32;
  const static char* STRING_LENGTH_64;
  const static char* STRING_LENGTH_8;
  const static char* SUB_CONVENTIONS;
  const static char* SWEEP;
  const static char* SWEEP_END_RAY_INDEX;
  const static char* SWEEP_MODE;
  const static char* SWEEP_NUMBER;
  const static char* SWEEP_START_RAY_INDEX;
  const static char* TARGET_SCAN_RATE;
  const static char* THRESHOLDING_XML;
  const static char* TILT;
  const static char* TILT_CORRECTION;
  const static char* TIME;
  const static char* TIME_COVERAGE_END;
  const static char* TIME_COVERAGE_START;
  const static char* TITLE;
  const static char* TRACK;
  const static char* UNAMBIGUOUS_RANGE;
  const static char* UNITS;
  const static char* UP;
  const static char* VALID_MAX;
  const static char* VALID_MIN;
  const static char* VALID_RANGE;
  const static char* VERSION;
  const static char* VERTICAL_VELOCITY;
  const static char* VERTICAL_VELOCITY_CORRECTION;
  const static char* VERTICAL_WIND;
  const static char* VOLUME;
  const static char* VOLUME_NUMBER;
  const static char* W3C_STR;
  const static char* WATTS;

  // long names for metadata

  const static char* ALTITUDE_AGL_LONG;
  const static char* ALTITUDE_CORRECTION_LONG;
  const static char* ALTITUDE_LONG;
  const static char* ANTENNA_TRANSITION_LONG;
  const static char* AZIMUTH_CORRECTION_LONG;
  const static char* AZIMUTH_LONG;
  const static char* CO_TO_CROSS_POLAR_CORRELATION_RATIO_H;
  const static char* CO_TO_CROSS_POLAR_CORRELATION_RATIO_V;
  const static char* CROSS_POLAR_DIFFERENTIAL_PHASE;
  const static char* DRIFT_CORRECTION_LONG;
  const static char* DRIFT_LONG;
  const static char* EASTWARD_VELOCITY_CORRECTION_LONG;
  const static char* EASTWARD_VELOCITY_LONG;
  const static char* EASTWARD_WIND_LONG;
  const static char* ELEVATION_CORRECTION_LONG;
  const static char* ELEVATION_LONG;
  const static char* FIXED_ANGLE_LONG;
  const static char* FOLLOW_MODE_LONG;
  const static char* FREQUENCY_LONG;
  const static char* GEOREF_TIME_LONG;
  const static char* HEADING_CHANGE_RATE_LONG;
  const static char* HEADING_CORRECTION_LONG;
  const static char* HEADING_LONG;
  const static char* INSTRUMENT_NAME_LONG;
  const static char* INSTRUMENT_TYPE_LONG;
  const static char* INTERMED_FREQ_HZ_LONG;
  const static char* LATITUDE_CORRECTION_LONG;
  const static char* LATITUDE_LONG;
  const static char* LIDAR_APERTURE_DIAMETER_LONG;
  const static char* LIDAR_APERTURE_EFFICIENCY_LONG;
  const static char* LIDAR_BEAM_DIVERGENCE_LONG;
  const static char* LIDAR_CONSTANT_LONG;
  const static char* LIDAR_FIELD_OF_VIEW_LONG;
  const static char* LIDAR_PEAK_POWER_LONG;
  const static char* LIDAR_PULSE_ENERGY_LONG;
  const static char* LONGITUDE_CORRECTION_LONG;
  const static char* LONGITUDE_LONG;
  const static char* NORTHWARD_VELOCITY_CORRECTION_LONG;
  const static char* NORTHWARD_VELOCITY_LONG;
  const static char* NORTHWARD_WIND_LONG;
  const static char* NYQUIST_VELOCITY_LONG;
  const static char* N_SAMPLES_LONG;
  const static char* PITCH_CHANGE_RATE_LONG;
  const static char* PITCH_CORRECTION_LONG;
  const static char* PITCH_LONG;
  const static char* PLATFORM_IS_MOBILE_LONG;
  const static char* PLATFORM_TYPE_LONG;
  const static char* POLARIZATION_MODE_LONG;
  const static char* PRESSURE_ALTITUDE_CORRECTION_LONG;
  const static char* PRIMARY_AXIS_LONG;
  const static char* PRT_MODE_LONG;
  const static char* PRT_RATIO_LONG;
  const static char* PRT_LONG;
  const static char* PULSE_WIDTH_LONG;
  const static char* RADAR_ANTENNA_GAIN_H_LONG;
  const static char* RADAR_ANTENNA_GAIN_V_LONG;
  const static char* RADAR_BEAM_WIDTH_H_LONG;
  const static char* RADAR_BEAM_WIDTH_V_LONG;
  const static char* RADAR_ESTIMATED_NOISE_DBM_HC_LONG;
  const static char* RADAR_ESTIMATED_NOISE_DBM_HX_LONG;
  const static char* RADAR_ESTIMATED_NOISE_DBM_VC_LONG;
  const static char* RADAR_ESTIMATED_NOISE_DBM_VX_LONG;
  const static char* RADAR_MEASURED_TRANSMIT_POWER_H_LONG;
  const static char* RADAR_MEASURED_TRANSMIT_POWER_V_LONG;
  const static char* RADAR_RX_BANDWIDTH_LONG;
  const static char* RANGE_CORRECTION_LONG;
  const static char* RANGE_LONG;
  const static char* RAYS_ARE_INDEXED_LONG;
  const static char* RAY_ANGLE_RES_LONG;
  const static char* ROLL_CHANGE_RATE_LONG;
  const static char* ROLL_CORRECTION_LONG;
  const static char* ROLL_LONG;
  const static char* ROTATION_CORRECTION_LONG;
  const static char* ROTATION_LONG;
  const static char* R_CALIB_ANTENNA_GAIN_H_LONG;
  const static char* R_CALIB_ANTENNA_GAIN_V_LONG;
  const static char* R_CALIB_BASE_DBZ_1KM_HC_LONG;
  const static char* R_CALIB_BASE_DBZ_1KM_HX_LONG;
  const static char* R_CALIB_BASE_DBZ_1KM_VC_LONG;
  const static char* R_CALIB_BASE_DBZ_1KM_VX_LONG;
  const static char* R_CALIB_COUPLER_FORWARD_LOSS_H_LONG;
  const static char* R_CALIB_COUPLER_FORWARD_LOSS_V_LONG;
  const static char* R_CALIB_DBZ_CORRECTION_LONG;
  const static char* R_CALIB_INDEX_LONG;
  const static char* R_CALIB_LDR_CORRECTION_H_LONG;
  const static char* R_CALIB_LDR_CORRECTION_V_LONG;
  const static char* R_CALIB_NOISE_HC_LONG;
  const static char* R_CALIB_NOISE_HX_LONG;
  const static char* R_CALIB_NOISE_SOURCE_POWER_H_LONG;
  const static char* R_CALIB_NOISE_SOURCE_POWER_V_LONG;
  const static char* R_CALIB_NOISE_VC_LONG;
  const static char* R_CALIB_NOISE_VX_LONG;
  const static char* R_CALIB_POWER_MEASURE_LOSS_H_LONG;
  const static char* R_CALIB_POWER_MEASURE_LOSS_V_LONG;
  const static char* R_CALIB_PULSE_WIDTH_LONG;
  const static char* R_CALIB_RADAR_CONSTANT_H_LONG;
  const static char* R_CALIB_RADAR_CONSTANT_V_LONG;
  const static char* R_CALIB_RECEIVER_GAIN_HC_LONG;
  const static char* R_CALIB_RECEIVER_GAIN_HX_LONG;
  const static char* R_CALIB_RECEIVER_GAIN_VC_LONG;
  const static char* R_CALIB_RECEIVER_GAIN_VX_LONG;
  const static char* R_CALIB_RECEIVER_MISMATCH_LOSS_LONG;
  const static char* R_CALIB_RECEIVER_SLOPE_HC_LONG;
  const static char* R_CALIB_RECEIVER_SLOPE_HX_LONG;
  const static char* R_CALIB_RECEIVER_SLOPE_VC_LONG;
  const static char* R_CALIB_RECEIVER_SLOPE_VX_LONG;
  const static char* R_CALIB_SUN_POWER_HC_LONG;
  const static char* R_CALIB_SUN_POWER_HX_LONG;
  const static char* R_CALIB_SUN_POWER_VC_LONG;
  const static char* R_CALIB_SUN_POWER_VX_LONG;
  const static char* R_CALIB_SYSTEM_PHIDP_LONG;
  const static char* R_CALIB_TEST_POWER_H_LONG;
  const static char* R_CALIB_TEST_POWER_V_LONG;
  const static char* R_CALIB_TIME_LONG;
  const static char* R_CALIB_TWO_WAY_RADOME_LOSS_H_LONG;
  const static char* R_CALIB_TWO_WAY_RADOME_LOSS_V_LONG;
  const static char* R_CALIB_TWO_WAY_WAVEGUIDE_LOSS_H_LONG;
  const static char* R_CALIB_TWO_WAY_WAVEGUIDE_LOSS_V_LONG;
  const static char* R_CALIB_XMIT_POWER_H_LONG;
  const static char* R_CALIB_XMIT_POWER_V_LONG;
  const static char* R_CALIB_ZDR_CORRECTION_LONG;
  const static char* SCAN_ID_LONG;
  const static char* SCAN_NAME_LONG;
  const static char* SCAN_RATE_LONG;
  const static char* SITE_NAME_LONG;
  const static char* SPACING_IS_CONSTANT_LONG;
  const static char* SPECTRUM_COPOLAR_HORIZONTAL;
  const static char* SPECTRUM_COPOLAR_VERTICAL;
  const static char* SPECTRUM_CROSSPOLAR_HORIZONTAL;
  const static char* SPECTRUM_CROSSPOLAR_VERTICAL;
  const static char* CROSS_SPECTRUM_OF_COPOLAR_HORIZONTAL;
  const static char* CROSS_SPECTRUM_OF_COPOLAR_VERTICAL;
  const static char* CROSS_SPECTRUM_OF_CROSSPOLAR_HORIZONTAL;
  const static char* CROSS_SPECTRUM_OF_CROSSPOLAR_VERTICAL;
  const static char* SWEEP_END_RAY_INDEX_LONG;
  const static char* SWEEP_MODE_LONG;
  const static char* SWEEP_NUMBER_LONG;
  const static char* SWEEP_START_RAY_INDEX_LONG;
  const static char* TARGET_SCAN_RATE_LONG;
  const static char* TILT_CORRECTION_LONG;
  const static char* TILT_LONG;
  const static char* TIME_COVERAGE_END_LONG;
  const static char* TIME_COVERAGE_START_LONG;
  const static char* TIME_LONG;
  const static char* TRACK_LONG;
  const static char* UNAMBIGUOUS_RANGE_LONG;
  const static char* VERTICAL_VELOCITY_CORRECTION_LONG;
  const static char* VERTICAL_VELOCITY_LONG;
  const static char* VERTICAL_WIND_LONG;
  const static char* VOLUME_NUMBER_LONG;

  // private methods for NcfRadial.cc
  
  int _writeSweepsToDir(const RadxVol &vol, const string &dir,
                        bool addDaySubDir, bool addYearSubDir);
  int _writeSweepToDir(const RadxVol &vol, const string &dir,
                       bool addDaySubDir, bool addYearSubDir);
  
  void _clearRays();
  void _clearSweeps();
  void _clearCals();
  void _clearFields();
  
  NcxxType _getNcxxType(Radx::DataType_t dtype);
  NcxxFile::FileFormat _getFileFormat(RadxFile::netcdf_format_t format);
  
  // private methods for NcfRadial_read.cc
  
  int _readPath(const string &path, size_t pathNum);
  int _getVolumePaths(const string &path, vector<string> &paths);
  void _addToPathList(const string &dir, const string &volStr,
                      int minHour, int maxHour, vector<string> &paths);

  int _loadSweepInfo(const vector<string> &paths);
  int _appendSweepInfo(const string &path);

  void _checkGeorefsActiveOnRead();
  void _checkCorrectionsActiveOnRead();

  int _readDimensions();
  int _readGlobalAttributes();
  int _readTimes(int pathNum);
  int _readRangeVariable();
  int _readScalarVariables();
  int _readCorrectionVariables();
  int _readPositionVariables();
  int _readSweepVariables();
  void _clearGeorefVariables();
  int _readGeorefVariables();
  void _clearRayVariables();
  int _readRayVariables();
  int _createRays(const string &path);
  int _readFrequencyVariable();
  void _readRayGateGeom();
  int _readRayNgatesAndOffsets();
  int _readCalibrationVariables();
  int _readCal(RadxRcalib &cal, int index);
  int _readFieldVariables(bool metaOnly);
  
  int _readRayVar(NcxxVar &var, const string &name, 
                  vector<double> &vals, bool required = true);
  int _readRayVar(NcxxVar &var, const string &name, 
                  vector<int> &vals, bool required = true);
  int _readRayVar(NcxxVar &var, const string &name, 
                  vector<bool> &vals, bool required = true);
  
  int _readRayVar(const string &name, 
                  vector<double> &vals, bool required = true);
  int _readRayVar(const string &name, 
                  vector<int> &vals, bool required = true);
  int _readRayVar(const string &name, 
                  vector<bool> &vals, bool required = true);
  
  int _getRayVar(NcxxVar &var, const string &name, bool required);
  int _readSweepVar(NcxxVar &var, const string &name,
                    vector<double> &vals, bool required = true);
  int _readSweepVar(NcxxVar &var, const string &name, 
                    vector<int> &vals, bool required = true);
  int _readSweepVar(NcxxVar &var, const string &name,
                    vector<string> &vals, bool required = true);
  int _getSweepVar(NcxxVar &var, const string &name);
  int _readCalTime(const string &name, NcxxVar &var, int index, time_t &val);
  int _readCalVar(const string &name, NcxxVar &var, int index,
                  double &val, bool required = false);

  int _addFl64FieldToRays(NcxxVar &var,
                          const string &name, const string &units,
                          const string &standardName, const string &longName,
                          bool isDiscrete, bool fieldFolds,
                          float foldLimitLower, float foldLimitUpper);
  int _addFl32FieldToRays(NcxxVar &var,
                          const string &name, const string &units,
                          const string &standardName, const string &longName,
                          bool isDiscrete, bool fieldFolds,
                          float foldLimitLower, float foldLimitUpper);
  int _addSi32FieldToRays(NcxxVar &var,
                          const string &name, const string &units,
                          const string &standardName, const string &longName,
                          double scale, double offset,
                          bool isDiscrete, bool fieldFolds,
                          float foldLimitLower, float foldLimitUpper);
  int _addSi16FieldToRays(NcxxVar &var,
                          const string &name, const string &units,
                          const string &standardName, const string &longName,
                          double scale, double offset,
                          bool isDiscrete, bool fieldFolds,
                          float foldLimitLower, float foldLimitUpper);
  int _addSi08FieldToRays(NcxxVar &var,
                          const string &name, const string &units,
                          const string &standardName, const string &longName,
                          double scale, double offset,
                          bool isDiscrete, bool fieldFolds,
                          float foldLimitLower, float foldLimitUpper);

  void _loadReadVolume();

  // private methods for NcfRadial_write.cc
  
  void _checkGeorefsActiveOnWrite();
  void _checkCorrectionsActiveOnWrite();

  int _addGlobalAttributes();
  int _addDimensions();

  int _addCoordinateVariables();
  int _addScalarVariables();
  int _addFrequencyVariable();
  int _addCorrectionVariables();
  int _addProjectionVariables();
  int _addSweepVariables();
  int _addCalibVariables();
  int _addRayVariables();
  void _setEstNoiseAvailFlags();
  int _addGeorefVariables();

  int _addCalVar(NcxxVar &var, const string &name, 
                 const string &standardName,
                 const string &units = "");
    
  int _writeCoordinateVariables();
  int _writeScalarVariables();
  int _writeCorrectionVariables();
  int _writeProjectionVariables();
  int _writeRayVariables();
  int _writeGeorefVariables();
  int _writeSweepVariables();
  int _writeCalibVariables();
  int _writeFrequencyVariable();

  int _writeFieldVariables();
  NcxxVar _addFieldVar(const RadxField &field);
  int _writeFieldVar(NcxxVar &var, RadxField *field);
  int _closeOnError(const string &caller);

  int _setCompression(NcxxVar &var);
  void _computeFixedAngles();

  Radx::fl64 _checkMissingDouble(double val);
  Radx::fl32 _checkMissingFloat(float val);
  
  string _computeWritePath(const RadxVol &vol,
                           const RadxTime &startTime,
                           int startMillisecs,
                           const RadxTime &endTime,
                           int endMillisecs,
                           const RadxTime &fileTime,
                           int fileMillisecs,
                           const string &dir);

};

#endif
