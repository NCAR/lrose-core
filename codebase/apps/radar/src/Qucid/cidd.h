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
//****************************************************************************/
// CIDD.H : Defines & includes for the Cartesian RADAR display program
 
#ifndef CIDD_H
#define CIDD_H

// #include <sys/param.h>
#include <math.h>           // System 
#include <errno.h>
#include <fcntl.h>
#include <memory.h>
#include <unistd.h>
#include <string.h>
#include <float.h>
#include <limits.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>


#include <X11/Xlib.h>       // X11 
#include <X11/Xutil.h>       
#include <X11/Xresource.h>       
// #include <xview/xview.h>    // XView 
// #include <xview/canvas.h>
// #include <xview/cms.h>
// #include <xview/icon_load.h>
// #include <xview/notify.h>
// #include <xview/notice.h>
// #include <xview/panel.h>
// #include <xview/scrollbar.h>
// #include <xview/cursor.h>
// #include <xview/svrimage.h>
// #include <xview/termsw.h>
// #include <xview/text.h>
// #include <xview/tty.h>
// #include <xview/textsw.h>
// #include <xview/xv_xrect.h>
// #include <devguide/gdd.h>  // Dev GUIDE 
// #include <devguide/gcc.h> 

// Undefine dangerous/silly xview macro
// #ifdef coord
// #undef coord
// #endif

#include <toolsa/os_config.h>     // OS Specific includes- defines 

// Library Utilities 
#include <dataport/port_types.h> 
#include <dataport/bigend.h> 
#include <dataport/swap.h> 
#include <toolsa/mem.h> 
#include <toolsa/globals.h> 
#include <toolsa/str.h>
#include <toolsa/pmu.h>     // process mapper structures 
#include <toolsa/http.h>    // HTTPgetURL support
#include <toolsa/HttpURL.hh>// HTTPURL class support
#include <toolsa/pjg.h>     // Map projection geometry 
#include <toolsa/sockutil.h>
#include <toolsa/str.h>     // string utilities 
#include <toolsa/utim.h>    // Unix time conversions 
#include <toolsa/umisc.h>
#include <toolsa/xdru.h>
#include <rapplot/xrs.h>
#include <Spdb/StationLoc.hh>  // Station locator class 
#include <rapplot/xutils.h> // X Windows related  support functions 

// MDVX support classes
#include <Mdv/Mdvx_typedefs.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxProj.hh>

// MDV Protocol classes
#include <Mdv/DsMdvxThreaded.hh>

// FMQ Support Classes
#include <Fmq/RemoteUIQueue.hh>

// TDRP Parameter Classes
#include "Csyprod_P.hh"
#include "Cdraw_P.hh"
#include "Cgui_P.hh"
#include "Cimages_P.hh"

// parameters

#include "Uparams.hh"

// Someone deines this under Solaris
#ifdef _P
#undef _P
#endif

// GUI  - Devguide Generated .h files 
// #include "h_win_ui.h"       // CIDD Object Definitions for horizontal view 
// #include "v_win_ui.h"       // CIDD Object Definitions for vertical view
// #include "page_pu_ui.h"     // CIDD Definitions for Extra features popup 
// #include "movie_pu_ui.h"    // CIDD Definitions for Movie Control popup 
// #include "zoom_pu_ui.h"     // CIDD Definitions for Zoom/Domain Control popup 
// #include "data_pu_ui.h"     // CIDD Definitions for Data field chooser popup 
// #include "draw_pu_ui.h"     // CIDD Definitions for Draw/Export Product OK/Edit popup 
// #include "fields_pu_ui.h"   // CIDD Definitions for Fields popup 
// #include "fcast_pu_ui.h"    // CIDD Definitions for Forecast Time Select popup 
// #include "past_pu_ui.h"     // CIDD Definitions for Past Time Select popup 
// #include "route_pu_ui.h"    // CIDD Definitions for Route Path popup 
// #include "save_pu_ui.h"     // CIDD Definitions for Save Image popup 
// #include "status_pu_ui.h"   // CIDD Definitions for Status text popup 
// #include "over_pu_ui.h"     // CIDD Definitions for Overlay Selector popup 
// #include "prod_pu_ui.h"     // CIDD Definitions for Product Selection popup 
// #include "gen_time_pu_ui.h" // CIDD Definitions for Model Run Time Selection popup 
// #include "cmd_pu_ui.h"      // CIDD Definitions for Command Menu popup 
 
