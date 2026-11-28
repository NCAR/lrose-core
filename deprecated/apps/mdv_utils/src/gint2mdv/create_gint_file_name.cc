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
 * CREATE_GINT_FILE_NAME:  create a name for a gint file based on 
 * time and user specified suffix and top directory.
 *
 * Return NULL on failure, a filename on success.
 *
 * based on create_mdv_file_name by F. Hage.  Rachel Ames 6/96
 */
 

#ifdef __cplusplus
extern "C" {
#endif
 
#include "gint2mdv.h"
using namespace std;

char *create_gint_file_name(long latest_data_time)
{
    
   struct stat dir_stat;			/* directory status */
   char format_str[BUFSIZE];                    /* temporary format string */
   char dir_name[BUFSIZE];                      /* full directory name (top dir + date)*/
   struct tm *tm;    				/* file time */

   static char file_name[BUFSIZE];	        /* full file name */

/*--------------------------------------------------------------------------------------*/

/* break out time into year,mon,day, etc  */
   tm = gmtime(&latest_data_time);  
 
/* build the directory name - input_dir/YYYYMMDD/  */
   strncpy(format_str,gd->params.input_dir,BUFSIZE);
   strncat(format_str,"/%Y%m%d",BUFSIZE);
   strftime(dir_name,BUFSIZE,format_str,tm);
 
 
/* Check for the existance of the directory, create one if not there already */
   errno = 0;
   if(stat(dir_name,&dir_stat) < 0) {
      if(errno == ENOENT) { // No directory found
          // Create one
         if(mkdir(dir_name,0775) < 0) perror("gint2mdv: Cannot create input dir");
      } else {
          perror("gint2mdv: failure in create_gint_file_name.cc ");
          exit(-1);
      }
   }
 
/* Build input filename - HHMMSS.suffix */

   strncpy(format_str,gd->params.input_dir,BUFSIZE);
   strncat(format_str,"/%Y%m%d/%H%M%S.",BUFSIZE);
   strncat(format_str,gd->params.input_file_suffix,BUFSIZE);
   strftime(file_name,BUFSIZE,format_str,tm);
 
   return(file_name);
}
 
#ifdef __cplusplus
}
#endif
