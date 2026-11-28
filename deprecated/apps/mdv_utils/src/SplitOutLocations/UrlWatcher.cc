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

#include <toolsa/pmu.h>
#include <Mdv/DsMdvxTimes.hh>
#include <iostream>
#include <strings.h>

#include "UrlWatcher.hh"
#include "Process.hh"
using namespace std;

//
// Constructor
//
UrlWatcher::UrlWatcher(){
  startTime = 0L; endTime = 0L;
  return;
}
//////////////////////////////////////////////  
//
// Destructor
//
UrlWatcher::~UrlWatcher(){
  return;
}

//////////////////////////////////////////////////
// Main method - run.
//
int UrlWatcher::init( int argc, char **argv ){

  if (argc < 3) {
    cerr << "USAGE : SplitOutLocations URL YYYYMMDD" << endl;
    exit(-1);
  }

  date_time_t dt;
  if (3 != sscanf(argv[2],"%4d%2d%2d", &dt.year, &dt.month, &dt.day)){
    cerr << "USAGE : SplitOutLocations URL YYYYMMDD" << endl;
    exit(-1);
  }

  dt.hour = 0; dt.min = 0; dt.sec = 0;
  uconvert_to_utime( &dt );
  startTime = dt.unix_time;
  endTime = startTime + 86399;

  cerr << "Working from " << utimstr(startTime) << " to " << utimstr(endTime) << endl;


  if (InMdv.setArchive(argv[1],
		       startTime,
		       endTime)){
    cerr << "Failed to set URL " << argv[1] << endl;
    return -1;
  }


  //
  // Input Mdv object should now be all set. Use it in the run function.
  //
  return 0;

}

int UrlWatcher::run(char **argv){

  Process S;
  do{

    time_t Time;
    
    InMdv.getNext( Time );

    if (Time != (time_t)NULL){
      S.Derive(argv[1], Time);
    } else {
      break;
    }

  } while (1);
  
  return 0;

}







