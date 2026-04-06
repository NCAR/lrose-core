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
// RadxCartDP.hh
//
// RadxCartDP object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2012
//
///////////////////////////////////////////////////////////////
//
// RadxCartDP reads moments from Radx-supported format files,
// interpolates onto a Cartesian grid, and writes out the
// results to Cartesian files in MDV or NetCDF.
//
///////////////////////////////////////////////////////////////

#ifndef RadxCartDP_HH
#define RadxCartDP_HH

#include "Args.hh"
#include "Params.hh"
#include "CartInterp.hh"
#include <string>
#include <deque>
#include <toolsa/TaArray.hh>
#include <radar/NoiseLocator.hh>
#include <Radx/RadxVol.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxProj.hh>
#include <Mdv/MdvxRemapInterp.hh>
#include <radar/TempProfile.hh>
#include <radar/KdpFiltParams.hh>
#include <radar/NcarPidParams.hh>
#include <radar/PrecipRateParams.hh>
#include <toolsa/TaThreadPool.hh>
class RadxFile;
class RadxRay;
class RadxField;
class ScalarsCompute;
class ScalarsThread;
using namespace std;

class RadxCartDP {
  
public:

  // constructor
  
  RadxCartDP (int argc, char **argv);

  // destructor
  
  ~RadxCartDP();

  // run 

  int Run();

  // data members

  int OK;

  // get methods for threading

  const Params &getParams() const { return _params; }
  double getRadarHtKm() const { return _radarHtKm; }
  double getWavelengthM() const { return _wavelengthM; }

  // get field names
  
  string getRadarInputName(Params::radar_field_type_t ftype);
  string getModelInputName(Params::model_field_type_t ftype);
  string getBeamBlockInputName(Params::bblock_field_type_t ftype);

  string getRadarOutputName(Params::radar_field_type_t ftype);
  string getModelOutputName(Params::model_field_type_t ftype);
  string getBeamBlockOutputName(Params::bblock_field_type_t ftype);

  // names for derived fields

  static string elevationFieldName;
  static string rangeFieldName;
  static string beamHtFieldName;
  static string tempFieldName;
  static string pidFieldName;
  static string pidInterestFieldName;
  static string mlFieldName;
  static string mlExtendedFieldName;
  static string convFlagFieldName;

  static string snrForPidFieldName;
  static string dbzForPidFieldName;
  static string zdrForPidFieldName;
  static string ldrForPidFieldName;
  static string rhohvForPidFieldName;
  static string phidpForPidFieldName;
  static string kdpForPidFieldName;
  static string zdrSdevForPidFieldName;
  static string phidpSdevForPidFieldName;

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
  
  RadxVol _readVol;
  bool _rhiMode;

  // radar properties

  double _radarHtKm;
  double _wavelengthM;

  // scalars rays - after computing scalars

  vector <RadxRay *> _scalarRays;

  // interpolation fields
  
  vector<BaseInterp::Field> _interpFields;
  vector<BaseInterp::Ray *> _interpRays;

  // target projection and vlevels

  MdvxProj _targetProj;
  vector<double> _targetVlevels;

  // model interpolation
  
  MdvxRemapInterp _modelRemap;
  DsMdvx _modelRawMdvx;
  DsMdvx _modelInterpMdvx;
  TempProfile _tempProfile;

  // radar interpolation

  CartInterp *_cartInterp;

  // mutex for debug prints
  
  pthread_mutex_t _debugPrintMutex;
  
  // instantiate thread pool for computations
  
  TaThreadPool _scalarsThreadPool;
  vector<ScalarsCompute *> _computeScalarsThreads;

  // private methods

  int _runFilelist();
  int _runArchive();
  int _runRealtime();
  int _processFile(const string &filePath);
  bool _fileNameValid(const string &filePath);

  int _readFile(const string &filePath);
  void _setupRead(RadxFile &file);

  void _addRayGeomFieldsToInput();

  int _computeScalars();
  int _storeScalarsRay(ScalarsThread *thread);
  int _mergeScalarsIntoReadVol();
  int _writeDebugPolarOutput();

  void _addAngleFields();
  void _addRangeField();
  void _addHeightField();
  void _addCoverageField();
  void _addTimeField();

  bool _isRhi();

  void _initTargetGrid();
  void _initInterp();
  void _initInterpFields();

  void _allocInterpToCart();
  void _freeInterpRays();
  void _loadInterpRays();
  void _checkInterpFields();

  int _readModel();
  int _computeTempProfile();
  void _interpModelToOutputGrid();

  int _writeOutputMdv();
  
  void _printParamsRate();
  void _printParamsPid();
  void _printParamsKdp();

};

#endif
