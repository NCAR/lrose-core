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
///////////////////////////////////////////////////////////////
// DerFieldCalcs.cc
//
// calculations of derived fields for HSRL 
//
// Brad Schoenrock, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Mar 2017
//
///////////////////////////////////////////////////////////////

#include "DerFieldCalcs.hh"
#include "FullCals.hh"
#include <Radx/RadxArray.hh>
#include <assert.h>
#include <math.h>
#include <iostream>
#include <cmath>
#include <cassert>
#include <algorithm>

using namespace std;

const double DerFieldCalcs::_BmsFactor =
  (5.45 * pow((550.0 / 532.0), 4.0) * 1.0e-32);

const double DerFieldCalcs::_BoltzmannConst = 1.38064852e-23; 

const double DerFieldCalcs::_depolFactor = 0.000365;

const double DerFieldCalcs::firCoeff_21[FIR_LEN_21] = {
  0.016976991942, 0.023294989742, 0.030244475217,
  0.037550056394, 0.044888313214, 0.051908191403,
  0.058254532798, 0.063592862330, 0.067633391375,
  0.070152221980, 0.071007947209, 0.070152221980,
  0.067633391375, 0.063592862330, 0.058254532798,
  0.051908191403, 0.044888313214, 0.037550056394,
  0.030244475217, 0.023294989742, 0.016976991942
};

const double DerFieldCalcs::firCoeff_11[FIR_LEN_11] = {
  0.03064579383,0.0603038422,0.09022859603,0.1159074511,
  0.1332367851,0.1393550634,0.1332367851,0.1159074511,
  0.09022859603,0.0603038422,0.03064579383
};

const double DerFieldCalcs::firCoeff_7[FIR_LEN_7] = {
  0.0264, 0.1261, 0.2190, 0.2569, 0.2190, 0.1261, 0.0264
};

/////////////////////////////////////////////////////////////////
// constructor

DerFieldCalcs::DerFieldCalcs(const Params &params,
                             const FullCals &fullCals) :
  _params(params),
  _fullCals(fullCals)

{

  // set up FIR filter for optical depth

  _setFIRFilterLen(FIR_LENGTH_7);

}

/////////////////////////////////////////////////////////////////
// destructor

DerFieldCalcs::~DerFieldCalcs()
{
  
}

/////////////////////////////////////////////////////////////////
// do calculations for derived fields

void DerFieldCalcs::computeDerived(size_t nGates,
                                   double startRangeKm,
                                   double gateSpacingKm,
                                   const Radx::fl32 *hiData,
                                   const Radx::fl32 *loData, 
                                   const Radx::fl32 *crossData,
                                   const Radx::fl32 *molData,
                                   const Radx::fl32 *htM,
                                   const Radx::fl32 *tempK, 
                                   const Radx::fl32 *presHpa,
                                   double shotCount, 
                                   double power)

