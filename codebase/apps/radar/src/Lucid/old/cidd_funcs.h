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
 * CIDD_FUNCS.H: Function prototypes 
 */

#ifndef CIDD_FUNCS_H
#define CIDD_FUNCS_H

#include <QObject>

#include "cidd_windows.h"
#include "cidd_maps.h"
#include "Params.hh"

#ifndef CIDD_COORDS
extern void get_bounding_box(double &min_lat, double &max_lat,
                      double &min_lon, double &max_lon);
extern void pixel_to_disp_proj( margin_t *margin, int pix_x, int pix_y,
        double *km_x,   /* RETURN */
        double *km_y);  /* RETURN */
extern void disp_proj_to_pixel( margin_t *margin, double km_x, double km_y,
        int    *pix_x,   /* RETURN */
        int    *pix_y);  /* RETURN */
extern void pixel_to_lonlat( margin_t *margin, int pix_x, int pix_y,
        double *lon,    /* RETURN */
        double *lat);   /* RETURN */
extern void lonlat_to_pixel( margin_t    *margin, double    lon, double    lat,
        int    *pix_x,    /* RETURN */
        int    *pix_y);   /* RETURN */ 
extern void pixel_to_grid( MetRecord *mr, margin_t *margin, int pix_x, int pix_y,
        int *grid_x,    /* RETURN */
        int *grid_y);   /* RETURN */
extern void disp_proj_to_grid( MetRecord *mr, double km_x, double km_y,
        int    *grid_x,    /* RETURN */
        int    *grid_y);   /* RETURN */
extern void grid_to_disp_proj( MetRecord *mr, int grid_x, int grid_y,
        double    *km_x,    /* RETURN */
        double    *km_y);   /* RETURN */ 
extern void grid_to_disp_proj_v( MetRecord *mr, int    grid_x, int    grid_y,
        double    *km_x,   /* RETURN */
        double    *km_y);  /* RETURN */
extern void pixel_to_disp_proj_v( margin_t    *margin, int    pix_x, int    pix_y,
        double    *km_x,  /* RETURN */
        double    *km_ht); /* RETURN */
extern void disp_proj_to_pixel_v( margin_t    *margin, double    km_x, double    km_ht,
        int    *pix_x,    /* RETURN */
        int    *pix_y);   /* RETURN */
extern void disp_proj_to_grid_v( MetRecord *mr, double    km_x, double    km_ht,
        int    *grid_x,    /* RETURN */
        int    *grid_y);   /* RETURN */
extern void rad_grid_to_pixel( margin_t *margin, MetRecord *mr, double grid_x, double grid_y,
        int    *pix_x,    /* RETURN */
        int    *pix_y);   /* RETURN */
extern void rad_grid_to_disp_proj( MetRecord *mr, double    grid_x, double    grid_y,
        double    *km_x,    /* RETURN */
        double    *km_y);   /* RETURN */
extern double disp_proj_dist(double x1, double y1, double x2, double y2);
extern double elapsed_time(struct timeval &tm1, struct timeval &tm2);
#endif

extern void pixel_to_grid_radar_no_cosine( MetRecord *mr, margin_t *margin,
                                           int pix_x, int pix_y,
                                           int *grid_x,    /* RETURN */
                                           int *grid_y);   /* RETURN */
#ifndef CIDD_FONT
// extern void load_fonts( Display *dpy);
extern QFont choose_font( const char *string, int x_size, int y_size, int *xmid, int *ymid);
#endif

#ifndef CIDD_INIT
extern int init_data_space(QObject *lucid);
extern void init_globals();
#endif

#ifndef CIDD_MAIN
extern int main(int  argc, char **argv);
#endif

#ifndef CIDD_PARAMS
const char *find_tag_text(const char *input_buf, const char * tag,
                          long *text_len, long *text_line_no);
void load_db_data( const string &fname);
#endif 

#ifndef CIDD_RESET
extern void invalidate_all_data();
extern void close_all_popups();
extern void reset_display();
#endif

#ifndef CIDD_REMOTE_COMMANDS
extern void ingest_remote_commands();
extern void remote_new_mdv_avail(const char *name);
extern void remote_new_spdb_avail(const char *name);
#endif

// #ifndef COLORSCALES
// // extern void setup_colorscales(Display *dpy);
// extern void setup_color_mapping(Valcolormap_t *vcm,
// 				double scale, double bias, int transform_type,
// 				double bad_value, double missing_value);
// // extern GC Val2GC(Valcolormap_t *vcm, double val);
// extern QBrush Val2Brush(Valcolormap_t *vcm, double val);
// #endif

