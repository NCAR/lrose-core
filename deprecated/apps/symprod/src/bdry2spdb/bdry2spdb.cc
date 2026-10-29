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
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

// SCCS info
//   %W% %D% %T%
//   %F% %E% %U%
//
// RCS info
//   $Author: dixon $
//   $Locker:  $
//   $Date: 2016/03/06 23:31:57 $
//   $Id: bdry2spdb.cc,v 1.14 2016/03/06 23:31:57 dixon Exp $
//   $Revision: 1.14 $
//   $State: Exp $
//

/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * bdry2spdb.cc: Program to convert ASCII boundary files to SPDB format.
 *
 * RAP, NCAR, Boulder CO
 *
 * May 1997
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <toolsa/os_config.h>
#include <dataport/port_types.h>
#include <rapformats/bdry.h>
#include <symprod/spdb_client.h>
#include <symprod/spdb_products.h>
#include <tdrp/tdrp.h>
#include <toolsa/InputDir.hh>
#include <toolsa/mem.h>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "bdry2spdb_tdrp.h"

// Prototypes for static functions

static int parse_args(int argc, char **argv,
		      int *check_params_p,
		      char **params_file_path_p,
		      tdrp_override_t *override);

static void send_spdb_data(SIO_shape_data_t *shape);

static void tidy_and_exit(int sig);


// Global variables

const int Forever = 1;
bdry2spdb_tdrp_struct Params;


/*********************************************************************
 * main()
 */

int main(int argc, char **argv)
{
  // basic declarations

  char *prog_name;
  path_parts_t progname_parts;
  char *params_path_name;
  
  int check_params;
  tdrp_override_t override;
  TDRPtable *table;
  
  InputDir *input_dir;
  
  // set program name

  uparse_path(argv[0], &progname_parts);
  prog_name = STRdup(progname_parts.base);

  // display ucopyright message

  ucopyright(prog_name);

  // parse the command line arguments, and open files as required

  if (parse_args(argc, argv, &check_params,
		 &params_path_name, &override) != 0)
    exit(-1);

  // load up parameters

  table = bdry2spdb_tdrp_init(&Params);
  
  if (params_path_name)
  {
    if (TDRP_read(params_path_name,
		  table, &Params,
		  override.list) == FALSE)
    {
      fprintf(stderr, "ERROR - %s::AppMain\n",
	      prog_name);
      fprintf(stderr, "Cannot read params file '%s'\n",
	      params_path_name);
      exit(-1);
    }
  }
  else
  {
    TDRP_set_defaults(table, &Params);
  }
  
  TDRP_free_override(&override);
  
  if (check_params)
  {
    TDRP_check_is_set(table, &Params);
    TDRP_print_params(table, &Params, prog_name, TRUE);
    exit(-1);
  }
  
  // Set the malloc debug level

  umalloc_debug(Params.malloc_debug_level);
  
  // Initialize PMU usage

  PMU_auto_init(prog_name,
		Params.instance,
		PROCMAP_REGISTER_INTERVAL);
  
  // Create the input directory object

  input_dir = new InputDir(Params.input_dir,
			   Params.input_substring,
			   Params.process_old_data);
  
  // Process boundary files forever

  while (Forever)
  {
    // Register with the process mapper

    PMU_auto_register("Checking for new files");
    
    // Process any new files

    char *input_filename;
    
    while ((input_filename = input_dir->getNextFilename(TRUE)) != NULL)
    {
      char procmap_string[BUFSIZ];
      
      // Register with the process mapper

      sprintf(procmap_string, "Processing file <%s>", input_filename);
      PMU_auto_register(procmap_string);
      
      FILE *input_file;
      
      // Allow the file activity to finish before processing the file

      if (Params.processing_delay > 0)
	sleep(Params.processing_delay);
      
      if (Params.debug || Params.mode == ARCHIVE)
	fprintf(stdout,
		"New data in file <%s>\n", input_filename);

      // Open the input file.

      if ((input_file = fopen(input_filename, "r")) == NULL)
      {
	fprintf(stderr,
		"%s: ERROR: Error opening input file <%s>\n",
		prog_name, input_filename);
	continue;
      }
      
      // Process the data

      SIO_shape_data_t shape;
      int position;
      
      int shape_count = 0;
      
      // Initialize the shape structure

      memset(&shape, 0, sizeof(shape));
      
      SIO_clear_read_buf();
      
      while (SIO_read_record(input_file,
			     &shape,
			     &position) != 0)
      {
	if (Params.debug)
	  SIO_write_data(stdout, &shape, TRUE);
	
	// Send to SPDB destinations

	send_spdb_data(&shape);
	
	// Increment the product count

	shape_count++;
	
      } /* endwhile - strike_kav != NULL */
      
      if (Params.debug)
	fprintf(stdout,
		"Found %d shapes in input file <%s>\n",
		shape_count, input_filename);
      
      // Close the input file.

      fclose(input_file);

      // Free the space used by the file name

      ufree(input_filename);
      
    } /* endwhile - input_filename != NULL */
    
    // If we are in ARCHIVE mode, we are done processing the current
    // files in the directory and so are ready to exit

    if (Params.mode == ARCHIVE)
      break;
    
    // Sleep a little bit

    sleep(Params.sleep_seconds);
    
  } /* endwhile - Forever */
  
  tidy_and_exit(0);
}


