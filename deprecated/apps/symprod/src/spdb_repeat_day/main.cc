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

#include <iostream>

#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <symprod/spdb_client.h> 
#include "Params.hh"

static void tidy_and_exit (int sig);

main(int argc, char *argv[])
{


  // Load up from param file.
  Params *P = new Params();

  if (P->loadFromArgs(argc,argv,NULL,NULL)){
    cerr << "Specify params file with -params option." << endl;
    exit(-1);
  }

  // Trap.
  PORTsignal(SIGINT, tidy_and_exit);
  PORTsignal(SIGHUP, tidy_and_exit);
  PORTsignal(SIGTERM, tidy_and_exit);
     

  PMU_auto_init("spdb_repeat_day",
		P->instance,PROCMAP_REGISTER_INTERVAL);



  date_time_t StartTim, EndTim;

  StartTim.year = P->year; StartTim.month = P->month; StartTim.day = P->day;
  StartTim.hour = 0; StartTim.min = 0; StartTim.sec = 0;
  uconvert_to_utime( &StartTim );

  EndTim.year = P->year; EndTim.month = P->month; EndTim.day = P->day;
  EndTim.hour = 23; EndTim.min = 59; EndTim.sec = 59;
  uconvert_to_utime( &EndTim );

  time_t DataEnd = 0;
  time_t DataStart = 0;

  do {

    //
    // Get the current time.
    //
    date_time_t Tim;
    date_time_t BackedTim;

    ugmtime( &Tim );
    int LastDay = Tim.day;

    do {

      PMU_auto_register("Waiting");
      sleep(1);
      ugmtime( &Tim );
      //
      // If the day has rolled over, then reset.
      //
      if (Tim.day != LastDay){
	DataEnd=0;
	DataStart=0;
      }

      //
      // Make a copy of the time and back
      // it up to the day in question.
      //

      BackedTim = Tim;
      BackedTim.year = P->year; 
      BackedTim.month = P->month; 
      BackedTim.day = P->day;
      uconvert_to_utime( &BackedTim );

    } while ( BackedTim.unix_time < DataEnd + P->interval);

    PMU_auto_register("Copying");


    if (DataEnd == 0){ // Data start and end unititialised, initialize.
      DataStart = StartTim.unix_time;
      DataEnd = DataStart + P->interval;
    } else { // Data start and end are already initialized, use them.
      DataStart = DataEnd + 1;
      DataEnd = DataStart + P->interval;
    }

    if (DataStart < StartTim.unix_time) 
      DataStart = StartTim.unix_time;

    int ResetTimeAfterThis=0;

    if (DataEnd > EndTim.unix_time){
      DataEnd = EndTim.unix_time;
      ResetTimeAfterThis=1;
    }

    if (P->debug) {

      cout << "Source start : " << utimstr(DataStart) << endl;
      cout << "End : "   << utimstr(DataEnd) << endl;
      cout << "Local : ";
      cout << Tim.year << "/" << Tim.month << "/" << Tim.day << " ";
      cout << Tim.hour << ":" << Tim.min   << ":" << Tim.sec;
      cout << endl << endl << flush;
 
    }


    //
    // Do the copy.
    //
    ui32 n_chunks, prod_id;
    spdb_chunk_ref_t *chunk_refs;
    void *chunks;

    //
    // Read the data.
    //
    if (SPDB_get_interval(P->source, 0,
			  0, DataStart, DataEnd,
			  &n_chunks, &chunk_refs, &chunks)) {

      cerr << "ERROR : Failed to read from " << P->source << endl;
      cerr << "For times : " << endl;
      cerr << "Source start : " << utimstr(DataStart) << "\t";
      cerr << "End : "   << utimstr(DataEnd)   << endl << flush;
 
      continue;
    }
    if (n_chunks ==0) continue; // Nothing found.

    //
    // Muck with the times.
    // 
    unsigned long total_len = 0;
    for (int i=0; i < n_chunks; i++){
      date_time_t d;
      d.unix_time = chunk_refs[i].valid_time;

      uconvert_from_utime( &d );
      d.year = Tim.year; d.month = Tim.month; d.day = Tim.day;
      uconvert_to_utime( &d );
      chunk_refs[i].valid_time = d.unix_time;

      d.unix_time = chunk_refs[i].expire_time;

      uconvert_from_utime( &d );
      d.year = Tim.year; d.month = Tim.month; d.day = Tim.day;
      uconvert_to_utime( &d );
      chunk_refs[i].expire_time = d.unix_time;

      total_len =+ chunk_refs[i].len;
    }

    prod_id  =  chunk_refs[0].prod_id;

    if (P->debug) cout << "Product ID : " << prod_id << endl << flush;

    //
    // Write it out, with modified time.
    //
    if (SPDB_put_over(P->destination,
		      prod_id,"",
		      n_chunks, chunk_refs,
		      chunks, total_len)) {
      
      cerr << "Failed to write to " << P->destination << endl;
      cerr << "For times : " << endl;
      cerr << "Source start : " << utimstr(DataStart) << endl;
      cerr << "End : "   << utimstr(DataEnd) << endl;
      cerr << " Local : ";
      cerr << Tim.year << "/" << Tim.month << "/" << Tim.day << " ";
      cerr << Tim.hour << ":" << Tim.min   << ":" << Tim.sec << endl << flush;
		      
    }

   if (ResetTimeAfterThis){
     //
     // Gone past the end of the day - need
     // to re-init times.
     //
     DataEnd = 0;
     DataStart = 0;
   }

  } while (1);


}


static void tidy_and_exit (int sig){
  PMU_auto_unregister();
  exit(sig);
}
 




