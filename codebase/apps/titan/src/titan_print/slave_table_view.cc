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
 * slave_table_view.c
 *
 * prints info about a radar to cartesian slave lookup table file
 *
 * RAP, NCAR, Boulder CO
 *
 * December 1990
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "titan_print.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

#define BOOL_STR(a) (((a) == 0)? "false" : "true")

void slave_table_view(void)

{

  long iaz, ielev;

  slave_table_index_t *t_handle;
  slave_table_file_handle_t slave_handle;
  rc_table_params_t *tparams;

  struct tm *time_struct;
  struct stat file_stat;

  /*
   * initialize slave_table file handle
   */

  RfInitSlaveTableHandle(&slave_handle,
			 Glob->prog_name,
			 Glob->file_name,
			 (FILE *) NULL);

  /*
   * read in the radar-to-cart table
   */

  if (RfReadSlaveTable(&slave_handle,
		       "slave_table_view") != R_SUCCESS)
    exit(1);

  tparams = slave_handle.table_params;

  /*
   * get the file status to access last modified time
   */

  stat(Glob->file_name, &file_stat);
  time_struct = localtime(&file_stat.st_mtime);

  /*
   * print out info
   */

  printf("\nRADAR TO CARTESIAN SLAVE TABLE FILE INFORMATION\n\n");

  printf("File last modified at time : %.4d/%.2d/%.2d %.2d:%.2d:%.2d\n\n",
	 time_struct->tm_year + 1900,
	 time_struct->tm_mon + 1,
	 time_struct->tm_mday,
	 time_struct->tm_hour,
	 time_struct->tm_min,
	 time_struct->tm_sec);
	 
  printf("Radar to cartesian table parameters : \n\n");

  printf("use_azimuth_table : %s\n",
	 BOOL_STR(tparams->use_azimuth_table));
  printf("extend_below : %s\n",
	 BOOL_STR(tparams->extend_below));

  printf("nelevations : %d\n", tparams->nelevations);
  printf("nazimuths : %d\n", tparams->nazimuths);
  printf("ngates : %d\n", tparams->ngates);
  printf("nbeams_vol : %d\n", tparams->nbeams_vol);

  printf("start_range (meters) : %g\n",
	 (float) tparams->start_range / 1000.0);
  printf("gate_spacing (meters) : %g\n",
	 (float) tparams->gate_spacing / 1000.0);

  printf("delta_azimuth (deg) : %g\n",
	 (float) tparams->delta_azimuth / DEG_FACTOR);
  printf("start_azimuth (deg) : %g\n",
	 (float) tparams->start_azimuth / DEG_FACTOR);
  printf("beam_width (deg) : %g\n\n",
	 (float) tparams->beam_width / DEG_FACTOR);

  printf("radar latitude : %g\n",
	 (double) tparams->radar_latitude / DEG_FACTOR);
  printf("radar longitude : %g\n",
	 (double) tparams->radar_longitude / DEG_FACTOR);

  printf("ndata - number of points in cartesian grid : %d\n",
	 tparams->ndata);
  printf("nlist - size of table list : %d\n\n", tparams->nlist);

  printf("file index offset in bytes : %d\n\n", tparams->index_offset);
  printf("file list offset in bytes  : %d\n\n", tparams->list_offset);

  RfPrintCartParams(stdout, "  ", &tparams->cart);
  
  RfPrintScanTable(stdout, "    ", slave_handle.scan_table);

  RfPrintPlaneHeights(stdout, "  ",
		      tparams->cart.nz,
		      slave_handle.plane_heights,
		      (double) tparams->cart.scalez);

  if (!Glob->summary) {

    for (ielev = 0; ielev < tparams->nelevations; ielev++) {

      for (iaz = 0; iaz < tparams->nazimuths; iaz++) {

	t_handle = slave_handle.table_index[ielev] + iaz;

	printf("  ielev, iaz, npoints: "
	       "%ld, %ld, %d\n", ielev, iaz, t_handle->npoints);

	if (Glob->full) {
	  for (unsigned int ipoint = 0; ipoint < t_handle->npoints; ipoint++) {
	    printf("    index: %d\n", t_handle->u.index[ipoint]);
	  } /* ipoint */
	} /* if (Glob->full)*/

      } /* iaz */

    } /* ielev */

  } /* if (!Glob->summary) */

  RfFreeSlaveTable(&slave_handle, "slave_table_view");

}
