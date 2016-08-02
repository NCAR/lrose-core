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
/***********************************************************************
 * CIDD_FUNC.H: Function prototypes 
 */

#ifdef H_WIN_PROC
void redraw_zoom_box();
void redraw_pan_line();
void redraw_rhi_line();
#endif

#ifndef H_WIN_PROC
extern void redraw_zoom_box();
extern void redraw_pan_line();
extern void redraw_rhi_line();
#endif

#ifndef GET_COLORS
extern int    combine_color_maps();
extern int    get_color_mapping();
#endif

#ifndef GET_X_DEF
extern float GetFloatDef();
extern double GetDoubleDef();
extern long GetLongDef();
extern char *GetStringDef();
#endif

#ifndef RD_MAIN
extern void process_args(); 
extern void init_xview();
extern void rd_h_msg();
extern void rd_v_msg();
#endif

#ifdef RD_MAIN
void process_args(); 
void init_xview();
void rd_h_msg();
void rd_v_msg();
#endif

#ifdef RD_INIT
int get_parameter_database();
void init_data_space();
void init_data_links();
void init_over_data_links();
void init_wind_data_links();
int init_product_info();
int init_contour_links();
int init_static_links();
void setup_colorscales();
void establish_shmem1();
void modify_xview_objects();
void init_field_menus();
#endif

#ifndef RD_INIT
extern int get_parameter_database();
extern void init_data_space();
extern void init_data_links();
extern void init_over_data_links();
extern void init_wind_data_links();
extern int init_product_info();
extern int init_contour_links();
extern int init_static_links();
extern void setup_colorscales();
extern void establish_shmem1();
extern void modify_xview_objects();
extern void init_field_menus();
#endif

#ifndef RD_TIMER
extern void start_timer();
extern void    timer_func();
extern void check_for_invalid_images();
#endif

#ifdef RD_TIMER
void start_timer();
void timer_func();
void check_for_invalid_images();
#endif

#ifndef RD_DATA
extern unsigned char *get_rhi_data();
extern unsigned char *get_cap_data();
extern Notify_value sig_io_handler();
#endif

#ifdef RD_FONT
Font    choose_font();
#endif

#ifndef RD_FONT
extern Font    choose_font();
#endif

#ifdef DATA_PU_PROC
void    set_field(int value);
#endif

#ifndef DATA_PU_PROC
extern void    set_field(int value);
#endif

#ifndef RD_GRAPH
extern double    compute_tick_interval();
extern double    compute_range();
#endif

extern int rle_image(u_char *in_data,u_char *out_data,u_char key,int size);
extern int rld_image(u_char *in_data,u_char *out_data,u_char key,int size);
extern u_char *RLDecode8(u_char *coded_data, u_int *nbytes_full);
extern u_char *RLEncode8(u_char *full_data, u_int nbytes_full, u_int key, u_int *nbytes_array);
