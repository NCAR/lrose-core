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
 * setup_track_shmem.c
 *
 * Sets up the shared memory to communicate with the main display
 * program for track data purposes.
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * April 1992
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "TimeHist.hh"
#include <toolsa/DateTime.hh>
using namespace std;

void setup_track_shmem(void)

{

  if (Glob->verbose) {
    fprintf(stderr, "** setup_track_shmem **\n");
  }

  /*
   * get shared memory - will block till available
   */

  if ((Glob->coord_export = (coord_export_t *)
       ushm_get(Glob->track_shmem_key,
		sizeof(coord_export_t))) == NULL) {
    fprintf(stderr, "ERROR - %s:setup_track_shmem.\n", Glob->prog_name);
    fprintf(stderr,
	    "Cannot create shared memory for coord_export, key = %d\n",
	    Glob->track_shmem_key);
    tidy_and_exit(-1);
  }
  
  if ((Glob->track_shmem = (time_hist_shmem_t *)
       ushm_get(Glob->track_shmem_key + 1,
		sizeof(time_hist_shmem_t))) == NULL) {
    fprintf(stderr, "ERROR - %s:setup_track_shmem.\n", Glob->prog_name);
    fprintf(stderr,
	    "Cannot get shared memory for track data, key = %d\n",
	    Glob->track_shmem_key + 1);
    tidy_and_exit(-1);
  }
  time_hist_shmem_t *tshmem = Glob->track_shmem;

  if (Glob->debug) {
    cerr << "Set up time_hist_shmem segment" << endl;
    cerr << "titan_server_url: " << tshmem->titan_server_url << endl;
    cerr << "time_label: " << tshmem->time_label << endl;
    cerr << "time: " << DateTime::strm(tshmem->time) << endl;
    cerr << "shmem_ready: " << tshmem->shmem_ready << endl;
    cerr << "localtime: " << tshmem->localtime << endl;
    cerr << "timeOffsetSecs: " << tshmem->timeOffsetSecs << endl;
    cerr << "time_hist_active: " << tshmem->time_hist_active << endl;
    cerr << "main_display_active: " << tshmem->main_display_active << endl;
    cerr << "main_display_must_update: " << tshmem->main_display_must_update << endl;
    cerr << "mode: " << tshmem->mode << endl;
    cerr << "plot_forecast: " << tshmem->plot_forecast << endl;
    cerr << "track_set: " << tshmem->track_set << endl;
    cerr << "scan_delta_t: " << tshmem->scan_delta_t << endl;
    cerr << "complex_track_num: " << tshmem->complex_track_num << endl;
    cerr << "simple_track_num: " << tshmem->simple_track_num << endl;
    cerr << "track_type: " << tshmem->track_type << endl;
    cerr << "track_seq_num: " << tshmem->track_seq_num << endl;
    cerr << "select_track_type: " << tshmem->select_track_type << endl;
    cerr << "track_data_time_margin: " << tshmem->track_data_time_margin << endl;
    cerr << "past_plot_period: " << tshmem->past_plot_period << endl;
    cerr << "future_plot_period: " << tshmem->future_plot_period << endl;
    cerr << "case_num: " << tshmem->case_num << endl;
    cerr << "partial_track_ref_time: " << tshmem->partial_track_ref_time << endl;
    cerr << "partial_track_past_period: " << tshmem->partial_track_past_period << endl;
    cerr << "partial_track_future_period: " << tshmem->partial_track_future_period << endl;
    cerr << "n_forecast_steps: " << tshmem->n_forecast_steps << endl;
    cerr << "forecast_interval: " << tshmem->forecast_interval << endl;
    cerr << "past_color: " << tshmem->past_color << endl;
    cerr << "current_color: " << tshmem->current_color << endl;
    cerr << "future_color: " << tshmem->future_color << endl;
    cerr << "forecast_color: " << tshmem->forecast_color << endl;
  }

  /*
   * initialize partial track params
   */
  
  tshmem->partial_track_past_period =
    Glob->partial_track_past_period;
  tshmem->partial_track_future_period =
    Glob->partial_track_future_period;

  /*
   * set flag on whether to use TitanServer
   */
  
  Glob->_titanLdata.setDirFromUrl(tshmem->titan_server_url);

  /*
   * set archive time if applicable
   */

  if (Glob->archive_only) {
    if (!Glob->archive_time_set) {
      Glob->archive_time = tshmem->time;
      Glob->archive_time_set = TRUE;
    }
  }

}
