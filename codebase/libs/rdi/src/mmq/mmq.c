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
		
	File: mmq.c	
				
	7/7/94

	Purpose: This module contains the shared memory message queue, 
	MMQ, library routines.

	Files used: mmq.h
	See also: 
	Author: 

*****************************************************************/


/*** System include files ***/

#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <memory.h>

/*** Local include files ***/

#include <rdi/mmq.h>


/*** Definitions / macros / types ***/

#define MMQ_SET   	1
#define MMQ_CLEAR 	0
#define MMQ_TRUE  	1
#define MMQ_FALSE 	0
#define MMQ_DONE    	1
#define MMQ_NOT_DONE   	0

#define MMQ_OK    	1

#define MAX_N_MMQS     16	/* maximum number of open MMQ's */
#define MIN_INDEX     128	/* the minimum MMQ index */
#define HEADER_SIZE     4	/* The MMQ control area size. 4 longs */
#define TAIL_SIZE       1	/* an extra long at the end is needed for the
				   termination mark */

#define RRPT_NO_VALUE 0x7fffffff /* used for rrpt field in struct Mmq_struct:
				   there is no value in recorded read pointer */
#define BUFFER_CANCELED  -1	/* used for ret_length field in Mmq_struct:
				   The returned buffer is cancelled */

#define ALIGNED_LENGTH(leng) ((leng + 3) >> 2) << 2  /* evaluate the actual
				   space needed in MMQ due to alignment */

#define SHIFT_FOR_PAGE  24	/* shift for page numbers */
#define POINTER_MASK  0xffffff	/* mask for pointers */

#define GET_POINTER(a) (a & POINTER_MASK)	/* extract pointer */
#define GET_PAGE(a) (a >> SHIFT_FOR_PAGE)	/* extract page number */
#define COMBINE_PTPG(pt,pg) (pt + (pg << SHIFT_FOR_PAGE))	
				/* combine a pointer and its page */
#define INC_PAGE(pg) ((pg + 1) % 2)	/* increment a page number */


typedef long mmq_t;		/* integer type used for MMQ - we use 4
				   bytes assuming memory access is atomic */


struct mmq_struct {		/* the MMQ structure */
    mmq_t *r_pt, *w_pt;		/* read/write point */
    mmq_t *r_flag, *w_flag;	/* read/write flags */
    mmq_t b_size;		/* buffer size for data excluding header and tail
				   (20 bytes) */
    char *dbuf;			/* pointer to the message area */
    mmq_t shmid;		/* shared memory id */
    short init;			/* initialization status flag */
    short type;			/* read/write | flush */
    mmq_t rrpt;			/* store read pointer for MMQ_read */
    mmq_t wpt5, wpg5;		/* store values for MMQ_request */
    int ret_length;		/* buffer size requested by MMQ_request call */
};
typedef struct mmq_struct Mmq_struct;


/*** Local references / local variables ***/

/* array of mmq structure pointers for open MMQs */
static Mmq_struct *Mmq_handle[MAX_N_MMQS] =
{NULL, NULL, NULL, NULL, NULL,
 NULL, NULL, NULL, NULL, NULL, NULL,
 NULL, NULL, NULL, NULL, NULL};

static int Msg_on = MMQ_FALSE;	/* message print flag */
static int size_unit = 1024;	/* MMQ size unit, internal feature */

static int Use_old_format = MMQ_FALSE;	/* to use old queue data format */

/* static functions */
static Mmq_struct * Get_mmq_structure (int md);
static int Check_connection_write_side (Mmq_struct *mmq);
static int Process_write_pointer (Mmq_struct *mmq, int leng, mmq_t *wpt_ret, 
				  mmq_t *wpg_ret);
static int Find_next_read_pointer (Mmq_struct *mmq, mmq_t owpt, mmq_t orpt, 
	mmq_t *nrpt, mmq_t *nrpg, int *len_ret, int *msg_len_ret);
