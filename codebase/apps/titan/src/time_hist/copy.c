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
 * copy.c
 *
 * Copy routines - for postscript output
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * April 1991
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "time_hist.h"

/*********************************************************************
 * copy_thist()
 *
 * make postscript file for time height profile
 */

void copy_thist(void)

{
  
  char *prologue_file_path, *ps_file_path;
  char line[BUFSIZ];

  int title_x, title_y;
  int plot_x, plot_y;

  ui32 title_width, title_height;
  ui32 plot_width, plot_height;
  ui32 title_to_plot_margin;  

  double ps_total_width, ps_total_height;

  FILE *prologue_file, *ps_file;

  if (Glob->debug) {
    fprintf(stderr, "** copy_thist **\n");
  }

  /*
   * open prologue file
   */

  prologue_file_path = xGetResString(Glob->rdisplay, Glob->prog_name,
				     "ps_prologue_file",
				     PS_PROLOGUE_FILE);

  if ((prologue_file = fopen(prologue_file_path, "r")) == NULL) {
    fprintf(stderr, "ERROR - %s:copy_thist\n", Glob->prog_name);
    fprintf(stderr, "Opening ps prologue file - ");
    perror(prologue_file_path);
    tidy_and_exit(1);
  }
  
  /*
   * open ps file
   */

  ps_file_path = xGetResString(Glob->rdisplay, Glob->prog_name,
			       "output_file", OUTPUT_FILE);

  if ((ps_file = fopen(ps_file_path, "w")) == NULL)
    {
      fprintf(stderr, "ERROR - %s:copy_thist\n", Glob->prog_name);
      fprintf(stderr, "Creating ps output file - ");
      perror(ps_file_path);
      tidy_and_exit(1);
    }

  /*
   * copy prologue file to ps file
   */

  while (fgets(line, BUFSIZ, prologue_file) != NULL) 
    fprintf(ps_file, "%s", line);

  /*
   * close the prologue file
   */
  
  fclose(prologue_file);

  /*
   * compute plot area positions and sizes
   */
  
  title_to_plot_margin = (ui32) (Glob->ps_title_to_plot_margin + 0.5);

  plot_width = (ui32) (Glob->ps_plot_width + 0.5);
  plot_height = (ui32) (Glob->ps_plot_height + 0.5);
  
  title_width = plot_width;
  title_height = (ui32) (Glob->ps_title_height + 0.5);
  
  ps_total_width = (double) title_width / Glob->ps_unitscale;
  ps_total_height = (double) (title_height + title_to_plot_margin +
				    plot_height) / Glob->ps_unitscale;
  
  title_x = 0;
  title_y = 0;
  
  plot_x = title_x;
  plot_y = title_y + title_height + title_to_plot_margin;
  
  /*
   * set up frames
   */
  
    
  GPsInitFrame(Glob->thist_title_frame->psgc, ps_file, Glob->ps_fontname,
	       Glob->ps_title_fontsize, 1L);
    
  GPsInitFrame(Glob->thist_plot_frame->psgc, ps_file, Glob->ps_fontname,
	       Glob->ps_ticklabel_fontsize, 1L);
    
  GPsSetGeomFrame(Glob->thist_title_frame, title_x, title_y,
		  title_width, title_height);
  
  GPsSetGeomFrame(Glob->thist_plot_frame, plot_x, plot_y,
		  plot_width, plot_height);
  
  GPsSetGeomFrame(Glob->vert_page_frame, 0, 0,
		  (ui32) (Glob->ps_page_width * Glob->ps_unitscale + 0.5),
		  (ui32) (Glob->ps_page_length * Glob->ps_unitscale + 0.5));

  GPsSetGeomFrame(Glob->horiz_page_frame, 0, 0,
		  (ui32) (Glob->ps_page_length * Glob->ps_unitscale + 0.5),
		  (ui32) (Glob->ps_page_width * Glob->ps_unitscale + 0.5));

  /*
   * set up graphics page 
   */

  GPageSetup(PSDEV,
	     Glob->vert_page_frame,
	     Glob->horiz_page_frame,
	     Glob->print_width,
	     ps_total_width,
	     ps_total_height,
	     Glob->ps_page_width,
	     Glob->ps_page_length,
	     Glob->ps_width_margin,
	     Glob->ps_length_margin,
	     ps_file);

  /*
   * set font for title window
   */

  PsSetFont(Glob->thist_title_frame->psgc->file,
	    Glob->thist_title_frame->psgc->fontname,
	    Glob->thist_title_frame->psgc->fontsize);

  /*
   * copy title window
   */

  if (Glob->draw_copy_title)
    draw_thist_title(PSDEV);
  
  /*
   * set font for plot window
   */

  PsSetFont(Glob->thist_plot_frame->psgc->file,
	    Glob->thist_plot_frame->psgc->fontname,
	    Glob->thist_plot_frame->psgc->fontsize);

  /*
   * draw plot
   */

  draw_thist_plot(PSDEV);

  /*
   * show page and close the ps file
   */

  PsShowPage(ps_file);
  fclose(ps_file);

  /*
   * print ps file
   */

  print_copy(ps_file_path);

  return;

}

