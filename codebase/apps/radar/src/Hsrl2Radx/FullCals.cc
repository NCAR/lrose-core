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
// FullCals.cc
//
// Brad Schoenrock, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Mar 2017
//
///////////////////////////////////////////////////////////////

#include "FullCals.hh"

#include <fstream>
#include <sstream>
#include <string>
#include <iostream>

using namespace std;

// constructors

FullCals::FullCals()
{
  
  RadxTime time(RadxTime::NOW);
  _time = time;

}

// destructor

FullCals::~FullCals() 
{
}

// read methods

void FullCals::readDeadTimeHi(const char* file, const char* variable)
{
  _deadTimeHi=_deadTimeHi.readCalVals(file,variable);  
  _deadTimeHiPos=_deadTimeHi.dateMatch(_deadTimeHi, _time);
}

void FullCals::readDeadTimeLo(const char* file, const char* variable)
{
  _deadTimeLo=_deadTimeLo.readCalVals(file,variable);
  _deadTimeLoPos=_deadTimeLo.dateMatch(_deadTimeLo, _time);
}

void FullCals::readDeadTimeCross(const char* file, const char* variable)
{
  _deadTimeCross=_deadTimeCross.readCalVals(file,variable);
  _deadTimeCrossPos=_deadTimeCross.dateMatch(_deadTimeCross, _time);
}

void FullCals::readDeadTimeMol(const char* file, const char* variable)
{
  _deadTimeMol=_deadTimeMol.readCalVals(file,variable);
  _deadTimeMolPos=_deadTimeMol.dateMatch(_deadTimeMol, _time);
}

void FullCals::readBinWidth(const char* file, const char* variable) 
{
  _binWidth=_binWidth.readCalVals(file,variable);
  _binWidthPos=_binWidth.dateMatch(_binWidth, _time);
}

void FullCals::readScanAdj(const char* file, const char* variable) 
{
  _scanAdj=_scanAdj.readCalVals(file,variable);
  _scanAdjPos=_scanAdj.dateMatch(_scanAdj, _time);
}

void FullCals::readMolGain(const char* file, const char* variable) 
{
  _molGain=_molGain.readCalVals(file,variable);
  _molGainPos=_molGain.dateMatch(_molGain, _time);
}

// set the time
// this also updates the time-bases positions

void FullCals::setTime(time_t rtime)
{
  _time = rtime;
  _deadTimeHiPos=_deadTimeHi.dateMatch(_deadTimeHi, _time);
  _deadTimeLoPos=_deadTimeLo.dateMatch(_deadTimeLo, _time);
  _deadTimeCrossPos=_deadTimeCross.dateMatch(_deadTimeCross, _time);
  _deadTimeMolPos=_deadTimeMol.dateMatch(_deadTimeMol, _time);
  _binWidthPos=_binWidth.dateMatch(_binWidth, _time);
  _scanAdjPos=_scanAdj.dateMatch(_scanAdj, _time);
  _molGainPos=_molGain.dateMatch(_molGain, _time);
}

///////////////////////////////////////////////////////
// read baseline correction

int FullCals::readBaselineCor(const char* file)
{
 
  std::ifstream infile(file);
  if (!infile) {
    cerr << "INFO - FullCals::readBaselineCorrection" << endl;
    cerr << "  Baseline Correction file: " << file << endl;
  }   

  std::string line;
  vector<double> vec_binnum;
  vector<double> vec_combined_hi;
  vector<double> vec_combined_lo;
  vector<double> vec_molecular;
  vector<double> vec_crosspol;
  vector<double> vec_mol_I2A;
  vector<double> vec_comb_1064;

  while (std::getline(infile, line)) {
    std::stringstream ss;
    ss << line;
           
    string test = "#";
            
    if(!(line.substr(0, test.length())==test)) {
      //comments at begining of file 
      //begin with #, ignore those lines and process the rest. 
      double binnum; 
      ss>>binnum;
      vec_binnum.push_back(binnum);
	 
      double combined_hi; 
      ss>>combined_hi;
      vec_combined_hi.push_back(combined_hi);
	 
      double combined_lo; 
      ss>>combined_lo;
      vec_combined_lo.push_back(combined_lo);
	 
      double molecular; 
      ss>>molecular;
      vec_molecular.push_back(molecular);
	 
      double crosspol; 
      ss>>crosspol;
      vec_crosspol.push_back(crosspol);
	 
      double mol_I2A; 
      ss>>mol_I2A;
      vec_mol_I2A.push_back(mol_I2A);
	 
      double comb_1064; 
      ss>>comb_1064;
      vec_comb_1064.push_back(comb_1064);
	 
    }
  }
 
  _blCorCombinedHi = vec_combined_hi;
  _blCorCombinedLo = vec_combined_lo;
  _blCorMolecular = vec_molecular;
  _blCorCrossPol = vec_crosspol;
  
  return 0;

}