#ifndef DATA_HANDLING
extern int gather_hwin_data( int page, time_t start_time, time_t end_time);
extern int gather_vwin_data( int page, time_t start_time, time_t end_time);
extern void check_for_io();
extern void cancel_pending_request();
extern void autoscale_vcm(Valcolormap_t *vcm, double min, double max);
extern void set_field(int value);
#endif

#ifndef DRAW_PU_PROC
void update_draw_export_panel();
#endif

#ifndef PAGE_PU_PROC
extern void set_color( const char *cname, void* c_data);
// extern void ref_mark_proc(Panel_item item, int value, Event *event);
// extern void wind_scale_proc(Panel_item item, int value, Event *event);
// extern void cont_activate_proc(Panel_item item, int value, Event *event);
// extern void cont_label_set_proc(Panel_item item, int value, Event *event);
// extern Panel_setting cont_fr_proc(Panel_item item, Event *event);
// extern Panel_setting cont_to_proc(Panel_item item, Event *event) ;
// extern Panel_setting cont_int_proc(Panel_item item, Event *event);
// extern Menu cont_field_mu_gen(Menu menu, Menu_generate op);
// extern void dim_im_proc(Panel_item item, int value, Event *event);
// extern void ref_color_proc(Panel_item item, Event *event);
// extern void cont_color_proc(Panel_item item, Event *event);
// extern void vect_set_color_proc(Panel_item item, Event *event);
// extern void set_ov_color_proc(Panel_item item, Event *event);
// extern int vect_field_num_set_proc(Panel_item item, const char *string,
//                             Xv_opaque client_data, Panel_list_op  op,
//                             Event *event, int row);
// extern int over_select_proc(Panel_item item, const char *string,
//                       Xv_opaque client_data, Panel_list_op  op,
//                       Event *event, int row);
// extern void prod_font_sel_proc(Panel_item item, int value, Event *event);
// extern void prod_line_width_proc(Panel_item item, int value, Event *event);
// extern void prod_time_sel_proc(Panel_item item, int value, Event *event);
// extern void prod_time_width_proc(Panel_item item, int value, Event *event);
// extern Panel_setting overlay_fld_max_proc(Panel_item item, Event *event);
// extern void overlay_fld_on_proc(Panel_item item, int value, Event *event);
// extern void set_cont_field_proc(Panel_item item, Event *event);
// extern void set_ov_fld_proc(Panel_item item, Event *event);
// extern void cont_field_proc(Menu menu, Menu_item item);
// extern void cont_layer_set_proc( Panel_item item, int value, Event *event);
extern void update_layered_contour_panel();
// extern void ov_field_num_sel_proc( Panel_item item, int value, Event *event);
extern void update_layered_field_panel();
// extern void save_h_image_proc(Panel_item item, Event *event);
// extern void layer_mode_proc(Panel_item item, int value, Event *event);
// extern void layer_legend_proc(Panel_item item, int value, Event *event);
// extern void cont_legend_proc(Panel_item item, int value, Event *event);
// extern void wind_legend_proc(Panel_item item, int value, Event *event);
#endif

#ifndef FIELDS_PU_PROC
// extern int field_display_proc(Panel_item item, const char *string, Xv_opaque client_data,
//                        Panel_list_op op, Event* eventr, int row);
// extern void config_field_proc(Menu menu, Menu_item item);
// extern void update_grid_config_gui();
// extern void update_wind_config_gui();
// extern void update_prod_config_gui();
// extern void set_cscale_apply_proc(Panel_item item, Event *event);
// extern void set_composite_proc(Panel_item item, int value, Event *event);
// extern void set_ren_meth_proc(Panel_item item, int value, Event *event);
// extern void set_update_proc(Panel_item item, int value, Event *event);
// extern void set_servmap_proc(Panel_item item, int value, Event *event);
// extern Menu field_mu_gen_proc( Menu  menu, Menu_generate op);
// extern Panel_setting set_host_proc(Panel_item item, Event *event);
// extern Panel_setting set_port_proc(Panel_item item, Event *event);
// extern Panel_setting set_field_no_proc(Panel_item item, Event *event);
// extern Panel_setting set_stretch_proc(Panel_item item, Event *event);
// extern Panel_setting set_subtype_proc(Panel_item item, Event *event);
// extern Panel_setting set_instance_proc(Panel_item item, Event *event);
// extern Panel_setting set_cscale_min_proc(Panel_item item, Event *event);
// extern Panel_setting set_cscale_max_proc(Panel_item item, Event *event);
// extern Panel_setting set_time_offset_proc(Panel_item item, Event *event);
#endif

