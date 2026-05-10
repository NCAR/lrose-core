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
#include "RadarInterp.hh"
#include <string>
#include <deque>
#include <toolsa/TaThreadPool.hh>
#include <Radx/RadxVol.hh>
#include <radar/NoiseLocator.hh>
#include <radar/TempProfile.hh>
#include <radar/KdpFiltParams.hh>
#include <radar/NcarPidParams.hh>
#include <radar/PrecipRateParams.hh>
#include <radar/NcarParticleId.hh>
#include <radar/PrecipRate.hh>
#include <radar/ConvStratFinder.hh>
#include <radar/ConvStratParams.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxProj.hh>
#include <Mdv/MdvxRemapInterp.hh>
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

  bool OK;

  // get methods for threading

  const Params &getParams() const { return _params; }
  double getRadarHtKm() const { return _radarHtKm; }
  double getWavelengthM() const { return _wavelengthM; }

  // get radar field from type
  // returns null on error
  // error cannot happen if _checkRadarFields() succeeded
  
  Params::radar_field_t *getRadarField(Params::radar_field_type_t rftype);

  // get field names
  
  string getRadarInputName(Params::radar_field_type_t rftype);
  string getRadarOutputName(Params::radar_field_type_t rftype);

  // get model field from type
  // returns null on error
  // error cannot happen if _checkModelFields() succeeded
  
  Params::model_field_t *getModelField(Params::model_field_type_t mftype);

  string getModelInputName(Params::model_field_type_t mftype);
  string getModelOutputName(Params::model_field_type_t mftype);

  Params::model_field_type_t getModelTypeFromInputName(const string name);
  Params::model_field_type_t getModelTypeFromOutputName(const string name);

  // names for derived fields

  static string elevationFieldName;
  static string rangeFieldName;
  static string beamHtFieldName;
  static string tempFieldName;

  static string pidFieldName;
  static string pidInterestFieldName;

  static string rateZrFieldName;
  static string rateHybridFieldName;

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
  vector<Params::radar_field_type_t> _radarFieldTypes;
  vector<Params::model_field_type_t> _modelFieldTypes;

  KdpFiltParams _kdpFiltParams;
  NcarPidParams _ncarPidParams;
  PrecipRateParams _precipRateParams;
  ConvStratParams _convStratParams;


  // input radar data
  
  RadxVol _radarVol;
  bool _rhiMode;

  // radar properties

  double _radarHtKm;
  double _wavelengthM;

  // scalars rays - after computing scalars

  vector <RadxRay *> _scalarRays;

  // radar interpolation

  RadarInterp *_radarInterp;

  // target interpolation projection and vlevels

  Mdvx::coord_t _interpCoord;
  MdvxProj _interpProj;
  size_t _interpNpointsVol, _interpNpointsPlane;
  vector<double> _interpVlevels;

  // interpolation fields
  
  vector<BaseInterp::Field> _interpFields;
  vector<BaseInterp::Ray *> _interpRays;

  // model interpolation
  
  MdvxRemapInterp _modelRemap;
  DsMdvx _modelRawMdvx;
  DsMdvx _modelInterpMdvx;
  TempProfile _tempProfile;

  // mutex for debug prints
  
  pthread_mutex_t _debugPrintMutex;
  
  // instantiate thread pool for computations
  
  TaThreadPool _scalarsThreadPool;
  vector<ScalarsCompute *> _computeScalarsThreads;

  // PID
  
  NcarParticleId _pid;
  MdvxField *_pidField;
  MdvxField *_pidModeField;
  vector<fl32> _pidArray, _pidFilt;

  // PRECIP
  
  MdvxField *_rateZrField;
  MdvxField *_rateHybridField;
  MdvxField *_rateZrFiltField;
  MdvxField *_rateHybridFiltField;
  vector<fl32> _rateZrFilt, _rateHybridFilt;

  // Beam Blockage and terrain height
  
  DsMdvx _beamBlockMdvx;
  MdvxField *_extinctionField;
  MdvxField *_terrainHtField;
  bool _haveBeamBlock;

  // QPE fields

  MdvxField *_qpeZrField;
  MdvxField *_qpeHybridField;
  vector<fl32> _qpeZr, _qpeHybrid;

  // convective / stratiform split

  ConvStratFinder _convStrat;
  bool _convStratAvailable;
  
  // private methods

  int _runFilelist();
  int _runArchive();
  int _runRealtime();
  int _processFile(const string &radarPath);
  bool _fileNameValid(const string &radarPath);

  int _readFile(const string &radarPath);
  void _setupRead(RadxFile &file);

  void _addRayGeomFieldsToInput();

  int _computeScalars();
  int _storeScalarsRay(ScalarsThread *thread);
  int _mergeScalarsIntoReadVol();
  void _freeScalarRays();
  int _writeDebugPolarOutput();

  void _addAngleFields();
  void _addRangeField();
  void _addHeightField();
  void _addCoverageField();
  void _addTimeField();

  bool _isRhi();

  void _initInterpGrid();
  void _initInterp();
  void _initInterpFields();

  void _allocInterpToCart();
  void _freeInterpRays();
  void _loadInterpRays();
  void _checkInterpFields();

  int _readModel();
  int _computeTempProfile();
  void _interpModelToOutputGrid();

  int _readBeamBlock();

  int _computePid();
  int _computePrecip();
  int _computeQpe();

  int _computeConvStrat();
  void _addConvStratToOutput(OutputMdv &out);

  int _createGridTemplate();
  int _writeOutputMdv();

  BaseInterp::Field *_getInterpField(const string &name);

  static inline size_t cartIndex(size_t iz, size_t iy, size_t ix,
                                 size_t ny, size_t nx)
  {
    return (iz * ny + iy) * nx + ix;
  }
  
  void _medianFilter2D(const fl32 *input,
                       fl32 *output,
                       size_t nz, size_t ny, size_t nx,
                       int kernelSize,
                       fl32 missingVal = Radx::missingFl32,
                       bool copyEdges = true);
  
  void _modeFilterPid2D(const fl32 *input,
                        fl32 *output,
                        size_t nz, size_t ny, size_t nx,
                        int kernelSize,
                        fl32 missingVal = Radx::missingFl32,
                        bool copyEdges = true,
                        bool preserveCenterOnTie = true);
  
  void _printParamsRate();
  void _printParamsPid();
  void _printParamsKdp();
  void _printParamsConvStrat();

  void _initRadarFieldTypes();
  int _checkRadarFields();
  string _radarFieldType2Str(Params::radar_field_type_t rftype);

  void _initModelFieldTypes();
  int _checkModelFields();
  string _modelFieldType2Str(Params::model_field_type_t mftype);

};

#endif
