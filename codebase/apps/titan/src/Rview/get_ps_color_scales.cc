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
 * get_ps_color_scales.c
 * 
 * reads a color table file for each field, and sets up GC's for 
 * each data value.
 *
 * RAP, NCAR, Boulder CO
 *
 * June 1990   
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "Rview.hh"
#include <cstring>
using namespace std;

void get_ps_color_scales()

{

  char *colorscale_dir;
  char colorscale_path[BUFSIZ];
  int ifield;
  Colormap cmap;

  if (Glob->verbose) {
    fprintf(stderr, "** get_color_scales **\n");
  }

  /*
   * get color map 
   */

  cmap = Glob->cmap;

  /*
   * get color scale directory
   */

  colorscale_dir = uGetParamString(Glob->prog_name,
				 "colorscale_dir", COLORSCALE_DIR);

  for (ifield = 0; ifield < Glob->nfields; ifield++) {

    /*
     * create file name
     */
    
    sprintf(colorscale_path, "%s%s%s",
	    colorscale_dir, PATH_DELIM,
	    Glob->fcontrol[ifield].ps_colorscale_name.c_str());

    /*
     * read color scale from file
     */

    if (GReadColorScale(colorscale_path,
			&Glob->fcontrol[ifield].pscolors) != 0) {
      
      fprintf(stderr, "ERROR - %s:get_ps_color_scales\n", Glob->prog_name);
      fprintf(stderr, "Color scale file %s invalid.\n",
	      colorscale_path);
      tidy_and_exit(1);
    }

    /*
     * load up color scale GC pointer array - this array is used
     * to point to the GC relevant for a given byte data value.
     * Note that dummy values of 0.0 and 1.0 are used for the scale
     * and bias - the correct scale and bias is returned with each
     * data plane, and the lookup table for the GCs is adjusted
     * using GAdjustGCScale()
     */

    if (GLoadPsgcScale(Glob->fcontrol[ifield].pscolors,
		       Glob->rdisplay, cmap,
		       1.0, 0.0) != 0) {

      fprintf(stderr, "ERROR - %s:get_ps_color_scales\n", Glob->prog_name);
      fprintf(stderr, "Loading psgc scale '%s' for field %d.\n",
	      colorscale_path, ifield);
      tidy_and_exit(1);

    }

  } /* ifield */

}
