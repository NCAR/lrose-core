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
# include   <sys/mtio.h>

# include	"tdwr2moments.h"

/*-------------------------------------------------------------------------*/

# define	BUFLEN	1536

/*-------------------------------------------------------------------------*/
/* reader */
/*-------------------------------------------------------------------------*/

Global  Glob;
char    *module = "FORMATTER";
int		fd_tape;
int		source = 0;
int		data_rate = 60000;
int		dropped_gates = 0;
int		tape_rewind = 0;
int		tape_skip = 0;
int		num_files = 0;
char	data_source[20];
int		go = 1;

unsigned	itime = 500;
unsigned	no_packet = 10000;

/*
 * file scope prototypes
 */

static int get_shmem_data (LL_beam_rec *enet_pak,
                           int shmem_key);

static int init_streams ();
static void tape_command (void);

/*-------------------------------------------------------------------------*/


int main (int argc, char **argv)

{
char		command [CMD_SIZE];

int		initialize_tape = 1;
int	 	tape_status = 5;

int		status;
unsigned char	*beam_ptr;
int		first_time = TRUE;

LL_beam_rec *enet_pak;

extern process_exit();
extern print_target_elev ();

	
	/* trap signals */
	(void) signal (SIGINT, ((void (*) ()) process_exit));
	(void) signal (SIGHUP, ((void (*) ()) process_exit));
	(void) signal (SIGTERM, ((void (*) ()) process_exit));

	enet_pak = (LL_beam_rec *) malloc ((unsigned) sizeof (LL_beam_rec));
	if (enet_pak == NULL)
	{
		(void) printf ("Malloc error \n");
		return (-1);
	}

	if ((!get_params (&Glob)))
		exit (2);

	/* initialize streams (fmq and/or shmem)  for sending beam data */
	if (init_streams ())
	{
		(void) fprintf (stderr, "streams initialization error");
		return (1);
	}

	if (system ("clear") != 0)
		perror ("clear");

/* 	Determine the source of the data to be reformatted */
	if (!Glob.real_time)
	{
		/* initialize the shared memory for receiving commands*/
    		if (recv_cmd (1, Glob.cmd_shmem_key, (unsigned char **) &command) == -1)
		{
			(void) printf ("recv_cmd fatal error\n");
			exit (-1);
		}

		for (;;)
		{
			if ((status = get_shm_cmd (command)) > 0)
				break;
			else
				if (first_time)
				{
					first_time = FALSE;
					(void) printf ("waiting for menu process to send start command\n");
				}
				sleep (2);
		}
		source = parse_settings (command, &Glob);
		if (source < 0)
			return (0);
	}
	else
		source = RADAR;

	switch (source)
	{
	case RADAR: 

		if (recv_shm (1, Glob.input_shmem_key, &beam_ptr))
		{
			(void) printf ("recv_shm fatal error\n");
			exit (-1);
		}

		for (;go ;)
		{
			if (get_shmem_data (enet_pak, Glob.input_shmem_key)
																		== 0)
			{
				if (first_time)
				{
					(void) printf ("waiting for data\n");
					first_time = FALSE;
				}
				usleep (no_packet);
			}

			/* Check for a stop command */
			if (!Glob.real_time)
			{
				if ((status = get_shm_cmd (command)) > 0)
				{
					source = parse_settings (command, &Glob);
					if (source < 0) go = 0;
					if (source == FLAG_CHANGE)
					{
						(void) printf ("\n  Flag status: Dealias failure = %s\n",
											Glob.caf ? "TRUE":"FALSE");
						(void) printf ("                 Point Target    = %s\n",
											Glob.ctf ? "TRUE":"FALSE");
						(void) printf ("                SNR thresholding = %s\n",
											Glob.cvf ? "TRUE":"FALSE");
						(void) printf ("                 Clutter = %s\n",
											Glob.ccv ? "TRUE":"FALSE");
						(void) printf ("              low PRF in valid gate = %s\n\n",
											Glob.cv ? "TRUE":"FALSE");
					}

				}
			}
		}
		break;

	case TAPE:

		if (Glob.remote_tape)
		{
			if (tape_init (&fd_tape, Glob.tape_device))
			{
				(void) printf("Error initializing the tape device %s", 
														Glob.tape_device);
				source = -1;
				return (0);
			}
		}

		for (;go ;)
		{
			if (initialize_tape)
			{
				initialize_tape = FALSE;
				tape_command ();
			}

			tape_status = get_tape_data (fd_tape, data_rate, enet_pak); 
			switch (tape_status)
			{
                case 0:
					/* automatically rewind the tape
					 * when end of tape is detected
					 */
					 /*
                        		tape_rewind = REWIND;
		                        initialize_tape = TRUE;
					  */
					  /* or stop the system at the end of the tape */
                    (void) printf ("Received STOP command \n");
		            go = 0;
					exit (0);
               		break;

                case INITIALIZE_RMT:
					(void) close (fd_tape);
					if (tape_init (&fd_tape, Glob.tape_device))
					{
						(void) printf("Error initializing the tape device %s", 
														Glob.tape_device);
						source = -1;
						return (0);
					}
               		break;
                case INITIALIZE:
                    initialize_tape = TRUE;
               		break;
		        case REWIND:
		            /*go = 0;
					exit (0);
               		break;*/
                    tape_rewind = REWIND;
		            initialize_tape = TRUE;
               		break;
               	case TAPE_ERR: 
                	(void) printf ("Tape read error / received STOP command \n");
		             go = 0;
               		break;
				case RATE_CHANGE:
					initialize_tape = FALSE;
					break;

				case PAUSE:
					for (;;)
					{
						if (Glob.remote_tape)
						{
							(void) rmt_close ();
						}
						if ((status = get_shm_cmd (command)) > 0)
						{
							if (Glob.remote_tape)
							{
								if (tape_init (&fd_tape, Glob.tape_device))
								{
									(void) printf("Error initializing the tape device %s", Glob.tape_device);
									go = 0;
								}
							}
							break;
						}
						else
							usleep (3000);
					}
					break;
				case FLAG_CHANGE:
					(void) printf ("\n  Flag status: Dealias failure = %s\n",
										Glob.caf ? "TRUE":"FALSE");
					(void) printf ("                 Point Target    = %s\n",
										Glob.ctf ? "TRUE":"FALSE");
					(void) printf ("                SNR thresholding = %s\n",
										Glob.cvf ? "TRUE":"FALSE");
					(void) printf ("                 Clutter = %s\n",
										Glob.ccv ? "TRUE":"FALSE");
					(void) printf ("              low PRF in valid gate = %s\n\n",
										Glob.cv ? "TRUE":"FALSE");
			}
		}
		break;

	default:
			(void) printf ("source unrecognized \n");
			break;
	}

	return (0);

}