/*********************************************************************
 * copy_timeht()
 *
 * make postscript file for time height profile
 */

void copy_timeht(g_color_scale_t *color_scale)

{
  
  char *prologue_file_path, *ps_file_path;
  char line[BUFSIZ];

  int i;
  int title_x, title_y;
  int scale_x, scale_y;
  int plot_x, plot_y;

  ui32 title_width, title_height;
  ui32 scale_width, scale_height;
  ui32 plot_width, plot_height;
  ui32 plot_to_scale_margin, title_to_plot_margin;  

  double ps_total_width, ps_total_height;

  FILE *prologue_file, *ps_file;

  /*
   * open prologue file
   */

  prologue_file_path = xGetResString(Glob->rdisplay, Glob->prog_name,
				     "ps_prologue_file",
				     PS_PROLOGUE_FILE);

  if ((prologue_file = fopen(prologue_file_path, "r")) == NULL) {
    fprintf(stderr, "ERROR - %s:copy_timeht\n", Glob->prog_name);
    fprintf(stderr, "Opening ps prologue file - ");
    perror(prologue_file_path);
    tidy_and_exit(1);
  }
  
  /*
   * open ps file
   */

  ps_file_path = xGetResString(Glob->rdisplay, Glob->prog_name,
			       "output_file", OUTPUT_FILE);

  if ((ps_file = fopen(ps_file_path, "w")) == NULL)
    {
      fprintf(stderr, "ERROR - %s:copy_timeht\n", Glob->prog_name);
      fprintf(stderr, "Creating ps output file - ");
      perror(ps_file_path);
      tidy_and_exit(1);
    }

  /*
   * copy prologue file to ps file
   */

  while (fgets(line, BUFSIZ, prologue_file) != NULL) 
    fprintf(ps_file, "%s", line);

  /*
   * close the prologue file
   */
  
  fclose(prologue_file);

  /*
   * compute plot area positions and sizes
   */
  
  plot_to_scale_margin = (ui32) (Glob->ps_plot_to_scale_margin + 0.5);
  title_to_plot_margin = (ui32) (Glob->ps_title_to_plot_margin + 0.5);

  plot_width = (ui32) (Glob->ps_plot_width + 0.5);
  plot_height = (ui32) (Glob->ps_plot_height + 0.5);
  
  scale_width = (ui32) (Glob->ps_timeht_scale_width + 0.5);
  scale_height = plot_height;

  title_width = plot_width + scale_width + plot_to_scale_margin;
  title_height = (ui32) (Glob->ps_title_height + 0.5);
  
  ps_total_width = (double) title_width / Glob->ps_unitscale;
  ps_total_height = (double) (title_height + title_to_plot_margin +
				    plot_height) / Glob->ps_unitscale;
  
  title_x = 0;
  title_y = 0;
  
  plot_x = title_x;
  plot_y = title_y + title_height + title_to_plot_margin;
  
  scale_x = plot_x + plot_width + plot_to_scale_margin;
  scale_y = plot_y;
  
  /*
   * set up frames
   */
  
    
  GPsInitFrame(Glob->timeht_title_frame->psgc, ps_file, Glob->ps_fontname,
	       Glob->ps_title_fontsize, 1L);
    
  GPsInitFrame(Glob->timeht_plot_frame->psgc, ps_file, Glob->ps_fontname,
	       Glob->ps_ticklabel_fontsize, 1L);
    
  GPsInitFrame(Glob->timeht_scale_frame->psgc, ps_file, Glob->ps_fontname,
	       Glob->ps_scale_fontsize, 1L);
    
  GPsSetGeomFrame(Glob->timeht_title_frame, title_x, title_y,
		  title_width, title_height);
  
  GPsSetGeomFrame(Glob->timeht_plot_frame, plot_x, plot_y,
		  plot_width, plot_height);
  
  GPsSetGeomFrame(Glob->timeht_scale_frame, scale_x, scale_y,
		  scale_width, scale_height);
  
  GPsSetGeomFrame(Glob->vert_page_frame, 0, 0,
		  (ui32) (Glob->ps_page_width * Glob->ps_unitscale + 0.5),
		  (ui32) (Glob->ps_page_length * Glob->ps_unitscale + 0.5));

  GPsSetGeomFrame(Glob->horiz_page_frame, 0, 0,
		  (ui32) (Glob->ps_page_length * Glob->ps_unitscale + 0.5),
		  (ui32) (Glob->ps_page_width * Glob->ps_unitscale + 0.5));

  /*
   * set up graphics page 
   */

  GPageSetup(PSDEV,
	     Glob->vert_page_frame,
	     Glob->horiz_page_frame,
	     Glob->print_width,
	     ps_total_width,
	     ps_total_height,
	     Glob->ps_page_width,
	     Glob->ps_page_length,
	     Glob->ps_width_margin,
	     Glob->ps_length_margin,
	     ps_file);

  /*
   * set file entry in the psgc structures for each color level
   */

  for (i = 0; i < color_scale->nlevels; i++)
    color_scale->level[i].psgc->file = ps_file;

  /*
   * set font for title window
   */

  PsSetFont(Glob->timeht_title_frame->psgc->file,
	    Glob->timeht_title_frame->psgc->fontname,
	    Glob->timeht_title_frame->psgc->fontsize);

  /*
   * copy title window
   */

  if (Glob->draw_copy_title)
    draw_timeht_title(PSDEV);

  /*
   * set font for scale window
   */

  PsSetFont(Glob->timeht_scale_frame->psgc->file,
	    Glob->timeht_scale_frame->psgc->fontname,
	    Glob->timeht_scale_frame->psgc->fontsize);

  if (Glob->timeht_mode == TIMEHT_VORTICITY) {

    draw_scale(PSDEV, Glob->timeht_scale_frame, color_scale,
	       SCALE_LABEL_FORMAT_E, SCALE_PLOT_LEGENDS);

  } else {

    draw_scale(PSDEV, Glob->timeht_scale_frame, color_scale,
	       SCALE_LABEL_FORMAT_G, SCALE_PLOT_LEGENDS);

  }

  /*
   * set font for plot window
   */

  PsSetFont(Glob->timeht_plot_frame->psgc->file,
	    Glob->timeht_plot_frame->psgc->fontname,
	    Glob->timeht_plot_frame->psgc->fontsize);

  /*
   * draw plot
   */

  draw_timeht_plot(PSDEV, color_scale);

  /*
   * show page and close the ps file
   */

  PsShowPage(ps_file);
  fclose(ps_file);

  /*
   * print ps file
   */

  print_copy(ps_file_path);

  return;

}

