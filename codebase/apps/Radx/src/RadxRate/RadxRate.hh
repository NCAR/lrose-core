// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2018                                         
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
// RadxRate.hh
//
// RadxRate object
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2018
//
///////////////////////////////////////////////////////////////
//
// RadxRate reads moments from Radx-supported format files, 
// computes the precip rate and writes out the
// results to Radx-supported format files
//
///////////////////////////////////////////////////////////////

#ifndef RadxRate_H
#define RadxRate_H

#include "Args.hh"
#include "Params.hh"
#include <string>
#include <toolsa/TaThreadPool.hh>
#include <radar/KdpFiltParams.hh>
#include <radar/NcarPidParams.hh>
#include <radar/PrecipRateParams.hh>
#include <Radx/RadxVol.hh>
class RadxVol;
class RadxFile;
class RadxRay;
class RadxField;
class Worker;
class WorkerThread;
using namespace std;

class RadxRate {
  
public:

  // constructor
  
  RadxRate (int argc, char **argv);

  // destructor
  
  ~RadxRate();

  // run 

  int Run();

  // data members

  int OK;

  // get methods for threading

  const Params &getParams() const { return _params; }
  double getRadarHtKm() const { return _radarHtKm; }
  double getWavelengthM() const { return _wavelengthM; }

  // names for extra fields

  static string smoothedDbzFieldName;
  static string smoothedRhohvFieldName;
  static string elevationFieldName;
  static string rangeFieldName;
  static string beamHtFieldName;
  static string tempFieldName;
  static string pidFieldName;
  static string pidInterestFieldName;
  static string mlFieldName;
  static string mlExtendedFieldName;
  static string convFlagFieldName;
  
protected:
private:

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;
  PrecipRateParams _precipRateParams;
  NcarPidParams _ncarPidParams;
  KdpFiltParams _kdpFiltParams;
  vector<string> _readPaths;

  // radar volume container
  
  RadxVol _vol;

  // derived rays - after compute

  vector <RadxRay *> _derivedRays;

  // radar properties

  double _radarHtKm;
  double _wavelengthM;

  // mutex for debug prints
  
  pthread_mutex_t _debugPrintMutex;
  
  // instantiate thread pool for computations

  TaThreadPool _threadPool;
  vector<Worker *> _workers;

  // private methods
  
  void _printParamsRate();
  void _printParamsPid();
  void _printParamsKdp();

  int _runFilelist();
  int _runArchive();
  int _runRealtime();
  void _setupRead(RadxFile &file);
  void _setupWrite(RadxFile &file);
  int _processFile(const string &filePath);
  void _encodeFieldsForOutput();
  
  void _addExtraFieldsToInput();
  void _addExtraFieldsToOutput();

  int _compute();
  int _storeDerivedRay(WorkerThread *thread);

  int _writeVol();

};

#endif
