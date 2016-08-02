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

#include <cstdio>
#include <cstring>
#include <toolsa/umisc.h>

void printHelp();

int main(int argc, char *argv[]){

  bool printUse = false;

  for (int i=1; i < argc; i++){
    if (!(strcmp("-h", argv[i]))) printUse = true;
  }

  if ((argc == 1) || (printUse)){
    printHelp();
    exit(0);
  }

  bool gotArg = false;
  date_time_t T;

  if (!(strcmp("-unix", argv[1]))){
    gotArg = true;

    if (argc < 3){
      fprintf(stderr, "ERROR : -unix requires yyyy,mm,dd,hh,mm,ss argument.\n");
      exit(-1);
    }

    if (6 != sscanf(argv[2], "%d,%d,%d,%d,%d,%d",
		    &T.year, &T.month, &T.day, &T.hour, &T.min, &T.sec)){
      fprintf(stderr,"Failed to parse calendar time from %s\n", argv[2]);
      exit(-1);
    }

    uconvert_to_utime( &T );
    fprintf(stdout,"%ld\n", T.unix_time);
  }



  if (!(strcmp("-cal", argv[1]))){
    gotArg = true;

    if (argc < 3){
      fprintf(stderr, "ERROR : -cal requires unixtime argument.\n");
      exit(-1);
    }

    if (1 != sscanf(argv[2], "%ld", &T.unix_time)){
      fprintf(stderr,"Failed to parse unix time from %s\n", argv[2]);
      exit(-1);
    }

    uconvert_from_utime( &T );
    fprintf(stdout,"%d,%02d,%02d,%02d,%02d,%02d\n",
	    T.year, T.month, T.day, T.hour, T.min, T.sec);
  }




  if (!(strcmp("-sum", argv[1]))){
    gotArg = true;

    if (argc < 4){
      fprintf(stderr, "ERROR : -sum requires calendar time and delta arguments.\n");
      exit(-1);
    }

    if (6 != sscanf(argv[2], "%d,%d,%d,%d,%d,%d",
		    &T.year, &T.month, &T.day, &T.hour, &T.min, &T.sec)){
      fprintf(stderr,"Failed to parse calendar time from %s\n", argv[2]);
      exit(-1);
    }

    long delta;
    if (1 != sscanf(argv[3], "%ld", &delta)){
      fprintf(stderr,"Failed to parse delta time from %s\n", argv[3]);
      exit(-1);
    }

    uconvert_to_utime( &T );
    T.unix_time += delta;
    uconvert_from_utime( &T );

    fprintf(stdout,"%d,%02d,%02d,%02d,%02d,%02d\n",
	    T.year, T.month, T.day, T.hour, T.min, T.sec);
  }



  if (!(strcmp("-diff", argv[1]))){
    gotArg = true;

    if (argc < 4){
      fprintf(stderr, "ERROR : -diff requires two calendar times.\n");
      exit(-1);
    }

    if (6 != sscanf(argv[2], "%d,%d,%d,%d,%d,%d",
		    &T.year, &T.month, &T.day, &T.hour, &T.min, &T.sec)){
      fprintf(stderr,"Failed to parse calendar time from %s\n", argv[2]);
      exit(-1);
    }

    date_time_t T2;
    if (6 != sscanf(argv[3], "%d,%d,%d,%d,%d,%d",
		    &T2.year, &T2.month, &T2.day, &T2.hour, &T2.min, &T2.sec)){
      fprintf(stderr,"Failed to parse calendar time from %s\n", argv[3]);
      exit(-1);
    }

    uconvert_to_utime( &T );
    uconvert_to_utime( &T2 );

    fprintf(stdout,"%ld\n", T.unix_time - T2.unix_time);

  }

  if (!(gotArg)){
    fprintf(stderr,"Failed to recognise argument %s\n", argv[1]);
    fprintf(stderr,"Valid options are -unix, -cal, -sum and -diff. Use -h option for help.\n");
    return -1;
  }

  return 0;

}

void printHelp(){

  fprintf(stderr,"\nTimeCalc is a small utility to do time calculations.\n\n"
	  "There are four modes of operation as determined by the first argument :\n\n"
	  "[1] TimeCalc -cal converts a unix time to a calendar time :\n"
	  "      TimeCalc -cal 0\n"
	  "      1970,01,01,00,00,00\n\n"
	  "[2] TimeCalc -unix converts a unix time to a calendar time :\n"
	  "      TimeCalc -unix 1970,01,01,01,00,00\n"
	  "      3600\n\n"
	  "[3] TimeCalc -sum adds a delta time (seconds) to a calendar time :\n"
	  "      TimeCalc -sum 1970,01,01,01,00,00 3600\n"
	  "      1970,01,01,02,00,00\n\n"
	  "[4] TimeCalc -diff gives the difference between two calendar times :\n"
	  "      TimeCalc -diff 1970,01,01,01,00,00 1970,01,01,00,00,00\n"
	  "      3600\n\n"
	  "Niles Oien September 2006\n\n");

  return;

}
