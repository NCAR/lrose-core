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
// NoiseLocator.hh
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2012
//
///////////////////////////////////////////////////////////////
//
// Find noise gates in Doppler radar data
//
///////////////////////////////////////////////////////////////

#ifndef NoiseLocator_H
#define NoiseLocator_H

#include <pthread.h>
#include <vector>
#include <radar/MomentsFields.hh>
#include <radar/IwrfCalib.hh>
#include <radar/InterestMap.hh>
using namespace std;

class NoiseLocator {
  
public:

  // constructor
  
  NoiseLocator();

  // destructor
  
  ~NoiseLocator();

  // set debugging state

  void setDebug(bool state) { _debug = state; }

  //////////////////////////////////////////////////////
  // set parameters for noise location

  // set kernel size for noise location
  
  void setNGatesKernel(int val) {
    _nGatesKernel = val;
  }

  // Set interest maps for identifying noise
  
  void setInterestMapPhaseChangeErrorForNoise
    (const vector<InterestMap::ImPoint> &pts,
     double weight);
  
  void setInterestMapDbmSdevForNoise
    (const vector<InterestMap::ImPoint> &pts,
     double weight);
  
  void setInterestMapNcpMeanForNoise
    (const vector<InterestMap::ImPoint> &pts,
     double weight);
  
  void setInterestThresholdForNoise(double val);
  
  // Set interest maps for identifying signal
  
  void setInterestMapPhaseChangeErrorForSignal
    (const vector<InterestMap::ImPoint> &pts,
     double weight);
  
  void setInterestMapDbmSdevForSignal
    (const vector<InterestMap::ImPoint> &pts,
     double weight);
  
  void setInterestThresholdForSignal(double val);
  
  ////////////////////////////////////////////////////////////////
  // set the computation method - defaults to RAY_MEDIAN

  // for the ray-by-ray method, we compute noise for individual rays
  // so the noise varies on a ray-by-ray basis

  void setComputeRayMedian(int minNGatesRayMedian);

  // for the running-median method, we compute a running median of
  // the noise which is applied to the rays in sequence
  // so the estimated noise varies slowly from time to time
  
  void setComputeRunningMedian(int nGatesRunningMedian);
  
  ////////////////////////////////////////////////////////////////
  // set the ray properties
  // must be called before locate() or computeNoise()

  void setRayProps(int nGates,
                   const IwrfCalib &calib,
                   time_t timeSecs, 
                   double nanoSecs,
                   double elevation, 
                   double azimuth);

  //////////////////////////////////////////////////////////
  // set flag to indicate that the noise bias should be set
  // to the same value for all channels
  //
  // Use Hc if appropriate for computing bias,
  // and set all channels to Hc bias.
  // If no Hc, use Vc.

  void setEqualBiasInAllChannels(bool state) {
    _equalBiasInAllChannels = state;
  }
  
  //////////////////////////////////////////////
  // perform noise location
  //
  // mfields: array of Moments Fields computed before
  //          calling this method
  //
  // Must call setRayProps first
  //
  // The following must be set in mfields prior to calling:
  //
  //   phase_for_noise
  //   dbm_for_noise
  //   ncp
  
  void locate(const MomentsFields *mfields);

  /////////////////////////////////////////////
  // Identify the noise, compute mean noise
  //
  // Must call setRayProps before any of these methods
  
  // Single pol H channel
  // The following must be set in mfields prior to calling:
  //   lag0_hc_db

  void computeNoiseSinglePolH(MomentsFields *mfields);

  // Single pol V channel
  // The following must be set in mfields prior to calling:
  //   lag0_vc_db

  void computeNoiseSinglePolV(MomentsFields *mfields);

  // Alternating mode dual pol, co-pol receiver only
  // The following must be set in mfields prior to calling:
  //   lag0_hc_db
  //   lag0_vc_db
  
  void computeNoiseDpAltHvCoOnly(MomentsFields *mfields);

  // Alternating mode dual pol, co/cross receivers
  // The following must be set in mfields prior to calling:
  //   lag0_hc_db
  //   lag0_vc_db
  //   lag0_hx_db
  //   lag0_vx_db
  
  void computeNoiseDpAltHvCoCross(MomentsFields *mfields);

  // Sim HV mode
  // The following must be set in mfields prior to calling:
  //   lag0_hc_db
  //   lag0_vc_db
  
  void computeNoiseDpSimHv(MomentsFields *mfields);

  // Dual pol, H-transmit only
  // The following must be set in mfields prior to calling:
  //   lag0_hc_db
  //   lag0_vx_db
  
  void computeNoiseDpHOnly(MomentsFields *mfields);

  // Dual pol, V-transmit only
  // The following must be set in mfields prior to calling:
  //   lag0_vc_db
  //   lag0_hx_db
  
  void computeNoiseDpVOnly(MomentsFields *mfields);
  
  // add the noise fields to a moments array
  
  void addToMoments(MomentsFields *mfields);
    
  // get results - after running locate
  // these arrays span the gates from 0 to nGates-1

