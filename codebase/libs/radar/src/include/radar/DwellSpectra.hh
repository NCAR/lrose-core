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
// DwellSpectra.hh
//
// DwellSpectra object
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2023
//
///////////////////////////////////////////////////////////////
//
// DwellSpectra object holds time series for a dwell, and
// computes spectra, SpectralCmd etc.
//
////////////////////////////////////////////////////////////////

#ifndef DwellSpectra_hh
#define DwellSpectra_hh

#include <pthread.h>
#include <string>
#include <vector>
#include <cstdio>
#include <toolsa/TaArray.hh>
#include <toolsa/TaArray2D.hh>
#include <radar/RadarMoments.hh>
#include <radar/RadarComplex.hh>
#include <radar/IwrfTsInfo.hh>
#include <radar/IwrfTsPulse.hh>
#include <radar/IwrfCalib.hh>
#include <radar/MomentsFields.hh>
#include <radar/InterestMap.hh>
#include <radar/AtmosAtten.hh>
#include <radar/ForsytheRegrFilter.hh>

using namespace std;

////////////////////////
// This class

class DwellSpectra {
  
public:

  // Constructor
  
  DwellSpectra();
  
  // destructor
  
  ~DwellSpectra();
  
  // set dimensions

  void setDimensions(size_t nGates, size_t nSamples);

  // set algorithm parameters

  void setTdbzKernelNGates(size_t val) { _tdbzKernelNGates = val; }
  void setTdbzKernelNSamples(size_t val) { _tdbzKernelNSamples = val; }
  void setSdevZdrKernelNGates(size_t val) { _sdevZdrKernelNGates = val; }
  void setSdevZdrKernelNSamples(size_t val) { _sdevZdrKernelNSamples = val; }
  void setSdevPhidpKernelNGates(size_t val) { _sdevPhidpKernelNGates = val; }
  void setSdevPhidpKernelNSamples(size_t val) { _sdevPhidpKernelNSamples = val; }

  // set calibration

  void setCalibration(const IwrfCalib &val) { _calib = val; }

  // set interest maps for cmd

  void setInterestMapTdbz
    (const vector<InterestMap::ImPoint> &pts,
     double weight);
  
  void setInterestMapZdrSdev
    (const vector<InterestMap::ImPoint> &pts,
     double weight);
  
  void setInterestMapPhidpSdev
    (const vector<InterestMap::ImPoint> &pts,
     double weight);
  
  void setCmdInterestThreshold(double val) { _cmdInterestThreshold = val; }

  // set metadata

  void setTime(time_t timeSecs, int nanoSecs) {
    _timeSecs = timeSecs;
    _nanoSecs = nanoSecs;
    _dtime = (double) timeSecs + nanoSecs * 1.0e-9;
  }
  
  void setElevation(double val) { _el = val; }
  void setAzimuth(double val) { _az = val; }
  void setAntennaRate(double val) { _antennaRate = val; }

  void setRangeGeometry(double startRangeKm, double gateSpacingKm) {
    _startRangeKm = startRangeKm;
    _gateSpacingKm = gateSpacingKm;
  }

  void setXmitRcvMode(iwrf_xmit_rcv_mode_t val) { _xmitRcvMode = val; }
  void setPrt(double val) { _prt = val; }
  void setNyquist(double val) { _nyquist = val; }
  void setPulseWidthUs(double val) { _pulseWidthUs = val; }
  void setWavelengthM(double val) { _wavelengthM = val; }

  // set window array

  void setWindow(const double *window, size_t nSamples);

  // set filtering
  
  void setWindowType(RadarMoments::window_type_t val =
                     RadarMoments::WINDOW_RECT) {
    _windowType = val;
  }
  void setClutterFilterType(RadarMoments::clutter_filter_type_t val =
                            RadarMoments::CLUTTER_FILTER_ADAPTIVE) {
    _clutterFilterType = val;
  }
  void setRegrOrder(int val = -1) { _regrOrder = val; }
  void setRegrClutWidthFactor(double val) { _regrClutWidthFactor = val; }
  void setRegrCnrExponent(double val) { _regrCnrExponent = val; }
  void setRegrFiltNotchInterpMethod(RadarMoments::notch_interp_method_t val =
                                    RadarMoments::INTERP_METHOD_GAUSSIAN) {
    _regrNotchInterpMethod = val;
  }

