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
 * clutter_view.c
 *
 * prints info about clutter table file
 *
 * RAP, NCAR, Boulder CO
 *
 * December 1990
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "titan_print.h"

void clutter_view(void)

{

  int ielev, iaz;

  clutter_table_file_handle_t clutter_handle;
  clutter_table_params_t *tparams;
  clutter_table_index_t **table_index;
  clutter_table_entry_t *entry;

  /*
   * initialize clutter_table file handle
   */

  RfInitClutterHandle(&clutter_handle,
		      Glob->prog_name,
		      Glob->file_name,
		      (FILE *) NULL);

  /*
   * read in the radar-to-cart table
   */

  if (RfReadClutterTable(&clutter_handle,
			 "clutter_table_view") != R_SUCCESS)
    exit(1);

  tparams = clutter_handle.table_params;

  /*
   * print out info
   */

  printf("\nCLUTTER TABLE FILE INFORMATION\n\n");

  fprintf(stdout, "Dates and times : \n");
  fprintf(stdout, "  File:  %s\n", utimstr(tparams->file_time));
  fprintf(stdout, "  Start: %s\n", utimstr(tparams->start_time));
  fprintf(stdout, "  Mid:   %s\n", utimstr(tparams->mid_time));
  fprintf(stdout, "  End:   %s\n", utimstr(tparams->end_time));
  fprintf(stdout, "\n");

  printf("\nTable parameters : \n\n");

  printf("Dbz scale : %g\n\n",
	 (float) tparams->dbz_scale /
	 (float) tparams->factor);
  printf("Dbz bias  : %g\n\n",
	 (float) tparams->dbz_bias /
	 (float) tparams->factor);
  printf("Dbz margin : %g\n\n",
	 (float) tparams->dbz_margin /
	 (float) tparams->factor);

  printf("nlist - size of clutter list : %d\n\n", tparams->nlist);

  printf("Radar-to-cart table data\n");
  printf("nelevations : %d\n", tparams->rc_params.nelevations);
  printf("nazimuths : %d\n", tparams->rc_params.nazimuths);
  printf("ngates : %d\n", tparams->rc_params.ngates);
  printf("ndata - number of points in cartesian grid : %d\n",
	 tparams->rc_params.ndata);
  printf("nlist - size of rc table list : %d\n\n",
	 tparams->rc_params.nlist);
  printf("file index offset in bytes : %d\n\n",
	 tparams->rc_params.index_offset);
  printf("file list offset in bytes  : %d\n\n",
	 tparams->rc_params.list_offset);

  printf("gate_spacing (meters) : %g\n",
	 (float) tparams->rc_params.gate_spacing / 1000.0);
  printf("start_range (meters) : %g\n",
	 (float) tparams->rc_params.start_range / 1000.0);
  printf("delta_azimuth (deg) : %g\n",
	 (float) tparams->rc_params.delta_azimuth / DEG_FACTOR);
  printf("start_azimuth (deg) : %g\n",
	 (float) tparams->rc_params.start_azimuth / DEG_FACTOR);
  printf("beam_width (deg) : %g\n\n",
	 (float) tparams->rc_params.beam_width / DEG_FACTOR);

  RfPrintCartParams(stdout, "  ", &tparams->rc_params.cart);
  
  RfPrintRadarElevations(stdout, "  ",
			 "Elevations",
			 tparams->rc_params.nelevations,
			 clutter_handle.radar_elevations);

  RfPrintPlaneHeights(stdout, "  ",
		      tparams->rc_params.cart.nz,
		      clutter_handle.plane_heights,
		      (double) tparams->rc_params.cart.scalez);

  fprintf(stdout, "  Table data:\n");

  table_index = clutter_handle.table_index;
  
  for (ielev = 0; ielev < tparams->rc_params.nelevations; ielev++) {
    
    for (iaz = 0; iaz < tparams->rc_params.nazimuths; iaz++) {
      
      /*
       * the fields in the index
       */
      
      fprintf(stdout,
	      "    ielev, iaz, nclut_points: "
	      "%d, %d, %d\n", ielev, iaz,
	      (int) table_index[ielev][iaz].nclut_points);
      
      /*
       * the fields in the list
       */
      
      fprintf(stdout, "      Points as (dbz, ipoint, cart_index): ");
      
      for (unsigned int i = 0; i < table_index[ielev][iaz].nclut_points; i++) {
	
        entry = table_index[ielev][iaz].u.entry + i;

	fprintf(stdout, "(%d,%d,%d)", (int) entry->dbz,
		(int) entry->ipoint, (int) entry->cart_index);
	
      } /* i */
      
      fprintf(stdout, "\n");

    } /* iaz */
    
  } /* ielev */
  
  RfFreeClutterTable(&clutter_handle, "clutter_table_view");

}

