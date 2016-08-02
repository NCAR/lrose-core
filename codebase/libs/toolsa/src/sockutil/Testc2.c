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
#include <time.h>

#include <toolsa/sockutil.h>
#include <toolsa/smu.h>
#include <toolsa/servmap.h>

int main( argc, argv)
    int  argc;
    char *argv[];
    {
#define MSIZE 10000
      	int 	ret, sd, cd;
      	int	port = 5432;
      	char     mess[MSIZE];
	int 	i, len;
	char	*host, *ptr;
	SKU_header_t head;
	SERVMAP_info_t *info;
	SERVMAP_reply_t *reply;
	SERVMAP_request_t req;
	time_t	now;


	printf("usage: type subtype instance\n");

	host = "piglet";

	strcpy( req.server_type, argv[1]);
	strcpy( req.server_subtype, argv[2]);
	strcpy( req.instance, argv[3]);
	req.want_realtime = TRUE;
	req.time = 0;

	cd = SKU_open_client (host, port);	
      	printf("client socket = %d\n", cd);
	if (cd < 0)
	    return;

	SMU_htonl_Request( &req);
	SKU_writeh( cd, &req, sizeof(SERVMAP_request_t), 
			SERVMAP_GET_SERVER_INFO, -1);
      
	ret = SKU_readh( cd, mess, MSIZE, &head);
	printf("...readh %d ret old %d\nid %d len %d seqno %d\n", 
		ret, SKU_use_old_header, 
		head.id, head.len, head.seq_no);
	reply = (SERVMAP_reply_t *) mess;
	printf("reply %d %d\n", reply->return_code, reply->n_servers);

	ptr= mess + sizeof(SERVMAP_reply_t);
	for (i=0; i < reply->n_servers; i++)
	    {
	    info = (SERVMAP_info_t *) ptr;
	    SMU_ntohl_Info( info);
	    printf("   type %s sub %s instance %s\n",
		info->server_type, info->server_subtype, info->instance);
	    printf("   host %s port %d realtime %d",
		info->host, info->port, info->realtime_avail);
	    printf(" time1 %s", ctime( &info->start_time));
	    printf(" time2 %s\n", ctime( &info->end_time));

	    ptr += sizeof( SERVMAP_info_t);
	    }

	SKU_close( cd);

        exit( 0);
    }