/*-------------------------------------------------------------------------*/

int parse_settings (packet, Glob)

char	*packet;
Global	*Glob;

{
Cmd_pkt		*command;
Alter_pkt	*substitute;

	(void) printf ("\nReceiving command menu instructions:\n");
	(void) printf ("'%s'\n", packet);


	if (strcmp (packet, "STOP") == 0)
		return (-1);
	else if (strncmp (packet, "TDWR ETHER", 10) == 0)
	{
		return (RADAR);
	}
	else if (strncmp (packet, "TDWR TAPE", 9) == 0)
	{
		command = (Cmd_pkt *) packet;

		(void) strcpy (Glob->tape_device, command->tape.device);
		(void) printf ("Tape device = %s\n", Glob->tape_device);

		if (strncmp(Glob->tape_device, "/", 1) == 0)
            Glob->remote_tape = FALSE;
		else
			Glob->remote_tape = TRUE;

		dropped_gates = command->dropped_gates;
		(void) printf ("Number of dropped gates = %d\n", dropped_gates);

		data_rate = command->tape.rate;
		(void) printf ("Data rate = %d\n", data_rate);

		tape_rewind = command->tape.rewind;
		if (tape_rewind)
			(void) printf ("Tape will be rewound\n");

		tape_skip = command->tape.position.direction;
		num_files = command->tape.position.change;

		return (TAPE);
	}
	else if (strncmp (packet, "PAUSE SYS", 9) == 0)
	{
		substitute = (Alter_pkt *) packet;
		(void) printf("pausing system - press 1 in menu to restart\n");
		return (PAUSE);
	}
	else if (strncmp (packet, "CV FLAG", 7) == 0)
	{
		substitute = (Alter_pkt *) packet;
		Glob->cv = substitute->data.value;
		return (FLAG_CHANGE);
	}

	else if (strncmp (packet, "CAF FLAG", 8) == 0)
	{
		substitute = (Alter_pkt *) packet;
		Glob->caf = substitute->data.value;
		return (FLAG_CHANGE);
	}

	else if (strncmp (packet, "CTF FLAG", 8) == 0)
	{
		substitute = (Alter_pkt *) packet;
		Glob->ctf = substitute->data.value;
		return (FLAG_CHANGE);
	}

	else if (strncmp (packet, "CVF FLAG", 8) == 0)
	{
		substitute = (Alter_pkt *) packet;
        Glob->cvf = substitute->data.value;
		return (FLAG_CHANGE);
	}

	else if (strncmp (packet, "CCV FLAG", 8) == 0)
	{
		substitute = (Alter_pkt *) packet;
		Glob->ccv = substitute->data.value;
		return (FLAG_CHANGE);
	}

	else if (strncmp (packet, "DATA RATE", 9) == 0)
	{
		substitute = (Alter_pkt *) packet;
		(void) printf ("Data Rate was %d\n", data_rate); 
		data_rate = substitute->data.value;
		(void) printf ("Data Rate has changed to %d\n", data_rate); 
		return (RATE_CHANGE);
 
	}
	else if (strncmp (packet, "TAPE DRIVE", 10) == 0)
	{
		substitute = (Alter_pkt *) packet;
		(void) strcpy (Glob->tape_device, substitute->data.string);
		(void) printf ("'%s'\n", Glob->tape_device);

		if (strncmp(Glob->tape_device, "/", 1) == 0)
            Glob->remote_tape = FALSE;
		else
		{
			Glob->remote_tape = TRUE;
			return (INITIALIZE_RMT);
		}
		return (INITIALIZE);

	}
	else if (strncmp (packet, "REWIND", 6) == 0)
	{
		substitute = (Alter_pkt *) packet;
		return (tape_rewind = substitute->data.value);
	}
	else if (strncmp (packet, "SKIP FORWARD", 12) == 0)
	{
		substitute = (Alter_pkt *) packet;
		num_files = substitute->data.value;
		tape_skip = FORWARD;
		return (INITIALIZE);
	}
	else if (strncmp (packet, "SKIP BACKWARD", 13) == 0)
	{
		substitute = (Alter_pkt *) packet;
		num_files = substitute->data.value;
		tape_skip = BACKWARD;
		return (INITIALIZE);
	}
	return (-1);
}

