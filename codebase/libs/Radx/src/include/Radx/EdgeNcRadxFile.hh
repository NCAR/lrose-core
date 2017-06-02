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
// EdgeNcRadxFile.hh
//
// EdgeNcRadxFile object
//
// NetCDF data for radar radial data converted from EDGE format
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// October 2016
//
///////////////////////////////////////////////////////////////

#ifndef EdgeNcRadxFile_HH
#define EdgeNcRadxFile_HH

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
/// FILE IO CLASS FOR EDGE NETCDF RADIAL FILE FORMAT
///
/// This subclass of RadxFile handles I/O for EDGE netcdf files.

class EdgeNcRadxFile : public RadxFile

{
  
public:

  /// Constructor
  
  EdgeNcRadxFile();
  
  /// Destructor
  
  virtual ~EdgeNcRadxFile();
  
  /// clear all data
  
  virtual void clear();

  /// Check if specified file is a EDGE file
  /// Returns true on success, false on failure
  
  virtual bool isSupported(const string &path);
    
  /// Check if specified file is a EDGE file.
  /// Returns true on success, false on failure
  
  bool isEdgeNc(const string &path);
    
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

  Nc3Dim *_azimuthDim;
  Nc3Dim *_gateDim;
  size_t _nAzimuthsInFile;
  size_t _nGatesInFile;

  // times
  
  RadxTime _startTime;
  vector<double> _dTimes;
  time_t _refTimeSecsFile;
  bool _rayTimesIncrease;
  
  // range

  vector<double> _rangeKm;
  bool _gateSpacingIsConstant;
  RadxRangeGeom _geom;
  RadxRemap _remap;
  bool _nGatesVary;

  // ray variables

  Nc3Var *_azimuthVar;
  Nc3Var *_beamWidthVar;
  Nc3Var *_gateWidthVar;

  vector<float> _azimuths;
  vector<float> _beamWidths;
  vector<float> _gateWidths;
  
  // global attributes

  string _TypeName_attr;
  string _DataType_attr;
  double _Latitude_attr;
  double _Longitude_attr;
  int _Height_attr;
  int _Time_attr;
  double _FractionalTime_attr;
  string _attributes_attr;
  string _NyquistVelocity_unit_attr;
  double _NyquistVelocity_value_attr;
  string _vcp_unit_attr;
  string _vcp_value_attr;
  string _radarName_unit_attr;
  string _radarName_value_attr;
  string _ColorMap_unit_attr;
  string _ColorMap_value_attr;
  double _Elevation_attr;
  string _ElevationUnits_attr;
  double _MissingData_attr;
  double _RangeFolded_attr;
  string _RadarParameters_attr;
  string _PRF_unit_attr;
  int _PRF_value_attr;
  string _PulseWidth_unit_attr;
  double _PulseWidth_value_attr;
  string _MaximumRange_unit_attr;
  double _MaximumRange_value_attr;
  string _ConversionPlugin_attr;

  string _title;
  string _institution;
  string _references;
  string _source;
  string _history;
  string _comment;
  string _statusXml;
  
  string _siteName;
  string _scanName;
  int _scanId;
  string _instrumentName;
  string _fieldName;

  double _nyquistMps;
  double _latitude;
  double _longitude;
  double _altitudeKm;
  double _maxRangeKm;
  
  double _elevationFixedAngle;
  double _prfHz;
  double _pulseWidthUs;

  double _missingDataValue;
  double _rangeFoldedValue;

  bool _firstFieldInSweep;
  int _volumeNumber;
  Radx::InstrumentType_t _instrumentType;
  Radx::PlatformType_t _platformType;
  Radx::PrimaryAxis_t _primaryAxis;

  // rays to be added to volume

  vector<RadxRay *> _rays;
  
  int _readSweepField(const string &sweepPath);
  int _readDimensions();
  int _readGlobalAttributes();
  void _clearRayVariables();
  int _readTimes();
  int _readRayVariables();
  int _createRays();
  void _setRangeArray();

  int _readRayVar(Nc3Var* &var, const string &name, 
                  vector<float> &vals, bool required = true);
  
  Nc3Var* _getRayVar(const string &name, bool required);
  
  int _readFieldVariable(bool metaOnly);

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

  void _initializeReadVolume();
  void _addRaysToVolume();
  int _finalizeReadVolume();

  void _getSecondaryFieldPaths(const string &primaryPath,
                               vector<string> &secondaryPaths);
  

};

#endif
