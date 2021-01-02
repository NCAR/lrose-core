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
/****************************************************************************
 * read_volume.c
 *
 * Reads in the relevant original radar volume
 *
 * Mike Dixon  RAP NCAR Boulder CO USA
 *
 * October 1991
 *
 ****************************************************************************/

#include "grid_forecast.h"

void read_volume(vol_file_handle_t *v_handle,
		 date_time_t *stime)

{


  char file_path[MAX_PATH_LEN];

  /*
   * compute the volume file path
   */

  sprintf(file_path, "%s%s%.4d%.2d%.2d%s%.2d%.2d%.2d.%s",
	  Glob->original_rdata_dir, PATH_DELIM,
	  stime->year, stime->month, stime->day,
	  PATH_DELIM,
	  stime->hour, stime->min, stime->sec,
	  Glob->dobson_file_ext);

  v_handle->vol_file_path = file_path;

  /*
   * read in file
   */

  if (RfReadVolume(v_handle, "read_volume") != R_SUCCESS)
    tidy_and_exit(-1);

}