static int Check_connection_read_side (Mmq_struct *mmq);



/****************************************************************
			
	MMQ_open()				Date: 7/7/94

	This function opens an MMQ of "type" with "key". It first 
	calculates the size of the MMQ and checks arguments. It then 
	looks for an unused MMQ descriptor. 

	The function opens, or creates if needed, the shared memory 
	segment. The number of attached users of the shared memory 
	segment is examined to ensure that there are at most two users. 
	It then allocates an MMQ structure and initializes it. 

	Finally, the control flag in the MMQ is set and the MMQ descriptor 
	is returned.
	
	This function returns an FMQ descriptor on success or a negative 
	number for indicating an error condition. 
*/

int 
MMQ_open (
    int type, 		/* the MMQ type */
    int key		/* the MMQ key */
)
{
    int size; 		/* size of the MMQ */
    struct shmid_ds buf;	/* shared memory structure */
    Mmq_struct *mmq;	/* MMQ structure */
    int index;		/* MMQ index */
    char *cpt;

    /* calculate the shared memory segment size */
    if (key < 0)
	return (MMQ_KEY_NEGATIVE);
    if (Use_old_format == MMQ_TRUE)
	size = (key % 10000) * size_unit;
    else
	size = (key % 10000) * size_unit + (HEADER_SIZE + TAIL_SIZE) * sizeof (mmq_t);

    /* get an available MMQ index */
    for (index = 0; index < MAX_N_MMQS; index++)
	if (Mmq_handle[index] == NULL)
	    break;
    if (index >= MAX_N_MMQS) 
	return (MMQ_TOO_MANY_OPENED);

    /* the MMQ must have a useful size */
    if (size < 2 * sizeof (mmq_t))
	return (MMQ_SIZE_TOO_SMALL);

    /* allocate space for the handle */
    if ((mmq = (Mmq_struct *) malloc (sizeof (Mmq_struct))) == NULL) 
	return (MMQ_MALLOC_FAILED);
  
    /* create the shared memory */
    if ((mmq->shmid = shmget ((key_t) key, size, 0666 | IPC_CREAT)) < 0) {
	if (Msg_on == MMQ_TRUE)
	    printf ("Failed in opening shared memory (size = %d)\n", size);
	return (MMQ_SHMGET_FAILED);
    }

    /* attach the shared memory regions */
    cpt = (char *) shmat ((int)mmq->shmid, 0, 0);
    if ((int) cpt == -1)
	return (MMQ_SHMAT_FAILED);

    /* check how many attached */
    shmctl ((int)mmq->shmid, IPC_STAT, &buf);
    if (buf.shm_nattch > 2) {
	if (Msg_on == MMQ_TRUE)
	    printf ("Too many (%d) connections to shm\n",
		    buf.shm_nattch);
	return (MMQ_TOO_MANY_CLIENTS);
    }

    /* initialize the MMQ structure */
    mmq->r_pt = (mmq_t *) cpt;
    cpt += sizeof (mmq_t);
    mmq->w_pt = (mmq_t *) cpt;
    cpt += sizeof (mmq_t);

    mmq->type = type;
    mmq->rrpt = RRPT_NO_VALUE;
    mmq->ret_length = 0;
    mmq->init = MMQ_NOT_DONE;

    if (Use_old_format == MMQ_FALSE) {		/* new format */
	mmq->r_flag = (mmq_t *) cpt;
	cpt += sizeof (mmq_t);
	mmq->w_flag = (mmq_t *) cpt;
	cpt += sizeof (mmq_t);
	mmq->dbuf = (char *) cpt;
	mmq->b_size = size - (HEADER_SIZE + TAIL_SIZE) * sizeof (mmq_t);
    
	/* set up starting flag */
	if ((type & MMQ_WRITE) == MMQ_FALSE) 	/* reader */
	    *mmq->r_flag = MMQ_SET;
	else 					/* writer */
	    *mmq->w_flag = MMQ_SET;
    }
    else {					/* old format */
	long *ow_pt, *op_pt;

	mmq->dbuf = (char *) cpt;
	mmq->b_size = size - 3 * sizeof (mmq_t);/* an extra mmq_t is needed at the end */

	if ((type & MMQ_WRITE) == MMQ_FALSE) {	/* read */
	    ow_pt = mmq->r_pt;
	    op_pt = mmq->w_pt;
	}
	else {
	    ow_pt = mmq->w_pt;
	    op_pt = mmq->r_pt;
	}

	if (*op_pt == -34009265) {	/* the other party is not active */
	    *ow_pt = 0;
	    *op_pt = 0;
	}
	else
	    *ow_pt = -34009265;
    }

    Mmq_handle[index] = mmq;

    /* return the MMQ descriptor */
    return (index + MIN_INDEX);
}


