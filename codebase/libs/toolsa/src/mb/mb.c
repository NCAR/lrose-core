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

/************************************************************************

Module:	mb.c

Author:	Z. Jing

Date:	1/5/95

Description: 

	This module contains the message buffer, MB, library routines.

************************************************************************/

/* System include files / Local include files */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/uio.h>

#include <toolsa/os_config.h>
#include <toolsa/mb.h>
#include <dataport/bigend.h>
#include <dataport/port_types.h>


/* Constant definitions / Macro definitions / Type definitions */

#define MAX_MB_NAME_SIZE MB_NAME_LEN - MB_MACHINE_NAME_SIZE
				/* maximum possible size of an MB file name */

#define MAX_N_MBS     128	/* maximum number of open MBs */
#define MB_START_FD   160	/* starting number of MB fds */

/* values for the id field in Mb_attribute */
#define MB_ID   93477256	/* a special number to identify the MB file */
#define UNINITIALIZED 0		/* the MB file is not initialized */
#define INVALID		-1	/* update_cnt is invalid */

#define MB_SET_LOCK  1		/* argument of Set_lock () */
#define MB_RELEASE_LOCK  0	/* argument of Set_lock () */

#define MB_FOUND	1	/* return value of Find_message_index */

#define SHARED 		0	/* lock type */
#define EXCLUSIVE	1

#define TRUE 		1	
#define FALSE		0

#define READ_ALL	-1	/* used by mb_read () */

/* MB attribute flags */
#define MB_FLAGS	(MB_MUST_READ | MB_REPLACE | MB_SEQUENTIAL)

/* types used in MB control areas */
/* In this implementation, we assume that there is no padding bytes between
   the fields in a structure containing only mb_t. This allows us to use
   effcient array conversion funtions in MB module. We also assume that
   mb_t is si32 and sit_t is ui16. These types must be machine
   independent */
typedef si32 mb_t;
typedef ui32 mbmsg_t;
typedef ui16 sit_t;		

/* MB attribute structure */
typedef struct {
    mb_t update_cnt;		/* update count */
    mbmsg_t read_id;		/* id of the message resently read */
				/* used only for sequential mbs */
    mb_t update_flag;		/* update flag, reserved for future use */
    mb_t mb_id;			/* MB file magic number (MB_ID or UNINITIALIZED) */
    mb_t mb_size;		/* message area size in the MB */
    mb_t msg_size;		/* average message size */
    mb_t size;			/* total MB file size */
    mb_t tbl_size;		/* single table area size */
    mb_t maxn_msgs;		/* maximum number of messages allowed in the message buffer */
    mb_t n_slots;		/* number of record slots in the table */
    mb_t perm;			/* MB file access perm */
    mb_t flags;			/* MB attribute flags (in set of MB_FLAGS) */
    mb_t reserved;		/* reserved field */
} Mb_attribute;

/* structure for message table header */
typedef struct {
    mb_t free_loc;		/* free space offset in the msg area */
    mb_t free_size;		/* size of the free space */
    mb_t n_msgs;		/* total number of messages in the message buffer */
    mbmsg_t id_pg;		/* page number of the msg ids; not used */
    mbmsg_t max_id;		/* maximum message id ever used */
				/* the above two fields are used only in seq MB. */
    mb_t sit_base;		/* base value of sit (sorted index table) */
    mb_t seq_sit;		/* sit is sequential in MB (TRUE/FALSE) */
    mbmsg_t dbloc_bits;		/* each bit in the field specifies which 
				   message buffer should be used */
    mb_t update_cnt;		/* duplicate Mb_attribute.update_cnt for 
				   update_cnt update failure checking */
    mb_t reserved;		/* reserved for future use */
} Tbl_header;

/* structure for message records in the message table */
typedef struct {
    mbmsg_t id;			/* message id, not usable in a sequentail MB */
    mb_t loc;			/* offset of the msg in the msg area */
} Msg_record;

/* data structure for an open MB */
typedef struct {
    int fd;			/* MB file fd */
    int mb_size;		/* MB size; copy of attr.mb_size */
    int msg_off;		/* offset of the message area */
    int ta_off;			/* offset of table A */
    int tb_off;			/* offset of table B */
    int maxn_msgs;		/* copy of attr.maxn_msgs */
    int n_slots;		/* copy of attr.n_slots */
    int flags;			/* local mb flags including MB attr. flags */

    Mb_attribute *attr;		/* attribute structure */
    Msg_record *list;		/* the message table */
    Tbl_header *tbl_hd;		/* pointer to the message header */
    sit_t *sit;			/* the sorted index table */
    sit_t *sit_buf;		/* the sorted index table buffer */
    int sit_size;		/* currently usable sit buffer size */
    int seq_sit;		/* the local sit is sequential (TRUE/FALSE) */
    int st_ind;			/* index of the first record in the list */
    int n_msgs;			/* copy of Tbl_header.n_msgs */

    int need_update;		/* the cache needs update (TRUE/FALSE) */
    time_t time;		/* last cache update time */

    int parity;			/* parity check number */

    mbmsg_t r_pt;		/* the read pointer */
    mbmsg_t r_pg;		/* the read pointer's page number; not used */
} Mb_struct;


/* offsets for certain fields in the file */

#define UPDATE_CNT_OFF   	0
#define READ_ID_OFF 		(sizeof (mb_t))
#define UPDATE_FLAG_OFF     	(sizeof (mb_t) + sizeof (mbmsg_t))
#define MB_ID_OFF      		(2 * sizeof (mb_t) + sizeof (mbmsg_t))
	
#define MAX_ID	((mbmsg_t)0xfe000000)		/* maximum id number */
#define ID_PAGE_SIZE ((mbmsg_t)0xfd000000)	/* id number page size */

/* External global variables / Non-static global variables / Static globals */

/* The open MBs */
static Mb_struct *Mb_handle[MAX_N_MBS] =
{NULL, NULL, NULL, NULL, NULL,
 NULL, NULL, NULL, NULL, NULL, NULL,
 NULL, NULL, NULL, NULL, NULL};


/* External functions / Internal global functions / Internal static functions */

static int Set_lock (int sw, int fd, int type);

static int Release_lock (int fd);

static int Initialize_mb_file (Mb_attribute *attr, int fd);

static int Unlock_return (int fd, int ret_value);

static int Check_msg_table (Mb_struct *mb);

static int Get_mb_descriptor (Mb_attribute *attr, Mb_struct **mb);

static int Get_mb_structure (int mbd, int lock_type, Mb_struct **new_mb);

static int Get_free_space (Mb_struct *mb, int length);

static int Find_index_in_table (Mb_struct *mb, mbmsg_t id);

static int Write_new_message (Mb_struct *mb, char *message, int length, 
		mbmsg_t *id, int index);

static int Find_message_index (Mb_struct *mb, mbmsg_t id, int *index);

static int Write_a_message (Mb_struct *mb, int loc, int length, char *message,
	int add_off);

static int Remove_a_message (Mb_struct *mb);

static int Create_new_mb (int flags, int fd, int msg_size, int maxn_msgs,
	int perm);

static int Update_cache (Mb_struct *mb);

static int Write_cache (Mb_struct * mb, int n_new);

static int Recind_from_sitind (Mb_struct *mb, int ind);

static int Get_msg_size (Mb_struct *mb, int ind);

static int Fix_r_pt (Mb_struct *mb);

static void Set_sequential_sit (Mb_struct *mb);

static int mb_read (int mbd, char *buffer, int st_offset, int length, 
	unsigned int msg_id, unsigned int *ret_id);

static int Find_parity (Mb_struct *mb);


/************************************************************************

Function Name: 	MB_open

Description: This function opens an MB of name "mb_name". Refer to 
	mb.doc for a detailed description of the function.

Notes:  If two processes create the same MB simultaneously, 
	only one of them should create the MB and the other should 
	see it as an existing MB. To implement this we cannot 
	simply remove an incorrect MB file when an error is detected. 
	Instead we must truncate and re-initialize it.

************************************************************************/

int 
_MB_open (
    char *mb_name,		/* name of the MB */
    int flags,			/* MB flags */
    int perm,			/* access perm of the MB file */
    int msg_size, 		/* average message size */
    int maxn_msgs		/* maximum number of messages */
)
{
    int fd, mbfd;
    Mb_attribute attr;
    Mb_struct *mb;
    int ret;
    int flg;

    /* check the name and flags */
    if (mb_name == NULL || (int)strlen (mb_name) > MAX_MB_NAME_SIZE || 
	mb_name[0] == '\0' || 
	((flags & MB_SEQUENTIAL) != 0 && (flags & MB_REPLACE) != 0) ||
	((flags & MB_SEQUENTIAL) == 0 && (flags & MB_MUST_READ) != 0) )
	return (MB_BAD_ARGUMENT);

    /* open the MB file */
    flg = O_RDWR; 
    if (flags & MB_INIT) {
        if ((perm & 0600) != 0600)
	    return (MB_BAD_ARGUMENT);
	flags |= MB_RDWR;
	flg |= O_CREAT;
    }
    fd = open (mb_name, flg, perm);
    if (fd < 0) 
	return (MB_OPEN_FAILED);

    /* lock the file */
    if (Set_lock (MB_SET_LOCK, fd, EXCLUSIVE) == MB_FAILURE) {
	close (fd);
	return (MB_LOCK_FAILED);
    }

    /* read the attributes and check attributes */
    if (lseek (fd, (off_t)0, 0) < 0 ||
        read (fd, (char *)&attr, sizeof (Mb_attribute))
	!= sizeof (Mb_attribute) ||
        BE_to_array_32 (&attr,
			sizeof (Mb_attribute)) < 0 ||
	attr.mb_id != MB_ID) {	/* not initialized or bad file */

        int mbd;

	if ((flags & MB_INIT) == 0) {	
	    close (fd);
	    return (MB_NONMB_FILE);
	}
	/* initialize the MB file */
	mbd = Create_new_mb (flags, fd, msg_size, maxn_msgs, perm);
	if (mbd < 0)
	    close (fd);
	return (mbd);
    }

    /* check attributes of the existing MB file */
    flg = (attr.flags | flags) & MB_FLAGS;
    if ((perm != 0 && attr.perm != perm) || 
	 (flg - attr.flags != 0) || 	/* new attribute flags introduced */
	 (msg_size != 0 && attr.msg_size != msg_size) ||
	 (maxn_msgs != 0 && attr.maxn_msgs != maxn_msgs)) {
	close (fd);
	return (MB_EXISTS);
    }

    /* initialize the mb structure and get the descriptor */
    mbfd = Get_mb_descriptor (&attr, &mb);
    if (mbfd < 0) {
	close (fd);
	return (mbfd);
    }
    mb->fd = fd;
    mb->flags = attr.flags | flags;

    /* read control info into the cache */
    ret = Update_cache (mb);
    if (ret < 0) {
	_MB_close (mbfd);
	return (ret);
    }
	
    /* check MB internal consistency */
    if ((ret = Check_msg_table (mb)) != MB_SUCCESS) {
	_MB_close (mbfd);
	return (ret);
    }

    /* no problem is found with the MB */
    Release_lock (fd);
	
    return (mbfd);
}

