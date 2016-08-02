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
 * CIDD.C : A program to display cartesian RADAR data
 *
 * For the Cartesian Radar Display (CIDD) 
 * Frank Hage    July 1991 NCAR, Research Applications Program
 */

#define    RD_MAIN    1        /* This is the main module */
#define    rd_main

#include "cidd.h"

/********************************************************************
 * MAIN: Process arguments, initialize and begin application
 */
 
void
main(int  argc, char **argv)
{

    ZERO_STRUCT (&gd);

    process_args(argc,argv);    /* process command line arguments */

    /* initialize globals, get/set defaults, establish data sources etc. */
    init_data_space();
 
    init_xview(&argc,argv); /* create all Xview objects */    

    setup_colorscales(gd.dpy);    /* Establish color table & mappings  */

    /* make changes to xview objects not available from DevGuide */
    modify_xview_objects();

    start_timer();                /* start up the redraw/movieloop timer */

    xv_main_loop(gd.h_win_horiz_bw->horiz_bw);
    exit(0);
}

#define ARG_OPTION_STRING   "v:d:i:t:u"
/*****************************************************************
 * PROCESS_ARGS: Progess command line arguments. Set option flags
 *       And print usage info if necessary
 */

void process_args(int argc, char *argv[])
{
    int err_flag =0;
    int c;
    int n_fields;
    int debug_level;
    static char dir_buf[1024];
    char dir_ptr;
    char *app_ptr;
    char *slash;
    extern  int optind; /* index to remaining arguments */
    extern  char *optarg;   /* option argument string */
    UTIMstruct     temp_utime;

    gd.db_name = APP_DATABASE;    /* Set the default data base name */
	if((app_ptr = strrchr(argv[0],'/')) == NULL) {
    	    gd.app_name = argv[0];
	} else {
	    gd.app_name = ++app_ptr;
	}
	gd.app_instance = "Generic";
     
    while ((c = getopt(argc, argv,ARG_OPTION_STRING)) != EOF) {
        switch(c) {
            case 'd' :    /* parameter database file */
                /* Change working directories to the place where the config file exists */
		STRcopy(dir_buf,optarg,1024);
                if((slash = strrchr(dir_buf,'/')) != NULL) {
                    gd.db_name = slash + 1;
                    *slash = '\0';
                    if(chdir(dir_buf) < 0) {
                         fprintf(stderr,"Couldn't cd to %s\n",dir_buf);
                         err_flag++;
                    }
                } else {
                    gd.db_name = optarg;
                }
            break;
 
            case 'v' :    
		debug_level = atoi(optarg);
		gd.debug |= (debug_level & 0x01);
		gd.debug1 |= (debug_level & 0x02);
		gd.debug2 |= (debug_level & 0x04);
            break;
 
            case 'i' :    
		gd.app_instance = optarg;
                printf("CIDD Instance: %s\n",optarg);
            break;
 
            case 'u' :    
		gd.run_unmapped = 1;
                printf("CIDD will run unmapped\n");
            break;
 
            case 't' :    
		n_fields = sscanf(optarg,"%4d%2d%2d%2d%2d",
		    &temp_utime.year,
		    &temp_utime.month,
		    &temp_utime.day,
		    &temp_utime.hour,
		    &temp_utime.min);
		
		if(n_fields == 5) {
		    temp_utime.sec = 0;
		    temp_utime.unix_time = UTIMdate_to_unix(&temp_utime);
		    gd.movie.mode = MOVIE_TS;
		    gd.movie.start_time.unix_time = temp_utime.unix_time;
                    printf("CIDD Starting with Data time: %s\n",ctime(&temp_utime.unix_time));
		} else {
		    fprintf(stderr,"Problems parsing time: %s\n",optarg);
		    fprintf(stderr,"Use: YYYYMMDDHHMM : Example 199806211624\n");
		} 
            break;
 
            case '?':   /* error in options */
            default:
                if(err_flag != 1) err_flag++;
            break;
        }
 
    };
 
    if(err_flag) {
        fprintf(stderr,"\nUsage:cidd [-d Defaults_file] [-v report_level] [-i Instance string] [-t YYYYMMDDHHMM]\n");
        fprintf(stderr,"\n           -d: CIDD will look for config files in dir where Defaults_file is located\n");
        fprintf(stderr,"\n           -v: Level 0-7 Verbosity (Bitwise flags)\n");
        fprintf(stderr,"           -i: CIDD registers with the process mapper using this instance\n");
        fprintf(stderr,"           -t: CIDD Starts up in archive mode at this time\n");
        fprintf(stderr,"\n");
    }
}  
 
