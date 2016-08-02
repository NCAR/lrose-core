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
 * copy_cappi_tiff: make a TIFF copy of a cappi
 *
 * copy routine
 * 
 * Nancy Rehak, RAP, NCAR, June 1990
 *************************************************************************/

#include "Rview.hh"

#include <X11/Xlib.h>
#include <dsserver/DsLdataInfo.hh>
#include <rapplot/xutils.h>
#include <toolsa/Path.hh>
using namespace std;

static char *xwd_file_path;
static char *web_file_path;
static char *gif_dir_path;
static int border_width;
static bool initDone = false;

static Pixmap _create_pixmap();

static void _copy_to_pixmap(Drawable drawable,
			    Pixmap pixmap,
			    int width, int height,
			    int xx, int yy);
  
static void doInit()
  
{

  if (!initDone) {
    xwd_file_path = uGetParamString(Glob->prog_name,
				    "cappi_xwd_file", CAPPI_XWD_FILE);
    web_file_path = uGetParamString(Glob->prog_name,
				    "cappi_web_file", CAPPI_WEB_FILE);
    gif_dir_path = uGetParamString(Glob->prog_name,
				   "cappi_gif_dir", CAPPI_GIF_DIR);
    border_width = uGetParamLong(Glob->prog_name,
				 "x_subborder", X_SUBBORDER);

    initDone = true;
  }

}

////////////////////////////////
// copy cappi image to web file

