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
// DerFieldCalcs.hh
//
// calculations of derived fields for HSRL 
//
// Brad Schoenrock, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Mar 2017
//
///////////////////////////////////////////////////////////////

#ifndef DerFieldCalcs_HH
#define DerFieldCalcs_HH

#include "Params.hh"
#include "FullCals.hh"
#include <string>
#include <Radx/RadxTime.hh>
#include <vector>
#include <Radx/Radx.hh>

using namespace std;

class DerFieldCalcs{
  
public:   

  // constructor

  DerFieldCalcs(const Params &params,
                const FullCals &fullCals,
                size_t nGates,
                const Radx::fl32 *hiData,
                const Radx::fl32 *loData, 
                const Radx::fl32 *crossData,
                const Radx::fl32 *molData,
                const Radx::fl32 *htM,
                const Radx::fl32 *tempK, 
                const Radx::fl32 *presHpa,
                double shotCount, 
                double power);
    
  // compute derived quantities

  void computeDerived();
  
  // get methods
  
  vector<Radx::fl32> &getVolDepol() { return _volDepol; }
  vector<Radx::fl32> &getBackscatRatio() { return _backscatRatio; }
  vector<Radx::fl32> &getPartDepol() { return _partDepol; }
  vector<Radx::fl32> &getBackscatCoeff() { return _backscatCoeff; }
  vector<Radx::fl32> &getExtinctionCoeff() { return _extinction; }
  vector<Radx::fl32> &getOpticalDepth() { return _opticalDepth; }

private:   

  const Params &_params;
  const FullCals &_fullCals;

  static const double _BmsFactor;
  static const double _BoltzmannConst;
  static const double _depolFactor;

  // input data
  
  size_t _nGates;
  size_t _nBinsPerGate;
  vector<Radx::fl32> _hiData;
  vector<Radx::fl32> _loData;
  vector<Radx::fl32> _crossData;
  vector<Radx::fl32> _molData;
  vector<Radx::fl32> _htM;   
  vector<Radx::fl32> _tempK;   
  vector<Radx::fl32> _presHpa;   
  double _shotCount, _power;
  
  // derived quantities 

  vector<Radx::fl32> _volDepol;
  vector<Radx::fl32> _backscatRatio;
  vector<Radx::fl32> _partDepol;
  vector<Radx::fl32> _backscatCoeff;
  vector<Radx::fl32> _extinction;
  vector<Radx::fl32> _opticalDepth;

  vector<Radx::fl32> _hiDataRate;
  vector<Radx::fl32> _loDataRate;
  vector<Radx::fl32> _crossDataRate;
  vector<Radx::fl32> _molDataRate;
  vector<Radx::fl32> _combinedRate;
  
  // apply corrections

  void _applyCorr(); 
  void _applyNonLinearCountCorr();
  void _applyBaselineCorr();
  void _applyBackgroundCorr();
  void _applyEnergyNorm();
  void _applyDiffGeoCorr();
  void _applyGeoCorr();

  // nonlinear count corrections

  double _nonLinCountCor(Radx::fl32 count, double deadtime, 
                         double binWid, double shotCount);
  
  // baseline subtraction

  double _baselineSubtract(double arrivalRate, double profile, 
                           double polarization);
  
  // background subtraction

  double _backgroundSubtract(double arrivalRate, double backgroundBins);
  
  // energy normalization

  double _energyNorm(double arrivalRate, double totalEnergy);
  
  // process QWP rotation

  vector<double> _processQWPRotation(vector<double> arrivalRate,
                                     vector<double> polCal);
  
  // merge hi and lo profiles 

  double _hiAndloMerge(double hiRate, double loRate);
  
  // geometric overlap correction

  double _geoOverlapCor(double arrivalRate, double geoOverlap);

  // initialize the arrays for derived fields

  void _initDerivedArrays();
  
  // compute volume depolarization

  Radx::fl32 _computeVolDepol(double crossRate, double combineRate);
  
  // compute backscatter ratio

  Radx::fl32 _computeBackscatRatio(double combineRate, double molRate);
  
  // compute particle depolarization
  Radx::fl32 _computePartDepol(Radx::fl32 volDepol, 
                               Radx::fl32 backscatRatio);
  
  // compute intermediate field for backscatter ratio and extinction

  double _computeBetaMSonde(double pressHpa, double tempK);

  // compute backscatter coefficient

  Radx::fl32 _computeBackscatCoeff(double pressHpa, double tempK, 
                                   Radx::fl32 backscatRatio);
  
  // compute optical depth calculation for extinction;

  Radx::fl32 _computeOpticalDepth(double pressHpa, double tempK,
                                  double molRate, double scanAdj);

  // compute extinction

  Radx::fl32 _computeExtinctionCoeff(Radx::fl32 optDepth1, 
                                     Radx::fl32 optDepth2,
                                     double alt1, double alt2);

  // printing
  
  void _printRateDiagnostics(const string &label,
                             bool includeCombined = false);

  void _printDerivedFields(ostream &out);

  // filtering

  void _filterOpticalDepth();

};
#endif
