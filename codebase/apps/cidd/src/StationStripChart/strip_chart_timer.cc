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
 * STRIP_CHART_TIMER.C - Timer Related Routines: Reads new data & Refreshes plot
 *
 * F. Hage. 9/95
 */

#include "strip_chart.h"
#include <toolsa/toolsa_macros.h>
#include <toolsa/pjg_flat.h>
#include <Spdb/DsSpdb.hh>

using namespace std;

/************************************************************************
 * TIMER_FUNC: This routine supervises timed operations 
 *   This read data from each source then redraw plot
 *
 */

void timer_func( Notify_client   client, int which)

{
  static time_t next_poll_time = 0;
  time_t now = time(0);
  char buf[1024];

  PMU_auto_register("Tick Tock");

  if(now >= next_poll_time && gd.want_live_updates) {
    check_retrieve(true); // Check to see if we need new data - Load if so.
    next_poll_time = now + (int) (gd.p->update_interval_min * 60.0)
                         - (now %  (int)(gd.p->update_interval_min * 60.0))
			+ gd.p->seconds_delay;
  }

  switch(  gd.p->mode ) {
    case Params::SLAVE:
      check_retrieve(false); // Check to see if we need new data - Load if so.
      check_focus();    // Check to see of the focus has changed - 
	break;

	 default:
	 break;
  }

  struct tm res;
  if (gd.new_data) {
     draw_new_data();
     if(gd.p->debug) {
		 if(gd.p->use_localtime) {
	       strftime(buf,128," %H:%M:%S", localtime_r(&next_poll_time,&res));
		 } else {
	       strftime(buf,128," %H:%M:%S", gmtime_r(&next_poll_time,&res));
		 }
	 fprintf(stderr,"\n**************************Next poll time: %s\n",buf);
     }
  }
}

/************************************************************************
 *
 */
int sort_func(const void *e1,const void *e2)
{
    source_info_t **s1 = (source_info_t **) e1;
    source_info_t **s2 = (source_info_t **) e2;
    source_info_t *source1 = *s1;
    source_info_t *source2 = *s2;


    // Handle stations with no data - Assumed far far away.
    if(source1->num_reports == 0) {
      if(source2->num_reports == 0) return 0;
      return 1;
    } 
    if(source2->num_reports == 0) return -1;

    double theta1,dist1;  
    double theta2,dist2;  

    PJGLatLon2RTheta((double) gd.p->focus_lat,
		     (double) gd.p->focus_lon,
		     (double) source1->reports[0].lat,
		     (double) source1->reports[0].lon,
		     &dist1,&theta1);

    PJGLatLon2RTheta((double) gd.p->focus_lat,
		     (double) gd.p->focus_lon,
		     (double) source2->reports[0].lat,
		     (double) source2->reports[0].lon,
		     &dist2,&theta2);

    if(dist1 < dist2) {
      return -1;
    }
    if(dist2 < dist1) {
      return 1;
    }
    return 0;
}

 
/************************************************************************
 * CHECK_FOCUS: Check to see of the focus has changed
 */

 void check_focus()
 {
    static int last_no = -1;
    static int last_time = -1;
    static double last_lat = 0.0;
    static double last_lon = 0.0;
	int    c_plot;

    if(gd.cidd_mem->pointer_seq_num != last_no && 
	  ((last_lat != gd.cidd_mem->pointer_lat) ||
	   (last_lon != gd.cidd_mem->pointer_lon))) {

      if (gd.p->debug) {
	    cerr << " Cursor Location updated " <<  endl;
      }
      last_no = gd.cidd_mem->pointer_seq_num;
	  last_lat = gd.cidd_mem->pointer_lat;
	  last_lon = gd.cidd_mem->pointer_lon;

      gd.p->focus_lat = gd.cidd_mem->pointer_lat;
      gd.p->focus_lon = gd.cidd_mem->pointer_lon;


      // SORT LIST
      qsort(gd.sorted_sources,gd.num_sources,sizeof(source_info_t *),sort_func);

      // If we're looking at a field - Replot with the new list order.
      if(gd.cur_plot < gd.num_active_fields) {
		  gd.new_data = true;
	  } else {  // We're looking at one station - Find the nearest one.
		  c_plot = gd.cur_plot;
		  gd.cur_plot =  gd.num_active_fields;
		  gd.new_data = true;
	  }
    }

    if(gd.cidd_mem->time_seq_num != last_time) { 
		last_time = gd.cidd_mem->time_seq_num;
		if(gd.cidd_mem->runtime_mode == RUNMODE_REALTIME) {
			gd.want_live_updates = 1;
		} else {
			gd.want_live_updates = 0;
		}
	}
 }

