/*-------------------------------------------------------------------------*/

# include   <signal.h>
# include	<strings.h>
# include	<stdlib.h>
# include	<stdio.h>
# include	<malloc.h>
# include	"command.h"

/*-------------------------------------------------------------------------*/

char	space_cmd[3];
char	skip_cmd[10];

char	*ray_tape    = "RAYTHEON TAPE";
char	*cft_tape    = "CFT TAPE";
char	*fof_tape    = "FOF TAPE";
char	*rp7_tape    = "TDWR TAPE";
char	*rp7_ether   = "TDWR ETHERNET";
char	*exabyte0    = "/dev/nrst8";
char	*exabyte1    = "/dev/nrst9";
char	*exabyte2    = "/dev/nrst0";
char	*exabyte3    = "/dev/nrst1";
char	remote_exabyte[30];
char	*nine_track  = "/dev/rmt12";
Cmd_pkt		*command;
Alter_pkt 	*alter;
int 	tape_drv;
int	sys_started = 0;

char    *prog_name = "TDWR_MENU";

/*-------------------------------------------------------------------------*/
/*
 * prototypes
  */

  void main_menu (int *ncmd);

  void send_packet (void);

  void activate_flags (void);

  void send_change (Alter_pkt *cmd);

  void tape_drive_menu (char *drive);

  void source_menu (void);

  void playback_menu (void);

  void data_rate_menu (int *d_rate);

  void skip_menu (int direction);

  void menu_item (int n, char *item);

  void pause_sys (void);

  int shm_init (int shmem_key);

  int process_exit (int sig);

  void get_def_remote_tape_drive (char *remote_exabyte);

  void get_tapedrive_name (char *remote_drive, char *tape_drive);


/*-------------------------------------------------------------------------*/

int main (int argc, char **argv)

{

int	ncmd;
extern	process_exit();
Global	Glob;

int	go = 1;

	get_def_remote_tape_drive (remote_exabyte);

	command = (Cmd_pkt *) malloc ((unsigned) sizeof (Cmd_pkt));
	alter = (Alter_pkt *) malloc ((unsigned) sizeof (Alter_pkt));
	if (command == NULL || alter == NULL)
	{
		(void) printf ("error in malloc\n");
		exit (0);
	}

   (void) strcpy (command->type, rp7_tape);
/*	(void) strcpy (command->type, rp7_ether);*/

/*    (void) strcpy (command->tape.device, exabyte2);*/
    (void) strcpy (command->tape.device, remote_exabyte);
/*      (void) strcpy (command->tape.device, nine_track); */

    command->tape.rate = 300000;

    command->tape.rewind = 0;
    command->tape.position.direction = 0;


	if ((!get_params (&Glob)))
		exit (2);

	command->flag.caf = Glob.caf;
	command->flag.ctf = Glob.ctf;
	command->flag.cvf = Glob.cvf;
	command->flag.ccv = Glob.ccv;
	command->flag.cv = Glob.cv;
	
	if (shm_init (Glob.cmd_shmem_key))
	{
		(void) fprintf (stderr, "shared memory error");
		return (1);
	}
    (void) signal (SIGINT, ((void (*) ()) process_exit));
	(void) signal (SIGHUP, ((void (*) ()) process_exit));
	(void) signal (SIGPIPE, ((void (*) ()) process_exit));

	do {
		main_menu (&ncmd);
		switch (ncmd)
		{
		case -1:(void) printf ("illegal input - try again\n");
			break;
		case 0:	(void) strcpy (command->type, "STOP");
			send_packet ();
			go = 0;
			break;

		case 1:	send_packet ();
			sys_started = 1;
			break;

		case 2:	source_menu ();
			break;

		case 3:	playback_menu ();
			break;

		case 4: activate_flags ();
			break;

		case 5: pause_sys ();
			break;
		}
	} while (go);

	return (0);

}

/*-------------------------------------------------------------------------*/

void main_menu (int *ncmd)

