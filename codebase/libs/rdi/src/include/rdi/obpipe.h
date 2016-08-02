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
/*******************************************************************

	File: obpipe.h

*******************************************************************/

/*******************************************************************
	Object pipe for interprocess data communication
	Shared memory version
	Nov. 16, 1992

	Notes: This is the old version which may fail in a extremely
	rare competative condition. An improved version is the MMQ module.
	Another problem with this old version is that the returned
	object length can be larger than the actual object length
	due to alignment. This could be easily improved, although I 
	have not done it yet.
	
	There are 4 routines - 

	opopen (int type, int key, int size) opens an object pipe in 
	shared memory with key and size of size. type = OP_READ (0) or
	OP_WRITE0 (1), which indicates that the pipe is for read or 
	write. The contents in the pipe is flushed everytime a 
	communication party connects to the pipe. 
	It returns a integer (!=-1) handle if success or -1 if 
	failed. 

	opread (int obj, char **buf)
	reads an object. This function returns a pointer (*buf)
	in the shared memory which contains the data. The read pointer 
	is updated when this function is called next time. This 
	is necessary to guarantee that the data will not be
	overwritten while the reader is using the data. This means
	that the next call to opread will complete this read.
	(There will be a duplicate object read if the program
	is killed and restarted with reconnection). 
	On success
	it returns the length of the object (probably bigger than
	true size due to alignment). Otherwise, it returns 0 if
	the buffer is empty or -1 if an error is detected.

	opwrite (int sw, int obj, char *buf, int length) 
	writes an object of length bytes to the buffer.
        The functions are controlled by sw:

        sw = WR_UPDATE: write a data segment (in *buf).
        sw = WR_NO_UPDATE: write without updating w_pt 
        sw = UPDATE_ONLY: update write pointer
        sw = GET_POINTER: get a pointer to write (return casted to int)
        sw = WR_DONE: finish writing (must call after sw=GET_POINTER call).

	This function returns 
	1 if success, or the pointer, casted to an integer,
	to the shared memory space if sw = WR_NO_UPDATE, or GET_POINTER;
        0 if client is not running or buffer full;
        -1 if fatal error;
        if return <= 0, that data is not written.

	opclose(int obj) closes the object. It returns
	0 if success or -1 if failed to remove the shared memory.

	The user may call set_obpipe_msg(1) to switch on (or
	set_obpipe_msg(0) for off) the error message printing in 
	the obpipe package.

	The space used for an object is always a contiguous
	space.

	Here we implemented an efficient and reliable object pipe
        without need of a lock scheme.
	Any of the communicating processes can terminate and
	resume the connection at any time. 
	It is uni-directinonal. It is the users resposibility to guarantee
	that there are only one writer and only one reader.
	The user is assumed to use a consistent size when opening
	the object pipe. The maximum message size should normally
	be less than half of the buffer size. We may add a flush function
	for the reader. It is, however, impossible for the writer to flush
	reliablly. 

*******************************************************************/

#define OP_READ  0
#define OP_WRITE  1
#define OP_FLUSH  2


#define WR_UPDATE    2
#define WR_NO_UPDATE 3 
#define UPDATE_ONLY  4
#define GET_POINTER  5
#define WR_DONE      6


struct object_pipe {
    long *r_pt, *w_pt;		/* read/write point */
    long b_size;		/* buffer size for data excluding control(12
				   bytes) */
    char *dbuf;
    int shmid;
    int shm_key;
    short init;			/* init status flag */
    short type;			/* 0 for read; 1 for write; */
    long *rr_pt, rrpt;		/* temp vars for opread */
    long wpt5, wpg5, pointer_returned;	/* temp vars for opwrite */
    long not_updated, ww_pt;	/* temp vars for opwrite */
};
typedef struct object_pipe Object_pipe;

#ifndef RKC

int opopen (int type, int key, int size);
int opread (int obj, char **buf);
int opwrite (int sw, int obj, char *buf, int length);
int opclose (int obj);
void set_obpipe_msg (int sw);

/* we include 2 functions for compatibility */
int send_shm (int sw, int shm_key, char *buf);
int recv_shm (int sw, int key, char **buf);

#else

void set_obpipe_msg ();

#endif

/* 
   recv_shm:
   sw=0: rm shm; 1: get shm segment; 2: read a data segm 
   return length if success, 0 if no data, -1 if error. 

   send_shm:
   sw=0: rm shm; 
   1: get shm segment; 
   2: write a data segment.
   3: write without updating w_pt 
   4: update write pointer
   5: get a pointer to write (return casted to int)
   6: finish writing (must call after sw=5 call ?).

   In writing call, key used for data length in bytes.

   The data length is arbitrary.

   if client is not running or buffer is full, return 1. 
   fatal error - return -1. 
   On success, if sw=3 or 5, return the pointer in shm, else return 0.
   Note: if return 1 or -1, that data is not written     */
#ifdef __cplusplus             
}
#endif
