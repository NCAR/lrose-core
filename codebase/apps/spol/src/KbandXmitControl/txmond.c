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
/*

  Project     : Ka-band radar
  Filename    : txmond.c
  Description : Ka-band radar transmitter monitoring and command processor
  daemon
  Author      : Gordon Farquharson
  Created     : Jun 2010
  Language    : C
  Compiler    : GNU C Compiler
  OS          : Linux

  Notes       : This program needs to be rewritten to use Unix domain
  sockets instead of named pipes (fifos) as the use of
  named pipes in this program is very limiting. For
  instance, it only supports one txmond client at a
  time.

  -----------------------------------------------------------------------
  Revision information 
  dd.mm.yyyy <name> - <description>
  -----------------------------------------------------------------------

*/

#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

#include <stdbool.h>
#include <strings.h>

#include <getopt.h>

#include "txmond.h"

/* Timeout (approximately in seconds) for txmon operations such as
   power on to complete. */

#define TIMEOUT 1800

/* Global variables */

#define TXMON_STAT_LEN 64 /* STX, 3 bytes (bit coded) + 12 bytes (3 four
                             character strings containing analog
                             voltages), ETX, checksum, plus some extra */

static char tx_status[TXMON_STAT_LEN];

static pid_t pid = -1;
static FILE *logfile;
static int txmon_init_flag = 0;

static struct {
  char device[128];
  time_t interval;	/* Number of seconds between status requests */
} opts = {
  .device = "/dev/ttyS0",.interval = 5,};

static int tty_fd = -1;		/* TTY file descriptor */
static struct termios oldtio, newtio;

static int fd_cmd_read = -1;
static int fd_cmd_write = -1;
static int fd_info_read = -1;
static int fd_info_write = -1;

/* command strings */

static char operate[5] = { 0x02, 0x4F, 0x03, 0x2E, 0 };
static char standby[5] = { 0x02, 0x53, 0x03, 0x2a, 0 };
static char reset[5] = { 0x02, 0x52, 0x03, 0x2b, 0 };
static char poweron[5] = { 0x02, 0x50, 0x03, 0x2d, 0 };
static char poweroff[5] = { 0x02, 0x70, 0x03, 0x0d, 0 };
static char status[5] = { 0x02, 0x57, 0x03, 0x26, 0 };

/* Function declarations */

void parse_args(int, char **);
void show_usage(char *);

void printf_log_mesg(char *, ...);
void printf_client(int, char *, char *, ...);
void sig_handler(int);
void txmond_atexit(void);
void txmond_process_cmd(int, char *);

void print_tx_fault(void);
void txmond_delay(time_t, long);

int tx_if_init();
int tx_if_shutdown();
void tx_if_flush();
int tx_if_send_cmd_str(char *);

int tx_poweron();
int tx_poweroff();
int tx_standby();
int tx_operate();
int tx_reset();
int tx_stat(char *);
static int fd_read_select(int fd, long wait_msecs);

