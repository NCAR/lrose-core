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
/******************************************************************
 * os_config.h
 *
 * Operating system configuration header
 *
 * There is one section for each operating system.
 *
 */

#ifndef os_config_h
#define os_config_h

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_PATH_LEN 1024
#define MAX_HOST_LEN 256

/****** LINUX ********************************************************/

#if defined(LINUX_ALPHA)

#define PATH_DELIM "/"
typedef struct timeval timeval_t;
#define FD_SET_P (fd_set *)

#elif defined(CYGWIN)   /*********************************************/

#define PATH_DELIM "/"
#include <sys/time.h>
typedef struct timeval timeval_t;
#define FD_SET_P (fd_set *)

#elif defined(__linux)  /*********************************************/

#define PATH_DELIM "/"
#include <sys/time.h>
typedef struct timeval timeval_t;
#define FD_SET_P (fd_set *)

#endif /* LINUX_ALPHA */

/** MAX OSX ***********************************************************/

#ifdef __APPLE__
#define PATH_DELIM "/"
#include <sys/time.h>
typedef struct timeval timeval_t;
#define FD_SET_P (fd_set *)

#endif /* __APPLE__ */

/** IBM **************************************************************/

#ifdef AIX

#define PATH_DELIM "/"
#define FD_SET_P (fd_set *)
#include <sys/select.h>
#include <sys/time.h>
#include <sys/fcntl.h>  /* declares open() */
typedef struct timeval timeval_t;
extern int setitimer(int, struct itimerval *, struct itimerval *);

#endif /* AIX */

/*** SUNOS5 **********************************************************/

#if defined(SUNOS5) || defined(SUNOS5_ETG)

#define PATH_DELIM "/"
#define HAVE_VFORK		/* support the vfork() system call */
#define FD_SET_P (fd_set *)

#include <fcntl.h>  /* declares open() */
#include <sys/types.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <dirent.h>

typedef struct timeval timeval_t;

union semun {
  int val;
  struct semid_ds *buf;
  unsigned short *array;
};

#ifndef __cplusplus
extern int gettimeofday(struct timeval *tp,void *unused);
#endif

extern int gethostname(char *name, int namelen);
struct tm *gmtime_r(const time_t *clock, struct tm *res);

#include <signal.h>

extern int sigblock(int mask);
extern int sigmask(int signum);
extern int sigpause(int mask);
extern int sigsetmask(int mask);

/* reentrant versions */

extern struct tm *gmtime_r(const time_t *clock, struct tm *res);
extern struct tm *localtime_r(const time_t *clock, struct tm *res);
extern char *strtok_r(char *s1, const char *s2, char **lasts);

#endif /* SUNOS5 */

/*******************************************************************/

#if defined(SUNOS5_INTEL)	/* Sunos 4.x, Solaris 1.x */

#define PATH_DELIM "/"

#define HAVE_VFORK		/* support the vfork() system call */
#define FD_SET_P (fd_set *)

#include <fcntl.h>  /* declares open() */
#include <sys/types.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/stat.h>

typedef struct timeval timeval_t;

union semun {
  int val;
  struct semid_ds *buf;
  unsigned short *array;
};

#ifndef __cplusplus
extern int gettimeofday(struct timeval *tp,void *unused);
#endif

extern int gethostname(char *name, int namelen);

#include <signal.h>

extern int sigblock(int mask);
extern int sigmask(int signum);
extern int sigpause(int mask);
extern int sigsetmask(int mask);

#endif /* SUNOS5_INTEL */

/*******************************************************************/

#if defined(SUNOS5_64)

#define PATH_DELIM "/"

#define HAVE_VFORK		/* support the vfork() system call */

#include <fcntl.h>
#include <sys/stat.h>

#define FD_SET_P (fd_set *)

#include <fcntl.h>  /* declares open() */
#include <sys/types.h>
#include <sys/time.h>
#include <sys/select.h>
typedef struct timeval timeval_t;

