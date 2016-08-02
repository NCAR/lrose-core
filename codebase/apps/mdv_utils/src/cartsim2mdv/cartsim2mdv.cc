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
 * cartsim2mdv.cc
 *
 * Read a cartesian simulation file and make an mdv file
 * Defaults contained in paramdef.cartsim2mdv
 *
 * More or less a hack of what's in ../gint2mdv
 *
 * Dave Albo, RAP, NCAR, Boulder CO, July, 1997.
 *
 *********************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#define MAIN
#include "cartsim2mdv.h"
using namespace std;
#undef MAIN

void signal_trap(int sig);

int main(int argc, char **argv)

{

   path_parts_t progname_parts;	 	        /* structure with path/file name info */
   tdrp_override_t  override;  		        /* override list from inline args */

   int n_input_files=0;                    	/* number files to process-ARCHIVE mode */
   char **input_file_list;                    	/* names of files to process-ARCHIVE mode */
   MDV_SIM_inputs_t in;
   CART_SIM_grid_parms_t G;
   int t;

/*--------------------------------------------------------------------------------------*/

/* the main part of the program! */
   fprintf(stdout,"\n cartsim2mdv starting....\n");

/* allocate global data */
   gd = (global_data_t *) umalloc((unsigned) sizeof(global_data_t));
   ZERO_STRUCT(gd);

/* set program name */
   uparse_path(argv[0], &progname_parts);
   gd->prog_name = (char *)umalloc((unsigned)(strlen(progname_parts.base) + 1));
   strcpy(gd->prog_name, progname_parts.base);

/* display ucopyright message  -- commented out for now */
/*   ucopyright(gd->prog_name);*/

/* parse command line arguments */
   parse_args(argc, argv, &override, &gd->params_file_name, &n_input_files, &input_file_list); 

   /* initialize tdrp parameter table */
   gd->table = cartsim2mdv_tdrp_init(&gd->params);

/* read tdrp parameters */
   if (TDRP_read(gd->params_file_name, gd->table, &gd->params, override.list) == FALSE) {
      fprintf(stderr, "ERROR - %s:main\n", gd->prog_name);
      fprintf(stderr, "TDRP read error. \n");
      exit(-1);
   }  /* endif tdrp_read error */


/* check parameters if requested */
   if (gd->params.check_params) {
      TDRP_check_is_set(gd->table, &gd->params);
   }
  
/* print out parameters if requested */
   if (gd->params.print_params || gd->params.debug) {
      fprintf(stdout, "Parameter list for '%s'\n", gd->prog_name);
      TDRP_print_params(gd->table, &gd->params, gd->prog_name,TRUE);
   }

/* read in the tass simulation parameters */
   CARTSIM_init(argv[0], gd->params.csim_config_file,
		gd->params.grid_config_file,
		(gd->params.print_params || gd->params.debug));
   
/* start the process mapper functions */
   PORTsignal(SIGINT,signal_trap);
   PORTsignal(SIGTERM,signal_trap);
   PORTsignal(SIGPIPE,signal_trap);
 
   /* Initialize tha Process Mapper Functions */
   /*PMU_auto_init(gd->prog_name,gd->params.instance,
     PROCMAP_REGISTER_INTERVAL);*/
 
   /*
    * Begin to create simulated data
    */
   for (t=0; t<= gd->params.maximum_time; t += gd->params.delta_time)
   {
       /*
	* Get the params for this time
	*/
       CARTSIM_get_params(t, gd->params.delta_time,
			  gd->params.vertical_type,
			  gd->params.projection_type,
			  gd->params.fields.len,
			  gd->params.fields.val, &in, &G);

       /*
	* Process using them
	*/
       fprintf(stdout,"Processing time %d\n", t);
       if (process(&in, &G) == MDV_FAILURE)
	   fprintf(stderr,"\nError processing\n");
   }

   /* clean up tdrp stuff */
   tdrpFree(gd->table);

   /*PMU_auto_unregister();*/

   fprintf(stdout,"\ncartsim2mdv finished\n");
   exit (0);
}

/*********************************************************************
 * SIGNAL_TRAP : Traps Signals so as to die gracefully
 *
 */

void signal_trap(int sig)
 {
         fprintf(stderr,"Caught Signal %d\n",sig);
         PMU_auto_unregister();
         exit(0);
 }
 
#ifdef __cplusplus
}
#endif
