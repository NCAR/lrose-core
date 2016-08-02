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
/************************************************************************
 * CIDD_INIT.C: Routines that read initialization files, set up
 *         communications between processes, etc.,
 *
 *
 * For the Cartesian Radar Display (CIDD)
 * Frank Hage   July 1991 NCAR, Research Applications Program
 */

#include <X11/Xlib.h>

#define RD_INIT    1
#include "cidd.h"

#include <toolsa/mem.h>
#include <toolsa/str.h>
#include <toolsa/utim.h>
#include <rapplot/xrs.h>

#include <xview/cursor.h>


#define NUM_PARSE_FIELDS     32
#define PARSE_FIELD_SIZE     32
#define INPUT_LINE_LEN      512

/************************************************************************
 * GET_PARAMETER_DATABASE: Load the parameter data base and set initial
 *     parameters
 *
 */

int    get_parameter_database(fname)
char    *fname;
{
    /* Retrieve the application's Default/Resource data base */
    gd.cidd_db = XrmGetFileDatabase(fname);
    if(gd.cidd_db == NULL) {
        fprintf(stderr,"Could'nt open or find %s\n",fname);
        exit(-1);
    }
    return 0;
}

/*****************************************************************
 * INIT_DATA_SPACE : Init all globals and set up defaults
 */

