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
/*********************************************************************
 * MDV_SERVER.CC: Provides a data service for mdv data files.
 *  Modified for Version 2.0 Protocol - Uses URL
 * F. Hage - NCAR/RAP
 */

#define MDV_SERVER_MAIN

#include "mdv_server.h"

/********************************************************************
 * MAIN: Process arguments, initialize and begin application
 */

int main(int argc, char **argv)
{
  int i, c_pid;
  memset((void *)&gd, 0, sizeof(gd)); /* clear global struct */

  process_args(argc, argv);    /* process command line arguments */

  if (gd.daemonize_flag) daemonize(); /* turn process into a daemon */

  /* initialize globals, get/set defaults, establish data source etc. */
  init_data_space();

  init_sockets();        /* Set up the TCP/IP prototype discriptor */

  // Pre-Fork Children to handle high loads or slow networks.
  for(i = 0; i < gd.num_children; i++) {

      if((c_pid = fork()) == 0) { // is a child
          i = gd.num_children; // Break out of loop 
       } else{
        fprintf(stderr,"Child Started. PID: %d\n",c_pid);
       }
  }

  get_client_loop();    /* Do the work */

  exit(0);

}
