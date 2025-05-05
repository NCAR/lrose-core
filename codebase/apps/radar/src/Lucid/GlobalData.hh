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
//////////////////////////////////////////////////////////////////////////////
// Class for Lucid global data.
// This is a singleton.
//////////////////////////////////////////////////////////////////////////////
 
#ifndef GLOBAL_DATA_HH
#define GLOBAL_DATA_HH

#include <QPaintDevice>
#include <QBrush>
#include <qtplot/ColorMap.hh>

#include "Constants.hh"
#include "Params.hh"
#include <toolsa/mem.h>
#include <rapformats/coord_export.h>
#include <Mdv/MdvxProj.hh>

class ProductMgr;
class MdvReader;
class RenderContext;
class StationLoc;
class RemoteUIQueue;

/////////////////////////////////////////////////////////////////
// enums

typedef enum ops_mode_t {
  REALTIME_MODE = 0,
  ARCHIVE_MODE = 1
} ops_mode_t;

typedef enum wind_mode_t {
  WIND_MODE_ON = 0,     /* Wind vectors on in each frame */
  WIND_MODE_LAST = 1,   /* Wind vectors on only in the last frame */
  WIND_MODE_STILL =  2  /* Wind vectors on only in the last framef movie is off */
} wind_mode_t;

typedef enum wind_marker_t {
  ARROWS = 1,           // Centered Arrow
  TUFT = 2,             // Trailing Tuft
  BARB = 3,             // Simple Wind Barb - N Hemisphere
  VECTOR = 4,           // Arrow with base at data point
  TICKVECTOR = 5,       // Arrow with base at data point with time ticks
  LABELEDBARB = 6,      // Labeled Wind Barb N. Hemisphere 
  METBARB = 7,          // Barb labeled like upper air charts, hemisphere correct 
  BARB_SH = 8,          // S. Hemisphere Simple Barb
  LABELEDBARB_SH = 9    // S. Hemisphere Labeled Barb
} wind_marker_;

// Methods of rendering Gridded false colored data

typedef enum {
  POLYGONS = 0,
  FILLED_CONTOURS = 1,
  DYNAMIC_CONTOURS = 2,
  LINE_CONTOURS = 3
} render_method_t;
 
/////////////////////////////////////////////////////////////////
// worker classes

class Val_color_t {   /* data value to color mappings */
public:
  Val_color_t() {
    min = 1.0e99;
    max = -1.0e99;
    pixval = 0;
    cname[0] = '\0';
    label[0] = '\0';
  }
  double min,max;  /* data Range to map onto color */
  long pixval;     /* X value to use to draw in this color */
  QBrush brush;    /* A brush for this color */
  char cname[Constants::NAME_LENGTH];   /* color name    */
  char label[Constants::LABEL_LENGTH];  /* label to use (use min value if empty) */
};

class Valcolormap_t {
public:
  Valcolormap_t() {
    nentries = 0;
    MEM_zero(val_pix);
    MEM_zero(vc);
  }
  int nentries;
  long val_pix[Constants::MAX_COLOR_CELLS]; /* Pixel values to draw in */
  QBrush brush[Constants::MAX_COLOR_CELLS]; /* brush for each value */
  Val_color_t *vc[Constants::MAX_COLOR_CELLS];
};
 
class Color_gc_t {
public:
  Color_gc_t() {
    name[0] = '\0';
    pixval = 0;
    r = g = b = 0;
  }
  char name[Constants::NAME_LENGTH];
  long pixval;
  QBrush brush;
  unsigned short r,g,b; /* Full intensity color values for this color */
};

class contour_info_t {
public:
  contour_info_t() {
    field = 0;
    active = 0;
    labels_on = 0;
    num_levels = 0;
    min = 0.0;
    max = 0.0;
    interval = 0.0;
    color = nullptr;
    vcm = nullptr;
    MEM_zero(color_name);
  }
  int field;      /* global field number to contour */
  int active;
  int labels_on;
  int num_levels;
  double  min;
  double  max;
  double  interval;
  Color_gc_t *color;
  Valcolormap_t *vcm;
  char color_name[Constants::NAME_LENGTH];  /* Color name*/
};