void init_data_space()
{
    int i,j,pid;
    int num_fields;
    int found;
    int index;
    long    clock;
    double delta_x,delta_y;
    char *demo_time;
    char *resource;
    char *field_str;
    char    string[128];    /* Space to build resource strings */
    char    *cfield[3];     /* Space to collect sub strings */

    UTIMstruct    temp_utime;

    INSTANCE = xv_unique_key(); /* get keys for retrieving data */
    MENU_KEY = xv_unique_key();

    get_parameter_database(gd.db_name); /* load the program's parameter database*/
    /* Initialize tha Process Mapper Functions */
     PMU_auto_init(gd.app_name,gd.app_instance,PROCMAP_REGISTER_INTERVAL);

	 PMU_auto_register("Initializing data");


    gd.debug |= XRSgetLong(gd.cidd_db, "cidd.debug_flag", 0);
    gd.debug1 |= XRSgetLong(gd.cidd_db, "cidd.debug1_flag", 0);
    gd.debug2 |= XRSgetLong(gd.cidd_db, "cidd.debug2_flag", 0);

    /* fill in server mapper hosts */
    gd.use_servmap = XRSgetLong(gd.cidd_db, "cidd.use_servmap",0);

    gd.servmap_host1 = XRSgetString(gd.cidd_db, "cidd.servmap_host1", 
				    getenv("SERVMAP_HOST"));
    gd.servmap_host2 = XRSgetString(gd.cidd_db, "cidd.servmap_host2",
				    getenv("SERVMAP_HOST2"));

    /* Establish the native projection type */
    resource = XRSgetString(gd.cidd_db, "cidd.projection_type", "CARTESIAN");
    if(strncmp(resource,"CARTESIAN",9) == 0) {
	found = 1;
	gd.projection_mode = CARTESIAN_PROJ;
    }

    if(strncmp(resource,"LAT_LON",7) == 0) {
	found = 1;
	gd.projection_mode = LAT_LON_PROJ;
    }
    if(!found) {
	fprintf(stderr,"Unknown projection type for resource: cidd.projection_type! \n");
	fprintf(stderr," Current valid types are: CARTESIAN, LAT_LON\n");
	exit(-1);
    }

    /* Establish whether CIDD puts out HTML imagery - Yes if set */
    resource = XRSgetString(gd.cidd_db, "cidd.html_image_dir", "");
    if(resource != NULL && strlen(resource) > 1) {
	 gd.html_image_dir = resource;
	 gd.html_mode = 1;
	 gd.h_win.zoom_level = 0;
    } else {
	gd.html_image_dir = NULL;
	gd.html_mode = 0;
    }

    resource = XRSgetString(gd.cidd_db, "cidd.html_convert_script", "");
    if(strlen(resource) > 1) {
	 gd.html_convert_script = resource;
    }

    gd.label_time_format = XRSgetString(gd.cidd_db,
	  "cidd.label_time_format", "%m/%d/%y %H:%M:%S");


    gd.expt_feature_scale = XRSgetLong(gd.cidd_db, "cidd.expt_feature_scale", 12500);
    gd.expt_font_scale = XRSgetLong(gd.cidd_db, "cidd.expt_font_scale", 45);
    gd.expt_mark_size = XRSgetLong(gd.cidd_db, "cidd.expt_mark_size", 5);

    gd.drawing_mode = FALSE;

    gd.prod.products_on = 1;
    gd.prod.product_time_select = XRSgetLong(gd.cidd_db, "cidd.product_time_selection", 0);
    gd.prod.product_time_width = XRSgetLong(gd.cidd_db, "cidd.product_time_width", 120);
    gd.prod.prod_line_width = XRSgetLong(gd.cidd_db, "cidd.product_line_width", 1);
    gd.prod.prod_font_num = XRSgetLong(gd.cidd_db, "cidd.product_font_size", 1);;

    gd.always_get_full_domain = XRSgetLong(gd.cidd_db, "cidd.always_get_full_domain", 0);
     
    gd.prds_command = XRSgetString(gd.cidd_db, "cidd.prds_command", "");
    gd.exprt_command = XRSgetString(gd.cidd_db, "cidd.exprt_command", "");

    STRcopy(gd.h_win.image_dir, XRSgetString(gd.cidd_db, "cidd.horiz_image_dir", "/tmp"),MAX_PATH_LEN);
    STRcopy(gd.h_win.image_fname, XRSgetString(gd.cidd_db, "cidd.horiz_image_fname", "cidd1.xwd"),MAX_PATH_LEN);
    STRcopy(gd.h_win.image_command, XRSgetString(gd.cidd_db, "cidd.horiz_image_command", ""),MAX_PATH_LEN);

    STRcopy(gd.v_win.image_dir, XRSgetString(gd.cidd_db, "cidd.vert_image_dir", "/tmp"),MAX_PATH_LEN);
    STRcopy(gd.v_win.image_fname, XRSgetString(gd.cidd_db, "cidd.vert_image_fname", "cidd2.xwd"),MAX_PATH_LEN);
    STRcopy(gd.v_win.image_command, XRSgetString(gd.cidd_db, "cidd.vert_image_command", ""),MAX_PATH_LEN);

    gd.north_angle = XRSgetDouble(gd.cidd_db, "cidd.north_angle",0.0);

    gd.aspect_ratio = XRSgetDouble(gd.cidd_db, "cidd.aspect_ratio",1.0);

    gd.h_win.last_field = -1;

/*    umalloc_debug(2); /* DEBUG */

    /* fill in movie frame names & other info */
    gd.movie_frame_dir = XRSgetString(gd.cidd_db, "cidd.movie_frame_dir", "/tmp");

    pid = getpid();
    for(i=0; i < MAX_FRAMES; i++) {
        sprintf(gd.movie.frame[i].fname,
                "%s/cidd_im%d_%d.",
                gd.movie_frame_dir, pid, i);
        gd.movie.frame[i].h_xid = 0;
        gd.movie.frame[i].v_xid = 0;
        gd.movie.frame[i].redraw_horiz = 1;
        gd.movie.frame[i].redraw_vert = 1;
    }
    gd.movie.time_interval = XRSgetDouble(gd.cidd_db, "cidd.time_interval",10.0);
    gd.movie.forecast_interval = XRSgetDouble(gd.cidd_db, "cidd.forecast_interval", 0.0);
    gd.movie.reset_frames = XRSgetLong(gd.cidd_db, "cidd.reset_frames", 0);
    gd.movie.mr_stretch_factor = XRSgetDouble(gd.cidd_db, "cidd.stretch_factor", 1.5);
    gd.movie.num_pixmaps = XRSgetLong(gd.cidd_db, "cidd.num_pixmaps", 5);
    gd.movie.delay = XRSgetLong(gd.cidd_db, "cidd.movie_delay",3000);

    gd.movie.start_frame = 0;
    gd.movie.end_frame = gd.movie.num_pixmaps -1 ;
    gd.movie.num_frames = gd.movie.num_pixmaps;
    gd.movie.cur_frame = gd.movie.num_frames -1;
    gd.movie.last_frame = gd.movie.cur_frame;
    gd.movie.first_index = 0;

    /* Toggle for displaying the analog clock */
    gd.show_clock = XRSgetLong(gd.cidd_db,"cidd.show_clock", 1);

    /* Set the time to display on the hwin clock */
    gd.draw_clock_local = XRSgetLong(gd.cidd_db,"cidd.draw_clock_local", 1);
    

    clock = time(0);    /* get current time */
    clock += (gd.movie.forecast_interval * 60);
    gd.movie.round_to_seconds = XRSgetLong(gd.cidd_db,"cidd.temporal_rounding", 300);

    /* IF this string is present in the .cidd. defaults - Run a Demo on this data time at the indicated time. */
    demo_time = XRSgetString(gd.cidd_db,"cidd.demo_time","");

    if((int)strlen(demo_time) < 12 && (gd.movie.mode != MOVIE_TS) ) { /* LIVE MODE */
        gd.movie.mode = MOVIE_MR;

        /* set the first index's time based on current time  */
        gd.movie.start_time.unix_time = clock - ((gd.movie.num_frames -1) * gd.movie.time_interval * 60.0);
        gd.movie.start_time.unix_time -= (gd.movie.start_time.unix_time % gd.movie.round_to_seconds);

        UTIMunix_to_date(gd.movie.start_time.unix_time,&temp_utime);

    } else {   /* DEMO MODE */

      if(gd.movie.mode != MOVIE_TS) { /* if not set by command line args */
        gd.movie.mode = MOVIE_TS;     /* time_series */
        parse_string_into_time(demo_time,&temp_utime);
        UTIMdate_to_unix(&temp_utime);
        /* set the first index's time  based on indicated time */
        gd.movie.start_time.unix_time = temp_utime.unix_time;
      }
	 
      /* Adjust the start time downward to the nearest round interval seconds */
      gd.movie.start_time.unix_time -= (gd.movie.start_time.unix_time % gd.movie.round_to_seconds);
      UTIMunix_to_date(gd.movie.start_time.unix_time,&temp_utime);

      gd.h_win.movie_field = gd.h_win.field;
      gd.v_win.movie_field = gd.v_win.field;
      gd.movie.cur_frame = 0;
    }

    gd.movie.movie_on = XRSgetLong(gd.cidd_db,"cidd.movie_on",0);

    gd.movie.display_time_msec = 75;
    gd.movie.start_time.year = temp_utime.year;
    gd.movie.start_time.month = temp_utime.month;
    gd.movie.start_time.day = temp_utime.day;
    gd.movie.start_time.hour = temp_utime.hour;
    gd.movie.start_time.min = temp_utime.min;
    gd.movie.start_time.sec = temp_utime.sec;

    index = gd.movie.first_index;
     /* Set the time window for each frame */
    for(i=0 ; i < gd.movie.num_frames; i++) {
        gd.movie.frame[index].time_mid.unix_time =
	  gd.movie.start_time.unix_time +
	    (index * gd.movie.time_interval * 60.0);
        UTIMunix_to_date(gd.movie.frame[index].time_mid.unix_time,
			 &gd.movie.frame[index].time_mid);

        gd.movie.frame[index].time_start.unix_time =
	  gd.movie.frame[index].time_mid.unix_time -
	    (gd.movie.time_interval * 60.0 / 2.0);
        UTIMunix_to_date(gd.movie.frame[index].time_start.unix_time,
			 &gd.movie.frame[index].time_start);

        gd.movie.frame[index].time_end.unix_time =
	  gd.movie.frame[index].time_mid.unix_time +
                (gd.movie.time_interval * 60.0 / 2.0);

        UTIMunix_to_date(gd.movie.frame[index].time_end.unix_time, &gd.movie.frame[index].time_end);

        index ++;
    }

    gd.image_fill_treshold = XRSgetLong(gd.cidd_db, "cidd.image_fill_treshold",    20000);
    gd.image_inten = XRSgetDouble(gd.cidd_db, "cidd.image_inten", 0.8);
    gd.inten_levels = XRSgetLong(gd.cidd_db, "cidd.inten_levels", 32);
    gd.data_timeout_secs = XRSgetLong(gd.cidd_db, "cidd.data_timeout_secs", 10);
    gd.h_win.origin_lat = XRSgetDouble(gd.cidd_db, "cidd.origin_latitude", 39.8783);
    gd.h_win.origin_lon = XRSgetDouble(gd.cidd_db, "cidd.origin_longitude", -104.7568);

    gd.aspect_correction = cos(gd.h_win.origin_lat * DEG_TO_RAD);

    gd.latlon_mode = XRSgetLong(gd.cidd_db, "cidd.latlon_mode",0);
     
    gd.font_display_mode = XRSgetLong(gd.cidd_db, "cidd.font_display_mode",1);

    gd.gather_data_mode = XRSgetLong(gd.cidd_db, "cidd.gather_data_mode",0);

    gd.num_field_menu_cols = XRSgetLong(gd.cidd_db, "cidd.num_field_menu_cols",0);

    gd.h_win.num_zoom_levels =  XRSgetLong(gd.cidd_db, "cidd.num_zoom_levels",1);
    if(gd.html_mode ==0) {
        gd.h_win.zoom_level =  XRSgetLong(gd.cidd_db, "cidd.start_zoom_level",0);
    }

    gd.h_win.zmin_x = (double *)  ucalloc(sizeof(double),gd.h_win.num_zoom_levels+NUM_CUSTOM_ZOOMS);
    gd.h_win.zmax_x = (double *)  ucalloc(sizeof(double),gd.h_win.num_zoom_levels+NUM_CUSTOM_ZOOMS);
    gd.h_win.zmin_y = (double *)  ucalloc(sizeof(double),gd.h_win.num_zoom_levels+NUM_CUSTOM_ZOOMS);
    gd.h_win.zmax_y = (double *)  ucalloc(sizeof(double),gd.h_win.num_zoom_levels+NUM_CUSTOM_ZOOMS);
    for(i=0; i < gd.h_win.num_zoom_levels; i++) {
        sprintf(string,"cidd.level%d_min_xkm",i);
        gd.h_win.zmin_x[i] = XRSgetDouble(gd.cidd_db,string,-200.0/(i+1));

        sprintf(string,"cidd.level%d_min_ykm",i);
        gd.h_win.zmin_y[i] = XRSgetDouble(gd.cidd_db,string,-200.0/(i+1));

        sprintf(string,"cidd.level%d_max_xkm",i);
        gd.h_win.zmax_x[i] = XRSgetDouble(gd.cidd_db,string,200.0/(i+1));

        sprintf(string,"cidd.level%d_max_ykm",i);
        gd.h_win.zmax_y[i] = XRSgetDouble(gd.cidd_db,string,200.0/(i+1));

	delta_x = gd.h_win.zmax_x[i] - gd.h_win.zmin_x[i];
	delta_y = gd.h_win.zmax_y[i] - gd.h_win.zmin_y[i];

	/* Make sure domains are consistant with the window aspect ratio */
	switch(gd.projection_mode) {
	    case LAT_LON_PROJ:
		/* forshorten the Y coords to make things look better */
		delta_y /= gd.aspect_correction;
	    break;
	}

	delta_x /= gd.aspect_ratio;

	if(delta_x > delta_y) {
	    gd.h_win.zmax_y[i] += ((delta_x - delta_y) /2.0) ;
	    gd.h_win.zmin_y[i] -= ((delta_x - delta_y) /2.0) ;
	} else {
	    gd.h_win.zmax_x[i] += ((delta_y - delta_x) /2.0) ;
	    gd.h_win.zmin_x[i] -= ((delta_y - delta_x) /2.0) ;
	}
    }

    gd.h_win.min_x = XRSgetDouble(gd.cidd_db, "cidd.domain_limit_min_x",gd.h_win.zmin_x[0]);
    gd.h_win.max_x = XRSgetDouble(gd.cidd_db, "cidd.domain_limit_max_x",gd.h_win.zmax_x[0]);
    gd.h_win.min_y = XRSgetDouble(gd.cidd_db, "cidd.domain_limit_min_y",gd.h_win.zmin_y[0]);
    gd.h_win.max_y = XRSgetDouble(gd.cidd_db, "cidd.domain_limit_max_y",gd.h_win.zmax_y[0]);

    gd.h_win.cmin_x = gd.h_win.zmin_x[gd.h_win.zoom_level];
    gd.h_win.cmax_x = gd.h_win.zmax_x[gd.h_win.zoom_level];
    gd.h_win.cmin_y = gd.h_win.zmin_y[gd.h_win.zoom_level];
    gd.h_win.cmax_y = gd.h_win.zmax_y[gd.h_win.zoom_level];


    /* Automatically add Custom Zoom levels */
    for(i=0; i < NUM_CUSTOM_ZOOMS; i++) {
        gd.h_win.zmin_x[gd.h_win.num_zoom_levels] = gd.h_win.zmin_x[0] / (2.0 * (i +1));
        gd.h_win.zmax_x[gd.h_win.num_zoom_levels] = gd.h_win.zmax_x[0] / (2.0 * (i +1));
        gd.h_win.zmin_y[gd.h_win.num_zoom_levels] = gd.h_win.zmin_y[0] / (2.0 * (i +1));
        gd.h_win.zmax_y[gd.h_win.num_zoom_levels] = gd.h_win.zmax_y[0] / (2.0 * (i +1));
        gd.h_win.num_zoom_levels++;
    }

    /* Compute radial limits of full domain */
    zoom_radial(gd.h_win.min_x, gd.h_win.min_y, gd.h_win.max_x, gd.h_win.max_y);
    gd.h_win.min_r = gd.h_win.zmin_r;
    gd.h_win.max_r = gd.h_win.zmax_r;
    gd.h_win.min_deg = gd.h_win.zmin_deg;
    gd.h_win.max_deg = gd.h_win.zmax_deg;

    gd.h_win.min_ht = XRSgetDouble(gd.cidd_db, "cidd.min_ht", 0.0);
    gd.h_win.max_ht = XRSgetDouble(gd.cidd_db, "cidd.max_ht", 30.0);
    gd.h_win.delta = XRSgetDouble(gd.cidd_db, "cidd.ht_interval", 0.5);

    gd.h_win.num_slices = (gd.h_win.max_ht - gd.h_win.min_ht) / gd.h_win.delta;

    gd.h_win.cmin_ht = XRSgetDouble(gd.cidd_db, "cidd.start_ht", 0.5) - (gd.h_win.delta/2.0);
    gd.h_win.cmax_ht = gd.h_win.cmin_ht + gd.h_win.delta;

    /* Compute radial limits of starting/current domain. */
    zoom_radial(gd.h_win.cmin_x, gd.h_win.cmin_y, gd.h_win.cmax_x, gd.h_win.cmax_y);
    gd.h_win.cmin_r = gd.h_win.zmin_r;
    gd.h_win.cmax_r = gd.h_win.zmax_r;
    gd.h_win.cmin_deg = gd.h_win.zmin_deg;
    gd.h_win.cmax_deg = gd.h_win.zmax_deg;


    gd.v_win.origin_lat = gd.h_win.origin_lat;
    gd.v_win.origin_lon = gd.h_win.origin_lon;
    gd.v_win.min_x = gd.h_win.min_x;
    gd.v_win.max_x = gd.h_win.max_x;
    gd.v_win.min_y = gd.h_win.min_y;
    gd.v_win.max_y = gd.h_win.max_y;
    gd.v_win.min_ht = gd.h_win.min_ht;
    gd.v_win.max_ht = gd.h_win.max_ht;
    gd.v_win.delta = gd.h_win.delta;

    /* Set Vertical display to show North pointing RHI in display area */
    gd.v_win.cmin_x = 0.0;
    gd.v_win.cmin_y = 0.0;
    gd.v_win.cmax_x = 0.0;
    gd.v_win.cmax_y = gd.h_win.max_y;
    gd.v_win.cmin_ht = gd.h_win.min_ht;
    gd.v_win.cmax_ht = gd.h_win.max_ht;

    /* Initialize Extra features data */
    gd.extras.wind_vectors = 1;
    gd.extras.wind_mode = XRSgetLong(gd.cidd_db, "cidd.wind_mode", 0);
    gd.extras.wind_time_scale_interval = XRSgetDouble(gd.cidd_db, "cidd.wind_time_scale_interval", 10.0);
    gd.extras.range = XRSgetLong(gd.cidd_db, "cidd.range_rings", 0) ? RANGE_BIT : 0;
    gd.extras.azmiths = XRSgetLong(gd.cidd_db, "cidd.azmith_lines", 0) ? AZMITH_BIT : 0;
    init_sockets();            /* Initialize Socket based communications */

    PMU_auto_register("Initializing SHMEM");
    init_shared();            /* Initialize Shared memory based communications */
    PMU_auto_register("READING Config files");
    init_data_links();      /* establish and initialize sources of data */

    init_wind_data_links();   /* Establish and initialize connections to products */
    
    if(gd.extras.num_wind_sets == 0) gd.extras.wind_vectors = 0;

    init_over_data_links();   /* establish and initialize  overlay data */

    for(i=0; i < NUM_CONT_LAYERS; i++) { gd.extras.cont[i].field = 0; }
    for(i=0; i < NUM_GRID_LAYERS; i++) { gd.extras.overlay_field[i] = 0; }

    for(j=0;j < gd.num_datafields; j++) {
       gd.mrec[j]->use_servmap = gd.use_servmap;
       gd.h_win.redraw[j] = 1;
       gd.v_win.redraw[j] = 1;
    }

    /* Get space for string parsing sub-fields */
    cfield[0] = ucalloc(1,NAME_LENGTH);
    cfield[1] = ucalloc(1,NAME_LENGTH);
    cfield[2] = ucalloc(1,NAME_LENGTH);

    if(cfield[0]  == NULL || cfield[1] == NULL || cfield[2] == NULL) {
	fprintf(stderr,"Cidd: Fatal Alloc failure of %d bytes\n", NAME_LENGTH);
    }

    /* Setup default CONTOUR FIELDS */
    for(i = 1; i <= NUM_CONT_LAYERS; i++ ) {

      sprintf(string,"cidd.contour%d_field",i);
      field_str = XRSgetString(gd.cidd_db,string, "NoMaTcH");

      num_fields = STRparse(field_str, cfield, NAME_LENGTH, 3, NAME_LENGTH); 

      strncpy(gd.extras.cont[i-1].color_name,"white",NAME_LENGTH);

	 if(gd.html_mode == 0) {
        /* Replace Underscores with spaces in field names */
        for(j=strlen(cfield[0])-1 ; j >= 0; j--) 
          if(cfield[0][j] == '_') cfield[0][j] = ' ';
	  }
      for(j=0;j < gd.num_datafields; j++) {
        if(strcmp(gd.mrec[j]->field_name,cfield[0]) == 0) {
          gd.extras.cont[i-1].field = j;
          if(num_fields >  2 && (strncmp(cfield[2],"off",3) == 0) ) {
	      gd.extras.cont[i-1].active = 0;
	  } else {
	      gd.extras.cont[i-1].active = 1;
	  }

          gd.extras.cont[i-1].min = gd.mrec[j]->cont_low;
          gd.extras.cont[i-1].max = gd.mrec[j]->cont_high;
          gd.extras.cont[i-1].interval = gd.mrec[j]->cont_interv;

	  if(num_fields > 1)
	      strncpy(gd.extras.cont[i-1].color_name,cfield[1],NAME_LENGTH);

        }
      }
    }
    /* Set up default OVERLAY FIELDS */
    for(i = 1; i <= NUM_GRID_LAYERS; i++ ) {
	 sprintf(string,"cidd.layer%d_field",i);
	 field_str =  XRSgetString(gd.cidd_db, string, "NoMaTcH");

         num_fields = STRparse(field_str, cfield, NAME_LENGTH, 3, NAME_LENGTH); 

	 if(gd.html_mode == 0) {
        /* Replace Undercores with spaces in field names */
        for(j=strlen(cfield[0])-1 ; j >= 0; j--) 
          if(field_str[j] == '_') cfield[0][j] = ' ';
	}
	 for(j=0; j <  gd.num_datafields; j++) {
           if(strcmp(gd.mrec[j]->field_name,cfield[0]) == 0) {  
	       if(num_fields >  1 && (strncmp(cfield[1],"off",3) == 0) ) {
	           gd.extras.overlay_field_on[i-1] = 0;
		} else {
	           gd.extras.overlay_field_on[i-1] = 1;
		}

	       gd.extras.overlay_field[i-1] = j;
	   }
	}
    }

    ufree(cfield[0]);
    ufree(cfield[1]);
    ufree(cfield[2]);

}

