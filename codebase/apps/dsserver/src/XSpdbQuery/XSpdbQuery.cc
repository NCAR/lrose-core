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
/*************************************************************************
 * XSPDB_CLIENT.c - A Simple SPDB Client
 *
 * F. hage. 3/98
 */

#define XSPDB_CLIENT_MAIN
 
#include "XSpdbQuery.h"
using namespace std;

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
 
int main(int argc, char **  argv)
{
	 
	ZERO_STRUCT (&gd);

	process_args(argc,argv);	/* process command line arguments */

	xv_init(XV_INIT_ARGC_PTR_ARGV,&argc,argv,XV_X_ERROR_PROC,x_error_proc,NULL);

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

#define ARG_OPTION_STRING   "Edqh:s:t:p:"
/*****************************************************************
 * PROCESS_ARGS: Progess command line arguments. Set option flags
 *	   And print usage info if necessary
 */

void process_args( int argc, char *argv[])
{
	int err_flag =0;
	int	 c;
	extern  char *optarg;   /* option argument string */
	STRcopy(gd.source_string,"./",1024);
	gd.product_type = 0;

	gd.app_name = "XSpdbQuery";
	 
	while ((c = getopt(argc, argv,ARG_OPTION_STRING)) != EOF) {
		switch(c) {
			case 'E' :	/* Expert Delete mode */
				gd.dangerous_delete = 1;
				fprintf(stderr,"\n\n\n######################################################\n");
				fprintf(stderr,"#  WARNING! YOU HAVE ENABLED THE EXPERT DELETE MODE  #\n");
				fprintf(stderr,"#    The Delete Button can remove multiple products  #\n");
				fprintf(stderr,"#          One click from data loss                 #\n");
				fprintf(stderr,"#####################################################\n");
			break;
 
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
			case 'p':   /* querry */
			default:
				err_flag++;
			break;
		}
 
	};
 
	if(err_flag) {
		fprintf(stderr,"\nUsage:XSpdbQuery [-d]  [-s source_URL]\n\n");
		fprintf(stderr,"d - Enable debug messages\n");
		fprintf(stderr,"E - \"Expert/Extremely Dangerous\" Delete Mode - Lose data faster!\n");
		fprintf(stderr,"\n NOTE:  DsURLS take the following form:\n\n");
		fprintf(stderr,">> ./dir                      - Explicit directory under 'cwd'\n");
		fprintf(stderr,">> /dir                       - Explicit directory on local filesystem\n");
		fprintf(stderr,">> dir                        - Explicit directory on local filesystem under $RAP_DATA_DIR\n");
		fprintf(stderr,"      If $RAP_DATA_DIR not defined - relative to 'cwd'\n");
		fprintf(stderr,"\nNote: DsServerMgr must be run on each host if spdbp is specified.\n\n");
		fprintf(stderr,">> spdbp:://host.dom::dir              - Directory on remote host under $RAP_DATA_DIR\n");
		fprintf(stderr,">> spdbp:transform_exe://host.dom::dir - Directory on remote host under $RAP_DATA_DIR\n");
		fprintf(stderr,"      transform_exe                    - Executable which translates to Symprod (Display) Format\n\n");
		fprintf(stderr,">> spdbp:transform_exec:param_name//host.dom::dir - Directory on remote host under $RAP_DATA_DIR\n");
		fprintf(stderr,"      transform_exe - Executable which translates to Symprod (Display) Format\n");
		fprintf(stderr,"      param_name - The extension of a parameter file sitting in the dir.\n\n");
		fprintf(stderr,"\n Note: Name the param file in the data dir; _transform_exec.param_name\n");
		fprintf(stderr,"       Example: _Metar2Symprod.no_labels   -->\n");
		fprintf(stderr,"   spdbp:Metar2Symprod:no_labels//localhost::spdb/metars\n");
		fprintf(stderr,"\n\n");
		fprintf(stderr,"HOW TO USE THE DELETE FUNCTION:\n");
		fprintf(stderr,"Set the time and data type settings such that *ONLY ONE* product is returned\n");
		fprintf(stderr,"The delete button will become active when a single product is identified.\n");
		fprintf(stderr,"Click - its Gone! - NO UNDO! The delete button is inactive if the \n");
		fprintf(stderr,"number of products returned by the latest request not equal to 1\n");
		fprintf(stderr,"except when you've enabled the Expert Delete mode with the -E option\n");
		fprintf(stderr,"Note: data type and data type2 can be set by entering a 4 char station ID\n");
		exit(-1);
	}
}  
 
/***************************************************************************
 * INIT_XVIEW : Initialize the base frame and other global objects
 */ 
 
void init_xview( int *argc_ptr, char	*argv[])
{
	gd.win = xspdb_client_gui_window1_objects_initialize(0, 0);
	
	notify_interpose_destroy_func(gd.win->window1,
				      (notify_value (*)(...)) base_win_destroy);
}
/*****************************************************************
 * INIT_DATA_SPACE: 
 */

void init_data_space(void)
{
    INSTANCE = xv_unique_key(); /* get keys for retrieving data */
    DATA_KEY = xv_unique_key(); /* get keys for retrieving data */

    gd.data_type = 0; 
    gd.data_type2 = 0;
    gd.num_active_products = 0;

	gd.end_time = time(0); /* Default to now, within an hour */

	gd.begin_time = gd.end_time - 3600;
	gd.delta_time = 3600;

	gd.request_type = 2; /* Interval */

}

/*****************************************************************
 * MODIFY_XVIEW_OBJECTS : Modify any Xview objects that couldn't
 *    be set up in Devguide. This is primarily to avoid manually
 *    changing any *ui.c file
 */

void modify_xview_objects(void)
{
   int i;

   int num_prod_types = sizeof(Prod) / sizeof(prod_info_t);
   
//   for(i=0; i < NUM_PROD_TYPES; i++) {
   for(i=0; i < num_prod_types; i++) {
       xv_set(gd.win->prod_typ_st,
		   PANEL_CHOICE_STRING, i , Prod[i].product_label,
		   NULL);

	   if(gd.product_type == Prod[i].product_type) {
		   xv_set(gd.win->prod_typ_st,PANEL_VALUE,i,NULL);
		}
	}

	xv_set(gd.win->textpane1,TEXTSW_MEMORY_MAXIMUM,8388608,NULL);


	textsw_reset(gd.win->textpane1,0,0);

        xv_set(gd.win->req_typ_st,PANEL_VALUE,gd.request_type,NULL);

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
	fprintf(stderr,"Generated X error : %s, ID:%u\n",text,
		(unsigned int)event->resourceid);
 
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

	return NOTIFY_DONE;
}

