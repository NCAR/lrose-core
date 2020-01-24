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
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/mount.h>

#if defined(__linux)
#include <sys/vfs.h>
#endif

#include <toolsa/umisc.h>
#include <toolsa/file_io.h>
#include <toolsa/pmu.h>
#include <toolsa/str.h>

#ifndef MAX_PATH_LEN
#define MAX_PATH_LEN 1024
#endif

/*************************************************
 * File io routines
 *
 * Mike Dixon, Rap, NCAR, Boulder, CO, 80303, USA
 *
 * April 1995
 */

static int file_uncompress(const char *path);

/********************************************************
 * ta_fread()
 *
 * Wrapper for fread() - takes care of interrupted read.
 * Returns same as fread()
 *
 */

size_t ta_fread(void *ptr, size_t size, size_t nitems, FILE *stream)

{

  size_t ireturn;

  errno = EINTR;
  
  while (errno == EINTR) {
    errno = 0;
    ireturn = fread(ptr, size, nitems, stream);
  }

  return ireturn;

}

/*********************************************************
 * ta_fwrite()
 *
 * Wrapper for fwrite() - takes care of interrupted write.
 * Returns same as fwrite()
 */

size_t ta_fwrite(const void *ptr, size_t size, size_t nitems, FILE *stream)

{

  size_t ireturn;

  errno = EINTR;

  while (errno == EINTR) {
    errno = 0;
    ireturn = fwrite(ptr, size, nitems, stream);
  }

  return ireturn;

}

/*******************************************************
 *
 * ta_file_uncompress()
 *
 * Uncompresses file if:
 *  (a) file is compressed and
 *  (b) the uncompressed file doesn't already exist.
 *
 * Search is done for compressed or gzipped files.
 *
 * Handles compressed and gzipped type files.
 *
 * If path has .Z, .gz or .bz2 extension, the extension is
 * removed before the function returns.
 *
 * Returns 1 if uncompression done, 0 if not, -1 if error
 */

int ta_file_uncompress(char *path)

{

  char *ext;

  /*
   * if file name indicates that it is compressed (.Z)
   * remove the extention and call the uncompress function with
   * the plain name
   */

  ext = path + strlen(path) - 2;

  if (!strncmp(ext, ".Z", 2)) {

    /*
     * alter file path to remove extension
     */

    *ext = '\0';

    return (file_uncompress (path));

  }

  /*
   * if file name indicates that it is gzipped (.gz)
   * remove the extention and call the uncompress function with
   * the plain name
   */

  ext = path + strlen(path) - 3;
  
  if (!strncmp(ext, ".gz", 3)) {

    /*
     * alter file path to remove extension
     */

    *ext = '\0';
    
    return (file_uncompress (path));

  }
  
  /*
   * if file name indicates that it is bzipped2 (.bz2)
   * remove the extention and call the uncompress function with
   * the plain name
   */

  ext = path + strlen(path) - 4;
  
  if (!strncmp(ext, ".bz2", 4)) {

    /*
     * alter file path to remove extension
     */

    *ext = '\0';
    
    return (file_uncompress (path));

  }
  
  /*
   * no compression extension indicated, call uncompress with plain name
   */

  return (file_uncompress (path));

}

/*******************************************************
 *
 * file_uncompress()
 *
 * Uncompresses file if:
 *  (a) file is compressed and
 *  (b) the uncompressed file doesn't already exist.
 *
 * Search is done for compressed or gzipped files.
 *
 * Returns 1 if uncompression done, 0 if not, -1 if error
 */

static int file_uncompress(const char *path)

