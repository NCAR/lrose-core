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
 * archive_loop.c  (adapted from $RAP_DIR/apps/src/ctrec/main_ctrec.c)
 *
 * Loop for TREC processing of files in a static directory
 * 
 * Terri L. Betancourt
 * 
 * RAP, NCAR, Boulder, CO, USA
 *
 * April 1997
 *                
 *************************************************************************/

#include "trec.h"
#include <dirent.h>

void archive_loop()
{
   int  i, j, n_topdirs, offset, dir_index;
   struct dirent **topList, **dirList;
   char top_path[1024], last_file[1024];
   bool first = true;

   //
   // Figure out which data files to use.
   // Input files can be specified on the command line (-if option) 
   // or via the parameter file (input_dir parameter).
   // If the command line option is used n_input_files and input_file_list
   // are already set for us by parse_args().  Otherwise we scan the 
   // input directory specified in the parameter settings to establish
   // the input_file_list.
   //
   if ( Glob->n_input_files == 0 ) {
      n_topdirs = scandir( Glob->params.input_dir, &topList, 
                           topdir_select, alphasort );
      for( i=0; i < n_topdirs; i++ ) {
         sprintf( top_path, "%s%s%s", 
                  Glob->params.input_dir, PATH_DELIM, topList[i]->d_name );

         Glob->n_input_files = scandir( top_path, &dirList, 
                                        input_select, alphasort );
         if ( Glob->n_input_files == 0 )
            continue;

         //
         // If this is not our first directory, the put the last file
         // processed from the previous directory onto the top of the
         // list to be processed with the current directory
         //
         if ( first ) {
            offset = 0;
         }
         else {
            offset = 1;
         }
         Glob->n_input_files += offset;

         //
         // put the file names in the input name list
         //
         Glob->input_file_list = (char **)umalloc( (Glob->n_input_files) * 
                                                   sizeof(char*) );
         if ( first == false ) {
            Glob->input_file_list[0] = &(last_file[0]);
         }
         
         for( j=offset; j<Glob->n_input_files; j++ ) {
            dir_index = j-offset;
            Glob->input_file_list[j] = (char *)umalloc( 1 +
                                               strlen(top_path) +
                                               strlen(PATH_DELIM) +
                                               strlen(dirList[dir_index]->d_name) );
            sprintf( Glob->input_file_list[j], "%s%s%s", 
                     top_path, PATH_DELIM, dirList[dir_index]->d_name );
         }

         //
         // Loop through the files for a single directory
         //
         do_archive();
         first = false;

         //
         // clean up memory allocations 
         //
         strcpy( last_file, Glob->input_file_list[Glob->n_input_files-1] );
         for( j=offset; j<Glob->n_input_files; j++ ) {
            free( Glob->input_file_list[j] );
         }
         free( dirList );
         free( Glob->input_file_list );
      }
      free( topList );
   }
   else {
      //
      // Loop through the input files specified on the command line
      //
      do_archive();
      free( Glob->input_file_list );
   }
}

void do_archive()
{
   int i, status;
   char output_path[1024], *rptr;
   struct stat fstat;

   //
   // Move through the input files two at a time
   // doing the trec processing
   //
   for( i=0; i < Glob->n_input_files-1; i++ ) {
      //
      // See if we want to skip over this one
      //
      if ( Glob->params.noupdate == TRUE ) {
         strcpy( output_path, Glob->input_file_list[i+1] );
         rptr = rindex( output_path, '/' );
         *rptr = '\0';
         sprintf( output_path, "%s%s%s%s%s",         
                  Glob->params.output_dir, PATH_DELIM, 
                  rindex( output_path, '/' )+1, PATH_DELIM, rptr+1 );
         if ( stat( output_path, &fstat ) == 0 ) {
            //
            // the file exists, don't update it
            //
            continue;
         }
      }

      status = process_files( Glob->input_file_list[i], 
                              Glob->input_file_list[i+1] );
      if ( status == FAILURE ) {
         fprintf( stderr,"%s: Error. Cannot process files '%s' and '%s'\n",
                          Glob->prog_name,
                          Glob->input_file_list[i], Glob->input_file_list[i+1]);
         tidy_and_exit( -1 );
      }
   }
}
