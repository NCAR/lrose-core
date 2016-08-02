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
#include <signal.h>

#include <tools_globals.h>
#include <toolsa/err.h>
#include <toolsa/sok2.h>

ModName( "Tests");


void Process_exit( sig)
   int sig;
   {
      SOK2exit( sig);
      printf("%s exited, code = %d\n", Module, sig);
      exit(1);
   }


int main( argc, argv)
    int  argc;
    char *argv[];
    {
#define MSIZE 10000
      	int 	ret, sd, i, count = 0;
      	int       wait = 1;
      	int	*message;
	int 	mess_start1 = 0;
	int	idx, client, len;
      	int printit = TRUE;     
      	int prompt = FALSE; 
	SOK2head head, *headp;
	char *messin;

      	if (argc == 1)
	    {
	    printf("Tests wait P\n");
	    exit(1);
	    }

      	if (argc > 1)
            wait = atoi( argv[1]);
      	if (wait == -2)
	    {wait = -1; prompt = TRUE;}  

      	if (argc > 2)
	    {
	    if (STRpos(argv[2], "P") != -1)
            	printit = FALSE;
	    }

      	if (NULL == (message = (int *) malloc (MSIZE * 4)))
	    {
	    printf( "malloc error");
	    return 1;
	    }

      	signal( SIGINT, Process_exit);
      	signal( SIGHUP, Process_exit);
      	signal( SIGTERM, Process_exit);
      	signal( SIGPIPE, Process_exit);

      	ERRinit( "Testserver", argc, argv);

      	SOK2init( Module);
      /* SOCKsetServiceFile( SOCK_FILE, "rap_services"); */
   
      	if (0 > (sd = SOK2openServer( 7777)))
            {
	    printf("openServer err ret = %d", sd);
            exit(10);
	    }
	printf("opened server %d\n");

  	/* wait for connection 
         if (0 > (ret = SOK2getMessage( -1, &idx, &client, &headp, &messin,
		&len)))
	    {
	    printf("SOK2getMessage  error = %d\n", ret);
	    exit(11);
	    }
	printf("Idx %d client %d Message = %s, len %d\n",
		idx, client, messin, len); */
		
	

      	while ( count++ < 1000)
            {
	    if (wait > 0)
            	sleep( wait);
            if (prompt)
               getchar();

            if (printit)
	    	printf("***try to send %d to %d\n", mess_start1, mess_start1+MSIZE-1);

	    for (i=0; i< MSIZE; i++)
	    	message[i] = mess_start1 + i;
	    mess_start1 += MSIZE;

            if (0 > (ret = SOK2sendMessageAll( wait == -1 ? -1 : wait*1000, 
			sd, 666, message, MSIZE*4)))
	    	{
	    	printf("SOK2sendMessage  error = %d\n", ret);
	    	break;
	    	}
	    while (ret == 0)
		{
		printf("continue write \n");
		ret = SOK2continueWrite( wait*1000, FALSE);
		}

	    if (printit)
            	fprintf( stderr, "sendMessage ret = %d\n", ret);
            }

      	exit( 0);
    }
