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
 * FILL_MDV_VLEVEL_HEADER: copy applicapble contents of gint
 * header into an mdv vlevel header
 * Return FILL_SUCCESS or RETURN FILL_FAILURE 
 * --Rachel Ames 1/96 RAP/NCAR
 */
 
#ifdef __cplusplus
extern "C" {
#endif

#include "gint2mdv.h"
using namespace std;

int fill_mdv_vlevel_header(Tvolume_header *gh, MDV_master_header_t mmh,
                           MDV_vlevel_header_t *mvh)
{
   int vlevel =0;

/* allocate space for header info */
   if (mvh == NULL) {
      if ((mvh = (MDV_vlevel_header_t *) 
                  ucalloc(1,sizeof(MDV_vlevel_header_t))) == NULL) {
         fprintf(stderr,"\nError occurred during calloc of mdv vlevel header\n");
         return(FILL_FAILURE);
      }
   }

/* fill in fortran record size -- (record_len1=record_len2) */
   mvh->record_len1 = sizeof(MDV_vlevel_header_t)-2*sizeof(mvh->record_len2);

   mvh->struct_id = MDV_VLEVEL_HEAD_MAGIC_COOKIE;

/* note all vlevels in gint files are the same size 
 * fill_mdv_field_header has already checked that there aren't too many vlevels  * add sensor altitude to each vlevel since gint has levels relative to sensor
 */

   for (vlevel = 0; vlevel < gh->vh->nz; vlevel++)  {

      mvh->vlevel_type[vlevel]   = gd->params.vertical_type; 

      if (mvh->vlevel_type[vlevel] == MDV_VERT_TYPE_Z) 
         mvh->vlevel_params[vlevel] = (gh->ai[vlevel].z)*METERS2KM+mmh.sensor_alt;
      else
         mvh->vlevel_params[vlevel] = (gh->ai[vlevel].z);
   }
 
/* fill in fortran record size -- (record_len1=record_len2) */
   mvh->record_len2 = mvh->record_len1;

/* normal exit */
    return(FILL_SUCCESS);
 
}  

#ifdef __cplusplus
}
#endif