class vert_spacing_t {    /* Vertical Spacing Information */
public:
  vert_spacing_t()
  {
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
  status_msg_t()
  {
    is_dynamic = 0;
    last_accessed = 0;
    status_fname = nullptr;
  }
  int is_dynamic;
  time_t last_accessed;
  string stat_msg;
  const char *status_fname;
};

typedef struct {
  double lat;
  double lon;
} world_pt_t;

class draw_export_info_t { // Draw-Export
public:
  draw_export_info_t()
  {
    default_serial_no = 0;
    default_valid_minutes = 0.0;
    data_time = 0;
    product_id_label = nullptr;
    product_label_text = nullptr;
    product_fmq_url = nullptr;
    param_line = nullptr;
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
  draw_export_t()
  {
    num_draw_products = 0;
    cur_draw_product = 0;
    dexport = nullptr;
  }
  int num_draw_products;
  int cur_draw_product;
  draw_export_info_t *dexport;
};

class time_list_t { // Time-List
public:
  time_list_t()
  {
    num_entries = 0;
    num_alloc_entries = 0;
    tim = nullptr;
  }
  unsigned int num_entries;
  unsigned int num_alloc_entries;
  time_t *tim;
};

class margin_t { /* Margin settings */
public:
  margin_t() {
    left = 0;
    top = 0;
    right = 0;
    bot = 0;
  }
  int left; /* pixels at left to avoid */
  int top; /* pixels at top to avoid */
  int right; /* pixels at right to avoid */
  int bot; /* pixels at bottom to avoid */
};

class draw_dim_t { /* Drawable dimensions */
public:
  draw_dim_t() {
    x_pos = 0;
    y_pos = 0;
    width = 0;
    height = 0;
    depth = 0;
    closed = 0;
  }
  int x_pos;
  int y_pos;
  int width;
  int height;
  int depth;
  int closed;
};

///////////////////////////////////////////////////////////////
// Wind Vectors

class wind_data_t {

public:

  wind_data_t() {
    active = 0;
    scale = 0;
    marker_type = ARROWS;      
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
  wind_marker_t marker_type;      
  int line_width;

  double units_scale_factor;
  double reference_speed;

  MdvReader *wind_u;
  MdvReader *wind_v;
  MdvReader *wind_w;
  
  char *data_info; /* Data source info string */
  char color_name[Constants::NAME_LENGTH];  /* Color name*/
  const char *units_label;  /* Color name*/
  
  Color_gc_t *color;

};

/////////////////////////////////////////////////////////////////////
// data layers

class earth_data_t
{
public:
  earth_data_t() {
    terrain_active = 0;
    landuse_active = 0;
    terr = nullptr;
    land_use = nullptr;
    color1 = nullptr;
    color2 = nullptr;
  }
  int terrain_active;       /* if True - draw terrain features */
  int landuse_active;       /* if True - draw landuse features */
  MdvReader *terr;          // terrain
  MdvReader *land_use;
  Color_gc_t *color1;  // skin color
  Color_gc_t *color2;  // core color
};

class route_track_t {  // Route track
public:
  route_track_t() {
    num_segments = 0;
    total_length = 0.0;
    MEM_zero(x_pos);
    MEM_zero(y_pos);
    MEM_zero(x_world);
    MEM_zero(y_world);
    MEM_zero(lat);
    MEM_zero(lon);
    MEM_zero(seg_length);
    MEM_zero(navaid_id);
    MEM_zero(route_label);
  }
  int num_segments;
  double total_length; // Distance in KM of all segments
  int x_pos[Constants::MAX_ROUTE]; // pixel coords
  int y_pos[Constants::MAX_ROUTE];
  double x_world[Constants::MAX_ROUTE];   // proj coords
  double y_world[Constants::MAX_ROUTE];
  double lat[Constants::MAX_ROUTE];   // world coords
  double lon[Constants::MAX_ROUTE];
  double seg_length[Constants::MAX_ROUTE];
  char navaid_id[Constants::MAX_ROUTE][16];
  char route_label[64];
};

class route_data_t {
public:
  route_data_t() {
    has_params = 0;
    active_route = 0;
    num_predef_routes = 0;
    route = nullptr;
    u_wind = nullptr;
    v_wind = nullptr;
  }
  int has_params;      // 1 =  Parameters exist in configuration file
  int active_route;    // which route is active.
  int num_predef_routes;
  route_track_t *route;// Predefined routes
  MdvReader *u_wind;
  MdvReader *v_wind;
}; 

class prod_detail_thresh_t
{
public:
  prod_detail_thresh_t() {
    threshold = 0.0;
    adjustment = 0;
  }
  double threshold;
  int adjustment;
};

class prod_info_t
{
public:
  prod_info_t() {
    products_on = 0;
    prod_line_width = 0;
    prod_font_num = 0;    
    MEM_zero(detail);
  }
  int products_on;     /* Flag to turn products on/off globally */
  int prod_line_width; /* How wide to make the lines */
  int prod_font_num;    
  prod_detail_thresh_t detail[Constants::NUM_PRODUCT_DETAIL_THRESHOLDS];
};

class layers_t {

public:
  
