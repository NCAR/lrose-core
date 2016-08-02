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
/***************************************************************************
 * track_client.c
 *
 * Tests the track server
 *
 * Mike Dixon
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * March 1992
 *
 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netinet/in.h>

#include <toolsa/umisc.h>
#include <toolsa/sockutil.h>
#include <toolsa/xdru.h>
#include <titan/radar.h>
#include <titan/storm.h>
#include <titan/track.h>

#define TDATA_USE_COMPLETE_MODE

#include <titan/tdata_index.h>

#define PORT 70000
#define WAIT_SECS 1000
#define STDIN_FD 0

int tidy_and_exit();
int verify_connect();
int request_notify();
int stop_notify();
int request_data();
int hangup();

extern int read_complete_data(int sockfd, int print_on);
extern int read_basic_data(int sockfd, int print_on);
static void parse_args(int argc, char **argv);

static char hostname[256];
static int debug;
static int port;
static int print_on = TDATA_TRUE;
static si32 request_num = 0;

int main(argc, argv)
     
     int argc;
     char **argv;
     
{
  
  char ans_str[32];
  char *end_pt;
  
  int sockfd;
  int done = FALSE;
  int mode, source;
  
  si32 ans;
  si32 time_margin;
  si32 track_num;
  
  date_time_t time;
  
  tdata_reply_t reply;
  tdata_request_t request;
  
  struct timeval wait_time;
  fd_set read_fd;
  
  /*
   * set some defaults
   */
  
  mode = TDATA_COMPLETE;
  source = TDATA_ARCHIVE;
  track_num = TDATA_ALL_AT_TIME;
  
  time.year = 1991;
  time.month = 6;
  time.day = 21;
  time.hour = 21;
  time.min = 0;
  time.sec = 0;
  uconvert_to_utime(&time);
  
  time_margin = 300;
  
  /*
   * register function to trap termination and interrupts
   */
  
  PORTsignal(SIGQUIT, (void (*)())tidy_and_exit);
  PORTsignal(SIGTERM, (void (*)())tidy_and_exit);
  PORTsignal(SIGINT, (void (*)())tidy_and_exit);
  PORTsignal(SIGHUP, (void (*)())tidy_and_exit);

  parse_args(argc, argv);
  
  /*
   * connect to the server
   */
  
  goto first_pass;
  
 reconnect:
  
  return (-1);
  
 first_pass:
  
  if((sockfd = SKU_open_client(hostname, port)) < 0) {
    fprintf(stderr,"Couldn't Begin Socket Operations\n");
    return(-1);
  }
  
  /*
   * verify the connection
   */
  
  verify_connect(sockfd, &request, &reply);
  
  while (!done) {
    
    /*
     * main menu
     */
    
    printf("\n");
    printf("Enter :\n");
    printf("         1 to request notification\n");
    printf("         2 to stop notification\n");
    printf("         3 to use old headers\n");
    printf("         4 to use new headers\n");
    printf("         5 to set mode = TDATA_COMPLETE\n");
    printf("         6 to set mode = TDATA_BASIC_ONLY\n");
    printf("         7 to set source = TDATA_ARCHIVE\n");
    printf("         8 to set source = TDATA_REALTIME\n");
    printf("         9 to set time\n");
    printf("        10 to set time margin\n");
    printf("        11 to set track num\n");
    printf("        12 to set max message len\n");
    printf("        13 to request data\n");
    
    if (print_on)
      printf("        14 to toggle print option (now on)\n");
    else
      printf("        14 to toggle print option (now off)\n");
    
    printf("        15 to hangup and quit\n");
    printf(".........? ");
    fflush(stdout);
    
    /*
     * set wait time for select to time out
     */
    
    wait_time.tv_usec = 0;
    wait_time.tv_sec = WAIT_SECS;
  
    /*
     * hang on a select, which will return as soon as there
     * is something to be read on the socket, or from standard
     * input
     */
    
    FD_ZERO(&read_fd);
    FD_SET(STDIN_FD, &read_fd);
    FD_SET(sockfd, &read_fd);
    
    select(FD_SETSIZE, &read_fd,
	   (fd_set *)0, (fd_set *)0, &wait_time);
    
    if (FD_ISSET(sockfd, &read_fd)) {
      
      /*
       * server wants to notify client that data is available
       */
      
      if (read_notify(sockfd)) {
	
	fprintf(stderr, "ERROR - track_client\n");
	fprintf(stderr, "Reading data notification\n");
	SKU_close (sockfd);
	goto reconnect;
	
      }
      
      printf("New data available\n");
      
    } else if (FD_ISSET(STDIN_FD, &read_fd)) {
      
      /*
       * menu input
       */
      
      if(fgets(ans_str, 32, stdin) == NULL)
	continue;
      
      errno = 0;
      ans = strtol(ans_str, &end_pt, 10);
      
      if (errno)
	continue;
      
      switch (ans) {
	
      case 1:
	
	if (request_notify(sockfd, &request, &reply)) {
	  SKU_close (sockfd);
	  goto reconnect;
	}
	break;
	
      case 2:
	
	if (stop_notify(sockfd, &request, &reply)) {
	  SKU_close (sockfd);
	  goto reconnect;
	}
	break;
	
      case 3:
	
	SKU_set_headers_to_old();
	break;
	
      case 4:
	
	SKU_set_headers_to_new();
	break;
	
      case 5:
	
	mode = TDATA_COMPLETE;
	break;
	
      case 6:
	
	mode = TDATA_BASIC_WITH_PARAMS;
	break;
	
      case 7:
	
	source = TDATA_ARCHIVE;
	break;
	
      case 8:
	
	source = TDATA_REALTIME;
	break;
	
      case 9:
	
	printf("Enter time (yyyy/mm/dd_hh:mm:ss) : ");
	
	if(fgets(ans_str, 32, stdin) == NULL)
	  continue;
	
	if(sscanf(ans_str, "%4ld/%2ld/%2ld_%2ld:%2ld:%2ld",
		  &time.year,
		  &time.month,
		  &time.day,
		  &time.hour,
		  &time.min,
		  &time.sec) != 6)
	  continue;
	
	uconvert_to_utime(&time);
	
	break;
	
      case 10:
	
	printf("Enter time margin (secs) : ");
	
	if(fgets(ans_str, 32, stdin) == NULL)
	  continue;
	
	errno = 0;
	time_margin = strtol(ans_str, &end_pt, 10);
	
	if (errno)
	  continue;
	
	break;
	
      case 11:
	
	printf("Enter track num (%d for all in window, %d for all in ops) : ",
	       TDATA_ALL_AT_TIME, TDATA_ALL_IN_FILE);
	
	if(fgets(ans_str, 32, stdin) == NULL)
	  continue;
	
	errno = 0;
	track_num = strtol(ans_str, &end_pt, 10);
	
	if (errno)
	  continue;
	
	break;
	
      case 12:
	
	printf("Enter max message len: ");
	
	if(fgets(ans_str, 32, stdin) == NULL)
	  continue;
	
	errno = 0;
	request.max_message_len = strtol(ans_str, &end_pt, 10);
	
	if (errno)
	  continue;
	
	if (set_max_message_len(sockfd, &request, &reply)) {
	  SKU_close (sockfd);
	  goto reconnect;
	}
	
	break;
	
      case 13:
	
	request.mode = mode;
	request.source = source;
	request.track_set = track_num;
	request.time = time.unix_time;
	request.time_margin = time_margin;
	if (request_data(sockfd, &request, &reply)) {
	  SKU_close (sockfd);
	  goto reconnect;
	}
	break;
	
      case 14:
	
	if (print_on)
	  print_on = TDATA_FALSE;
	else
	  print_on = TDATA_TRUE;
	
	break;
	
      case 15:
	
	hangup(sockfd, &request, &reply);
	SKU_close(sockfd);
	done = TRUE;
	break;
	
      } /* switch */
      
    } /* if (FD_ISSET ... */
    
  } /* while */
  
  return (0);
  
}