#ifndef GRAPHIC_MARGIN_EVENTS
// extern void top_margin_event(Event *event);
// extern void bot_margin_event(Event *event);
// extern void left_margin_event(Event *event);
// extern void right_margin_event(Event *event);
void set_height(int index);
#endif

#ifndef GRAPHIC_CANVAS_EVENTS_BKWD
// extern Notify_value can_event_proc_bkwd(Xv_window win, Event *event,
//                             Notify_arg arg, Notify_event_type type);
#endif

#ifndef GRAPHIC_CANVAS_EVENTS
// extern Notify_value can_event_proc(Xv_window win, Event *event,
//                             Notify_arg arg, Notify_event_type type);
// extern void can_repaint( Canvas canvas, Window paint_window, Display *display,
//                          Window xid /* , Xv_xrectlist *rects */);
// extern void process_rotate_keys(Event*);
#endif
// extern void handle_click_h(Event *event,
//                            int clickXPixel, int clickYPixel,
//                            double clickXKm, double clickYKm,
//                            double clickLat, double clickLon,
//                            int clickType);

#ifndef GRAPHIC_CHECK
extern void check_for_invalid_images(int index);
#endif

#ifndef GRAPHIC_COMPUTE
extern double compute_tick_interval(double range);
extern double compute_cont_interval(double vaule);
extern double compute_range(double x1, double y1, double x2, double y2);
#endif

#ifndef GRAPHIC_CROSS_SECTION
extern void setup_route_area(int clear_flag);
extern void redraw_route_line(win_param_t * win);
#endif

#ifndef GRAPHIC_DUMP_IMAGE
extern const char * gen_image_fname(const char* prefix,MetRecord *mr);
extern void dump_cidd_image(int win, int confirm_flag,int print_flag,int page);
#endif

#ifndef GRAPHIC_MANAGE
extern void manage_h_pixmaps(int  mode);
extern void manage_v_pixmaps( int mode);
#endif

#ifndef GRAPHIC_PANEL
// extern Notify_value h_pan_event_proc( Xv_window   win, Event *event,
//                                Notify_arg  arg, Notify_event_type type);
#endif

#ifndef GRAPHIC_RESET
extern void reset_time_list_valid_flags();
extern void reset_data_valid_flags(int hflag,int vflag);
extern void reset_time_allowances();
extern void reset_terrain_valid_flags(int hflag,int vflag);
extern void set_redraw_flags(int h_flag,int v_flag);
extern void next_cache_image();
extern void prev_cache_image();
#endif

#ifndef GRAPHIC_RESIZE
// extern Notify_value h_win_events( Frame frame, Event *event,
//                        Notify_arg arg, Notify_event_type type);
#endif

#ifndef GRAPHIC_XSECT_CANVAS_EVENTS
// extern Notify_value v_can_events( Xv_window  win, Event *event, Notify_arg arg,
//                            Notify_event_type type);
// extern void v_can_repaint( Canvas canvas, Window paint_window, Display *display,
//                    Window xid /*, Xv_xrectlist *rects */);
#endif

#ifndef GRAPHIC_XSECT_RESIZE
// extern Notify_value v_win_events(Frame frame, Event *event, 
//                          Notify_arg arg, Notify_event_type type);
#endif

#ifndef GRAPHIC_ZOOM
extern void do_zoom();
extern void do_vert_zoom();
extern void zoom_radial(double min_x, double min_y, double max_x, double max_y);
extern void do_zoom_pan();
extern void redraw_zoom_box();
extern void redraw_vert_zoom_box();
extern void redraw_pan_line();
#endif

extern void save_current_zoom(double zoom_min_x, double zoom_min_y,
                              double zoom_max_x, double zoom_max_y);
extern void clear_zoom_stack();
extern void set_domain_zoom(double zoom_min_x, double zoom_min_y,
                            double zoom_max_x, double zoom_max_y);
extern void zoom_back();
  
#ifndef GUI_DESTROY
// extern Notify_value x_error_proc( Display *disp, XErrorEvent *event);
// extern Notify_value base_win_destroy( Notify_client client, Destroy_status status);
#endif

