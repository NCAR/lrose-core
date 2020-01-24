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
#ifdef __cplusplus
 extern "C" {
#endif
#ifndef FILEIO_WAS_INCLUDED
#define FILEIO_WAS_INCLUDED

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifndef JAWS_FE_TECH_TRANSFER
#include <toolsa/heartbeat.h>

typedef struct stat stat_struct_t; 
typedef struct statfs statfs_struct_t; 

/* FILE_IO - cover for file io routines. */

/*********************************************************
 * filecopy() - Utility routine to copy the contents of one
 *              file to another file.
 *
 * Returns -1 on error, 0 otherwise.
 */

extern int filecopy(FILE *dest, FILE *source);

/********************************************************
 * filecopy_by_name()
 *
 * Utility routine to copy the contents of one file
 * to another file.
 *
 * Returns -1 on error, 0 otherwise.
 */

extern int filecopy_by_name(const char *dest_path, const char *source_path);

/********************************************************
 * makedir()
 *
 * Utility routine to create a directory.  If the directory
 * already exists, does nothing. Uses ta_makedir().
 *
 * Returns -1 on error, 0 otherwise.
 */

extern int makedir(const char *path);

/********************************************************
 * ta_makedir()
 *
 * Utility routine to create a directory.  If the directory
 * already exists, does nothing.
 *
 * Returns -1 on error, 0 otherwise.
 */

extern int ta_makedir(const char *path);

/********************************************************
 * ta_makedir_recurse()
 *
 * Utility routine to create a directory recursively.
 * If the directory already exists, does nothing.
 * Otherwise it recurses through the path, making all
 * needed directories.
 *
 * Returns -1 on error, 0 otherwise.
 */
extern int ta_makedir_recurse(const char *path);

/********************************************************
 * ta_makedir_for_file()
 *
 * Utility routine to create a directory recursively,
 * given a file path. The directory name is determined
 * by stripping the file name off the end of the file path.
 * If the directory already exists, does nothing.
 * Otherwise it recurses through the path, making all
 * needed directories.
 *
 * Returns -1 on error, 0 otherwise.
 */

extern int ta_makedir_for_file(const char *file_path);

/********************************************************
 * ta_fread()
 *
 * Wrapper for fread() - takes care of interrupted read.
 * Returns same as fread()
 *
 */

extern size_t ta_fread(void *ptr, size_t size, size_t nitems, FILE *stream);

/*********************************************************
 * ta_fwrite()
 *
 * Wrapper for fwrite() - takes care of interrupted write.
 * Returns same as fwrite()
 */

extern size_t ta_fwrite(const void *ptr, size_t size, size_t nitems, FILE *stream);

/*******************************************************
 *
 * ta_file_uncompress()
 *
 * Uncompresses file if it is compressed and the uncompressed file
 * doesn't already exist.
 * Handles compress and gzip type files.
 * Returns 1 if uncompression done, 0 if not, -1 if error
 * If file_path has .Z or .gz extension, the extension is
 * removed before the function returns.
 */

extern int ta_file_uncompress(char *file_path);

/*********************************************************
 * ta_fopen_uncompress()
 *
 * Uncompresses the file if necessary, then opens it
 *
 * Return is identical to fopen()
 */

extern FILE *ta_fopen_uncompress(const char *filename, const char *type);

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

extern int ta_lock_file(const char *file_path, FILE *fd,
			const char *type);

/*********************************************
 * ta_lock_file_threaded()
 *
 * Same as ta_lock_file(), except it also calls
 * flockfile() to lock files for threading.
 *
 * Returns 0 on success, -1 on failure.
 */

extern int ta_lock_file_threaded(const char *file_path,
				 FILE *fp,
				 const char *type);

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

extern int ta_lock_file_fd(const char *file_path, int fd,
			   const char *type);

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

extern int ta_lock_file_procmap(const char *file_path, FILE *fd,
				const char *type);

/*********************************************
 * ta_lock_file_procmap_threaded()
 *
 * Same as ta_lock_file_threaded(), except it also calls
 * ftrylockfile() to lock files for threading.
 *
 * Returns 0 on success, -1 on failure.
 */

extern int ta_lock_file_procmap_threaded(const char *file_path,
					 FILE *fp,
					 const char *type);

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

extern int ta_lock_file_procmap_fd(const char *file_path, int fd,
				   const char *type);

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

extern int ta_lock_file_heartbeat(const char *file_path, FILE *fp,
                                  const char *type,
                                  TA_heartbeat_t heartbeat_func);

/*********************************************
 * ta_lock_file_heartbeat_threaded()
 *
 * Same as ta_lock_file_heartbeat(), except it also calls
 * ftrylockfile() to lock files for threading.
 *
 * Returns 0 on success, -1 on failure.
 */

extern int ta_lock_file_heartbeat_threaded(const char *file_path, FILE *fp,
                                           const char *type,
                                           TA_heartbeat_t heartbeat_func);

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

extern int ta_lock_file_heartbeat_fd(const char *file_path, int fd,
                                     const char *type,
                                     TA_heartbeat_t heartbeat_func);

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

extern int ta_unlock_file(const char *file_path, FILE *fd);

/*********************************************
 * ta_unlock_file_threaded()
 *
 * Same as ta_unlock_file(), except also calls
 * funlockfile() to unlock the file for threading.
 *
 * Returns 0 on success, -1 on failure.
 */

extern int ta_unlock_file_threaded(const char *file_path, FILE *fp);

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

extern int ta_unlock_file_fd(const char *file_path, int fd);

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

extern FILE *ta_create_lock_file(const char *lock_file_path);

/*********************************************
 * ta_unlock_lock_file()
 *
 * Unlocks the file pointer and closes it.
 *
 * Returns 0 on success, -1 on failure.
 */

extern int ta_unlock_lock_file(const char *lock_file_path, FILE *fd);

/*********************************************
 * ta_remove_lock_file()
 *
 * Unlocks the file pointer and closes it.
 * Removes the lock file with the given path.
 *
 * Returns 0 on success, -1 on failure.
 */

extern int ta_remove_lock_file(const char *lock_file_path, FILE *fd);

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

extern void ta_tmp_path_from_dir(const char *final_file_dir,
				 char *tmp_file_path,
				 int max_path_len,
				 const char *tmp_file_name);

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

extern void ta_tmp_path_from_final(const char *final_file_path,
				   char *tmp_file_path,
				   int max_path_len,
				   const char *tmp_file_name);

/******************************************************
 * ta_read_select - waits for read access on a file ptr
 *
 * returns 1 on success, -1 on timeout, -2 on failure
 *
 * Blocks if wait_msecs == -1
 */

extern int ta_read_select(FILE *fp, long wait_msecs);

/***************************************************************
 * ta_fd_read_select - waits for read access on a file descriptor
 *
 * returns 1 on success, -1 on timeout, -2 on failure
 *
 * Blocks if wait_msecs == -1
 */

extern int ta_fd_read_select(int fd, long wait_msecs);

/******************************************************
 * ta_stat - wrapper for stat()
 *
 * Same functionality as stat(). This function is used in
 * some places because some implementations of stat() use
 * inline functions. This can cause problems in linking
 * C executable with libraries such as toolsa which mix
 * C and C++ code.
 */

extern int ta_stat(const char *path, struct stat *buf);


/******************************************************
 * ta_lstat - wrapper for lstat()
 *
 * Same functionality as stat(). This function is used in
 * some places because some implementations of stat() use
 * inline functions. This can cause problems in linking
 * C executable with libraries such as toolsa which mix
 * C and C++ code.
 */

extern int ta_lstat(const char *path, struct stat *buf);

/******************************************************
 * ta_fstat - wrapper for fstat()
 *
 * Same functionality as fstat(). This function is used in
 * some places because some implementations of fstat() use
 * inline functions. This can cause problems in linking
 * C executable with libraries such as toolsa which mix
 * C and C++ code.
 */

extern int ta_fstat(int filedes, struct stat *buf);

/******************************************************
 * ta_stat_exists - check if a file exists
 *
 * Returns TRUE if exists, FALSE if not.
 */

extern int ta_stat_exists(const char *path);

/******************************************************
 * ta_stat_exists_compress
 * check if a file exists, also check .gz and .Z extension
 *
 * Returns TRUE if exists, FALSE if not.
 */

extern int ta_stat_exists_compress(const char *path);

/*********************************************************
 * ta_stat_uncompress()
 *
 * stats a file in uncompressed or compressed state.
 *
 * Return is identical to stat()
 */

extern int ta_stat_uncompress(char *path, stat_struct_t *buf);

/******************************************************
 * stat a file, whether compressed or not.
 * Checks path, plus .gz and .Z extension
 * Returns 0 on success, -1 on failure
 */

extern int ta_stat_compressed(const char *path, stat_struct_t *buf);

/******************************************************
 * ta_stat_is_file - check if a plain file exists
 *
 * Returns TRUE if exists, FALSE if not.
 */

extern int ta_stat_is_file(const char *path);

/******************************************************
 * ta_stat_is_link - check if a link exists
 *
 * Returns TRUE if exists, FALSE if not.
 */

extern int ta_stat_is_link(const char *path);

/**********************************************************
 * ta_stat_is_file_or_link - check if a file or link exists
 *
 * Returns TRUE if exists, FALSE if not.
 */

extern int ta_stat_is_file_or_link(const char *path);

/**********************************************************
 * ta_stat_is_dir - check if a dir exists
 *
 * Returns TRUE if exists, FALSE if not.
 */

extern int ta_stat_is_dir(const char *path);

/******************************************************
 * ta_stat_get_len - get file len in bytes
 *
 * Returns len on success, -1 on failure
 */

 extern long ta_stat_get_len(const char *path);

/******************************************************
 * ta_statfs - wrapper for statfs()
 *
 * Same functionality as stat(). This function is used in
 * some places because some implementations of stat() use
 * inline functions. This can cause problems in linking
 * C executable with libraries such as toolsa which mix
 * C and C++ code.
 */

extern int ta_statfs(const char *path, statfs_struct_t *buf);

/*******************************************************
 *
 * ta_remove()
 *
 * Remove file, if it exists.
 *
 * Returns 0 on success, -1 on error, errno set appropriately.
 */

extern int ta_remove(const char *file_path);

/*******************************************************
 *
 * ta_remove_compressed()
 *
 * Remove compressed version of file path, if it exists.
 *
 * Returns 0 on success, -1 on error, errno set appropriately.
 */

extern int ta_remove_compressed(const char *file_path);

#else /* specialized subset for JAWS FrontEnd Tech Transfer */

/********************************************************
 * makedir()
 *
 * Utility routine to create a directory.  If the directory
 * already exists, does nothing. Uses ta_makedir().
 *
 * Returns -1 on error, 0 otherwise.
 */

extern int makedir(const char *path);

/********************************************************
 * ta_makedir()
 *
 * Utility routine to create a directory.  If the directory
 * already exists, does nothing.
 *
 * Returns -1 on error, 0 otherwise.
 */

extern int ta_makedir(const char *path);

/********************************************************
 * ta_makedir_recurse()
 *
 * Utility routine to create a directory recursively.
 * If the directory already exists, does nothing.
 * Otherwise it recurses through the path, making all
 * needed directories.
 *
 * Returns -1 on error, 0 otherwise.
 */
extern int ta_makedir_recurse(const char *path);

/******************************************************
 * ta_stat - wrapper for stat()
 *
 * Same functionality as stat(). This function is used in
 * some places because some implementations of stat() use
 * inline functions. This can cause problems in linking
 * C executable with libraries such as toolsa which mix
 * C and C++ code.
 */

extern int ta_stat(const char *path, struct stat *buf);

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

extern int ta_lock_file(const char *path, FILE *fd,
			const char *type);

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

extern int ta_lock_file_fd(const char *path, int fd,
			   const char *type);

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

extern int ta_unlock_file(const char *path, FILE *fd);

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

extern int ta_unlock_file_fd(const char *path, int fd);

#endif /* JAWS_FE_TECH_TRANSFER */

#endif /*  FILEIO_WAS_INCLUDED */

#ifdef __cplusplus
}
#endif


