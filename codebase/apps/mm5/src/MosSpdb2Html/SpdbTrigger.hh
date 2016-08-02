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

#ifndef _SPDB_TRIGGER_INC_
#define _SPDB_TRIGGER_INC_


#include <vector>
#include <string>

#include "Params.hh"
using namespace std;

class SpdbTrigger {

public:

  //
  // Struct that holds enough to define a unique spdb
  // entry.
  //
  typedef struct {
    time_t dataTime;
    int dataType;
    int dataType2;
  } entry_t;


  //
  // Constructor. Just make copies of input args.
  //
  SpdbTrigger( const Params *params);


  //
  // Main method - get vector of entries.
  //
  int getNextTimes(vector<SpdbTrigger::entry_t> &UnprocessedEntries );

  //
  // Destructor
  //
  ~SpdbTrigger();

  private :

  //
  // Interval for regular triggering, and the
  // next and last times.
  //
  time_t             _prevTime;
  time_t             _nextTime;

  //
  // TDRP parameters.
  //
  Params *_params;

  //
  // Methods.
  //
  time_t _triggerOnInterval();


};




#endif