// #ifndef GUI_INIT
// extern void init_xview(int *argc_ptr, char    *argv[]);
// #endif

#ifndef GUI_LABEL_FRAME
extern void gui_label_h_frame(const char    *string,int persistance);
extern void gui_label_v_frame (const char *string);
extern void set_busy_state(int state);
extern void update_ticker(time_t cur_time);
#endif

#ifndef GUI_MODIFY
extern void modify_gui_objects();
extern void init_field_menus();
extern const char * frame_time_msg(int index);
void update_frame_time_msg(int index);
#endif

#ifndef H_WIN_PROC
// extern void main_st_proc(Panel_item item, u_int value, Event *event);
extern void set_height_label();
extern void show_dpd_menu(u_int value);
extern void show_cmd_menu(u_int value);
extern void show_view_menu(u_int value);
extern void show_exprt(u_int value);
extern void show_dpd_panel( u_int value);
extern void show_map_menu( u_int value);
extern void show_xsect_panel( u_int value);
extern void show_time_panel(u_int value);
extern void show_grid_panel( u_int value);
extern void show_draw_panel( u_int value);
extern void winds_onoff( u_int value);
extern void forecast_onoff( u_int value);
extern void symprods_onoff( u_int value);
extern void set_draw_export_mode( u_int value);
extern void set_route_export_mode( u_int value);
extern void set_pick_export_mode( u_int value);
extern void startup_snapshot( u_int value);
#endif

#ifndef MOVIE_CONTROL
extern int time_for_a_new_frame();
extern void reset_time_points();
extern void rotate_movie_frames();
extern void adjust_pixmap_allocation();
extern void parse_string_into_time( const char *string, UTIMstruct *time);
#endif

#ifndef MOVIE_FRAME_RETRIEVE
extern void retrieve_h_movie_frame(int    index, QPaintDevice *pdev);
extern void retrieve_v_movie_frame(int index, QPaintDevice *pdev);
#endif

#ifndef MOVIE_FRAME_SAVE
extern int save_h_movie_frame( int index, QPaintDevice *pdev, int page);
extern int save_v_movie_frame( int index, QPaintDevice *pdev);
#endif

#ifndef MOVIE_PU_PROC
extern void update_movie_popup();
extern void movie_start(u_int value);
// extern void movie_start_proc(Panel_item item, int value, Event *event);
// extern void movie_type_proc(Panel_item item, int value, Event *event);
// extern void movie_frame_proc(Panel_item item, int value, Event *event);
// extern void movie_speed_proc(Panel_item item, int value, Event *event);
// extern void movie_delay_proc(Panel_item item, int value, Event *event);
extern void set_display_time(time_t utime);
// extern Panel_setting start_time_proc(Panel_item item, Event *event);
// extern Panel_setting start_frame_proc(Panel_item item, Event *event);
// extern Panel_setting end_frame_proc(Panel_item item, Event *event);
// extern Panel_setting time_interv_proc(Panel_item item, Event *event);
// extern Panel_setting set_fcast_period_proc(Panel_item item, Event *event);
extern void set_end_frame(int num_frames);
#endif

#ifndef GEN_TIME_PU_PROC
extern MetRecord    *choose_model_mr(int page);
extern void show_gen_time_menu( u_int value);
// extern void gen_time_proc(Panel_item item, int value, Event *event);
#endif

#ifndef OVER_PU_PROC
// extern void over_pu_proc(Panel_item item, u_int value, Event *event);
#endif

#ifndef OVERLAY_INIT
// extern void normalize_longitude(double min_lon, double max_lon, double *normal_lon);
extern void init_over_data_links(const char *param_buf, long param_buf_len, long line_no);
extern int load_map_info(const char *param_buf, long param_buf_len, long line_no,
                         MapOverlay_t **overlays, int  max_overlays);
int load_map_data(MapOverlay_t **overlays, int  num_overlays);
extern void load_rap_map(MapOverlay_t *overlay, const char *map_file_subdir);
extern void load_shape_map(MapOverlay_t *overlay, const char    *map_file_subdir);

#endif

#ifndef PROCESS_ARGS
extern void process_args(int argc, char *argv[]);
#endif

#ifndef RENDER_AZMIUTHS
extern void draw_cap_azimuth_lines(QPaintDevice *pdev);
#endif

