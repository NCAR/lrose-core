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
/*****************************************************************

	Header file defining the file message queue module (FMQ).

	File: fmq.h

        Jing

*****************************************************************/

#ifndef JFMQ_H
#define JFMQ_H

#ifndef RKC

int JFMQ_open (char *name, int size, int mode);
int JFMQ_read (int fd, char *buffer, int buf_size);
int JFMQ_write (int fd, char *message, int length);
int JFMQ_close (int fd);
int JFMQ_clear (int fd);
void JFMQ_messages (int sw);
void JFMQ_lock (int sw);
int JFMQ_reset_rpt (int fd, int n_steps);

#else
void JFMQ_messages();
void JFMQ_lock ();

#endif


#define JFMQ_ON   1
#define JFMQ_OFF  0

#define JFMQ_TRUE   1
#define JFMQ_FALSE  0

#define JFMQ_FAILURE  -1
#define JFMQ_SUCCESS  0

#define JFMQ_CREATE   0x1000   		/* enable FMQ creation */
#define JFMQ_SIZE_CHECK_OFF   0x2000   	/* disable size check */

/* error code */

#define JFMQ_EMPTY  		-62
#define JFMQ_FULL  		-66

#define JFMQ_SIZE_TOO_SMALL	-50
#define JFMQ_READ_HD_FAILED  	-51
#define JFMQ_SIZE_INCORRECT	-52
#define JFMQ_PERMISSION_DENIED	-53
#define JFMQ_LOCK_FAILED	-54
#define JFMQ_GENERATION_FAILED	-55
#define JFMQ_WRITE_HD_FAILED	-56

#define JFMQ_TOO_MANY_OPEN_FMQ	-57
#define JFMQ_MEM_ALLOC_FAILED	-58
#define JFMQ_OPEN_FAILED	-59

#define JFMQ_INVALID_FD		-60
#define JFMQ_READ_LEN_FAILED	-61

#define JFMQ_BUFFER_TOO_SMALL	-63
#define JFMQ_DATA_READ_FAILED	-64

#define JFMQ_MSG_TOO_LARGE	-65
#define JFMQ_DATA_WRITE_FAILED	-67

#define JFMQ_BAD_HD_INFOR        -68
#define JFMQ_NULL_NAME  	-69
#define JFMQ_BAD_MODE		-70

#define JFMQ_READ_INTERRUPTED	-71
#define JFMQ_WRITE_INTERRUPTED	-72

#endif
#ifdef __cplusplus             
}
#endif