/***************************************************************************
 * INIT_XVIEW : Initialize the base frame and other global objects
 */ 
 
void init_xview(int *argc_ptr, char    *argv[])
{
    Notify_value    base_win_destroy(); /* destroy interposer */
    int    x_error_proc();
    extern Notify_value    h_win_events();
    extern Notify_value    v_win_events();
 
    xv_init(XV_INIT_ARGC_PTR_ARGV, argc_ptr, argv,
            XV_X_ERROR_PROC, x_error_proc,
            NULL);

    gd.h_win_horiz_bw = h_win_horiz_bw_objects_initialize(NULL, NULL);    
    notify_interpose_destroy_func(gd.h_win_horiz_bw->horiz_bw,
                                  base_win_destroy);

    gd.v_win_v_win_pu =
        v_win_v_win_pu_objects_initialize(NULL, gd.h_win_horiz_bw->horiz_bw);

    gd.extras_pu = extras_pu_extras_pu_objects_initialize(NULL, gd.h_win_horiz_bw->horiz_bw);

    gd.movie_pu = movie_pu_movie_pu_objects_initialize(NULL, gd.h_win_horiz_bw->horiz_bw);

    gd.zoom_pu = zoom_pu_zoom_pu_objects_initialize(NULL, gd.h_win_horiz_bw->horiz_bw);

    gd.data_pu = data_pu_data_pu_objects_initialize(NULL, gd.h_win_horiz_bw->horiz_bw);

    gd.over_pu = over_pu_over_pu_objects_initialize(NULL, gd.h_win_horiz_bw->horiz_bw);

    gd.save_pu = save_pu_save_im_pu_objects_initialize(NULL, gd.h_win_horiz_bw->horiz_bw);

    gd.fields_pu = fields_pu_fields_pu_objects_initialize(NULL, gd.h_win_horiz_bw->horiz_bw);

    gd.dpy = (Display *) xv_get(gd.h_win_horiz_bw->horiz_bw, XV_DISPLAY);
 
    gcc_initialize(gd.h_win_horiz_bw->horiz_bw, "Overlay Color Chooser"); 

    /* get a default global GC */
    gd.def_gc = DefaultGC(gd.dpy, DefaultScreen(gd.dpy));

    /* get the horizontal view canvas's XID */
    gd.hcan_xid = xv_get(canvas_paint_window(gd.h_win_horiz_bw->canvas1),
                         XV_XID);
    gd.h_win.vis_xid = gd.hcan_xid;

    /* Trap resize events in main base window */
    notify_interpose_event_func(gd.h_win_horiz_bw->horiz_bw,
                                h_win_events, NOTIFY_SAFE);

    /* get the vertical view canvas's XID */
    gd.vcan_xid = xv_get(canvas_paint_window(gd.v_win_v_win_pu->canvas1),
                         XV_XID);
    gd.v_win.vis_xid = gd.vcan_xid;

    /* Trap resize events in vertical display  popup window */
    notify_interpose_event_func(gd.v_win_v_win_pu->v_win_pu,
                                v_win_events, NOTIFY_SAFE);

    load_fonts(gd.dpy);
}

/*****************************************************************
 * X_ERROR_PROC: Handle errors generated by the X server
 */
 
