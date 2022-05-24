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
/********************************************************
 * remote_tape.c
 *
 * Gary Blackburn
 *
 * RAP, NCAR, Boulder, CO, USA
 *
 * May 1997
 *
 **********************************************************/

#include <toolsa/ttape.h>
#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/ioctl.h>

#if defined (AIX)
#include <sys/tape.h>
#elif defined (__linux)
#include <sys/mtio.h>
#endif

#include <errno.h>
#include <setjmp.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define CHILD_PROCESS 0

/* Maximum size of a fully qualified host name.  */
#define MAX_HOST_LEN 256

/* Size of buffers for reading and writing commands to rmt.
   (An arbitrary limit.)  */
#define CMD_BUF_SZ 64

/* Return the parent's read side of the remote stdout connection file desc. */
#define GET_DATA (rmt_stdout[0])

/* Return the parent's write side of remote stdin connection file descriptor. */
#define SND_DATA (rmt_stdin[1])

/* The pipes for receiving data from remote tape drives. */
static int rmt_stdout[2] = {-1, -1};

/* The pipes for sending data to remote tape drives. */
static int rmt_stdin[2] = {-1, -1};


/***************************************************************************/
/* Close remote tape connection file descriptor.  */
static void
  rmt_shutdown (void)

{
  close (GET_DATA);
  close (SND_DATA);
}

/***************************************************************************/

/* Attempt to perform the remote tape command specified in BUF
   on remote tape connection file descriptor.
   Return 0 if successful, -1 on error.  */

static int
  snd_command (char *cmd)

{
  int cmdlen;
  void (*pipe_handler) ();

  /* Save the current pipe handler and try to make the request.  */

  pipe_handler = signal (SIGPIPE, SIG_IGN);
  cmdlen = strlen (cmd);
  if (write (SND_DATA, cmd, cmdlen) == cmdlen)
  {
    signal (SIGPIPE, pipe_handler);
    return 0;
  }
  else
  {
    /* Something went wrong.  Close down and go home.  */
    signal (SIGPIPE, pipe_handler);
    rmt_shutdown ();
    errno = EIO;
    return -1;
  }
}

/***************************************************************************/

/* Read and return the status from remote tape connection file descriptor.
   If an error occurred, return -1 and set errno.  */

static int
  rmt_tape_status (void)

{
  int i;
  char skip_ptr, *cmd_ptr;
  char buffer [CMD_BUF_SZ];

  /* Read the reply command line.  */
  for (i = 0, cmd_ptr = buffer; i < CMD_BUF_SZ; i++, cmd_ptr++)
  {
    if (read (GET_DATA, cmd_ptr, 1) != 1)
    {
      rmt_shutdown ();
      errno = EIO;
      return -1;
    }
    if (*cmd_ptr == '\n')
    {
      *cmd_ptr = '\0';
      break;
    }
  }

  if (i == CMD_BUF_SZ)
  {
    rmt_shutdown ();
    errno = EIO;
    return -1;
  }

  /* Check the return status.  */
  for (cmd_ptr = buffer; *cmd_ptr; cmd_ptr++)
    if (*cmd_ptr != ' ')
      break;

  if (*cmd_ptr == 'E' || *cmd_ptr == 'F')
  {
    errno = atoi (cmd_ptr + 1);
    /* Skip the error message line.  */
    while (read (GET_DATA, &skip_ptr, 1) == 1)
      if (skip_ptr == '\n')
        break;

    if (*cmd_ptr == 'F')
      rmt_shutdown ();

    return -1;
  }

  /* Check for mis-synced pipes. */
  if (*cmd_ptr != 'A')
  {
    rmt_shutdown ();
    errno = EIO;
    return -1;
  }

  /* Got an `A' (success) response.  */
  return atoi (cmd_ptr + 1);
}


/***************************************************************************/

static void interpret_pathname (char *path, char *host_name,
				char *tape_dev, char *user_login)