/*****************************************************************
 * parse_args: parse command line arguments. Set option flags
 * And print usage info if necessary
 */

static void parse_args(int argc, char **argv)
     
{
  
  int error_flag = FALSE;
  int i;
  
  char usage[BUFSIZ];
  char *end_pt;
  
  /*
   * check the # of args
   */
  
  sprintf(usage, "%s%s%s%s%s%s%s%s%d%s%s",
	  "Usage:\n\n", argv[0], " [options]\n\n",
	  "options:\n",
	  "         [ --, -help, -man] produce this list\n",
	  "         [ -debug] set debugging on\n",
	  "         [ -h host (default local)] set mode\n",
	  "         [ -p port (default ", PORT, ")]\n",
	  "\n");
  
  /*
   * set defaults
   */
  
  debug = TDATA_FALSE;
  ustrncpy(hostname, PORThostname(), 256);
  port = PORT;
  
  /*
   * search for command options
   */
  
  for (i =  1; i < argc; i++) {
    
    if (!strcmp(argv[i], "--") ||
	!strcmp(argv[i], "-help") ||
	!strcmp(argv[i], "-man")) {
      
      fprintf(stderr, "%s", usage);
      tidy_and_exit(0);
      
    } else if (!strcmp(argv[i], "-debug")) {
      
      debug = TDATA_TRUE;
      
    } else if (!strcmp(argv[i], "-h")) {
      
      if (i < argc - 1)
	strcpy(hostname, argv[i+1]);
      else
	error_flag = TDATA_TRUE;
      
    } else if (!strcmp(argv[i], "-p")) {
      
      if (i < argc - 1) {
	errno = 0;
	port = strtol(argv[i+1], &end_pt, 10);
	if (errno)
	  error_flag = TDATA_TRUE;
      } else {
	error_flag = TRUE;
      }
      
    }
    
  } /* i */
  
  /*
   * check for errors
   */
  
  if(error_flag) {
    fprintf(stderr, "%s\n", usage);
    exit(-1);
  }
  
}

