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
// Cf2RadxFile.hh
//
// Cf2RadxFile object
//
// CfRadial 2
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2017
//
///////////////////////////////////////////////////////////////

#ifndef Cf2RadxFile_HH
#define Cf2RadxFile_HH

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
/// FILE IO CLASS FOR NETCDF CFRADIAL2 FILE FORMAT
///
/// This subclass of RadxFile handles I/O for netcdf files.

class Cf2RadxFile : public RadxFile, public RadxNcfStr

{
  
public:

  /// Constructor
  
  Cf2RadxFile();
  
  /// Destructor
  
  virtual ~Cf2RadxFile();
  
  /// clear all data
  
  virtual void clear();

  /// Check if specified file is a CfRadialFile
  /// Returns true on success, false on failure
  
  virtual bool isSupported(const string &path);
    
  /// Check if specified file is a CfRadialXx file.
  /// Returns true on success, false on failure
  
  bool isCfRadial2(const string &path);
    
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

  NcxxDim _timeDimSweep;
  NcxxDim _rangeDimSweep;
  NcxxDim _sweepDim;
  NcxxDim _frequencyDim;

  // flags

  bool _georefsActive;
  bool _georefsApplied;
  bool _correctionsActive;
  bool _fixedAngleFound;

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

  vector<double> _rayTimes;
  time_t _refTimeSecsFile;
  bool _rayTimesIncrease;

  int _volumeNumber;
  Radx::InstrumentType_t _instrumentType;
  Radx::PlatformType_t _platformType;
  Radx::PrimaryAxis_t _primaryAxis;

  double _latitude;
  double _longitude;
  double _altitudeM;
  double _altitudeAglM;

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

  vector<double> _rangeKm;
  size_t _nRangeInSweep;

  vector<double> _sweepTimes;
  vector<double> _sweepRangeKm;

  RadxRangeGeom _geomSweep;
  RadxRemap _remapSweep;
  bool _gateSpacingIsConstant;

  vector<RadxRay *> _raysVol, _raysFromFile;
  vector<RadxRcalib *> _rCals;
  vector<RadxField *> _fields;
  RadxCfactors _cfactors;

  // storing sweep information

  vector<string> _sweepGroupNames;
  vector<NcxxGroup> _sweepGroups;
  NcxxGroup _sweepGroup;
  vector<RadxSweep *> _sweeps;
  vector<RadxSweep *> _sweepsInFile;

  class SweepInfo {
  public:
    string path;
    int sweepNum;
    size_t indexInFile;
    double fixedAngle;
  };

  vector<SweepInfo> _sweepsOrig;
  vector<SweepInfo> _sweepsToRead;
  vector<RadxRay *> _sweepRays;

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
  vector<Radx::si64> _geoUnitNum;
  vector<Radx::si64> _geoUnitId;
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
  
  // private methods for NcfRadial_read.cc
  
  int _readPath(const string &path, size_t pathNum);
  int _getVolumePaths(const string &path, vector<string> &paths);
  void _addToPathList(const string &dir, const string &volStr,
                      int minHour, int maxHour, vector<string> &paths);

  int _loadSweepInfo(const vector<string> &paths);
  int _appendSweepInfo(const string &path);

  void _readRootDimensions();
  void _readGlobalAttributes();

  void _readTimes();
  void _readSweepTimes(NcxxGroup &group,
                       vector<double> &times);
  void _readSweepsMetaAsInFile();
  void _readSweepMeta(NcxxGroup &group, RadxSweep *sweep);

  void _readRootScalarVariables();
  void _readRadarParameters();
  void _readLidarParameters();
  void _readFrequency(NcxxGroup &group);
  void _readGeorefCorrections();
  void _readLocation();

  void _readRadarCalibration();
  void _readRcal(NcxxGroup &group, NcxxDim &dim,
                 RadxRcalib &cal, size_t index);
  void _readCalTime(NcxxGroup &group, NcxxDim &dim,
                    const string &name, size_t index, time_t &val);
  NcxxVar _readCalVar(NcxxGroup &group, NcxxDim &dim,
                      const string &name, size_t index,
                      double &val, bool required = false);
  
  void _readLidarCalibration();
  
  void _readSweeps();
  void _readSweep(RadxSweep *sweep);
  void _readSweepRange(NcxxGroup &group,
                       NcxxDim &dim,
                       vector<double> rangeKm);

  void _clearRayVariables();
  void _readRayVariables();

  void _clearGeorefVariables();
  void _readGeorefVariables();

  void _createSweepRays(const RadxSweep *sweep);

