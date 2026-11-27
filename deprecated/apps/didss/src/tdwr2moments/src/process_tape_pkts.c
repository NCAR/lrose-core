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
/****************************************************************************
 * process_tape_pkts.c
 *
 *      this routine reads tape data, seqments the data into logical records,
 *      and divides each record into simulated ethernet packets.  The 
 *	justification for this methodology is to provide a mechanism for 
 *	testing modifications made to the formatter without requiring the radar
 *	to be on line.  
 *
 * RAP, NCAR, Boulder CO
 *
 * Feb 1991
 *
 * Gary Blackburn
 *
 ****************************************************************************/

# include <dataport/bigend.h>
# include   "tdwr2moments.h"
# include   "tape.h"


extern Global   Glob;

static int read_logical (int tape_fd,
                         unsigned char **logical_rec);

static int get_physical_rec (int tape_fd, unsigned char *buffer);

static int tape_set_var (int tape);

/*-------------------------------------------------------------------------*/
  
int	get_tape_data (int tape_fd, int data_rate, LL_beam_rec *enet_pak)
/*
 * Read a file from tape and process each record until end of tape
 */

{
     
unsigned char   *buffer;
char			command[CMD_SIZE];
int             log_rec_size = 1;
int				result;
int				status;
TDWR_data_header   *data_hdr;
TDWR_data_header   tdwr_hdr;
TDWR_normal_prf_data *data_ptr;
int				i = 0;

	log_rec_size = read_logical (tape_fd, &buffer);

	while (log_rec_size > 0)
	{
		if ((status = get_shm_cmd (command)) > 0)
		{
			result = parse_settings (command, &Glob);
			return (result);
		}

		data_hdr = (TDWR_data_header *) buffer;
		/*be_to_TDWR_data_header (data_hdr);*/

		if (0)
		{
			data_ptr = (TDWR_normal_prf_data *) (buffer + sizeof (TDWR_data_header));
			(void) printf ("tilt num - %d, azimuth-  %.2f\n", data_hdr->scan_info_flag >> 24, data_hdr->azimuth);

			(void) printf ("DBZ    SNR   VEL   WIDTH\n");

			for (i = 0; i < data_hdr->rng_samples_per_dwell; i++)
			{
				data_ptr->vel = BE_to_ui16(data_ptr->vel);
				(void) printf ("%d %d %d %d\n", data_ptr->dbz, data_ptr->snr, data_ptr->vel, data_ptr->width);
				data_ptr++;
			}
		}
		else
			if (data_hdr->message_id == NORMAL_PRF_BASE_DATA || data_hdr->message_id
																	== LOW_PRF_BASE_DATA)
			{
				tdwr_process_logical (data_hdr, enet_pak);

				usleep ((unsigned) data_rate);
			}

		log_rec_size = read_logical (tape_fd, &buffer);
		if (log_rec_size == 0)
			log_rec_size = read_logical (tape_fd, &buffer);
	}
	/* send out the last tilt if we reach end of tape of have a
	 * tape error
	 */
	tdwr_process_logical (&tdwr_hdr, enet_pak);
	return (TAPE_ERR);
}

/*-------------------------------------------------------------------------*/

int	read_logical (int tape_fd, unsigned char **logical_rec)
/*
 * read_logical: reads a logical record from tape and returns its 
 * size and a pointer to it.
 */


{

static unsigned char	physical_rec[MAX_REC_LEN];
static unsigned short	offset = 0;	/* Start of a logical record */ 
unsigned short	 	log_rec_size, nread;
static short	left = 0;		/* Portion of a physical record yet to be processed */
int		counter =0;
TDWR_data_header *data_hdr;

	if (left == 0)
	{
		nread = get_physical_rec (tape_fd, physical_rec);
		if (nread == 0)
		{
			nread = get_physical_rec (tape_fd, physical_rec);
			if (nread == 0)
			{
				nread = get_physical_rec (tape_fd, physical_rec);
				if (nread == 0)
				{
					nread = get_physical_rec (tape_fd, physical_rec);
				}
			}
		}

		offset = 0;
		left = nread;
	}

	if (left == 0)
	{
		(void) printf ("End of tape\n");
		return (left);
	}

	if (left < 0)
	{
		(void) printf ("tape read error = %d\n", nread);
		
		*logical_rec = (unsigned char *) 0;
		log_rec_size = left;

	}
	else
	{
redo:
		*logical_rec = physical_rec + offset;
		data_hdr = (TDWR_data_header *) *logical_rec;
		be_to_TDWR_data_header (data_hdr);
		log_rec_size = data_hdr->message_length;

		offset += log_rec_size;
		left -= log_rec_size;

		if (left < log_rec_size) left = 0;

		if (log_rec_size == 0 && left > 0)
		{
			nread = get_physical_rec (tape_fd, physical_rec);
			offset = 0;
			left = nread;
			goto redo;

		}
	}
	return log_rec_size;	
}

/*------------------------------------------------------------------------*/

int      get_physical_rec (int tape_fd, unsigned char *buffer)

{
	int status;

	if (Glob.remote_tape)
	{
		return (rmt_read ((char *) buffer, RECSIZE));
	}
	return (read (tape_fd, (char *) buffer, RECSIZE));
}

/*-------------------------------------------------------------------------*/

tape_init (int *fd, char *device)

{

	if (Glob.remote_tape)
	{
		if ((*fd = rmt_open (device, O_RDONLY, 0666)) < 0)
		{
			perror (device);
			return (1);
		}
	}
	else
	{
		if ((*fd = open (device, O_RDONLY)) < 0)
		{
			perror (device);
			return (1);
		}
		tape_set_var (*fd);
	}   
	(void) printf ("successfully opened tape, desc = %d\n", *fd);
	return (0);
}

 
/***************************************************************************
 * tape_set_var()
 *
 * Set variable block size mode
 *                          
 * returns -1 if error, 0 if success      
 *                               
 **************************************************************************/

static int tape_set_var(int tape)

{

#ifdef LINUX                      

#include <sys/mtio.h>
        
  struct mtop mt_command;
        
  mt_command.mt_op = MTSETBLK;
  mt_command.mt_count = 0;
  return ioctl(tape, MTIOCTOP, (caddr_t) &mt_command); 

#else

  return (0);

#endif

}

/************************************************************************/