/************************************************************************

Function Name: 	Create_new_mb

Description: This function initializes the mb attribute structure and
	calls Initialize_mb_file to initialize the MB file. It then
	calls Get_mb_descriptor to allocate and initialize the MB
	data structure and returns the MB descriptor associated with
	it.
	
	The function returns the opened MB descriptor on success, or
	one of the following negative error numbers on failure:

	MB_BAD_ARGUMENT, MB_TOO_MANY, MB_ALLOCATE_FAILED.

************************************************************************/

static int Create_new_mb (
    int flags,			/* mb flags */
    int fd,			/* mb file descriptor */
    int msg_size,		/* mean message size */
    int maxn_msgs,		/* max number of message */
    int perm			/* mb file access perm */
) {
    int thd_z, n, rec_z;
    Mb_attribute attr;
    Mb_struct *mb;
    int ret, mbfd;

    /* check arguments */
    /* the maximum number of messages is 0x10000 because the unsigned short is used
       for SIT. In the replaceable MB case the number is further limited by 32
       since a bit is needed for each message and we have only 32 bits in the
       dbloc_bits field. The minimum number for replaceable MB is 1 and 2 for
       other MB types - in order to avoid immediate expiration of a message */

    if (maxn_msgs >= 0x10000 || maxn_msgs < 1 || 
	msg_size <=0 ||
	((flags & MB_REPLACE) != 0 && maxn_msgs > 32) ||
	((flags & MB_REPLACE) == 0 && maxn_msgs < 2)) {
	return (MB_BAD_ARGUMENT);
    }

    /* initialize attribute structure */
    thd_z = sizeof (Tbl_header);  /* Size of table header */
    rec_z = sizeof (Msg_record);  /* Size of each slot    */

    /* n = No. of slots needed to store the table header and SIT (if not Sequential) */
    if (flags & MB_SEQUENTIAL)
	n = (thd_z / rec_z) + 1;
    else
	n = ((maxn_msgs * sizeof (sit_t) + thd_z) / rec_z) + 1;

    attr.mb_id = UNINITIALIZED;

    /* Table size. 
       Note: Double slots for the header and sit are allocated to allow
             enough memory so that the header and sit are not across 
	     the end of the table boundary. E.g. if the current header 
	     and sit is at the end of the table space, now one record comes 
	     in, the new header and sit will be put at the beginning of the 
	     table space. Now we need double of the header and sit space. */
    attr.tbl_size = rec_z * (maxn_msgs + 2 * n);
    /* Message area size */
    attr.mb_size = maxn_msgs * msg_size;
    /*     Message area size + (table size * 2)  + Attribute size */
    attr.size = attr.mb_size + attr.tbl_size * 2 + sizeof (Mb_attribute);

    if ((flags & MB_REPLACE) != 0)  
	attr.size += attr.mb_size; /* Replacable MB needs double message area */

    attr.maxn_msgs = maxn_msgs;
    attr.msg_size = msg_size;
    attr.n_slots = maxn_msgs + n; /* No of slots in a table (header+SIT+record_list) */
    attr.update_cnt = 0; 
    attr.update_flag = 0;
    attr.perm = perm;
    attr.flags = flags & MB_FLAGS;
    attr.reserved = 0;
    attr.read_id = 0;

    /* initialize mb file, write the attribute to the MB file */
    ret = Initialize_mb_file (&attr, fd);
    if (ret != MB_SUCCESS) 
	return (ret);

    Release_lock (fd);

    /* get a new MB structure and its descriptor */
    mbfd = Get_mb_descriptor (&attr, &mb);
    if (mbfd < 0) 
	return (mbfd);

    mb->fd = fd;
    mb->r_pg = 0;
    mb->flags = flags;
	
    return (mbfd);
}

/************************************************************************

Function Name: 	Get_mb_descriptor

Description: This function allocates an MB structure for a new open
	MB and finds an available descriptor for it. The MB structure
	is returned in argument "new_mb".

	It returns the MB descriptor on success. It returns one
	of the following negative numbers on failure:

	MB_TOO_MANY, MB_ALLOCATE_FAILED.

************************************************************************/

static int
Get_mb_descriptor (
    Mb_attribute *attr,		/* MB attributes */
    Mb_struct **new_mb		/* return MB structure */
)
{
    int index;
    Mb_struct *mb;

    /* find an available MB descriptor */
    for (index = 0; index < MAX_N_MBS; index++)
	if (Mb_handle[index] == NULL)
	    break;
    if (index >= MAX_N_MBS)
	return (MB_TOO_MANY);

    /* allocate space for MB structure */
    mb = (Mb_struct *) malloc (sizeof (Mb_struct));
    if ((char *) mb == NULL) 
	return (MB_ALLOCATE_FAILED);

    /* Note:
     * Memories are allocated to the following pointers but they are not yet
     * assigned any values to them in this routine
     */
    mb->attr = (Mb_attribute *) malloc (sizeof (Mb_attribute));
    mb->tbl_hd = (Tbl_header *) malloc (sizeof (Tbl_header));
    mb->sit_size = attr->maxn_msgs + 64;
    mb->sit_buf = (sit_t *) malloc (mb->sit_size * (int) sizeof (sit_t));
    mb->sit = mb->sit_buf;
    mb->list = (Msg_record *) malloc (attr->tbl_size);
    if (mb->attr == NULL ||
	mb->tbl_hd == NULL ||
	mb->sit_buf == NULL ||
	mb->list == NULL)
	return (MB_ALLOCATE_FAILED);

    /* initialize the MB structure */
    mb->mb_size = attr->mb_size;
    mb->ta_off = sizeof (Mb_attribute);        /* Table A offset in the MB file */
    mb->tb_off = mb->ta_off + attr->tbl_size;  /* Table B offset in the MB file */
    mb->msg_off = mb->tb_off + attr->tbl_size; /* Message area in the MB file   */
    mb->maxn_msgs = attr->maxn_msgs;           /* Maximum messages of the MB    */
    mb->n_slots = attr->n_slots;               /* No of slots in a table        */

    mb->st_ind = attr->n_slots - 1;            /* Index of the slot for the first message */
    mb->n_msgs = 0;                            /* Number of messages currently in the MB  */
    mb->r_pt = 0;                              /* Message read pointer */
    mb->r_pg = 0;                              /* Probably not used ?  */
    mb->need_update = TRUE;                    /* The cache needs to be updated from MB file */
    mb->time = 0;

    Set_sequential_sit (mb);		       /* Initialize the SIT */
    mb->tbl_hd->seq_sit = TRUE;
    mb->attr->update_cnt = INVALID;	       /* Indicate just initialized, assign an impossible 
						  value to enforce the cache to be updated */
    Mb_handle[index] = mb;
    *new_mb = mb;

    return (index + MB_START_FD);
}

/************************************************************************

Function Name: Set_sequential_sit	

Description: This function sets up the SIT to be sequential.

************************************************************************/

static void 
Set_sequential_sit (
    Mb_struct *mb		/* MB structure */
)
{
int i; 
sit_t *sit;

    sit = mb->sit_buf;
    for (i = 0; i < mb->maxn_msgs; i++)
	sit [i] = i;
    mb->sit = mb->sit_buf;
    mb->sit_size = mb->maxn_msgs + 64;

    mb->tbl_hd->sit_base = 0;
    mb->seq_sit = TRUE;
    return;
}

/************************************************************************

Function Name: 	Initialize_mb_file

Description: This function initializes a new MB file. First, the MB
	attributes are written to the file. Then the message table 
	headers are initialized. Finally the MB id field is set 
	to make the MB file ready for use.

	This function returns MB_SUCCESS on success or 
	MB_UPDATE_FAILED if it fails to update the file.

************************************************************************/

