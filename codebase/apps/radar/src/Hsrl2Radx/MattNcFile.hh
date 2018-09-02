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
// MattNcFile.hh
//
// HSRL NetCDF data produced by Matt Haymann's python code
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2018
//
///////////////////////////////////////////////////////////////

#ifndef MattNcFile_HH
#define MattNcFile_HH

#include <string>
#include <vector>

#include <Radx/Radx.hh>
#include <Radx/RadxFile.hh>
#include <Radx/RadxVol.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxGeoref.hh>
#include <Radx/RadxRangeGeom.hh>
#include <Ncxx/Nc3xFile.hh>
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

class MattNcFile

{
  
public:

  /// Constructor
  
  MattNcFile(const Params &params);
  
  /// Destructor
  
  virtual ~MattNcFile();
  
  /// clear all data
  
  virtual void clear();
  
  /// Check if specified file is a Matt type file.
  /// Returns true on success, false on failure
  
  bool isMattNcFile(const string &path);
    
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
  
  // compute angles from georef

  static void computeRadarAngles(RadxGeoref &georef,
                                 RadxCfactors &corr,
                                 double &azimuthDeg,
                                 double &elevationDeg);
  
  // Read georeference from SPDB
  // Returns 0 on success, -1 on error
  
  static int readGeorefFromSpdb(string georefUrl,
                                time_t searchTime,
                                int searchMarginSecs,
                                bool debug,
                                RadxGeoref &radxGeoref);

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
  
  Nc3xFile _file;

  // output volume

  RadxVol *_readVol;
  
  // dimensions

  Nc3Dim *_timeDim;
  Nc3Dim *_rangeDim;

  size_t _nTimesInFile;
  size_t _nRangeInFile;
  ssize_t _nPoints;

  // global attributes

  string _history;
  string _project;
  
  // times

  RadxTime _startTime;
  Nc3Var *_timeVar;
  vector<RadxTime> _dataTimes;
  vector<double> _dTimes;

  // range geometry

  Nc3Var *_rangeVar;
  RadxRangeGeom _geom;
  bool _gateSpacingIsConstant;
  vector<double> _rangeKm;
  // double _startRangeKm;
  // double _gateSpacingKm;
  
  // polarization

  vector<double> _polAngle;
  
  // georeference variables
  
  vector<double> _telescopeDirection;
  vector<double> _lat;
  vector<double> _lon;
  vector<double> _alt;
  vector<double> _roll;
  vector<double> _pitch;
  vector<double> _heading;
  vector<double> _pressure;
  vector<double> _tas;
  vector<double> _temp;

  // metadata

  Radx::InstrumentType_t _instrumentType;
  Radx::PlatformType_t _platformType;
  Radx::PrimaryAxis_t _primaryAxis;

  // rays

  vector<RadxRay *> _rays;

  // extra attributes in XML

  string _statusXml;
  
  // private methods for NcfRadial.cc
  
  void _clearRays();
  void _clearFields();
  
  int _readPath(const string &path, size_t pathNum);

  int _readDimensions();
  int _readGlobalAttributes();
  int _readTimes();
  int _readRange();
  void _clearRayVariables();
  int _readRayVariables();

  int _readRayVar(const string &name, vector<double> &vals);
  Nc3Var* _getRayVar(const string &name, bool required);

  int _createRays(const string &path);

  void _loadReadVolume();

  int _readFieldVariables();

  int _addFl64FieldToRays(Nc3Var* var,
                          const string &name,
                          const string &units,
                          const string &description);
  
  int _addSi08FieldToRays(Nc3Var* var,
                          const string &name,
                          const string &units,
                          const string &description);

  /// add integer value to error string, with label

  void _addErrInt(string label, int iarg,
                  bool cr = true);

  /// add double value to error string, with label

  void _addErrDbl(string label, double darg,
                  string format = "%g", bool cr = true);

  /// add string value to error string, with label
  
  void _addErrStr(string label, string strarg = "",
                  bool cr = true);

};

#endif

