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
// DsrGrabber.hh
//
// DsrGrabber object
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2007
//
///////////////////////////////////////////////////////////////
//
// DsrGrabber reads an input radar FMQ, and
// writes CP2Moments UDP data
//
///////////////////////////////////////////////////////////////////////

#ifndef DsrGrabber_HH
#define DsrGrabber_HH

#include <cstdio>
#include <string>
#include <vector>
#include <algorithm>
#include <functional>
#include <rapformats/DsRadarMsg.hh>
#include <Fmq/DsRadarQueue.hh>
#include <didss/DsInputPath.hh>
#include <euclid/SunPosn.hh>
#include <toolsa/DateTime.hh>

#include "Args.hh"
#include "Params.hh"
#include "GridLoc.hh"
#include "Beam.hh"

using namespace std;

////////////////////////
// This class

class DsrGrabber {
  
public:

  // constructor

  DsrGrabber (int argc, char **argv);

  // destructor
  
  ~DsrGrabber();

  // run 

  int Run();

  // data members

  bool isOK;

protected:
  
private:

  typedef struct {
    double maxSunPower;
    double quadSunPower;
    double centroidAzOffset;
    double centroidElOffset;
  } sun_props_t;
    
  // members

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;
  SunPosn _sunPosn;
  Beam _latestBeam;

  // field names
  
  vector<string> _fieldNames;
  
  // FMQ input

  DsRadarQueue _inputQueue;
  DsRadarMsg _inputMsg;
  int _inputContents;

  // FILE input

  DsInputPath *_dsInput;
  char *_inputPath;
  FILE *_inFile;
  bool _done;

  // keeping track of number of beams in the volume

  int _nBeamsVol;

  // latest data time

  time_t _latestTime;
  time_t _volStartTime;
  time_t _volEndTime;
  time_t _fileTime;
  
  // individual beams
  
  vector<string> _beamStrings;
  
  // regular grid

  int _gridNEl, _gridNAz;
  double _gridMinAz, _gridMinEl;
  double _gridMaxAz, _gridMaxEl;
  double _gridDeltaEl, _gridDeltaAz;

  vector<GridLoc *> _gridLocs;

  // minimum values

  vector<double> _minima;

  // field indexes

  int _diffIndexFirst;
  int _diffIndexSecond;

  // sun centroid

  vector<sun_props_t> _sunProps;

  // functions
  
  int _run();
  
  int _readBeam();
  int _readBeamFromFmq();
  int _readBeamFromFile();

  int _openNextFile();
  void _closeInputFile();

  int _processBeam();
  
  int _loadLatestBeamFromFmq();

  void _handleEndOfVol();

  double _computeFieldMean(const string &fieldName,
			   int dsrFieldNum,
			   const DsRadarParams &radarParams,
			   const DsFieldParams &fieldParams,
			   const DsRadarBeam &radarBeam);

  bool _endOfVol();

  string _formatOutputLine(const Beam &beam);

  double _getAz(const DsRadarMsg &radarMsg);
  double _getEl(const DsRadarMsg &radarMsg);
  time_t _getTime(const DsRadarMsg &radarMsg);

  void _addBeamToGrid(const Beam &beam);
  void _interpolateGrid();
  void _clearGridBeams();
  void _initMinima();
  void _computeMinima(const vector<double> &fields);

  double _computeMaxSunPower(int fieldNum);

  void _computeSunProps(int fieldNum);

  int _quadFit(int n,
	       const vector<double> &x,
	       const vector<double> &y,
	       double &cc,
	       double &bb,
	       double &aa,
	       double &std_error_est,
	       double &r_squared);

  int _writeBeamsFile();

  int _writeGridFiles();

  int _writeGridField(const char *subdirName,
		      const char *subdirPath,
		      const DateTime &btime,
		      int fieldNum,
		      const string &fieldName);

  int _writeSunpropsFile();

};

#endif

