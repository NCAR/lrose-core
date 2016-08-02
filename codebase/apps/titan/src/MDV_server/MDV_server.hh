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

/************************************************************************
 * MDV_server.hh
 *
 */

#include <toolsa/umisc.h>
#include <toolsa/sockutil.h>
#include <toolsa/str.h>
#include <toolsa/pjg.h>
#include <toolsa/pmu.h>
#include <toolsa/smu.h>
#include <cidd/cdata_util.h>
#include <mdv/MdvRead.hh>
#include <tdrp/tdrp.h>
#include "MDV_server_tdrp.h"
using namespace std;

/*
 * DATA INFO NOT AVAILABLE IN FILES - (IMPLIED)
 */

#define VALID_TIME 10         /* seconds that live data remains valid */

#define DATA_ORDER 1     /* 1 = Y increases UP, (right hand)
			  * 0 = Down (left hand) */
#define RADAR_CONST 1.0
#define NOISE_THRESH 5.0

/*
 * global data struct
 */

typedef struct {

  char *prog_name;                  /* the applications name */


  si32 time_last_request;        /* time at which last request for data
				  * was made */

  si32 n_data_requests;          /* number of data requests */

  si32 latest_data_time;

  double data_to_com_delta_x;    /* x diff between the data origin and
				  * the command origin */
  
  double data_to_com_delta_y;    /* y diff between the data origin and
				  * the command origin */
  
  TDRPtable *table;              /* TDRP parsing table */

  MDV_server_tdrp_struct params; /* parameter struct */

} global_t;


/*
 * declare the global structure locally in the main,
 * and as an extern in all other routines
 */

#ifdef MAIN

global_t *Glob = NULL;

#else

extern global_t *Glob;

#endif

/*
 * prototypes
 */

extern int find_best_file(si32 min_time,
			  si32 target_time,
			  si32 max_time,
			  si32 ntop_dir,
			  char **top_dir,
			  char *file_path);

extern void file_search_debug(int dflag);

extern ui08 *get_grid_data(cdata_comm_t *com,
			   cdata_reply_t *reply,
			   cdata_info_t *info,
			   MdvRead &mdv);

extern int get_latest_file(si32 ntop_dir,
			   char **top_dir,
			   char *file_path);

extern void operate(void);

extern void parse_args(int argc,
		       char **argv,
		       int *check_params_p,
		       int *print_params_p,
		       tdrp_override_t *override,
		       char **params_file_path_p);

extern int read_mdv(MdvRead &mdv,
		    cdata_comm_t *com);
     
extern void register_server_init(void);

extern void set_derived_params(void);

extern void tidy_and_exit(int sig);

extern void unregister_server(void);

