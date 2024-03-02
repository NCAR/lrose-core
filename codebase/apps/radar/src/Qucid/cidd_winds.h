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

#ifndef CIDD_WINDS_H
#define CIDD_WINDS_H

/*************************************************************************
 * CIDD_WINDS.H : Definitions and control structures/variables 
 *
 */

/* Wind Vectors */

#define WIND_MODE_ON     0   /* Wind vectors on in each frame */
#define WIND_MODE_LAST   1   /* Wind vectors on only in the last frame */
#define WIND_MODE_STILL  2   /* Wind vectors on only in the last framef movie is off */

#define ARROWS 1           // Centered Arrow
#define TUFT 2             // Trailing Tuft
#define BARB 3             // Simple Wind Barb - N Hemisphere
#define VECTOR 4           // Arrow with base at data point
#define TICKVECTOR 5       // Arrow with base at data point with time ticks
#define LABELEDBARB 6      // Labeled Wind Barb N. Hemisphere 
#define METBARB 7          // Barb labeled like upper air charts, hemisphere correct 
#define BARB_SH 8          // S. Hemisphere Simple Barb
#define LABELEDBARB_SH 9   // S. Hemisphere Labeled Barb

class wind_data_t {

public:

  wind_data_t() {
    active = 0;
    scale = 0;
    marker_type = 0;      
    line_width = 0;
    units_scale_factor = 0;
    reference_speed = 0;
    wind_u = NULL;
    wind_v = NULL;
    wind_w = NULL;
    data_info = NULL;
    MEM_zero(color_name);
    units_label = NULL;
    color = NULL;
  }
  
  int active;       /* if True - draw wind features (vectors) */
  int scale;
  int marker_type;      
  int line_width;

  double units_scale_factor;
  double reference_speed;

  met_record_t *wind_u;
  met_record_t *wind_v;
  met_record_t *wind_w;
  
  char *data_info; /* Data source info string */
  char color_name[NAME_LENGTH];  /* Color name*/
  const char *units_label;  /* Color name*/
  
  Color_gc_t *color;

};

#endif
