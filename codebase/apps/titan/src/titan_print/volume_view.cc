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
 * volume_view.c
 *
 * prints info about a radar volume scan file
 *
 * RAP, NCAR, Boulder CO
 *
 * December 1990
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "titan_print.h"

void volume_view(void)

{

  ui08 *bval;
  long iz, i;
  long ifield;
  long zero_count;
  long nfields, nplanes;
  vol_file_handle_t v_handle;

  /*
   * initialize volume file handle
   */

  RfInitVolFileHandle(&v_handle,
		      Glob->prog_name,
		      Glob->file_name,
		      (FILE *) NULL);

  /*
   * read in radar volume
   */

  if (RfReadVolume(&v_handle, "volume_view") != R_SUCCESS)
    exit(1);

  nfields = v_handle.vol_params->nfields;
  nplanes = v_handle.vol_params->cart.nz;

  /*
   * print out info
   */

  printf("\nRADAR VOLUME SCAN FILE INFORMATION\n\n");

  RfPrintVolParams(stdout, "  ", v_handle.vol_params);

  RfPrintRadarElevations(stdout, "  ",
			 "Elevations",
			 v_handle.vol_params->radar.nelevations,
			 v_handle.radar_elevations);

  RfPrintPlaneHeights(stdout, "  ", nplanes,
		      v_handle.plane_heights,
		      (double) v_handle.vol_params->cart.scalez);

  printf("  Field parameters\n");
  printf("\n");

  printf("  nfields : %ld\n\n", nfields);

  for (ifield = 0; ifield < nfields; ifield++) {
    
    RfPrintFieldParams(stdout, "    ",
		       ifield, v_handle.field_params[ifield],
		       v_handle.field_params_offset,
		       nplanes, v_handle.plane_offset);

    if (Glob->full) {

      for (iz = 0; iz < v_handle.vol_params->cart.nz; iz++) {

	zero_count = 0;
	printf("    Plane: %ld\n", iz);

	bval = v_handle.field_plane[ifield][iz];
	for (i = 0;
	     i < v_handle.vol_params->cart.ny * v_handle.vol_params->cart.nx;
	     i++, bval++) {
	  if (*bval == 0) {
	    zero_count++;
	  } else {
	    if (zero_count > 0) {
	      if (zero_count > 9999) {
		printf("%9ld*b000 ", zero_count);
	      } else {
		printf("%4ld*b000 ", zero_count);
	      }
	      zero_count = 0;
	    }
	    printf("b%.3d ",*bval);
	  }
	} /* i */
	
	if (zero_count > 0) {
	  if (zero_count > 9999) {
	    printf("%9ld*b000 ", zero_count);
	  } else {
	    printf("%4ld*b000 ", zero_count);
	  }
	}
	printf("\n");

      } /* iz */

    } /* if (Glob->full) */
	
  } /* ifield */

}



