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
// NoaaFslRadxFile.hh
//
// NoaaFslRadxFile object
//
// NetCDF data for radar radial data NASA/CSU D3R netcdf format
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2018
//
///////////////////////////////////////////////////////////////

#ifndef NoaaFslRadxFile_HH
#define NoaaFslRadxFile_HH

#include <string>
#include <vector>

#include <Radx/Radx.hh>
#include <Radx/RadxFile.hh>
#include <Radx/RadxRangeGeom.hh>
#include <Radx/RadxRemap.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxRay.hh>
#include <Ncxx/Nc3xFile.hh>

class RadxField;
class RadxVol;
class RadxSweep;
class RadxRcalib;
using namespace std;

///////////////////////////////////////////////////////////////
/// FILE IO CLASS FOR NASA/CSU D3R RADIAL FILE FORMAT
///
/// This subclass of RadxFile handles I/O for netcdf files.

class NoaaFslRadxFile : public RadxFile

{
  
public:

  /// Constructor
  
  NoaaFslRadxFile();
  
  /// Destructor
  
  virtual ~NoaaFslRadxFile();
  
  /// clear all data
  
  virtual void clear();

  /// Check if specified file is a DOE file
  /// Returns true on success, false on failure
  
  virtual bool isSupported(const string &path);
    
  /// Check if specified file is a DOE file.
  /// Returns true on success, false on failure
  
  bool isNoaaFsl(const string &path);
    
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
  /// \name Checks
  //@{
  
  /// Check if number of gates vary in volume
  
  // bool getNGatesVary() const { return _nGatesVary; }
    
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

  // netcdf file
  
  Nc3xFile _file;

  // dimensions

  Nc3Dim *_timeDim;  // "radial"
  Nc3Dim *_rangeDim; // "bin"
  Nc3Dim *_sweepDim; // "sweep"

  // global att

  string _title;
  string _institution;
  string _references;
  string _source;
  string _history;
  string _comment;
  string _statusXml;
  
  // times
  
  double _refTimeSecsFile;
  vector<time_t> _rayTimes;
  vector<double> _dTimes;
  bool _rayTimesIncrease;
  size_t _nTimesInFile;
  
  // range

  vector<double> _rangeKm;
  size_t _nRangeInFile;
  bool _gateSpacingIsConstant;
  RadxRangeGeom _geom;
  RadxRemap _remap;

  // sweep

  size_t _nSweeps;
  vector<double> _elevList;

  // scalars

  int _sweepNumber;
  double _elevationAngle;
  double _latitudeDeg;
  double _longitudeDeg;
  double _altitudeM;
  double _startRangeM;
  double _gateSpacingM;
  double _nyquistVel;
  double _calibConst;
  double _radarConst;
  double _beamWidthH;
  double _beamWidthV;
  double _pulseWidthUsec;
  double _bandWidthHertz;

  double _sqiThresh;
  double _logThresh;
  double _sigThresh;
  double _csrThresh;

  int _dbtThreshFlag;
  int _dbzThreshFlag;
  int _velThreshFlag;
  int _widThreshFlag;

  // ray variables
  
  vector<double> _azimuth;
  vector<double> _elevation;
  string _azimuthUnits, _elevationUnits;

  // instrument

  static int _volumeNumber;
  Radx::InstrumentType_t _instrumentType;
  Radx::PlatformType_t _platformType;
  Radx::PrimaryAxis_t _primaryAxis;

  // rays to be added to volume

  vector<RadxRay *> _raysVol;

  // rays in file

  vector<RadxRay *> _raysFile;

  int _readAggregatePaths(const string &path);
  int _readFile(const string &path);
  void _getVolumePaths(const string &path, vector<string> &paths);

  int _readDimensions();
  int _readGlobalAttributes();
  int _readScalars();
  int _readSweepAngles();
  int _readTimes();
  void _clearRayVariables();
  int _readRayVariables();
  int _createRays(const string &path);
  int _readFieldVariables(bool metaOnly);
  
  int _readRayVar(const string &name, string &units,
                  vector<double> &vals, bool required = true);
  
  Nc3Var* _getRayVar(const string &name, bool required);

  int _addFl64FieldToRays(Nc3Var* var,
                          const string &name, const string &units,
                          const string &standardName, const string &longName);

  int _addFl32FieldToRays(Nc3Var* var,
                          const string &name, const string &units,
                          const string &standardName, const string &longName);

  int _addSi32FieldToRays(Nc3Var* var,
                          const string &name,
                          const string &units,
                          const string &standardName,
                          const string &longName,
                          double scaleFactor,
                          double addOffset);

  int _addSi16FieldToRays(Nc3Var* var,
                          const string &name,
                          const string &units,
                          const string &standardName,
                          const string &longName,
                          double scaleFactor,
                          double addOffset);

  int _addSi08FieldToRays(Nc3Var* var,
                          const string &name,
                          const string &units,
                          const string &standardName,
                          const string &longName,
                          double scaleFactor,
                          double addOffset);

  int _loadReadVolume();
  void _computeFixedAngles();

};

#endif
