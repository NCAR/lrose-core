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

/************************************************************************

Header: confread.h

Author: Z. jing

Date:	8/16/94

Description: Header file for the configuration read module (CFRD).

*************************************************************************/

#ifndef CFRDREAD_H
#define CFRDREAD_H

#include <stdio.h>


/* return values of confread functions */
#define CFRD_SUCCESS 0
#define CFRD_FAILURE  -1

/* error number returned by CFRD_get_line */
enum {CFRD_LINE_TOO_LONG = 1, CFRD_NON_ASCII, CFRD_BAD_BACK_SLASH, CFRD_EOF, CFRD_BAD_ENV};

enum {CFRD_CHAR, CFRD_SHORT, CFRD_INT,
      CFRD_LONG, CFRD_FLOAT, CFRD_DOUBLE};

#define CFRD_SUCCESS		0
#define CFRD_KEY_NOT_FOUND	-1
#define CFRD_TYPE_ERROR		-2

#define LINE_BUF_SIZE 		256	/* maximum line size processed */

void CFRD_close (void);
int CFRD_open (char *file_name, void (*err_func)(char *msg));
int CFRD_find_token (char *string, int n, int *off);
int CFRD_read_check (char **keyword);
int CFRD_get_next_line (char *key_word,	char **line_buf);	
char *CFRD_get_line (FILE *fl, int *l_num, int *err);

int CFRD_read_next_line (char *key_word, int max_len, char **line_buf);	

int CFRD_read_array (char *key_word, int type, int num, void *array, int *err);


#endif
#ifdef __cplusplus             
}
#endif
