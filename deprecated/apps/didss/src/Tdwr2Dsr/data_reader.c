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

process_beam (packet, packet_len, pkt_set)

char	*packet;
int		packet_len;
Packet_info pkt_set[];
{

Packet_hdr     	*frame_hdr = (Packet_hdr *) packet;
unsigned char	*data_ptr;
int				max_msg_len;
extern int 		glob_max_pkt_size;
extern int 		glob_max_num_pkts;
int				first_time = TRUE;
int				i;
int				data_len = 0;

static	int		npkts 			= 0;
static 	int		pkts_per_beam 	= 0;
static	int		frame_seq 		= -1;
static	int		last_frame_seq 	= -1; 

/*static	Packet_info pkt_set[MAX_PKTS];*/   /* a set represents one beam */

	max_msg_len = glob_max_num_pkts * glob_max_pkt_size;
	if (max_msg_len != 7360)
	{
		(void) printf ("Memory over run!\n");
	}

	if ((frame_seq = frame_hdr->fr_seq) != last_frame_seq + 1)
	/* a packet was dropped */
	{
		(void) printf ("Detected sequence error; expected %d - got %d\n", last_frame_seq + 1, frame_seq);

		if (frame_hdr->fr_frame_num_in_msg > FIRST_PKT) 
		{
			/* part of the current beam was dropped */
	/*		(void) printf ("dropping beam missing CONTINUE_PKT\n"); */

			last_frame_seq = frame_seq;
			npkts = 0;

			/* do not continue to process this beam */
			return;
																						}
		/* else if this is a first packet then continue processing */
		/*(void) printf ("dropping beam missing FIRST_PKT\n");*/
	}
   	last_frame_seq = frame_seq;

	/* if this packet is a first packet then initialize variables */
	if (frame_hdr->fr_frame_num_in_msg == FIRST_PKT)
	{
		npkts = 0;
		pkts_per_beam = frame_hdr->fr_frames_per_msg;

	}

	if (npkts > pkts_per_beam - 1)
	{
		(void) ("Packets in set exceed maximum\n");
		npkts = 0;
		return;
	}
	if (npkts > glob_max_num_pkts)
	{
		(void) ("Total packets (%d) exceeded maximum - %d\n", npkts,
													glob_max_num_pkts);
		npkts = 0;
		return;
	}


	/* add this packet to the queue */
	pkt_set[npkts].data_ptr = packet;
	pkt_set[npkts].data_len = packet_len;
	npkts++;

	/* if there are a complete compliment of packets then */
	if (npkts == pkts_per_beam)
	{

		for (;;)
		{
			if ((i = send_shm (GET_WRITE_PTR, max_msg_len, data_ptr)) 
														== ERR_FATAL)
			{
				(void) printf ("send_shm fatal error\n");
				exit (ERR_FATAL);
			}

			if (i <= WARNING_BUFFER_FULL && i >= WARNING_NO_CLIENT)
			{
				if (first_time)
				{
					first_time = FALSE;
					if (i == WARNING_BUFFER_FULL)
						(void) printf ("buffer is full\n"); 
					else
						(void) printf ("Client delay trying again\n");
				}
				sleep (2);
			}
			else
			{
				first_time = TRUE;
				data_ptr = (unsigned char *) i;
				break;
			}
		}

		/* process all the queued packets */
		process_pkt_set (pkt_set, data_ptr, &data_len);

		if (0)
		(void) printf ("msg sent\n");

		for (;;)
		{
			if ((i = send_shm (FINISH_WRITE, data_len, data_ptr)) == ERR_FATAL)
			{
				(void) printf ("send_shm fatal error");
				exit (ERR_FATAL);
			}

			if (i <= WARNING_BUFFER_FULL && i >= WARNING_NO_CLIENT)
			{
				if (first_time)
				{
					first_time = FALSE;
					if (i == WARNING_BUFFER_FULL)
						(void) printf ("buffer is full\n"); 
					else
						(void) printf ("Client delay trying again\n");
				}	
				sleep (2);
			}
			else
			{
				first_time = TRUE;
				break;
			}
		}
		npkts = 0;
	}

}
/*---------------------------------------------------------------------------*/
