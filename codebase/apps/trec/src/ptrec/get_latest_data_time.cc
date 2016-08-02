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
 * get_latest_data_time.cc
 *
 * Reads current index file and returns name of new file. 
 *
 * Rachel Ames, RAP, NCAR, Boulder CO, January, 1996.
 *
 *********************************************************************/

#include "trec.h"
#include <toolsa/ldata_info.h>

long get_latest_data_time(char *rdata_dir,
			  char *prog_name,
			  long max_valid_data_age,
			  int debug)

{

  // initialize the latest data info handle

  static int ldata_init = FALSE;
  static LDATA_handle_t ldata;

  if (!ldata_init) {
    LDATA_init_handle(&ldata, prog_name, debug);
    ldata_init = TRUE;
  }
   
  /*
   * get latest data info.
   * blocks until new data available.
   */
      
  LDATA_info_read_blocking(&ldata,
			   rdata_dir,
			   max_valid_data_age,
			   WAIT_TIME,
			   PMU_auto_register);

  return (ldata.info.latest_time);

}

/*********************************************************************
 * Gets a listing of the data times within the user specfied limits
 * and fills in unix time of the two most recent images.  Returns
 * success or failure.
 *
 * Rachel Ames, RAP, NCAR, Boulder CO, December, 1996.
 *
 *********************************************************************/

int get_data_times(char *rdata_dir,
                   long *first_data_time,
                   long *latest_data_time)

{

   long *list;
   int num_top_dirs = 1;
   time_t now_time;
   int num_times=0;
    
   /* get a list of data times */

   now_time = time(0);
 
   num_times = find_all_data_times(&list,
                                   num_top_dirs,
                                   &rdata_dir,
                                   Glob->params.input_file_suffix,
                                   now_time - Glob->params.max_time_between_images,
                                   now_time + 100000);
 
 
 
   if (num_times < 2) {
      return(FAILURE);
   }
 
   *first_data_time = list[num_times-2]; 
   *latest_data_time = list[num_times-1]; 

   return (SUCCESS);
}
