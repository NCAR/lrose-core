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
// OdimHdf5RadxFile.hh
//
// ODIM Hdf5 data for radar radial data
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 2013
//
///////////////////////////////////////////////////////////////

#ifndef OdimHdf5RadxFile_HH
#define OdimHdf5RadxFile_HH

#include <string>
#include <vector>

#include <Radx/Radx.hh>
#include <Radx/RadxFile.hh>
#include <Radx/RadxRangeGeom.hh>
#include <Radx/RadxRemap.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxRay.hh>
#include <Ncxx/Hdf5xx.hh>

class RadxVol;
using namespace std;

///////////////////////////////////////////////////////////////
/// FILE IO CLASS FOR NETCDF CF/RADIAL FILE FORMAT
///
/// This subclass of RadxFile handles I/O for netcdf files.

class OdimHdf5RadxFile : public RadxFile

{
  
public:

  /// Constructor
  
  OdimHdf5RadxFile();
  
  /// Destructor
  
  virtual ~OdimHdf5RadxFile();
  
  /// clear all data
  
  virtual void clear();

  /// Check if specified file is a CfRadialFile
  /// Returns true on success, false on failure
  
  virtual bool isSupported(const string &path);
    
  /// Check if specified file is a CfRadial file.
  /// Returns true on success, false on failure
  
  bool isOdimHdf5(const string &path);
    
  //////////////////////////////////////////////////////////////
  /// \name Perform writing:
  //@{

  //////////////////////////////////////////////////////////////
  /// Write data from RadxVol volume to the specified directory.
  ///
  /// If addDaySubDir is true, a  subdir will be
  /// created with the name dir/yyyymmdd/
  ///
  /// If addYearSubDir is true, a subdir will be
  /// created with the name dir/yyyy/
  ///
  /// If both addDaySubDir and addYearSubDir are true,
  /// the subdir will be dir/yyyy/yyyymmdd/
  ///
  /// Returns 0 on success, -1 on failure
  ///
  /// Use getErrStr() if error occurs
  /// Use getDirInUse() for dir written
  /// Use getPathInUse() for path written

  virtual int writeToDir(const RadxVol &vol,
                         const string &dir,
                         bool addDaySubDir,
                         bool addYearSubDir);

  //////////////////////////////////////////////////////////////
  /// Write data from RadxVol volume to the specified path.
  ///
  /// Write data from volume to specified path
  ///
  /// Returns 0 on success, -1 on failure
  ///
  /// Use getErrStr() if error occurs
  /// Use getPathInUse() for path written
  
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

  //@}

protected:
private:

  // volume for writing
  
  H5File *_file;

  // hdf5 utilities

  Hdf5xx _utils;

  // basics

  Radx::InstrumentType_t _instrumentType;
  Radx::PlatformType_t _platformType;
  Radx::PrimaryAxis_t _primaryAxis;
  int _volumeNumber;
  int _nSweeps;
  
  // root attributes

  string _conventions;

  // root 'what' attributes
  
  string _objectStr;
  string _version;
  string _dateStr;
  string _timeStr;
  string _source;
  
  // root 'where' attributes

  double _latitudeDeg;
  double _longitudeDeg;
  double _altitudeKm;

  // 'how' attributes

  string _task;
  string _system;
  string _simulated;
  string _software;
  string _swVersion;

  string _polTypeStr;
  string _polModeStr;
  Radx::PolarizationMode_t _polMode;

  double _wavelengthM;
  double _frequencyHz;
  double _scanRateRpm;
  double _scanRateDegPerSec;
  
  double _pulseWidthUs;
  double _rxBandwidthMhz;
  
  double _lowPrfHz;
  double _highPrfHz;
  Radx::PrtMode_t _prtMode;
  double _prtRatio;
  
  double _txLossH, _txLossV;
  double _injectLossH, _injectLossV;
  double _rxLossH, _rxLossV;

  double _radomeLossOneWayH, _radomeLossOneWayV;
  double _antennaGainH, _antennaGainV;
  double _beamWidthH, _beamWidthV;
  double _radarConstantH;
  double _radarConstantV;
  double _dbz0H, _dbz0V;
  double _noiseH, _noiseV;

  double _gasAttenDbPerKm;
  double _nomTxPowerKw;
  double _unambigVelMps;

  double _powerDiff;
  double _phaseDiff;
  
  int _nSamples;
  string _azMethod;
  string _binMethod;

  double _pointAccEl;
  double _pointAccAz;

  bool _malfuncFlag;
  string _malfuncMsg;
  double _maxRangeKm;
  string _comment;

  double _sqiThreshold;
  double _csrThreshold;
  double _logThreshold;
  double _snrThreshold;

  double _peakPowerKw;
  double _avPowerKw;
  double _dynRangeDb;
  
  string _polarization;

