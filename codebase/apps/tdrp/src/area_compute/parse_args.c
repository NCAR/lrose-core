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
 * parse_args.c: parse command line args
 *
 * RAP, NCAR, Boulder CO
 *
 * Sept 1998
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "area_compute.h"

static void usage(char *prog_name, FILE *out)

{

  fprintf(out, "%s%s%s%s",
	  "Usage: ", prog_name, " [options as below]\n",
	  "   [ -h] produce this list.\n"
	  "   [ -debug ] print debug messages\n"
	  "   [ -output ?] set output_path\n"
	  "   [ -size ?] set shape size\n"
	  "   [ -shape ?] set shape type\n"
	  "      options are SQUARE, CIRCLE and EQ_TRIANGLE\n");
  TDRP_usage(out);
  
}

void parse_args(int argc,
		char **argv,
		char *prog_name,
                tdrp_override_t *override)

{

  int error_flag = 0;
  int i;
  char tmp_str[BUFSIZ];
  
  /*
   * look for command options
   */

  for (i =  1; i < argc; i++) {

    if (!strcmp(argv[i], "-h")) {
  
      usage(prog_name, stdout);
      exit(0);
      
    } else if (!strcmp(argv[i], "-debug")) {
      
      sprintf(tmp_str, "debug = TRUE;");
      TDRP_add_override(override, tmp_str);
      
    } else if (!strcmp(argv[i], "-size")) {
      
      if (i < argc - 1) {
        sprintf(tmp_str, "size = %s;", argv[++i]);
	TDRP_add_override(override, tmp_str);
      } else {
	error_flag = TRUE;
      }
	
    } else if (!strcmp(argv[i], "-shape")) {
      
      if (i < argc - 1) {
        sprintf(tmp_str, "shape = %s;", argv[++i]);
	TDRP_add_override(override, tmp_str);
      } else {
	error_flag = TRUE;
      }
	
    } else if (!strcmp(argv[i], "-output")) {
      
      if (i < argc - 1) {
        sprintf(tmp_str, "output_path = %s;", argv[++i]);
	TDRP_add_override(override, tmp_str);
      } else {
	error_flag = TRUE;
      }
	
    } /* if */
    
  } /* i */
  
  /*
   * print message if error flag set
   */

  if(error_flag) {
    usage(prog_name, stderr);
    exit(-1);
  }

  return;

}


