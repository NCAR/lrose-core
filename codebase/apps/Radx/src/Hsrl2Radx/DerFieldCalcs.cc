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

using namespace std;


DerFieldCalcs::DerFieldCalcs(FullCals fullCals_in,
		 vector<Radx::fl32> hiData_in, vector<Radx::fl32> loData_in, 
		 vector<Radx::fl32> crossData_in, vector<Radx::fl32> molData_in,
			     vector<Radx::fl32> tempK_in, vector<Radx::fl32> presHpa_in,  
		 double shotCount_in, double power_in)
{
  fullCals=fullCals_in;
  
  hiData=hiData_in;
  loData=loData_in;
  crossData=crossData_in;
  molData=molData_in;
  
  tempK=tempK_in;
  presHpa=presHpa_in;
  
  shotCount=shotCount_in;
  power=power_in;
  
  if( hiData.size() == loData.size() &&
      hiData.size() == crossData.size() &&
      hiData.size() == molData.size() )
    {
      applyCorr();
      if( hiData.size() == combData.size() )
	derive_quantities();
    }
  
}

void DerFieldCalcs::set_fullCals(FullCals fullCals_in)
{
  fullCals=fullCals_in;
  if( hiData.size() == loData.size() &&
      hiData.size() == crossData.size() &&
      hiData.size() == molData.size() )
    {
      applyCorr();
      if( hiData.size() == combData.size() )
	derive_quantities();
    }
}
void DerFieldCalcs::set_hiData (vector<Radx::fl32> in)
{
  hiData=in;
  if( hiData.size() == loData.size() &&
      hiData.size() == crossData.size() &&
      hiData.size() == molData.size() )
    {
      applyCorr();
      if( hiData.size() == combData.size() )
	derive_quantities();
    }
}
void DerFieldCalcs::set_loData (vector<Radx::fl32> in)
{
  loData=in;
  if( hiData.size() == loData.size() &&
      hiData.size() == crossData.size() &&
      hiData.size() == molData.size() )
    {
      applyCorr();
      if( hiData.size() == combData.size() )
	derive_quantities();
    }
}
void DerFieldCalcs::set_crossData (vector<Radx::fl32> in)
{
  crossData=in;
  if( hiData.size() == loData.size() &&
      hiData.size() == crossData.size() &&
      hiData.size() == molData.size() )
    {
      applyCorr();
      if( hiData.size() == combData.size() )
	derive_quantities();
    }
}
void DerFieldCalcs::set_molData (vector<Radx::fl32> in)
{
  molData=in;
  if( hiData.size() == loData.size() &&
      hiData.size() == crossData.size() &&
      hiData.size() == molData.size() )
    {
      applyCorr();
      if( hiData.size() == combData.size() )
	derive_quantities();
    }
}

void DerFieldCalcs::set_tempK (vector<Radx::fl32> in)
{
  tempK=in;
  if( hiData.size() == loData.size() &&
      hiData.size() == crossData.size() &&
      hiData.size() == molData.size() )
    {
      applyCorr();
      if( hiData.size() == combData.size() )
	derive_quantities();
    }
}
void DerFieldCalcs::set_presHpa (vector<Radx::fl32> in)
{
  presHpa=in;
  if( hiData.size() == loData.size() &&
      hiData.size() == crossData.size() &&
      hiData.size() == molData.size() )
    {
      applyCorr();
      if( hiData.size() == combData.size() )
	derive_quantities();
    }
}

void DerFieldCalcs::set_shotCount(double in)
{
  shotCount=in;
  if( hiData.size() == loData.size() &&
      hiData.size() == crossData.size() &&
      hiData.size() == molData.size() )
    {
      applyCorr();
      if( hiData.size() == combData.size() )
	derive_quantities();
    }
}
void DerFieldCalcs::set_power(double in)
{
  power=in;
  if( hiData.size() == loData.size() &&
      hiData.size() == crossData.size() &&
      hiData.size() == molData.size() )
    {
      applyCorr();
      if( hiData.size() == combData.size() )
	derive_quantities();
    }
}

FullCals DerFieldCalcs::get_fullCals()
{return fullCals;}

