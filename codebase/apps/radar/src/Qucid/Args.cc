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
//////////////////////////////////////////////////////////
// Args.cc : command line args
//
// Mike Dixon, EOL, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
//////////////////////////////////////////////////////////

#include "Args.hh"
#include "Params.hh"
#include "LegacyParams.hh"
#include "cidd.h"
#include <cstring>
#include <cstdlib>
#include <toolsa/DateTime.hh>
using namespace std;

// Constructor

Args::Args (const string &prog_name)

{
  _progName = prog_name;
  _usingLegacyParams = false;
}


// Destructor

Args::~Args ()

{
  TDRP_free_override(&override);
}

// Parse command line
// Returns 0 on success, -1 on failure

int Args::parse (const int argc, const char **argv)

{

  char tmp_str[BUFSIZ];

  // intialize

  int iret = 0;
  TDRP_init_override(&override);
  gd.app_instance = "test";

  // process the legacy args
  
  if (_processLegacyArgs(argc, argv)) {
    iret = -1;
  }

  // loop through args
  
  for (int i =  1; i < argc; i++) {

    if (!strcmp(argv[i], "--") ||
	!strcmp(argv[i], "-h") ||
	!strcmp(argv[i], "-help") ||
	!strcmp(argv[i], "-man")) {
      
      _usage(cout);
      exit (0);
      
    } else if (!strcmp(argv[i], "-debug") ||
               !strcmp(argv[i], "-d")) {
      
      snprintf(tmp_str, BUFSIZ,  "debug = DEBUG_NORM;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-verbose") ||
               !strcmp(argv[i], "-v")) {
      
      snprintf(tmp_str, BUFSIZ,  "debug = DEBUG_VERBOSE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-extra") ||
               !strcmp(argv[i], "-vv")) {
      
      snprintf(tmp_str, BUFSIZ,  "debug = DEBUG_EXTRA;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-color_scales")) {
      
      if (i < argc - 1) {
        snprintf(tmp_str, BUFSIZ,  "color_scale_dir = \"%s\";", argv[++i]);
        TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-instance")) {
      
      if (i < argc - 1) {
        const char *instance = argv[++i];
        snprintf(tmp_str, BUFSIZ,  "instance = \"%s\";", instance);
        TDRP_add_override(&override, tmp_str);
        snprintf(tmp_str, BUFSIZ,  "register_with_procmap = TRUE;");
        TDRP_add_override(&override, tmp_str);
        gd.app_instance = strdup(instance);
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-start_x")) {
      
      if (i < argc - 1) {
        snprintf(tmp_str, BUFSIZ,  "main_window_start_x = %s;", argv[++i]);
        TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-start_y")) {
      
      if (i < argc - 1) {
        snprintf(tmp_str, BUFSIZ,  "main_window_start_y = %s;", argv[++i]);
        TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-realtime")) {
      
      snprintf(tmp_str, BUFSIZ,  "begin_in_archive_mode = FALSE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-start_time")) {
      
      if (i < argc - 1) {
        snprintf(tmp_str, BUFSIZ,  "archive_start_time = \"%s\";", argv[++i]);
        TDRP_add_override(&override, tmp_str);
        snprintf(tmp_str, BUFSIZ,  "begin_in_archive_mode = TRUE;");
        TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-images_start_time")) {
      
      if (i < argc - 1) {
        snprintf(tmp_str, BUFSIZ,  "images_archive_start_time = \"%s\";", argv[++i]);
        TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-images_end_time")) {
      
      if (i < argc - 1) {
        snprintf(tmp_str, BUFSIZ,  "images_archive_end_time = \"%s\";", argv[++i]);
        TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-image_interval")) {
      
      if (i < argc - 1) {
        snprintf(tmp_str, BUFSIZ,  "images_schedule_interval_secs = %s;", argv[++i]);
        TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-time_span")) {
      
      if (i < argc - 1) {
        snprintf(tmp_str, BUFSIZ,  "bscan_time_span_secs = %s;", argv[++i]);
        TDRP_add_override(&override, tmp_str);
        snprintf(tmp_str, BUFSIZ,  "archive_time_span_secs = %s;", argv[i]);
        TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
      
    } else if (argv[i][0] == '-') {

      cerr<< "====>> WARNING - invalid command line argument: '"
          << argv[i] << "' <<====" << endl;

    } // if
    
  } // i

  if (iret) {
    _usage(cerr);
  }

  return (iret);
    
}

//////////////////////////////////////////////////////
// get the legacy params file from the command line
// returns 0 on success, -1 on failure

int Args::getLegacyParamsPath(const int argc, const char **argv,
                              string &legacyPath)
{
  for (int ii =  1; ii < argc; ii++) {
    if (!strcmp(argv[ii], "-p")) {
      if (ii < argc - 1) {
        const char *optarg = argv[++ii];
        legacyPath = optarg;
        return 0;
      }
    }
  } // ii
  // failure
  return -1;
}

//////////////////////////////////////////////////////
// get the tdrp params path from the command line
// returns 0 on success, -1 on failure

int Args::getTdrpParamsPath(const int argc, const char **argv,
                            string &tdrpPath)
{
  for (int ii =  1; ii < argc; ii++) {
    if (!strcmp(argv[ii], "-params")) {
      if (ii < argc - 1) {
        const char *optarg = argv[++ii];
        tdrpPath = optarg;
        return 0;
      }
    }
  } // ii
  // failure
  return -1;
}


//////////////////////////////////////////////////////
// get the print mode from the command line
// returns 0 on success, -1 on failure

int Args::getTdrpPrintMode(const int argc, const char **argv,
                           tdrp_print_mode_t &printMode)
{
  printMode = NO_PRINT;
  for (int ii =  1; ii < argc; ii++) {
    if (!strcmp(argv[ii], "-print_params")) {
      if (ii < argc - 1) {
        const char *mode = argv[++ii];
	if (!strcmp(mode, "short")) {
	  printMode = PRINT_SHORT;
	} else if (!strcmp(mode, "norm")) {
	  printMode = PRINT_NORM;
	} else if (!strcmp(mode, "long")) {
	  printMode = PRINT_LONG;
	} else if (!strcmp(mode, "verbose")) {
	  printMode = PRINT_VERBOSE;
	} else {
	  printMode = PRINT_NORM;
        }
      } else {
        printMode = NO_PRINT;
      }
      return 0;
    }
  } // ii
  // failure
  return -1;
}


//////////////////////////////////////////////////////
// usage

void Args::_usage(ostream &out)

{

  out << "Usage: " << _progName << " [options as below]\n"
      << "options:\n"
      << "       [ --, -h, -help, -man ] produce this list.\n"
      << "       [ -archive_url ?] URL for data in archive mode\n"
      << "       [ -color_scales ? ] specify color scale directory\n"
      << "       [ -debug, -d ] print debug messages\n"
      << "       [ -images_end_time \"yyyy mm dd hh mm ss\"]\n"
      << "            set end time for image generation mode\n"
      << "       [ -image_interval ?]\n"
      << "            set image generation interval (secs)\n"
      << "       [ -images_start_time \"yyyy mm dd hh mm ss\"]\n"
      << "            set start time for image generation mode\n"
      << "       [ -instance ?] set instance for procmap\n"
      << "       [ -realtime] start in realtime mode\n"
      << "       [ -start_time \"yyyy mm dd hh mm ss\"]\n"
      << "            set start time for archive mode\n"
      << "       [ -start_x ? ] start x location of main window\n"
      << "       [ -start_y ? ] start y location of main window\n"
      << "       [ -time_span ?]\n"
      << "            set time span (secs)\n"
      << "            applies to bscan time width\n"
      << "            or archive mode time span\n"
      << "       [ -v, -verbose ] print verbose debug messages\n"
      << "       [ -vv, -extra ] print extra verbose debug messages\n"
      << "legacy options:\n"
      << "       [ -p ?] legacy params file\n"
      << "       [ -quiet ] quiet mode\n"
      << "       [ -unmapped ] run unmapped - i.e. no window pops up\n"
      << "       [ -vert ] run in vert mode, cosine correction for elevation not applied\n"
      << "       [ -print_legacy_params ] print out legacy params\n"
      << "       [ -vvv ? ] legacy CIDD verbose level\n"
      << "       [ -proxy_url ?] use this proxy server for data requests\n"
      << "                       in the param file, and in HTML Mode will automatically\n"
      << "       [ -h ?,?,?...] starts up at the first height, overiding the setting\n"
      << "                       in the param file, and in HTML Mode will automatically\n"
      << "                       render each height in the list\n"
      << "       [ -t ?] starts up in archive mode at this time\n"
      << "               YYYYMMDDHHMM(SS - seconds optional)\n"
      << "               -t minus3600 starts in archive mode with the\n"
      << "                  first frame 1 hour ago\n"
      << "               i.e. use -t minusX to start X seconds ago\n"
      << endl;

  Params::usage(out);

}

int Args::_processLegacyArgs(int argc, const char **argv)
{

  int n_fields;
  const char *app_ptr;
  const char *token;
  UTIMstruct temp_utime;

  // gd.db_name = strdup("");    /* Set the default data base name */
  TDRP_str_replace(&_params.http_proxy_url, "");

  gd.argv = (char **) argv;
  gd.argc = argc;
  gd.orig_wd = getcwd(NULL,0);

  if((app_ptr = strrchr(argv[0],'/')) == NULL) {
    gd.app_name = argv[0];
  } else {
    gd.app_name = ++app_ptr;
  }

  // Look for the quiet mode flag first.
  for(int ii=1; ii < argc; ii++) {
    if(strcmp(argv[ii], "-quiet") == 0 ) {
      gd.quiet_mode = 1;
    }
  }
     
  // loop through args

  int iret = 0;
  
  for (int ii =  1; ii < argc; ii++) {
    
    if (!strcmp(argv[ii], "-p")) {

      if (ii < argc - 1) {
        const char *optarg = argv[++ii];
        // gd.db_name = strdup(optarg);
        _usingLegacyParams = true;
        _legacyParamsPath = strdup(optarg); 
      } else {
	iret = -1;
      }

    } else if (!strcmp(argv[ii], "-print_legacy_params")) {

      LegacyParams lparams;
      lparams.printLegacyDefaults(cout);
      exit(0);

    } else if (!strcmp(argv[ii], "-vvv")) {
      
      if (ii < argc - 1) {
        const char *optarg = argv[++ii];
        int debug_level = atoi(optarg);
        gd.debug |= (debug_level & 0x01);
        gd.debug1 |= (debug_level & 0x02);
        gd.debug2 |= (debug_level & 0x04);
      } else {
        gd.debug = 1;
      }
 
    } else if (!strcmp(argv[ii], "-proxy_url")) {
      
      if (ii < argc - 1) {
        const char *optarg = argv[++ii];
        _params.http_proxy_url = strdup(optarg);
        if(!gd.quiet_mode) {
          printf("Loading Parameters via Proxy: %s\n",_params.http_proxy_url);
        }
      } else {
	iret = -1;
      }
 
    } else if (!strcmp(argv[ii], "-start_height")) {
      
      if (ii < argc - 1) {
        const char *optarg = argv[++ii];
        gd.num_render_heights = 0;
        gd.cur_render_height = 0;
        token = strtok((char *) optarg,","); // Prime strtok
        gd.h_win.cur_ht = atof(token);
        while(token != NULL && gd.num_render_heights < MAX_SECTS) {
          gd.height_array[gd.num_render_heights] = atof(token);
          gd.num_render_heights++;
          token = strtok(NULL,","); // get next rtoken strtok
        }
        if(!gd.quiet_mode) printf("CIDD Found %d heights starting at: %g\n",
                                  gd.num_render_heights,gd.h_win.cur_ht);
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[ii], "-unmapped")) {

      gd.run_unmapped = 1;
      if(!gd.quiet_mode) printf("CIDD will run unmapped\n");
      
    } else if (!strcmp(argv[ii], "-vert")) {
      
      gd.use_cosine_correction = 1;
      if(!gd.quiet_mode) printf("CIDD will run in VERT pointing mode\n");
      
    } else if (!strcmp(argv[ii], "-start_time")) {
      
      if (ii < argc - 1) {

        const char *optarg = argv[++ii];
        if (strncmp(optarg, "minus", strlen("minus"))==0){
          time_t throwBack;
          if (1!=sscanf(optarg,"minus%ld", &throwBack)){
            fprintf(stderr,"Problems parsing time: %s\n",optarg);
            fprintf(stderr,"Use: -t minus3600 to look back one hour\n");
          } else {
            time_t minusTime = time(NULL) - throwBack;
            gd.movie.mode = ARCHIVE_MODE;
            gd.movie.start_time = minusTime;
            gd.movie.demo_time = minusTime;
            if(!gd.quiet_mode)
              printf("11 CIDD Starting in ARCHIVE mode with Data time: %s\n",
                     DateTime::strm(minusTime).c_str());
          }
        } else {
          //
          // Are seconds included in the time string?
          //
          if (strlen(optarg) == strlen("YYYYMMDDhhmmss")){
            //
            // Yes, we do have seconds.
            //
            n_fields = sscanf(optarg,"%4ld%2ld%2ld%2ld%2ld%2ld",
                              &temp_utime.year,
                              &temp_utime.month,
                              &temp_utime.day,
                              &temp_utime.hour,
                              &temp_utime.min,
                              &temp_utime.sec );
            
            if(n_fields == 6) {
              temp_utime.unix_time = UTIMdate_to_unix(&temp_utime);
              gd.movie.mode = ARCHIVE_MODE;
              gd.movie.start_time = temp_utime.unix_time;
              gd.movie.demo_time = temp_utime.unix_time;
              if(!gd.quiet_mode)
                printf("22 CIDD Starting in ARCHIVE mode with Data time: %s\n",
                       DateTime::strm(temp_utime.unix_time).c_str());
            } else {
              fprintf(stderr,"Problems parsing time: %s\n",optarg);
              fprintf(stderr,"Use: YYYYMMDDHHMMSS : Example 199806211624\n");
              fprintf(stderr,"     YYYYMMDDHHMM is also an option.\n");
            } 
          } else {
            //
            // No seconds in time string.
            //
            n_fields = sscanf(optarg,"%4ld%2ld%2ld%2ld%2ld",
                              &temp_utime.year,
                              &temp_utime.month,
                              &temp_utime.day,
                              &temp_utime.hour,
                              &temp_utime.min);
            
            if(n_fields == 5) {
              temp_utime.sec = 0;
              temp_utime.unix_time = UTIMdate_to_unix(&temp_utime);
              gd.movie.mode = ARCHIVE_MODE;
              gd.movie.start_time = temp_utime.unix_time;
              gd.movie.demo_time = temp_utime.unix_time;
              if(!gd.quiet_mode) 
                printf("33 CIDD Starting in ARCHIVE mode with Data time: %s\n",
                       DateTime::strm(temp_utime.unix_time).c_str());
            } else {
              fprintf(stderr,"Problems parsing time: %s\n",optarg);
              fprintf(stderr,"Use: YYYYMMDDHHMM : Example 199806211624\n");
              fprintf(stderr,"     YYYYMMDDHHMMSS is also an option.\n");
            }
          }

        } // if (strncmp(optarg, "minus", strlen("minus"))==0) ...

      } // if (ii < argc - 1) {

    } // else if (!strcmp(argv[i], "-start_time"))

  } // ii

  return iret;
  
}  