{

  // init

  _nGates = nGates;
  _startRangeKm = startRangeKm;
  _gateSpacingKm = gateSpacingKm;
  _shotCount = shotCount;
  _power = power;
  
  // load vectors

  _hiData.resize(nGates);
  _loData.resize(nGates);
  _crossData.resize(nGates);
  _molData.resize(nGates);
  _htM.resize(nGates);
  _tempK.resize(nGates);
  _presHpa.resize(nGates);

  for(size_t igate = 0; igate < nGates; igate++) {
    _hiData[igate] = hiData[igate];
    _loData[igate] = loData[igate];
    _crossData[igate] = crossData[igate];
    _molData[igate] = molData[igate];
    _htM[igate] = htM[igate];
    _tempK[igate] = tempK[igate];
    _presHpa[igate] = presHpa[igate];
  }

  // set bins per gate

  _nBinsPerGate = 1;
  if (_params.combine_bins_on_read) {
    _nBinsPerGate = _params.n_bins_per_gate;
  }
  
  // apply corrections
  
  _applyCorr();

  // compute the filtered rates

  // _computeFilteredRates();

  // init arrays

  _initDerivedArrays();

  // betaMSonde
  
  for(size_t igate = 0; igate < _nGates; igate++) {
    _betaMSonde[igate] = _computeBetaMSonde(_presHpa[igate], _tempK[igate]);
  }

  // vol depol

  for(size_t igate = 0; igate < _nGates; igate++) {
    _volDepol[igate] = _computeVolDepol(_combRate[igate],
                                        _crossRate[igate]);
    // _volDepolF[igate] = _computeVolDepol(_combRateF[igate],
    //                                      _crossRateF[igate]);
  }
  
  // backscatter ratio
  
  for(size_t igate = 0; igate < _nGates; igate++) {
    _backscatRatio[igate] = _computeBackscatRatio(_combRate[igate], 
                                                  _crossRate[igate],
                                                  _molRate[igate]);
    // _backscatRatioF[igate] = _computeBackscatRatio(_combRateF[igate], 
    //                                                _crossRateF[igate],
    //                                                _molRate[igate]);
  }
  
  // particle depol
  
  for(size_t igate = 0; igate < _nGates; igate++) {
    _partDepol[igate] = _computePartDepol(_volDepol[igate],
                                          _backscatRatio[igate]);
    // _partDepolF[igate] = _computePartDepol(_volDepolF[igate],
    //                                        _backscatRatioF[igate]);
  }
  
  // backscatter coefficient
  
  for(size_t igate = 0; igate < _nGates; igate++) {
    _backscatCoeff[igate] = _computeBackscatCoeff(_betaMSonde[igate],
                                                  _backscatRatio[igate]);
  }

  // optical depth

  // get cal adjustment
  
  double scanAdj = 1;
  CalReader scanAdjustCal = _fullCals.getScanAdj();
  if (scanAdjustCal.dataTypeisNum()) {
    int binPos = _fullCals.getBinPos();
    const vector<vector<double> > &dataNum = scanAdjustCal.getDataNum();
    if(dataNum[binPos].size() == 1) {
      scanAdj = dataNum[binPos][0];
    }
  }

  // double optDepthOffset = _computeOptDepthRefOffset(scanAdj);
  for(size_t igate = 0; igate < _nGates; igate++) {
    double optDepth = _computeOpticalDepth(_betaMSonde[igate],
                                           _molRate[igate], scanAdj);
    // _opticalDepth[igate] = optDepth + optDepthOffset;
    _opticalDepth[igate] = optDepth - _params.optical_depth_reference_value;
  }

  // filter the optical depth
  
  _applyFirFilter(_opticalDepth);

  // extinction

  int filterHalf = _params.optical_depth_median_filter_len / 2;
  if (filterHalf < 1) {
    filterHalf = 1;
  }
  for(size_t igate = filterHalf; igate < _nGates - filterHalf; igate++) {
    _extinction[igate] = 
      _computeExtinctionCoeff(_opticalDepth[igate - filterHalf],
                              _opticalDepth[igate + filterHalf],
                              _htM[igate - filterHalf],
                              _htM[igate + filterHalf]);
  }

  // set all zero vals to missing
  
  _setZeroValsToMissing();

  // perform thresholding

  for(size_t igate = 0; igate < _nGates; igate++) {
    if (_hiData[igate] < _params.combined_high_count_threshold_for_backscat_coeff) {
      _backscatCoeff[igate] = Radx::missingFl32;
    }
    if (_hiData[igate] < _params.combined_high_count_threshold_for_vol_depol_ratio) {
      _volDepol[igate] = Radx::missingFl32;
    }
  }



}

/////////////////////////////////////////////////////////////////
// apply corrections

void DerFieldCalcs::_applyCorr()
{
  
  // filter the molecular counts

  _applyMedianFilter(_params.molecular_count_median_filter_len, _molData);

  // allocate the rate vectors

  _hiRate.resize(_nGates);
  _loRate.resize(_nGates);
  _crossRate.resize(_nGates);
  _molRate.resize(_nGates);
  _combRate.resize(_nGates);

  // non-linear count correction

  _applyNonLinearCountCorr();

  // subtract baseline
  // this is a pass-through for now
  
  _applyBaselineCorr();

  // subtract the background signal

  _applyBackgroundCorr();
  
  // apply energy normalization

  _applyEnergyNorm();

  // correct for differential overlap

  _applyDiffGeoCorr();
  
  // for now we ignore QWP rotation correction

  // merge hi and lo channels into combined rate
  
  for(size_t igate = 0; igate < _nGates; igate++) {
    _combRate[igate] = _hiAndloMerge(_hiRate[igate], 
                                         _loRate[igate]);
  }
  _printRateDiagnostics("hiAndloMerge", true);

  // apply geo correction

  _applyGeoCorr();

}

/////////////////////////////////////////////////////////////////
// non-linear count correction

void DerFieldCalcs::_applyNonLinearCountCorr()
{

  CalReader dt_hi = _fullCals.getDeadTimeHi();
  CalReader dt_lo = _fullCals.getDeadTimeLo();
  CalReader dt_cross = _fullCals.getDeadTimeCross();
  CalReader dt_mol = _fullCals.getDeadTimeMol();
  CalReader binwid = _fullCals.getBinWidth();
  
  double hiDeadTime = dt_hi.getDataNum()[_fullCals.getHiPos()][0];
  double loDeadTime = dt_lo.getDataNum()[_fullCals.getLoPos()][0];
  double crossDeadTime = dt_cross.getDataNum()[_fullCals.getCrossPos()][0];
  double molDeadTime = dt_mol.getDataNum()[_fullCals.getMolPos()][0];
  double binW = binwid.getDataNum()[_fullCals.getBinPos()][0];

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "==>> hiDead, loDead, crossDead, molDead, binW, shotCount: "
         << hiDeadTime << ", "
         << loDeadTime << ", "
         << crossDeadTime << ", "
         << molDeadTime << ", "
         << binW << ", "
         << _shotCount << endl;
  }

  for(size_t igate = 0; igate < _nGates; igate++) {
    _hiRate[igate] =
      _nonLinCountCor(_hiData[igate], hiDeadTime, binW, _shotCount); 
    _loRate[igate] = 
      _nonLinCountCor(_loData[igate], loDeadTime, binW, _shotCount); 
    _crossRate[igate] =
      _nonLinCountCor(_crossData[igate], crossDeadTime, binW, _shotCount); 
    _molRate[igate] =
      _nonLinCountCor(_molData[igate], molDeadTime, binW, _shotCount); 
  } // igate

  _printRateDiagnostics("countCorrection");

}

