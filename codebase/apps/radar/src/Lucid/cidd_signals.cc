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
/*****************************************************************
 * CIDD_SIGNALS.CC 
 */

#define CIDD_SIGNALS
#include "cidd.h"
#include "cidd_funcs.h"

/*****************************************************************
 * SIGNAL_TRAP : Traps Signals so as to die gracefully
 */
void signal_trap(int signal)
{
    fprintf(stderr,"Lucid: received signal %d\n",signal);
    // if(gd.finished_init) base_win_destroy(gd.h_win_horiz_bw->horiz_bw,DESTROY_PROCESS_DEATH);
    exit(0);
}

/*****************************************************************
 * SIGIO_TRAP : Traps IO Signal
 */
void sigio_trap(int signal)
{
    signal = 0;
    if(gd.io_info.outstanding_request) check_for_io();
}

/*****************************************************************
 * INIT_SIGNAL_HANDLERS: 
 */

void  init_signal_handlers()
{
    PORTsignal(SIGINT,signal_trap);
    PORTsignal(SIGTERM,signal_trap);
    PORTsignal(SIGIO,sigio_trap);
}
