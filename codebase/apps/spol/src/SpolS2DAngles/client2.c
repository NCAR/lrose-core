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
//  client.c - UDP client 
// Last modify dec 7 2010 by paloma@ucar.edu
// Last modify dec 9 2010 by paloma@ucar.edu: change to accept broad cast. 

#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <time.h>
#include <netinet/in.h>

#if defined(__linux)
#include <asm/ioctls.h>
#include <sys/ioctl.h>
#else
#include <sys/filio.h>
#endif

using namespace std;

int main(int argc, char *argv[])
{

  int _debug = 0;
  if (argc > 1) {
    if (strstr(argv[1], "-debug") != NULL) {
      _debug = 1;
    }
  }

  const int port = 12080;          /* port for socket */
  int blocking_flag = 1;          /* argument for ioctl call */
  int yes = 1;
  struct sockaddr_in addr;        /* address structure for socket */
  socklen_t addr_len = sizeof addr;

  fprintf(stderr, "Opening UDP, port %d\n", port);
  
  /* create socket */
  
  int udpfd;
  if((udpfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
    perror("socket");
    fprintf(stderr, "Could not create UDP socket\n");
    exit(1);
  }
  
  /* make the socket non-blocking */
  
  if (ioctl(udpfd, FIONBIO, &blocking_flag) != 0) {
    fprintf(stderr, "Could not make socket non-blocking\n");
    perror ("ioctl error:");
    exit(1);
  }
  
  /* set the socket for reuse */
  
  setsockopt(udpfd, SOL_SOCKET, SO_REUSEADDR, (char *) &yes, sizeof(yes));

  /* bind local address to the socket */
	     
  memset((void *) &addr, 0, sizeof (addr));
  addr.sin_port = htons (port);
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl (INADDR_ANY);
  if (bind (udpfd, (struct sockaddr *) &addr, sizeof (addr)) < 0) {
    fprintf(stderr, "Could not bind UDP socket, port %d\n", port);
    perror ("bind error:");
    exit(1);
  }

  int count = 0;
  struct timeval tv;
  gettimeofday(&tv, NULL);
  double prevTime = (double) tv.tv_sec + tv.tv_usec / 1.0e6;
 
  while (1)
  {
    
    int numbytes;
    char buf[256];
    if ((numbytes = recvfrom(udpfd, buf, sizeof(buf), 0,
                             (struct sockaddr *) &addr, &addr_len)) > 0) {

      if (numbytes < 256) {
        buf[numbytes] = '\0';
      } else {
        buf[255] = '\0';
      }
      if (_debug) {
	fprintf(stderr, "got msg: '%s'\n", buf);
      }

      int binAz, binEl;
      long unixTime, uSecs;
      int seqNum;
      
      if (sscanf(buf, "%ld %ld %d %d %d", &unixTime, &uSecs, &binEl, &binAz, &seqNum) == 5) {
	double el = binEl * 360.0 / 65536.0;
	double az = binAz * 360.0 / 65536.0;
	char timestr[256];
	strftime(timestr, 256, "%Y %m %d %H %M %S", gmtime(&unixTime));
	fprintf(stderr, "time, usecs, el, az, seq: %s, %g, %g, %g, %d\n", timestr, uSecs / 1000000.0 , el, az, seqNum);
      }
      
      count++;

      if (count % 1000 == 0) {
	gettimeofday(&tv, NULL);
	double now = (double) tv.tv_sec + tv.tv_usec / 1.0e6;
	double dt = now - prevTime;
	double rate = 1000.0 / dt;
	fprintf(stderr, "Current rate (msg/sec): %g\n", rate);
	prevTime = now;
      }

    }

  } /* while */
	
  return 0;

}