/****************************************************************
			
	MMQ_read()				Date: 7/7/94

	This function reads a message from the MMQ "md" and returns the 
	message length on success. The message is returned by returning 
	the pointer to the message (in "msg") in the MMQ.

	Refer to mmq.doc for a description on how a message is stored 
	in the MMQ. We will not explain the steps in this function 
	since they are quite straightforward.

	This function calls Check_connection_read_side to process MMQ
	initialization and reconnection.

	The new read pointer value is stored in mmq->rrpt and the read 
	pointer is updated next time this function is called.

	This function returns the message length on success, or 
	MMQ_WOULD_BLOCK (0) if there is no message in the queue, or a
	negative number to indicate an error condition.
*/

int 
MMQ_read (
    int md, 		/* the MMQ descriptor */
    char **msg		/* the address to return the pointer to the message */
)
{
    int len;
    mmq_t rpt, wpt, rpg;
    Mmq_struct *mmq;
    int msg_len;
    int ret;

    /* get the MMQ structure */
    mmq = Get_mmq_structure (md);
    if (mmq == NULL) 
	return (MMQ_BAD_DESCRIPTOR);

    /* check that the mmq was opened for read */
    if ((mmq->type & MMQ_WRITE) != MMQ_FALSE)
	return (MMQ_BAD_ACCESS);

    /* check and process connection status */
    ret = Check_connection_read_side (mmq);
    if (ret != MMQ_OK) return (ret);

    /* update the read pointer - finishing the previous read */
    if (mmq->rrpt != RRPT_NO_VALUE)
	*mmq->r_pt = mmq->rrpt;
    mmq->rrpt = RRPT_NO_VALUE;

    /* get current pointers and pages */
    rpt = *mmq->r_pt;
    wpt = *mmq->w_pt;

    /* find pointer, page and length for the next message */
    ret = Find_next_read_pointer (mmq, wpt, rpt, &rpt, &rpg, &len, &msg_len);
    if (ret != MMQ_OK) 
	return (ret);

    /* output the message pointer */
    *msg = mmq->dbuf + (rpt + sizeof (mmq_t));

    /* increment the read pointer and store the page-pointer combination */
    rpt += len;
    mmq->rrpt = COMBINE_PTPG (rpt, rpg);

    /* return the actual message length */
    return (msg_len);

}

/****************************************************************
			
	MMQ_preview()				Date: 7/7/94

	This function is similar to MMQ_read except that it does not 
	update the read pointer and it returns lengths and pointers of 
	multiple messages. 

	Refer to mmq.doc for a description on how a message is stored 
	in the MMQ. We will not explain the steps in this function 
	since they are quite straightforward.

	This function calls Check_connection_read_side to process MMQ
	initialization and reconnection.

	This function returns the number of messages returned on success, 
	or MMQ_WOULD_BLOCK (0) if there is no message in the queue, or a
	negative number to indicate an error condition.
*/


