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
// GateData.hh
//
// Container for memory for IQ gate data.
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2007
//
///////////////////////////////////////////////////////////////
//
// Field data and IQ data for a single gate.
//
////////////////////////////////////////////////////////////////

#ifndef GateData_hh
#define GateData_hh

#include <radar/RadarComplex.hh>
#include <radar/MomentsFields.hh>
#include <radar/iwrf_data.h>
#include <toolsa/TaArray.hh>
using namespace std;

class GateData {
  
friend class Cmd;

public:

  // Default constructor.
  // Initializes all arrays to NULL.
  // Call allocArrays() before using.

  GateData();

  // destructor

  ~GateData();
  
  // Allocate all arrays, irrespective of mode.
  // On object allocated in this way can be used for any
  // computation in any mode.
  // Use with default constructor.
  
  void allocArrays(int nSamples, bool needFiltering, bool isStagPrt, bool isSz);

  // clear fields

  void initFields();

  // set the field data to zero
  
  void initFieldsToZero();

  // set field references

  void setFieldsToNormalTrip();
  void setFieldsToSecondTrip();

  // original time series - unwindowed IQ
  // single pol uses iqhc

  RadarComplex_t *iqhcOrig;
  RadarComplex_t *iqvcOrig;
  RadarComplex_t *iqhxOrig;
  RadarComplex_t *iqvxOrig;

  // windowed IQ
  // may have rectangular window (no difference from raw)
  // single pol uses iqhc

  RadarComplex_t *iqhc;
  RadarComplex_t *iqvc;
  RadarComplex_t *iqhx;
  RadarComplex_t *iqvx;

  // spectrum for windowed time series
  
  RadarComplex_t *specHc;
  RadarComplex_t *specVc;

  bool specHcComputed;
  bool specVcComputed;

  // filtered IQ dual pol
  // single pol uses iqhcF

  RadarComplex_t *iqhcF;
  RadarComplex_t *iqvcF;
  RadarComplex_t *iqhxF;
  RadarComplex_t *iqvxF;

  // notched time series for some dual pol moments

  RadarComplex_t *iqhcNotched;
  RadarComplex_t *iqvcNotched;
  RadarComplex_t *iqhxNotched;
  RadarComplex_t *iqvxNotched;

  // staggered PRT
  // original time series - unwindowed IQ
  
  RadarComplex_t *iqhcPrtShortOrig;
  RadarComplex_t *iqhcPrtLongOrig;
  RadarComplex_t *iqvcPrtShortOrig;
  RadarComplex_t *iqvcPrtLongOrig;

  RadarComplex_t *iqhxPrtShortOrig;
  RadarComplex_t *iqhxPrtLongOrig;
  RadarComplex_t *iqvxPrtShortOrig;
  RadarComplex_t *iqvxPrtLongOrig;

  // staggered PRT
  // windowed IQ
  
  RadarComplex_t *iqhcPrtShort;
  RadarComplex_t *iqhcPrtLong;
  RadarComplex_t *iqvcPrtShort;
  RadarComplex_t *iqvcPrtLong;

  RadarComplex_t *iqhxPrtShort;
  RadarComplex_t *iqhxPrtLong;
  RadarComplex_t *iqvxPrtShort;
  RadarComplex_t *iqvxPrtLong;

  // staggered PRT
  // filtered IQ
  
  RadarComplex_t *iqhcPrtShortF;
  RadarComplex_t *iqhcPrtLongF;
  RadarComplex_t *iqvcPrtShortF;
  RadarComplex_t *iqvcPrtLongF;

  RadarComplex_t *iqhxPrtShortF;
  RadarComplex_t *iqhxPrtLongF;
  RadarComplex_t *iqvxPrtShortF;
  RadarComplex_t *iqvxPrtLongF;

  RadarComplex_t *iqhcPrtShortNotched;
  RadarComplex_t *iqvcPrtShortNotched;
  RadarComplex_t *iqhxPrtShortNotched;
  RadarComplex_t *iqvxPrtShortNotched;

  RadarComplex_t *iqhcPrtLongNotched;
  RadarComplex_t *iqvcPrtLongNotched;
  RadarComplex_t *iqhxPrtLongNotched;
  RadarComplex_t *iqvxPrtLongNotched;

  RadarComplex_t *iqStrong;
  RadarComplex_t *iqWeak;
  RadarComplex_t *iqStrongF;
  RadarComplex_t *iqWeakF;

  RadarComplex_t *iqMeas;
  RadarComplex_t *iqTrip1;
  RadarComplex_t *iqTrip2;
  RadarComplex_t *iqTrip3;
  RadarComplex_t *iqTrip4;

  // fields

  MomentsFields fields;
  MomentsFields fieldsF; // filtered

  MomentsFields secondTrip; // sz
  MomentsFields secondTripF; // sz filtered

  // field pointers
  // can be switched between normal and second trip fields
  // used for computations in SZ mode

  MomentsFields *flds;
  MomentsFields *fldsF;

  // SZ8-64
  
  bool censorStrong;
  bool censorWeak;
  bool trip1IsStrong;
  bool clutterInStrong;
  bool clutterInWeak;
  double szLeakage;

protected:
private:

  // number of samples for this object

