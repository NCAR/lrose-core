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
/****************************************************************
 * draw_verify.c
 *
 * module for reading in and displaying verification data
 *
 * Mike Dixon, RAP, NCAR, Boulder, CO, USA
 *
 * May 1993
 *
 *****************************************************************/

#include "rview.h"

typedef enum {ALGORITHM, VERIFY} report_enum ;

typedef struct {

  date_time_t time;
  double lat, lon;
  char id[256];
  int hail;
  int rain;
  report_enum report_type;
  double u, v;
  char color[256];
  double x, y;

} verify_t;

/*****************************************************************
 * draw_verify()
 */

void draw_verify(int dev,
		 gframe_t *frame,
		 date_time_t *cappi_time)

{

  static int first_call = TRUE;
  static char *verify_file_path;
  static char *plot_verify;
  static char *past_storm_color;
  static si32 verify_time_margin;
  static GC verify_gc;
  static psgc_t *verify_psgc;
  static Colormap cmap;

  char line[BUFSIZ];
  char time_str[32];
  int report_type;
  FILE *verify_file;
  verify_t verify;

  if (Glob->debug) {
    fprintf(stderr, "** draw_verify **\n");
  }

  /*
   * initialize
   */
  
  if (first_call) {
    
    cmap = Glob->cmap;

    plot_verify =
      xGetResString(Glob->rdisplay, Glob->prog_name,
		    "plot_verify", "false");
    
    verify_file_path =
      xGetResString(Glob->rdisplay, Glob->prog_name,
		    "verify_file", "null");
    
    verify_time_margin =
      xGetResLong(Glob->rdisplay, Glob->prog_name,
		  "verify_time_margin", 180);
    
    past_storm_color =
      xGetResString(Glob->rdisplay, Glob->prog_name,
		    "x_past_storm_color",
		    X_PAST_STORM_COLOR);
    
    verify_psgc = &Glob->past_storm_psgc[0];
    
    first_call = FALSE;
    
  } /* if (first_call) */

  /*
   * return now if verification plot not needed
   */
  
  if (strcmp(plot_verify, "true"))
    return;

  /*
   * open verify file
   */
  
  if ((verify_file = fopen(verify_file_path, "r")) == NULL) {
    
    fprintf(stderr, "%s:draw_verify\n", Glob->prog_name);
    fprintf(stderr, "Cannot open verify file for reading\n");
    perror(verify_file_path);
    return;
    
  }
  
  /*
   * rewind the verify file
   */
  
  fseek(verify_file, 0L, 0);
  clearerr(verify_file);

  /*
   * save postscript state
   */
  
  if (dev == PSDEV)
    PsGsave(frame->psgc->file);

  /*
   * read in tracks
   */

  while (!feof(verify_file)) {
    
    if (fgets(line, BUFSIZ, verify_file) != NULL) {

      if (sscanf(line, "%d%d%d%d%d%d%lg%lg%s%d%d%d%lg%lg%s",
		 &verify.time.year,
		 &verify.time.month,
		 &verify.time.day,
		 &verify.time.hour,
		 &verify.time.min,
		 &verify.time.sec,
		 &verify.lat,
		 &verify.lon,
		 verify.id,
		 &verify.hail,
		 &verify.rain,
		 &report_type,
		 &verify.u,
		 &verify.v,
		 verify.color) != 15)
	continue;

      verify.report_type = (report_enum) report_type;

      uconvert_to_utime(&verify.time);

      sprintf(time_str, "%2d:%2d",
	      verify.time.min,
	      verify.time.sec);

      
      if (verify.time.unix_time >=
	  cappi_time->unix_time - verify_time_margin &&
	  verify.time.unix_time <=
	  cappi_time->unix_time + verify_time_margin) {
	
	/*
	 * set color and linestyle
	 */
	
	verify_gc = xGetColorGC(Glob->rdisplay, cmap,
				&Glob->color_index, verify.color);

	if (dev == PSDEV) {
	  PsSetLineStyle(frame->psgc->file, verify_psgc);
	  verify_psgc->file = frame->psgc->file;
	}

	/*
	 * compute (x, y) relative to grid
	 */

	TITAN_latlon2xy(&Glob->grid_comps, verify.lat, verify.lon,
		      &verify.x, &verify.y);

	if (verify.hail == 0 && verify.rain == 0) {

	  if (!(verify.id[0] == 'N')) {

	    /*
	     * plot car
	     */

	    GDrawLine(dev, 
		      frame, 
		      verify_gc, 
		      verify_psgc, 
		      verify.x - 0.5,
		      verify.y,
		      verify.x + 0.5,
		      verify.y);
	    
	    GDrawLine(dev, 
		      frame, 
		      verify_gc, 
		      verify_psgc, 
		      verify.x,
		      verify.y - 0.5,
		      verify.x,
		      verify.y + 0.5);

	  }

	} else {

	  /*
	   * draw in cross-hair at verify location
	   */
	
	  GDrawLine(dev, 
		    frame, 
		    verify_gc, 
		    verify_psgc, 
		    verify.x - 0.5,
		    verify.y,
		    verify.x + 0.5,
		    verify.y);
	  
	  GDrawLine(dev, 
		    frame, 
		    verify_gc, 
		    verify_psgc, 
		    verify.x,
		    verify.y - 0.5,
		    verify.x,
		    verify.y + 0.5);
	  
	  /*
	   * id
	   */
	  
	  XSetFont(Glob->rdisplay, verify_gc, Glob->x_ticklabel_font->fid);
	  
	  GDrawString(dev, frame, verify_gc,
		      Glob->x_ticklabel_font,
		      frame->psgc,
		      XJ_LEFT,
		      YJ_ABOVE,
		      verify.x + 0.1,
		      verify.y + 0.1,
		      verify.id);
	  
	  /*
	   * time
	   */
	  
	  GDrawString(dev, frame, verify_gc,
		      Glob->x_ticklabel_font,
		      frame->psgc,
		      XJ_LEFT,
		      YJ_BELOW,
		      verify.x + 0.1,
		      verify.y - 0.1,
		      time_str);

	}  /* f (verify.hail == 0 && verify.rain == 0) */

      } /* if (verify.time ... */

    } /* if (fgets ... */

  } /* while (!feof ... */
  
  /*
   * restore postscript
   */

  if (dev == PSDEV)
    PsGrestore(frame->psgc->file);
  
  /*
   * close file
   */

  fclose(verify_file);

}