int main(int argc, char *argv[])
{

  char *program_name;

  FILE *pid_file_p;

  int fd_flags;
  struct stat stat_cmd_fifo;
  struct stat stat_info_fifo;

  fd_set rfds;
  struct timeval select_timeout;
  int retcode;

  int txctl_command = CMD_INVALID;
  char info_buf[1024];

  time_t fault_time = -1, last_fault_time = -1;
  int fault_reset = 0;

  if ((program_name = strrchr(*argv, '/')) == (char *)NULL)
    program_name = *argv;
  else
    program_name++;

  parse_args(argc, argv);

  atexit(txmond_atexit);

  if (!access(PID_FILE, R_OK)) {
    if ((pid_file_p = fopen(PID_FILE, "r"))) {
      fscanf(pid_file_p, "%d", &pid);
      fclose(pid_file_p);

      if (!kill(pid, 0) || errno == EPERM) {
        fprintf(stderr,
                "%s is already running as process %d:\n"
                "if it is no longer running, remove %s\n",
                program_name, pid, PID_FILE);
        exit(EXIT_FAILURE);
      }
    }
  }

  /* Need to check that we are running as root */
  if (getuid() != 0) {
    fprintf(stderr, "%s must be run as root\n", program_name);
    exit(EXIT_FAILURE);
  }

  if (signal(SIGINT, SIG_IGN) != SIG_IGN)
    signal(SIGINT, sig_handler);
  if (signal(SIGQUIT, SIG_IGN) != SIG_IGN)
    signal(SIGQUIT, sig_handler);
  if (signal(SIGTERM, SIG_IGN) != SIG_IGN)
    signal(SIGTERM, sig_handler);

  if ((pid = fork())) {
    /* Parent process runs this code. */
    if ((pid_file_p = fopen(PID_FILE, "w"))) {
      fprintf(pid_file_p, "%d\n", pid);
      fclose(pid_file_p);
    } else
      printf("error creating pid file %s\n", PID_FILE);
    exit(EXIT_SUCCESS);	/* Exit the parent process */
  }

  /* Child process starts here */

  if ((logfile = fopen("txmond.log", "a")) == NULL) {
    /* Hmmm... no log file, so how do I display the error
     * message now that we've detacted from the pts? */
    exit(EXIT_FAILURE);
  }

  if (pid < 0) {		/* Error forking */
    fprintf(stderr, "unable to start %s: %m", program_name);
    exit(EXIT_FAILURE);
  }

  /* Child. Follow the rules of daemons in W. Richard
     Stevens. Advanced Programming in the UNIX Environment
     (Addison-Wesley Publishing Co., 1992). Page 417.).  */
  if (setsid() < 0) {
    printf_log_mesg("setsid: %m");
    exit(EXIT_FAILURE);
  }
  if (chdir("/") < 0) {
    printf_log_mesg("chdir: %m");
    exit(EXIT_FAILURE);
  }
  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);
  umask(0);

  /* Daemon main code (from this point on, errors must be logged
     to the log file) */
  printf_log_mesg("%s started", program_name);

  /* Check that we can talk to the transmitter and initialize it */
  if (tx_if_init() == -1)
    exit(EXIT_FAILURE);
  txmon_init_flag = 1;

  /* Send an initial transmitter command (I don't know why one
   * needs to do this) */
  if (tx_reset() == -1) {
    printf_log_mesg("unable to send reset to transmitter");
    exit(EXIT_FAILURE);
  }

  /* Flush the serial port buffer to try to get in sync */
  tx_if_flush();

  /* Make the command fifo */
  if (stat(TXMON_CMD_FIFO, &stat_cmd_fifo) == -1) {
    if (mkfifo(TXMON_CMD_FIFO, S_IRWXU | S_IRWXG | S_IRWXO) < 0) {
      printf_log_mesg("unable to make fifo %s: %m",
                      TXMON_CMD_FIFO);
      exit(EXIT_FAILURE);
    }
  } else {
    /* A file called TXMON_CMD_FIFO exists. Check that is
     * it a fifo */
    if (!S_ISFIFO(stat_cmd_fifo.st_mode)) {
      printf_log_mesg("%s exists and is not a fifo",
                      TXMON_CMD_FIFO);
      exit(EXIT_FAILURE);
    }
  }

  /* This is a trick from Advanced programming in the UNIX
     environment, page 708 no. 14.10. It ensures that an EOF
     does not get generated when the client closes the fifo
     which would make select return immediately causing the
     program to spin. */
  if ((fd_cmd_read = open(TXMON_CMD_FIFO, O_RDONLY | O_NONBLOCK)) < 0) {
    printf_log_mesg("unable to open fifo %s: %m", TXMON_CMD_FIFO);
    exit(EXIT_FAILURE);
  }
  if ((fd_cmd_write = open(TXMON_CMD_FIFO, O_WRONLY)) < 0) {
    printf_log_mesg("unable to open fifo %s: %m", TXMON_CMD_FIFO);
    exit(EXIT_FAILURE);
  }
  if ((fd_flags = fcntl(fd_cmd_read, F_GETFL, 0)) < 0) {
    printf_log_mesg("fcntl (F_GETFL) failed: %m");
    exit(EXIT_FAILURE);
  }
  fd_flags &= ~O_NONBLOCK;
  if ((fd_flags = fcntl(fd_cmd_read, F_SETFL, 0)) < 0) {
    printf_log_mesg("fcntl (F_SETFL) failed: %m");
    exit(EXIT_FAILURE);
  }

  /* Make the info fifo */
  if (stat(TXMON_INFO_FIFO, &stat_info_fifo) == -1) {
    if (mkfifo(TXMON_INFO_FIFO, S_IRWXU | S_IRWXG | S_IRWXO) < 0) {
      printf_log_mesg("unable to make fifo %s: %m",
                      TXMON_INFO_FIFO);
      exit(EXIT_FAILURE);
    }
  } else {
    /* A file called TXMON_INFO_FIFO exists. Check that is
     * it a fifo */
    if (!S_ISFIFO(stat_info_fifo.st_mode)) {
      printf_log_mesg("%s exists and is not a fifo",
                      TXMON_INFO_FIFO);
      exit(EXIT_FAILURE);
    }
  }

  /* Open the info fifo (reading and writing as above) */
  if ((fd_info_read = open(TXMON_INFO_FIFO, O_RDONLY | O_NONBLOCK)) < 0) {
    printf_log_mesg("unable to open fifo %s: %m", TXMON_INFO_FIFO);
    exit(EXIT_FAILURE);
  }
  if ((fd_info_write = open(TXMON_INFO_FIFO, O_WRONLY)) < 0) {
    printf_log_mesg("unable to open fifo %s: %m", TXMON_INFO_FIFO);
    exit(EXIT_FAILURE);
  }
  if ((fd_flags = fcntl(fd_info_read, F_GETFL, 0)) < 0) {
    printf_log_mesg("fcntl (F_GETFL) failed: %m");
    exit(EXIT_FAILURE);
  }
  fd_flags &= ~O_NONBLOCK;
  if ((fd_flags = fcntl(fd_info_read, F_SETFL, 0)) < 0) {
    printf_log_mesg("fcntl (F_SETFL) failed: %m");
    exit(EXIT_FAILURE);
  }

  while (1) {

    FD_ZERO(&rfds);
    FD_SET(fd_cmd_read, &rfds);

    /* Need to reinitialize the timespec structure due to
       the impletmentation of select on Linux. In Linux,
       on return, select modifies the structure to relfect
       the amount of time not slept. */
    select_timeout.tv_sec = opts.interval;
    select_timeout.tv_usec = 0;

    /* Wait until a command is received on the FIFO or
       until the specified number of seconds (interval)
       elapses */
    retcode =
      select(fd_cmd_read + 1, &rfds, NULL, NULL, &select_timeout);
    if (retcode < 0) {
      /* Report an error on waiting for a command on
       * the FIFO */
      printf_log_mesg("fifo error %s: %m", TXMON_CMD_FIFO);
      exit(EXIT_FAILURE);
    } else if (retcode == 0) {
      /* select() timed out (i.e. no command
         received) so check that the tranmitter has not
         faulted */
      if ((tx_stat(tx_status) == 0) && 
          (tx_status[1] & FAULT_SUM)) {
        fault_time = time((time_t *) NULL);
        print_tx_fault();
        if (last_fault_time < 0)
          last_fault_time = fault_time;
        if (difftime(fault_time, last_fault_time) > 10) {
          /* More than 10 seconds
           * elapsed betweem this and
           * the last fault */
          if (tx_reset() == -1)
            exit(EXIT_FAILURE);
          fault_reset = 1;
        } else {
          /* Only reset the transmitter
           * 3 times */
          if (fault_reset < 3) {
            if (tx_reset() == -1)
              exit(EXIT_FAILURE);
            fault_reset++;
          }
        }
        last_fault_time = fault_time;
      }
    } else {
      /* Received a command from the client program. */
      if (FD_ISSET(fd_cmd_read, &rfds)) {
        retcode =
          read(fd_cmd_read, &txctl_command,
               sizeof(int));
        if (retcode < 0) {
          printf_log_mesg
            ("error reading from fifo %s: %m",
             TXMON_CMD_FIFO);
          exit(EXIT_FAILURE);
        } else if (retcode > 0) {
          txmond_process_cmd(txctl_command,
                             info_buf);
        }	/* if (retcode > 0) for read() */
      }	/* if (FD_ISSET()) */
      printf_client(fd_info_write, info_buf, "eoc");

    }		/* else (retcode > 0) for select() */
  }			/* while (1) */
}

