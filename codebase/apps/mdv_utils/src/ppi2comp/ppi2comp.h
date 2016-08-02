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

/************************************************************************
 * ppi2comp.h : header file for ppi2comp program
 *
 * RAP, NCAR, Boulder CO
 *
 * December 1990
 *
 * Mike Dixon
 ************************************************************************/

/*
 **************************** includes *********************************
 */

#include <toolsa/umisc.h>
#include <rapmath/umath.h>
#include <titan/file_io.h>
#include <titan/radar.h>
#include <tdrp/tdrp.h>
#include <toolsa/ldata_info.h>
#include <toolsa/pmu.h>
#include "ppi2comp_tdrp.h"

/*
 ******************************** defines ********************************
 */

#define DBZ_FIELD (si32) 1
#define CLUTTER_TABLE_PATH "clutter_table"

/*
 ******************************* globals struct ******************************
 */

typedef struct {
  char file_path[MAX_PATH_LEN];
  vol_file_handle_t v_handle;
  int valid;
  ui08 *mask;
} ppi_t;

typedef struct {
  
  char *prog_name;                        /* program name */
  TDRPtable *table;                       /* TDRP parsing table */
  ppi2comp_tdrp_struct params;            /* parameter struct */

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
 *********************** function definitions ****************************
 */

extern void create_comp(int nppis, ppi2comp_input *input, ppi_t *ppi);

extern void find_ppi_file(ppi_t *trigger,
			  ppi_t *ppi,
			  ppi2comp_input *input);

extern void parse_args(int argc,
		       char **argv,
		       int *check_params_p,
		       int *print_params_p,
		       char **params_file_path_p,
		       tdrp_override_t *override,
		       si32 *n_input_files_p,
		       char ***input_file_list);

extern void process_ppis(char *input_file_path);

extern void tidy_and_exit(int sig);
     
#ifdef __cplusplus
}
#endif
