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
#ifndef CARTSIM2MDV_H
#define CAETSIM2MDV_H 

#ifdef __cplusplus
 extern "C" {
#endif

#include <sim/mdv_sim.h>

#include "cartsim2mdv_tdrp.h"
#include "loc_cart_sim.h"

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

#define FILL_SUCCESS 0
#define FILL_FAILURE -1

#define MDV_ENCODE_KEY 255


/*  global data */

typedef struct {

  char *params_file_name;		/* name of alternate  parameter file */
  TDRPtable  *table;			/* TDRP parsing table */
  cartsim2mdv_tdrp_struct params;        /* TDRP parameter structure */
  char *prog_name;                  	/* program name */

} global_data_t;

#ifdef MAIN

global_data_t *gd = NULL;

#else

extern global_data_t *gd;

#endif

/* forward declarations */
 
int process_args(int argc, char *argv[]);
int process(MDV_SIM_inputs_t *in, CART_SIM_grid_parms_t *G);
extern void parse_args(int argc, char **argv,
		       tdrp_override_t *override,
		       char **params_file_name_p,
		       int *n_input_files,
		       char ***input_file_list);

#ifdef __cplusplus
}
#endif

#endif

