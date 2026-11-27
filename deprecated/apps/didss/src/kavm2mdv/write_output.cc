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
 * write_output.cc
 *
 * writes the output file
 *
 * RAP, NCAR, Boulder CO
 *
 * January 1997
 *
 * Nancy Rehak
 *
 * Based on write_output.c by Mike Dixon
 *
 *********************************************************************/

#include <sys/stat.h>

#include "kavm2mdv.h"
using namespace std;

void write_output(vol_file_handle_t *v_handle)
{
  char dir_path[MAX_PATH_LEN];
  char file_path[MAX_PATH_LEN];
  char call_str[BUFSIZ];
  int write_current_index;
  si32 nbytes;
  cart_params_t *cart;
  struct stat dir_stat;
  FILE *out;

  if (Glob->params.mode == REALTIME)
    write_current_index = TRUE;
  else
    write_current_index = FALSE;
    
  switch(Glob->params.output_type)
  {
  case DOBSON :
  case MDV :
    
    if (RfWriteDobsonRemote(v_handle,
			    write_current_index,
			    Glob->params.debug,
			    Glob->params.output_host,
			    Glob->params.output_dir,
			    Glob->params.output_file_ext,
			    Glob->params.local_tmp_dir,
			    Glob->prog_name,
			    "write_output") != R_SUCCESS)
    {
      fprintf(stderr, "ERROR - %s:process_file\n", Glob->prog_name);
      fprintf(stderr, "Cannot write output file\n");
    }

    break;
    
  case PLAIN :
    
    /*
     * compute output dir path
     */
    
    sprintf(dir_path, "%s%s%.4d%.2d%.2d",
	    Glob->params.output_dir, PATH_DELIM,
	    v_handle->vol_params->mid_time.year,
	    v_handle->vol_params->mid_time.month,
	    v_handle->vol_params->mid_time.day);
    
    /*
     * create directory, if needed
     */
    
    if (0 != stat(dir_path, &dir_stat))
    {
      if (mkdir(dir_path, 0755))
      {
	fprintf(stderr, "ERROR - %s:write_output\n", Glob->prog_name);
	fprintf(stderr, "Trying to make output dir\n");
	perror(dir_path);
	return;
      }
    }
    
    /*
     * create output file path
     */
    
    sprintf(file_path, "%s%s%.2d%.2d%.2d.%s",
	    dir_path, PATH_DELIM,
	    v_handle->vol_params->mid_time.hour,
	    v_handle->vol_params->mid_time.min,
	    v_handle->vol_params->mid_time.sec,
	    Glob->params.output_file_ext);
    
    /*
     * open file
     */

    if ((out = fopen(file_path, "w")) == NULL)
    {
      fprintf(stderr, "ERROR - %s:process_file\n", Glob->prog_name);
      fprintf(stderr, "Cannot open output file for writing\n");
      perror(file_path);
      return;
    }

    cart = &v_handle->vol_params->cart;
    nbytes = cart->nx * cart->ny;

    /*
     * write grid - for fortran output, write array size
     * at start and end of buffer
     */

    if (Glob->params.fortran_output)
    {
      if (ufwrite((char *) &nbytes,
		  (int) sizeof(si32), 1, out) != 1)
      {
	fprintf(stderr, "ERROR - %s:write_output\n", Glob->prog_name);
	fprintf(stderr, "Cannot complete write to output file\n");
	perror(file_path);
	fclose(out);
	return;
      }
    }
  
    if (fwrite((char *) v_handle->field_plane[0][0],
	       1, nbytes, out) != (size_t) nbytes)
    {
      fprintf(stderr, "ERROR - %s:process_file\n", Glob->prog_name);
      fprintf(stderr, "Cannot write %ld bytes to output file\n",
	      (long) nbytes);
      perror(file_path);
    }

    if (Glob->params.fortran_output)
    {
      if (ufwrite((char *) &nbytes,
		  (int) sizeof(si32), 1, out) != 1)
      {
	fprintf(stderr, "ERROR - %s:write_output\n", Glob->prog_name);
	fprintf(stderr, "Cannot complete write to output file\n");
	perror(file_path);
	fclose(out);
	return;
      }
    }
  
    fclose(out);

    /*
     * compress file
     */

    sprintf(call_str, "compress %s", file_path);
    system(call_str);
    
    break;
  } /* endswitch - Glob->params.output_type */
  
  return;
  
}