/////////////////////////////////////////////////////////////////
// subtract baseline
// this is a pass-through for now

void DerFieldCalcs::_applyBaselineCorr()
{
  
  const vector<double> &blCorCombinedHi = _fullCals.getBlCorCombinedHi();
  const vector<double> &blCorCombinedLo = _fullCals.getBlCorCombinedLo();
  const vector<double> &blCorMolecular = _fullCals.getBlCorMolecular();
  const vector<double> &blCorCrossPol = _fullCals.getBlCorCrossPol();
  
  size_t calBin = _nBinsPerGate / 2;
  double polPosn = 1.0;

  for(size_t igate = 0; igate < _nGates; igate++, calBin += _nBinsPerGate) {
    _hiRate[igate] = _baselineSubtract(_hiRate[igate],
                                       blCorCombinedHi[calBin], polPosn);
    _loRate[igate] = _baselineSubtract(_loRate[igate],
                                       blCorCombinedLo[calBin], polPosn);
    _crossRate[igate] = _baselineSubtract(_crossRate[igate],
                                          blCorCrossPol[calBin], polPosn);
    _molRate[igate] = _baselineSubtract(_molRate[igate],
                                        blCorMolecular[calBin], polPosn);
  } // igate
  
  _printRateDiagnostics("baselineSubtract");

}

/////////////////////////////////////////////////////////////////
// subtract background
// as computed from last n gates

void DerFieldCalcs::_applyBackgroundCorr()
{
  
  // compute background rates from last 'n' gates
  
  double hibackgroundRate = _computeBackgroundRate(_hiRate,
                                                   _hiRateBackground);
  
  double lobackgroundRate = _computeBackgroundRate(_loRate,
                                                   _loRateBackground);
  
  double molbackgroundRate = _computeBackgroundRate(_molRate,
                                                    _molRateBackground);
  
  double crossbackgroundRate = _computeBackgroundRate(_crossRate,
                                                      _crossRateBackground);
  
  // adjust for background rate
  
  for(size_t igate = 0; igate < _nGates; igate++) {
    _hiRate[igate] =
      _backgroundSubtract(_hiRate[igate], hibackgroundRate);
    _loRate[igate] =
      _backgroundSubtract(_loRate[igate], lobackgroundRate);
    _crossRate[igate] =
      _backgroundSubtract(_crossRate[igate], crossbackgroundRate);
    _molRate[igate] =
      _backgroundSubtract(_molRate[igate], molbackgroundRate);
  }
  
  if(_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "hibackgroundRate = " << hibackgroundRate<<endl;
    cerr << "lobackgroundRate = " << lobackgroundRate<<endl;
    cerr << "crossbackgroundRate = " << crossbackgroundRate<<endl;
    cerr << "molbackgroundRate = " << molbackgroundRate<<endl;
    cerr << "==================================" << endl;
    _printRateDiagnostics("backgroundSub");
  }

  // fill missing with the min value

  _fillMissingWithMinVal(_hiRate);
  _fillMissingWithMinVal(_loRate);
  _fillMissingWithMinVal(_crossRate);
  _fillMissingWithMinVal(_molRate);

}

////////////////////////////////////////////
// apply energy normalization

void DerFieldCalcs::_applyEnergyNorm()

{

  // normalize with respect to transmit energy

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "=========== _power: " << _power << endl;
  }
  
  for(size_t igate = 0; igate < _nGates; igate++) {
    _hiRate[igate] = _energyNorm(_hiRate[igate], _power);
    _loRate[igate] = _energyNorm(_loRate[igate], _power);
    _crossRate[igate] = _energyNorm(_crossRate[igate], _power);
    _molRate[igate] = _energyNorm(_molRate[igate], _power);
  }
  
  _printRateDiagnostics("energyNorm");

}

////////////////////////////////////////////
// correct for differential overlap

void DerFieldCalcs::_applyDiffGeoCorr()