static int 
Initialize_mb_file (
    Mb_attribute *attr,		/* the attribute structure */
    int fd			/* file descriptor of the MB file */
)
{
    int tblb_off;
    Tbl_header t_hd;
    mb_t id;

    /* possible junk file left */
    if (ftruncate(fd, 0) < 0 ||
        lseek (fd, (off_t)0, 0) < 0) 
	return (MB_UPDATE_FAILED);

    /* Write the attribute */
    if (BE_from_array_32 (attr,
			  sizeof (Mb_attribute)) < 0 ||
	write (fd, (char *)attr,
	       sizeof (Mb_attribute)) != sizeof (Mb_attribute) ||
	BE_to_array_32 (attr,
			sizeof (Mb_attribute)) < 0) 
	return (MB_UPDATE_FAILED);

    /* Initialize both table headers */
    t_hd.free_loc = 0;
    t_hd.free_size = attr->mb_size;
    t_hd.n_msgs = 0;
    t_hd.max_id = 0;
/*  t_hd.max_id = MAX_ID - 20; test id overflow */

    t_hd.id_pg = 0;
    t_hd.sit_base = 0;
    t_hd.seq_sit = TRUE;	/* initialized as sequential */
    t_hd.dbloc_bits = 0;
    t_hd.update_cnt = attr->update_cnt;
    t_hd.reserved = 0;

    tblb_off = attr->tbl_size + sizeof (Mb_attribute);
    /* 
     * Write table A, seek to table B and then write table B. 
     * Note: the current file pointer is at the beginning of table A so that
     *       no lseek is required before writing the table A.
     */
    if (BE_from_array_32 (&t_hd, sizeof (Tbl_header)) < 0 ||
	write (fd, (char *)&t_hd,
	       sizeof (Tbl_header)) != sizeof (Tbl_header) ||
	lseek (fd, (off_t)tblb_off, 0) < 0 ||
        write (fd, (char *)&t_hd, sizeof (Tbl_header)) != sizeof (Tbl_header))
	return (MB_UPDATE_FAILED);

    /* Update the MB file magic flag */
    id = MB_ID;
    BE_from_array_32 (&id, sizeof(mb_t));
    if (lseek (fd, (off_t)MB_ID_OFF, 0) < 0 ||
	write (fd, (char *)&id, sizeof (mb_t)) != sizeof (mb_t))
	return (MB_UPDATE_FAILED);

    /* Note: No update is necessary in the message area */

    return (MB_SUCCESS); 
}

/************************************************************************

Function Name: 	Update_cache

Description: This function updates the control info cache if necessary.
	It first reads the MB structure to get the update_cnt value in
	the file. If the cached info has not been modified, and the 
	update_cnt field in the MB file has not changed and the last 
	cache update is done within one hour, the cache update is not
	necessary. Otherwise the cache is updated by reading the MB file.

	The least significant bit in update_cnt specifies which table
	is currently in use. Its upper 29 bits is used for the counter of 
	messages ever written into the MB. When update_cnt exceeds its
	maximum allowable value, it is reduced and the total number of 
	messages is lost, although the location information of the tbl_hd
	in the table area is preserved. This will require a full update 
	of their cache for other processes.

	Only the part in the MB file that has been changed since the last
	cache update is read. The read may need up to 3 read calls,
	although most of the time the first and the third calls are
	not necessary and will be skipped. 

	The MB structure is refreshed. Before returning, this function
	calls Fix_r_pt to fix possible problems with the
	read pointer (Refer to mb.doc for further description).

	This function returns MB_SUCCESS on success or MB_CORRUPTED on 
	failure.  

************************************************************************/

static int Update_cache (
    Mb_struct *mb			/* the MB structure */
)
{
    int old_cnt, n_slots, upd_cnt;
    int st_rec, end_rec, st_ind;
    struct iovec io [3];
    int n, to_read, size, off;
    int offset, nitems, n_msgs, n_sit_read;
    int tm;
    int must_read;
    Mb_attribute *attr;
    Tbl_header *tbl_hd;
    char *cpt;

    attr = mb->attr;
    tbl_hd = mb->tbl_hd;

    if (attr->update_cnt == INVALID) {	/* MB is just created and the structure is empty */
	old_cnt = 0;			/* attr->update_cnt not available */
	must_read = TRUE;		/* the info must be read initially */
    }
    else {
	old_cnt = attr->update_cnt;	/* where the last read has reached */
	must_read = FALSE;
    }
    
    tm = time (NULL);

    /* read attributes */
    if (lseek (mb->fd, (off_t)0, 0) < 0 ||
        read (mb->fd, (char *)attr,
	      sizeof (Mb_attribute)) != sizeof (Mb_attribute) ||
	BE_to_array_32 (attr, sizeof (Mb_attribute)) < 0 ||
	attr->mb_id != MB_ID) {	
      mb->need_update = TRUE;  /* This statement probably is
				* not necessary */
	return (MB_CORRUPTED);
    }

    /* check update cnt and time (3600 - one hour) */
    upd_cnt = attr->update_cnt;
    if (must_read == FALSE && 		
	mb->need_update == FALSE && 
	upd_cnt == old_cnt &&  /* The upd_cnt values are the same in the cache and the MB file */
	tm - mb->time <= 3600) {   /* The last cache update time is within one hour */
        /* No need to update the cache */
        mb->time = tm;
	return (Fix_r_pt (mb));
    }

    /* which part of the file needs to be read? - file read parameters */
    n_slots = mb->n_slots;    /* total number of slots in a table */
    end_rec = upd_cnt >> 3;   /* end_rec - total no. of messages ever written to the MB file */
    /* 
     * (old_cnt >> 3) = total no. message ever written to the MB file when the cache 
     *                  was updated last time.
     * to_read = difference of the total no. of messages ever written
     *           between the MB file and the cache. We need update the last
     *           'to_read' record slots to the cache from the MB file.
     */
    to_read = end_rec - (old_cnt >> 3);

    /* at most update n_slots records. If update_cnt falls back, we do a full update */
    if (to_read < 0 || to_read > n_slots)
	to_read = n_slots;

    if ((mb->flags & MB_REPLACE) &&    /* eliminates repeated updates
					  for replaceable MB, where Write_cache
					  with 0 new msg is called frequently */
	must_read == FALSE && (upd_cnt >> 3) == (old_cnt >> 3))
	to_read = 0;

    /* st_rec = the message number of the first message that needs to be read 
                into the cache. It is also equal to (old_cnt >> 3) + 1. */
    st_rec = end_rec + 1 - to_read;

    /* The slot index of the first message that needs to be read. */
    st_ind = (st_rec - 1 + n_slots) % n_slots;

    if (upd_cnt % 2 == 0)
	offset = mb->ta_off; /* upd_cnt is ever, update cache from table A */
    else 
	offset = mb->tb_off; /* upd_cnt is odd, update cache from table B  */

    /* the cache will be modified */
    mb->need_update = TRUE;

    /* first read */
    if (to_read + st_ind > n_slots)
        /* the slots of the messages to be read cross the end of the boundary */
	n = n_slots - st_ind;
    else 
	n = 0;

    if (n > 0) {  
        /* Read the first part of the slots from the first slot to 
	   the end of the boundary if needed. */
	size = n * sizeof (Msg_record);
	off = st_ind * sizeof (Msg_record);
	cpt = (char *)(mb->list + st_ind);
	if (lseek (mb->fd, (off_t)(offset + off), 0) < 0 ||
            read (mb->fd, cpt, size) != size ||
	    BE_to_array_32 (cpt, size) < 0)
	    return (MB_CORRUPTED);
	st_ind = 0;
    }

    /* Read the rest of the slots and the table header */
    io[0].iov_base = (char *) (mb->list + st_ind);
    io[0].iov_len = (to_read - n) * sizeof (Msg_record);
    size = io[0].iov_len;
    io[1].iov_base = (char *) (tbl_hd);
    io[1].iov_len = sizeof (Tbl_header);
    size += io[1].iov_len;
    io[2].iov_base = (char *) (mb->sit_buf);
    if (tbl_hd->seq_sit == FALSE) {	/* previous sit was not sequential */
	nitems = mb->n_msgs + 16;
	if (nitems > mb->maxn_msgs)
	    nitems = mb->maxn_msgs;
	mb->seq_sit = FALSE;		/* sit may be modified */
    }
    else 
	nitems = 0;
    io[2].iov_len = nitems * sizeof (sit_t);
    size += io[2].iov_len;
    off = st_ind * sizeof (Msg_record);
    if (lseek (mb->fd, (off_t)(offset + off), 0) < 0 ||
        readv (mb->fd, io, 3) != size ||
	BE_to_array_32 (io[0].iov_base, io[0].iov_len) < 0 ||
	BE_to_array_32 (io[1].iov_base,	io[1].iov_len) < 0 ||
	BE_to_array_16 (io[2].iov_base, io[2].iov_len) < 0)
      return (MB_CORRUPTED);

    /* Read the SIT array */
    if (tbl_hd->seq_sit == FALSE) {
	if (mb->flags & MB_SEQUENTIAL) {	/* This should never happen */
	    printf ("MB error 2\n");
	    return (MB_CORRUPTED);
	}
	n_sit_read = tbl_hd->n_msgs;
	mb->seq_sit = FALSE;		/* sit is not sequential */
    }
    else {
	n_sit_read = 0;			/* disable following read */
	if (mb->seq_sit != TRUE)	/* set up internal seq sit if necessary */
	    Set_sequential_sit (mb);
    }
    if (n_sit_read > nitems) {
	size = (n_sit_read - nitems) * sizeof (sit_t);
	cpt = (char *) (mb->sit_buf + nitems);
	if (n_sit_read > mb->maxn_msgs ||
	    read (mb->fd, cpt, size) != size ||
	    BE_to_array_16 (cpt, size) < 0)
	return (MB_CORRUPTED);
    }

    /* verify update_cnt */
    if (tbl_hd->update_cnt != attr->update_cnt)
	return (MB_CORRUPTED);

    /* update MB structure */
    n_msgs = tbl_hd->n_msgs;
    mb->time = tm;
    mb->need_update = FALSE;
    mb->sit = mb->sit_buf;
    mb->sit_size = mb->maxn_msgs + 64;
    mb->n_msgs = n_msgs;
    mb->st_ind = ((upd_cnt >> 3) - n_msgs + n_slots) % n_slots;

    if (mb->flags & MB_PARITY)
	mb->parity = Find_parity (mb);

    return (Fix_r_pt (mb));
}