  // sweep 'how' attributes

  int _nDataFields;
  int _nQualityFields;
  int _nFields; // _nDataFields + _nQualityFields
  Radx::SweepMode_t _sweepMode;

  // sweep 'where' attributes

  int _a1Gate;
  double _aStart;
  double _fixedAngleDeg;
  int _nGates;
  int _nRaysSweep;
  double _gateSpacingKm;
  double _startRangeKm;

  // sweep 'what' attributes

  string _product;
  string _fieldName;
  string _startDateStr;
  string _startTimeStr;
  string _endDateStr;
  string _endTimeStr;
  time_t _sweepStartSecs;
  time_t _sweepEndSecs;
  double _scale;
  double _offset;
  double _missingDataVal;
  double _lowDataVal;

  // sweep extended attributes

  string _sweepStatusXml;
  string _statusXml;
  
  // gate geometry variables
  
  bool _gateGeomVaries;

  // extended metadata on write

  bool _extendedMetaDataOnWrite;

  // arrays for angles and rays

  vector<double> _rayAz;
  vector<double> _rayEl;
  vector<double> _rayTime;
  vector<RadxRay *> _sweepRays;
  
  // methods

  // finalize read volume

  int _finalizeReadVolume();

  // HDF5 access

  int _readFromPath(const string &path, RadxVol &vol);

  int _getNSweeps(Group &root);
  int _getNFields(Group &sweep);
  void _clearSweepVars();
  int _readSweep(Group &root, int sweepNum);
  void _setStatusXml();
  void _setSweepStatusXml(int sweepNum);
  int _readRootSubGroups(Group &root);
  int _readRootWhat(Group &what);
  int _readRootWhere(Group &where);
  void _readHow(Group &how, const string &label);
  int _readSweepHow(Group &how, const string &label);
  int _readSweepWhat(Group &what, const string &label);
  int _readDataWhat(Group &what, const string &label);
  int _readSweepWhere(Group &where, const string &label);
  int _readRays(Group &sweep, int sweepNumber);
  void _createRaysForSweep(int sweepNumber);
  
  int _addFieldToRays(const char *label,
                      Group &sweep,
                      vector<RadxRay *> &rays, 
                      int fieldNum);

  int _loadFloatArray(DataSet &ds,
                      const string dsname,
                      int npoints,
                      double scale,
                      double offset,
                      Radx::fl32 *floatVals);

  void _loadSi08Field(DataSet &ds,
                      const string &fieldName,
                      const string &units,
                      const string &standardName,
                      const string &longName,
                      int nRays,
                      int nGates,
                      int nPoints,
                      double scale,
                      double offset,
                      vector<RadxRay *> &rays);
  
  void _loadSi16Field(DataSet &ds,
                      const string &fieldName,
                      const string &units,
                      const string &standardName,
                      const string &longName,
                      int nRays,
                      int nGates,
                      int nPoints,
                      double scale,
                      double offset,
                      vector<RadxRay *> &rays);
  
  void _loadSi32Field(DataSet &ds,
                      const string &fieldName,
                      const string &units,
                      const string &standardName,
                      const string &longName,
                      int nRays,
                      int nGates,
                      int nPoints,
                      double scale,
                      double offset,
                      vector<RadxRay *> &rays);
  
  void _loadFl32Field(DataSet &ds,
                      const string &fieldName,
                      const string &units,
                      const string &standardName,
                      const string &longName,
                      int nRays,
                      int nGates,
                      int nPoints,
                      vector<RadxRay *> &rays);

  void _loadFl64Field(DataSet &ds,
                      const string &fieldName,
                      const string &units,
                      const string &standardName,
                      const string &longName,
                      int nRays,
                      int nGates,
                      int nPoints,
                      vector<RadxRay *> &rays);

  void _lookupUnitsAndNames(const string &fieldName, 
                            string &units,
                            string &standardName,
                            string &longName);
    
  bool _isGematronikFieldFile(const string &path,
                              string &dateStr,
                              string &fieldName,
                              string &volName);
  
  int _readGemFieldFiles(const string &path,
                         const string &dateStr,
                         const string &fieldName,
                         RadxVol &vol);

  int _openFileForWriting(const string &writePath);
  void _closeFile();

  string _computeWritePath(const RadxVol &vol,
                           const RadxTime &startTime,
                           int startMillisecs,
                           const RadxTime &endTime,
                           int endMillisecs,
                           const RadxTime &fileTime,
                           int fileMillisecs,
                           const string &dir);

  int _doWrite(const RadxVol &vol,
               const string &tmpPath);

  int _writeSweep(RadxVol &sweepVol,
                  size_t isweep);

  int _writeField(RadxVol &sweepVol,
                  size_t isweep,
                  RadxField &field,
                  size_t ifield,
                  Group &dataset);

};

#endif
