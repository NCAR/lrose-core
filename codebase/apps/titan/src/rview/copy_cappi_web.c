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
/************************************************************************
 * copy_cappi_tiff: make a TIFF copy of a cappi
 *
 * copy routine
 * 
 * Nancy Rehak, RAP, NCAR, June 1990
 *************************************************************************/

#include "rview.h"

#include <X11/Xlib.h>

#include <rapplot/xutils.h>

void copy_cappi_web(void)
{
  static char *routine_name = "copy_cappi_web";
  
  static char *xwd_file_path;
  static char *web_file_path;
  static int first_time = TRUE;
  
  XWindowAttributes attributes;
  FILE *xwd_file;
  
  /*
   * Get the output filenames
   */

  if (first_time)
  {
    xwd_file_path = xGetResString(Glob->rdisplay, Glob->prog_name,
				  "cappi_xwd_file", CAPPI_XWD_FILE);
    
    web_file_path = xGetResString(Glob->rdisplay, Glob->prog_name,
				  "cappi_web_file", CAPPI_WEB_FILE);
    
    first_time = FALSE;
  }
  
  /*
   * Get the window attributes.
   */

  if (!XGetWindowAttributes(Glob->rdisplay,
			    Glob->main_window,
			    &attributes))
  {
    fprintf(stderr,
	    "ERROR - %s:%s\n",
	    Glob->prog_name, routine_name);
    fprintf(stderr,
	    "Cannot get X window attributes for creating XWD file.\n");
    return;
  }
      
  /*
   * Open the output file.
   */

  if ((xwd_file = fopen(xwd_file_path, "w")) == NULL)
  {
    fprintf(stderr,
	    "ERROR - %s:%s\n",
	    Glob->prog_name, routine_name);
    fprintf(stderr,
	    "Cannot open XWD file for output\n");
    perror(xwd_file_path);
    
    return;
  }
  
  /*
   * Copy the CAPPI to an xwd file
   */

  XUTIL_dump_pixmap_xwd(Glob->rdisplay, 
			Glob->zoom[Glob->zoom_level].pixmap,
			&attributes,
			xwd_file);
  
  /*
   * Close the output file.
   */

  fclose(xwd_file);
  
  /*
   * Convert to web format
   */

  convert_xwd_to_web(xwd_file_path,
		     web_file_path);
  
  return;

}
