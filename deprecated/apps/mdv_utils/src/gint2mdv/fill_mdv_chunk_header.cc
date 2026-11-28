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
 * FILL_MDV_CHUNK_HEADER:  Not used in gint2mdv but may be helpful
 * sometime for filling in a chunk header.
 * Return FILL_SUCCESS or RETURN FILL_FAILURE 
 * --Rachel Ames 1/96 RAP/NCAR
 */
 
#ifdef __cplusplus
extern "C" {
#endif

#include "gint2mdv.h"
using namespace std;

int fill_mdv_chunk_header(MDV_chunk_header_t *mch)
{

/* allocate space for header info */
   if (mch == NULL) {
      if ((mch = (MDV_chunk_header_t *) 
                  calloc(1,sizeof(MDV_chunk_header_t))) == NULL) {
         fprintf(stderr,"\nError occurred during calloc of mdv chunk header\n");
         return(FILL_FAILURE);
      }
   } /* endif need to allocat chunck header memory */

/* fill in fortran record size -- (record_len1=record_len2) */
   mch->record_len1 = sizeof(MDV_chunk_header_t)-2*sizeof(mch->record_len2);

   mch->struct_id = MDV_CHUNK_HEAD_MAGIC_COOKIE;

   /* mch->chunk_id = ;                                      */
   /* mch->chunk_data_offset = ;                             */
   /* mch->size = ;                                          */
   /* sprintf((char *)mch->info,"This is bogus chunk info"); */

/* fill in fortran record size -- (record_len1=record_len2) */
   mch->record_len2 = mch->record_len1;

/* normal exit */
    return(FILL_SUCCESS);
 
}  

#ifdef __cplusplus
}
#endif

