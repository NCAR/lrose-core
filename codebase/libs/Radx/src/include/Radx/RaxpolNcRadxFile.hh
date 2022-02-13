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
// RaxpolNcRadxFile.hh
//
// RaxpolNcRadxFile object
//
// NetCDF file data for radar radial data from OU Raxpol radar
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2022
//
///////////////////////////////////////////////////////////////

#ifndef RaxpolNcRadxFile_HH
#define RaxpolNcRadxFile_HH

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
/// FILE IO CLASS FOR DOE NETCDF RADIAL FILE FORMAT
///
/// This subclass of RadxFile handles I/O for DOE netcdf files.

class RaxpolNcRadxFile : public RadxFile

{
  
public:

  /// Constructor
  
  RaxpolNcRadxFile();
  
  /// Destructor
  
  virtual ~RaxpolNcRadxFile();
  
  /// clear all data
  
  virtual void clear();

  /// Check if specified file is a DOE file
  /// Returns true on success, false on failure
  
  virtual bool isSupported(const string &path);
    
  /// Check if specified file is a DOE file.
  /// Returns true on success, false on failure
  
  bool isRaxpolNc(const string &path);
    
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
  vector<string> _allPathsInDir;

  // new sweep or vol?

  bool _newSweep;
  bool _newVol;

  // dimensions
  
  Nc3Dim *_timeDim;
  Nc3Dim *_gateDim;

  // times
  
  size_t _nTimesSweep;
  vector<double> _dTimesSweep;
  time_t _refTimeSecsFile;
  bool _rayTimesIncrease;
  
  // range

  double _startRangeKm;
  double _gateSpacingKm;
  vector<double> _rangeKm;
  size_t _nGates;
  bool _gateSpacingIsConstant;
  RadxRangeGeom _geom;
  RadxRemap _remap;

  // georef variables
  
  double _latitudeDeg;
  double _longitudeDeg;
  double _altitudeKm;

  // ray variables

  Nc3Var *_azimuthVar;
  Nc3Var *_elevationVar;
  
  vector<float> _azimuths;
  vector<float> _elevations;
  
  // string global attributes

  string _DataType_attr;
  string _ScanType_attr;
  string _attributes_attr;
  string _Wavelength_unit_attr;
  string _Nyquist_Vel_unit_attr;
  string _radarName_unit_attr;
  string _radarName_value_attr;
  string _vcp_unit_attr;
  string _vcp_value_attr;
  string _ElevationUnits_attr;
  string _AzimuthUnits_attr;
  string _RangeToFirstGateUnits_attr;
  string _RadarParameters_attr;
  string _PRF_unit_attr;
  string _PulseWidth_unit_attr;
  string _MaximumRange_unit_attr;
  string _ProcessParameters_attr;
  string _RadarKit_VCP_Definition_attr;
  string _Waveform_attr;
  string _CreatedBy_attr;
  string _ContactInformation_attr;
  string _NCProperties_attr;

  // time global attributes

  double _Time_attr;
  float _FractionalTime_attr;

  // integer global attributes

  int _PRF_value_attr;

  // floatint pt global attributes

  float _Latitude_attr;
  double _LatitudeDouble_attr;
  float _Longitude_attr;
  double _LongitudeDouble_attr;
  float _Heading_attr;
  float _Height_attr ;
  float _Wavelength_value_attr;
  float _Nyquist_Vel_value_attr;
  float _Elevation_attr;
  float _Azimuth_attr;
  float _GateSize_attr;
  float _RangeToFirstGate_attr;
  float _MissingData_attr;
  float _RangeFolded_attr;
  float _PulseWidth_value_attr;
  float _MaximumRange_value_attr;
  float _NoiseH_ADU_attr;
  float _NoiseV_ADU_attr;
  float _SystemZCalH_dB_attr;
  float _SystemZCalV_dB_attr;
  float _SystemDCal_dB_attr;
  float _SystemPCal_Radians_attr;
  float _ZCalH1_dB_attr;
  float _ZCalV1_dB_attr;
  float _ZCalH2_dB_attr;
  float _ZCalV2_dB_attr;
  float _DCal1_dB_attr;
  float _DCal2_dB_attr;
  float _PCal1_Radians_attr;
  float _PCal2_Radians_attr;
  float _SNRThreshold_dB_attr;
  float _SQIThreshold_dB_attr;

  double _frequencyHz;
  double _prfHz;
  double _prtSec;
  double _nyquistMps;
  double _pulseWidthUsec;
  int _nSamples;
  Radx::SweepMode_t _sweepMode;

  // CfRadial
  
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

  int _volumeNumber;
  Radx::InstrumentType_t _instrumentType;
  Radx::PlatformType_t _platformType;
  Radx::PrimaryAxis_t _primaryAxis;

  // rays to be added to volume

  vector<RadxRay *> _sweepRays;
  vector<RadxRay *> _volRays;

  // methods

  int _readDimensions();
  int _readSweepField(const string &path,
                      size_t sweepNum,
                      const string &fieldNameFromFile);
  int _readGlobalAttributes();
  void _setTimes();
  void _setRangeGeometry();
  void _setPositionVariables();
  void _clearRayVariables();
  int _readRayVariables();
  int _createSweepRays(const string &path, size_t sweepNum);
  int _readFieldVariables(bool metaOnly,
                          string shortName,
                          string standardName,
                          string longName);

  int _readRayVar(Nc3Var* &var, const string &name, 
                  vector<double> &vals, bool required = true);
  int _readRayVar(Nc3Var* &var, const string &name, 
                  vector<float> &vals, bool required = true);
  int _readRayVar(Nc3Var* &var, const string &name, 
                  vector<int> &vals, bool required = true);
  
  Nc3Var* _getRayVar(const string &name, bool required);

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
                          bool isDiscrete, bool fieldFolds,
                          float foldLimitLower, float foldLimitUpper);

  int _addSi16FieldToRays(Nc3Var* var,
                          const string &name, const string &units,
                          const string &standardName, const string &longName,
                          bool isDiscrete, bool fieldFolds,
                          float foldLimitLower, float foldLimitUpper);

  int _loadReadVolume();
  void _computeFixedAngles();

  int _getAllPathsInDir(const string &primaryPath);
  
  int _getSweepPrimaryPaths(const string &primaryPath,
                            vector<string> &sweepPaths);

  int _getFieldPaths(const string &primaryPath,
                     vector<string> &fileNames,
                     vector<string> &fieldPaths,
                     vector<string> &fieldNames);

  string _getFieldName(const string &fileName);

  int _getFixedAngle(const string &fileName,
                     double &fixedAngle,
                     bool &isRhi);

};

#endif
