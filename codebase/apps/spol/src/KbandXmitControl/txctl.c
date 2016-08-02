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
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/time.h>

#include "txmond.h"

void txctl_atexit(void);

static int fd_cmd_fifo = -1;	/* Command fifo, receives commands from
				   client */
static int fd_info_fifo = -1;	/* Command fifo, receives commands from
				   client */

int main(int argc, char *argv[])
{
	char *program_name;

	fd_set rfds;
	int retcode;
	char buf[1024];

	// pid_t daemon_pid;
	// FILE *pid_file_p;

	ssize_t cmd_len;
	int tx_command = CMD_INVALID;

	if ((program_name = strrchr(*argv, '/')) == (char *)NULL)
		program_name = *argv;
	else
		program_name++;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s [command]\n", program_name);
		fprintf(stderr,
			"       command = poweron | poweroff | operate | "
			"standby | reset | status\n");
		exit(EXIT_FAILURE);
	}

	if (!access(PID_FILE, F_OK)) {
		// if ((pid_file_p = fopen(PID_FILE, "r"))) {
		//   fscanf(pid_file_p, "%d", &daemon_pid);
		//   fclose(pid_file_p);
		// }
	} else {
		fprintf(stderr,
			"%s: the transmitter monitoring daemon (txmond) "
			"is not running\n", program_name);
		exit(EXIT_FAILURE);
	}

	if ((fd_cmd_fifo = open(TXMON_CMD_FIFO, O_WRONLY | O_NONBLOCK)) < 0) {
		perror("txctl: unable to open cmd fifo");
		exit(EXIT_FAILURE);
	}

	if ((fd_info_fifo = open(TXMON_INFO_FIFO, O_RDONLY | O_NONBLOCK)) < 0) {
		perror("txctl: unable to open info fifo");
		exit(EXIT_FAILURE);
	}

	atexit(txctl_atexit);

	/* Parse the transmitter command */
	if (!strcmp(argv[1], "poweron"))
		tx_command = CMD_POWERON;
	else if (!strcmp(argv[1], "poweroff"))
		tx_command = CMD_POWEROFF;
	else if (!strcmp(argv[1], "operate"))
		tx_command = CMD_OPERATE;
	else if (!strcmp(argv[1], "standby"))
		tx_command = CMD_STANDBY;
	else if (!strcmp(argv[1], "reset"))
		tx_command = CMD_RESET;
	else if (!strcmp(argv[1], "status"))
		tx_command = CMD_STATUS;

	if (tx_command == CMD_INVALID) {
		fprintf(stderr, "%s command not recognized\n", argv[1]);
		exit(EXIT_FAILURE);
	} else {
		cmd_len = write(fd_cmd_fifo, &tx_command, sizeof(int));
		if (cmd_len < 0) {
			perror("txctl: write error to fifo %s");
			exit(EXIT_FAILURE);
		}
	}

	while (strcmp(buf, "eoc")) {

		FD_ZERO(&rfds);
		FD_SET(fd_info_fifo, &rfds);

		retcode = select(fd_info_fifo + 1, &rfds, NULL, NULL, NULL);
		if (retcode < 0) {
			/* Report an error on waiting for a command on
			   the FIFO */
			fprintf(stderr, "fifo error %s: %m", TXMON_INFO_FIFO);
			exit(EXIT_FAILURE);
		} else {
			if (FD_ISSET(fd_info_fifo, &rfds)) {
				retcode = read(fd_info_fifo, buf, 1024);
				if (retcode < 0) {
					fprintf(stderr,
						"error reading from fifo %s: %m",
						TXMON_INFO_FIFO);
					exit(EXIT_FAILURE);
				} else if (retcode > 0) {
					if (strcmp(buf, "eoc"))
						printf("txmond: %s\n", buf);
				}
			}
		}
	}

	return EXIT_SUCCESS;
}

void txctl_atexit(void)
{
	if (fd_cmd_fifo != -1)
		if (close(fd_cmd_fifo) < 0)
			perror("tuibeui: unable to close cmd fifo");
	if (fd_info_fifo != -1)
		if (close(fd_info_fifo) < 0)
			perror("tuibeui: unable to close info fifo");
}
