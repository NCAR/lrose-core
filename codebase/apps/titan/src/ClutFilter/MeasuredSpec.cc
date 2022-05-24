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
//////////////////////////////////////////////////////////
// MeasuredSpec.cc
//
// Measured spectrum
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2004
//
//////////////////////////////////////////////////////////

#include "MeasuredSpec.hh"

#include <cstdio>
#include <iostream>
#include <cstring>
#include <toolsa/udatetime.h>
using namespace std;

// constructor

MeasuredSpec::MeasuredSpec()

{
  _iq = NULL;
}

// destructor

MeasuredSpec::~MeasuredSpec()

{
  if (_iq != NULL) {
    delete[] _iq;
  }
}

////////////////////////////////////////////////
// set state by reading input file
// returns 0 on success, -1 on failure

int MeasuredSpec::read(FILE *inFile)

{
  
  date_time_t stime;
  
  if (fscanf(inFile, "%d %d %d %d %d %d %d",
	     &_index,
	     &stime.year, &stime.month, &stime.day,
	     &stime.hour, &stime.min, &stime.sec) != 7) {
    return -1;
  }
  uconvert_to_utime(&stime);
  _time = stime.unix_time;
  
  if (fscanf(inFile, "%lg %lg %d %lg %lg %lg %d",
	     &_el, &_az, &_gateNum, &_snr, &_vel, &_width, &_nSamples) != 7) {
    return -1;
  }
  if (_nSamples > 1024) {
    return -1;
  }

  _iq = new Complex_t[_nSamples];
  for (int ii = 0; ii < _nSamples; ii++) {
    if (fscanf(inFile, "%lg %lg",
	       &_iq[ii].re, &_iq[ii].im) != 2) {
      return -1;
    }
  } // ii

  return 0;
  
}