  layers_t() {
    wind_vectors = 0;
    init_state_wind_vectors = 0;
    set_field_mode = 0;
    cur_overlay_layer = 0;
    cur_contour_layer = 0;
    cur_map_overlay = 0;
    cur_wind_overlay = 0;
    layer_legends_on = 0;
    cont_legends_on = 0;
    wind_legends_on = 0;
    contour_line_width = 0;
    smooth_contours = 0;
    use_alt_contours = 0;
    add_noise = 0;
    map_bad_to_min_value = 0;
    map_missing_to_min_value = 0;
    num_wind_sets = 0;
    wind_mode = WIND_MODE_ON;
    wind_scaler = 0;
    wind_time_scale_interval = 0;
    special_contour_value = 0;
    MEM_zero(overlay_field_on);
    MEM_zero(overlay_field);
    MEM_zero(overlay_min);
    MEM_zero(overlay_max);
    missing_data_color = nullptr;
    bad_data_color = nullptr;
  }
  
  int wind_vectors;   /* Flag to turn on/off wind vectors as a whole */
  int init_state_wind_vectors;  /* starting state of wind_vectors */
  int set_field_mode;  /* 1 = set contour field parameters, 2 = set overlay field params */
  
  int cur_overlay_layer;    /* Which layer we are setting 0 - NUM_GRID_LAYERS -1 */
  int cur_contour_layer;    /* Which layer we are setting 0 - NUM_CONT_LAYERS -1 */
  int cur_map_overlay;      /* Which map overlay is being manipulated */
  int cur_wind_overlay;     /* Which wind set is being manipulated */   
  
  int layer_legends_on;     // Control variables for plotting legend/Titles
  int cont_legends_on;
  int wind_legends_on;
  
  int contour_line_width;
  int smooth_contours;     // Apply smoothing before contouring ;  0-2 are valid values 
  int use_alt_contours;    // 1 =  Use the Alternate Contouring routines 
  int add_noise;           // Add 1 part in 250 of noise to contours 1 == true
  int map_bad_to_min_value; // 1 == true
  int map_missing_to_min_value; // 1 == true
  
  int overlay_field_on[Constants::NUM_GRID_LAYERS];  /* Flag to turn on overlaid grid */
  int overlay_field[Constants::NUM_GRID_LAYERS];  /* Which field to render */
  
  int num_wind_sets;    /* */
  wind_mode_t wind_mode;  /* wind display mode */
  int wind_scaler; // Time displayed is wind_scaler * wind_time_scale_interval
  double wind_time_scale_interval; /* minutes interval-  wind vector scale */
  
