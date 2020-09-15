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
// SimScanStrategy.hh
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2020
//
///////////////////////////////////////////////////////////////
//
// SimScanStrategy creates a simulated scan strategy, and
// serves that out to a calling client.
//
////////////////////////////////////////////////////////////////

#ifndef SimScanStrategy_H
#define SimScanStrategy_H

#include <string>
#include <vector>
#include <cstdio>

#include <Radx/Radx.hh>
#include <tdrp/tdrp.h>
#include "Params.hh"

using namespace std;

////////////////////////
// This class

class SimScanStrategy {

public:

  typedef struct {
    double el;
    double az;
    int sweepNum;
    int volNum;
    Radx::SweepMode_t sweepMode;
  } angle_t;

  // constructor
  
  SimScanStrategy(const Params &params);
  
  // destructor
  
  ~SimScanStrategy();
  
  // get the next entry

  angle_t getNextAngle() const;
  
protected:
  
private:

  Params _params;

  mutable int _simVolNum;
  mutable size_t _angleIndex;
  vector<angle_t> _angles;

  void _init();
  
};

#endif