/************************************************************************
 * INIT_SHARED:  Initialize the shared memory communications
 *
 */

int    init_shared()
{
    int coord_key;

    coord_key = XRSgetLong(gd.cidd_db, "cidd.coord_key", 63500);

    if((gd.coord_expt = (coord_export_t *) ushm_create(coord_key, sizeof(coord_export_t), 0666)) == NULL) {
        fprintf(stderr, "Couldn't create shared memory segment for Aux process communications\n");
        exit(-1);
    }


    /* Initialize shared memory area for coordinate/selection communications */
    gd.coord_expt->selection_sec = 0;
    gd.coord_expt->selection_usec = 0;

    gd.coord_expt->epoch_start = gd.movie.start_time.unix_time;
    gd.coord_expt->epoch_end = gd.movie.start_time.unix_time +
               ((gd.movie.num_frames +1) * gd.movie.time_interval * 60.0); 

    gd.coord_expt->time_min = gd.movie.frame[gd.movie.num_frames -1].time_start.unix_time;
    gd.coord_expt->time_max = gd.movie.frame[gd.movie.num_frames -1].time_end.unix_time;
    if(gd.movie.movie_on) {
        gd.coord_expt->time_cent = gd.coord_expt->epoch_end; 
    } else {
            gd.coord_expt->time_cent = gd.coord_expt->time_min +
              ((gd.coord_expt->time_max - gd.coord_expt->time_min)/2);
    } 
    gd.coord_expt->pointer_seq_num = 0;

    gd.coord_expt->pointer_x = 0.0;
    gd.coord_expt->pointer_y = 0.0;
    gd.coord_expt->pointer_lon = 0.0;
    gd.coord_expt->pointer_lat = 0.0;

    gd.coord_expt->focus_x = 0.0;
    gd.coord_expt->focus_y = 0.0;
    gd.coord_expt->focus_lat = 0.0;
    gd.coord_expt->focus_lon = 0.0;

    gd.coord_expt->datum_latitude = XRSgetDouble(gd.cidd_db, "cidd.origin_latitude", 39.8783);
    gd.coord_expt->datum_longitude = XRSgetDouble(gd.cidd_db, "cidd.origin_longitude", -104.7568);
    gd.coord_expt->shmem_ready = 1;

    CSPR_init();

    if(init_import()) {            /* initialize the prds  product importer */
        fprintf(stderr,
	 "Error: Could not setup shared memory segment for prds\n");
        exit(-1);
    }


    init_expti();         /* Initialize shared memory/constants for drawing */

    return 0;
}