  // reset, ready for setting new data
  
  void prepareForData();

  // set IQ arrays
  
  void setIqHc(const RadarComplex_t *iqHc,
               size_t gateNum, size_t nSamples);

  void setIqVc(const RadarComplex_t *iqVc,
               size_t gateNum, size_t nSamples);

  void setIqHx(const RadarComplex_t *iqHx,
               size_t gateNum, size_t nSamples);

  void setIqVx(const RadarComplex_t *iqVx,
               size_t gateNum, size_t nSamples);

  // Compute spectra

  void computePowerSpectra();
  void computeDbzSpectra();
  void computeZdrSpectra();
  void computePhidpSpectra();
  void computeRhohvSpectra();
  void computeTdbz();
  void computeZdrSdev();
  void computePhidpSdev();
  void computeSpectralCmd();
  
  // Compute spectral noise for entire dwell for specified variable
  // this is the min noise at any gate
  
  double computeDwellSpectralNoise(const TaArray2D<double> &specPower2D,
                                   TaArray<double> &specNoise1D);
  
  // Compute noise from a power spectrum
  
  double computeSpectralNoise(const double *powerSpec, size_t nSamples);

  // get spectra

  bool getHcAvail() const { return _hcAvail; }
  bool getVcAvail() const { return _vcAvail; }
  bool getHxAvail() const { return _hxAvail; }
  bool getVxAvail() const { return _vxAvail; }
  
  RadarComplex_t **getIqHc() const { return _iqHc2D.dat2D(); }

  RadarComplex_t **getIqHc2D() const { return _iqHc2D.dat2D(); }
  RadarComplex_t **getIqVc2D() const { return _iqVc2D.dat2D(); }
  RadarComplex_t **getIqHx2D() const { return _iqHx2D.dat2D(); }
  RadarComplex_t **getIqVx2D() const { return _iqVx2D.dat2D(); }

  RadarComplex_t **getSpecCompHc2D() const { return _specCompHc2D.dat2D(); }
  RadarComplex_t **getSpecCompVc2D() const { return _specCompVc2D.dat2D(); }
  RadarComplex_t **getSpecCompHx2D() const { return _specCompHx2D.dat2D(); }
  RadarComplex_t **getSpecCompVx2D() const { return _specCompVx2D.dat2D(); }
  
  double **getSpecPowerHc2D() const { return _specPowerHc2D.dat2D(); }
  double **getSpecPowerVc2D() const { return _specPowerVc2D.dat2D(); }
  double **getSpecPowerHx2D() const { return _specPowerHx2D.dat2D(); }
  double **getSpecPowerVx2D() const { return _specPowerVx2D.dat2D(); }
  
  double **getSpecDbmHc2D() const { return _specDbmHc2D.dat2D(); }
  double **getSpecDbmVc2D() const { return _specDbmVc2D.dat2D(); }
  double **getSpecDbmHx2D() const { return _specDbmHx2D.dat2D(); }
  double **getSpecDbmVx2D() const { return _specDbmVx2D.dat2D(); }
  
  double **getSpecDbz2D() const { return _specDbz2D.dat2D(); }
  double **getSpecZdr2D() const { return _specZdr2D.dat2D(); }
  double **getSpecPhidp2D() const { return _specPhidp2D.dat2D(); }
  double **getSpecRhohv2D() const { return _specRhohv2D.dat2D(); }

  double **getSpecTdbz2D() const { return _specTdbz2D.dat2D(); }
  double **getSpecZdrSdev2D() const { return _specZdrSdev2D.dat2D(); }
  double **getSpecPhidpSdev2D() const { return _specPhidpSdev2D.dat2D(); }

  double **getSpecTdbzInterest2D() const { return _specTdbzInterest2D.dat2D(); }
  double **getSpecZdrSdevInterest2D() const { return _specZdrSdevInterest2D.dat2D(); }
  double **getSpecPhidpSdevInterest2D() const { return _specPhidpSdevInterest2D.dat2D(); }
  
  double **getSpecCmd2D() const { return _specCmd2D.dat2D(); }
  
protected:
  
private:

  static const double _missingDbl;

  // FFTs

  static pthread_mutex_t _fftMutex;
  RadarFft _fft;
  ForsytheRegrFilter _regr;

  // debug printing
  
