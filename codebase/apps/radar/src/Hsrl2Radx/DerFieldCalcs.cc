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

/////////////////////////////////////////////////////////////////
// constructor

DerFieldCalcs::DerFieldCalcs(const Params &params,
                             const FullCals &fullCals,
                             size_t nGates,
                             const vector<Radx::fl32> &hiData,
                             const vector<Radx::fl32> &loData, 
                             const vector<Radx::fl32> &crossData,
                             const vector<Radx::fl32> &molData,
                             const vector<Radx::fl32> &htM,
                             const vector<Radx::fl32> &tempK, 
                             const vector<Radx::fl32> &presHpa,
                             double shotCount, 
                             double power) :
  _params(params),
  _fullCals(fullCals),
  _nGates(nGates),
  _hiData(hiData),
  _loData(loData),
  _crossData(crossData),
  _molData(molData),
  _htM(htM),
  _tempK(tempK),
  _presHpa(presHpa),
  _shotCount(shotCount),
  _power(power)

{

  // check sizes are correct

  assert(hiData.size() == _nGates);
  assert(loData.size() == _nGates);
  assert(crossData.size() == _nGates);
  assert(molData.size() == _nGates);
  assert(htM.size() == _nGates);
  assert(tempK.size() == _nGates);
  assert(presHpa.size() == _nGates);

  _nBinsPerGate = 1;
  if (_params.combine_bins_on_read) {
    _nBinsPerGate = _params.n_bins_per_gate;
  }

}

/////////////////////////////////////////////////////////////////
// apply corrections