int 
MMQ_preview (
    int md, 		/* the MMQ descriptor */
    int n_msg,		/* number of messages to return */
    char **msg,		/* array to return the pointers to the messages */
    int *length		/* array to return the lengths of the messages */
)
{
    mmq_t crpt, cwpt;	/* combined pointers */
    Mmq_struct *mmq;
    int ret, cnt, i;

    /* get the MMQ structure */
    mmq = Get_mmq_structure (md);
    if (mmq == NULL) 
	return (MMQ_BAD_DESCRIPTOR);

    /* check that the mmq is opened for read */
    if ((mmq->type & MMQ_WRITE) != MMQ_FALSE)
	return (MMQ_BAD_ACCESS);

    /* check and process connection status */
    ret = Check_connection_read_side (mmq);
    if (ret != MMQ_OK) return (ret);

    /* get combined pointers for the first message */
    crpt = *mmq->r_pt;
    cwpt = *mmq->w_pt;

    /* search for all n_msg messages */
    cnt = 0;
    for (i=0; i<n_msg; i++) {
	mmq_t rpt, rpg;
	int len, msg_len;

        /* find pointer, page and length for the next message */
        ret = Find_next_read_pointer (mmq, cwpt, crpt, &rpt, &rpg, &len, &msg_len);
        if (ret == MMQ_WOULD_BLOCK) 	/* no more messages */
	    break;
        if (ret != MMQ_OK) 		/* error encountered */
	    return (ret);

	/* output the message pointer and the message length */
        msg[cnt] = mmq->dbuf + (rpt + sizeof (mmq_t));
	length[cnt] = msg_len;
	cnt++;

	/* next combined read pointer */
	rpt += len;
        crpt = COMBINE_PTPG (rpt, rpg);
    }

    /* return the number of messages found */
    return (cnt);

}


/****************************************************************
			
	Find_next_read_pointer()		Date: 7/7/94

	This function looks for the oldest message in the queue. On
	success it returns the page number, offset, the message length
	and the space used in MMQ for the message.

	Refer to mmq.doc for a description on how a message is stored 
	in the MMQ.

	If a message is found, a further check is performed to make sure
	that the message is within the boundary of the shared memory
	segment.

	This function returns MMQ_OK (1) on success, or MMQ_WOULD_BLOCK (0) 
	if there is no message in the queue, or a negative number to 
	indicate an error condition.
*/

static int
Find_next_read_pointer (
    Mmq_struct *mmq,		/* mmq structure */
    mmq_t cwpt, 		/* current combined write pointer */
    mmq_t crpt,			/* current combined read pointer */
    mmq_t *nrpt, 		/* output read pointer */
    mmq_t *nrpg,		/* output read page */
    int *len_ret,		/* output length used in MMQ */
    int *msg_len_ret		/* output message length */
)
{
    mmq_t rpt, wpt, rpg, wpg;
    int msg_len, len;

    rpg = GET_PAGE (crpt);
    rpt = GET_POINTER (crpt);
    wpg = GET_PAGE (cwpt);
    wpt = GET_POINTER (cwpt);

    /* check if any message available */
    if (rpg == wpg && rpt >= wpt)	/* no message */
	return (MMQ_WOULD_BLOCK);	

    /* length of the next message */
    msg_len = (int)(*((mmq_t *) (mmq->dbuf + rpt)));

    if (msg_len == 0) { /* we must return to the beginning to look for a message */
	rpt = 0;
	rpg = INC_PAGE (rpg);
	if (rpg == wpg && rpt >= wpt)
	    return (MMQ_WOULD_BLOCK);	/* message not available */
	msg_len = (int)(*((mmq_t *) (mmq->dbuf + rpt)));
    }
    if (Use_old_format == MMQ_TRUE)
	msg_len -= sizeof (long);

    /* space needed for this message including alignment pad and length field */
    len = ALIGNED_LENGTH (msg_len + sizeof (mmq_t));

    /* do a further check of the internal consistency */
    if (len <= sizeof (mmq_t) || len + rpt > mmq->b_size || 
	(len + rpt > wpt && rpg == wpg)) {
	if (Msg_on == MMQ_TRUE) {
	    printf ("Fatal internal error: \n");
	    printf ("len = %d rpg = %d wpg = %d rpt = %d wpt = %d\n", len, (int)rpg, 
		     (int)wpg, (int)rpt, (int)wpt);
	}
	return (MMQ_INTERNAL_ERROR);
    }

    *nrpt = rpt;
    *nrpg = rpg;
    *len_ret = len;
    *msg_len_ret = msg_len;

    return (MMQ_OK);

}

