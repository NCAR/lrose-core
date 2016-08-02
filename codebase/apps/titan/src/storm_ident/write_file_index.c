/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/* ** Copyright UCAR (c) 1990 - 2016                                         */
/* ** University Corporation for Atmospheric Research (UCAR)                 */
/* ** National Center for Atmospheric Research (NCAR)                        */
/* ** Boulder, Colorado, USA                                                 */
/* ** BSD licence applies - redistribution and use in source and binary      */
/* ** forms, with or without modification, are permitted provided that       */
/* ** the following conditions are met:                                      */
/* ** 1) If the software is modified to produce derivative works,            */
/* ** such modified software should be clearly marked, so as not             */
/* ** to confuse it with the version available from UCAR.                    */
/* ** 2) Redistributions of source code must retain the above copyright      */
/* ** notice, this list of conditions and the following disclaimer.          */
/* ** 3) Redistributions in binary form must reproduce the above copyright   */
/* ** notice, this list of conditions and the following disclaimer in the    */
/* ** documentation and/or other materials provided with the distribution.   */
/* ** 4) Neither the name of UCAR nor the names of its contributors,         */
/* ** if any, may be used to endorse or promote products derived from        */
/* ** this software without specific prior written permission.               */
/* ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  */
/* ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      */
/* ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    */
/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/***********************************************************************
 * write_file_index()
 *
 * writes the current file handle - this small file indicates the latest 
 * time of the data which has been written
 *
 * RAP, NCAR, Boulder, CO, USA
 *
 * October 1994
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "storm_ident.h"
#include <toolsa/ldata_info.h>

void write_file_index(storm_file_handle_t *s_handle,
		      char *storm_header_file_path)

{

  static int first_call = TRUE;
  static LDATA_handle_t ldata;

  path_parts_t parts;

  if (first_call) {
    LDATA_init_handle(&ldata, Glob->prog_name, FALSE);
    first_call = FALSE;
  }
  
  /*
   * parse storm file path to get base part of path
   * (i.e. name without extension)
   */
  
  uparse_path(storm_header_file_path, &parts);

  /*
   * write info
   */

  if (LDATA_info_write(&ldata,
		       Glob->params.storm_data_dir,
		       s_handle->header->end_time,
		       STORM_HEADER_FILE_EXT,
		       parts.base, NULL, 0, NULL)) {
    
    fprintf(stderr, "WARNING - %s:write_file_index\n",
	    Glob->prog_name);
    fprintf(stderr, "Cannot write index file to dir %s\n",
	    Glob->params.storm_data_dir);

  }

  /*
   * free path
   */

  ufree_parsed_path(&parts);

  return;
  
}


