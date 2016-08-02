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


#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <ctime>

#include <Spdb/DsSpdb.hh>



/*****

Very simple test of putting something to the Spdb server.

g++ -D_BSD_TYPES -DF_UNDERSCORE2 -g -c  -I/home/steves/tech/taiwan/sigwx2symprod/rapinstall/include -I/home/steves/tech/taiwan/sigwx2symprod/rapbase/include   createSpdb.cc


g++ -g -o createSpdb createSpdb.o -L/home/steves/tech/taiwan/sigwx2symprod/rapinstall/lib -L/home/steves/tech/taiwan/sigwx2symprod/rapbase/lib   -lSpdb -ldsserver -ldidss -lrapformats -ltoolsa -ldataport -ltdrp -lpthread -lm


./createSpdb -o tempdir

*****/



void badparms( char * msg) {
  printf("\nError: %s\n", msg);
  printf("Parms:\n");
  printf("  -o outDir       output file\n");
  exit(1);
}



int main( int argc, char *argv[]) {
  int irc, ii;

  char * outDir = NULL;
  if (argc % 2 != 1) badparms("parms must be pairs:  -key value");
  for (int ii = 1; ii < argc; ii += 2) {    // skip command name
    char * key = argv[ii];
    char * val = argv[ii+1];
    if (0 == strcmp( key, "-o")) outDir = val;
    else badparms("unknown parm");
  }
  if (outDir == NULL) badparms("outDir not specified");
  printf("outDir: \"%s\"\n", outDir);

  DsSpdb spdb;

  spdb.clearPutChunks();

  int buflen = 1000;
  char * buft = new char[buflen];
  memset( buft, 0, buflen);

  char * bufmsg = new char[buflen];
  memset( bufmsg, 0, buflen);

  int dataType = 777777;
  time_t curTime = time( NULL);         // seconds since 1970
  struct tm tmrec;
  gmtime_r(&curTime, &tmrec);
  strftime( buft, buflen, "%Y-%m-%d %H:%M:%S", &tmrec);
  printf("curTime: %d  %s\n", curTime, buft);

  // Start at the beginning of the current day
  int secsPerHour = 3600;
  int secsPerDay = 24 * secsPerHour;
  time_t curDay = (curTime / secsPerDay) * secsPerDay;

  // Back up, go forward a few days
  time_t startTime = curDay - 5 * secsPerDay;
  time_t endTime = curDay + 5 * secsPerDay;

  for (time_t itime = startTime; itime < endTime; itime += secsPerHour) {
    gmtime_r(&itime, &tmrec);
    strftime( buft, buflen, "%Y-%m-%d %H:%M:%S", &tmrec);
    printf("  adding time: %d  %s\n", itime, buft);
    spdb.addPutChunk(
      dataType,
      itime,                  // start time
      itime,                  // end time
      buflen,
      bufmsg);
    
    irc = spdb.put( outDir, SPDB_WAFS_SIGWX_ID, SPDB_WAFS_SIGWX_LABEL);
    if (irc != 0) {
      printf("spdb put error: %s\n", spdb.getErrStr().c_str());
      badparms("spdb put error");
    }
  } // for itime

} // end main
