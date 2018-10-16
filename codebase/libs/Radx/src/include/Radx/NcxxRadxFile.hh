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
#include <Radx/RadxNcfStr.hh>
#include <Radx/RadxRangeGeom.hh>
#include <Radx/RadxCfactors.hh>
#include <Radx/RadxRemap.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxGeoref.hh>
#include <Ncxx/Ncxx.hh>

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

class NcxxRadxFile : public RadxFile, public RadxNcfStr

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

  /// Get the date and time from a CfRadial file path.
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
  NcxxVar _rCalKSquaredWaterVar;
  NcxxVar _rCalRadarConstHVar;
  NcxxVar _rCalRadarConstVVar;
  NcxxVar _rCalAntennaGainHVar;
  NcxxVar _rCalAntennaGainVVar;
  NcxxVar _rCalNoiseHcVar;
  NcxxVar _rCalNoiseHxVar;
  NcxxVar _rCalNoiseVcVar;
  NcxxVar _rCalNoiseVxVar;
  NcxxVar _rCalI0HcVar;
  NcxxVar _rCalI0HxVar;
  NcxxVar _rCalI0VcVar;
  NcxxVar _rCalI0VxVar;
  NcxxVar _rCalReceiverGainHcVar;
  NcxxVar _rCalReceiverGainHxVar;
  NcxxVar _rCalReceiverGainVcVar;
  NcxxVar _rCalReceiverGainVxVar;
  NcxxVar _rCalReceiverSlopeHcVar;
  NcxxVar _rCalReceiverSlopeHxVar;
  NcxxVar _rCalReceiverSlopeVcVar;
  NcxxVar _rCalReceiverSlopeVxVar;
  NcxxVar _rCalDynamicRangeHcVar;
  NcxxVar _rCalDynamicRangeHxVar;
  NcxxVar _rCalDynamicRangeVcVar;
  NcxxVar _rCalDynamicRangeVxVar;
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

  void _addCalVar(NcxxVar &var, const string &name, 
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