  int _nSamples;
  int _nSamplesHalf;

  // number of samples allocated by allocArrays()

  int _nSamplesAlloc;
  bool _needFiltering;
  bool _isStagPrt;
  bool _isSz;
  
  // original time series - unwindowed IQ
  // single pol uses iqhc

  TaArray<RadarComplex_t> _iqhcOrig;
  TaArray<RadarComplex_t> _iqvcOrig;
  TaArray<RadarComplex_t> _iqhxOrig;
  TaArray<RadarComplex_t> _iqvxOrig;

  // windowed IQ
  // may have rectangular window (no difference from raw)
  // single pol uses iqhc

  TaArray<RadarComplex_t> _iqhc;
  TaArray<RadarComplex_t> _iqvc;
  TaArray<RadarComplex_t> _iqhx;
  TaArray<RadarComplex_t> _iqvx;

  // spectrum for windowed time series
  
  TaArray<RadarComplex_t> _specHc;
  TaArray<RadarComplex_t> _specVc;

  // filtered IQ dual pol
  // single pol uses iqhcF

  TaArray<RadarComplex_t> _iqhcF;
  TaArray<RadarComplex_t> _iqvcF;
  TaArray<RadarComplex_t> _iqhxF;
  TaArray<RadarComplex_t> _iqvxF;

  // notched time series for some dual pol moments

  TaArray<RadarComplex_t> _iqhcNotched;
  TaArray<RadarComplex_t> _iqvcNotched;
  TaArray<RadarComplex_t> _iqhxNotched;
  TaArray<RadarComplex_t> _iqvxNotched;

  // staggered PRT
  // original time series - unwindowed IQ
  
  TaArray<RadarComplex_t> _iqhcPrtShortOrig;
  TaArray<RadarComplex_t> _iqhcPrtLongOrig;
  TaArray<RadarComplex_t> _iqvcPrtShortOrig;
  TaArray<RadarComplex_t> _iqvcPrtLongOrig;

  TaArray<RadarComplex_t> _iqhxPrtShortOrig;
  TaArray<RadarComplex_t> _iqhxPrtLongOrig;
  TaArray<RadarComplex_t> _iqvxPrtShortOrig;
  TaArray<RadarComplex_t> _iqvxPrtLongOrig;

  // staggered PRT IQ
  
  TaArray<RadarComplex_t> _iqhcPrtShort;
  TaArray<RadarComplex_t> _iqhcPrtLong;
  TaArray<RadarComplex_t> _iqvcPrtShort;
  TaArray<RadarComplex_t> _iqvcPrtLong;

  TaArray<RadarComplex_t> _iqhxPrtShort;
  TaArray<RadarComplex_t> _iqhxPrtLong;
  TaArray<RadarComplex_t> _iqvxPrtShort;
  TaArray<RadarComplex_t> _iqvxPrtLong;

  // staggered PRT filtered IQ
  
  TaArray<RadarComplex_t> _iqhcPrtShortF;
  TaArray<RadarComplex_t> _iqhcPrtLongF;
  TaArray<RadarComplex_t> _iqvcPrtShortF;
  TaArray<RadarComplex_t> _iqvcPrtLongF;

  TaArray<RadarComplex_t> _iqhxPrtShortF;
  TaArray<RadarComplex_t> _iqhxPrtLongF;
  TaArray<RadarComplex_t> _iqvxPrtShortF;
  TaArray<RadarComplex_t> _iqvxPrtLongF;

  // staggered PRT notched IQ
  
  TaArray<RadarComplex_t> _iqhcPrtShortNotched;
  TaArray<RadarComplex_t> _iqvcPrtShortNotched;
  TaArray<RadarComplex_t> _iqhxPrtShortNotched;
  TaArray<RadarComplex_t> _iqvxPrtShortNotched;

  TaArray<RadarComplex_t> _iqhcPrtLongNotched;
  TaArray<RadarComplex_t> _iqvcPrtLongNotched;
  TaArray<RadarComplex_t> _iqhxPrtLongNotched;
  TaArray<RadarComplex_t> _iqvxPrtLongNotched;

  // SZ
  
  TaArray<RadarComplex_t> _iqStrong;
  TaArray<RadarComplex_t> _iqWeak;
  TaArray<RadarComplex_t> _iqStrongF;
  TaArray<RadarComplex_t> _iqWeakF;

  TaArray<RadarComplex_t> _iqMeas;
  TaArray<RadarComplex_t> _iqTrip1;
  TaArray<RadarComplex_t> _iqTrip2;
  TaArray<RadarComplex_t> _iqTrip3;
  TaArray<RadarComplex_t> _iqTrip4;

  // modes

  // iwrf_xmit_rcv_mode_t _xmitRcvMode;
  // bool _applyFiltering;
  // bool _applySz;

  // methods

  void _initArraysToNull();
  void _freeArrays();

  // allocate a field array, if not previously done

  inline void _allocArray(RadarComplex_t* &field,
                          TaArray<RadarComplex_t> &array,
                          int nSamples) {
    field = array.alloc(nSamples);
  }

  // free up a field array

  inline void _freeArray(RadarComplex_t* &field,
                         TaArray<RadarComplex_t> &array) {
    array.free();
    field = NULL;
  }

};

#endif

