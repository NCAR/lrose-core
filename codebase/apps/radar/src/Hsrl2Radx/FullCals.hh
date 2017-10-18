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

  // hold the deadtime calibration blocks from calvals file   

  CalReader _deadTimeHi, _deadTimeLo, _deadTimeCross;
  CalReader _deadTimeMol, _binWidth, _scanAdj;

  // hold other calibration file data

  vector< vector<double> > _blCor, _diffDGeoCor, _geoDefCor, _afPulCor;
    
  // hold which spot in the vectors are the correct calibration point. 

  int _hiPos, _loPos, _crossPos, _molPos, _binPos, _scanPos;
  
  RadxTime _time;

public:   

  // constructors

  FullCals();
  
  FullCals(CalReader dtHi, CalReader dtLo, 
	   CalReader dtCross, CalReader dtMol, 
	   CalReader binW, CalReader scanAdjust, 
	   vector< vector<double> > blCorIn, 
	   vector< vector<double> > diffDGeoCorIn,
	   vector< vector<double> > geoDefCorIn, 
	   vector< vector<double> > afPulCorIn);

  // destructor

  ~FullCals();
  
  // read in cals

  void readDeadTimeHi(const char* file, const char* variable);
  void readDeadTimeLo(const char* file, const char* variable);
  void readDeadTimeCross(const char* file, const char* variable);
  void readDeadTimeMol(const char* file, const char* variable);
  void readBinWidth(const char* file, const char* variable); 
  void readScanAdj(const char* file, const char* variable); 

  void readBLCor(const char* file);
  void readDiffDGeoCor(const char* file);
  void readGeoDefCor(const char* file);
  void readAfPulCor(const char* file);
  
  vector <vector<double> > readBaselineCorrection(const char* file);
  vector <vector<double> > readDiffDefaultGeo(const char* file);
  vector <vector<double> > readGeofileDefault(const char* file);
  vector <vector<double> > readAfterPulse(const char* file);

  // get methods

  const CalReader &getDeadTimeHi() const { return _deadTimeHi; }
  const CalReader &getDeadTimeLo() const { return _deadTimeLo; }
  const CalReader &getDeadTimeCross() const { return _deadTimeCross; }
  const CalReader &getDeadTimeMol() const { return _deadTimeMol; }
  const CalReader &getBinWidth() const { return _binWidth; }
  const CalReader &getScanAdj() const { return _scanAdj; } 

  int getHiPos() const { return _hiPos; }
  int getLoPos() const { return _loPos; }
  int getCrossPos() const { return _crossPos; }
  int getMolPos() const { return _molPos; }
  int getBinPos() const { return _binPos; }
  
  const vector<vector<double>> &getBLCor() const { return _blCor; }
  const vector<vector<double>> &getDiffDGeoCor() const { return _diffDGeoCor; }
  const vector<vector<double>> &getGeoDefCor() const { return _geoDefCor; }
  const vector<vector<double>> &getAfPulCor() const { return _afPulCor; }
  
};
#endif

