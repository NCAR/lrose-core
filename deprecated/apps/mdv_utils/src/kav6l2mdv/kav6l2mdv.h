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
/*******************************************************************************
 * KAV6L2MDV.H : Defines & includes for the KAV6L2MDV program
 */

#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <strings.h>
#include <math.h>
#include <malloc.h>
#include <dirent.h>
#include <sys/param.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#include <toolsa/utim.h>
#include <toolsa/port.h>
#include <toolsa/pmu.h>
#include <toolsa/umisc.h>
#include <mdv/mdv_user.h>
#include <rapformats/kavouras_io.h>

#define ZERO_STRUCT(p)      bzero((char *)(p), sizeof (*(p)))

#define TRUE 1
#define FALSE 0

#define CHECK_SECS  15   /* Progrem checks every CHECK_SECS for new data in cont_mode */
#define QUESCENT_SECS 2  /* Ignore files that have been changed within this time to avoid incomplete files */

#define KAV_DX 2.0   /* 2.0 km grid spacing */
#define KAV_DY 2.0
#define KAV_X_START (-128.0 * KAV_DX)
#define KAV_Y_START (-120.5 * KAV_DY)

/******************************************************************************
 * GLOBAL DATA DECLERATIONs
 */

struct    Global_data {
    int    debug;        /* Normal debugging flag  */
    int    field;        /* Which field to output  -1 = all -> default*/
    int    plane;        /* Which Plane to output  -1 = all -> default*/
    int    x1,y1;        /* starting grid coordinates */
    int    x2,y2;        /* ending grid coordinates */
    int    compress;     /* Set to 1 to compress data planes */
    int    cont_mode;    /* Set to 1 to monitor a directory for new data files*/

    int    nfiles;

    double origin_lat,origin_lon;

    char  **f_name;     /* Array of names - Pulled from arg list */

    char *output_dir;  /* Top level directory to leave data in */
    char *suffix;      /* Data output file suffix */
    char *input_dir;   /* Directory to examine for new data files*/
    char *match_string;/* Data files must contain this string to be converted */
    char *app_name;    /* Application name */
    char *app_instance;/* Application instance */


    dcmp6h_header_t *k_head;  /* kavouras data header */


    MDV_master_header_t mh;
    MDV_field_header_t  fh;

    unsigned char k_image[NUM_6L_COLS * NUM_6L_ROWS];   /* Space for Kavouras data image */
};

#ifdef KAV6L2MDV_MAIN
struct    Global_data gd;
#endif

/************************* External reference to global data structure ********/

#ifndef    KAV6L2MDV_MAIN
extern    struct    Global_data    gd;
#endif
