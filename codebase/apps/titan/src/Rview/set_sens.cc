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

#include "Rview.hh"
using namespace std;

/**************************************************************************
 * init_flags - set flag bytes
 */

static void get_activity(ui08 *flags,
			 int n_buttons,
			 const char *set)

{

  si32 i;
  char *tmp_set;
  char *startptr, *endptr;

  tmp_set = (char *) umalloc((ui32)strlen(set) + 1);
  strcpy(tmp_set, set);

  startptr = tmp_set;

  for (i = 0; i < n_buttons; i++) {
    errno = 0;
    flags[i] = strtol(startptr, &endptr, 10);
    startptr = endptr;
    if (errno != 0) {
      fprintf(stderr, "ERROR - %s:set_sens:get_activity.\n",
	      Glob->prog_name);
      perror(set);
      tidy_and_exit(-1);
    }
  }
  
  ufree(tmp_set);

}

/*********************************************************************
 * set_cappi_sens()
 */

void set_cappi_sens()

{

  static int  first_call = TRUE;

  static ui08 realtime[N_CAPPI_BUTTONS];
  static ui08 archive[N_CAPPI_BUTTONS];
  static ui08 no_tracks[N_CAPPI_BUTTONS];
  static ui08 tracks_off[N_CAPPI_BUTTONS];
  static ui08 use_time_hist[N_CAPPI_BUTTONS];
  
  si32 i;
  ui08 sens;

  if (Glob->debug) {
    fprintf(stderr, "** set_cappi_sens **\n");
  }

  if (first_call) {

    /*
     * set bits in the flag arrays
     */

    get_activity(realtime, N_CAPPI_BUTTONS, CAPPI_BUTTON_REALTIME);
    get_activity(archive, N_CAPPI_BUTTONS, CAPPI_BUTTON_ARCHIVE);
    get_activity(no_tracks, N_CAPPI_BUTTONS, CAPPI_BUTTON_NO_TRACKS);
    get_activity(tracks_off, N_CAPPI_BUTTONS, CAPPI_BUTTON_TRACKS_OFF);
    get_activity(use_time_hist, N_CAPPI_BUTTONS, CAPPI_BUTTON_USE_TIME_HIST);

    first_call = FALSE;

  } /* first_call */

  /*
   * cappi input event solicitation
   */

  XSelectInput(Glob->rdisplay, Glob->main_window, StructureNotifyMask);

  XSelectInput(Glob->rdisplay, Glob->cappi_title_frame->x->drawable,
	       ExposureMask);

  XSelectInput(Glob->rdisplay, Glob->main_scale_frame->x->drawable,
	       ExposureMask);

  XSelectInput(Glob->rdisplay, Glob->cappi_plot_frame->x->drawable,
	       ExposureMask | ButtonPressMask | ButtonReleaseMask |
	       EnterWindowMask | LeaveWindowMask | 
	       ButtonMotionMask | PointerMotionMask);

  /*
   * set button sensitivity
   */

  for (i = 0; i < N_CAPPI_BUTTONS; i++) {

    if (Glob->mode == REALTIME)
      sens = realtime[i];
    else
      sens = archive[i];

    if (!Glob->use_track_data)
      sens = sens && no_tracks[i];

    if (!Glob->plot_tracks)
      sens = sens && tracks_off[i];

    if (Glob->use_time_hist)
      sens = sens && use_time_hist[i];

    if (sens) {
      XSelectInput(Glob->rdisplay, Glob->cappi_button_frame[i]->x->drawable,
		   ButtonPressMask | ButtonReleaseMask | LeaveWindowMask |
		   EnterWindowMask | ExposureMask);
    } else {
      XSelectInput(Glob->rdisplay, Glob->cappi_button_frame[i]->x->drawable,
		   ExposureMask);
    }
  }

}

/*********************************************************************
 * set_vsection_sens()
 */

void set_vsection_sens()

{

  static int  first_call = TRUE;

  static ui08 all[N_VSECTION_BUTTONS];
  
  si32 i;
  ui08 sens;

  if (Glob->debug) {
    fprintf(stderr, "** set_vsection_sens **\n");
  }

  if (first_call) {

    /*
     * set bytes in the flag arrays
     */

    get_activity(all, N_VSECTION_BUTTONS, VSECTION_BUTTON_SENS);
    first_call = FALSE;

  } /* first_call */

  /*
   * vsection input event solicitation
   */
  
  XSelectInput(Glob->rdisplay, Glob->vsection_window, StructureNotifyMask);

  XSelectInput(Glob->rdisplay, Glob->vsection_title_frame->x->drawable,
	       ExposureMask);

  XSelectInput(Glob->rdisplay, Glob->vsection_plot_frame->x->drawable,
	       ExposureMask | ButtonPressMask | ButtonReleaseMask);

  /*
   * set button sensitivity
   */

  for (i = 0; i < N_VSECTION_BUTTONS; i++) {
    
    sens = all[i];

    if (sens) {
      XSelectInput(Glob->rdisplay, Glob->vsection_button_frame[i]->x->drawable,
		   ButtonPressMask | ButtonReleaseMask | LeaveWindowMask |
		   EnterWindowMask | ExposureMask);
    } else {
      XSelectInput(Glob->rdisplay, Glob->vsection_button_frame[i]->x->drawable,
		   ExposureMask);
    }
  }

}

/*********************************************************************
 * set_help_sens.c: set sensitivity of windows to events
 *
 *********************************************************************/

void set_help_sens()

{

  static int  first_call = TRUE;

  static ui08 all[N_HELP_BUTTONS];
  
  si32 i;
  ui08 sens;

  if (Glob->debug) {
    fprintf(stderr, "** set_help_sens **\n");
  }

  if (first_call) {

    /*
     * set bytes in the flag arrays
     */

    get_activity(all, N_HELP_BUTTONS, HELP_BUTTON_SENS);
    first_call = FALSE;

  } /* first_call */

  /*
   * help input event solicitation
   */
  
  XSelectInput(Glob->rdisplay, Glob->help_window, StructureNotifyMask);
  
  XSelectInput(Glob->rdisplay, Glob->help_title_frame->x->drawable,
	       ExposureMask);
  
  XSelectInput(Glob->rdisplay, Glob->help_text_frame->x->drawable,
	       ExposureMask);

  /*
   * set button sensitivity
   */

  for (i = 0; i < N_HELP_BUTTONS; i++) {
    
    sens = all[i];

    if (sens) {
      XSelectInput(Glob->rdisplay, Glob->help_button_frame[i]->x->drawable,
		   ButtonPressMask | ButtonReleaseMask | LeaveWindowMask |
		   EnterWindowMask | ExposureMask);
    } else {
      XSelectInput(Glob->rdisplay, Glob->help_button_frame[i]->x->drawable,
		   ExposureMask);
    }
  }

}