// Exported Coordinate information for auxilliary programs
#include <rapformats/coord_export.h> 
#include "cidd_macros.h"    // CIDD specific defines 

// Display's structures
#include "cidd_structs.h"   // CIDD specific structure definitions 
#include "cidd_colorscales.h"// CIDD structs for Colorscales 
#include "cidd_field_data.h"// CIDD structs for Data IO 
#include "cidd_data_io.h"   // CIDD structs for Data IO
#include "cidd_winds.h"     // CIDD structs for Winds Fields
#include "cidd_contours.h"  // CIDD structs for Contours
#include "cidd_movies.h"    // CIDD structs for Movie Control popup 
#include "cidd_overlays.h"  // CIDD structs for Map Overlays 
#include "cidd_products.h"  // CIDD structs for Symbolic Products 
#include "cidd_layers.h"    // CIDD structs for Layered products 
#include "cidd_windows.h"   // CIDD structs for GUI Windows 
#include "cidd_legend.h"    // CIDD structs for Window Legends 
// #include "cidd_dpd.h"       // CIDD structs for Display Page Descriptions

#include "cidd_funcs.h"     // CIDD specific function prototype definitions 

#include "RenderContext.hh"
#include "ProductMgr.hh"
#include "TimePlot.hh"

//************* GLOBAL DATA DECLERATIONS *************************************/
 

struct    Global_data {
  // GUI OBJECTS
  // h_win_horiz_bw_objects  *h_win_horiz_bw; // Handles to Plan view widgets
  Drawable    hcan_xid;    

  // v_win_v_win_pu_objects  *v_win_v_win_pu;// Handles to Vert view widgets
  Drawable    vcan_xid;    

  // page_pu_page_pu_objects *page_pu; // Overlays popup widgets
  // movie_pu_movie_pu_objects   *movie_pu; // Movie controls popup widgets
  // zoom_pu_zoom_pu_objects   *zoom_pu; // Domain selector popup widgets
  // zoom_pu_bookmk_pu_objects *bookmk_pu; // URL selector widgets
  // data_pu_data_pu_objects   *data_pu; // Config popup widgets
  // draw_pu_draw_pu_objects   *draw_pu; // Draw/Export Wdit/OK popup wigets
  // over_pu_over_pu_objects   *over_pu; // Overlays popup widgets
  // prod_pu_prod_pu_objects   *prod_pu; // Product selection popup widgets
  // fields_pu_fields_pu_objects *fields_pu; // Overlays popup widget;
  // fcast_pu_fcast_pu_objects *fcast_pu; // Forecast time menu widgets;
  // past_pu_past_pu_objects   *past_pu; // Past Menu widgets;
  // route_pu_route_pu_objects  *route_pu; // Route popup widgets
  // save_pu_save_im_pu_objects  *save_pu; // Save Image popup widgets
  // status_pu_status_pu_objects *status_pu; // Status text popup widgets
  // gen_time_pu_popup1_objects *gen_time_pu; // Model Run time Selection popup widgets
  // cmd_pu_cmd_pu_objects *cmd_pu; // Command Menu Popup

  int    debug;        // Normal debugging flag  
  int    debug1;       // More verbose debugging  flag 
  int    debug2;       // Very verbose debug statements flag 

  // DISPLAY PREFERENCE VARIABLES
  int    argc;
  int    display_projection;// Which projection CIDD Uses for display  - Enum
  int    always_get_full_domain; // 1 = Data request full domain always
  int    do_not_clip_on_mdv_request;
  int    do_not_decimate_on_mdv_request;
  int    enable_status_window;   // 1 = Status window enabled.
  int    wsddm_mode;   // 1 =  End User/WSDDM  mode - 
  int    quiet_mode;   // 1 =  Do not output normal startup and shutdown messages
  int    report_mode;   // 1 =  Do continious data value reporting
  int    close_popups;   // Close popups also when main window is closed.
  int    one_click_rhi;  // 1 =  Disallow Multiple segment Routes - First button release ends route.

