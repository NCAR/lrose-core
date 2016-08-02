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
 * MDV_SERVER_CHOOSE_FILE
 * F. Hage - NCAR/RAP
 */

#define MDV_SERVER_CHOOSE_FILE

#include "mdv_server.h"
#include <toolsa/str.h>

char *get_data_filename(long time, int num_top_dirs,
                        char **top_dir, char *suffix,
			time_t *update_time);

/*****************************************************************
 * CHOOSE_AND_OPEN_FILE: Choose a file to open, Load the headers and 
 * determine if one of the fields matches the request.
 * Copy grid info into grid_info_reply struct
 */

int choose_and_open_file(cdata_ieee_comm_t *com, cdata_ieee_comm2_t * com2)
{
  int i;
  long    data_time;
  int     opened_new_file = FALSE;
  char    *data_filename = NULL;
  time_t  last_file_update;
  int     input_file_mode = 0; // 0 = Regular, 1 = Static file, 2 = Static URL
  struct stat file_stat;
  
  if (com->primary_com & (GET_MOST_RECENT | GET_NEW)) {
    if (gd.input_filename != NULL) {
      input_file_mode = 1;
    } else {  /* find time handle */

    // Check to see if the URL refers to a plain file
      for(i=0; i < gd.num_top_dirs; i++) {
         if (stat(gd.top_dir_url[i], &file_stat) == 0) {
	   // If file is a regular file - as apposed to a dir
	   if(file_stat.st_mode & S_IFREG) {
	      input_file_mode = 2;
	      data_filename = gd.top_dir_url[i];
	      i = gd.num_top_dirs; // Bail out of the rest of the loop
	   }
	 }
      }
      if (input_file_mode == 0 && 
	  (data_time = find_latest_data_time(gd.num_top_dirs,
				gd.top_dir_url,
				gd.data_file_suffix)) == 0)
      { 
      if (gd.data_file_open)
	fclose(gd.data_file);
      gd.data_file = NULL;
      gd.data_file_open = FALSE;
      gd.file_read_time = -1;
      return NO_INFO | NO_DATA_AVAILABLE;
      }
    }
  } else { /* get  best file time */
    // Check to see if the URL refers to a plain file
    for(i=0; i < gd.num_top_dirs; i++) {
         if (stat(gd.top_dir_url[i], &file_stat) == 0) {
	   // If file is a regular file - as apposed to a dir
	   if(file_stat.st_mode & S_IFREG) {
	      input_file_mode = 2;
	      data_filename = gd.top_dir_url[i];
	      i = gd.num_top_dirs; // Bail out of the rest of the loop
	   }
	 }
    }
    if (input_file_mode == 0 &&
	(data_time = find_best_data_time(com->time_min, com->time_cent,
					 com->time_max,
					 gd.num_top_dirs, gd.top_dir_url,
					 gd.data_file_suffix)) == 0)
    { 
      if (gd.data_file_open)
	fclose(gd.data_file);
      gd.data_file = NULL;
      gd.data_file_open = FALSE;
      gd.file_read_time = -1;
      return (NO_INFO | NO_DATA_AVAILABLE);
    }
  }

  /* Get the data filename and the last update time to see if we
   * need to open a new data file. */
  switch (input_file_mode) {
  case 1:
    data_filename = STRdup(gd.input_filename);
    if (stat(data_filename, &file_stat) != 0) {
      if (!gd.daemonize_flag)
	fprintf(stderr, "Error stating input file <%s>\n", data_filename);
      if (gd.data_file_open) fclose(gd.data_file);
      gd.data_file = NULL;
      gd.data_file_open = FALSE;
      gd.file_read_time = -1;
      return (NO_INFO | NO_DATA_AVAILABLE|  NO_SUCH_FILE);
    }
    last_file_update = file_stat.st_mtime;
  break;

  case 0:
    if ((data_filename = get_data_filename(data_time,
			      gd.num_top_dirs,
			      gd.top_dir_url,
			      gd.data_file_suffix,
			      &last_file_update)) == NULL)
    {
      if (gd.data_file_open) fclose(gd.data_file);
      gd.data_file = NULL;
      gd.data_file_open = FALSE;
      gd.file_read_time = -1;
      return NO_INFO | NO_DATA_AVAILABLE | NO_SUCH_FILE;
    }
  break;

  case 2:  // data_filename already set above 
     if(data_filename == NULL) {
      return (NO_INFO | NO_DATA_AVAILABLE | NO_SUCH_FILE);
     }
  break;
  }


  if (!gd.daemonize_flag)
     fprintf(stderr, "PID: %d Processing file %s\n", getpid(), data_filename);
  
  /* See if we really need to open a new data file */
  if (strcmp(gd.last_data_filename, data_filename) != 0 ||
      !gd.data_file_open ||
      gd.last_file_time != data_time ||
      gd.file_read_time < last_file_update)
    {
  
      /* Close the old data file */
      if (gd.data_file_open)
	fclose(gd.data_file);

      gd.data_file = NULL;
      gd.data_file_open = FALSE;
      
      /* Open up the new data file */
      gd.file_read_time = time(NULL);
      STRncopy(gd.last_data_filename,data_filename,1024);

      if ((gd.data_file = ta_fopen_uncompress(data_filename, "r")) == NULL)
      {
	gd.file_read_time = -1;
        return NO_INFO | NO_DATA_AVAILABLE;
      } 

      gd.data_file_open = TRUE;
      gd.last_file_time = data_time;
      opened_new_file = TRUE;

      /*
       * Read in the header information for the file and the fields.
       */

      if (MDV_load_master_header(gd.data_file, &gd.cur_m_head) != MDV_SUCCESS)
      {
        fclose(gd.data_file);
        gd.data_file = NULL;
        gd.data_file_open = FALSE;
        gd.last_file_time = -1;
	gd.file_read_time = -1;
    
        return NO_INFO | NO_DATA_AVAILABLE;
      }

      // Make sure space for field headers has been allocated
      if(gd.cur_m_head.n_fields > MAX_FILE_FIELDS)
	  gd.cur_m_head.n_fields = MAX_FILE_FIELDS;

      if(gd.cur_m_head.n_fields > gd.num_fheads_alloc) {
	  for(i= gd.num_fheads_alloc ; i < gd.cur_m_head.n_fields; i++) {
	      gd.fhead[i] = (MDV_field_vlevel_header_t *)
	          calloc(1,sizeof(MDV_field_vlevel_header_t));
	  }
	 gd.num_fheads_alloc = gd.cur_m_head.n_fields;
      }
      for(i = 0; i < gd.cur_m_head.n_fields; i++) {
        if (MDV_load_field_vlevel_header(gd.data_file,
				   gd.fhead[i], &gd.cur_m_head,
				   i) != MDV_SUCCESS)
          {
	  fclose(gd.data_file);
	  gd.data_file = NULL;
	  gd.data_file_open = FALSE; 
          gd.last_file_time = -1;
	  gd.file_read_time = -1;
	  
          return NO_INFO | NO_DATA_AVAILABLE;
        }
      }
     
    } else {
      if (!gd.daemonize_flag) fprintf(stderr, "Don't need to open new file\n");
    }

    // Determine which field is desired
    switch(gd.protocol_version) {
      case 0:
	  if(gd.request_field_index >= gd.cur_m_head.n_fields) {
             if (!gd.daemonize_flag) fprintf(stderr,
                "Non-Existant field requested\n");

	    return NO_DATA_AVAILABLE;
	  }

	  if(gd.request_field_index < 0) return NO_DATA_AVAILABLE;
	  gd.found_field_index = gd.request_field_index;
	  gd.cur_f_head = gd.fhead[gd.request_field_index];
      break;

      case 2:
       // Check for the special '#' char - with directly indicates field number
       if(gd.req_field_name[0] == '#') {
	      gd.found_field_index = atoi(gd.req_field_name +1);
	      if(gd.found_field_index < 0 || 
	         gd.found_field_index >= gd.cur_m_head.n_fields) {
                  if (!gd.daemonize_flag) fprintf(stderr,
                     "Non-Existant field requested: %s\n",
		     gd.req_field_name);
	         return NO_DATA_AVAILABLE | NO_SUCH_FIELD;
	      }
	      gd.cur_f_head = gd.fhead[gd.found_field_index];

       } else {
	 gd.found_field_index = -1;
	 for(i=0; i < gd.cur_m_head.n_fields && gd.found_field_index < 0; i++) {
	     if(strncmp(gd.req_field_name,
		        gd.fhead[i]->fld_hdr->field_name,
                        MDV_SHORT_FIELD_LEN-1) == 0) {

		gd.found_field_index = i;
		gd.cur_f_head = gd.fhead[i];
	     }
	 }

	 if(gd.found_field_index < 0 || 
	    gd.found_field_index >= gd.cur_m_head.n_fields) {
             if (!gd.daemonize_flag) fprintf(stderr,
                "Non-Existant field requested: %s\n",
		gd.req_field_name);
	    return NO_DATA_AVAILABLE | NO_SUCH_FIELD;
	 }
	}
      break;
    }

  if (!gd.daemonize_flag)
     fprintf(stderr, "PID: %d Choosing Field %d\n", getpid(), gd.found_field_index);
    if (!gd.override_origin) {
        gd.origin_lon = gd.cur_f_head->fld_hdr->proj_origin_lon;
        gd.origin_lat = gd.cur_f_head->fld_hdr->proj_origin_lat;
    }

    gd.num_planes = gd.cur_f_head->fld_hdr->nz;

  return 0;
}

