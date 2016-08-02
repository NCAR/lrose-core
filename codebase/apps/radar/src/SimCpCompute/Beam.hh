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
// Beam.hh
//
// Beam object
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2006
//
///////////////////////////////////////////////////////////////

#ifndef Beam_hh
#define Beam_hh

#include <string>
#include <vector>
#include <deque>
#include <cstdio>
#include <radar/IwrfTsInfo.hh>
#include <radar/IwrfTsPulse.hh>
#include "Params.hh"
#include "MomentData.hh"
using namespace std;

////////////////////////
// This class

class Beam {
  
public:

  // constructor
  
  Beam(const Params &params,
       double dtime,
       double az,
       double el,
       const vector<MomentData> &moments);
  
  // destructor
  
  ~Beam();
  
  // print
  
  void print(ostream &out);
  
  // get methods
  
  int getNGates() const { return _nGates; }
  double getEl() const { return _el; }
  double getAz() const { return _az; }
  time_t getTime() const { return _time; }
  double getDoubleTime() const { return _dtime; }
  
  const vector<MomentData> &getMoments() const { return _moments; }
  
protected:
  
private:
  
  const Params &_params;
  
  double _dtime;
  time_t _time;

  double _az;
  double _el;

  int _nGates;
  
  vector<MomentData> _moments;
  
  // private functions
  
};

#endif