{
	char	input[10];

	for (;;)
	{
		(void) printf ("\n\n\n\n\n\nMAIN MENU -\n");
		(void) printf ("\n		Current system parameters:\n");
		(void) printf ("\n	Data source: %s\n", command->type);

		(void) printf ("\n	Flag status: Dealias failure = %s\n", 
											command->flag.caf ? "TRUE":"FALSE");
		(void) printf ("	             Point Target    = %s\n", 
											command->flag.ctf ? "TRUE":"FALSE");
		(void) printf ("	  SNR thresholding / clutter = %s\n", 
											command->flag.cvf ? "TRUE":"FALSE");
		(void) printf ("	             Clutter         = %s\n", 
											command->flag.ccv ? "TRUE":"FALSE");
		(void) printf ("	        low PRF invalid gate = %s\n\n", 
											command->flag.cv ? "TRUE":"FALSE");

		if ((strncmp (command->type, "TDWR TAPE", 11) == 0)
			|| (strncmp (command->type, "RP7 TAPE", 7) == 0)) 
		{
			(void) printf ("	Exabyte drive: %s\n",
							 command->tape.device);

			if (command->tape.rewind)
				(void) printf ("        Tape rewound\n");
			else 
				(void) printf ("        Tape not rewound\n");

			if (command->tape.position.change)
				(void) printf ("        Tape repositioned\n");

			(void) printf ("	Data rate:	%d\n\n\n",
							 command->tape.rate);
		}
		else
			(void) printf ("\n\n\n");

    	command->tape.rewind = 0;
		command->tape.position.change = 0;
    	command->tape.position.direction = 0;

		menu_item (0, "STOP system");
		menu_item (1, "START system");
		menu_item (2, "Respecify data SOURCE");	
		menu_item (3, "Modify PLAYBACK parameters");
		menu_item (4, "Activate DATA FLAGS");
		menu_item (5, "PAUSE system\n");


		(void) scanf ("%s", input);
		if (strcmp (input, "0") == 0)
		{
			*ncmd = 0;
			break;
		}
		else if (strcmp (input, "1") == 0)
		{
			*ncmd = 1;
			break;
		}
		else if (strcmp (input, "2") == 0)
		{
			*ncmd = 2;
			break;
		}
		else if (strcmp (input, "3") == 0)
		{
			*ncmd = 3;
			break;
		}
		else if (strcmp (input, "4") == 0)
		{
			*ncmd = 4;
			break;
		}
		else if (strcmp (input, "5") == 0)
		{
			*ncmd = 5;
			break;
		}
		else
			(void) printf ("\n\nUnacceptable input\n\n");
	}
}

/*----------------------------------------------------------------------------*/

void send_packet (void)

{
/*...	get packet to send 
 *	copy command into packet to be sent to the doer's 
 *	send the packet
 */
	int	i;

    for (;;)
	{
		if ((i = send_shm (2, sizeof (Cmd_pkt), (unsigned char *) command)) == -1)
		{
			(void) printf ("send_shm fatal error\n");
			exit (-1);
		}
		if (i <= 3 && i >= 1)
		{
			if (i == 3)
				(void) printf ("buffer is full\n");
			else
				(void) printf ("Client delay trying again\n");
			sleep (2);
		}
		else
		{
			break;
		}
	}

	(void) printf ("\n\n\n\nSending commands: \n\n");
	(void) printf ("	 -> %s\n", command->type);
}

/*-------------------------------------------------------------------------*/
void send_change (Alter_pkt *cmd)

{
	int	i;

    for (;;)
	{
		if ((i = send_shm (2, sizeof (Alter_pkt), (unsigned char *) cmd)) == -1)
		{
			(void) printf ("send_shm fatal error\n");
			exit (-1);
		}
		if (i <= 3 && i >= 1)
		{
			if (i == 3)
				(void) printf ("buffer is full\n");
			else
				(void) printf ("Client delay trying again\n");
			sleep (2);
		}
		else
		{
			break;
		}
	}
	(void) printf ("\n\n\n\nSending command: \n\n");
	(void) printf ("	-> %s\n", cmd->type);

}
/*-------------------------------------------------------------------------*/

