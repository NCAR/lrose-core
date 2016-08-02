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

/*******************************************************************
 * titan/file_io.h: main header file for titan file utility routines
 ******************************************************************/

#ifndef titan_file_io_h
#define titan_file_io_h

#ifdef __cplusplus
extern "C" {
#endif

#ifndef R_FILE_LABEL_LEN
#define R_FILE_LABEL_LEN 40
#endif

#ifndef R_LABEL_LEN
#define R_LABEL_LEN 40
#endif
  
#define RF_FILE_NAME_LEN 80

#define R_SUCCESS 0
#define R_FAILURE -1

#include <stdio.h>
#include <sys/stat.h>
#include <toolsa/umisc.h>

/*
 * function prototypes
 */

extern int Rf_file_uncompress(char *file_path);

extern FILE *Rf_fopen_uncompress(char *filename, const char *type);

extern int RfReadFileLabel(const char *file_name,
			   char **file_label);

extern int Rf_stat_uncompress(const char *path, struct stat *buf);

#ifdef __cplusplus
}
#endif

#endif