union semun {
  int val;
  struct semid_ds *buf;
  unsigned short *array;
};

extern int gethostname(char *name, int namelen);

#include <strings.h>      /* declares bzero */
#include <signal.h>

extern int sigblock(int mask);
extern int sigmask(int signum);
extern int sigpause(int mask);
extern int sigsetmask(int mask);

#endif /* SUNOS5_64 */

/**** SUNOS4 *********************************************************/

#if defined(SUNOS4)			/* Sunos 4.x, Solaris 1.x */

#define PATH_DELIM "/"

#ifdef LINT
#define sun
#endif

#define HAVE_VFORK		/* support the vfork() system call */
#define FD_SET_P (fd_set *)

#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>

extern double strtod(const char *, char **);

typedef struct timeval timeval_t;

extern int printf(const char *format, ...);
extern int fprintf(FILE *stream, const char *format, ...);
extern int fflush(FILE *stream);
extern int fseek(FILE *stream, long offset, int whence);
extern size_t fread(void *ptr, size_t size, size_t nitems, FILE *stream);
extern size_t fwrite(const void *ptr, size_t size,
		     size_t nitems, FILE *stream);
extern int fclose(FILE *stream);
extern int sscanf(const char *s, const char *format, ...);

extern void perror(const char *s);

extern time_t time(time_t *tloc);

extern int system(const char *string);

extern int select(int width,
		  fd_set *rdset, fd_set *wrset, fd_set *exset,
		  timeval_t *timeout);

int semctl(int, int, int, ...);
int semget(key_t, int, int);
int semop(int, struct sembuf *, unsigned);

int shmctl(int, int, ...);
int shmget(key_t, int, int);
void *shmat(int, void *, int);
int shmdt(void *);

extern int setitimer (int which,
		      struct itimerval *value,
		      struct itimerval *ovalue);

extern int ftruncate(int fd, off_t length);

extern int wait3(int *statusp, int options,
		 struct rusage *rusage);

extern int getopt (int argc, char * const argv[], const char *optstring);

extern int openlog(char * ident, int logopt, int facility);
extern int syslog( int priority, char *message, ...);

extern int gethostname(char *name, int namelen);
extern int gettimeofday(struct timeval *tp,struct timezone *tzp);

#ifndef bzero
extern void bzero(void *d, size_t len);        /* needed for FD_ZERO ! */
#endif

#endif /* SUNOS4 */

/** HP UNIX **********************************************************/

#ifdef HPUX

#define PATH_DELIM "/"
#define HAVE_VFORK		/* support the vfork() system call */
#define FD_SET_P (int *)

#include <sys/time.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/ipc.h>

typedef struct timeval timeval_t;
extern int setitimer(int, struct itimerval *, struct itimerval *);

union semun {
  int		val;		/* value for SETVAL */
  struct semid_ds	*buf;	/* buffer for IPC_STAT & IPC_SET */
  unsigned short	*array;	/* array for GETALL & SETALL */
};

#endif /* HPUX */

/** SGI IRIX 6 *********************************************************/

#if defined(IRIX6)

#define PATH_DELIM "/"
#define _BSD_TYPES
#include <sys/bsd_types.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/select.h>
#define FD_SET_P (fd_set *)
typedef struct timeval timeval_t;

extern int select(int width,
		  fd_set *rdset, fd_set *wrset, fd_set *exset,
		  struct timeval *timeout);

extern int kill(pid_t, int);

#endif /* IRIX6 */

/** DEC OSF1 ***********************************************************/

#ifdef DECOSF1

#define PATH_DELIM "/"
#define HAVE_VFORK		/* support the vfork() system call */
#define FD_SET_P (fd_set *)

#include <sys/types.h>
#include <sys/time.h>
#include <sys/select.h>

typedef struct timeval timeval_t;

union semun {
  int val;
  struct semid_ds *buf;
  unsigned short *array;
};

#endif /* DECOSF1 */

/*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* os_config_h */