void DerFieldCalcs::_applyCorr()
{
  
  // allocate the rate vectors

  _hiDataRate.resize(_nGates);
  _loDataRate.resize(_nGates);
  _crossDataRate.resize(_nGates);
  _molDataRate.resize(_nGates);
  _combinedRate.resize(_nGates);

  // non-linear count correction
  
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

  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "==>> hiDead, loDead, crossDead, molDead, binW: "
         << hiDeadTime << ", "
         << loDeadTime << ", "
         << crossDeadTime << ", "
         << molDeadTime << ", "
         << binW << endl;
  }

  for(size_t igate = 0; igate < _nGates; igate++) {
    _hiDataRate[igate] =
      _nonLinCountCor(_hiData[igate], hiDeadTime, binW, _shotCount); 
    _loDataRate[igate] = 
      _nonLinCountCor(_loData[igate], loDeadTime, binW, _shotCount); 
    _crossDataRate[igate] =
      _nonLinCountCor(_crossData[igate], crossDeadTime, binW, _shotCount); 
    _molDataRate[igate] =
      _nonLinCountCor(_molData[igate], molDeadTime, binW, _shotCount); 
  } // igate

  _printRateDiagnostics("countCorrection");

  // subtract baseline
  // this is a pass-through
  
  const vector<double> &blCorCombinedHi = _fullCals.getBlCorCombinedHi();
  const vector<double> &blCorCombinedLo = _fullCals.getBlCorCombinedLo();
  const vector<double> &blCorMolecular = _fullCals.getBlCorMolecular();
  const vector<double> &blCorCrossPol = _fullCals.getBlCorCrossPol();
  
  size_t calBin = _nBinsPerGate / 2;
  double polPosn = 1.0;

  for(size_t igate = 0; igate < _nGates; igate++, calBin += _nBinsPerGate) {
    _hiDataRate[igate] = _baselineSubtract(_hiDataRate[igate],
                                           blCorCombinedHi[calBin], polPosn);
    _loDataRate[igate] = _baselineSubtract(_loDataRate[igate],
                                           blCorCombinedLo[calBin], polPosn);
    _crossDataRate[igate] = _baselineSubtract(_crossDataRate[igate],
                                              blCorCrossPol[calBin], polPosn);
    _molDataRate[igate] = _baselineSubtract(_molDataRate[igate],
                                            blCorMolecular[calBin], polPosn);
  } // igate
  
  _printRateDiagnostics("baselineSubtract");

  // compute background rates from last 'n' gates
  
  double hibackgroundRate = 0.0;
  double lobackgroundRate = 0.0;
  double crossbackgroundRate = 0.0;
  double molbackgroundRate = 0.0;
  
  int nGatesBackground = _params.ngates_for_background_correction;
  int startBackgroundGate = _nGates - nGatesBackground;
  if (startBackgroundGate < 1) {
    startBackgroundGate = 1;
  }
  double bgBinCount = 0.0;
  for(int cgate = startBackgroundGate - 1; cgate < (int) _nGates; cgate++) {
    hibackgroundRate += _hiDataRate.at(cgate);
    lobackgroundRate += _loDataRate.at(cgate);
    crossbackgroundRate += _crossDataRate.at(cgate);
    molbackgroundRate += _molDataRate.at(cgate);
    bgBinCount++;	    
  }
  
  hibackgroundRate /= bgBinCount;
  lobackgroundRate /= bgBinCount;
  crossbackgroundRate /= bgBinCount;
  molbackgroundRate /= bgBinCount;

  // adjust for background rate
  
  for(size_t igate = 0; igate < _nGates; igate++) {
    _hiDataRate[igate] =
      _backgroundSub(_hiDataRate[igate], hibackgroundRate);
    _loDataRate[igate] =
      _backgroundSub(_loDataRate[igate], lobackgroundRate);
    _crossDataRate[igate] =
      _backgroundSub(_crossDataRate[igate], crossbackgroundRate);
    _molDataRate[igate] =
      _backgroundSub(_molDataRate[igate], molbackgroundRate);
  }

  if(_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "hibackgroundRate = " << hibackgroundRate<<endl;
    cerr << "lobackgroundRate = " << lobackgroundRate<<endl;
    cerr << "crossbackgroundRate = " << crossbackgroundRate<<endl;
    cerr << "molbackgroundRate = " << molbackgroundRate<<endl;
    _printRateDiagnostics("backgroundSub");
  }

  // normalize with respect to transmit energy

  for(size_t igate = 0; igate < _nGates; igate++) {
    _hiDataRate[igate] = _energyNorm(_hiDataRate[igate], _power);
    _loDataRate[igate] = _energyNorm(_loDataRate[igate], _power);
    _crossDataRate[igate] = _energyNorm(_crossDataRate[igate], _power);
    _molDataRate[igate] = _energyNorm(_molDataRate[igate], _power);
  }
  
  _printRateDiagnostics("energyNorm");

  // correct for differential overlap
  
  const vector<double> &diffGeoCombHiMol = _fullCals.getDiffGeoCombHiMol();
  const vector<double> &diffGeoCombLoMol = _fullCals.getDiffGeoCombLoMol();
  calBin = _nBinsPerGate / 2;
  for(size_t igate = 0; igate < _nGates; igate++, calBin += _nBinsPerGate) {
    _hiDataRate[igate] /= diffGeoCombHiMol[calBin];
    _loDataRate[igate] /= diffGeoCombLoMol[calBin];
    // _crossDataRate[igate] /= diffGeoCombHiMol[calBin];
  }
  _printRateDiagnostics("diffOverlapCor");

  // for now we ignore QWP rotation correction

  // merge hi and lo channels into combined rate
  
  for(size_t igate = 0; igate < _nGates; igate++) {
    _combinedRate[igate] = _hiAndloMerge(_hiDataRate[igate], 
                                         _loDataRate[igate]);
  }
  _printRateDiagnostics("hiAndloMerge", true);

  // geo correction

  const vector<double> &geoCorr = _fullCals.getGeoCorr();
  calBin = _nBinsPerGate / 2;
  for(size_t igate = 0; igate < _nGates; igate++, calBin += _nBinsPerGate) {
    double corr = geoCorr[calBin];
    _combinedRate[igate] *= corr;
    _crossDataRate[igate] *= corr;
    _molDataRate[igate] *= corr;
  }
  
  _printRateDiagnostics("geoOverlapCor", true);
  
}

/////////////////////////////////////////////////////////////////
// print diagnostics

