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
// NcfRadxFile.hh
//
// NcfRadxFile object
//
// NetCDF data for radar radial data in CF-compliant file
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2009
//
///////////////////////////////////////////////////////////////

#ifndef NcfRadxFile_HH
#define NcfRadxFile_HH

#include <string>
#include <vector>

#include <Radx/Radx.hh>
#include <Radx/RadxFile.hh>
#include <Radx/RadxNcfStr.hh>
#include <Radx/RadxRangeGeom.hh>
#include <Radx/RadxCfactors.hh>
#include <Radx/RadxRemap.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxGeoref.hh>
#include <Ncxx/Nc3xFile.hh>

class RadxField;
class RadxVol;
class RadxRay;
class RadxSweep;
class RadxRcalib;
using namespace std;

///////////////////////////////////////////////////////////////
/// FILE IO CLASS FOR NETCDF CF/RADIAL FILE FORMAT
///
/// This subclass of RadxFile handles I/O for netcdf files.

class NcfRadxFile : public RadxFile, public RadxNcfStr

{
  
public:

  /// Constructor
  
  NcfRadxFile();
  
  /// Destructor
  
  virtual ~NcfRadxFile();
  
  /// clear all data
  
  virtual void clear();

  /// Check if specified file is a CfRadialFile
  /// Returns true on success, false on failure
  
  virtual bool isSupported(const string &path);
    
  /// Check if specified file is a CfRadial file.
  /// Returns true on success, false on failure
  
  bool isCfRadial(const string &path);
    
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

  /// Get the date and time from a cfRadial file path.
  /// returns 0 on success, -1 on failure

  static int getTimeFromPath(const string &path, RadxTime &rtime);

  //@}

protected:
private:

  // string length

  static const int NCF_STRING_LEN_8 = 8;
  static const int NCF_STRING_LEN_32 = 32;
  typedef char String8_t[NCF_STRING_LEN_8];
  typedef char String32_t[NCF_STRING_LEN_32];

  // volume for writing
  
  const RadxVol *_writeVol; ///< volume from which data is written
  
  // format version

  string _conventions;
  string _subconventions;
  string _version;
  
  // netcdf file
  
  Nc3xFile _file;
  string _tmpPath;

  // dimensions

  Nc3Dim *_timeDim;
  Nc3Dim *_rangeDim;
  Nc3Dim *_nPointsDim;
  Nc3Dim *_sweepDim;
  Nc3Dim *_calDim;
  Nc3Dim *_stringLen8Dim;
  Nc3Dim *_stringLen32Dim;
  Nc3Dim *_statusXmlDim;
  Nc3Dim *_frequencyDim;

  // coordinate variables

  Nc3Var *_timeVar;
  Nc3Var *_rangeVar;

  // gate geometry variables
  
  Nc3Var *_rayNGatesVar;
  Nc3Var *_rayStartIndexVar;

  Nc3Var *_rayStartRangeVar;
  Nc3Var *_rayGateSpacingVar;
  bool _gateGeomVaries;

  // scalar variables
  
  Nc3Var *_volumeNumberVar;
  Nc3Var *_instrumentTypeVar;
  Nc3Var *_platformTypeVar;
  Nc3Var *_primaryAxisVar;
  Nc3Var *_statusXmlVar;
  Nc3Var *_startTimeVar;
  Nc3Var *_endTimeVar;

  // frequencies

  Nc3Var *_frequencyVar;

  // projection variables
  
  Nc3Var *_projVar;
  Nc3Var *_latitudeVar;
  Nc3Var *_longitudeVar;
  Nc3Var *_altitudeVar;
  Nc3Var *_altitudeAglVar;
  
  // operational modes

  Nc3Var *_polModeVar;
  Nc3Var *_prtModeVar;

  // sweep variables

  Nc3Var *_sweepNumberVar;
  Nc3Var *_sweepModeVar;
  Nc3Var *_sweepFollowModeVar;
  Nc3Var *_sweepFixedAngleVar;
  bool _fixedAnglesFound;
  Nc3Var *_targetScanRateVar;
  Nc3Var *_sweepStartRayIndexVar;
  Nc3Var *_sweepEndRayIndexVar;
  Nc3Var *_raysAreIndexedVar;
  Nc3Var *_rayAngleResVar;
  Nc3Var *_intermedFreqHzVar;

  // radar param variables

  Nc3Var *_radarAntennaGainHVar;
  Nc3Var *_radarAntennaGainVVar;
  Nc3Var *_radarBeamWidthHVar;
  Nc3Var *_radarBeamWidthVVar;
  Nc3Var *_radarRxBandwidthVar;

