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
#include <dataport/port_types.h>

#include "nexrad.h"

/*----------------------------------------------------------------------------*/

#define MAX_NAME_SIZE	120

#define UDP_FRAME_SIZE 	1024     /* largest frame to be sent (bytes) */
/*#define UDP_FRAME_SIZE 	1472*/     /* largest frame to be sent (bytes) */
#define COMMUNICATION_HDR_LEN 8
#define MAX_PKTS		5

/*#define MAX_PKTS 0x7fff*/

#define DEFAULT_SHMEM_KEY 2750400
#define DEFAULT_OUTPUT_SHMEM_KEY 2750400
#define OUTPUT_KEY 3740800
#define DEFAULT_PORT_NUM 3280
#define DEFAULT_BROADCAST_ADDR "128.117.200.0"

# define	FIRST_PKT	1

# define	TRUE	1
# define	FALSE	0

/* shared memory function switches */
#define REMOVE_SHM          0
#define ATTACH_SHM          1
#define READ                2
#define WRITE               2
#define WRITE_NO_UPDATE     3
#define UPDATE_WRITE_PTR    4
#define GET_WRITE_PTR       5
#define FINISH_WRITE        6

/* Error codes */
#define ERR_FATAL          -1

#define WARNING_NO_CLIENT		1
#define WARNING_CLIENT_STARTUP 	2
#define WARNING_BUFFER_FULL    	3
#define WARNING_BAD_MSG_LEN		4
#define WARNING_WRONG_PORT_NUM	5

#define OK	0

typedef struct
{
	ui32    fr_seq;	/* incremental counter for all packets*/
	ui16	mess_length; /* length of entire message*/
	ui16	total_frames;	/* number of packets to send message*/
	ui16    packet_no;	/* packet number for this packet ( n of x)	*/
	ui16	packet_offset; /* number of bytes previosly sent in packet*/
	ui16	packet_length; /* length of the packet minus this header*/
	ui16    flags;		/* specific flags used for various indicators*/ 
	NEXRAD_msg_hdr	msg_hdr;
} Packet_hdr;


typedef struct
{
	int				data_len;
	char			*data_ptr;
} Packet_info;

typedef struct
{
	Packet_hdr		packet_hdr;
	unsigned char	data[MAX_PKTS * UDP_FRAME_SIZE + 256];
} Message;
