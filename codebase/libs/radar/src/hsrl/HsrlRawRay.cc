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
// HsrlRawRay
//
// Holds raw data from a single HSRL ray or beam
//
// Mike Dixon, Brad Schoenrock, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2017
//
///////////////////////////////////////////////////////////////

#include <radar/HsrlRawRay.hh>
#include <cstring>
using namespace std;

///////////////////////////////////////////////////////////////
// default constructor

HsrlRawRay::HsrlRawRay()

{

}
  
///////////////////////////////////////////////////////////////
// destructor
  
HsrlRawRay::~HsrlRawRay()

{

  _combinedHi.clear();
  _combinedLo.clear();
  _molecular.clear();
  _cross.clear();

}

///////////////////////////////////////////////////////////////
// set the fields

void HsrlRawRay::setFields(int nGates,
                           const float32 *combinedHi,
                           const float32 *combinedLo,
                           const float32 *molecular,
                           const float32 *cross)

{

  _nGates = nGates;

  _combinedHi.resize(_nGates);
  memcpy(&_combinedHi[0], combinedHi, _nGates * sizeof(float32));

  _combinedLo.resize(_nGates);
  memcpy(&_combinedLo[0], combinedLo, _nGates * sizeof(float32));

  _molecular.resize(_nGates);
  memcpy(&_molecular[0], molecular, _nGates * sizeof(float32));

  _cross.resize(_nGates);
  memcpy(&_cross[0], cross, _nGates * sizeof(float32));

}


