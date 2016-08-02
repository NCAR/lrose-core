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
/****************************************************************
		
	File: fmq.c	
				
	3/7/94

	Purpose: This module contains the file message queue, FMQ,
	library routines.

	Files used: fmq.h
	See also: 
	Author: Jing

*****************************************************************/

#ifdef	   LINT
static char RCSid[] = "$Id: fmq.c,v 1.8 2016/03/03 18:47:03 dixon Exp $";
static char SCCSid[] = "%W% %D% %T%";
#endif /* not LINT */


/*** System include files ***/

#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>     /* needed by writev */
#include <sys/uio.h>


/*** Local include files ***/

#include <rdi/fmq.h>


/*** Definitions / macros / types ***/

typedef long jfmq_t;		/* integer type used for FMQ - must be 4
				   bytes long */

/* data structure for an open FMQ */
struct jfmq_struct {
    jfmq_t jfmq_id;		/* file identification number: JFMQ_ID_NUMBER */
    jfmq_t jfmq_size;		/* FMQ size for messages excluding header */
    int hd_size;		/* header size: JFMQ_HEADER_SIZE * sizeof
				   (jfmq_t) */
    int r_off;		/* offset for read pointer */
    int w_off;		/* offset for write pointer */
    int fd;			/* FMQ file fd */
    int mode;			/* FMQ mode */
};
typedef struct jfmq_struct Jfmq_struct;

#define MAX_N_FMQS 16		/* maximum number of open FMQs */
#define JFMQ_HEADER_SIZE 6	/* FMQ file header size */
#define JFMQ_ID_NUMBER 24578594	/* id for identifying a FMQ file */
#define JFMQ_START_FD   144	/* starting number of the FMQ fd */

#define RP_OFFSET 2		/* location of the read pointer */
#define WP_OFFSET 3		/* location of the write pointer */

#define FILL_IN_BUFFER_SIZE 16384	/* size of a temp buffer */

#define JFMQ_SET_LOCK  1		/* argument of Set_lock () */
#define JFMQ_RELEASE_LOCK  0	/* argument of Set_lock () */

#define JFMQ_EXIST 1		/* return value of jfmq_create */
#define JFMQ_END_MESSAGE 1 	/* return value of Set_a_step_* */

#define JFMQ_TRUE   1
#define JFMQ_FALSE  0

#define SHIFT_FOR_PAGE  24	/* shift for page numbers */
#define POINTER_MASK  0xffffff	/* mask for pointers */

#define GET_POINTER(a) (a & POINTER_MASK)	/* macro extracting pointer */
#define GET_PAGE(a) (a >> SHIFT_FOR_PAGE)		/* macro extracting page
				 			   number */

#define ROUND_SIZE(size) (((size + 3) >> 2) << 2) /* size of stored message */

#define USE_WRITEV		/* we use writev version */

/* macro combining page number and pointer */
#define COMBINE_PAGE_POINTER(a, b) (b + (a << SHIFT_FOR_PAGE))


/*** External references / external global variables ***/

extern int errno;

/*** Local references / local variables ***/

static int Set_lock (int sw, int fd);
static Jfmq_struct *Get_jfmq_structure (int fd);
static void Sig_block (int sw);
static int Release_lock_return (int fd, int return_value);
static int Jfmq_create (char *name, int size, int mode);
static int Jfmq_open (char *name, int size, int mode);
static int Read_pointers (Jfmq_struct *fmq, jfmq_t *rpg, jfmq_t *wpg, 
			  jfmq_t *rpt, jfmq_t *wpt);
static int Read_a_message (Jfmq_struct *fmq, int buf_size, jfmq_t *rpt, 
			   jfmq_t *rpg, char *buf);
static int Write_a_message (Jfmq_struct *fmq, jfmq_t length, int n_pad, 
			    char *message, jfmq_t *wpt);

static int Read_a_segment (int fd, off_t where, int len, char *data);

static int Set_a_step_back (Jfmq_struct *fmq, jfmq_t *rpg, jfmq_t *wpg, jfmq_t *rpt,
			    jfmq_t *wpt);
static int Set_a_step_forward (Jfmq_struct *fmq, jfmq_t *rpg, jfmq_t *wpg,	jfmq_t *rpt,
			       jfmq_t *wpt);

static int Initialize_queue_area (int fd, int size);

#ifndef USE_WRITEV
static int Write_a_segment (int fd, off_t where, int len, char *data);
#endif 

/* The opened FMQs */
static Jfmq_struct *Jfmq_handle[MAX_N_FMQS] =
{NULL, NULL, NULL, NULL, NULL,
 NULL, NULL, NULL, NULL, NULL, NULL,
 NULL, NULL, NULL, NULL, NULL};

static int Jfmq_lock = JFMQ_ON;  /* file lock flag */

/****************************************************************
			
	JFMQ_open()				Date: 3/7/94

	This function opens a FMQ of "name" and size of "size".
	If the FMQ does not exist and "mode" > 0, a new FMQ is 
	created and then opened.

	All calling arguments are checked. This function calls
	Jfmq_create and Jfmq_open to perform the task.

	This function returns an FMQ descriptor on success or
	one of the following negative error numbers:

	JFMQ_NULL_NAME: The argument "name" is a null string.
	JFMQ_SIZE_TOO_SMALL: The argument "size" is less than 12 bytes.
	JFMQ_BAD_MODE: The argument "mode" is incorrect. It must be 
		      0600, 0660 or 0666.
	Other error return values from Jfmq_create and Jfmq_open.
*/

int
  JFMQ_open
  (
      char *name,		/* name of the FMQ */
      int size,			/* size of the FMQ */
      int mode			/* mode of the FMQ file */
) {
    int rounded_size;		/* we use size of multiple of 4 */

    /* check name */
    if (strlen (name) == 0)
	return (JFMQ_NULL_NAME);

    /* check size */
    if (size < 3 * sizeof (jfmq_t))  /* The minimum stored size of a message */
	return (JFMQ_SIZE_TOO_SMALL);

    rounded_size = ROUND_SIZE(size);

    if ((mode & JFMQ_CREATE) != 0) {
	int ac_mode;  /* access mode */
	int ret;

	/* check access mode */
	ac_mode = mode & 0777;
	if (ac_mode != 0666 && ac_mode != 0660 && ac_mode != 0600) 
	    return (JFMQ_BAD_MODE);

	/* create the FMQ file */
	ret = Jfmq_create (name, rounded_size, mode);
	if (ret < 0)
	    return (ret);
    }

    /* open the FMQ */
    return (Jfmq_open (name, rounded_size, mode));
}