/****************************************************************
			
	Check_connection_read_side()		Date: 7/7/94

	This function initializes the MMQ, if the writer's control flag
	is set, and then resets the flag.

	It checks its own control flag. If it is set, it locks itself
	from reading messages (waiting for the writer to initialize the 
	MMQ).

	Note: We need the mmq->init to guarantee that the initialization 
	is only performed once to save the messages in the MMQ. We cannot 
	use *mmq->r_flag == MMQ_SET for the check. The latter will not 
	protect from a second reader that tries to connect to the MMQ.

	This function returns MMQ_OK (1) on success, or MMQ_WOULD_BLOCK (0) 
	if the MMQ is not initialized yet.
*/

static int
Check_connection_read_side (
    Mmq_struct *mmq		/* mmq structure */
)
{	

    if (Use_old_format == MMQ_TRUE) {
	if (*mmq->w_pt == -34009265) {		/* writer is just started */
	    *mmq->r_pt = 0;
	    *mmq->w_pt = 0;
	    mmq->rrpt = RRPT_NO_VALUE;
	}
        if (*mmq->r_pt < 0)
	    return (MMQ_WOULD_BLOCK);		/* self locked */
	return (MMQ_OK);
    }

    /* writer is just started - initialize MMQ */
    if (*mmq->w_flag == MMQ_SET) {	
	if (mmq->init == MMQ_NOT_DONE || (mmq->type & MMQ_FLUSH) != MMQ_FALSE) {
	    *mmq->w_pt = MMQ_CLEAR;
	    *mmq->r_pt = MMQ_CLEAR;
	    mmq->rrpt = RRPT_NO_VALUE;
	}
	mmq->init = MMQ_DONE;
	*mmq->w_flag = MMQ_CLEAR;
    }

    /* self locked - the writer is not responding */
    if (*mmq->r_flag == MMQ_SET)
	return (MMQ_WOULD_BLOCK);

    mmq->init = MMQ_DONE;

    return (MMQ_OK);
}
	
/****************************************************************
			
	MMQ_write()				Date: 7/7/94

	This function writes a message "msg" of length "length" to the 
	MMQ "md" and returns the message length on success. 

	Refer to mmq.doc for a description on how a message is stored 
	in the MMQ. We will not explain the steps in this function 
	since they are quite straightforward.

	This function calls Check_connection_write_side to process MMQ
	initialization and reconnection.

	The function checks mmq->ret_length to make sure that there is no
	unfinished write. i.e. An MMQ_request was called and the MMQ_send
	has not been called yet since.

	This function returns the message length on success, or 
	MMQ_WOULD_BLOCK (0) if a space for the new message is not 
	available in the queue, or a negative number to indicate an error 
	condition.
*/