vector<Radx::fl32> DerFieldCalcs::get_hiData()
{return hiData;}
vector<Radx::fl32> DerFieldCalcs::get_loData()
{return loData;}
vector<Radx::fl32> DerFieldCalcs::get_crossData()
{return crossData;}
vector<Radx::fl32> DerFieldCalcs::get_molData()
{return molData;}

vector<Radx::fl32> DerFieldCalcs::get_tempK()
{return tempK;}
vector<Radx::fl32> DerFieldCalcs::get_presHpa()
{return presHpa;}

double DerFieldCalcs::get_shotCount()
{return shotCount;}
double DerFieldCalcs::get_power()
{return power;}

vector<Radx::fl32> DerFieldCalcs::get_volDepol()
{return volDepol;}
vector<Radx::fl32> DerFieldCalcs::get_backscatRatio()
{return backscatRatio;}
vector<Radx::fl32> DerFieldCalcs::get_partDepol()
{return partDepol;}
vector<Radx::fl32> DerFieldCalcs::get_backscatCoeff()
{return backscatCoeff;}
vector<Radx::fl32> DerFieldCalcs::get_extinction()
{return extinction;}



void DerFieldCalcs::applyCorr()
{
  bool debug=false;
  
  int nGates=hiData.size();
  
  vector<Radx::fl32> hiDataRate;
  vector<Radx::fl32> loDataRate;
  vector<Radx::fl32> crossDataRate;
  vector<Radx::fl32> molDataRate;
  
  CalReader dt_hi=(fullCals.getDeadTimeHi());
  CalReader dt_lo=(fullCals.getDeadTimeLo());
  CalReader dt_cross=(fullCals.getDeadTimeCross());
  CalReader dt_mol=(fullCals.getDeadTimeMol());
  CalReader binwid=(fullCals.getBinWidth());
   
  for(int igate=0;igate<nGates;igate++)
    {
      if(dt_hi.dataTypeisNum() && 
	 ( ( dt_hi.getDataNum() ).at(fullCals.getHiPos()) ).size()==1 && 
	 dt_lo.dataTypeisNum() && 
	 ( ( dt_lo.getDataNum() ).at(fullCals.getLoPos()) ).size()==1 && 
	 dt_cross.dataTypeisNum() && 
	 ( ( dt_cross.getDataNum() ).at(fullCals.getCrossPos()) ).size()==1 && 
	 dt_mol.dataTypeisNum() &&   
	 ( ( dt_mol.getDataNum() ).at(fullCals.getMolPos()) ).size()==1 &&
	 binwid.dataTypeisNum() && 
	 ( ( binwid.getDataNum() ).at(fullCals.getBinPos()) ).size()==1 )
	{
	  
	  double hiDeadTime=((dt_hi.getDataNum()).at(fullCals.getHiPos())).at(0);
	  double loDeadTime=((dt_lo.getDataNum()).at(fullCals.getLoPos())).at(0);
	  double crossDeadTime=((dt_cross.getDataNum()).at(fullCals.getCrossPos())).at(0);
	  double molDeadTime=((dt_mol.getDataNum()).at(fullCals.getMolPos())).at(0);
	  double binW= ((binwid.getDataNum()).at(fullCals.getBinPos())).at(0);
	  hiDataRate.push_back(_nonLinCountCor(hiData[igate], hiDeadTime, binW, shotCount)); 
	  loDataRate.push_back(_nonLinCountCor(loData[igate], loDeadTime, binW, shotCount)); 
	  crossDataRate.push_back(_nonLinCountCor(crossData[igate], 
						  crossDeadTime, binW, shotCount)); 
	  molDataRate.push_back(_nonLinCountCor(molData[igate], 
						molDeadTime, binW, shotCount)); 
	}
      
    }
  
  if(debug)
    for(int igate=0;igate<1;igate++)
      {
	cout<<"hiDataRate["<<igate<<"]="<<hiDataRate[igate]<<'\n';
	cout<<"loDataRate["<<igate<<"]="<<loDataRate[igate]<<'\n';
	cout<<"crossDataRate["<<igate<<"]="<<crossDataRate[igate]<<'\n';
	cout<<"molDataRate["<<igate<<"]="<<molDataRate[igate]<<'\n';
      }
  if(debug)
    cout<<"baselineSubtract^^^^^"<<'\n';
  
  vector< vector<double> > blCor=(fullCals.getBLCor());
  int blCorSize=(blCor.at(0)).size();
  for(int igate=0;igate<nGates;igate++)
    {
      //need pol baseline from file to replace 1.0
      //_baselineSubtract is a passthrough function for now anyway
      
      int calGate= blCorSize/nGates * igate +  0.5 * blCorSize/nGates;
      
      hiDataRate.at(igate)=_baselineSubtract(hiDataRate.at(igate),
					     (blCor.at(1)).at(calGate),1.0);
      loDataRate.at(igate)=_baselineSubtract(loDataRate.at(igate),
					     (blCor.at(2)).at(calGate),1.0);
      crossDataRate.at(igate)=_baselineSubtract(crossDataRate.at(igate),
						(blCor.at(3)).at(calGate),1.0);
      molDataRate.at(igate)=_baselineSubtract(molDataRate.at(igate),
					      (blCor.at(4)).at(calGate),1.0);
    }
  
  if(debug)
    for(int igate=0;igate<1;igate++)
      {
	cout<<"hiDataRate["<<igate<<"]="<<hiDataRate[igate]<<'\n';
	cout<<"loDataRate["<<igate<<"]="<<loDataRate[igate]<<'\n';
	cout<<"crossDataRate["<<igate<<"]="<<crossDataRate[igate]<<'\n';
	cout<<"molDataRate["<<igate<<"]="<<molDataRate[igate]<<'\n';
      }
  
  double hibackgroundRate=0;
  double lobackgroundRate=0;
  double crossbackgroundRate=0;
  double molbackgroundRate=0;
  if(debug)
    cout<<"backgroundSub^^^^^"<<'\n';
  
  //grabs last 100 out of 4000 bins for background, or fractionally adjusts for less bins
  int bgBinCount=0;
  for(int cgate=nGates-1;cgate>=nGates*0.975;cgate--)
    {
      hibackgroundRate=hibackgroundRate+hiDataRate.at(cgate);
      lobackgroundRate=lobackgroundRate+loDataRate.at(cgate);
      crossbackgroundRate=crossbackgroundRate+crossDataRate.at(cgate);
      molbackgroundRate=molbackgroundRate+molDataRate.at(cgate);
      bgBinCount++;	    
    }
  hibackgroundRate/=bgBinCount;
  lobackgroundRate/=bgBinCount;
  crossbackgroundRate/=bgBinCount;
  molbackgroundRate/=bgBinCount;
  
  for(int igate=0;igate<nGates;igate++)
    {
      hiDataRate.at(igate)=_backgroundSub(hiDataRate.at(igate),hibackgroundRate);
      loDataRate.at(igate)=_backgroundSub(loDataRate.at(igate),lobackgroundRate);
      crossDataRate.at(igate)=_backgroundSub(crossDataRate.at(igate),crossbackgroundRate);
      molDataRate.at(igate)=_backgroundSub(molDataRate.at(igate),molbackgroundRate);
    }
  if(debug)
    {
      cout<<"hibackgroundRate="<<hibackgroundRate<<'\n';
      cout<<"lobackgroundRate="<<lobackgroundRate<<'\n';
      cout<<"crossbackgroundRate="<<crossbackgroundRate<<'\n';
      cout<<"molbackgroundRate="<<molbackgroundRate<<'\n';
      
      for(int igate=0;igate<1;igate++)
	{
	  cout<<"hiDataRate["<<igate<<"]="<<hiDataRate[igate]<<'\n';
	  cout<<"loDataRate["<<igate<<"]="<<loDataRate[igate]<<'\n';
	  cout<<"crossDataRate["<<igate<<"]="<<crossDataRate[igate]<<'\n';
	  cout<<"molDataRate["<<igate<<"]="<<molDataRate[igate]<<'\n';
	}
      
      cout<<"energyNorm^^^^^"<<'\n';
    }
  
  for(int igate=0;igate<nGates;igate++)
    {
      hiDataRate.at(igate)=_energyNorm(hiDataRate.at(igate), power);
      loDataRate.at(igate)=_energyNorm(loDataRate.at(igate), power);
      crossDataRate.at(igate)=_energyNorm(crossDataRate.at(igate), power);
      molDataRate.at(igate)=_energyNorm(molDataRate.at(igate), power);
    }
  
  if(debug)
    for(int igate=0;igate<1;igate++)
      {
	cout<<"hiDataRate["<<igate<<"]="<<hiDataRate[igate]<<'\n';
	cout<<"loDataRate["<<igate<<"]="<<loDataRate[igate]<<'\n';
	cout<<"crossDataRate["<<igate<<"]="<<crossDataRate[igate]<<'\n';
	cout<<"molDataRate["<<igate<<"]="<<molDataRate[igate]<<'\n';
      }
  if(debug)
    cout<<"diffOverlapCor^^^^^"<<'\n';
  
  vector< vector<double> > diffDGeo=(fullCals.getDiffDGeoCor());
  int diffDGeoSize=(diffDGeo.at(1)).size();	
  
  for(int igate=0;igate<nGates;igate++)
    {
      
      int calGate= diffDGeoSize/nGates * igate + 0.5 * diffDGeoSize/nGates;
      
      vector<double> rates;
      vector<double> diffOverlap;
      
      rates.push_back(hiDataRate.at(igate));
      diffOverlap.push_back( (diffDGeo.at(1)).at(calGate) );
      rates.push_back(loDataRate.at(igate));
      diffOverlap.push_back( (diffDGeo.at(2)).at(calGate) );
      rates.push_back(crossDataRate.at(igate));
      diffOverlap.push_back( 1.0 ); 
      // ***** need cross overlap correction from another file
      rates.push_back(molDataRate.at(igate));
      diffOverlap.push_back( 1.0 );
      
      rates=_diffOverlapCor(rates, diffOverlap);
      
      hiDataRate.at(igate)=rates.at(0);
      loDataRate.at(igate)=rates.at(1);
      crossDataRate.at(igate)=rates.at(2);
      molDataRate.at(igate)=rates.at(3);
    }
  
  if(debug)
    for(int igate=0;igate<1;igate++)
      {
	cout<<"hiDataRate["<<igate<<"]="<<hiDataRate[igate]<<'\n';
	cout<<"loDataRate["<<igate<<"]="<<loDataRate[igate]<<'\n';
	cout<<"crossDataRate["<<igate<<"]="<<crossDataRate[igate]<<'\n';
	cout<<"molDataRate["<<igate<<"]="<<molDataRate[igate]<<'\n';
      }
  if(debug)
    cout<<"processQWPRotation^^^^^"<<'\n';
  
  for(int igate=0;igate<nGates;igate++)
    {
      vector<double> rates;
      vector<double> polCal;
      rates.push_back(hiDataRate.at(igate));
      polCal.push_back( 1.0 );
      rates.push_back(loDataRate.at(igate));
      polCal.push_back( 1.0 );
      rates.push_back(crossDataRate.at(igate));
      polCal.push_back( 1.0 );
      rates.push_back(molDataRate.at(igate));
      polCal.push_back( 1.0 );
      // need to replace 1.0 with polarization calibration info, 
      // _processQWPRotation is passthrough for now though 
      rates=_processQWPRotation(rates, polCal);
      
      hiDataRate.at(igate)=rates.at(0);
      loDataRate.at(igate)=rates.at(1);
      crossDataRate.at(igate)=rates.at(2);
      molDataRate.at(igate)=rates.at(3);
      
    }
  
  if(debug)
    for(int igate=0;igate<1;igate++)
      {
	cout<<"hiDataRate["<<igate<<"]="<<hiDataRate[igate]<<'\n';
	cout<<"loDataRate["<<igate<<"]="<<loDataRate[igate]<<'\n';
	cout<<"crossDataRate["<<igate<<"]="<<crossDataRate[igate]<<'\n';
	cout<<"molDataRate["<<igate<<"]="<<molDataRate[igate]<<'\n';
      }
  if(debug)
    cout<<"hiAndloMerge^^^^^"<<'\n';
  
  vector<Radx::fl32> combineRate;
  
  for(int igate=0;igate<nGates;igate++)
    {
      combineRate.push_back(_hiAndloMerge(hiDataRate.at(igate), loDataRate.at(igate)));
    }
  
  if(debug)
    for(int igate=0;igate<1;igate++)
      {
	cout<<"hiDataRate["<<igate<<"]="<<hiDataRate[igate]<<'\n';
	cout<<"loDataRate["<<igate<<"]="<<loDataRate[igate]<<'\n';
	cout<<"crossDataRate["<<igate<<"]="<<crossDataRate[igate]<<'\n';
	cout<<"molDataRate["<<igate<<"]="<<molDataRate[igate]<<'\n';
	cout<<"combineRate["<<igate<<"]="<<combineRate[igate]<<'\n';
      }
  if(debug)
    cout<<"geoOverlapCor^^^^^"<<'\n';
  
  vector< vector<double> > geoDef=(fullCals.getGeoDefCor());
  int geoDefSize=(geoDef.at(1)).size();	
  
  for(int igate=0;igate<nGates;igate++)
    {
      int calGate= geoDefSize/nGates * igate + 0.5*geoDefSize/nGates;
      
      double calibr=(geoDef.at(1)).at(calGate);
      
      combineRate.at(igate)=_geoOverlapCor(combineRate.at(igate), calibr);
      crossDataRate.at(igate)=_geoOverlapCor(crossDataRate.at(igate), calibr);
      molDataRate.at(igate)=_geoOverlapCor(molDataRate.at(igate), calibr);
    }
  
  if(debug)
    for(int igate=0;igate<1;igate++)
      {
	cout<<"hiDataRate["<<igate<<"]="<<hiDataRate[igate]<<'\n';
	cout<<"loDataRate["<<igate<<"]="<<loDataRate[igate]<<'\n';
	cout<<"crossDataRate["<<igate<<"]="<<crossDataRate[igate]<<'\n';
	cout<<"molDataRate["<<igate<<"]="<<molDataRate[igate]<<'\n';
	cout<<"combineRate["<<igate<<"]="<<combineRate[igate]<<'\n';
      }      
  
  hiData=hiDataRate;
  loData=loDataRate;
  crossData=crossDataRate;
  molData=molDataRate;
  combData=combineRate;
  
}