x_error_proc(disp,event)
    Display *disp;
    XErrorEvent *event;
{
    char    text[256];
 
    XGetErrorText(disp,event->error_code,text,256);
    fprintf(stderr,"Generated X error : %s, ID:%u\n",text,event->resourceid);
 
    switch(event->error_code) {
        default :
            return   XV_OK;
        break;
 
        case BadAlloc :
            return   XV_OK;
        break;
    }
    return XV_OK;
}
 
/*****************************************************************
 * BASE_WIN_DESTROY: Interposition for base frame destroys
 */

Notify_value
base_win_destroy(client,status)
    Notify_client   client;
    Destroy_status  status;
{
    int i;
    char    fname[MAX_PATH_LEN];

    switch(status) {
        case DESTROY_CLEANUP:
 
        case DESTROY_PROCESS_DEATH:
            if(gd.debug) fprintf(stderr,"Removing Movie Frame Files \n");
            /* destroy movie frames  */
            for(i=0; i < MAX_FRAMES ; i++) {
                /* Build the complete filename */
                STRcopy(fname,gd.movie.frame[i].fname,MAX_PATH_LEN-2);
                strcat(fname,"h");
                unlink(fname);    
            
                /* Build the complete filename */
                STRcopy(fname,gd.movie.frame[i].fname,MAX_PATH_LEN-2);
                strcat(fname,"v");
                unlink(fname);    
            }    

            if(gd.debug) fprintf(stderr,"Removing Movie Pixmaps \n");
            manage_h_pixmaps(3);
            manage_v_pixmaps(3);

            /* Remove prod_sel shared memory */
            if(gd.debug) fprintf(stderr,"Removing Prod_sel shared memory\n");
            CSPR_destroy();

            /* Remove Exported coordinate shared memory */
            if(gd.debug) fprintf(stderr,"Removing Exported Coordinate shared memory\n");
	    ushm_detach((void *)gd.coord_expt);
	    if (ushm_nattach(XRSgetLong(gd.cidd_db,
					"cidd.coord_key",
					63500)) <= 0)
	      ushm_remove(XRSgetLong(gd.cidd_db, "cidd.coord_key", 63500));

            if(gd.debug) fprintf(stderr,"Removing PRDS shared memory\n");
	    delete_prds_shmem();


            if(gd.debug) fprintf(stderr,"Removing EXPRT shared memory\n");
	    delete_expt_shmem();

            if(gd.debug) fprintf(stderr,"De-Registering from Procmap\n");
            PMU_auto_unregister();

            fprintf(stderr,"\nStandard Clean Exit from Cidd\n\n");
	    PMU_unregister(gd.app_name, gd.app_instance);
            return notify_next_destroy_func(client,status);
        break;
 
        case DESTROY_CHECKING:
            return NOTIFY_DONE;
        break;
 
        case DESTROY_SAVE_YOURSELF:
            return NOTIFY_DONE;
        break;
    }
    return NOTIFY_DONE;
}
 
/************************************************************************
 * RD_H_MSG : Display a message in the title bar of the horiz base window
 */

void rd_h_msg(char    *string,int persistance)
{
    static time_t ok_time = 0;
	time_t now;

	now = time(0);
     
    if(now  >= ok_time || persistance < 0) {
        xv_set(gd.h_win_horiz_bw->horiz_bw,
               XV_LABEL, string,
               NULL);
	    ok_time = now + abs(persistance);
    }
}

/************************************************************************
 * RD_V_MSG : Display a message in the title bar of the Vert base window
 */

void rd_v_msg(string)
    char    *string;
{
    xv_set(gd.h_win_horiz_bw->horiz_bw,
           XV_LABEL, string,
           NULL);
}

#ifndef LINT
static char RCS_id[] = "$Id: cidd.c,v 1.28 2016/03/07 18:28:26 dixon Exp $";
static char SCCS_id[] = "%W% %D% %T%";
#endif /* not LINT */