void tape_drive_menu (char *drive)

{
char	input[10];

	for (;;)
	{
		(void) printf ("Tape drive menu -\n");
		menu_item (0, exabyte0);
		menu_item (1, exabyte1);
		menu_item (2, exabyte2);
		menu_item (3, exabyte3);
		menu_item (4, "specify a remote exabyte");
		menu_item (5, nine_track);
		menu_item (6, "return");

		(void) scanf ("%s", input);

		if (strcmp (input, "0") == 0)
		{
			(void) strcpy (drive, exabyte0);
			break;
		}
		else if (strcmp (input, "1") == 0)
		{
			(void) strcpy (drive, exabyte1);
			break;
		}
		else if (strcmp (input, "2") == 0)
		{
			(void) strcpy (drive, exabyte2);
			break;
		}
		else if (strcmp (input, "3") == 0)
		{
			(void) strcpy (drive, exabyte3);
			break;
		}
		else if (strcmp (input, "4") == 0)
		{
			get_tapedrive_name (remote_exabyte, drive);
			break;
		}
		else if (strcmp (input, "5") == 0)
		{
			(void) strcpy (drive, nine_track);
			break;
		}
		else if (strcmp (input, "6") == 0)
		{
			break;
		}
		else
			(void) printf ("\n\nUnacceptable input\n\n");


	}
}

/*-------------------------------------------------------------------------*/

void source_menu (void)

{

char	input[10];

	for (;;)
	{

		(void) printf ("Radar data source menu -\n");
		menu_item (0, "Return to Main Menu");
		menu_item (1, rp7_ether);
		menu_item (2, rp7_tape);

		(void) scanf ("%s", input);
		
		if (strcmp (input, "0") == 0)
		{
			return;
		}

		else if (strcmp (input, "1") == 0)
		{
			(void) strcpy (command->type, rp7_ether);
			break;
		}
		else if (strcmp (input, "2") == 0)
		{
			(void) strcpy (command->type, rp7_tape);
			break;
		}
		else
			(void) printf ("\n\nUnacceptable input\n\n");
	}

}


/*-------------------------------------------------------------------------*/

void playback_menu (void)

{
char	input[10];
char	device[20];
int	rate = 0;

	for (;;)
	{
	
		(void) printf ("\n\nModify PLAYBACK parameters -\n");

		menu_item (0, "Return");
		menu_item (1, "Change tape DATA acquisition RATE");
		menu_item (2, "Change tape DRIVE");
		menu_item (3, "REWIND tape");
		menu_item (4, "Space FORWARD files");
		menu_item (5, "Space BACKWARD files");
		menu_item (6, "PAUSE");


		(void) scanf ("%s", input);

		if (strcmp (input, "0") == 0)
		{
			return;
		}

		else if (strcmp (input, "1") == 0)
		{
		    data_rate_menu (&rate);

			if (sys_started)
			{
				(void) strcpy (alter->type, "DATA RATE");
				alter->data.value = rate;
				send_change (alter);
			}

			command->tape.rate = rate;
			break;
		}
		else if (strcmp (input, "2") == 0)
		{
		    tape_drive_menu (device);

			if (*device != 0)
			{
				if (sys_started)
				{
					(void) strcpy (alter->type, "TAPE DRIVE");
					(void) strcpy (alter->data.string, device);
					send_change (alter);
				}
				(void) strcpy (command->tape.device, device);
			}
				
			break;
		}
		else if (strcmp (input, "3") == 0)
		{
			if (sys_started)
			{
				(void) strcpy (alter->type, "REWIND");
				alter->data.value = REWIND;
				send_change (alter);
			}
			command->tape.rewind = 1;
			break;
		}
		else if (strcmp (input, "4") == 0)
		{
		       	skip_menu (FORWARD);
			break;
		}
		else if (strcmp (input, "5") == 0)
		{
		       	skip_menu (BACKWARD);
			break;
		}  
		else if (strcmp (input, "6") == 0)
		{
			if (sys_started)
			{
				(void) strcpy (alter->type, "PAUSE");
				alter->data.value = PAUSE;
				send_change (alter);
			}
			break;
		}  
		else
			(void) printf ("\n\nUnacceptable input\n\n");
	}
}