//do calculations for derived fields

void DerFieldCalcs::derive_quantities()
{
   
  bool debug=false;
   
  vector<Radx::fl32> crossDataRate=crossData;
  vector<Radx::fl32> molDataRate=molData;
  vector<Radx::fl32> combineRate=combData;
  
  int nGates=crossDataRate.size();
   
  for(int igate=0;igate<nGates;igate++)
    {
      volDepol.push_back(_volDepol( crossDataRate.at(igate), combineRate.at(igate)));
    }

  if(debug)
    for(int igate=0;igate<1;igate++)
      cout<<"volDepol["<<igate<<"]="<<volDepol[igate]<<'\n';
  
  if(debug)
    cout<<"backscatRatio^^^^^"<<'\n';
    
  for(int igate=0;igate<nGates;igate++)
    {
      backscatRatio.push_back(_backscatRatio(combineRate.at(igate), 
					     molDataRate.at(igate) ));
    }

  if(debug)
    for(int igate=0;igate<1;igate++)
      cout<<"backscatRatio["<<igate<<"]="<<backscatRatio[igate]<<'\n';
  if(debug)
    cout<<"partDepol^^^^^"<<'\n';
  
  for(int igate=0;igate<nGates;igate++)
    {
      partDepol.push_back(_partDepol(volDepol[igate], backscatRatio[igate] ));
    }
  
  if(debug)
    for(int igate=0;igate<1;igate++)
      cout<<"partDepol["<<igate<<"]="<<partDepol[igate]<<'\n';
  if(debug)
    cout<<"backscatCo^^^^^"<<'\n';
  
  for(int igate=0;igate<nGates;igate++)
    {
      backscatCoeff.push_back(_backscatCo(presHpa[igate], tempK[igate], 
					  backscatRatio[igate]));
    }
  
  if(debug)
    for(int igate=0;igate<1;igate++)
      cout<<"backscatCoeff["<<igate<<"]="<<backscatCoeff[igate]<<'\n';
  if(debug)
    cout<<"extinction^^^^^"<<'\n';
  
  for(int igate=0;igate<nGates;igate++)
    {
      extinction.push_back(1.0);
      //there is no extinction calculation, so have placeholder here for now. 
    }
  if(debug)
    for(int igate=0;igate<1;igate++)
      cout<<"extinction["<<igate<<"]="<<extinction[igate]<<'\n';
  
  
}