/****************************************************************
			
	JFMQ_write()				Date: 3/7/94

	This function writes a "msg_length" byte message in "message" 
	to the FMQ "fd". Refer to fmq.doc for a description on
	how a message is stored in the FMQ. We will not explain
	the steps in this function since they are quite straight-
	forward.

	If "message" is a NULL pointer, this function returns  
	0 without writing any message if there is enough
	free space in the FMQ.

	This function returns the message length on success or one
	of the following negative numbers indicating an error condition:

	JFMQ_INVALID_FD: The "fd" argument is not a valid FMQ descriptor.
	JFMQ_MSG_TOO_LARGE: The message size is larger than the FMQ size.
	JFMQ_LOCK_FAILED: Failed in locking the FMQ.
	JFMQ_READ_HD_FAILED: The FMQ has been corrupted - could not read 
			header or the FMQ id is incorrect.
	JFMQ_FULL: The FMQ is full. There is no room for the new message.
	JFMQ_DATA_WRITE_FAILED: Failed in writing the message.
	JFMQ_WRITE_HD_FAILED: Failed in updating the FMQ header.
	JFMQ_BAD_HD_INFOR: The FMQ has been corrupted - incorrect header 
		      information was found. 
*/

int
  JFMQ_write
  (
      int fd,			/* FMQ descriptor */
      char *message,		/* message to be written */
      int msg_length		/* message length */
) {
    Jfmq_struct *fmq;		/* FMQ structure */
    int stored_len;		/* stored length of the message */
    int n_pad;			/* number of padding bytes */
    jfmq_t rpt;			/* read pointer */
    jfmq_t wpt;			/* write pointer */
    jfmq_t rpg;			/* read page number */
    jfmq_t wpg;			/* write page number */
    int free_size;		/* size of free FMQ space */
    jfmq_t length;		/* message length */
    int ret;

    length = msg_length;

    if (length <= 0)
	return (0);

    /* get the fmq structure */
    if ((fmq = Get_jfmq_structure (fd)) == NULL)
	return (JFMQ_INVALID_FD);

    /* message size in the fmq */
    stored_len = ROUND_SIZE(length);
    n_pad = stored_len - length;
    stored_len += 2 * sizeof (jfmq_t); /* add space for length info */

    /* check if message is larger than the fmq size */
    if (stored_len > fmq->jfmq_size)
	return (JFMQ_MSG_TOO_LARGE);

    /* lock the file */
    if (Set_lock (JFMQ_SET_LOCK, fmq->fd) == JFMQ_FAILURE)
	return (JFMQ_LOCK_FAILED);

    /* get the pointers */
    ret = Read_pointers (fmq, &rpg, &wpg, &rpt, &wpt);
    if (ret != JFMQ_SUCCESS ) 
	return (Release_lock_return (fmq->fd, ret));

    /* find free space size in FMQ */
    if (rpg == wpg)   /* on the same page */
	free_size = fmq->jfmq_size - (wpt - rpt);
    else
	free_size = rpt - wpt;

    /* check free space size */
    if (stored_len > free_size)	/* FMQ full */
	return (Release_lock_return (fmq->fd, JFMQ_FULL));

    /* return if message is NULL */
    if (message == NULL) 
	return (Release_lock_return (fmq->fd, 0));

    /* update page number */
    if (stored_len >= fmq->jfmq_size - wpt)
	wpg = (wpg + 1) % 2;

    /* write the message */
    ret = Write_a_message (fmq, length, n_pad, message, &wpt);
    if (ret != JFMQ_SUCCESS ) 
	return (Release_lock_return (fmq->fd, ret));

    /* update the header */
    wpt = COMBINE_PAGE_POINTER (wpg, wpt);
    Sig_block (JFMQ_TRUE); 
    if (lseek (fmq->fd, (off_t)fmq->w_off, SEEK_SET) < 0 ||
        write (fmq->fd, (char *) &wpt, sizeof (jfmq_t)) < sizeof (jfmq_t)) {
	Sig_block (JFMQ_FALSE);
	return (Release_lock_return (fmq->fd, JFMQ_WRITE_HD_FAILED));
    }
    Sig_block (JFMQ_FALSE);

    return (Release_lock_return (fmq->fd, length));

}

#ifndef USE_WRITEV

/****************************************************************
			
	Write_a_message()			Date: 3/7/94
	
	This function writes a "length" byte message "message" at
	"wpt" in the FMQ "fmq". "wpt" is updated in the function.

	Three parts need to be written for a message: The leading 
	length, the message and the following length. Refer to 
	fmq.doc for a description of the message format used
	in FMQ. If the message is at the end of the FMQ, the second
	part of the message may need to be continued at the beginning
	of the FMQ.

	This function returns JFMQ_SUCCESS on success or 		
	JFMQ_DATA_WRITE_FAILED if it failed in writing the message.

	This version uses the "write" system call.
*/

static int 
  Write_a_message
  (
    Jfmq_struct *fmq,		/* FMQ structure */
    jfmq_t length,		/* message length */
    int n_pad,			/* number of padding bytes */
    char *message,		/* message to be written */
    jfmq_t *wpt			/* write pointer */
) {
    int first_len;		/* length of the first part of message */
    int ret;

    /* write the first length */
    ret = Write_a_segment (fmq->fd, (off_t)(fmq->hd_size + *wpt), sizeof (jfmq_t), 
			   (char *) &length);
    if (ret != JFMQ_SUCCESS) return (ret);
    *wpt = (*wpt + sizeof (jfmq_t)) % fmq->jfmq_size;

    /* write the first part of the data */
    first_len = length;
    if (first_len > fmq->jfmq_size - *wpt)
	first_len = fmq->jfmq_size - *wpt;
    ret = Write_a_segment (fmq->fd, (off_t)(fmq->hd_size + *wpt), first_len, message);
    if (ret != JFMQ_SUCCESS) return (ret);
    *wpt = (*wpt + first_len) % fmq->jfmq_size;

    /* write the second part of the data */
    if (first_len < length) {
        ret = Write_a_segment (fmq->fd, (off_t)(fmq->hd_size + *wpt), 
				length - first_len, message + first_len);
        if (ret != JFMQ_SUCCESS) return (ret);
	*wpt = (*wpt + length - first_len) % fmq->jfmq_size;
    }

    /* add padding bytes */
    *wpt = (*wpt + n_pad) % fmq->jfmq_size;

    /* write the second length */
    ret = Write_a_segment (fmq->fd, (off_t)(fmq->hd_size + *wpt), sizeof (jfmq_t), 
			   (char *) &length);
    if (ret != JFMQ_SUCCESS) return (ret);
    *wpt = (*wpt + sizeof (jfmq_t)) % fmq->jfmq_size;

    return (JFMQ_SUCCESS);
}

