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
 * tidy_and_exit.c
 *
 * tidies up and quits
 *
 * Mike Dixon
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * April 1991
 *
 ****************************************************************************/

#include <signal.h>
#include "Rview.hh"
using namespace std;

void tidy_and_exit(int sig)

{

  if (Glob->debug) {
    fprintf(stderr, "** tidy_and_exit **\n");
  }

  /*
   * unregister process
   */

  PMU_auto_unregister();

  /*
   * clean up system call environment
   */

  usystem_call_clean();

  /*
   * remove shared memory if applicable
   */

  if (Glob->use_time_hist && Glob->track_shmem != NULL) {
    
    Glob->track_shmem->main_display_active = FALSE;

    if (ushm_remove(Glob->track_shmem_key) != 0) {
      fprintf(stderr, "WARNING - %s:tidy_and_exit\n", Glob->prog_name);
      fprintf(stderr,
	      "Cannot remove shared memory for coord_export, key = %x.\n",
	      Glob->track_shmem_key);
    }

    if (ushm_remove(Glob->track_shmem_key + 1) != 0) {
      fprintf(stderr, "WARNING - %s:tidy_and_exit\n", Glob->prog_name);
      fprintf(stderr,
	      "Cannot remove shared memory for track data, key = %x.\n",
	      Glob->track_shmem_key);
    }

  }

  /*
   * verify mallocs
   */

  umalloc_map();
  umalloc_verify();

  /*
   * free X resources unless called by xerror_handler
   */

  if (sig != -1)
    free_resources();

  /*
   * exit with code sig
   */

  exit(sig);

}