{

  char compressed_path[MAX_PATH_LEN];
  char call_str[BUFSIZ];
  struct stat file_stat;
  int iret;

  /*
   * if file uncompressed file exists, return now
   */

  if (stat(path, &file_stat) == 0) {
    return (0);
  }
    
  /*
   * Check if the compressed file exists.
   * If this is the case, uncompress the file.
   */

  STRncopy(compressed_path, path, MAX_PATH_LEN);
  STRconcat(compressed_path, ".Z", MAX_PATH_LEN);

  if (stat(compressed_path, &file_stat) == 0) {
    
    /*
     * uncompress file
     */
    
    sprintf(call_str, "uncompress -f %s", compressed_path);

    iret = system (call_str);

    if (iret) {
      fprintf(stderr, "WARNING - could not uncompress file\n");
      fprintf(stderr, "  Return from %s: iret = %d\n",
	      call_str, iret);
      return -1;
    } else {
      return 0;
    }

  }

  /*
   * Check if the gzipped file exists.
   * If this is the case, gunzip the file.
   */

  STRncopy(compressed_path, path, MAX_PATH_LEN);
  STRconcat(compressed_path, ".gz", MAX_PATH_LEN);
  
  if (stat(compressed_path, &file_stat) == 0) {

    /*
     * gunzip file
     */
    
    sprintf(call_str, "gunzip -f %s", compressed_path);

    iret = system (call_str);

    if (iret) {
      fprintf(stderr, "WARNING - could not gunzip file\n");
      fprintf(stderr, "  Return from %s: iret = %d\n",
	      call_str, iret);
      return -1;
    } else {
      return 0;
    }
    
  }

  /*
   * Check if the bzipped file exists.
   * If this is the case, gunzip the file.
   */

  STRncopy(compressed_path, path, MAX_PATH_LEN);
  STRconcat(compressed_path, ".bz2", MAX_PATH_LEN);
  
  if (stat(compressed_path, &file_stat) == 0) {

    /*
     * gunzip file
     */
    
    sprintf(call_str, "bunzip2 -f %s", compressed_path);

    iret = system (call_str);

    if (iret) {
      fprintf(stderr, "WARNING - could not bunzip2 file\n");
      fprintf(stderr, "  Return from %s: iret = %d\n",
	      call_str, iret);
      return -1;
    } else {
      return 0;
    }
    
  }

  /*
   * file does not exist
   */

  return 0;

}

/*********************************************************
 * ta_fopen_uncompress()
 *
 * Uncompresses the file if necessary, then opens it
 *
 * Return is identical to fopen()
 */

FILE *ta_fopen_uncompress(const char *filename, const char *type)

{
  FILE *local_file;
  char *filename_copy = STRdup(filename);
  
  ta_file_uncompress(filename_copy);
  local_file = fopen(filename_copy, type);

  ufree(filename_copy);
  
  return local_file;
}

/*********************************************
 * ta_lock_file()
 *
 * Sets up read or write lock on entire file.
 *
 * File must already be open - path is passed in for
 * error reporting only.
 *
 * Type is "r" or "w", for read or write lock respectively.
 *
 * Blocks until lock is obtained.
 *
 * Returns 0 on success, -1 on failure.
 */

int ta_lock_file(const char *path, FILE *fp,
		 const char *type)

{
  if (fp == NULL) {
    return (-1);
  }
  return (ta_lock_file_fd(path, fileno(fp), type));
}

/*********************************************
 * ta_lock_file_threaded()
 *
 * Same as ta_lock_file(), except it also calls
 * flockfile() to lock files for threading.
 *
 * Returns 0 on success, -1 on failure.
 */

int ta_lock_file_threaded(const char *path,
			  FILE *fp,
			  const char *type)

{
  if (ta_lock_file(path, fp, type)) {
    return -1;
  }
  flockfile(fp);
  return 0;
}

/*********************************************
 * ta_lock_file_fd()
 *
 * Sets up read or write lock on entire file.
 *
 * File must already be open - path is passed in for
 * error reporting only.
 *
 * Type is "r" or "w", for read or write lock respectively.
 *
 * Blocks until lock is obtained.
 *
 * Returns 0 on success, -1 on failure.
 */

int ta_lock_file_fd(const char *path, int fd,
		    const char *type)

{
  
  struct flock lock;

  if (!strcmp(type, "r")) {
    lock.l_type = F_RDLCK;
  } else if (!strcmp(type, "w")) {
    lock.l_type = F_WRLCK;
  } else {
    fprintf(stderr, "WARNING - ta_lock_file\n"
	    "  Illegal lock type: '%s', file path '%s'\n",
	    path, type);
    return (-1);
  }
  lock.l_start = 0;
  lock.l_whence = SEEK_SET;
  lock.l_len = 0;
  
  if (fcntl(fd, F_SETLKW, &lock) == -1) {
    fprintf(stderr,
	    "WARNING - ta_lock_file - cannot lock, type '%s'\n", type);
    perror(path);
    return (-1);
  }
  
  return (0);

}