/****************************************************************
			
	Write_a_segment()			Date: 3/7/94

	This function writes a segment of data in "data" into
	file "fd" at offset "where". The length of the data segment is 
	"len".

	The function returns JFMQ_SUCCESS on success, JFMQ_WRITE_INTERRUPTED
	if the write is interrupted by a signal, or JFMQ_DATA_WRITE_FAILED
	if one of other fatal errors is encountered.
	
*/

static int 
  Write_a_segment 
  (
    int fd,			/* file fd */
    off_t where,		/* file offset */
    int len,			/* length of the data segment */
    char *data			/* data buffer */
) {
    int ret;

    if (lseek (fd, where, SEEK_SET) < 0)
	return (JFMQ_DATA_WRITE_FAILED);

    ret = write (fd, data, len);
    if (ret < len) {
	if(ret < 0 && errno != EINTR) 
	    return (JFMQ_DATA_WRITE_FAILED);
	else return (JFMQ_WRITE_INTERRUPTED);
    }

    return (JFMQ_SUCCESS);
}

#else

/****************************************************************
			
	Write_a_message()			Date: 3/7/94
	
	This function writes a "length" byte message "message" at
	"wpt" in the FMQ "fmq". "wpt" is updated in the function.

	Three parts need to be written for a message: The leading 
	length, the message and the following length. Refer to 
	fmq.doc for a description of the message format used
	in FMQ. If the message is at the end of the FMQ, the second
	part of the message may need to be continued at the beginning
	of the FMQ.

	This function returns JFMQ_SUCCESS on success or 		
	JFMQ_DATA_WRITE_FAILED if it failed in writing the message.

	This version uses the "writev" system call, which is faster.
*/

static int 
  Write_a_message
  (
    Jfmq_struct *fmq,		/* FMQ structure */
    jfmq_t length,		/* message length */
    int n_pad,			/* number of padding bytes */
    char *message,		/* message to be written */
    jfmq_t *wpt			/* write pointer */
) {
    int first_len;		/* length of the first part of message */
    int stored_len;		/* message stored length */
    struct iovec iov[4];	/* structure for writev */
    char pad_bytes[4];
    int first_space;		/* size of the first (FMQ end) free space */
    int len;			/* length to be written by next writev */
    int cnt;			/* number of pieces for next writev */
    int ret;

    /* position the file pointer */
    if (lseek (fmq->fd, (off_t)(fmq->hd_size + *wpt), SEEK_SET) < 0)
	return (JFMQ_DATA_WRITE_FAILED);

    /* prepare parameters */
    stored_len = length + n_pad + 2 * sizeof (jfmq_t);
    first_space = fmq->jfmq_size - *wpt;
    first_len = length;
    if (first_len > first_space - sizeof (jfmq_t))
	first_len = first_space - sizeof (jfmq_t);

    /* write the first part data */
    cnt = 0;
    len = 0;
    if (first_space >= 4) {
	iov[cnt].iov_base = (char *) &length;
	iov[cnt++].iov_len = sizeof (jfmq_t);
	len += sizeof (jfmq_t);
    }
    if(first_space > 4) {
	iov[cnt].iov_base = (char *) message;
	iov[cnt++].iov_len = first_len;
	len += first_len;
    }
    if (first_len == length) {
	if(n_pad > 0) {
	    iov[cnt].iov_base = pad_bytes;
	    iov[cnt++].iov_len = n_pad;
	    len += n_pad;
	}
    }
    if (first_space >= stored_len) {
	iov[cnt].iov_base = (char *) &length;
	iov[cnt++].iov_len = sizeof (jfmq_t);
	len += sizeof (jfmq_t);
    }

    ret = writev (fmq->fd, iov, cnt);
    if (ret < len) {
	if (ret < 0 && errno != EINTR)
	    return (JFMQ_DATA_WRITE_FAILED);
	else return (JFMQ_WRITE_INTERRUPTED);
    }
    *wpt = (*wpt + len) % fmq->jfmq_size;

    /* write the second part data */
    cnt = 0;
    len = 0;
    if (first_space < 4) {
	iov[cnt].iov_base = (char *) &length;
	iov[cnt++].iov_len = sizeof (jfmq_t);
	len += sizeof (jfmq_t);
    }
    if(first_len < length) {
	iov[cnt].iov_base = (char *) (message + first_len);
	iov[cnt++].iov_len = length - first_len;
	len += length - first_len;
	if(n_pad > 0) {
	    iov[cnt].iov_base = pad_bytes;
	    iov[cnt++].iov_len = n_pad;
	    len += n_pad;
	}
    }
    if (first_space < stored_len) {
	iov[cnt].iov_base = (char *) &length;
	iov[cnt++].iov_len = sizeof (jfmq_t);
	len += sizeof (jfmq_t);
    }

    if (cnt > 0) {
        if (lseek (fmq->fd, (off_t)(fmq->hd_size + *wpt), SEEK_SET) < 0)
	    return (JFMQ_DATA_WRITE_FAILED);
        ret = writev (fmq->fd, iov, cnt);
        if (ret < len) {
	    if (ret < 0 && errno != EINTR)
	        return (JFMQ_DATA_WRITE_FAILED);
	    else return (JFMQ_WRITE_INTERRUPTED);
        }
    }
    *wpt = (*wpt + len) % fmq->jfmq_size;

    return (JFMQ_SUCCESS);
}

#endif

/****************************************************************
			
	JFMQ_read()				Date: 3/7/94

	This function reads a message from the FMQ "fd" and puts
	it in "buf" provided the message is not larger than "buf_size". 
	Refer to fmq.doc for a description on how a message is stored 
	in the FMQ. We will not explain the steps in this function 
	since they are quite straightforward.

	This function returns the message length on success or one
	of the following negative numbers indicating an error condition:

	JFMQ_INVALID_FD: The "fd" argument is not a valid FMQ descriptor.
	JFMQ_BUFFER_TOO_SMALL: The message size is larger than the buffer size.
	JFMQ_LOCK_FAILED: Failed in locking the FMQ.
	JFMQ_READ_HD_FAILED: The FMQ has been corrupted - could not read 
			header or the FMQ id is incorrect.
	JFMQ_EMPTY: There is no message in the FMQ.
	JFMQ_DATA_READ_FAILED: Failed while reading the FMQ massage.
	JFMQ_WRITE_HD_FAILED: Failed in updating the FMQ header.
	JFMQ_BAD_HD_INFOR: The FMQ has been corrupted - incorrect header 
		      information was found. 
	JFMQ_READ_LEN_FAILED: The message length is found to be incorrect - FMQ 
			 is corrupted.
*/