  vector<wind_data_t> wind; /* Pointer to Array of wind set info */
  
  route_data_t route_wind; // Route winds data and info
  
  earth_data_t earth; // Terrain/ Land use data and  info
  
  contour_info_t cont[Constants::NUM_CONT_LAYERS];
  
  prod_info_t prod;
  
  double overlay_min[Constants::NUM_GRID_LAYERS]; /* Clip all values in grid below this */
  double overlay_max[Constants::NUM_GRID_LAYERS]; /* Clip all values in grid above this */
  
  Color_gc_t *missing_data_color;  // Used for Displaying missing data
  Color_gc_t *bad_data_color;      // Used for Displaying bad  data
  
  double special_contour_value;      // which value to draw wider 
  
};

// window data - for each display mode, horiz and vcert

class win_param_t {

public:
  
  win_param_t()
  {
    
    ip = nullptr;
    
    active = 0;
    page = 0;
    prev_page = 0;
    movie_page = 0;
    min_height = 0;
    min_width = 0;
    zoom_level = 0;
    prev_zoom_level = 0;
    cur_cache_im = 0;
    last_cache_im = 0;
    num_zoom_levels = 0;
    start_zoom_level = 0;
    MEM_zero(redraw_flag);
    selectionChanged = 0;
    
    legends_start_x = 0;
    legends_start_y = 0;
    legends_delta_y = 0;
    
    origin_lat = 0.0;
    origin_lon = 0.0;
    reset_click_lon = 0.0;
    reset_click_lat = 0.0;
    min_x = max_x = 0.0;
    min_y = max_y = 0.0;
    min_lat = max_lat = 0.0;
    min_lon = max_lon = 0.0;
    min_ht = max_ht = 0.0;
    min_r = max_r = 0.0;
    min_deg = max_deg = 0.0;
    cmin_x = cmax_x = 0.0;
    cmin_y = cmax_y = 0.0;
    cur_ht = 0.0;
    
    km_across_screen = 0.0;
    
    // MEM_zero(route);
    
    vis_pdev = nullptr;
    can_pdev = nullptr;
    tmp_pdev = nullptr;
    MEM_zero(page_pdev);
    
    MEM_zero(title);
    MEM_zero(image_dir);
    MEM_zero(image_fname);
    MEM_zero(image_command);
    
  }
  
  int *ip; /* instance pointer; hook to XView object structures*/
  int active; /* set to 1 if window is currently active */
  int page; /* Current page selected in the window */
  int prev_page; /* The last page viewed */
  int movie_page; /* The page currently viewed in movie loops */
  int min_height; /* minimum height window is allowed to get to */
  int min_width; /* minimum width window is allowed to get to */
  int zoom_level; /* index to use for zoom boundary coords */
  int prev_zoom_level; /* index to use for zoom boundary coords */
  int cur_cache_im; /* index to current XID Cache */
  int last_cache_im; /* index to last XID Cache */
  int num_zoom_levels; /* number of stored zoom levels */
  int start_zoom_level; /* The starting zoom level - for reset function */
  int redraw_flag[Constants::MAX_DATA_FIELDS]; /* set to 1 when field needs re-rendered */
  int selectionChanged;
  
  int legends_start_x; /* the X value for the first legend character */
  int legends_start_y; /* the Y value for the first legend */
  int legends_delta_y; /* the Y spacing between the legends */

  double origin_lat; /* Latitude origin of the window's coord system */
  double origin_lon; /* Longitude origin of the window's coord system */
  double reset_click_lon; /* Latitude */
  double reset_click_lat; /* Longitude */

  double min_x,max_x; /* Km limits of full display area */
  double min_y,max_y; /* Km limits of full display area */
  double min_lat,max_lat; /* Globe limits of full display area */
  double min_lon,max_lon; /* Globe limits of full display area */
  double min_ht,max_ht; /* Km limits of full display area */
  double min_r,max_r; /* Radial r limits of full display area */