/*-------------------------------------------------------------------------*/

void data_rate_menu (int *d_rate)

{
char	input[10];

	for (;;)
	{
		(void) printf ("Enter the new data rate\n");
		(void) scanf ("%s", input);
		*d_rate = atoi (input);

		if (*d_rate < 0 || *d_rate > 65000)
		{
			(void) printf ("\n\nUnacceptable input\n\n");
		}
			break;
	}
}

/*-------------------------------------------------------------------------*/

void skip_menu (int direction)

{
char	input[10];
int	num_files;

	for (;;)
	{
		(void) printf ("Enter the number of files to skipped\n");
		(void) scanf ("%s", input);
		num_files = atoi (input);

		if (num_files < 0)
		{
			(void) printf ("\n\nUnacceptable input\n\n");
		}
		else
		{
			if (sys_started)
			{
				if (direction == FORWARD)
					(void) strcpy (alter->type, 
							"SKIP FORWARD");
				else
					(void) strcpy (alter->type, 
							"SKIP BACKWARD");
	
				alter->data.value = num_files;
				send_change (alter);
			}
			command->tape.position.change = num_files;
			command->tape.position.direction = direction;
			break;
		}
	}
}

/*-------------------------------------------------------------------------*/

void menu_item (int n, char *item)


{

	(void) printf ("%3d: %s\n", n, item);

}

/*----------------------------------------------------------------------------*/

void activate_flags (void)
{

char	input[10];

	while (1)
	{

		(void) printf ("\nCurrent Flag status:     Dealias failure = %s\n",
											command->flag.caf ? "TRUE":"FALSE");
		(void) printf ("                         Point Target    = %s\n",
											command->flag.ctf ? "TRUE":"FALSE");
		(void) printf ("                        SNR thresholding = %s\n",
											command->flag.cvf ? "TRUE":"FALSE");
		(void) printf ("                         Clutter         = %s\n",
											command->flag.ccv ? "TRUE":"FALSE");
		(void) printf ("                    low PRF invalid gate = %s\n\n",
											command->flag.cv ? "TRUE":"FALSE");


		menu_item (0, "Return");
		menu_item (1, "CAF - compressed dealias algorithm failure flag");
		menu_item (2, "CTF - compressed Point target filter flag");
		menu_item (3, "CVF - compressed conditioned valid velocity flag (SN)");
		menu_item (4, "CCV - comp conditioned valid flag - clutter");
		menu_item (5, "CV - compressed valid flag - low PRF invalid gate");

		(void) printf ("\n    Press 0 to exit menu\n");

    	(void) scanf ("%s", input);

		if (strcmp (input, "0") == 0)
		{
			return;
		}

		else if (strcmp (input, "1") == 0)
		{
			if (command->flag.caf == TRUE)
				command->flag.caf = FALSE;
			else
				command->flag.caf = TRUE;

			if (sys_started)
			{
				(void) strcpy (alter->type, "CAF FLAG");
				alter->data.value = command->flag.caf;
				send_change (alter);
			}
		}
		else if (strcmp (input, "2") == 0)
		{

			if (command->flag.ctf == TRUE)
				command->flag.ctf = FALSE;
			else
				command->flag.ctf = TRUE;

			if (sys_started)
			{
				(void) strcpy (alter->type, "CTF FLAG");
				alter->data.value = command->flag.ctf;
				send_change (alter);
			}
		}
		else if (strcmp (input, "3") == 0)
		{

			if (command->flag.cvf == TRUE)
				command->flag.cvf = FALSE;
			else
				command->flag.cvf = TRUE;

			if (sys_started)
			{
				(void) strcpy (alter->type, "CVF FLAG");
				alter->data.value = command->flag.cvf;
				send_change (alter);
			}
		}
		else if (strcmp (input, "4") == 0)
		{
			if (command->flag.ccv == TRUE)
				command->flag.ccv = FALSE;
			else
				command->flag.ccv = TRUE;

			if (sys_started)
			{
				(void) strcpy (alter->type, "CCV FLAG");
				alter->data.value = command->flag.ccv;
				send_change (alter);
			}
		}
		else if (strcmp (input, "5") == 0)
		{
			if (command->flag.cv == TRUE)
				command->flag.cv = FALSE;
			else
				command->flag.cv = TRUE;

			if (sys_started)
			{
				(void) strcpy (alter->type, "CV FLAG");
				alter->data.value = command->flag.cv;
				send_change (alter);
			}
		}
		else
			(void) printf ("\n\nUnacceptable input\n\n^G");
	}
}