/***********************************************************************
 *
 * verify_connect()
 *
 */

int verify_connect(sockfd, request, reply)
     
     int sockfd;
     tdata_request_t *request;
     tdata_reply_t *reply;
     
{
  
  request->request = BE_from_si32((ui32) TDATA_VERIFY_CONNECT);
  
  if (SKU_writeh(sockfd, (char *) request,
		 (si32) sizeof(tdata_request_t),
		 (si32) TDATA_REQUEST_PACKET_ID,
		 request_num) < 0) {
    
    fprintf(stderr, "ERROR - track_client\n");
    fprintf(stderr, "Could not verify_connect.\n");
    return (-1);
    
  }
  
  request_num++;
  
  if (read_reply(sockfd, reply)) {
    
    fprintf(stderr, "ERROR - track_client\n");
    fprintf(stderr, "Reading verify_connect reply.\n");
    return (-1);
    
  }
  
  if (reply->status == TDATA_FAILURE) {
    
    fprintf(stderr, "FAILURE - track_client - verify_connect\n");
    fprintf(stderr, "%s\n", reply->text);
    
  }
  
  return (0);
  
}

/***********************************************************************
 *
 * request_notify()
 *
 */

int request_notify(sockfd, request, reply)
     
     int sockfd;
     tdata_request_t *request;
     tdata_reply_t *reply;
     
{
  
  request->request = BE_from_si32((ui32) TDATA_REQUEST_NOTIFY);
  
  if (SKU_writeh(sockfd, (char *) request,
		 (si32) sizeof(tdata_request_t),
		 (si32) TDATA_REQUEST_PACKET_ID,
		 request_num) < 0) {
    
    fprintf(stderr, "ERROR - track_client\n");
    fprintf(stderr, "Could not request_notify.\n");
    return (-1);
    
  }
  
  request_num++;
  
  if (read_reply(sockfd, reply)) {
    
    fprintf(stderr, "ERROR - track_client\n");
    fprintf(stderr, "Reading request_notify reply.\n");
    return (-1);
    
  }
  
  if (reply->status == TDATA_FAILURE) {
    
    fprintf(stderr, "FAILURE - track_client - request_notify\n");
    fprintf(stderr, "%s\n", reply->text);
    
  }
  
  return (0);
  
}

/***********************************************************************
 *
 * stop_notify()
 *
 */

int stop_notify(sockfd, request, reply)
     
     int sockfd;
     tdata_request_t *request;
     tdata_reply_t *reply;
     
{
  
  request->request = BE_from_si32((ui32) TDATA_STOP_NOTIFY);
  
  if (SKU_writeh(sockfd, (char *) request,
		 (si32) sizeof(tdata_request_t),
		 (si32) TDATA_REQUEST_PACKET_ID,
		 request_num) < 0) {
    
    fprintf(stderr, "ERROR - track_client\n");
    fprintf(stderr, "Could not stop_notify.\n");
    return (-1);
    
  }
  
  request_num++;
  
  if (read_reply(sockfd, reply)) {
    
    fprintf(stderr, "ERROR - track_client\n");
    fprintf(stderr, "Reading stop_notify reply.\n");
    return (-1);
    
  }
  
  if (reply->status == TDATA_FAILURE) {
    
    fprintf(stderr, "FAILURE - track_client - stop_notify\n");
    fprintf(stderr, "%s\n", reply->text);
    
  }
  
  return (0);
  
}

/***********************************************************************
 *
 * set_max_len()
 *
 */

int set_max_message_len(sockfd, request, reply)
     
     int sockfd;
     tdata_request_t *request;
     tdata_reply_t *reply;
     
