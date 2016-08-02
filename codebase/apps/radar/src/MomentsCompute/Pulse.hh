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
// Pulse.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2006
//
///////////////////////////////////////////////////////////////

#ifndef Pulse_hh
#define Pulse_hh

#include <string>
#include <vector>
#include <deque>
#include "Params.hh"
#include "Complex.hh"

using namespace std;

////////////////////////
// This class

class Pulse {
  
public:

  // constructor

  // iqc: copolar IQ data, [i,q, i,q, i,q, ...]
  //      Must be set.
  // iqx: cross-opolar IQ data, [i,q, i,q, i,q, ...]
  //      Set to NULL if no cross-pol data

  Pulse(const Params &params,
        long long seqNum,
        int nGates,
        double time,
        double prt,
        double el,
        double az,
        bool isHoriz,
        const float *iqc,
        const float *iqx = NULL);
  
  ~Pulse();
  
  // printing
  
  void print(ostream &out) const;

  // get methods

  int getNGates() const { return _nGates; }
  long long getSeqNum() const { return _seqNum; }
  double getTime() const { return _time; } // secs in decimal
  double getPrt() const { return _prt; }
  double getPrf() const { return _prf; }
  double getEl() const { return _el; }
  double getAz() const { return _az; }
  bool isHoriz() const { return _isHoriz; } // is horizontally polarized?

  const float *getIqc() const { return _iqc; }
  const float *getIqx() const { return _iqx; }

  // Memory management.
  // This class uses the notion of clients to decide when it should be deleted.
  // If removeClient() returns 0, the object should be deleted.
  
  int addClient(const string &clientName);
  int removeClient(const string &clientName);
  
protected:
private:

  const Params &_params;

  int _nClients;

  long long _seqNum;
  int _nGates;
  double _time;
  double _prt;
  double _prf;
  double _el;
  double _az;
  bool _isHoriz;
  
  // floating point IQ data
  // stored:
  //   (i,q,i,q,i,q,.... ) for iqc and iqx

  float *_iqc;
  float *_iqx;

};

#endif