{
  
  const vector<double> &diffGeoCombHiMol = _fullCals.getDiffGeoCombHiMol();
  const vector<double> &diffGeoCombLoMol = _fullCals.getDiffGeoCombLoMol();
  size_t calBin = _nBinsPerGate / 2;
  for(size_t igate = 0; igate < _nGates; igate++, calBin += _nBinsPerGate) {
    _hiRate[igate] /= diffGeoCombHiMol[calBin];
    _loRate[igate] /= diffGeoCombLoMol[calBin];
    // _crossRate[igate] /= diffGeoCombHiMol[calBin];
  }
  _printRateDiagnostics("diffOverlapCor");

}

///////////////////////////////////////////
// apply geo correction

void DerFieldCalcs::_applyGeoCorr()

{

  const vector<double> &geoCorr = _fullCals.getGeoCorr();
  size_t  calBin = _nBinsPerGate / 2;
  for(size_t igate = 0; igate < _nGates; igate++, calBin += _nBinsPerGate) {
    double corr = geoCorr[calBin];
    _combRate[igate] *= corr;
    _crossRate[igate] *= corr;
    _molRate[igate] *= corr;
  }
  
  _printRateDiagnostics("geoOverlapCor", true);

}
  
////////////////////////////////////////////
// Fill missing values with min val in ray

void DerFieldCalcs::_fillMissingWithMinVal(vector<Radx::fl32> &vals)

{

  double minVal = 1.e99;
  for(size_t igate = 0; igate < _nGates; igate++) {
    double val = vals[igate];
    if (val > 0 && val < minVal) {
      minVal = val;
    }
  }
  for(size_t igate = 0; igate < _nGates; igate++) {
    if (vals[igate] == 0.0) {
      vals[igate] = minVal;
    }
  }

}

/////////////////////////////////////////////////////////////////
// compute background rate for a given channel

double DerFieldCalcs::_computeBackgroundRate(vector<Radx::fl32> &rate,
                                             deque<Radx::fl32> &background)
{
  
  // compute median background rate from last 'n' gates
  
  int nGatesBackground = _params.ngates_for_background_correction;
  int startBackgroundGate = _nGates - nGatesBackground;
  if (startBackgroundGate < 1) {
    startBackgroundGate = 1;
  }

  vector<double> endRates;
  for(int cgate = startBackgroundGate - 1; cgate < (int) _nGates; cgate++) {
    endRates.push_back(rate[cgate]);
  }
  sort(endRates.begin(), endRates.end());
  double medianRate = endRates[endRates.size() / 2];

  if ((int) background.size() >= _params.nrays_for_background_correction) {
    background.pop_front();
  }
  background.push_back(medianRate);

  double minVal = 1.0e99;
  for (size_t ii = 0; ii < background.size(); ii++) {
    if (background[ii] < minVal) {
      minVal = background[ii];
    }
  }

  return minVal;
  // return medianRate;

}

/////////////////////////////////////////////////////////////////
// nonlinear count corrections

double DerFieldCalcs::_nonLinCountCor(Radx::fl32 count, double deadtime, 
                                      double binWid, double shotCount)
{

  if(count < 0.0) {
    return 0;
  }

  double photonRate = (count / shotCount) / binWid;
  double corrFactor = photonRate * deadtime;

  if(corrFactor > 0.95) {
    corrFactor = 0.95;
  }

  double corrRate = count / (1.0 - corrFactor);

  if(_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "11111111 count, dead, binWid, shot, prate, corr, rate: "
         << count << ", "
         << deadtime << ", "
         << binWid << ", "
         << shotCount << ", "
         << photonRate << ", "
         << corrFactor << ", "
         << corrRate << endl;
  }
    
  return corrRate;

}


/////////////////////////////////////////////////////////////////
// baseline subtraction

double DerFieldCalcs::_baselineSubtract(double arrivalRate, double profile, 
					double polarization)
{
  // pass through for now, not expected to significantly impact displays
  return arrivalRate;
}


/////////////////////////////////////////////////////////////////
// background subtraction

double DerFieldCalcs::_backgroundSubtract(double arrivalRate, double backgroundBins)
{
  // background bins is average of the last n bins
  double rate = arrivalRate - backgroundBins;
  if (rate < 0.0) {
    rate = 0.0;
  }
  return rate;
}


/////////////////////////////////////////////////////////////////
// energy normalization

double DerFieldCalcs::_energyNorm(double arrivalRate, double totalEnergy)
{
  if (totalEnergy == 0) {
    totalEnergy = 1.5e6;
  }
  return (arrivalRate / totalEnergy) * 1.0e6;
}


/////////////////////////////////////////////////////////////////
//process QWP rotation

vector<double> DerFieldCalcs::_processQWPRotation(vector<double> arrivalRate, 
						  vector<double> polCal)
{
  return arrivalRate;
}


/////////////////////////////////////////////////////////////////
// merge hi and lo profiles 