///////////////////////////////////////////////////////
// read diff geo correction

int FullCals::readDiffGeoCor(const char* file)
{

  std::ifstream infile(file);
  if (!infile) {
    cerr << "INFO - FullCals::readDiffDefaultGeo" << endl;
    cerr << "  DiffDefaultGeo file: " << file << endl;
  }  

  std::string line;
  vector<double> vec_altitudes;
  vector<double> vec_comb_himol;
  vector<double> vec_comb_lomol;
  vector<double> vec_scomb_himol;
  vector<double> vec_scomb_lomol;

  while (std::getline(infile, line)) {
    std::stringstream ss;
    ss << line;
     
    string test = "#";
            
    if(!(line.substr(0, test.length())==test)) {
      //comments at begining of file 
      //begin with #, ignore those lines and process the rest. 
      double altitudes; 
      ss>>altitudes;
      vec_altitudes.push_back(altitudes);
	
      double comb_himol; 
      ss>>comb_himol;
      vec_comb_himol.push_back(comb_himol);
	 
      double comb_lomol; 
      ss>>comb_lomol;
      vec_comb_lomol.push_back(comb_lomol);
		  	  
      double scomb_himol; 
      ss>>scomb_himol;
      vec_scomb_himol.push_back(scomb_himol);
	 
      double scomb_lomol; 
      ss>>scomb_lomol;
      vec_scomb_lomol.push_back(scomb_lomol);
	
    }
  }

  _diffGeoCombHiMol = vec_comb_himol;
  _diffGeoCombLoMol = vec_comb_lomol;
  _diffGeoSCombHiMol = vec_scomb_himol;
  _diffGeoSCombLoMol = vec_scomb_lomol;

  return 0;

} 

//////////////////////////////////////////////////////////////////
// read geo correction

int FullCals::readGeoCor(const char* file)
{

  std::ifstream infile(file);
  if (!infile) {
    cerr << "INFO - FullCals::readGeofileDefault" << endl;
    cerr << "  GeofileDefault file: " << file << endl;
  }  
  std::string line;
  vector<double> vec_range;
  vector<double> vec_geo_corr;

  while (std::getline(infile, line)) {
    std::stringstream ss;
    ss << line;
    
    string test = "#";
            
    if(!(line.substr(0, test.length())==test)) {
      //comments at begining of file 
      //begin with #, ignore those lines and process the rest. 
      double range; 
      ss>>range;
      vec_range.push_back(range);
	 
      double geo_corr; 
      ss>>geo_corr;
      vec_geo_corr.push_back(geo_corr);
	
    }
  }

  _geoCorr = vec_geo_corr;

  return 0;

} 

///////////////////////////////////////////////////////////////
// read afterpulse correction

int FullCals::readAfterPulseCor(const char* file)
{

  std::ifstream infile(file);
  if (!infile) {
    cerr << "INFO - FullCals::readAfterPulseCor" << endl;
    cerr << "  readAfterPulseCor file: " << file << endl;
  }  

  std::string line;
  vector<double> vec_bin;
  vector<double> vec_mol;
  vector<double> vec_comb;
  vector<double> vec_crossPol;
  vector<double> vec_refftMol;
  vector<double> vec_imfftMol;
  vector<double> vec_refftComb;
  vector<double> vec_imfftComb;
  vector<double> vec_refftCPol;
  vector<double> vec_imfftCPol;

  while (std::getline(infile, line)) {
    std::stringstream ss;
    ss << line;
    
    string test = "#";
            
    if(!(line.substr(0, test.length())==test)) {
      //comments at begining of file 
      //begin with #, ignore those lines and process the rest. 
      double bin; 
      ss>>bin;
      vec_bin.push_back(bin);
	
      double mol; 
      ss>>mol;
      vec_mol.push_back(mol);
	 
      double comb; 
      ss>>comb;
      vec_comb.push_back(comb);
	 
      double crossPol; 
      ss>>crossPol;
      vec_crossPol.push_back(crossPol);
	
      double refftMol; 
      ss>>refftMol;
      vec_refftMol.push_back(refftMol);
	
      double imfftMol; 
      ss>>imfftMol;
      vec_imfftMol.push_back(imfftMol);
	 
      double refftComb; 
      ss>>refftComb;
      vec_refftComb.push_back(refftComb);
	 
      double imfftComb; 
      ss>>imfftComb;
      vec_imfftComb.push_back(imfftComb);
	 
      double refftCPol; 
      ss>>refftCPol;
      vec_refftCPol.push_back(refftCPol);
	
      double imfftCPol; 
      ss>>imfftCPol;
      vec_imfftCPol.push_back(imfftCPol);
	 
    }
  }

  _afterPulseMol = vec_mol;
  _afterPulseComb = vec_comb;
  _afterPulseCross = vec_crossPol;

  return 0;

} 



  