/************************************************************************

Function Name: 	Fix_r_pt

Description: This function updates the r_pt field in the MB structure 
	if there was an id fall-back.

************************************************************************/

static int Fix_r_pt (Mb_struct *mb)
{

    /* The max_id falls back by ID_PAGE_SIZE in the Write_new_message
       (called by the MB_write) when the new message id is equal to 
       MAX_ID, but the r_pt keeps incrementing by each MB_read/MB_seek
       function. The r_pt needs to be reduced if we find the max_id 
       already falls back. The number added to the max_id can be any
       number between 1 and ID_PAGE_SIZE. */
    if ((mb->flags & MB_SEQUENTIAL) && 
	mb->r_pt > mb->tbl_hd->max_id + 2 && mb->r_pt >= ID_PAGE_SIZE)
	mb->r_pt -= ID_PAGE_SIZE;

    return (MB_SUCCESS);
}



/************************************************************************

Function Name: 	Write_cache

Description: This function writes out the control info cache to the MB
        file. It is called from the MB_write function. The argument "n_new" 
        must be either 0 or 1. 

	n_new is 0 - if certain old messages are removed for freeing up 
	             space to the new message.

        n_new is 1 - if a new message is written to the message buffer. 

	The function first writes the part of the control area that needs
	to be updated in the file. In the worst case three write calls
	are needed. Most often the first and the third write calls are 
	unnecessary and will be skipped. The function finishes the 
	cache write-out by updating the update_cnt in the file.

	This function returns MB_SUCCESS on success or MB_UPDATE_FAILED 
	on failure.

************************************************************************/

static int Write_cache (
    Mb_struct * mb, 			/* MB structure */
    int n_new				/* number of new records added */
)
{
    int cur_ind, offset, off, s, size;
    int n_slots, to_write, n;
    struct iovec io [3];
    int upd_cnt, new_cnt;
    Tbl_header *tbl_hd;
    char *cpt;
    si32 tmp;

    tbl_hd = mb->tbl_hd;

    n_slots = mb->n_slots;
    upd_cnt = mb->attr->update_cnt;

    /* slot index of the latest message */
    cur_ind = ((upd_cnt >> 3) - 1 + n_slots) % n_slots;

    /* next table location */
    if (upd_cnt % 2 == 0) {
        /* upd_cnt is even, set 1 to the lower 3 bits in the upd_cnt to make 
	   it to be an odd number */
	s = 1;  
        /* The current upd_cnt is even. The upd_cnt will be odd after the cache
	   being written, so that we need to write cache to table B */
	offset = mb->tb_off;
    }
    else {
        /* upd_cnt is odd, set 0 to the lowser 3 bits in the upd_cnt to make
	   it to be an even number */
	s = 0;  
	/* The current upd_cnt is odd. The upd_cnt will be even after the cache
	   being written, so that we need to write cache to table A */
	offset = mb->ta_off;
    }

    /* figure out the new update_cnt */
    if (n_new > 0) {
        /* n_new is 1. A new message is written into the message buffer.
	   Add 1 to the higher 29-bit and set the lower 3-bit to s of the upd_cnt */
	new_cnt = (((upd_cnt >> 3) + n_new) << 3) + s;
    }
    else { 
        /* n_new is 0. Some old messages are removed from the message buffer */
	if ((upd_cnt & 7) < 7)
	    /* Add one to the lower 3-bit of the upd_cnt such that the table update 
	       will apply to the non-current one. i.e. If the current table is A 
	       (upd_cnt is even), it will write to table B and then change the upd_cnt 
	       to an odd number. Note: the higher 29-bit of upd_cnt is not changed.
	       Since a Write_cache(0) function (if it is called) is always followed 
	       by a Write_cache(1) call (see MB_write source), and the Write_cache(1) 
	       function always resets the lower 3-bit of the upd_cnt to 0 or 1 (see above),
	       the lower 3-bit of the upd_cnt should not be greater 2 usually */
	    new_cnt = upd_cnt + 1;
	else 		
	    /* This happens only in exceptional cases */
	    /* This will cause a full cache update in other MB users */
	    new_cnt = (((upd_cnt >> 3) + n_slots) << 3) + s;
    }

    /* process update cnt overflow */
    /* because n_new <= 1 and n_slots >= n_new, the following right sight and new_cnt
       are always positive */
    if (new_cnt > ((0x10000000 - n_slots - 4) << 3)) {	/* 2^28 */
	int n;

	n = new_cnt >> 3;
	n = n % n_slots;
	new_cnt = (new_cnt & 7) + (n << 3);
    }

    /* Update the new_cnt */
    tbl_hd->update_cnt = new_cnt;

    to_write = n_new + 1;	/* One more record has to be written out because the 
				   table is double buffered. The current table always 
				   contains one more record than the other. */

    /* remove uninitialized read */
/* the following code is to remove the write of uninitialized records when the MB
   is initialized and contains very few messages. This, however introduces a
   bug when MB_clear is called, in which case mb->n_msgs = 0 and n_new = 0, and   
   cur_ind = n_slots - 1. In this case the tbl_hd will be written to the
   beginning instead of after the last slot. I couldn't find an easy fix. I
   just remove this and understand that warning will be generated by some 
   memory tools.

    while (to_write > mb->n_msgs) {		
	to_write--;
	cur_ind = (cur_ind + 1) % n_slots;
    }
*/

    /* first write */
    if (to_write + cur_ind > n_slots)
	n = n_slots - cur_ind;
    else 
	n = 0;
    if (n > 0) {
	size = n * sizeof (Msg_record);
	off = cur_ind * sizeof (Msg_record);
	cpt = (char *)(mb->list + cur_ind);
	if (lseek (mb->fd, (off_t)(offset + off), 0) < 0 ||
	    BE_from_array_32 (cpt, size) < 0 ||
            write (mb->fd, cpt, size) != size ||
	    BE_to_array_32 (cpt, size) < 0)
	  return (MB_UPDATE_FAILED);
	cur_ind = 0;
    }

    /* second write */
    io[0].iov_base = (char *) (mb->list + cur_ind);
    io[0].iov_len = (to_write - n) * sizeof (Msg_record);
    size = io[0].iov_len;
    io[1].iov_base = (char *) (tbl_hd);
    io[1].iov_len = sizeof (Tbl_header);
    size += io[1].iov_len;
    io[2].iov_base = (char *) (mb->sit);
    if (mb->seq_sit == FALSE) {
	io[2].iov_len = tbl_hd->n_msgs * sizeof (sit_t);
	tbl_hd->seq_sit = FALSE;
    }
    else {				/* no sit write is needed */
	io[2].iov_len = 0;
	tbl_hd->seq_sit = TRUE;
    }
    size += io[2].iov_len;
    off = cur_ind * sizeof (Msg_record);
    if (lseek (mb->fd, (off_t)(offset + off), 0) < 0 ||
	BE_from_array_32 (io[0].iov_base, io[0].iov_len) < 0 ||
	BE_from_array_32 (io[1].iov_base, io[1].iov_len) < 0 ||
	BE_from_array_16 (io[2].iov_base, io[2].iov_len) < 0 ||
        writev (mb->fd, io, 3) != size ||
	BE_to_array_32 (io[0].iov_base, io[0].iov_len) < 0 ||
	BE_to_array_32 (io[1].iov_base,	io[1].iov_len) < 0 ||
	BE_to_array_16 (io[2].iov_base,	io[2].iov_len) < 0)
	return (MB_UPDATE_FAILED);

    /* write update_cnt */
    tmp = new_cnt;
    if (lseek (mb->fd, (off_t)UPDATE_CNT_OFF, 0) < 0 ||
	BE_from_array_32 (&tmp, sizeof(mb_t)) < 0 ||
	write (mb->fd, (char *) &tmp, sizeof (mb_t)) != sizeof (mb_t))
	return (MB_UPDATE_FAILED);	

    mb->attr->update_cnt = new_cnt;
    mb->need_update = FALSE;
    return (MB_SUCCESS);
}

/************************************************************************

Function Name: 	Check_msg_table

Description: This function checks the internal consistency of the message
	table in mb. Although, this is still not a complete check, which 
	should also checks the n_slots, tbl_size and other attributes, a
	corrupted MB is likely to be detected.

	It returns MB_SUCCESS on success or MB_FAILURE if the message
	table is corrupted.

************************************************************************/

static int 
Check_msg_table (
    Mb_struct *mb		/* the MB structure */
)
{
    int ploc, n_msgs, n_slots, mb_size;
    int i, st_ind, ind, s, size, cnt;
    Msg_record *list;
    Tbl_header *thd;
    sit_t *sit;
    int free_loc;
    mbmsg_t pid;

    thd = mb->tbl_hd;
    free_loc = thd->free_loc;
    n_msgs = mb->n_msgs;
    mb_size = mb->mb_size;
    n_slots = mb->n_slots;

    /* preliminary checks */
    if (n_msgs < 0 || n_msgs > mb->maxn_msgs ||
	n_slots < mb->maxn_msgs || 
	free_loc < 0 || free_loc > mb_size ||
	thd->free_size < 0 || thd->free_size > mb_size)
	return (MB_CORRUPTED);

    if (n_msgs == 0)
	return (MB_SUCCESS);

    /* go over the table, check locations */
    list = mb->list;
    st_ind = mb->st_ind;
    ind = st_ind;
    ploc = list[ind].loc;		/* location of previous message */
    size = 0;
    for (i = 1; i < n_msgs; i++) {
	int loc;

	ind = (ind + 1) % n_slots;
	loc = list[ind].loc;
	s = loc - ploc;
	if (s < 0)
	    s += mb_size;
	if (loc < 0 || loc >= mb_size || s < 0) 
	    return (MB_CORRUPTED);

	size += s;			/* total size */
	ploc = loc;
    }
    s = free_loc - ploc;		/* size of the very last message */
    if (s <= 0)
	s += mb_size;
    size += s;
    if (list[st_ind].loc != ((free_loc + thd->free_size) % mb_size) ||
	size + thd->free_size != mb_size) 
	return (MB_CORRUPTED);

    /* go over the table, check ids */
    sit = mb->sit;
    pid = 0;	
    cnt = 0;    
    for (i = 0; i < n_msgs; i++) {
	mbmsg_t id;

	ind = (int) sit [i] - thd->sit_base;
	if (ind < 0 || ind >= n_msgs)
	    return (MB_CORRUPTED);
	id = list [Recind_from_sitind (mb, i)].id;
	if (id <= pid && i > 0)
	    cnt++;
	pid = id;
    }
    if (mb->flags & MB_SEQUENTIAL) 	/* there can be one decreasing */
	cnt--;
    if (cnt >= 1)
	return (MB_CORRUPTED);

    return (MB_SUCCESS);
}

