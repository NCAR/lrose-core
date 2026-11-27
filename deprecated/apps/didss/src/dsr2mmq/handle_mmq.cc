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
/***************************************************************************
 * handle_mmq.c
 *
 * routines to open and close the mmq
 *
 * Jaimi Yee
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * April 1998
 *
 ****************************************************************************/
#include "dsr2mmq.h"
using namespace std;

/*
 * static variables
 */
static int Mmq_id;

/******************************************
 * open_mmq()
 *
 * opens mmq for writing
 *****************************************/
int open_mmq(void)
{
   MMQ_old();

   if ((Mmq_id = MMQ_open (MMQ_WRITE, Glob->params.rdi_mmq_key)) < 0) {
       fprintf(stderr, "ERROR - %s:dsr2mmq\n", Glob->prog_name);
       fprintf(stderr, "Failed in opening rdi mmq, key = %d, ret = %d\n",
	       (int) Glob->params.rdi_mmq_key, Mmq_id);
       tidy_and_exit(-1);
   } else {
      if (Glob->params.debug) {
         fprintf(stderr, "Opened rdi mmq, key = %d, ret = %d\n",
	         (int) Glob->params.rdi_mmq_key, Mmq_id);
      }
   }
    
   return(Mmq_id);
}


/*****************************************
 * close_mmq()
 *
 * closes mmq
 *
 *****************************************/
void close_mmq()

{
  MMQ_close(Mmq_id);
}
