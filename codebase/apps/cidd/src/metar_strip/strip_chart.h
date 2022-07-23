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
/*******************************************************************************
 * STRIP_CHART.H : Defines & includes for the Mesonet Strip chaart plotter
 */

#include <math.h>           /* System */
#include <sys/param.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/errno.h>

#include <X11/Xlib.h>       /* X11 - XView */
#include <X11/Xutil.h>       /* X11 - XView */
#include <X11/Xresource.h>
#include <xview/xview.h>
#include <xview/canvas.h>
#include <xview/cms.h>
#include <xview/icon_load.h>
#include <xview/notify.h>
#include <xview/notice.h>
#include <xview/panel.h>
#include <xview/scrollbar.h>
#include <xview/svrimage.h>
#include <xview/termsw.h>
#include <xview/text.h>
#include <xview/tty.h>
#include <xview/textsw.h>
#include <xview/xv_xrect.h>
#include <devguide/gdd.h>                /* Dev GUIDE */

#include <rapplot/xrs.h>
#include <rapplot/xutils.h>
#include <Spdb/StationLoc.hh>
#include <toolsa/port.h>
#include <toolsa/pmu.h>    
#include <toolsa/mem.h>    
#include <toolsa/utim.h>    
#include <toolsa/udatetime.h>    
#include <toolsa/ushmem.h>
#include <dataport/port_types.h>
#include <rapformats/station_reports.h>
#include <rapformats/coord_export.h>

#include <Spdb/DsSpdb.hh>
#include <Spdb/Product_defines.hh>
 
#include "strip_chart_ui.h"
#include "Params.hh"

#include "metar_strip.hh"

/******************************************************************************
 * GLOBAL DATA DECLERATIONs
 */

struct Global_data {

  char *app_name;
  
  Params *p; // TDRP parameters
  
  int variable; // current variable
  int num_sources;
  
  int win_width, win_height;
  int plot_width, plot_height;

  time_t start_time, end_time;
  double pixels_per_sec;
  
  int fg_cell;      /* foreground color cell */
  int bg_cell;      /* background color cell */
  int now_cell;     /* reference color for "now" on the time line */
  int *fcat_cells;  /* color cells for flight cat */
  
  time_t archive_time; /* In archive Mode - When is "Now" */
  
  double data_min, data_max;      /* dynamic range of the data */
  double data_range;
  bool new_data;
  
  vector<SourceInfo> sources; 
  
  // X Windows Drawing variables
  
  Display *dpy;
  XFontStruct *fontst;
  Font font;
  Drawable canvas_xid;
  Drawable back_xid;
  GC def_gc;
  
  strip_chart_win1_objects *Strip_chart_win1;
  
  coord_export_t *cidd_mem; // pointer to cidd's shmem control interface

  StationLoc *stationLoc;
  
};

// Set Up Local and External references to global data structure

#ifdef STRIP_CHART_MAIN
struct Global_data gd;
Attr_attribute INSTANCE;
#else
extern struct Global_data gd;
extern Attr_attribute INSTANCE;
#endif
