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
#ifdef __cplusplus
extern "C" {
#endif
/**************************************************************************
 * time_hist_shmem.h - struct definition for shared memory communication
 *                 between main displays (rview, CIDD) and time_hist
 *
 * Mike Dixon
 *
 * RAP NCAR Boulder CO USA
 *
 * April 1992
 *
 **************************************************************************/

#ifndef time_hist_shmem_h
#define time_hist_shmem_h
  
#include <titan/track.h>
#include <titan/tdata_server.h>
#include <toolsa/servmap.h>
  
#define TIME_HIST_SHMEM_COLOR_NAME_LEN 64
#define TIME_HIST_SHMEM_HOST_NAME_LEN 64
#define TIME_HIST_SHMEM_URL_LEN 1024
#define TIME_HIST_SHMEM_TIME_LABEL_LEN 16

typedef struct {

  char titan_server_url[TIME_HIST_SHMEM_URL_LEN];
  char time_label[TIME_HIST_SHMEM_TIME_LABEL_LEN]; /* set by Rview */

  time_t time;			/* originally set by main display, the
				 * modified by time_hist -
				 * unix time - secs since Jan 1 1970 */

  int shmem_ready;		/* set by main display - 
				 * starts at 0, set to 1 by main
				 * display when it has loaded relevant data */
  int localtime;                /* use local time instead of utc
                                 * set by Rview */
  int timeOffsetSecs;           /* diff between UTC and display time
                                 * non-zero if localtime is used */

  int time_hist_active;	/* set by time_hist - 
			 * 1 if time hist is active, 0 otherwise */
  
  int main_display_active;	/* set by main display -
				 * 1 if main display (rview, CIDD) is active,
				 * 0 otherwise */
  
  int main_display_must_update; /* set to TRUE by time_hist when the 
				 * main display needs to update to 
				 * synchronize with time_hist -
				 * reset by main display */
  
  int mode;			/* originally set by main display, then
				 * altered by time_hist -
				 * TDATA_REALTIME or TDATA_ARCHIVE -
				 * see tdata_server.h */
  
  int plot_forecast;		/* set by main display -
				 * flag to indicate whether forecast is
				 * to be plotted */
  
  int track_set;		/* set by time_hist -
				 * flag to indicate whether current
				 * tracks only are required, or all
				 * tracks in ops period - values:
				 * TDATA_ALL_AT_TIME, or
				 * TDATA_ALL_IN_FILE -
				 * see tdata_server.h */
  
  int scan_delta_t;             /* time between scans - secs
                                 * set by main display
                                 * Used by time_hist to move back and forward
                                 * by one scan if no TITAN data is available */

  int complex_track_num;	/* set by time_hist -
				 * active complex track */
  
  int simple_track_num;	        /* set by time_hist -
				 * active simple track */
  
  int track_type;		/* set by time_hist -
				 * SIMPLE_TRACK, COMPLEX_TRACK, PARTIAL_TRACK
				 * see dix_track.h */
  
  int track_seq_num;		/* set by time_hist -
				 * increments by 1 each time the
				 * track number is changed */
  
  int select_track_type;	/* Type of track selected - set by rview
				 * and/or time_hist */
  
  int track_data_time_margin;   /* set by main display -
                                 * (secs) search margin on either side 
                                 * of requested time */
  
  int past_plot_period;	        /* set by main display -
				 * (secs) length of past tracks
				 * shown plotted */
  
  int future_plot_period;	/* set by main display -
				 * (secs) length of future tracks
				 * shown plotted */

  int case_num;                 /* case number if applicable */

  int partial_track_ref_time;   /* set by time_hist - the reference time
				 * for partial_tracks */
  
  int partial_track_past_period;  /* set by time_hist -
				   * (secs) length of past partial
				   * tracks shown plotted */
  
  int partial_track_future_period;  /* set by time_hist -
				     * (secs) length of future partial
				     * tracks shown plotted */
  
  int n_forecast_steps;	        /* set by main_display -
				 * the number of steps for which the
				 * forecast track positions are
				 * displayed */
  
  int forecast_interval;	/* set by main_display -
				 * (secs) the interval between
				 * the forecast steps */
  
  /*
   * display grid
   */
  
  titan_grid_t grid;
  
  /*
   * the following colors set by main display
   */
  
  char past_color[TIME_HIST_SHMEM_COLOR_NAME_LEN];
  char current_color[TIME_HIST_SHMEM_COLOR_NAME_LEN];
  char future_color[TIME_HIST_SHMEM_COLOR_NAME_LEN];
  char forecast_color[TIME_HIST_SHMEM_COLOR_NAME_LEN];
  
} time_hist_shmem_t;

#endif
#ifdef __cplusplus
}
#endif
