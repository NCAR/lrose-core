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

  CalReader _deadTimeHi, _deadTimeLo, _deadTimeCross, _deadTimeMol;
  CalReader _binWidth, _scanAdj, _molGain;

  // calibration corrections
  
  vector<double> _blCorCombinedHi;
  vector<double> _blCorCombinedLo;
  vector<double> _blCorMolecular;
  vector<double> _blCorCrossPol;
    
  vector<double> _diffGeoCombHiMol;
  vector<double> _diffGeoCombLoMol;
  vector<double> _diffGeoSCombHiMol;
  vector<double> _diffGeoSCombLoMol;

  vector<double> _geoCorr;

  vector<double> _afterPulseMol;
  vector<double> _afterPulseComb;
  vector<double> _afterPulseCross;

  // hold which position in the vectors for the data time

  int _deadTimeHiPos, _deadTimeLoPos, _deadTimeCrossPos, _deadTimeMolPos;
  int _binWidthPos, _scanAdjPos, _molGainPos;
  RadxTime _time;
  
public:   

  // constructor

  FullCals();
  
  // destructor

  ~FullCals();
  
  // read in cals

  void readDeadTimeHi(const char* file, const char* variable);
  void readDeadTimeLo(const char* file, const char* variable);
  void readDeadTimeCross(const char* file, const char* variable);
  void readDeadTimeMol(const char* file, const char* variable);
  void readBinWidth(const char* file, const char* variable); 
  void readScanAdj(const char* file, const char* variable); 
  void readMolGain(const char* file, const char* variable); 

  int readBaselineCor(const char* file);
  int readDiffGeoCor(const char* file);
  int readGeoCor(const char* file);
  int readAfterPulseCor(const char* file);
  
  // set the time
  // this also updates the time-bases positions

  void setTime(time_t rtime);

  // get methods

  const CalReader &getDeadTimeHi() const { return _deadTimeHi; }
  const CalReader &getDeadTimeLo() const { return _deadTimeLo; }
  const CalReader &getDeadTimeCross() const { return _deadTimeCross; }
  const CalReader &getDeadTimeMol() const { return _deadTimeMol; }
  const CalReader &getBinWidth() const { return _binWidth; }
  const CalReader &getScanAdj() const { return _scanAdj; } 
  const CalReader &getMolGain() const { return _molGain; } 

  int getHiPos() const { return _deadTimeHiPos; }
  int getLoPos() const { return _deadTimeLoPos; }
  int getCrossPos() const { return _deadTimeCrossPos; }
  int getMolPos() const { return _deadTimeMolPos; }
  int getBinPos() const { return _binWidthPos; }
  
  const vector<double> &getBlCorCombinedHi() const { return _blCorCombinedHi; }
  const vector<double> &getBlCorCombinedLo() const { return _blCorCombinedLo; }
  const vector<double> &getBlCorMolecular() const { return _blCorMolecular; }
  const vector<double> &getBlCorCrossPol() const { return _blCorCrossPol; }

  const vector<double> &getDiffGeoCombHiMol() const { return _diffGeoCombHiMol; }
  const vector<double> &getDiffGeoCombLoMol() const { return _diffGeoCombLoMol; }
  const vector<double> &getDiffGeoSCombHiMol() const { return _diffGeoSCombHiMol; }
  const vector<double> &getDiffGeoSCombLoMol() const { return _diffGeoSCombLoMol; }

  const vector<double> &getGeoCorr() const { return _geoCorr; }

  const vector<double> &getAfterPulseMol() const { return _afterPulseMol; }
  const vector<double> &getAfterPulseComb() const { return _afterPulseComb; }
  const vector<double> &getAfterPulseCross() const { return _afterPulseCross; }


};
#endif