/*********************************************
 * ta_lock_file_procmap()
 *
 * Sets up read or write lock on entire file.
 * Loops waiting for lock. Reports to procmap while
 * waiting.
 *
 * File must already be open - path is passed in for
 * error reporting only.
 *
 * Type is "r" or "w", for read or write lock respectively.
 *
 * Returns 0 on success, -1 on failure.
 */

int ta_lock_file_procmap(const char *path, FILE *fp,
			 const char *type)
{
  if (fp == NULL) {
    return (-1);
  }
  return (ta_lock_file_procmap_fd(path, fileno(fp), type));
}

/*********************************************
 * ta_lock_file_procmap_threaded()
 *
 * Same as ta_lock_file_procmap(), except it also calls
 * ftrylockfile() to lock files for threading.
 *
 * Returns 0 on success, -1 on failure.
 */

int ta_lock_file_procmap_threaded(const char *path, FILE *fp,
				  const char *type)
{
  char message[BUFSIZ];
  if (ta_lock_file_procmap(path, fp, type)) {
    return -1;
  }
  sprintf(message, "Waiting for ftrylockfile() to succeed on file '%s'",
	  path);
  while (ftrylockfile(fp) ) {
    PMU_auto_register(message);
    umsleep(1000);
  }
  return 0;
}

/*********************************************
 * ta_lock_file_procmap_fd()
 *
 * Sets up read or write lock on entire file.
 * Loops waiting for lock. Reports to procmap while
 * waiting.
 *
 * File must already be open - path is passed in for
 * error reporting only.
 *
 * Type is "r" or "w", for read or write lock respectively.
 *
 * Returns 0 on success, -1 on failure.
 */

int ta_lock_file_procmap_fd(const char *path, int fd,
			    const char *type)

{

  char message[BUFSIZ];
  struct flock lock;

  sprintf(message, "Waiting for '%s' lock on file '%s'",
	  type, path);

  if (!strcmp(type, "r")) {
    lock.l_type = F_RDLCK;
  } else if (!strcmp(type, "w")) {
    lock.l_type = F_WRLCK;
  } else {
    fprintf(stderr, "WARNING - ta_lock_file\n"
	    "  Illegal lock type: '%s', file path '%s'\n",
	    path, type);
    return (-1);
  }
  lock.l_start = 0;
  lock.l_whence = SEEK_SET;
  lock.l_len = 0;
  
  while (fcntl(fd, F_SETLK, &lock) == -1) {
    if (errno == EACCES || errno == EAGAIN) {
      PMU_auto_register(message);
      sleep(1);
    } else {
      fprintf(stderr,
	      "WARNING - ta_lock_file - cannot lock, type '%s'\n", type);
      perror(path);
      return (-1);
    }
  }
  
  return (0);

}

/*********************************************
 * ta_lock_file_heartbeat()
 *
 * Sets up read or write lock on entire file.
 * Loops waiting for lock.
 *
 * If heartbeat function is non-NULL, calls it while waiting.
 *
 * File must already be open - path is passed in for
 * error reporting only.
 *
 * Type is "r" or "w", for read or write lock respectively.
 *
 * Returns 0 on success, -1 on failure.
 */

int ta_lock_file_heartbeat(const char *path, FILE *fp,
                           const char *type,
                           TA_heartbeat_t heartbeat_func)
{
  if (fp == NULL) {
    return (-1);
  }
  return (ta_lock_file_heartbeat_fd(path, fileno(fp),
                                    type, heartbeat_func));
}

/*********************************************
 * ta_lock_file_heartbeat_threaded()
 *
 * Same as ta_lock_file_heartbeat(), except it also calls
 * ftrylockfile() to lock files for threading.
 *
 * Returns 0 on success, -1 on failure.
 */

int ta_lock_file_heartbeat_threaded(const char *path, FILE *fp,
                                    const char *type,
                                    TA_heartbeat_t heartbeat_func)
{
  char message[BUFSIZ];
  if (ta_lock_file_heartbeat(path, fp, type, heartbeat_func)) {
    return -1;
  }
  sprintf(message, "Waiting for ftrylockfile() to succeed on file '%s'",
	  path);
  while (ftrylockfile(fp) ) {
    if (heartbeat_func != NULL) {
      heartbeat_func(message);
    }
    umsleep(1000);
  }
  return 0;
}

