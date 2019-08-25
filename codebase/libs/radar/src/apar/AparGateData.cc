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
// AparGateData.cc
//
// Container for memory for IQ gate data.
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Aug 2019
//
////////////////////////////////////////////////////////////////
//
// Field data and IQ data for a single gate.
//
////////////////////////////////////////////////////////////////

#include <iostream>
#include <radar/AparGateData.hh>
using namespace std;

// Default constructor.
// Initializes all arrays to NULL.
// Call allocArrays() before using.

AparGateData::AparGateData() :
        flds(fields)

{
  
  _nSamples = 0;
  _nSamplesHalf = 0;
  _nSamplesAlloc = 0;
  _initArraysToNull();

}

// destructor

AparGateData::~AparGateData()

{
  _freeArrays();
}

// clear the field data

void AparGateData::initFields()

{
  fields.init();
}

//////////////////////////////////////
// init arrays to NULL

void AparGateData::_initArraysToNull()

{

  iq0 = NULL;
  iq1 = NULL;

  iqhc = NULL;
  iqvc = NULL;
  iqhx = NULL;
  iqvx = NULL;

  iqhcPrtShort = NULL;
  iqhcPrtLong = NULL;
  iqvcPrtShort = NULL;
  iqvcPrtLong = NULL;

  iqhxPrtShort = NULL;
  iqhxPrtLong = NULL;
  iqvxPrtShort = NULL;
  iqvxPrtLong = NULL;

}

/////////////////////////////////////////////
// Allocate all arrays.
// On object allocated in this way can be used for any
// computation in any mode.

void AparGateData::allocArrays(int nSamples, 
                               bool isStagPrt)

{

  // do not reallocate if nsamples is between (0.8 * alloc) and (1.0 * alloc)
  // and other requirements are unchanged

  if (nSamples <= _nSamplesAlloc &&
      nSamples >= (int) ((double) _nSamplesAlloc * 0.8) &&
      isStagPrt == _isStagPrt) {
    return;
  }

  // save requirements

  _nSamples = nSamples;
  _nSamplesHalf = _nSamples / 2;
  _nSamplesAlloc = nSamples;
  _isStagPrt = isStagPrt;

  // perform allocation

  _allocArray(iq0, _iq0, _nSamples);
  _allocArray(iq1, _iq1, _nSamples);

  _allocArray(iqhc, _iqhc, _nSamples);
  _allocArray(iqvc, _iqvc, _nSamples);
  _allocArray(iqhx, _iqhx, _nSamples);
  _allocArray(iqvx, _iqvx, _nSamples);
  
  if (_isStagPrt) {

    _allocArray(iqhcPrtShort, _iqhcPrtShort, _nSamplesHalf);
    _allocArray(iqhcPrtLong, _iqhcPrtLong, _nSamplesHalf);
    _allocArray(iqvcPrtShort, _iqvcPrtShort, _nSamplesHalf);
    _allocArray(iqvcPrtLong, _iqvcPrtLong, _nSamplesHalf);
    
    _allocArray(iqhxPrtShort, _iqhxPrtShort, _nSamplesHalf);
    _allocArray(iqhxPrtLong, _iqhxPrtLong, _nSamplesHalf);
    _allocArray(iqvxPrtShort, _iqvxPrtShort, _nSamplesHalf);
    _allocArray(iqvxPrtLong, _iqvxPrtLong, _nSamplesHalf);
    
  } else {

    _freeArray(iqhcPrtShort, _iqhcPrtShort);
    _freeArray(iqhcPrtLong, _iqhcPrtLong);
    _freeArray(iqvcPrtShort, _iqvcPrtShort);
    _freeArray(iqvcPrtLong, _iqvcPrtLong);
    
    _freeArray(iqhxPrtShort, _iqhxPrtShort);
    _freeArray(iqhxPrtLong, _iqhxPrtLong);
    _freeArray(iqvxPrtShort, _iqvxPrtShort);
    _freeArray(iqvxPrtLong, _iqvxPrtLong);
    
  }
  
}

///////////////////////////////////////////////////
// free up all field arrays

void AparGateData::_freeArrays()

{

  _freeArray(iq0, _iq0);
  _freeArray(iq1, _iq1);

  _freeArray(iqhc, _iqhc);
  _freeArray(iqhx, _iqhx);
  _freeArray(iqvc, _iqvc);
  _freeArray(iqvx, _iqvx);

  _freeArray(iqhcPrtShort, _iqhcPrtShort);
  _freeArray(iqhcPrtLong, _iqhcPrtLong);
  _freeArray(iqvcPrtShort, _iqvcPrtShort);
  _freeArray(iqvcPrtLong, _iqvcPrtLong);

  _freeArray(iqhxPrtShort, _iqhxPrtShort);
  _freeArray(iqhxPrtLong, _iqhxPrtLong);
  _freeArray(iqvxPrtShort, _iqvxPrtShort);
  _freeArray(iqvxPrtLong, _iqvxPrtLong);

}