{

  char *host_ptr, *dev_ptr, *user_ptr;

  host_ptr = host_name;
  dev_ptr = tape_dev;
  user_ptr = user_login;

  while (*path != '@' && *path != ':')
  {
    *host_ptr++ = *path++;
  }
  *host_ptr = '\0';

  if (*path == '@')
  {
    /* defined user part of user@host,  now determine the remote host. */
    strcpy (user_login, host_name);

    /* reinitialize host pointer */
    host_ptr = host_name;
    path++;
    while (*path != ':')
    {
      *host_ptr++ = *path++;
    }

    *host_ptr = '\0';
    path++;
  }
  else
  {
    /* the path contained no user */
    path++;
    *user_ptr = '\0';
  }

  while (*path)
  {
    *dev_ptr++ = *path++;
  }
  *dev_ptr = '\0';

}
/***************************************************************************/

/***************************************************************************/

/* Open a tape device on the system specified in PATH, as the given user.
   PATH has the form `[user@]system:/dev/????'.
   If successful, return the remote tape pipe number.  On error, return -1.  
*/

int
  RMT_open (char *path, int oflag, int mode)

{
  int new_process;
  char buffer[CMD_BUF_SZ];			/* Command buffer.  */
  char host_name[MAX_HOST_LEN];			/* The remote host name.  */
  char tape_dev[CMD_BUF_SZ];			/* The remote device name.  */
  char user_login[CMD_BUF_SZ];	    /* The remote user name.  */

  /* determine the host_name, tape_device, and optional user from the path */
  interpret_pathname (path, host_name, tape_dev, user_login);

  /* Set up two sets of pipes for inputing commands to and receiving data 
   * from the remote tape device
   */
  if (pipe (rmt_stdin) == -1 || pipe (rmt_stdout) == -1)
  {
    perror ("cannot execute pipes");
    return -1;
  }
	
  /* We have a unidirectional pipe, add two way flow capability */
  new_process = fork ();

  if (new_process == -1)
  {
    perror ("cannot execute fork");
    return -1;
  }


  /* We have a bidirectional set of pipes for transfering data between
   * two processes on a local machine; add the ability to transfer data
   * between a local and remote machine
   */ 
  if (new_process == CHILD_PROCESS)
  {
    /* dup the stdin pipe with fd 0; dup the stdout pipe with fd 1; close
     * the original pipes for the child process; execl a rsh command to
     * transfer these file descriptors a remote process */

    close (0);
    dup (rmt_stdin [0]); 

    close (rmt_stdin [0]);
    close (rmt_stdin [1]);

    close (1);
    dup (rmt_stdout [1]);

    close (rmt_stdout [0]);
    close (rmt_stdout [1]);

    /*
     * the group and user ID's will be defined by the account which
     * which originally logged in to start this process ignoring changes
     * defined by it's file permissions (such as sticky bits)
     */
    setuid (getuid ());
    setgid (getgid ());

    if (*user_login)
    {
      execlp ("rsh", host_name, "-l", user_login,
              "/etc/rmt", (char *) 0);
    }
    else
    {
      execlp ("rsh", host_name, "/etc/rmt", (char *) 0);
    }

    perror ("cannot execute remote shell");
    exit (1);
  }

  /* Parent */

  /* complete the uindirectional property of each pipe in the set */
  close (rmt_stdin[0]);
  close (rmt_stdout[1]);

  /* Attempt to open the tape device.  */
  sprintf (buffer, "O%s\n%d\n", tape_dev, oflag);

  if (snd_command (buffer) == -1 || rmt_tape_status () == -1)
    return -1;

  return 0;
}

/***************************************************************************/

/* Close remote tape connection file descriptor and shut down.
   Return 0 if successful, -1 on error.  */

int
  RMT_close (void)

{
  int rmt_cmd;

  if (snd_command ("C\n") == -1)
    return -1;

  rmt_cmd = rmt_tape_status ();
  rmt_shutdown ();
  return rmt_cmd;
}

/***************************************************************************/