int
  JFMQ_read
  (
      int fd,			/* FMQ descriptor */
      char *buf,		/* message buffer for the message */
      int buf_size		/* buffer size */
) {
    Jfmq_struct *fmq;		/* FMQ handle */
    jfmq_t rpt;			/* read pointer */
    jfmq_t wpt;			/* write pointer */
    jfmq_t rpg;			/* read page number */
    jfmq_t wpg;			/* write page number */
    int length; 		/* read message length */
    int ret;

    /* get the fmq structure */
    if ((fmq = Get_jfmq_structure (fd)) == NULL)
	return (JFMQ_INVALID_FD);

    /* lock the file */
    if (Set_lock (JFMQ_SET_LOCK, fmq->fd) == JFMQ_FAILURE)
	return (JFMQ_LOCK_FAILED);

    /* get the pointers */
    ret = Read_pointers (fmq, &rpg, &wpg, &rpt, &wpt);
    if (ret != JFMQ_SUCCESS ) 
	return (Release_lock_return (fmq->fd, ret));

    /* check if message available */
    if (rpg == wpg && rpt >= wpt)	/* message not available */
	return (Release_lock_return (fmq->fd, JFMQ_EMPTY));

    /* Read a message at rpt. Point rpt and page # rpg are updated on success */
    ret = Read_a_message (fmq, buf_size, &rpt, &rpg, buf);
    if (ret < 0) 
	return (Release_lock_return (fmq->fd, ret));
    length = ret;

    /* update the header */
    rpt = COMBINE_PAGE_POINTER (rpg, rpt);
    Sig_block (JFMQ_TRUE);
    if (lseek (fmq->fd, (off_t) fmq->r_off, SEEK_SET) < 0 ||
        write (fmq->fd, (char *) &rpt, sizeof (jfmq_t)) < sizeof (jfmq_t)) {
	Sig_block (JFMQ_FALSE);
	return (Release_lock_return (fmq->fd, JFMQ_WRITE_HD_FAILED));
    }
    Sig_block (JFMQ_FALSE);

    return (Release_lock_return (fmq->fd, length));
}

/****************************************************************
			
	Read_a_message()			Date: 3/7/94

	This function reads a message at position "rpt" in the FMQ 
	"fmq" and puts the message in "buf" of size "buf_size". The
	read page number "rpg" and pointer "rpt" are updated after
	message read.

	This function returns the length of the message on success 
	or one of the following negative error numbers:

	JFMQ_BUFFER_TOO_SMALL: The message size is larger than the buffer size.
	JFMQ_DATA_READ_FAILED: Failed while reading the FMQ massage.
	JFMQ_READ_LEN_FAILED: The message length is found to be incorrect - FMQ 
			 is corrupted.
*/

static int
  Read_a_message
  (
    Jfmq_struct *fmq,     	/* FMQ structure */
    int buf_size,		/* size of buffer */
    jfmq_t *rpt,			/* read pointer */
    jfmq_t *rpg,			/* read page number */
    char *buf			/* buffer for message read (output) */
) {
    int stored_len;		/* stored length of the message */
    int first_len;		/* first part of data length */
    jfmq_t length;		/* message length */
    int n_pad;			/* number of padding bytes */
    int ret;

    /* read message length */
    ret = Read_a_segment (fmq->fd, (off_t)(fmq->hd_size + *rpt), sizeof (jfmq_t), 
			  (char *) &length);
    if (ret != JFMQ_SUCCESS) return (ret);
    if (length < 0 || length + 2 * sizeof (jfmq_t) > fmq->jfmq_size)
	return (JFMQ_READ_LEN_FAILED);

    /* check buffer size */
    if (length > buf_size)
	return (JFMQ_BUFFER_TOO_SMALL);

    /* message size in the fmq */
    stored_len = ROUND_SIZE(length);
    n_pad = stored_len - length;
    stored_len += 2 * sizeof (jfmq_t); /* augment length to include storage for 
					 length info */

    /* update page number */
    if (stored_len >= fmq->jfmq_size - *rpt)
	*rpg = (*rpg + 1) % 2;

    /* read the data */
    *rpt = (*rpt + sizeof (jfmq_t)) % fmq->jfmq_size;

    /* read the first part data */
    first_len = length;
    if (first_len > fmq->jfmq_size - *rpt)
	first_len = fmq->jfmq_size - *rpt;
    ret = Read_a_segment (fmq->fd, (off_t)(fmq->hd_size + *rpt), first_len, buf);
    if (ret != JFMQ_SUCCESS) return (ret);
    *rpt = (*rpt + first_len) % fmq->jfmq_size;

    /* read the second part data */
    if (first_len < length) {
        ret = Read_a_segment (fmq->fd, (off_t)(fmq->hd_size + *rpt), 
				length - first_len, buf + first_len);
        if (ret != JFMQ_SUCCESS) return (ret);
	*rpt = (*rpt + length - first_len) % fmq->jfmq_size;
    }

    /* add padding bytes */
    *rpt = (*rpt + n_pad) % fmq->jfmq_size;

    /* second length is ignored */
    *rpt = (*rpt + sizeof (jfmq_t)) % fmq->jfmq_size;

    return (length);
}

/****************************************************************
			
	Read_a_segment()			Date: 3/7/94

	This function reads a segment of data in file "fd" at 
	offset "where" and puts it in buffer "data". The length of 
	the data read is "len".

	The function returns JFMQ_SUCCESS on success, JFMQ_READ_INTERRUPTED
	if the read is interrupted by a signal, or JFMQ_DATA_READ_FAILED
	if one of other fatal errors is encountered.
	
*/

static int 
  Read_a_segment 
  (
    int fd,			/* file fd */
    off_t where,		/* file offset */
    int len,			/* length of the data segment */
    char *data			/* data buffer */
) {
    int ret;

    if (lseek (fd, where, SEEK_SET) < 0)
	return (JFMQ_DATA_READ_FAILED);

    ret = read (fd, data, len);
    if (ret < len) {
	if(ret < 0 && errno != EINTR)
	    return (JFMQ_DATA_READ_FAILED);
	else return (JFMQ_READ_INTERRUPTED);
    }

    return (JFMQ_SUCCESS);
}


