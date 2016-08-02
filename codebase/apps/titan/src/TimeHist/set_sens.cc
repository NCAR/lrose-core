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
 * set_sens.c
 *
 * Set sensitivity of windows to events
 *
 * RAP, NCAR, Boulder CO
 *
 * May 1992
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "TimeHist.hh"
using namespace std;

/**************************************************************************
 * init_flags - set flag bytes
 */

static void get_activity(ui08 *activity,
			 int n_buttons,
			 const char *button_sens)

{

  si32 i;
  char *tmp_sens;
  char *startptr, *endptr;

  tmp_sens = (char *) umalloc((ui32)strlen(button_sens) + 1);
  strcpy(tmp_sens, button_sens);

  startptr = tmp_sens;

  for (i = 0; i < n_buttons; i++) {
    errno = 0;
    activity[i] = strtol(startptr, &endptr, 10);
    startptr = endptr;
    if (errno != 0) {
      fprintf(stderr, "ERROR - %s:set_sens:get_activity.\n",
	      Glob->prog_name);
      perror(button_sens);
      tidy_and_exit(-1);
    }
  }
  
  ufree(tmp_sens);

}

/*********************************************************************
 * set_sens()
 *
 *********************************************************************/

static void set_sens(Window main_window,
		     gframe_t *title_frame,
		     gframe_t *plot_frame,
		     gframe_t **button_frame,
		     int n_buttons,
		     const char *button_sens,
		     int plot_active)

{
  
  si32 i;
  ui08 sens;
  ui08 *activity;
  
  activity = (ui08 *) umalloc ((ui32) n_buttons * sizeof(ui08));
  get_activity (activity, n_buttons, button_sens);

  XSelectInput(Glob->rdisplay, main_window, StructureNotifyMask);
  
  XSelectInput(Glob->rdisplay, title_frame->x->drawable, ExposureMask);

  if (plot_active)
    XSelectInput(Glob->rdisplay, plot_frame->x->drawable,
		 ExposureMask | ButtonPressMask | ButtonReleaseMask);
  else
    XSelectInput(Glob->rdisplay, plot_frame->x->drawable, ExposureMask);

  /*
   * set button sensitivity
   */

  for (i = 0; i < n_buttons; i++) {
    
    sens = activity[i];

    if (sens) {
      XSelectInput(Glob->rdisplay, button_frame[i]->x->drawable,
		   ButtonPressMask | ButtonReleaseMask | LeaveWindowMask |
		   EnterWindowMask | ExposureMask);
    } else {
      XSelectInput(Glob->rdisplay, button_frame[i]->x->drawable,
		   ExposureMask);
    }
  }

  ufree((char *) activity);

}

/*********************************************************************
 * set_tscale_sens()
 */

void set_tscale_sens(void)

{

  if (Glob->debug) {
    fprintf(stderr, "** set_tscale_sens **\n");
  }

  if (Glob->archive_only) {
    
    if (Glob->use_case_tracks) {
      set_sens(Glob->tscale_window,
               Glob->tscale_title_frame, Glob->tscale_plot_frame,
               Glob->tscale_button_frame,
               N_TSCALE_BUTTONS, TSCALE_BUTTON_SENS_CASE_ANALYSIS,
               TRUE);
    } else {
      set_sens(Glob->tscale_window,
               Glob->tscale_title_frame, Glob->tscale_plot_frame,
               Glob->tscale_button_frame,
               N_TSCALE_BUTTONS, TSCALE_BUTTON_SENS_ARCHIVE_ONLY,
               TRUE);
    }
    
  } else if (Glob->realtime_only) {
    
    set_sens(Glob->tscale_window,
	     Glob->tscale_title_frame, Glob->tscale_plot_frame,
	     Glob->tscale_button_frame,
	     N_TSCALE_BUTTONS, TSCALE_BUTTON_SENS_REALTIME_ONLY,
	     FALSE);
    
  } else {
    
    set_sens(Glob->tscale_window,
	     Glob->tscale_title_frame, Glob->tscale_plot_frame,
	     Glob->tscale_button_frame,
	     N_TSCALE_BUTTONS, TSCALE_BUTTON_SENS,
	     TRUE);
    
  }
    
}