int 
MMQ_write (
    int md, 		/* MMQ descriptor */
    char *msg, 		/* the pointer to the new message */
    int length		/* length of the message */
)
{
    int ret, leng;
    mmq_t wpt, wpg;
    mmq_t *lpt;
    Mmq_struct *mmq;

    if (length <= 0)		/* check length */
	return (MMQ_ZERO_MESSAGE);

    mmq = Get_mmq_structure (md);	/* get mmq structure */
    if (mmq == NULL)
	return (MMQ_BAD_DESCRIPTOR);

    if ((mmq->type & MMQ_WRITE) == MMQ_FALSE)	/* check type */
	return (MMQ_BAD_ACCESS);

    /* check if the previous write is finished */
    if (mmq->ret_length > 0)
	return (MMQ_CANNOT_WRITE);

    /* check and process connection status */
    ret = Check_connection_write_side (mmq);
    if (ret != MMQ_OK) 
	return (ret);

    /* size of space needed for the message */
    leng = ALIGNED_LENGTH (length + sizeof (mmq_t));

    ret = Process_write_pointer (mmq, leng, &wpt, &wpg);
    if (ret != MMQ_OK) 
	return (ret);

    /* copy the message to the mmq */
    memcpy (mmq->dbuf + (wpt + sizeof (mmq_t)), msg, length);

    /* write length */
    lpt = (mmq_t *) (mmq->dbuf + wpt);
    if (Use_old_format == MMQ_TRUE) 
	*lpt = leng;
    else
	*lpt = length;

    /* update write pointer */
    wpt += leng;
    wpt = COMBINE_PTPG (wpt, wpg);
    *mmq->w_pt = wpt;

    return (length);

}


/****************************************************************
			
	Process_write_pointer()			Date: 7/7/94

	This function looks for a space for a new message, which requires 
	"leng" bytes free space in the MMQ. If the space is found, the
	page number and the offset of the space are returned. A 
	termination mark is placed in the MMQ is the write pointer
	folds back. The free space is guaranteed to be within the MMQ
	memory boundary.

	This function returns MMQ_OK (1) on success, or MMQ_WOULD_BLOCK (0) 
	if there is no free space in the queue, or a negative number to 
	indicate an error condition.
*/

static int 
Process_write_pointer (
    Mmq_struct *mmq, 		/* mmq structure */
    int leng, 			/* message buffer length */
    mmq_t *wpt_ret, 		/* output new write pointer */
    mmq_t *wpg_ret		/* output new write page */
)
{
    mmq_t rpt, rpg, wpto, wpt, wpg;

    if (leng * 2 > mmq->b_size)		/* check length */
	return (MMQ_MSG_TOO_LARGE);

    rpt = *mmq->r_pt;
    wpt = *mmq->w_pt;

    rpg = GET_PAGE (rpt);
    wpg = GET_PAGE (wpt);
    rpt = GET_POINTER (rpt);
    wpt = GET_POINTER (wpt);

    /* is the queue full? */
    if (rpg != wpg && wpt + leng > rpt)	/* full */
	return (MMQ_WOULD_BLOCK);		
    
    if (wpt + leng > mmq->b_size) {	/* we try to back to the beginning */
	wpto = wpt;
	wpt = 0;
	wpg = INC_PAGE (wpg);
	if (rpg != wpg && wpt + leng > rpt)	/* still full */
	    return (MMQ_WOULD_BLOCK);

	/* we found a free space and put a termination mark */		
	*((mmq_t *)(mmq->dbuf + wpto)) = 0;
    }

    *wpg_ret = wpg;
    *wpt_ret = wpt;

    return (MMQ_OK);
}
	
/****************************************************************
			
	MMQ_request()				Date: 7/7/94

	This function is similar to MMQ_write except that it returns the 
	pointer to the caller instead of copying the message and updating 
	the write pointer.

	The length, the page number and the offset of the returned free 
	space are recorded in MMQ structure for later use by MMQ_send call.

	This function returns the message length on success, or 
	MMQ_WOULD_BLOCK (0) if a space for the new message is not 
	available in the queue, or a negative number to indicate an error 
	condition.
*/


