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

class vert_spacing_t {    /* Vertical Spacing Information */
public:
  vert_spacing_t() {
    min = 0.0;
    cent = 0.0;
    max = 0.0;
  }
  double min;
  double cent;
  double max;
};

class status_msg_t {    /* Dynamic Data Status info */
public:
  status_msg_t() {
    is_dynamic = 0;
    last_accessed = 0;
    status_fname = NULL;
  }
  int is_dynamic;
  time_t last_accessed;
  string stat_msg;
  const char *status_fname;
};

class bookmark_t { // Bookmarks
public:
  bookmark_t() {
    label = NULL;
    url = NULL;
  }
  const char *label;
  const char *url;
};

typedef struct {
  double lat;
  double lon;
} world_pt_t;

class draw_export_info_t { // Draw-Export
public:
  draw_export_info_t() {
    default_serial_no = 0;
    default_valid_minutes = 0.0;
    data_time = 0;
    product_id_label = NULL;
    product_label_text = NULL;
    product_fmq_url = NULL;
    param_line = NULL;
  }
  int default_serial_no;
  double default_valid_minutes;
  time_t data_time;
  char *product_id_label;
  char *product_label_text;
  char *product_fmq_url;
  char *param_line;
};

class draw_export_t { // Draw-Export
public:
  draw_export_t() {
    num_draw_products = 0;
    cur_draw_product = 0;
    dexport = NULL;
  }
  int num_draw_products;
  int cur_draw_product;
  draw_export_info_t *dexport;
};

class time_list_t { // Time-List
public:
  time_list_t() {
    num_entries = 0;
    num_alloc_entries = 0;
    tim = NULL;
  }
  unsigned int num_entries;
  unsigned int num_alloc_entries;
  time_t *tim;
};

#endif