/****************************************************************
			
	JFMQ_reset_rpt()				Date: 3/7/94

	This function resets the read pointer |"n_steps"| backward 
	(if "n_steps" < 0) or forward (if "n_steps" > 0). If
	"n_steps" = 0, the function does nothing. 

	The backward or forward movement is limited be available
	messges. That is, for backward movement, the read pointer
	will be moved at most to the oldest message that has not been
	overwritten, and, for forward movement, the read pointer
	will be moved at most to the latest message.
	
	This function calls Set_a_step_back or Set_a_step_forward
	repeatly until the required movement is performed or a
	limit is reached.

	The function returns the number of steps actually moved.

*/

int
  JFMQ_reset_rpt
  (
      int fd,			/* FMQ descriptor */
      int n_steps		/* number of steps to move */
) {
    int i;
    Jfmq_struct *fmq;		/* FMQ handle */
    jfmq_t rpt;			/* read pointer */
    jfmq_t wpt;			/* write pointer */
    jfmq_t rpg;			/* read page number */
    jfmq_t wpg;			/* write page number */
    int ret;
    int cnt;
    int set_ret;

    if(n_steps == 0) return (0);

    /* get the fmq structure */
    if ((fmq = Get_jfmq_structure (fd)) == NULL)
	return (JFMQ_INVALID_FD);

    /* lock the file */
    if (Set_lock (JFMQ_SET_LOCK, fmq->fd) == JFMQ_FAILURE)
	return (JFMQ_LOCK_FAILED);

    /* get the pointers */
    ret = Read_pointers (fmq, &rpg, &wpg, &rpt, &wpt);
    if (ret != JFMQ_SUCCESS ) 
	return (Release_lock_return (fmq->fd, ret));

    /* reset the read pointer n_steps steps */
    cnt = 0;
    if (n_steps < 0) {
	for (i = 0; i < -n_steps; i++) {
	    set_ret = Set_a_step_back (fmq, &rpg, &wpg, &rpt, &wpt);
	    if (set_ret == JFMQ_SUCCESS) {
		cnt++;
		continue;
	    }
	    if (set_ret == JFMQ_END_MESSAGE) 
		break;
	    return (Release_lock_return (fmq->fd, set_ret));
	}
    }
    else {
	for (i = 0; i < n_steps; i++) {
	    set_ret = Set_a_step_forward (fmq, &rpg, &wpg, &rpt, &wpt);
	    if (set_ret == JFMQ_SUCCESS) {
		cnt++;
		continue;
	    }
	    if (set_ret == JFMQ_END_MESSAGE) 
		break;
	    return (Release_lock_return (fmq->fd, set_ret));
	}
    }
    if (cnt == 0) 
        return (Release_lock_return (fmq->fd, cnt));

    /* update the header */
    rpt = COMBINE_PAGE_POINTER (rpg, rpt);
    Sig_block (JFMQ_TRUE);
    if (lseek (fmq->fd, (off_t) fmq->r_off, SEEK_SET) < 0 ||
        write (fmq->fd, (char *) &rpt, sizeof (jfmq_t)) < sizeof (jfmq_t)) {
	Sig_block (JFMQ_FALSE);
	return (Release_lock_return (fmq->fd, JFMQ_WRITE_HD_FAILED));
    }
    Sig_block (JFMQ_FALSE);

    return (Release_lock_return (fmq->fd, cnt));

}

/****************************************************************
			
	Set_a_step_back()			Date: 3/7/94

	This function sets the read point one step back.

	Refer to the FMQ design decument for the FMQ structure and
	how the read pointer can be set back.

	This function returns JFMQ_SUCCESS on success or JFMQ_END_MESSAGE
	if the current message is the very oldest one and the pointer
	can not be moved back. It may return other error messages returned 
	from function Read_a_segment.

*/

static int
  Set_a_step_back
  (
    Jfmq_struct *fmq,		/* FMQ handle */
    jfmq_t *rpg,			/* read page number */
    jfmq_t *wpg,			/* write page number */
    jfmq_t *rpt,			/* read pointer */
    jfmq_t *wpt			/* write pointer */
) {
    int ret;
    jfmq_t tail_value;
    jfmq_t hd_value;
    jfmq_t hd_pt;
    jfmq_t tail_pt;
    jfmq_t stored_len;
    jfmq_t new_rpg;
    jfmq_t new_rpt;

    if(*rpg == *wpg){ /* on the same page */

	/* find the tail location of the previous message */
	tail_pt = *rpt - sizeof (jfmq_t);
	if (tail_pt < 0) {
	    tail_pt = fmq->jfmq_size - sizeof (jfmq_t);
	    if (tail_pt < *wpt) 
		return (JFMQ_END_MESSAGE);
	}

	/* read the message length */
	ret = Read_a_segment (fmq->fd, (off_t)(fmq->hd_size + tail_pt), sizeof (jfmq_t), 
			      (char *)&tail_value);
        if (ret != JFMQ_SUCCESS) return (ret);
	if (tail_value < 1 || tail_value > fmq->jfmq_size) 
	    return (JFMQ_END_MESSAGE);

	/* find the stored message length */
        stored_len = ROUND_SIZE(tail_value);
        stored_len += 2 * sizeof (jfmq_t);
	if(stored_len > fmq->jfmq_size) 
	    return (JFMQ_END_MESSAGE);

	/* find the location of the message header */
	hd_pt = *rpt - stored_len;
	if (hd_pt < 0) {
	    hd_pt += fmq->jfmq_size;
	    new_rpg = (*rpg + 1) % 2;
	    if (hd_pt < *wpt) 
		return (JFMQ_END_MESSAGE);
	}
	else new_rpg = *rpg;

	/* read the new value */
	ret = Read_a_segment (fmq->fd, (off_t)(fmq->hd_size + hd_pt), sizeof (jfmq_t), 
			      (char *)&hd_value);
        if (ret != JFMQ_SUCCESS) 
	    return (ret);
	if(hd_value != tail_value)  /* check that beginning and ending message 
				      length agree */
	    return (JFMQ_END_MESSAGE);

	new_rpt = hd_pt;
    }
    else { /* rpt and wpt are on different pages */

	/* find the tail location of the previous message */
	tail_pt = *rpt - sizeof (jfmq_t);
	if (tail_pt < *wpt) 
	    return (JFMQ_END_MESSAGE);

	/* read the message length */
	ret = Read_a_segment (fmq->fd, (off_t)(fmq->hd_size + tail_pt), sizeof (jfmq_t), 
			     (char *)&tail_value);
        if (ret != JFMQ_SUCCESS) 
	    return (ret);
	if(tail_value < 1 || tail_value > fmq->jfmq_size) 
	    return (JFMQ_END_MESSAGE);

	/* find the stored message length */
        stored_len = ROUND_SIZE(tail_value);
        stored_len += 2 * sizeof (jfmq_t);
	if(stored_len > fmq->jfmq_size) 
	    return (JFMQ_END_MESSAGE);

	/* find the length of the message header */
	hd_pt = *rpt - stored_len;
	if (hd_pt < *wpt) 
	    return (JFMQ_END_MESSAGE);

	/* read the new value */
	ret = Read_a_segment (fmq->fd, (off_t)(fmq->hd_size + hd_pt), sizeof (jfmq_t), 
			      (char *)&hd_value);
        if (ret != JFMQ_SUCCESS) 
	    return (ret);
	if(hd_value != tail_value)   /* check that beginning and ending message 
				      length agree */

	    return (JFMQ_END_MESSAGE);

	new_rpg = *rpg;
	new_rpt = hd_pt;
    }

    *rpg = new_rpg;
    *rpt = new_rpt;
    return (JFMQ_SUCCESS);
}

