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
 * Jan 1995
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "read_bprp_tape.h"

int parse_args(int argc, char **argv,
	       char **tape_device_p,
	       int *port_p,
	       int *swap_p,
	       int *old_tape_p,
	       int *wait_msecs_p)
     
{
  
  int error_flag = 0;
  int i;
  
  char usage[BUFSIZ];
  
  /*
   * set defaults
   */
  
  *tape_device_p = getenv("TAPE");
  *port_p = 1600;
  *swap_p = FALSE;
  *old_tape_p = FALSE;
  *wait_msecs_p = 0;
  
  /*
   * set usage
   */
  
  sprintf(usage, "%s%s%s%s",
	  "Usage: ", argv[0], "\n",
	  "       [-h, -help, --, -man] produce this list.\n"
	  "       [-f tape_device] set tape device (defaults to $TAPE)\n"
	  "       [-old_tape] old Nelspruit tape, raycount is different\n"
	  "       [-p port] set port (defaults to 1600)\n"
	  "       [-swap] swap bytes\n"
	  "       [-wait ?] wait between beams (msecs)\n"
	  "\n");
  
  /*
   * look for command options
   */
  
  for (i =  1; i < argc; i++) {
    
    if (!strcmp(argv[i], "--") ||
	!strcmp(argv[i], "-h") ||
	!strcmp(argv[i], "-help") ||
	!strcmp(argv[i], "-man")) {
      
      fprintf(stderr, "%s", usage);
      return (-1);
      
    } else if (!strcmp(argv[i], "-f")) {
      
      if (i < argc - 1)
	*tape_device_p = argv[i+1];
      else
	error_flag = TRUE;
      
    } else if (!strcmp(argv[i], "-old_tape")) {

      *old_tape_p = TRUE;
      
    } else if (!strcmp(argv[i], "-p")) {
      
      if (i < argc - 1)
	*port_p = atol(argv[i+1]);
      else
	error_flag = TRUE;
      
    } else if (!strcmp(argv[i], "-swap")) {

      *swap_p = TRUE;
      
    } else if (!strcmp(argv[i], "-wait")) {
      
      if (i < argc - 1)
	*wait_msecs_p = atol(argv[i+1]);
      else
	error_flag = TRUE;
      
    }
    
  }
  
  /*
   * print message if error flag set
   */

  if(error_flag) {
    fprintf(stderr, "%s", usage);
    return(-1);
  }

  return (0);

}
