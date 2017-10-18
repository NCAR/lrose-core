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
                const vector<Radx::fl32> &hiData,
                const vector<Radx::fl32> &loData, 
                const vector<Radx::fl32> &crossData,
                const vector<Radx::fl32> &molData,
                const vector<Radx::fl32> &htM,
                const vector<Radx::fl32> &tempK, 
                const vector<Radx::fl32> &presHpa,
                double shotCount, 
                double power);
    
  // compute derived quantities

  void computeDerived();
  
  // get methods
  
  const vector<Radx::fl32> &getVolDepol() const { return _volDepol; }
  const vector<Radx::fl32> &getBackscatRatio() const { return _backscatRatio; }
  const vector<Radx::fl32> &getPartDepol() const { return _partDepol; }
  const vector<Radx::fl32> &getBackscatCoeff() const { return _backscatCoeff; }
  const vector<Radx::fl32> &getExtinction() const { return _extinction; }
  const vector<Radx::fl32> &getOpticalDepth() const { return _opticalDepth; }

private:   

  const Params &_params;

  // input data

  const FullCals &_fullCals;
  size_t _nGates;
  const vector<Radx::fl32> &_hiData;
  const vector<Radx::fl32> &_loData;
  const vector<Radx::fl32> &_crossData;
  const vector<Radx::fl32> &_molData;
  const vector<Radx::fl32> &_htM;   
  const vector<Radx::fl32> &_tempK;   
  const vector<Radx::fl32> &_presHpa;   
  double _shotCount, _power;
  
  // derived quantities 

  vector<Radx::fl32> _volDepol;
  vector<Radx::fl32> _backscatRatio;
  vector<Radx::fl32> _partDepol;
  vector<Radx::fl32> _backscatCoeff;
  vector<Radx::fl32> _extinction;
  vector<Radx::fl32> _opticalDepth;
  vector<Radx::fl32> _combData;

  vector<Radx::fl32> _hiDataRate;
  vector<Radx::fl32> _loDataRate;
  vector<Radx::fl32> _crossDataRate;
  vector<Radx::fl32> _molDataRate;
  vector<Radx::fl32> _combineRate;
  
  // apply corrections

  void _applyCorr(); 

  // nonlinear count corrections
  double _nonLinCountCor(Radx::fl32 count, double deadtime, 
                         double binWid, double shotCount);
  
  // baseline subtraction
  double _baselineSubtract(double arrivalRate, double profile, 
                                  double polarization);
  
  // background subtraction
  double _backgroundSub(double arrivalRate, double backgroundBins);
  
  // energy normalization
  double _energyNorm(double arrivalRate, double totalEnergy);
  
  // differential overlap correction the vector coresponds to hi, lo, cross, mol
  vector<double> _diffOverlapCor(vector<double> arrivalRate, vector<double> diffOverlap);
  
  // process QWP rotation
  vector<double> _processQWPRotation(vector<double> arrivalRate, vector<double> polCal);
  
  // merge hi and lo profiles 
  double _hiAndloMerge(double hiRate, double loRate);
  
  // geometric overlap correction
  double _geoOverlapCor(double arrivalRate, double geoOverlap);
  
  // volume depolarization
  double _computeVolDepol(double crossRate, double combineRate);
  
  // backscatter ratio
  double _computeBackscatRatio(double combineRate, double molRate);
  
  // particle depolarization
  double _computePartDepol(double volDepol, double backscatRatio);
  
  // intermediate field for backscatter ratio and extinction
  double _computeBetaMSonde(double pressure, double temperature);

  // backscatter coefficient
  double _computeBackscatCo(double pressure, double temperature, 
                            double backscatRatio);
  
  // optical depth calculation for extinction;
  double _computeOptDepth(double pressure, double temperature, double molRate,double scan);

  // extinction
  double _computeExtinction(double opDepth1, double opDepth2,
                            double alt1, double alt2);
  
};
#endif
