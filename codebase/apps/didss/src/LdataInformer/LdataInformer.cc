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
///////////////////////////////////////////////////////////////
// LdataInformer.cc
//
//
// This is a small program that registers with the data mapper 
// using command line spefied time and directory. It is
// intended for use in perl scripts that ingest data.
//
// Niles Oien, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May  2000
//
///////////////////////////////////////////////////////////////

#include <iostream>
#include <toolsa/pmu.h>
#include <didss/RapDataDir.hh>
#include <didss/LdataInfo.hh>
#include <dsserver/DmapAccess.hh>
#include <cstdlib>
using namespace std;

void Usage();

int main(int argc, char *argv[])
{

  //
  // Check if DATA_MAPPER_ACTIVE is set.
  //
  if (NULL==getenv("DATA_MAPPER_ACTIVE")){
    cerr << "Environment variable DATA_MAPPER_ACTIVE needs to be set." << endl;
    cerr << "LdataInformer will run, but will have no effect." << endl;
  }

  //
  // Exit if too few args.
  //

  if (argc < 5){
    Usage();
    exit(-1);
  }

  //
  // Process the command line.
  // Initialise variables to silly values for
  // overwriting.
  //

  date_time_t T;
  char Dir[MAX_PATH_LEN];
  char dataType[256];

  T.unix_time = 0;
  sprintf(Dir,"%s","");
  sprintf(dataType,"%s","");

  int leadTime = -1;

  for(int i=0; i < argc; i++){

    if (
	(!(strcmp(argv[i],"-h")))   ||
	(!(strcmp(argv[i],"-man"))) ||
	(!(strcmp(argv[i],"--")))   ||
	(!(strcmp(argv[i],"-help")))
	){
      Usage();
      exit(0);
    }

    if (!(strcmp(argv[i],"-t"))){
      if (i == argc-1){
	Usage();
	exit(-1);
      }
      if ( 6 != sscanf(argv[i+1],
	     "%4d%02d%02d%02d%02d%02d",
	     &T.year, &T.month, &T.day,
	     &T.hour, &T.min, &T.sec)){
	Usage();
	cerr << endl << "Time format is not correct." << endl;
	exit(-1);
      }
      uconvert_to_utime( &T );
    }

   if (!(strcmp(argv[i],"-d"))){
      if (i == argc-1){
	Usage();
	exit(-1);
      }
      sprintf(Dir,"%s",argv[i+1]);
    }


   if (!(strcmp(argv[i],"-D"))){
      if (i == argc-1){
	Usage();
	exit(-1);
      }
      sprintf(dataType,"%s",argv[i+1]);
    }

   if (!(strcmp(argv[i],"-l"))){
      if (i == argc-1){
	Usage();
	exit(-1);
      }
      leadTime = atoi(argv[i+1]);
      if (leadTime <= 0){
	cerr << "WARNING : lead time in seconds is " << leadTime << endl;
      }
   }

  }

  //
  // Exit if the inititial silly values have not been overwritten.
  //
  if ((strlen(Dir) == 0) || (T.unix_time == 0)){
    Usage();
    exit(-1);
  }

  //
  // Register with the data mapper.
  //
  DmapAccess access;

  if (access.regLatestInfo(T.unix_time, Dir, dataType, leadTime)) {
    cerr << "  Failed to register with data mapper." << endl;
    cerr << "  Dir: " << Dir << endl;
    cerr << "  Data Time UTC: " << utimstr(T.unix_time) << endl;
    cerr << "  Data type : " << dataType << endl;
  }

  return 0;

}

///////////////////////////////////////////

void Usage()
{

  cerr << "LdataInformer allows you to contact the data mapper and register\n"
       << "  a data set, by specifying the details on the command line.\n"
       << endl;

  cerr << "Usage : " << endl;
  cerr << "LdataInformer -t YYYYMMDDHHMMSS -d Directory [-D dataType] [-l leadTime (secs)]" << endl;
  cerr << endl;
  cerr << "If a lead time is specified, the time specified by -t is taken" << endl;
  cerr << "to be the generation time." << endl;
  return;

}
