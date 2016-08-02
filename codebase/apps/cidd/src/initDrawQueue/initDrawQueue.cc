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
// Small program to initialize draw queue FMQs. Useful to
// initialize draw queues used with CIDD since otherwise CIDD
// will only initialize the FMQs when the user enters data - that
// behavior is not ideal, but nor am I convinced that it needs to
// be changed (otherwise CIDD may init draw FMQs everywhere) - and
// the first boubdary/polygon entered in those circumstances will
// get lost in the FMQ.
//
// Niles Oien May 2011.
//

#include <Fmq/DrawQueue.hh>

int main(int argc, char *argv[]){

  if (argc < 2){
    fprintf(stderr,"USAGE : %s <list of draw FMQs to init>\n", argv[0]);
    return -1;
  }

  for (int i=1; i < argc; i++){

    char *p = argv[i];
    if (p[0] == '-'){
      fprintf(stderr,"%s needs a list of draw FMQs to init on the command line - no command line arguments like %s\n", 
	      argv[0], p);
      continue;
    }

    DrawQueue dq;
    if (dq.init( argv[i], (char *) "CIDD" ) != 0 ) {  
	    fprintf(stderr,"Problems initialising Draw Fmq: %s - aborting\n",
		    argv[i]);
	    continue;
    }
  }

  return 0;

}
