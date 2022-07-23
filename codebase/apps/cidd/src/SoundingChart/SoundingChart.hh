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
//////////////////////////////////////////////////////////
// SoundingChart.hh
//
// Original Structure by 
// Niles Oien, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
// 
// Xview, plotting added by  Frank Hage, Spring 2004 
//
///////////////////////////////////////////////////////////////
//
// SoundingChart reads user key-clicks from the display, and
// passes these on to the sub classes.
//
///////////////////////////////////////////////////////////////

#ifndef SoundingChart_H
#define SoundingChart_H

using namespace std;

#include <vector>
#include <string>

#include <rapformats/coord_export.h>
#include <Spdb/StationLoc.hh> 
#include <toolsa/pmu.h>


#include <stdio.h>
#include <sys/param.h>
#include <sys/types.h>
#include <xview/xview.h>
#include <xview/panel.h>
#include <xview/textsw.h>
#include <xview/xv_xrect.h>

#include "Args.hh"
#include "Params.hh"

#include "ColorScale.hh"
#include "genPtMgr.hh"
#include "soundingMgr.hh"
#include "mdvSoundingMgr.hh"
#include "TimePlot.hh"

#include "SoundingChart_GUI_ui.h"

////////////////////////
// This class

class SoundingChart {
  
public:

  // constructor

  SoundingChart (int argc, char **argv);

  // destructor
  
  ~SoundingChart();

  // Run Method
  int Run(int argc, char **argv);

  // Timer interrupt thread function
  Notify_value timer_func( Notify_client   client, int which);

  // Instantiate the XView Objects.
  void  init_xview(void );

  // Modify XView Object's default behavior
  void modify_xview_objects(void);

  // Update the Time Message on the Control Panel
  void update_time_msg(char *message);

  // Update the Data Message on the control panel
  void update_data_msg(char *message);

  // Draw the Plot
  void doPlot(void);

  // Draw the Plot
  void doGridPlot(TimePlot &tp);

  // Draw the Plot
  void doSoundingPlot(TimePlot &tp);

  // Gather Data 
  void gather_data(void);

  // Gather Mdv Data 
  void gather_mdv_data(void);

  // Gather Sounding Data 
  void gather_sounding_data(void);

  // Nofify CIDD of new data 
  void notify_cidd(void);

  // Start the Interval Timer.
  void start_timer(void);

  // Window Destroy funct.
  Notify_value  base_win_destroy(Notify_client client, Destroy_status  status);

  // Checks for User input (Mouse clicks on map)
  int check_focus(void);

  // Check if it's time to get new data 
  int check_retrieve(bool force /* = false*/);

  string progName;
  char *paramsPath;
  Args args;
  Params params;

  // Shared Memory segment with CIDD
  coord_export_t *coordShmem;

  // Station Lat lon to ID lookup 
  StationLoc *station_loc;

  // XView Objects.
  SoundingChart_GUI_window1_objects    *SoundingChart_GUI;
  SoundingChart_GUI_config_pu_objects  *SoundingChart_GUI_config_pu;

  // Display Variables.
  Display *dpy;
  int win_height;
  int win_width;
  int can_height;
  int can_width;

  Drawable canvas_xid;
  Drawable backing_xid;
  GC       def_gc;
  XFontStruct *fontst;
  Font font;

  // X Pixel COlor values
  int   fg_cell;  // Foreground Cell
  int   bg_cell;  // Background
  int   ref_cell; // Reference Line Color Cell
  int   grid_winds_cell;  // Grid Wind barbs
  int   sounding_winds_cell;  // Sounding Wind barbs

  // Run time variables.
  time_t interest_time; // Time of current interest
  time_t time_start;    // Start Time of plot
  time_t time_end;      // End time of plot

  int	cur_grid_field;
  int	cur_sound_field;

  int   grid_winds_on;  // 1 =  Display Grid Winds
  int   grid_lines_on;  // 1 =  Display Grid Data Line Plots
  int   grid_color_on;  // 1 =  Display Grid Color filled cells
  int   sound_winds_on; // 1 = Display Sounding Winds.
  int   sound_lines_on; // 1 = Display Sounding Data Line Plots.
  int   sound_color_on; // 1 = Display Sounding Color filled cells.
  int   reference_points_on; // 1 = output GenPt click points.

  int sounding_width_secs;

  int   num_points; // click point count.

  double interest_lat;  // Current Click point in CIDD
  double interest_lon;
  double interest_alt;

protected:
  
private:
   vector<ColorScale* > grid_cs; // Grid color scales.
   vector<ColorScale* > sound_cs; // Sounding color scales.

   soundingMgr *SG;  // Data handler for Soundings

   mdvSoundingMgr *M; // Data handler for Mdv data

};

#ifdef SoundingChart_CC
Attr_attribute INSTANCE;
SoundingChart *P;
#else
extern Attr_attribute INSTANCE;
extern SoundingChart *P;
#endif

#endif
