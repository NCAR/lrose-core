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
 * CIDD_CLIENT.C  A Simple program to querry CIDD for key clicks
 *  and report the lat lon of the clicked position.
 * Frank Hage    May 1996 NCAR, Research Applications Program
 */

#define MAIN

#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <toolsa/utim.h>
#include <toolsa/udatetime.h>

#include <time.h>

#include <rapformats/coord_export.h>
 
void *ushm_create( long key, int size, int permissions);
/********************************************************************
 * MAIN: Process arguments, initialize and begin application
 */
 
int
main(int argc, char **argv)
{
    int last_no = 0;
    int last_time = 0;
    int coord_key;
    coord_export_t *coord;

    if(argc != 2) {
        fprintf(stderr,"Usage cidd_client key_number\n");
        exit(-1);
    }

    coord_key = atoi(argv[1]);

    /* Attach to CIDD's shared memory segment */
    if((coord = (coord_export_t *) ushm_create(coord_key, sizeof(coord_export_t), 0666)) == NULL) {
        fprintf(stderr, "Couldn't Get shared memory segment for Aux process communications\n");
        exit(-1);
    }

    last_no = coord->pointer_seq_num;  /* Pick up the click number at startup */

    /* Look every so often for a new key click */
    while(1) {
	if(coord->shmem_ready > 0 &&
	  (coord->pointer_seq_num != last_no ||
	  coord->time_seq_num != last_time) ) {  /* A new click has happened */

	    /* Do some work based on the mouse click */
	  printf("=====================================\n");
	  printf("current time: %s\n", utimstr(time(NULL)));
	  printf("%d Lat, Lon: %g,%g  Type: %d\n",
		 coord->pointer_seq_num,
		 coord->pointer_lat,coord->pointer_lon,
		 coord->click_type);

	  switch(coord->runtime_mode) {
		case RUNMODE_REALTIME:
		  printf("Runtime: RUNMODE_REALTIME num: %ld\n",(long int) coord->time_seq_num);
		break;

		case RUNMODE_ARCHIVE:
		  printf("Runtime: RUNMODE_ARCHIVE num: %ld\n",(long int) coord->time_seq_num);
		break;

	  }
	  printf("time_cent: %s\n", utimstr(coord->time_cent));
	  printf("time_min: %s\n", utimstr(coord->time_min));
	  printf("time_max: %s\n", utimstr(coord->time_max));
	  printf("time_max: %s\n", utimstr(coord->time_max));
	  printf("epoch_start: %s\n", utimstr(coord->epoch_start));
	  printf("epoch_end: %s\n", utimstr(coord->epoch_end));

	  printf("checkWriteTimeOnRead: %s\n",
                 coord->checkWriteTimeOnRead? "true" : "false");
          
          printf("latestValidWriteTime: %s\n",
                 utimstr(coord->latestValidWriteTime));
	  
	  printf("pointer_x: %g\n", coord->pointer_x);
	  printf("pointer_y: %g\n", coord->pointer_y);
	  
	  printf("pointer_lat: %g\n", coord->pointer_lat);
	  printf("pointer_lon: %g\n", coord->pointer_lon);
	  
	  printf("datum_lat: %g\n", coord->datum_latitude);
	  printf("datum_lon: %g\n", coord->datum_longitude);
	  printf("data_altitude: %g\n", coord->data_altitude);
	  
	  last_no = coord->pointer_seq_num;
	  last_time = coord->time_seq_num;
	}
	usleep(10000); /* 10 msec */
    }
     
    exit(0);
}

/****************************************************************************
 * USHM_CREATE
 * creates and attaches shared memory
 *
 * Mike Dixon RAP NCAR Boulder Co December 1990
 * Obtained from Bob Barron, RAP
 */

void *ushm_create( long key, int size, int permissions)
{
  void *memory_region;
  int shmid;

  /* * create the shared memory */

  if ((shmid = shmget((key_t)key, size, permissions | IPC_CREAT)) < 0) {
    fprintf(stderr, "ERROR - ushm_create.\n");
    perror("Getting shared memory with 'shmget'");
    return (NULL);
  }

  /* * attach the shared memory regions */
  errno = 0;

  memory_region = (void *) shmat(shmid, (char *) 0, 0);

  if (errno != 0) {
    fprintf(stderr, "ERROR - ushm_create.\n");
    perror("Attaching shared memory with 'shmat'");
    return (NULL);
  }

  return(memory_region);
}
