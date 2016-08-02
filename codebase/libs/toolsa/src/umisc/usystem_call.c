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
 * usystem_call.c
 * 
 * Calls for executing a program from within a process
 *
 * Mike Dixon, RAP, NCAR, Boulder, Co, USA
 *
 * October 1992
 *
 ***************************************************************************/

/***************************************************************************
 *
 * The usystem_call group of functions perform the equivalent of the
 * 'system' function, without the overhead of duplicating the entire
 * program memory.
 *
 * For SUNOS, this is achieved using the vfork() function followed
 * by an execve(). The init and kill functions are dummies.
 *
 * For non-SUN machines, which do not have vfork(), the procedure
 * is to spawn a child early in the program before much memory
 * has been allocated. The parent then communicates with the child
 * and the child executes the system call. So you need to call
 * usystem_call_init() early in the program, and usystem_call_kill()
 * at the end of the program or after the last usystem_call().
 *
 ****************************************************************************/

#include <toolsa/umisc.h>

#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>

#ifdef HAVE_VFORK

#ifdef SUNOS4
extern int vfork(void);
#endif

#else

#ifdef OBSOLETE
static FILE *parent_read_fd;
static FILE *parent_write_fd;
static int pid;
static int s_pipe(int fd[2]);
#endif

extern int socketpair(int, int, int, int *);
extern FILE *fdopen(int, const char *);

#define DONE_LEN 8

#endif

/****************************************************************************
 * usystem_call_init()
 *
 * sets up a child process for the execution of the system function
 *
 * returns 0 on success, -1 on failure
 *
 * Mike Dixon, RAP, NCAR, Boulder, CO, USA
 *
 * August 1992
 *
 ****************************************************************************/

int usystem_call_init(void)

{

#ifdef HAVE_VFORK
  
  return (0);

#else

#ifdef OBSOLETE

  FILE *child_read_fd, *child_write_fd;
  int pipefds[2];
  char call_str[BUFSIZ];

  /*
   * create the pipe
   */

  if (s_pipe(pipefds) < 0) {
    fprintf(stderr, "ERROR - usystem_call_init\n");
    perror("s_pipe");
    return (-1);
  }
  
  if ((pid = fork()) < 0) {
    fprintf(stderr, "ERROR - usystem_call_init\n");
    perror("fork");
    return (-1);
  }

  if (pid == 0) {

    /*
     * child
     */

    /*
     * convert to stdio
     */

    child_read_fd = fdopen(pipefds[1], "r");
    child_write_fd = fdopen(pipefds[1], "w");

    /*
     * read the pipe, a line at a time, and execute the system command
     * or quit as applicable
     */

    while (fgets(call_str, BUFSIZ, child_read_fd) != NULL) {

      uusleep(1000000);

      /*
       * quit if asked by parent
       */

      if (!strncmp(call_str, "quit", 4)) {
	
	fprintf(child_write_fd, "%s\n", "done");
	fflush(child_write_fd);
	break;
	
      }

      /*
       * execute system function with the call str
       */

      errno = 0;
      system(call_str);
      
      if (errno) {
	fprintf(stderr, "WARNING - usystem_call_init - child\n");
	perror(call_str);
      }

      fprintf(child_write_fd, "%s\n", "done");
      fflush(child_write_fd);

    } /* while */

    exit (0);

  } else {

    /*
     * parent
     */

    /*
     * convert to stdio
     */

    close(pipefds[1]);
    parent_read_fd = fdopen(pipefds[0], "r");
    parent_write_fd = fdopen(pipefds[0], "w");

  }

#endif

  return (0);

#endif /* HAVE_VFORK */

}

/**********************************************************************
 * usystem_call()
 *
 * sends the system call to the child process, waits for the
 * done message to come back
 */

void usystem_call(char *call_str)

{

#ifdef HAVE_VFORK

#define MAXARGC 256
#define MAXCALLLEN 4096

  int pid;
  int i;
  int argc;
  
  char *file_path;
  char *last_slash;
  char *argv[MAXARGC];
  char *token;
  char local_str[MAXCALLLEN];

  if ((pid = vfork()) < 0) {
    fprintf(stderr, "ERROR - usystem_call\n");
    perror("vfork");
    return;
  }
  
  if (pid == 0) {

    /*
     * child
     */

    /*
     * make local copy of call str
     */

    strncpy(local_str, call_str, MAXCALLLEN);

    /*
     * determine how many args there are
     */

    token = strtok(local_str, " ");
    argc = 1;

    while ((token = strtok((char *) NULL, " ")) != NULL)
      argc++;

    if (argc > MAXARGC)
      argc = MAXARGC;
    
    /*
     * load up file name and first arg which by convention is the
     * program name
     */

    strncpy(local_str, call_str, MAXCALLLEN);

    token = strtok(local_str, " ");
    file_path = token;
    last_slash = strrchr(file_path, '/');
    if (last_slash == NULL)
      argv[0] = file_path;
    else
      argv[0] = last_slash + 1;

    /*
     * load up arg list
     */

    for (i = 1; i < argc; i++)
      argv[i] = strtok((char *) NULL, " ");

    argv[argc] = (char *) NULL;

    /*
     * execute the program
     */

    errno = 0;

    if (execvp(file_path, argv)) {

      fprintf(stderr, "%s", file_path);
      for (i = 1; i < argc; i++)
	fprintf(stderr, " %s", argv[i]);
      perror(" ");
      _exit(-1);

    }

  } else {

    /*
     * parent - wait for child to exit
     */

    wait((int *)0);

  }

#else

#ifdef OBSOLETE
  
  char done_str[DONE_LEN];
  
  fprintf(parent_write_fd, "%s\n", call_str);
  fflush(parent_write_fd);
  
  fgets(done_str, DONE_LEN, parent_read_fd);

#else

  system(call_str);

#endif

#endif /* HAVE_VFORK */
  
}

/************************************************************************
 * usystem_call_clean()
 *
 * kills the child
 */

extern int kill(pid_t, int);

void usystem_call_clean(void)

{

#ifdef HAVE_VFORK

  return;

#else

#ifdef OBSOLETE

  if (pid != 0)
    kill(pid, SIGHUP);
  
  while (waitpid((pid_t) -1,
		 (int *) NULL,
		 (int) (WNOHANG | WUNTRACED)) > 0) {
    uusleep(100);
  };

#endif

#endif /* HAVE_VFORK */

}

#ifndef HAVE_VFORK

/*****************************************************************
 * stream pipe function
 */

#ifdef OBSOLETE

#if defined(IRIX4) || defined(AIX3)

static int s_pipe (int fd[2])

{

  return (socketpair(AF_UNIX, SOCK_STREAM, 0, fd));

}

#else

/*
 * SYSV
 */

static int s_pipe (int fd[2])

{

  return (pipe(fd));

}


#endif /* defined(IRIX4) || defined(AIX3) */

#endif

#endif /* HAVE_VFORK */
