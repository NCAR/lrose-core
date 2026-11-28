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
#include <stdio.h>
#include <time.h>

#define LINE_LENGTH 22
#define CCB_HEADER 22
#define UNIX_TIME_DIFF 320630400
#define LON_CONST 0.001068115234
#define LAT_CONST 0.001281738

main (argc, argv)
int argc;
char **argv;
{
FILE           *source_fd, 
               *target_fd;
char           data_buffer[2], 
               buffer[LINE_LENGTH];
int            x = 0, 
               time_stamp = 0,
               flash_message = 0,
               multiplicity, 
               duration, 
               kiloamps, 
               quality_flag;
float          latitude, 
               longitude;
unsigned short date[4], 
               temp_short = 0, 
               second_short = 0,
               days = 0, 
               hours, 
               mins, 
               secs;
unsigned long  time = 0,
               calc_time,
               position = 0;
  if (argc != 3)
  {
    printf ("Usage: %s\n",argv[0]);
    printf ("%s <source filename> <target filename>\n",argv[0]);
    printf ("Include the path if not '.'\n");
    exit (1);
  }
  if ((source_fd = fopen(argv[1], "r")) == NULL)
  {
    printf ("Could not open input file %s\n",argv[2]);
    exit (1);
  }
  if ((target_fd = fopen(argv[2], "w")) == NULL)
  {
    printf ("Could not open output file %s\n",argv[2]);
    if (source_fd) fclose(source_fd);
    exit (1);
  }
  /* Write out the header information */
  fprintf (target_fd, "Unix-time Lon Lat kiloamps decisecs multiplicity quality\n");
  /* Get rid of the ccb Header */
  fgets(buffer, CCB_HEADER, source_fd);

  /* Read out the lightning data. */
  while (fgets(buffer, 2, source_fd) != NULL)
  {
    time_stamp = 0;
    temp_short = 0;
    second_short = 0;
    memcpy((void *)&temp_short, buffer, 1);
    fgets(buffer, 2, source_fd);
    memcpy((void *)&second_short, buffer, 1);
    if (0x96 == second_short)
    {
      flash_message = 1; /* Indicates one or more flashes at this time */
      memset((void*)date, 0, sizeof(date[0])*4);
      for (x=0;x<4;x++)
      {
	fgets(buffer, 2, source_fd);
	temp_short = 0;
	memcpy((void *)&temp_short, buffer, 1);
	date[3 - x] = temp_short;
      }
      memset((void*)data_buffer, 0, sizeof(data_buffer[0])*2);
      memcpy((void *)&data_buffer[0], &date[1], 1);
      memcpy((void *)&data_buffer[1], &date[0], 1);
      memcpy((void *)&temp_short, data_buffer, 2);
      days = temp_short >> 1;
      hours = temp_short << 15;
      hours = hours >> 11;
      temp_short = 0;
      memcpy((void *)&temp_short, &date[2], 1);
      hours = (temp_short >> 4) ^ hours;
      mins = (temp_short << 4) & 255;
      mins = mins >> 2;
      temp_short = 0;
      memcpy((void *)&temp_short, &date[3], 1);
      mins = mins | (temp_short >> 6);
      secs = temp_short & 63;
      calc_time = (((((days * 24) + hours) * 60) + mins) * 60) + secs;
      calc_time += UNIX_TIME_DIFF;
      /* Assumes the first stamp is the correct time stamp */
      if (!time ||  labs(calc_time - time) < 120)
      {
	time = calc_time;
	time_stamp = 1;
      }
      else
      {
	time_stamp = 0;
	position = ftell(source_fd);
	position -= 5;
	if (fseek(source_fd, position, SEEK_SET))
	{
	  /* I don't think this can happen, but just in case. */
	  printf ("Error in time stamp resolution.\n");
	  if (source_fd) fclose(source_fd);
	  if (target_fd) fclose(target_fd);
	  exit (2);
	}
	temp_short = 0;
	second_short = 0;
	memcpy((void *)&temp_short, buffer, 1);
	fgets(buffer, 2, source_fd);
	memcpy((void *)&second_short, buffer, 1);
      }
    }/* IF time stamp if signaled. */
    if (!time_stamp && flash_message)
    {
      second_short = (second_short << 8) | temp_short;
      longitude = (second_short * LON_CONST) - 130;
      fgets(buffer, 2, source_fd);
      temp_short = 0;
      memcpy((void *)&temp_short, buffer, 1);
      fgets(buffer, 2, source_fd);
      second_short = 0;
      memcpy((void *)&second_short, buffer, 1);
      second_short = (second_short << 8) | temp_short;
      quality_flag = second_short >> 15;
      second_short = second_short & 32767;
      latitude = (second_short * LAT_CONST) + 18;
      fgets(buffer, 2, source_fd);
      temp_short = 0;
      memcpy((void *)&temp_short, buffer, 1);
      kiloamps = (temp_short & 127)*2;
      if (temp_short >> 7)
	kiloamps = -kiloamps;
      fgets(buffer, 2, source_fd);
      temp_short = 0;
      memcpy((void *)&temp_short, buffer, 1);
      duration = (temp_short >> 4);
      multiplicity = temp_short & 15;

      fprintf (target_fd, "%d %f %f %d %d %d %d\n", 
	       time, longitude, latitude, kiloamps, duration, 
	       multiplicity, quality_flag);
    }/* If this is flash message */
  }/* While there is still data */
  if (source_fd) fclose(source_fd);
  if (target_fd) fclose(target_fd);
  /********************************/
  /* Remove the binary file here. */
  /********************************/
}