//nonlinear count corrections
double DerFieldCalcs::_nonLinCountCor(Radx::fl32 count, double deadtime, 
				      double binWid, double shotCount_in)
{
  if(shotCount_in==0)
    return count;
  double photonRate=count/shotCount_in/binWid;
  double corrFactor=photonRate*deadtime;
  if(corrFactor > 0.99)
    corrFactor=0.95;
  return count/(1-corrFactor);
}


//baseline subtraction
double DerFieldCalcs::_baselineSubtract(double arrivalRate, double profile, 
					double polarization)
{
  //pass through for now, not expected to significantly impact displays
  return arrivalRate;
}


//background subtraction
double DerFieldCalcs::_backgroundSub(double arrivalRate, double backgroundBins)
{
  //background bins is average of the last 100 bins, this can be negative
  return arrivalRate-backgroundBins;
}


//energy normalization
double DerFieldCalcs::_energyNorm(double arrivalRate, double totalEnergy)
{
  if(totalEnergy==0)
    return arrivalRate;
  return arrivalRate/totalEnergy * pow(10,6);
}


//differential overlap correction the vector coresponds to hi, lo, cross, mol
vector<double> DerFieldCalcs::_diffOverlapCor(vector<double> arrivalRate, vector<double> diffOverlap)
{
  assert(arrivalRate.size()==4);
  assert(diffOverlap.size()==4);
  arrivalRate.at(0) = arrivalRate.at(0)/diffOverlap.at(0);
  arrivalRate.at(1) = arrivalRate.at(1)/diffOverlap.at(1);
  arrivalRate.at(2) = arrivalRate.at(2)/(diffOverlap.at(3)*diffOverlap.at(0));
  // arrivalRate.at(3) is unchanged

  return arrivalRate;//note the passthrough functionality for now
}

