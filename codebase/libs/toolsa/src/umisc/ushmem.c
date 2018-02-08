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
/****************************************************************************
 * ushmem.c
 *
 * message queue, shared mem and semaphore routines
 *
 * Mike Dixon RAP NCAR Boulder CO USA
 *
 * Jan 1991
 *
 ****************************************************************************/

#include <toolsa/ushmem.h>
#include <toolsa/mem.h>
#include <toolsa/uusleep.h>

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <memory.h>
#include <math.h>
#include <signal.h>
#include <sys/msg.h>

#define SEM_IS_SET 1
#define SEM_IS_NOT_SET 0

#define FAILURE -1
#define SUCCESS 0

#define MAX_RETRIES 3600

#include <sys/types.h>
#if defined(__linux) && !defined(__USE_XOPEN)
#define __USE_XOPEN
#endif
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>

#if !defined(__APPLE__)
#include <sys/msg.h>
#endif

#if defined(__linux)
#if defined(__GNU_LIBRARY__) && !defined(_SEM_SEMUN_UNDEFINED)
/* union semun is defined by including <sys/sem.h> */
#else
/* according to X/OPEN we have to define it ourselves */
union semun {
  int val;                    /* value for SETVAL */
  struct semid_ds *buf;       /* buffer for IPC_STAT, IPC_SET */
  unsigned short int *array;  /* array for GETALL, SETALL */
  struct seminfo *__buf;      /* buffer for IPC_INFO */
};
#endif
#endif

/****************************************************************************
 * umsg_create()
 *
 * create msg queue 
 *
 ****************************************************************************/

int umsg_create(key_t key, int permissions)
{

  int msgid;

  /*
   * create the msg queue
   */

  if ((msgid = msgget(key, permissions | IPC_CREAT)) < 0) {
    fprintf(stderr, "ERROR - umsg_create.\n");
    fprintf(stderr, "Key = %x\n", (int) key);
    perror("Creating msg queue with 'msgget'");
    return (FAILURE);
  } else {
    return(msgid);
  }

}

/****************************************************************************
 * umsg_get()
 *
 * get msg queue - blocks until one is available, or until it times out
 *
 ****************************************************************************/

int umsg_get(key_t key)
{

  int msgid;
  int retries = 0;

  /*
   * get the msg queue
   */

  while ((msgid = msgget(key, 0666)) < 0) {

    uusleep(1000000);
    retries++;

    if (retries > MAX_RETRIES) {

      fprintf(stderr, "ERROR - umsg_get.\n");
      fprintf(stderr, "Key = %x\n", (int) key);
      perror("Getting msg queue with 'msgget' - timed out");
      return (FAILURE);

    }

  }

  return(msgid);

}

/****************************************************************************
 * umsg_recv()
 *
 * reads next message of size nbytes from queue
 *
 * If block is TRUE, waits for data. Otherwise returns immediately if
 * no data available.
 *
 * Memory is allocated for the data - it is reused on next call.
 *
 * Returns pointer to data on success, NULL on failure.
 *
 ****************************************************************************/

void *umsg_recv(int msqid, size_t nbytes, int block)

{

  static unsigned char *buf = NULL;
  static int nalloc = 0;
  int nneeded;
  int flag, iret;
  long type = 0;

  if (block) {
    flag = 0;
  } else {
    flag = IPC_NOWAIT;
  }
  
  /*
   * allocate buffer
   */

  nneeded = nbytes + sizeof(long);
  if (nneeded > nalloc) {
    buf = (unsigned char *) urealloc (buf, nneeded);
    nalloc = nneeded;
  }
  
  iret = msgrcv(msqid, (void *) buf, nbytes, type, flag);

  if (iret != nbytes) {
    if (block) {
      fprintf(stderr, "ERROR - umsg_recv.\n");
      fprintf(stderr, "Msqid = %d, nbytes %d\n", msqid, (int) nbytes);
      perror("Could not read message");
    }
    return (NULL);
  } else {
    return (buf + sizeof(long));
  }

}

