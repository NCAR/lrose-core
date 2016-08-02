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
// NcGeneric2Mdv.hh
//
// NcGeneric2Mdv object
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2008
//
///////////////////////////////////////////////////////////////
//
// NcGeneric2Mdv reads McIdas data in NetCDF format, and
// converts to MDV
//
////////////////////////////////////////////////////////////////

#ifndef NcGeneric2Mdv_H
#define NcGeneric2Mdv_H

#include <string>
#include <toolsa/TaArray.hh>
#include <didss/DsInputPath.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxProj.hh>
#include <Mdv/MdvxRemapLut.hh>
#include <netcdf.hh>
#include <netcdfcpp.h>
#include "Args.hh"
#include "Params.hh"
#include "SunAngle.hh"
using namespace std;

////////////////////////
// This class

class NcGeneric2Mdv {
  
public:

  // constructor

  NcGeneric2Mdv (int argc, char **argv);

  // destructor
  
  ~NcGeneric2Mdv();

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

  // NetCDF file

  NcFile *_ncFile;
  NcError *_ncErr;
  
  // NetCDF dimensions

  NcDim *_timeDim;
  NcDim *_zDim;
  NcDim *_yDim;
  NcDim *_xDim;

  // NetCDF coordinate variables

  NcVar *_baseTimeVar;
  NcVar *_timeOffsetVar;
  NcVar *_zVar;
  NcVar *_yVar;
  NcVar *_xVar;

  TaArray<float> _zArray_, _yArray_, _xArray_;
  float *_zArray, *_yArray, *_xArray;

  // NetCDF attributes

  string _source;
  string _history;

  // data set members
  
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

  // sun angle correction

  SunAngle _sunAngle;

  // private methods

  int _processFile(const char *input_path);
  void _initInputProjection();

  /// open netcdf file
  /// create error object so we can handle errors
  /// Returns 0 on success, -1 on failure

  int _openNcFile(const string &path);

  /// close netcdf file if open
  /// remove error object if it exists
  
  void _closeNcFile();

  // load up dimensions and variables

  int _loadMetaData();

  /// set MDV headers

  int _setMasterHeader(DsMdvx &mdvx, int itime);
  int _addDataFields(DsMdvx &mdvx, int itime);
  int _addDataField(NcVar *var, DsMdvx &mdvx, int itime);
  
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

  void _printFile(NcFile &ncf);
  void _printAtt(NcAtt *att);
  void _printVarVals(NcVar *var);

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

