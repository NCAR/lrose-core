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

/**************************************************************************
 * mdv_update_data.h - header file for mdv_update_data program
 *
 * Mike Dixon RAP NCAR August 1990
 *
 **************************************************************************/

#include <dataport/port_types.h>
#include <tdrp/tdrp.h>

#include "mdv_update_data_tdrp.h"

/*
 * global struct
 */

typedef struct
{
  char *prog_name;                    /* program name */
  TDRPtable *table;                   /* TDRP parsing table */
  mdv_update_data_tdrp_struct params; /* parameter struct */

  int mdv_data_type;                  /* Uncompressed type of data in
				       * input file.  MDV_INT8 etc. */
  int mdv_output_type;                /* Type of data to write to 
				       * output file. */
  
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
 * functions
 */

#ifdef __cplusplus
 extern "C" {
#endif

extern void convert_params(void);
   
extern void parse_args(int argc,
		       char **argv,
		       int *check_params,
		       int *print_params,
		       char **params_file_path_p,
		       tdrp_override_t *override,
		       si32 *n_input_files_p,
		       char ***input_file_list);

extern void tidy_and_exit(int sig);

extern void update_file(char *input_file_path);

#ifdef __cplusplus
}
#endif