/****************************************************************************
 * umsg_remove()
 *
 * remove msg queue
 *
 ****************************************************************************/

int umsg_remove(key_t key)
{

  int msgid;

  /*
   * find the msg queue
   */

  if ((msgid = msgget(key, 0666)) >= 0) {
  
    /*
     * remove the shared memory
     */

    if (msgctl(msgid, IPC_RMID,  (struct msqid_ds *) 0) != 0) {
      fprintf(stderr, "ERROR - umsg_remove.\n");
      perror("Removing msg queue with 'msgctl'");
      return (FAILURE);
    }
  
    return (SUCCESS);

  } else {

    return(FAILURE);

  }

}

/****************************************************************************
 * umsg_snd()
 *
 * send message to queue, length nbytes.
 *
 * If block is TRUE, blocks. Otherwise returns ERROR if immediate service
 * not available.
 *
 * Returns 0 on success, -1 on failure
 *
 ****************************************************************************/

int umsg_snd(int msqid, size_t nbytes, void *data, int block)

{

  static unsigned char *buf = NULL;
  static int nalloc = 0;
  int nneeded;
  int flag, iret;
  long type = 1;
  
  if (block) {
    flag = 0;
  } else {
    flag = IPC_NOWAIT;
  }
  
  /*
   * allocate buffer
   */

  nneeded = nbytes + sizeof(long);
  if (nneeded > nalloc) {
    buf = (unsigned char *) urealloc (buf, nneeded);
    nalloc = nneeded;
  }

  /*
   * load buffer
   */

  memcpy(buf, &type, sizeof(long));
  memcpy(buf + sizeof(long), data, nbytes);

  /*
   * put the message on the queue
   */

  iret = msgsnd(msqid, (void *) buf, nbytes, flag);

  if (iret) {
    if (block) {
      fprintf(stderr, "ERROR - umsg_snd, iret = %d.\n", iret);
      fprintf(stderr, "Msqid = %d, nbytes %d\n", msqid, (int) nbytes);
      perror("Could not send message");
    }
    return (FAILURE);
  } else {
    return (SUCCESS);
  }

}

/****************************************************************************
 * usem_check()
 *
 * check semaphore for value of 0 - returns 0 if sem value is 0,
 * 1 otherwise
 *
 ****************************************************************************/

int usem_check(int sem_id, int n)
{

#if defined(AIX) || defined(SUNOS5_ETG) || defined(CYGWIN)
  int set = 0;
#else
  union semun set;
  set.val = 0;
#endif

  /*
   * test the semaphore
   */

  if (semctl(sem_id, n, GETVAL, set) == 0) {

    return (SEM_IS_NOT_SET);

  } else {

    return (SEM_IS_SET);

  }

}

/****************************************************************************
 * usem_clear()
 *
 * clear semaphore to 0
 *
 ****************************************************************************/

int usem_clear(int sem_id, int n)
{

#if defined(AIX) || defined (SUNOS5_ETG) || defined(CYGWIN)
  int clear = 0;
#else
  union semun clear;
  clear.val = 0;
#endif

  /*
   * clear the semaphore
   */

  if (semctl(sem_id, n, SETVAL, clear) != 0) {

    fprintf(stderr, "ERROR - usem_clear.\n");
    fprintf(stderr, "Sem_id = %d, n = %d\n", sem_id, n);
    perror("Clearing semaphore with 'semctl'");
    return (FAILURE);

  } else {

    return(SUCCESS);

  }

}

/****************************************************************************
 * usem_create()
 *
 * create semaphore 
 *
 ****************************************************************************/

int usem_create(key_t key, int n, int permissions)
{

  int semid;

  /*
   * create the semaphore
   */

  if ((semid = semget(key, n, permissions | IPC_CREAT)) < 0) {
    fprintf(stderr, "ERROR - usem_create.\n");
    fprintf(stderr, "Key = %x, n = %d\n", (int) key, n);
    perror("Creating semaphore with 'semget'");
    return (FAILURE);
  } else {
    return(semid);
  }

}