/*****************************************************************
 * SIGNAL_TRAP : Traps Signals so as to die gracefully
 */
void
signal_trap(signal,code)
    int    signal,code;
{
    fprintf(stderr,"CIDD: Received signal %d\n",signal);
}

/*****************************************************************
 * SIGIO_TRAP : Traps IO Signal
 */
void
sigio_trap(signal,code)
    int    signal,code;
{
    extern void check_for_io();
    if(gd.io_info.outstanding_request) check_for_io();
}


/************************************************************************
 * INIT_SOCKETS:  Initialize the Socket Communications
 *
 */

int    init_sockets()
{
    int pid;
    void    signal_trap();

    signal(SIGINT,signal_trap);
    signal(SIGTERM,signal_trap);
    signal(SIGIO,sigio_trap);
    return 0;
}
/************************************************************************
 * INIT_DATA_LINKS:  Scan cidd_grid_data.info file and establish links to data
 *         sources.
 *
 */

void
init_data_links()
{
    int i,j;
    int    num_fields;
    char    *info_file_name;
    FILE    *info_file;
    char    buf[INPUT_LINE_LEN];
    char    *cfield[NUM_PARSE_FIELDS];

    info_file_name = XRSgetString(gd.cidd_db, "cidd.data_info_file", "data.cidd_grid_info");
    if((info_file = fopen(info_file_name,"r")) == NULL) {
        fprintf(stderr,"cidd: Couldn't open %s\n",info_file_name);
        exit(-1);
    }

    /* read all the lines in the data information file */
    gd.num_datafields = 0;
    while (fgets(buf, INPUT_LINE_LEN, info_file) != NULL) {
        if(buf[0] != '#' && (int)strlen(buf) > 20) {  /* don't use commented lines or short/bad lines */
	    if(gd.num_datafields < MAX_DATA_FIELDS -1) {
              gd.data_info[gd.num_datafields] = ucalloc(INPUT_LINE_LEN, 1);
              STRcopy(gd.data_info[gd.num_datafields],buf,INPUT_LINE_LEN);

	      /* DO Environment variable substitution */
	      usubstitute_env(gd.data_info[gd.num_datafields], INPUT_LINE_LEN);
              gd.num_datafields++;
	    } else {
		fprintf(stderr,
		  "Cidd: Warning. Too many Data Fields. Data field not processed\n");
		fprintf(stderr,"%s\n",buf);
	    }
        }
    }
    fclose(info_file); 
    if(gd.num_datafields <=0) {
	 fprintf(stderr,"CIDD requirtes at least one valid data field to be defined. - Sorry\n");
	 exit(-1);
    }

    /* get temp space for substrings */
    for(i = 0; i < NUM_PARSE_FIELDS; i++) {
        cfield[i] = ucalloc(PARSE_FIELD_SIZE, 1);
    }

    /* scan through each of the data information lines */
    for(i = 0; i < gd.num_datafields; i++) {
        /* get space for data info */
        gd.mrec[i] =  (met_record_t *) ucalloc(sizeof(met_record_t), 1);

        /* separate into substrings */
        num_fields = STRparse(gd.data_info[i], cfield, INPUT_LINE_LEN, NUM_PARSE_FIELDS, PARSE_FIELD_SIZE);
        STRcopy(gd.mrec[i]->source_name,cfield[0],NAME_LENGTH);
        STRcopy(gd.mrec[i]->field_name,cfield[1],NAME_LENGTH);
		if(gd.html_mode == 0) {
          /* Replace Underscores with spaces in names */
          for(j=strlen(gd.mrec[i]->field_name)-1 ; j >= 0; j--) {
            if(gd.mrec[i]->field_name[j] == '_') gd.mrec[i]->field_name[j] = ' ';
          }
		}
        STRcopy(gd.mrec[i]->data_hostname,cfield[2],NAME_LENGTH);

        gd.mrec[i]->port = atoi(cfield[3]);
        gd.mrec[i]->sub_field = atoi(cfield[4]);

        STRcopy(gd.mrec[i]->color_file,cfield[5],NAME_LENGTH);
        STRcopy(gd.mrec[i]->field_units,cfield[6],LABEL_LENGTH);

        gd.mrec[i]->cont_low = atof(cfield[7]);
        gd.mrec[i]->cont_high = atof(cfield[8]);
        gd.mrec[i]->cont_interv = atof(cfield[9]);

        gd.mrec[i]->time_allowance = gd.movie.mr_stretch_factor * gd.movie.time_interval;

        if (strncasecmp(cfield[10],"rad",3) == 0) {
            gd.mrec[i]->data_format = PPI_DATA_FORMAT;
            gd.mrec[i]->render_method = RECTANGLES;
        } else {
            gd.mrec[i]->data_format = CART_DATA_FORMAT;
            gd.mrec[i]->render_method = RECTANGLES;
        }

        if (strncasecmp(cfield[10],"cont",4) == 0) {
            gd.mrec[i]->render_method = FILLED_CONTOURS;
        }

        if (strstr(cfield[10],"comp") != NULL) {
            gd.mrec[i]->composite_mode = TRUE;
        }

        STRcopy(gd.mrec[i]->data_server_type, cfield[11], NAME_LENGTH);
        STRcopy(gd.mrec[i]->data_server_subtype, cfield[12], NAME_LENGTH);
        STRcopy(gd.mrec[i]->data_server_instance, cfield[13], NAME_LENGTH);
        gd.mrec[i]->currently_displayed = atoi(cfield[14]);
        gd.mrec[i]->background_render = atoi(cfield[15]);

        gd.mrec[i]->last_elev = (char *)NULL;
        gd.mrec[i]->elev_size = 0;

        gd.mrec[i]->plane = 0;
        gd.mrec[i]->h_data_valid = 0;
        gd.mrec[i]->v_data_valid = 0;
        gd.mrec[i]->h_scale  = 0.0;
        gd.mrec[i]->h_bias  = 0.0;
        gd.mrec[i]->h_last_scale  = -1.0;
        gd.mrec[i]->h_last_bias  = -1.0;
        gd.mrec[i]->v_scale  = 0.0;
        gd.mrec[i]->v_bias  = 0.0;
        gd.mrec[i]->v_last_scale  = -1.0;
        gd.mrec[i]->v_last_bias  = -1.0;
        gd.mrec[i]->origin_lat  = 0.0;
        gd.mrec[i]->origin_lon  = 0.0;

        if (gd.mrec[i]->data_format == PPI_DATA_FORMAT)
        {
            STRcopy(gd.mrec[i]->units_label_cols,"DEG",LABEL_LENGTH);
            STRcopy(gd.mrec[i]->units_label_rows,"KM",LABEL_LENGTH);
            STRcopy(gd.mrec[i]->units_label_sects,"DEG",LABEL_LENGTH);
        }
        else
        {
            STRcopy(gd.mrec[i]->units_label_cols,"KM",LABEL_LENGTH);
            STRcopy(gd.mrec[i]->units_label_rows,"KM",LABEL_LENGTH);
            STRcopy(gd.mrec[i]->units_label_sects,"KM",LABEL_LENGTH);
        }

    }
    /* Make sure the first field is always on */
    gd.mrec[0]->background_render = 1;
    gd.mrec[0]->currently_displayed = 1;

    /* free up temp storage for substrings */
    for(i = 0; i < NUM_PARSE_FIELDS; i++) {
        ufree(cfield[i]);
    }
}