/*********************************************
 * ta_lock_file_heartbeat_fd()
 *
 * Sets up read or write lock on entire file.
 * Loops waiting for lock.
 *
 * If heartbeat function is non-NULL, calls it while waiting.
 *
 * File must already be open - path is passed in for
 * error reporting only.
 *
 * Type is "r" or "w", for read or write lock respectively.
 *
 * Returns 0 on success, -1 on failure.
 */

int ta_lock_file_heartbeat_fd(const char *path, int fd,
                              const char *type,
                              TA_heartbeat_t heartbeat_func)

{

  char message[BUFSIZ];
  struct flock lock;

  sprintf(message, "Waiting for '%s' lock on file '%s'",
	  type, path);

  if (!strcmp(type, "r")) {
    lock.l_type = F_RDLCK;
  } else if (!strcmp(type, "w")) {
    lock.l_type = F_WRLCK;
  } else {
    fprintf(stderr, "WARNING - ta_lock_file\n"
	    "  Illegal lock type: '%s', file path '%s'\n",
	    path, type);
    return (-1);
  }
  lock.l_start = 0;
  lock.l_whence = SEEK_SET;
  lock.l_len = 0;
  
  while (fcntl(fd, F_SETLK, &lock) == -1) {
    if (errno == EACCES || errno == EAGAIN) {
      if (heartbeat_func != NULL) {
        heartbeat_func(message);
      }
      sleep(1);
    } else {
      fprintf(stderr,
	      "WARNING - ta_lock_file - cannot lock, type '%s'\n", type);
      perror(path);
      return (-1);
    }
  }
  
  return (0);

}

/*********************************************
 * ta_unlock_file()
 *
 * Clear lock on entire file.
 *
 * File must already be open - path is passed in for
 * error reporting only.
 *
 * Returns 0 on success, -1 on failure.
 */

int ta_unlock_file(const char *path, FILE *fp)

{

  if (fp == NULL) {
    return (-1);
  }
  return (ta_unlock_file_fd(path, fileno(fp)));
}

/*********************************************
 * ta_unlock_file_threaded()
 *
 * Same as ta_unlock_file(), except also calls
 * funlockfile() to unlock the file for threading.
 *
 * Returns 0 on success, -1 on failure.
 */

int ta_unlock_file_threaded(const char *path, FILE *fp)

{
  if (ta_unlock_file(path, fp)) {
    return -1;
  }
  funlockfile(fp);
  return 0;
}

/*********************************************
 * ta_unlock_file_fd()
 *
 * Clear lock on entire file.
 *
 * File must already be open - path is passed in for
 * error reporting only.
 *
 * Returns 0 on success, -1 on failure.
 */

int ta_unlock_file_fd(const char *path, int fd)

{

  struct flock lock;

  lock.l_type = F_UNLCK;
  lock.l_start = 0;
  lock.l_whence = SEEK_SET;
  lock.l_len = 0;
  
  if (fcntl(fd, F_SETLKW, &lock) == -1) {
    fprintf(stderr,
	    "WARNING - ta_lock_file - cannot unlock\n");
    perror(path);
    return (-1);
  }
  
  return (0);

}

/*********************************************
 * ta_create_lock_file()
 *
 * Creates a lock file with the given path.
 * If the file does not exist it is created and opened.
 * If it exists, it is opened.
 * After opening it is write locked.
 *
 * Returns FILE pointer on success, NULL on failure.
 * Failure occurs either because the file cannot be
 * opened, or because the file is already write-locked.
 */

FILE *ta_create_lock_file(const char *lock_file_path)

