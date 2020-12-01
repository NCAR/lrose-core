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
// IpsGateData.hh
//
// Container for memory for IQ gate data.
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Aug 2019
//
///////////////////////////////////////////////////////////////
//
// Support for Independent Pulse Sampling.
//
// Field data and IQ data for a single gate.
//
////////////////////////////////////////////////////////////////

#ifndef IpsGateData_hh
#define IpsGateData_hh

#include <radar/RadarComplex.hh>
#include <radar/IpsMomFields.hh>
#include <radar/ips_ts_data.h>
#include <toolsa/TaArray.hh>
using namespace std;

class IpsGateData {
  
friend class Cmd;

public:

  // Default constructor.
  // Initializes all arrays to NULL.
  // Call allocArrays() before using.

  IpsGateData();

  // destructor

  ~IpsGateData();
  
  // Allocate all arrays, irrespective of mode.
  // On object allocated in this way can be used for any
  // computation in any mode.
  // Use with default constructor.
  
  void allocArrays(int nSamples, bool isStagPrt);

  // clear fields

  void initFields();

  // IQ data, single pol uses iqhc

  RadarComplex_t *iq0;
  RadarComplex_t *iq1;

  RadarComplex_t *iqhc;
  RadarComplex_t *iqvc;
  RadarComplex_t *iqhx;
  RadarComplex_t *iqvx;

  // staggered PRT IQ
  
  RadarComplex_t *iqhcPrtShort;
  RadarComplex_t *iqhcPrtLong;
  RadarComplex_t *iqvcPrtShort;
  RadarComplex_t *iqvcPrtLong;

  RadarComplex_t *iqhxPrtShort;
  RadarComplex_t *iqhxPrtLong;
  RadarComplex_t *iqvxPrtShort;
  RadarComplex_t *iqvxPrtLong;

  // fields

  IpsMomFields fields;
  IpsMomFields secondTrip;

  // field references
  // can be switched between normal and second trip fields
  // used for computations in SZ mode

  IpsMomFields &flds;

protected:
private:

  // number of samples for this object

  int _nSamples;
  int _nSamplesHalf;

  // number of samples allocated by allocArrays()

  int _nSamplesAlloc;
  bool _isStagPrt;
  
  // IQ, single pol uses iqhc

  TaArray<RadarComplex_t> _iq0;
  TaArray<RadarComplex_t> _iq1;

  TaArray<RadarComplex_t> _iqhc;
  TaArray<RadarComplex_t> _iqvc;
  TaArray<RadarComplex_t> _iqhx;
  TaArray<RadarComplex_t> _iqvx;

  // staggered PRT IQ
  
  TaArray<RadarComplex_t> _iqhcPrtShort;
  TaArray<RadarComplex_t> _iqhcPrtLong;
  TaArray<RadarComplex_t> _iqvcPrtShort;
  TaArray<RadarComplex_t> _iqvcPrtLong;

  TaArray<RadarComplex_t> _iqhxPrtShort;
  TaArray<RadarComplex_t> _iqhxPrtLong;
  TaArray<RadarComplex_t> _iqvxPrtShort;
  TaArray<RadarComplex_t> _iqvxPrtLong;

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

