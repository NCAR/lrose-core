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
/*************************************************************************
 * XSPDB_CLIENT.c - A Simple SPDB Client
 *
 * F. hage. 3/98
 */

#define XSPDB_CLIENT_MAIN
 
#include "xspdb_client.h"

void process_args( int argc, char *argv[]);
void init_xview( int *argc_ptr, char    *argv[]);
int  x_error_proc( Display *disp, XErrorEvent *event);
Notify_value base_win_destroy( Notify_client   client, Destroy_status  status);
void init_data_space(void);
void modify_xview_objects(void);
void tidy_and_exit(int sig);

extern void update_controls(void);

/********************************************************************
 * MAIN: Process arguments, initialize and begin application
 */
 

void
main(argc, argv)
	int		argc;
	char		**argv;
{
	xv_init(XV_INIT_ARGC_PTR_ARGV,&argc,argv,XV_X_ERROR_PROC,x_error_proc,NULL);
	 
	ZERO_STRUCT (&gd);

	process_args(argc,argv);	/* process command line arguments */

	/* initialize globals, get/set defaults, establish data sources etc. */
	init_data_space();

	PORTsignal(SIGINT, tidy_and_exit);
	PORTsignal(SIGTERM, tidy_and_exit);
	PORTsignal(SIGQUIT, tidy_and_exit);
 
	init_xview(&argc,argv); /* create all Xview objects */	

	/* make changes to xview objects not available from DevGuide */
	modify_xview_objects();

	/*
	 * Turn control over to XView.
	 */
	xv_main_loop(gd.win->window1);
	exit(0);
}

#define ARG_OPTION_STRING   "dqhs:t:"
/*****************************************************************
 * PROCESS_ARGS: Progess command line arguments. Set option flags
 *	   And print usage info if necessary
 */

void process_args( int argc, char *argv[])
{
	int err_flag =0;
	int	 c;
	extern  int optind; /* index to remaining arguments */
	extern  char *optarg;   /* option argument string */
	STRcopy(gd.source_string,"./",1024);
	gd.product_type = 0;

	gd.app_name = "xspdb_client";
	 
	while ((c = getopt(argc, argv,ARG_OPTION_STRING)) != EOF) {
		switch(c) {
			case 'd' :	/* debug mode */
				gd.debug = 1;
			break;
 
			case 's' :	/* alternate parameter database file */
				STRcopy(gd.source_string,optarg,1024);
			break;

			case 't' :	/* debug mode */
				gd.product_type = atoi(optarg);
			break;
 
 
			case '?':   /* error in options */
			case 'h':   /* help */
			case 'q':   /* querry */
			default:
				err_flag++;
			break;
		}
 
	};
 
	if(err_flag) {
		fprintf(stderr,"Usage:xspdb_client [-d] [-t product_type] [-s source_string]\n");
		exit(-1);
	}
}  
 
/***************************************************************************
 * INIT_XVIEW : Initialize the base frame and other global objects
 */ 
 
void init_xview( int *argc_ptr, char	*argv[])
{
	Notify_value	base_win_destroy(); /* destroy interposer */
 
	gd.win = xspdb_client_gui_window1_objects_initialize(NULL, NULL);
	
	notify_interpose_destroy_func(gd.win->window1,base_win_destroy);
}
/*****************************************************************
 * INIT_DATA_SPACE: 
 */

void init_data_space(void)
{
    int i;
    char res_string[128];
    char buf[128];
    char *dtime;

    INSTANCE = xv_unique_key(); /* get keys for retrieving data */
    DATA_KEY = xv_unique_key(); /* get keys for retrieving data */

	gd.product_id = 0;   /* Will pick up all products */

	gd.end_time = time(0); /* Default to now, within an hour */

	gd.begin_time = gd.end_time - 3600;
	gd.delta_time = 3600;

	gd.request_type = 0; /* Interval by default */

}

/*****************************************************************
 * MODIFY_XVIEW_OBJECTS : Modify any Xview objects that couldn't
 *    be set up in Devguide. This is primarily to avoid manually
 *    changing any *ui.c file
 */

void modify_xview_objects(void)
{
   int i;

   for(i=0; i < NUM_PROD_TYPES; i++) {
       xv_set(gd.win->prod_typ_st,
		   PANEL_CHOICE_STRING, i , Prod[i].product_label,
		   NULL);

	   if(gd.product_type == Prod[i].product_type) {
		   xv_set(gd.win->prod_typ_st,PANEL_VALUE,i,NULL);
		}
	}

	xv_set(gd.win->textpane1,TEXTSW_MEMORY_MAXIMUM,2097152,NULL);

	textsw_reset(gd.win->textpane1,0,0);


	update_controls();
}


/***************************************************************
 * TIDY_AND_EXIT
 */
void tidy_and_exit(int sig)
{
  exit(sig);
}

/*****************************************************************
 * X_ERROR_PROC: Handle errors generated by the X server
 */
 
int x_error_proc( Display *disp, XErrorEvent *event)
{
	char	text[256];
 
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
}
 
/*****************************************************************
 * BASE_WIN_DESTROY: Interposition for base frame destroys
 */

Notify_value
base_win_destroy( Notify_client   client, Destroy_status  status)
{

	switch(status) {
		case DESTROY_CLEANUP:
 
		case DESTROY_PROCESS_DEATH:
			return notify_next_destroy_func(client,status);
		break;
 
		case DESTROY_CHECKING:
			return NOTIFY_DONE;
		break;
 
		case DESTROY_SAVE_YOURSELF:
			return NOTIFY_DONE;
		break;
	}
}