  const vector<bool> &getNoiseFlag() const { return _noiseFlag; }
  const vector<bool> &getSignalFlag() const { return _signalFlag; }

  const vector<size_t> &getStartGate() const { return _startGate; }
  const vector<size_t> &getEndGate() const { return _endGate; }

  const vector<double> &getAccumPhaseChange() const { 
    return _accumPhaseChange;
  }
  const vector<double> &getPhaseChangeError() const { 
    return _phaseChangeError; 
  }

  const vector<double> &getDbmSdev() const { return _dbmSdev; }
  const vector<double> &getNcpMean() const { return _ncpMean; }

  double getMedianNoiseDbmHc() const { return _medianNoiseDbmHc; }
  double getMedianNoiseDbmVc() const { return _medianNoiseDbmVc; }
  double getMedianNoiseDbmHx() const { return _medianNoiseDbmHx; }
  double getMedianNoiseDbmVx() const { return _medianNoiseDbmVx; }
  double getNoiseBiasDbHc() const { return _noiseBiasDbHc; }
  double getNoiseBiasDbVc() const { return _noiseBiasDbVc; }
  double getNoiseBiasDbHx() const { return _noiseBiasDbHx; }
  double getNoiseBiasDbVx() const { return _noiseBiasDbVx; }

  ////////////////////////////////////
  // print parameters for debugging
  
  void printParams(ostream &out);

protected:
private:

  // debugging
  
  bool _debug;

  // ray properties

  int _nGates;
  time_t _timeSecs;
  double _nanoSecs;
  double _elevation;
  double _azimuth;
  IwrfCalib _calib;

  // flag to indicate that the noise bias should be set
  // to the same value for all channels

  bool _equalBiasInAllChannels;
  
  // results

  vector<bool> _noiseFlag;
  vector<bool> _signalFlag;
  vector<size_t> _startGate;
  vector<size_t> _endGate;
  vector<double> _accumPhaseChange;
  vector<double> _phaseChangeError;
  vector<double> _dbmSdev;
  vector<double> _ncpMean;
  
  double _medianNoiseDbmHc;
  double _medianNoiseDbmVc;
  double _medianNoiseDbmHx;
  double _medianNoiseDbmVx;

  double _noiseBiasDbHc;
  double _noiseBiasDbVc;
  double _noiseBiasDbHx;
  double _noiseBiasDbVx;

  // keeping track of previously-determined noise values
  // using an elevation/azimuth grid at 0.5 deg resolution

  typedef struct {
    float noiseHc;
    float noiseVc;
    float noiseHx;
    float noiseVx;
    time_t time;
  } noise_val_t;

  int _gridIndexAz;
  int _gridIndexEl;
  static noise_val_t **_previousGrid;
  static pthread_mutex_t _prevGridMutex;

  static const double _gridResEl;
  static const double _gridResAz;
  static const int _gridSizeEl = 720;
  static const int _gridSizeAz = 720;

  typedef struct {
    int ix;
    int iy;
  } search_kernel_t;
  static search_kernel_t _searchKernel[25];

  // compute method
  
  typedef enum {
    RAY_BY_RAY,
    RUNNING_MEDIAN
  } compute_method_t;
  
  int _minNGatesRayMedian;

  static compute_method_t _computeMethod;
  static pthread_mutex_t _computeMethodMutex;

  static int _nGatesRunningMedian;
  static int _nGatesRunningCount;
  static vector<double> _runningValsDbmHc;
  static vector<double> _runningValsDbmVc;
  static vector<double> _runningValsDbmHx;
  static vector<double> _runningValsDbmVx;
  static double _runningMedianNoiseDbmHc;
  static double _runningMedianNoiseDbmVc;
  static double _runningMedianNoiseDbmHx;
  static double _runningMedianNoiseDbmVx;
  static pthread_mutex_t _runningMedianMutex;
  
  // fuzzy logic for noise and signal

  int _nGatesKernel;

  InterestMap *_interestMapPhaseChangeErrorForNoise;
  InterestMap *_interestMapDbmSdevForNoise;
  InterestMap *_interestMapNcpMeanForNoise;
  double _weightPhaseChangeErrorForNoise;
  double _weightDbmSdevForNoise;
  double _weightNcpMeanForNoise;
  double _interestThresholdForNoise;

  InterestMap *_interestMapPhaseChangeErrorForSignal;
  InterestMap *_interestMapDbmSdevForSignal;
  double _weightPhaseChangeErrorForSignal;
  double _weightDbmSdevForSignal;
  double _interestThresholdForSignal;

  // private methods
  
  double _computePhaseChangeError(int startGate, int endGate);
  void _computeDbmSdev(const MomentsFields *mfields);
  void _computeNcpMean(const MomentsFields *mfields);
  double _computeMean(const vector<double> &vals);
  double _computeMedian(const vector<double> &vals);
  int _getSavedNoiseClosestHc(noise_val_t &closest);
  int _getSavedNoiseClosestVc(noise_val_t &closest);

  void _createDefaultInterestMaps();

};

#endif
