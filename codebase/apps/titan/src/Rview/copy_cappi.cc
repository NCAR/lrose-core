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
/************************************************************************
 * copy_cappi: make postscript copy of a cappi
 *
 * copy routine
 * 
 * Mike Dixon, RAP, NCAR, June 1990
 *************************************************************************/

#include "Rview.hh"
using namespace std;

void copy_cappi()

{

  char *prologue_file_path, *ps_file_path;
  char line[BUFSIZ];

  int i;

  double ps_pagewidth, ps_pagelength;
  double ps_widthmargin, ps_lengthmargin;

  g_color_scale_t *pscolors;

  FILE *prologue_file, *ps_file;

  if (Glob->verbose) {
    fprintf(stderr, "** copy_cappi **\n");
  }

  pscolors = Glob->fcontrol[Glob->field].pscolors;
  
  /*
   * open prologue file
   */

  prologue_file_path = uGetParamString(Glob->prog_name,
				     "ps_prologue_file",
				     PS_PROLOGUE_FILE);

  if ((prologue_file = fopen(prologue_file_path, "r")) == NULL)
    {
      fprintf(stderr, "ERROR - %s:copy_cappi\n", Glob->prog_name);
      fprintf(stderr, "Opening ps prologue file - ");
      perror(prologue_file_path);
      tidy_and_exit(1);
    }
  
  /*
   * open ps file
   */

  ps_file_path = uGetParamString(Glob->prog_name,
			       "output_file", OUTPUT_FILE);

  if ((ps_file = fopen(ps_file_path, "w")) == NULL)
    {
      fprintf(stderr, "ERROR - %s:copy_cappi\n", Glob->prog_name);
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
   * set up basic page layout
   */

  setup_cappi_page(ps_file);

  /*
   * set file pointer for each color level psgc
   */

  for (i = 0; i < pscolors->nlevels; i++)
    pscolors->level[i].psgc->file = ps_file;

  /*
   * set up graphics page
   */

  /*
   * get the params settings
   */

  ps_pagewidth = (double) uGetParamDouble(Glob->prog_name,
					"ps_pagewidth", PS_PAGEWIDTH);
  ps_pagelength = (double) uGetParamDouble(Glob->prog_name,
					 "ps_pagelength", PS_PAGELENGTH);
  ps_lengthmargin = (double) uGetParamDouble(Glob->prog_name,
					   "ps_lengthmargin", PS_LENGTHMARGIN);
  ps_widthmargin = (double) uGetParamDouble(Glob->prog_name,
					  "ps_widthmargin", PS_WIDTHMARGIN);
  
  GPageSetup(PSDEV, Glob->vert_page_frame,
	     Glob->horiz_page_frame, Glob->print_width,
	     Glob->ps_total_width,
	     Glob->ps_total_height,
	     ps_pagewidth, ps_pagelength,
	     ps_widthmargin, ps_lengthmargin,
	     ps_file);

  /*
   * set font for title window
   */

  PsSetFont(Glob->cappi_title_frame->psgc->file,
	    Glob->cappi_title_frame->psgc->fontname,
	    Glob->cappi_title_frame->psgc->fontsize);

  /*
   * copy title window
   */

  if (Glob->draw_copy_title)
    draw_cappi_title(PSDEV);

  /*
   * set font for scale window
   */

  PsSetFont(Glob->main_scale_frame->psgc->file,
	    Glob->main_scale_frame->psgc->fontname,
	    Glob->main_scale_frame->psgc->fontsize);

  draw_main_scale(PSDEV, pscolors,
		  SCALE_PLOT_LEGENDS);

  /*
   * set font for plot window
   */

  PsSetFont(Glob->cappi_ps_plot_frame->psgc->file,
	    Glob->cappi_ps_plot_frame->psgc->fontname,
	    Glob->cappi_ps_plot_frame->psgc->fontsize);

  /*
   * draw plot
   */

  draw_cappi_plot(PSDEV, pscolors);

  /*
   * set border line width
   */

  PsSetLineWidth(Glob->cappi_ps_plot_frame->psgc->file,
		 uGetParamDouble(Glob->prog_name,
			       "ps_border_width", PS_BORDER_WIDTH));

  /*
   * draw borders
   */

  GDrawRectangle(PSDEV,
		 Glob->cappi_ps_plot_frame,
		 Glob->cappi_ps_plot_frame->x->gc,
		 Glob->cappi_ps_plot_frame->psgc,
		 Glob->cappi_ps_plot_frame->w_xmin,
		 Glob->cappi_ps_plot_frame->w_ymin,
		 (Glob->cappi_ps_plot_frame->w_xmax -
		  Glob->cappi_ps_plot_frame->w_xmin),
		 (Glob->cappi_ps_plot_frame->w_ymax -
		  Glob->cappi_ps_plot_frame->w_ymin));

  GDrawRectangle(PSDEV,
		 Glob->main_scale_frame,
		 Glob->main_scale_frame->x->gc, 
		 Glob->main_scale_frame->psgc,
		 Glob->main_scale_frame->w_xmin,
		 Glob->main_scale_frame->w_ymin,
		 (Glob->main_scale_frame->w_xmax -
		  Glob->main_scale_frame->w_xmin),
		 (Glob->main_scale_frame->w_ymax -
		  Glob->main_scale_frame->w_ymin));

  /*
   * close the ps file
   */

  fprintf(ps_file, "showpage\n");
  fclose(ps_file);

  /*
   * print
   */

  print_copy(ps_file_path);

  return;

}