/************************************************************************

Function Name: 	Release_lock

Description: This function calls Set_lock to release a lock.

************************************************************************/

static int
Release_lock  (
      int fd			/* file descriptor */
) 
{
    return (Set_lock (MB_RELEASE_LOCK, fd, 0));
}


/************************************************************************

Function Name: 	Set_lock

Description: This function locks (sw = MB_SET_LOCK) or unlocks 
	(sw != MB_SET_LOCK) a file "fd". The lock type is specified
	by "type". This function will not return until a lock
	is granted.

	This function returns MB_SUCCESS on success or MB_FAILURE 
	if the "fcntl" call returns with an error. 

************************************************************************/

static int
Set_lock  (
      int sw,			/* function switch */
      int fd,			/* file descriptor */
      int type			/* EXCLUSIVE or SHARED */
)
{
    static struct flock fl;	/* structure used by fcntl */
    static int init = 0;	/* flag for initializing fl */
    int err;

    if (init == 0) {
	init = 1;
	fl.l_whence = SEEK_SET;
	fl.l_start = 0;
	fl.l_len = 0;
    }

    if (sw == MB_SET_LOCK) {
	if (type == EXCLUSIVE)
	    fl.l_type = F_WRLCK;
	else 
	    fl.l_type = F_RDLCK;
    }
    else
	fl.l_type = F_UNLCK;

    while ((err = fcntl (fd, F_SETLKW, &fl)) == -1 && errno == EINTR);

    if (err == -1)
	return (MB_FAILURE);
    else
	return (MB_SUCCESS);
}

/************************************************************************

Function Name: Unlock_return	

Description: This function releases a lock on the file fd and returns 
	the value "ret_value".

	We do not check the error of the unlock call. It is impossible
	to fail in unlocking a file while the file was successfully
	locked unless there is an OS problem.

************************************************************************/

static int 
Unlock_return (
    int fd,			/* file fd to be unlocked */
    int ret_value		/* return value */
) 
{

    Set_lock (MB_RELEASE_LOCK, fd, 0);
    return (ret_value);
}


/************************************************************************

Function Name: 	Get_mb_structure

Description: This function checks an MB descriptor "mbd" and, if it is OK,
	locks the file, updates the the control info cache and 
	returns the pointer to the MB structure in argument "new_mb". 
	The file is locked in this function unless the function fails.

	It returns MB_INVALID if "mbd" is an invalid MB descriptor or
	MB_LOCK_FAILED if it failed to lock the file. It returns 
	MB_SUCCESS on success.

************************************************************************/

static int
  Get_mb_structure
  (
    int mbd,			/* MB descriptor */
    int lock_type,		/* type of lock */
    Mb_struct **new_mb		/* returns the mb structure */
) {
    Mb_struct *mb;
    int ret;

    /* check the MB descriptor for validity */
    if (mbd - MB_START_FD < 0 || mbd - MB_START_FD >= MAX_N_MBS ||
	Mb_handle[mbd - MB_START_FD] == NULL)
	return (MB_INVALID);

    mb = Mb_handle[mbd - MB_START_FD];
    *new_mb = mb;

    if ((mb->flags & MB_PARITY) && lock_type == EXCLUSIVE &&
	mb->need_update == FALSE && mb->parity != Find_parity (mb)) 
	return (MB_PARITY_ERROR);

    /* lock the file */
    if (Set_lock (MB_SET_LOCK, mb->fd, lock_type) == MB_FAILURE) 
	return (MB_LOCK_FAILED);

    /* read MB information */
    if ((ret = Update_cache (mb)) != MB_SUCCESS)
	return (Unlock_return (mb->fd, ret));

    return (MB_SUCCESS);
}


/************************************************************************

Function Name: 	Find_index_in_table

Description: By using a binary search, this function finds the first
	message in the sorted list of all available messages in "mb" 
	that has an id bigger than or equal to "id". If all message ids
	are less than "id", an index pointing to the location after 
	the last message (n_msgs) is returned.

	This function returns the message index (in the range of 0 through 
	n_msgs) in the list of all available messages on success. 
	If n_msgs <= 0, the function returns MB_FAILURE.

************************************************************************/

static int 
Find_index_in_table (
    Mb_struct *mb,			/* MB structure */
    mbmsg_t id				/* id to search for */
)
{
    int st_ind, end_ind, base_ind, n_slots;
    int ind, max_id;
    Msg_record *msgtbl;
    sit_t *sit;

    st_ind = 0;
    end_ind = mb->n_msgs - 1;
    if (st_ind > end_ind) 
	return (MB_FAILURE);

    if (mb->flags & MB_SEQUENTIAL) {	/* we don't need binary search */
	max_id = mb->tbl_hd->max_id;    /* Maximum message ID ever used */

	if (id > max_id + 2 && id >= ID_PAGE_SIZE)	
	    /* If MB_read message ID is MB_CURRENT, the r_pt is passed
	       into the function as "id", which can be greater than max_id.
	       See the comments in the Fix_r_pt function */
	    id -= ID_PAGE_SIZE;

	if (max_id >= id + (mbmsg_t)mb->n_msgs)
	    ind = 0;            /* id is less than any message id's */
	else if (id > max_id)
	    ind = mb->n_msgs;   /* id is greater than all message id's */
	else 
	    ind = end_ind - (max_id - id);
	return (ind);
    }

    sit = mb->sit;
    msgtbl = mb->list;
    n_slots = mb->n_slots;
    base_ind = mb->st_ind + n_slots - mb->tbl_hd->sit_base;

    /* we first do this check (which is needed anyway)
       This increases efficiency when messages tend to come in order */
    if (id > msgtbl[(base_ind + (int) sit[end_ind]) % n_slots].id) 
	return (end_ind + 1);

    ind = (st_ind + end_ind) / 2;
    while (ind != st_ind) {
	if (id > msgtbl[((int) sit[ind] + base_ind) % n_slots].id) 
	    st_ind = ind;
	else 
	    end_ind = ind;

	ind = (st_ind + end_ind) / 2 ;
    }

    if (id > msgtbl[((int) sit[ind] + base_ind) % n_slots].id) 
	return (ind + 1);
    else
        return (ind);
}

/************************************************************************

Function Name: 	Get_free_space

Description: This function searches for expired messages and increases
	the free space to at least "length" bytes and decrease the message 
	number to be less than maxn_msgs. The first come
	messages are first expired as described in mb.doc. 

	The argument "length" must be smaller than the size of the MB.

	Structures of mb is updated to conform to the new list with 
	expired messages removed. 

	This function does not expire any message if the flag
	MB_REPLACE is set.

	This function returns MB_SUCCESS on success or MB_FAILURE 
	if some of the messages could not be expired. 

************************************************************************/

static int 
Get_free_space (
    Mb_struct *mb, 		/* MB structure */
    int length			/* target free space size */
)
{

    if (length <= mb->tbl_hd->free_size && mb->n_msgs < mb->attr->maxn_msgs)
	return (MB_SUCCESS);

    if (mb->flags & MB_REPLACE)
	return (MB_FAILURE);

    while (mb->n_msgs >= mb->attr->maxn_msgs) {
	if (Remove_a_message (mb) == MB_FAILURE)
	    return (MB_FAILURE);
    }

    while (length > mb->tbl_hd->free_size) {
	if (Remove_a_message (mb) == MB_FAILURE)
	    return (MB_FAILURE);
    }

    return (MB_SUCCESS);
}

/************************************************************************

Function Name: 	Remove_a_message

Description: This function removed the oldest message in the message
	list. If the message is never read and MB_MUST_READ is 
	set, the message is not expired.

	Structures of mb is updated to conform to the new list with 
	the expired message removed. 

	This function returns MB_SUCCESS or MB_FAILURE. 

************************************************************************/