  // lidar param variables

  Nc3Var *_lidarConstantVar;
  Nc3Var *_lidarPulseEnergyJVar;
  Nc3Var *_lidarPeakPowerWVar;
  Nc3Var *_lidarApertureDiamCmVar;
  Nc3Var *_lidarApertureEfficiencyVar;
  Nc3Var *_lidarFieldOfViewMradVar;
  Nc3Var *_lidarBeamDivergenceMradVar;

  // calibration variables

  Nc3Var *_rCalTimeVar;
  Nc3Var *_rCalPulseWidthVar;
  Nc3Var *_rCalXmitPowerHVar;
  Nc3Var *_rCalXmitPowerVVar;
  Nc3Var *_rCalTwoWayWaveguideLossHVar;
  Nc3Var *_rCalTwoWayWaveguideLossVVar;
  Nc3Var *_rCalTwoWayRadomeLossHVar;
  Nc3Var *_rCalTwoWayRadomeLossVVar;
  Nc3Var *_rCalReceiverMismatchLossVar;
  Nc3Var *_rCalKSquaredWaterVar;
  Nc3Var *_rCalRadarConstHVar;
  Nc3Var *_rCalRadarConstVVar;
  Nc3Var *_rCalAntennaGainHVar;
  Nc3Var *_rCalAntennaGainVVar;
  Nc3Var *_rCalNoiseHcVar;
  Nc3Var *_rCalNoiseHxVar;
  Nc3Var *_rCalNoiseVcVar;
  Nc3Var *_rCalNoiseVxVar;
  Nc3Var *_rCalI0HcVar;
  Nc3Var *_rCalI0HxVar;
  Nc3Var *_rCalI0VcVar;
  Nc3Var *_rCalI0VxVar;
  Nc3Var *_rCalReceiverGainHcVar;
  Nc3Var *_rCalReceiverGainHxVar;
  Nc3Var *_rCalReceiverGainVcVar;
  Nc3Var *_rCalReceiverGainVxVar;
  Nc3Var *_rCalReceiverSlopeHcVar;
  Nc3Var *_rCalReceiverSlopeHxVar;
  Nc3Var *_rCalReceiverSlopeVcVar;
  Nc3Var *_rCalReceiverSlopeVxVar;
  Nc3Var *_rCalDynamicRangeHcVar;
  Nc3Var *_rCalDynamicRangeHxVar;
  Nc3Var *_rCalDynamicRangeVcVar;
  Nc3Var *_rCalDynamicRangeVxVar;
  Nc3Var *_rCalBaseDbz1kmHcVar;
  Nc3Var *_rCalBaseDbz1kmHxVar;
  Nc3Var *_rCalBaseDbz1kmVcVar;
  Nc3Var *_rCalBaseDbz1kmVxVar;
  Nc3Var *_rCalSunPowerHcVar;
  Nc3Var *_rCalSunPowerHxVar;
  Nc3Var *_rCalSunPowerVcVar;
  Nc3Var *_rCalSunPowerVxVar;
  Nc3Var *_rCalNoiseSourcePowerHVar;
  Nc3Var *_rCalNoiseSourcePowerVVar;
  Nc3Var *_rCalPowerMeasLossHVar;
  Nc3Var *_rCalPowerMeasLossVVar;
  Nc3Var *_rCalCouplerForwardLossHVar;
  Nc3Var *_rCalCouplerForwardLossVVar;
  Nc3Var *_rCalDbzCorrectionVar;
  Nc3Var *_rCalZdrCorrectionVar;
  Nc3Var *_rCalLdrCorrectionHVar;
  Nc3Var *_rCalLdrCorrectionVVar;
  Nc3Var *_rCalSystemPhidpVar;
  Nc3Var *_rCalTestPowerHVar;
  Nc3Var *_rCalTestPowerVVar;

  // ray variables

  Nc3Var *_azimuthVar;
  Nc3Var *_elevationVar;
  Nc3Var *_pulseWidthVar;
  Nc3Var *_prtVar;
  Nc3Var *_prtRatioVar;
  Nc3Var *_nyquistVar;
  Nc3Var *_unambigRangeVar;
  Nc3Var *_antennaTransitionVar;
  Nc3Var *_georefsAppliedVar;
  Nc3Var *_georefTimeVar;
  Nc3Var *_georefUnitNumVar;
  Nc3Var *_georefUnitIdVar;
  Nc3Var *_nSamplesVar;
  Nc3Var *_calIndexVar;
  Nc3Var *_xmitPowerHVar;
  Nc3Var *_xmitPowerVVar;
  Nc3Var *_scanRateVar;
  Nc3Var *_estNoiseDbmHcVar;
  Nc3Var *_estNoiseDbmVcVar;
  Nc3Var *_estNoiseDbmHxVar;
  Nc3Var *_estNoiseDbmVxVar;

