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
/*********************************************************************
 * print_file.c
 *
 * prints info about a storm properties file
 *
 * RAP, NCAR, Boulder CO
 *
 * March 1991
 *
 * Mike Dixon
 *
 *********************************************************************/

#include <math.h>
#include "storm_file_v3_to_v5.h"

#define BOOL_STR(a) (a == 0? "false" : "true")

void print_file(storm_v3_file_index_t *v3_s_handle,
		char *file_path)
     
{

  char *file_label;

  si32 iscan, istorm;
  si32 n_scans;

  storm_v3_file_params_t *params;
  storm_v3_file_scan_header_t *scan;
  storm_v3_float_scan_header_t fl_scan;
  
  /*
   * check the file label
   */
  
  if (RfReadFileLabel(file_path, &file_label) != R_SUCCESS) {
    tidy_and_exit(-1);
  }
  
  if (strcmp(file_label, STORM_V3_HEADER_FILE_TYPE)) {
    fprintf(stderr, "File %s not storm file type v3\n",
	    Glob->prog_name);
    tidy_and_exit(-1);
  }
  
  /*
   * open storm properties files
   */
  
  if (Rfv3OpenStormFiles (v3_s_handle, "r",
			  file_path,
			  (char *) NULL,
			  "print_file")) {
    
    fprintf(stderr, "ERROR - %s:print_file\n", Glob->prog_name);
    fprintf(stderr, "Opening storm file.\n");
    tidy_and_exit(-1);

  }

  /*
   * read in storm properties file header
   */

  if (Rfv3ReadStormHeader(v3_s_handle, "print_file") != R_SUCCESS) {
    fprintf(stderr, "ERROR - %s:print_file\n", Glob->prog_name);
    fprintf(stderr, "Reading file %s.\n", file_path);
    tidy_and_exit(-1);
  }

  n_scans = v3_s_handle->header->n_scans;
  params = &v3_s_handle->header->params;
  
  /*
   * print out header
   */

  fprintf(stdout, "STORM FILE\n");
  fprintf(stdout, "==========\n");
  fprintf(stdout, "\n");

  fprintf(stdout, "Header file label : %s\n",
	  v3_s_handle->header_file_label);
  fprintf(stdout, "Data file label   : %s\n",
	  v3_s_handle->data_file_label);
  fprintf(stdout, "\n");
  
  Rfv3PrintStormHeader(stdout, "  ", v3_s_handle->header);

  /*
   * loop through scans
   */
  
  for (iscan = 0; iscan < n_scans; iscan++) {

    /*
     * read in scan info
     */

    if (Rfv3ReadStormScan(v3_s_handle, iscan, "print_file") != R_SUCCESS) {
      fprintf(stderr, "ERROR - %s:print_file\n", Glob->prog_name);
      fprintf(stderr, "Reading file %s.\n", file_path);
      tidy_and_exit(-1);
    }

    scan = v3_s_handle->scan;
    Rfv3DecodeStormScan(params, scan, &fl_scan);

    /*
     * print out v3_s_handle->scan info
     */

    Rfv3PrintStormScan(stdout, "    ", params, v3_s_handle->scan);
    
    for (istorm = 0; istorm < scan->nstorms; istorm++) {
	  
      if (Rfv3ReadStormProps(v3_s_handle, istorm,
			     "print_track") != R_SUCCESS)
	tidy_and_exit(-1);
      
      Rfv3PrintStormProps(stdout, "      ", params,
			  v3_s_handle->gprops + istorm);
      
      Rfv3PrintStormLayer(stdout, "      ", params,
			  v3_s_handle->scan,
			  v3_s_handle->gprops + istorm,
			  v3_s_handle->layer);
      
      Rfv3PrintStormHist(stdout, "      ", params,
			 v3_s_handle->gprops + istorm,
			 v3_s_handle->hist);
      
      
      Rfv3PrintStormRuns(stdout, "      ",
			 v3_s_handle->gprops + istorm,
			 v3_s_handle->runs);
      
    } /* istorm */
    
  } /* iscan */
  
  /*
   * close files
   */

  Rfv3CloseStormFiles(v3_s_handle, "print_file");
  
}