/*********************************************************************
 * copy_rdist()
 *
 * make postscript file for reflectivity distribution
 */

void copy_rdist(g_color_scale_t *color_scale)

{
  
  char *prologue_file_path, *ps_file_path;
  char line[BUFSIZ];

  int i;
  int title_x, title_y;
  int scale_x, scale_y;
  int plot_x, plot_y;

  ui32 title_width, title_height;
  ui32 scale_width, scale_height;
  ui32 plot_width, plot_height;
  ui32 plot_to_scale_margin, title_to_plot_margin;  

  double ps_total_width, ps_total_height;

  FILE *prologue_file, *ps_file;

  /*
   * open prologue file
   */

  prologue_file_path = xGetResString(Glob->rdisplay, Glob->prog_name,
				     "ps_prologue_file",
				     PS_PROLOGUE_FILE);

  if ((prologue_file = fopen(prologue_file_path, "r")) == NULL) {
    fprintf(stderr, "ERROR - %s:copy_rdist\n", Glob->prog_name);
    fprintf(stderr, "Opening ps prologue file - ");
    perror(prologue_file_path);
    tidy_and_exit(1);
  }
  
  /*
   * open ps file
   */

  ps_file_path = xGetResString(Glob->rdisplay, Glob->prog_name,
			       "output_file", OUTPUT_FILE);

  if ((ps_file = fopen(ps_file_path, "w")) == NULL)
    {
      fprintf(stderr, "ERROR - %s:copy_rdist\n", Glob->prog_name);
      fprintf(stderr, "Creating ps output file - ");
      perror(ps_file_path);
      tidy_and_exit(1);
    }

  /*
   * copy prologue file to ps file
   */

  while (fgets(line, BUFSIZ, prologue_file) != NULL) 
    fprintf(ps_file, "%s", line);

  /*
   * close the prologue file
   */
  
  fclose(prologue_file);

  /*
   * compute plot area positions and sizes
   */
  
  plot_to_scale_margin = (ui32) (Glob->ps_plot_to_scale_margin + 0.5);
  title_to_plot_margin = (ui32) (Glob->ps_title_to_plot_margin + 0.5);

  plot_width = (ui32) (Glob->ps_plot_width + 0.5);
  plot_height = (ui32) (Glob->ps_plot_height + 0.5);
  
  scale_width = (ui32) (Glob->ps_rdist_scale_width + 0.5);
  scale_height = plot_height;

  title_width = plot_width + scale_width + plot_to_scale_margin;
  title_height = (ui32) (Glob->ps_title_height + 0.5);
  
  ps_total_width = (double) title_width / Glob->ps_unitscale;
  ps_total_height = (double) (title_height + title_to_plot_margin +
				    plot_height) / Glob->ps_unitscale;
  
  title_x = 0;
  title_y = 0;
  
  plot_x = title_x;
  plot_y = title_y + title_height + title_to_plot_margin;
  
  scale_x = plot_x + plot_width + plot_to_scale_margin;
  scale_y = plot_y;
  
  /*
   * set up frames
   */
  
    
  GPsInitFrame(Glob->rdist_title_frame->psgc, ps_file, Glob->ps_fontname,
	       Glob->ps_title_fontsize, 1L);
    
  GPsInitFrame(Glob->rdist_plot_frame->psgc, ps_file, Glob->ps_fontname,
	       Glob->ps_ticklabel_fontsize, 1L);
    
  GPsInitFrame(Glob->rdist_scale_frame->psgc, ps_file, Glob->ps_fontname,
	       Glob->ps_scale_fontsize, 1L);
    
  GPsSetGeomFrame(Glob->rdist_title_frame, title_x, title_y,
		  title_width, title_height);
  
  GPsSetGeomFrame(Glob->rdist_plot_frame, plot_x, plot_y,
		  plot_width, plot_height);
  
  GPsSetGeomFrame(Glob->rdist_scale_frame, scale_x, scale_y,
		  scale_width, scale_height);
  
  GPsSetGeomFrame(Glob->vert_page_frame, 0, 0,
		  (ui32) (Glob->ps_page_width * Glob->ps_unitscale + 0.5),
		  (ui32) (Glob->ps_page_length * Glob->ps_unitscale + 0.5));

  GPsSetGeomFrame(Glob->horiz_page_frame, 0, 0,
		  (ui32) (Glob->ps_page_length * Glob->ps_unitscale + 0.5),
		  (ui32) (Glob->ps_page_width * Glob->ps_unitscale + 0.5));

  /*
   * set up graphics page 
   */

  GPageSetup(PSDEV,
	     Glob->vert_page_frame,
	     Glob->horiz_page_frame,
	     Glob->print_width,
	     ps_total_width,
	     ps_total_height,
	     Glob->ps_page_width,
	     Glob->ps_page_length,
	     Glob->ps_width_margin,
	     Glob->ps_length_margin,
	     ps_file);

  /*
   * set file entry in the psgc structures for each color level
   */

  for (i = 0; i < color_scale->nlevels; i++)
    color_scale->level[i].psgc->file = ps_file;

  /*
   * set font for title window
   */

  PsSetFont(Glob->rdist_title_frame->psgc->file,
	    Glob->rdist_title_frame->psgc->fontname,
	    Glob->rdist_title_frame->psgc->fontsize);

  /*
   * copy title window
   */

  if (Glob->draw_copy_title)
    draw_rdist_title(PSDEV);

  /*
   * set font for scale window
   */

  PsSetFont(Glob->rdist_scale_frame->psgc->file,
	    Glob->rdist_scale_frame->psgc->fontname,
	    Glob->rdist_scale_frame->psgc->fontsize);

  draw_scale(PSDEV, Glob->rdist_scale_frame, color_scale,
	     SCALE_LABEL_FORMAT_G, SCALE_NO_LEGENDS);

  /*
   * set font for plot window
   */

  PsSetFont(Glob->rdist_plot_frame->psgc->file,
	    Glob->rdist_plot_frame->psgc->fontname,
	    Glob->rdist_plot_frame->psgc->fontsize);

  /*
   * draw plot
   */

  draw_rdist_plot(PSDEV, color_scale);

  /*
   * show page and close the ps file
   */

  PsShowPage(ps_file);
  fclose(ps_file);

  /*
   * print ps file
   */

  print_copy(ps_file_path);

  return;

}
