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
/************************************************************************
 * parse_threshold.c
 *
 * Parse the reflectivity threshold from the note in the volume
 * file
 *
 * Mike Dixon  RAP NCAR Boulder CO USA
 *
 * August 1995
 *
 ***********************************************************************/

#include "verify_grid.h"
#include <ctype.h>

double parse_threshold(vol_file_handle_t *v_handle)

{

  int i;
  char *start, *end;
  char note[VOL_PARAMS_NOTE_LEN];
  double threshold;

  memcpy(note, v_handle->vol_params->note, VOL_PARAMS_NOTE_LEN);
  for(i = 0; i < VOL_PARAMS_NOTE_LEN; i++) {
    note[i] = tolower(note[i]);
  }

  /*
   * look through note for refl threshold
   */

  start = strstr(note, "refl threshold");
  if (start == NULL) {
    start = strstr(note, "dbz threshold");
  }

  if (start == NULL) {
    return (-1.0);
  }

  end = strstr(start, "\n");
  if (end != NULL) {
    *end = '\0';
  } else {
    end = start + strlen(start);
  }

  while (!isdigit(*start) && start < end) {
    start++;
  }

  if (sscanf(start, "%lg", &threshold) == 1) {
    return (threshold);
  } else {
    return (-1.0);
  }

}
