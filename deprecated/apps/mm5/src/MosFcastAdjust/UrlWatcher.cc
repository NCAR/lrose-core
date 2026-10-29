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
#include <toolsa/udatetime.h>
#include <iostream>
#include <strings.h>
#include <rapformats/ComboPt.hh>

#include <vector>

#include "Params.hh"
#include "UrlWatcher.hh"
#include "SpdbTrigger.hh"
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
  tdrp_override_t override;
  TDRP_init_override(&override);
  if (ParseArgs(argc,argv,override)) return -1;
  //
  // Get TDRP args and check in to PMU.
  //

  
  if (P.loadFromArgs(argc,argv,override.list,NULL)){
    cerr << "Specify params file with -params option." << endl ;
    return(-1);
  }                       
  
  PMU_auto_init("MosFcastAdjust", P.Instance,
                PROCMAP_REGISTER_INTERVAL);     

  //
  // If times are specified on the command line, set the
  // mode to archive.
  //
  if ((startTime != 0) || (endTime != 0)){
    P.Mode = Params::ARCHIVE;
  }


  return 0;
}

//////////////////////////////////////////////////////
//
//
//
int UrlWatcher::run(){
  //
  // Set up for input.
  // Depends on what mode we're in.
  //
  if (P.Mode == Params::ARCHIVE){
    if (InSpdb.getInterval(P.TriggerUrl,startTime, endTime)) {
      cerr << "Failed to get spdb data from " << P.TriggerUrl << endl;
      return -1;
    }

    if (P.Debug){
      cerr << "Archive mode : ";
      cerr << InSpdb.getNChunks() << " chunks between ";
      cerr << utimstr(startTime) << " and " << utimstr(endTime);
      cerr << endl;
    }
    
    //
    // Fill a list of entry type for use in the Process module.
    //
    SpdbTrigger::entry_t E;
    list<SpdbTrigger::entry_t> UnprocessedEntries;

    //
    // For each station in the input, get a list of
    // metars and process them. If the UseStationIDs parameter is
    // set to true, then only accept those stations that are on
    // the list.
    //

    int *DesiredIDs = NULL;
    if (P.UseStationIDs){
      //
      // Allocate room for the array.
      //
      DesiredIDs = (int *) malloc(sizeof(int) * P.station_ids_n);
      if (DesiredIDs == NULL){
	cerr << "Malloc failed." << endl;
	exit(-1); // Unlikely.
      }
      //
      // Fill this array with the integer values that
      // are represented by the list of desired stations.
      //
      for (int q=0; q < P.station_ids_n; q++){
	DesiredIDs[q] = Spdb::hash4CharsToInt32(P._station_ids[q]);
      }
    }


    const vector<Spdb::chunk_t> &chunks = InSpdb.getChunks();

    for (int ichunk = 0; ichunk < InSpdb.getNChunks(); ichunk++){

      E.dataTime = chunks[ichunk].valid_time;
      E.dataType = chunks[ichunk].data_type;
      E.dataType2= chunks[ichunk].data_type2;
      

      if (!(P.UseStationIDs)){
	//
	// Just push them all back in this case. Not efficient.
	//
	UnprocessedEntries.push_back( E );
      } else {
	//
	// Check if this is from a station we can use
	// before pushing it back.
	//
	if (IsInArray(E.dataType, DesiredIDs,
		      P.station_ids_n)){
	  
	  UnprocessedEntries.push_back( E );

	}
      }

    }

    Process(&P, UnprocessedEntries);


  } else { // REALTIME mode

    int go;
    do {
      SpdbTrigger Trig( &P );             

      list<SpdbTrigger::entry_t> UnprocessedEntries;
      go=Trig.getNextTimes( UnprocessedEntries ); 

      Process(&P, UnprocessedEntries);

    } while (go == 0);

    return go;

  } // End of if we are in realtime.

  //
  //
  return 0;

}


///////////////////////////////////////////////
// Parse command line args.
//
int UrlWatcher::ParseArgs(int argc,char *argv[], tdrp_override_t &override)

{

  char tmp_str[BUFSIZ];

  for (int i=1; i<argc; i++){
 
    if ( (!strcmp(argv[i], "-h")) ||
         (!strcmp(argv[i], "--")) ||
         (!strcmp(argv[i], "-?")) ) {                
      cout << "USAGE : MosFcastAdjust" << endl;
      cout << "  [-h or -- or -?] to get this help message" << endl;
      cout << "  [-interval YYYYMMDDhhmmss YYYYMMDDhhmmss] for archive mode"
	   << endl;
      cout << "  [-debug] for debug output" << endl;
      cout << "  [-verbose] for verbose debug output" << endl;
      cout << endl;
      Params::usage(cout);
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

    } else if (!strcmp(argv[i], "-debug")) {
      
      sprintf(tmp_str, "Debug = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-verbose")) {
      
      sprintf(tmp_str, "Verbose = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    }


  }

  return 0; // All systems go.
  
}

     