{
  
  request->request = BE_from_si32((ui32) TDATA_SET_MAX_MESSAGE_LEN);
  request->max_message_len =
    BE_from_si32((ui32) request->max_message_len);
  
  if (SKU_writeh(sockfd, (char *) request,
		 (si32) sizeof(tdata_request_t),
		 (si32) TDATA_REQUEST_PACKET_ID,
		 request_num) < 0) {
    
    fprintf(stderr, "ERROR - track_client\n");
    fprintf(stderr, "Could not set_max_message_len.\n");
    return (-1);
    
  }
  
  request_num++;
  
  if (read_reply(sockfd, reply)) {
    
    fprintf(stderr, "ERROR - track_client\n");
    fprintf(stderr, "Reading set_max_message_len reply.\n");
    return (-1);
    
  }
  
  if (reply->status == TDATA_FAILURE) {
    
    fprintf(stderr, "FAILURE - track_client - set_max_message_len\n");
    fprintf(stderr, "%s\n", reply->text);
    
  }
  
  return (0);
  
}

/***********************************************************************
 *
 * request_data()
 *
 */

int request_data(sockfd, request, reply)
     
     int sockfd;
     tdata_request_t *request;
     tdata_reply_t *reply;
     
{
  
  tdata_request_t local_request;
  
  printf("\n");
  
  if (request->mode == TDATA_COMPLETE)
    printf("mode: tdata_complete\n");
  else
    printf("mode: tdata_basic_only\n");
  
  if (request->source == TDATA_ARCHIVE)
    printf("source: tdata_archive\n");
  else
    printf("source: tdata_realtime\n");
  
  printf("track_num = %ld\n", request->track_set);
  printf("time_margin = %ld\n", request->time_margin);
  
  request->request = TDATA_REQUEST_DATA;
  
  local_request = *request;
  
  BE_from_array_32((ui32 *) &local_request,
		   (si32) sizeof(tdata_request_t));
  
  if (SKU_writeh(sockfd, (char *) &local_request,
		 (si32) sizeof(tdata_request_t),
		 (si32) TDATA_REQUEST_PACKET_ID,
		 request_num) < 0) {
    
    fprintf(stderr, "ERROR - track_client\n");
    fprintf(stderr, "Could not request_data.\n");
    return (-1);
    
  }
  
  request_num++;
  
  if (read_reply(sockfd, reply)) {
    
    fprintf(stderr, "ERROR - track_client\n");
    fprintf(stderr, "Reading request_data reply.\n");
    return (-1);
    
  }
  
  if (reply->status == TDATA_FAILURE) {
    
    fprintf(stderr, "FAILURE - track_client - request_data\n");
    fprintf(stderr, "%s\n", reply->text);
    return (0);
    
  }
  
  if (request->mode == TDATA_COMPLETE) {
    
    if (read_complete_data(sockfd, print_on))
      return (-1);
    
  } else {
    
    if (read_basic_data(sockfd, print_on))
      return (-1);
    
    
  }
  
  return (0);
  
}

/***********************************************************************
 *
 * hangup()
 *
 */

int hangup(sockfd, request, reply)
     
     int sockfd;
     tdata_request_t *request;
     tdata_reply_t *reply;
     
{
  
  request->request = BE_from_si32((ui32) TDATA_HANGUP);
  
  if (SKU_writeh(sockfd, (char *) request,
		 (si32) sizeof(tdata_request_t),
		 (si32) TDATA_REQUEST_PACKET_ID,
		 request_num) < 0) {
    
    fprintf(stderr, "ERROR - track_client\n");
    fprintf(stderr, "Could not hangup.\n");
    return (-1);
    
  }
  
  request_num++;
  
  if (read_reply(sockfd, reply)) {
    
    fprintf(stderr, "ERROR - track_client\n");
    fprintf(stderr, "Reading hangup reply.\n");
    return (-1);
    
  }
  
  if (reply->status == TDATA_FAILURE) {
    
    fprintf(stderr, "FAILURE - track_client - hangup\n");
    fprintf(stderr, "%s\n", reply->text);
    
  }
  
  return (0);
  
}

/***************************************************************************
 * tidy_and_exit.c
 *
 * tidies up and quits
 *
 * Mike Dixon
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * March 1992
 *
 ****************************************************************************/

int tidy_and_exit(sig)
     int sig;
     
{
  
  /*
   * exit with code sig
   */
  
  printf("Exiting on signal %d\n", sig);
  
  exit(sig);
  
}