  double min_deg,max_deg;/* Radial deg limits of full display area */

  double cmin_x,cmax_x; /* X limits of current display area */
  double cmin_y,cmax_y; /* Y limits of current display area */
  double cur_ht; /* Z Height of current display area */

  double km_across_screen; /* Approx. distance across the window (KM) */

  route_track_t route;

  // Drawable vis_xid; /* X ID of Visible canvas */
  // Drawable *can_xid; /* X ID of last stage canvases to draw products on top of */
  // Drawable tmp_xid; /* X ID of area to drap fields that aren't updated */
  // Drawable page_xid[MAX_DATA_FIELDS]; /* draw Pixmap for each field */

  QPaintDevice *vis_pdev; /* X ID of Visible canvas */
  QPixmap **can_pdev; /* X ID of last stage canvases to draw products on top of */
  QPixmap *tmp_pdev; /* X ID of area to draw fields that aren't updated */
  QPixmap *page_pdev[Constants::MAX_DATA_FIELDS]; /* draw Pixmap for each field */

  draw_dim_t win_dim; /* Window dimensions and position */
  draw_dim_t can_dim; /* Canvas dimensions and position */
  draw_dim_t img_dim; /* Image Pixmaps dimensions and positions */
  margin_t margin; /* Canvas Margin dimensions */

  char title[Constants::MAX_TITLE];
  char image_dir[Constants::MAX_PATH];
  char image_fname[Constants::MAX_PATH];
  char image_command[Constants::MAX_PATH];
	 
};

////////////////////////////////////////////////////////////////
// movie loops

class movie_frame_t
{
public:
  movie_frame_t() {
    init();
  }
  movie_frame_t(const movie_frame_t &rhs) {
    h_pdev = rhs.h_pdev;
    v_pdev = rhs.v_pdev;
    time_start = rhs.time_start;
    time_end = rhs.time_end;
    time_mid = rhs.time_mid;
    redraw_horiz = rhs.redraw_horiz;
    redraw_vert = rhs.redraw_vert;
    memcpy(fname, rhs.fname, Constants::NAME_LENGTH);
    memcpy(vfname, rhs.vfname, Constants::NAME_LENGTH);
  }
  void init() {
    h_pdev = nullptr;
    v_pdev = nullptr;
    time_start = 0;
    time_end = 0;
    time_mid = 0;
    redraw_horiz = 0;
    redraw_vert = 0;
    MEM_zero(fname);
    MEM_zero(vfname);
  }
  QPixmap *h_pdev;       /* A memory pixmap for movie frames */
  QPixmap *v_pdev;       /* A memory pixmap for movie frames */
  time_t time_start;  /* Time at starting point of image */
  time_t time_end;    /* Time at ending point of image */
  time_t time_mid;    /* Time at mid point of image */
  int redraw_horiz;       /* 1 = rerender */
  int redraw_vert;        /* 1 = rerender */
  char fname[Constants::NAME_LENGTH];   /* file name for this image */
  char vfname[Constants::NAME_LENGTH];  /* file name for this image */
};
     

class movie_control_t
{

public:

  movie_control_t() {
    active = 0;
    mode = 0;
    magnify_mode = 0;
    sweep_on = 0;
    sweep_dir = 0;
    display_time_msec = 0;
    num_frames = 0;
    start_frame = 0;
    end_frame = 0;
    cur_frame = 0;
    last_frame = 0;
    movie_on = 0;
    delay = 0;
    round_to_seconds = 0;
    magnify_factor = 0.0;
    time_interval_mins = 0.0;
    mr_stretch_factor = 0.0;
    start_time = 0;
    demo_time = 0;
    demo_mode = 0;
    reset_frames = 0;
    climo_mode = 0;
    forecast_interval = 0.0;
    past_interval = 0.0;
    frame_span = 0.0;
  }

