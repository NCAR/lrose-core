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
#include <iostream>

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
  
  //////////////////////////////////////////////////////////////
  /// Perform the read:
  /// Read in data file from specified path,
  /// Returns 0 on success, -1 on failure
  /// Use getErrStr() if error occurs
  
  int readFromPath(const string &path, int timeIndex);
  
  // print data details after read
  
  void printData(ostream &out);

  ////////////////////////
  /// \name Error string:
  //@{
  
  /// Clear error string.
  
  void clearErrStr() { _errStr.clear(); }
  
  /// Get methods
  
  string getErrStr() const { return _errStr; }
  string getPathInUse() const { return _pathInUse; }

  size_t getNTimesInFile() const { return _nTimesInFile; }
  size_t getNLevels() const { return _nLevels; }
  size_t getNLat() const { return _nLat; }
  size_t getNLon() const { return _nLon; }
  size_t getNPointsPlane() const { return _nPointsPlane; }
  size_t getNPointsVol() const { return _nPointsVol; }
  
  string getDataSource() const { return _dataSource; }
  string getHistory() const { return _history; }
  string getDatasetUrl() const { return _datasetUrl; }
  string getDatasetDoi() const { return _datasetDoi; }
  
  const DateTime &getRefTime() const { return _refTime; }
  const DateTime &getStartTime() const { return _startTime; }
  const vector<DateTime> &getDataTimes() const { return _dataTimes; }
  const vector<int> &getITimes() const { return _iTimes; }

  const vector<double> &getLon() const { return _lon; }
  const vector<double> &getLat() const { return _lat; }
   double getLevel(int index) const {
    if (index < (int) _levels.size()) {
      return _levels[index];
    } else {
      return -9999.0;
    }
  }
  const vector<double> &getLevels() const { return _levels; }
  int getTimeIndex() const { return _timeIndex; }
  string getFieldName() const { return _fieldName; }
  string getLongName() const { return _longName; }
  string getShortName() const { return _shortName; }
  string getUnits() const { return _units; }
  float getFillValue() const { return _fillValue; }
  float getMinValue() const { return _minValue; }
  float getMaxValue() const { return _maxValue; }
  const vector<float> &getFieldData() const { return _fieldData; }

  //@}

protected:
private:

  // app params

  const Params &_params;
  
  // error string for read errors

  string _errStr;
  
  // netcdf file
  
  NcxxFile _file;
  string _pathInUse;

  // dimensions

  NcxxDim _timeDim;
  NcxxDim _levelDim;
  NcxxDim _lonDim;
  NcxxDim _latDim;
  
  size_t _nTimesInFile;
  size_t _nLevels;
  size_t _nLat, _nLon;
  size_t _nPointsPlane;
  size_t _nPointsVol;

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

  // lon/lat

  NcxxVar _lonVar, _latVar;
  vector<double> _lon, _lat;
  
  // levels
  
  NcxxVar _levelVar;
  vector<double> _levels;
  
  // field data

  int _timeIndex;
  string _fieldName;
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
  int _readLevels();
  int _readField(int timeIndex);
  int _readFieldVariable(string fieldName,
                         int timeIndex,
                         NcxxVar &var);

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