//process QWP rotation
vector<double> DerFieldCalcs::_processQWPRotation(vector<double> arrivalRate, vector<double> polCal)
{
  return arrivalRate;
}


//merge hi and lo profiles 
double DerFieldCalcs::_hiAndloMerge(double hiRate, double loRate)
{  
  //pass through for now, not expected to significantly impact displays
  return hiRate;
}


//geometric overlap correction
double DerFieldCalcs::_geoOverlapCor(double arrivalRate, double geoOverlap)
{
  return arrivalRate*geoOverlap;
}


//volume depolarization
double DerFieldCalcs::_volDepol(double crossRate, double combineRate)
{
  if(crossRate+combineRate == 0)
    return 0;
  return crossRate/(crossRate+combineRate);
}


//backscatter ratio
double DerFieldCalcs::_backscatRatio(double combineRate, double molRate)
{
  if(molRate==0)
    return 0;
  return combineRate/molRate;
}


//particle depolarization
double DerFieldCalcs::_partDepol(double volDepol, double backscatRatio)
{
  double d_mol=2*0.000365/(1+0.000365);
  double pDepol1=volDepol/(1-1/backscatRatio)-d_mol/(backscatRatio-1);
  //double pDepol2=(-d_mol+backscatRatio*volDepol)/(backscatRatio-1);
  
  //cout<<"pDepol1="<<pDepol1<<'\n';
  //cout<<"pDepol2="<<pDepol2<<'\n';
  
  return pDepol1;
}


//backscatter coefficient
double DerFieldCalcs::_backscatCo(double pressure, double temperature, 
				  double backscatRatio)
{
  //If temp is 0 this causes errors but also there is a case where temp is -1.99384e+34
  if(temperature<=0)
    return 0;
  
  double beta_m_sonde = 5.45*(550/532)*4 * pow(10,-32) * pressure /(temperature*1.3805604 * pow(10,-23));
  double aer_beta_bs = (backscatRatio-1)*beta_m_sonde;
  
  return aer_beta_bs;
}

