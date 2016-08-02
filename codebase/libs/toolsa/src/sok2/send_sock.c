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
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/time.h>

#include <signal.h>

#include <toolsa/globals.h>
#include <toolsa/err.h>
#include <toolsa/str.h>

#include <toolsa/sok2.h>

ModName( "send_sock");


static void Process_exit( int code)
   {
      SOK2exit( code);
      printf("%s exited, code = %d\n", Module, code);
      exit(1);
   }


int main( int argc, char *argv[])
    {
      	int 	ret, sd, port;
	int	idx, client, len;
	SOK2head head;
	char message[100], *host;

      	if (argc == 1)
	    {
	    printf("send_sock <host> <port>\n");
	    exit(1);
	    }

        host = argv[1];
        port = atoi( argv[2]);

      	signal( SIGINT, Process_exit);
      	signal( SIGHUP, Process_exit);
      	signal( SIGTERM, Process_exit);

      	ERRinit( Module, argc, argv);
      	SOK2init( Module);
   
      	if (0 > (sd = SOK2openClient( host, port, -1)))
            {
	    printf("openClient err ret = %d", sd);
            exit(10);
	    }
	printf("opened client to %s %d\n", host, port);

	/* get messages, print them out */
	while (TRUE)
	    {
	    scanf("%s", message);
	    len = strlen(message) + 1;
            if (0 > (ret = SOK2sendMessage( -1, sd, 0, 666, message, len)))
	    	printf("SOK2sendMessage  error = %d\n", ret);
	    else
	    	printf("Message sent\n");
	    }	

	return 0;
    }
