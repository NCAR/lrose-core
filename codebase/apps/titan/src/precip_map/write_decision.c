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
 * write_decision.c
 *
 * Check for file write
 *
 * RAP, NCAR, Boulder CO
 *
 * January 1997
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "precip_map.h"
#include <sys/stat.h>

static int Always_overwrite = FALSE;

void set_overwrite(void)
{
  Always_overwrite = TRUE;
}
  
void clear_overwrite(void)
{
  Always_overwrite = FALSE;
}
  
int check_write(char *file_path)

{

  struct stat file_stat;

  /*
   * if always overwrite flag is set, write file
   */

  if (Always_overwrite) {
    return (TRUE);
  }

  /*
   * if overwrite param is set, write file
   */

  if (Glob->params.overwrite) {
    return (TRUE);
  }

  /*
   * if archive mode, always write file
   */

  if (Glob->params.mode == ARCHIVE) {
    return (TRUE);
  }

  /*
   * if map_type is ACCUM_FROM_START, always write file
   */

  if (Glob->params.map_type == ACCUM_FROM_START) {
    return (TRUE);
  }

  /*
   * if storing files at forecast time, always write file
   */
  
  if (Glob->params.file_time_stamp == FORECAST_TIME) {
    return (TRUE);
  }
  
  /*
   * check for file existence - if it exists, do
   * not rewrite it
   */
  
  if (stat(file_path, &file_stat) == 0) {
    return (FALSE);
  }

  /*
   * no file in existence, so write it
   */

  return (TRUE);

}