double DerFieldCalcs::_hiAndloMerge(double hiRate, double loRate)
{  
  // pass through for now, not expected to significantly impact displays
  return hiRate;
}


/////////////////////////////////////////////////////////////////
// geometric overlap correction

double DerFieldCalcs::_geoOverlapCor(double arrivalRate, double geoOverlap)
{
  return arrivalRate * geoOverlap;
}


/////////////////////////////////////////////////////////////////
// compute filtered rates

// void DerFieldCalcs::_computeFilteredRates()
// {
  
//   // allocate the rate vectors

//   _hiRateF.resize(_nGates);
//   _loRateF.resize(_nGates);
//   _crossRateF.resize(_nGates);
//   _molRateF.resize(_nGates);
//   _combRateF.resize(_nGates);

//   _filterRate(_hiRate, _hiRateF);
//   _filterRate(_loRate, _loRateF);
//   _filterRate(_crossRate, _crossRateF);
//   _filterRate(_molRate, _molRateF);
//   _filterRate(_combRate, _combRateF);

// }

//////////////////////////////////////////////////////////////
// Filter rate field

// void DerFieldCalcs::_filterRate(const vector<Radx::fl32> &rate,
//                                 vector<Radx::fl32> &rateF)
  
// {
  
//   // make copy

//   rateF = rate;

//   if (_params.rate_censoring_threshold > 0) {
//     for (size_t ii = 0; ii < _nGates; ii++) {
//       if (rateF[ii] < _params.rate_censoring_threshold) {
//         rateF[ii] = 0.0;
//       }
//     }
//   }

//   // despeckle
  
//   if (_params.apply_speckle_filter) {
//     _applySpeckleFilter(_params.speckle_filter_len, 0.0, rateF);
//   }

// }

///////////////////////////////////////////////////////
// run speckle filter for a given length
// checks for missing data
//
// minRunLen: length of run being tested for

// void DerFieldCalcs::_applySpeckleFilter(int minRunLen,
//                                         Radx::fl32 missingVal,
//                                         vector<Radx::fl32> &data)
  
// {

//   int count = 0;
//   // loop through all gates
//   for (int ii = 0; ii < (int) data.size(); ii++) {
//     // check for non-missing
//     if (data[ii] != missingVal) {
//       // set, so count up length of run
//       count++;
//     } else {
//       // not set, end of run
//       if (count <= minRunLen) {
//         // run too short, indicates possible speckle
//         for (int jj = ii - count; jj < ii; jj++) {
//           // remove speckle gates
//           data[jj] = Radx::missingFl32;
//         }
//       }
//       count = 0;
//     }
//   } // ii

// }

//////////////////////////////////////////////////////////////
// Set zero vals to missing

void DerFieldCalcs::_setZeroValsToMissing()
{

  _setZeroValsToMissing(_hiRate);
  _setZeroValsToMissing(_loRate);
  _setZeroValsToMissing(_crossRate);
  _setZeroValsToMissing(_molRate);
  _setZeroValsToMissing(_combRate);

  // _setZeroValsToMissing(_hiRateF);
  // _setZeroValsToMissing(_loRateF);
  // _setZeroValsToMissing(_crossRateF);
  // _setZeroValsToMissing(_molRateF);
  // _setZeroValsToMissing(_combRateF);

  _setZeroValsToMissing(_volDepol);
  _setZeroValsToMissing(_partDepol);
  _setZeroValsToMissing(_backscatRatio);
  _setZeroValsToMissing(_backscatCoeff);
  _setZeroValsToMissing(_opticalDepth);
  _setZeroValsToMissing(_extinction);

  // _setZeroValsToMissing(_volDepolF);
  // _setZeroValsToMissing(_partDepolF);
  // _setZeroValsToMissing(_backscatRatioF);
  // _setZeroValsToMissing(_backscatCoeffF);
  // _setZeroValsToMissing(_opticalDepthF);
  // _setZeroValsToMissing(_extinctionF);

}

void DerFieldCalcs::_setZeroValsToMissing(vector<Radx::fl32> &data)
  
{

  for (size_t ii = 0; ii < _nGates; ii++) {
    if (data[ii] <= 0.0) {
      data[ii] = Radx::missingFl32;
    }
  }

}

///////////////////////////////////////////////////////////////////
// init arrays for derived fields

void DerFieldCalcs::_initDerivedArrays()

