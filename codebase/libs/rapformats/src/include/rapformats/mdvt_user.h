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
/*************************************************************************
 * MDVT_USER.H : Defines necessary for using MDVT format data and routines.
 *
 * F. Hage Dec 1993. NCAR, RAP.
 *
 */
#ifdef __cplusplus
 extern "C" {
#endif

#define MDVT_USER_H

#include "mdvt_data.h"

#define MDVT_SUCCESS 0
#define MDVT_FAILURE -1

#ifndef MDVT_USER_C
 
extern int MDVT_load_data_header( FILE *infile, master_header_t  *m_head);
extern int MDVT_load_field_header( FILE *infile, field_header_t  *f_head, int  field_num);
extern float * MDVT_get_plane( FILE *infile, field_header_t  *f_head, int   plane_num);
extern float * MDVT_get_volume( FILE *infile, field_header_t  *f_head);
extern unsigned char * MDVT_get_char_plane( FILE *infile,
                field_header_t  *f_head,
                int    plane_num,
                double *scale, double *bias,
                unsigned char *bad_data, unsigned char *missing_data);

extern unsigned char * MDVT_get_char_volume( FILE *infile,
                field_header_t  *f_head,
                double *scale, double *bias,
                unsigned char *bad_data, unsigned char *missing_data);

extern int MDVT_find_data_sets( char    **top_dir, int     num_dirs, char    *name_list[],
            char    *match, /* the file name must contain this string to be valid */
            int len,        /* minimum length of the file names wanted in bytes */
            int min,        /* minimum integer the files will atoi() to */
            int max,         /* max number of dirs to find */
            int list_size); /* Size of the name list buffers */


extern int MDVT_find_forecast_files( char    *dir, char    *name_list[],
            char    *match, /* the file name must contain this string to be valid */
            int len,        /* length of the file names wanted in bytes */
            int min,        /* minimum integer the files will atoi() to */
            int max,        /* max number of files to find */
            int list_size); /* Size of the name list buffers */



extern long MDVT_name_to_utime(char *dir_name,char *f_name);

extern FILE *MDVT_open_data_time(long dtime,int num_data_dirs,char **data_dir,char *data_set_suffix,char *file_suffix);

#endif

#ifdef __cplusplus
}
#endif
