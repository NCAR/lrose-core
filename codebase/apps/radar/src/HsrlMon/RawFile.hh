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
// Oct 2017
//
///////////////////////////////////////////////////////////////

#ifndef RawFile_HH
#define RawFile_HH

#include <string>
#include <vector>

#include "MonField.hh"
#include <Radx/Radx.hh>
#include <Radx/RadxTime.hh>
#include <Ncxx/Nc3xFile.hh>
class RadxField;
class RadxVol;
class RadxRay;
class RadxSweep;
class RadxRcalib;
using namespace std;

#include "Params.hh"

///////////////////////////////////////////////////////////////
/// FILE IO CLASS FOR RAW HSRL NETCDF FILE FORMAT

class RawFile

{
  
public:

  /// Constructor
  
  RawFile(const Params &params);
  
  /// Destructor
  
  virtual ~RawFile();
  
  /// clear all data
  
  virtual void clear();
  
  // Open file
  // Returns true on success, false on failure
  
  int openFile(const string &path);
  
  // Close file

  void closeFile();
  
  /// Check if specified file is a raw HSRL file.
  /// Returns true on success, false on failure
  
  bool isRawHsrlFile(const string &path);
    
  /// Get the date and time from a dorade file path.
  /// returns 0 on success, -1 on failure

  static int getTimeFromPath(const string &path, RadxTime &rtime);

  /// get start and end times for data in file
  /// returns 0 on success, -1 on failure
  
  int getStartAndEndTimes(const string &filePath,
                          time_t &dataStartTime,
                          time_t &dataEndTime);

  // Opens specified path
  // Reads times
  // Remains open, ready for use
  // Returns 0 on success, -1 on failure
  
  int openAndReadTimes(const string &path);
  
  // get the index for a given time
  // returns -1 on error
  
  int getTimeIndex(time_t timeVal);

  // append to monitoring stats
  // returns 0 on success, -1 on failure
  
  int appendMonStats(MonField &monField,
                     int startTimeIndex,
                     int endTimeIndex);

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

  // dimensions

  Nc3Dim *_timeDim;
  Nc3Dim *_timeVecDim;
  Nc3Dim *_binCountDim;

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
  
  Nc3Var *_timeVar;
  vector<RadxTime> _dataTimes;
  vector<double> _dTimes;

  // georeference variables

  Nc3Var *_telescopeLockedVar;
  Nc3Var *_telescopeDirectionVar;

  Nc3Var *_latitudeVar;
  Nc3Var *_longitudeVar;
  Nc3Var *_altitudeVar;
  Nc3Var *_headingVar;
  Nc3Var *_gndSpeedVar;
  Nc3Var *_vertVelVar;
  Nc3Var *_pitchVar;
  Nc3Var *_rollVar;

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

  Nc3Var *_pollAngleVar;
  Nc3Var *_totalEnergyVar;

  vector<float> _polAngle;
  vector<int> _totalEnergy;

  // private methods for NcfRadial.cc
  
  int _readPath(const string &path, size_t pathNum);

  int _readDimensions();
  int _readGlobalAttributes();
  int _readTimes();

  int _readRayVar2Doubles(const string &name,
                          vector<double> &vals);

  int _readRayVar(Nc3Var* &var, const string &name, 
                  vector<double> &vals);
  int _readRayVar(Nc3Var* &var, const string &name, 
                  vector<float> &vals);
  int _readRayVar(Nc3Var* &var, const string &name, 
                  vector<int> &vals);
  int _readRayVar(Nc3Var* &var, const string &name, 
                  vector<bool> &vals);
  
  int _readRayVar(const string &name, 
                  vector<double> &vals);
  int _readRayVar(const string &name, 
                  vector<int> &vals);
  int _readRayVar(const string &name, 
                  vector<bool> &vals);
  
  Nc3Var* _getRayVar(const string &name);

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

