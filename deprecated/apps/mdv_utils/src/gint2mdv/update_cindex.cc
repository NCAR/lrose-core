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
 * update_index.cc
 *
 * update the current index file 
 *
 * Rachel Ames, RAP, NCAR, Boulder CO, January, 1996.
 *
 *********************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#include "gint2mdv.h"
#include <toolsa/ldata_info.h>
using namespace std;

int update_cindex(long file_time)
{

  // initialize the latest data info handle

  static int ldata_init = FALSE;
  static LDATA_handle_t ldata;

  if (!ldata_init) {
    LDATA_init_handle(&ldata, gd->prog_name, gd->params.debug);
    ldata_init = TRUE;
  }
   
  // write index file

  LDATA_info_write(&ldata, gd->params.output_dir, file_time,
		   "mdv", "MDV from GINT Nexrad Data ","mdv",
		   0, NULL);

  return(MDV_SUCCESS);

} /* end updating current index file */

#ifdef __cplusplus
}
#endif
