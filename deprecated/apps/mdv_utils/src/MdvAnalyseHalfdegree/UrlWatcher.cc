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
#include <unistd.h>

#include "Params.hh"
#include "UrlWatcher.hh"
#include "Process.hh"
using namespace std;

//
// Constructor
//
UrlWatcher::UrlWatcher(){
  startTime = 0; endTime = 0; // Default to REALTIME mode.
}
//////////////////////////////////////////////  
//
// Destructor
//
UrlWatcher::~UrlWatcher(){
}

//////////////////////////////////////////////////
// Main method - run.
//
int UrlWatcher::init( int argc, char **argv ){

  //
  // Parse command line args. Pretty minimal for this program.
  //
  if (ParseArgs(argc,argv)) return -1;
  //
  // Get TDRP args and check in to PMU.
  //

  
  if (P.loadFromArgs(argc,argv,NULL,NULL)){
    cerr << "Specify params file with -params option." << endl ;
    return(-1);
  }                       
  
  PMU_auto_init("MdvAnalyseHalfdegree", P.Instance,
                PROCMAP_REGISTER_INTERVAL);     

  if ((startTime != 0) || (endTime != 0)){
    P.Mode = Params::ARCHIVE;
  }

  //
  // Remove the input file if it exists.
  //
  unlink(P.outFileName);


  //
  // Set up for input.
  // Depends on what mode we're in.
  //
  if (P.Mode == Params::ARCHIVE){
    //
    // Only set this up if we are not dealing with forecast data
    //
    if (!(P.printLeadTime)){
      if (InMdv.setArchive(P.TriggerUrl,
			   startTime,
			   endTime)){
	cerr << "Failed to set URL " << P.TriggerUrl << endl;
	return -1;
      }
    }

  } else { // REALTIME mode


    if (InMdv.setRealtime(P.TriggerUrl,
			  P.MaxRealtimeValidAge,
			  PMU_auto_register)){
      cerr << "Failed to set URL " << P.TriggerUrl << endl;
      return -1;
    }



  } // End of if we are in realtime.

  //
  // Input Mdv object should now be all set. Use it in the run function.
  //
  return 0;

}

int UrlWatcher::run(){

  if (
      (P.Mode == Params::ARCHIVE) &&
      (P.printLeadTime)
      ){
    //
    // Special case - get all lead times.
    //
    DsMdvx Tlist;
    //
    // Get gen times.
    //
    Tlist.setTimeListModeGen(P.TriggerUrl, startTime, endTime);
    //
    // Loop through gen times getting valid times.
    //
    Tlist.compileTimeList();
    vector<time_t> inTimes = Tlist.getTimeList();

    if (inTimes.size() == 0){
      cerr << "No data for specified date found." << endl;
      return -1;
    }

    if (P.Debug){
      cerr << inTimes.size() << " generate times found." << endl;
      for (unsigned i=0; i < inTimes.size(); i++){
	cerr << "Time " << i << " : " << utimstr( inTimes[i] ) << endl;
      }
    }

    vector <time_t>::iterator it;
    for (it = inTimes.begin(); it != inTimes.end(); it++){

      DsMdvx inTim;
      inTim.setTimeListModeForecast( P.TriggerUrl, *it );
      inTim.compileTimeList();

      vector <time_t> vtimes = inTim.getValidTimes();

      if (P.Debug){
	cerr << vtimes.size() << " valid times found for gentime " << utimstr( *it ) << endl;
	for (unsigned ik=0; ik < vtimes.size(); ik++){
	  cerr << "  Valid time " << ik << " is " << utimstr(vtimes[ik]) << endl;
	}
      }

      for (unsigned ik=0; ik < vtimes.size(); ik++){
	Process S;
	S.Derive(&P, vtimes[ik], *it);
      }
    }

    return 0;

  }



  do{

    time_t Time;
    
    InMdv.getNext( Time );
    
    if ((Time == (time_t)NULL) && (P.Mode == Params::ARCHIVE)){
      return 0; // Reached end of the list in archive mode.
    }

    if (Time != (time_t)NULL){
      Process S;
      S.Derive(&P, Time, 0L);
    }

  } while (1);
  
  return 0;

}
///////////////////////////////////////////////
// Parse command line args.
//
int UrlWatcher::ParseArgs(int argc,char *argv[]){

  for (int i=1; i<argc; i++){
 
    if ( (!strcmp(argv[i], "-h")) ||
         (!strcmp(argv[i], "--")) ||
         (!strcmp(argv[i], "-?")) ) {                
      cout << "USAGE : MdvAnalyseHalfdegree [-print_params to get parameters]" << endl;
      cout << "[-h or -- or -? to get this help message]" << endl;
      cout << "[-interval YYYYMMDDhhmmss YYYYMMDDhhmmss -params pfile to run in archive mode]" << endl;
      return -1;

    }
    
    if (!strcmp(argv[i], "-interval")){
      i++;
      if (i == argc) {
	cerr << "Must specify start and end times with -interval" << endl;
	exit(-1);
      }
      
      date_time_t T;
      if (6!=sscanf(argv[i],"%4d%2d%2d%2d%2d%2d",
		    &T.year, &T.month, &T.day,
		    &T.hour, &T.min, &T.sec)){
	cerr << "Format for start time is YYYYMMDDhhmmss" << endl;
	return -1;
      }
      uconvert_to_utime( &T );
      startTime = T.unix_time;
    
      i++;
      if (i == argc) {
	cerr << "Must specify end time with -interval" << endl;
	return -1;
      }

      if (6!=sscanf(argv[i],"%4d%2d%2d%2d%2d%2d",
		    &T.year, &T.month, &T.day,
		    &T.hour, &T.min, &T.sec)){
	cerr << "Format for end time is YYYYMMDDhhmmss" << endl;
	exit(-1);
      }
      uconvert_to_utime( &T );
      endTime = T.unix_time;


    }


  }

  return 0; // All systems go.
  
}

     






