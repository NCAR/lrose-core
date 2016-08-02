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
/***********************************************************************
 * client_comm.c
 *
 * Mike Dixon  RAP NCAR Boulder CO USA
 * Orig code by Mark Shorman
 *
 * August 1996
 *
 ************************************************************************/

#include "bprp2gate.h"

void client_comm(bprp_beam_t *beam)

{
  
  static int first_call = TRUE;
  static int proto_fd;
  static int client_fd = -1;
  static int count = 0;

  if (first_call) {

    /*
     * open prototype fd for client side
     */
    
    if ((proto_fd = SKU_open_server(Glob->params.output_port)) < 0) {
      fprintf(stderr, "ERROR - %s:client_comm\n", Glob->prog_name);
      fprintf(stderr, "Cannot open port %ld for output\n",
	      Glob->params.output_port);
      perror("");
      tidy_and_exit(-1);
    }
    
    first_call = FALSE;

  } /* if (first_call) */

  /*
   * if client is not open, try to open
   */
  
  if (client_fd < 0) {
    
    if ((client_fd = SKU_get_client_timed(proto_fd, 0)) < 0) {
      
      /*
       * no client, return
       */
      
      if (Glob->params.debug >= DEBUG_VERBOSE) {
	count++;
	if (count > 20) {
	  fprintf(stderr, "%s: no client yet\n", Glob->prog_name);
	  count = 0;
	}
      }

    }

  } /* if (client_fd < 0) */

  /*
   * process this beam
   */

  if (send_beam(client_fd, beam) == -1) {
    SKU_close(client_fd);
    client_fd = -1;
  }

  return;

}


