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
#include <signal.h>  
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <cstdio>
#include <ctime>

#include "Params.hh"
using namespace std;

void cleanExit( int signal );                                                                               
int main(int argc, char *argv[]){

  //
  // Trap signals for a clean exit
  //
  PORTsignal( SIGINT,  cleanExit );
  PORTsignal( SIGTERM, cleanExit );
  PORTsignal( SIGQUIT, cleanExit );
  PORTsignal( SIGKILL, cleanExit );
          
  Params P;
  if (P.loadFromArgs(argc,argv,NULL,NULL)){
    cerr << "Specify params file with -params option." << endl ;
    return(-1);
  }
 
  PMU_auto_init("WsmrLtgSim", P.Instance,
                PROCMAP_REGISTER_INTERVAL);   

  const float TopLat = 34.18;
  const float MidLat = 33.07;
  const float BotLat = 32.0;

  const float LftLon = -107.44;
  const float MidLon = -106.11;
  const float RgtLon = -104.8;



  do {

    date_time_t Go;

    date_time_t t;
    t.unix_time = time(NULL);
    uconvert_from_utime( &t );
 
    Go.year = t.year;
    Go.month = t.month;
    Go.day = t.day;
    Go.hour = 0;
    Go.min = 0;
    Go.sec = 0;
    uconvert_to_utime ( &Go );

   char FileName[1024];

    sprintf(FileName,"%s/%04d%02d%02d%02d%02d%02d.ltg.asc",
	    P.OutputDir,
	    t.year, t.month, t.day, t.hour, t.min, t.sec);

    FILE *fp = fopen(FileName,"wt");
    if (fp == NULL){
      fprintf(stderr,"Failed to create %s\n",FileName);
      exit(-1);
    }

    if (P.Debug){
      fprintf(stderr,"Writing %s\n",FileName);
    }

    float lat, lon;

    double q;

    q = (t.unix_time - Go.unix_time)/86400.0;

    lat = q*TopLat + (1.0 - q)*BotLat;

    lon = q * RgtLon + (1.0 - q)*LftLon;

    fprintf(fp,"%ld %g %g -18\n",
	    t.unix_time, lon,lat);

    fprintf(fp,"%ld %g %g -18\n",
	    t.unix_time, MidLon,lat);

    fprintf(fp,"%ld %g %g -18\n",
	    t.unix_time, lon,MidLat);


    fclose(fp);



    for (int i=0; i < P.TimeStep; i++){
      PMU_auto_register(FileName);
      sleep(1);
    }

  } while (1); // Eternal loop.


  return 0;

}

/////////////////////////////////////////////////
void cleanExit( int signal )
{
   //
   // Unregister with process mapper and exit
   //
   PMU_auto_unregister();
   exit( signal );
}
     