{

  FILE *fd;
  struct flock lock;

  /*
   * open the file
   */
  
  if ((fd = fopen(lock_file_path, "w+")) == NULL) {
    fprintf(stderr, "ERROR - cannot create lock file '%s'.\n",
	    lock_file_path);
    return (NULL);
  }
  
  /*
   * check if a lock already exists
   */

  lock.l_type = F_WRLCK;
  lock.l_start = 0;
  lock.l_whence = SEEK_SET;
  lock.l_len = 0;
  
  if (fcntl(fileno(fd), F_GETLK, &lock) < 0) {
    fprintf(stderr, "ERROR - cannot check lock file '%s'.\n",
	    lock_file_path);
    fclose(fd);
    return (NULL);
  }
  
  if (lock.l_type != F_UNLCK) {
    fprintf(stderr, "ERROR - file '%s' already locked - cannot access.\n",
	    lock_file_path);
    fclose(fd);
    return (NULL);
  }

  /*
   * get a lock on the file
   */
    
  if (ta_lock_file_procmap(lock_file_path, fd, "w")) {
    fprintf(stderr, "ERROR - Cannot get lock on file '%s'.\n",
	    lock_file_path);
    fclose(fd);
    return (NULL);
  }

  /*
   * success
   */

  return (fd);

}

/*********************************************
 * ta_unlock_lock_file()
 *
 * Unlocks the file pointer and closes it.
 *
 * Returns 0 on success, -1 on failure.
 */

int ta_unlock_lock_file(const char *lock_file_path, FILE *fd)

{
  int iret = 0;

  if (ta_unlock_file(lock_file_path, fd)) {
    fprintf(stderr, "ERROR - Cannot unlock file '%s'.\n",
	    lock_file_path);
    iret = -1;
  }

  if (fclose(fd)) {
    fprintf(stderr, "ERROR - Cannot close lock file '%s'.\n",
	    lock_file_path);
    iret = -1;
  }

  return (iret);

}

/*********************************************
 * ta_remove_lock_file()
 *
 * Unlocks the file pointer and closes it.
 * Removes the lock file with the given path.
 *
 * Returns 0 on success, -1 on failure.
 */

int ta_remove_lock_file(const char *lock_file_path, FILE *fd)

{
  int iret = 0;

  if (ta_unlock_lock_file(lock_file_path, fd)) {
    fprintf(stderr, "ERROR - Cannot unlock file '%s'.\n",
	    lock_file_path);
    iret = -1;
  }

  if (unlink(lock_file_path)) {
    fprintf(stderr, "ERROR - Cannot remove lock file '%s'.\n",
	    lock_file_path);
    iret = -1;
  }

  return (iret);

}

/*********************************************************************
 * ta_tmp_path_from_dir()
 *
 * Given a final file dir, fills a string with a
 * temporary file path.
 *
 * The intended use is to provide a tmp file path to which a file
 * is written prior to renaming to the final name.
 *
 * The tmp path is in the same directory as the final path.
 *
 * If tmp_file_name is non-null, it is used for the file name.
 * If it is NULL, the name is 'tmp.pid.tmp', where pid is
 * determined using the getpid() function.
 *
 * Memory for the string tmp_file_path must be allocated by the calling
 * routine to be at least max_path_len long.
 *
 */

void ta_tmp_path_from_dir(const char *final_file_dir,
			  char *tmp_file_path,
			  int max_path_len,
			  const char *tmp_file_name)

{

  char tmp_pid_name[128];

  /*
   * load up the tmp path
   */

  STRncopy(tmp_file_path, final_file_dir, max_path_len);
  STRconcat(tmp_file_path, PATH_DELIM, max_path_len);

  if (tmp_file_name != NULL) {
    STRconcat(tmp_file_path, tmp_file_name, max_path_len);
  } else {
    sprintf(tmp_pid_name, "tmp.%d.tmp", getpid());
    STRconcat(tmp_file_path, tmp_pid_name, max_path_len);
  }

}

/*********************************************************************
 * ta_tmp_path_from_final()
 *
 * Given a final file path, fills a string with a
 * temporary file path.
 *
 * The intended use is to provide a tmp file path to which a file
 * is written prior to renaming to the final name.
 *
 * The tmp path is in the same directory as the final path.
 *
 * If tmp_file_name is non-null, it is used for the file name.
 * If it is NULL, the name is 'tmp.pid.tmp', where pid is
 * determined using the getpid() function.
 *
 * Memory for the string tmp_file_path must be allocated by the calling
 * routine to be at least max_path_len long.
 *
 */

void ta_tmp_path_from_final(const char *final_file_path,
			    char *tmp_file_path,
			    int max_path_len,
			    const char *tmp_file_name)