void parse_args(int argc, char *argv[])
{
  static struct option longOptions[] = {
    {"interval", required_argument, 0, 'i'},
    {"help", no_argument, 0, 'h'},
    {0, 0, 0, 0}
  };

  while (1) {
    int optionIndex = 0;
    int c;

    /* no default error messages printed. */
    opterr = 0;

    c = getopt_long(argc, argv, "hi:", longOptions, &optionIndex);

    if (c < 0)
      break;

    switch (c) {
      case 'i':
        opts.interval = atoi(optarg);
        break;
      case 'h':
        show_usage(argv[0]);
        exit(EXIT_SUCCESS);
      case '?':
      default:
        fprintf(stderr, "Unrecognized option.\n");
        fprintf(stderr, "Run with '--help'.\n");
        exit(EXIT_FAILURE);
    }
  }			/* while */

  if ((argc - optind) < 1) {
    fprintf(stderr, "No port given\n");
    exit(EXIT_FAILURE);
  }
  strncpy(opts.device, argv[optind], sizeof(opts.device) - 1);
  opts.device[sizeof(opts.device) - 1] = '\0';
}

void show_usage(char *name)
{
  char *s;

  s = strrchr(name, '/');
  s = s ? s + 1 : name;

  printf("txmond\n");
  printf("Usage is: %s [options] <tty device>\n", s);
  printf("Options are:\n");
  printf("  --<i>polling interval\n");
  printf("  --<h>elp\n");
  printf("<?> indicates the equivalent short option.\n");
  printf("Short options are prefixed by \"-\" instead of by \"--\".\n");
}

