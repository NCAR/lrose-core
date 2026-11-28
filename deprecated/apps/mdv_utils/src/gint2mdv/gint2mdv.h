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
#ifdef __cplusplus
 extern "C" {
#endif

#include <toolsa/os_config.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <memory.h>
#include <toolsa/umisc.h>
#include <rapformats/gint_user.h>
#include <dataport/swap.h>
#include <toolsa/file_io.h>
#include <toolsa/pmu.h>
#include <mdv/mdv_file.h>
#include <mdv/mdv_macros.h>
#include <mdv/mdv_user.h>
#include <mdv/mdv_print.h>
#include <mdv/mdv_utils.h>
#include <sys/file.h>
#include <sys/time.h>
#include <tdrp/tdrp.h>

#if defined (__linux)
#else
#include <sys/param.h>
#endif

#include "gint2mdv_tdrp.h"

#define BUFSIZE 1024

#define ZERO_STRUCT(p) bzero((char*)(p),sizeof(*(p)))

#ifndef TRUE 
#define TRUE 1
#endif
#ifndef FALSE 
#define FALSE 0
#endif

#define DEBUG_INDEX FALSE

#define MAX_FIELDS 96

#define METERS2KM .001

#define FILL_SUCCESS 0
#define FILL_FAILURE -1

#define MDV_ENCODE_KEY 255


/*  global data */

typedef struct {

  char *params_file_name;		/* name of alternate  parameter file */
  TDRPtable  *table;			/* TDRP parsing table */
  gint2mdv_tdrp_struct params;	        /* TDRP parameter structure */
  char *prog_name;                  	/* program name */

} global_data_t;

#ifdef MAIN

global_data_t *gd = NULL;

#else

extern global_data_t *gd;

#endif

/* forward declarations */
 
int process_args(int argc, char *argv[]);

int fill_mdv_master_header(Tvolume_header *gh, MDV_master_header_t *mmh);

int fill_mdv_field_header(Tvolume_header *gh, 
                          MDV_master_header_t *mmh,
                          MDV_field_header_t *mfh,
                          int ifield);

int fill_mdv_vlevel_header(Tvolume_header *gh, 
                          MDV_master_header_t mmh,
                          MDV_vlevel_header_t *mvh);

int fill_mdv_chunk_header(MDV_chunk_header_t *mch);

char * create_mdv_file_name (MDV_master_header_t *mmh);

char * create_gint_file_name (long latest_data_time);

extern void parse_args(int argc, char **argv,
                tdrp_override_t *override,
                char **params_file_name_p,
                int *n_input_files,
                char ***input_file_list);

int process_file(char *input_file_name);

int update_cindex(long file_time);

long get_latest_data_time(char *rdata_dir,
			  char *prog_name,
			  long max_valid_data_age,
			  int debug);


#ifdef __cplusplus
}
#endif
