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

	Header file for the shared memory message queue module (MMQ).

	File: mmq.h

*******************************************************************/

#ifndef MMQ_H
#define MMQ_H


#define MMQ_READ  0
#define MMQ_WRITE  1
#define MMQ_FLUSH  2

#define MMQ_SUCCESS 0

#define MMQ_ON    	1
#define MMQ_OFF   	0

#define MMQ_WOULD_BLOCK 	0

/* error returns */
#define MMQ_TOO_MANY_OPENED	 -80
#define MMQ_SIZE_TOO_SMALL	 -81
#define MMQ_MALLOC_FAILED	 -82
#define MMQ_SHMGET_FAILED	 -83

#define MMQ_SHMAT_FAILED	 -84
#define MMQ_TOO_MANY_CLIENTS	 -85
#define MMQ_BAD_DESCRIPTOR	 -86
#define MMQ_BAD_ACCESS		 -87

#define MMQ_INTERNAL_ERROR	 -88
#define MMQ_NO_BUFFER_RETURNED	 -89
#define MMQ_CANNOT_WRITE	 -90
#define MMQ_LENGTH_ERROR	 -91

#define MMQ_KEY_NEGATIVE	 -92
#define MMQ_ZERO_MESSAGE 	 -93
#define MMQ_MSG_TOO_LARGE	 -94

/* public functions */

#ifndef RKC

int MMQ_open (int type, int key);
int MMQ_read (int md, char **buf);
int MMQ_write (int md, char *buf, int length);
int MMQ_close (int md);
void MMQ_msg (int sw);
void MMQ_change_size_unit (int unit);

int MMQ_preview (int md, int n_msg, char **buf, int *length);
int MMQ_request (int md, int length, char **buffer);
int MMQ_send (int md, int length);

void MMQ_old ();

#else

void MMQ_msg ();
void MMQ_change_size_unit ();

#endif


#endif
#ifdef __cplusplus             
}
#endif