  void _readFieldVariables(bool metaOnly);
  
  NcxxVar _readRayVar(NcxxGroup &group, NcxxDim &dim, const string &name, 
                      vector<double> &vals, bool required = true);
  NcxxVar _readRayVar(NcxxGroup &group, NcxxDim &dim, const string &name, 
                      vector<int> &vals, bool required = true);
  NcxxVar _readRayVar(NcxxGroup &group, NcxxDim &dim, const string &name, 
                      vector<Radx::si64> &vals, bool required = true);
  NcxxVar _readRayVar(NcxxGroup &group, NcxxDim &dim, const string &name, 
                      vector<bool> &vals, bool required = true);
  
  void _addFl64FieldToRays(NcxxVar &var,
                           const string &name, const string &units,
                           const string &standardName, const string &longName,
                           bool isDiscrete, bool fieldFolds,
                           float foldLimitLower, float foldLimitUpper);
  void _addFl32FieldToRays(NcxxVar &var,
                           const string &name, const string &units,
                           const string &standardName, const string &longName,
                           bool isDiscrete, bool fieldFolds,
                           float foldLimitLower, float foldLimitUpper);
  void _addSi32FieldToRays(NcxxVar &var,
                           const string &name, const string &units,
                           const string &standardName, const string &longName,
                           double scale, double offset,
                           bool isDiscrete, bool fieldFolds,
                           float foldLimitLower, float foldLimitUpper);
  void _addSi16FieldToRays(NcxxVar &var,
                           const string &name, const string &units,
                           const string &standardName, const string &longName,
                           double scale, double offset,
                           bool isDiscrete, bool fieldFolds,
                           float foldLimitLower, float foldLimitUpper);
  void _addSi08FieldToRays(NcxxVar &var,
                           const string &name, const string &units,
                           const string &standardName, const string &longName,
                           double scale, double offset,
                           bool isDiscrete, bool fieldFolds,
                           float foldLimitLower, float foldLimitUpper);

  void _loadReadVolume();

  NcxxVar _read1DVar(NcxxGroup &group, NcxxDim &dim,
                     const string &name,
                     vector<double> &vals, bool required = true);
  NcxxVar _read1DVar(NcxxGroup &group, NcxxDim &dim,
                     const string &name, 
                     vector<int> &vals, bool required = true);
  NcxxVar _read1DVar(NcxxGroup &group, NcxxDim &dim,
                     const string &name,
                     vector<string> &vals, bool required = true);

  // preparation

  void _checkGeorefsActiveOnWrite();
  void _checkCorrectionsActiveOnWrite();
  void _setEstNoiseAvailFlags();

  // writing top level
  
  void _addGlobalAttributes();
  void _addRootDimensions();
  void _addRootScalarVariables();

  // parameters

  void _addRadarParameters();
  void _addLidarParameters();
  
  void _addFrequency(NcxxGroup &group);
  void _addGeorefCorrections();
  void _addLocation();
  void _addProjection();

  // writing

  void _addSweeps();

  void _addSweepAttributes(const RadxSweep *sweep,
                           NcxxGroup &sweepGroup);
  
  void _addSweepVariables(const RadxSweep *sweep,
                          RadxVol &sweepVol,
                          NcxxGroup &sweepGroup,
                          NcxxDim &timeDim,
                          NcxxDim &rangeDim);

  void _addSweepScalars(const RadxSweep *sweep,
                        NcxxGroup &sweepGroup);
  
  void _addSweepGeorefs(RadxVol &sweepVol,
                        NcxxGroup &sweepGroup,
                        NcxxDim &timeDim);
  
  void _addSweepMon(RadxVol &sweepVol,
                    NcxxGroup &sweepGroup,
                    NcxxDim &timeDim);
  
  void _addSweepFields(const RadxSweep *sweep,
                       RadxVol &sweepVol,
                       NcxxGroup &sweepGroup,
                       NcxxDim &timeDim,
                       NcxxDim &rangeDim);
  
  NcxxVar _createFieldVar(const RadxField &field,
                          NcxxGroup &sweepGroup,
                          NcxxDim &timeDim,
                          NcxxDim &rangeDim);

  void _writeFieldVar(NcxxVar &var, RadxField *field);
  
  void _addRadarCalibration();

  void _addCalVar(NcxxGroup &group,
                  NcxxDim &dim,
                  float *vals,
                  const string &name,
                  const string &standardName,
                  const string &units = "");
    
  int _closeOnError(const string &caller);

  int _setCompression(NcxxVar &var);
  void _computeFixedAngle(RadxSweep *sweep);

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