  // georeference variables

  bool _georefsActive;
  bool _georefsApplied;

  // correction factor variables

  bool _correctionsActive;
  Nc3Var *_azimuthCorrVar;
  Nc3Var *_elevationCorrVar;
  Nc3Var *_rangeCorrVar;
  Nc3Var *_longitudeCorrVar;
  Nc3Var *_latitudeCorrVar;
  Nc3Var *_pressureAltCorrVar;
  Nc3Var *_altitudeCorrVar;
  Nc3Var *_ewVelCorrVar;
  Nc3Var *_nsVelCorrVar;
  Nc3Var *_vertVelCorrVar;
  Nc3Var *_headingCorrVar;
  Nc3Var *_rollCorrVar;
  Nc3Var *_pitchCorrVar;
  Nc3Var *_driftCorrVar;
  Nc3Var *_rotationCorrVar;
  Nc3Var *_tiltCorrVar;

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
  vector<int> _geoUnitNum;
  vector<int> _geoUnitId;
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

  // private methods for NcfRadial.cc
  
  int _writeSweepsToDir(const RadxVol &vol, const string &dir,
                        bool addDaySubDir, bool addYearSubDir);
  int _writeSweepToDir(const RadxVol &vol, const string &dir,
                       bool addDaySubDir, bool addYearSubDir);
  
  void _clearRays();
  void _clearSweeps();
  void _clearCals();
  void _clearFields();
  
  Nc3Type _getNc3Type(Radx::DataType_t dtype);
  Nc3File::FileFormat _getFileFormat(RadxFile::netcdf_format_t format);
  
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
  
  int _readRayVar(Nc3Var* &var, const string &name, 
                  vector<double> &vals, bool required = true);
  int _readRayVar(Nc3Var* &var, const string &name, 
                  vector<int> &vals, bool required = true);
  int _readRayVar(Nc3Var* &var, const string &name, 
                  vector<bool> &vals, bool required = true);
  
  int _readRayVar(const string &name, 
                  vector<double> &vals, bool required = true);
  int _readRayVar(const string &name, 
                  vector<int> &vals, bool required = true);
  int _readRayVar(const string &name, 
                  vector<bool> &vals, bool required = true);
  
  Nc3Var* _getRayVar(const string &name, bool required);
  int _readSweepVar(Nc3Var* &var, const string &name,
                    vector<double> &vals, bool required = true);
  int _readSweepVar(Nc3Var* &var, const string &name, 
                    vector<int> &vals, bool required = true);
  int _readSweepVar(Nc3Var* &var, const string &name,
                    vector<string> &vals, bool required = true);
  Nc3Var* _getSweepVar(const string &name, bool required);
  int _readCalTime(const string &name, Nc3Var* &var, int index, time_t &val);
  int _readCalVar(const string &name, Nc3Var* &var, int index,
                  double &val, bool required = false);

  int _addFl64FieldToRays(Nc3Var* var,
                          const string &name, const string &units,
                          const string &standardName, const string &longName,
                          bool isDiscrete, bool fieldFolds,
                          float foldLimitLower, float foldLimitUpper);
  int _addFl32FieldToRays(Nc3Var* var,
                          const string &name, const string &units,
                          const string &standardName, const string &longName,
                          bool isDiscrete, bool fieldFolds,
                          float foldLimitLower, float foldLimitUpper);
  int _addSi32FieldToRays(Nc3Var* var,
                          const string &name, const string &units,
                          const string &standardName, const string &longName,
                          double scale, double offset,
                          bool isDiscrete, bool fieldFolds,
                          float foldLimitLower, float foldLimitUpper);
  int _addSi16FieldToRays(Nc3Var* var,
                          const string &name, const string &units,
                          const string &standardName, const string &longName,
                          double scale, double offset,
                          bool isDiscrete, bool fieldFolds,
                          float foldLimitLower, float foldLimitUpper,
			  float samplingRatio = 1.0);
  int _addSi08FieldToRays(Nc3Var* var,
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
  int _addCalVar(Nc3Var* &var, const string &name, const string &standardName,
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
  Nc3Var *_createFieldVar(const RadxField &field);
  int _writeFieldVar(Nc3Var *var, RadxField *field);
  int _closeOnError(const string &caller);

  int _setCompression(Nc3Var *var);
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
