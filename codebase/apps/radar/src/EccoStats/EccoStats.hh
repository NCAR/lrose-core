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

  fl32 ***_stratCount;
  fl32 ***_mixedCount;
  fl32 ***_convCount;
  
  fl32 ***_stratLowCount;
  fl32 ***_stratMidCount;
  fl32 ***_stratHighCount;
  
  fl32 ***_convShallowCount;
  fl32 ***_convMidCount;
  fl32 ***_convDeepCount;
  fl32 ***_convElevCount;
  
  fl32 ***_stratSumConv;
  fl32 ***_stratLowSumConv;
  fl32 ***_stratMidSumConv;
  fl32 ***_stratHighSumConv;
  
  fl32 ***_mixedSumConv;

  fl32 ***_convSumConv;
  fl32 ***_convShallowSumConv;
  fl32 ***_convMidSumConv;
  fl32 ***_convDeepSumConv;
  fl32 ***_convElevSumConv;
  
  fl32 ***_validCount;
  fl32 ***_totalCount;

  fl32 **_terrainHt;
  fl32 **_waterFlag;

  fl32 **_sumCovMinHt;
  fl32 **_sumCovMaxHt;
  fl32 **_sumCovHtFrac;
  fl32 **_countCov;

  double **_lat;
  double **_lon;
  
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
                          string units,
                          double missingVal = 0.0);
                                 
  MdvxField *_make2DField(fl32 **data,
                          string fieldName,
                          string longName,
                          string units,
                          double missingVal = 0.0);
                                 
  MdvxField *_computeMean3DField(fl32 ***data,
                                 fl32 ***counts,
                                 string fieldName,
                                 string longName,
                                 string units,
                                 double missingVal = 0.0);
                                 
  MdvxField *_computeMean2DField(fl32 ***data,
                                 fl32 ***counts,
                                 string fieldName,
                                 string longName,
                                 string units,
                                 double missingVal = 0.0);

  MdvxField *_makeMrms2DField(fl32 **data,
                              string fieldName,
                              string longName,
                              string units,
                              double missingVal = 0.0);
  
  MdvxField *_computeCov2DField(fl32 **sum,
                                fl32 **counts,
                                string fieldName,
                                string longName,
                                string units,
                                double missingVal = 0.0);
  
  MdvxField *_sumHourlyField(fl32 ***counts,
                             string fieldName,
                             string longName,
                             string units,
                             double missingVal = 0.0);
  
  MdvxField *_makeHourlyField(int hour,
                              fl32 ***data,
                              string fieldName,
                              string longName,
                              string units,
                              double missingVal = 0.0);
                                 
  MdvxField *_computeHourlyMeanField(int hour,
                                     fl32 ***data,
                                     fl32 ***counts,
                                     string fieldName,
                                     string longName,
                                     string units,
                                     double missingVal = 0.0);
                                 
};

#endif
