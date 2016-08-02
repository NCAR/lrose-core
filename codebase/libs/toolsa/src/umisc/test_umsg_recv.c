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
#include <toolsa/umisc.h>
#include <toolsa/port.h>

void tidy_and_exit(int sig);

int main(int argc, char **argv)

{

  int i;
  int msqid;
  int *buf;

  PORTsignal(SIGQUIT, tidy_and_exit);
  PORTsignal(SIGTERM, tidy_and_exit);
  PORTsignal(SIGINT, tidy_and_exit);
  PORTsignal(SIGHUP, tidy_and_exit);

  if ((msqid = umsg_create(5432, 0666)) < 0) {
    fprintf(stderr, "Error creating msg queue\n");
    return (-1);
  }

  while (1) {

    fprintf(stderr, "Waiting on queue id %d\n", msqid);

    if ((buf = umsg_recv(msqid, 100, FALSE)) == NULL) {
      fprintf(stderr, "No data yet\n");
      sleep(1);
    } else {
      for (i = 0; i < 25; i++) {
	fprintf(stderr, "%d %d, ", i, buf[i]);
      }
      fprintf(stderr, "\n\n");
    }
    
  }

  return (0);

}

void tidy_and_exit(int sig)

{

  if (umsg_remove(5432)) {
    fprintf(stderr, "Error removind msg queue\n");
    exit (-1);
  }

  exit (sig);

}