static int 
Remove_a_message (
    Mb_struct *mb			/* MB structure */
)
{
    Tbl_header *tbl_hd; 
    Msg_record *list;
    sit_t *sit;
    int i, k, size, base;
    int n_msgs, n_slots, st_ind;

    n_msgs = mb->n_msgs;
    if (n_msgs <= 0)
	return (MB_FAILURE);

    tbl_hd = mb->tbl_hd;
    sit = mb->sit;
    list = mb->list;
    st_ind = mb->st_ind;
    base = tbl_hd->sit_base;

    if (mb->flags & MB_MUST_READ) {	/* check if the msg can be expired */
	mbmsg_t tmp, read_id;		/* must be unsigned if id is unsigned */

	read_id = mb->attr->read_id;
	if (read_id > tbl_hd->max_id && read_id >= ID_PAGE_SIZE)
	    read_id -= ID_PAGE_SIZE;
	tmp = list[st_ind].id;
	if (tmp > tbl_hd->max_id && tmp >= ID_PAGE_SIZE)
	    tmp -= ID_PAGE_SIZE;
	if (read_id < tmp)
	    return (MB_FAILURE);	/* unable to expire */
    }

    n_slots = mb->n_slots;
    size = Get_msg_size (mb, st_ind);

    mb->need_update = TRUE;
    if (mb->seq_sit == FALSE) {

	/* search through SIT to find the expired message */
	for (i = 0; i < n_msgs; i++) {
	    if (sit[i] == base)  /* sit[i] points to the earliest message */
		break;
	}
	if (i >= n_msgs) {		/* internal error, should never happen */
	    printf ("MB error 1\n");
	    return (MB_FAILURE);
	}	

	/* modify SIT, shift the sit from (i-1) to 0 by 1 such that
	   sit[i] = sit[i-1], sit[i-2]=sit[i-1], ... sit[1]=sit[0] */
	for (k = i - 1; k >= 0; k--)
	    sit [k + 1] = sit [k];
	mb->sit++;           /* sit points to the (sit_buf+1) */
	mb->sit_size--;      /* subtract sit_size by 1 */
	tbl_hd->sit_base++;  /* Increment sit_base by 1 
				Note: This is the only place to update the sit_base */
    }

    mb->n_msgs--;            /* subtract number of messages by 1 */
    mb->st_ind = (mb->st_ind + 1) % n_slots;  /* increment the slot starting index */
    tbl_hd->free_size += size;
    tbl_hd->n_msgs = mb->n_msgs;

    return (MB_SUCCESS);
}

/************************************************************************

Function Name: 	Write_new_message

Description: This function copies "message" of length "length" bytes to
	the MB "mb". The free space size is assumed to be sufficient
	for the message. The mb structure is then updated. The message
	id may be modified in the function for sequential mbs.

	This function returns MB_SUCCESS on success. It returns
	MB_UPDATE_FAILED on failure.

************************************************************************/

static int 
Write_new_message (
    Mb_struct *mb, 		/* MB structure */
    char *message, 		/* the new message */
    int length,			/* new message length */
    mbmsg_t *id,		/* new message id */
    int index			/* new message index in the table */
)
{
    static int cnt = 0;
    Tbl_header *tbl_hd; 	/* table header */
    int ret, new_ind, n_msgs, sit_base;
    Msg_record *msgtbl;
    sit_t *sit;
    int i;

    /* write the message to the message area */
    tbl_hd = mb->tbl_hd;
    ret = Write_a_message (mb, tbl_hd->free_loc, length, message, 0);
    if (ret != MB_SUCCESS)
	return (ret);

    /* we modify cache in the following */
    mb->need_update = TRUE;

    /* make sure the sit space is enough for the new item and the sit-base
       is not too large; Checks if the sit is sequential and turns it to
       sequential if it is. 
       Note: Both the sit_base value and the sit pointer are incremented 
             by 1, respectively, when an old message is removed (see 
	     Remove_a_message function). */

    n_msgs = tbl_hd->n_msgs;
    sit_base = tbl_hd->sit_base;

    if ((mb->flags & MB_SEQUENTIAL) == 0)
        /* It is not a sequential MB, 
	   gives a chance to turn to seq sit */
	cnt++;

    if (cnt % 64 == 63 ||          /* There are max. 64 sit items can be grown */
	mb->sit_size <= n_msgs ||  /* sit_size become too small */
	sit_base >= mb->n_slots || /* sit_base is too big */
	sit_base + mb->maxn_msgs >= 0xffff) {  /* Value in sit array is short int (16-bit) */ 

	int is_seq;

	/* Reset the sit pointer to sit_buffer, the sit_size to (maxn_msgs+64), 
	   and the sit_base value to 0, if any of the above cases happen. */

	is_seq = 1;
	for (i = 0; i < n_msgs; i++)  {
	    mb->sit_buf[i] = mb->sit[i] - sit_base;
	    if (mb->sit_buf[i] != i)
		is_seq = 0;
	}
	mb->sit = mb->sit_buf;
	mb->sit_size = mb->maxn_msgs + 64;  /* That's why the cnt is set to 64 */
	tbl_hd->sit_base = 0;
	if (is_seq == 1 && n_msgs > 1)	/* turns into a sequential sit */
	    Set_sequential_sit (mb);
    }
	
    /* turn SIT into non-sequential */
    if (index != n_msgs) {
	if (mb->flags & MB_SEQUENTIAL) {	/* This should never happen */
	    printf ("MB error 3\n");
	    return (MB_CORRUPTED);
	}
	mb->seq_sit = FALSE;		/* turns into a non-seq sit */
    }

    /* insert the new item in SIT */
    if (mb->seq_sit == FALSE) {		/* sit update is needed */
	sit = mb->sit;
	/* Shift the items after the sit[index] by 1 */
	for (i = n_msgs; i > index; i--) {  
	    sit[i] = sit[i-1];
	}
	/* The sit[index] points to the latest message n_msgs */
	sit [index] = (sit_t) (n_msgs + mb->tbl_hd->sit_base);
	/* Note: the items in the SIT table are stored in sit[0] ... sit[n_msgs-1] */
    }

    if ((mb->flags & MB_SEQUENTIAL) && *id >= MAX_ID)
        /* Sequential MB id becomes too large, reduce it by ID_PAGE_SIZE */
	*id -= ID_PAGE_SIZE;

    /* add the new record in the message record table */
    new_ind = (mb->st_ind + n_msgs) % mb->n_slots;
    msgtbl = mb->list;
    msgtbl[new_ind].id = *id;
    msgtbl[new_ind].loc = tbl_hd->free_loc;
    mb->n_msgs++;

    /* update table header */
    tbl_hd->free_loc = (tbl_hd->free_loc + length) % mb->mb_size;
    tbl_hd->free_size -= length;
    tbl_hd->n_msgs = mb->n_msgs;
    tbl_hd->max_id = *id;

    return (MB_SUCCESS);
}

/************************************************************************

Function Name: 	Write_a_message

Description: This function writes a message at "loc" in the MB "mb".
	It returns MB_SUCCESS on success or MB_UPDATE_FAILED
	if file write failed.

************************************************************************/

static int
Write_a_message (
    Mb_struct *mb, 		/* MB structure */
    int loc,			/* the location in the message area */
    int length,			/* msg length */
    char *message,		/* message */
    int add_off			/* additional offset of the message */
)
{
    int copied;
    int offset;

    copied = 0;			/* bytes copied */
    offset = loc;
    while (copied < length) {
	int len;

	len = length - copied;
	if (len > mb->mb_size - offset)
	    len = mb->mb_size - offset;
	if (lseek (mb->fd, (off_t)(offset + mb->msg_off + add_off), 0) < 0 ||
	    write (mb->fd, message + copied, len) != len)
	    return (MB_UPDATE_FAILED);

	copied += len;
	offset = (offset + len) % mb->mb_size;
    }

    return (MB_SUCCESS);
}

/************************************************************************

Function Name: 	Recind_from_sitind

Description: This function calculates and returns the index in message 
	record table (0 through n_slots) for a message given the 
	message's index in the SIT. 

************************************************************************/

static int 
Recind_from_sitind (
    Mb_struct *mb,			/* MB structure */
    int ind				/* SIT index */
)
{
    return (((int) mb->sit[ind] + mb->st_ind -
		 mb->tbl_hd->sit_base + mb->n_slots) % mb->n_slots);
}


/************************************************************************

Function Name: 	Get_msg_size

Description: This function calculates and returns the size of the message
	located at slot ind in the message record table. The ind must
	be a slot index, where a  valid message record exists.

************************************************************************/

static int 
Get_msg_size (
    Mb_struct *mb,			/* mb structure */
    int ind				/* index in the massage record 
					   table (0 through n_slots - 1) */
) {
    int size;
    Msg_record *list;

    list = mb->list;

    if (ind != (mb->st_ind + mb->n_msgs - 1) % mb->n_slots) 
	size = list [(ind + 1) % mb->n_slots].loc - list[ind].loc;
    else
	size = mb->tbl_hd->free_loc - list[ind].loc;
    if (size <= 0)
	size += mb->mb_size;

    return (size);
}

/************************************************************************

Function Name: 	MB_write

Description: Refer to mb.doc for a description of this function and its
	design.

************************************************************************/

