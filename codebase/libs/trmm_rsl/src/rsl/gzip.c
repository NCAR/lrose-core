/*
    NASA/TRMM, Code 910.1.
    This is the TRMM Office Radar Software Library.
    Copyright (C) 1996, 1997
            John H. Merritt
            Space Applications Corporation
            Vienna, Virginia

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#define _USE_BSD
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <signal.h>

/* Prototype definitions within this file. */
int no_command (char *cmd);
FILE *uncompress_pipe (FILE *fp);
FILE *compress_pipe (FILE *fp);


/* Avoids the 'Broken pipe' message by reading the rest of the stream. */
void rsl_readflush(FILE *fp)
{
  if (fork() == 0) { /* Child */
	char buf[1024];
	while(fread(buf, sizeof(char), sizeof(buf), fp)) continue;
	exit(0);
  } else { /* Parent - just waits for child to terminate. This
			  is thus a rather pointless fork() but I will leave it here
			  mostly as a record of the modification - Niles Oien, UCAR. */
    int status;
	wait( &status );
  }
}
	
int rsl_pclose(FILE *fp)
{
  int rc;
  if ((rc=pclose(fp)) == EOF) {
	perror ("pclose");  /* This or fclose do the job. */
	if ((rc=fclose(fp)) == EOF)
	  perror ("fclose");  /* This or fclose do the job. */
  }
  return rc;
}

int no_command (char *cmd)
{
  int rc;
  /* Return 0 if there is the command 'cmd' on the system. */
  /* Return !0 otherwise. */
  rc = system(cmd);
  if (rc == 0) return rc;
  else return !0;
}

FILE *uncompress_pipe (FILE *fp)
{
  /* Pass the file pointed to by 'fp' through the gzip pipe. */

  FILE *fpipe;
  int save_fd;

  if (no_command("gzip --version > /dev/null 2>&1")) return fp;
  save_fd = dup(0);
  close(0); /* Redirect stdin for gzip. */
  dup(fileno(fp));

  fpipe = popen("gzip -q -d -f --stdout", "r");
  if (fpipe == NULL) perror("uncompress_pipe");
  close(0);
  dup(save_fd);
  return fpipe;
}

FILE *compress_pipe (FILE *fp)
{
  /* Pass the file pointed to by 'fp' through the gzip pipe. */

  FILE *fpipe;
  int save_fd;

  if (no_command("gzip --version > /dev/null 2>&1")) return fp;
  fflush(NULL); /* Flush all buffered output before opening this pipe. */
  save_fd = dup(1);
  close(1); /* Redirect stdout for gzip. */
  dup(fileno(fp));

  fpipe = popen("gzip -q -1 -c", "w");
  if (fpipe == NULL) perror("compress_pipe");
  close(1);
  dup(save_fd);
  return fpipe;
}


