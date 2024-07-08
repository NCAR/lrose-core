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

// system includes

#include <cmath>
#include <cerrno>
#include <fcntl.h>
#include <cstring>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <vector>

#include <X11/Xlib.h>
#include <X11/Xutil.h>       
#include <X11/Xresource.h>       

// LROSE includes

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
#include <toolsa/Path.hh> 
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
#include "Params.hh"

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
#include "cidd_funcs.h"     // CIDD specific function prototype definitions 

#include "RenderContext.hh"
#include "ProductMgr.hh"
#include "TimePlot.hh"

// GLOBAL DATA object

class Global_data {

public:
  
  // Command line arguments
  
  int argc;
  char **argv;

  Drawable hcan_xid;    
  Drawable vcan_xid;    

  int debug;        // Normal debugging flag  
  int debug1;       // More verbose debugging  flag 
  int debug2;       // Very verbose debug statements flag 

  string cacheDir;  // directory for storing cached temporary files
  string mapCacheDir;  // directory for storing cached map files
  string colorscaleCacheDir;  // directory for storing cached color scale files

  int display_projection;// Which projection CIDD Uses for display  - Enum

  int wsddm_mode;   // 1 =  End User/WSDDM  mode - 
  int quiet_mode;   // 1 =  Do not output normal startup and shutdown messages
  int report_mode;   // 1 =  Do continious data value reporting
  int run_unmapped;   // 1 runs unmapped
  int use_cosine_correction; // use cosine correction for computing range in polar data
  int drawing_mode;   // Flag for Internal drawing mode - FMQ output
  double product_detail_threshold[NUM_PRODUCT_DETAIL_THRESHOLDS];
  int product_detail_adjustment[NUM_PRODUCT_DETAIL_THRESHOLDS];
  
  int mark_latest_client_location; // place a mark at latest click location from client
  int forecast_mode;     // 1 = Apply forecast interval
  int data_format; // format of received data:  CART_DATA_FORMAT  
  int num_colors;       // Total colors allocated 
  int num_draw_colors;  // Total number of colors excluding color scale colors 
  int map_overlay_color_index_start;
  time_t last_event_time;  // the unix time of when the last activity occured
  time_t epoch_start;      //  The start of the movie loop
  time_t epoch_end ;       //  the end of the movie loop
  time_t model_run_time;  //  Which Model run to use.  When 0 = Ask for "lastest"
  time_t data_request_time; // Time for which Mdv data is requested
  int finished_init;    // 0 before xview init, 1 after.

  int num_datafields;   // the number of fields in the data.info file 
  int num_menu_fields;  // the number of items in the data field menus 
  int num_field_menu_cols; // number of columns in the data field menus 
  int num_map_overlays; // the number of overlays in the system 
  int num_bookmarks;    // the number of bookmarks defined
  int num_render_heights;// the number of heights to render in HTML mode
  int num_cache_zooms;   // the number of cached images for quick access
  int cur_render_height; // Current height being rendered in HTML mode
  int cur_field_set;     // Current set of fields that is active
  int save_im_win;       // Which window to dump: PLAN_VIEW, XSECT_VIEW or BOTH_VIEWS 
  int image_needs_saved; // Set to 1 when a finished image needs saved to disk
  int generate_filename; // Flag, indicating a new name needs generated.
  int max_time_list_span; // Maximum number of days in epoch to a request a time list 

  // State Variables
  int pan_in_progress;    // Set to 1 while panning 
  int zoom_in_progress;   // Set to 1 while zooming 
  int route_in_progress;  // Set to 1 while defining an ROUTE 
  int data_timeout_secs;  // number of seconds before request is canceled
  int data_status_changed;// 1 =  all data is now invalidated 
  int series_save_active; // When 1 - In series save mode.

  int num_field_labels;  // Current number of labels spaces occupied by field data labels 
  int field_index[MAX_DATA_FIELDS];   // menu item to field number lookup table 
  int movieframe_time_mode;  // mode for determining output file name

  double aspect_correction; // Aspect ratio correction for LAT-LON mode 
  double height_array[MAX_SECTS];  // Heights to render in HTML MODE

  // projections
  
  const char *projection_type;
  double proj_param[8];

  // start time etc

  const char *demo_time;
  
  // Files, names, Commands and other text parameters
  const char *orig_wd;           // Original working directory
  const char *frame_label;       // The default string to show on the frame 
  const char *app_name;          // Application name 
  const char *app_instance;      // Application Instance 

  char *data_info[MAX_DATA_FIELDS]; // information about each field  
  
  time_list_t gen_time_list;
  
  MdvxProj proj; // Display projection
  
  // X WINDOW Params

  win_param_t h_win;     // Plan view X window parameters
  win_param_t v_win;     // Cross Section (Vert) view X window parameters

  // Drawing contexts

  GC def_gc;       // default gc for copy & misc X operations  
  GC ol_gc;        // Gc for drawing in the reference reference overlay color 
  GC clear_ol_gc;  // Gc for Removing the reference overlay color 
  char *dpyName; // display name
  Display *dpy;     // default Display pointer for copy operations 
  Colormap cmap;
  Color_gc_t color[MAX_COLORS];   // stores all colors and GCs 
  Color_gc_t null_color;          // The color "transparent" 

  // FONTS
  int num_fonts;                  // number of fonts in use 
  Font ciddfont[MAX_FONTS];        // fonts in size order 
  XFontStruct *fontst[MAX_FONTS]; // Font info 

  // Symbolic products - features
  
  prod_info_t prod;
  ProductMgr *prod_mgr;       // Products Manager

  // gridded data records
  
  met_record_t *mrec[MAX_DATA_FIELDS];
  
  // Control variables for layers features
  
  layers_t layers;

  // Control variables for Legends
  
  legend_t legends;
  
  // map overlays
  
  vector<Overlay_t *> over;

  // Time scale plotter class for movie panel

  TimePlot *time_plot;

  // Control vartiables for movie looping
  
  movie_control_t movie;
  
  // I/O status infomation
  
  io_info_t io_info;

  // Dynamic Status message info
  
  status_msg_t status;
  
  // Symprod/Gdraw Contains Window, Domain geometries, etc
  
  RenderContext *r_context;
  
  // locations of surface stations
  
  StationLoc *station_loc;    // Station locator class

  // Control variables for Draw-Export feature

  draw_export_t draw;

  // Remote User Interface/Command class

  RemoteUIQueue *remote_ui;

  // Array of bookmarks
  
  bookmark_t *bookmark;
  
  // shared memory

  int coord_key;
  coord_export_t *coord_expt; // Pointer to exported coordinates & info

  // image copy

  int h_copy_flag;
  int v_copy_flag;

};

// global params

#ifdef THIS_IS_MAIN
Global_data gd; // global data
Params _params; // tdrp params
Path _paramsPathRequested;
Path _paramsPathUsed;
#else
// External reference to global data structure
extern Global_data gd; // global data
extern Params _params; // tdrp params
extern Path _paramsPathRequested;
extern Path _paramsPathUsed;
#endif

#endif

#include "cidd_funcs.h"