#ifndef RENDER_BOTTOM_MARGIN
extern time_t time_from_pixel(int x_pixel);
extern int draw_hwin_bot_margin( QPaintDevice *pdev, int page,
                          time_t start_time, time_t end_time);
#endif

#ifndef RENDER_CART_GRID
extern int render_cart_grid( QPaintDevice *pdev, MetRecord *mr, time_t start_time,
                      time_t end_time, int is_overlay_field);
#endif

#ifndef RENDER_CBAR
// extern void draw_colorbar(Display *dpy, QPaintDevice *pdev, GC gc,  int x1,  int y1,
//                           int width, int height,
//                           int num_entries,  Val_color_t **vc,
//                           int orient,  const char *units);
#endif


#ifndef RENDER_HT_SEL
extern double height_from_pixel(int y_pixel, MetRecord *mr);
extern MetRecord    *choose_ht_sel_mr(int page);
// extern void draw_height_selector(Display *dpy, QPaintDevice *pdev, GC gc_axis,
//                                  GC gc_ind, int page,  int x1,  int y1, int width, int height);
#endif

#ifndef RENDER_CONTROL
// extern int render_horiz_display(QPaintDevice *pdev, int page, time_t start_time,
//                                time_t end_time);
#endif

#ifndef RENDER_DISTORTED_GRID
extern int  render_distorted_grid(QPaintDevice *pdev, MetRecord *mr, 
                                  time_t start_time, time_t end_time,
                                  int is_overlay_field);
#endif

#ifndef RENDER_FILLED_CONTOURS
extern void draw_filled_contours(QPaintDevice *pdev,
                                 int x_start[], int y_start[],
                                 MetRecord *mr);
extern void draw_filled_contours_d(QPaintDevice *pdev, MetRecord *mr);

extern void draw_xsect_filled_contours( QPaintDevice *pdev,
                                        int x_start[], int y_start[],
                                        MetRecord *mr);
#endif

extern void RenderFilledPolygons(QPaintDevice *pdev, MetRecord *mr, 
				 bool is_vert = false);

#ifndef RENDER_FILLED_IMAGE
extern int draw_filled_image(QPaintDevice *pdev, int x_start[], int y_start[], MetRecord *mr);
#endif

#ifndef RENDER_GRIDS
// extern int render_grid(QPaintDevice *pdev,  MetRecord *mr, time_t start_time, time_t end_time, int is_overlay_field);
#endif

#ifndef RENDER_LEFT_MARGIN
extern int draw_hwin_left_margin(QPaintDevice *pdev);
#endif

#ifndef RENDER_LINE_CONTOURS
extern void render_line_contours(QPaintDevice *pdev, contour_info_t *crec);
#endif

extern void RenderLineContours(QPaintDevice *pdev, contour_info_t *crec,
			        bool is_vert = false);

#ifndef RENDER_XSECT_LINE_CONTOURS
extern void render_xsect_line_contours(QPaintDevice *pdev, contour_info_t *crec);
#endif

#ifndef RENDER_LEGENDS
extern const char *vlevel_label(Mdvx::field_header_t *fhdr);
extern const char * field_label( MetRecord *mr);
extern const char* height_label();
extern int draw_hwin_interior_labels(QPaintDevice *pdev, int page, time_t start_time, time_t end_time);
#endif

#ifndef RENDER_MARGINS
extern void render_horiz_margins(QPaintDevice *pdev, int page, time_t start_time, time_t end_time);
#endif

#ifndef RENDER_MOVIE_FRAME
extern int render_h_movie_frame( int    index, QPaintDevice *pdev);
extern int render_v_movie_frame( int    index, QPaintDevice *pdev);
#endif

#ifndef RENDER_OVERLAYS
// extern void calc_local_over_coords();
// extern void render_map_overlays(QPaintDevice *pdev);
#endif

#ifndef RENDER_POLAR_GRID
extern void rotate_points(double theta, double x_cent, double y_cent, double *xarr, double *yarr, int num_points);
extern int render_polar_grid(QPaintDevice *pdev, MetRecord *mr,
                      time_t start_time, time_t end_time, int is_overlay_field);
#endif

#ifndef RENDER_PRODUCTS
extern void render_products(QPaintDevice *pdev, time_t start_time, time_t end_time);
#endif

#ifndef RENDER_RANGE_RINGS
extern void draw_cap_range_rings(QPaintDevice *pdev);
#endif

#ifndef RENDER_RIGHT_MARGIN
extern int draw_hwin_right_margin(QPaintDevice *pdev, int page);
#endif

