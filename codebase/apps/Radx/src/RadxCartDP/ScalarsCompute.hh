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
// ScalarsCompute.hh
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 2025
//
///////////////////////////////////////////////////////////////
//
// ScalarsCompute computation engine
// Computes dual pol scalars in polar coords
// There is one object per thread.
//
///////////////////////////////////////////////////////////////

#ifndef ScalarsCompute_HH
#define ScalarsCompute_HH

#include "Params.hh"
#include <radar/KdpFilt.hh>
#include <radar/KdpFiltParams.hh>
#include <radar/NcarPidParams.hh>
#include <radar/NcarParticleId.hh>
#include <radar/PrecipRate.hh>
#include <radar/PrecipRateParams.hh>
#include <Radx/Radx.hh>
#include <Radx/RadxTime.hh>
class RadxCartDP;
class RadxRay;
class RadxField;
class TempProfile;
#include <pthread.h>
#include <vector>
using namespace std;

class ScalarsCompute {
  
public:
  
  // constructor
  
  ScalarsCompute(RadxCartDP *parent, 
                 const Params &params,
                 const KdpFiltParams &kdpFiltParams,
                 const NcarPidParams &ncarPidParams,
                 const PrecipRateParams &precipRateParams,
                 int id);

  // destructor
  
  ~ScalarsCompute();

  // Load the temperature profile for PID computations,
  // for the specified time.
  // This reads in a new sounding if needed.
  // If no sounding is available, the static profile is used
  
  void loadTempProfile(time_t dataTime);
  
  // Set the temperature profile
  
  void setTempProfile(const TempProfile &profile);

  // Get the temperature profile
  
  const TempProfile &getTempProfile() const;

  // Creates derived fields ray and returns it.
  // It must be freed by caller.
  //
  // Returns NULL on error.
  
  RadxRay *doCompute(RadxRay *inputRay,
                     double radarHtKm,
                     double wavelengthM);

  bool OK;
  
protected:
private:

  static const double missingDbl;
  
  // parent object

  RadxCartDP *_parent;

  // parameters

  const Params &_params;
  const KdpFiltParams &_kdpFiltParams;
  const NcarPidParams &_ncarPidParams;
  const PrecipRateParams &_precipRateParams;

  int _id; // thread ID
  
  // current ray properties
  
  time_t _timeSecs;
  double _nanoSecs;
  double _azimuth;
  double _elevation;
  
  // radar properties

  double _radarHtKm;
  double _wavelengthM;

  // geometry
  
  size_t _nGates;
  double _startRangeKm, _gateSpacingKm;
  double _nyquist;
  
  // input arrays

  vector<double> _snrArray;
  vector<double> _dbzArray;
  vector<double> _zdrArray;
  vector<double> _ldrArray;
  vector<double> _rhohvArray;
  vector<double> _phidpArray;
  
  // derived arrays

  vector<double> _kdpArray;
  vector<double> _kdpScArray;
  vector<double> _tempForPid;
  vector<double> _sdZdr;
  vector<double> _sdPhidp;

  // vector<int> _pidArray;
  // vector<double> _pidInterest;
  // vector<double> _rateZ;
  // vector<double> _rateZSnow;
  // vector<double> _rateZZdr;
  // vector<double> _rateKdp;
  // vector<double> _rateKdpZdr;
  // vector<double> _rateHybrid;
  // vector<double> _ratePid;
  // vector<double> _rateHidro;
  // vector<double> _rateBringi;

  // computing kdp
  
  KdpFilt _kdp;

  // pid

  NcarParticleId _pid;
  const TempProfile *_tempProfile;

  // precip

  PrecipRate _precip;

  // debug printing

  static pthread_mutex_t _debugPrintMutex;

  // private methods
  
  void _kdpInit();
  void _kdpCompute();

  int _pidInit();
  void _pidPrepare();
  
  // void _precipInit();
  // void _precipCompute();

  void _allocArrays();
  
  int _loadInputArrays(RadxRay *inputRay);

  int _loadFieldArray(RadxRay *inputRay,
                      const string &fieldName,
                      double *array);

  int _addFieldToRay(RadxRay *inputRay,
                     const string &fieldName,
                     const string &units,
                     double *array,
                     double miss);

  void _computeSnrFromDbz();

  void _loadOutputFields(RadxRay *inputRay,
                         RadxRay *derivedRay);
  
  void _addField(RadxRay *derivedRay,
                 const string &name,
                 const string &units,
                 const string &longName,
                 const string standardName,
                 const double *array64);
  
  void _addField(RadxRay *outputRay,
                 const string &name,
                 const string &units,
                 const string &longName,
                 const string standardName,
                 const Radx::fl32 *array32);

  void _addField(RadxRay *derivedRay,
                 const string &name,
                 const string &units,
                 const string &longName,
                 const string standardName,
                 const bool *arrayBool);


};

#endif