/****************************************************************
			
	Set_a_step_forward()			Date: 3/7/94
		
	This function sets the read point one step foreward.

	Setting the read pointer forward is similar to read a message
	without actually reading the message itself.

	This function returns JFMQ_SUCCESS on success or JFMQ_END_MESSAGE
	if the current message is the latest one and the pointer can not 
	be moved foreward. It may return other error messages indicating
	various FMQ reading failures.

*/

static int
  Set_a_step_forward
  (
    Jfmq_struct *fmq,		/* FMQ handle */
    jfmq_t *rpg,			/* read page number */
    jfmq_t *wpg,			/* write page number */
    jfmq_t *rpt,			/* read pointer */
    jfmq_t *wpt			/* write pointer */
) {
    int stored_len;		/* stored length of the message */
    jfmq_t length;		/* message length */
    int ret;

    /* check if message available */
    if (*rpg == *wpg && *rpt >= *wpt)	/* message not available */
	return (JFMQ_END_MESSAGE);

    /* read message length */
    ret = Read_a_segment (fmq->fd, (off_t)(fmq->hd_size + *rpt), sizeof (jfmq_t), 
			  (char *) &length);
    if (ret != JFMQ_SUCCESS) return (ret);
    if (length <= 0 || length > fmq->jfmq_size) 
	return (JFMQ_READ_LEN_FAILED);

    /* find message size in the fmq */
    stored_len = ROUND_SIZE(length);
    stored_len += 2 * sizeof (jfmq_t); /* augment length to include storage for 
					 length info */

    /* update page number */
    if (stored_len >= fmq->jfmq_size - *rpt)
	*rpg = (*rpg + 1) % 2;

    /* update read pointer */
    *rpt = (*rpt + stored_len) % fmq->jfmq_size;

    return (JFMQ_SUCCESS);
}

/****************************************************************
			
	JFMQ_clear()				Date: 3/7/94

	This function resets the read and write pointers to zero.
	This will discard any message, read or unread, in the FMQ. 

	We don't check the FMQ id because it was checked when the FMQ
	was opened.

	This function returns JFMQ_SUCCESS on success or one of the 
	following negative numbers in an error condition.

	JFMQ_INVALID_FD: The "fd" argument is not a valid FMQ descriptor.
	JFMQ_LOCK_FAILED: Failed in locking the FMQ.
	JFMQ_WRITE_HD_FAILED: Failed in writing the FMQ header.

*/

int
  JFMQ_clear
  (
      int fd			/* FMQ descriptor */
) {
    Jfmq_struct *fmq;		/* FMQ structure */
    jfmq_t pointers[2];		/* read and write pointers */

    /* get the fmq structure */
    if ((fmq = Get_jfmq_structure (fd)) == NULL)
	return (JFMQ_INVALID_FD);

    /* lock the fmq */
    if (Set_lock (JFMQ_SET_LOCK, fmq->fd) == JFMQ_FAILURE)
	return (JFMQ_LOCK_FAILED);

    /* reset the pointers */
    pointers[0] = pointers[1] = 0;
    Sig_block (JFMQ_TRUE);
    if (lseek (fmq->fd, (off_t) fmq->r_off, SEEK_SET) < 0 ||
	write (fmq->fd, (char *) pointers, 2 * sizeof (jfmq_t)) < 
	    2 * sizeof (jfmq_t)) {
	Sig_block (JFMQ_FALSE);
	return (Release_lock_return (fmq->fd, JFMQ_WRITE_HD_FAILED));
    }
    Sig_block (JFMQ_FALSE);
    if (Set_lock (JFMQ_RELEASE_LOCK, fmq->fd) == JFMQ_FAILURE)
	return (JFMQ_LOCK_FAILED);

    return (JFMQ_SUCCESS);

}

/****************************************************************
			
	JFMQ_close()				Date: 3/7/94

	This function closes an open FMQ, "fd", frees the descriptor
	and the FMQ structure.

	This function returns JFMQ_SUCCESS on success or JFMQ_INVALID_FD
	if the "fd" argument is not a valid FMQ descriptor.
*/

int
  JFMQ_close
  (
      int fd
) {
    Jfmq_struct *fmq;

    if ((fmq = Get_jfmq_structure (fd)) == NULL)
	return (JFMQ_INVALID_FD);

    close (fmq->fd);
    free (fmq);
    Jfmq_handle[fd - JFMQ_START_FD] = NULL;

    return (JFMQ_SUCCESS);
}

/****************************************************************
			
	JFMQ_lock()				Date: 3/7/94

	This function turns on (sw = JFMQ_ON) or off (sw = JFMQ_OFF)
	the file lock.

	This function has no return value.
*/

void
  JFMQ_lock
  (
      int sw			/* JFMQ_ON or JFMQ_OFF */
) {

    Jfmq_lock = sw;
    return;
}

/****************************************************************
			
	Read_pointers()				Date: 3/7/94
	
	This function reads the read and write pointers and checks
	if they are within normal ranges. 

	It returns JFMQ_SUCCESS on success or one of the following
	negative error numbers:

	JFMQ_READ_HD_FAILED: The FMQ has been corrupted - could not read 
			header or the FMQ id is incorrect.
	JFMQ_BAD_HD_INFOR: The FMQ has been corrupted - incorrect header 
		      information was found. 
*/

