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
#include <assert.h>
#include <math.h>  
#include "FullCals.hh"
#include <iostream>
#include <cmath>
#include <cassert>

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
  
  CalReader dt_hi=(_fullCals.getDeadTimeHi());
  CalReader dt_lo=(_fullCals.getDeadTimeLo());
  CalReader dt_cross=(_fullCals.getDeadTimeCross());
  CalReader dt_mol=(_fullCals.getDeadTimeMol());
  CalReader binwid=(_fullCals.getBinWidth());
   
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
    
    int calGate= blCorSize/_nGates * igate +  0.5 * blCorSize/_nGates;
    
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
  
  double hibackgroundRate=0;
  double lobackgroundRate=0;
  double crossbackgroundRate=0;
  double molbackgroundRate=0;
  
  // grabs last 100 out of 4000 bins for background,
  // or fractionally adjusts for less bins
  int bgBinCount=0;
  for(int cgate=_nGates-1;cgate>=_nGates*0.975;cgate--) {
    hibackgroundRate=hibackgroundRate+_hiDataRate.at(cgate);
    lobackgroundRate=lobackgroundRate+_loDataRate.at(cgate);
    crossbackgroundRate=crossbackgroundRate+_crossDataRate.at(cgate);
    molbackgroundRate=molbackgroundRate+_molDataRate.at(cgate);
    bgBinCount++;	    
  }
  hibackgroundRate/=bgBinCount;
  lobackgroundRate/=bgBinCount;
  crossbackgroundRate/=bgBinCount;
  molbackgroundRate/=bgBinCount;
  
  for(size_t igate=0;igate<_nGates;igate++) {
    _hiDataRate.at(igate)=_backgroundSub(_hiDataRate.at(igate),
                                         hibackgroundRate);
    _loDataRate.at(igate)=_backgroundSub(_loDataRate.at(igate),
                                         lobackgroundRate);
    _crossDataRate.at(igate)=_backgroundSub(_crossDataRate.at(igate),
                                            crossbackgroundRate);
    _molDataRate.at(igate)=_backgroundSub(_molDataRate.at(igate),
                                          molbackgroundRate);
  }

  if(_params.debug >= Params::DEBUG_VERBOSE) {
    cerr<<"hibackgroundRate="<<hibackgroundRate<<'\n';
    cerr<<"lobackgroundRate="<<lobackgroundRate<<'\n';
    cerr<<"crossbackgroundRate="<<crossbackgroundRate<<'\n';
    cerr<<"molbackgroundRate="<<molbackgroundRate<<'\n';
    _printRateDiagnostics("backgroundSub");
  }
  
  for(size_t igate=0;igate<_nGates;igate++) {
    _hiDataRate.at(igate)=_energyNorm(_hiDataRate.at(igate), _power);
    _loDataRate.at(igate)=_energyNorm(_loDataRate.at(igate), _power);
    _crossDataRate.at(igate)=_energyNorm(_crossDataRate.at(igate), _power);
    _molDataRate.at(igate)=_energyNorm(_molDataRate.at(igate), _power);
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
// do calculations for derived fields

void DerFieldCalcs::computeDerived()
{
  
  // apply corrections
  
  _applyCorr();

  CalReader scanAdjust=(_fullCals.getScanAdj());
  double scan=1;

  // init arrays

  _volDepol.clear();
  _backscatRatio.clear();
  _partDepol.clear();
  _backscatCoeff.clear();
  _extinction.clear();
  _opticalDepth.clear();
  
  if(scanAdjust.dataTypeisNum() && 
     ((scanAdjust.getDataNum()).at(_fullCals.getBinPos()) ).size() == 1) {
    scan= ((scanAdjust.getDataNum()).at(_fullCals.getBinPos())).at(0);
  }

  // vol depol

  for(size_t igate=0;igate<_nGates;igate++) {
    _volDepol.push_back(_computeVolDepol( _crossDataRate.at(igate), 
                                          _combinedRate.at(igate)));
  }

  if(_params.debug >= Params::DEBUG_VERBOSE) {
    cerr<<"volDepol^^^^^"<<'\n';
    for(size_t igate=0;igate<1;igate++) {
      cerr<<"volDepol["<<igate<<"]="<<_volDepol[igate]<<'\n';
    }
  }

  // backscatter ratio

  for(size_t igate=0;igate<_nGates;igate++) {
    _backscatRatio.push_back(_computeBackscatRatio(_combinedRate.at(igate), 
                                                   _molDataRate.at(igate) ));
  }

  if(_params.debug >= Params::DEBUG_VERBOSE) {
    cerr<<"backscatRatio^^^^^"<<'\n';
    for(size_t igate=0;igate<1;igate++) {
      cerr<<"backscatRatio["<<igate<<"]="<< _backscatRatio[igate]<<'\n';
    }
  }

  // particle depol

  for(size_t igate=0;igate<_nGates;igate++) {
    _partDepol.push_back(_computePartDepol(_volDepol[igate],
                                           _backscatRatio[igate] ));
  }
  
  if(_params.debug >= Params::DEBUG_VERBOSE) {
    cerr<<"partDepol^^^^^"<<'\n';
    for(size_t igate=0;igate<1;igate++) {
      cerr<<"partDepol["<<igate<<"]="<<_partDepol[igate]<<'\n';
    }
  }

  // backscatter coefficient

  for(size_t igate=0;igate<_nGates;igate++) {
    _backscatCoeff.push_back(_computeBackscatCo(_presHpa[igate],
                                                _tempK[igate], 
                                                _backscatRatio[igate]));
  }
  
  if(_params.debug >= Params::DEBUG_VERBOSE) {
    cerr<<"backscatCo^^^^^"<<'\n';
    for(size_t igate=0;igate<1;igate++) {
      cerr<<"backscatCoeff["<<igate<<"]="<<_backscatCoeff[igate]<<'\n';
    }
  }

  // optical depth and extinction

  double opDepth1, opDepth2, alt1, alt2;
  opDepth2=_computeOptDepth(_presHpa[0],_tempK[0],_molDataRate.at(0),scan);
  alt2=_htM[0];
  _opticalDepth.push_back(opDepth2);
 
  for(size_t igate=0;igate<_nGates-1;igate++) {
    opDepth1=opDepth2;
    opDepth2=_computeOptDepth(_presHpa[igate+1],_tempK[igate+1],
                              _molDataRate.at(igate+1),scan);
    alt1=alt2;
    alt2=_htM[igate+1];
    _extinction.push_back(_computeExtinction(opDepth1, opDepth2, alt1, alt2));
    _opticalDepth.push_back(opDepth2);
  }

  if(_params.debug >= Params::DEBUG_VERBOSE) {
    cerr<<"extinction^^^^^"<<'\n';
    for(size_t igate=0;igate<1;igate++) {
      cerr<<"extinction["<<igate<<"]="<<_extinction[igate]<<'\n';
    }
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

  return arrivalRate;//note the passthrough functionality for now
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
  return arrivalRate*geoOverlap;
}


/////////////////////////////////////////////////////////////////
// volume depolarization

double DerFieldCalcs::_computeVolDepol(double crossRate, double combineRate)
{
  if(crossRate+combineRate == 0)
    return 0;
  return crossRate/(crossRate+combineRate);
}


/////////////////////////////////////////////////////////////////
// backscatter ratio

double DerFieldCalcs::_computeBackscatRatio(double combineRate, double molRate)
{
  if(molRate==0)
    return 0;
  return combineRate/molRate;
}


/////////////////////////////////////////////////////////////////
// particle depolarization

double DerFieldCalcs::_computePartDepol(double volDepol, double backscatRatio)
{
  double d_mol=2*0.000365/(1+0.000365);
  double pDepol1=volDepol/(1-1/backscatRatio)-d_mol/(backscatRatio-1);
  //double pDepol2=(-d_mol+backscatRatio*volDepol)/(backscatRatio-1);
  
  //cerr<<"pDepol1="<<pDepol1<<'\n';
  //cerr<<"pDepol2="<<pDepol2<<'\n';
  
  return pDepol1;
}


/////////////////////////////////////////////////////////////////
// beta M sonde

double DerFieldCalcs::_computeBetaMSonde(double pressure, double temperature)
{
  //If temp is 0 this causes errors but also there is a case where temp 
  //is -1.99384e+34
  if(temperature<=0)
    return 0;
  return 5.45*(550/532)*4 * pow(10,-32) * pressure /
    (temperature*1.3805604 * pow(10,-23));
}


/////////////////////////////////////////////////////////////////
// backscatter coefficient

double DerFieldCalcs::_computeBackscatCo(double pressure, double temperature, 
                                         double backscatRatio)
{
  //If temp is 0 this causes errors but also there is a case where temp 
  //is -1.99384e+34
  if(temperature<=0)
    return 0;
  
  double beta_m_sonde =_computeBetaMSonde(pressure,temperature);
  double aer_beta_bs = (backscatRatio-1)*beta_m_sonde;
  
  return aer_beta_bs;
}

/////////////////////////////////////////////////////////////////
// optical depth

double DerFieldCalcs::_computeOptDepth(double pressure, double temperature, 
                                       double molRate, double scan)
{
  double beta_m_sonde =_computeBetaMSonde(pressure,temperature);
  //cerr << "scan=" << scan << '\n';
  //cerr << "molRate=" << molRate << '\n';
  //cerr << "beta_m_sonde=" << beta_m_sonde << '\n';
   
  return log( scan * molRate / beta_m_sonde );
}


/////////////////////////////////////////////////////////////////
// extinction coefficient

double DerFieldCalcs::_computeExtinction(double opDepth1, double opDepth2,
                                         double alt1, double alt2)
{

  //cerr << "opDepth1=" << opDepth1 << '\n';
  //cerr << "opDepth2=" << opDepth2 << '\n';
  //cerr << "alt1=" << alt1 << '\n';
  //cerr << "alt2=" << alt2 << '\n';
  //cerr << "extinction=" << (opDepth1-opDepth2)/(alt1-alt2) << '\n';
    
  if(!std::isnan(opDepth1) && !std::isnan(opDepth2) && alt1!=alt2)
    {
      //cerr << "extinction=" << (opDepth1-opDepth2)/(alt1-alt2)<<'\n';
      return (opDepth1-opDepth2)/(alt1-alt2);
    }
  return 0;
}
