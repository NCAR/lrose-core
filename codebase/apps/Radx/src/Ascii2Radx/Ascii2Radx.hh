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
// Ascii2Radx.hh
//
// Ascii2Radx object
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2016
//
///////////////////////////////////////////////////////////////

#ifndef Ascii2Radx_HH
#define Ascii2Radx_HH

#include "Args.hh"
#include "Params.hh"
#include <string>
#include <cstdio>
#include <Radx/RadxTime.hh>
#include <Radx/RadxArray.hh>
class RadxVol;
class RadxFile;
class RadxRay;
class VarTransform;
using namespace std;

class Ascii2Radx {
  
public:

  // constructor
  
  Ascii2Radx (int argc, char **argv);

  // destructor
  
  ~Ascii2Radx();

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
  vector<string> _readPaths;
  FILE *_inFile;

  vector<VarTransform *> _varTrans;

  int _volNum;
  int _sweepNum;

  int _year, _month, _day, _hour, _min, _sec;
  RadxTime _volStartTime;

  int _nSamples;
  int _nGates;
  int _nAz;
  int _nPtsData;

  double _latitude;
  double _longitude;
  double _altitudeM;

  double _antennaGain;
  double _beamWidth;

  double _antennaSpeedAz;
  double _antennaSpeedEl;

  double _frequencyHz;
  double _peakPowerWatts;
  double _prf;
  double _pulseWidthSec;
  double _rxBandWidthHz;

  double _noiseLevelDbm;
  double _dynamicRangeDb;

  double _gateSpacingM;
  double _startRangeM;

  double _azimuthResDeg;

  double _radarConstant;

  double _elevDeg;
  double _startAz;

  RadxArray<double> _fieldData_;
  double *_fieldData;

  int _runFilelist();
  int _runArchive();
  int _runRealtimeWithLdata();
  int _runRealtimeNoLdata();
  int _readFile(const string &filePath,
                RadxVol &vol);
  void _finalizeVol(RadxVol &vol);
  void _convertFields(RadxVol &vol);
  void _convertAllFields(RadxVol &vol);
  void _setupWrite(RadxFile &file);
  void _setGlobalAttr(RadxVol &vol);
  int _writeVol(RadxVol &vol);

  int _readBufrAscii(const string &readPath,
                     RadxVol &vol);

  int _readBufrMetaData();
  int _readBufrFieldData();

  int _readBufrMetaVariable(string varLabel, int &ival,
                            string precedingLabel = "");
  int _readBufrMetaVariable(string varLabel, double &dval,
                            string precedingLabel = "");
  int _readBufrDataValue(string varLabel, double &dval);
  

};

#endif
