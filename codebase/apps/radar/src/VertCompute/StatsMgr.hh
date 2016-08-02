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
// StatsMgr.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2008
//
///////////////////////////////////////////////////////////////

#ifndef StatsMgr_H
#define StatsMgr_H

#include <string>
#include <vector>
#include <cstdio>

#include "Params.hh"
#include "LayerStats.hh"

using namespace std;

////////////////////////
// This class

class StatsMgr {
  
public:

  // constructor

  StatsMgr(const string &prog_name,
	   const Params &params);

  // destructor
  
  ~StatsMgr();

  // set methods

  void setStartTime(double start_time) { _startTime = start_time; }
  void setEndTime(double end_time) { _endTime = end_time; }
  void setPrt(double prt) { _prt = prt; }
  void setEl(double el) { _el = el; }
  void setAz(double az);

  // add data to layer
  
  void addLayerData(double range,
		    const MomentData &mdata);

  // check and compute when ready
  
  void checkCompute();

  // compute methods
  
  void clearStats360();
  void computeStats360();
  void computeGlobalStats();
  int writeResults360ToSpdb();
  
  // print methods
  
  int writeResults360();
  void printResults360(FILE *out);

  int writeGlobalResults();
  void printGlobalResults(FILE *out);

protected:
  
private:

  string _progName;
  Params _params;

  // moments data
  
  int _nGates;

  // analysis times

  double _startTime;
  double _endTime;
  double _startTime360;
  double _prt;
  double _el;
  double _az;
  double _prevAz;
  double _azMoved;
  int _nRotations;

  // layers

  int _nLayers;
  double _startHt, _deltaHt;
  double _maxHt;
  vector<LayerStats *> _layers;

  // 360 deg values for ZDRm

  double _countZdrm;
  double _meanZdrm;
  double _sdevZdrm;
  
  // global values for ZDRm and ZDRc
  
  double _globalSumZdrm;
  double _globalSumSqZdrm;
  double _globalCountZdrm;
  double _globalMeanZdrm;
  double _globalSdevZdrm;
  double _globalMeanOfSdevZdrm;

};

#endif