static int
  Read_pointers
  (
    Jfmq_struct *fmq,
    jfmq_t *rpg,
    jfmq_t *wpg,
    jfmq_t *rpt,
    jfmq_t *wpt
) {
    int same_page;		/* flag indicating that rpt and wpt are on
				   the same page */
    jfmq_t tmp[2];		/* temp space for reading pointers */

    /* get the pointers */
    if (lseek (fmq->fd, (off_t) fmq->r_off, SEEK_SET) < 0 ||
	read (fmq->fd, (char *) tmp, 2 * sizeof (jfmq_t)) < 2 * sizeof (jfmq_t)) {
	return (JFMQ_READ_HD_FAILED);
    }

    *rpg = GET_PAGE (tmp[0]);
    *wpg = GET_PAGE (tmp[1]);
    *rpt = GET_POINTER (tmp[0]);
    *wpt = GET_POINTER (tmp[1]);
    if (*rpg == *wpg)
	same_page = JFMQ_TRUE;
    else
	same_page = JFMQ_FALSE;

    /* check pointers */
    if (*rpt > fmq->jfmq_size || *wpt > fmq->jfmq_size ||
	(same_page == JFMQ_TRUE && *rpt > *wpt) ||
	(same_page == JFMQ_FALSE && *rpt < *wpt))
	return (JFMQ_BAD_HD_INFOR);

    return (JFMQ_SUCCESS);
}

/****************************************************************
			
	Release_lock_return()			Date: 3/7/94

	This function unlocks the file "fd" and returns a value of
	"return_value" if the unlocking is a success. It returns
	JFMQ_LOCK_FAILED if it fails to unlock the file.
*/

static int
  Release_lock_return
  (
      int fd,			/* file descriptor */
      int return_value		/* a passing return value */
) {

    if (Set_lock (JFMQ_RELEASE_LOCK, fd) == JFMQ_FAILURE)
	return (JFMQ_LOCK_FAILED);
    return (return_value);
}

/****************************************************************
			
	Jfmq_create()				Date: 3/7/94

	This function creates a FMQ file and initializes it. The name
	of the file is "name". The size of the FMQ, the space for
	messages, is "size" bytes and the access mode is "mode".

	If the file exists, the function returns JFMQ_EXIST, which is 
	a positive number. JFMQ_EXIST must be positive to be different
	from an error return.

	Note that, because we use "O_CREAT | O_EXCL" for the "open" flag,
	this function will not open an existing FMQ. This eliminates
	creation competition between processes.

	The file is filled with zero values. This physically allocates
	the disk space for the FMQ. The FMQ header is initialized. 

	This function locks the file before writing to the file.

	This function closes the file before exiting. The file lock is
	automatically released upon closing.

	It returns JFMQ_SUCCESS on success, JFMQ_EXIST if the file exists,
	or one of the following negative numbers specifying different
	error conditions:

	JFMQ_PERMISSION_DENIED: Permission of creating the file was denied.
	JFMQ_LOCK_FAILED: Failed in locking the FMQ.
	JFMQ_WRITE_HD_FAILED: Failed in writing the FMQ header.
	JFMQ_GENERATION_FAILED: Failed in initializing the FMQ space for some 
			   reason such as the file system was full.

*/

static int
  Jfmq_create
  (
      char *name,		/* fmq name */
      int size,			/* fmq size */
      int mode			/* access mode */
) {
    jfmq_t header[JFMQ_HEADER_SIZE];	/* fmq header */
    int fd;			/* file fd */
    int ret;

    errno = 0;
    fd = open (name, O_RDWR | O_CREAT | O_EXCL, (mode & 0777));
    if (fd < 0) {

	if (errno == EEXIST)
	    return (JFMQ_EXIST);

	return (JFMQ_PERMISSION_DENIED);
    }

    /* lock the file */
    if (Set_lock (JFMQ_SET_LOCK, fd) == JFMQ_FAILURE) {
	close (fd);
	return (JFMQ_LOCK_FAILED);
    }

    /* initialize the queue area with zeros */
    ret = Initialize_queue_area (fd, size);
    if (ret != JFMQ_SUCCESS) {
	close (fd);
	return (ret);
    }

    /* initialize the header */
    header[0] = JFMQ_ID_NUMBER;
    header[1] = size;
    header[4] = mode;
    header[2] = header[3] = header[5] = 0;

    Sig_block (JFMQ_TRUE);
    if (lseek (fd, 0, SEEK_SET) < 0 ||
	write (fd, (char *) header, JFMQ_HEADER_SIZE * sizeof (jfmq_t)) <
	JFMQ_HEADER_SIZE * sizeof (jfmq_t)) {
	Sig_block (JFMQ_FALSE);
	close (fd);
	return (JFMQ_WRITE_HD_FAILED);
    }
    Sig_block (JFMQ_FALSE);

    close (fd);
    return (JFMQ_SUCCESS);
}

/****************************************************************
			
	Initialize_queue_area ()		Date: 3/7/94

	This function fills in the FMQ queue area with zeros. This
	physically allocates the disk space for the FMQ. If it fails,
	it closes the file.

	It allocates a working space for the zero filling. The space
	is freed before it returns.

	This function returns JFMQ_SUCCESS on success or one of the 
	following negative numbers specifying error conditions:

	JFMQ_LOCK_FAILED: Failed in locking the FMQ.
	JFMQ_MEM_ALLOC_FAILED: Failed in allocating a work space.
	JFMQ_GENERATION_FAILED: Failed in initializing the FMQ space for some 
			   reason such as the file system was full.
*/

static int 
  Initialize_queue_area
  (
    int fd,			/* FMQ file fd */
    int size			/* FMQ size */
) {
    int file_size;		/* the file size */
    char *buf;			/* temp buffer */
    int i, cnt;

    /* compute file size */
    file_size = size + JFMQ_HEADER_SIZE * sizeof (jfmq_t);

    /* allocate work space */
    buf = (char *) malloc (FILL_IN_BUFFER_SIZE);
    if (buf == NULL) 
	return (JFMQ_MEM_ALLOC_FAILED);

    /* write zeros to the file */
    for (i = 0; i < FILL_IN_BUFFER_SIZE; i++)
	buf[i] = 0;
    cnt = 0;
    while (1) {
	int len;

	if (cnt >= file_size)
	    break;
	len = file_size - cnt;
	if (len > FILL_IN_BUFFER_SIZE)
	    len = FILL_IN_BUFFER_SIZE;
	len = write (fd, buf, len);
	if (len < 0) {
	    free (buf);
	    return (JFMQ_GENERATION_FAILED);
	}
	cnt += len;
    }

    free (buf);
    return (JFMQ_SUCCESS);
}