/************************************************************************
 * GET_DATA_FILENAME: Returns the name of the appropriate data file.  If
 *                    the data file isn't found in any of the given
 *                    directories, returns NULL.
 *
 * NOTE:  Returns a pointer to static memory.  This pointer should NOT be
 *        freed by the calling routine.
 */

char *get_data_filename(long time, int num_top_dirs,
			char **top_dir, char *suffix,
			time_t *update_time)
{
  static char path_name[1024];

  int i;
  char file_name[1024];
  char file_name2[1024];
  char format_str[32];
  struct tm *tm;
  struct stat file_stat;
  
  tm = gmtime(&time);
  strcpy(format_str, "%Y%m%d/%H%M%S."); /* Build the file name format string */
  strncat(format_str, suffix, 32);      /* Add the file suffix */


  strftime(file_name, 128, format_str, tm);  /* build the file name */

  /* Check in each directory until found */
  for (i = 0; i < num_top_dirs; i++)
  {
    sprintf(path_name, "%s/%s", top_dir[i],file_name);
    if (stat(path_name, &file_stat) == 0)
    {
      *update_time = file_stat.st_mtime;
      return(path_name);
    }

    /* Check for a compressed file too */
    STRncopy(file_name2,path_name,1024);
    strncat(file_name2,".Z",1024);
    if (stat(file_name2, &file_stat) == 0)
    {
      *update_time = file_stat.st_mtime;
      return(path_name);
    }

    /* Check for a gziped file too */
    STRncopy(file_name2,path_name,1024);
    strncat(file_name2,".gz",1024);
    if (stat(file_name2, &file_stat) == 0)
    {
      *update_time = file_stat.st_mtime;
      return(path_name);
    }


  } /* endfor - i */
    
  return(NULL);
}
