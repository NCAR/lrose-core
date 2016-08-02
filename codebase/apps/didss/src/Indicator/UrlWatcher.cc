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

#include "Params.hh"
#include "UrlWatcher.hh"
#include "Reader.hh"
using namespace std;

//
// Constructor
//
UrlWatcher::UrlWatcher(){
  startTime = 0; endTime = 0; // Default to REALTIME mode.

  /*
  for (int i=0; i < MaxUrls; i++){
    TruthMdv[i] = NULL; ForecastMdv[i] = NULL;
  }
  */
}
//////////////////////////////////////////////  
//
// Destructor
//
UrlWatcher::~UrlWatcher(){

  /*
  for (int i=0; i < MaxUrls; i++){
    delete TruthMdv[i]; delete ForecastMdv[i];
  }
  */
}

//////////////////////////////////////////////////
// Main method - run.
//
int UrlWatcher::run( int argc, char **argv ){

  //
  // Parse command line args. Pretty minimal for this program.
  //
  if (ParseArgs(argc,argv)) return -1;
  //
  // Get TDRP args and check in to PMU.
  //

  Params P;
  
  if (P.loadFromArgs(argc,argv,NULL,NULL)){
    cerr << "Specify params file with -params option." << endl ;
    return(-1);
  }                       
  
  PMU_auto_init("Indicator", P.Instance,
                PROCMAP_REGISTER_INTERVAL);     

  if ((startTime != 0) || (endTime != 0)){
    P.Mode = Params::ARCHIVE;
  }

  //
  // Check the validity of the input parameters.
  //
  if (P.ForecastUrls_n != P.TruthUrls_n){
    cerr << "Specified " << P.ForecastUrls_n;
    cerr << " forecast URLs but only " << P.TruthUrls_n;
    cerr << " truth URLs, which is a fatal error." << endl;
    return -1;
  }

  int NumUrls = P.TruthUrls_n;
  if (NumUrls > MaxUrls){
    cerr << "Too many URLs defined." << endl;
    return -1;
  }


  //
  // Set up the truth and forecast MDV time arrays.
  // Depends on what mode we're in.
  //
  if (P.Mode == Params::ARCHIVE){

    for (int i=0; i < NumUrls; i++){
      //  TruthMdv[i] = new DsMdvxTimes();       

      if (TruthMdv[i].setArchive(P._TruthUrls[i],
				  startTime,
				  endTime)){
	cerr << "Failed to set URL " << P._TruthUrls[i] << endl;
	return -1;
      }

      //
      // The following is OK for now, but
      // later we may have to add the forecast lead time
      // to the start and end times in this call.
      //
      //  ForecastMdv[i] = new DsMdvxTimes();       
      if (ForecastMdv[i].setArchive(P._ForecastUrls[i],
				     startTime,
				     endTime)){       
	cerr << "Failed to set URL " << P._ForecastUrls[i] << endl;
	return -1;
      }
    }

  } else { // REALTIME mode

    for (int i=0; i < NumUrls; i++){

      /*
      ForecastMdv[i] = new DsMdvxTimes();  
      TruthMdv[i]    = new DsMdvxTimes();  
      */

      if (TruthMdv[i].setRealtime(P._TruthUrls[i],
				  P.MaxRealtimeValidAge,
				  PMU_auto_register,
				  -1)){
	cerr << "Failed to set URL " << P._TruthUrls[i] << endl;
	return -1;
      }

      if (ForecastMdv[i].setRealtime(P._ForecastUrls[i],
				     P.MaxRealtimeValidAge,
				     PMU_auto_register,
				     -1)){
	cerr << "Failed to set URL " << P._ForecastUrls[i] << endl;
	return -1;
      }

    } // End of loop through URLs

  } // End of If we are in realtime.

  //
  // OK - array of MDV for truth and forecasts all set.
  // Now, use them.
  //

  int NumDuds = 0; 
  //
  // A dud being getting NULL on both Truth and Forecast,
  // indicating that that URL is empty in archive mode.
  // Once NumDuds == NumUrls in ARCHIVE mode, we're done.
  //
  do{

    for (int i=0; i < NumUrls; i++){

      time_t TruthTime, ForecastTime;
      
      TruthMdv[i].getNext( TruthTime );
      ForecastMdv[i].getNext( ForecastTime );

      if ((TruthTime == (time_t)NULL) && (ForecastTime == (time_t)NULL)){
	NumDuds++;
      } else {
	NumDuds = 0;
      }

      if (!((TruthTime == (time_t)NULL))){
	//
	// Have new input here.
	//

	if (P.Debug){
	  cerr << "Truth data at " << utimstr(TruthTime) << endl;
	}
	Reader Rt(TruthTime, &P, i, true);
	Rt.Process(&P);
      }
      if (!((ForecastTime == (time_t)NULL))){
	//
	// Have new input here.
	//

	if (P.Debug){
	  cerr << "Forecast data at " << utimstr(ForecastTime) << endl;
	}
	Reader Rf(ForecastTime, &P, i, false);
	Rf.Process(&P);
      }

    }

    sleep(10);
  } while ((P.Mode == Params::REALTIME) || (NumDuds < NumUrls));


  /*
  for (int i=0; i < MaxUrls; i++){
    delete TruthMdv[i]; delete ForecastMdv[i];
    TruthMdv[i] = NULL; ForecastMdv[i] = NULL;
  }
  */

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
      cout << "USAGE : Indicator [-print_params to get parameters]" << endl;
      cout << "[-h or -- or -? to get this help message]" << endl;
      cout << "[-interval YYYYMMDDhhmmss YYYYMMDDhhmmss to run in archive mode]" << endl;
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

     






