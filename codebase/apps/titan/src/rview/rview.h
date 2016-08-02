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
#ifdef __cplusplus
 extern "C" {
#endif

/************************************************************************
 * rview.h : header file for rview program
 *
 * RAP, NCAR, Boulder CO
 *
 * June 1990
 *
 * Mike Dixon
 ************************************************************************/

/*
 **************************** includes *********************************
 */

#include <toolsa/umisc.h>
#include <rapmath/umath.h>
#include <rapplot/gplot.h>
#include <toolsa/servmap.h>
#include <toolsa/pmu.h>
#include <toolsa/mem.h>
#include <rapformats/titan_grid.h>
#include <rapformats/coord_export.h>
#include <titan/time_hist_shmem_legacy.h>
#include <cidd/cdata_util.h>
#include <titan/radar.h>
#include <titan/tdata_index.h>
#include <symprod/prod_queue.h>

/*
 ******************************** defines ********************************
 */

/*
 * timer interval in seconds
 */

#define BASE_TIMER_INTERVAL 1.0

/*
 * interval (secs) for updating main display
 */

#define UPDATE_INTERVAL 20.0

/*
 * interval (secs) for checking for new track data
 */

#define TRACK_CHECK_INTERVAL 2.0

/*
 * modes
 */

#define REALTIME TDATA_REALTIME
#define ARCHIVE TDATA_ARCHIVE

/*
 * draw scale legend options
 */

#define SCALE_NO_LEGENDS 0
#define SCALE_PLOT_LEGENDS 1

/*
 * map plotting
 */
   
#define MAPS_LIMITED 1
#define MAPS_ALL 2

/*
 * track, past and forecast plot options
 */

#define SELECTED_TRACK 1
#define ALL_TRACKS 2

#define ELLIPSES 1
#define POLYGONS 2

#define PAST_LIMITED 1
#define PAST_ALL 2

#define FUTURE_LIMITED 1
#define FUTURE_ALL 2

#define FORECAST_LIMITED 1
#define FORECAST_ALL 2

#define SPEED_ANNOTATION 1
#define MAX_DBZ_ANNOTATION 2
#define VIL_ANNOTATION 3
#define TOPS_ANNOTATION 4
#define NUMBER_ANNOTATION 5
#define N_ANNOTATIONS 6

#define PROJECTION "flat"

/*
 * forecast parameters
 */

#define N_FORECAST_STEPS (si32) 5
#define FORECAST_INTERVAL 6.0

/*
 * server mapper
 */

#define SERVMAP_HOST1_STR "stratus"
#define SERVMAP_HOST2_STR "cyclone"

/*
 * track server instance
 */

#define TRACK_SERVER_INSTANCE ""
#define TRACK_SERVER_SUBTYPE ""
#define TRACK_SERVER_DEFAULT_HOST ""
#define TRACK_SERVER_DEFAULT_PORT 20000L

/*
 * keys for semaphores and shared memory
 */

#define TRACK_SHMEM_KEY (si32) 44444

/*
 * directory and file name defaults
 */

#define COLORSCALE_DIR "colorscales"
#define PS_PROLOGUE_FILE "prologue.ps"
#define PS_PRINTER "ps"
#define PRINT_COMMAND "lpr -h -P"
#define OUTPUT_FILE "rview.ps"
#define TIME_HIST_COMMAND_LINE "time_hist &"

/*
 * Defaults for creating output files for use on the Web
 */

#define WEB_CONVERT_CMD "convert"

#define OUTPUT_CAPPI_WEB_FILES "false"
#define CAPPI_WEB_OUTPUT_INTERVAL 30
#define CAPPI_XWD_FILE "cappi.xwd"
#define CAPPI_WEB_FILE "cappi.tiff"

#define OUTPUT_VSECTION_WEB_FILES "false"
#define VSECTION_WEB_OUTPUT_INTERVAL 30
#define VSECTION_XWD_FILE "vsection.xwd"
#define VSECTION_WEB_FILE "vsection.tiff"

/*
 * defaults for options
 */

#define MODE "archive"
#define DEBUG_STR "false"
#define USE_TRACK_DATA "false"
#define USE_TIME_HIST "false"

#define PLOT_COMPOSITE "false"
#define PLOT_RINGS "false"
#define PLOT_MAPS "false"
#define PLOT_IMAGE "false"
#define PLOT_TRACKS "false"
#define PLOT_FLIGHT_TRACKS "false"
#define PLOT_CAPPI_CONTOURS "false"
#define PLOT_VSECTION_CONTOURS "false"
#define INTERPOLATE_VSECTION "false"
#define PLOT_FORECAST "false"
#define PLOT_PAST "false"
#define PLOT_FUTURE "false"
#define PLOT_CURRENT "true"
#define PLOT_VECTORS "true"
#define TRACK_GRAPHIC "ellipses"
#define FILL_GRAPHIC "false"
#define PLOT_RUNS "false"
#define FILL_RUNS "false"
#define RUNS_INCLUDED "false"
#define DRAW_COPY_TITLE "true"
#define DRAW_COPY_HEADER "true"
#define ANNOTATE_TRACKS "false"

#define PQ_STATUS_SHMEM_KEY 33333
#define PQ_BUFFER_SHMEM_KEY 33334

#define TIME_REQUESTED "1991/12/06_20:00:00"
#define Z_REQUESTED 2.0
#define VSECTION_INTERP_RES 0.5
#define FIELD_REQUESTED (si32) 1

#define DOUBLE_CLICK_DELAY 0.5

#define TRACK_DATA_TIME_MARGIN 9.0
#define PAST_PLOT_PERIOD 30.0
#define FUTURE_PLOT_PERIOD 30.0

#define MAX_MESSAGE_LEN (si32) 8192

/*
 * buttons
 */

#define N_CAPPI_BUTTONS 17

#define CAPPI_HELP 0
#define CAPPI_LEVEL 1
#define CAPPI_FIELD 2
#define CAPPI_ZOOM 3
#define CAPPI_CONT 4
#define CAPPI_IMAGE 5
#define CAPPI_RINGS 6
#define CAPPI_MAPS 7
#define CAPPI_TRACKS 8
#define CAPPI_TRACK_GRAPHIC 9
#define CAPPI_TRACK_ANNOTATE 10
#define CAPPI_PAST 11
#define CAPPI_FORECAST 12
#define CAPPI_FUTURE 13
#define CAPPI_TIME 14
#define CAPPI_COPY 15
#define CAPPI_QUIT 16

#define N_VSECTION_BUTTONS 4

#define VSECTION_INTERP 17
#define VSECTION_CONT 18
#define VSECTION_COPY 19
#define VSECTION_CLOSE 20

#define N_HELP_BUTTONS 1

#define HELP_CLOSE 21

/*
 * help codes for plot windows
 */

#define HELP_CAPPI_PLOT -2
#define HELP_VSECTION_PLOT -3

/*
 * window labels and sensitivity
 */

#define CAPPI_BUTTON_LABELS "Help Level Field Zoom Cont Image Rings Maps Tracks TType Annot Past Fcast Future Time Copy Quit"
#define CAPPI_BUTTON_REALTIME "1 1 1 1 1 1 1 1 1 1 1 1 1 1 0 1 1"
#define CAPPI_BUTTON_ARCHIVE "1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1"
#define CAPPI_BUTTON_NO_TRACKS "1 1 1 1 1 1 1 1 0 0 0 0 0 0 1 1 1"
#define CAPPI_BUTTON_TRACKS_OFF "1 1 1 1 1 1 1 1 1 0 0 0 0 0 1 1 1"
#define CAPPI_BUTTON_USE_TIME_HIST "1 1 1 1 1 1 1 1 1 1 1 1 1 1 0 1 1"

#define VSECTION_BUTTON_LABELS "Interp Cont Copy Close"
#define VSECTION_BUTTON_SENS "1 1 1 1"

#define HELP_BUTTON_LABELS "Close"
#define HELP_BUTTON_SENS "1"

/*
 * X server synchronization
 */

#define X_SYNC "false"

/*
 * full-grid data
 */

#define FULL_MIN_X -200.0
#define FULL_MIN_Y -200.0
#define FULL_MAX_X 200.0
#define FULL_MAX_Y 200.0

#define DELTA_Z 0.5

#define GRID_LAT 0.0
#define GRID_LON 0.0
#define GRID_ROT 0.0

/*
 * time between scans
 */

#define SCAN_DELTA_T 360.0

/*
 * x dimensions
 */

#define X_MAINX (si32) 15          /* default x position of main window */
#define X_MAINY (si32) 20          /* default y position of main window */
#define X_MAINWIDTH (ui32) 900     /* default width of main window */
#define X_MAINHEIGHT (ui32) 700    /* default height of main window */
#define X_MAINX_SIGN (si32) 1      /* sign of x position */
#define X_MAINY_SIGN (si32) 1      /* sign of y position */
#define X_MINMAINWIDTH (ui32) 300  /* min width of main window */
#define X_MINMAINHEIGHT (ui32) 250 /* min height of main window */

#define X_VSECTION_X_FROM_MAIN (si32) 50  /* x pixel offset of vsect window */
#define X_VSECTION_Y_FROM_MAIN (si32) 50  /* y pixel offset of vsect window */
#define X_VSECTION_WIDTH (si32) 500       /* default width of vsect window */
#define X_VSECTION_HEIGHT (si32) 500      /* default height of vsect window */

#define X_HELP_X_FROM_MAIN (si32) 50  /* x pixel offset of vsect window */
#define X_HELP_Y_FROM_MAIN (si32) 50  /* y pixel offset of vsect window */
#define X_HELP_WIDTH (si32) 500       /* default width of vsect window */
#define X_HELP_HEIGHT (si32) 500      /* default height of vsect window */

#define X_MAX_BUTTON_WIDTH (si32) 150     /* max button width */

#define X_MAINBORDER (si32) 5      /* border width of main window */
#define X_SUBBORDER (si32) 1       /* border width of sub windows */
#define X_SCALEWIDTH (si32) 100    /* width of scale sub-window */

#define X_LEGEND_ELEMENT_WIDTH (si32) 20
#define X_MAX_LEGEND_ELEMENT_HT (si32) 20

#define ARROW_HEAD_KM 1.0

/*
 * x margins in pixels
 */

#define X_XAXISMARGIN (si32) 30
#define X_YAXISMARGIN (si32) 50
#define X_XAXISENDMARGIN 20.0
#define X_YAXISENDMARGIN 20.0
#define X_TICKLENGTH 6.0
#define X_TOPMARGIN (si32) 30
#define X_TICK_CLEARANCE 0.45
#define X_TEXT_MARGIN 8.0

/*
 * colors
 */

#define X_FOREGROUND "white"
#define X_BACKGROUND "black"
#define X_TITLETEXT_COLOR "blue"
#define X_BUTTONTEXT_COLOR "blue"
#define X_SCALETEXT_COLOR "blue"
#define X_RING_COLOR "green"
#define X_CROSSHAIR_COLOR "red"
#define X_POS_CONTOUR_COLOR "red"
#define X_ZERO_CONTOUR_COLOR "magenta"
#define X_NEG_CONTOUR_COLOR "blue"
#define X_VSECTION_POS_COLOR "yellow"
#define X_ZOOM_BOX_COLOR "yellow"
#define X_TICK_COLOR "blue"
#define X_TICKLABEL_COLOR "red"
#define X_TRACK_ANNOTATION_COLOR "green"
#define X_TEXT_COLOR "red"
#define X_BORDER_COLOR "blue"
#define X_HLIGHT_BACKGROUND "orange"

#define X_PAST_STORM_COLOR "yellow"
#define X_PAST_STORM_COLOR_2 "gold"
#define X_CURRENT_STORM_COLOR "cyan"
#define X_FUTURE_STORM_COLOR "seagreen2"
#define X_FUTURE_STORM_COLOR_2 "seagreen"
#define X_FORECAST_STORM_COLOR "red"
#define X_FORECAST_STORM_COLOR_2 "gray50"

#define X_PAST_VECTOR_COLOR "magenta"
#define X_FUTURE_VECTOR_COLOR "seagreen2"
#define X_FORECAST_VECTOR_COLOR "red"

#define COLOR_DIM_PERCENT 60.0

/*
 * x line widths
 */

#define X_TICK_WIDTH (si32) 1
#define X_RING_WIDTH (si32) 1
#define X_CROSSHAIR_WIDTH (si32) 1
#define X_VSECTION_POS_WIDTH (si32) 1
#define X_ZOOM_BOX_WIDTH (si32) 1
#define X_CURRENT_STORM_LINE_WIDTH (si32) 2
#define X_PAST_STORM_LINE_WIDTH (si32) 1
#define X_FUTURE_STORM_LINE_WIDTH (si32) 1
#define X_FORECAST_STORM_LINE_WIDTH (si32) 1
#define X_STORM_VECTOR_WIDTH (si32) 1

/*
 * x fonts
 */

#define X_TITLE_FONT "fixed"
#define X_BUTTON_FONT "6x10"
#define X_SCALE_FONT "6x10"
#define X_RINGLABEL_FONT "6x10"
#define X_TICKLABEL_FONT "6x10"
#define X_TRACK_ANNOTATION_FONT "6x10"
#define X_TEXT_FONT "fixed"

/*
 * postscript page dimensions in inches
 */

#define PS_TITLEHEIGHT 0.5
#define PS_SCALEWIDTH 1.5
#define PS_IMAGEWIDTH 6.0
#define PS_IMAGEHEIGHT 6.0
#define PS_TITLE_TO_PLOT_MARGIN 0.4
#define PS_PLOT_TO_SCALE_MARGIN 0.25
#define PS_PRINTWIDTH 8.5   /* user selected width for hard copy */
#define PS_MAXPRINTWIDTH 11.0
#define PS_MINPRINTWIDTH 2.0

#define PS_LEGEND_ELEMENT_WIDTH 0.5
#define PS_MAX_LEGEND_ELEMENT_HT 0.3

/*
 * postscript line widths in points
 */

#define PS_TICK_WIDTH  1.0
#define PS_RING_WIDTH  1.0
#define PS_CROSSHAIR_WIDTH 1.0
#define PS_BORDER_WIDTH 1.0

/*
 * ps margins in units (inches if ps unitscale is set to 72)
 * PS_UNITSCALE is defined in "gutil.h"
 */

#define PS_XAXISMARGIN 0.4
#define PS_YAXISMARGIN 0.6
#define PS_XAXISENDMARGIN 0.6
#define PS_YAXISENDMARGIN 0.4
#define PS_TICKLENGTH 0.1
#define PS_TOPMARGIN 0.3
#define PS_TICK_CLEARANCE 0.4
#define PS_TEXT_MARGIN 0.1

/*
 * ps fonts
 */

#define PS_FONTNAME "Helvetica"
#define PS_TITLE_FONTSIZE 18.0
#define PS_SCALE_FONTSIZE 12.0
#define PS_RINGLABEL_FONTSIZE 8.0
#define PS_TICKLABEL_FONTSIZE 8.0
#define PS_TRACK_ANNOTATION_FONTSIZE 8.0
#define PS_TEXT_FONTSIZE 12.0

/*
 * ps linestyles - width, dash_length, space_length, <graylevel>
 */

#define PS_CURRENT_STORM_LINESTYLE "2 100.0 0.0 0.9"
#define PS_PAST_STORM_LINESTYLE "0 100.0 0.0 0.6"
#define PS_PAST_STORM_LINESTYLE_2 "0 100.0 0.0 0.8"
#define PS_FUTURE_STORM_LINESTYLE "1 1.0 1.0 0.4"
#define PS_FUTURE_STORM_LINESTYLE_2 "2 100.0 0.0 0.7"
#define PS_FORECAST_STORM_LINESTYLE "0 1.0 1.0 0.2"
#define PS_FORECAST_STORM_LINESTYLE_2 "0 1.0 1.0 0.6"

#define PS_PAST_VECTOR_LINESTYLE "0 100.0 0.0"
#define PS_FUTURE_VECTOR_LINESTYLE "1 100.0 0.0"
#define PS_FORECAST_VECTOR_LINESTYLE "1 1.0 1.0"

#define PS_POS_CONTOUR_LINESTYLE "0 100.0 0.0"
#define PS_ZERO_CONTOUR_LINESTYLE "1 100.0 0.0"
#define PS_NEG_CONTOUR_LINESTYLE "1 1.0 0.0"

/*
 * approx number of ticks per axis
 */

#define APPROX_NTICKS (si32) 7

#define NZOOM 3

/*
 ******************************** structures *****************************
 */

typedef struct {
  double min_x, min_y;
  double max_x, max_y;
  double range_x, range_y;
  int active;
  int current;
  Pixmap pixmap;
} zoom_t;

typedef struct {
  double req_start_x, req_start_y;
  double req_end_x, req_end_y;
  double start_x, start_y;
  double end_x, end_y;
  double min_z, max_z;
  double length;
  int active;
  int current;
  Pixmap pixmap;
} vsection_t;

/*
 * field control struct
 */

typedef struct {
  
  double contour_min;
  double contour_max;
  double contour_int;

  int server_used;
  
  si32 field;
  si32 time_window; /* the number of secs about the given time
		     * to search for data */

  char *description;
  char *x_colorscale_name;
  char *ps_colorscale_name;

  g_contour_t contours;
  g_color_scale_t *xcolors;
  g_color_scale_t *pscolors;

  cdata_index_t cdata_index;

} field_control_t;

/*
 * structure for globals
 */

typedef struct {

  int argc;
  char **argv;
    
  char *prog_name;                        /* program name */
  char *instance;                         /* program instance */
  char *params_path_name;                 /* params file path name */
  char *time_hist_command_line;           /* for starting storm time 
					   * history program */
  /*
   * track server
   */

  char *servmap_host1;                    /* server mapper hosts */
  char *servmap_host2;

  char *track_server_subtype;             /* subtype string to identify the
					   * track server */
  char *track_server_instance;            /* instance string to identify the
					   * track server */
  char *track_server_default_host;        /* default host for track server */
  int track_server_default_port;          /* default port for track server */

  si32 max_message_len;                   /* max message len for server data
					   * buffer */
  
  /*
   * status
   */

  si32 time;                              /* ref time (secs) */
  ui32 nfields;                         /* no of data fields */
  field_control_t *fcontrol;              /* field control array */
  zoom_t zoom[NZOOM];                     /* zoom area data structures */
  si32 zoom_level;                        /* from 0 (full) to
					   * NZOOM - 1 (max zoom) */
  vsection_t vsection;                    /* vertical section data struct */
  double z_requested;                     /* requested ht of initial cappi */
  double z_cappi;                         /* height of current cappi */

  /*
   * data server structs
   */

  cd_grid_info_t *cappi_info;
  cd_grid_info_t *vsection_info;
  tdata_index_t tdata_index;

  /*
   * shared memory
   */
  
  key_t track_shmem_key;                  /* key for track shared mem */

  coord_export_t *coord_export;           /* shared memory for 
					   * coord export */

  time_hist_shmem_t *track_shmem;         /* shared memory header
					   * for track data */

  pq_handle_t *pq_handle;                 /* handle for the product queue
					   * in shared memory */
  
  /*
   * display grid
   */

  titan_grid_t grid;
  titan_grid_comps_t grid_comps;
  
  /*
   * flags
   */

  int debug;                           /* debug on - true or false */

  int help;                            /* TRUE for help mode on */
  si32 help_code;                      /* code for which help is needed */

  int track_data_available;            /* flag to indicate if the 
					* track server had data */

  int use_track_data;                  /* flag for track data option */
  int use_time_hist;                   /* flag for time hist option */

  int cappi_active;                    /* set once windows allocated */

  int main_scale_invalid;

  int cappi_title_invalid;
  int cappi_requires_expose;

  int vsection_title_invalid;
  int vsection_requires_expose;

  int help_title_invalid;
  int help_requires_text;

  int plot_composite;                  /* composite plot flag - if set, 
					* the max values at all heights
					* are plotted on a plane */

  int plot_rings;                      /* plot rings flag */
  int plot_maps;                       /* plot maps flag */
  int plot_image;                      /* plot image flag */
  int plot_tracks;                     /* plot tracks flag */
  int plot_cappi_contours;             /* plot contours on cappi flag */
  int plot_vsection_contours;          /* plot contours on vsection flag */
  int vsection_interp;                 /* interpolate vsection */
  int plot_forecast;                   /* plot track forecast flag */
  int plot_past;                       /* plot track past flag */
  int plot_future;                     /* plot track future flag */
  int plot_current;                    /* plot track current flag */
  int plot_vectors;                    /* plot vectors */
  int track_graphic;                   /* graphic used for tracks */
  int fill_graphic;                    /* fill track graphic flag */
  int plot_runs;                       /* plot the storm runs */
  int fill_runs;                       /* storm runs - filled or not */
  int runs_included;                   /* include runs in track data */
  int draw_copy_title;                 /* draw title on copy flag */
  int draw_copy_header;                /* draw header on copy flag */
  int annotate_tracks;                 /* plot track numbers or speed */
  
  /*
   * options
   */
  
  int x_sync;                          /* x synchronization flag */
  int mode;                            /* realtime or archive */
  si32 field;                          /* data field number */

  double double_click_delay;           /* max double click delay
					* in millisecs */

  double print_width;                  /* width of copy */
  double color_dim_percent;            /* percent of full brightness
					* for dimmed colors */

  double vsection_interp_res;          /* vertical resolution to which the
					* vsection data is interpolated and
					* displayed */

  char *display_name;                  /* X display name */
  char *foregroundstr;
  char *backgroundstr;

  /* 
   * cursor - activated on button click in main window when
   * no zoom, pan or vsection is drawn
   */

  int cursor_active;
  double cursor_x;
  double cursor_y;
  double cursor_bearing;    /* bearing between the last two cursor clicks */
  double cursor_dist;       /* distance between the last two cursor clicks */
  int cursor_magnetic;      /* cursor direction in magnetic - true or false */
  int cursor_dist_nm;       /* cursor distance in nm - true or false */
  double magnetic_variation;

  /*
   * full-grid data
   */

  double full_min_x;
  double full_min_y;
  double full_max_x;
  double full_max_y;
  double full_range_x;
  double full_range_y;
  double full_aspect;
  double delta_z;

  /*
   * time between scans - secs
   */

  double scan_delta_t;

  /*
   * track display
   */

  double past_plot_period;             /* length of past track plotted
					* if PAST_LIMITED option selected
					* input as mins, stored as secs */

  double future_plot_period;           /* length of future track plotted
					* if FUTURE_LIMITED option selected
					* input as mins, stored as secs */

  /*
   * forecast parameters
   */

  si32 n_forecast_steps;               /* the number of steps for which the
					* forecast track positions are
					* displayed */

  double forecast_interval;            /* the interval between
					* the forecast steps - read in as
					* mins, converted to secs */
  
  /*
   * X declarations
   */
  
  int rscreen;                         /* screen id */
  Display *rdisplay;                   /* display device */
  Window main_window;                  /* main window */
  Window vsection_window;              /* vert section main window */
  Window help_window;                  /* help main window */
  Colormap cmap;

  /*
   * X graphics contexts
   */

  GC tick_gc;                          /* GC for tick marks */
  GC ring_gc;                          /* GC for rings */
  GC vsection_pos_gc;                  /* GC for vsection pos line */
  GC zoom_box_gc;                      /* GC for current zoom box */
  GC text_gc;                          /* GC for header in plot frame */
  GC ticklabel_gc;                     /* GC for axis labels */
  GC track_annotation_gc;              /* GC for track annotation */
  GC pos_contour_gc;                   /* GC for contouring positive values */
  GC zero_contour_gc;                  /* GC for contouring zero values */
  GC neg_contour_gc;                   /* GC for contouring negative values */
  GC crosshair_gc;                     /* GC for crosshair */
  GC copyarea_gc;                      /* GC for copying pixmap
					     to screen */
  GC pixmap_gc;                        /* for initializing pixmap */
  GC xor_gc;                           /* for rubberband lines */
  
  GC current_storm_gc;                 /* GC for current storm shapes */
  GC past_storm_gc[2];                 /* GC for past storm shapes */
  GC future_storm_gc[2];               /* GC for future storm shapes */
  GC forecast_storm_gc[2];             /* GC for forecast storm shapes */
  GC past_vector_gc;                   /* GC for past actual storm vector */
  GC future_vector_gc;                 /* GC for future actual storm vector */
  GC forecast_vector_gc;               /* GC for forecast storm vector */

  GC current_storm_dim_gc;      /* dimmed GC for current storm shapes */
  GC past_storm_dim_gc[2];      /* dimmed GC for previous storm shapes */
  GC future_storm_dim_gc[2];    /* dimmed GC for future storm shapes */
  GC forecast_storm_dim_gc[2];  /* dimmed GC for forecast storm shapes */
  GC past_vector_dim_gc;        /* dimmed GC for past actual storm vector */
  GC future_vector_dim_gc;      /* dimmed GC for future actual storm vector */
  GC forecast_vector_dim_gc;    /* dimmed GC for forecast storm vector */

  /*
   * color index struct
   */

  x_color_list_index_t color_index;   /* struct to hold the index to the
				       * color list entries */
  
  /*
   * postscript graphics contexts
   */

  psgc_t current_storm_psgc;    /* psgc for current storm shapes */
  psgc_t past_storm_psgc[2];    /* psgc for past storm shapes */
  psgc_t future_storm_psgc[2];  /* psgc for future storm shapes */
  psgc_t forecast_storm_psgc[2];/* psgc for forecast storm shapes */
  psgc_t past_vector_psgc;      /* psgc for past actual storm vectors */
  psgc_t future_vector_psgc;    /* psgc for future actual storm vectors */
  psgc_t forecast_vector_psgc;  /* psgc for forecast storm vectors */

  psgc_t pos_contour_psgc;      /* psgc for contouring positive values */
  psgc_t zero_contour_psgc;     /* psgc for contouring zero values */
  psgc_t neg_contour_psgc;      /* psgc for contouring negative values */
  
  /*
   * main window geometry
   */
  
  int mainx;
  int mainy;
  int mainx_sign;
  int mainy_sign;
  unsigned int mainwidth;
  unsigned int mainheight;

  int vsection_x;
  int vsection_y;
  ui32 vsection_width;
  ui32 vsection_height;

  int help_x;
  int help_y;
  ui32 help_width;
  ui32 help_height;

  ui32 max_button_width;

  /*
   * postscript page geometry
   */
  
  double ps_total_width;
  double ps_total_height;
  
  /*
   * fonts
   */
  
  XFontStruct *x_title_font;
  XFontStruct *x_button_font;
  XFontStruct *x_scale_font;
  XFontStruct *x_ringlabel_font;
  XFontStruct *x_ticklabel_font;
  XFontStruct *x_track_annotation_font;
  XFontStruct *x_text_font;
  
  double ps_title_fontsize;
  double ps_scale_fontsize;
  double ps_ringlabel_fontsize;
  double ps_ticklabel_fontsize;
  double ps_track_annotation_fontsize;
  double ps_text_fontsize;
  
  /*
   * colors
   */
  
  ui32 foreground;
  ui32 background;
  ui32 hlight_background;
  ui32 border_color;

  /*
   * frames for graphics
   */
  
  gframe_t *cappi_title_frame;
  gframe_t **cappi_button_frame;
  gframe_t *cappi_plot_frame;
  gframe_t *cappi_ps_plot_frame;

  gframe_t *main_scale_frame;

  gframe_t *vsection_title_frame;
  gframe_t **vsection_button_frame;
  gframe_t *vsection_plot_frame;
  gframe_t *vsection_ps_plot_frame;

  gframe_t *help_title_frame;
  gframe_t **help_button_frame;
  gframe_t *help_text_frame;

  gframe_t *vert_page_frame;
  gframe_t *horiz_page_frame;
  
} global_t;

/*
 * declare the global structure locally in the main,
 * and as an extern in all other routines
 */

#ifdef MAIN

global_t *Glob = NULL;

#else

extern global_t *Glob;

#endif

/*
 * function prototypes
 */

extern void clip_map_gcs(int dev, gframe_t *frame,
			 GRectangle *clip_rectangle);

extern void convert_xwd_to_web(char *xwd_file_path,
			       char *web_file_path);

extern void copy_cappi(void);

extern void copy_cappi_web(void);

extern void copy_vsection(void);

extern void copy_vsection_web(void);

extern void create_frames(void);

extern void draw_ac_posn(int dev,
			 gframe_t *frame,
			 date_time_t *cappi_time);

extern void draw_cappi_button(si32 n,
			      ui32 background);

extern void draw_cappi_plot(int dev,
			    g_color_scale_t *colors);

extern void draw_cappi_title(int dev);

extern void draw_flight_tracks(int dev,
			       gframe_t *frame,
			       date_time_t *cappi_time);

extern void draw_help_button(si32 n,
			     ui32 background);

extern void draw_help_text(void);

extern void draw_help_title(void);

extern void draw_main_scale(int dev,
			    g_color_scale_t *colors,
			    int symbol_legend_flag);

extern void draw_maps(int dev,
		      gframe_t *frame);

extern void draw_products(int dev,
			  gframe_t *frame,
			  date_time_t *cappi_time);

extern void draw_tracks(int dev,
			gframe_t *frame,
			date_time_t **track_time_ptr,
			double *dbz_threshold,
			si32 *n_tracks_plotted);

extern void draw_verify(int dev,
			gframe_t *frame,
			date_time_t *cappi_time);

extern void draw_vsection_button(si32 n,
				 ui32 background);

extern void draw_vsection_plot(int dev,
			       g_color_scale_t *colors);

extern void draw_vsection_title(int dev);

extern int entry_in_partial(tdata_basic_track_entry_t *entry);

extern void event_loop(void);

extern void expose_cappi_pixmap(void);

extern void expose_vsection_pixmap(void);

extern void free_map_gcs(void);

extern void free_map_fonts(void);

extern void free_resources(void);

extern void get_ac_idents(int *n_ident_p,
			  char ***ident_array_p,
			  char ***color_array_p);

extern void get_contour_intervals(void);

extern void get_init_data();

extern void get_ps_color_scales(void);

extern int get_radar_data(cd_command_t *command,
			  cd_reply_t *reply,
			  cd_grid_info_t *grid_info,
			  double **plane_heights,
			  ui08 **plane_data,
			  char *host_name,
			  int port,
			  char *prog_name);

extern int get_server_info(field_control_t *fcontrol,
			   si32 call_seq);

extern void get_track_data(void);

extern void get_x_color_scales(void);

extern GC get_aircraft_gc(int index);
extern psgc_t *get_aircraft_psgc(int index);
extern char *get_aircraft_ident(int index);
extern GC get_end_burn_gc(void);
extern GC get_bip_gc(void);
extern GC get_end_burn_and_bip_gc(void);
extern int get_plot_flares(void);

extern int handle_tserver_sigpipe(int sig);

extern int n_aircraft(void);

extern void parse_args(int argc,
		       char **argv);

extern void partial_track(void);

extern void print_copy(char *ps_file_path);

extern void read_field_control(void);

extern void read_map_files(void);

extern void read_params(void);

extern void render_cappi();

extern void render_vsection();

extern int respond_to_timer(int sig);

extern void safe_XFlush(Display *display);

extern void safe_XClearWindow(Display *display, Window w);

extern void set_cappi_invalid(void);

extern void set_cappi_sens(void);

extern void set_cappi_window_label(char *label);

extern void set_cappi_window_label_default(void);

extern void set_help_sens(void);

extern void set_timer(void);

extern void set_vsection_invalid(void);

extern void set_vsection_sens(void);

extern void set_xfonts(void);

extern void set_xgcs(void);

extern void setup_cappi_page(FILE *copy_file);

extern void setup_cappi_windows(void);

extern void setup_help_windows(void);

extern void setup_track_shmem(void);

extern void setup_vsection_page(FILE *copy_file);

extern void setup_vsection_windows(void);

extern void setup_x(void);

extern void tidy_and_exit(int sig);

extern void unclip_map_gcs(void);

extern void vsection_calc(int dev,
			  gframe_t *frame,
			  cd_grid_info_t *grid_info,
			  cd_reply_t *reply,
			  double *plane_heights);

extern int xerror_handler(Display *display,
			  XErrorEvent *err);

extern void zoom_calc(int dev,
		      gframe_t *frame);

#ifdef __cplusplus
}
#endif