void copy_cappi_web()
{

  static const char *routine_name = "copy_cappi_web";
  
  
  XWindowAttributes attributes;
  FILE *xwd_file;

  doInit();

  /*
   * Get the window attributes.
   */

  if (!XGetWindowAttributes(Glob->rdisplay,
			    Glob->main_window,
			    &attributes)) {
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

  if (ta_makedir_for_file(xwd_file_path)) {
    fprintf(stderr,
	    "ERROR - %s:%s\n",
	    Glob->prog_name, routine_name);
    fprintf(stderr,
	    "Cannot make dir for file '%s'\n", xwd_file_path);
    return;
  }

  if ((xwd_file = fopen(xwd_file_path, "w")) == NULL) {
    fprintf(stderr,
	    "ERROR - %s:%s\n",
	    Glob->prog_name, routine_name);
    fprintf(stderr,
	    "Cannot open XWD file for output\n");
    perror(xwd_file_path);
    return;
  }

  // create the pixmap with title, plot and scale windows

  Pixmap pixmap = _create_pixmap();
  
  /*
   * Copy the CAPPI to an xwd file
   */
  
  XUTIL_dump_pixmap_xwd(Glob->rdisplay, pixmap,
			&attributes, xwd_file);

  XFreePixmap(Glob->rdisplay, pixmap);

  if (Glob->debug) {
    cerr << "Writing XWD dump to file: " << xwd_file_path << endl;
  }
  
  /*
   * Close the output file.
   */
  
  fclose(xwd_file);
  
  /*
   * Convert to web format
   */
  
  convert_xwd_to_web(xwd_file_path, web_file_path);
  
  if (Glob->debug) {
    cerr << "Converting XWD to WEB file: " << web_file_path << endl;
  }
  
  return;

}

///////////////////////////////////////////////
// copy cappi image to gif file named from time

void copy_cappi_named_gif()
{

  static const char *routine_name = "copy_cappi_named_gif";
  
  XWindowAttributes attributes;
  FILE *xwd_file;

  doInit();

  // make sure output dir exists
  
  if (ta_makedir_recurse(gif_dir_path)) {
    fprintf(stderr,
	    "ERROR - %s:%s\n",
	    Glob->prog_name, routine_name);
    fprintf(stderr,
	    "Cannot make cappi gif directory\n");
    return;
  }

  /*
   * Get the window attributes.
   */

  if (!XGetWindowAttributes(Glob->rdisplay, Glob->main_window,
			    &attributes)) {
    fprintf(stderr,
	    "ERROR - %s:%s\n",
	    Glob->prog_name, routine_name);
    fprintf(stderr,
	    "Cannot get X window attributes for creating XWD file.\n");
    return;
  }

  Pixmap pixmap = _create_pixmap();

  /*
   * Open the output file.
   */

  if (ta_makedir_for_file(xwd_file_path)) {
    fprintf(stderr,
	    "ERROR - %s:%s\n",
	    Glob->prog_name, routine_name);
    fprintf(stderr,
	    "Cannot make dir for file '%s'\n", xwd_file_path);
    return;
  }

  if ((xwd_file = fopen(xwd_file_path, "w")) == NULL) {
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
  
  XUTIL_dump_pixmap_xwd(Glob->rdisplay, pixmap,
			&attributes, xwd_file);

  /*
   * free up pixmap
   */
  
  XFreePixmap(Glob->rdisplay, pixmap);
  
  /*
   * Close the output file.
   */
  
  fclose(xwd_file);

  if (Glob->debug) {
    cerr << "Writing XWD dump to file: " << xwd_file_path << endl;
  }
  
  /*
   * compute gif file path
   */
  
  char gif_file_name[MAX_PATH_LEN];
  char gif_file_path[MAX_PATH_LEN];
  const date_time_t &cappiTime = get_cappi_time();
  sprintf(gif_file_name, "%.4d%.2d%.2d_%.2d%.2d%.2d_%s.gif",
	  cappiTime.year, cappiTime.month, cappiTime.day,
	  cappiTime.hour, cappiTime.min, cappiTime.sec,
	  get_cappi_field_name().c_str());
  sprintf(gif_file_path, "%s%s%s",
	  gif_dir_path, PATH_DELIM, gif_file_name);

  /*
   * Convert to gif format
   */

  convert_xwd_to_web(xwd_file_path, gif_file_path);
  
  if (Glob->debug) {
    cerr << "Converting XWD to GIF file: " << gif_file_path << endl;
  }
  
  /*
   * Write ldata file
   */

  DsLdataInfo ldata;
  ldata.setDir(gif_dir_path);
  ldata.setWriter(Glob->prog_name);
  ldata.setDataFileExt("gif");
  ldata.setDataType("gif");
  if (Glob->debug) {
    ldata.setDebug();
  }
  ldata.setRelDataPath(gif_file_name);
  ldata.setUserInfo2(gif_file_name);
  ldata.write(cappiTime.unix_time);
  
  return;

}

static Pixmap _create_pixmap()

{

  /*
   * create a pixmap combining all 3 frames for the cappi
   */
  
  int title_width = Glob->cappi_title_frame->x->width;
  int title_height = Glob->cappi_title_frame->x->height;
  int plot_width = Glob->cappi_plot_frame->x->width;
  int plot_height = Glob->cappi_plot_frame->x->height;
  int scale_width = Glob->main_scale_frame->x->width;
  int scale_height = Glob->main_scale_frame->x->height;

  int pixmap_width = MAX(title_width,
			 (plot_width + scale_width));
  pixmap_width += 2 * border_width;

  int pixmap_height =
    title_height + MAX(plot_height, scale_height);
  pixmap_height += 4 * border_width;
  
  Pixmap pixmap =
    XCreatePixmap(Glob->rdisplay,
		  Glob->main_window,
		  pixmap_width, pixmap_height,
		  XDefaultDepth(Glob->rdisplay, Glob->rscreen));

  // title

  _copy_to_pixmap(Glob->cappi_title_frame->x->pixmap,
		  pixmap,
		  title_width, title_height,
		  0, 0);

  // plot window

  _copy_to_pixmap(Glob->cappi_plot_frame->x->pixmap,
		  pixmap,
		  plot_width, plot_height,
		  0,
		  2 * border_width + title_height);

  // scale window

  _copy_to_pixmap(Glob->main_scale_frame->x->pixmap,
		  pixmap,
		  scale_width, scale_height,
		  2 * border_width + plot_width,
		  2 * border_width + title_height);

  return pixmap;

}

static void _copy_to_pixmap(Drawable drawable,
			    Pixmap pixmap,
			    int width, int height,
			    int xx, int yy)
  
{

  // draw the border

  for (int i = 0; i < border_width; i++) {
    XDrawLine(Glob->rdisplay, pixmap, Glob->border_gc,
	      xx, yy + i,
	      xx + width + 2 * border_width, yy + i);
    XDrawLine(Glob->rdisplay, pixmap, Glob->border_gc,
	      xx, yy + i + height + border_width,
	      xx + width + 2 * border_width, yy + i + height + border_width);
    XDrawLine(Glob->rdisplay, pixmap, Glob->border_gc,
	      xx + i, yy,
	      xx + i, yy + height + 2 * border_width);
    XDrawLine(Glob->rdisplay, pixmap, Glob->border_gc,
	      xx + i + width + border_width, yy,
	      xx + i + width + border_width, yy + height + 2 * border_width);
  }

  // copy the pixmap

  XCopyArea(Glob->rdisplay,
	    drawable,
	    pixmap,
	    Glob->copyarea_gc,
	    0, 0,
	    width, height,
	    xx + border_width, yy + border_width);

}







