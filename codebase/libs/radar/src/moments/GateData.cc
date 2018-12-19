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
// GateData.cc
//
// Container for memory for IQ gate data.
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2007
//
////////////////////////////////////////////////////////////////
//
// Field data and IQ data for a single gate.
//
////////////////////////////////////////////////////////////////

#include <iostream>
#include <radar/GateData.hh>
using namespace std;

// Default constructor.
// Initializes all arrays to NULL.
// Call allocArrays() before using.

GateData::GateData() :
        flds(fields),
        fldsF(fieldsF)

{
  
  _nSamples = 0;
  _nSamplesHalf = 0;
  _nSamplesAlloc = 0;
  _initArraysToNull();

  censorStrong = false;
  censorWeak = false;
  trip1IsStrong = false;
  clutterInStrong = false;
  clutterInWeak = false;
  szLeakage = 0;

}

// destructor

GateData::~GateData()

{
  _freeArrays();
}

// clear the field data

void GateData::initFields()

{
  fields.init();
  fieldsF.init();
  specHcComputed = false;
  specVcComputed = false;
}

// set field references

void GateData::setFieldsToNormalTrip()
  
{
  flds = fields;
  fldsF = fieldsF;
}

void GateData::setFieldsToSecondTrip()

{
  flds = secondTrip;
  fldsF = secondTripF;
}

//////////////////////////////////////
// init arrays to NULL

void GateData::_initArraysToNull()

{

  iqhcOrig = NULL;
  iqvcOrig = NULL;
  iqhxOrig = NULL;
  iqvxOrig = NULL;

  iqhc = NULL;
  iqvc = NULL;
  iqhx = NULL;
  iqvx = NULL;

  specHc = NULL;
  specVc = NULL;

  iqhcF = NULL;
  iqvcF = NULL;
  iqhxF = NULL;
  iqvxF = NULL;

  iqhcNotched = NULL;
  iqvcNotched = NULL;

  iqhcPrtShortOrig = NULL;
  iqhcPrtLongOrig = NULL;
  iqvcPrtShortOrig = NULL;
  iqvcPrtLongOrig = NULL;

  iqhxPrtShortOrig = NULL;
  iqhxPrtLongOrig = NULL;
  iqvxPrtShortOrig = NULL;
  iqvxPrtLongOrig = NULL;

  iqhcPrtShort = NULL;
  iqhcPrtLong = NULL;
  iqvcPrtShort = NULL;
  iqvcPrtLong = NULL;

  iqhxPrtShort = NULL;
  iqhxPrtLong = NULL;
  iqvxPrtShort = NULL;
  iqvxPrtLong = NULL;

  iqhcPrtShortF = NULL;
  iqhcPrtLongF = NULL;
  iqvcPrtShortF = NULL;
  iqvcPrtLongF = NULL;

  iqhxPrtShortF = NULL;
  iqhxPrtLongF = NULL;
  iqvxPrtShortF = NULL;
  iqvxPrtLongF = NULL;

  iqStrong = NULL;
  iqWeak = NULL;
  iqStrongF = NULL;
  iqWeakF = NULL;

  iqMeas = NULL;
  iqTrip1 = NULL;
  iqTrip2 = NULL;
  iqTrip3 = NULL;
  iqTrip4 = NULL;

}

/////////////////////////////////////////////
// Allocate all arrays.
// On object allocated in this way can be used for any
// computation in any mode.

void GateData::allocArrays(int nSamples, 
                           bool needFiltering, 
                           bool isStagPrt,
                           bool isSz)