{

  _betaMSonde.resize(_nGates);
  _volDepol.resize(_nGates);
  _backscatRatio.resize(_nGates);
  _partDepol.resize(_nGates);
  _backscatCoeff.resize(_nGates);
  _extinction.resize(_nGates);
  _opticalDepth.resize(_nGates);

  // _volDepolF.resize(_nGates);
  // _backscatRatioF.resize(_nGates);
  // _partDepolF.resize(_nGates);
  // _backscatCoeffF.resize(_nGates);
  // _extinctionF.resize(_nGates);
  // _opticalDepthF.resize(_nGates);

  for (size_t ii = 0; ii < _nGates; ii++) {

    _betaMSonde[ii] = Radx::missingFl32;
    _volDepol[ii] = Radx::missingFl32;
    _backscatRatio[ii] = Radx::missingFl32;
    _partDepol[ii] = Radx::missingFl32;
    _backscatCoeff[ii] = Radx::missingFl32;
    _extinction[ii] = Radx::missingFl32;
    _opticalDepth[ii] = Radx::missingFl32;

    // _volDepolF[ii] = Radx::missingFl32;
    // _backscatRatioF[ii] = Radx::missingFl32;
    // _partDepolF[ii] = Radx::missingFl32;
    // _backscatCoeffF[ii] = Radx::missingFl32;
    // _extinctionF[ii] = Radx::missingFl32;
    // _opticalDepthF[ii] = Radx::missingFl32;

  }

}

/////////////////////////////////////////////////////////////////
// volume depolarization

Radx::fl32 DerFieldCalcs::_computeVolDepol(double combineRate,
                                           double crossRate)
{

  if (crossRate < 0.0 || (crossRate + combineRate) <= 0.0) {
    return Radx::missingFl32;
  }

  double depol = crossRate / (crossRate + combineRate);
  if (depol > 1.0) {
    return 1.0;
    // return Radx::missingFl32;
  }

  return depol;

}


/////////////////////////////////////////////////////////////////
// backscatter ratio

Radx::fl32 DerFieldCalcs::_computeBackscatRatio(double combineRate,
                                                double crossRate,
                                                double molRate)
{

  if(combineRate < 0.0 || molRate <= 0.0) {
    return Radx::missingFl32;
  }

  double ratio = (combineRate + crossRate) / molRate;
  if (ratio < 1.0) {
    ratio = 1.0;
  }

  return ratio;

}


/////////////////////////////////////////////////////////////////
// particle depolarization

Radx::fl32 DerFieldCalcs::_computePartDepol(Radx::fl32 volDepol, 
                                            Radx::fl32 backscatRatio)
{

  if (volDepol == Radx::missingFl32 ||
      backscatRatio == Radx::missingFl32) {
    return Radx::missingFl32;
  }
  
  double d_mol = 2.0 * _depolFactor / (1.0 + _depolFactor);

  double pDepol = ((volDepol / (1.0 - (1.0 / backscatRatio))) -
                   (d_mol / (backscatRatio - 1.0)));
  
  if (pDepol < 0.0 || pDepol > 1.0) {
    return Radx::missingFl32;
  }
  
  return pDepol;

}


/////////////////////////////////////////////////////////////////
// beta M sonde

double DerFieldCalcs::_computeBetaMSonde(double pressHpa, double tempK)
{

  if(tempK <= 0.0) {
    return NAN;
  }

  double val = _BmsFactor * ((pressHpa * 100.0) / (tempK * _BoltzmannConst));
  if (!std::isfinite(val)) {
    return NAN;
  }
  
  return val;

}

/////////////////////////////////////////////////////////////////
// backscatter coefficient

Radx::fl32 DerFieldCalcs::_computeBackscatCoeff(double betaMSonde,
                                                Radx::fl32 backscatRatio)
{

  if (backscatRatio == Radx::missingFl32) {
    return Radx::missingFl32;
  }

  // compute betaM sonde

  if (betaMSonde == NAN) {
    return Radx::missingFl32;
  }

  // get cal gain adjustment
  
  double molGain = 1.0;
  CalReader molGainCal = _fullCals.getMolGain();
  if (molGainCal.dataTypeisNum()) {
    int binPos = _fullCals.getBinPos();
    const vector<vector<double> > &dataNum = molGainCal.getDataNum();
    if(dataNum[binPos].size() == 1) {
      molGain = dataNum[binPos][0];
    }
  }

  // compute coefficient

  double aerosolBscat = (backscatRatio / molGain - 1.0) * betaMSonde;
  if (aerosolBscat <= 0.0) {
    return Radx::missingFl32;
  }

  return aerosolBscat;

}

/////////////////////////////////////////////////////////////////
// optical depth

Radx::fl32 DerFieldCalcs::_computeOpticalDepth(double betaMSonde,
                                               double molRate, double scanAdj)
{

  if (molRate <= 0) {
    return Radx::missingFl32;
  }
  
  if (betaMSonde == NAN) {
    return Radx::missingFl32;
  }

  double xx = (scanAdj * molRate) / betaMSonde;
  if (xx <= 0.0) {
    return Radx::missingFl32;
  }

  double optDepth = - 0.5 * log(xx);

  // if(_params.debug >= Params::DEBUG_EXTRA) {
  //   if (pressHpa > 820) {
  //     cerr << "press, tempK, scanAdj, molRate, betaMSonde, .5*log(xx), optDepth: "
  //          << pressHpa << ", "
  //          << tempK << ", "
  //          << scanAdj << ", "
  //          << molRate << ", "
  //          << betaMSonde << ", "
  //          << 0.5*log(xx) << ", "
  //          << optDepth << endl;
  //   }
  // }
  
  return optDepth;

}