/****************************************************************************
 * usem_get()
 *
 * get semaphore - blocks until one is available, or until it times out
 *
 ****************************************************************************/

int usem_get(key_t key, int n)
{

  int semid;
  int retries = 0;

  /*
   * get the semaphore
   */

  while ((semid = semget(key, n, 0666)) < 0) {

    uusleep(1000000);
    retries++;

    if (retries > MAX_RETRIES) {

      fprintf(stderr, "ERROR - usem_get.\n");
      fprintf(stderr, "Key = %x, n = %d\n", (int) key, n);
      perror("Getting semaphore with 'semget' - timed out");
      return (FAILURE);

    }

  }

  return(semid);

}

/****************************************************************************
 * usem_remove()
 *
 * remove semaphore
 *
 ****************************************************************************/

int usem_remove(key_t key)
{

  int semid;

#if defined(AIX) || defined(SUNOS5_ETG) || defined(CYGWIN)
  int arg = 0;
#else
  union semun arg;
  arg.val = 0;
#endif

  /*
   * find the semaphore
   */

  if ((semid = semget(key, 0, 0666)) >= 0) {
  
    /*
     * remove the shared memory
     */

    if (semctl(semid, 0, IPC_RMID, arg) != 0) {
      fprintf(stderr, "ERROR - usem_remove.\n");
      perror("Removing semaphore with 'semctl'");
      return (FAILURE);
    }
  
    return (SUCCESS);

  } else {

    return(FAILURE);

  }

}

/****************************************************************************
 * usem_set()
 *
 * set semaphore to 1
 *
 ****************************************************************************/

int usem_set(int sem_id, int n)
{

#if defined(AIX) || defined(SUNOS5_ETG) || defined(CYGWIN)
  int set = 1;
#else
  union semun set;
  set.val = 1;
#endif

  /*
   * set the semaphore
   */

  if (semctl(sem_id, n, SETVAL, set) != 0) {

    fprintf(stderr, "ERROR - usem_set.\n");
    fprintf(stderr, "Sem_id = %d, n = %d\n", sem_id, n);
    perror("Setting semaphore with 'semctl'");
    return (FAILURE);

  } else {

    return(SUCCESS);

  }

}

/****************************************************************************
 * usem_test()
 *
 * test semaphore for value of 0 - blocks until value
 * becomes 0
 *
 ****************************************************************************/

int usem_test(int sem_id, int n)
{

  struct sembuf operand;

  operand.sem_op = 0;
  operand.sem_flg = 0;
  operand.sem_num = n;

  /*
   * test the semaphore
   */

 retry:

  if(semop(sem_id, &operand, 1) != 0) {

    /*
     * retry if system call interrrupt
     */

    if (errno == EINTR)
      goto retry;

    fprintf(stderr, "ERROR - usem_test.\n");
    fprintf(stderr, "Sem_id = %d, n = %d\n", sem_id, n);
    perror("Testing semaphore with 'semop'");
    return (FAILURE);

  } else {

    return(SUCCESS);

  }

}

/****************************************************************************
 * ushm_create()
 *
 * creates and attaches shared memory
 *
 * Mike Dixon RAP NCAR Boulder Co December 1990
 *
 * Obtained from Bob Barron, RAP
 *
 ****************************************************************************/

void *ushm_create(key_t key, int size, int permissions)
{

  void *memory_region;
  int shmid;

  /*
   * create the shared memory 
   */

  if ((shmid = shmget(key, size, permissions | IPC_CREAT)) < 0) {
    fprintf(stderr, "ERROR - ushm_create.\n");
    perror("Getting shared memory with 'shmget'");
    return (NULL);
  }
  
  /*
   * attach the shared memory regions
   */

  errno = 0;

  memory_region = (void *) shmat(shmid, (char *) 0, 0);

  if (errno != 0) {
    fprintf(stderr, "ERROR - ushm_create.\n");
    perror("Attaching shared memory with 'shmat'");
    return (NULL);
  }
  
  return(memory_region);

}