  int    click_posn_rel_to_origin; // report posn relative to origin 
  int    report_clicks_in_status_window; // report click details in status window
  int    report_clicks_in_degM_and_nm; /* also report click locations in degM and nautical
                                        * miles, suitable for aviation ops */
  double magnetic_variation_deg; // magnetic variation in degrees
  int    display_labels; // 1 == On (default)
  int    display_ref_lines; // 1 == On (default) - Display Route and height lines
  int	   show_clock;     // 1 =  Diplays an analogue clock
  int	   show_data_messages;// 1 =  Diplays data access and Rendering messages in top panel
  int    draw_clock_local; // 0 = UTC, 1 = Local time  on analogue clock
  int    use_local_timestamps; // 0 = UTC time  1 = Local times 
  int	   run_unmapped;   // 1 runs unmapped
  int    html_mode;      // 1 = CIDD is in HTML Generator mode 
  int    transparent_images; /* 0 = images have normal background
                              * 1 = images have transparent background */
  int    range_ring_follows_data; // 0 = fixed rings, 1 = Plots at the data origin
  int    range_ring_for_radar_only; /* 1 = only show rings for radar data
                                     * 0 = show for all data types */
  int    domain_follows_data; // 1 = Domain shifts so data is always at the center
  int    show_height_sel;// 1 = Show Vertical Height selector in Right Margin
  int    use_cosine_correction; // use cosine correction for computing range in polar data
  int    latlon_mode;    // 0 = Report decimal degrees, 1 = deg,min,sec  
  int    num_zoom_levels; // number of zoom levels available
  int    start_zoom_level; // starting zoom level - 1-based
  int    zoom_limits_in_latlon; // 1 = set zoom limits in lat/lon degrees instead of km

  double domain_limit_min_x;
  double domain_limit_max_x;
  double domain_limit_min_y;
  double domain_limit_max_y;
  
  double min_ht;
  double max_ht;
  double start_ht;
  int planview_start_page;
  int xsect_start_page;
  
  int    label_contours; // 0 = No numeric Labels, 1 = Add numeric value labels
  int    top_margin_render_style; //  1 = Distance scale, 2 = Frame time, etc, 3 = None.
  int    bot_margin_render_style; //  1 = Distance scale, 2 = Frame time, etc
  int    drawing_mode;   // Flag for Internal drawing mode - FMQ output
  // 0 = not in drawing mode (initial value) 
  // != 0  Disable - zoom/pan, reporting 

  int layer_legends_on;     // Control variables for plotting legend/Titles
  int cont_legends_on;
  int wind_legends_on;
  int contour_line_width;
  int smooth_contours;     // Apply smoothing before contouring ;  0-2 are valid values 
  int use_alt_contours;    // 1 =  Use the Alternate Contouring routines 
  int add_noise;           // Add 1 part in 250 of noise to contours 1 == true
  double special_contour_value;      // which value to draw wider 
  int map_bad_to_min_value; // 1 == true
  int map_missing_to_min_value; // 1 == true

  int products_on;
  int product_line_width;
  int product_font_size;
  double product_detail_threshold[NUM_PRODUCT_DETAIL_THRESHOLDS];
  int product_detail_adjustment[NUM_PRODUCT_DETAIL_THRESHOLDS];
  
  int draw_main_on_top;  //  1 = Draw the main grid over all the other layers. 
  int mark_latest_click_location; // place a mark at latest click location
  int mark_latest_client_location; // place a mark at latest click location from client
  int check_data_times;  //  1 =  Reject data that is not valid 
  int check_clipping;    //  1 = Check for data that renders off the screen.
  int run_once_and_exit; //  1 =  Plot all HTML Output and exit
  int request_compressed_data;  //  1 =  Ask for data to be compressed during xfer
  int request_gzip_vol_compression;  //  1 =  Ask for data to be compressed Using GZIP_VOL
  int add_height_to_filename;   // 1 = Add the height to html mode filenames
  int add_frame_time_to_filename; // 1 = Add the frame time to html mode filenames
  int add_gen_time_to_filename; // 1 = Add the model gen time to html mode filenames
  int add_valid_time_to_filename; // 1 = Add the data valid  time to html mode filenames
  int add_frame_num_to_filename; // 1 = Add the Frame Number  time to html mode filenames
  int add_button_name_to_filename; /* 1 = Add the button_name (second string in GRIDS 
                                    * section of config file) to html mode filenames.
                                    * It immediately follows the legend_name, which
                                    * is also obtained from the config file. */
  int save_images_to_day_subdir; /* 1 = save images to subdirs of image_dir,
                                  * using yyyymmdd format */
  int simple_command_timeout_secs; // Timeout for simple commands
  int complex_command_timeout_secs; // Timeout for simple commands

