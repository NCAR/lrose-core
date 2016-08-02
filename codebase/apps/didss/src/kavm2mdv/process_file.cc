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
 * process_file.cc
 *
 * process the kav file
 *
 * RAP, NCAR, Boulder CO
 *
 * January 1997
 *
 * Nancy Rehak
 *
 * Based on process_file.c by Mike Dixon
 *
 *********************************************************************/

#include <sys/stat.h>

#include <rapformats/km.h>
#include <toolsa/globals.h>

#include "kavm2mdv.h"
using namespace std;

#define TIME_STR_LEN 12
#define MON_STR_LEN 3

void process_file(vol_file_handle_t *v_handle, char *input_file_path)
{
  static char *routine_name = "process_file";
  static char pmu_message[BUFSIZ];
  
  struct stat file_stat;
  static char *file_buf = NULL;
  int file_size;
  FILE *in_file;
  KM_header_t header;

  /*
   * timeout - register process
   */
  
  sprintf(pmu_message, "Processing file <%s>", input_file_path);
  PMU_auto_register(pmu_message);
      
  if (Glob->params.debug >= DEBUG_NORM)
    fprintf(stderr, "Processing file %s\n", input_file_path);

  /*
   * get file size
   */

  if (0 != stat(input_file_path, &file_stat))
  {
    fprintf(stderr, "ERROR - %s:process_file\n", Glob->prog_name);
    fprintf(stderr, "cant stat kavouras file\n");
    perror(input_file_path);
    return;
  }

  file_size = file_stat.st_size;

  /*
   * allocate buffer
   */

  if (file_buf == NULL)
  {
    file_buf = (char *) umalloc((ui32) file_size);
  }
  else
  {
    file_buf = (char *) urealloc((char *) file_buf,
				 (ui32) file_size);
  }

  /*
   * open file and read in
   */

  if ((in_file = fopen(input_file_path, "r")) == NULL)
  {
    fprintf(stderr, "ERROR - %s:process_file\n", Glob->prog_name);
    fprintf(stderr, "cant open kavouras file\n");
    perror(input_file_path);
    return;
  }
  
  if (ufread(file_buf, 1, file_size, in_file) != file_size)
  {
    fprintf(stderr, "ERROR - %s:process_file\n", Glob->prog_name);
    fprintf(stderr, "cant read in kavouras file\n");
    perror(input_file_path);
    fclose(in_file);
    return;
  }

  fclose(in_file);

  /*
   * decode header
   */

  PMU_auto_register(pmu_message);
  KM_decode_header(file_buf, file_size, &header, Glob->params.debug);

  /*
   * There has been some problem with the day in the header not being
   * changed at 0000Z as it should be.  If we are running in realtime
   * mode, compare the file time with the time in the header and make
   * sure they aren't too far off.
   */

  if (Glob->params.mode == REALTIME)
    KM_fix_header_time(file_stat.st_mtime, &header);
  else if (Glob->params.update_archive_date)
  {
    fprintf(stderr,
	    "WARNING: %s: Adding a day to the Kavouras file header.\n",
	    routine_name);
    KM_set_header_time(header.time.unix_time + SECS_IN_DAY,
		       &header);
  }
  
  /*
   * set vol index time
   */

  set_vol_time(v_handle, &header);

  /*
   * load data grid
   */

  PMU_auto_register(pmu_message);
  load_data_grid(v_handle, file_buf, file_size);

  /*
   * write output file
   */

  PMU_auto_register(pmu_message);
  write_output(v_handle);

  return;

}