void printf_log_mesg(char *fmt, ...)
{
  va_list args;
  char logmesg[1024];
  char timestamp[16];	/* e.g. Jun 18 09:09:56 */
  time_t unixtime;
  struct tm *ltime;

  unixtime = time(NULL);
  ltime = localtime(&unixtime);
  strftime(timestamp, 16, "%b %d %H:%M:%S", ltime);

  va_start(args, fmt);
  vsnprintf(logmesg, 1024, fmt, args);

  fprintf(logfile, "%s %s\n", timestamp, logmesg);
  fflush(logfile);

  va_end(args);
}

void printf_client(int fd, char *buf, char *fmt, ...)
{
  va_list args;

  va_start(args, fmt);
  vsprintf(buf, fmt, args);	/* Should really use vsnprintf */
  if (write(fd, buf, 1024) < 0) {
    printf_log_mesg("unable to write to %s: %m", TXMON_INFO_FIFO);
  }
  va_end(args);
}

void sig_handler(int sig)
{
  /* I should probably write a log message here, but I'm too
     lazy. */
  exit(EXIT_SUCCESS);
}

void txmond_atexit(void)
{
  if (pid == 0) {

    /* Clean up stuff for the child process. */

    if (fd_cmd_write != -1) {
      if (close(fd_cmd_write) < 0)
        printf_log_mesg("unable to close %s: %m",
                        TXMON_CMD_FIFO);
    }
    if (fd_cmd_read != -1) {
      if (close(fd_cmd_read) < 0)
        printf_log_mesg("unable to close %s: %m",
                        TXMON_CMD_FIFO);
      else if (remove(TXMON_CMD_FIFO) < 0)
        printf_log_mesg("unable to remove %s: %m",
                        TXMON_CMD_FIFO);
    }

    if (fd_info_write != -1) {
      if (close(fd_info_write) < 0)
        printf_log_mesg("unable to close %s: %m",
                        TXMON_INFO_FIFO);
    }
    if (fd_info_read != -1) {
      if (close(fd_info_read) < 0)
        printf_log_mesg("unable to close %s: %m",
                        TXMON_INFO_FIFO);
      else if (remove(TXMON_INFO_FIFO) < 0)
        printf_log_mesg("unable to remove %s: %m",
                        TXMON_INFO_FIFO);
    }

    if (txmon_init_flag == 1) {
      if (tx_if_shutdown() != 0)
        exit(EXIT_FAILURE);
      txmon_init_flag = 0;
    }

    unlink(PID_FILE);

    printf_log_mesg("txmond exiting");
    exit(EXIT_SUCCESS);
  }
}