#ifndef RENDER_ROUTE_WINDS
extern void render_route_winds(QPaintDevice *pdev);
extern void  ave_winds(MetRecord *mr_u, MetRecord *mr_v,
         double start_km, double end_km, double *ave_deg, double *ave_spd);
extern double peak_turb(MetRecord *mr_turb, double start_km, double end_km);
extern double peak_icing(MetRecord *mr_icing, double start_km, double end_km); 
extern void rotate_route(double theta); 
#endif

#ifndef RENDER_TERRAIN
extern void render_h_terrain(QPaintDevice *pdev, int page);
extern void render_v_terrain(QPaintDevice *pdev);
#endif

#ifndef RENDER_CLICK_MARKS
extern int render_click_marks();
#endif

#ifndef RENDER_TOP_LAYERS
extern int render_top_layers(QPaintDevice *pdev);
#endif

#ifndef RENDER_TOP_MARGIN
extern int draw_hwin_top_margin(QPaintDevice *pdev);
#endif

#ifndef RENDER_WINDS
extern int render_wind_vectors(QPaintDevice *pdev, int start_time, int end_time);
#endif

#ifndef RENDER_XSECT_CONTROL
extern int render_vert_display(QPaintDevice *pdev, int page, time_t start_time, time_t end_time);
#endif

#ifndef RENDER_XSECT_GRIDS
extern int render_xsect_grid(QPaintDevice *pdev,  MetRecord *mr, time_t start_time, time_t end_time, int is_overlay_field);
#endif

#ifndef RENDER_XSECT_MARGINS
extern void draw_vwin_right_margin(QPaintDevice *pdev, int page);
extern void draw_vwin_left_margin(QPaintDevice *pdev, int page);
extern void draw_vwin_top_margin(QPaintDevice *pdev, int page);
extern void draw_vwin_bot_margin(QPaintDevice *pdev, int page);
#endif

#ifndef RENDER_XSECT_PRODUCTS
// extern void render_vert_products(QPaintDevice *pdev);
#endif

#ifndef RENDER_XSECT_TOP_LAYERS
extern void render_xsect_top_layers(QPaintDevice *pdev, int page);
#endif

#ifndef RENDER_XSECT_WINDS
extern int render_vert_wind_vectors(QPaintDevice *pdev);
#endif

#ifndef ROUTE_WINDS_INIT
extern void route_winds_init();
#endif

#ifndef SAVE_PU_PROCS
extern void update_save_panel();
// extern Panel_setting set_dir_proc(Panel_item item, Event *event);
// extern Panel_setting set_fname_proc(Panel_item item, Event *event);
// extern Panel_setting set_command_proc(Panel_item item, Event *event);
// extern void save_image_proc(Panel_item item, Event *event);
// extern void cancel_save_proc(Panel_item item, Event *event);
#endif

#ifndef SHMEM_INIT
extern void init_shared();
#endif

#ifndef STATUS_PU_PROC
extern void add_message_to_status_win(const char *mess, int display_flag);
extern void add_report_to_status_win(const char *mess);
#endif

#ifndef SYMPROD_INIT
extern void init_symprods();
#endif

#ifndef TIMER_CONTROL
// #include <QTimer>
// extern void timer_func(QTimerEvent *event);
extern void start_timer();
extern void stop_timer();
#endif

#ifndef V_WIN_PROC
extern void set_v_field(int field_no);
// extern Notify_value v_pan_event_proc( Xv_window win, Event *event,
//                                Notify_arg  arg, Notify_event_type type);
// extern void set_v_field_proc( Menu menu, Menu_item item);
// extern Menu v_field_mu_gen_proc(Menu menu, Menu_generate op);
// extern void v_panel_dismiss(Panel_item item, Event *event);
// extern void show_route_pu_proc(Panel_item item, unsigned int value, Event *event);
#endif

#ifndef WINDS_INIT
extern void init_wind_data_links(const char *param_buf, long param_buf_len, long line_no);
#endif

#ifndef XVIEW_FILE_ACCESS
// extern FILE* open_check_write(const char *file_name, Frame   owner);
// extern FILE * open_check_read( const char    *file_name, Frame    owner);
// extern int chdir_check( const char * path, Frame    owner);
#endif

#ifndef ZOOM_PU_PROC
// extern void set_domain_proc(Panel_item item, int value, Event *event);
#endif

#endif
