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


#include <toolsa/DateTime.hh>
#include <cstdio>
#include <ctime>
#include <toolsa/umisc.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>
#include <vector>

#include "Params.hh"
#include "FileHandler.hh"
#include "FileLister.hh"
#include "Statistician.hh"
#include "Map.hh"
#include "MapFileHandler.hh"
using namespace std;

////////////////////////////////////////////
//
// Constructor
//
FileLister::FileLister(){
  startTime = DateTime::NEVER;
  endTime = DateTime::NEVER;
}

///////////////////////////////////////////////////
//
// Initialization
//
int FileLister::init( int argc, char **argv )
{


  //
  // Grab TDRP parameters.
  //
  P = new Params();

  if (P->loadFromArgs(argc,argv,NULL,NULL)){
    fprintf(stderr,"Specify params file with -params option.\n");
    usage();
    return(-1);
  } 

  if ((strlen(P->TruthDir)==0) && (strlen(P->ForecastDir)==0)){
    fprintf(stderr,
       "Specify the truth and forecast directories in the parameter file.\n");
    usage();
    return -1;
  }

  //
  // Now that we know the instance, check in.
  //
  PMU_auto_init("Validator",P->Instance,
		PROCMAP_REGISTER_INTERVAL);

  //
  // Process the command line arguments - just
  // the start and end times.
  //
  for(int i=1; i<argc; i++){
    date_time_t T;

    if ( !strcmp(argv[i], "--" ) ||
	 !strcmp(argv[i], "-h" ) ||
	 !strcmp(argv[i], "-help" ) ||
	 !strcmp(argv[i], "-man" )) {
      usage();
      return -1;
    }
   
    if (!(strcmp(argv[i],"-start"))){
      i++;
      if (6!=sscanf(argv[i],"%d %d %d %d %d %d",
		   &T.year, &T.month, &T.day,
		   &T.hour, &T.min, &T.sec)){
	fprintf(stderr,"Start time not specified correctly.\n");
	usage();
	return -1;
      }
      uconvert_to_utime(&T);
      startTime = T.unix_time;
    }

    if (!(strcmp(argv[i],"-end"))){
      i++;
      if (6!=sscanf(argv[i],"%d %d %d %d %d %d",
		   &T.year, &T.month, &T.day,
		   &T.hour, &T.min, &T.sec)){
	fprintf(stderr,"End time not specified correctly.\n");
	usage();
	return -1;
      }
      uconvert_to_utime(&T);
      endTime = T.unix_time;
    }

    if (!(strcmp(argv[i],"-yesterday"))){

      // Get the time now.
      date_time_t Now;
      ugmtime(&Now);

      // Get the start and end of today
      date_time_t Begin,End;
      Begin=Now; End=Now;
      Begin.hour = 0; Begin.min = 0; Begin.sec=0;
      uconvert_to_utime(&Begin);

      End.unix_time = Begin.unix_time + 86400;
      uconvert_from_utime(&End);

      // Back up a day.
      const long SecsPerDay = 86400;

      Begin.unix_time = Begin.unix_time - SecsPerDay;
      End.unix_time = End.unix_time - SecsPerDay;

      // Convert back - not really necessary
      // since I just want a utime, but I should
      // be careful with variable names like these.
      //
      uconvert_from_utime(&Begin);
      uconvert_from_utime(&End);

      startTime=Begin.unix_time; endTime=End.unix_time;

      fprintf(stderr,"%s -> %s\n",
	      utimstr(startTime),
	      utimstr(endTime));


    }

  }

  if ((startTime == DateTime::NEVER) || (endTime == DateTime::NEVER)){
    fprintf(stderr,"\nSpecify start and end times on the command line.\n\n");
    usage();
    return -1; 
  }
  


  //
  // Set up the input.
  //

  input = new DsMdvxTimes();

  input->setArchive(P->ForecastDir,
		   startTime - P->ForecastLead,
		   endTime - P->ForecastLead );
	

  return 0;

}

