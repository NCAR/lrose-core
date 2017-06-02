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
// GamicHdf5RadxFile.hh
//
// GAMIC Hdf5 data for radar radial data
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 2013
//
///////////////////////////////////////////////////////////////

#ifndef GamicHdf5RadxFile_HH
#define GamicHdf5RadxFile_HH

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

class GamicHdf5RadxFile : public RadxFile

{
  
public:

  /// Constructor
  
  GamicHdf5RadxFile();
  
  /// Destructor
  
  virtual ~GamicHdf5RadxFile();
  
  /// clear all data
  
  virtual void clear();

  /// Check if specified file is a CfRadialFile
  /// Returns true on success, false on failure
  
  virtual bool isSupported(const string &path);
    
  /// Check if specified file is a CfRadial file.
  /// Returns true on success, false on failure
  
  bool isGamicHdf5(const string &path);
    
  //////////////////////////////////////////////////////////////
  /// \name Perform writing:
  //@{

  //////////////////////////////////////////////////////////////
  /// Write data from RadxVol volume to the specified directory.
  ///
  /// This class is for reading only, to the write methods
  /// simply print a warning that they are not implemented.
  ///
  /// Returns 0 on success, -1 on failure

  virtual int writeToDir(const RadxVol &vol,
                         const string &dir,
                         bool addDaySubDir,
                         bool addYearSubDir);

  //////////////////////////////////////////////////////////////
  /// Write data from RadxVol volume to the specified path.
  ///
  /// This class is for reading only, to the write methods
  /// simply print a warning that they are not implemented.
  ///
  /// Returns 0 on success, -1 on failure
  
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

  /// Get the date and time from a gamic file path.
  /// returns 0 on success, -1 on failure

  int getTimeFromPath(const string &path, RadxTime &rtime);
  
  //@}

protected:
private:

  // hdf5 utilities

  Hdf5xx _utils;

  // basics

  int _volumeNumber;
  int _sweepNumber;
  Radx::InstrumentType_t _instrumentType;
  Radx::PlatformType_t _platformType;
  Radx::PrimaryAxis_t _primaryAxis;

  // root 'how' attributes

  double _azBeamWidthDeg;
  double _elBeamWidthDeg;
  string _hostName;
  string _sdpName;
  string _sdpVersion;
  string _simulated;
  string _siteName;
  string _software;
  string _swVersion;
  string _templateName;

  // root 'what' attributes

  string _dateStr;
  string _objectStr;
  int _nSweeps;
  string _setsScheduled;
  string _version;

  // root 'where' attributes

  double _latitudeDeg;
  double _longitudeDeg;
  double _altitudeKm;

  // sweep 'how' attributes
  
  double _prfHz;
  double _angleStepDeg;
  bool _angleSync;
  int _nGates;
  int _clutterFilterNumber;
  double _angleStart;
  double _angleStop;
  double _fixedAngleDeg;
  int _malfunc;
  double _pulseWidthUs;
  double _wavelengthM;
  double _frequencyHz;
  double _maxRangeKm;
  int _nSamplesRange;
  int _nSamplesTime;
  double _startRangeKm;
  double _gateSpacingKm;
  int _nRaysSweep;
  double _scanRateDegPerSec;
  string _sweepTimeStamp;
  Radx::PrtMode_t _prtMode;
  double _prtRatio;

  // sweep 'what' attributes

  string _product;
  string _scanTypeStr;
  bool _isRhi;
  int _nFields;

  // sweep extended attributes

  string _sweepStatusXml;
  double _unambigVel;
  string _statusXml;
  
  // finalize read volume

  int _finalizeReadVolume();

  // HDF5 access

  int _readRootSubGroups(Group &root);
  int _readRootHow(Group &how);
  int _readRootWhat(Group &what);
  int _readRootWhere(Group &where);
  int _readSweep(Group &root, int sweepNum);
  void _clearSweepVars();
  int _readSweepHow(Group &how, int sweepNum);
  int _readSweepWhat(Group &what, int sweepNum);
  int _readSweepExtended(Group &extended);
  int _readRays(Group &sweep, int sweepNumber);
  int _loadRayMetadata(CompType compType, int iray, 
                       char *buf, RadxRay *ray);
  
  int _addFieldToRays(Group &sweep,
                      vector<RadxRay *> &rays, 
                      int fieldNum);

  int _loadFloatArray(DataSet &ds,
                      const string dsname,
                      int npoints,
                      double dynRangeMin,
                      double dynRangeMax,
                      Radx::fl32 *floatVals);

  void _loadSi08Field(DataSet &ds,
                      const string &fieldName,
                      const string &units,
                      const string &standardName,
                      const string &longName,
                      int nRays,
                      int nGates,
                      int nPoints,
                      double dynRangeMin,
                      double dynRangeMax,
                      vector<RadxRay *> &rays);
  
  void _loadSi16Field(DataSet &ds,
                      const string &fieldName,
                      const string &units,
                      const string &standardName,
                      const string &longName,
                      int nRays,
                      int nGates,
                      int nPoints,
                      double dynRangeMin,
                      double dynRangeMax,
                      vector<RadxRay *> &rays);
  
  void _loadSi32Field(DataSet &ds,
                      const string &fieldName,
                      const string &units,
                      const string &standardName,
                      const string &longName,
                      int nRays,
                      int nGates,
                      int nPoints,
                      double dynRangeMin,
                      double dynRangeMax,
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

  void _lookupStandardName(const string &fieldName, 
                           const string &units,
                           string &standardName,
                           string &longName);
    
};

#endif
