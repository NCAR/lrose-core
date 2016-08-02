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
// metar_strip.hh
//
// metar_strip object
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2000
//
///////////////////////////////////////////////////////////////
//
// Displays METAR data in strip chart form
//
///////////////////////////////////////////////////////////////

#ifndef metar_strip_H
#define metar_strip_H

#include <string>
#include <rapformats/coord_export.h>
#include <rapformats/WxObs.hh>
#include "Args.hh"
#include "Params.hh"

////////////////////////
// This class

class metar_strip {
  
public:

  // constructor

  metar_strip (int argc, char **argv);

  // destructor
  
  ~metar_strip();

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

class SourceInfo {
public:
  SourceInfo() {
    station_info = NULL;
    color_cell = 0;
  }
  Params::station_t *station_info; // station-related info
  int color_cell;  // A  color for each field
  vector<WxObs> obsArray;
};

extern int strip_chart_main(int argc, char **argv, Params *params,
			    coord_export_t *coord_shmem);

extern void strip_chart_free();

extern void timer_func(Notify_client client, int which);
extern void do_draw();
extern void start_timer();

extern void set_frame_label(void);

extern void set_times(time_t end_time);
extern void check_retrieve(bool force = false);
extern void retrieve_data(void);

extern void draw_plot();
extern void draw_plot_xwd();

extern void draw_common_plot();
extern void draw_station_plot();
extern void draw_station_plot_xwd();

#endif

