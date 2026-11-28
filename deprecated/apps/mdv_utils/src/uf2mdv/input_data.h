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
#ifndef INPUT_DATA_H
#define INPUT_DATA_H

typedef struct
{
    int wanted;       /* 1 if data is wanted */
    int prf;          /* 1 if prf is in range */
    int range;        /* 1 if gates are in range */
    int time;         /* 1 if time is in range */
    int file;         /* file # (0,1,..) */
    int record;       /* record # in sequence of records (0,1,..)*/
    int sweep;        /* sweep number from data */
    int volume;       /* volume number */
    int az_index;     /* azimuth index from grid */
    int elev_index;   /* elevation index from grid */
    float azimuth;    /* azimuth value */
    float elevation;  /* elevation value */
    int sticky_azimuth; /* 1 if this is a sticky azimuth*/
} input_data_state;

typedef struct
{
    input_data_state analyzed_state; /* filled when analyzed */
    input_data_state init_state;     /* filled as best can from data */
    int nbytes;             /* number of bytes of data */
    char *data;             /* the data */
} input_data_t;

extern void input_data_init(void);
extern void input_data_rewind(void);
extern input_data_t *input_data_get_next(void);
extern void input_data_free(input_data_t *d);
extern void input_browse_data(input_data_t *uf);
extern int input_data_num_records(void);

#endif
