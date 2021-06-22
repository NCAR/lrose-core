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
// GpmHdf5ToMdv.hh
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2021
//
///////////////////////////////////////////////////////////////
//
// GpmHdf5ToMdv reads GPM data in HDF5 format, and
// converts to MDV.
// Originally based on NcGeneric2Mdv.
//
////////////////////////////////////////////////////////////////

#ifndef GpmHdf5ToMdv_H
#define GpmHdf5ToMdv_H

#include <string>
#include <toolsa/TaArray.hh>
#include <toolsa/DateTime.hh>
#include <didss/DsInputPath.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxProj.hh>
#include <Mdv/MdvxRemapLut.hh>
#include <Ncxx/Hdf5xx.hh>
#include "Args.hh"
#include "Params.hh"
using namespace std;

////////////////////////
// This class

class GpmHdf5ToMdv {
  
public:

  // constructor

  GpmHdf5ToMdv (int argc, char **argv);

  // destructor
  
  ~GpmHdf5ToMdv();

  // run 

  int Run();

  // data members

  bool isOK;

protected:
  
private:

  static const fl32 _missingFloat;

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;
  DsInputPath *_input;

  // data set members

  vector<DateTime> _times;
  DateTime _startTime, _endTime;
  size_t _nScans, _nRays, _nGates;

  string _fileHeader, _fileInfo;
  string _inputRecord, _jaxaInfo;
  string _navigationRecord, _history;

  vector<NcxxPort::si32> _dataQuality, _dataWarning;
  vector<NcxxPort::si32> _geoError, _geoWarning;
  vector<NcxxPort::si32> _limitErrorFlag, _missingScan;
  
  vector<NcxxPort::fl64> _lats;
  vector<NcxxPort::fl64> _lons;
  NcxxPort::fl64 _missingLat;
  NcxxPort::fl64 _missingLon;
  
  vector<NcxxPort::fl32> _dbzVals;
  NcxxPort::fl32 _missingDbz;
  string _dbzUnits;

  MdvxProj _inputProj;
  MdvxRemapLut _remapLut;
  int _nTimes;
  time_t _validTime;
  int _nx, _ny, _nz;
  double _minx, _miny, _minz;
  double _maxx, _maxy, _maxz;
  double _dx, _dy, _dz;
  bool _yIsReversed;
  bool _dxIsConstant, _dyIsConstant;

  int _nxValid, _nyValid;
  int _ixValidStart, _ixValidEnd;
  int _iyValidStart, _iyValidEnd;

  TaArray<float> _zArray_, _yArray_, _xArray_;
  float *_zArray, *_yArray, *_xArray;

  // hdf5 utilities

  Hdf5xx _utils;
  string _conventions;

  // private methods

  int _processFile(const char *input_path);
  void _initInputProjection();

  string _readStringAttribute(Group &group,
                              const string &attrName,
                              const string &context);
  
  int _readGroupNs(Group &ns);
  int _readTimes(Group &ns);
  int _readLatLon(Group &ns);
  int _readQcFlags(Group &ns);
  int _readReflectivity(Group &ns);

  // load up dimensions and variables

  int _loadMetaData();

  /// set MDV headers

  int _setMasterHeader(DsMdvx &mdvx, int itime);
  int _addDataFields(DsMdvx &mdvx, int itime);
  // int _addDataField(Nc3Var *var, DsMdvx &mdvx, int itime, bool xySwapped);
  
  MdvxField *_createMdvxField(const string &fieldName,
                              const string &longName,
                              const string &units,
                              int nx, int ny, int nz,
                              double minx, double miny, double minz,
                              double dx, double dy, double dz,
                              const float *vals);

  MdvxField *_createRegularLatlonField(const string &fieldName,
                                       const string &longName,
                                       const string &units,
                                       const float *vals);

  // void _printFile(Nc3File &ncf);
  // void _printAtt(Nc3Att *att);
  // void _printVarVals(Nc3Var *var);

  void _correctForSunAngle(MdvxField *field);

  void _remapOutput(DsMdvx &mdvx);
  void _autoRemapToLatLon(DsMdvx &mdvx);

  bool _checkDxIsConstant();
  bool _checkDyIsConstant();
  void _initMercatorFromInputCoords();
  int _findValidLatLonLimits();

  int _getClosestLatIndex(double latitude, double tolerance);
  int _getClosestLonIndex(double longitude, double tolerance);

};

#endif

