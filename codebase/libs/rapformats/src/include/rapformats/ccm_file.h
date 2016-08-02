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
 * CCM.H Structure & definitions for ccm format files 
 *
 * F. Hage May 1991.
 * Keyword: CCM File Header Structure 
 */
#ifdef __cplusplus
 extern "C" {
#endif

#define NUM_FIELDS 64
#define NUM_HEADER_INTS 65
#define NUM_HEADER_FLOATS 128
#define CCM_MISSING_DATA -32768

#ifdef GET_CCM
int    get_ccm_header(/* FILE *infile, struct ccm_header *head */);
unsigned short *get_data_plane(/*int field,int plane, struct ccm_header c_head, FILE *file */);
void swap_ccm_header_bytes(/* struct ccm_header *header */);
void print_ccm_header(/* FILE *outfile,struct ccm_header *header */);
#endif

#ifndef GET_CCM
extern int    get_ccm_header(/* FILE *infile, struct ccm_header *head */);
extern unsigned short *get_data_plane(/*int field,int plane, struct ccm_header c_head, FILE *file */);
extern void swap_ccm_header_bytes(/* struct ccm_header *header */);
extern void print_ccm_header(/* FILE *outfile,struct ccm_header *header */);
#endif

struct    grid_scale_bias {
    float    scale;
    float    bias;
};

struct ccm_header {    
    int        pad1;
    int        nx;
    int        ny;
    int        nz;
    int        num_fields;
    int        bad_data_value;
    int        time_begin;
    int        time_end;
    int        time_cent;
    int        i_not_used[56];

    float    dx;        /* grid dimensions (KM) */
    float    dy;
    float    dz;
    float    min_x;    /* Grid Starting - COordinates - KM */
    float    min_y;
    float    min_z;
    float    max_x;    /* Grid Ending - COordinates - KM */
    float    max_y;
    float    max_z;
    float    origin_lat;
    float    origin_lon;
    float    origin_alt;        /* Km MSL */
    float    north_angle;    /* Angle of the grid relative to true north */
    float    r_not_used[37];

    struct  grid_scale_bias gsb[39];

    char    var_name[64][32];    /* Field Names */

    char    var_units[64][32];    /* Field Units */
    int        pad2;
};

#ifdef __cplusplus
}
#endif
