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
 * tprod_print.c
 *
 * Print out the track product data stream.
 *
 * Mike Dixon  RAP NCAR Boulder CO USA
 *
 * March 1997
 *
 **********************************************************************/

#define MAIN
#include "tprod_print.h"
#undef MAIN

int main(int argc, char **argv)

{

  char *data_buf;
  int forever = TRUE;
  int server_fd;
  long data_len;
  SKU_header_t header; 
  path_parts_t progname_parts;

  /*
   * allocate space for the global structure
   */
  
  Glob = (global_t *)
    umalloc((ui32) sizeof(global_t));

  /*
   * set program name
   */
  
  uparse_path(argv[0], &progname_parts);
  Glob->prog_name = (char *)
    umalloc ((ui32) (strlen(progname_parts.base) + 1));
  strcpy(Glob->prog_name, progname_parts.base);
  
  /*
   * display ucopyright message
   */

  ucopyright(Glob->prog_name);

  /*
   * parse command line arguments
   */

  parse_args(argc, argv);

  /*
   * open the socket to the server
   */
  
  if ((server_fd = SKU_open_client(Glob->host, Glob->port)) < 0) {
    fprintf(stderr, "ERROR - %s\n", Glob->prog_name);
    fprintf(stderr, "Cannot open connection to server.\n");
    fprintf(stderr, "Hostame, port : %s, %d\n", Glob->host, Glob->port);
    tidy_and_exit(-1);
  }

  while (forever) {

    if (SKU_read_message(server_fd, &header,
			 &data_buf, &data_len, -1) < 0) {
      fprintf(stderr, "ERROR - %s\n", Glob->prog_name);
      fprintf(stderr, "Reading message from server.\n");
      tidy_and_exit(-1);
    }

    if (Glob->debug) {
      fprintf(stderr, "Read message, data_len %ld\n", data_len);
    }

    do_print(data_buf);
    
  }
    
  /*
   * quit
   */
  
  tidy_and_exit(0);

  return(0);

}

