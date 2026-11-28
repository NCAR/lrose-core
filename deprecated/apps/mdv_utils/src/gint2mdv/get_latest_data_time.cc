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

#ifdef __cplusplus
extern "C" {
#endif


#define WAIT_TIME 5000

#include "gint2mdv.h"
#include <toolsa/ldata_info.h>
using namespace std;

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
			   2000,
			   PMU_auto_register);

  return (ldata.info.latest_time);

}


#ifdef __cplusplus
}
#endif