/************************************************************************
 * INIT_OVER_DATA_LINKS:  Scan cidd_overlays.info file and setup
 *
 */

void
init_over_data_links()
{
    int     i;
    char    *file_name;

    file_name = XRSgetString(gd.cidd_db, "cidd.overlay_info_file", "cidd_overlays.info");

    gd.num_map_overlays = load_overlay_info(file_name,gd.over,MAX_OVERLAYS);

    if(load_overlay_data(gd.over,gd.num_map_overlays) != SUCCESS) {
        fprintf(stderr,"Problem loading overlay data\n");
        exit(0);
    }

    calc_local_over_coords();

}

/************************************************************************
 * INIT_WIND_DATA_LINKS:  Scan cidd_wind_data.info file and setup link to
 *         data source for the wind fields.
 *
 */

void
init_wind_data_links()
{
    int     i;
    int        num_sets;    /* number of sets of wind data */
    int        num_fields;
    char    *file_name;
    FILE    *info_file;
    char    buf[INPUT_LINE_LEN];
    char    *cfield[NUM_PARSE_FIELDS];

    /* get temp space for substrings */
    for(i = 0; i < NUM_PARSE_FIELDS; i++) {
        cfield[i] = ucalloc(PARSE_FIELD_SIZE, 1);
    }


    file_name = XRSgetString(gd.cidd_db, "cidd.wind_info_file", "cidd_wind_data.info");
    if((info_file = fopen(file_name,"r")) == NULL) {
        fprintf(stderr,"cidd: Couldn't open %s\n",file_name);
        exit(-1);
    }

    /* PASS 1 - Count the wind sets */
    num_sets = 0;
    /* find the first valid line in the wind data information file */
    while (fgets(buf, INPUT_LINE_LEN, info_file) != NULL) {
        if(buf[0] != '#') {
            num_sets++;
        }
    }

    /* ALLOCATE Space */
    if(num_sets > 0) {
      if((gd.extras.wind = ucalloc(num_sets,sizeof(wind_data_t))) == NULL) {
	 fprintf(stderr,"Unable to allocate space for %d wind sets\n",num_sets);
	 perror("Cidd");
	 exit(-1);
      }

      /* PASS 2 - fill in the info the wind sets */
      rewind(info_file);
      num_sets = 0;
      /* find the first valid line in the wind data information file */
      while (fgets(buf, INPUT_LINE_LEN, info_file) != NULL) {
	num_fields = STRparse(buf, cfield,
			 INPUT_LINE_LEN,
			 NUM_PARSE_FIELDS, PARSE_FIELD_SIZE); 
        if( buf[0] != '#' && num_fields >= 11) {

           STRcopy(gd.extras.wind[num_sets].data_info,buf,NAME_LENGTH);

           /* DO Environment variable substitution */
	   usubstitute_env(gd.extras.wind[num_sets].data_info, NAME_LENGTH);
           num_sets++;
        }
      }
    }

    gd.extras.num_wind_sets = num_sets;

    /* fclose(info_file); */

    for(i=0; i < gd.extras.num_wind_sets; i++) {
        num_fields = STRparse(gd.extras.wind[i].data_info, cfield,
                              INPUT_LINE_LEN,
                              NUM_PARSE_FIELDS, PARSE_FIELD_SIZE);
        if(num_fields < 11) {
            fprintf(stderr,
                    "Error in wind field line. Wrong number of parameters,  -Line: \n %s"
                    ,gd.extras.wind[i].data_info);
        }

        /* Allocate Space for U record and initialize */
        gd.extras.wind[i].wind_u = (met_record_t *)
            ucalloc(sizeof(met_record_t), 1);
        gd.extras.wind[i].wind_u->h_data_valid = 0;
        gd.extras.wind[i].wind_u->v_data_valid = 0;
        gd.extras.wind[i].wind_u->h_vcm.nentries = 0;
        gd.extras.wind[i].wind_u->v_vcm.nentries = 0;
        gd.extras.wind[i].wind_u->h_scale = -1.0;
        gd.extras.wind[i].wind_u->h_last_scale = 0.0;
        gd.extras.wind[i].wind_u->server_id = 0;
        gd.extras.wind[i].wind_u->use_servmap = gd.use_servmap;
        STRcopy(gd.extras.wind[i].wind_u->source_name,cfield[0],NAME_LENGTH);
        STRcopy(gd.extras.wind[i].wind_u->field_name,cfield[0],NAME_LENGTH);
        STRcopy(gd.extras.wind[i].wind_u->data_hostname,cfield[1],NAME_LENGTH);
        gd.extras.wind[i].wind_u->port =  atoi(cfield[2]);
        gd.extras.wind[i].wind_u->sub_field = atoi(cfield[3]);
        STRcopy(gd.extras.wind[i].wind_u->field_units,cfield[6],LABEL_LENGTH);
        STRcopy(gd.extras.wind[i].wind_u->data_server_type, cfield[7], NAME_LENGTH);
        gd.extras.wind[i].wind_u->data_server_type[NAME_LENGTH-1] = '\0';
        STRcopy(gd.extras.wind[i].wind_u->data_server_subtype, cfield[8], NAME_LENGTH);
        gd.extras.wind[i].wind_u->data_server_subtype[NAME_LENGTH-1] = '\0';
        STRcopy(gd.extras.wind[i].wind_u->data_server_instance, cfield[9], NAME_LENGTH);
        gd.extras.wind[i].wind_u->data_server_instance[NAME_LENGTH-1] = '\0';
        gd.extras.wind[i].wind_u->currently_displayed = atoi(cfield[10]);
	gd.extras.wind[i].active = atoi(cfield[10]);
        if(num_fields > 11) {
	    STRcopy(gd.extras.wind[i].color_name, cfield[11], NAME_LENGTH);
	} else {
	    STRcopy(gd.extras.wind[i].color_name, "white", NAME_LENGTH);
	}
        gd.extras.wind[i].wind_u->data_format = CART_DATA_FORMAT;
	gd.extras.wind[i].wind_u->time_allowance = gd.movie.mr_stretch_factor * gd.movie.time_interval;
        gd.extras.wind[i].wind_u->origin_lon = 0.0;
        gd.extras.wind[i].wind_u->origin_lat = 0.0;

        /* Allocate Space for V record and initialize */
        gd.extras.wind[i].wind_v = (met_record_t *)
            ucalloc(sizeof(met_record_t), 1);
        gd.extras.wind[i].wind_v->h_data_valid = 0;
        gd.extras.wind[i].wind_v->v_data_valid = 0;
        gd.extras.wind[i].wind_v->h_vcm.nentries = 0;
        gd.extras.wind[i].wind_v->v_vcm.nentries = 0;
        gd.extras.wind[i].wind_v->h_scale = -1.0;
        gd.extras.wind[i].wind_v->h_last_scale = 0.0;
        gd.extras.wind[i].wind_v->server_id = 0;
        gd.extras.wind[i].wind_v->use_servmap = gd.use_servmap;
        STRcopy(gd.extras.wind[i].wind_v->source_name,cfield[0],NAME_LENGTH);
        STRcopy(gd.extras.wind[i].wind_v->field_name,cfield[0],NAME_LENGTH);
        STRcopy(gd.extras.wind[i].wind_v->data_hostname,cfield[1],NAME_LENGTH);
        gd.extras.wind[i].wind_v->port =  atoi(cfield[2]);
        gd.extras.wind[i].wind_v->sub_field = atoi(cfield[4]);
        STRcopy(gd.extras.wind[i].wind_v->field_units,cfield[6],LABEL_LENGTH);
        STRcopy(gd.extras.wind[i].wind_v->data_server_type, cfield[7], NAME_LENGTH);
        STRcopy(gd.extras.wind[i].wind_v->data_server_subtype, cfield[8], NAME_LENGTH);
        gd.extras.wind[i].wind_v->data_server_subtype[NAME_LENGTH-1] = '\0';
        STRcopy(gd.extras.wind[i].wind_v->data_server_instance, cfield[9], NAME_LENGTH);
        gd.extras.wind[i].wind_v->data_server_instance[NAME_LENGTH-1] = '\0';
        gd.extras.wind[i].wind_v->currently_displayed = atoi(cfield[10]);
	gd.extras.wind[i].wind_v->time_allowance = gd.movie.mr_stretch_factor * gd.movie.time_interval;
        gd.extras.wind[i].wind_v->origin_lon = 0.0;
        gd.extras.wind[i].wind_v->origin_lat = 0.0;

        /* Allocate Space for W  record (If necessary)  and initialize */
        if(atoi(cfield[5]) >= 0) {
            gd.extras.wind[i].wind_w = (met_record_t *) ucalloc(sizeof(met_record_t), 1);
            gd.extras.wind[i].wind_w->h_data_valid = 0;
            gd.extras.wind[i].wind_w->v_data_valid = 0;
            gd.extras.wind[i].wind_w->v_vcm.nentries = 0;
            gd.extras.wind[i].wind_w->h_vcm.nentries = 0;
            gd.extras.wind[i].wind_w->h_scale = -1.0;
            gd.extras.wind[i].wind_w->h_last_scale = 0.0;
            gd.extras.wind[i].wind_w->server_id = 0;
            gd.extras.wind[i].wind_w->use_servmap = gd.use_servmap;
            STRcopy(gd.extras.wind[i].wind_w->source_name,cfield[0],NAME_LENGTH);
            sprintf(gd.extras.wind[i].wind_w->field_name,"%s_W ",cfield[0]);
            STRcopy(gd.extras.wind[i].wind_w->data_hostname,cfield[1],NAME_LENGTH);
            gd.extras.wind[i].wind_w->port =  atoi(cfield[2]);
            gd.extras.wind[i].wind_w->sub_field = atoi(cfield[5]);
            STRcopy(gd.extras.wind[i].wind_w->field_units,cfield[6],LABEL_LENGTH);
            STRcopy(gd.extras.wind[i].wind_w->data_server_type, cfield[7], NAME_LENGTH);
            STRcopy(gd.extras.wind[i].wind_w->data_server_subtype, cfield[8], NAME_LENGTH);
            STRcopy(gd.extras.wind[i].wind_w->data_server_instance, cfield[9], NAME_LENGTH);
            gd.extras.wind[i].wind_w->currently_displayed = atoi(cfield[10]);
	    gd.extras.wind[i].wind_w->time_allowance = gd.movie.mr_stretch_factor * gd.movie.time_interval;
            gd.extras.wind[i].wind_w->origin_lon = 0.0;
            gd.extras.wind[i].wind_w->origin_lat = 0.0;
        } else {
            gd.extras.wind[i].wind_w =  (met_record_t *) NULL;
        }
    }

        /* free temp space */
    for(i = 0; i < NUM_PARSE_FIELDS; i++) {
        ufree(cfield[i]);
    }
}