//////////////////////////////////////////////////////////
void  FileLister::usage()
{

  fprintf(stderr,
	  "USAGE : Validator -start \"YYYY MM DD hh mm ss\"\n"
	  "-end \"YYYY MM DD hh mm ss\" -params paramfile\n\n"
	  );
}

///////////////////////////////////////////////////////////
//
// Execution 
//
int  FileLister::run()
{

  if (P->Debug){
    fprintf(stderr,"Truth directory : %s\n",P->TruthDir);
    fprintf(stderr,"Forecast directory : %s\n",P->ForecastDir);
    fprintf(stderr,"Truth start : %s",asctime(gmtime(&startTime)));
    fprintf(stderr,"Truth end : %s\n",asctime(gmtime(&endTime)));
    fprintf(stderr,"Map mode : ");
    if (P->MapMode) fprintf(stderr,"ON\n");
    else fprintf(stderr,"OFF\n");
  }


  if (P->MapMode){

    vector< Map* > MapVec;
    vector< Statistician* > StatsVec;
    //
    // Init a map and a statistician for each
    // map specified.
    //
    for (int i=0; i < P->MapList_n; i++){
      MapVec.push_back( new Map() );
    }

    vector< Map* >::iterator im;
    int j=0;
    for( im=MapVec.begin(); im != MapVec.end(); im++ ) {
      
      if(P->Debug) fprintf(stderr,"Looking at %s\n",
			   P->_MapList[j]);
      
      if ( (*im)->Init(P->_MapList[j],P)){
	fprintf(stderr,"Something wrong in %s\n",P->_MapList[j]);
	exit(-1);
      }
      StatsVec.push_back( new Statistician( (*im)->Name ));
      j++;
    }

    vector< Statistician* >::iterator is;
    for( is=StatsVec.begin(); is != StatsVec.end(); is++ )
      (*is)->Init(startTime,P);
    //
    // Maps and stats OK - now process the forecasts.
    //
    time_t ForecastTime;
    int num=0,proc=0;

    while(!(input->getNext(ForecastTime))){
      num++;
      //
      // Get the truth file.
      //
      fprintf(stderr,"\nForecast pair [%d] at %s\n",num,utimstr(ForecastTime));

      MapFileHandler *MF = new MapFileHandler();

      if (MF->ProcessPair(ForecastTime,
			  ForecastTime + P->ForecastLead, P,
			  MapVec, StatsVec)){
	fprintf(stderr,"Processing failed on this pair.\n\n");
      } else {
	proc++;
      }

      delete MF;
    }

    for( im=MapVec.begin(); im != MapVec.end(); im++ )  delete *im;
    for( is=StatsVec.begin(); is != StatsVec.end(); is++ )  delete *is;

    fprintf (stderr,"%d forecasts processed.\n",proc);

  } else { // No maps, do the whole grid.

    //
    // Process each input forecast file
    //
    time_t ForecastTime;
    int num=0,proc=0;
    Statistician *S = new Statistician("Grid");
    S->Init(startTime,P);

    while(!(input->getNext(ForecastTime))){
      
      num++;
      //
      // Get the truth file.
      //

      fprintf(stderr,"\nForecast pair [%d] at %s\n",num,
	      utimstr(ForecastTime));

      FileHandler *F = new FileHandler();

      if (F->ProcessPair(ForecastTime,
			 ForecastTime + P->ForecastLead, P)){
	fprintf(stderr,"Processing failed on this pair.\n\n");
      } else {
	S->Accumulate(F->num_non, F->num_fail,
		      F->num_false, F->num_success,
		      ForecastTime + P->ForecastLead, P);
     
	if ( P->Debug){

	  fprintf(stderr,"Non-events : %ld\n",F->num_non);
	  fprintf(stderr,"Failures : %ld\n",F->num_fail);
	  fprintf(stderr,"False alarms : %ld\n",F->num_false);
	  fprintf(stderr,"Successes : %ld\n\n",F->num_success);
	}
      }
      delete F;
      proc++;
    }

    delete S;

    fprintf (stderr,"%d forecasts processed.\n",proc);

  }

  return 0;

}