/*********************************************************************
 * set_thist_sens.c: set sensitivity of windows to events
 *
 *********************************************************************/

void set_thist_sens(void)

{

  if (Glob->debug) {
    fprintf(stderr, "** set_thist_sens **\n");
  }

  if (Glob->realtime_only) {

    set_sens(Glob->thist_window,
	     Glob->thist_title_frame, Glob->thist_plot_frame,
	     Glob->thist_button_frame,
	     N_THIST_BUTTONS, THIST_BUTTON_SENS,
	     FALSE);

  } else {

    set_sens(Glob->thist_window,
	     Glob->thist_title_frame, Glob->thist_plot_frame,
	     Glob->thist_button_frame,
	     N_THIST_BUTTONS, THIST_BUTTON_SENS,
	     TRUE);


  }

}

/*********************************************************************
 * set_timeht_sens.c: set sensitivity of windows to events
 *
 *********************************************************************/

void set_timeht_sens(void)

{

  if (Glob->debug) {
    fprintf(stderr, "** set_timeht_sens **\n");
  }

  if (Glob->realtime_only) {

    set_sens(Glob->timeht_window,
	     Glob->timeht_title_frame, Glob->timeht_plot_frame,
	     Glob->timeht_button_frame,
	     N_TIMEHT_BUTTONS, TIMEHT_BUTTON_SENS,
	     FALSE);

  } else {

    set_sens(Glob->timeht_window,
	     Glob->timeht_title_frame, Glob->timeht_plot_frame,
	     Glob->timeht_button_frame,
	     N_TIMEHT_BUTTONS, TIMEHT_BUTTON_SENS,
	     TRUE);

  }

}

/*********************************************************************
 * set_rdist_sens.c: set sensitivity of windows to events
 *
 *********************************************************************/

void set_rdist_sens(void)

{

  if (Glob->debug) {
    fprintf(stderr, "** set_rdist_sens **\n");
  }

  if (Glob->realtime_only ) {

    set_sens(Glob->rdist_window,
	     Glob->rdist_title_frame, Glob->rdist_plot_frame,
	     Glob->rdist_button_frame,
	     N_RDIST_BUTTONS, RDIST_BUTTON_SENS,
	     FALSE);

  } else {

    set_sens(Glob->rdist_window,
	     Glob->rdist_title_frame, Glob->rdist_plot_frame,
	     Glob->rdist_button_frame,
	     N_RDIST_BUTTONS, RDIST_BUTTON_SENS,
	     TRUE);

  }

}

/*********************************************************************
 * set_union_sens.c: set sensitivity of windows to events
 *
 *********************************************************************/

void set_union_sens(void)

{

  if (Glob->debug) {
    fprintf(stderr, "** set_union_sens **\n");
  }

  if (Glob->realtime_only) {

    set_sens(Glob->union_window,
	     Glob->union_title_frame, Glob->union_plot_frame,
	     Glob->union_button_frame,
	     N_UNION_BUTTONS, UNION_BUTTON_SENS,
	     FALSE);

  } else {

    set_sens(Glob->union_window,
	     Glob->union_title_frame, Glob->union_plot_frame,
	     Glob->union_button_frame,
	     N_UNION_BUTTONS, UNION_BUTTON_SENS,
	     TRUE);


  }

}

/*********************************************************************
 * set_help_sens.c: set sensitivity of windows to events
 *
 *********************************************************************/

void set_help_sens(void)

{

  if (Glob->debug) {
    fprintf(stderr, "** set_help_sens **\n");
  }

  set_sens(Glob->help_window,
	   Glob->help_title_frame, Glob->help_text_frame,
	   Glob->help_button_frame,
	   N_HELP_BUTTONS, HELP_BUTTON_SENS,
	   FALSE);

}
