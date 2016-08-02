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
 * CREATE_FILE_NAMES: contains routines to deal with
 * creating input/output file names.
 */

#include "trec.h"

/*****************************************************************
 * CREATE_MDV_INPUT_FILE_NAME:  create a name for a mdv file based on 
 * time and user specified suffix and top directory.
 *
 * fills in file_name on success, null on return if error.
 *
 */

char *create_mdv_input_fname(long file_time, char *file_name)
{
    
   struct stat dir_stat;	  /* directory status */
   char format_str[NAMESIZE];     /* temporary format string */
   char dir_name[NAMESIZE];       /* full directory name (top dir+date)*/
   struct tm *tm;    		  /* file time */


/* break out time into year,mon,day, etc  */
   tm = gmtime(&file_time);  
 
/* build the directory name - input_dir/YYYYMMDD/  */
   strncpy(format_str,Glob->params.input_dir,NAMESIZE);
   strncat(format_str,"/%Y%m%d",NAMESIZE);
   strftime(dir_name,NAMESIZE,format_str,tm);
 
/* Check for the existance of the directory, directory should be there! */
   errno = 0;
   if(stat(dir_name,&dir_stat) < 0) {

      if(errno == ENOENT) {
          fprintf(stderr,"%s: Error. Directory '%s' not found\n",
                  Glob->prog_name,dir_name);
          return NULL;
      }
   }
 
/* Build input filename - HHMMSS.suffix */
   strncpy(format_str,Glob->params.input_dir,NAMESIZE);
   strncat(format_str,"/%Y%m%d/%H%M%S.",NAMESIZE);
   strncat(format_str,Glob->params.input_file_suffix,NAMESIZE);
   strftime(file_name,NAMESIZE,format_str,tm);

   return file_name;
}
 
/*****************************************************************
 * CREATE_MDV_OUTPUT_FNAME:  create a name for a mdv file based on 
 * time and user specified suffix and top directory.  
 *
 * Return NULL on failure, a filename on success.
 */

char *create_mdv_output_fname(char *file_name, MDV_master_header_t *mmh)
{
    
   struct stat dir_stat;	     /* directory status */
   char format_str[NAMESIZE];        /* temporary format string */
   char dir_name[NAMESIZE];          /* full directory name *(top dir + date)*/
   struct tm *tm;    		     /* file time */
   long file_time;		     /* unix time of file */


/* break up time into year,mon,day, etc  */
   file_time = mmh->time_centroid;
   tm = gmtime(&file_time);  
 
/* build the directory name - output_dir/YYYYMMDD/  */
   strncpy(format_str,Glob->params.output_dir,NAMESIZE);
   strncat(format_str,"/%Y%m%d",NAMESIZE);
   strftime(dir_name,NAMESIZE,format_str,tm);
 
/* Check for the existance of the directory, create one if not there already */
   errno = 0;

   if (stat(dir_name,&dir_stat) < 0) {

      if(errno == ENOENT) { 
         if (mkdir(dir_name,0775) < 0) {
            fprintf(stderr,"%s: Error. Cannot create directory '%s'\n",
                    Glob->prog_name,dir_name);
            return NULL;
         }
      }

      else { /*directory found but some other error */
         fprintf(stderr,"%s: Failed to create mdv file name.\n",
                 Glob->prog_name);
         return NULL;
      }

   } /* endif error when stat directory */
 
/* Build output filename - HHMMSS.suffix */
   strncpy(format_str,Glob->params.output_dir,NAMESIZE);
   strncat(format_str,"/%Y%m%d/%H%M%S.",NAMESIZE);
   strncat(format_str,Glob->params.output_file_suffix,NAMESIZE);
   strftime(file_name,NAMESIZE,format_str,tm);
 
   return file_name;
}