/*-------------------------------------------------------------------------*/

void
tape_command (void)

{
	
	char	sys_cmd[32];

	int	tape_init ();
	int	system ();
	struct mtop op;
	int		i;

	if (!Glob.remote_tape)
		(void) close (fd_tape);

	if (tape_rewind)
	{
		if (Glob.remote_tape)
		{
			op.mt_op    = MTREW;
			op.mt_count = 1;

			(void) printf ("\nissuing REWIND command\n");
			i = rmt_ioctl (MTIOCTOP, (char *) &op);
		}
		else
		{
			(void) sprintf (sys_cmd, "mt -f %10s rew", Glob.tape_device);
			(void) printf ("\nissuing '%s' command\n", sys_cmd);

			if (system (sys_cmd) != 0)
			{
				perror ("mt rew");
				source = -1;
				return;
			}
		}
		tape_rewind = 0;
		(void) printf ("tape rewound\n");
	}

	if (tape_skip)
	{
		if (tape_skip == FORWARD)
		{
			if (Glob.remote_tape)
			{
				op.mt_op    = MTFSF;
				op.mt_count = num_files;

				(void) printf ("\nissuing FILE SKIP FORWARD %d command\n",
																num_files);
				i == rmt_ioctl (MTIOCTOP, (char *) &op);
				if (i == -1)
					(void) printf ("rmt_ioctl failed\n");
				else
					(void) printf ("rmt tape status %d\n", i);

			}
			else
			{
				(void) sprintf (sys_cmd,"mt -f %10s fsf %d",
					Glob.tape_device, num_files);
			}
		}
		else if (tape_skip == BACKWARD)
		{
			if (Glob.remote_tape)
			{
				op.mt_op    = MTBSF;
				op.mt_count = num_files;

				(void) printf ("\nissuing SKIP BACKWARD %d files command\n",
																num_files);
				i == rmt_ioctl (MTIOCTOP, (char *) &op);
				if (i == -1)
					(void) printf ("rmt_ioctl failed\n");
				else
					(void) printf ("rmt tape status %d\n", i);
			}
			else
			{
				(void) sprintf (sys_cmd,"mt -f %10s bsf %d",
						Glob.tape_device, num_files);
			}
		}
		if (!Glob.remote_tape)
		{
			(void) printf ("\nissuing '%s' command\n", sys_cmd);
			if (system (sys_cmd) != 0)
			{
				if (tape_skip == FORWARD)
					perror ("mt fsf");
				else
					perror ("mt bsf");

			}
			else
			{
				tape_skip = 0;
				(void) printf ("tape repositioned, statistics: \n");
				(void) sprintf (sys_cmd,"mt -f %10s stat", Glob.tape_device);
				system (sys_cmd);
			}
		}
	}
	if (!Glob.remote_tape)
	{
		if (tape_init (&fd_tape, Glob.tape_device))
		{
			(void) printf("Error initializing the tape device %s", Glob.tape_device);
			source = -1;
			exit;
		}
	}
}