  int font_display_mode; // 1 = Use ImageString, 0 = Plain. 
  int forecast_mode;     // 1 = Apply forecast interval
  int gather_data_mode;  // 0 = Use Mid point of movie frame for requests, 1 = End 
  int enable_save_image_panel; // 1 = Middle Click will bring up save image panel
  int disable_pick_mode;  // Don't allow product pick when set
  int clip_overlay_fields; // If set = Overlaid fields will use filled polygons 
  int output_geo_xml;       // If set = Output an additional XML file with images
  int use_latlon_in_geo_xml;       // If set = Use lat/lon to describe the geo location rather than projection coordinates
  int replace_underscores;  // If set = Replace Underscores in legends and labels with spaces.

  int    data_format; // format of received data:  CART_DATA_FORMAT  
  int    image_fill_threshold; // threshold (grid points)  for image fills 
  int    dynamic_contour_threshold; // threshold for dynamic contours fills 
  int    inten_levels;    // number of color dimming intensity levels
  int    idle_reset_seconds;   // number of idle seconds before automatically resetting
  int    model_run_list_hours; // Gather all model run times within this number of
  // hours from the end of the movie loop
  // WIND Vector & barb preferences.
  int ideal_x_vects;
  int ideal_y_vects;
  int wind_head_size;     // Vectors
  double wind_head_angle;     // Wind barb head angle.
  int barb_shaft_len;     // Wind barbs
  int all_winds_on;
  int wind_mode;
  double wind_time_scale_interval;
  int wind_scaler;

  // Run-time counters and indicies
  int    num_colors;       // Total colors allocated 
  int    num_draw_colors;  // Total number of colors excluding color scale colors 
  int    map_overlay_color_index_start;
  time_t last_event_time;  // the unix time of when the last activity occured
  time_t epoch_start;      //  The start of the movie loop
  time_t epoch_end ;       //  the end of the movie loop
  time_t model_run_time;  //  Which Model run to use.  When 0 = Ask for "lastest"
  time_t data_request_time; // Time for which Mdv data is requested
  int    finished_init;    // 0 before xview init, 1 after.

  int    num_datafields;   // the number of fields in the data.info file 
  int    num_menu_fields;  // the number of items in the data field menus 
  int    num_field_menu_cols; // number of columns in the data field menus 
  int    num_map_overlays; // the number of overlays in the system 
  int    num_bookmarks;    // the number of bookmarks defined
  int    num_render_heights;// the number of heights to render in HTML mode
  int    num_cache_zooms;   // the number of cached images for quick access
  int    cur_render_height; // Current height being rendered in HTML mode
  int    cur_field_set;     // Current set of fields that is active
  int    save_im_win;       // Which window to dump: PLAN_VIEW, XSECT_VIEW or BOTH_VIEWS 
  int    image_needs_saved; // Set to 1 when a finished image needs saved to disk
  int    generate_filename; // Flag, indicating a new name needs generated.
  int    max_time_list_span; // Maximum number of days in epoch to a request a time list 

  // State Variables
  int    pan_in_progress;    // Set to 1 while panning 
  int    zoom_in_progress;   // Set to 1 while zooming 
  int    route_in_progress;  // Set to 1 while defining an ROUTE 
  int    data_timeout_secs;  // number of seconds before request is canceled
  int    data_status_changed;// 1 =  all data is now invalidated 
  int    series_save_active; // When 1 - In series save mode.

  int    num_field_labels;  // Current number of labels spaces occupied by field data labels 
  int    db_data_len;       // Length of the data base 
  int    field_index[MAX_DATA_FIELDS];   // menu item to field number lookup table 
  int movieframe_time_mode;  // mode for determining output file name

