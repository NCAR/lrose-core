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

#include <Mdv/DsMdvxTimes.hh>
#include <toolsa/umisc.h>
#include <cstdlib>

#include "Params.hh"

using namespace std;
//
/////////////////////////////////////////////////////
//
int main( int argc, char **argv )
{

  //
  // Get TDRP args. Don't check in to PMU.
  //
  Params P;
  if (P.loadFromArgs(argc,argv,NULL,NULL)){
    cerr << "Specify params file with -params option." << endl ;
    return(-1);
  }                       

  //
  // Parse the start and end times.
  //
  date_time_t start, end;

  if (6 != sscanf(P.startTime, "%d/%d/%d_%d:%d:%d",
		  &start.year, &start.month, &start.day,
		  &start.hour, &start.min, &start.sec)){
    cerr << "unable to parse start time from " << P.startTime << endl;
    exit(-1);
  }

  if (6 != sscanf(P.endTime, "%d/%d/%d_%d:%d:%d",
		  &end.year, &end.month, &end.day,
		  &end.hour, &end.min, &end.sec)){
    cerr << "unable to parse end time from " << P.endTime << endl;
    exit(-1);
  }

  uconvert_to_utime ( &start ); uconvert_to_utime ( &end );

  //
  // Get the list of input times.
  //

  DsMdvxTimes      InMdv;

  if (InMdv.setArchive(P.TriggerUrl,
		       start.unix_time,
		       end.unix_time)){
    cerr << "Failed to set URL " << P.TriggerUrl << endl;
    return -1;
  }

  //
  // Open output file, loop through times.
  //
  FILE *ofp = fopen(P.Outfile, "w");
  if (ofp == NULL){
    cerr << "Failed to create " << P.Outfile << endl;
    exit(-1);
  }

  fprintf(ofp,"#!/bin/csh -f\n");
  fprintf(ofp,"#\n# This file created by ciddArchiveDriver\n# by Niles Oien\n#\n");

  fprintf(ofp,"# Start : %04d/%02d/%02d %02d:%02d:%02d UTC\n",
	  start.year, start.month, start.day,
	  start.hour, start.min, start.sec);

  fprintf(ofp,"# End : %04d/%02d/%02d %02d:%02d:%02d UTC\n",
	  end.year, end.month, end.day,
	  end.hour, end.min, end.sec);
  

  fprintf(ofp,"# File created : %s\n",
	  utimstr(time(NULL)));

  fprintf(ofp,"# URL : %s\n#\n", P.TriggerUrl );


  if (P.startEndStrings.includeStartEndStrings){
    fprintf(ofp,"%s\n", P.startEndStrings.startString);
  }


  do {

    time_t Time;
    
    InMdv.getNext( Time );
    
    if (Time == (time_t)NULL){
      break; // Reached end of the list.
    }
    
    date_time_t dataTime;
    dataTime.unix_time = Time;
    uconvert_from_utime( &dataTime );

    fprintf(ofp,"CIDD %s -t %04d%02d%02d%02d%02d%02d\n", P.ciddParams,
	    dataTime.year, dataTime.month, dataTime.day,
	    dataTime.hour, dataTime.min, dataTime.sec);

  } while (1);

  if (P.startEndStrings.includeStartEndStrings){
    fprintf(ofp,"%s\n", P.startEndStrings.endString);
  }


  fprintf(ofp,"#\n");
  fclose(ofp);

  char com[1024];
  sprintf(com,"chmod 777 %s", P.Outfile);
  system (com);

  return 0;

}