/*-------------------------------------------------------------------------*/
static int
get_shmem_data (enet_pak, shmem_key)

LL_beam_rec *enet_pak;
int		shmem_key;

{
	int		data_len;

	TDWR_data_header     *beam_ptr;

	if ((data_len = recv_shm (2, shmem_key, (unsigned char **) &beam_ptr)) < 0)
	{
		(void) printf ("recv_shm fatal error\n");
		exit (-1);
	}
	else if (data_len == 0)
		return (data_len);
	else 
	{
	 	be_to_TDWR_data_header ((TDWR_data_header *) beam_ptr);

		if (((TDWR_data_header *)beam_ptr)->message_length != data_len)
		{
			((TDWR_data_header *)beam_ptr)->message_length = data_len;
		}

		tdwr_process_logical ((TDWR_data_header *) beam_ptr, enet_pak);
			
	}
	return (data_len);
}
/************************************************************************/
int get_shm_cmd (char *command)

{
	int		data_len;
	unsigned char	*data_ptr;

	if ((data_len = recv_cmd (2, 512, &data_ptr)) == -1)
	{
		(void) printf ("recv_cmd fatal error\n");
		exit (-1);
	}

	if (data_len > 0)
	{
		memcpy (command, data_ptr, data_len);
	}
	return (data_len);
}

/************************************************************************/
int init_streams (void)

{
	int i;
	unsigned char   buffer[10];
	int iret = 0;


	if (Glob.write_fmq_output)
	{
		
		if (open_output_fmq(Glob.output_fmq_path,
                      		Glob.output_fmq_size,
                        	Glob.output_fmq_nslots,
                        	Glob.output_fmq_compress,
                        	Glob.prog_name,
       	                	Glob.debug)) 
		{
    
			iret = -1;
    		}

       	}

 	if (Glob.write_shm_output) 
	{
    		if (send_shm (1, Glob.output_shmem_key, &buffer[0]) 
							== -1)
		{
			(void) printf ("send_shm fatal error\n");
      			iret = -1;
		}
  	}

	return (iret);
}
/************************************************************************/

int process_exit (int sig)

{
	if (Glob.remote_tape)
	{
		(void) rmt_close ();
	}

	send_shm (0,0,"    ");
	(void) printf ("%s exited, signal = %d\n", module, sig);
	exit (1);
}

/************************************************************************/
