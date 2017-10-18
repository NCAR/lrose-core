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
// FullCals.hh
//
// Holds full information on calibration of the HSRL for data processing
//
// Brad Schoenrock, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Mar 2017
//
///////////////////////////////////////////////////////////////

#ifndef FullCals_HH
#define FullCals_HH

#include <Radx/RadxTime.hh>
#include "CalReader.hh"
#include <vector>

using namespace std;

class FullCals{
  
private:  
  CalReader deadTimeHi, deadTimeLo, deadTimeCross, 
    deadTimeMol, binWidth, scanAdj;
  //holds the deadtime calibration blocks from calvals file   
  
  //holds other calibration file data
  vector< vector<double> > blCor, diffDGeoCor, geoDefCor, afPulCor;
    
  //hold which spot in the vectors are the correct calibration point. 
  int hi_pos, lo_pos, cross_pos, mol_pos, bin_pos, scan_pos;
  
  RadxTime ti;

public:   
  FullCals();
  
  FullCals(CalReader dtHi, CalReader dtLo, 
	   CalReader dtCross, CalReader dtMol, 
	   CalReader binW, CalReader scanAdjust, 
	   vector< vector<double> > blCorIn, 
	   vector< vector<double> > diffDGeoCorIn,
	   vector< vector<double> > geoDefCorIn, 
	   vector< vector<double> > afPulCorIn);
  
  void setDeadTimeHi(CalReader dtHi);
  void setDeadTimeLo(CalReader dtLo);
  void setDeadTimeCross(CalReader dtCross);
  void setDeadTimeMol(CalReader dtMol);
  void setBinWidth(CalReader binWidth); 
  void setScanAdj(CalReader scanAdjust); 

  void readDeadTimeHi(const char* file, const char* variable);
  void readDeadTimeLo(const char* file, const char* variable);
  void readDeadTimeCross(const char* file, const char* variable);
  void readDeadTimeMol(const char* file, const char* variable);
  void readBinWidth(const char* file, const char* variable); 
  void readScanAdj(const char* file, const char* variable); 

  void setBLCor(vector< vector<double> > blCor);
  void setDiffDGeoCor(vector< vector<double> > diffDGeoCor);
  void setGeoDefCor(vector< vector<double> > geoDefCor);
  void setAfPulCor(vector< vector<double> > afPulCor);

  void readBLCor(const char* file);
  void readDiffDGeoCor(const char* file);
  void readGeoDefCor(const char* file);
  void readAfPulCor(const char* file);
  
  const CalReader &getDeadTimeHi() const { return deadTimeHi; }
  const CalReader &getDeadTimeLo() const { return deadTimeLo; }
  const CalReader &getDeadTimeCross() const { return deadTimeCross; }
  const CalReader &getDeadTimeMol() const { return deadTimeMol; }
  const CalReader &getBinWidth() const { return binWidth; }
  const CalReader &getScanAdj() const { return scanAdj; } 

  int getHiPos() const { return hi_pos; }
  int getLoPos() const { return lo_pos; }
  int getCrossPos() const { return cross_pos; }
  int getMolPos() const { return mol_pos; }
  int getBinPos() const { return bin_pos; }
  
  const vector<vector<double>> &getBLCor() const { return blCor; }
  const vector<vector<double>> &getDiffDGeoCor() const { return diffDGeoCor; }
  const vector<vector<double>> &getGeoDefCor() const { return geoDefCor; }
  const vector<vector<double>> &getAfPulCor() const { return afPulCor; }
  
  vector <vector<double> > readBaselineCorrection(const char* file);
  vector <vector<double> > readDiffDefaultGeo(const char* file);
  vector <vector<double> > readGeofileDefault(const char* file);
  vector <vector<double> > readAfterPulse(const char* file);

  ~FullCals();
  
};
#endif

