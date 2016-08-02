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
//
// Print some stats about SPDB latency times.
//

#include <toolsa/umisc.h>
#include <Spdb/DsSpdb.hh>
#include <cstdio>
#include <cmath>

int main(int argc, char *argv[]){

  if (argc != 4){
    fprintf(stderr,"USAGE : %s url YYYYMMDDhhmmss YYYYMMDDhhmmss\n", argv[0]);
    return -1;
  }

  date_time_t s;
  if (6!=sscanf(argv[2],"%4d%2d%2d%2d%2d%2d",
		&s.year, &s.month, &s.day,
		&s.hour, &s.min, &s.sec)){
    fprintf(stderr,"Failed to parse start time from %s\n", argv[2]);
    return -1;
  }


  date_time_t e;
  if (6!=sscanf(argv[3],"%4d%2d%2d%2d%2d%2d",
		&e.year, &e.month, &e.day,
		&e.hour, &e.min, &e.sec)){
    fprintf(stderr,"Failed to parse end time from %s\n", argv[3]);
    return -1;
  }

  uconvert_to_utime( &s ); uconvert_to_utime( &e );

   DsSpdb data;
   if (data.getInterval(argv[1], s.unix_time, e.unix_time)){
     fprintf(stderr, "getInterval failed from %s\n", argv[1]);
     return -1;
   }
   
   if (data.getNChunks() == 0){
     fprintf(stderr,"No chunks found.\n");
     return -1;
   }

   double total = 0;
   vector <Spdb::chunk_t>  chunk = data.getChunks();
   
   for (int i=0; i < data.getNChunks(); i++){

     total += chunk[i].write_time - chunk[i].valid_time; 

   }

   double mean = total / double(data.getNChunks());
 
   if (mean > 3600){
     fprintf(stderr,"Mean latency is %g hours, ",
	     mean/3600.0);
   } else if (mean > 600){
     fprintf(stderr,"Mean latency is %g minutes, ",
	     mean/60.0);
   } else {
     fprintf(stderr,"Mean latency is %g seconds, ",
	     mean);
   }

   double totalDiffSqrd = 0;
   for (int i=0; i < data.getNChunks(); i++){

      double lat =  chunk[i].write_time - chunk[i].valid_time; 
      totalDiffSqrd += (lat-mean)*(lat-mean);

   }

   double ms =  totalDiffSqrd / double(data.getNChunks());
   double rms = sqrt(ms);
     
   if (rms > 3600){
     fprintf(stderr,"standard deviation is %g hours based on %d samples\n",
	     rms/3600.0, data.getNChunks());
   } else if (rms > 600){
     fprintf(stderr,"standard deviation is %g minutes based on %d samples\n",
	     rms/60.0, data.getNChunks());
   } else {
     fprintf(stderr,"standard deviation is %g seconds based on %d samples\n",
	     rms, data.getNChunks());
   }

  return 0;


}

