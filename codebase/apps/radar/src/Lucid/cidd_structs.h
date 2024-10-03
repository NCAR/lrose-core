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
#ifndef CIDD_STRUCTS_H
#define CIDD_STRUCTS_H
/**********************************************************************
 * CIDD_STRUCTS.H:  Misc Data structure defns for CIDD
 */

typedef struct {    /* Vertical Spacing Information */
  double min, cent, max;
} vert_spacing_t;

typedef struct {    /* Dynamic Data Status info */
  int  is_dynamic;
  time_t last_accessed;
  char stat_msg[TITLE_LENGTH];
  const char *status_fname;
} status_msg_t;

// typedef struct {  // Menu Bar - NOTE: Only 32 bits can be defined!
//     int num_menu_bar_cells;
//     u_int last_callback_value;

//     int loop_onoff_bit;     // Menu bar cell numbers
//     int winds_onoff_bit;    // 0 == Not used
//     int symprods_onoff_bit; 
//     int show_forecast_menu_bit;
//     int print_button_bit;
//     int help_button_bit;
//     int show_time_panel_bit;
//     int show_dpd_menu_bit;
//     int show_dpd_panel_bit;
//     int show_view_menu_bit; // 10
//     int show_xsect_panel_bit;
//     int show_grid_panel_bit;
//     int show_map_menu_bit;
//     int show_bookmark_menu_bit;
//     int reload_bit;
//     int set_draw_mode_bit;
//     int reset_bit;
//     int set_to_now_bit;
//     int show_prod_menu_bit;
//     int show_status_win_bit; // 20
//     int show_gen_time_win_bit;
//     int close_popups_bit;
//     int report_mode_bit;
//     int exit_button_bit;
//     int clone_button_bit;
//     int show_past_menu_bit;
//     int show_cmd_menu_bit;
//     int snapshot_bit;   
//     int landuse_onoff_bit;  // 29
//     int set_route_mode_bit; //30
//     int set_pick_mode_bit; //31
//     int zoom_back_bit;  // 32

// } menu_bar_t;

/**********************************************************************
 * CIDD_STRUCTS.H:  Misc Data structure defns for CIDD
 */


typedef struct { // Bookmarks
  const char *label;
  const char *url;
} bookmark_t;

typedef struct { // Draw-Export
  int  default_serial_no;
  double  default_valid_minutes;
  time_t	data_time;
  char *product_id_label;
  char *product_label_text;
  char *product_fmq_url;
  char *param_line;
} draw_export_info_t;

typedef struct {
  double lat;
  double lon;
} world_pt_t;

typedef struct { // Draw-Export
  int num_draw_products;
  int cur_draw_product;
  draw_export_info_t *dexport;
} draw_export_t;

typedef struct { // Time-List
  unsigned int num_entries;
  unsigned int num_alloc_entries;
  time_t *tim;
} time_list_t;

#endif
