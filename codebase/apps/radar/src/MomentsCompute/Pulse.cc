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
///////////////////////////////////////////////////////////////
// Pulse.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2003
//
////////////////////////////////////////////////////////////////

#include <iomanip>
#include <iostream>
#include <fstream>
#include <cerrno>
#include <cmath>
#include <cstring>
#include "Pulse.hh"
using namespace std;

// Constructor

Pulse::Pulse(const Params &params,
             long long seqNum,
             int nGates,
             double time,
             double prt,
             double el,
             double az,
             bool isHoriz,
             const float *iqc,
             const float *iqx /* = NULL */) :
        _params(params),
        _nClients(0),
        _seqNum(seqNum),
        _nGates(nGates),
        _time(time),
        _prt(prt),
        _el(el),
        _az(az),
        _isHoriz(isHoriz)
  
{
  _prf = 1.0 / _prt;

  _iqc = new float[2*nGates];
  memcpy(_iqc, iqc, 2*nGates * sizeof(float));

  if (iqx == NULL) {
    _iqx = NULL;
  } else {
    _iqx = new float[2*nGates];
    memcpy(_iqx, iqx, 2*nGates * sizeof(float));
  }

}

// destructor

Pulse::~Pulse()

{
  if (_iqc) {
    delete[] _iqc;
  }
  if (_iqx) {
    delete[] _iqx;
  }
}

/////////////////////////////////////
// print

void Pulse::print(ostream &out) const
{

  out << "  nGates: " << _nGates << endl;
  out << "  seqNum: " << _seqNum << endl;
  out << "  time: " << _time << endl;
  out << "  prt: " << _prt << endl;
  out << "  prf: " << _prf << endl;
  out << "  el: " << _el << endl;
  out << "  az: " << _az << endl;
  out << "  isHoriz: " << _isHoriz << endl;
  out << "  have Xpol: " << (_iqx? "true" : "false") << endl;
  
}

/////////////////////////////////////////////////////////////////////////
// Memory management.
// This class uses the notion of clients to decide when to delete itself.
// When _nClients drops from 1 to 0, it will call delete on the this pointer.

int Pulse::addClient(const string &clientName)
  
{
  _nClients++;
  if (_params.debug >= Params::DEBUG_EXTRA_VERBOSE) {
    cerr << "Pulse add client, seqNum, nClients, az, client: "
	 << _seqNum << ", " << _nClients << ", " << _az << ", "
         << clientName << endl;
  }
  return _nClients;
}

int Pulse::removeClient(const string &clientName)

{
  _nClients--;
  if (_params.debug >= Params::DEBUG_EXTRA_VERBOSE) {
    cerr << "Pulse rem client, seqNum, nClients, az, client: "
	 << _seqNum << ", " << _nClients << ", " << _az << ", "
         << clientName << endl;
  }
  return _nClients;
}

