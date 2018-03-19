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
 * fix_header_time.c
 *
 * Fix the time in the Kavouras header so that the date makes it no more
 * than 12 hours from the file time.  This is needed because we sometimes
 * receive Kavouras files with the wrong date in the header right after
 * 0000Z.
 *
 * Nancy Rehak
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * March 1997
 *
 ****************************************************************************/

#include <stdio.h>

#include <toolsa/os_config.h>
#include <rapformats/km.h>
#include <toolsa/udatetime.h>
#include <toolsa/umisc.h>


/************************************************************************
 * KM_fix_header_time(): Reset the header time so that it is within 12
 *                       hours of the file time.
 */

void KM_fix_header_time(time_t file_time,
			KM_header_t *header)
{
  static char *routine_name = "KM_fix_header_time";
  
  int offset = 0;
  
  /*
   * If the header time is more than 12 hours off from the file
   * time, assume there is a problem and fix it.
   */

  if (labs(file_time - header->time.unix_time) > SECS_IN_12HRS)
  {
    /*
     * Change the day of the header time until it is within 12 hours
     * of the file time.
     */

    if (file_time > header->time.unix_time)
    {
      while (file_time - header->time.unix_time > SECS_IN_12HRS)
      {
	header->time.unix_time += SECS_IN_DAY;
	offset += SECS_IN_DAY;
      }
    }
    else
    {
      while (header->time.unix_time - file_time > SECS_IN_12HRS)
      {
	header->time.unix_time -= SECS_IN_DAY;
	offset -= SECS_IN_DAY;
      }
    }
    
    KM_set_header_time(header->time.unix_time,
		       header);
    
    fprintf(stderr,
	    "WARNING: %s: Kavouras header time looks wrong.  Fixing it (offset = %d.\n",
	    routine_name, offset);
    
  } /* endif - error in header time */
  
  return;
}

