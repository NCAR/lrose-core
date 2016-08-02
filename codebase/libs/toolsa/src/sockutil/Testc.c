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
#include <errno.h>
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
	int 	i, len;
	char	*host;
	SKU_header_t head;

	for (i=0; i<1000; i++)
	    mess[i] = 0;

	host = argv[1];

	cd = SKU_open_client (host, port);	
      	printf("client socket = %d\n", cd);
	if (cd < 0)
	    {
	    printf("SKU_open_client failed: errno = %d\n", SKUerrno);
	    return 1;
	    }
      
    again:
	len = sizeof(mess);
	ret = SKU_readh( cd, mess, sizeof(mess), &head, 15000);
	printf("...readh %d ret old %d\nid %d len %d seqno %d\n", 
		ret, SKU_use_old_header, 
		head.id, head.len, head.seq_no);
	if (ret == 1)
	    goto again;

	SKU_close( cd);

        return 0;
    }
