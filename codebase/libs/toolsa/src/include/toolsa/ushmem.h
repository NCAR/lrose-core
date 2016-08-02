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

/**********
 * ushmem.h
 **********/

#ifndef ushmem_h
#define ushmem_h

#ifdef __cplusplus
extern "C" {
#endif

/*
 * includes
 */

#include <toolsa/os_config.h>
#include <sys/types.h>

#ifdef __linux
#if !defined __key_t_defined
typedef __key_t key_t;
#define __key_t_defined
#endif
#endif

/* umsg_create()
 * create msg queue
 */

extern int umsg_create(key_t key, int permissions);

/* umsg_get()
 * get msg queue - blocks until one is available, or until it times out
 */

extern int umsg_get(key_t key);

/* umsg_recv()
 * reads next message from queue.
 * If block is TRUE, waits for data. Otherwise returns immediately if
 * no data available.
 *
 * Memory is allocated for the data - it is reused on next call.
 *
 * Returns pointer to data on success, NULL on failure
 */

extern void *umsg_recv(int msqid, size_t nbytes, int block);

/* umsg_remove()
 * remove msg queue
 */

extern int umsg_remove(key_t key);

/* umsg_snd()
 * send message to queue
 * If block is TRUE, blocks. Otherwise returns ERROR if immediate service
 * not available.
 * Returns 0 on success, -1 on failure
 */

extern int umsg_snd(int msqid, size_t nbytes, void *data, int block);

/* usem_check()
 * check semaphore for value of 0 - returns 0 if sem value is 0,
 * 1 otherwise
 */

extern int usem_check(int sem_id, int n);

/* usem_clear()
 * clear semaphore to 0
 */

extern int usem_clear(int sem_id, int n);

/* usem_create()
 * create semaphore 
 */

extern int usem_create(key_t key, int n, int permissions);

/* usem_get()
 * get semaphore - blocks until one is available, or until it times out
 */

extern int usem_get(key_t key, int n);

/* usem_remove()
 * remove semaphore
 */

extern int usem_remove(key_t key);

/* usem_set()
 * set semaphore to 1
 */

extern int usem_set(int sem_id, int n);

/* usem_test()
 * test semaphore for value of 0 - blocks until value
 * becomes 0
 */

extern int usem_test(int sem_id, int n);

/* ushm_create()
 * creates and attaches shared memory
 */

extern void *ushm_create(key_t key, int size, int permissions);

/* Returns TRUE if the segment currently exists, FALSE otherwise */
extern int ushm_exists(key_t key);

/* ushm_get()
 * gets and attaches shared memory - blocks until the segment is
 * available, or until it times out
 */

extern void *ushm_get(key_t key, int size);

/* ushm_nattach()
 * get the number of processes currently attached to a shared memory segment
 */

int ushm_nattach(key_t key);
				 
/* ushm_detach()
 *
 * detach from the shared memory segment
 *
 * returns 0 on success, -1 on failure
 */

int ushm_detach(void *shm_ptr);

/* ushm_remove()
 * remove shared memory segment
 */

extern int ushm_remove(key_t key);

#ifdef __cplusplus
}
#endif

#endif