/*******************************************************************
 * parse_args() - Parse the command line arguments.
 *
 * Returns 0 if successful and command line is proper, returns -1
 * if the program should exit.
 */

static int parse_args(int argc, char **argv,
		      int *check_params_p,
		      char **params_file_path_p,
		      tdrp_override_t *override)
{
  int error_flag = 0;
  int warning_flag = 0;
  int i;

  char usage[BUFSIZ];
  char tmp_str[BUFSIZ];
  
  // load up usage string

  sprintf(usage, "%s%s%s",
	  "Usage:\n\n", argv[0], " [options] as below:\n\n"
	  "       [ --, -help, -man] produce this list.\n"
	  "       [ -check_params] check parameter usage\n"
	  "       [ -debug ] debugging on\n"
	  "       [ -dir <data_dir>] input data directory\n"
	  "       [ -params name] parameters file name (required)\n"
	  "       [ -print_params] print parameter usage\n"
	  "\n");

  // initialize

  *check_params_p = FALSE;
  TDRP_init_override(override);
  *params_file_path_p = (char *)NULL;
  
  // search for command options
  
  for (i =  1; i < argc; i++)
  {
    if (STRequal_exact(argv[i], "--") ||
	STRequal_exact(argv[i], "-help") ||
	STRequal_exact(argv[i], "-man"))
    {
      printf("%s", usage);
      return(-1);
    }
    else if (STRequal_exact(argv[i], "-check_params") ||
	     STRequal_exact(argv[i], "-print_params"))
    {
      *check_params_p = TRUE;
    }
    else if (STRequal_exact(argv[i], "-debug"))
    {
      sprintf(tmp_str, "debug = true;");
      TDRP_add_override(override, tmp_str);
    }
    else if (i < argc - 1)
    {
      if (STRequal_exact(argv[i], "-dir"))
      {
	sprintf(tmp_str, "input_dir = \"%s\";", argv[i+1]);
	TDRP_add_override(override, tmp_str);
	
	i++;
      }
      else if (STRequal_exact(argv[i], "-params"))
      {
	*params_file_path_p = argv[i+1];

	i++;
      }
    } /* if (i < argc - 1) */
    
  } /* i */

  // print usage if there was an error

  if (error_flag || warning_flag)
  {
    fprintf(stderr, "%s\n", usage);
    fprintf(stderr, "Check the parameters file '%s'.\n\n",
	    *params_file_path_p);
  }

  if (error_flag)
    return(-1);

  return(0);
}


/************************************************************************
 * send_spdb_data()
 */

static void send_spdb_data(SIO_shape_data_t *shape)
{
  int prod_size;
  BDRY_spdb_product_t *prod;
  si32 forecast_period;
  
  // Now convert the shape to the boundary format used in the
  // SPDB database.  Note that bDRY_product_from_SIO_shape()
  // returns a pointer to a static buffer and so that pointer
  // should NOT be freed here.

  prod = BDRY_spdb_product_from_SIO_shape(shape, &prod_size);

  // Print out the boundary buffer for debugging

  if (Params.debug)
    BDRY_print_spdb_product(stdout, prod, TRUE);
  
  spdb_chunk_ref_t chunk_hdr;
  
  // Fill in the chunk header information

  forecast_period = (prod->forecast_time - prod->data_time)/60;

  chunk_hdr.valid_time = prod->data_time;
  if (Params.expire_secs >= 0)
     chunk_hdr.expire_time = prod->data_time + Params.expire_secs;
  else
     chunk_hdr.expire_time = prod->expire_time;
  chunk_hdr.data_type = 
         BDRY_set_data_type(prod->type, prod->subtype, forecast_period);
  chunk_hdr.prod_id = SPDB_BDRY_ID;
  chunk_hdr.offset = 0;
  chunk_hdr.len = prod_size;
 
  // Make sure the boundary data is in big-endian format
  
  BDRY_spdb_product_to_BE(prod);
  
  // Send the data to each of the SPDB destinations

  for (int i = 0; i < Params.spdb_destinations.len; i++)
  {
    SPDB_put_add(Params.spdb_destinations.val[i],
		 SPDB_BDRY_ID,
		 SPDB_BDRY_LABEL,
		 1,
		 &chunk_hdr,
		 (void *)prod,
		 prod_size);
  }
  
}


/************************************************************************
 * tidy_and_exit()
 */

static void tidy_and_exit(int sig)
{
  // verify mallocs

  umalloc_map();
  umalloc_verify();

  // exit with code sig

  exit(sig);
}
