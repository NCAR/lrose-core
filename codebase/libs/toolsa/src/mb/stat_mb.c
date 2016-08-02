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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <toolsa/mb.h>

void main (int argc, char **argv)
{
  char *mb_name;
  int  ret;
  int  mbd;
  MB_status mb_st_buf;

  if (argc != 2) {
    fprintf (stderr, "Usage: %s mb_name\n", argv[0]);
    exit (1);
  }

  mb_name = argv[1];

  mbd = MB_open (mb_name, 0, 0, 0, 0);
  if (mbd < 0) {
    fprintf (stderr, "Failed to open MB %s, ret = %d\n", mb_name, mbd);
    exit (1);
  }

  ret = MB_stat (mbd, &mb_st_buf);
  if (ret < 0) {
    fprintf (stderr, "Failed to stat MB %s, ret = %d\n", mb_name, ret);
    exit (1);
  }

  fprintf (stderr, "MB %s status:\n", mb_name);
  fprintf (stderr, "size = %d,  msg_size = %d, max_msgs = %d\n",
	   mb_st_buf.size, mb_st_buf.msg_size,
	   mb_st_buf.maxn_msgs);
  fprintf (stderr, "perm = %d, flags = %o, "
	   "n_msgs = %d, latest id = %d\n",
	   mb_st_buf.perm, mb_st_buf.flags,
	   mb_st_buf.n_msgs, mb_st_buf.latest_id);
  
  MB_close (mbd);
}