int 
_MB_write (
    int mbd, 			/* MB descriptor */
    char *message, 		/* message buffer */
    int length,			/* message length */
    unsigned int id, 		/* message id */
    unsigned int *ret_id	/* id actually used */
)
{
    Mb_struct *mb;
    Mb_attribute *attr;
    int ret, fd;
    int index;
    Tbl_header *tbl_hd;

    /* check arguments */
    if ((id >= MB_LATEST && id != MB_NEW_ID) || length <= 0 || message == NULL)
	return (MB_BAD_ARGUMENT);

    /* get MB structure, lock the file and update the struct */
    ret = Get_mb_structure (mbd, EXCLUSIVE, &mb);
    if (ret != MB_SUCCESS)
	return (ret);
    fd = mb->fd;
    tbl_hd = mb->tbl_hd;
    attr = mb->attr;
    if ((mb->flags & MB_RDWR) == 0)
	return (Unlock_return (fd, MB_READ_ONLY));

    /* we assume the MB can hold at least 2 messages, which guarantees that 
	the new message will not be removed immediately */
    if (((mb->flags & MB_REPLACE) == 0) && length * 2 > mb->mb_size)
	return (Unlock_return (fd, MB_TOO_LARGE));

    /* assign an id */
    if (id == MB_NEW_ID) {
	if ((mb->flags & MB_SEQUENTIAL) == 0)
	    return (Unlock_return (fd, MB_NEED_ID));
	id = mb->tbl_hd->max_id + 1;
    }
    else if ((mb->flags & MB_SEQUENTIAL) != 0) {
        if (id >= MAX_ID || (mb->n_msgs > 0 && id != mb->tbl_hd->max_id + 1))
	    return (Unlock_return (fd, MB_ID_NOT_ACCEPTED));
    }

    /* find where to insert */
    index = Find_index_in_table (mb, id);
    if (index == MB_FAILURE)			/* empty table */
	index = 0;
    else if (index < mb->n_msgs) {
	Msg_record *rec;
	int ind;

	ind = Recind_from_sitind (mb, index);
	rec = mb->list + ind; 

	if(id == rec->id) {			/* check if the msg exists */
	    if (mb->flags & MB_REPLACE) {	/* replace the message */
		int size, add_off;
		
		size = Get_msg_size (mb, ind);
		if (size != length)
		    return (Unlock_return (fd, MB_DUPLICATED));

		mb->need_update = TRUE;
		if ( (tbl_hd->dbloc_bits & (0x1 << ind)) == 0) {
		    tbl_hd->dbloc_bits |= (0x1 << ind);
		    add_off = mb->mb_size;
		}
		else {
		    tbl_hd->dbloc_bits -= 0x1 << ind;
		    add_off = 0;
		}

		ret = Write_a_message (mb, rec->loc, length, message, add_off);
		if (ret == MB_SUCCESS) {
		    ret = Write_cache (mb, 0);

		    if (ret != MB_SUCCESS)
			return (Unlock_return (fd, ret));
		    else {
			if (ret_id != NULL)
			    *ret_id = id;
			if (mb->flags & MB_PARITY)
			    mb->parity = Find_parity (mb);
			return (Unlock_return (fd, length));
		    }
		}
		else
		    return (Unlock_return (fd, ret));
	    }
	    else				/* duplicate message */
		return (Unlock_return (fd, MB_DUPLICATED));
	}
    }

    if (length > tbl_hd->free_size || mb->n_msgs >= attr->maxn_msgs) {	
        /* Remove expired msgs for free space */
	if (Get_free_space (mb, length) == MB_SUCCESS) {
	    /* Switch the current table by adding 1 to the lowser 3-bit of
	       the upd_cnt (but no update to the higher 29-bit of the upd_cnt)
	       and write out the cache. */
	    if (Write_cache (mb, 0) != MB_SUCCESS)
		return (Unlock_return (fd, MB_UPDATE_FAILED));

	    /* we need to update the index */
	    index = Find_index_in_table (mb, id);
	    if (index == MB_FAILURE)			/* empty table */
	        index = 0;
	}
	else 
	    return (Unlock_return (fd, MB_FULL));
    }

    /* write the new message and update tbl_hd and mb */
    ret = Write_new_message (mb, message, length, &id, index);
    if (ret != MB_SUCCESS)
	return (Unlock_return (fd, ret));

    /* reserve free space for next write */
    Get_free_space (mb, length);

    /* Switch the current table (changing the lower 3-bit of the upd_cnt to
       0 if current table is B and 1 if current table is A, and write out 
       the cache */
    if (Write_cache (mb, 1) != MB_SUCCESS)
	return (Unlock_return (fd, MB_UPDATE_FAILED));

    if (mb->flags & MB_PARITY)
	mb->parity = Find_parity (mb);

    if (ret_id != NULL)
	*ret_id = id;
    return (Unlock_return (fd, length));
}

/************************************************************************

Function Name: 	Find_message_index

Description: This function finds and returns the index (in the range of 0 
	through n_msgs - 1) in the sorted list of all available messages in 
	terms of the message id. Various special values for the message 
	id are processed here. The index found is returned in the 
	argument index on success.

	It returns MB_FOUND on success, MB_NOT_FOUND if the specified
	message is not found, MB_BAD_ARGUMENT if the argument "msg_id" 
	is illegal, or MB_ID_NOT_ACCEPTED if the id is normal and the 
	MB is of MB_SEQUENTIAL type.

************************************************************************/

static int 
Find_message_index (
    Mb_struct *mb,			/* mb structure */
    mbmsg_t msg_id,			/* message id to search for */
    int *index				/* returns the index found */
)
{
    int ind;

    if (msg_id >= MB_LATEST && msg_id != MB_LARGEST && msg_id != MB_CURRENT && 
			msg_id != MB_LATEST && msg_id != MB_SMALLEST) 
	return (MB_BAD_ARGUMENT);

    if (mb->n_msgs == 0) 
	return (MB_NOT_FOUND);

    if (msg_id == MB_SMALLEST)
        /* choose the first index in the SIT */
	ind = 0;
    else if (msg_id == MB_LARGEST)
        /* choose the last index in the SIT */
	ind = mb->n_msgs - 1;
    else if (msg_id == MB_CURRENT)
        /* find the index from the SIT such that the corresponding message 
	   id is equal to or greater than the read pointer. */
        ind = Find_index_in_table (mb, mb->r_pt);
    else if (msg_id == MB_LATEST) 
        /* find the index from the SIT such that the corresponding message 
	   is latest to be added to the table. */
        ind = Find_index_in_table (mb, 
		mb->list [(mb->st_ind + mb->n_msgs - 1) % mb->n_slots].id);
    else 
        /* find the index from SIT such that the corresponding message id
	   is equal to msg_id */
        ind = Find_index_in_table (mb, msg_id);

    if (ind < 0 ||				/* empty table */
	ind >= mb->n_msgs)			/* to the end of the list */
	return (MB_NOT_FOUND);

    *index = ind;
    return (MB_FOUND);
}

/************************************************************************

Function Name: MB_read	

Description: Refer to mb.doc for a description of this function.

************************************************************************/

int 
_MB_read (
    int mbd, 			/* MB descriptor */
    char *buffer, 		/* buffer for the message */
    int buf_size, 		/* buffer size */
    unsigned int msg_id, 	/* id of the message to read */
    unsigned int *ret_id	/* to return the id of the message read */
)
{

    return (mb_read (mbd, buffer, READ_ALL, buf_size, msg_id, ret_id));
}

/************************************************************************

Function Name: MB_read_part	

Description: Refer to mb.doc for a description of this function.

************************************************************************/

int 
_MB_read_part (
    int mbd, 			/* MB descriptor */
    char *buffer, 		/* buffer for the message */
    int offset,			/* starting offset in the message */
    int length, 		/* number of bytes to read */
    unsigned int msg_id, 	/* id of the message to read */
    unsigned int *ret_id	/* to return the id of the message read */
)
{

    /* check arguments */
    if (offset < 0 || length <= 0)
	return (MB_BAD_ARGUMENT);

    return (mb_read (mbd, buffer, offset, length, msg_id, ret_id));
}

/************************************************************************

Function Name: mb_read	

Description: This is a generic message reading function which is called by
	both MB_read and MB_read_part. For a detailed description of this 
	function and its implementation refer to mb.doc.

************************************************************************/

static int 
mb_read (
    int mbd, 			/* MB descriptor */
    char *buffer, 		/* buffer for the message */
    int st_offset,		/* starting offset in the message */
    int length, 		/* number of bytes to read */
    unsigned int msg_id, 	/* id of the message to read */
    unsigned int *ret_id	/* to return the id of the message read */
)
{
    Mb_struct *mb;
    int ret, size, ind, add_off;
    int index, fd;
    Msg_record *msgrec;
    int nread, off;
    int mbsize, msgsize;

    /* check arguments ("msg_id" will be checked later in Find_message_index) */
    if (buffer == NULL || length <= 0)
	return (MB_BAD_ARGUMENT);

    /* get MB structure, lock the file and update the struct */
    ret = Get_mb_structure (mbd, SHARED, &mb);
    if (ret != MB_SUCCESS)
	return (ret);
    fd = mb->fd;

    /* find where is the message, return index ranging 
       from 0 - (n_mesg-1), or MB_NOT_FOUND */
    if ((ret = Find_message_index (mb, msg_id, &index)) != MB_FOUND)
	return (Unlock_return (fd, ret));

    /* ind = index of the slot storing the Msg_record, range from 0 - (n_slots-1) */
    ind = Recind_from_sitind (mb, index);

    msgrec = mb->list + ind;
    if (msg_id < MB_LATEST && msgrec->id != msg_id) 	/* must match if normal id */
	return (Unlock_return (fd, MB_NOT_FOUND));

    msgsize = Get_msg_size (mb, ind);  /* how many bytes to read from the message area */
    if (st_offset == READ_ALL) {
	if (msgsize > length)
	    return (Unlock_return (fd, MB_TOO_SMALL));
	size = msgsize;
	st_offset = 0;
    }
    else {
	size = msgsize - st_offset;
	if (size > length)
	    size = length;
    }

    /* read the message */
    nread = 0;	           /* bytes read */
    add_off = 0;
    mbsize = mb->mb_size;
    if ((mb->flags & MB_REPLACE) != 0 &&
	(mb->tbl_hd->dbloc_bits & (0x1 << ind)) != 0)
	add_off = mbsize;
    off = (msgrec->loc + st_offset) % mbsize;	/* file read offset */
    while (nread < size) {
	int len;

	len = size - nread;
	if (len > mbsize - off)
	    len = mbsize - off;
	if (lseek (fd, (off_t)(off + mb->msg_off + add_off), 0) < 0 ||
	    read (fd, buffer + nread, len) != len)
	    return (Unlock_return (fd, MB_READ_FAILED));

	nread += len;
	off = (off + len) % mbsize;
    }

    /* update the read id if needed */
    if (mb->flags & MB_MUST_READ) {
	si32 tmp = msgrec->id;

	if (lseek (fd, (off_t)READ_ID_OFF, 0) < 0 ||
	    BE_from_array_32 (&tmp, sizeof(mb_t)) < 0 ||
	    write (fd, (char *) &tmp, sizeof(mb_t)) != sizeof(mb_t))
	    return (Unlock_return (fd, MB_UPDATE_FAILED));
	mb->attr->read_id = tmp;
    }

    /* Increment the read pointer */
    mb->r_pt = msgrec->id + 1;

    if (ret_id != NULL)
	*ret_id = msgrec->id;

    return (Unlock_return (fd, msgsize));
}

