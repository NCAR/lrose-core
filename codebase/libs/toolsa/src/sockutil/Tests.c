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

#include <toolsa/sockutil.h>

int main( argc, argv)
    int  argc;
    char *argv[];
    {
#define MSIZE 10000
      	int 	ret, sd, cd;
      	int	port = 9999;
      	int     mess[1000];
	int 	i, len, count = 0, wait = 1;

	if (argc > 1)
	    wait = atoi(argv[1]);

	for (i=0; i<1000; i++)
	    mess[i] = 0;

	sd = SKU_open_server (port);	
      	printf("server socket = %d\n", sd);
	if (sd < 0)
	    return 1;

	sleep(wait);

      	while (1)
      	    {
	    if ( 0 < (cd = SKU_get_client_timed( sd, 10000)))
            	{
	    	printf("***client connect...");

	    	SKU_use_old_header = ! SKU_use_old_header;
	    	len = sizeof(mess);
	    	ret = SKU_writeh( cd, mess, len, 666, -1);
	    	printf("SKU writeh return = %d\n", ret);

	    	SKU_close( cd);
	    	}
	    else
		printf("get_client_timed timeout %d\n", ++count);
	    }

      return 0;
   }