/****************************************************************************
 * ushm_get()
 *
 * gets and attaches shared memory - blocks until the segment is
 * available, or until it times out
 *
 * Mike Dixon RAP NCAR Boulder Co December 1990
 *
 * Hacked from Bob Barron, RAP
 *
 ****************************************************************************/

void *ushm_get(key_t key, int size)
{

  void *memory_region;
  int shmid;
  int retries = 0;

  /*
   * get the shared memory 
   */

  while ((int)(shmid = shmget(key, size, 0666)) < 0) {

    uusleep(1000000);
    retries++;

    if (retries > MAX_RETRIES) {

      fprintf(stderr, "ERROR - ushm_get.\n");
      fprintf(stderr, "Key = %x, size = %d\n", (int) key, size);
      perror("Getting shared memory with 'shmget' - timed out");
      return (NULL);

    }

  }
  
  /*
   * attach the shared memory regions
   */

  errno = 0;

  memory_region = (void *) shmat(shmid, (char *) 0, 0);

  if (errno != 0) {
    fprintf(stderr, "ERROR - ushm_get.\n");
    fprintf(stderr, "Key = %x, size = %d\n", (int) key, size);
    perror("Attaching shared memory with 'shmat'");
    return (NULL);
  }
  
  return(memory_region);

}

/****************************************************************************
 * ushm_exists
 *
 * Return TRUE if the segment currently exists, FALSE otherwise
 *
 ****************************************************************************/

int ushm_exists(key_t key)
{
  return  (shmget(key, 0, 0666) >= 0) ? 1 : 0;
}

/****************************************************************************
 * ushm_nattach
 *
 * get the number of processes currently attached to a shared memory segment
 *
 *
 * Returns the number of processes currently attached to the indicated
 * shared memory segment, or -1 on error.
 *
 * Nancy Rehak RAP NCAR Boulder Co July 1997
 *
 ****************************************************************************/

int ushm_nattach(key_t key)
{
  int shmid;
  struct shmid_ds shm_info;
  
  /*
   * find the shared memory 
   */

  if ((shmid = shmget(key, 0, 0666)) >= 0)
  {
    /*
     * Get the shared memory information
     */

    if (shmctl(shmid, IPC_STAT, &shm_info) != 0) 
    {
      fprintf(stderr, "ERROR - ushm_nattach.\n");
      fprintf(stderr, "Cannot get shared memory info with shmctl\n");
      return (-1);
    }
  
    return (shm_info.shm_nattch);

  }
  else
  {
    return(-1);
  }
}

/****************************************************************************
 * ushm_detach
 *
 * detach from the shared memory segment
 *
 * returns 0 on success, -1 on failure
 *
 * Nancy Rehak RAP NCAR Boulder Co November 1997
 *
 ****************************************************************************/

int ushm_detach(void *shm_ptr)
{
  if (shmdt((char *)shm_ptr) == 0)
    return(SUCCESS);
  else
    return(FAILURE);
}

/****************************************************************************
 * ushm_remove.c
 *
 * remove shared memory segment
 *
 * Mike Dixon RAP NCAR Boulder Co December 1990
 *
 ****************************************************************************/

int ushm_remove(key_t key)
{

  int shmid;

  /*
   * find the shared memory 
   */

  if ((shmid = shmget(key, 0, 0666)) >= 0) {
  
    /*
     * remove the shared memory
     */

    if (shmctl(shmid, IPC_RMID, (struct shmid_ds *) 0) != 0) {
      fprintf(stderr, "ERROR - ushm_remove.\n");
      perror("Removing shared memory with 'shmctl'");
      return (FAILURE);
    }
  
    return (SUCCESS);

  } else {

    return(FAILURE);

  }

}

