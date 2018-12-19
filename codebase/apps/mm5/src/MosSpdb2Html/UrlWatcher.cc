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

#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <iostream>
#include <strings.h>
#include <rapformats/ComboPt.hh>
#include <toolsa/DateTime.hh>
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

  char *paramsPath = (char *) "unknown";
  if (P.loadFromArgs(argc,argv,override.list,&paramsPath)){
    cerr << "Specify params file with -params option." << endl ;
    return(-1);
  }                       
  
  PMU_auto_init("MosSpdb2Html", P.Instance,
                PROCMAP_REGISTER_INTERVAL);     

  //
  // Sanity check on parameters.
  //
  if (P.VisibilityReplaceStrings_n != P.VisibilityReplaceValues_n){
    cerr << "Parameter arrays VisibilityReplaceStrings and VisibilityReplaceValues" << endl;
    cerr << "must be the same length - please rectify parameter file." << endl;
    exit(-1);
  }

  if (P.CeilingReplaceStrings_n != P.CeilingReplaceValues_n){
    cerr << "Parameter arrays CeilingReplaceStrings and CeilingReplaceValues" << endl;
    cerr << "must be the same length - please rectify parameter file." << endl;
    exit(-1);
  }

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
    // Fill a vector of entry type for use in the Process module.
    //
    SpdbTrigger::entry_t E;
    vector<SpdbTrigger::entry_t> UnprocessedEntries;

    const vector<Spdb::chunk_t> &chunks = InSpdb.getChunks();

    for (int ichunk = 0; ichunk < InSpdb.getNChunks(); ichunk++){

      E.dataTime = chunks[ichunk].valid_time;
      E.dataType = chunks[ichunk].data_type;
      E.dataType2= chunks[ichunk].data_type2;
      
      UnprocessedEntries.push_back( E );

    }

    Process(&P, UnprocessedEntries);


  } else { // REALTIME mode

    int go;
    do {

      if (P.Debug){
        time_t now = time(NULL);
        cerr << "Realtime mode, time: " << DateTime::strm(now) << endl;;
        cerr << "  Input URL: " << P.TriggerUrl << endl;
      }
    
      SpdbTrigger Trig( &P );             

      vector<SpdbTrigger::entry_t> UnprocessedEntries;
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
int UrlWatcher::ParseArgs(int argc,char *argv[],
                          tdrp_override_t &override){

  for (int i=1; i<argc; i++){
 
    if ( (!strcmp(argv[i], "-h")) ||
         (!strcmp(argv[i], "--")) ||
         (!strcmp(argv[i], "-?")) ) {                

      cout << "USAGE : MosSpdb2Html [-print_params to get parameters]" << endl;
      cout << "[-h or -- or -? to get this help message]" << endl;
      cout << "[-d, -debug or -v, -verbose to set debugging on]" << endl;
      cout << "[-interval YYYYMMDDhhmmss YYYYMMDDhhmmss -params pfile to run in archive mode]" << endl;
      return -1;

    }
    
    if (!strcmp(argv[i], "-d") || !strcmp(argv[i], "-debug")) {
      TDRP_add_override(&override, "Debug = TRUE;");
    }

    if (!strcmp(argv[i], "-v") || !strcmp(argv[i], "-verbose")) {
      TDRP_add_override(&override, "Debug = TRUE;");
      TDRP_add_override(&override, "Verbose = TRUE;");
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

     






