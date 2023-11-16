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
// Era5File.hh
//
// ERA5 reanalysis NetCDF data produced by CISL
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 2023
//
///////////////////////////////////////////////////////////////

#ifndef Era5File_HH
#define Era5File_HH

#include <string>
#include <vector>

#include <Ncxx/Ncxx.hh>
#include <Ncxx/NcxxFile.hh>
#include <toolsa/DateTime.hh>
using namespace std;

#include "Params.hh"

///////////////////////////////////////////////////////////////
/// FILE IO CLASS FOR ERA5 DATA IN NETCDF FILE FORMAT

class Era5File

{
  
public:

  /// Constructor
  
  Era5File(const Params &params);
  
  /// Destructor
  
  virtual ~Era5File();
  
  /// clear all data
  
  virtual void clear();
  
  /// Check if specified file is a Matt type file.
  /// Returns true on success, false on failure
  
  bool isEra5File(const string &path);
    
  //////////////////////////////////////////////////////////////
  /// Perform the read:
  
  /// Read in data file from specified path,
  /// Returns 0 on success, -1 on failure
  /// Use getErrStr() if error occurs
  
  int readFromPath(const string &path, int timeIndex);
  
  /// Get the date and time from a dorade file path.
  /// returns 0 on success, -1 on failure
  
  // int getTimeFromPath(const string &path, DateTime &dtime);
  
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
  
  NcxxFile _file;

  // dimensions

  NcxxDim _timeDim;
  NcxxDim _levelDim;
  NcxxDim _lonDim;
  NcxxDim _latDim;
  
  size_t _nTimesInFile;
  size_t _nLevels;
  size_t _nLat, _nLon;
  ssize_t _nPoints;

  // global attributes

  string _dataSource;
  string _history;
  string _datasetUrl;
  string _datasetDoi;
  
  // times

  DateTime _refTime;
  DateTime _startTime;
  NcxxVar _timeVar;
  vector<DateTime> _dataTimes;
  vector<int> _iTimes;
  int _hour;

  // lon/lat

  NcxxVar _lonVar, _latVar;
  vector<double> _lon, _lat;
  
  // levels
  
  NcxxVar _levelVar;
  vector<double> _level;
  
  // field data

  int _timeIndex;
  string _name;
  string _longName;
  string _shortName;
  string _units;
  float _fillValue;
  float _minValue, _maxValue;
  vector<float> _fieldData;

  // private methods
  
  int _readPath(const string &path, size_t pathNum);
  int _readDimensions();
  int _readGlobalAttributes();
  int _readTimes();
  int _readLatLon();
  int _readField(int timeIndex);
  int _readFieldVariable(string fieldName,
                         int timeIndex,
                         NcxxVar &var);

#ifdef JUNK
  int _readRayVar(const string &name, vector<double> &vals);
  int _readRayVar(const string &name, vector<float> &vals);
  int _getRayVar(NcxxVar &var, const string &name, bool required);

  int _readFieldVariablesAuto();
  int _readFieldVariablesSpecified();
  
  int _readFieldVariable(string inputName,
                         string outputName,
                         NcxxVar &var,
                         bool &gotStatus,
                         bool required = false,
                         bool applyMask = false,
                         const string maskName = "",
                         int maskValidValue = 0);
  
  int _readMaskVar(const string &maskFieldName,
                   vector<int> &maskVals);

  int _addFl64FieldData(NcxxVar &var,
                        const string &name,
                        const string &units,
                        const string &description);
  
  int _addFl32FieldData(NcxxVar &var,
                        const string &name,
                        const string &units,
                        const string &description);
#endif

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

