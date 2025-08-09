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
 * parse_args.c: parse command line args, open files as required
 *
 * RAP, NCAR, Boulder CO
 *
 * June 1990   
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "Rview.hh"
#include "params.hh"
#include <iostream>
using namespace std;

static void _usage(FILE *out);

void parse_args(int argc, char **argv)

{

  int error_flag = 0;
  int warning_flag = 0;
  int i;
  int geometrystatus;

  double dummy;

  char *end_pt;
  char *mode_str, *debug_str, *verbose_str, *localtime_str;
  char *time_requested_str;
  char *auto_advance_start_time_str;
  char *auto_advance_end_time_str;
  date_time_t ttime;

  Glob->instance = uGetParamString(Glob->prog_name,
				   "instance", "Test");
  
  mode_str = uGetParamString(Glob->prog_name,
			     "mode", MODE);
  
  debug_str = uGetParamString(Glob->prog_name,
			      "debug", DEBUG_STR);
  
  verbose_str = uGetParamString(Glob->prog_name,
                                "verbose", "false");
  
  localtime_str = uGetParamString(Glob->prog_name,
                                  "localtime", "false");
  
  time_requested_str =
    uGetParamString(Glob->prog_name,
		    "time_requested", "1970 01 01 00 00 00");
  
  auto_advance_start_time_str =
    uGetParamString(Glob->prog_name,
		    "auto_advance_start_time", "1970 01 01 00 00 00");
  
  auto_advance_end_time_str =
    uGetParamString(Glob->prog_name,
		    "auto_advance_end_time", "1970 01 01 00 00 00");
  
  /*
   * search for command options
   */
  
  for (i =  1; i < argc; i++) {
    
    if (!strcmp(argv[i], "--") ||
	!strcmp(argv[i], "-h") ||
	!strcmp(argv[i], "-help") ||
	!strcmp(argv[i], "-man")) {
      _usage(stderr);
      tidy_and_exit(1);
      
    } else if (!strcmp(argv[i], "-auto_advance")) {

      Glob->auto_advance = true;
      
    } else if (!strcmp(argv[i], "-debug")) {

      debug_str = (char *) "true";
      
    } else if (!strcmp(argv[i], "-verbose")) {

      verbose_str = (char *) "true";
      
    } else if (!strcmp(argv[i], "-localtime")) {

      localtime_str = (char *) "true";
      
    } else if (!strcmp(argv[i], "-no_time_hist")) {
      
      Glob->use_time_hist = FALSE;
      
    } else if (!strcmp(argv[i], "-no_tracks")) {
      
      Glob->use_track_data = FALSE;
      Glob->use_time_hist = FALSE;
      Glob->plot_tracks = FALSE;
      
    } else if (i < argc - 1) {
      
      if (!strcmp(argv[i], "-mode")) {
	
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
	
      } else if (!strcmp(argv[i], "-start_time")) {

	auto_advance_start_time_str = argv[i+1];
	
      } else if (!strcmp(argv[i], "-end_time")) {

	auto_advance_end_time_str = argv[i+1];
	
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
   * set verbose option
   */
  
  if (!strcmp(verbose_str, "true")) {
    Glob->verbose = TRUE;
  } else if (!strcmp(verbose_str, "false")) {
    Glob->verbose = FALSE;
  } else {
    fprintf(stderr, "ERROR - %s:parse_args\n", Glob->prog_name);
    fprintf(stderr,
	    "Verbose option '%s' invalid - should be 'true' or 'false'.\n",
	    verbose_str);
    error_flag = TRUE;
  }
  
  /*
   * set localtime option
   */
  
  if (!strcmp(localtime_str, "true")) {
    Glob->localtime = TRUE;
  } else if (!strcmp(localtime_str, "false")) {
    Glob->localtime = FALSE;
  } else {
    fprintf(stderr, "ERROR - %s:parse_args\n", Glob->prog_name);
    fprintf(stderr,
	    "Localtime option '%s' invalid - should be 'true' or 'false'.\n",
	    localtime_str);
    error_flag = TRUE;
  }
  
  /*
   * set ops mode
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

  if (Glob->mode == REALTIME) {
    Glob->time = std::time(NULL);
  } else {
    if (sscanf(time_requested_str, "%d %d %d %d %d %d",
	       &ttime.year, &ttime.month, &ttime.day,
	       &ttime.hour, &ttime.min, &ttime.sec) != 6) {
      
      fprintf(stderr, "ERROR - %s:parse_args\n", Glob->prog_name);
      fprintf(stderr, "Incorrect format for requested date and time.\n");
      fprintf(stderr, "Trying to decode '%s'\n", time_requested_str);
      fprintf(stderr, "Format is \"yyyy mm dd hh mm ss\"\n");
      error_flag = TRUE;
    } /* if (sscanf ....... */
    Glob->time = uunix_time(&ttime);
  }
  
  // auto_advance is only applicable in ARCHIVE mode

  if (Glob->mode != ARCHIVE) {
    Glob->auto_advance = false;
  }

  if (Glob->auto_advance) {

    date_time_t dtime;

    if (sscanf(auto_advance_start_time_str, "%d %d %d %d %d %d",
	       &dtime.year, &dtime.month, &dtime.day,
	       &dtime.hour, &dtime.min, &dtime.sec) == 6) {
      uconvert_to_utime(&dtime);
      Glob->auto_advance_start_time = dtime.unix_time;
    } else {
      cerr << "ERROR - Rview::parse_args()" << endl;
      cerr << "  cannot parse auto_advance_start_time: "
	   << auto_advance_start_time_str << endl;
      cerr << "  auto_advance will be disabled" << endl;
      Glob->auto_advance = false;
    }
    
    if (sscanf(auto_advance_end_time_str, "%d %d %d %d %d %d",
	       &dtime.year, &dtime.month, &dtime.day,
	       &dtime.hour, &dtime.min, &dtime.sec) == 6) {
      uconvert_to_utime(&dtime);
      Glob->auto_advance_end_time = dtime.unix_time;
    } else {
      cerr << "ERROR - Rview::parse_args()" << endl;
      cerr << "  cannot parse auto_advance_end_time: "
	   << auto_advance_end_time_str << endl;
      cerr << "  auto_advance will be disabled" << endl;
      Glob->auto_advance = false;
    }

  }
    
  if (Glob->auto_advance) {
    Glob->time = Glob->auto_advance_start_time;
  }

  /*
   * print usage if there was an error
   */
  
  if(error_flag || warning_flag) {
    _usage(stderr);
    fprintf(stderr, "Check the parameters file '%s'.\n\n",
	    Glob->params_path_name);
  }

  if (error_flag)
    tidy_and_exit(1);

}

//////////////////////////////
// check for -print_params arg

int check_for_print_params(int argc, char **argv)

{

  for (int i =  1; i < argc; i++) {
    
    if (!strcmp(argv[i], "-print_params")) {
      cout << "#################################################################" << endl;
      cout << "#  Example parameters file for Rview\n";
      cout << "#" << endl;
      cout << "#  " << DateTime::str() << endl;
      cout << "#################################################################" << endl;
      cout << endl;
      cout << ParamsText;
      return -1;
    }
    
  }

  return 0;

}

///////////////////////////////////////////////
// print usage

static void _usage(FILE *out)

{

  fprintf(out, "Usage:\n\n");
  fprintf(out, "Rview [options] as below:\n\n");
  fprintf(out, "   [ -- -h, -help, -man] produce this list.\n");
  fprintf(out, "   [ -bg -background ?] set background color\n");
  fprintf(out, "   [ -auto_advance ] set auto_advance on\n");
  fprintf(out, "     ARCHIVE mode only.\n");
  fprintf(out, "   [ -debug ] debugging on\n");
  fprintf(out, "   [ -verbose ] verbose debugging on\n");
  fprintf(out, "   [ -d, -display ?] display name\n");
  fprintf(out, "   [ -fg, -foreground ?] set foreground color\n");
  fprintf(out, "   [ -g, -geometry ?] geometry as per X manual\n");
  fprintf(out, "   [ -instance ?] program instance\n");
  fprintf(out, "   [ -localtime ] use local time instead of UTC\n");
  fprintf(out, "   [ -mode ?] archive or realtime\n");
  fprintf(out, "   [ -noparams] use X data base instead of params file\n");
  fprintf(out, "   [ -no_time_hist ?] do not use time_hist\n");
  fprintf(out, "   [ -no_tracks ?] do not use track data\n");
  fprintf(out, "   [ -params ?] parameters file name\n");
  fprintf(out, "   [ -print_params] print parameters\n");
  fprintf(out, "   [ -time  ? ] set time\n");
  fprintf(out, "     Format: \"yyyy mm dd hh mm ss\"\n");
  fprintf(out, "   [ -start_time  ? ] auto_advance start time\n");
  fprintf(out, "     Format: \"yyyy mm dd hh mm ss\"\n");
  fprintf(out, "   [ -end_time  ? ] auto_advance end time\n");
  fprintf(out, "     Format: \"yyyy mm dd hh mm ss\"\n");
  fprintf(out, "   [ -z ?] requested ht above MSL\n");
  fprintf(out, "\n");

}


