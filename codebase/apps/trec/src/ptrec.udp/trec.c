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
/***************************************************************************
  
 * trec.c
 *
 * Reads the moments net, stores data in a buffer, which the
 * algorithm then uses.
 *
 * For each beam, the rp7 header is written, followed by the
 * velocity data and then the dbz data
 *
 * Mike Dixon
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * July 1991
 *
 ****************************************************************************/

#include "trec.h"

int main(int argc, char **argv)

{
  
  /*
   * basic declarations
   */

  char *params_file_path = NULL;
  int check_params;
  int print_params_brief;
  int print_params_full;
  path_parts_t progname_parts;
  tdrp_override_t override;
  
  /*
   * set signal traps
   */

  PORTsignal(SIGINT, tidy_and_exit);
  PORTsignal(SIGTERM, tidy_and_exit);
  PORTsignal(SIGQUIT, tidy_and_exit);

  /*
   * allocate global structure
   */

  Glob = (global_t *)
    umalloc((unsigned) sizeof(global_t));
  
  /*
   * set program name
   */

  uparse_path(argv[0], &progname_parts);
  Glob->prog_name = (char *)
    umalloc ((unsigned) (strlen(progname_parts.base) + 1));
  strcpy(Glob->prog_name, progname_parts.base);

  /*
   * parse the command line arguments
   */

  parse_args(argc, argv,
	     &check_params,
	     &print_params_brief,
	     &print_params_full,
	     &override,
	     &params_file_path);

  /*
   * read in the params
   */
  
  read_params(params_file_path,
	      &override,
	      check_params,
	      print_params_brief,
	      print_params_full);

  /*
   * initialize process registration
   */

  PMU_auto_init(Glob->prog_name, Glob->params.instance,
                PROCMAP_REGISTER_INTERVAL);
  
  /*
   * process the beams
   */
  
  process_beams();

  return(0);

}

