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
 * October 1990
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "cart_convert.h"

void parse_args(int argc, char **argv)
{

  int error_flag = FALSE;
  int warning_flag = FALSE;
  int i;
  char usage[BUFSIZ];

  /*
   * check the # of args
   */

  sprintf(usage, "%s%s%s%s%s%s%s%s%s",
	  "Usage:\n\n", argv[0], " [options] filename\n\n",
	  "options:\n",
	  "         [ --, -help, -man, -h] produce this list\n",
          "         [ -noparams ] use no parameters file\n",
          "         [ -params path] parameters file path\n",
	  "         [ -rctable path] lookup table file path\n",
	  "\n");

  if (argc < 2) {
    fprintf(stderr, "%s", usage);
  }

  /*
   * get defaults
   */

  Glob->rc_table_path =
    uGetParamString(Glob->prog_name, "rc_table", RC_TABLE);
  
  /*
   * set file name
   */

  Glob->rdata_path = argv[argc - 1];

  /*
   * search for command options
   */

  for (i =  1; i < argc; i++) {

    if (!strcmp(argv[i], "-help") ||
	!strcmp(argv[i], "-h") ||
	!strcmp(argv[i], "-man") ||
	!strcmp(argv[i], "--")) {

      fprintf(stderr, "%s", usage);
      exit(0);

    } else if (!strcmp(argv[i], "-rctable")) {

      if (i < argc - 1)
	Glob->rc_table_path = argv[i+1];
      else
	error_flag = TRUE;

    }

  } /* i */

  /*
   * check for errors or warnings
   */

  if(error_flag || warning_flag) {
    fprintf(stderr, "%s\n", usage);
    fprintf(stderr, "Check the parameters file '%s'.\n\n",
	    Glob->params_path_name);
  }

  if (error_flag)
    exit(1);

}
