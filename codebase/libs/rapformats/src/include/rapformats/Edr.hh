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
/*
 *   Module: Edr.hh
 *
 *   Author: Sue Dettling
 *
 *   Description: 
 */

#ifndef EDR_HH
#define EDR_HH

#include <cstdio>
#include <iostream>
#include <string>
#include <vector>
#include <toolsa/MemBuf.hh>
#include <dataport/port_types.h>
using namespace std;

class Edr {

private:
public:

  static const si32 SRC_NAME_LEN = 80;
  static const si32 SRC_FMT_LEN = 40;
  static const si32 AIRPORT_NAME_LEN = 4;
  static const si32 TAILNUM_NAME_LEN = 16; 
  static const si32 FLIGHTNUM_NAME_LEN = 16; 
  static const si32 AIRLINE_NAME_LEN = 8; 

  static const si32 NUM_SPARE_INTS = 4;
  static const si32 NUM_SPARE_FLOATS = 4;
  static const si32 NUM_QC_INDS = 4;
  static const si32 NUM_QC_FLAGS = 4;

  static const si32 NBYTES_32 = 156;
  
  static const fl32 VALUE_UNKNOWN;
  static const si32 QC_NO_VALUE = 99990;
  static const si32 QC_BAD_CHR = 99991;
  static const si32 QC_BAD_CGA = 99992;
  static const si32 QC_BAD_ALT = 99993;
  static const si32 QC_BAD_MACH = 99994;
  static const si32 QC_BAD_OTHER = 99995;
  static const si32 QC_BAD_BASIS = 99996;
  static const si32 QC_BAD_VALUE = 99999;
  static const si32 QC_GOOD_VALUE = 1;

  static const si32 BELOW_MIN_ALT = 1;
  static const si32 FAILED_ONBOARD_QC = 2;
  static const si32 FAILED_BOUNDS_CK = 4;
  static const si32 BAD_TAIL = 8;
  static const si32 LOW_ONBOARD_QC = 16;
  static const si32 UNKNOWN_TAIL_STATUS = 32;

  typedef struct
  {
    si32 time;
    si32 fileTime;
    fl32 lat;
    fl32 lon;
    fl32 alt;
    si32 interpLoc;
    fl32 edrPeak;
    fl32 edrAve;
    fl32 wspd;
    fl32 wdir;
    fl32 sat;
    fl32 qcConf;
    fl32 qcThresh;
    fl32 qcVersion;
    fl32 edrAlgVersion;

    // Used the 4 float values previously stored in edrPeakQcInds 
    // for the following values required for the new Delta
    // and Southwest messages.
 
    fl32 PeakConf;
    fl32 MeanConf;
    fl32 PeakLocation;
    fl32 NumGood;

    fl32 edrAveQcInds[NUM_QC_FLAGS];
    si32 edrPeakQcFlags[NUM_QC_FLAGS]; // maybe change this to something more descriptive 
                                       
    si32 edrAveQcFlags[NUM_QC_FLAGS];


    // Used the 3 of the spare float values from spareFloats 
    // for the following values required for the new Delta
    // MDCRS data.
    fl32 mach;  // Delta MDCRS specific information
    fl32 rms;   // Max rms during previous 15 minutes, Delta MDCRS
    fl32 runningMinConf;            
    fl32 spareFloats[NUM_SPARE_FLOATS - 3];

    // Used 3 of the spare ints to store, computed air speed, running
    // Min confidence and bit flags for storing the ground based QC information
    si32 maxNumBad;                  
    si32 QcDescriptionBitFlags;      // Bit flag - set to VALUE_UNKNOWN if undefined
    si32 computedAirspeed;           // Delta MDCRS specific data
    // Used 1 of the spare ints to store Delta's is767 flag (0 = FALSE, 1 = TRUE);
    si32 is767;
    si32 spareInts[NUM_SPARE_INTS - 4];

    char aircraftRegNum[TAILNUM_NAME_LEN];
    char encodedAircraftRegNum[TAILNUM_NAME_LEN];
    char flightNum[FLIGHTNUM_NAME_LEN];
    char origAirport[AIRPORT_NAME_LEN];
    char destAirport[AIRPORT_NAME_LEN];
    char airlineId[AIRLINE_NAME_LEN];
    char sourceName[SRC_NAME_LEN];
    char sourceFmt[SRC_FMT_LEN];

  } edr_t;

  // constructor

  Edr();

  // destructor

  ~Edr();
  
  //
  //
  // assemble()
  // Load up the buffer from the object.
  // Handles byte swapping.
  //
  void assemble();

  //
  // get the assembled buffer info
  //
  void *getBufPtr() const { return _memBuf.getPtr(); }
  int getBufLen() const { return _memBuf.getLen(); }

  //
  // disassemble()
  // Disassembles a buffer, sets the values in the object.
  // Handles byte swapping.
  // Returns 0 on success, -1 on failure
  //
  int disassemble(const void *buf, int len);

  //
  // set methods
  //
  void setEdr(const edr_t &rep);
 
  //
  // get methods
  //
  si32 getTime() { return _edr.time; }  
  const edr_t &getRep() const { return _edr; }
 
	 
  //
  // print edr
  //
  void print(ostream &out, string spacer = "") const;

protected:

private:
  
  //
  // data members 
  //
  MemBuf _memBuf;
  
  edr_t _edr; 
  
  //
  // byte swapping
  //
  static void _edr_to_BE(edr_t& rep);
  static void _edr_from_BE(edr_t& rep);

};

#endif /* EDR_HH */