/****************************************************************
			
	Jfmq_open()				Date: 3/7/94

	This function opens an existing FMQ, allocates and initializes
	an FMQ structure for it.

	It first looks for an available FMQ descriptor. If all descriptors
	are in use, it returns JFMQ_TOO_MANY_OPEN_FMQ.

	It then opens the file and reads the FMQ header. The FMQ id number 
	is examined and the FMQ size is compared with the calling argument.

	It finally allocates and initializes the FMQ structure for the
	new open FMQ.

	This function returns the FMQ descriptor on success or one of
	the following error numbers:

	JFMQ_TOO_MANY_OPEN_FMQ: Too many (more than MAX_N_FMQS) FMQs have been 
			   opened.
	JFMQ_OPEN_FAILED: Failed in opening an existing FMQ "name" 
			 (permission was denied or it did not exist). 
	JFMQ_LOCK_FAILED: Failed in locking the FMQ.
	JFMQ_READ_HD_FAILED: The FMQ exists, but its header is incorrect.
	JFMQ_SIZE_INCORRECT: The FMQ exists, but its size is different from the
			calling "size" argument.
	JFMQ_MEM_ALLOC_FAILED: Failed in allocating a work space.

*/

static int
  Jfmq_open
  (
      char *name,		/* fmq name */
      int size,			/* fmq size */
      int mode			/* opening mode */
) {
    Jfmq_struct *fmq;		/* fmq structure */
    jfmq_t header[JFMQ_HEADER_SIZE];	/* fmq header */
    int index;			/* available FMQ descriptor index */
    int fd;			/* the file fd */

    /* find an available FMQ descriptor */
    for (index = 0; index < MAX_N_FMQS; index++)
	if (Jfmq_handle[index] == NULL)
	    break;
    if (index >= MAX_N_FMQS)
	return (JFMQ_TOO_MANY_OPEN_FMQ);

    /* open the file */
    if ((fd = open (name, O_RDWR, 0)) < 0)
	return (JFMQ_OPEN_FAILED);

    /* lock before reading it */
    if (Set_lock (JFMQ_SET_LOCK, fd) == JFMQ_FAILURE) {
	close (fd);
	return (JFMQ_LOCK_FAILED);
    }

    /* read the header */
    if (read (fd, (char *) header, JFMQ_HEADER_SIZE * sizeof (jfmq_t)) <
	JFMQ_HEADER_SIZE * sizeof (jfmq_t)) {
	close (fd);
	return (JFMQ_READ_HD_FAILED);
    }

    /* release the file lock */
    if (Set_lock (JFMQ_RELEASE_LOCK, fd) == JFMQ_FAILURE) {
	close (fd);
	return (JFMQ_LOCK_FAILED);
    }

    /* check id number */
    if (header[0] != JFMQ_ID_NUMBER) {
	close (fd);
	return (JFMQ_READ_HD_FAILED);
    }

    /* check size */
    if (((mode & JFMQ_SIZE_CHECK_OFF) == 0) && header[1] != size) {
	close (fd);
	return (JFMQ_SIZE_INCORRECT);
    }

    /* allocate space for fmq structure */
    fmq = (Jfmq_struct *) malloc (sizeof (Jfmq_struct));
    if ((char *) fmq == NULL) {
	close (fd);
	return (JFMQ_MEM_ALLOC_FAILED);
    }

    /* initialize the fmq structure */
    fmq->jfmq_id = header[0];
    fmq->jfmq_size = header[1];
    fmq->hd_size = JFMQ_HEADER_SIZE * sizeof (jfmq_t);
    fmq->r_off = RP_OFFSET * sizeof (jfmq_t);
    fmq->w_off = WP_OFFSET * sizeof (jfmq_t);
    fmq->fd = fd;
    fmq->mode = header[4];
    Jfmq_handle[index] = fmq;

    return (index + JFMQ_START_FD);
}

/****************************************************************
			
	Get_jfmq_structure()				Date: 3/7/94

	This function checks a FMQ descriptor "fd" and, if it is OK, 
	returns the pointer to the FMQ structure associated with it. 
	It returns NULL if "fd" is an invalid FMQ descriptor.
*/

static Jfmq_struct *
  Get_jfmq_structure
  (
      int fd
) {

    if (fd - JFMQ_START_FD < 0 || fd - JFMQ_START_FD >= MAX_N_FMQS ||
	Jfmq_handle[fd - JFMQ_START_FD] == NULL)
	return (NULL);

    return (Jfmq_handle[fd - JFMQ_START_FD]);
}

/****************************************************************
			
	Set_lock()				Date: 3/7/94

	This function locks (sw = JFMQ_SET_LOCK) or unlocks 
	(sw != JFMQ_SET_LOCK) a file "fd". The lock is an exclusive
	lock. This function will not return before an exclusive lock
	is awarded.

	This function returns JFMQ_SUCCESS on success or JFMQ_FAILURE 
	if the "fcntl" call returns with an error. 
*/

static int
  Set_lock
  (
      int sw,			/* function switch */
      int fd			/* file descriptor */
) {
    static struct flock fl;	/* structure used by fcntl */
    static int init = 0;	/* flag for initializing fl */
    int err;

    if (init == 0) {
	init = 1;
	fl.l_whence = SEEK_SET;
	fl.l_start = 0;
	fl.l_len = 0;
    }

    if (Jfmq_lock != JFMQ_ON) return (JFMQ_SUCCESS);

    if (sw == JFMQ_SET_LOCK)
	fl.l_type = F_WRLCK;
    else
	fl.l_type = F_UNLCK;

    while ((err = fcntl (fd, F_SETLKW, &fl)) == -1 && errno == EINTR);

    if (err == -1)
	return (JFMQ_FAILURE);
    else
	return (JFMQ_SUCCESS);
}

/****************************************************************
			
	Sig_block()				Date: 3/7/94

	This function blocks all signals if "sw" is JFMQ_TRUE. It sets
	the original signal mask if "sw" is not JFMQ_TRUE.

	The first call of this function must use "sw" = JFMQ_TRUE.
*/

static void Sig_block(int sw)
{

  static sigset_t oldmask, newmask;

  if(sw==JFMQ_TRUE) {

    /*
     * suspend signals
     */
    
    sigfillset(&newmask);

    if (sigprocmask(SIG_BLOCK, &newmask, &oldmask) < 0) {
      fprintf(stderr, "SIG_BLOCK error\n");
    }

  } else {

    /*
     * activate signals
     */

    if (sigprocmask(SIG_SETMASK, &oldmask, NULL) < 0) {
      fprintf(stderr, "SIG_SETMASK error\n");
    }

  }

  return;
}

