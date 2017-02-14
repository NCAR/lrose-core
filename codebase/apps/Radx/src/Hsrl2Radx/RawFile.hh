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
// RawFile.hh
//
// UW Raw HSRL NetCDF data
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2017
//
///////////////////////////////////////////////////////////////

#ifndef RawFile_HH
#define RawFile_HH

#include <string>
#include <vector>

#include <Radx/Radx.hh>
#include <Radx/RadxFile.hh>
#include <Radx/RadxVol.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxGeoref.hh>
#include <Radx/NetcdfClassic.hh>
class RadxField;
class RadxVol;
class RadxRay;
class RadxSweep;
class RadxRcalib;
using namespace std;

#include "Params.hh"

///////////////////////////////////////////////////////////////
/// FILE IO CLASS FOR NETCDF CF/RADIAL FILE FORMAT
///
/// This subclass of RadxFile handles I/O for netcdf files.

class RawFile

{
  
public:

  /// Constructor
  
  RawFile(const Params &params);
  
  /// Destructor
  
  virtual ~RawFile();
  
  /// clear all data
  
  virtual void clear();
  
  /// Check if specified file is a raw HSRL file.
  /// Returns true on success, false on failure
  
  bool isRawHsrlFile(const string &path);
    
  //////////////////////////////////////////////////////////////
  /// Perform the read:
  
  /// Read in data file from specified path,
  /// load up volume object.
  /// Returns 0 on success, -1 on failure
  /// Use getErrStr() if error occurs
  
  int readFromPath(const string &path,
                   RadxVol &vol);
  
  /// Get the date and time from a dorade file path.
  /// returns 0 on success, -1 on failure

  int getTimeFromPath(const string &path, RadxTime &rtime);

  ////////////////////////
  /// \name Error string:
  //@{
  
  /// Clear error string.
  
  void clearErrStr() { _errStr.clear(); }
  
  /// Get the Error String.
  ///
  /// The contents are only meaningful if an error has returned.
  
  string getErrStr() const { return _errStr; }
  
  //@}

protected:
private:

  // app params

  const Params &_params;
  
  // error string for read errors

  string _errStr;
  
  // netcdf file
  
  NetcdfClassic _file;

  // output volume

  RadxVol *_readVol;
  
  // dimensions

  NcDim *_timeDim;
  NcDim *_timeVecDim;
  NcDim *_binCountDim;

  size_t _nTimesInFile;
  size_t _timeVecSize;
  size_t _nBinsInFile;
  
  size_t _nPoints;
  size_t _nBinsPerGate;
  size_t _nGates;

  // global attributes

  string _machType;
  string _hostName;
  string _userName;
  string _gitCommit;
  int _hsrlVersion;
  string _dataAdded;
  string _sourceSoftware;
  
  // times
  
  NcVar *_timeVar;
  vector<RadxTime> _dataTimes;
  vector<double> _dTimes;

  // georeference variables

  NcVar *_telescopeLockedVar;
  NcVar *_telescopeDirectionVar;

  NcVar *_latitudeVar;
  NcVar *_longitudeVar;
  NcVar *_altitudeVar;
  NcVar *_headingVar;
  NcVar *_gndSpeedVar;
  NcVar *_vertVelVar;
  NcVar *_pitchVar;
  NcVar *_rollVar;

  vector<int> _telescopeLocked;
  vector<int> _telescopeDirection;
  
  vector<double> _latitude;
  vector<double> _longitude;
  vector<double> _altitude;
  vector<double> _heading;
  vector<double> _gndSpeed;
  vector<double> _vertVel;
  vector<double> _pitch;
  vector<double> _roll;

  Radx::InstrumentType_t _instrumentType;
  Radx::PlatformType_t _platformType;
  Radx::PrimaryAxis_t _primaryAxis;

  // range geometry

  double _rawGateSpacingKm;
  double _startRangeKm;
  double _gateSpacingKm;
  
  vector<RadxRay *> _rays;

  // field variables


  
  // private methods for NcfRadial.cc
  
  void _clearRays();
  void _clearFields();
  
  int _readPath(const string &path, size_t pathNum);

  int _readDimensions();
  int _readGlobalAttributes();
  int _readTimes();
  void _clearRayVariables();
  int _readRayVariables();

  int _readRayVar(NcVar* &var, const string &name, 
                  vector<double> &vals, bool required = true);
  int _readRayVar(NcVar* &var, const string &name, 
                  vector<int> &vals, bool required = true);
  int _readRayVar(NcVar* &var, const string &name, 
                  vector<bool> &vals, bool required = true);
  
  int _readRayVar(const string &name, 
                  vector<double> &vals, bool required = true);
  int _readRayVar(const string &name, 
                  vector<int> &vals, bool required = true);
  int _readRayVar(const string &name, 
                  vector<bool> &vals, bool required = true);
  
  NcVar* _getRayVar(const string &name, bool required);

  int _createRays(const string &path);

  void _loadReadVolume();

  int _readFieldVariables();

  int _addCountFieldToRays(NcVar* var,
                           const string &name,
                           const string &units,
                           const string &longName);
  
  /// add integer value to error string, with label

  void _addErrInt(string label, int iarg,
                  bool cr = true);

  /// add double value to error string, with label

  void _addErrDbl(string label, double darg,
                  string format = "%g", bool cr = true);

  /// add string value to error string, with label
  
  void _addErrStr(string label, string strarg = "",
                  bool cr = true);

  // compute angles from georef

  void _computeRadarAngles(RadxGeoref &georef,
                           RadxCfactors &corr,
                           double &azimuthDeg,
                           double &elevationDeg);
  
};

#endif

