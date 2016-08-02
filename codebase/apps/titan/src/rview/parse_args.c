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
/*********************************************************************
 * parse_args.c: parse command line args, open files as required
 *
 * RAP, NCAR, Boulder CO
 *
 * June 1990   
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "rview.h"

void parse_args(int argc, char **argv)

{

  int error_flag = 0;
  int warning_flag = 0;
  int i;
  int geometrystatus;
  int malloc_debug_level;

  double dummy;
  si32 ldummy;

  char *end_pt;
  char *mode_str, *debug_str, *malloc_debug_str;
  char *time_requested_str;
  char usage[BUFSIZ];

  date_time_t ttime;

  /*
   * load up usage string
   */

  sprintf(usage, "%s%s%s%s",
	  "Usage:\n\n", argv[0], " [options] as below:\n\n",
	  "       [ --, -h, -help, -man] produce this list.\n"
	  "       [ -bg, -background ?] set background color\n"
	  "       [ -debug ] debugging on\n"
	  "       [ -d, -display ?] display name\n"
	  "       [ -fg, -foreground ?] set foreground color\n"
	  "       [ -g, -geometry ?] geometry as per X manual\n"
	  "       [ -instance ?] program instance\n"
	  "       [ -noparams] use X data base instead of params file\n"
	  "       [ -mdebug ?] set malloc debug level\n"
	  "       [ -mode ?] archive or realtime\n"
	  "       [ -no_time_hist ?] do not use time_hist\n"
	  "       [ -no_tracks ?] do not use track data\n"
	  "       [ -params ?] parameters file name\n"
	  "       [ -time  yyyy/mm/dd_hh:mm:ss ] data date and time\n"
	  "       [ -z ?] requested ht above MSL\n"
	  "\n");

  Glob->instance = xGetResString(Glob->rdisplay, Glob->prog_name,
				 "instance", "Test");

  mode_str = xGetResString(Glob->rdisplay, Glob->prog_name,
			   "mode", MODE);

  debug_str = xGetResString(Glob->rdisplay, Glob->prog_name,
			    "debug", DEBUG_STR);

  malloc_debug_str = xGetResString(Glob->rdisplay, Glob->prog_name,
				   "malloc_debug_level", "0");
  
  time_requested_str = xGetResString(Glob->rdisplay, Glob->prog_name,
			   "time_requested", TIME_REQUESTED);
  
  /*
   * search for command options
   */
  
  for (i =  1; i < argc; i++) {
    
    if (!strcmp(argv[i], "--") ||
	!strcmp(argv[i], "-h") ||
	!strcmp(argv[i], "-help") ||
	!strcmp(argv[i], "-man")) {
      printf("%s", usage);
      tidy_and_exit(1);
      
    } else if (!strcmp(argv[i], "-debug")) {

      debug_str = "true";
      
    } else if (!strcmp(argv[i], "-no_time_hist")) {
      
      Glob->use_time_hist = FALSE;
      
    } else if (!strcmp(argv[i], "-no_tracks")) {
      
      Glob->use_track_data = FALSE;
      Glob->use_time_hist = FALSE;
      Glob->plot_tracks = FALSE;
      
    }
    
    if (i < argc - 1) {
      
      if (!strcmp(argv[i], "-mdebug")) {
	
	malloc_debug_str = argv[i+1];
	
      } else if (!strcmp(argv[i], "-mode")) {
	
	mode_str = argv[i+1];

      } else if (!strcmp(argv[i], "-instance")) {
	
	Glob->instance = argv[i+1];
	
      } else if (!strcmp(argv[i], "-z")) {
	
        errno = 0;
	dummy = (double) strtod(argv[i+1], &end_pt);
	if (errno == 0)
	  Glob->z_cappi = dummy;

      } else if (!strcmp(argv[i], "-foreground") ||
		 !strcmp(argv[i], "-fg")) {
	
	Glob->foregroundstr =
	  (char *) umalloc((ui32)strlen(argv[i+1]) + 1);
	strcpy(Glob->foregroundstr, argv[i+1]);
	
      } else if (!strcmp(argv[i], "-background") ||
		 !strcmp(argv[i], "-bg")) {
	
	Glob->backgroundstr =
	  (char *) umalloc((ui32)strlen(argv[i+1]) + 1);
	strcpy(Glob->backgroundstr, argv[i+1]);
	
      } else if (!strcmp(argv[i], "-geometry") ||
		 !strcmp(argv[i], "-g")) {
	
	geometrystatus = XParseGeometry(argv[i+1], &Glob->mainx,
					&Glob->mainy,
					&Glob->mainwidth,
					&Glob->mainheight);

	if(!(geometrystatus & XValue))
	  Glob->mainx = X_MAINX;
	
	if(!(geometrystatus & YValue))
	  Glob->mainy = X_MAINY;
	
	if(!(geometrystatus & WidthValue))
	  Glob->mainwidth = (ui32) X_MAINWIDTH;
	
	if(!(geometrystatus & HeightValue))
	  Glob->mainheight = (ui32) X_MAINHEIGHT;
	
	if((geometrystatus & XNegative))
	  Glob->mainx_sign = -1;
	else
	  Glob->mainx_sign = 1;
	
	if((geometrystatus & YNegative))
	  Glob->mainy_sign = -1;
	else
	  Glob->mainy_sign = 1;

      } else if (!strcmp(argv[i], "-time")) {

	time_requested_str = argv[i+1];
	
      } /* if (!strcmp ..... */
    
    } /* if (i < argc - 1) */
    
  } /* i */

  /*
   * set debug option
   */
  
  if (!strcmp(debug_str, "true")) {
    Glob->debug = TRUE;
  } else if (!strcmp(debug_str, "false")) {
    Glob->debug = FALSE;
  } else {
    fprintf(stderr, "ERROR - %s:parse_args\n", Glob->prog_name);
    fprintf(stderr,
	    "Debug option '%s' invalid - should be 'true' or 'false'.\n",
	    debug_str);
    error_flag = TRUE;
  }
  
  /*
   * set the malloc debug level
   */

  errno = 0;
  ldummy = (double) strtol(malloc_debug_str, &end_pt, 10);

  if (errno == 0)
    malloc_debug_level = ldummy;
  else
    malloc_debug_level = 0;

  umalloc_debug ((int) malloc_debug_level);

  /*
   * set data type mode
   */

  if (!strcmp(mode_str, "realtime"))
    Glob->mode = REALTIME;
  else if (!strcmp(mode_str, "archive"))
    Glob->mode = ARCHIVE;
  else {
    fprintf(stderr, "ERROR - %s:parse_args\n", Glob->prog_name);
    fprintf(stderr, "Invalid data mode '%s'\n", mode_str);
    fprintf(stderr, "Options are 'realtime' and 'archive'\n");
    error_flag = TRUE;
  }

  /*
   * set requested time
   */

  if (sscanf(time_requested_str, "%d/%d/%d_%d:%d:%d",
	     &ttime.year, &ttime.month, &ttime.day,
	     &ttime.hour, &ttime.min, &ttime.sec) != 6) {
    
    fprintf(stderr, "ERROR - %s:parse_args\n", Glob->prog_name);
    fprintf(stderr, "Incorrect format for requested date and time.\n");
    fprintf(stderr, "Trying to decode '%s'\n", time_requested_str);
    fprintf(stderr, "Format is yyyy/mm/dd_hh:mm:ss\n");
    error_flag = TRUE;
    
  } /* if (sscanf ....... */

  Glob->time = uunix_time(&ttime);
	
  /*
   * print usage if there was an error
   */

  if(error_flag || warning_flag) {
    fprintf(stderr, "%s\n", usage);
    fprintf(stderr, "Check the parameters file '%s'.\n\n",
	    Glob->params_path_name);
  }

  if (error_flag)
    tidy_and_exit(1);

}