void txmond_process_cmd(int cmd, char *info_buf)
{
  volatile long int i;

  if (cmd == CMD_INVALID) {
    printf_log_mesg("transmitter command (%d) not recognized", cmd);
    printf_client(fd_info_write,
                  info_buf,
                  "transmitter command (%d) not recognized\n", cmd);
  } else if ((tx_stat(tx_status) == 0) && (cmd == CMD_STATUS)) {
    printf_log_mesg("transmitter status bytes: 0x%x 0x%x 0x%x ",
                    tx_status[1], tx_status[2], tx_status[3]);
    printf_client(fd_info_write,
                  info_buf,
                  "transmitter status bytes: 0x%x 0x%x 0x%x ",
                  tx_status[1], tx_status[2], tx_status[3]);
  } else if ((tx_stat(tx_status) == 0) && (tx_status[2] & REMOTE_MODE)) {
    switch (cmd) {

      case CMD_POWERON:

        printf_log_mesg("power on command received");
        printf_client(fd_info_write,
                      info_buf, "poweron command received");
        if ((tx_status[1] & UNIT_ON) == 0) {
          if (tx_poweron() == -1)
            exit(EXIT_FAILURE);
          /* Wait until the transmitter has turned on */
          i = 0;
          do {
            /* txmond_delay(1, 0); */
            if (tx_stat(tx_status) == -1) {
              printf_log_mesg("aborting command");
              printf_client(fd_info_write,
                            info_buf,
                            "aborting command");
              goto out;
            }
          } while (((tx_status[1] & UNIT_ON) == 0)
                   && (i++ < TIMEOUT));
          /* If the transmitter has faulted,
           * return an error */
          if (tx_status[1] & FAULT_SUM)
            print_tx_fault();
          else if (tx_status[1] & UNIT_ON) {
            /* Wait until the transmitter
             * has warmed up */
            printf_log_mesg
              ("transmitter warm up in progress");
            printf_client(fd_info_write, info_buf,
                          "transmitter warm up in progress");
            i = 0;
            do {
/*               txmond_delay(1, 0); */
              if (tx_stat(tx_status) == -1) {
                printf_log_mesg("aborting command");
                printf_client(fd_info_write,
                              info_buf,
                              "aborting command");
                goto out;
              }
            } while ((tx_status[1] & WARMUP)
                     && (i++ < TIMEOUT));

            if (tx_stat(tx_status) == -1) {
              printf_log_mesg("aborting command");
              printf_client(fd_info_write,
                            info_buf,
                            "aborting command");
              goto out;
            }
            if (tx_status[1] & STANDBY) {
              printf_log_mesg
                ("warm up complete, transmitter in standby");
              printf_client(fd_info_write,
                            info_buf,
                            "warm up complete, transmitter in standby");
            } else {
              printf_log_mesg
                ("warm up complete, transmitter did not switch to standby");
              printf_client(fd_info_write,
                            info_buf,
                            "warm up complete, transmitter did not switch to standby");
            }
          } else {
            printf_log_mesg
              ("transmitter did not turn on");
            printf_client(fd_info_write, info_buf,
                          "transmitter did not turn on");
          }
        } else {
          printf_log_mesg("transmitter is already on");
          printf_client(fd_info_write,
                        info_buf,
                        "transmitter is already on");
        }
        break;

      case CMD_POWEROFF:

        printf_log_mesg("power off command received");
        printf_client(fd_info_write,
                      info_buf, "poweroff command received");
        if (tx_status[1] & UNIT_ON) {
          /* Switch the transmitter to standby
           * if is not in standby already */
          if (tx_status[1] & STANDBY) {
            if (tx_standby() == -1)
              exit(EXIT_FAILURE);
            /* Wait until the transmitter
             * switches to standby */
            i = 0;
            do {
/*               txmond_delay(1, 0); */
              if (tx_stat(tx_status) == -1) {
                printf_log_mesg("aborting command");
                printf_client(fd_info_write,
                              info_buf,
                              "aborting command");
                goto out;
              }
            } while (((tx_status[1] & STANDBY) == 0)
                     && (i++ < TIMEOUT));
          }

          if (tx_status[1] & STANDBY) {
            if (tx_poweroff() == -1)
              exit(EXIT_FAILURE);
            /* Wait until the transmitter
             * switches to cooldown */
            printf_log_mesg
              ("transmitter cool down in progress");
            printf_client(fd_info_write, info_buf,
                          "transmitter cool down in progress");
            i = 0;
            do {
/*               txmond_delay(1, 0); */
              if (tx_stat(tx_status) == -1) {
                printf_log_mesg("aborting command");
                printf_client(fd_info_write,
                              info_buf,
                              "aborting command");
                goto out;
              }
            } while (((tx_status[1] & COOLDOWN) ==
                      0)
                     && (i++ < TIMEOUT));
            if (tx_status[1] & COOLDOWN) {
              /* Wait until the
               * transmitter has
               * finished cooling
               * down */
              i = 0;
              do {
/*                 txmond_delay(1, 0); */
                if (tx_stat(tx_status) == -1) {
                  printf_log_mesg("aborting command");
                  printf_client(fd_info_write,
                                info_buf,
                                "aborting command");
                  goto out;
                }
              } while ((tx_status[1] &
                        COOLDOWN)
                       && (i++ < TIMEOUT));
              printf_log_mesg
                ("cool down complete, transmitter off");
              printf_client(fd_info_write,
                            info_buf,
                            "cool down complete, transmitter off");
            } else {
              printf_log_mesg
                ("transmitter in standby but did not start cool down");
              printf_client(fd_info_write,
                            info_buf,
                            "transmitter in standby but did not start cool down");
            }
          }
        } else {
          printf_log_mesg("transmitter is already off");
          printf_client(fd_info_write,
                        info_buf,
                        "transmitter is already off");
        }
        break;

      case CMD_STANDBY:

        printf_log_mesg("standby command received");
        printf_client(fd_info_write,
                      info_buf, "standby command received");
        if ((tx_status[1] & UNIT_ON)
            && ((tx_status[1] & STANDBY) == 0)) {
          if (tx_standby() == -1)
            exit(EXIT_FAILURE);
          /* Wait until the transmitter switches
           * to standby */
          i = 0;
          do {
/*             txmond_delay(1, 0); */
            if (tx_stat(tx_status) == -1) {
              printf_log_mesg("aborting command");
              printf_client(fd_info_write,
                            info_buf,
                            "aborting command");
              goto out;
            }
          } while (((tx_status[1] & STANDBY) == 0)
                   && (i++ < TIMEOUT));
          if (tx_status[1] & FAULT_SUM)
            print_tx_fault();
          else if (tx_status[1] & STANDBY) {
            printf_log_mesg
              ("transmitter in standby");
            printf_client(fd_info_write, info_buf,
                          "transmitter in standby");
          } else {
            printf_log_mesg
              ("transmitter did not switch to standby");
            printf_client(fd_info_write, info_buf,
                          "transmitter did not switch to standby");
          }
        } else if ((tx_status[1]
                    & UNIT_ON) == 0) {
          printf_log_mesg("transmitter is off");
          printf_client(fd_info_write,
                        info_buf, "transmitter is off");
        } else {
          printf_log_mesg
            ("transmitter already in standby");
          printf_client(fd_info_write, info_buf,
                        "transmitter already in standby");
        }
        break;

      case CMD_OPERATE:

        printf_log_mesg("operate command received");
        printf_client(fd_info_write,
                      info_buf, "operate command received");
        if (tx_status[1] & STANDBY) {
          if (tx_operate() == -1)
            exit(EXIT_FAILURE);
          /* Wait until the HVPS runup is
           * complete */
          i = 0;
          do {
/*             txmond_delay(1, 0); */
            if (tx_stat(tx_status) == -1) {
              printf_log_mesg("aborting command");
              printf_client(fd_info_write,
                            info_buf,
                            "aborting command");
              goto out;
            }
          } while (((tx_status[1] & HV_RUNUP) == 0)
                   && (i++ < TIMEOUT));
          if (tx_status[1] & FAULT_SUM)
            print_tx_fault();
          else if (tx_status[1] & HV_RUNUP) {
            printf_log_mesg
              ("transmitter in operate");
            printf_client(fd_info_write, info_buf,
                          "transmitter in operate");
          } else {
            printf_log_mesg
              ("transmitter did not switch to operate");
            printf_client(fd_info_write, info_buf,
                          "transmitter did not switch to operate");
          }
        } else if ((tx_status[1] & UNIT_ON) == 0) {
          printf_log_mesg("transmitter is off");
          printf_client(fd_info_write,
                        info_buf, "transmitter is off");
        } else {
          printf_log_mesg
            ("transmitter already in operate");
          printf_client(fd_info_write, info_buf,
                        "transmitter already in operate");
        }
        break;

      case CMD_RESET:

        printf_log_mesg("reset command received");
        printf_client(fd_info_write,
                      info_buf, "reset command received");
        if (tx_status[1] & UNIT_ON) {
          if (tx_reset() == -1)
            exit(EXIT_FAILURE);
          /* Wait until the fault summary bit is
           * cleared */
          i = 0;
          do {
/*             txmond_delay(1, 0); */
            if (tx_stat(tx_status) == -1) {
              printf_log_mesg("aborting command");
              printf_client(fd_info_write,
                            info_buf,
                            "aborting command");
              goto out;
            }
          } while ((tx_status[1] & FAULT_SUM)
                   && (i++ < TIMEOUT));
          if (tx_status[1] & FAULT_SUM)
            print_tx_fault();
          else if (tx_status[1] & STANDBY) {
            printf_log_mesg
              ("transmitter reset complete; "
               "transmitter in standby");
            printf_client(fd_info_write, info_buf,
                          "transmitter reset complete; "
                          "transmitter in standby");
          } else {
            printf_log_mesg
              ("transmitter reset failed");
            printf_client(fd_info_write, info_buf,
                          "transmitter reset failed");
          }
        } else {
          printf_log_mesg("transmitter is off");
          printf_client(fd_info_write,
                        info_buf, "transmitter is off");
        }
        break;

      default:
        printf_log_mesg("unrecognized command (%d) received",
                        cmd);
        printf_client(fd_info_write,
                      info_buf,
                      "unrecognised command (%d) received",
                      cmd);
        break;
    }		/* switch(cmd) */

  } else {
    printf_log_mesg("transmitter command not processed "
                    "(transmitter in local mode)");
    printf_client(fd_info_write,
                  info_buf,
                  "transmitter command unprocessed "
                  "(transmitter in local mode)");
  }
 out:
  ; /* To make the compiler happy */
}