int 
MMQ_request (
    int md, 		/* MMQ descriptor */
    int length,		/* requested length for the message */
    char **buffer	/* address returning the pointer to the requested 
			   space */
)
{
    int ret, leng;
    mmq_t wpt, wpg;
    Mmq_struct *mmq;

    if (length <= 0)		/* check length */
	return (MMQ_ZERO_MESSAGE);

    mmq = Get_mmq_structure (md);	/* get mmq structure */
    if (mmq == NULL)
	return (MMQ_BAD_DESCRIPTOR);

    if ((mmq->type & MMQ_WRITE) == MMQ_FALSE)	/* check type */
	return (MMQ_BAD_ACCESS);

    /* check and process connection status */
    ret = Check_connection_write_side (mmq);
    if (ret != MMQ_OK) 
	return (ret);

    /* size of space needed for the message */
    leng = ALIGNED_LENGTH (length + sizeof (mmq_t));

    ret = Process_write_pointer (mmq, leng, &wpt, &wpg);
    if (ret != MMQ_OK) 
	return (ret);

    /* record the length and the write pointer and return the buffer */
    mmq->ret_length = length;
    mmq->wpt5 = wpt;
    mmq->wpg5 = wpg;
    *buffer = (char *)(mmq->dbuf + (wpt + sizeof (mmq_t)));

    return (length);

}
	
/****************************************************************
			
	MMQ_send()				Date: 7/7/94

	This function finishes a message writing by putting the actual
	message length in the MMQ and updating the write pointer.

	This function returns the message length on success, or a 
	negative number to indicate an error condition.
*/

int 
MMQ_send (
    int md, 		/* MMQ descriptor */
    int length		/* actual length of the message */
)
{
    int ret, leng;
    mmq_t  wpt, wpg;
    mmq_t *lpt;
    Mmq_struct *mmq;

    if (length <= 0)			/* check length */
	return (MMQ_ZERO_MESSAGE);

    mmq = Get_mmq_structure (md);	/* get mmq structure */
    if (mmq == NULL)
	return (MMQ_BAD_DESCRIPTOR);

    if ((mmq->type & MMQ_WRITE) == MMQ_FALSE)	/* check type */
	return (MMQ_BAD_ACCESS);

    /* check and process connection status */
    ret = Check_connection_write_side (mmq);
    if (ret != MMQ_OK) 
	return (ret);
	
    /* size of space needed for the message */
    leng = ALIGNED_LENGTH (length + sizeof (mmq_t));

    /* the space previously returned has been cancelled due to reader's reconnection */
    if (mmq->ret_length == BUFFER_CANCELED) {
	mmq->ret_length = 0;
	return (MMQ_WOULD_BLOCK);
    }

    if (mmq->ret_length == 0)  		/* MMQ_request not called */
	return (MMQ_NO_BUFFER_RETURNED);

    if (length > mmq->ret_length)	/* the actual msg size is larger than 
					   buffer size requested */
	return (MMQ_LENGTH_ERROR);

    mmq->ret_length = 0;
    wpt = mmq->wpt5;
    wpg = mmq->wpg5;

    /* write the length to the mmq */
    lpt = (mmq_t *) (mmq->dbuf + wpt);
    if (Use_old_format == MMQ_TRUE)
	*lpt = leng;
    else
	*lpt = length;

    /* update the write pointer */
    wpt += leng;
    wpt = COMBINE_PTPG (wpt, wpg);
    *mmq->w_pt = wpt;

    return (length);

}

/****************************************************************
			
	Check_connection_write_side()		Date: 7/7/94

	This function initializes the MMQ, if the reader's control flag
	is set, and then resets the flag.

	It checks its own control flag. If it is set, it locks itself
	from writing messages (waiting for the reader to initialize the 
	MMQ).

	Note: We need the mmq->init to guarantee that the initialization 
	is only performed once to save the messages in the MMQ. We cannot 
	use *mmq->w_flag == MMQ_SET for the check. The latter will not 
	protect from a second writer that tries to connect to the MMQ.

	This function returns MMQ_OK (1) on success, or MMQ_WOULD_BLOCK (0) 
	if the MMQ is not initialized yet.
*/

