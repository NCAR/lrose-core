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
/*****************************************************************
 * PROCESS_ARGS: Progess command line arguments. Set option flags
 *       And print usage info if necessary
 */

#define PROCESS_ARGS

#include "cidd.h"
#include <toolsa/DateTime.hh>
#define ARG_OPTION_STRING   "h:v:x:d:p:i:t:Vuq"

void process_args(int argc, char *argv[])
{
    int c,i;
    int n_fields;
    int debug_level;
    char dir_buf[MAX_PATH_LEN];
    const char *app_ptr;
    // char *slash;
    const char *token;
    extern  char *optarg;   /* option argument string */
    UTIMstruct     temp_utime;
    int err_flag =0;

    // gd.db_name = strdup("");    /* Set the default data base name */
    TDRP_str_replace(&_params.http_proxy_url, "");

    gd.argv = argv;
    gd.argc = argc;
    gd.orig_wd = getcwd(NULL,0);

    if((app_ptr = strrchr(argv[0],'/')) == NULL) {
      gd.app_name = argv[0];
    } else {
      gd.app_name = ++app_ptr;
    }
    gd.app_instance = "Generic";

    // Look for the quiet mode flag first.
    for(i=1; i < argc; i++) {
	if(strncmp(argv[i],"-q",2) == 0 ) gd.quiet_mode = 1;
    }
     
    while ((c = getopt(argc, argv,ARG_OPTION_STRING)) != EOF) {
        switch(c) {
            case 'd' :    /* parameter database file */
            case 'p' :    
                /* Catch -params */
		STRcopy(dir_buf,optarg,1024);
		if(strncmp(dir_buf,"arams",5) == 0) { // Catch -pararms
		    STRcopy(dir_buf,argv[optind],1024);
		}
                /* Change working directories to the place where the config file exists */
                // if(strstr(dir_buf,"http") == NULL) {
                //   *slash = '\0';
                //     if(chdir(dir_buf) < 0) {
                //          fprintf(stderr,"Couldn't cd to %s\n",dir_buf);
                //          err_flag++;
                //     }
                //     gd.db_name = strdup(slash + 1);
                // } else {
                // Check for -print_params
                if(strstr(dir_buf,"rint_params") == NULL) {
                  // gd.db_name = strdup(optarg);
                } else {  // Output the master param file
                  load_db_data("");
                  // fputs(gd.db_data,stdout);
                  exit(0);
                }
                // }


            break;
 
            case 'q' :    
		gd.quiet_mode = 1;
	    break;

            case 'v' :    
		debug_level = atoi(optarg);
		gd.debug |= (debug_level & 0x01);
		gd.debug1 |= (debug_level & 0x02);
		gd.debug2 |= (debug_level & 0x04);
            break;
 
            case 'x' :    
		_params.http_proxy_url = optarg;
                if(!gd.quiet_mode) printf("Loading Parameters via Proxy: %s\n",optarg);
            break;
 
            case 'h' :    // Height List (Generally)
		// Escape hatch for -help argument 
		if(strstr(optarg,"elp") != NULL) { 
		   err_flag = 1;
		   goto USAGE;   // OOohhhhh...  It feels good once in a while  :)

		} else { //   Set the starting height
		    gd.num_render_heights = 0;
		    gd.cur_render_height = 0;
		    token = strtok(optarg,","); // Prime strtok
		    gd.h_win.cur_ht = atof(token);
		    while(token != NULL && gd.num_render_heights < MAX_SECTS) {
		      gd.height_array[gd.num_render_heights] = atof(token);
		      gd.num_render_heights++;
		      token = strtok(NULL,","); // get next rtoken strtok
		    }
                    if(!gd.quiet_mode) printf("CIDD Found %d heights starting at: %g\n",
					      gd.num_render_heights,gd.h_win.cur_ht);
		}    
            break;
 
            case 'i' :    
		gd.app_instance = optarg;
                if(!gd.quiet_mode) printf("CIDD Instance: %s\n",optarg);
            break;
 
            case 'u' :    
		gd.run_unmapped = 1;
                if(!gd.quiet_mode) printf("CIDD will run unmapped\n");
            break;
 
            case 'V' :    
		_params.use_cosine_correction = pFALSE;
                if(!gd.quiet_mode) printf("CIDD will run in VERT pointing mode\n");
            break;
 
            case 't' :    
	      //
	      // Are we looking for now minus a certain number of seconds?
	      //
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
		break;
	      }

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
            break;
 
            case '?':   /* unknown options - no longer complain about XView args. */
            default:
            break;
        }
 
    };

    USAGE:
    if(err_flag) {
        fprintf(stderr,"\nUsage:CIDD [-p Parameter_file] [-v report_level] [-i Instance string] \\");
	fprintf(stderr,"[-t YYYYMMDDHHMM(SS - seconds optional)] [-h height,height,height,...  ] [-x http://proxy_host:3128] \\");
	fprintf(stderr," [-print_params] \n\n");
        fprintf(stderr,"\n       -p: CIDD will look for config files in dir where Parameter_file  is located\n");
        fprintf(stderr,"\n");
        fprintf(stderr,"       -v: Level 0-7 Verbosity (Bitwise flags)\n");
        fprintf(stderr,"       -h: CIDD starts up at the first height, overiding the setting in the param file\n");
        fprintf(stderr,"                and in HTML Mode will automatically render each height in the list\n");
        fprintf(stderr,"       -i: CIDD registers with the process mapper using this instance\n");
        fprintf(stderr,"       -q: Quiet mode: CIDD outputs no messages - Note: put this before the t,h,i args\n");
        fprintf(stderr,"       -t: CIDD Starts up in archive mode at this time\n");
	fprintf(stderr,"           (Note : -t minus3600 starts CIDD in archive mode with the first frame\n");
	fprintf(stderr,"            one hour ago, ie. use -t minusX to start X seconds ago in archive mode)\n");
        fprintf(stderr,"       -u: CIDD runs unmapped (hidden)\n");
        fprintf(stderr,"       -x: Use this proxy server for data requests\n");
        fprintf(stderr,"       -V: Run in vertical pointing mode - no cosine correction)\n");
        fprintf(stderr,"       -print_params: Output a default/example param file\n");
        fprintf(stderr,"\n");
    }
}  