  // 0: use the end time for file name
  // 1: use the mid time for file name

  double image_inten;    // Image color intensity 
  double data_inten;     // Data Cell color intensity 
  double aspect_correction; // Aspect ratio correction for LAT-LON mode 
  double aspect_ratio;  // Aspect ratio for domains - Width/Height 
  double scale_units_per_km; // For scaling distance to things other than km.
  double locator_margin_km;  // Max dist between requested point and station 
  double height_array[MAX_SECTS];  // Heights to render in HTML MODE

  double origin_latitude;
  double origin_longitude;
  double reset_click_latitude;
  double reset_click_longitude;

  // projections
  
  const char *projection_type;
  double proj_param[8];    // Projection parameters - Like Mdv
  double north_angle;      // radar cart
  double lambert_lat1;     // lambert
  double lambert_lat2;     // lambert
  double tangent_lat;      // stereographic
  double tangent_lon;      // stereographic
  double central_scale;    // stereographic

  // movies
  
  int movie_on;
  double movie_magnify_factor;
  double time_interval;
  double frame_span;
  int starting_movie_frames;
  int reset_frames;
  double movie_delay;
  double forecast_interval;
  double past_interval;
  double stretch_factor;
  const char *climo_mode;
  int temporal_rounding;
  int movie_speed_msec;

  // start time etc

  const char *demo_time;
  
  // Files, names, Commands and other text parameters
  char **argv;             // Command line arguments
  const char *orig_wd;           // Original working directory
  const char *db_data;           // Pointer to the parameter data
  const char *db_name;           // The  parameter database filename 
  const char *frame_label;       // The default string to show on the frame 
  const char *no_data_message;   // The default message to display on no data conditions 
  const char *help_command;      // the command used to spawn A Help Viewer
  const char *bookmark_command;  // the command used to spawn URL Sender 
  const char *app_name;          // Application name 
  const char *app_instance;      // Application Instance 
  const char *scale_units_label; // Units label to use for distance scales

  const char *image_dir;           // The place to store images
  const char *horiz_image_dir;     // The place to store horiz images
  const char *vert_image_dir;      // The place to store vert images
  const char *horiz_image_fname;   // File name for horiz images
  const char *vert_image_fname;    // File name for vert images
  const char *horiz_image_command; // Convert command for horiz images
  const char *vert_image_command;  // Convert command for vert images
  
  const char *image_convert_script; // Cidd Calls this script with the image name as the argument  
  const char *series_convert_script; // Cidd Calls this script with all image names as arguments 

  const char *image_ext; // Image output extension - only png is legal
  const char *image_horiz_prefix;   // Prefix for plan-view images
  const char *image_vert_prefix;   // Prefix for vert-section images
  const char *image_name_separator; // Char used to separate parts of the image file name. Defaults to '_'.

  const char *print_script; // Cidd Calls this script with the image name as the argument  
  
  const char *http_tunnel_url;   // The url of a DsServer tunnel
  const char *http_proxy_url;    // The url of a http proxy server
  const char *station_loc_url;   // The url of a station locator file
  const char *remote_ui_url;     // The url of a Remote User Interface/Command FMQ
  const char *datamap_host;      // The Host to query for data mapper information.

  const char *label_time_format;  // strftime() format string for top label formatting

  const char *moviestart_time_format;   // strftime() format string for the movie start time text field
  
  const char *frame_range_time_format;   // strftime() format string for the frame time range text fields
  
  const char *movieframe_time_format;  // strftime() format string for naming movie frame xwd files
  // Files are named cidd_fieldname_zoomlevel_time.xwd
  char *data_info[MAX_DATA_FIELDS]; // information about each field  

  time_list_t gen_time_list;

  MdvxProj proj; // Display projection

  // PARAMETER DATABASE
  // XrmDatabase    cidd_db;     // The application's default parameter database 

  // X WINDOW Params
  win_param_t    h_win;     // Plan view X window parameters
  win_param_t    v_win;     // Cross Section (Vert) view X window parameters


