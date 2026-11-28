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
 * gint2mdv.cc
 *
 * Read a gint file and make an mdv file
 * Defaults contained in paramdef.gint2mdv
 *
 * Rachel Ames, RAP, NCAR, Boulder CO, January, 1996.
 *
 *********************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#define MAIN
#include "gint2mdv.h"
using namespace std;
#undef MAIN


void signal_trap(int sig);

int main(int argc, char **argv)

{

   path_parts_t progname_parts;	 	        /* structure with path/file name info */
   tdrp_override_t  override;  		        /* override list from inline args */

   int n_input_files=0;                    	/* number files to process-ARCHIVE mode */
   char **input_file_list;                    	/* names of files to process-ARCHIVE mode */
   char *input_file_name;                    	/* name of file to process-REALTIME mode */

   int ifile;   	                  	/* loop variable */

   long latest_data_time;                       /* unix time of most recent data */
   
   char status_str[BUFSIZE];         		/* PMU status */

/*--------------------------------------------------------------------------------------*/

/* the main part of the program! */
   fprintf(stdout,"\n gint2mdv starting....\n");

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
   gd->table = gint2mdv_tdrp_init(&gd->params);

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

/* ARCHIVE vs. REALTIME mode */
   if (gd->params.mode == ARCHIVE && n_input_files == 0) {
      fprintf(stderr, "Error in %s'\n", gd->prog_name);
      fprintf(stderr,"\nFor ARCHVIE mode user must specify input file list.\n");
      exit(-1);
   }

/* start the process mapper functions */
   PORTsignal(SIGINT,signal_trap);
   PORTsignal(SIGTERM,signal_trap);
   PORTsignal(SIGPIPE,signal_trap);
 
   /* Initialize tha Process Mapper Functions */
   PMU_auto_init(gd->prog_name,gd->params.instance,PROCMAP_REGISTER_INTERVAL);
 
   if (gd->params.mode == ARCHIVE) {
       for (ifile = 0; ifile < n_input_files; ifile++) {
          if ((process_file(input_file_list[ifile]) == MDV_FAILURE)) {
             fprintf(stderr,"\nError processing file %s",input_file_list[ifile]);
          } 
       }
   }
   else {

      sprintf(status_str,"Checking %s",gd->params.input_dir);
      fprintf(stdout,"Checking %s",gd->params.input_dir);
      PMU_auto_register(status_str);

      while (TRUE) {

         latest_data_time = get_latest_data_time(gd->params.input_dir,
                            gd->prog_name,
                            gd->params.max_input_data_age,
                            DEBUG_INDEX);
  
         if ((input_file_name = create_gint_file_name(latest_data_time)) == NULL) {
            fprintf(stderr,"\nError creating gint file name");
            PMU_auto_unregister();
            exit(-1);
         }

         sprintf(status_str,"Converting gint file");
         PMU_auto_register(status_str);
         if ((process_file(input_file_name) == MDV_FAILURE)) {
            fprintf(stderr,"\nError processing file %s",input_file_name);
         } 
         free(input_file_name);
         input_file_name = NULL;
      }
   }

   /* clean up tdrp stuff */
   tdrpFree(gd->table);

   PMU_auto_unregister();
   fprintf(stdout,"\ngint2mdv finished\n");
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
