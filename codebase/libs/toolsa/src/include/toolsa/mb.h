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

Header: mb.h

Author: Z. jing

Date:	1/5/95

Description: Include file for the MB module.

*************************************************************************/

#ifndef MB_H
#define MB_H

#ifdef __cplusplus
 extern "C" {
#endif

/* System include files / Local include files */
#include <sys/types.h>

/* Constant definitions / Macro definitions / Type definitions */
#define MB_SUCCESS 0		/* general function return */
#define MB_FAILURE  -1		/* general function return */

#define MB_NAME_LEN	128	/* max name length including the host name */
#define MB_MACHINE_NAME_SIZE 24 /* max allowable host name size (including ':') */

/* special numbers for id argument */
#define MB_NEW_ID	(0xffffffff)
#define MB_LARGEST	(0xfffffffe)
#define MB_CURRENT	(0xfffffffd)
#define MB_SMALLEST	(0xfffffffc)
#define MB_LATEST	(0xfffffffb)

/* MB_open flags */
#define MB_MUST_READ	0x1000
#define MB_REPLACE	0x4000
#define MB_INIT		0x8000
#define MB_RDWR		0x10000
#define MB_SEQUENTIAL	0x20000
#define MB_PARITY	0x40000

#define MB_DEFAULT	0
#define MB_RANDOM	0

#define MB_NOT_FOUND		 0	/* specified message is not found */

/* error returns */
#define MB_ID_NOT_ACCEPTED	 -13
#define MB_NEED_ID		 -14
#define MB_READ_ONLY		 -15
#define MB_EXISTS		 -16
#define MB_FULL			 -17
#define MB_TOO_SMALL		 -18
#define MB_READ_FAILED		 -19

#define MB_TOO_MANY		 -20
#define MB_UPDATE_FAILED	 -21
#define MB_INVALID	 	 -22

#define MB_DUPLICATED		 -23
#define MB_CORRUPTED	 	 -24

#define MB_BAD_ARGUMENT		 -25
#define MB_LOCK_FAILED		 -26

#define MB_TOO_LARGE		 -27
#define MB_OPEN_FAILED		 -28

#define MB_ALLOCATE_FAILED	 -29
#define MB_NONMB_FILE		 -30

#define MB_PARITY_ERROR		 -31



/* MB messages structure */
typedef struct {
    unsigned int id;		/* message id */
    int size;			/* message size */
} _MB_List;

/* MB status structure */
typedef struct {
    int size;			/* size of the MB */
    int msg_size;		/* average message size */
    int maxn_msgs;		/* maximum number of messages */
    int perm;			/* MB access perm */
    int flags;			/* MB flags */
    int n_msgs;			/* number of messages in the MB */
    unsigned int latest_id;	/* latest message id */
} _MB_status;


/* External global variables / Non-static global variables / Static globals */

/* External functions / Internal global functions / Internal static functions */

/* external interface to MB routines */


int _MB_open (char *name, int flags, int perm, int msg_size, int maxn_msgs);

int _MB_get_list (int mbd, _MB_List *list, int nmsgs, unsigned int id);

int _MB_write (int mbd, char *msg, int length, unsigned int msg_id, 
	unsigned int *ret_id);

int _MB_read (int mbd, char *buffer, int buf_size, unsigned int msg_id, 
	unsigned int *ret_id);

int _MB_clear (int mbd);

int _MB_close (int mbd);

int _MB_stat (int mbd, _MB_status *mb_st_buf);

int _MB_seek (int mbd, int offset, unsigned int msg_id);

int _MB_read_part (int mbd, char *buffer, int offset, int length, 
	unsigned int msg_id, unsigned int *ret_id);

#ifdef __cplusplus
}
#endif

#endif
