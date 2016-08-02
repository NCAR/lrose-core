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
/****************************************************************************
 * real_loop.c  (adapted from $RAP_DIR/apps/src/ctrec/main_ctrec.c)
 * 
 * Loop for TREC processing of directory which is being continuously updated
 * with input data file
 *
 * Terri L. Betancourt
 * 
 * RAP, NCAR, Boulder, CO, USA
 *
 * April 1997
 *                
 *************************************************************************/

#include "trec.h"

void real_loop()
{
   int  status;
   char first_file_name[NAMESIZE];
   char latest_file_name[NAMESIZE];
   long first_data_time, latest_data_time, last_time_processed=0;


   if ( Glob->params.debug ) {
      printf( "%s: Begin realTime loop.\n", Glob->prog_name );
   }

   while (TRUE) {
 
      /* register with process mapper */
      if ( Glob->params.debug ) 
         printf("%s: Checking '%s' for latest mdv files.\n", 
                 Glob->prog_name, Glob->params.input_dir );
      PMU_auto_register( "Checking for latest mdv files" );
 
      /* get data times of two most recent images */
      if ((get_data_times(Glob->params.input_dir,
                              &first_data_time,
                              &latest_data_time)) != SUCCESS) {
          fprintf( stderr, "%s: Warning. Cannot get data times from '%s'.\n",
                            Glob->prog_name, Glob->params.input_dir );

          /* skip processing and try again*/
          latest_data_time = last_time_processed;
       }

      /* make sure images haven't already been processed 
       * and that they are close enough */
      if (latest_data_time != last_time_processed) {

         if ( latest_data_time - first_data_time <  
              Glob->params.max_time_between_images ) {

            /* create file names */
            if ( create_mdv_input_fname( first_data_time, 
                                         first_file_name ) == NULL ||
                 create_mdv_input_fname( latest_data_time,
                                         latest_file_name ) == NULL ) {
               fprintf( stderr,"%s: Error. Cannot create mdv file names.\n",
                                Glob->prog_name );
               tidy_and_exit( -1 );
            }

            if (Glob->params.debug)  {
               printf( "%s: Calling process_files with files '%s' and '%s'\n",
                        Glob->prog_name, first_file_name, latest_file_name );
            }

            /* force process registration */
            PMU_force_register("Calling process_files");
 
            status = process_files(first_file_name,latest_file_name);
            if ( status == FAILURE ) {
               fprintf( stderr,"%s: Error. Cannot process files '%s' and '%s'\n",
                        Glob->prog_name, first_file_name, latest_file_name );
               tidy_and_exit( -1 );
            }

            PMU_force_register( "Finished Processing images." );

         } /* endif two sucessive images were processed */

         /* update current_index_file */
         if (update_cindex(latest_data_time) != SUCCESS) {
            tidy_and_exit( -1 );
         }

         if (Glob->params.debug) 
            printf( "%s: process_files completed.\n", Glob->prog_name );

         /* switch around file names so last becomes first */
         last_time_processed = latest_data_time;
            
      } /* endif two images are different */

      else {
         /* hang out a little waiting for new data to come in */
         sleep( 5 );
      }
   }
}
