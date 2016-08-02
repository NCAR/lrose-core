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
 * setup_vsection_page.c
 *
 * compute page posn and sizes for the frames which will
 * be used in a vertical section hard_copy
 *
 * RAP, NCAR, Boulder CO
 *
 * April 1991
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "Rview.hh"
using namespace std;

void setup_vsection_page(FILE *copy_file)

{

  int titlex, titley;
  int plotx, ploty;
  int scalex, scaley;
  ui32 titlewidth;
  ui32 plotwidth, plotheight;
  ui32 scaleheight;
  ui32 imagewidth, imageheight;

  static int first_call = TRUE;
  static double unitscale;
  static double ps_xaxismargin, ps_yaxismargin, ps_topmargin;
  static double ps_pagewidth, ps_pagelength;
  static double ps_imagewidth, ps_imageheight;
  static ui32 scalewidth, titleheight;
  static ui32 plot_to_scale_margin, title_to_plot_margin;
  static char *fontname;
  
  if (Glob->debug) {
    fprintf(stderr, "** setup_vsection_page **\n");
  }

  if (first_call) {

    /*
     * get the resources set by the parameters
     */

    unitscale = uGetParamDouble(Glob->prog_name, "ps_unitscale",
			     PS_UNITSCALE);

    ps_xaxismargin = uGetParamDouble(Glob->prog_name,
				  "ps_xaxismargin", PS_XAXISMARGIN) * unitscale;

    ps_yaxismargin = uGetParamDouble(Glob->prog_name,
				  "ps_yaxismargin", PS_YAXISMARGIN) * unitscale;

    ps_topmargin = uGetParamDouble(Glob->prog_name, "ps_topmargin",
				PS_TOPMARGIN) * unitscale;

    ps_pagewidth = uGetParamDouble(Glob->prog_name,
				"ps_pagewidth", PS_PAGEWIDTH);

    ps_pagelength = uGetParamDouble(Glob->prog_name,
				 "ps_pagelength", PS_PAGELENGTH);

    ps_imageheight = uGetParamDouble(Glob->prog_name,
				  "ps_imageheight",
				  PS_IMAGEHEIGHT) * unitscale;
    
    ps_imagewidth = uGetParamDouble(Glob->prog_name,
				 "ps_imagewidth",
				 PS_IMAGEWIDTH) * unitscale;

    scalewidth = (ui32)
      (uGetParamDouble(Glob->prog_name, "ps_scalewidth",
		    PS_SCALEWIDTH) * unitscale + 0.5);
    titleheight = (ui32)
      (uGetParamDouble(Glob->prog_name, "ps_titleheight",
		    PS_TITLEHEIGHT) * unitscale + 0.5);

    title_to_plot_margin = (ui32)
      (uGetParamDouble(Glob->prog_name, "ps_title_to_plot_margin",
		    PS_TITLE_TO_PLOT_MARGIN) * unitscale + 0.5);

    plot_to_scale_margin = (ui32)
      (uGetParamDouble(Glob->prog_name, "ps_plot_to_scale_margin",
		    PS_PLOT_TO_SCALE_MARGIN) * unitscale + 0.5);

    Glob->ps_title_fontsize = uGetParamDouble(Glob->prog_name,
					   "ps_title_fontsize",
					   PS_TITLE_FONTSIZE);

    Glob->ps_scale_fontsize = uGetParamDouble(Glob->prog_name,
					   "ps_scale_fontsize",
					   PS_SCALE_FONTSIZE);

    Glob->ps_ringlabel_fontsize = uGetParamDouble(Glob->prog_name,
					       "ps_ringlabel_fontsize",
					       PS_RINGLABEL_FONTSIZE);

    Glob->ps_ticklabel_fontsize = uGetParamDouble(Glob->prog_name,
					       "ps_ticklabel_fontsize",
					       PS_TICKLABEL_FONTSIZE);

    Glob->ps_text_fontsize = uGetParamDouble(Glob->prog_name,
					   "ps_text_fontsize",
					   PS_TEXT_FONTSIZE);

    fontname = uGetParamString(Glob->prog_name,
			     "ps_fontname", PS_FONTNAME);

  }
  
  /*
   * compute ps plot frame geometry
   */

  imagewidth = (ui32) (ps_imagewidth + 0.5);
  imageheight = (ui32) (ps_imageheight + 0.5);

  /*
   * compute plot area positions and sizes
   */

  plotwidth = (ui32) (imagewidth + ps_yaxismargin);
  plotheight = (ui32) (imageheight + ps_xaxismargin + ps_topmargin);

  scaleheight = plotheight;
  titlewidth = plotwidth + scalewidth + plot_to_scale_margin;

  Glob->ps_total_width = (double) titlewidth / unitscale;
  Glob->ps_total_height = (double) (titleheight + title_to_plot_margin +
			  plotheight) / unitscale;

  titlex = 0;
  titley = 0;

  plotx = titlex;
  ploty = titley + titleheight + title_to_plot_margin;

  scalex = plotx + plotwidth + plot_to_scale_margin;
  scaley = ploty;

  /*
   * set up or adjust frames
   */

  if (first_call) {

    GPsInitFrame(Glob->vsection_title_frame->psgc, copy_file, fontname,
		 Glob->ps_title_fontsize, 1L);

    GPsInitFrame(Glob->vsection_ps_plot_frame->psgc, copy_file, fontname,
		 Glob->ps_ticklabel_fontsize, 1L);

    GPsInitFrame(Glob->main_scale_frame->psgc, copy_file, fontname,
		 Glob->ps_scale_fontsize, 1L);

    first_call = FALSE;

  } else {

    Glob->vsection_title_frame->psgc->file = copy_file;
    Glob->vsection_ps_plot_frame->psgc->file = copy_file;
    Glob->main_scale_frame->psgc->file = copy_file;

  }

  /*
   * set frame geometry
   */

  GPsSetGeomFrame(Glob->vsection_title_frame, titlex, titley,
		  titlewidth, titleheight);

  GPsSetGeomFrame(Glob->vsection_ps_plot_frame, plotx, ploty,
		  plotwidth, plotheight);

  GPsSetGeomFrame(Glob->main_scale_frame, scalex, scaley,
		  scalewidth, scaleheight);

  GPsSetGeomFrame(Glob->vert_page_frame, 0, 0,
		  (ui32) (ps_pagewidth * unitscale + 0.5),
		  (ui32) (ps_pagelength * unitscale + 0.5));

  GPsSetGeomFrame(Glob->horiz_page_frame, 0, 0,
		  (ui32) (ps_pagelength * unitscale + 0.5),
		  (ui32) (ps_pagewidth * unitscale + 0.5));

}
