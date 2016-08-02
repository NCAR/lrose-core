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
 * storm_view.c
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
#include "titan_print.h"

#define BOOL_STR(a) (a == 0? "false" : "true")

void storm_view(void)

{

  long iscan, istorm;
  long n_scans;

  storm_file_handle_t s_handle;
  storm_file_params_t *params;
  storm_file_scan_header_t *scan;
  
  /*
   * initialize
   */

  RfInitStormFileHandle(&s_handle, Glob->prog_name);

  /*
   * open storm properties files
   */
  
  if (RfOpenStormFiles (&s_handle, "r",
			Glob->file_name,
			(char *) NULL,
			"storm_view")) {
    
    fprintf(stderr, "ERROR - %s:storm_view\n", Glob->prog_name);
    fprintf(stderr, "Opening storm file.\n");
    exit(-1);

  }

  /*
   * read in storm properties file header
   */

  if (RfReadStormHeader(&s_handle, "storm_view") != R_SUCCESS) {
    fprintf(stderr, "ERROR - %s:storm_view\n", Glob->prog_name);
    fprintf(stderr, "Reading file %s.\n", Glob->file_name);
    exit(1);
  }

  n_scans = s_handle.header->n_scans;
  params = &s_handle.header->params;
  
  /*
   * print out header
   */

  fprintf(stdout, "STORM FILE\n");
  fprintf(stdout, "==========\n");
  fprintf(stdout, "\n");

  fprintf(stdout, "Header file label : %s\n",
	  s_handle.header_file_label);
  fprintf(stdout, "Data file label   : %s\n",
	  s_handle.data_file_label);
  fprintf(stdout, "\n");
  
  RfPrintStormHeader(stdout, "  ", s_handle.header);

  /*
   * loop through scans
   */
  
  for (iscan = 0; iscan < n_scans; iscan++) {

    /*
     * read in scan info
     */

    if (RfReadStormScan(&s_handle, iscan, "storm_view") != R_SUCCESS) {
      fprintf(stderr, "ERROR - %s:storm_view\n", Glob->prog_name);
      fprintf(stderr, "Reading file %s.\n", Glob->file_name);
      exit(1);
    }

    scan = s_handle.scan;
    
    /*
     * print out s_handle.scan info
     */

    if (Glob->summary) {

      printf("Scan, time, nstorms : %4d %s %4d\n",
	     s_handle.scan->scan_num,
	     utimstr(s_handle.scan->time),
	     s_handle.scan->nstorms);

    } else {

      RfPrintStormScan(stdout, "    ", params, s_handle.scan);
      
      if (Glob->full) {
	
	for (istorm = 0; istorm < scan->nstorms; istorm++) {
	  
	  if (RfReadStormProps(&s_handle, istorm,
			       "print_track") != R_SUCCESS)
	    exit(1);
	  
	  RfPrintStormProps(stdout, "      ", params,
			    s_handle.scan,
			    s_handle.gprops + istorm);
	  
	  RfPrintStormLayer(stdout, "      ", params,
			    s_handle.scan,
			    s_handle.gprops + istorm,
			    s_handle.layer);
	  
	  RfPrintStormHist(stdout, "      ", params,
			   s_handle.gprops + istorm,
			   s_handle.hist);
	  
	  
	  RfPrintStormRuns(stdout, "      ",
			   s_handle.gprops + istorm,
			   s_handle.runs);
	  
	  RfPrintStormProjRuns(stdout, "      ",
			       s_handle.gprops + istorm,
			       s_handle.proj_runs);
	  
	} /* istorm */

      } /* if (Glob->full) */

    } /* if (Glob->summary) */

  } /* iscan */
  
  /*
   * close files
   */

  RfCloseStormFiles(&s_handle, "storm_view");
  RfFreeStormScan(&s_handle, "storm_view");
  RfFreeStormHeader(&s_handle, "storm_view");
  RfFreeStormFileHandle(&s_handle);
  
}
