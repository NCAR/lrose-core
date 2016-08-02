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
 * decode_header()
 *
 * decode the kavouras header
 *
 * RAP, NCAR, Boulder CO
 *
 * December 1994
 *
 * Mike Dixon
 *
 * Moved to the rapformats library by N. Rehak, January 1997
 *********************************************************************/

#include <toolsa/os_config.h>
#include <rapformats/kav_grid.h>
#include <rapformats/km.h>
#include <toolsa/globals.h>
#include <toolsa/umisc.h>


static char *months[12] = { "JAN", "FEB", "MAR", "APR", "MAY", "JUN",
			    "JUL", "AUG", "SEP", "OCT", "NOV", "DEC" };

static int ConvertInt16(char *ptr);

/************************************************************************
 * KM_decode_header()
 */

void KM_decode_header(char *file_buf,
		      int file_size,
		      KM_header_t *header,
		      int debug_level)
{
  int  i;
  int month_found;
  int year;
  
  /*
   * convert header to native structure
   */
  
  memset((void *) header, 0, sizeof(KM_header_t));
  
#ifndef NEW_DBS_HEADER
  
  memcpy((void *) header->day, (void *) (file_buf), 2);
  memcpy((void *) header->mon, (void *) (file_buf+2), 3);
  memcpy((void *) header->year, (void *) (file_buf+5), 2);
  
  memcpy((void *) header->hour, (void *) (file_buf+8), 2);
  memcpy((void *) header->min, (void *) (file_buf+10), 2);
  
  /* get site info */
  
  header->nsites = MAX_SITES;
  
  for (i = 0; i < header->nsites; i++)
  {
    memcpy((void *) &(header->site_id[i][0]),
	   (void *) (file_buf + (13 + i*3)), 3);
    header->site_x[i] = ConvertInt16((char *) (file_buf + (397 + i*2)));
    header->site_y[i] = ConvertInt16((char *) (file_buf + (653 + i*2)));
    header->site_status[i] =
      ConvertInt16((char *) (file_buf + (909 + i*2)));
    
    if (debug_level >= KM_DEBUG_EXTRA)
    {
      fprintf(stderr, "%3d %3s %6d%6d %3d\n",
	      i, header->site_id[i], header->site_x[i],
	      header->site_y[i], header->site_status[i]);
    }
      
  }
  
#else
  
  memcpy((void *) header->day, (void *) (file_buf+18), 2);
  memcpy((void *) header->mon, (void *) (file_buf+20), 2);
  memcpy((void *) header->year, (void *) (file_buf+22), 2);
  
  memcpy((void *) header->hour, (void *) (file_buf+12), 2);
  memcpy((void *) header->min, (void *) (file_buf+14), 2);
  
  memcpy((void *) header->filename, (void *) (file_buf), 12);
  memcpy((void *) header->validtime, (void *) (file_buf+12), 4);
  memcpy((void *) header->validdate, (void *) (file_buf+18), 6);
  
  /* There is an inconsistancy in the docs for this 2 bytes vs 4 bytes */
  memcpy((void *) header->projection, (void *) (file_buf+24), 2);
  
  header->lat0 = (double) ConvertInt16(file_buf + 26) *.01;
  header->lon0 = (double) ConvertInt16(file_buf + 28) *.01;
  header->lon_width = (double) ConvertInt16(file_buf + 30) *.01;
  header->aspect = (double) ConvertInt16(file_buf + 32) *.0001;
  header->nx = ConvertInt16(file_buf + 34);
  header->ny = ConvertInt16(file_buf + 36);
  
  /* get site info */
  header->nsites = ConvertInt16((char *) (file_buf + 44));
  for (i = 0; i < header->nsites; i++)
  {
    memcpy((void *) &(header->site_id[i][0]),
	   (void *) (file_buf + (46 + i*3)), 3);
    header->site_x[i] = ConvertInt16((char *) (file_buf + (646 + i*2)));
    header->site_y[i] = ConvertInt16((char *) (file_buf + (1046 + i*2)));
    header->site_status[i] = ConvertInt16((char *) (file_buf + (1446 + i*2)));
    if (debug_level >= KM_DEBUG_EXTRA)
    {
      fprintf(stderr, "%3d %3s %6d%6d %3d\n",
	      i, header->site_id[i], header->site_x[i],
	      header->site_y[i], header->site_status[i]);
    }
  }

  if (debug_level >= KM_DEBUG_EXTRA)
  {
    double lat_width  = header->lon_width * header->aspect;
    printf("header for %s\n", header->filename);
    printf("  valid %s %s\n", header->validtime, header->validdate);
    printf("  projection %s\n", header->projection);
    fprintf(stderr, "  lat %6.2f lon %6.2f lon_width = %6.2f", 
	    header->lat0,  header->lon0, header->lon_width);
    fprintf(stderr, "  lat_width = %6.2f aspect = %8.4f",
	    lat_width,  header->aspect);
    fprintf(stderr, "   nx = %d ny = %d\n",
	    header->nx, header->ny);
    fprintf(stderr, "  lat %6.2f to %6.2f delta = %10.6f", 
	    header->lat0 -lat_width/2,header->lat0 + lat_width/2, 
	    lat_width / header->ny);
    fprintf(stderr, "  lon %6.2f to %6.2f delta = %10.6f", 
	    header->lon0 - header->lon_width/2,
	    header->lon0 + header->lon_width/2, 
	    header->lon_width / header->nx);
  }
  
#endif

  /*
   * Extract Time Information
   */

  year = atoi(header->year);
  
  if (year > 90) {
    header->time.year = 1900 + year;
  } else {
    header->time.year = 2000 + year;
  }

#ifndef NEW_DBS_HEADER

  month_found = FALSE;
  for (i = 0; i < 12; i++)
  {
    if (strcmp(header->mon, months[i]) == 0) {
      header->time.month = i + 1;
      month_found = TRUE;
      break;
    }
  }
  
  if (!month_found)
  {
    fprintf(stderr, "cant decode month %s", header->mon);
    header->time.month = 1;
  }

#else

  header->time.month = atoi(header->month);

#endif

  header->time.day = atoi(header->day);
  header->time.hour = atoi(header->hour);
  header->time.min = atoi(header->min);
  header->time.sec = 0;
  uconvert_to_utime(&header->time);
  
  if (debug_level >= KM_DEBUG_NORM)
  {
    fprintf(stderr, "Valid time: %s\n", utimestr(&header->time));
  }

  return;

}

/************************************************************************
 * ConvertInt16()
 */

static int ConvertInt16(char *ptr)
{
  static char temp[2],*tmp=temp; /* These are needed to avoid bus errors */

  si16 old;

  temp[0] = *ptr;
  temp[1] = *(ptr+1);
  old = *((si16 *) tmp);
  
  return((int) old);
  
}

