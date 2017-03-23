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
// holds information on calculations of derived fields for HSRL 
//
// Brad Schoenrock, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Mar 2017
//
///////////////////////////////////////////////////////////////

#ifndef DerFieldCalcs_HH
#define DerFieldCalcs_HH

#include "CalReader.hh"
#include "FullCals.hh"
#include <string>
#include <Radx/RadxTime.hh>
#include <vector>
#include <Radx/Radx.hh>

using namespace std;

class DerFieldCalcs{
  
private:   
  vector<Radx::fl32> hiData, loData, crossData, molData;
  vector<Radx::fl32> htKm, tempK, presHpa;   
  double shotCount, power;
  FullCals fullCals;
  
  //derived quantities are not set, they are calculated. 
  vector<Radx::fl32> volDepol, backscatRatio, partDepol, backscatCoeff, extinction;
  vector<Radx::fl32> combData;
public:   
    
  DerFieldCalcs();
  
  DerFieldCalcs( FullCals fullCals_in,
		 vector<Radx::fl32> hiData_in, vector<Radx::fl32> loData_in, 
		 vector<Radx::fl32> crossData_in, vector<Radx::fl32> molData_in,
		 vector<Radx::fl32> htKm_in,vector<Radx::fl32> tempK_in, 
		 vector<Radx::fl32> presHpa_in, double shotCount_in, 
		 double power_in);
    
  void set_fullCals(FullCals fullCals_in);
  void set_hiData (vector<Radx::fl32> in);
  void set_loData (vector<Radx::fl32> in);
  void set_crossData (vector<Radx::fl32> in);
  void set_molData (vector<Radx::fl32> in);
  
  void set_htKm (vector<Radx::fl32> in);
  void set_tempK (vector<Radx::fl32> in);
  void set_presHpa (vector<Radx::fl32> in);

  void set_shotCount(double in);
  void set_power(double in);

  FullCals get_fullCals();
  
  vector<Radx::fl32> get_hiData();
  vector<Radx::fl32> get_loData();
  vector<Radx::fl32> get_crossData();
  vector<Radx::fl32> get_molData();
  
  vector<Radx::fl32> get_htKm();
  vector<Radx::fl32> get_tempK();
  vector<Radx::fl32> get_presHpa();

  double get_shotCount();
  double get_power();

  vector<Radx::fl32> get_volDepol();
  vector<Radx::fl32> get_backscatRatio();
  vector<Radx::fl32> get_partDepol();
  vector<Radx::fl32> get_backscatCoeff();
  vector<Radx::fl32> get_extinction();

  void applyCorr(); 
  
  void derive_quantities();
  
  //nonlinear count corrections
  double _nonLinCountCor(Radx::fl32 count, double deadtime, 
			 double binWid, double shotCount_in);
  
  //baseline subtraction
  double _baselineSubtract(double arrivalRate, double profile, 
			   double polarization);
  
  //background subtraction
  double _backgroundSub(double arrivalRate, double backgroundBins);
  
  //energy normalization
  double _energyNorm(double arrivalRate, double totalEnergy);
  
  //differential overlap correction the vector coresponds to hi, lo, cross, mol
  vector<double> _diffOverlapCor(vector<double> arrivalRate, vector<double> diffOverlap);
  
  //process QWP rotation
  vector<double> _processQWPRotation(vector<double> arrivalRate, vector<double> polCal);
  
  //merge hi and lo profiles 
  double _hiAndloMerge(double hiRate, double loRate);
  
  //geometric overlap correction
  double _geoOverlapCor(double arrivalRate, double geoOverlap);
  
  //volume depolarization
  double _volDepol(double crossRate, double combineRate);
  
  //backscatter ratio
  double _backscatRatio(double combineRate, double molRate);
  
  //particle depolarization
  double _partDepol(double volDepol, double backscatRatio);
  
  //intermediate field for backscatter ratio and extinction
  double _BetaMSonde(double pressure, double temperature);

  //backscatter coefficient
  double _backscatCo(double pressure, double temperature, 
		     double backscatRatio);
  
  //optical depth calculation for extinction;
  double _op_depth(double pressure, double temperature, double molRate);

  //extinction
  double _extinction(double opDepth1, double opDepth2,
		     double alt1, double alt2);
  
};
#endif
