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
 * CIDD_LEGEND.H : Definitions and control variables for legend
 *    features for the CIDD display
 *
 */

#ifndef CIDD_LEGEND_H
#define CIDD_LEGEND_H

/* Reference markers */
#define RANGE_BIT       1
#define AZIMUTH_BIT      2
#define EARTH_BIT       4

#define NUM_TICK_COLORS 6

typedef struct {
    int    range;          /* Flag to indicate range rings are desired */
    int    azimuths;        /* Flag to indicate azimuth lines are desired */
    int    earth;          /* Flag to indicate earth reference lines are desired */

    Color_gc_t   *foreground_color;
    Color_gc_t   *background_color;
    Color_gc_t   *out_of_range_color;

    Color_gc_t    *margin_color;     // Background color of the margins
    Color_gc_t    *range_ring_color; // for x-section routes 
    Color_gc_t    *route_path_color; // for x-section routes 
    Color_gc_t    *time_axis_color;  // Color of the time axis text, etc
    Color_gc_t    *time_frame_color; // Color of frame temporal limit lines
    Color_gc_t    *epoch_indicator_color;   // Color of epoch indicators 
    Color_gc_t    *now_time_color;   // Color of the Now Line 

    Color_gc_t    *time_tick_color[NUM_TICK_COLORS];   // Colors of the data time ticks 


    Color_gc_t    *height_axis_color;  // Color of the height axis text, etc
    Color_gc_t    *height_indicator_color; // Color of the height indicator

    Color_gc_t    *latest_click_mark_color; // Color of the mark for latest click
    Color_gc_t    *latest_client_mark_color; // Color of the mark for latest client location

} legend_t;


#endif