  // Drawing contexts
  GC  def_gc;       // default gc for copy & misc X operations  
  GC  ol_gc;        // Gc for drawing in the reference reference overlay color 
  GC  clear_ol_gc;  // Gc for Removing the reference overlay color 
  char *dpyName; // display name
  Display *dpy;     // default Display pointer for copy operations 
  Colormap cmap;
  Color_gc_t    color[MAX_COLORS];   // stores all colors and GCs 
  Color_gc_t    null_color;          // The color "transparent" 

  // FONTS
  int    num_fonts;                  // number of fonts in use 
  Font   ciddfont[MAX_FONTS];        // fonts in size order 
  XFontStruct    *fontst[MAX_FONTS]; // Font info 

  prod_info_t prod;	// Symbolic product control info 

  Overlay_t  *over[MAX_OVERLAYS];    // Overlay Data

  met_record_t  *mrec[MAX_DATA_FIELDS]; // data records  

  layers_t    layers;        // Control variables for layers features 

  legend_t    legends;       // Control variables for Legends

  menu_bar_t  menu_bar;      // Control variables for the Main Menu bar

  bookmark_t *bookmark;      // Array of bookmarks 

  movie_control_t    movie;    // Control vartiables for movie looping 

  io_info_t    io_info;            // I/O infomation 

  status_msg_t    status;  // Dynamic Status message info 

  draw_export_t   draw;   // Control variables for Draw-Export feature

  coord_export_t  *coord_expt; // Pointer to exported coordinates & info  

  ProductMgr  *prod_mgr;       // Products Manager

  TimePlot *time_plot;         // Time scale plotter class for movie panel

  RenderContext *r_context;    // Symprod/Gdraw Contains Window, Domain geometries, etc

  StationLoc *station_loc;    // Station locator class

  RemoteUIQueue *remote_ui;   // REmote User Interface/Command class


  // TDRP CLASSES
  Csyprod_P *syprod_P;  // Symbolic Products
  Cdraw_P *draw_P;      // Draw Params
  Cgui_P *gui_P;        // GUI Config Params
  Cimages_P *images_P;  // image generation params

  // parameter object
  Uparams *uparams;

  // contours

  int contour_font_num;
  int n_ideal_contour_labels;

  // canvas events

  double rotate_coarse_adjust;
  double rotate_medium_adjust;
  double rotate_fine_adjust;

  // zoom

  double min_zoom_threshold;

  // shmem

  int coord_key;

  // gui
  
  const char *status_info_file;

  // positions

  int horiz_default_x_pos;
  int horiz_default_y_pos;
  
  // overlays

  const char *map_file_sub_dir;

  // page_pu_proc

  int ideal_x_vectors;
  int ideal_y_vectors;
  double azimuth_interval;
  double azimuth_radius;
  int latest_click_mark_size;
  int range_ring_x_space;
  int range_ring_y_space;
  double range_ring_spacing;
  double max_ring_range;
  int range_ring_labels;
  double wind_units_scale_factor;
  const char *wind_units_label;
  double wind_w_scale_factor;

  // symprods

  double scale_constant;
  
  // timer control

  int redraw_interval;
  int update_interval;

  // winds init
  
  const char *wind_marker_type;
  double wind_reference_speed;

};
 
#ifdef CIDD_MAIN
struct    Global_data gd;
// Attr_attribute  INSTANCE;
// Attr_attribute  MENU_KEY;
int INSTANCE;
int MENU_KEY;
#endif
    
//************************ External reference to global data structure *******/

#ifndef    CIDD_MAIN
extern    struct    Global_data    gd;
// extern Attr_attribute  INSTANCE;
// extern Attr_attribute  MENU_KEY;
extern int INSTANCE;
extern int MENU_KEY;
#endif


//*************************** Global values and macros ***********************/

#ifndef DEG_TO_RAD
#define DEG_TO_RAD  0.01745329251994372   // degrees to radians conversion
#endif

#define RADIAN90    1.570795      // radian value for 90 degrees 
#define RADIAN180   3.14159       // radian value for 180 degrees 
#define RADIAN270   4.712385      // radian value for 270 degrees 

#ifndef ABS
#define ABS(a)   (a)>=0 ? (a) : -(a)
#endif

// distance of (x,y) from origin 
#define R(x,y)  (sqrt((double)x*(double)x + (double)y*(double)y))

#endif

#include "cidd_funcs.h"