{

  char tmp_pid_name[128];
  path_parts_t parts;

  /*
   * parse the parts of the final file path
   */
  
  uparse_path(final_file_path, &parts);

  /*
   * load up the tmp path
   */

  STRncopy(tmp_file_path, parts.dir, max_path_len);

  if (tmp_file_name != NULL) {
    STRconcat(tmp_file_path, tmp_file_name, max_path_len);
  } else {
    sprintf(tmp_pid_name, "tmp.%d.tmp", getpid());
    STRconcat(tmp_file_path, tmp_pid_name, max_path_len);
  }

  /*
   * free up
   */

  ufree_parsed_path(&parts);

}

/******************************************************
 * ta_read_select - waits for read access on a file ptr
 *
 * returns 1 on success, -1 on timeout, -2 on failure
 *
 * Blocks if wait_msecs == -1
 */

int ta_read_select(FILE *fp, long wait_msecs)

{
  
  int fd = fileno(fp);

  return (ta_fd_read_select(fd, wait_msecs));

}

/***************************************************************
 * ta_fd_read_select - waits for read access on a file descriptor
 *
 * returns 1 on success, -1 on timeout, -2 on failure
 *
 * Blocks if wait_msecs == -1
 */

int ta_fd_read_select(int fd, long wait_msecs)

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

/******************************************************
 * ta_stat - wrapper for stat()
 *
 * Same functionality as stat(). This function is used in
 * some places because some implementations of stat() use
 * inline functions. This can cause problems in linking
 * C executable with libraries such as toolsa which mix
 * C and C++ code.
 */

int ta_stat(const char *path, stat_struct_t *buf)

{
  return (stat(path, buf));
}


/******************************************************
 * ta_lstat - wrapper for lstat()
 *
 * Same functionality as lstat(). This function is used in
 * some places because some implementations of lstat() use
 * inline functions. This can cause problems in linking
 * C executable with libraries such as toolsa which mix
 * C and C++ code.
 */

int ta_lstat(const char *path, stat_struct_t *buf)

{
  return (lstat(path, buf));
}

/******************************************************
 * ta_fstat - wrapper for fstat()
 *
 * Same functionality as fstat(). This function is used in
 * some places because some implementations of fstat() use
 * inline functions. This can cause problems in linking
 * C executable with libraries such as toolsa which mix
 * C and C++ code.
 */

int ta_fstat(int filedes, stat_struct_t *buf)

{
  return (fstat(filedes, buf));
}

/******************************************************
 * ta_stat_exists - check if a file exists
 *
 * Returns TRUE if exists, FALSE if not.
 */

int ta_stat_exists(const char *path)

{
  struct stat buf;
  if (stat(path, &buf)) {
    return FALSE;
  }
  return TRUE;
}

/******************************************************
 * ta_stat_exists_compress
 * check if a file exists, also check .gz and .Z extension
 *
 * Returns TRUE if exists, FALSE if not.
 */

int ta_stat_exists_compress(const char *path)

{
  struct stat buf;
  char zpath[MAX_PATH_LEN];
  if (stat(path, &buf) == 0) {
    return TRUE;
  }
  sprintf(zpath, "%s.gz", path);
  if (stat(zpath, &buf) == 0) {
    return TRUE;
  }
  sprintf(zpath, "%s.Z", path);
  if (stat(zpath, &buf) == 0) {
    return TRUE;
  }
  return FALSE;
}

/******************************************************
 * stat a file, whether compressed or not.
 * Checks path, plus .gz and .Z extension
 * Returns 0 on success, -1 on failure
 */

int ta_stat_compressed(const char *path, stat_struct_t *buf)

{
  char zpath[MAX_PATH_LEN];
  if (stat(path, buf) == 0) {
    return 0;
  }
  sprintf(zpath, "%s.gz", path);
  if (stat(zpath, buf) == 0) {
    return 0;
  }
  sprintf(zpath, "%s.Z", path);
  if (stat(zpath, buf) == 0) {
    return 0;
  }
  return -1;
}

/*********************************************************
 * ta_stat_uncompress()
 *
 * stats a file in uncompressed or compressed state.
 * Uncompresses the file if it is compressed.
 *
 * Return is identical to stat()
 */

