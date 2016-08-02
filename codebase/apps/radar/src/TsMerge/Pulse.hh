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
#include <dataport/port_types.h>
#include "Params.hh"
#include "Complex.hh"

#define NINT(fvalue) ((si32)floor(0.5+(double)(fvalue)))

using namespace std;

////////////////////////
// This class

class Pulse {
  
public:

  Pulse(const Params &params,
        double fSyClkMhz);

  ~Pulse();
  
  // read pulse data
  // Returns 0 on success, -1 on failure

  int read(FILE *in);
  
  // printing
  
  void print(ostream &out) const;

  // get methods

  int getNIq() const { return _nIQ; }
  int getNGates() const { return _nGates; }
  int getNChannels() const { return _nChannels; }

  double getFTime() const { return _ftime; }
  time_t getTime() const { return _time; }
  double getPrt() const { return _prt; }
  double getPrf() const { return _prf; }
  double getEl() const { return _el; }
  double getAz() const { return _az; }
  double getPhaseDiff0() const { return _phaseDiff[0]; }
  double getPhaseDiff1() const { return _phaseDiff[1]; }
  bool isHoriz() const; // is horizontally polarized

  // pack floats into int16 array

  // RVP8 pulse header fields
  
  int iVersion;
  int iFlags;
  int iMSecUTC;
  int iTimeUTC;
  int iBtimeAPI;
  int iSysTime;
  int iPrevPRT;
  int iNextPRT;
  int iSeqNum;
  int iAqMode;
  int iPolarBits;
  int iTxPhase;
  int iAz;
  int iEl;
  int iNumVecs;
  int iMaxVecs;
  int iVIQPerBin;
  int iTgBank;
  int iTgWave;
  
  int uiqPerm[2];
  int uiqOnce[2];

  double fBurstMag[2];
  int iBurstArg[2];
  
  // pack the float IQ data into int16
  
  void packIq();

  // reduce the IQ values by a given DB value

  void reduceIq(double reductionDb);

  // merge IQ data from another pulse into this one by
  // adding the IQ data
  //
  // returns 0 on success, -1 on failure
  
  int addIq(const fl32 *iq, int niq,
            double reductionDb);

  // get IQ data
  
  const ui16 *getPacked() const { return _packed; }
  const fl32 *getIq() const { return _iq; }

protected:
private:

  const Params &_params;

  // IFD clock rate in MHZ

  double _fSyClkMhz;

  // derived from pulse header

  int _nGates;
  int _nChannels;
  time_t _time;
  double _ftime;
  double _prt;
  double _prf;
  double _el;
  double _az;
  double _phaseDiff[2]; // phase difference from previous pulse

  // floating point IQ data

  int _nIQ;

  fl32 *_iq;
  ui16 *_packed;

  // functions
  
  int _readPulseHeader(FILE *in);
  void _deriveFromPulseHeader();
  int _readPulseData(FILE *in);
  
  void _vecFloatIQFromPackIQ
    (volatile fl32 fIQVals_a[], volatile const ui16 iCodes_a[],
     si32 iCount_a);

  void _vecPackIQFromFloatIQ
    ( volatile ui16 iCodes_a[], volatile const fl32 fIQVals_a[],
      si32 iCount_a);

  void _subtract(const Complex_t &aa, const Complex_t &bb,
		 Complex_t &result, double &angle);

#ifdef JUNK
  void vecPackIQFromFloatIQ_
    ( volatile UINT2 iCodes_a[], volatile const FLT4 fIQVals_a[],
      SINT4 iCount_a, UINT4 iFlags_a );

  void vecFloatIQFromPackIQ_
    ( volatile FLT4 fIQVals_a[], volatile const UINT2 iCodes_a[],
      SINT4 iCount_a, UINT4 iFlags_a );

  void vecFloatIQFromPackIQ_comp
    ( volatile FLT4 fIQVals_a[], volatile const UINT2 iCodes_a[],
      SINT4 iCount_a, UINT4 iFlags_a );
#endif

};

#endif

