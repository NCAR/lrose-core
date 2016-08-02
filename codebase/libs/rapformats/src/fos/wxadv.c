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
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

/* RCS info
 *   $Author: dixon $
 *   $Locker:  $
 *   $Date: 2016/03/03 18:45:40 $
 *   $Id: wxadv.c,v 1.3 2016/03/03 18:45:40 dixon Exp $
 *   $Revision: 1.3 $
 *   $State: Exp $
 */

/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * wxadv.c: Routines to manipulate general FOS weather advisory data.
 *
 * RAP, NCAR, Boulder CO
 *
 * March 1997
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <toolsa/os_config.h>

#include <toolsa/mem.h>
#include <toolsa/str.h>

#include <rapformats/fos.h>


/*
 * Local constants
 */

#define FIX_START_SIZE      100
#define FIX_INCREMENT        50

#define MIN_FIX_LINE_LEN     46
#define MAX_FIX_LINE_LEN    256


/************************************************************************
 * WXADV_read_fixes() - Read the fixes from a NWS fixes file and return
 *                      them in an array of structures.  These fixes are
 *                      used in describing the polygon vertices in
 *                      AIRMETs and SIGMETs.
 */

WXADV_fix_t *WXADV_read_fixes(char *fixes_file,
			      int *num_fixes)
{
  static char *routine_name = "WXADV_read_fixes";
  
  WXADV_fix_t *fixes = NULL;
  int fixes_alloc = 0;
    
  FILE *infile;
    
  char input_line[MAX_FIX_LINE_LEN];
    
  /*
   * Initialize returned values.
   */

  *num_fixes = 0;
    
  /*
   * Open the input file.
   */

  if ((infile = fopen(fixes_file, "r")) == NULL)
  {
    fprintf(stderr,
	    "ERROR:  rapformats:%s\n", routine_name);
    fprintf(stderr,
	    "Error opening NWS fixes file\n");
    perror(fixes_file);
    
    return(NULL);
  }
  
    
  /*
   * Read the fixes in the file.
   */

  while (fgets(input_line, MAX_FIX_LINE_LEN, infile) != NULL)
  {
    int file_lat, file_lon;
        
    if (strlen(input_line) < MIN_FIX_LINE_LEN)
    {
      fprintf(stderr,
	      "WARNING:  rapformats:%s\n", routine_name);
      fprintf(stderr,
	      "Error processing line of fixes file: %s",
	      input_line);
      continue;
    }

    if (fixes == (WXADV_fix_t *)NULL)
    {
      fixes_alloc = FIX_START_SIZE;
      fixes = (WXADV_fix_t *)umalloc(fixes_alloc *
				     sizeof(WXADV_fix_t));
    }
    else if (*num_fixes == fixes_alloc)
    {
      fixes_alloc += FIX_INCREMENT;
      fixes = (WXADV_fix_t *)urealloc(fixes,
				      fixes_alloc *
				      sizeof(WXADV_fix_t));
    }
        
    STRcopy(fixes[*num_fixes].id, input_line, FIX_ID_LEN);

    file_lat = atoi(&(input_line[35]));
    file_lon = atoi(&(input_line[41]));

    fixes[*num_fixes].lat = (double)(file_lat / 100) +
      ((double)(file_lat % 100) / 60.0);
    fixes[*num_fixes].lon = -((double)(file_lon / 100) +
			      ((double)(file_lon % 100) / 60.0));
            
    (*num_fixes)++;
        
  }
    
  /*
   * Close the input file.
   */

  fclose(infile);
    
  return(fixes);
}
