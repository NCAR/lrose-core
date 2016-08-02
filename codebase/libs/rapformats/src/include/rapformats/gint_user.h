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
#ifndef GINT_USER_WAS_INCLUDED
#define GINT_USER_WAS_INCLUDED
#ifdef __cplusplus
 extern "C" {
#endif
/******************************************************************
    the header file for gint_user.c
    File: gint_user.h
    Date: Sept. 11, 1991
    Notes:
******************************************************************/

#include <stdio.h>
#include "v_data.h"

#define SHM_KEY  4097
 
struct tvolume_header {
    Volume_header *vh;
    Field_infor   *fi;
    Altitude_infor *ai;
    Location_infor *li;
};

typedef struct tvolume_header Tvolume_header;

#ifdef GINT_USER
int GINT_get_header(Tvolume_header *hd,  FILE *file);
unsigned char *GINT_get_plane(int field, int cappi_ind,  FILE *file, Tvolume_header *hd);
int GINT_run_length_encode_byte(int len, unsigned char *stri,unsigned char *stro);
int GINT_run_length_decode_byte( int len, unsigned char *stri, unsigned char *stro);
int GINT_run_length_encode(int len, unsigned char *stri, unsigned char *stro,unsigned char mask);
int GINT_run_length_decode(int len, unsigned char *stri,unsigned char *stro);
int GINT_put_header(Tvolume_header *hd, FILE *file);
int GINT_put_plane( unsigned char *buf, int field, int cappi_ind, FILE *file, Tvolume_header *hd);
int GINT_free_header(Tvolume_header *hd);
void GINT_print_header(Tvolume_header *hd, FILE *outfile);
#else
extern int GINT_get_header(Tvolume_header *hd,  FILE *file);
extern unsigned char *GINT_get_plane(int field, int cappi_ind,  FILE *file, Tvolume_header *hd);
extern int GINT_run_length_encode_byte(int len, unsigned char *stri,unsigned char *stro);
extern int GINT_run_length_decode_byte( int len, unsigned char *stri, unsigned char *stro);
extern int GINT_run_length_encode(int len, unsigned char *stri, unsigned char *stro,unsigned char mask);
extern int GINT_run_length_decode(int len, unsigned char *stri,unsigned char *stro);
extern int GINT_put_header(Tvolume_header *hd, FILE *file);
extern int GINT_put_plane( unsigned char *buf, int field, int cappi_ind, FILE *file, Tvolume_header *hd);
extern int GINT_free_header(Tvolume_header *hd);
extern void GINT_print_header(Tvolume_header *hd, FILE *outfile);
#endif
#endif

#ifdef __cplusplus
}
#endif
