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
#ifndef CIDD_MOVIES_H
#define CIDD_MOVIES_H

class QPixmap;

/*************************************************************************
 * CIDD_MOVIES.H : Definitions and control structures/variables for "movie"
 *    features for the CIDD display
 *
 */
typedef struct {
    QPixmap *h_pdev;       /* A memory pixmap for movie frames */
    QPixmap *v_pdev;       /* A memory pixmap for movie frames */
    time_t time_start;  /* Time at starting point of image */
    time_t time_end;    /* Time at ending point of image */
    time_t time_mid;    /* Time at mid point of image */
    int    redraw_horiz;       /* 1 = rerender */
    int    redraw_vert;        /* 1 = rerender */
    char   fname[NAME_LENGTH];   /* file name for this image */
    char   vfname[NAME_LENGTH];  /* file name for this image */
} movie_frame_t;
     

typedef struct {
    int    active;                /*  Set to 1 when movie panel is open */
    int    mode;                /* 0= realtime mode, 1= archive mode, 2 = elevation movie */
    int    magnify_mode;         /* 1 = Magnify mode  turned on - When setting forecast mode. */
    int    sweep_on;             /*  1 = Sweep instead of loop */
    int    sweep_dir;            /*  Either +1 (forward)  or -1 (backward) */
    int    display_time_msec;    /* TIme each movie frame should be displayed - msecs */
    int    num_frames;            /* Total number of frames in loop */
    int    start_frame;        /* The frame to start with */
    int    end_frame;            /* The frame to end with */
    int    cur_frame;            /* the currently visible frame number */
    int    last_frame;            /* The last frame visible */
    int    movie_on;           /* 0 = off, Other: movie is on */
    int    delay;                /* Number of frame ticks to delay at end of loop */
    int    round_to_seconds;    /* The number of seconds to round data times to */
    double    magnify_factor;   /* Amount to magnify time scales when switching into forecast mode */
    double    time_interval_mins;   /* in minutes */
    double    mr_stretch_factor;    /* Factor to mult time_interval by for most recent data */

    time_t    start_time;           /* Time of the first frame */
    time_t    demo_time;            /* First frame in demo mode */
    int demo_mode;                  /* Starts up in demo mode */

    movie_frame_t    frame[MAX_FRAMES];    /* info about each frame */

    int reset_frames;       /* Reset valid flags on frames when frame index updates */
    int climo_mode;         /* Climotology mode - restricts the time span of data requests. */

    double forecast_interval; /* Interval to display menu options into the future in hours */
    double past_interval;     /* Interval to display menu options into the past in hours */
    double    frame_span;     /* minutes spanning one movie frame - Used for climo mode  */

} movie_control_t;

#endif
