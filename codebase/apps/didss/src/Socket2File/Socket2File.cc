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
#include <toolsa/pmu.h>
#include <toolsa/port.h>
#include <unistd.h>
#include <toolsa/Socket.hh>
#include "Params.hh"
using namespace std;

static void tidy_and_exit (int sig);
 
int main(int argc, char *argv[])
{

  //
  // See if they want help.
  //
  for (int i=1; i < argc; i++){
    if (
	(!(strcmp("-h", argv[i]))) ||
	(!(strcmp("--", argv[i]))) ||
	(!(strcmp("-?", argv[i])))
	){
      fprintf(stderr,"Try the -print_params option for help.\n");
      exit(0);
    }
  }
  // Trap.
  PORTsignal(SIGINT, tidy_and_exit);
  PORTsignal(SIGHUP, tidy_and_exit);
  PORTsignal(SIGTERM, tidy_and_exit);
  

  // Get the TDRP parameters.

  Params *TDRP_params = new  Params(); 

  if (TDRP_params->loadFromArgs(argc, argv, NULL, NULL)){
    fprintf(stderr,"Specify params file with -params option.\n");
    exit(-1);
  } 

  // Check in.

  PMU_auto_init("Socket2File", TDRP_params->instance, PROCMAP_REGISTER_INTERVAL);

  //
  // Init the socket reading object.
  //

  Socket S;
  int retVal = S.open(TDRP_params->hostname, TDRP_params->port);
  if (retVal){
    fprintf(stderr,"Attempt to open port %d at %s returned %d\n",
	    TDRP_params->port, TDRP_params->hostname, retVal);
    fprintf(stderr,"Error string : %s\n",S.getErrString().c_str());

    switch(S.getErrNum()){

    case Socket::UNKNOWN_HOST :
      fprintf(stderr,"Unknown host.\n");
      break;
       
    case Socket::SOCKET_FAILED :
      fprintf(stderr,"Could not set up socked (maxed out decriptors?).\n");
      break;
      
    case Socket::CONNECT_FAILED :
      fprintf(stderr,"Connect failed.\n");
      break;
 
    case Socket::TIMED_OUT :
      fprintf(stderr,"Timed out..\n");
      break;

    case Socket::UNEXPECTED :
      fprintf(stderr,"Unexpected error.\n");
      break;

    default :
      fprintf(stderr,"Unknown error.\n");
      break;

    }
    exit(-1);
  }

  //
  // Open the output file.
  //
  FILE *fp = fopen(TDRP_params->OutFile,"w");
  if (fp == NULL){
    fprintf(stderr,"Could not open %s write append.\n", TDRP_params->OutFile);
    S.close();
    S.freeData();
    exit(-1);
  }
  //
  // Loop, reading the socket and dumping to a file.
  //
  int numBytesRead = 0;
  do {

    const int bufferSize = 128;
    unsigned char buffer[bufferSize];

    if (S.readSelectPmu()){

      S.close();
      S.freeData();

      switch (S.getErrNum()){

      case Socket::TIMED_OUT :
	fprintf(stderr,"Read select timed out.\n");
	exit(-1);
	break;

      case Socket::SELECT_FAILED :
	fprintf(stderr,"Read select failed.\n");
	exit(-1);
	break;

      case Socket::UNEXPECTED :
	fprintf(stderr,"Read select - unexpected error.\n");
	exit(-1);
	break;

      default :
	fprintf(stderr,"Unkown error with read select.\n");
	exit(-1);
	break;

      }
    }


    if (S.readBufferHb(buffer,
		       bufferSize,
		       bufferSize,
		       (Socket::heartbeat_t)PMU_auto_register) != 0 ){

      S.close();
      S.freeData();

      switch (S.getErrNum()){

      case Socket::TIMED_OUT :
	fprintf(stderr,"Read buffer timed out.\n");
	exit(-1);
	break;

      case Socket::BAD_BYTE_COUNT :
	fprintf(stderr,"Read buffer gave bad byte count.\n");
	exit(-1);
	break;

      default :
	fprintf(stderr,"Unkown error with read buffer.\n");
	exit(-1);
	break;
      }

      return -1;
    }

    int numRead = S.getNumBytes();

    if (numRead > bufferSize){
      fprintf(stderr,"Buffer size exceeded.\n");
      fclose(fp);    S.close();   S.freeData();
      exit(-1);
    }

    numBytesRead = numBytesRead + numRead;
    fwrite(buffer, sizeof(unsigned char), numRead, fp);

    if (TDRP_params->debug){
      fprintf(stderr,"Read %d bytes and wrote to file.\n", numRead);
    }

  } while (numBytesRead < TDRP_params->NumBytesToRead);

  fclose(fp);    S.close();   S.freeData();

  if (TDRP_params->debug){
    fprintf(stderr,"Done.");
  }

  return 0;

}

/////////////////////////////////////////////

static void tidy_and_exit (int sig){
  // PMU_auto_unregister(); // This seems to cause problems on cntrl-C
  exit(sig);
}