/*****************************************************************
 * MODIFY_XVIEW_OBJECTS : Modify any Xview objects that couldn't
 *    be set up in Devguide. This is primarily to avoid manually
 *    changing any *ui.c file
 */

void modify_xview_objects()
{
    int    i,j;
    int    render_line;
    int    value;
    int    cpwidth;
	int    xpos;
    extern void  field_mi_proc();
    extern void  zoom_mi_proc();

    char    str[NAME_LENGTH];
    char    string[128];
    char  *fname;
    char  *help_string;
    char    panel_choice_string[128];
    Menu_item mi;
    Xv_cursor cursor;

    for(i=0; i < MAX_DATA_FIELDS; i++) {
        gd.h_win.field_xid[i] = 0;
        gd.v_win.field_xid[i] = 0;
    }

    XSetBackground(gd.dpy,gd.def_gc,gd.extras.background_color->pixval);
     
    for(i=0; i < PRDS_NUM_COLORS; i++) {
        XSetLineAttributes(gd.dpy, gd.extras.prod.prds_color[i]->gc,
	    gd.extras.prod.prod_line_width,LineSolid,CapButt,JoinBevel);
    }

    for(i=0; i < gd.extras.num_wind_sets; i++) {
        XSetLineAttributes(gd.dpy,gd.extras.wind[i].color->gc,
	    gd.prod.prod_line_width,LineSolid,CapButt,JoinBevel);
    }

    for(i=0; i < NUM_CONT_LAYERS; i++) {
        XSetLineAttributes(gd.dpy,gd.extras.cont[i].color->gc,
	    gd.prod.prod_line_width,LineSolid,CapButt,JoinBevel);
    }

    /* determine margin and other window assoc sizes for HORIZ window */
    gd.h_win.margin.top =  XRSgetLong(gd.cidd_db, "cidd.horiz_top_margin", 20);
    gd.h_win.margin.bot =  XRSgetLong(gd.cidd_db, "cidd.horiz_bot_margin", 20);
    gd.h_win.margin.left = XRSgetLong(gd.cidd_db, "cidd.horiz_left_margin", 20);
    gd.h_win.margin.right = XRSgetLong(gd.cidd_db, "cidd.horiz_right_margin", 80);

    gd.h_win.min_height = XRSgetLong(gd.cidd_db, "cidd.horiz_min_height", 440);
    gd.h_win.min_width = XRSgetLong(gd.cidd_db, "cidd.horiz_min_width", 580);
    gd.h_win.active = 1;

    gd.limited_mode = XRSgetLong(gd.cidd_db, "cidd.limited", 0);

    gd.check_data_times = XRSgetLong(gd.cidd_db, "cidd.check_data_times", 0);

    gd.frame_label = XRSgetString(gd.cidd_db, "cidd.horiz_frame_label", "CIDD"),
    gd.no_data_message = XRSgetString(gd.cidd_db, "cidd.no_data_message", "NO DATA FOUND (in this area at the selected time)"),

    fname = XRSgetString(gd.cidd_db, "cidd.status_info_file", ""); 
    if(strlen(fname) < 2) {
        gd.status.status_fname = NULL;
    } else {
	gd.status.status_fname = fname;
    }
    xv_set(gd.h_win_horiz_bw->horiz_bw,
        XV_X, XRSgetLong(gd.cidd_db, "cidd.horiz_default_x_pos",0),
        XV_Y, XRSgetLong(gd.cidd_db, "cidd.horiz_default_y_pos",0),
        XV_HEIGHT, XRSgetLong(gd.cidd_db, "cidd.horiz_default_height", 440),
        XV_LABEL, gd.frame_label,
        NULL);

    XSetWindowColormap(gd.dpy,((Window) xv_get(gd.h_win_horiz_bw->horiz_bw,XV_XID,NULL)),gd.cmap);
    XSetWindowColormap(gd.dpy,gd.hcan_xid,gd.cmap);
    XSetWindowColormap(gd.dpy,gd.vcan_xid,gd.cmap);

    xv_set(gd.h_win_horiz_bw->overlay_st,
        PANEL_CHOICE_STRING, 0,XRSgetString(gd.cidd_db,"cidd.overlay_bt_label","Overlays"), NULL,
        NULL);
    xv_set(gd.h_win_horiz_bw->product_st,
        PANEL_CHOICE_STRING, 0,XRSgetString(gd.cidd_db,"cidd.product_bt_label","Warnings"),NULL,
        NULL);

    if(gd.limited_mode) {
        /* Hide Aux Popup controls. */
        xv_set(gd.h_win_horiz_bw->movie_st,XV_SHOW,FALSE,NULL);
        xv_set(gd.h_win_horiz_bw->x_sect_st,XV_SHOW,FALSE,NULL);

		xpos = 142;
        xv_set(gd.h_win_horiz_bw->overlay_st,
            XV_SHOW,TRUE,PANEL_VALUE,0,
            XV_X,xpos,NULL);
		xpos += xv_get(gd.h_win_horiz_bw->overlay_st,XV_WIDTH) +1;

        xv_set(gd.h_win_horiz_bw->product_st,
            XV_SHOW,TRUE,XV_X,xpos,
	        PANEL_VALUE,1,NULL);
		xpos += xv_get(gd.h_win_horiz_bw->product_st,XV_WIDTH) +1;

        xv_set(gd.h_win_horiz_bw->vector_st,
			XV_SHOW,TRUE,
			PANEL_VALUE,1,
			XV_X,xpos,
			NULL);
		xpos += xv_get(gd.h_win_horiz_bw->vector_st,XV_WIDTH) +5;


        xv_set(gd.h_win_horiz_bw->movie_frame_msg,
		    XV_X,xpos,
			XV_Y,6,
			PANEL_LABEL_STRING, "Frame 00: 00:00",
			NULL);

		xpos += xv_get(gd.h_win_horiz_bw->movie_frame_msg,XV_WIDTH) +2;

        xv_set(gd.h_win_horiz_bw->movie_spd_sl,XV_X,xpos,XV_Y,6,NULL);

        xv_set(gd.h_win_horiz_bw->cp,XV_HEIGHT,30,NULL);
        xv_set(gd.h_win_horiz_bw->canvas1,XV_Y,30,NULL);

        xv_set(gd.h_win_horiz_bw->cur_time_msg,XV_Y,6,NULL);

        xv_set(gd.h_win_horiz_bw->export_st,XV_SHOW,FALSE,NULL);
        xv_set(gd.h_win_horiz_bw->slice_sl,XV_SHOW,FALSE,NULL);
        xv_set(gd.h_win_horiz_bw->cur_ht_msg,XV_SHOW,FALSE,NULL);
        xv_set(gd.h_win_horiz_bw->field_sel_st,XV_SHOW,FALSE,NULL);


    } else {
        xv_set(gd.h_win_horiz_bw->overlay_st, XV_SHOW,TRUE,PANEL_VALUE,0,NULL);
        xv_set(gd.h_win_horiz_bw->vector_st,XV_SHOW,FALSE,NULL);
    }

	update_ticker(time(0));
	xv_set(gd.h_win_horiz_bw->cur_time_msg,
	   XV_X,
	   (xv_get(gd.h_win_horiz_bw->cp,XV_WIDTH) -
	   xv_get(gd.h_win_horiz_bw->cur_time_msg,XV_WIDTH) -5),
	   NULL);

    /* Make Movie controls available from the front panel */
    xv_set(gd.h_win_horiz_bw->movie_spd_sl,
            XV_SHOW,TRUE,
            PANEL_MIN_VALUE, 0,
            PANEL_MAX_VALUE,    MOVIE_SPEED_RANGE,
            PANEL_VALUE,        (MOVIE_SPEED_RANGE - (gd.movie.display_time_msec/ MOVIE_DELAY_INCR)),
            NULL);


    xv_set(gd.movie_pu->movie_type_st, PANEL_VALUE,gd.movie.mode, NULL);
    xv_set(gd.h_win_horiz_bw->movie_frame_msg, XV_SHOW,TRUE, NULL);

    if(gd.movie.movie_on) {
        xv_set(gd.movie_pu->start_st, PANEL_VALUE,1, NULL);
        xv_set(gd.h_win_horiz_bw->movie_start_st, PANEL_VALUE,1,XV_SHOW,TRUE, NULL);
    } else {
        xv_set(gd.movie_pu->start_st, PANEL_VALUE,0, NULL);
        xv_set(gd.h_win_horiz_bw->movie_start_st, PANEL_VALUE,0,XV_SHOW,TRUE, NULL);
    }

    xv_set(gd.h_win_horiz_bw->field_st,PANEL_VALUE,0,NULL);
    xv_set(gd.h_win_horiz_bw->zoom_st,PANEL_VALUE,0,NULL);
    sprintf(string,"Frame %2d: %2d:%02d",
	gd.movie.cur_frame + 1,
	gd.movie.frame[gd.movie.cur_frame].time_mid.hour,
	gd.movie.frame[gd.movie.cur_frame].time_mid.min);
    xv_set(gd.h_win_horiz_bw->movie_frame_msg, PANEL_LABEL_STRING, string, NULL);


    /* Put the field choice setting in a reasonable location */

    /* Set the labels on the Zoom choice_panel - last one is "Custom" */
    for(i=0; i < gd.h_win.num_zoom_levels-NUM_CUSTOM_ZOOMS; i++) {
        sprintf(panel_choice_string,"%d",i);
        sprintf(string,"cidd.level%d_label",i);
        sprintf(panel_choice_string,"%s",XRSgetString(gd.cidd_db,string , panel_choice_string));
        xv_set(gd.zoom_pu->domain_st,PANEL_CHOICE_STRING,i,panel_choice_string,NULL);
    }

    for(i=gd.h_win.num_zoom_levels-NUM_CUSTOM_ZOOMS, j=1; i < gd.h_win.num_zoom_levels; i++, j++) {
	sprintf(panel_choice_string,"Custom%d",j);
        xv_set(gd.zoom_pu->domain_st,PANEL_CHOICE_STRING,
              i,panel_choice_string,NULL);
    }

    xv_set(gd.zoom_pu->domain_st, PANEL_VALUE, gd.h_win.zoom_level,XV_X,0,XV_Y,0, NULL);
    xv_set(gd.zoom_pu->zoom_pu,XV_HEIGHT,xv_get(gd.zoom_pu->domain_st,XV_HEIGHT),NULL);
    xv_set(gd.zoom_pu->zoom_pu,XV_WIDTH,xv_get(gd.zoom_pu->domain_st,XV_WIDTH),NULL);
	 
    /* Set the labels on the Overlay choice_panel */
    value = 0;
    for(i=0; i < gd.num_map_overlays && i < 32; i++) {
        xv_set(gd.over_pu->over_pu_st,PANEL_CHOICE_STRING,i,gd.over[i]->control_label,NULL);
		if(gd.over[i]->active) value |= 1 <<i;
    }
    xv_set(gd.over_pu->over_pu_st, PANEL_VALUE, value,XV_X,0,XV_Y,0, NULL);
    xv_set(gd.over_pu->over_pu,XV_HEIGHT,xv_get(gd.over_pu->over_pu_st,XV_HEIGHT),NULL);
    xv_set(gd.over_pu->over_pu,XV_WIDTH,xv_get(gd.over_pu->over_pu_st,XV_WIDTH),NULL);

    /* determine margin and other window assoc sizes for VERT window */
    gd.v_win.margin.top =  XRSgetLong(gd.cidd_db, "cidd.vert_top_margin", 20);
    gd.v_win.margin.bot =  XRSgetLong(gd.cidd_db, "cidd.vert_bot_margin", 20);
    gd.v_win.margin.left = XRSgetLong(gd.cidd_db, "cidd.vert_left_margin", 20);
    gd.v_win.margin.right = XRSgetLong(gd.cidd_db, "cidd.vert_right_margin", 80);

    gd.v_win.min_height = XRSgetLong(gd.cidd_db, "cidd.vert_min_height", 440);
    gd.v_win.min_width = XRSgetLong(gd.cidd_db, "cidd.vert_min_width", 540);
    gd.v_win.active = 0;

    xv_set(gd.v_win_v_win_pu->v_win_pu,
           WIN_HEIGHT,    XRSgetLong(gd.cidd_db, "cidd.vert_default_height", 240),
           WIN_WIDTH,    XRSgetLong(gd.cidd_db, "cidd.vert_default_width", 360),
           FRAME_SHOW_HEADER,    FALSE,
           XV_SHOW,    FALSE,
           NULL);


    /* Build the Field menus & lists */
    for (i = 0, render_line = 0; i < gd.num_datafields; i++) {

        xv_set(gd.fields_pu->display_list,
               PANEL_LIST_INSERT, i,
               PANEL_LIST_STRING, i, gd.mrec[i]->field_name,
               PANEL_LIST_CLIENT_DATA, i, i,
               NULL);

        if (gd.mrec[i]->currently_displayed) {
            xv_set(gd.fields_pu->display_list,
                   PANEL_LIST_SELECT, i, TRUE,
                   NULL);

            render_line++;
        }

    }

    /* Build all of the field menu's and choice panels. */
    init_field_menus();

    /* make sure there is at least one field in the choice lists */
    if (render_line <= 0) {
        gd.mrec[0]->currently_displayed = TRUE;
        xv_set(gd.fields_pu->display_list,
               PANEL_LIST_SELECT, 0, TRUE,
               NULL);

    }

    /* set the current field for each window */
    gd.h_win.field = gd.v_win.field = gd.field_index[0];

    xv_set(gd.extras_pu->prod_tm_sel_st,
        PANEL_VALUE,gd.prod.product_time_select,
        NULL);

    xv_set(gd.extras_pu->prod_tm_wdth_sl,
        PANEL_VALUE,gd.prod.product_time_width,
        NULL);

    xv_set(gd.extras_pu->prod_line_sl,
        PANEL_VALUE,gd.prod.prod_line_width,
        NULL);

    xv_set(gd.extras_pu->prod_font_sl,
        PANEL_MAX_VALUE,    gd.num_fonts -1 ,
        PANEL_VALUE,gd.prod.prod_font_num,
        NULL);

    xv_set(gd.extras_pu->dim_sl,
        PANEL_MAX_VALUE,    gd.inten_levels,
        PANEL_VALUE,(int)(gd.inten_levels * gd.image_inten),
        PANEL_NOTIFY_LEVEL,PANEL_ALL,
        NULL);

    /* make sure the horiz window's slider has the correct range */
    xv_set(gd.h_win_horiz_bw->slice_sl,
            PANEL_MIN_VALUE, 0,
         /*   PANEL_NOTIFY_LEVEL,PANEL_NONE, /*  */
            PANEL_NOTIFY_LEVEL,PANEL_ALL,    /*  */
            PANEL_MAX_VALUE, gd.h_win.num_slices -1,
            PANEL_VALUE, (int)((gd.h_win.cmin_ht - gd.h_win.min_ht) / gd.h_win.delta),
         /*    PANEL_TICKS, (int) (gd.h_win.max_ht - gd.h_win.min_ht), /*  */
            NULL);

    sprintf(str,"%g km",((gd.h_win.cmin_ht+gd.h_win.cmax_ht) /2.0));
    xv_set(gd.h_win_horiz_bw->cur_ht_msg,
           PANEL_LABEL_STRING, str,
           NULL);

    /* set extras popup parameters */
    xv_set(gd.extras_pu->extras_pu,
           XV_SHOW, FALSE,
           FRAME_CMD_PUSHPIN_IN,    TRUE,
           FRAME_SHOW_HEADER, FALSE,
           NULL);

    /* Fill in WIND Vector scrolling List */
    for (i = 0; i < gd.extras.num_wind_sets; i++ ) {
        sprintf(str,"%s  %s",
	    gd.extras.wind[i].wind_u->source_name,
	    gd.extras.wind[i].color->name);

	xv_set(gd.extras_pu->wind_on_list,
			PANEL_LIST_STRING, i, str,
			PANEL_LIST_SELECT, i, gd.extras.wind[i].active,
			NULL); 
				      
    }

    xv_set(gd.extras_pu->wind_sl,
           PANEL_VALUE, 2,
           NULL);
    xv_set(gd.extras_pu->wind_msg,
           PANEL_LABEL_STRING, "30 min",
           NULL);

    xv_set(gd.extras_pu->ref_mark_color_msg,
           PANEL_LABEL_STRING, gd.extras.range_color->name,
           NULL);


    /* Set default condition of reference marker setting */
    value = 0;
    if (gd.extras.range) value |= RANGE_BIT;
    if (gd.extras.azmiths) value |= AZMITH_BIT;
    xv_set(gd.extras_pu->ref_st,
           PANEL_VALUE, value,
           NULL);


    /* Fill in Geographic Overlay Scrolling List */
    if(gd.debug) printf("%d Overlays sets found\n",gd.num_map_overlays);
    for(i=0; i < gd.num_map_overlays; i++) {
        sprintf(string,"%s  %s",gd.over[i]->control_label, gd.over[i]->color_name);
        xv_set(gd.extras_pu->overlay_list,
            PANEL_LIST_STRING, i, string,
            PANEL_LIST_CLIENT_DATA, i,i,
            PANEL_LIST_SELECT, i, gd.over[i]->active,
            NULL);
    }


    /* set movie popup parameters */
    xv_set(gd.movie_pu->movie_frame_sl,
        PANEL_MIN_VALUE, 1,
        PANEL_MAX_VALUE,    gd.movie.num_frames,
        PANEL_VALUE,        gd.movie.cur_frame,
        NULL);

    xv_set(gd.movie_pu->movie_dwell_sl,
        PANEL_MIN_VALUE, 0,
        PANEL_MAX_VALUE,    10,
        PANEL_VALUE,        2,
        NULL);


    xv_set(gd.movie_pu->movie_speed_sl,
        PANEL_MIN_VALUE, 0,
        PANEL_MAX_VALUE,    MOVIE_SPEED_RANGE,
        PANEL_VALUE,        (MOVIE_SPEED_RANGE - (gd.movie.display_time_msec/ MOVIE_DELAY_INCR)),
        NULL);

    if( gd.movie.movie_on ) {
        xv_set(gd.movie_pu->start_st, PANEL_VALUE,1, NULL);
    } else {
        xv_set(gd.movie_pu->start_st, PANEL_VALUE,0, NULL);
    }


    update_movie_popup();

    xv_set(gd.movie_pu->movie_pu,
           XV_SHOW, FALSE,
           FRAME_CMD_PUSHPIN_IN, TRUE,
           FRAME_SHOW_HEADER, FALSE,
           NULL);

    /* set fields popup parameters */
    xv_set(gd.fields_pu->fields_pu,
           XV_SHOW, FALSE,
           FRAME_CMD_PUSHPIN_IN, TRUE,
           FRAME_SHOW_HEADER, FALSE,
           NULL);

    /* set data selector popup parameters */
    xv_set(gd.data_pu->data_pu,
           XV_SHOW, FALSE,
           FRAME_CMD_PUSHPIN_IN, TRUE,
           FRAME_SHOW_HEADER, FALSE,
           NULL);


    /* set overlay selector popup parameters */
    xv_set(gd.over_pu->over_pu,
           XV_SHOW, FALSE,
           FRAME_CMD_PUSHPIN_IN, TRUE,
           FRAME_SHOW_HEADER, FALSE,
           NULL);

    /* set zoom level popup parameters */
    xv_set(gd.zoom_pu->zoom_pu,
           XV_SHOW, FALSE,
           FRAME_CMD_PUSHPIN_IN, TRUE,
           FRAME_SHOW_HEADER, FALSE,
           NULL);

    /* set up the help text */
    xv_set(gd.h_win_horiz_bw->movie_start_st, XV_HELP_DATA,
           XRSgetString(gd.cidd_db, "cidd.loop_button_help", "cidd_help:loop_button"),
           NULL);

    xv_set(gd.h_win_horiz_bw->zoom_st, XV_HELP_DATA,
           XRSgetString(gd.cidd_db, "cidd.view_button_help", "cidd_help:view_button"),
           NULL);

    xv_set(gd.h_win_horiz_bw->field_st, XV_HELP_DATA,
           XRSgetString(gd.cidd_db, "cidd.field_button_help", "cidd_help:field_button"),
           NULL);

    xv_set(gd.h_win_horiz_bw->movie_st, XV_HELP_DATA,
           XRSgetString(gd.cidd_db, "cidd.movie_button_help", "cidd_help:movie_button"),
           NULL);

    xv_set(gd.h_win_horiz_bw->overlay_st, XV_HELP_DATA,
           XRSgetString(gd.cidd_db, "cidd.overlay_button_help", "cidd_help:overlay_button"),
           NULL);

    xv_set(gd.h_win_horiz_bw->product_st, XV_HELP_DATA,
           XRSgetString(gd.cidd_db, "cidd.warnings_button_help", "cidd_help:warnings_button"),
           NULL);

    xv_set(gd.h_win_horiz_bw->export_st, XV_HELP_DATA,
           XRSgetString(gd.cidd_db, "cidd.draw_button_help", "cidd_help:draw_button"),
           NULL);

    xv_set(gd.h_win_horiz_bw->x_sect_st, XV_HELP_DATA,
           XRSgetString(gd.cidd_db, "cidd.vert_button_help", "cidd_help:vert_button"),
           NULL);

    xv_set(gd.h_win_horiz_bw->field_sel_st, XV_HELP_DATA,
           XRSgetString(gd.cidd_db, "cidd.config_button_help", "cidd_help:config_button"),
           NULL);

    xv_set(gd.h_win_horiz_bw->canvas1, XV_HELP_DATA,
           /* WIN_CURSOR, cursor,  /*  */
           XRSgetString(gd.cidd_db, "cidd.canvas_help", "cidd_help:canvas"),
           NULL);

    xv_set(gd.h_win_horiz_bw->movie_frame_msg, XV_HELP_DATA,
           XRSgetString(gd.cidd_db, "cidd.frame_message_help", "cidd_help:frame_message"),
           NULL);

    xv_set(gd.h_win_horiz_bw->movie_spd_sl, XV_HELP_DATA,
           XRSgetString(gd.cidd_db, "cidd.speed_slider_help", "cidd_help:speed_slider"),
           NULL);

    update_layered_contour_panel();
    update_layered_field_panel();
}

