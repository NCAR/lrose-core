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
 * set_header_time.c
 *
 * Set the Kavouras header time to the given time.
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


static char *months[12] = { "JAN", "FEB", "MAR", "APR", "MAY", "JUN",
			    "JUL", "AUG", "SEP", "OCT", "NOV", "DEC" };

/************************************************************************
 * KM_set_header_time(): Set the Kavouras header time fields to the
 *                       given time.
 */

void KM_set_header_time(time_t new_time,
			KM_header_t *header)
{
  char temp_year[5];
  
  header->time.unix_time = new_time;
    
  uconvert_from_utime(&header->time);
    
  sprintf(header->day, "%02d", header->time.day);
  sprintf(header->mon, "%s", months[header->time.month - 1]);
  sprintf(temp_year, "%04d", header->time.year);
  header->year[0] = temp_year[2];
  header->year[1] = temp_year[3];
  header->year[2] = '\0';
    
  sprintf(header->hour, "%02d", header->time.hour);
  sprintf(header->min, "%02d", header->time.min);
  
  sprintf(header->validdate, "%02d", header->time.month);
  header->validdate[2] = header->day[0];
  header->validdate[3] = header->day[1];
  header->validdate[4] = header->year[0];
  header->validdate[5] = header->year[1];
  header->validdate[6] = '\0';
  
  header->validtime[0] = header->hour[0];
  header->validtime[1] = header->hour[1];
  header->validtime[2] = header->min[0];
  header->validtime[3] = header->min[1];
  header->validtime[4] = '\0';
  
  return;
}