  static pthread_mutex_t _debugPrintMutex;

  // dimensions

  size_t _nSamples;
  size_t _nGates;

  // algorithm parameters
  
  size_t _tdbzKernelNGates;
  size_t _tdbzKernelNSamples;
  size_t _sdevZdrKernelNGates;
  size_t _sdevZdrKernelNSamples;
  size_t _sdevPhidpKernelNGates;
  size_t _sdevPhidpKernelNSamples;
  
  // calibration
    
  IwrfCalib _calib;
  
  // dwell metadata - from radar obs
  
  time_t _timeSecs;
  int _nanoSecs;
  double _dtime;

  double _el;
  double _az;

  double _antennaRate;

  double _startRangeKm;
  double _gateSpacingKm;

  iwrf_xmit_rcv_mode_t _xmitRcvMode;
  
  double _prt;
  double _nyquist;
  double _pulseWidthUs;
  double _wavelengthM;

  // phidp sdev

  bool _phidpFoldsAt90;
  double _phidpFoldVal, _phidpFoldRange;

  // filtering

  RadarMoments::window_type_t _windowType;
  RadarMoments::clutter_filter_type_t _clutterFilterType;
  int _regrOrder; // auto if negative
  double _regrClutWidthFactor;
  double _regrCnrExponent;
  RadarMoments::notch_interp_method_t _regrNotchInterpMethod;

  // noise

  double _specNoiseDwellHc;
  double _specNoiseDwellVc;
  double _specNoiseDwellHx;
  double _specNoiseDwellVx;
  
  // interest maps for CMD
  
  InterestMap *_interestMapTdbz;
  InterestMap *_interestMapZdrSdev;
  InterestMap *_interestMapPhidpSdev;

  double _cmdInterestThreshold;

  // Arrays

  TaArray<double> _window1D;
  TaArray<double> _specNoiseHc1D;

  bool _hcAvail, _vcAvail, _hxAvail, _vxAvail;
  
  TaArray2D<RadarComplex_t> _iqHc2D;
  TaArray2D<RadarComplex_t> _iqVc2D;
  TaArray2D<RadarComplex_t> _iqHx2D;
  TaArray2D<RadarComplex_t> _iqVx2D;
  
  TaArray2D<RadarComplex_t> _specCompHc2D;
  TaArray2D<RadarComplex_t> _specCompVc2D;
  TaArray2D<RadarComplex_t> _specCompHx2D;
  TaArray2D<RadarComplex_t> _specCompVx2D;
  
  TaArray2D<double> _specPowerHc2D;
  TaArray2D<double> _specPowerVc2D;
  TaArray2D<double> _specPowerHx2D;
  TaArray2D<double> _specPowerVx2D;
  
  TaArray2D<double> _specDbmHc2D;
  TaArray2D<double> _specDbmVc2D;
  TaArray2D<double> _specDbmHx2D;
  TaArray2D<double> _specDbmVx2D;
  
  TaArray2D<double> _specDbz2D;
  TaArray2D<double> _specZdr2D;
  TaArray2D<double> _specPhidp2D;
  TaArray2D<double> _specRhohv2D;

  TaArray2D<double> _specTdbz2D;
  TaArray2D<double> _specZdrSdev2D;
  TaArray2D<double> _specPhidpSdev2D;
  
  TaArray2D<double> _specTdbzInterest2D;
  TaArray2D<double> _specZdrSdevInterest2D;
  TaArray2D<double> _specPhidpSdevInterest2D;
  
  TaArray2D<double> _specCmd2D;
  
  // member functions
  
  void _allocArrays(size_t nGates, size_t nSamples);
  void _freeArrays();

  void _computePowerSpectra(TaArray2D<RadarComplex_t> &iq2D,
                            double calibNoise,
                            RadarMoments &moments,
                            TaArray2D<RadarComplex_t> &specComp2D,
                            TaArray2D<double> &specPower2D,
                            TaArray2D<double> &specDbm2D);

  void _computePowerSpectrum(RadarComplex_t *specComp1D,
                             double *specPower1D,
                             double *specDbm1D);

  double _computeSdev(const vector<double> &zdr);
  double _computeSdevPhidp(const vector<double> &phidp);
  void _computePhidpFoldingRange();
  void _createDefaultInterestMaps();
  
};

#endif

