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

// set the field data to missing

void GateData::initFields()

{
  fields.init();
  fieldsF.init();
  specHcComputed = false;
  specVcComputed = false;
}

// set the field data to zero

void GateData::initFieldsToZero()

{
  fields.initToZero();
  fieldsF.initToZero();
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
  iqhxNotched = NULL;
  iqvxNotched = NULL;

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

  iqhcPrtShortNotched = NULL;
  iqvcPrtShortNotched = NULL;
  iqhxPrtShortNotched = NULL;
  iqvxPrtShortNotched = NULL;

  iqhcPrtLongNotched = NULL;
  iqvcPrtLongNotched = NULL;
  iqhxPrtLongNotched = NULL;
  iqvxPrtLongNotched = NULL;

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

  _allocArray(iqhcOrig, _iqhcOrig, _nSamples);
  _allocArray(iqvcOrig, _iqvcOrig, _nSamples);
  _allocArray(iqhxOrig, _iqhxOrig, _nSamples);
  _allocArray(iqvxOrig, _iqvxOrig, _nSamples);
  
  _allocArray(iqhc, _iqhc, _nSamples);
  _allocArray(iqvc, _iqvc, _nSamples);
  _allocArray(iqhx, _iqhx, _nSamples);
  _allocArray(iqvx, _iqvx, _nSamples);
  
  _allocArray(specHc, _specHc, _nSamples);
  _allocArray(specVc, _specVc, _nSamples);

  if (_needFiltering) {

    _allocArray(iqhcF, _iqhcF, _nSamples);
    _allocArray(iqvcF, _iqvcF, _nSamples);
    _allocArray(iqhxF, _iqhxF, _nSamples);
    _allocArray(iqvxF, _iqvxF, _nSamples);

    _allocArray(iqhcNotched, _iqhcNotched, _nSamples);
    _allocArray(iqvcNotched, _iqvcNotched, _nSamples);
    _allocArray(iqhxNotched, _iqhxNotched, _nSamples);
    _allocArray(iqvxNotched, _iqvxNotched, _nSamples);

  } else {

    _freeArray(iqhcF, _iqhcF);
    _freeArray(iqvcF, _iqvcF);
    _freeArray(iqhxF, _iqhxF);
    _freeArray(iqvxF, _iqvxF);
    
    _freeArray(iqhcNotched, _iqhcNotched);
    _freeArray(iqvcNotched, _iqvcNotched);
    _freeArray(iqhxNotched, _iqhxNotched);
    _freeArray(iqvxNotched, _iqvxNotched);

  }

  if (_isStagPrt) {

    _allocArray(iqhcPrtShortOrig, _iqhcPrtShortOrig, _nSamplesHalf);
    _allocArray(iqhcPrtLongOrig, _iqhcPrtLongOrig, _nSamplesHalf);
    _allocArray(iqvcPrtShortOrig, _iqvcPrtShortOrig, _nSamplesHalf);
    _allocArray(iqvcPrtLongOrig, _iqvcPrtLongOrig, _nSamplesHalf);
    
    _allocArray(iqhxPrtShortOrig, _iqhxPrtShortOrig, _nSamplesHalf);
    _allocArray(iqhxPrtLongOrig, _iqhxPrtLongOrig, _nSamplesHalf);
    _allocArray(iqvxPrtShortOrig, _iqvxPrtShortOrig, _nSamplesHalf);
    _allocArray(iqvxPrtLongOrig, _iqvxPrtLongOrig, _nSamplesHalf);
    
    _allocArray(iqhcPrtShort, _iqhcPrtShort, _nSamplesHalf);
    _allocArray(iqhcPrtLong, _iqhcPrtLong, _nSamplesHalf);
    _allocArray(iqvcPrtShort, _iqvcPrtShort, _nSamplesHalf);
    _allocArray(iqvcPrtLong, _iqvcPrtLong, _nSamplesHalf);
    
    _allocArray(iqhxPrtShort, _iqhxPrtShort, _nSamplesHalf);
    _allocArray(iqhxPrtLong, _iqhxPrtLong, _nSamplesHalf);
    _allocArray(iqvxPrtShort, _iqvxPrtShort, _nSamplesHalf);
    _allocArray(iqvxPrtLong, _iqvxPrtLong, _nSamplesHalf);
    
  } else {

    _freeArray(iqhcPrtShortOrig, _iqhcPrtShortOrig);
    _freeArray(iqhcPrtLongOrig, _iqhcPrtLongOrig);
    _freeArray(iqvcPrtShortOrig, _iqvcPrtShortOrig);
    _freeArray(iqvcPrtLongOrig, _iqvcPrtLongOrig);
    
    _freeArray(iqhxPrtShortOrig, _iqhxPrtShortOrig);
    _freeArray(iqhxPrtLongOrig, _iqhxPrtLongOrig);
    _freeArray(iqvxPrtShortOrig, _iqvxPrtShortOrig);
    _freeArray(iqvxPrtLongOrig, _iqvxPrtLongOrig);
    
    _freeArray(iqhcPrtShort, _iqhcPrtShort);
    _freeArray(iqhcPrtLong, _iqhcPrtLong);
    _freeArray(iqvcPrtShort, _iqvcPrtShort);
    _freeArray(iqvcPrtLong, _iqvcPrtLong);
    
    _freeArray(iqhxPrtShort, _iqhxPrtShort);
    _freeArray(iqhxPrtLong, _iqhxPrtLong);
    _freeArray(iqvxPrtShort, _iqvxPrtShort);
    _freeArray(iqvxPrtLong, _iqvxPrtLong);
    
  }
  
  if (_isStagPrt && _needFiltering) {
    
    _allocArray(iqhcPrtShortF, _iqhcPrtShortF, _nSamplesHalf);
    _allocArray(iqhcPrtLongF, _iqhcPrtLongF, _nSamplesHalf);
    _allocArray(iqvcPrtShortF, _iqvcPrtShortF, _nSamplesHalf);
    _allocArray(iqvcPrtLongF, _iqvcPrtLongF, _nSamplesHalf);
    
    _allocArray(iqhxPrtShortF, _iqhxPrtShortF, _nSamplesHalf);
    _allocArray(iqhxPrtLongF, _iqhxPrtLongF, _nSamplesHalf);
    _allocArray(iqvxPrtShortF, _iqvxPrtShortF, _nSamplesHalf);
    _allocArray(iqvxPrtLongF, _iqvxPrtLongF, _nSamplesHalf);
    
    _allocArray(iqhcPrtShortNotched, _iqhcPrtShortNotched, _nSamples);
    _allocArray(iqvcPrtShortNotched, _iqvcPrtShortNotched, _nSamples);
    _allocArray(iqhxPrtShortNotched, _iqhxPrtShortNotched, _nSamples);
    _allocArray(iqvxPrtShortNotched, _iqvxPrtShortNotched, _nSamples);

    _allocArray(iqhcPrtLongNotched, _iqhcPrtLongNotched, _nSamples);
    _allocArray(iqvcPrtLongNotched, _iqvcPrtLongNotched, _nSamples);
    _allocArray(iqhxPrtLongNotched, _iqhxPrtLongNotched, _nSamples);
    _allocArray(iqvxPrtLongNotched, _iqvxPrtLongNotched, _nSamples);

  } else {
    
    _freeArray(iqhcPrtShortF, _iqhcPrtShortF);
    _freeArray(iqhcPrtLongF, _iqhcPrtLongF);
    _freeArray(iqvcPrtShortF, _iqvcPrtShortF);
    _freeArray(iqvcPrtLongF, _iqvcPrtLongF);
    
    _freeArray(iqhxPrtShortF, _iqhxPrtShortF);
    _freeArray(iqhxPrtLongF, _iqhxPrtLongF);
    _freeArray(iqvxPrtShortF, _iqvxPrtShortF);
    _freeArray(iqvxPrtLongF, _iqvxPrtLongF);
    
    _freeArray(iqhcPrtShortNotched, _iqhcPrtShortNotched);
    _freeArray(iqvcPrtShortNotched, _iqvcPrtShortNotched);
    _freeArray(iqhxPrtShortNotched, _iqhxPrtShortNotched);
    _freeArray(iqvxPrtShortNotched, _iqvxPrtShortNotched);

    _freeArray(iqhcPrtLongNotched, _iqhcPrtLongNotched);
    _freeArray(iqvcPrtLongNotched, _iqvcPrtLongNotched);
    _freeArray(iqhxPrtLongNotched, _iqhxPrtLongNotched);
    _freeArray(iqvxPrtLongNotched, _iqvxPrtLongNotched);

  }

  if (_isSz) {

    _allocArray(iqStrong, _iqStrong, _nSamples);
    _allocArray(iqWeak, _iqWeak, _nSamples);
    _allocArray(iqStrongF, _iqStrongF, _nSamples);
    _allocArray(iqWeakF, _iqWeakF, _nSamples);
    
    _allocArray(iqMeas, _iqMeas, _nSamples);
    _allocArray(iqTrip1, _iqTrip1, _nSamples);
    _allocArray(iqTrip2, _iqTrip2, _nSamples);
    _allocArray(iqTrip3, _iqTrip3, _nSamples);
    _allocArray(iqTrip4, _iqTrip4, _nSamples);

  } else {

    _freeArray(iqStrong, _iqStrong);
    _freeArray(iqWeak, _iqWeak);
    _freeArray(iqStrongF, _iqStrongF);
    _freeArray(iqWeakF, _iqWeakF);
    
    _freeArray(iqMeas, _iqMeas);
    _freeArray(iqTrip1, _iqTrip1);
    _freeArray(iqTrip2, _iqTrip2);
    _freeArray(iqTrip3, _iqTrip3);
    _freeArray(iqTrip4, _iqTrip4);

  }
    
}

