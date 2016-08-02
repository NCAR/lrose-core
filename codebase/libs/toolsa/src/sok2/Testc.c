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
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include <toolsa/globals.h>
#include <toolsa/err.h>
#include <toolsa/str.h>

#include <toolsa/sok2.h>

static int printit = TRUE;

ModName("Testc");

void Process_exit( int code)
   {
      SOK2exit( code);
      printf("%s exited, code = %d\n", Module, code);
      exit(1);
   }



#define MSIZE 10000

int main (int argc, char *argv[])
    {
      	int 	i, sd, ret, cd;
      	int 	err_cnt = 0, count_poll = 0;
      	time_t	start, end;
      	long    tot_bytes = 0;
	int 	lenm, len, port = 7777;
      	int 	wait = -1, dotime = FALSE;
	char mess[60];

      	if (argc == 1)
	   {
	   printf("Testc port host TP\n");
	   exit(1);
	   }

      	if (argc > 1)
	    port = atoi( argv[1]);

      	if (argc > 3)
            {
	    if (STRequal("T", argv[3]))
	    	{
	    	dotime = TRUE;
	    	printit = FALSE;
	    	}
	    if (STRequal("P", argv[3]))
	    	{
	    	printit = FALSE;
	    	}
	    }

      	signal( SIGINT, Process_exit);
      	signal( SIGHUP, Process_exit);
      	signal( SIGTERM, Process_exit);
      	signal( SIGPIPE, Process_exit);

      	ERRinit( Module, argc, argv);

      	SOK2init( Module);

      	if (0 > (sd = SOK2openClient( argv[2], port, 5)))
	   {
	   fprintf(stderr, "error %d\n", sd);
           return 1;
	   }
        fprintf(stderr, "client socket %d connected wait = %d\n", sd, wait);

	/* send connect mesage */
	STRncopy(mess,"connect!",60);
	len = strlen(mess) + 1;
	
	if (0 > (ret = SOK2sendMessage( -1, sd, -1, 1234, mess, len)))
	   {
	   printf("sendMessage failed %d\n", ret);
	   exit(10);
	   }
	printf ("sent message %d\n", ret); 

      	time( &start);

      	while (err_cnt <  10) 
            {
	    char *message;
	    int idx, client, len;
	    SOK2head head;

            if (0 > (ret = SOK2getMessage( wait, &idx, &client, &head, &message,
		&len)))
	    	{
	    	printf("SOK2getMessage  error = %d\n", ret);
	    	exit(11);
	    	}
            else if (ret == 0)
	    	{
	    	fprintf(stderr, "polling wait = %d ...\n", count_poll++);
	    	continue;
	    	}
	    if (printit)
	        printf("Idx %d client %d len %d seq %d\n",
			idx, client, len, head.seq_no);
		

	    {
      	    int count, i;
      	    int *messi;
      	    int mess_next = 0;
      	    int startn = 0, nexti;

	    messi = (int *) message;
            count = len / 4;
	    startn = messi[0];
	    nexti = startn;

            tot_bytes += len;

	    for (i=0; i<count; i++)
	    	{
	    	if (messi[i] != nexti)
	       	    {
	            fprintf(stderr,"\nout of sequence at index %d seq %d; %d != %d\n",
		   	i, head.seq_no, messi[i], nexti);
		    break;
		    }
	        nexti++;
	        }

	    if (printit)
	    	printf(" *nread %d got %d to %d\n", 
		    count, startn, nexti-1); 
	    }

	 if (dotime)
	    {
            time (&end);
	    if ((end % 30 == 0) && (start != end))
               {
	       printf("\n%s:%d bytes/ %d secs = %6.2f Kbytes/sec\n",
	     	ctime( &end), tot_bytes, end - start, (float)( tot_bytes) /
		 (1000* (end - start)) );
	       start = end;
	       tot_bytes = 0;
	       }
	    }

	 }

      SOK2exit(1);
      return 0;
   }

