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
 * CIDD_LAYERS.H : Definitions and control structures/variables
 */

#ifndef CIDD_LAYERS_H
#define CIDD_LAYERS_H

#include "Cterrain_P.hh" // TDRP Definitions
#include "Croutes_P.hh"
#include "cidd_winds.h"
#include "cidd_contours.h"
#include "cidd_products.h"

typedef struct  {
    int terrain_active;       /* if True - draw terrain features */
    int landuse_active;       /* if True - draw landuse features */
    met_record_t    *terr;          // terrain
    met_record_t    *land_use;

    Color_gc_t *color1;  // skin color
    Color_gc_t *color2;  // core color

    Cterrain_P *_P; // TDRP PARAMS - See paramdef.Cidd_terrain

} earth_data_t;

typedef struct {  // Route track
    int num_segments;
    double total_length; // Distance in KM of all segments
    int x_pos[MAX_ROUTE_SEGMENTS]; // pixel coords
    int y_pos[MAX_ROUTE_SEGMENTS];
    double x_world[MAX_ROUTE_SEGMENTS];   // proj coords
    double y_world[MAX_ROUTE_SEGMENTS];
    double lat[MAX_ROUTE_SEGMENTS];   // world coords
    double lon[MAX_ROUTE_SEGMENTS];
    double seg_length[MAX_ROUTE_SEGMENTS];
    char navaid_id[MAX_ROUTE_SEGMENTS][16];
    char route_label[64];
} route_track_t;

typedef struct {
    int has_params;      // 1 =  Parameters exist in configuration file
    int active_route;    // which route is active.
    int num_predef_routes;
    route_track_t *route;// Predefined routes

    met_record_t *u_wind;
    met_record_t *v_wind;
    met_record_t *turb;
    met_record_t *icing;

    Croutes_P *_P;  // TDRP Params - See paramdef.Cidd_routes

} route_data_t; 

typedef struct {
    int    wind_vectors;   /* Flag to turn on/off wind vectors as a whole */
    int    init_state_wind_vectors;  /* starting state of wind_vectors */
    int    set_field_mode;  /* 1 = set contour field parameters, 2 = set overlay field params */

    int    cur_overlay_layer;    /* Which layer we are setting 0 - NUM_GRID_LAYERS -1 */
    int    cur_contour_layer;    /* Which layer we are setting 0 - NUM_CONT_LAYERS -1 */
    int    cur_map_overlay;      /* Which map overlay is being manipulated */
    int    cur_wind_overlay;     /* Which wind set is being manipulated */   

    int	   layer_legends_on;     // Control variables for plotting legend/Titles
    int	   cont_legends_on;
    int	   wind_legends_on;

    int    contour_line_width;
    int    smooth_contours;     // Apply smoothing before contouring ;  0-2 are valid values 
    int    use_alt_contours;    // 1 =  Use the Alternate Contouring routines 
    int    add_noise;           // Add 1 part in 250 of noise to contours 1 == true
    int    map_bad_to_min_value; // 1 == true
    int    map_missing_to_min_value; // 1 == true

    int    overlay_field_on[NUM_GRID_LAYERS];  /* Flag to turn on overlaid grid */
    int    overlay_field[NUM_GRID_LAYERS];  /* Which field to render */

    int    num_wind_sets;    /* */
    int    wind_mode;  /* wind display mode */
	int    wind_scaler; // Time displayed is wind_scaler * wind_time_scale_interval
    double wind_time_scale_interval; /* minutes interval-  wind vector scale */

    wind_data_t    *wind;    /* Pointer to Array of wind set info */

    route_data_t route_wind;  // Route winds data and  info

    earth_data_t earth;         // Terrain/ Land use data and  info

    contour_info_t cont[NUM_CONT_LAYERS];

    prod_info_t prod;

    double overlay_min[NUM_GRID_LAYERS]; /* Clip all values in grid below this */
    double overlay_max[NUM_GRID_LAYERS]; /* Clip all values in grid above this */

    Color_gc_t    *missing_data_color;  // Used for Displaying missing data
    Color_gc_t    *bad_data_color;      // Used for Displaying bad  data

    double special_contour_value;      // which value to draw wider 

} layers_t;


#endif
