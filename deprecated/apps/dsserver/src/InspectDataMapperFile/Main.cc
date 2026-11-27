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
#include <dsserver/DmapMessage.hh>
//
// Use cstdio throughout - used for file I/O so
// might as well use it for console. Niles Oien.
//
#include <cstdio>

char *printTime(time_t T);

int main(int argc, char *argv[]){

  //
  // Either use default filename or command line
  // specified one.
  //
  char inFile[1024];
  if (argc < 2){
    sprintf(inFile, "%s", "/tmp/_dmap_table");
  } else {
    sprintf(inFile,"%s", argv[1]);
  }

  FILE *ifp = fopen(inFile,"r");
  if (ifp == NULL){
    fprintf(stderr, "Unable to open %s\n", inFile);
    fprintf(stderr, "Check data mapper param file and specify filename on command line.\n");
    return -1;
  }

  DMAP_info_t info;

  int num = 1;

  //
  // Loop through, reading DMAP_info_t structs and printing them.
  // Break out of loop when read fials.
  //
  do {

    int numRead = fread(&info, sizeof(unsigned char), sizeof(info), ifp);
    if (numRead != sizeof(info)){
      break;
    }


    fprintf(stderr, "Dataset %d :\n", num); num++;
    fprintf(stderr, "Start time : %s\n", printTime(info.start_time));
    fprintf(stderr, "End time : %s\n", printTime(info.end_time));
    fprintf(stderr, "Latest time : %s\n", printTime(info.latest_time));
    fprintf(stderr, "Last registration : %s\n", printTime(info.last_reg_time));

    fprintf(stderr, "Files : %g\n", info.nfiles);
    fprintf(stderr, "Bytes : %g\n", info.total_bytes);
    fprintf(stderr, "Forecast : ");
    if (info.forecast_lead_time < 0){
      fprintf(stderr, "not set.\n");
    } else {
       fprintf(stderr, "%g\n", info.forecast_lead_time);
    }

    fprintf(stderr, "DataMapper check time : %s\n", printTime(info.check_time));

    fprintf(stderr, "Host : %s\n", info.hostname);
    fprintf(stderr, "IP : %s\n", info.ipaddr);
    fprintf(stderr, "Datatype : %s\n", info.datatype);
    fprintf(stderr, "Dir : %s\n", info.dir);
    fprintf(stderr, "Status : %s\n", info.status);

    fprintf(stderr, "\n");

  } while( 1 );

  fclose(ifp);

  return 0;

}

//
// Small method to print time string or "not set" if time is 0L.
//
char *printTime(time_t T){

  if (T == 0L){
    return "not set";
  }

  return utimstr(T);

}