void print_tx_fault()
{
  if (tx_status[2] & REV_POWER) {
    printf_log_mesg("Reverse power fault detected");
  }
  if (tx_status[2] & SAFETY_INLCK) {
    printf_log_mesg("Safety interlock fault detected");
  }
  if (tx_status[2] & BLOWER) {
    printf_log_mesg("Blower fault detected");
  }
  if (tx_status[2] & MAG_AV_CUR) {
    printf_log_mesg("Magnetron average current fault detected");
  }
  if (tx_status[3] & HVPS_OV) {
    printf_log_mesg("HVPS over voltage fault detected");
  }
  if (tx_status[3] & HVPS_UV) {
    printf_log_mesg("HVPS under voltage fault detected");
  }
  if (tx_status[3] & WG_PRESSURE) {
    printf_log_mesg("Waveguide pressure fault detected");
  }
  if (tx_status[3] & HVPS_OC) {
    printf_log_mesg("HVPS over current fault detected");
  }
  if (tx_status[3] & PIP) {
    printf_log_mesg("Pulse input protection fault detected");
  }
}

void txmond_delay(time_t sec, long nsec)
{
  struct timespec td;
  struct timespec tr;

  td.tv_sec = sec;
  td.tv_nsec = nsec;
  nanosleep(&td, &tr);
}

