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
 * MDV_SERVER_INIT
 * F. Hage - NCAR/RAP
 */

#define MDV_SERVER_INIT

#include "mdv_server.h"

void signal_trap(int sig);

#define REG_SECS 30
/*****************************************************************
 * INIT_DATA_SPACE : Init all globals and set up defaults
 */

void init_data_space(void)
{
  gd.last_time_cent = -1;
  gd.last_requested_field = -1;
  gd.file_read_time = -1;
  
  if (gd.reg)
  {
    /* Initialize tha Process Mapper Functions */
    PMU_auto_init(gd.app_name, gd.app_instance,
		  PROCMAP_REGISTER_INTERVAL);
  }

  PORTsignal(SIGINT,signal_trap);
  PORTsignal(SIGTERM,signal_trap);
  PORTsignal(SIGPIPE,signal_trap);

  return;
}

/*****************************************************************
 * INIT_SOCKETS : Init all Socket Connections  
 */

void init_sockets(void)
{
  if ((gd.protofd = SKU_open_server(gd.port)) < 0)
  {
    fprintf(stderr, "Couldn't Begin Operations on Socket %d\n", gd.port);
    fprintf(stderr, "A Server Is probably already Running\n");
    exit(-1);
  }

  return;
}

/*****************************************************************
 * SIGNAL_TRAP : Traps Signals so as to die gracefully
 */

void signal_trap(int sig)
{
  if (!gd.daemonize_flag) fprintf(stderr,"Caught Signal %d\n",sig);

  switch(sig)
  {
  case SIGPIPE:
    close(gd.protofd);
    break;

  default:
    close(gd.protofd);
    PMU_auto_unregister();
    exit(0);
    break;
  }

  return;
}

