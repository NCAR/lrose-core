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
// Worker.hh
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2018
//
///////////////////////////////////////////////////////////////
//
// Worker computation engine
// There is one object per thread.
//
///////////////////////////////////////////////////////////////

#ifndef Worker_HH
#define Worker_HH

#include "Params.hh"
#include <radar/KdpFilt.hh>
#include <radar/KdpFiltParams.hh>
#include <radar/AtmosAtten.hh>
#include <Radx/RadxArray.hh>
#include <Radx/RadxTime.hh>
class RadxRay;
class RadxField;
#include <pthread.h>
using namespace std;

class Worker {
  
public:
  
  // constructor
  
  Worker(const Params &params,
         const KdpFiltParams &kdpFiltParams,
         int id);

  // destructor
  
  ~Worker();

  // Creates derived fields ray and returns it.
  // It must be freed by caller.
  //
  // Returns NULL on error.
  
  RadxRay *compute(RadxRay *inputRay,
                   double wavelengthM);

  bool OK;
  
protected:
private:

  static const double missingDbl;
  
  // parameters

  const Params &_params;
  const KdpFiltParams &_kdpFiltParams;

  int _id; // thread ID
  
  // current ray properties
  
  time_t _timeSecs;
  double _nanoSecs;
  double _azimuth;
  double _elevation;
  
  // radar properties

  double _wavelengthM;

  // geometry
  
  size_t _nGates;
  double _startRangeKm, _gateSpacingKm;
  
  // input arrays

  RadxArray<double> _snrArray_;
  RadxArray<double> _dbzArray_;
  RadxArray<double> _zdrArray_;
  RadxArray<double> _rhohvArray_;
  RadxArray<double> _phidpArray_;
  
  double *_snrArray;
  double *_dbzArray;
  double *_zdrArray;
  double *_rhohvArray;
  double *_phidpArray;

  // derived arrays

  RadxArray<double> _kdpArray_;
  RadxArray<double> _kdpZZdrArray_;
  RadxArray<double> _kdpCondArray_;

  double *_kdpArray;
  double *_kdpZZdrArray;
  double *_kdpCondArray;

  // computing kdp
  
  KdpFilt _kdp;

  // debug printing

  static pthread_mutex_t _debugPrintMutex;

  // private methods
  
  void _kdpInit();
  void _kdpCompute();

  void _allocInputArrays();
  void _allocDerivedArrays();
  
  int _loadInputArrays(RadxRay *inputRay);

  int _loadFieldArray(RadxRay *inputRay,
                      const string &fieldName,
                      bool required,
                      double *array);

  void _computeSnrFromDbz();

  void _loadOutputFields(RadxRay *inputRay,
                         RadxRay *derivedRay);
  
  void _addDebugFields(RadxRay *derivedRay);

  void _addField(RadxRay *derivedRay,
                 const string &name,
                 const string &units,
                 const string &longName,
                 const string standardName,
                 const double *array64);


  void _addField(RadxRay *derivedRay,
                 const string &name,
                 const string &units,
                 const string &longName,
                 const string standardName,
                 const bool *arrayBool);


};

#endif