/*****************************************************************
 * INIT_FIELD_MENUS(void);
 *
 */


void init_field_menus(void)
{
    int i;
    int x,y;
	int ncols;
    static Menu field_menu;

    /* Clear out any old items */
    xv_set(gd.data_pu->data_st, PANEL_CHOICE_STRINGS, "", NULL, NULL);

    gd.num_menu_fields = 0;
    for(i=0; i < gd.num_datafields; i++) {
        if(gd.mrec[i]->currently_displayed) {
           xv_set(gd.data_pu->data_st, PANEL_CHOICE_STRING,gd.num_menu_fields, gd.mrec[i]->field_name,NULL);
           gd.field_index[gd.num_menu_fields] = i;
           gd.num_menu_fields++;
        }
    }
    ncols = (gd.num_field_menu_cols <=0 ) ? (int)(gd.num_menu_fields / 30.0) + 1
                                         : gd.num_field_menu_cols;

    xv_set(gd.data_pu->data_st,XV_X,0,XV_Y,0,PANEL_CHOICE_NCOLS,ncols,NULL);
    x = xv_get(gd.data_pu->data_st,XV_WIDTH);
    y = xv_get(gd.data_pu->data_st,XV_HEIGHT);

    xv_set(gd.data_pu->data_pu,XV_HEIGHT,y,XV_WIDTH,x,NULL);

}
