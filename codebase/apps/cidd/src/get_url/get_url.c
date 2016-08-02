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
/******************************************************************************
 *  GET_URL : Get a URL and print to stdout.  - Basically a test program for 
 * HTTPgetURL();
 * 
 */
#include <stdio.h>

/******************************************************************************
 * MAIN :   Open files and send the output to STDOUT
 *
 */
main(argc,argv)
	int	argc;
	char	*argv[];
{
	int  len,ret_stat;
	char *buf;


	if(argc != 2) { 	/* take the input from stdin */
		fprintf(stderr,"Usage: get_url URL\n");
		exit(-1);
	}

	ret_stat =  HTTPgetURL(argv[1],5000, &buf, &len);

	if(ret_stat < 0 || buf == NULL ) {
	 switch(ret_stat) {
	   default:
	     fprintf(stderr,"ERROR: %d\n",ret_stat);
	   break;

	   case -1:
	     fprintf(stderr,"ERROR: %d - Bad URL\n",ret_stat);
	   break;

	   case -2:
	     fprintf(stderr,"ERROR: %d - Failure to Connect to server\n",ret_stat);
	   break;

	   case -3:
	     fprintf(stderr,"ERROR: %d - Read Reply Failure\n",ret_stat);
	   break;

	   case -4:
	     fprintf(stderr,"ERROR: %d - Alloc Failure\n",ret_stat);
	   break;

	   case -5:
	     fprintf(stderr,"ERROR: %d - Couldn't find Content-Length: in reply\n",ret_stat);
	   break;

	   case 400:
	     fprintf(stderr,"ERROR: %d - Bad Request\n",ret_stat);
	   break;

	   case 401:
	     fprintf(stderr,"ERROR: %d - Unauthorized\n",ret_stat);
	   break;

	   case 402:
	     fprintf(stderr,"ERROR: %d - Payment Required\n",ret_stat);
	   break;

	   case 403:
	     fprintf(stderr,"ERROR: %d - Forbidden/Access Denied\n",ret_stat);
	   break;

	   case 404:
	     fprintf(stderr,"ERROR: %d - Not Found\n",ret_stat);
	   break;

	   case 405:
	     fprintf(stderr,"ERROR: %d - Method Not Allowed\n",ret_stat);
	   break;

	   case 406:
	     fprintf(stderr,"ERROR: %d - Method Not Acceptable\n",ret_stat);
	   break;

	   case 407:
	     fprintf(stderr,"ERROR: %d - Proxy Authentication Required\n",ret_stat);
	   break;

	   case 408:
	     fprintf(stderr,"ERROR: %d - Request Timeout\n",ret_stat);
	   break;

	   case 409:
	     fprintf(stderr,"ERROR: %d - Conflict\n",ret_stat);
	   break;

	   case 410:
	     fprintf(stderr,"ERROR: %d - Document Removed\n",ret_stat);
	   break;

	   case 411:
	     fprintf(stderr,"ERROR: %d - Length Required\n",ret_stat);
	   break;

	   case 412:
	     fprintf(stderr,"ERROR: %d - Precondition Failed\n",ret_stat);
	   break;

	   case 413:
	     fprintf(stderr,"ERROR: %d - Request Entity Too Large\n",ret_stat);
	   break;

	   case 414:
	     fprintf(stderr,"ERROR: %d - Request URI Too Large\n",ret_stat);
	   break;

	   case 415:
	     fprintf(stderr,"ERROR: %d - Unsupported Media Type\n",ret_stat);
	   break;

	   case 500:
	     fprintf(stderr,"ERROR: %d - Internal Server Error\n",ret_stat);
	   break;

	   case 501:
	     fprintf(stderr,"ERROR: %d - Not Implemented\n",ret_stat);
	   break;

	   case 502:
	     fprintf(stderr,"ERROR: %d - Bad Gateway\n",ret_stat);
	   break;

	   case 503:
	     fprintf(stderr,"ERROR: %d - Service Unavailable\n",ret_stat);
	   break;

	   case 504:
	     fprintf(stderr,"ERROR: %d - Gateway Timeout\n",ret_stat);
	   break;

	   case 505:
	     fprintf(stderr,"ERROR: %d - HTTP Version Not Supported\n",ret_stat);
	   break;

	 }
	} else {
	    fputs(buf,stdout);
	}
}
