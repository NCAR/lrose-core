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
/*********************************************************************
 * tstorms_spdb2symprod.h
 *
 * Header file for tstorms_spdb2symprod
 *
 * RAP, NCAR, Boulder CO
 *
 * Sept 1997
 *
 * Mike Dixon
 *
 *********************************************************************/

#include <assert.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <sys/param.h>
#include <sys/types.h>

#include <rapformats/tstorm_spdb.h>
#include <rapformats/tstorm_hull_smooth.h>
#include <toolsa/toolsa_macros.h>
#include <symprod/spdb_products.h>
#include <symprod/symprod.h>
#include <tdrp/tdrp.h>
#include <toolsa/pjg_flat.h>
#include <toolsa/pmu.h>
#include <toolsa/port.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include <SpdbServer/SpdbServer.h>

#include "tstorms_spdb2symprod_tdrp.h"

/**********************************************************************
 * Global variables.
 */

#ifdef MAIN
  SpdbServer *Spdb_server = NULL;
  tstorms_spdb2symprod_tdrp_struct Params;
  char *Prog_name = NULL;
#else
  extern SpdbServer *Spdb_server;
  extern tstorms_spdb2symprod_tdrp_struct Params;
  extern char *Prog_name;
#endif

/**********************************************************************
 * Prototypes
 */

extern void add_current_polygon(symprod_product_t *prod,
				tstorm_spdb_header_t *header,
				tstorm_spdb_entry_t *entry);

extern void add_forecast_polygon(symprod_product_t *prod,
				 tstorm_spdb_header_t *header,
				 tstorm_spdb_entry_t *entry);

extern void add_forecast_vector(symprod_product_t *prod,
				tstorm_spdb_entry_t *entry);
  
extern void add_text(symprod_product_t *prod,
		     tstorm_spdb_entry_t *entry);

extern int convert_capstyle_param(int capstyle);

extern int convert_joinstyle_param(int joinstyle);

extern int convert_line_type_param(int line_type);

extern void *convert_to_symprod(spdb_chunk_ref_t *spdb_hdr,
				void *spdb_data,
				int spdb_len,
				int *symprod_len);

extern void handle_sigpipe(int sig);

extern void init_bbox(symprod_box_t *bb);

extern void parse_args(char *prog_name, int argc, char **argv,
		       int *check_params_p,
		       int *print_params_p,
		       char **params_file_path_p,
		       tdrp_override_t *override);

extern void tidy_and_exit(int sig);

