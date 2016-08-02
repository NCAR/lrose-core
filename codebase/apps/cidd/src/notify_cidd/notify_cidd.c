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
 * NOTIFY_CIDD.C  A Simple program to tell CIDD to re-read its data
 * Frank Hage	July 1993 NCAR, Research Applications Program
 */

#define MAIN

#include <stdio.h>

#include <rapformats/coord_export.h>
 
void process_args( int argc, char *argv[]);

int coord_key;
int command;
char *args;

/********************************************************************
 * MAIN: Process arguments, initialize and begin application
 */
 
int
main(int argc, char **argv)
{
    coord_export_t *coord;
    int not_done;

	process_args(argc,argv);	/* process command line arguments */

    if((coord = (coord_export_t *) ushm_create(coord_key, sizeof(coord_export_t), 0666)) == NULL) {
            fprintf(stderr, "Couldn't Get shared memory segment for Aux process communications\n");
            exit(-1);
    }

	not_done = 5;
    while(not_done) {
        if(coord->client_event == NO_MESSAGE) { // clear to go
          coord->client_event = command;
          if(args != NULL) strncpy(coord->client_args,args,MAX_CLIENT_EVENT_ARG);
          not_done = 0;
        } else { // Wait 50 miliseconds
          uusleep(50000); // 50 miliseconds
          not_done--;
        }
    }


	exit(0);
}

/*****************************************************************
 * PROCESS_ARGS: Progess command line arguments. Set option flags
 *	   And print usage info if necessary
 */

void process_args( int argc, char *argv[])
{
	int err_flag = 0;

    command = 0;
	if(argc == 4 || argc == 3) {
        coord_key =  atoi(argv[1]); 
	    if(strncmp(argv[2],"NEW_MDV_AVAIL",MAX_CLIENT_EVENT_ARG) == 0) command = NEW_MDV_AVAIL;
	    if(strncmp(argv[2],"NEW_SPDB_AVAIL",MAX_CLIENT_EVENT_ARG) == 0) command = NEW_SPDB_AVAIL;
	    if(strncmp(argv[2],"SET_TIME",MAX_CLIENT_EVENT_ARG) == 0) command = SET_TIME;
	    if(strncmp(argv[2],"SET_NUM_FRAMES",MAX_CLIENT_EVENT_ARG) == 0) command = SET_NUM_FRAMES;
	    if(strncmp(argv[2],"SET_FRAME_NUM",MAX_CLIENT_EVENT_ARG) == 0) command = SET_FRAME_NUM;
	    if(strncmp(argv[2],"SET_REALTIME",MAX_CLIENT_EVENT_ARG) == 0) command = SET_REALTIME;
		switch(command) {
		  default:
			args = NULL;
		  break;

		  case NEW_MDV_AVAIL:
		  case NEW_SPDB_AVAIL:
		  case SET_TIME:
		  case SET_NUM_FRAMES:
		  case SET_FRAME_NUM:
	        args = argv[3];
		  break;
		}

	} else {
	  err_flag++;;
	}  

	if(command == 0) err_flag++;

	if(err_flag) {
		fprintf(stderr,"Usage:notify_cidd key command arg\n");
		fprintf(stderr,"\t key - Shared memory Key \n");
		fprintf(stderr,"\tCommand        Argument\n");
		fprintf(stderr,"\t=======================================================================\n");
		fprintf(stderr,"\tRELOAD_DATA    None\n");
		fprintf(stderr,"\tSET_REALTIME   None\n");
		fprintf(stderr,"\tNEW_MDV_AVAIL  Menu Label of Data to invalidate - use quotes for spaces\n");
		fprintf(stderr,"\tNEW_SPDB_AVAIL Menu Label of Data to invalidate\n");
		fprintf(stderr,"\tSET_TIME       Start of Movie loop - \"Year Month Day Hour Min Sec\"\n");
		fprintf(stderr,"\tSET_NUM_FRAMES Number of frames in  Movie loop \n");
		fprintf(stderr,"\tSET_FRAME_NUM  Frame in Movie loop \n");
		exit(-1);
	}
}  