int tx_if_init()
{
  tty_fd = open(opts.device, O_RDWR | O_NOCTTY | O_NDELAY);
  if (tty_fd < 0) {
    printf_log_mesg("opening %s: %m", opts.device);
    return -1;
  }

  tcgetattr(tty_fd, &oldtio);	/* save current port settings */

  tcgetattr(tty_fd, &newtio);

  cfsetispeed(&newtio, B9600);
  cfsetospeed(&newtio, B9600);

  newtio.c_cflag |= CLOCAL | CREAD;

  newtio.c_cflag &= ~PARENB;
  newtio.c_cflag &= ~CSTOPB;
  newtio.c_cflag &= ~CSIZE;
  newtio.c_cflag |= CS8;

  newtio.c_cflag &= ~CRTSCTS;

  newtio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

  newtio.c_iflag &= ~(IXON | IXOFF | IXANY);

  newtio.c_oflag &= ~OPOST;

  newtio.c_cc[VTIME] = 0;	/* inter-character timer */
  newtio.c_cc[VMIN] = 0;	/* blocking read until 5 chars received */

  tcsetattr(tty_fd, TCSAFLUSH, &newtio);

  return 0;
}

int tx_if_shutdown()
{
  tcsetattr(tty_fd, TCSANOW, &oldtio);
  close(tty_fd);
  return 0;
}

void tx_if_flush()
{
  tcflush(tty_fd, TCIFLUSH);
}

int tx_if_send_cmd(char *cmd)
{
  int len, ret;
  len = strlen(cmd);
  if ((ret = write(tty_fd, cmd, len)) != len) {
    printf_log_mesg("send cmd (write): %m");
    return -1;
  }
  return 0;
}

int tx_poweron()
{
  return tx_if_send_cmd(poweron);
}

int tx_poweroff()
{
  return tx_if_send_cmd(poweroff);
}

int tx_standby()
{
  return tx_if_send_cmd(standby);
}

int tx_operate()
{
  return tx_if_send_cmd(operate);
}

int tx_reset()
{
  return tx_if_send_cmd(reset);
}

