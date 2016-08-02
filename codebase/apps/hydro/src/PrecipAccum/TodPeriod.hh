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
// TodPeriod.hh
//
// Keeps track of which Time-Of-Day period we are in.
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 1998
//
///////////////////////////////////////////////////////////////

#ifndef TodPeriod_HH
#define TodPeriod_HH

#include "PrecipAccum.hh"
#include <toolsa/umisc.h>
#include <rapformats/zrpf.h>
#include <string>
using namespace std;

typedef struct {
  time_t start;
  time_t end;
} period_times_t;

////////////////////////////////
// TodPeriod - abstract base class

class TodPeriod {
  
public:

  // constructor

  TodPeriod(const string &prog_name, const Params &params);
  
  // destructor
  
  virtual ~TodPeriod();

  // Set the period times given the trigger time.
  // Returns TRUE if period number has changed, FALSE otherwise.

  int set(time_t trigger_time);

  // start and end times of current period

  time_t periodStart;
  time_t periodEnd;

  // flag for success in construction

  int OK;

protected:
  
private:

  const string &_progName;
  const Params &_params;
  int _nPeriods;
  int _periodNum;;
  period_times_t *_periods;
  zrpf_handle_t _zrHandle;

};

#endif
