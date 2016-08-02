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
 * initialize.c
 *
 * Sets up the shared memory areas for realtime data
 *
 * Mike Dixon
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * April 1991
 *
 ****************************************************************************/

#include "cart_slave.h"

void initialize(void)

{

  PMU_auto_register("Initialize()");

  /*
   * read in slava table
   */

  read_slave_table();

  /*
   * initialize output module
   */

  init_output();

  /*
   * set up memory for packet buffer
   */

  Glob->packet = (char *) umalloc(Glob->params.max_packet_length);
  
  /*
   * connect to the service.
   */

  connect_to_server();
  
  /*
   * read in vol params packet - this also sets up the
   * rest of the shared memory areas
   */
  
  if (read_volparams() != CS_SUCCESS) {
    fprintf(stderr, "ERROR - %s:initialize\n", Glob->prog_name);
    fprintf(stderr, "Attempt to read volume params failed.\n");
    tidy_and_exit(-1);
  }

  if (Glob->params.debug) {
    fprintf(stderr, "Got vol params packet\n");
  }
  
  /*
   * read in field params packet
   */

  if (read_field_params() != CS_SUCCESS) {
    fprintf(stderr, "ERROR - %s:initialize.\n", Glob->prog_name);
    fprintf(stderr, "Attempt to read field params failed.\n");
    tidy_and_exit(-1);
  }

  if (Glob->params.debug) {
    fprintf(stderr, "Got field params packet\n");
  }
  
}
