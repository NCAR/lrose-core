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
 * bprp_ac_ingest()
 *
 * Reads in aircraft GPS position from socket, and writes two types of
 * file:
 *
 *  1. Short file for realtime ops
 *  2. Long file for archive ops
 */

#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <netdb.h>

#include <toolsa/umisc.h>

static FILE *Archive_file = NULL;
static FILE *Realtime_file = NULL;
static char Archive_path[MAX_PATH_LEN];

static void store_line(double lat,
		       double lon,
		       double alt,
		       char *ident,
		       char *realtime_path,
		       char *archive_dir);

void close_output_files(void);

int main(int argc, char **argv)

{

  char *bptr;
  char buf[128];
  char *host;
  char *prog;
  char *realtime_path;
  char *archive_dir;
  char ident[16];
  char ns_hemi[16];
  char ew_hemi[16];

  int port;
  int c;
  int count;
  int iret;
  int forever = 1;

  int ilat, lat_fract;
  int ilon, lon_fract;
  
  double lat, lon;

  date_time_t dtime;

  struct hostent *hp;
  struct sockaddr_in sock_addr;

  /*
   * check usage
   */
  
  if (argc != 5) {
    printf ("Usage:  %s <remote host> <port> <realtime_file> <archive_dir>\n", argv[0]);
    exit (1);
  }

  prog = argv[0];
  host = argv[1];
  port = atoi(argv[2]);
  realtime_path = argv[3];
  archive_dir = argv[4];

  /* 
   *  Clear out socket address structure
   */

  memset ((char *) &sock_addr, 0, sizeof (struct sockaddr_in));

  /* 
   *  Set up the peer address to which we will connect. 
   */

  sock_addr.sin_family = AF_INET;

  /* 
   *  Get the host information for the hostname that the
   *  user passed in.
   */

  hp = gethostbyname (host);

  if (hp == NULL) {
    fprintf (stderr, "ERROR - %s: %s not found in /etc/hosts\n", prog, host);
    exit (-1);
  }

  /* 
   *  Create the socket. 
   */

 re_try:
  sleep(1);

  if((c = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket");
    exit(-1);
  }

  sock_addr.sin_family = AF_INET;
  sock_addr.sin_port = port;
  sock_addr.sin_addr.s_addr = ((struct in_addr *)(hp->h_addr))->s_addr;
 
  /* 
   *  Try to connect to the remote server at the address
   *  which was just built into peeraddr.
   */
  
  fprintf(stdout, "%s: trying to connect to host '%s', port %d\n",
	  prog, host, port);
  fflush(stdout);

  if(connect(c, (void *) &sock_addr, sizeof(sock_addr)) < 0) {
    perror("connect");
    goto re_try;
  }

  while (forever) {

    /*
     * read in the message
     */

    count = sizeof(buf);
    bptr = buf;

    /* Read response in (possibly) multiple bits */

    while(count) {
      iret = read(c, bptr, count);
      if(iret > 0) {
	count -= iret;
	bptr += iret;
      } else {
	fprintf(stderr, "read error - ret = %d\n", iret);
	close(c);
	goto re_try;
      }
    }

    /*
     * parse the input buffer
     */

    if (sscanf(buf, "%s %d %d %d %d %d %d %s %d %d %s %d %d",
	       ident,
	       &dtime.year, &dtime.month, &dtime.day,
	       &dtime.hour, &dtime.min, &dtime.sec,
	       ns_hemi, &ilat, &lat_fract,
	       ew_hemi, &ilon, &lon_fract) != 13) {
      fprintf(stderr, "ERROR - %s, bad data '%s'\n", prog, buf);
      continue;
    }

    fprintf(stderr, "%s\n", buf);

    uconvert_to_utime(&dtime);
    lat = (double) ilat + (double) lat_fract / 6000.0;
    lon = (double) ilon + (double) lon_fract / 6000.0;
    if (!strcmp(ns_hemi, "S")) {
      lat *= -1.0;
    }
    if (!strcmp(ew_hemi, "W")) {
      lon *= -1.0;
    }
	       
    store_line(lat, lon, 0.0, ident, realtime_path, archive_dir);

  } /* while (forever) */

  close(c);

  exit(0);

}

/**************
 * store_line()
 */

static void store_line(double lat,
		       double lon,
		       double alt,
		       char *ident,
		       char *realtime_path,
		       char *archive_dir)

{

  static int first_call = TRUE;
  char arch_path[MAX_PATH_LEN];
  date_time_t now;
  
  /*
   * on first call, init Archive_path
   */

  if (first_call) {
    strcpy(Archive_path, "");
    first_call = FALSE;
  }

  now.unix_time = time(NULL);
  uconvert_from_utime(&now);

  /*
   * compute file path
   */

  sprintf(arch_path, "%s%s%.4d%.2d%.2d", 
	  archive_dir, PATH_DELIM,
	  now.year, now.month, now.day);
  
  if (Archive_file != NULL) {

    if (strcmp(arch_path, Archive_path)) {

      /*
       * names differ, so close the opened files
       */

      close_output_files();

    }
    
  } /* if (Archive_file != NULL) */
  
  /*
   * if output file is not open, open it and store path
   * for future use
   */
  
  if (Archive_file == NULL) {

    /*
     * open archive file for appending
     */
    
    if ((Archive_file = fopen(arch_path, "a")) == NULL) {
      fprintf(stderr, "ERROR - bprp_ac_ingest\n");
      fprintf(stderr, "Cannot open archive file for appending\n");
      perror(arch_path);
      return;
    }

    /*
     * open fresh realtime file for writing
     */

    if ((Realtime_file = fopen(realtime_path, "w"))
	== NULL) {
      fprintf(stderr, "ERROR - bprp_ac_ingest\n");
      fprintf(stderr, "Cannot open realtime file for writing\n");
      perror(realtime_path);
      return;
    }
    strcpy(Archive_path, arch_path);
  }

  /*
   * write to file
   */

  fprintf(Archive_file, "%d %.6g %.6g %g %s\n",
	  (int) now.unix_time, lat, lon, alt, ident);

  fflush(Archive_file);

  fprintf(Realtime_file, "%d %.6g %.6g %g %s\n",
	  (int) now.unix_time, lat, lon, alt, ident);

  fflush(Realtime_file);

  return;
  
}

/**********************
 * close_output_files()
 *
 * cleanup
 */

void close_output_files(void)

{

  if (Archive_file != NULL) {
    fclose (Archive_file);
    Archive_file = NULL;
  }

  if (Realtime_file != NULL) {
    fclose (Realtime_file);
    Realtime_file = NULL;
  }

  return;

}