int tx_stat(char *stat)
{
  int ret;
  int i;
  unsigned char chksum = 0;
  int etx_found;
  int nbytes_read;

  if (tx_if_send_cmd(status) != 0)
    return -1;

  /* wait for up to 1 second for a response */

  if (fd_read_select(tty_fd, 1000) != 1) {
    printf_log_mesg("unable to read transmitter status (%m)");
    tx_if_flush();
    return -1;
  }

  /* txmond_delay(1, 0); */

  /* read bytes into txmon status, looking for ETX */

  memset(stat, 0, TXMON_STAT_LEN);
  etx_found = 0;
  nbytes_read = 0;
  for (i = 0; i < TXMON_STAT_LEN; i++) {
    /* check we have data availble */
    if (fd_read_select(tty_fd, 20) != 1) {
      printf_log_mesg("unable to read transmitter status (%m)");
      tx_if_flush();
      return -1;
    }
    char cc;
    if ((ret = read(tty_fd, &cc, 1)) == -1) {
      printf_log_mesg("unable to read transmitter status (%m)");
      tx_if_flush();
      return -1;
    }
    stat[i] = cc;
    nbytes_read++;
    if (cc == ETX) {
      etx_found = 1;
      break;
    }
  }
  
  if (!etx_found) {
    printf_log_mesg("unable to read transmitter status (%m)");
    tx_if_flush();
    return -1;
  }

#ifdef DEBUG
  printf_log_mesg("read %d bytes", nbytes_read);
  for (int i = 0; i < nbytes_read; i++)
    printf_log_mesg("%02x", stat[i]);
#endif

  /* Check the checksum of the status message. */
  for (i = 1; i < nbytes_read; i++)
    chksum += stat[i];

  printf_log_mesg("chksum: %d\n", chksum);
  
/*   if (chksum & 0x7f) { */
/*     printf_log_mesg("unable to read transmitter status (checksum error)"); */
/*     tx_if_flush(); */
/*     return -1; */
/*   } */

  return 0;
}

int tx_stat2(char *stat)
{
  int ret;
  int i;
  unsigned char chksum = 0;

  if (tx_if_send_cmd(status) != 0)
    return -1;
  txmond_delay(1, 0);
  if ((ret = read(tty_fd, stat, TXMON_STAT_LEN)) == -1) {
    printf_log_mesg("unable to read transmitter status (%m)");
    tx_if_flush();
    return -1;
  }

#ifdef DEBUG
  printf_log_mesg("read %d bytes", ret);
  for (int i = 0; i < TXMON_STAT_LEN; i++)
    printf_log_mesg("%02x", stat[i]);
#endif

  if (ret == 0) {
    printf_log_mesg("unable to read transmitter status (read 0 bytes)");
    tx_if_flush();
    return -1;
  }

  /* Check the checksum of the status message. */
  for (i = 1; i < TXMON_STAT_LEN; i++)
    chksum += stat[i];
  if (chksum & 0x7f) {
    printf_log_mesg("unable to read transmitter status (checksum error)");
    tx_if_flush();
    return -1;
  }

  return 0;

}

/***************************************************************
 * fd_read_select - waits for read access on a file descriptor
 *
 * returns 1 on success, -1 on timeout, -2 on failure
 *
 * Blocks if wait_msecs == -1
 */

static int fd_read_select(int fd, long wait_msecs)

{
  
  int ret, maxfdp1;
  fd_set read_fd;
  
  struct timeval wait;
  struct timeval * waitp;
  
  waitp = &wait;
  
  /*
   * check only on fd file descriptor
   */

  FD_ZERO(&read_fd);
  FD_SET(fd, &read_fd);
  maxfdp1 = fd + 1;
  
 again:

  /*
   * set timeval structure
   */
  
  if (-1 == wait_msecs) {
    waitp = NULL;
  } else {
    wait.tv_sec = wait_msecs / 1000;
    wait_msecs -= wait.tv_sec * 1000;
    wait.tv_usec = wait_msecs * 1000;
  }
  
  errno = 0;
  if (0 > (ret = select(maxfdp1, &read_fd, NULL, NULL, waitp))) {

      if (errno == EINTR) /* system call was interrupted */
	goto again;
      
      fprintf(stderr,"Read select failed on server %d; error = %d\n",
	      fd, errno);
      return -2; /* select failed */

    } 
  
  if (ret == 0) {
    return (-1); /* timeout */
  }
  
  return (1);

}

