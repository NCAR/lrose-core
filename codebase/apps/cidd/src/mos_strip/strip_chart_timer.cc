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
/*************************************************************************
 * STRIP_CHART_TIMER.C - Timer Related Routines: Reads new data & Refreshes plot
 *
 * F. Hage. 9/95
 */

#include "strip_chart.h"
#include <rapmath/math_macros.h>
#include <Spdb/DsSpdb.hh>

/************************************************************************
 * TIMER_FUNC: This routine supervises timed operations 
 *   This read data from each source then redraw plot
 *
 */

void timer_func( Notify_client   client, int which)

{

  check_retrieve();
  do_draw();
  
}
 
/************************************************************************
 * CHECK_RETRIEVE: check to see whether we need to retrieve data.
 *
 */

void check_retrieve(bool force = false)
  
{
  
  static bool archive_retrieved = false;
  static int last_no = 0;
  static time_t last_retrieved = 0;
  time_t now;

  switch (gd.p->mode) {
  
  case Params::REALTIME:
    
    if (gd.p->debug >= Params::DEBUG_VERBOSE) {
      cerr << "check_retrieve: realtime check" << endl;
    }
    now = time(NULL);
    set_times( now + gd.p->LookAhead );
    retrieve_data();
    break;

  case Params::ARCHIVE:
  
    if (force || !archive_retrieved) {
      set_times(gd.archive_time);
      retrieve_data();
      archive_retrieved = true;
    } 
    break;

  case Params::FOLLOW_CIDD:
    
    if (force) {
      set_times(gd.end_time);
      retrieve_data();
      break;
    }

    now = time(NULL);
    if(gd.cidd_mem->pointer_seq_num != last_no) {
      if (gd.p->debug) {
	cerr << "check_retrieve: seq number changed" << endl;
	cerr << "  new seq_num: " << gd.cidd_mem->pointer_seq_num << endl;
	cerr << "  last_retrieved: " << utimstr(last_retrieved) << endl;
      }
      last_no = gd.cidd_mem->pointer_seq_num;
      set_times(gd.cidd_mem->time_cent);
      last_retrieved = now;
      retrieve_data();
    } else if ((now - last_retrieved) > gd.p->poll_interval_sec) {
      if (gd.p->debug) {
	cerr << "now: " << utimstr(now) << endl;
	cerr << "last_retrieved: " << utimstr(last_retrieved) << endl;
	cerr << "interval_sec: " << gd.p->poll_interval_sec << endl;
	cerr << "check_retrieve: poll interval expired" << endl;
      }
      set_times(gd.cidd_mem->time_cent);
      last_retrieved = now;
      retrieve_data();
    }

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
 * DO_DRAW: perform the data gathering and drawing
 *
 */

void do_draw()

{

  if (!gd.new_data) {
    return;
  }

  if(gd.p->show_window == 0) {
    xv_set(gd.Strip_chart_win1->win1,XV_SHOW,FALSE,NULL);
  }
  
  if(gd.p->debug) fprintf(stderr,"Updating Plot\n");
  if(gd.p->show_window) draw_plot();
  
  if(gd.p->output_html) {
    draw_plot_xwd();
    draw_station_plot_xwd();
  }

  gd.new_data = false;

}
 
/**********************************************************************
 * START_TIMER:  Start up the interval timer
 */

void start_timer(void)
{
  
  struct  itimerval   timer;
  
  int psecs;
  if (gd.p->mode == Params::FOLLOW_CIDD) {
    psecs = 1;
  } else {
    psecs = gd.p->poll_interval_sec;
    if( psecs <= 1 || psecs > 3600) {
      psecs = 30;
    }
  }

  if (gd.p->debug) {
    cerr << "---->> Starting timer, psecs: " << psecs << endl;
  }
  
  /* set up interval timer interval */
  timer.it_value.tv_sec = psecs;
  timer.it_value.tv_usec = 0;
  timer.it_interval.tv_sec = psecs;
  timer.it_interval.tv_usec = 0;
  
  /* Set the interval timer function and start timer */
  notify_set_itimer_func(gd.Strip_chart_win1->win1,
			 (Notify_func)timer_func,
			 ITIMER_REAL, &timer, NULL); /*  */
  timer_func(0,0);
}

/*************************************************************************
 * RETRIEVE_DATA: Request the closest data to now for each station
 *
 */

void retrieve_data(void)

{

  // clean out old data

  for(int i = 0; i < gd.num_sources; i++) {
    if (gd.sources[i].reports) {
      ufree(gd.sources[i].reports);
      gd.sources[i].reports = NULL;
    }
    gd.sources[i].num_reports = 0;
  }
  gd.new_data = true;
  
  // get the data
  
  int interval = gd.end_time - gd.start_time;
  int margin = interval / 5;
  margin = MIN(margin, 3600);

  if (gd.p->debug) {
    cerr << "Retrieving spdb data" << endl;
    cerr << "  Start time: " << utimstr(gd.start_time) << endl;
    cerr << "  End   time: " << utimstr(gd.end_time) << endl;
    cerr << "  Margin (secs): " << margin << endl;
  }
  
  DsSpdb spdb;
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


    //
    // This is now more involved - now that we have
    // multiple reports at a given time with different
    // lead times, we have to sort through and take the
    // minimum lead time.
    //
    /*
    int nn = 0;
    for (int j = 0; j < nreports; j++) {
      if (refs[j].data_type == dataType) {
	nn++;
      }
    }
    */

    int nn = 0;
    time_t last_vt = 0;
    for (int j = 0; j < nreports; j++) {
      if (refs[j].data_type != dataType) continue; // Not the right station.

      //
      // The following assumes that we get the data in chronological
      // order. Only count each valid time once.
      //
      if (last_vt < refs[j].valid_time){
	last_vt = refs[j].valid_time;
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

    nn=0; last_vt = 0;
    for (int j = 0; j < nreports; j++) {

      // Skip it if the data type is not right.
      if (refs[j].data_type != dataType) continue;

      // Skip it if we have already processed this time.
      if (last_vt >= refs[j].valid_time) continue;

      // Otherwise, store this valid time.
      last_vt = refs[j].valid_time;

      // And process it by finding the minimum datatype2

      // Find the start and end indicies for this valid time.
      int istart = j; int iend = j;
      do{
	iend++;
      }while ((iend < nreports) && (last_vt == refs[j].valid_time));

      // And find the minimum data tye 2 (lead time) for this valid time.
      int min_dt2; int first = 1; int min_dt2_index;
      for (int ii = istart; ii <= iend; ii++){
	if (first){
	  min_dt2 = refs[ii].data_type2;
	  min_dt2_index = ii;
	  first = 0;
	} else {
	  if (min_dt2 > refs[ii].data_type2){
	    min_dt2 = refs[ii].data_type2;
	    min_dt2_index = ii;
	  }
	}
      }

      // And put this minimum lead time report into the output.
      gd.sources[i].reports[nn] = reports[min_dt2_index];
      nn++;
      
    }
    
  } // i
  
}
 