/************************************************************************
 * CHECK_RETRIEVE: check to see whether we need to retrieve data.
 *
 */

void check_retrieve(bool force /* = false*/ )
  
{
  static bool archive_retrieved = false;
  static time_t last_cidd_time = 0;
  static time_t last_retrieved = 0;
  time_t now;

  switch (gd.p->mode) {
  
  case Params::REALTIME:
    
    if (gd.p->debug >= Params::DEBUG_VERBOSE) {
      cerr << "check_retrieve: realtime check" << endl;
    }
    now = time(NULL);
    set_times(now);
    retrieve_data();
    break;

  case Params::ARCHIVE:
  
    if (force || !archive_retrieved) {
      set_times(gd.archive_time);
      retrieve_data();
      archive_retrieved = true;
    } 
    break;

  case Params::SLAVE:

    now = time(NULL);
	if(gd.want_live_updates) {
      last_retrieved = now;
      set_times(now);
	} else {
      if(gd.cidd_mem->epoch_end != last_cidd_time) {
        if (gd.p->debug) {
	      cerr << "Epoch End Changed. Now: " << utimstr(gd.cidd_mem->epoch_end) << endl;
        }
        last_retrieved = now;
        set_times(gd.cidd_mem->epoch_end);
        last_cidd_time = gd.cidd_mem->epoch_end;
        retrieve_data();
	  }
	}

	if(force) {
     last_retrieved = now;
     retrieve_data();
    }

    break;

  } // switch
}

/************************************************************************
 * SET_TIMES: set the times perform the data gathering and drawing
 *
 */
void set_times(time_t end_time)
  
{
  gd.end_time = end_time;
  gd.start_time = (time_t) (gd.end_time - ((gd.plot_width + 1) /
					   (gd.pixels_per_sec)));
  if (gd.p->debug) {
    cerr << "set_times - reset to:" << endl;
    cerr << "  Start time: " << utimstr(gd.start_time) << endl;
    cerr << "  End   time: " << utimstr(gd.end_time) << endl;
    cerr << "  plot_height: " << gd.plot_height << endl;
    cerr << "  plot_width: " << gd.plot_width << endl;
  }
}
 
/************************************************************************
 * Draw_new_data
 *
 */

void draw_new_data()

{


  if(gd.p->output_html) {
    if(gd.p->debug) fprintf(stderr,"Ploting all images\n");
    if(gd.p->show_window == 0) {
      xv_set(gd.Strip_chart_win1->win1,XV_SHOW,FALSE,NULL);
    }
  
    draw_common_plot_images(); // Plot Each variable -  All stations
    draw_station_plot_images(); // Plot Each Station - All variables
  } else {
    if(gd.p->debug) fprintf(stderr,"Updating Plot\n");
    draw_plot();
  }

  gd.new_data = false;
}
 
/**********************************************************************
 * START_TIMER:  Start up the interval timer
 */

