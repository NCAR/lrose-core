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
///////////////////////////////////////////////
//
// Program to find the latest available input data
// given an MDV URL. Niles Oien late July 2001.
//

#include <Mdv/DsMdvxTimes.hh>
#include <ctime>
#include <cstdio>
#include <cstring>
#include <ctype.h> 
#include <toolsa/umisc.h>

int main(int argc, char *argv[]){

  if ((argc < 2) || (!(strcmp(argv[1],"-h")))){
    fprintf(stderr,"USAGE : MdvGetLatestTime url <lookbackSeconds> (default is 86400 second lookback)\n");
    fprintf(stderr,"        Program prints 1 Jan 1970 if URL is not resolved.\n");
    exit(-1);
  }

  char *inUrl = argv[1];

  char url[1024];
  sprintf(url, "%s", inUrl);


  //
  // If the port number, 5440, is specified in the URL, this causes issues.
  // We need to strip that out.
  //
  char *p = strstr(url, "5440");
  if (p != NULL){
    sprintf(p, "%s", p+4);
  }

  // fprintf(stderr, "Using URL %s\n", url);

  int lookBack = 86400;
  if (argc > 2){
    lookBack = atoi(argv[2]);
  }

  //
  DsMdvxTimes D;
  
  D.setRealtime(url, lookBack, NULL, -1);
  //
  // And get the time.
  //
  time_t unixtime = 0L;
  if (D.getNext( unixtime )) {
    unixtime = 0L;
  }
  //
  //
  if (!(D.getNextSuccess())) {
    unixtime = 0L;
  }
  //
  // And convert from unix time.
  //  
  date_time_t T;
  T.unix_time = unixtime;

  uconvert_from_utime( &T );
 
  fprintf(stdout,"%04d%02d%02d%02d%02d%02d\n",
	  T.year, T.month, T.day, T.hour, T.min, T.sec);

  return 0;

}






