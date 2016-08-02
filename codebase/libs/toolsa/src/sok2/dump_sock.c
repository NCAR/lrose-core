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

ModName( "dump_sock");


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
	char *message;

      	if (argc == 1)
	    {
	    printf("dump_sock <port>\n");
	    exit(1);
	    }

      	if (argc > 1)
            port = atoi( argv[1]);

      	signal( SIGINT, Process_exit);
      	signal( SIGHUP, Process_exit);
      	signal( SIGTERM, Process_exit);

      	ERRinit( Module, argc, argv);
      	SOK2init( Module);
   
      	if (0 > (sd = SOK2openServer( port)))
            {
	    printf("openServer err ret = %d", sd);
            exit(10);
	    }
	printf("opened server on port %d\n", port);

	/* get messages, print them out */
	while (TRUE)
	    {
            if (0 > (ret = SOK2getMessage( -1, &idx, &client, &head, &message,
		&len)))
	       	{
	    	printf("SOK2getMessage  error = %d\n", ret);
	    	exit(11);
	    	}
	    printf("Message type %d len %d = %s\n", head.id, len, message);
	    }	

	return 0;
    }
