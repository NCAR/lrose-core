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
// MpdNcFile.hh
//
// MPD NetCDF data produced by Mpd Haymann's python code
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2018
//
///////////////////////////////////////////////////////////////

#ifndef MpdNcFile_HH
#define MpdNcFile_HH

#include <string>
#include <vector>

#include <Radx/Radx.hh>
#include <Radx/RadxFile.hh>
#include <Radx/RadxVol.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxGeoref.hh>
#include <Radx/RadxRangeGeom.hh>
#include <Ncxx/Ncxx.hh>
#include <Ncxx/NcxxFile.hh>
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

class MpdNcFile

{
  
public:

  /// Constructor
  
  MpdNcFile(const Params &params);
  
  /// Destructor
  
  virtual ~MpdNcFile();
  
  /// clear all data
  
  virtual void clear();
  
  /// Check if specified file is a Mpd type file.
  /// Returns true on success, false on failure
  
  bool isMpdNcFile(const string &path);
    
  //////////////////////////////////////////////////////////////
  /// Perform the read:
  
  /// Read in data file from specified path,
  /// load up volume object.
  /// Returns 0 on success, -1 on failure
  /// Use getErrStr() if error occurs
  
  int readFromPath(const string &path,
                   RadxVol &vol);
  
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

  // output volume

  RadxVol *_readVol;
  
  // dimensions

  NcxxDim _timeDim;
  NcxxDim _rangeDim;

  size_t _nTimesInFile;
  size_t _nRangeInFile;
  ssize_t _nPoints;

  // global attributes

  string _history;
  string _project;
  
  // times

  RadxTime _startTime;
  NcxxVar _timeVar;
  vector<RadxTime> _dataTimes;
  vector<double> _dTimes;

  // range geometry

  NcxxVar _rangeVar;
  RadxRangeGeom _geom;
  bool _gateSpacingIsConstant;
  vector<double> _rangeKm;

  // ray variables

  vector<double> _nSamples;

  // location

  double _latDeg, _lonDeg, _altM;

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
  int _readRayQualifierFields();
  void _clearRayVariables();
  int _readRayVariables();

  int _readRayVar(const string &name, vector<double> &vals);
  int _readRayVar(const string &name, vector<float> &vals);
  int _getRayVar(NcxxVar &var, const string &name, bool required);

  int _createRays(const string &path);

  void _loadReadVolume();
  void _convertPressureToHpa();

  int _readFieldVariablesAuto();
  int _readFieldVariablesSpecified();

  int _readFieldVariable(string inputName,
                         string outputName,
                         NcxxVar &var,
                         bool &gotStatus,
                         bool required = false);
  
  int _readMaskVar(const string &maskFieldName,
                   vector<int> &maskVals);

  int _addFl64FieldToRays(const NcxxVar &var,
                          const string &name,
                          const string &units,
                          const string &longName,
                          const string &standardName,
                          bool isQualifier);
  
  int _addFl32FieldToRays(const NcxxVar &var,
                          const string &name,
                          const string &units,
                          const string &longName,
                          const string &standardName,
                          bool isQualifier);
  
  int _addSi32FieldToRays(const NcxxVar &var,
                          const string &name,
                          const string &units,
                          const string &longName,
                          const string &standardName,
                          bool isQualifier);
  
  int _addMaskedFieldToRays(NcxxVar &var,
                            const string &name,
                            const string &units,
                            const string &description,
                            vector<int> &maskVals,
                            int maskValidValue);
    
  int _addRawFieldToRays(NcxxVar &var,
                         const string &name,
                         const string &units,
                         const string &description,
                         bool applyMask,
                         const vector<int> &maskVals,
                         int maskValidValue);

  int _addSi08FieldToRays(NcxxVar &var,
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

