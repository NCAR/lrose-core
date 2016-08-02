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
 * find_field.c
 *
 * finds field number in mdv file, user is responsible
 * for setting value of field name in the params file.
 *
 * returns location index on success, otherwise -1 on failure
 *
 * Rachel Ames, RAP, NCAR, Boulder CO, July, 1996.
 * adapted for PTREC April 1997 (Terri Betancourt)
 *
 *********************************************************************/

#include "trec.h"

int find_field( int field, MDV_handle_t *mdv )

{
   int i;
   char *field_name = Glob->params.dbz_field_name;

   switch( field ) {
      case DBZ_FIELD:
           field_name = Glob->params.dbz_field_name;
           break;
      case VEL_FIELD:
           field_name = Glob->params.vel_field_name;
           break;
   }

   for (i = 0; i < mdv->master_hdr.n_fields; i++) {
     if ( (strcmp(field_name, (const char*)mdv->fld_hdrs[i].field_name)) == 0) {
       return(i);
      }
   }

   fprintf( stderr, "%s: Error. "               
            "Cannot find field '%s' for processing.\n",
            Glob->prog_name, field_name );
   return(-1);
}
