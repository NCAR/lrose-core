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
 * SoundingChart_timer.cc
 imer Related Routines: Reads new data & Refreshes plot
 *
 * F. Hage. 3/2004
 */

using namespace std;

#include "SoundingChart.hh"
#include <rapmath/math_macros.h>
#include <toolsa/pjg_flat.h>
#include <Spdb/DsSpdb.hh>

using namespace std;

/************************************************************************
 * TIMER_FUNC: This routine supervises timed operations 
 *   This read data from each source then redraw plot
 *
 */

static Notify_value t_func( Notify_client   client, int which)
{
    return P->timer_func(client, which);
}


Notify_value SoundingChart::timer_func( Notify_client   client, int which)

{
  static time_t next_poll_time = 0;
  int new_data = 0;
  time_t now = time(0);

  PMU_auto_register("Tick Tock");

  if(now >= next_poll_time) {
    // Check to see if we need new data - Load if so.
    new_data = check_retrieve(false);

    next_poll_time = now +  P->params.auto_click_interval;
  }

  // Check to see of the focus has changed - 
  new_data |= check_focus(); 

  // Check to see if we need new data - Load if so.
  new_data |= check_retrieve(new_data);

  if (new_data) {
	   P->doPlot();
  }
  return NOTIFY_DONE;
}

/************************************************************************
 * CHECK_FOCUS: Check to see of the focus has changed
 * Returns 1 if focus changed.
 */

int SoundingChart::check_focus(void)
{
    static int last_no = -1;
    static double last_lat = 0.0;
    static double last_lon = 0.0;
    static time_t last_time = 0;

	char msg_buf[128];

	int focus_changed = 0;

    if(P->coordShmem->pointer_seq_num != last_no &&
	  
	  ((last_lat != P->coordShmem->pointer_lat) ||
	   (last_lon != P->coordShmem->pointer_lon))) {

      if (P->params.debug) {
	    cerr << " CIDD Cursor Location updated " <<  endl;
		 fprintf(stderr,
			 "Click - lat = %g, lon = %g, mouse button = %d\n",
			 P->coordShmem->pointer_lat,
			 P->coordShmem->pointer_lon,
			 (int) P->coordShmem->button);
      }
      last_no = P->coordShmem->pointer_seq_num;
	  last_lat =P->coordShmem->pointer_lat;
	  last_lon = P->coordShmem->pointer_lon;

	   // Set globals
	   P->interest_lat = P->coordShmem->pointer_lat;
	   P->interest_lon = P->coordShmem->pointer_lon;

	   sprintf(msg_buf,"Lat:%.4f, Lon:%.4f", P->interest_lat,P->interest_lon); 
	   P->update_data_msg(msg_buf);

	  focus_changed |= 1;
    }

	if(P->coordShmem->time_cent != last_time )  {
		last_time = P->coordShmem->time_cent;
	    P->interest_time = P->coordShmem->time_cent;
		P->time_start = P->interest_time - 3600*P->params.lookBack;
		P->time_end = P->interest_time + 3600*P->params.lookAhead;

		if(P->coordShmem->epoch_end > P->time_end) P->time_end = P->coordShmem->epoch_end;
		if(P->coordShmem->epoch_start < P->time_start) P->time_start = P->coordShmem->epoch_start;

	    focus_changed |= 1;
	}

	return focus_changed;
}

/************************************************************************
 * CHECK_RETRIEVE: check to see whether we need to retrieve data.
 * - Returns 1 = New Data is available, 0 = No new data.
 */

int SoundingChart::check_retrieve(bool force /* = false*/ )
  
{
  static time_t next_poll_time = 0;

  int new_data = 0;
  time_t now = time(0);;

  if(now >= next_poll_time || force == true) {
     P->gather_data();
	 new_data = 1;
    next_poll_time = now +  P->params.auto_click_interval;
  }
  return new_data;

}

/**********************************************************************
 * START_TIMER:  Start up the interval timer
 */

void SoundingChart::start_timer(void)
{
  
  struct  itimerval   timer;
  
  int psecs = 1;
  
  /* set up interval timer interval */
  timer.it_value.tv_sec = psecs;
  timer.it_value.tv_usec = 0;
  timer.it_interval.tv_sec = psecs;
  timer.it_interval.tv_usec = 0;

  
  /* Set the interval timer function and start timer */
  notify_set_itimer_func(P->SoundingChart_GUI->window1,
			 (Notify_func) t_func,
			 ITIMER_REAL, &timer, NULL); /*  */
}