/************************************************************************

Function Name: 	MB_close

Description: This function closes an open MB, "mbd", frees the descriptor,
	the MB structure and other allocated resources.

	This function returns MB_SUCCESS on success or MB_INVALID
	if the "mbd" argument is not a valid MB descriptor.

************************************************************************/

int
  _MB_close
  (
      int mbd			/* the MB descriptor */
) {
    Mb_struct *mb;
    int ret;

    /* get MB structure, lock the file and update the struct */
    ret = Get_mb_structure (mbd, EXCLUSIVE, &mb);
    if (ret != MB_SUCCESS)
	return (ret);

    close (mb->fd);
    if (mb->attr != NULL)
	free ((char *)mb->attr);
    if (mb->list != NULL)
	free ((char *)mb->list);
    if (mb->tbl_hd != NULL)
	free ((char *)mb->tbl_hd);
    if (mb->sit_buf != NULL)
	free ((char *)mb->sit_buf);
    free ((char *)mb);
    Mb_handle[mbd - MB_START_FD] = NULL;

    return (MB_SUCCESS);
}

/************************************************************************

Function Name: 	MB_stat

Description: This function returns the MB status structure. It returns
	MB_SUCCESS on success or a negative error number.

************************************************************************/

int 
_MB_stat (
    int mbd, 			/* MB descriptor */
    _MB_status *mb_st		/* buffer for returning the MB status */
)
{
    Mb_struct *mb;
    Mb_attribute *attr;
    int ret;

    /* check arguments */
    if (mb_st == NULL)
	return (MB_BAD_ARGUMENT);

    /* get MB structure, lock the file and update the struct */
    ret = Get_mb_structure (mbd, SHARED, &mb);
    if (ret != MB_SUCCESS)
	return (ret);

    /* release the lock */
    Release_lock (mb->fd);

    /* return the MB status */
    attr = mb->attr;
    mb_st->size = attr->size;
    mb_st->msg_size = attr->msg_size;
    mb_st->maxn_msgs = attr->maxn_msgs;
    mb_st->perm = attr->perm;
    mb_st->flags = mb->flags;
    mb_st->n_msgs = mb->n_msgs;
    if (mb->n_msgs > 0)
	mb_st->latest_id = 
	    mb->list [(mb->st_ind + mb->n_msgs - 1) %  mb->n_slots].id;
    else 
	mb_st->latest_id = 0;

    return (MB_SUCCESS);	
}

/************************************************************************

Function Name: 	MB_get_list

Description: This function fills in a list of message information started
	from "start_id".

	It returns the number of messages placed in the list. On failure
	it returns a negative error number.

************************************************************************/

int _MB_get_list 
(
    int mbd, 			/* MB descriptor */
    _MB_List *list,		/* buffer for returning the list */
    int nmsgs, 			/* number of messages to return */
    unsigned int start_id 	/* where to start */
)
{
    Mb_struct *mb;
    int ret;
    int n_msgs;
    int index;
    Msg_record *msgtbl;
    int cnt;
 
    /* check arguments (start_id will be checked later in Find_message_index) */
    if (nmsgs <= 0 || list == NULL)
	return (MB_BAD_ARGUMENT);

    /* get MB structure, lock the file and update the struct */
    ret = Get_mb_structure (mbd, SHARED, &mb);
    if (ret != MB_SUCCESS)
	return (ret);

    /* release the lock */
    Release_lock (mb->fd);

    /* find where to start */
    if ((ret = Find_message_index (mb, start_id, &index)) != MB_FOUND)
	return (ret);
    if (start_id == MB_LARGEST) {
	index = index - nmsgs + 1;
	if (index < 0)
	    index = 0;
    }

    /* fill in the message list */
    cnt = 0;
    msgtbl = mb->list;
    n_msgs = mb->tbl_hd->n_msgs;
    while (index < n_msgs) {
	int ind;

	ind = Recind_from_sitind (mb, index);
	list[cnt].size = Get_msg_size (mb, ind);

	list[cnt].id = msgtbl[ind].id;
/*        mb->r_pt = msgtbl[ind].id + 1; */
	cnt++;
	if (cnt >= nmsgs) break;
	index++;
    }

    return (cnt);
}

/************************************************************************

Function Name: MB_seek	

Description: This function moves the read pointer by "offset" messages
	started from the message "id". If there is no message, the
	function does nothing and return 0.

	The function returns the id of the message pointed to by 
	the read pointer on success or a negative error number on
	failure. If the MB is empty, it returns 0.

************************************************************************/

int _MB_seek 
(
    int mbd, 			/* MB descriptor */
    int offset,			/* amount of offset */
    unsigned int id 		/* where to start */
)
{
    Mb_struct *mb;
    int ret;
    int index, new_ind;
    int n_msgs;

    /* check arguments (id will be checked later in Find_message_index) */

    /* get MB structure, lock the file and update the struct */
    ret = Get_mb_structure (mbd, SHARED, &mb);
    if (ret != MB_SUCCESS)
	return (ret);

    /* release the lock */
    Release_lock (mb->fd);

    n_msgs = mb->n_msgs;
    if (n_msgs <= 0)
	return (0);

    /* find where to start */
    if ((ret = Find_message_index (mb, id, &index)) != MB_FOUND)
	return (ret);

    /* move the index  and set the new read pointer */
    new_ind = index + offset;
    if (new_ind < 0) 
	new_ind = 0;
    if (new_ind < n_msgs) 
	mb->r_pt = mb->list [Recind_from_sitind (mb, new_ind)].id;
    else {
	mb->r_pt = mb->list [Recind_from_sitind (mb, n_msgs - 1)].id + 1;
	new_ind = n_msgs;
    }

    ret = new_ind - index;
    if (ret < 0)
	ret = -ret;
    return (ret);
}

/************************************************************************

Function Name: 	MB_clear

Description: This function expires all messages in an open MB, "mbd".

	This function returns MB_SUCCESS on success or MB_INVALID
	if the "mbd" argument is not a valid MB descriptor.

************************************************************************/

int
  _MB_clear
  (
      int mbd			/* the MB descriptor */
) {
    Mb_struct *mb;
    Tbl_header *tbl_hd;
    int ret;

    /* get MB structure, lock the file and update the struct */
    ret = Get_mb_structure (mbd, EXCLUSIVE, &mb);
    if (ret != MB_SUCCESS)
	return (ret);
    tbl_hd = mb->tbl_hd;
    if ((mb->flags & MB_RDWR) == 0)
	return (Unlock_return (mb->fd, MB_READ_ONLY));

    if (mb->n_msgs <= 0)
        return (Unlock_return (mb->fd, MB_SUCCESS));

    /* move the read pointer to the end */
    if (mb->flags & MB_MUST_READ) {
	si32 tmp;
	mbmsg_t id;

	id = mb->list[(mb->st_ind + mb->n_msgs - 1) % mb->n_slots].id;
	tmp = id;
	if (lseek (mb->fd, (off_t)READ_ID_OFF, 0) < 0 ||
	    BE_from_array_32 (&tmp, sizeof(mb_t)) < 0 || 
	    write (mb->fd, (char *)&tmp, sizeof(mb_t)) != sizeof(mb_t))
	    return (Unlock_return (mb->fd, MB_UPDATE_FAILED));
	mb->attr->read_id = id;
    }

    while (mb->n_msgs > 0) {
	if (Remove_a_message (mb) == MB_FAILURE)
	    return (Unlock_return (mb->fd, MB_FAILURE));
    }

    /* relocate the free space pointer */
    tbl_hd = mb->tbl_hd;
    tbl_hd->free_loc = 0;    
    tbl_hd->dbloc_bits = 0;

    /* update the message tables in the MB file */
    if (Write_cache (mb, 0) != MB_SUCCESS)
	return (Unlock_return (mb->fd, MB_UPDATE_FAILED));

    if (mb->flags & MB_PARITY)
	mb->parity = Find_parity (mb);

    return (Unlock_return (mb->fd, MB_SUCCESS));
}

/************************************************************************

Function Name: 	Find_parity

Description: This function returns the parity value of the critical cache 
	space.

************************************************************************/

static int Find_parity (
    Mb_struct *mb
) {
    int p, size, *ip, i;

    p = 0;

    size = (sizeof (Mb_struct) - sizeof (int) - 2 * sizeof (mbmsg_t) - 
		sizeof (time_t)) / sizeof (int);	
			/* we ignore parity, r_pt and r_pg and time */
    ip = (int *)mb;
    for (i = 0; i < size; i++)
	p += ip [i];

    p += (int)mb->attr->update_cnt;	
    /* we exclude read_id and update_flag which can be updated by others
	and we don't update parity when they change */
    size = (sizeof (Mb_attribute) - MB_ID_OFF) / sizeof (int);
    ip = (int *)((char *)mb->attr + MB_ID_OFF);
    for (i = 0; i < size; i++)
	p += ip [i];

    if (mb->n_msgs > 0) {
	size = sizeof (Msg_record) / sizeof (int);
	ip = (int *)(mb->list + Recind_from_sitind (mb, mb->n_msgs - 1));
	for (i = 0; i < size; i++)
	    p += ip [i];
    }

    size = sizeof (Tbl_header) / sizeof (int);
    ip = (int *)mb->tbl_hd;
    for (i = 0; i < size; i++)
	p += ip [i];

    if (mb->seq_sit == FALSE) {
	size = mb->n_msgs * sizeof (sit_t) / sizeof (int);
	ip = (int *)mb->sit_buf + ((char *)mb->sit - (char *)mb->sit_buf) / 
		sizeof (int);		/* this guarantees alignment */
	for (i = 0; i < size; i++)
	    p += ip [i];
    }

    return (p);
}