static int
Check_connection_write_side (
    Mmq_struct *mmq		/* mmq structure */
)
{	

    if (Use_old_format == MMQ_TRUE) {
	if (*mmq->r_pt == -34009265) {		/* reader just started */
	    *mmq->w_pt = 0;
	    *mmq->r_pt = 0;
	    mmq->ret_length = BUFFER_CANCELED;
	}
	if (*mmq->w_pt == -34009265)
	    return (MMQ_WOULD_BLOCK);		/* self_locked */
	return (MMQ_OK);
    }

    if (*mmq->r_flag == MMQ_SET) {		/* reader just started */
	if (mmq->init == MMQ_NOT_DONE || (mmq->type & MMQ_FLUSH) != MMQ_FALSE) {
	    *mmq->r_pt = MMQ_CLEAR;
	    *mmq->w_pt = MMQ_CLEAR;
	    mmq->ret_length = BUFFER_CANCELED;
	}
	mmq->init = MMQ_DONE;
	*mmq->r_flag = MMQ_CLEAR;
    }

    if (*mmq->w_flag == MMQ_SET)
	return (MMQ_WOULD_BLOCK);		/* self_locked */

    mmq->init = MMQ_DONE;

    return (MMQ_OK);
}
	
/****************************************************************
			
	MMQ_close()				Date: 7/7/94

	This function closes an open MMQ. It checks how many clients 
	attach to the share memory segment. If it is the only one
	(the last one) that attaches to the shared memory, it removes
	the shared memory segment. Otherwise it detaches from the shared
	memory. Allocated MMQ structure is freed and the MMQ descriptor 
	is released for reuse.
	
	This function returns MMQ_SUCCESS on success, or MMQ_BAD_DESCRIPTOR
	if "md" is not a valid MMQ descriptor.
*/

int 
MMQ_close (
    int md		/* mmq descriptor */
)
{
    struct shmid_ds buf;
    Mmq_struct *mmq;
    int back;

    back = HEADER_SIZE;
    if (Use_old_format == MMQ_TRUE)
	back = 2;

    mmq = Get_mmq_structure (md);	/* get mmq structure */
    if (mmq == NULL)
	return (MMQ_BAD_DESCRIPTOR);
    
    shmctl ((int)mmq->shmid, IPC_STAT, &buf);
    if (buf.shm_nattch <= 1) 		/* remove the shared memory */
	shmctl ((int)mmq->shmid, IPC_RMID, &buf);
    else				/* detach from the shared memory */
	shmdt (mmq->dbuf - (back * sizeof(mmq_t))); 

    free ((char *)mmq);				/* free the mmq structure */
    Mmq_handle[md - MIN_INDEX] = NULL;	/* free the descriptor */

    return (MMQ_SUCCESS);
}
	
/****************************************************************
			
	Get_mmq_structure()			Date: 7/7/94

	This function returns the MMQ structure of MMQ "md".
	
	This function returns the pointer to the structure on success, 
	or NULL if the md is not a valid MMQ descriptor.
*/

static Mmq_struct * 
Get_mmq_structure (
    int md		/* mmq descriptor */
)
{

    if (md - MIN_INDEX < 0 || md - MIN_INDEX >= MAX_N_MMQS || 
	Mmq_handle[md - MIN_INDEX] == NULL)
	return (NULL);

    return (Mmq_handle[md - MIN_INDEX]);
}

	
/****************************************************************
			
	MMQ_msg()				Date: 7/7/94

	This function sets the message print flag.
*/

void 
MMQ_msg (
    int sw		/* action code */
)
{

    Msg_on = sw;
    return;
}
	
/****************************************************************
			
	MMQ_change_size_unit()			Date: 7/7/94

	This function sets the unit for calculating the MMQ size.
	The default unit is 1024 bytes. By setting the unit to smaller
	numbers such as 1 byte, one can test small MMQs. This feature
	is kept internal and used mainly for testing.
*/

void 
MMQ_change_size_unit (
    int unit
)
{

    size_unit = unit;
    return;
}
	
/****************************************************************
			
	MMQ_old()				Date: 10/7/94

	This function directs the module to use old obpipe format
	data. This must be called before any other MMQ function
	calls.
*/

void 
MMQ_old ()
{

    Use_old_format = MMQ_TRUE;
    return;
}
