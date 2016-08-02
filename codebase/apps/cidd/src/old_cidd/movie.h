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
 * MOVIE.H : Definitions and control struictures/variables for "movie"
 *    features for the CIDD display
 *
 */
typedef struct {
    Pixmap    h_xid;            /* A memory pixmap for movie frames */
    Pixmap    v_xid;            /* A memory pixmap for movie frames */
    UTIMstruct    time_start;    /* Time at starting point of image */
    UTIMstruct    time_end;    /* Time at ending point of image */
    UTIMstruct time_mid;    /* Time at mid point of image */
    int    redraw_horiz;         /* 1 = rerender */
    int    redraw_vert;        /* 1 = rerender */
    char    fname[NAME_LENGTH];        /* file name for this image */
} movie_frame_t;
     

typedef struct {
    int    active;                /* ??? Doesn't seem to change with looping */
    int    mode;                /* 0= realtime mode, 1= archive mode, 2 = elevation movie */
    int    display_time_msec;    /* TIme each movie frame should be displayed - msecs */
    int    num_frames;            /* Total number of frames in loop */
    int    start_frame;        /* The frame to start with */
    int    end_frame;            /* The frame to end with */
    int    cur_frame;            /* the currently visible frame number */
    int    last_frame;            /* The last frame visible */
    int    num_pixmaps;        /* Number of pixmap sets for movie frame storage */
    int    first_index;        /* The index of the first active frame */
    int    cur_index;
    int    movie_on;           /* 0 = off, Other: movie is on */
    int    delay;                /* Number of frame ticks to delay at end of loop */
    int    round_to_seconds;    /* The number of seconds to round data times to */
    double    time_interval;    /* in minutes */
    double    mr_stretch_factor;    /* Factor to mult time_interval by for most recent data */

    UTIMstruct    start_time;            /* Time of the first frame */

    movie_frame_t    frame[MAX_FRAMES];    /* info about each frame */
    unsigned char key;        /* key value for run length encoding */
    int reset_frames;       /* Reset valid flags on frames when frame index updates */
    double forecast_interval; /* Time to look into the future in minutes */
} movie_control_t;

