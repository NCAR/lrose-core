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
// SweepMgr.hh
//
// Manager for sweep scan strategy
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2011
//
///////////////////////////////////////////////////////////////

#ifndef SweepMgr_H
#define SweepMgr_H

#include <string>
#include <vector>
#include <algorithm>
#include <functional>

#include "Args.hh"
#include "Params.hh"
#include <rapformats/DsRadarMsg.hh>
#include <Fmq/DsRadarQueue.hh>
#include <Radx/RadxRay.hh>

using namespace std;

typedef struct {
  double sumAngle;
  double meanAngle;
  int nBeams;
  bool inUse;
} sweep_t;

typedef struct {
  double angle;
  int nBeams;
} sweep_peak_t;

////////////////////////
// This class

class SweepMgr {
  
public:

  // constructor
  
  SweepMgr (const string &prog_name,
            const Params &params);
  
  // destructor
  
  ~SweepMgr();

  // set the sweep numbers in the input beams
  
  void setSweepNumbers(bool isRhi,
                       const vector<RadxRay *> &rays);

protected:
private:

  // members
  
  string _progName;
  const Params &_params;

  // histogram

  bool _isRhi;
  int _nHist;
  int _histOffset;
  double _histIntv;
  int _histSearchWidth;
  int *_hist;
  vector<double> _angleTable;     // angles in use
  vector<int> _sweepTable;        // sweep numbers in use

  // lookup table for sweep indices
  // converts a sweepNum into an index
  // in the _angleTable
  
  vector<int> _sweepIndexLookup;

  // functions
  
  void _loadAngleTableFromHist(const vector<RadxRay *> rays);
  
  int _computeHist(const vector<RadxRay *> rays);
  void _printAngles();

  void _freeHist();
  void _clearHist();
  
};

#endif