/*-------------------------------------------------------------------------*/
void pause_sys (void)
{
	if (sys_started)
	{
		(void) strcpy (alter->type, "PAUSE SYS");
		alter->data.value = 0;
		send_change (alter);
	}
}
/*-------------------------------------------------------------------------*/

int shm_init (int shmem_key)

{
	int i;
	unsigned char   buffer[10];

	for (;;)
	{
		if ((i = send_shm (1, shmem_key, &buffer[0])) == -1)
		{
			(void) printf ("send_shm fatal error\n");
			return (1);
		}

		if (i <= 3 && i >= 1)
		{
			if (i == 3)
				(void) printf ("buffer is full\n");
			else
				(void) printf ("Client delay trying again\n");
			sleep (2);
		}
		else
		{
			return (0);
		}
	}
}
/*----------------------------------------------------------------------------*/

int process_exit (int sig)

{
	send_shm (0,0,"    ");
	(void) printf ("%s exited, signal = %d\n", prog_name, sig);
	exit (1);
}

/*-------------------------------------------------------------------------*/

void get_def_remote_tape_drive (char *remote_exabyte)

{
	char    line [MAX_LINE_SZ];
	FILE    *file_ptr;
    char    rmt_tape_file [MAX_NAME_SIZE];
	 
	(void) strcpy (rmt_tape_file, getenv ("REMOTE_TAPE_FILE") ?
					 getenv ("REMOTE_TAPE_FILE") : "remote.tape");

	if ((file_ptr = fopen (rmt_tape_file, "r")) == (FILE *) NULL)
	{
		(void) printf ("no remote tape drive specified \n");
	}
	else
	{
		(void) fgets (line, sizeof (line), file_ptr);     /* skip the comment */
		(void) fgets (line, sizeof (line), file_ptr);
		(void) sscanf (line, "%s", remote_exabyte);
	}

}

/*-------------------------------------------------------------------------*/

void get_tapedrive_name (char *remote_drive, char *tape_drive)

{

int opt, i = 0;
int	newname = FALSE;

	if (strlen (remote_drive) > 0)
	{
		(void) printf ("The DEFAULT remote tape drive is:    %s\n", 
														remote_drive);
		(void) printf ("Enter the new remote tape drive name\n");
		(void) printf ("           or press return to accept the default\n");
	}
	else
	    (void) printf ("Please enter the remote tape drive name: ");


	opt = getchar();

	for (;;)
	{
		opt = getchar();
		if (opt == 10 || opt == 32)
		{
			if (newname) 
				tape_drive[i] = '\0';
			else
				(void) strcpy (tape_drive, remote_exabyte);
			break;
		}
		newname = 1;
		tape_drive[i] = (char ) opt;
		i++;
	}
}

/*-------------------------------------------------------------------------*/