void start_timer(void)
{
  
  struct  itimerval   timer;
  
  int psecs = 1;
  
  /* set up interval timer interval */
  timer.it_value.tv_sec = psecs;
  timer.it_value.tv_usec = 0;
  timer.it_interval.tv_sec = psecs;
  timer.it_interval.tv_usec = 0;
  
  /* Set the interval timer function and start timer */
  notify_set_itimer_func(gd.Strip_chart_win1->win1,
			 (Notify_func)timer_func,
			 ITIMER_REAL, &timer, NULL); /*  */
}

/*************************************************************************
 * RETRIEVE_DATA: Request the closest data to now for each station
 *
 */

void retrieve_data(void)

{

  xv_set(gd.Strip_chart_win1->win1,FRAME_BUSY,TRUE,NULL);
  xv_set(gd.Strip_chart_config_pu->config_pu,FRAME_BUSY,TRUE,NULL);

  // clean out old data

  for(int i = 0; i < gd.num_sources; i++) {
    if (gd.sources[i].reports) {
      ufree(gd.sources[i].reports);
      gd.sources[i].reports = NULL;
    }
    gd.sources[i].num_reports = 0;
  }
  
  // get the data
  
  int interval = gd.end_time - gd.start_time;
  int margin = interval / 5;
  margin = MIN(margin, 3600);

  if (gd.p->debug) {
    cerr << "----------------------------------------------------" << endl;
    cerr << "----------------------------------------------------" << endl;
    cerr << "Retrieving spdb data" << endl;
    cerr << "  Start time: " << utimstr(gd.start_time) << endl;
    cerr << "  End   time: " << utimstr(gd.end_time) << endl;
    cerr << "  Margin (secs): " << margin << endl;
  }
  
  DsSpdb spdb;

  // should we check write time?
  
  if (gd.cidd_mem &&
      gd.cidd_mem->checkWriteTimeOnRead) {
    spdb.setCheckWriteTimeOnGet(gd.cidd_mem->latestValidWriteTime);
  } else {
    spdb.clearCheckWriteTimeOnGet();
  }

  if (spdb.getInterval(gd.p->input_url,
		       gd.start_time - margin,
		       gd.end_time + margin)) {
    cerr << "ERROR - retrieve_data" << endl;
    cerr << spdb.getErrStr() << endl;
    return;
  }
  
  // swap the reports data

  int nreports = spdb.getNChunks();
  Spdb::chunk_ref_t *refs = spdb.getChunkRefs();
  station_report_t *reports = (station_report_t *) spdb.getChunkData();
  for (int j = 0; j < nreports; j++) {
    station_report_from_be(reports + j);
  }

  if (gd.p->debug >= Params::DEBUG_VERBOSE) {
    cerr << "After GetInterval" << endl;
    cerr << "Total number of reports for all stations: " << nreports << endl;
  }
      
  // load up the data for each gauge

  for(int i = 0; i < gd.num_sources; i++) {

    int dataType =
      Spdb::hash4CharsToInt32(gd.sources[i].station_info->name);

    // count the number of reports for this station

    int nn = 0;
    for (int j = 0; j < nreports; j++) {
      if (refs[j].data_type == dataType) {
	nn++;
      }
    }
    
    // allocate space for the reports

    gd.sources[i].reports =
      (station_report_t *) umalloc (nn * sizeof(station_report_t));
    gd.sources[i].num_reports = nn;
    
    if (gd.p->debug >= Params::DEBUG_VERBOSE) {
      cerr << "Number of reports for station: "
	   << gd.sources[i].station_info->name << " = "
	   << nn << endl;
    }
      
    // copy in the reports

    nn = 0;
    for (int j = 0; j < nreports; j++) {
      if (refs[j].data_type == dataType) {
	gd.sources[i].reports[nn] = reports[j];
	nn++;
      }
    }
    
  } // i

  gd.new_data = true;

  xv_set(gd.Strip_chart_win1->win1,FRAME_BUSY,FALSE,NULL);
  xv_set(gd.Strip_chart_config_pu->config_pu,FRAME_BUSY,FALSE,NULL);
}
 