  int active; /* Set to 1 when movie panel is open */
  int mode; /* 0= realtime mode, 1= archive mode, 2 = elevation movie */
  int magnify_mode; /* 1 = Magnify mode  turned on - When setting forecast mode. */
  int sweep_on; /*  1 = Sweep instead of loop */
  int sweep_dir; /*  Either +1 (forward)  or -1 (backward) */
  int display_time_msec; /* TIme each movie frame should be displayed - msecs */
  int num_frames; /* Total number of frames in loop */
  int start_frame; /* The frame to start with */
  int end_frame; /* The frame to end with */
  int cur_frame; /* the currently visible frame number */
  int last_frame; /* The last frame visible */
  int movie_on; /* 0 = off, Other: movie is on */
  int delay; /* Number of frame ticks to delay at end of loop */
  int round_to_seconds; /* The number of seconds to round data times to */
  int demo_mode; /* Starts up in demo mode */
  int reset_frames; /* Reset valid flags on frames when frame index updates */
  int climo_mode; /* Climotology mode - restricts the time span of data requests. */
  double magnify_factor; /* Amount to magnify time scales when switching into forecast mode */
  double time_interval_mins; /* in minutes */
  double mr_stretch_factor; /* Factor to mult time_interval by for most recent data */
  double forecast_interval; /* Interval to display menu options into the future in hours */
  double past_interval; /* Interval to display menu options into the past in hours */
  double frame_span; /* minutes spanning one movie frame - Used for climo mode  */
  time_t start_time; /* Time of the first frame */
  time_t demo_time; /* First frame in demo mode */
  movie_frame_t frame[Constants::MAX_FRAMES]; /* info about each frame */

};

///////////////////////////////////////////////////////////////////////
// map overlays

/* geographic feature Icon & label */

class Geo_feat_icondef_t {
public:
  Geo_feat_icondef_t() {
    num_points = 0;
    x = nullptr;
    y = nullptr;
    MEM_zero(name);
  }
  int num_points;
  short *x;
  short *y;
  char name[Constants::NAME_LENGTH];
};

class Geo_feat_label_t {
public:
  Geo_feat_label_t() {
    min_lat = 0.0;
    min_lon = 0.0;
    max_lat = 0.0;
    max_lon = 0.0;
    attach_lat = 0.0;
    attach_lon = 0.0;
    proj_x = 0.0;
    proj_y = 0.0;
    rotation = 0.0;
    MEM_zero(display_string);
  }
  double min_lat; /* latitude, longitude bounding box for text */
  double min_lon;
  double max_lat;
  double max_lon;
  double attach_lat;
  double attach_lon; /* latitude, longitude of the object associated with the label */
  double proj_x;
  double proj_y;
  double rotation; /* 0 = left to right, 90 = bottom to top, etc */
  char display_string[Constants::NAME_LENGTH]; /* String to display */
};

/* geographic feature Icon & label */

class Geo_feat_icon_t {
public:
  Geo_feat_icon_t() {
    lat = 0.0;
    lon = 0.0;
    proj_x = 0.0;
    proj_y = 0.0;
    text_x = 0;
    text_y = 0;
    MEM_zero(label);
    icon = nullptr;
  }
  double lat;
  double lon;
  double proj_x;
  double proj_y;
  short text_x;
  short text_y;
  char label[Constants::LABEL_LENGTH]; /* Label to display */
  Geo_feat_icondef_t *icon;
};

/* geographic feature lines */

class Geo_feat_polyline_t {
public:
  Geo_feat_polyline_t() {
    num_points = 0;
    min_x = 0.0;
    max_x = 0.0;
    min_y = 0.0;
    max_y = 0.0;
    lat = nullptr;
    lon = nullptr;
    proj_x = nullptr;
    proj_y = nullptr;
    MEM_zero(label);
  }
  int num_points;
  double min_x; /* Local coords bounding box */
  double max_x;
  double min_y;
  double max_y;
  double *lat;
  double *lon;
  double *proj_x;
  double *proj_y;
  char label[Constants::LABEL_LENGTH]; /* Label of polyline */
};

/* geographic feature coordinates & label */

class Geo_feat_coord_t {
public:
  Geo_feat_coord_t() {
    lab_pos = 0;
    lat = 0.0;
    lon = 0.0;
    MEM_zero(label);
  }
  int lab_pos; /* 1 = rt, 2 = top 3 = left, 4 = bot */
  double lat;
  double lon; /* latitude, longitude */
  char label[Constants::LABEL_LENGTH]; /* String to label coordinate with */
};

/* Arbitrary geographic coordinate */
class Geo_coord_t {
public:
  Geo_coord_t() {
    x = 0.0;
    y = 0.0;
  }
  double x;
  double y;
};
 
// main map overlay struct

class MapOverlay_t { /* Overlay data */

public:

  MapOverlay_t() {
    active = 0;
    default_on_state = 0;
    num_labels = 0;
    num_icons = 0;
    num_icondefs = 0;
    num_polylines = 0;
    num_alloc_labels = 0;
    num_alloc_icons = 0;
    num_alloc_icondefs = 0;
    num_alloc_polylines = 0;
    pixval = 0;
    line_width = 0;
    detail_thresh_min = 0;
    detail_thresh_max = 0;
    geo_label = NULL;
    geo_icon = NULL;
    geo_icondef = NULL;
    geo_polyline = NULL;
    color = NULL;
    mapParams = NULL;
  }
  
  long active; /* Current on/off state; 1 = active */
  long default_on_state; /* If set to 1, This map should appear by default */

  long num_labels; /* number of associated text strings in map overlay */
  long num_icons; /* number of icons in map overlay */
  long num_icondefs; /* number of icons in map overlay */
  long num_polylines; /* number of polylines in map overlay */

  long num_alloc_labels; /* number of allocated pointers for labels*/
  long num_alloc_icons; /* number of allocated pointers for number of icons */
  long num_alloc_icondefs; /* number of allocated pointers for number of icons */
  long num_alloc_polylines; /* number of allocated pointers for number of polylines */

  long pixval; /* X color cell value to use to draw in the proper color */
  long line_width; /* How wide to draw the line */

  double detail_thresh_min; /* Overlays are visible when distance (km) across */
  double detail_thresh_max; /* across the screen is between min and max */

  Geo_feat_label_t **geo_label;
  Geo_feat_icon_t **geo_icon;
  Geo_feat_icondef_t **geo_icondef;
  Geo_feat_polyline_t **geo_polyline;

  string map_file_name; /* Name of map file to read */
  string control_label; /* The overlay's GUI label */
  string map_name; /* Long ID */
  string map_code; /* SHORT NAME */
  string color_name; /* Current Color */

  Color_gc_t *color;

  Params::map_t *mapParams;

};

////////////////////////////////////////////////////////////
// GLOBAL DATA object - singleton

class GlobalData {
  
public:

  // destructor
  
  virtual ~GlobalData();
  
  // Retrieve the singleton instance of this class.
  
  static GlobalData &Instance() {
    if (_instance == nullptr) {
      _instance = new GlobalData;
    }
    return *_instance;
  }

  // Command line arguments
  
  QPaintDevice *hcan_pdev;    
  QPaintDevice *vcan_pdev;    

  int debug;        // Normal debugging flag  
  int debug1;       // More verbose debugging  flag 
  int debug2;       // Very verbose debug statements flag 

  string cacheDir;  // directory for storing cached temporary files
  string mapCacheDir;  // directory for storing cached map files
  string colorscaleCacheDir;  // directory for storing cached color scale files
  string stationlocCacheDir;  // directory for storing cached station locator file

  int display_projection;// Which projection CIDD Uses for display  - Enum

  int quiet_mode;   // 1 =  Do not output normal startup and shutdown messages
  int report_mode;   // 1 =  Do continious data value reporting
  int run_unmapped;   // 1 runs unmapped
  int use_cosine_correction; // use cosine correction for computing range in polar data
  int drawing_mode;   // Flag for Internal drawing mode - FMQ output
  double product_detail_threshold[Constants::NUM_PRODUCT_DETAIL_THRESHOLDS];
  int product_detail_adjustment[Constants::NUM_PRODUCT_DETAIL_THRESHOLDS];
  
