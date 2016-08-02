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
// Beam.cc
//
// Beam object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2006
//
///////////////////////////////////////////////////////////////
//
// Beam object holds moment data.
//
////////////////////////////////////////////////////////////////

#include "Beam.hh"
#include <iostream>
#include <iomanip>
#include <cmath>
#include <toolsa/DateTime.hh>
#include <toolsa/toolsa_macros.h>
using namespace std;

////////////////////////////////////////////////////
// Constructor

Beam::Beam(const Params &params,
           double dtime,
           double az,
           double el,
           const vector<MomentData> &moments) :
        _params(params),
        _dtime(dtime),
        _az(az),
        _el(el)
  
{

  _nGates = (int) moments.size();
  _moments = moments;
  _time = (time_t) (_dtime + 0.5);

}

//////////////////////////////////////////////////////////////////
// destructor

Beam::~Beam()

{


}

// print

void Beam::print(ostream &out)

{

  out << "============ Beam ==============" << endl;
  
  out << "  nGates: " << _nGates << endl;
  out << "  el: " << _el << endl;
  out << "  az: " << _az << endl;
  int psecs = (int) ((_dtime - (int) _dtime) * 1000.0);
  char psecsStr[32];
  sprintf(psecsStr, "%.3d", psecs);
  out << "  time: " << DateTime::strm(_time) << "." << psecsStr << endl;
  
}

