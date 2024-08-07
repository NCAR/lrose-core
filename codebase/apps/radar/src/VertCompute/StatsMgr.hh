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

#include "Args.hh"
#include "Params.hh"
#include "LayerStats.hh"

using namespace std;

////////////////////////
// This class

class StatsMgr {
  
public:

  // constructor

  StatsMgr(const string &prog_name,
           const Args &args,
	   const Params &params);

  // destructor
  
  virtual ~StatsMgr();

  // run 

  virtual int run() = 0;

  // set methods

  void setStartTime(double start_time);
  void setEndTime(double latest_time);
  void setPrt(double prt) { _prt = prt; }
  void setEl(double el);
  void setAz(double az);

  // add data for a point
  
  void addDataPoint(double range,
		    MomentData mdata);

  // check and compute when ready
  
  void checkCompute();

  // compute methods
  
  void clearStats();
  int computeStats();
  int computeGlobalStats();
  
  // write methods
  
  int writeStats();
  void printStats(FILE *out);

  int writeGlobalStats();
  void printGlobalStats(FILE *out);

  int writeZdrPoints();

  int writeStatsToSpdb();

protected:
  
  string _progName;
  const Args &_args;
  const Params &_params;

  // moments data
  
  int _nGates;

  // analysis

  double _startTimeGlobal;
  double _endTimeGlobal;
  double _startTimeStats;
  double _endTimeStats;
  double _prevTime;
  double _prt;
  double _el;
  double _az;
  double _prevAz;
  double _azMovedGlobal;
  double _azMovedStats;
  double _azMovedPrint;

  // elevation

  double _sumEl;
  double _nEl;
  double _meanEl;
  double _globalSumEl;
  double _globalNEl;
  double _globalMeanEl;

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

private:

};

#endif
