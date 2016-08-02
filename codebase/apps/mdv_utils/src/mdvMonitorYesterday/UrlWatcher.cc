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

  //
  // Parse command line args. Pretty minimal for this program.
  //
  if (ParseArgs(argc,argv)) return -1;
  //
  // Get TDRP args. Don't check in to PMU.
  //
  
  if (P.loadFromArgs(argc,argv,NULL,NULL)){
    cerr << "Specify params file with -params option." << endl ;
    return(-1);
  }                       
  
  //
  // Set up for input.
  //

  endTime = time(NULL) + P.timeOffset;
  startTime = endTime - 86400; //  Number of seconds in a day

  if (InMdv.setArchive(P.url,
		       startTime,
		       endTime)){
    cerr << "Failed to set URL " << P.url << endl;
    return -1;
  }


  //
  // Input Mdv object should now be all set. Use it in the run function.
  //
  return 0;

}

int UrlWatcher::run(){

  vector <time_t> badTimes;
  vector <double> badPercents;
  int num = 0;

  do{

    time_t Time;
    
    InMdv.getNext( Time );
    
    if (Time == (time_t)NULL){
      break;
    }

    double pb;
    if (Time != (time_t)NULL){
      Process S;
      pb = S.Derive(&P, Time);
      if (pb > P.percentBadThreshold){
	badTimes.push_back( Time );
	badPercents.push_back( pb );
      }
      num++;
    }

  } while (1);

  if ((badTimes.size() ==0) && (!(P.sendEmailIfAllWell)) && (num > 0)){
    return 0; // Don't send email in this case.
  }
  
  FILE *fp = fopen(P.tmpFile, "w");
  if (fp == NULL){
    cerr << "Failed to open " << P.tmpFile << endl;
    return -1;
  }

  fprintf(fp,"\nSummary of check of mdv url :\n%s\nfrom %s to %s UTC\n\n",
	  P.url, utimstr(startTime), utimstr(endTime));
  fprintf(fp, "Number of volumes checked : %d\n", num);
  fprintf(fp, "Number of volumes for which field %s had more than %g percent bad : %d\n\n",
	  P.fieldName, P.percentBadThreshold, badTimes.size());

  for (unsigned j=0; j < badTimes.size(); j++){
    fprintf(fp, "Volume at %s was %g percent bad\n", utimstr(badTimes[j]), badPercents[j]);
  }

  fprintf(fp, "\n\nThis was sent to :\n");
  for (int j=0; j < P.subscribers_n; j++){
    fprintf(fp, "%s\n", P._subscribers[j]);
  }

  fclose(fp);

  for (int j=0; j < P.subscribers_n; j++){
    char com[1024];
    sprintf(com,"cat %s | mail -s \"%s\" %s",
	    P.tmpFile, P.subject, P._subscribers[j]);
    system(com);
  }

  unlink(P.tmpFile);

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
      cout << "USAGE : mdvMonitorYesterday [-print_params to get parameters]" << endl;
      cout << "[-h or -- or -? to get this help message]" << endl;
      return -1;

    }
    
  }

  return 0; 
  
}

     