void DerFieldCalcs::_printRateDiagnostics(const string &label,
                                          bool includeCombined /* = false */)
{
  if(_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << label << "^^^^^" << endl;
    for(size_t igate=0;igate<1;igate++) {
      cerr<<"hiDataRate["<<igate<<"]="<<_hiDataRate[igate]<<endl;
      cerr<<"loDataRate["<<igate<<"]="<<_loDataRate[igate]<<endl;
      cerr<<"crossDataRate["<<igate<<"]="<<_crossDataRate[igate]<<endl;
      cerr<<"molDataRate["<<igate<<"]="<<_molDataRate[igate]<<endl;
      if (includeCombined) {
        cerr<<"combineRate["<<igate<<"]="<<_combinedRate[igate]<<endl;
      }
    }
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
  
/////////////////////////////////////////////////////////////////
// do calculations for derived fields

void DerFieldCalcs::computeDerived()
{
  
  // apply corrections
  
  _applyCorr();

  // init arrays

  _initDerivedArrays();
  
  CalReader scanAdjust = _fullCals.getScanAdj();
  double scanAdj = 1;
  if(scanAdjust.dataTypeisNum() && 
     ((scanAdjust.getDataNum()).at(_fullCals.getBinPos()) ).size() == 1) {
    scanAdj = ((scanAdjust.getDataNum()).at(_fullCals.getBinPos())).at(0);
  }

  // vol depol

  for(size_t igate=0;igate<_nGates;igate++) {
    _volDepol[igate] = _computeVolDepol(_crossDataRate[igate], 
                                        _combinedRate[igate]);
  }
  
  // backscatter ratio
  
  for(size_t igate=0;igate<_nGates;igate++) {
    _backscatRatio[igate] = _computeBackscatRatio(_combinedRate[igate], 
                                                  _molDataRate[igate]);
  }
  
  // particle depol
  
  for(size_t igate=0;igate<_nGates;igate++) {
    _partDepol[igate] = _computePartDepol(_volDepol[igate],
                                          _backscatRatio[igate]);
  }
  
  // backscatter coefficient
  
  for(size_t igate=0;igate<_nGates;igate++) {
    _backscatCoeff[igate] = _computeBackscatCoeff(_presHpa[igate],
                                                  _tempK[igate], 
                                                  _backscatRatio[igate]);
  }
  
  // optical depth
  
  for(size_t igate = 0; igate < _nGates; igate++) {
    _opticalDepth[igate] = 
      _computeOpticalDepth(_presHpa[igate], _tempK[igate], _molDataRate[igate], scanAdj);
  }
  _filterOpticalDepth();

  // extinction

  int filterHalf = 2;
  for(size_t igate = filterHalf; igate < _nGates - filterHalf; igate++) {
    _extinction[igate] = 
      _computeExtinctionCoeff(_opticalDepth[igate - filterHalf],
                              _opticalDepth[igate + filterHalf],
                              _htM[igate - filterHalf],
                              _htM[igate + filterHalf]);
  }
                              
  // if(_params.debug >= Params::DEBUG_EXTRA) {
  //   _printDerivedFields(cerr);
  // }

}

/////////////////////////////////////////////////////////////////
// nonlinear count corrections

double DerFieldCalcs::_nonLinCountCor(Radx::fl32 count, double deadtime, 
                                      double binWid, double shotCount)
{
  if(count < 1.0) {
    return 0;
  }
  double photonRate = (count / shotCount) / binWid;
  double corrFactor = photonRate * deadtime;
  if(corrFactor > 0.99) {
    corrFactor=0.95;
  }
  return count / (1.0 - corrFactor);
}


/////////////////////////////////////////////////////////////////
// baseline subtraction

double DerFieldCalcs::_baselineSubtract(double arrivalRate, double profile, 
					double polarization)
{
  //pass through for now, not expected to significantly impact displays
  return arrivalRate;
}


/////////////////////////////////////////////////////////////////
// background subtraction

double DerFieldCalcs::_backgroundSub(double arrivalRate, double backgroundBins)
{
  //background bins is average of the last 100 bins, this can be negative
  return arrivalRate - backgroundBins;
}


/////////////////////////////////////////////////////////////////
// energy normalization

double DerFieldCalcs::_energyNorm(double arrivalRate, double totalEnergy)
{
  if(totalEnergy==0) {
    return arrivalRate;
  }
  return (arrivalRate / totalEnergy) * pow(10.0, 6.0);
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
  //pass through for now, not expected to significantly impact displays
  return hiRate;
}


/////////////////////////////////////////////////////////////////
// geometric overlap correction

double DerFieldCalcs::_geoOverlapCor(double arrivalRate, double geoOverlap)
{
  return arrivalRate * geoOverlap;
}


///////////////////////////////////////////////////////////////////
// init arrays for derived fields

void DerFieldCalcs::_initDerivedArrays()

{
  _volDepol.resize(_nGates);
  _backscatRatio.resize(_nGates);
  _partDepol.resize(_nGates);
  _backscatCoeff.resize(_nGates);
  _extinction.resize(_nGates);
  _opticalDepth.resize(_nGates);
  for (size_t ii = 0; ii < _nGates; ii++) {
    _volDepol[ii] = Radx::missingFl32;
    _backscatRatio[ii] = Radx::missingFl32;
    _partDepol[ii] = Radx::missingFl32;
    _backscatCoeff[ii] = Radx::missingFl32;
    _extinction[ii] = Radx::missingFl32;
    _opticalDepth[ii] = Radx::missingFl32;
  }
}

/////////////////////////////////////////////////////////////////
// volume depolarization

Radx::fl32 DerFieldCalcs::_computeVolDepol(double crossRate, double combineRate)
{

  if (crossRate < 1.0 || (crossRate + combineRate) < 1.0) {
    return Radx::missingFl32;
  }

  double depol = crossRate / (crossRate + combineRate);
  if (depol > 1.0) {
    return Radx::missingFl32;
  }

  return depol;

}


/////////////////////////////////////////////////////////////////
// backscatter ratio

Radx::fl32 DerFieldCalcs::_computeBackscatRatio(double combineRate, double molRate)
{

  if(combineRate < 1.0 || molRate < 1.0) {
    return Radx::missingFl32;
  }

  double ratio = combineRate / molRate;
  if (ratio < 1.0) {
    return Radx::missingFl32;
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
  
  //double pDepol2=(-d_mol+backscatRatio*volDepol)/(backscatRatio-1);
  //cerr<<"pDepol1="<<pDepol1<<endl;
  //cerr<<"pDepol2="<<pDepol2<<endl;
  
  return pDepol;

}


/////////////////////////////////////////////////////////////////
// beta M sonde

double DerFieldCalcs::_computeBetaMSonde(double pressHpa, double tempK)
{

  if(tempK <= 0.0) {
    return NAN;
  }

  double val = _BmsFactor * (pressHpa / (tempK * _BoltzmannConst));
  if (!finite(val)) {
    return NAN;
  }
  
  return val;

}


/////////////////////////////////////////////////////////////////
// backscatter coefficient

Radx::fl32 DerFieldCalcs::_computeBackscatCoeff(double pressHpa, 
                                                double tempK, 
                                                Radx::fl32 backscatRatio)
{

  if (backscatRatio == Radx::missingFl32) {
    return Radx::missingFl32;
  }

  double betaMSonde = _computeBetaMSonde(pressHpa, tempK);
  if (betaMSonde == NAN) {
    return Radx::missingFl32;
  }

  double aerosolBscat = (backscatRatio - 1.0) * betaMSonde;
  if (aerosolBscat < 0.0) {
    return Radx::missingFl32;
  }

  return aerosolBscat;

}

/////////////////////////////////////////////////////////////////
// optical depth

Radx::fl32 DerFieldCalcs::_computeOpticalDepth(double pressHpa, double tempK, 
                                               double molRate, double scanAdj)
{

  if (molRate < 1) {
    return Radx::missingFl32;
  }

  double betaMSonde =_computeBetaMSonde(pressHpa, tempK);
  if (betaMSonde == NAN) {
    return Radx::missingFl32;
  }

  //cerr << "scanAdj=" << scanAdj << endl;
  //cerr << "molRate=" << molRate << endl;
  //cerr << "betaMSonde=" << betaMSonde << endl;
   
  // double optDepth = 28.0 - log( scanAdj * molRate / betaMSonde );

  double xx = (scanAdj * molRate) / betaMSonde;
  if (xx <= 0.0) {
    return Radx::missingFl32;
  }
  double optDepth = 16.0 - 0.5 * log(xx);
  
  return optDepth;

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

  double minExtinction = 1.0e-10;
  double extinction = minExtinction;
  
  if(std::isnan(optDepth1) || std::isnan(optDepth2) || alt1 == alt2) {
    return Radx::missingFl32;
  }
  
  extinction = (optDepth1 - optDepth2) / (alt1 - alt2);

  if (extinction < minExtinction) {
    return Radx::missingFl32;
  }
  
  return extinction;

}

/////////////////////////////////////////////////////////////////
// Apply median filter to optical depth

void DerFieldCalcs::_filterOpticalDepth()

{

  // make sure filter len is odd

  size_t halfFilt = _params.optical_depth_median_filter_len / 2;
  size_t len = halfFilt * 2 + 1;
  if (len < 3) {
    return;
  }

  // make copy
  
  vector<Radx::fl32> copy = _opticalDepth;

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

    if (vals.size() == len) {
      sort(vals.begin(), vals.end());
      _opticalDepth[ii] = vals[vals.size() / 2];
    } else {
      _opticalDepth[ii] = Radx::missingFl32;
    }
    
  } // ii

}

