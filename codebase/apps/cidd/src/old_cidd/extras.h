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
 * EXTRAS.H : Definitions and control struictures/variables for "extra"
 *    features for the CIDD display
 *
 */

/* Reference markers */
#define RANGE_BIT        1
#define AZMITH_BIT       2
#define EARTHH_BIT       4

/* Contours */
#define MAX_CONT_LEVELS 64

/* Wind Vectors */
#define WIND_MODE_ON     0   /* Wind vectors on in each frame */
#define WIND_MODE_LAST   1   /* Wind vectors on only in the last frame */
#define WIND_MODE_STILL  2   /* Wind vectors on only in the last framef movie is off */

typedef struct  {
    int field;      /* global field number to contour */

    int active;
    int labels_on;

    int num_levels;
    double  min;
    double  max;
    double  interval;

    Color_gc_t *color;
    char color_name[NAME_LENGTH];  /* Color name*/

}contour_info_t ;

typedef struct  {
    int active;       /* if True - draw wind features (vectors) */
    int    scale;
    double    wind_ms;    /* 10km on grid = X m/sec */    
    met_record_t    *wind_u;
    met_record_t    *wind_v;
    met_record_t    *wind_w;

    char data_info[NAME_LENGTH];   /* Data source info string */
    char color_name[NAME_LENGTH];  /* Color name*/

    Color_gc_t *color;

} wind_data_t;

typedef struct  {  
    int    products_on;         /* Flag to turn products on/off globally */
    int    product_time_select;  /* Mode flag: Set to different methods of product time selection */
    int    product_time_width; /* Seconds wide the time window for products is */
    int    prod_line_width;    /* How wide to make the lines */
    int    prod_font_num;    

    Color_gc_t *prds_color[PRDS_NUM_COLORS];

} prod_info_t;

typedef struct {
    int    range;          /* Flag to indicate range rings are desired */
    int    azmiths;        /* Flag to indicate azmith lines are desired */
    int    earth;          /* Flag to indicate earth reference lines are desired */
    int    wind_vectors;   /* Flag to turn on wind vectors */
    int    set_field_mode;  /* 1 = set contour field parameters, 2 = set overlay field params */

    int    cur_overlay_layer;    /* Which layer we are setting 0 - NUM_GRID_LAYERS -1 */
    int    cur_contour_layer;    /* Which layer we are setting 0 - NUM_CONT_LAYERS -1 */
    int    cur_map_overlay;      /* Which map overlay is being manipulated */
    int    cur_wind_overlay;     /* Which wind set is being manipulated */   

    int    overlay_field_on[NUM_GRID_LAYERS];  /* Flag to turn on overlaid grid */
    int    overlay_field[NUM_GRID_LAYERS];  /* Which field to render */

    int    num_wind_sets;    /* */

    int    wind_mode;  /* wind display mode */
    double wind_time_scale_interval;    /* minutes interval-  wind vector scale */

    wind_data_t    *wind;    /* Pointer to Array of wind set info */

    contour_info_t cont[NUM_CONT_LAYERS];

    prod_info_t prod;

    double overlay_min[NUM_GRID_LAYERS]; /* Clip all values in grid below this */
    double overlay_max[NUM_GRID_LAYERS]; /* Clip all values in grid above this */

    Color_gc_t    *range_color;
    Color_gc_t    *earth_reference_color;

    Color_gc_t   *foreground_color;
    Color_gc_t   *background_color;

} extras_t;


