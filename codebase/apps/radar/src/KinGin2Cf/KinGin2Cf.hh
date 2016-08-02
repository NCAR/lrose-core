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
// KinGin2Cf.hh
//
// KinGin2Cf object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2012
//
///////////////////////////////////////////////////////////////

#ifndef KinGin2Cf_H
#define KinGin2Cf_H

#include "Args.hh"
#include "Params.hh"
#include <string>
#include <Radx/RadxVol.hh>
#include <Radx/RadxTime.hh>
#include <ncvalues.h>
#include <netcdf.hh>
class RadxFile;
using namespace std;

class KinGin2Cf {
  
public:

  // constructor
  
  KinGin2Cf (int argc, char **argv);

  // destructor
  
  ~KinGin2Cf();

  // run 

  int Run();

  // data members

  int OK;

protected:
private:

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;

  // reading
  
  NcFile *_file;
  NcError *_err;
  time_t _prevTime;

  // writing
  
  RadxVol _vol;
  vector<RadxRay *> _rays;
  vector<double> _ranges;
  RadxRangeGeom _geom;

  // NetCDF dimensions
  
  NcDim *_binsDim;
  NcDim *_raysDim;
  int _nBins, _nRaysSweep;

  // metadata

  int _volumeNumber;
  int _sweepNumber;

  // times
  
  time_t _startTimeSecs, _endTimeSecs;
  double _startNanoSecs, _endNanoSecs;

  vector<time_t> _sweepStartTime;
  vector<int> _nRaysPerSweep;
  
  // geometry

  double _startRangeKm;
  double _gateSpacingKm;

   // Attributes

  string _convention;
  int _binNumber;
  int _rayNumber;

  // scalar variables

  int _PDP_COR_ON;

  double _Radar_lat;
  double _Radar_lon;
  double _Radar_altitude;
  double _FREQ;
  double _PRF_HIGH;
  double _PRF_LOW;
  double _NOISE_H;
  double _NOISE_V;
  double _NYQVEL;
  double _A_CONST_AH;
  double _B_CONST_AH;
  double _A_CONST_ADP;
  double _B_CONST_ADP;
  double _SCANTIME;

  RadxTime _scanTime;

  // methods

  void _clear();
  int _runFilelist();
  int _runArchive();
  int _runRealtime();
  void _setupRead(RadxFile &file);
  int _processFile(const string &filePath);

  int _readFile(const string &filePath);
  int _openRead(const string &path);
  void _close();
  
  int _readDimensions();
  int _readDim(const string &name, NcDim* &dim);
  void _readGlobalAttributes();
  int _readScalarVariables();

  string _asString(const NcTypedComponent *component,
                   int index = 0);

  int _readIntVar(const string &name, int &val, bool required);
  int _readDoubleVar(const string &name, double &val, bool required);

  int _readRangeVariable();
  int _readRayVariables();

  int _readRayVar(const string &name, vector<double> &vals);
  NcVar* _getRayVar(const string &name);

  int _readFieldVariables();

  int _addFl32FieldToRays(NcVar* var,
                          const string &name,
                          const string &units,
                          const string &standardName,
                          const string &longName,
                          Params::output_encoding_t encoding);

  int _writeVolume();
  void _setupWrite(RadxFile &file);
  void _setRayTimes();
  void _computeFixedAngles();

};

#endif
