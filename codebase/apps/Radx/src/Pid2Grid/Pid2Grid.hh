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
// Pid2Grid.hh
//
// Pid2Grid object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2012
//
///////////////////////////////////////////////////////////////
//
// Pid2Grid reads moments from Radx-supported format files,
// interpolates onto a Cartesian grid, and writes out the
// results to Cartesian files in MDV or NetCDF.
//
///////////////////////////////////////////////////////////////

#ifndef Pid2Grid_HH
#define Pid2Grid_HH

#include "Args.hh"
#include "Params.hh"
#include "CartInterp.hh"
#include <string>
#include <deque>
#include <toolsa/TaArray.hh>
#include <radar/NoiseLocator.hh>
#include <Radx/RadxVol.hh>
#include <Mdv/MdvxProj.hh>
#include <radar/KdpFiltParams.hh>
#include <radar/NcarPidParams.hh>
#include <radar/PrecipRateParams.hh>
#include <toolsa/TaThreadPool.hh>
class RadxFile;
class RadxRay;
class RadxField;
class PolarCompute;
class PolarThread;
using namespace std;

class Pid2Grid {
  
public:

  // constructor
  
  Pid2Grid (int argc, char **argv);

  // destructor
  
  ~Pid2Grid();

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

  // input data
  
  vector<string> _readPaths;
  RadxVol _readVol;
  bool _rhiMode;

  // radar properties

  double _radarHtKm;
  double _wavelengthM;

  // derived rays - after computing PID

  vector <RadxRay *> _derivedRays;

  // interpolation fields
  
  vector<Interp::Field> _interpFields;
  vector<Interp::Ray *> _interpRays;

  // interpolation objects

  CartInterp *_cartInterp;

  // mutex for debug prints
  
  pthread_mutex_t _debugPrintMutex;
  
  // instantiate thread pool for computations

  TaThreadPool _threadPool;
  vector<PolarCompute *> _computeThreads;

  // private methods

  int _runFilelist();
  int _runArchive();
  int _runRealtime();
  void _setupRead(RadxFile &file);
  int _processFile(const string &filePath);
  int _readFile(const string &filePath);
  void _checkFields(const string &filePath);

  void _addExtraFieldsToInput();
  void _addExtraFieldsToOutput();
  void _encodeFieldsForOutput();

  int _computeDerived();
  int _storeDerivedRay(PolarThread *thread);

  void _loadInterpRays();
  void _addGeometryFields();
  void _addTimeField();

  bool _isRhi();

  void _initInterpFields();
  void _allocCartInterp();
  void _freeInterpRays();


  void _printParamsRate();
  void _printParamsPid();
  void _printParamsKdp();

};

#endif