/* Read up to NBYTE bytes into BUF from remote tape connection file descriptor.
   Return the number of bytes read on success, -1 on error.  */

int
  RMT_read (char *buf, unsigned int nbyte)

{
  int bytes_red, i;
  char buffer[CMD_BUF_SZ];

  sprintf (buffer, "R%d\n", nbyte);
  if (snd_command (buffer) == -1 || (bytes_red = rmt_tape_status ()) == -1)
    return -1;

  for (i = 0; i < bytes_red; i += nbyte, buf += nbyte)
  {
    nbyte = read (GET_DATA, buf, bytes_red - i);
    if (nbyte <= 0)
    {
      rmt_shutdown ();
      errno = EIO;
      return -1;
    }
  }

  return bytes_red;
}

/***************************************************************************/

/* Write NBYTE bytes from BUF to remote tape connection file descriptor.
   Return the number of bytes written on success, -1 on error.  */

int
  RMT_write (char *buf, unsigned int nbyte)

{
  char buffer [CMD_BUF_SZ];
  void (*pipe_handler) ();

  sprintf (buffer, "W%d\n", nbyte);
  if (snd_command (buffer) == -1)
    return -1;

  pipe_handler = signal (SIGPIPE, SIG_IGN);
  if (write (SND_DATA , buf, nbyte) == nbyte)
  {
    signal (SIGPIPE, pipe_handler);
    return rmt_tape_status ();
  }

  /* Write error.  */
  signal (SIGPIPE, pipe_handler);
  rmt_shutdown ();
  errno = EIO;
  return -1;
}

/* Perform an imitation lseek operation on remote tape connection file desc.
   Return the new file offset if successful, -1 if on error.  */

/***************************************************************************/

long
  RMT_lseek (long offset, int whence)

{
  char buffer [CMD_BUF_SZ];

  sprintf (buffer, "L%ld\n%d\n", offset, whence);
  if (snd_command (buffer) == -1)
    return -1;

  return rmt_tape_status ();
}

/* Perform a raw tape operation on remote tape connection file descriptor.
   Return the results of the ioctl, or -1 on error.  */

int
  RMT_ioctl (int op, char *arg)

{

#if defined(AIX) || defined(__linux)

  char c;
  int bytes_red, cnt;
  char buffer [CMD_BUF_SZ];

  switch (op)
  {
    default:
      errno = EINVAL;
      return -1;

    case MTIOCTOP:
      /* MTIOCTOP is the easy one.  Nothing is transfered in binary.  */
      sprintf (buffer, "I%d\n%d\n", ((struct mtop *) arg)->mt_op,
               ((struct mtop *) arg)->mt_count);
      if (snd_command (buffer) == -1)
        return -1;
      return rmt_tape_status ();	/* Return the count.  */

    case MTIOCGET:
      /* Grab the status and read it directly into the structure.
       * This assumes that the status buffer is not padded
       * and that 2 shorts fit in a long without any word
       * alignment problems; i.e., the whole struct is contiguous.
       * NOTE - this is probably NOT a good assumption.  */

      if (snd_command ("S") == -1 || 
          (bytes_red = rmt_tape_status ()) == -1)
        return -1;

      for (; bytes_red > 0; bytes_red -= cnt, arg += cnt)
      {
        cnt = read (GET_DATA, arg, bytes_red);
        if (cnt <= 0)
        {
          rmt_shutdown ();
          errno = EIO;
          return -1;
        }
      }

      /* Check for byte position.  mt_type is a small integer field
         (normally) so we will check its magnitude.  If it is larger than
         256, we will assume that the bytes are swapped and go through
         and reverse all the bytes.  */

      if (((struct mtget *) arg)->mt_type < 256)
        return 0;

      for (cnt = 0; cnt < bytes_red; cnt += 2)
      {
        c = arg[cnt];
        arg[cnt] = arg[cnt + 1];
        arg[cnt + 1] = c;
      }

      return 0;

  } /* switch */

#else

  errno = EINVAL;
  return -1;

#endif

}

