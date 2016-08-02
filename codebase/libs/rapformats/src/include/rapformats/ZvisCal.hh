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
// ZvisCal.hh
//
// C++ class for dealing with z-v probability calibration
//
// Mike Dixon, RAP, NCAR
// POBox 3000, Boulder, CO, USA
//
// July 2006
//////////////////////////////////////////////////////////////

#ifndef _ZvisCal_hh
#define _ZvisCal_hh

#include <cstdio>
#include <iostream>
#include <string>
#include <vector>
#include <toolsa/MemBuf.hh>

using namespace std;

class ZvisCal {

public:

  typedef struct {
    double quantile;
    double dbz;
    double viskm;
  } cal_entry_t;

  // constructor

  ZvisCal();

  // destructor

  ~ZvisCal();

  //////////////////////// set methods /////////////////////////

  // clear all data members

  void clear();

  //////////////////////// set methods /////////////////////////

  void setStartTime(time_t val) {
    _startTime = val;
  }
  
  void setEndTime(time_t val) {
    _endTime = val;
  }

  void setCalibrationPeriod(int val) {
    _calibrationPeriod = val;
  }

  void setSurfaceWindAveragingPeriod(int val) {
    _surfaceWindAveragingPeriod = val;
  }

  void setTrecWindAveragingPeriod(int val) {
    _trecWindAveragingPeriod = val;
  }

  void setTrecWindKernelRadius(double val) {
    _trecWindKernelRadius = val;
  }

  void setMinFractionTrecWindKernel(double val) {
    _minFractionTrecWindKernel = val;
  }

  void setTrecWindWeight(double val) {
    _trecWindWeight = val;
  }

  void setFallTime(int val) {
    _fallTime = val;
  }

  void setMinQuantile(double val) {
    _minQuantile = val;
  }

  void setMaxQuantile(double val) {
    _maxQuantile = val;
  }

  void setDeltaQuantile(double val) {
    _deltaQuantile = val;
  }

  void addEntry(double quantile, double dbz, double ee);

  // perform fit to cal data
  // returns 0 on success, -1 on failure

  int computeFit();

  //////////////////////// get methods /////////////////////////

  int getVersionNum() const { return _versionNum; }
  time_t getStartTime() const { return  _startTime; }
  time_t getEndTime() const { return  _endTime; }
  int getCalibrationPeriod() const { return  _calibrationPeriod; }
  int getSurfaceWindAveragingPeriod() const
    { return  _surfaceWindAveragingPeriod; }
  int getTrecWindAveragingPeriod() const
    { return  _trecWindAveragingPeriod; }
  double getTrecWindKernelRadius() const
    { return  _trecWindKernelRadius; }
  double getMinFractionTrecWindKernel() const
    { return  _minFractionTrecWindKernel; }
  double getTrecWindWeight() const { return  _trecWindWeight; }
  int getFallTime() const { return  _fallTime; }
  double getMinQuantile() const { return  _minQuantile; }
  double getMaxQuantile() const { return  _maxQuantile; }
  double getDeltaQuantile() const { return  _deltaQuantile; }
  double getCalIntercept() const { return  _calIntercept; }
  double getCalSlope() const { return  _calSlope; }
  double getCalCorr() const { return  _calCorr; }

  // get the full calibration array

  const vector<cal_entry_t> &getCal() const { return _cal; }

  // get an estimated vis value given a dBZ value
  
  double getVisKm(double dbz) const;
  
  ///////////////////////////////////////////
  // assemble()
  // Load up the buffer from the object.
  // Handles byte swapping.
  
  void assemble();
  
  // get the assembled buffer info
  
  const void *getBufPtr() const { return _memBuf.getPtr(); }
  int getBufLen() const { return _memBuf.getLen(); }
  
  ///////////////////////////////////////////////////////////
  // disassemble()
  // Disassembles a buffer, sets the values in the object.
  // Handles byte swapping.
  // Returns 0 on success, -1 on failure
  
  int disassemble(const void *buf, int len);
  
  /////////////////////////
  // print
  
  void print(ostream &out, string spacer = "") const;
  
  // definition of struct for storing sweep

  static const int MISSING_VAL = -9999;

protected:

  int _versionNum;
  time_t _startTime;
  time_t _endTime;
  int _calibrationPeriod;
  int _surfaceWindAveragingPeriod;
  int _trecWindAveragingPeriod;
  double _trecWindKernelRadius;
  double _minFractionTrecWindKernel;
  double _trecWindWeight;
  int _fallTime;
  double _minQuantile;
  double _maxQuantile;
  double _deltaQuantile;

  vector<cal_entry_t> _cal;

  // slope, intercept and correlation for linear fit cal equation:
  //   log10(Vkm) = intercept + slope * dBZ
  
  double _calIntercept;
  double _calSlope;
  double _calCorr;

  // buffer for assemble / disassemble

  MemBuf _memBuf;

private:

};


#endif

