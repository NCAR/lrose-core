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

#include <Spdb/DsSpdb.hh>
#include <toolsa/umisc.h>
#include <iostream>
#include <rapformats/bdry.h> 
#include <toolsa/pmu.h>

#include "Bdry2Axf.hh"
#include "Params.hh"
#include "Trigger.hh"
using namespace std;

static void tidy_and_exit (int sig);  
void Usage(int sig);
void ProcessInterval(time_t Start, time_t End, Params *P);

int main(int argc, char *argv[]){

  //
  // Trap signals.
  PORTsignal(SIGINT, tidy_and_exit);
  PORTsignal(SIGHUP, tidy_and_exit);
  PORTsignal(SIGTERM, tidy_and_exit);

  // Grab params.
  Params *P = new Params();

  if (P->loadFromArgs(argc,argv,NULL,NULL)){
    fprintf(stderr,"Specify params file with -params option.\n");
    exit(-1);
  }

  // Check in
  PMU_auto_init("SpdbBdry2Axf",P->Instance,PROCMAP_REGISTER_INTERVAL);    

  date_time_t Start, End;
  bool archiveMode = false;

  //
  // Parse the command line.
  //

  for (int i=1; i < argc; i++){

    if ((!(strcmp(argv[i],"-h"))) ||
	(!(strcmp(argv[i],"--"))) ||
	(!(strcmp(argv[i],"-?"))) ||
	(!(strcmp(argv[i],"-help")))
	){
      Usage(0);
    }



    if (!(strcmp(argv[i],"-archive"))){
      archiveMode = true;

      if (argc <= i + 2) Usage(-1); // Make sure we have enough arguments.


      if (6 != sscanf(argv[i+1],"%4d%2d%2d%2d%2d%2d",
                      &Start.year, &Start.month, &Start.day,
                      &Start.hour, &Start.min,&Start.sec)){


        Usage(-1);

      }                      
      uconvert_to_utime( &Start );

      if (6 != sscanf(argv[i+2],"%4d%2d%2d%2d%2d%2d",
                      &End.year, &End.month, &End.day,
                      &End.hour, &End.min,&End.sec)){
        Usage(-1);
      }

      uconvert_to_utime( &End );
      
    }
  }                   

  if (!(archiveMode)){
    RealtimeTimeTrigger Trig("SpdbBdry2Axf", P);
    time_t triggerTime;
    
    while ((triggerTime = Trig.next()) >= 0) {
      cerr << "Processing for " <<  utimstr(triggerTime) << endl;
      PMU_auto_register(utimstr(triggerTime));
      ProcessInterval(triggerTime - P->LookBackTime,
		      triggerTime, P);
    }
  } else { // Archive mode.
    ProcessInterval(Start.unix_time, End.unix_time, P);
  }

  tidy_and_exit(0);

}


//////////////////////////////////////////////////////////////////////
// tidy up on exit

static void tidy_and_exit (int sig)

{
  PMU_auto_unregister();

  cerr << "done." << endl;
  exit(sig);

}

///////////////////////////////////////////////
/////////////////////////////////////////////// 
          
void Usage(int sig){

  cerr << "Usage : " << endl;
  cerr << "SpdbBdry2Axf -params paramfile [-archive YYYYMMDDhhmmss YYYYMMDDhhmmss]" << endl; 
  exit(sig);

}


/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////

void ProcessInterval(time_t Start, time_t End, Params *P){

  DsSpdb S;

  if (S.getInterval( P->InURL, Start, End) != 0 ){
    cerr << "SPDB get failed" << endl;
    exit(-1);
  } 
    
  BDRY_spdb_product_t *bdrySpdbProduct;
  BDRY_product_t      *bdryProduct = NULL;       

  Spdb::chunk_ref_t  *bdryProductHdrs;
  char               *bdryProductBuffer;             

  bdryProductHdrs   = S.getChunkRefs();
  bdryProductBuffer = (char *)S.getChunkData();      
 
  //
  // If we have boundaries, process them.
  //
  //


  int j=0;

  for (int m=0; m < S.getNChunks(); m++){
    bdrySpdbProduct = (BDRY_spdb_product_t *)
      (bdryProductBuffer + bdryProductHdrs[m].offset);
    //
    // byte swapping
    //
    BDRY_spdb_product_from_BE( bdrySpdbProduct );        
  }

  while (j < S.getNChunks()){

    bdrySpdbProduct = (BDRY_spdb_product_t *)
      (bdryProductBuffer + bdryProductHdrs[j].offset);
    //
    // convert to BDRY_product_t
    //
    bdryProduct = BDRY_spdb_to_product(bdrySpdbProduct);
    //
    // Go through and pick off all the chunks that apply to this
    // time - store the indicies in IndexStart and IndexEnd.
    //
    int IndexStart, IndexEnd; // First and last indicies at this time.
    time_t ThisTime;

    ThisTime = bdryProduct->data_time;

    IndexStart = j;
    IndexEnd = -1;

    BDRY_free_product(bdryProduct);

    do {
      j++;
      if (j >= S.getNChunks()){
	IndexEnd = S.getNChunks()-1;
	continue;
      }
      bdrySpdbProduct = (BDRY_spdb_product_t *)
	(bdryProductBuffer + bdryProductHdrs[j].offset);

      bdryProduct = BDRY_spdb_to_product(bdrySpdbProduct);

      if (bdryProduct->data_time != ThisTime){
	if (j==IndexStart){
	  IndexEnd =IndexStart;
	} else {
	  IndexEnd = j-1;
	}
      }
      BDRY_free_product(bdryProduct);
    }while(IndexEnd==-1);



    int NumAtThisTime = IndexEnd-IndexStart+1;

    int *LeadTimes = (int *)malloc(sizeof(int)*NumAtThisTime);

    cerr << "Time : " << utimstr( ThisTime ) << " has ";
    cerr << NumAtThisTime << " boundaries." << endl;

    int u=0;
    for(int k=IndexStart; k<= IndexEnd; k++){

      bdrySpdbProduct = (BDRY_spdb_product_t *)
	(bdryProductBuffer + bdryProductHdrs[k].offset);

      bdryProduct = BDRY_spdb_to_product(bdrySpdbProduct);
 
      for (int h=0; h < bdryProduct->num_polylines; h++){
 
	LeadTimes[u] = bdryProduct->polylines[h].num_secs;
      	cerr << "\tLead time : " << LeadTimes[u] << endl;
	u++;

      }
      BDRY_free_product(bdryProduct);
    }


    Bdry2Axf B(P,ThisTime,NumAtThisTime,LeadTimes);

    int w=0;
    for(int k=IndexStart; k<= IndexEnd; k++){

      bdrySpdbProduct = (BDRY_spdb_product_t *)
	(bdryProductBuffer + bdryProductHdrs[k].offset);

      bdryProduct = BDRY_spdb_to_product(bdrySpdbProduct);

      B.Add( *bdryProduct,  bdrySpdbProduct->line_type, 
	     LeadTimes[w]);
      w++;

      BDRY_free_product(bdryProduct);


    }


    B.Close();
    free(LeadTimes);

  }

  //
  // If we had no boundaries, output a file that
  // holds only the header. This sort-of works in archive
  // mode - it is really meant for realtime operation.
  //
  if (S.getNChunks()==0){
    int LeadTim[1];
    LeadTim[0]=0; // Won't get looked at anyway.
    Bdry2Axf B(P,End,0,LeadTim);
    B.Close();
  }


}


