/////////////////////////////////////////////////////////////////
// Apply median filter to optical depth

void DerFieldCalcs::_filterOpticalDepth(vector<Radx::fl32> &optDepth)

{

  // make sure filter len is odd

  size_t halfFilt = _params.optical_depth_median_filter_len / 2;
  size_t len = halfFilt * 2 + 1;
  if (len < 3) {
    return;
  }

  // make copy
  
  vector<Radx::fl32> copy = optDepth;

  // remove isolated points, up to 2 in length

  int maxRun = 2;
  int ngood = 0;
  for (size_t ii = 0; ii < _nGates; ii++) {
    if (copy[ii] == Radx::missingFl32) {
      if (ngood > 0 && ngood <= maxRun) {
        for (int jj = (int) ii - ngood; jj < (int) ii; jj++) {
          copy[jj] = Radx::missingFl32;
        }
      }
      ngood = 0;
    } else {
      ngood++;
    }
  }
  
  // fill in gaps up to 2 in length

  int nmiss = 0;
  for (size_t ii = 0; ii < _nGates; ii++) {
    if (copy[ii] != Radx::missingFl32) {
      if (nmiss > 0 && nmiss <= maxRun && (ii - nmiss) > 0) {
        for (int jj = (int) ii - nmiss; jj < (int) ii; jj++) {
          copy[jj] = copy[ii - nmiss];
        }
      }
      nmiss = 0;
    } else {
      nmiss++;
    }
  }

  // apply median filter
  
  for (size_t ii = halfFilt; ii < _nGates - halfFilt; ii++) {

    vector<Radx::fl32> vals;
    for (size_t jj = ii - halfFilt; jj <= ii + halfFilt; jj++) {
      if (copy[jj] != Radx::missingFl32) {
        vals.push_back(copy[jj]);
      }
    } // jj

    if (vals.size() > 0) {
      sort(vals.begin(), vals.end());
      optDepth[ii] = vals[vals.size() / 2];
    } else {
      optDepth[ii] = Radx::missingFl32;
    }
    
  } // ii

}

/////////////////////////////////////////////////////////////////
// Compute the reference offset for optical depth

double DerFieldCalcs::_computeOptDepthRefOffset(double scanAdj)

{

  // compute the reference range gate
  
  double refRangeKm = _params.optical_depth_reference_range_m / 1000.0;
  int refGate = (int) ((refRangeKm - _startRangeKm) / _gateSpacingKm + 0.5);
  if (refGate < 0) {
    refGate = 0;
  } else if (refGate > (int) _nGates - 1) {
    refGate = _nGates - 1;
  }

  // get optical depth at reference range
  // and save to queue
  
  double refDepth =
    _computeOpticalDepth(_betaMSonde[refGate], _molRate[refGate], scanAdj);
  if (refDepth != Radx::missingFl32) {
    if ((int) _refOptDepth.size() >= _params.optical_depth_n_reference_obs) {
      _refOptDepth.pop_front();
    }
    _refOptDepth.push_back(refDepth);
  }

  // compute the reference value

  double refVal = 16.0;
  if (_refOptDepth.size() > 0) {
    double sum = 0.0;
    for (size_t ii = 0; ii < _refOptDepth.size(); ii++) {
      sum += _refOptDepth[ii];
    }
    refVal = sum / (double) _refOptDepth.size();
  }
  double refOffset = _params.optical_depth_reference_value - refVal;
  
  return refOffset;

}

/////////////////////////////////////////////////////////////////
// extinction coefficient

Radx::fl32 DerFieldCalcs::_computeExtinctionCoeff(Radx::fl32 optDepth1, 
                                                  Radx::fl32 optDepth2,
                                                  double alt1, double alt2)
{

  if (optDepth1 == Radx::missingFl32 || optDepth2 == Radx::missingFl32) {
    return Radx::missingFl32;
  }

  double minExtinction = 0.00005;
  double extinction = minExtinction;
  
  if(std::isnan(optDepth1) || std::isnan(optDepth2) || alt1 == alt2) {
    return Radx::missingFl32;
  }

  extinction = fabs(optDepth1 - optDepth2) / fabs(alt1 - alt2);

  // if (alt1 > alt2) {
  //   extinction = (optDepth1 - optDepth2) / (alt1 - alt2);
  // } else {
  //   extinction = (optDepth1 - optDepth2) / (alt2 - alt1);
  // }

  // if (extinction < minExtinction) {
  //   return minExtinction;
  //   // return Radx::missingFl32;
  // }
  
  return extinction;

}