///////////////////////////////////////////////////
// free up all field arrays

void GateData::_freeArrays()

{

  _freeArray(iqhcOrig, _iqhcOrig);
  _freeArray(iqhxOrig, _iqhxOrig);
  _freeArray(iqvcOrig, _iqvcOrig);
  _freeArray(iqvxOrig, _iqvxOrig);

  _freeArray(iqhc, _iqhc);
  _freeArray(iqhx, _iqhx);
  _freeArray(iqvc, _iqvc);
  _freeArray(iqvx, _iqvx);

  _freeArray(specHc, _specHc);
  _freeArray(specVc, _specVc);

  _freeArray(iqhcF, _iqhcF);
  _freeArray(iqhxF, _iqhxF);
  _freeArray(iqvcF, _iqvcF);
  _freeArray(iqvxF, _iqvxF);

  _freeArray(iqhcNotched, _iqhcNotched);
  _freeArray(iqvcNotched, _iqvcNotched);
  _freeArray(iqhxNotched, _iqhxNotched);
  _freeArray(iqvxNotched, _iqvxNotched);

  _freeArray(iqhcPrtShortOrig, _iqhcPrtShortOrig);
  _freeArray(iqhcPrtLongOrig, _iqhcPrtLongOrig);
  _freeArray(iqvcPrtShortOrig, _iqvcPrtShortOrig);
  _freeArray(iqvcPrtLongOrig, _iqvcPrtLongOrig);

  _freeArray(iqhxPrtShortOrig, _iqhxPrtShortOrig);
  _freeArray(iqhxPrtLongOrig, _iqhxPrtLongOrig);
  _freeArray(iqvxPrtShortOrig, _iqvxPrtShortOrig);
  _freeArray(iqvxPrtLongOrig, _iqvxPrtLongOrig);

  _freeArray(iqhcPrtShort, _iqhcPrtShort);
  _freeArray(iqhcPrtLong, _iqhcPrtLong);
  _freeArray(iqvcPrtShort, _iqvcPrtShort);
  _freeArray(iqvcPrtLong, _iqvcPrtLong);

  _freeArray(iqhxPrtShort, _iqhxPrtShort);
  _freeArray(iqhxPrtLong, _iqhxPrtLong);
  _freeArray(iqvxPrtShort, _iqvxPrtShort);
  _freeArray(iqvxPrtLong, _iqvxPrtLong);

  _freeArray(iqhcPrtShortF, _iqhcPrtShortF);
  _freeArray(iqhcPrtLongF, _iqhcPrtLongF);
  _freeArray(iqvcPrtShortF, _iqvcPrtShortF);
  _freeArray(iqvcPrtLongF, _iqvcPrtLongF);

  _freeArray(iqhxPrtShortF, _iqhxPrtShortF);
  _freeArray(iqhxPrtLongF, _iqhxPrtLongF);
  _freeArray(iqvxPrtShortF, _iqvxPrtShortF);
  _freeArray(iqvxPrtLongF, _iqvxPrtLongF);

  _freeArray(iqhcPrtShortNotched, _iqhcPrtShortNotched);
  _freeArray(iqvcPrtShortNotched, _iqvcPrtShortNotched);
  _freeArray(iqhxPrtShortNotched, _iqhxPrtShortNotched);
  _freeArray(iqvxPrtShortNotched, _iqvxPrtShortNotched);

  _freeArray(iqhcPrtLongNotched, _iqhcPrtLongNotched);
  _freeArray(iqvcPrtLongNotched, _iqvcPrtLongNotched);
  _freeArray(iqhxPrtLongNotched, _iqhxPrtLongNotched);
  _freeArray(iqvxPrtLongNotched, _iqvxPrtLongNotched);
  
  _freeArray(iqStrong, _iqStrong);
  _freeArray(iqWeak, _iqWeak);
  _freeArray(iqStrongF, _iqStrongF);
  _freeArray(iqWeakF, _iqWeakF);

  _freeArray(iqMeas, _iqMeas);
  _freeArray(iqTrip1, _iqTrip1);
  _freeArray(iqTrip2, _iqTrip2);
  _freeArray(iqTrip3, _iqTrip3);
  _freeArray(iqTrip4, _iqTrip4);

}
