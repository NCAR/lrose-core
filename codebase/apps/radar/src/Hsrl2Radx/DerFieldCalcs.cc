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

}

/////////////////////////////////////////////////////////////////
// apply corrections

void DerFieldCalcs::_applyCorr()
{

  _hiDataRate.clear();
  _loDataRate.clear();
  _crossDataRate.clear();
  _molDataRate.clear();
  _combinedRate.clear();
  
  CalReader dt_hi = _fullCals.getDeadTimeHi();
  CalReader dt_lo = _fullCals.getDeadTimeLo();
  CalReader dt_cross = _fullCals.getDeadTimeCross();
  CalReader dt_mol = _fullCals.getDeadTimeMol();
  CalReader binwid = _fullCals.getBinWidth();
   
  for(size_t igate=0;igate<_nGates;igate++) {
    
    if(dt_hi.dataTypeisNum() && 
       ( ( dt_hi.getDataNum() ).at(_fullCals.getHiPos()) ).size()==1 && 
       dt_lo.dataTypeisNum() && 
       ( ( dt_lo.getDataNum() ).at(_fullCals.getLoPos()) ).size()==1 && 
       dt_cross.dataTypeisNum() && 
       ( ( dt_cross.getDataNum() ).at(_fullCals.getCrossPos()) ).size()==1 && 
       dt_mol.dataTypeisNum() &&   
       ( ( dt_mol.getDataNum() ).at(_fullCals.getMolPos()) ).size()==1 &&
       binwid.dataTypeisNum() && 
       ( ( binwid.getDataNum() ).at(_fullCals.getBinPos()) ).size()==1 ) {
      
      double hiDeadTime=((dt_hi.getDataNum()).at(_fullCals.getHiPos())).at(0);
      double loDeadTime=((dt_lo.getDataNum()).at(_fullCals.getLoPos())).at(0);
      double crossDeadTime=
        ((dt_cross.getDataNum()).at(_fullCals.getCrossPos())).at(0);
      double molDeadTime=
        ((dt_mol.getDataNum()).at(_fullCals.getMolPos())).at(0);
      double binW= ((binwid.getDataNum()).at(_fullCals.getBinPos())).at(0);

      _hiDataRate.push_back(_nonLinCountCor(_hiData[igate], 
                                            hiDeadTime, binW, _shotCount)); 
      
      _loDataRate.push_back(_nonLinCountCor(_loData[igate], 
                                            loDeadTime, binW, _shotCount)); 

      _crossDataRate.push_back(_nonLinCountCor(_crossData[igate], 
					       crossDeadTime, binW, _shotCount)); 

      _molDataRate.push_back(_nonLinCountCor(_molData[igate], 
                                             molDeadTime, binW, _shotCount)); 
    }
    
  } // igate

  _printRateDiagnostics("countCorrection");
  
  vector< vector<double> > blCor=(_fullCals.getBLCor());
  int blCorSize=(blCor.at(0)).size();
  for(size_t igate=0;igate<_nGates;igate++) {
    //need pol baseline from file to replace 1.0
    //_baselineSubtract is a passthrough function for now anyway
    
    int calGate = blCorSize/_nGates * igate +  0.5 * blCorSize/_nGates;
    
    _hiDataRate.at(igate)=_baselineSubtract(_hiDataRate.at(igate),
                                            (blCor.at(1)).at(calGate),1.0);
    _loDataRate.at(igate)=_baselineSubtract(_loDataRate.at(igate),
                                            (blCor.at(2)).at(calGate),1.0);
    _crossDataRate.at(igate)=_baselineSubtract(_crossDataRate.at(igate),
                                               (blCor.at(3)).at(calGate),1.0);
    _molDataRate.at(igate)=_baselineSubtract(_molDataRate.at(igate),
                                             (blCor.at(4)).at(calGate),1.0);
  }
  
  _printRateDiagnostics("baselineSubtract");
  
  double hibackgroundRate = 0.0;
  double lobackgroundRate = 0.0;
  double crossbackgroundRate = 0.0;
  double molbackgroundRate = 0.0;
  
  // grabs last 100 out of 4000 bins for background,
  // or fractionally adjusts for less bins
  double bgBinCount = 0.0;
  int startBackgroundGate = (int) ((double) _nGates * 0.975);
  for(int cgate = startBackgroundGate; cgate < (int) _nGates; cgate++) {
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
  
  for(size_t igate = 0; igate < _nGates; igate++) {
    _hiDataRate.at(igate) =
      _backgroundSub(_hiDataRate.at(igate), hibackgroundRate);
    _loDataRate.at(igate) =
      _backgroundSub(_loDataRate.at(igate), lobackgroundRate);
    _crossDataRate.at(igate) =
      _backgroundSub(_crossDataRate.at(igate), crossbackgroundRate);
    _molDataRate.at(igate) =
      _backgroundSub(_molDataRate.at(igate), molbackgroundRate);
  }

  if(_params.debug >= Params::DEBUG_VERBOSE) {
    cerr<<"hibackgroundRate="<<hibackgroundRate<<endl;
    cerr<<"lobackgroundRate="<<lobackgroundRate<<endl;
    cerr<<"crossbackgroundRate="<<crossbackgroundRate<<endl;
    cerr<<"molbackgroundRate="<<molbackgroundRate<<endl;
    _printRateDiagnostics("backgroundSub");
  }
  
  for(size_t igate = 0; igate < _nGates; igate++) {
    _hiDataRate.at(igate) = _energyNorm(_hiDataRate.at(igate), _power);
    _loDataRate.at(igate) = _energyNorm(_loDataRate.at(igate), _power);
    _crossDataRate.at(igate) = _energyNorm(_crossDataRate.at(igate), _power);
    _molDataRate.at(igate) = _energyNorm(_molDataRate.at(igate), _power);
  }
  
  _printRateDiagnostics("energyNorm");
  
  vector< vector<double> > diffDGeo=(_fullCals.getDiffDGeoCor());
  int diffDGeoSize=(diffDGeo.at(1)).size();	
  
  for(size_t igate=0;igate<_nGates;igate++) {
      
    int calGate= diffDGeoSize/_nGates * igate + 0.5 * diffDGeoSize/_nGates;
    
    vector<double> rates;
    vector<double> diffOverlap;
    
    rates.push_back(_hiDataRate.at(igate));
    diffOverlap.push_back( (diffDGeo.at(1)).at(calGate) );
    rates.push_back(_loDataRate.at(igate));
    diffOverlap.push_back( (diffDGeo.at(2)).at(calGate) );
    rates.push_back(_crossDataRate.at(igate));
    diffOverlap.push_back( 1.0 ); 
    // ***** need cross overlap correction from another file
    rates.push_back(_molDataRate.at(igate));
    diffOverlap.push_back( 1.0 );
    
    rates=_diffOverlapCor(rates, diffOverlap);
    
    _hiDataRate.at(igate)=rates.at(0);
    _loDataRate.at(igate)=rates.at(1);
    _crossDataRate.at(igate)=rates.at(2);
    _molDataRate.at(igate)=rates.at(3);

  }
  
  _printRateDiagnostics("diffOverlapCor");

  for(size_t igate=0;igate<_nGates;igate++) {
    vector<double> rates;
    vector<double> polCal;
    rates.push_back(_hiDataRate.at(igate));
    polCal.push_back( 1.0 );
    rates.push_back(_loDataRate.at(igate));
    polCal.push_back( 1.0 );
    rates.push_back(_crossDataRate.at(igate));
    polCal.push_back( 1.0 );
    rates.push_back(_molDataRate.at(igate));
    polCal.push_back( 1.0 );
    // need to replace 1.0 with polarization calibration info, 
    // _processQWPRotation is passthrough for now though 
    rates=_processQWPRotation(rates, polCal);
    
    _hiDataRate.at(igate)=rates.at(0);
    _loDataRate.at(igate)=rates.at(1);
    _crossDataRate.at(igate)=rates.at(2);
    _molDataRate.at(igate)=rates.at(3);
    
  }
  
  _printRateDiagnostics("processQWPRotation");

  for(size_t igate=0;igate<_nGates;igate++) {
    _combinedRate.push_back(_hiAndloMerge(_hiDataRate.at(igate), 
					  _loDataRate.at(igate)));
  }
  
  _printRateDiagnostics("hiAndloMerge", true);

  vector< vector<double> > geoDef=(_fullCals.getGeoDefCor());
  int geoDefSize=(geoDef.at(1)).size();	
  
  for(size_t igate=0;igate<_nGates;igate++) {
    int calGate= geoDefSize/_nGates * igate + 0.5*geoDefSize/_nGates;
    double calibr=(geoDef.at(1)).at(calGate);
    _combinedRate.at(igate)=_geoOverlapCor(_combinedRate.at(igate), calibr);
    _crossDataRate.at(igate)=_geoOverlapCor(_crossDataRate.at(igate), calibr);
    _molDataRate.at(igate)=_geoOverlapCor(_molDataRate.at(igate), calibr);
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
  double scan = 1;
  if(scanAdjust.dataTypeisNum() && 
     ((scanAdjust.getDataNum()).at(_fullCals.getBinPos()) ).size() == 1) {
    scan = ((scanAdjust.getDataNum()).at(_fullCals.getBinPos())).at(0);
  }

  // vol depol

  for(size_t igate=0;igate<_nGates;igate++) {
    _volDepol[igate] = _computeVolDepol(_crossDataRate.at(igate), 
                                        _combinedRate.at(igate));
  }
  
  // backscatter ratio
  
  for(size_t igate=0;igate<_nGates;igate++) {
    _backscatRatio[igate] = _computeBackscatRatio(_combinedRate.at(igate), 
                                                  _molDataRate.at(igate));
  }
  
  // particle depol
  
  for(size_t igate=0;igate<_nGates;igate++) {
    _partDepol[igate] = _computePartDepol(_volDepol[igate],
                                          _backscatRatio[igate]);
  }
  
  // backscatter coefficient
  
  for(size_t igate=0;igate<_nGates;igate++) {
    _backscatCoeff[igate] = _computeBackscatCo(_presHpa[igate],
                                               _tempK[igate], 
                                               _backscatRatio[igate]);
  }
  
  // optical depth
  
  for(size_t igate = 0; igate < _nGates; igate++) {
    _opticalDepth[igate] = 
      _computeOptDepth(_presHpa[igate], _tempK[igate], _molDataRate.at(igate), scan);
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
                              
  if(_params.debug >= Params::DEBUG_EXTRA) {
    _printDerivedFields(cerr);
  }

}

/////////////////////////////////////////////////////////////////
// nonlinear count corrections

double DerFieldCalcs::_nonLinCountCor(Radx::fl32 count, double deadtime, 
                                      double binWid, double shotCount)
{
  if(shotCount == 0) {
    return count;
  }
  double photonRate = count/shotCount / binWid;
  double corrFactor = photonRate * deadtime;
  if(corrFactor > 0.99) {
    corrFactor=0.95;
  }
  return count / (1 - corrFactor);
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
  return arrivalRate-backgroundBins;
}


/////////////////////////////////////////////////////////////////
// energy normalization

double DerFieldCalcs::_energyNorm(double arrivalRate, double totalEnergy)
{
  if(totalEnergy==0)
    return arrivalRate;
  return arrivalRate/totalEnergy * pow(10,6);
}


/////////////////////////////////////////////////////////////////
// differential overlap correction the vector coresponds
// to hi, lo, cross, mol

vector<double> DerFieldCalcs::_diffOverlapCor(vector<double> arrivalRate, 
					      vector<double> diffOverlap)
{
  assert(arrivalRate.size()==4);
  assert(diffOverlap.size()==4);
  arrivalRate.at(0) = arrivalRate.at(0)/diffOverlap.at(0);
  arrivalRate.at(1) = arrivalRate.at(1)/diffOverlap.at(1);
  arrivalRate.at(2) = arrivalRate.at(2)/(diffOverlap.at(3)*diffOverlap.at(0));
  // arrivalRate.at(3) is unchanged

  return arrivalRate;

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

  if ((crossRate + combineRate) == 0.0) {
    return Radx::missingFl32;
  }

  double depol = crossRate / (crossRate + combineRate);
  if (depol < 0.0 || depol > 1.0) {
    return Radx::missingFl32;
  }

  return depol;

}


/////////////////////////////////////////////////////////////////
// backscatter ratio

Radx::fl32 DerFieldCalcs::_computeBackscatRatio(double combineRate, double molRate)
{

  if(molRate == 0.0) {
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
  
  double d_mol = 2.0 * 0.000365 / (1.0 + 0.000365);
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

double DerFieldCalcs::_computeBetaMSonde(double pressure, double temperature)
{

  // If temp is 0 this causes errors but also there is a case where temp 
  // is -1.99384e+34

  double minVal = 0.0;

  if(temperature <= 0.0) {
    return minVal;
  }
  
  double val = ((5.45 * (550.0 / 532.0) * 4.0 * pow(10.0, -32.0) * pressure) /
                (temperature * 1.3805604 * pow(10.0,-23.0)));

  if (val < minVal) {
    val = minVal;
  }
  
  return val;

}


/////////////////////////////////////////////////////////////////
// backscatter coefficient

Radx::fl32 DerFieldCalcs::_computeBackscatCo(double pressure, 
                                             double temperature, 
                                             Radx::fl32 backscatRatio)
{

  if (backscatRatio == Radx::missingFl32) {
    return Radx::missingFl32;
  }

  //If temp is 0 this causes errors but also there is a case where temp 
  //is -1.99384e+34

  double minCoeff = 0.0;

  if(temperature <= 0.0) {
    return Radx::missingFl32;
  }
  
  double beta_m_sonde = _computeBetaMSonde(pressure, temperature);
  double aer_beta_bs = (backscatRatio - 1.0) * beta_m_sonde;

  if (aer_beta_bs < minCoeff) {
    return Radx::missingFl32;
  }

  return aer_beta_bs;

}

/////////////////////////////////////////////////////////////////
// optical depth

Radx::fl32 DerFieldCalcs::_computeOptDepth(double pressure, double temperature, 
                                           double molRate, double scan)
{
  double beta_m_sonde =_computeBetaMSonde(pressure,temperature);
  //cerr << "scan=" << scan << endl;
  //cerr << "molRate=" << molRate << endl;
  //cerr << "beta_m_sonde=" << beta_m_sonde << endl;
   
  double optDepth = 28.0 - log( scan * molRate / beta_m_sonde );

  // if (optDepth > 10.0) {
  //   return Radx::missingFl32;
  // }

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

