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
// EccoStats.hh
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2024
//
////////////////////////////////////////////////////////////////////
//
// EccoStats computes statistics from the Ecco output files.
// See the Ecco app for details.
//
/////////////////////////////////////////////////////////////////////

#ifndef EccoStats_H
#define EccoStats_H

#include <string>
#include <Mdv/MdvxField.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxChunk.hh>
#include <toolsa/TaArray.hh>
#include <didss/DsInputPath.hh>
#include "Args.hh"
#include "Params.hh"
using namespace std;

////////////////////////
// This class

class EccoStats {
  
public:

  // constructor

  EccoStats (int argc, char **argv);

  // destructor
  
  ~EccoStats();

  // run 

  int Run();

  // data members

  bool isOK;

protected:
  
private:

  static const fl32 _missingFl32;

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;

  DsInputPath *_eccoPaths;
  DsMdvx _eccoMdvx, _statsMdvx;
  DsMdvx _mrmsMdvx, _covMdvx;
  time_t _firstEccoTime, _lastEccoTime;

  MdvxProj _proj;
  int _inNx, _inNy;
  int _nx, _ny, _nz;
  double _minx, _miny, _minz;
  double _dx, _dy, _dz;
  int _agNx, _agNy;

  MdvxField *_eccoTypeField;
  MdvxField *_convectivityField;
  MdvxField *_terrainHtField;
  MdvxField *_waterFlagField;
  MdvxField *_mrmsDbzField;
  MdvxField *_covMinHtField;
  MdvxField *_covMaxHtField;
  MdvxField *_covHtFractionField;

  fl32 ***_stratCount, ***_mixedCount, ***_convCount;
  fl32 ***_stratLowCount, ***_stratMidCount, ***_stratHighCount;
  fl32 ***_convShallowCount, ***_convMidCount, ***_convDeepCount, ***_convElevCount;
  fl32 ***_stratLowConv, ***_stratMidConv, ***_stratHighConv, ***_mixedConv;
  fl32 ***_convShallowConv, ***_convMidConv, ***_convDeepConv, ***_convElevConv;
  fl32 ***_validCount, ***_totalCount;
  fl32 **_terrainHt, **_waterFlag;
  fl32 **_sumCovMinHt, **_sumCovMaxHt, **_sumCovHtFrac, **_countCov;

  double **_lat, **_lon;
  int **_hourOfDay; // hour of day index

  int _computeEccoStats();

  void _initArraysToNull();
  void _allocArrays();
  void _freeArrays();
  void _initForStats();

  void _loadTerrain();
  void _updateStats();

  int _readEcco(const char *path);
  void _initStatsFile();
  void _addFieldsToStats();
  int _writeStats();
  int _writeHourlyStats(int hour);
  
  int _computeCoverage();
  void _addCoverageFields();
  int _readMrms();
  int _readCoverage();
  
  MdvxField *_make3DField(fl32 ***data,
                          string fieldName,
                          string longName,
                          string units);
                                 
  MdvxField *_make2DField(fl32 **data,
                          string fieldName,
                          string longName,
                          string units);
                                 
  MdvxField *_computeFrac3DField(fl32 ***data,
                                 fl32 ***counts,
                                 string fieldName,
                                 string longName,
                                 string units);
                                 
  MdvxField *_computeFrac2DField(fl32 ***data,
                                 fl32 ***counts,
                                 string fieldName,
                                 string longName,
                                 string units);

  MdvxField *_makeMrms2DField(fl32 **data,
                              string fieldName,
                              string longName,
                              string units);
  
  MdvxField *_computeCov2DField(fl32 **sum,
                                fl32 **counts,
                                string fieldName,
                                string longName,
                                string units);
  
  MdvxField *_sumCountsField(fl32 ***counts,
                             string fieldName,
                             string longName,
                             string units);
  
  MdvxField *_makeHourlyField(int hour,
                              fl32 ***data,
                              string fieldName,
                              string longName,
                              string units);
                                 
  MdvxField *_computeHourlyFracField(int hour,
                                     fl32 ***data,
                                     fl32 ***counts,
                                     string fieldName,
                                     string longName,
                                     string units);
                                 
};

#endif
