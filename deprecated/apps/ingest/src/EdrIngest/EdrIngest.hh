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
/////////////////////////////////////////////////////////////////////////
// EdrIngest.hh
//
// EdrIngest object
//
// Gary Blackburn, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2008
//
///////////////////////////////////////////////////////////////////////////


#ifndef EDRINGEST_HH
#define EDRINGEST_HH

#include <tdrp/tdrp.h>
#include <iostream>
#include <string>
#include <vector>
#include <toolsa/DateTime.hh>
#include <mel_bufr/mel_bufr.h>
#include <stdlib.h>
#include <limits>       // std::numeric_limits

#include "Args.hh"
#include "Params.hh"
#include "EdrInput.hh"
#include "EdrReport.hh"
#include "UalTailMap.hh"

using namespace std;

class EdrIngest {
  
public:

  // constructor

  EdrIngest (int argc, char **argv);

  // destructor
  
  ~EdrIngest();

  // run 

  int Run();

  // decode

  EdrReport::status_t decodeEdrMsg (ui08* buffer, DateTime msgTime);
  EdrReport::status_t decodeAsciiMsg (ui08 *buffer, DateTime msgTime);
  EdrReport::status_t decodeBufrMsg (ui08 *buffer, DateTime msgTime);

  // encode tail numbers
  string encrypt(const char* orig_tail);
  string decrypt(const char * encoded_string);

  // data members

  int isOK;

protected:
  

private:

  static const ui32 iters;
  static const ui32 ui32Nbytes;
  static const si32 shifts[];    //number of bits to shift for each iter.  Positive ++ 2^n
  static const ui32 key[];
  static const ui32 permutes[];
  //static const ui32 perms[][];

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;

  EdrInput  *inputStream;

  EdrReport *_report;

  UalTailMap *tailMap;


  // methods

  void clearAll();

  int gotFlightNumber(char * flight_id, BUFR_Val_t &bv);
  int getBufrVar(char *stringVar, char* fxy,  BUFR_Val_t &bv);

  string unbitify (ui32 data, int encoding);
  ui32 bitify(const char* orig_tail, int encoding);
  vector <fl64>  alpha2Numeric (const char *orig_string, int encoding);
  string numeric2Alpha (vector <fl64> numeric, int encoding);
  ui32 permute (ui32 bitEncoding, si32 shifts, const ui32 permutes[]);
  ui32 invpermute (ui32 inputNum, si32 shift, const ui32 *perms);
  ui32 circularBitShift (ui32 data, si32 shift);

};

#endif
