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
/////////////////////////////////////////////////////////////
// StationStripChart.hh
//
// StationStripChart object
//
// Displays RAP Surface Station Data (METAR) data in strip chart form
//
///////////////////////////////////////////////////////////////

#ifndef StationStripChart_H
#define StationStripChart_H

#include <string>
#include <rapformats/coord_export.h>
#include <rapformats/station_reports.h>
#include "Args.hh"
#include "Params.hh"

////////////////////////
// This class

class StationStripChart {
  
public:

  // constructor

  StationStripChart (int argc, char **argv);

  // destructor
  
  ~StationStripChart();

  // run 

  int Run();

  // data members

  bool isOK;

protected:
  
private:

  int _argc;
  char **_argv;
  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;
  coord_export_t *_coordShmem;

};

///////////////////////////////////////
// extern prototypes to non-object code

#include <xview/notify.h>
using namespace std;
#ifdef MIN
#undef MIN
#endif
#ifdef MAX
#undef MAX
#endif

typedef struct {

  Params::station_t *station_info; // station-related info
  int num_reports;
  int color_cell;  // A  color for each field
  double data_min; //
  double data_max; //
  double data_range;
  station_report_t *reports;

} source_info_t;

extern int strip_chart_main(int argc, char **argv, Params *params,
			    coord_export_t *coord_shmem);

extern void strip_chart_free();

extern void timer_func(Notify_client client, int which);
extern void draw_new_data();
extern void start_timer();

extern void set_frame_label(void);

extern void set_times(time_t end_time);
extern void check_retrieve(bool force = false);
extern void check_focus();
extern void retrieve_data(void);

extern void draw_plot(); // Picks the right plot to draw

extern void draw_common_plot(); // All Stations - One field
extern void draw_common_plot_images();

extern void draw_station_plot(); // All Fields, One station
extern void draw_station_plot_images();

#endif