int ta_stat_uncompress(char *path, stat_struct_t *buf)

{

  char compressed_path[MAX_PATH_LEN];
  int iret;
  stat_struct_t tmp_buf;

  /*
   * if standard stat works, return results
   */
  
  iret = stat(path, buf);

  if (iret == 0) {
    return (iret);
  }
  
  /*
   * check if the compressed file exists.
   * If so, uncompress and stat it
   */
  
  STRncopy(compressed_path, path, MAX_PATH_LEN);
  STRconcat(compressed_path, ".Z", MAX_PATH_LEN);
  if (stat(compressed_path, &tmp_buf) == 0) {
    ta_file_uncompress(compressed_path);
    return (stat(path, buf));
  }

  STRncopy(compressed_path, path, MAX_PATH_LEN);
  STRconcat(compressed_path, ".gz", MAX_PATH_LEN);
  if (stat(compressed_path, &tmp_buf) == 0) {
    ta_file_uncompress(compressed_path);
    return (stat(path, buf));
  }

  return (-1);

}

/******************************************************
 * ta_stat_is_file - check if a plain file exists
 *
 * Returns TRUE if exists, FALSE if not.
 */

int ta_stat_is_file(const char *path)

{
  struct stat buf;
  if (stat(path, &buf)) {
    return FALSE;
  }
  if (S_ISREG(buf.st_mode)) {
    return TRUE;
  }
  return FALSE;
}

/******************************************************
 * ta_stat_is_link - check if a link exists
 *
 * Returns TRUE if exists, FALSE if not.
 */

int ta_stat_is_link(const char *path)

{
  struct stat buf;
  if (stat(path, &buf)) {
    return FALSE;
  }
  if (S_ISLNK(buf.st_mode)) {
    return TRUE;
  }
  return FALSE;
}

/**********************************************************
 * ta_stat_is_file_or_link - check if a file or link exists
 *
 * Returns TRUE if exists, FALSE if not.
 */

int ta_stat_is_file_or_link(const char *path)

{
  struct stat buf;
  if (stat(path, &buf)) {
    return FALSE;
  }
  if (S_ISREG(buf.st_mode) || S_ISLNK(buf.st_mode)) {
    return TRUE;
  }
  return FALSE;
}

/**********************************************************
 * ta_stat_is_dir - check if a dir exists
 *
 * Returns TRUE if exists, FALSE if not.
 */

int ta_stat_is_dir(const char *path)

{
  struct stat buf;
  if (stat(path, &buf)) {
    return FALSE;
  }
  if (S_ISDIR(buf.st_mode)) {
    return TRUE;
  }
  return FALSE;
}

/******************************************************
 * ta_stat_get_len - get file len in bytes
 *
 * Returns len on success, -1 on failure
 */

long ta_stat_get_len(const char *path)

{
  struct stat buf;
  if (stat(path, &buf)) {
    return -1;
  }
  return buf.st_size;
}

/******************************************************
 * ta_statfs - wrapper for statfs()
 *
 * Same functionality as stat(). This function is used in
 * some places because some implementations of stat() use
 * inline functions. This can cause problems in linking
 * C executable with libraries such as toolsa which mix
 * C and C++ code.
 */

int ta_statfs(const char *path, statfs_struct_t *buf)

{
  return (statfs(path, buf));
}

/*******************************************************
 *
 * ta_remove()
 *
 * Remove file, if it exists.
 *
 * Returns 0 on success, -1 on error, errno set appropriately.
 */

int ta_remove(const char *path)
     
{
  
  if (ta_stat_exists(path)) {
    return unlink(path);
  }

  return 0;

}

/*******************************************************
 *
 * ta_remove_compressed()
 *
 * Remove compressed version of file path, if it exists.
 *
 * Returns 0 on success, -1 on error, errno set appropriately.
 */

int ta_remove_compressed(const char *path)

{

  char comp_path[MAX_PATH_LEN];

  sprintf(comp_path, "%s.gz", path);
  if (ta_stat_exists(comp_path)) {
    return unlink(comp_path);
  }

  sprintf(comp_path, "%s.Z", path);
  if (ta_stat_exists(comp_path)) {
    return unlink(comp_path);
  }

  return 0;

}