/////////////////////////////////////////////////////////////////
// print diagnostics

void DerFieldCalcs::_printRateDiagnostics(const string &label,
                                          bool includeCombined /* = false */)
{
  if(_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "===== RATES FOR: " << label << "=====" << endl;
    for(size_t igate = 350; igate < 360; igate++) {
      cerr << "igate, hiRate, loRate, crossRate, molRate, combRate: "
           << igate << ", "
           << _hiRate[igate] << ", "
           << _loRate[igate] << ", "
           << _crossRate[igate] << ", "
           << _molRate[igate] << ", "
           << _combRate[igate] << endl;
    } // igate
  }
}
  
/////////////////////////////////////////////////////////////////
// print derived fields

void DerFieldCalcs::_printDerivedFields(ostream &out)
{
  for(size_t igate = 0; igate < _nGates; igate++) {
    out << "gate, backScatRatio, backScatCoeff, volDepol, partDepol, optDepth, exctint: "
        << igate << ", "
        << _backscatRatio[igate] << ", "
        << _backscatCoeff[igate] << ", "
        << _volDepol[igate] << ", "
        << _partDepol[igate] << ", "
        << _opticalDepth[igate] << ", "
        << _extinction[igate] << endl;
  }
}
  
/////////////////////////////////////
// set FIR filter length

void DerFieldCalcs::_setFIRFilterLen(fir_filter_len_t len)

{
  
  switch (len) {
    case FIR_LENGTH_21:
      _firLength = FIR_LEN_21;
      _firCoeff = firCoeff_21;
      break;
    case FIR_LENGTH_11:
      _firLength = FIR_LEN_11;
      _firCoeff = firCoeff_11;
    case FIR_LENGTH_7:
    default:
      _firLength = FIR_LEN_7;
      _firCoeff = firCoeff_7;
  }

  _firLenHalf = _firLength / 2;

}

/////////////////////////////////////////////
// Apply FIR filter

void DerFieldCalcs::_applyFirFilter(vector<Radx::fl32> &data)
  
{

  // compute required array size, given that we need to
  // have space for the FIR filter on each side
  
  int arrayOffset = _firLength + 1;
  int arrayLen = _nGates + 2 * arrayOffset;
  
  // allocate working array
  
  RadxArray<double> xxx_;
  double *xxx = xxx_.alloc(arrayLen) + arrayOffset;

  // copy data into array

  for (int ii = 0; ii < (int) _nGates; ii++) {
    xxx[ii] = data[ii];
  }

  // pad array at each end

  for (int ii = -_firLength; ii < 0; ii++) {
    xxx[ii] = xxx[0];
  }
  for (int ii = _nGates; ii < (int) _nGates + _firLength; ii++) {
    xxx[ii] = xxx[_nGates - 1];
  }

  // fill any holes in the data

  if (xxx[-_firLenHalf] < 0) {
    xxx[-_firLenHalf] = 0;
  }
  for (int ii = -_firLenHalf + 1; ii < (int) _nGates + _firLenHalf; ii++) {
    if (xxx[ii] < 0) {
      xxx[ii] = xxx[ii-1];
    }
  }

  // apply FIR

  for (int ii = -_firLenHalf; ii < (int) _nGates + _firLenHalf; ii++) {
    double acc = 0.0;
    int kk = ii - _firLenHalf;
    for (int jj = 0; jj < _firLength; jj++, kk++) {
      acc = acc + _firCoeff[jj] * xxx[kk];
    }
    if (ii >= 0 && ii < (int) _nGates) {
      data[ii] = acc;
    }
  } // ii

  // ensure that the value does not decrease with range

  // double minVal = data[0];
  // for (int ii = 1; ii < (int) _nGates; ii++) {
  //   if (data[ii] < minVal) {
  //     data[ii] = minVal;
  //   } else {
  //     minVal = data[ii];
  //   }
  // }

}
    
/////////////////////////////////////////////////////////////////
// Apply median filter

void DerFieldCalcs::_applyMedianFilter(int filtLen,
                                       vector<Radx::fl32> &data)

{

  // make sure filter len is odd

  size_t halfFilt = filtLen / 2;
  size_t len = halfFilt * 2 + 1;
  if (len < 3) {
    // too short
    return;
  }

  // make copy of data
  
  vector<Radx::fl32> copy = data;
  
  // apply median filter
  
  for (size_t ii = halfFilt; ii < _nGates - halfFilt; ii++) {
    
    vector<Radx::fl32> vals;
    for (size_t jj = ii - halfFilt; jj <= ii + halfFilt; jj++) {
      vals.push_back(copy[jj]);
    } // jj

    sort(vals.begin(), vals.end());
    data[ii] = vals[vals.size() / 2];
    
  } // ii

}