  int mark_latest_client_location; // place a mark at latest click location from client
  int forecast_mode;     // 1 = Apply forecast interval
  int data_format; // format of received data:  CART_DATA_FORMAT  
  int num_colors;       // Total colors allocated 
  int num_draw_colors;  // Total number of colors excluding color scale colors 
  int map_overlay_color_index_start;
  int finished_init;    // 0 before xview init, 1 after.

  int num_datafields;   // the number of fields in the data.info file 
  int num_menu_fields;  // the number of items in the data field menus 
  int num_field_menu_cols; // number of columns in the data field menus 
  int num_map_overlays; // the number of overlays in the system 
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
  int field_index[Constants::MAX_DATA_FIELDS];   // menu item to field number lookup table 
  int movieframe_time_mode;  // mode for determining output file name

  double aspect_correction; // Aspect ratio correction for LAT-LON mode 
  double height_array[Constants::MAX_SECTS];  // Heights to render in HTML MODE

  // selected values

  // bool redraw_horiz;
  bool redraw_vert;
  bool time_has_changed;
  bool field_has_changed;
  bool zoom_has_changed;
  bool vsect_has_changed;
  bool ht_has_changed;

  time_t prev_time;
  int prev_field;
  double prev_ht;

  time_t selected_time;
  int selected_field;
  double selected_ht;

  time_t last_event_time;  // the unix time of when the last activity occured
  time_t epoch_start;      //  The start of the movie loop
  time_t epoch_end ;       //  the end of the movie loop
  time_t model_run_time;  //  Which Model run to use.  When 0 = Ask for "lastest"
  time_t data_request_time; // Time for which Mdv data is requested

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

  char *data_info[Constants::MAX_DATA_FIELDS]; // information about each field  
  
  time_list_t gen_time_list;
  
  MdvxProj proj; // Display projection
  
  // X WINDOW Params

  win_param_t h_win;     // Plan view X window parameters
  win_param_t v_win;     // Cross Section (Vert) view X window parameters

  // Drawing contexts

  QBrush def_brush;       // default brush for copy & misc X operations  
  QBrush ol_brush;        // Brush for drawing in the reference reference overlay color 
  QBrush clear_ol_brush;  // Brush for Removing the reference overlay color 

  // char *dpyName; // display name
  // Display *dpy;     // default Display pointer for copy operations 
  ColorMap cmap;
  Color_gc_t color[Constants::MAX_COLORS];   // stores all colors and GCs 
  Color_gc_t null_color;          // The color "transparent" 

  // FONTS
  // int num_fonts;                  // number of fonts in use 
  // QFont ciddfont[Constants::MAX_FONTS];        // fonts in size order 
  // QFont *fontst[Constants::MAX_FONTS]; // Font info 

  // Symbolic products - features
  
  prod_info_t prod;
  ProductMgr *prod_mgr;       // Products Manager

  // gridded data records
  
  MdvReader *mread[Constants::MAX_DATA_FIELDS];
  
  // Control variables for layers features
  
  layers_t layers;

  // Control variables for Legends
  
  // legend_t legends;
  
  // map overlays
  
  vector<MapOverlay_t *> overlays;

  // Time scale plotter class for movie panel

  // TimePlot *time_plot;

  // Control vartiables for movie looping
  
  movie_control_t movie;
  
  // I/O status infomation
  
  // io_info_t io_info;

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
  
  // bookmark_t *bookmark;
  
  // shared memory

  int coord_key;
  coord_export_t *coord_expt; // Pointer to exported coordinates & info

  // image copy

  int h_copy_flag;
  int v_copy_flag;

private:
  
  // constructor - private so it cannot be called directly
  
  GlobalData();

  // instance

  static GlobalData *_instance;
  
};

#endif