{

  // do not reallocate if nsamples is between (0.8 * alloc) and (1.0 * alloc)
  // and other requirements are unchanged

  if (nSamples <= _nSamplesAlloc &&
      nSamples >= (int) ((double) _nSamplesAlloc * 0.8) &&
      needFiltering == _needFiltering &&
      isStagPrt == _isStagPrt &&
      isSz == _isSz) {
    return;
  }

  // save requirements

  _nSamples = nSamples;
  _nSamplesHalf = _nSamples / 2;
  _nSamplesAlloc = nSamples;
  _needFiltering = needFiltering;
  _isStagPrt = isStagPrt;
  _isSz = isSz;

  // perform allocation

  _allocArray(iqhcOrig, _nSamples);
  _allocArray(iqvcOrig, _nSamples);
  _allocArray(iqhxOrig, _nSamples);
  _allocArray(iqvxOrig, _nSamples);
  
  _allocArray(iqhc, _nSamples);
  _allocArray(iqvc, _nSamples);
  _allocArray(iqhx, _nSamples);
  _allocArray(iqvx, _nSamples);
  
  _allocArray(specHc, _nSamples);
  _allocArray(specVc, _nSamples);

  if (_needFiltering) {

    _allocArray(iqhcF, _nSamples);
    _allocArray(iqvcF, _nSamples);
    _allocArray(iqhxF, _nSamples);
    _allocArray(iqvxF, _nSamples);

    _allocArray(iqhcNotched, _nSamples);
    _allocArray(iqvcNotched, _nSamples);

  } else {

    _freeArray(iqhcF);
    _freeArray(iqvcF);
    _freeArray(iqhxF);
    _freeArray(iqvxF);

    _freeArray(iqhcNotched);
    _freeArray(iqvcNotched);

  }

  if (_isStagPrt) {

    _allocArray(iqhcPrtShortOrig, _nSamplesHalf);
    _allocArray(iqhcPrtLongOrig, _nSamplesHalf);
    _allocArray(iqvcPrtShortOrig, _nSamplesHalf);
    _allocArray(iqvcPrtLongOrig, _nSamplesHalf);
    
    _allocArray(iqhxPrtShortOrig, _nSamplesHalf);
    _allocArray(iqhxPrtLongOrig, _nSamplesHalf);
    _allocArray(iqvxPrtShortOrig, _nSamplesHalf);
    _allocArray(iqvxPrtLongOrig, _nSamplesHalf);
    
    _allocArray(iqhcPrtShort, _nSamplesHalf);
    _allocArray(iqhcPrtLong, _nSamplesHalf);
    _allocArray(iqvcPrtShort, _nSamplesHalf);
    _allocArray(iqvcPrtLong, _nSamplesHalf);
    
    _allocArray(iqhxPrtShort, _nSamplesHalf);
    _allocArray(iqhxPrtLong, _nSamplesHalf);
    _allocArray(iqvxPrtShort, _nSamplesHalf);
    _allocArray(iqvxPrtLong, _nSamplesHalf);
    
  } else {

    _freeArray(iqhcPrtShortOrig);
    _freeArray(iqhcPrtLongOrig);
    _freeArray(iqvcPrtShortOrig);
    _freeArray(iqvcPrtLongOrig);
    
    _freeArray(iqhxPrtShortOrig);
    _freeArray(iqhxPrtLongOrig);
    _freeArray(iqvxPrtShortOrig);
    _freeArray(iqvxPrtLongOrig);
    
    _freeArray(iqhcPrtShort);
    _freeArray(iqhcPrtLong);
    _freeArray(iqvcPrtShort);
    _freeArray(iqvcPrtLong);
    
    _freeArray(iqhxPrtShort);
    _freeArray(iqhxPrtLong);
    _freeArray(iqvxPrtShort);
    _freeArray(iqvxPrtLong);
    
  }
  
  if (_isStagPrt && _needFiltering) {
    
    _allocArray(iqhcPrtShortF, _nSamplesHalf);
    _allocArray(iqhcPrtLongF, _nSamplesHalf);
    _allocArray(iqvcPrtShortF, _nSamplesHalf);
    _allocArray(iqvcPrtLongF, _nSamplesHalf);
    
    _allocArray(iqhxPrtShortF, _nSamplesHalf);
    _allocArray(iqhxPrtLongF, _nSamplesHalf);
    _allocArray(iqvxPrtShortF, _nSamplesHalf);
    _allocArray(iqvxPrtLongF, _nSamplesHalf);
    
  } else {
    
    _freeArray(iqhcPrtShortF);
    _freeArray(iqhcPrtLongF);
    _freeArray(iqvcPrtShortF);
    _freeArray(iqvcPrtLongF);
    
    _freeArray(iqhxPrtShortF);
    _freeArray(iqhxPrtLongF);
    _freeArray(iqvxPrtShortF);
    _freeArray(iqvxPrtLongF);
    
  }

  if (_isSz) {

    _allocArray(iqStrong, _nSamples);
    _allocArray(iqWeak, _nSamples);
    _allocArray(iqStrongF, _nSamples);
    _allocArray(iqWeakF, _nSamples);
    
    _allocArray(iqMeas, _nSamples);
    _allocArray(iqTrip1, _nSamples);
    _allocArray(iqTrip2, _nSamples);
    _allocArray(iqTrip3, _nSamples);
    _allocArray(iqTrip4, _nSamples);

  } else {

    _freeArray(iqStrong);
    _freeArray(iqWeak);
    _freeArray(iqStrongF);
    _freeArray(iqWeakF);
    
    _freeArray(iqMeas);
    _freeArray(iqTrip1);
    _freeArray(iqTrip2);
    _freeArray(iqTrip3);
    _freeArray(iqTrip4);

  }
    
}

///////////////////////////////////////////////////
// free up all field arrays

void GateData::_freeArrays()

{

  _freeArray(iqhcOrig);
  _freeArray(iqhxOrig);
  _freeArray(iqvcOrig);
  _freeArray(iqvxOrig);

  _freeArray(iqhc);
  _freeArray(iqhx);
  _freeArray(iqvc);
  _freeArray(iqvx);

  _freeArray(specHc);
  _freeArray(specVc);

  _freeArray(iqhcF);
  _freeArray(iqhxF);
  _freeArray(iqvcF);
  _freeArray(iqvxF);

  _freeArray(iqhcNotched);
  _freeArray(iqvcNotched);

  _freeArray(iqhcPrtShortOrig);
  _freeArray(iqhcPrtLongOrig);
  _freeArray(iqvcPrtShortOrig);
  _freeArray(iqvcPrtLongOrig);

  _freeArray(iqhxPrtShortOrig);
  _freeArray(iqhxPrtLongOrig);
  _freeArray(iqvxPrtShortOrig);
  _freeArray(iqvxPrtLongOrig);

  _freeArray(iqhcPrtShort);
  _freeArray(iqhcPrtLong);
  _freeArray(iqvcPrtShort);
  _freeArray(iqvcPrtLong);

  _freeArray(iqhxPrtShort);
  _freeArray(iqhxPrtLong);
  _freeArray(iqvxPrtShort);
  _freeArray(iqvxPrtLong);

  _freeArray(iqhcPrtShortF);
  _freeArray(iqhcPrtLongF);
  _freeArray(iqvcPrtShortF);
  _freeArray(iqvcPrtLongF);

  _freeArray(iqhxPrtShortF);
  _freeArray(iqhxPrtLongF);
  _freeArray(iqvxPrtShortF);
  _freeArray(iqvxPrtLongF);

  _freeArray(iqStrong);
  _freeArray(iqWeak);
  _freeArray(iqStrongF);
  _freeArray(iqWeakF);

  _freeArray(iqMeas);
  _freeArray(iqTrip1);
  _freeArray(iqTrip2);
  _freeArray(iqTrip3);
  _freeArray(iqTrip4);

}

