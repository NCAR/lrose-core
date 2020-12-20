// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 

/************************************************************************
 * TimeHist.hh : header file for TimeHist program
 *
 * RAP, NCAR, Boulder CO
 *
 * April 1991
 * Converted to C++ Feb 2001
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
#include <titan/time_hist_shmem.h>
#include <rapformats/coord_export.h>
#include <titan/track.h>
#include <titan/tdata_index.h>
#include <titan/DsTitan.hh>
#include <dsserver/DsLdataInfo.hh>
using namespace std;

/*
 ******************************** defines ********************************
 */

/*
 * interval (secs) for checking for new data or a change in track
 */

#define BASE_TIMER_INTERVAL 0.25
  
/*
 * directory and file name defaults
 */

#define COLORSCALE_DIR "color_scales"
#define PS_PROLOGUE_FILE "prologue.ps"
#define PS_PRINTER "lp"
#define PRINT_COMMAND "lpr -h -P"
#define OUTPUT_FILE "TimeHist.ps"

/*
 * status values
 */

#define CURRENT 0
#define NOT_CURRENT 1
#define NEEDS_EXPOSE 2
#define NEEDS_DATA 3
#define NEEDS_REDRAW 4

#define SELECTED_LIMITED 1
#define SELECTED_ALL 2

/*
 * time history fields
 */

typedef enum {
  THIST_VOL_FIELD,
  THIST_AREA_FIELD,
  THIST_PFLUX_FIELD,
  THIST_MASS_FIELD,
  THIST_VIL_FIELD,
  THIST_N_FIELDS
} thist_field_t;
  
/*
 * time history fields
 */

typedef enum {
  UNION_0_FIELD,
  UNION_1_FIELD,
  UNION_2_FIELD,
  UNION_3_FIELD,
  UNION_N_FIELDS
} union_field_t;

/*
 * main products
 */

typedef enum {
  THIST_PLOT,
  TIMEHT_PLOT,
  RDIST_PLOT,
  UNION_PLOT,
  N_PLOTS
} plot_t;

/*
 * union plot details
 */

typedef enum {
  UNION_MODE_FLOATS,
  UNION_MODE_HAIL,
  N_UNION_MODES
} union_mode_t;

/*
 * buttons
 */

#define N_TSCALE_BUTTONS 12

#define TSCALE_HELP 0
#define TSCALE_CASES 1
#define TSCALE_SELECT 2
#define TSCALE_TRACK_SET 3
#define TSCALE_YEAR 4
#define TSCALE_30DAYS 5
#define TSCALE_5DAYS 6
#define TSCALE_1DAY 7
#define TSCALE_1HR 8
#define TSCALE_TIME 9
#define TSCALE_NOW 10
#define TSCALE_QUIT 11

#define N_THIST_BUTTONS 8

#define THIST_VOL 12
#define THIST_AREA 13
#define THIST_PFLUX 14
#define THIST_MASS 15
#define THIST_VIL 16
#define THIST_FORECAST 17
#define THIST_FIT 18
#define THIST_COPY 19

#define N_TIMEHT_BUTTONS 7

#define TIMEHT_MAXZ 20
#define TIMEHT_MEANZ 21
#define TIMEHT_MASS 22
#define TIMEHT_VORTICITY 23
#define TIMEHT_CENTROIDS 24
#define TIMEHT_HTMAXZ 25
#define TIMEHT_COPY 26

#define N_RDIST_BUTTONS 5

#define RDIST_VOL 27
#define RDIST_AREA 28
#define RDIST_FLIP 29
#define RDIST_FIT 30
#define RDIST_COPY 31

#define N_UNION_BUTTONS 6

#define UNION_0 32
#define UNION_1 33
#define UNION_2 34
#define UNION_3 35
#define UNION_FIT 36
#define UNION_COPY 37

#define N_HELP_BUTTONS 1

#define HELP_CLOSE 38

/*
 * help codes for plot windows
 */

#define HELP_PLOT -1

/*
 * button lables and sensitivity
 */

#define TSCALE_BUTTON_LABELS "Help Cases Select Tr_set 1Year 30Days 5Days 1Day 1Hr Time Now Quit"
#define TSCALE_BUTTON_LABELS_ARCHIVE_ONLY "Help Cases Select Tr_set Year 30Days 5Days 1Day 1Hr Time Reset Quit"
#define TSCALE_BUTTON_SENS "1 1 1 1 1 1 1 1 1 1 1 1"
#define TSCALE_BUTTON_SENS_CASE_ANALYSIS "1 1 0 0 1 1 1 1 1 1 1 1"
#define TSCALE_BUTTON_SENS_ARCHIVE_ONLY  "1 0 1 1 1 1 1 1 1 1 1 1"
#define TSCALE_BUTTON_SENS_REALTIME_ONLY "1 0 1 0 0 0 0 0 0 0 0 1"

#define THIST_BUTTON_LABELS "Vol Area Pflux Mass Vil Fcast Fit Copy"
#define THIST_BUTTON_SENS "1 1 1 1 1 1 1 1 1"

#define TIMEHT_BUTTON_LABELS "MaxZ MeanZ %Mass Vort Centr HtmaxZ Copy"
#define TIMEHT_BUTTON_SENS "1 1 1 1 1 1 1"

#define RDIST_BUTTON_LABELS "Vol Area Flip Fit Copy"
#define RDIST_BUTTON_SENS "1 1 1 1 1"

#define UNION_BUTTON_FLOAT_LABELS "Float0 Float1 Float2 Float3 Fit Copy"
#define UNION_BUTTON_HAIL_LABELS "FOKR POH HMass VilHm Fit Copy"
#define UNION_BUTTON_SENS "1 1 1 1 1 1"

#define HELP_BUTTON_LABELS "Close"
#define HELP_BUTTON_SENS "1"

/*
 * draw scale format and legend options
 */

#define SCALE_LABEL_FORMAT_G 0
#define SCALE_LABEL_FORMAT_E 1

#define SCALE_NO_LEGENDS 0
#define SCALE_PLOT_LEGENDS 1

/*
 * X server synchronization
 */

#define X_SYNC "false"

/*
 * color scale names
 */

#define X_DBZ_COLOR_SCALE "dbz_color_scale"
#define PS_DBZ_COLOR_SCALE "dbz_gray_scale"

#define X_VORTICITY_COLOR_SCALE "vorticity_color_scale"
#define PS_VORTICITY_COLOR_SCALE "vorticity_gray_scale"

#define X_PERCENT_COLOR_SCALE "percent_color_scale"
#define PS_PERCENT_COLOR_SCALE "percent_gray_scale"

/*
 * x dimensions
 */

#define X_HELP_X 50L                  /* x pixel offset of vsect window */
#define X_HELP_Y 50L                  /* y pixel offset of vsect window */
#define X_HELP_WIDTH 400L             /* default width of vsect window */
#define X_HELP_HEIGHT 300L            /* default height of vsect window */

#define X_TSCALE_X 15L                /* default x position of tscale window */
#define X_TSCALE_Y 650L               /* default y position of tscale window */
#define X_TSCALE_WIDTH 600L           /* default width of tscale window */
#define X_TSCALE_HEIGHT 100L          /* default height of tscale window */

#define X_THIST_X 15L                 /* default x position of thist window */
#define X_THIST_Y 20L                 /* default y position of thist window */
#define X_THIST_WIDTH 600L            /* default width of thist window */
#define X_THIST_HEIGHT 400L           /* default height of thist window */

#define X_TIMEHT_X 50L                 /* x pixel offset of timeht window */
#define X_TIMEHT_Y 50L                 /* y pixel offset of timeht window */
#define X_TIMEHT_WIDTH 500L            /* default width of timeht window */
#define X_TIMEHT_HEIGHT 300L           /* default height of timeht window */
#define X_TIMEHT_SCALE_WIDTH 60L       /* width of timeht scale window */

#define X_RDIST_X 100L                 /* x pixel offset of rdist window */
#define X_RDIST_Y 100L                 /* y pixel offset of rdist window */
#define X_RDIST_WIDTH 500L             /* default width of rdist window */
#define X_RDIST_HEIGHT 300L            /* default height of rdist window */
#define X_RDIST_SCALE_WIDTH 60L        /* width of rdist scale window */

#define X_UNION_X 100L                 /* x pixel offset of union window */
#define X_UNION_Y 100L                 /* y pixel offset of union window */
#define X_UNION_WIDTH 500L             /* default width of union window */
#define X_UNION_HEIGHT 300L            /* default height of union window */

#define X_MIN_WIDTH 300L               /* min width for any main window */
#define X_MIN_HEIGHT 250L              /* min height for any main window */
#define X_TSCALE_MIN_HEIGHT 100L       /* min height for tscale main window */
#define X_MAX_BUTTON_WIDTH 100L        /* max button width */

#define X_SCALE_ELEMENT_WIDTH 25L         /* width of legend color blocks */
#define X_MAX_SCALE_ELEMENT_HT 25L        /* max ht of legend color blocks */
#define X_SCALE_WIDTH 100L                /* width of scale sub-windows */

/*
 * margins - pixels in X, inches in postscript
 */

#define X_XAXIS_MARGIN 25
#define X_TSCALE_YAXIS_MARGIN 50
#define X_THIST_YAXIS_MARGIN 50L
#define X_TIMEHT_YAXIS_MARGIN 35L
#define X_RDIST_YAXIS_MARGIN 35L
#define X_UNION_YAXIS_MARGIN 35L
#define X_TOP_MARGIN 25L
#define X_XAXIS_ENDMARGIN 30L
#define X_YAXIS_ENDMARGIN 15L
#define X_TICK_LENGTH 6L
#define X_HEADER_CLEARANCE 6L
#define X_TEXT_MARGIN 8

#define PS_XAXIS_MARGIN 0.0
#define PS_TSCALE_YAXIS_MARGIN 0.0
#define PS_THIST_YAXIS_MARGIN 0.0
#define PS_TIMEHT_YAXIS_MARGIN 0.0
#define PS_RDIST_YAXIS_MARGIN 0.0
#define PS_UNION_YAXIS_MARGIN 0.0
#define PS_TOP_MARGIN 0.0
#define PS_XAXIS_ENDMARGIN 0.3
#define PS_YAXIS_ENDMARGIN 0.15
#define PS_TICK_LENGTH 0.06
#define PS_TICK_CLEARANCE 0.05
#define PS_HEADER_CLEARANCE 0.07

/*
 * colors
 */

#define X_FOREGROUND "white"
#define X_BACKGROUND "black"

#define X_TITLETEXT_COLOR "blue"
#define X_BUTTONTEXT_COLOR "blue"
#define X_SCALETEXT_COLOR "blue"
#define X_TICK_COLOR "blue"
#define X_TICKLABEL_COLOR "red"

#define X_CROSSHAIR_COLOR "red"

#define X_HEADER_COLOR "red"
#define X_DIVIDER_COLOR "blue"
#define X_BORDER_COLOR "blue"
#define X_HLIGHT_BACKGROUND "orange"

#define X_THIST_VOL_COLOR "gray80"
#define X_THIST_AREA_COLOR "cyan"
#define X_THIST_PFLUX_COLOR "green"
#define X_THIST_MASS_COLOR "magenta"
#define X_THIST_VIL_COLOR "yellow"
#define X_THIST_FORECAST_COLOR "red"

#define X_UNION_0_COLOR "yellow"
#define X_UNION_1_COLOR "cyan"
#define X_UNION_2_COLOR "green"
#define X_UNION_3_COLOR "magenta"

#define X_HT_MAXDBZ_COLOR "magenta"
#define X_TOP_BASE_COLOR "yellow"
#define X_HT_CENTROID_COLOR "cyan"
#define X_HT_REFL_CENTROID_COLOR "green"
#define X_TEXT_COLOR "red"

/*
 * x line widths
 */

#define X_TICK_LINE_WIDTH 1L
#define X_DIVIDER_LINE_WIDTH 1L
#define X_MAIN_BORDER_WIDTH 1L
#define X_SUB_BORDER_WIDTH 1L
#define X_CROSSHAIR_WIDTH 1L

/*
 * x fonts
 */

#define X_TITLE_FONT "fixed"
#define X_BUTTON_FONT "6x10"
#define X_SCALE_FONT "6x10"
#define X_TICKLABEL_FONT "6x10"
#define X_HEADER_FONT "fixed"
#define X_TEXT_FONT "fixed"

/*
 * postscript page dimensions
 */

#define PS_PAGE_WIDTH 8.5
#define PS_PAGE_LENGTH 11.0
#define PS_WIDTH_MARGIN 0.5
#define PS_LENGTH_MARGIN 0.5
#define PS_TEXT_MARGIN 0.1

#define PS_TITLE_HEIGHT 0.5
#define PS_TIMEHT_SCALE_WIDTH 1.5
#define PS_RDIST_SCALE_WIDTH 1.5
#define PS_UNION_SCALE_WIDTH 1.5
#define PS_PLOT_WIDTH 6.0
#define PS_PLOT_HEIGHT 6.0
#define PS_TITLE_TO_PLOT_MARGIN 0.4
#define PS_PLOT_TO_SCALE_MARGIN 0.25
#define PS_SCALE_ELEMENT_WIDTH 0.5   /* width of scale color blocks */
#define PS_MAX_SCALE_ELEMENT_HT 0.5  /* max ht of scale color blocks */
#define PS_PRINT_WIDTH 8.5   /* user selected width for hard copy */
#define PS_MAX_PRINT_WIDTH 11.0
#define PS_MIN_PRINT_WIDTH 2.0

/*
 * ps fonts
 */

#define PS_FONTNAME "Helvetica"
#define PS_TITLE_FONTSIZE 18.0
#define PS_SCALE_FONTSIZE 12.0
#define PS_TICKLABEL_FONTSIZE 8.0
#define PS_HEADER_FONTSIZE 12.0
#define PS_TEXT_FONTSIZE 12.0

/*
 * postscript line widths
 */

#define PS_TICK_LINE_WIDTH 1.0
#define PS_BORDER_LINE_WIDTH 1.0
#define PS_DIVIDER_LINE_WIDTH 1.0
#define PS_CROSSHAIR_WIDTH 1.0

/*
 * postscript linestyles
 */

#define PS_THIST_VOL_LINESTYLE "1.0 100.0 0.0"
#define PS_THIST_AREA_LINESTYLE "1.0 1.0 1.0"
#define PS_THIST_PFLUX_LINESTYLE "1.0 3.0 1.0"
#define PS_THIST_MASS_LINESTYLE "1.0 5.0 2.0"
#define PS_THIST_VIL_LINESTYLE "1.0 2.0 2.0"
#define PS_THIST_FORECAST_LINESTYLE "1.0 100.0 0.0"

#define PS_UNION_0_LINESTYLE "1.0 100.0 0.0"
#define PS_UNION_1_LINESTYLE "1.0 1.0 1.0"
#define PS_UNION_2_LINESTYLE "1.0 3.0 1.0"
#define PS_UNION_3_LINESTYLE "1.0 5.0 2.0"

#define PS_HT_MAXDBZ_LINESTYLE "1.0 100.0 0.0"
#define PS_HT_CENTROID_LINESTYLE "1.0 1.0 1.0"
#define PS_HT_REFL_CENTROID_LINESTYLE "1.0 4.0 2.0"
#define PS_TOP_BASE_LINESTYLE "1.0 100.0 0.0"

/*
 * approx number of ticks per axis
 */

#define APPROX_NTICKS 8L

/*
 *************************** structs *******************************
 */

/*
 * struct for plot data from a given track
 */

typedef struct {

  si32 time;
  si32 n_layers;
  si32 n_dbz_intervals;

  double delta_z;
  double min_z;
  double max_z;
  double dbz_max;
  double ht_of_dbz_max;
  double top;
  double base;
  double vol_centroid_z;
  double refl_centroid_z;
  double volume;

  double thist_data[THIST_N_FIELDS];
  double thist_dval_dt[THIST_N_FIELDS];

  double union_data[UNION_N_FIELDS];

  int *timeht_flag;
  double *timeht_data;

  int *rdist_flag;
  double *rdist_data;

} thist_scan_data_t;
  
typedef struct {

  si32 nscans;

  /*
   * time
   */

  si32 plot_start_time, plot_end_time;
  si32 forecast_end_time;
  si32 scan_duration;

  /*
   * time history graph
   */

  double min_thist_val[THIST_N_FIELDS];
  double max_thist_val[THIST_N_FIELDS];
  char *thist_label[THIST_N_FIELDS];
  char *thist_units[THIST_N_FIELDS];

  /*
   * timeht profile
   */

  si32 max_layers;
  double min_base, max_top;
  char *timeht_label;

  /*
   * dbz distribution
   */

  double dbz_threshold;
  double dbz_hist_interval;
  si32 max_dbz_intervals;
  char *rdist_label;

  /*
   * union graph
   */

  double min_union_val[UNION_N_FIELDS];
  double max_union_val[UNION_N_FIELDS];
  char *union_label[UNION_N_FIELDS];
  char *union_units[UNION_N_FIELDS];

  /*
   * scan data - one entry for each scan
   */

  thist_scan_data_t *scan;

} thist_track_data_t;
  
/*
 * globals
 */

class TimeHistGlobal {

public:

  TimeHistGlobal();
  ~TimeHistGlobal();

  // data members

  int start_c;

  int argc;
  char **argv;
    
  char *prog_name;                        /* program name */
  char *instance;                         /* program instance */
  char *params_path_name;                 /* params file path name */

  int archive_time_set;                   /* flag */
  time_t archive_time;                    /* initial archive time */

  /*
   * track server
   */

  char *servmap_host1;                    /* server mapper hosts */
  char *servmap_host2;
  si32 max_message_len;                   /* max message len for server data
					   * buffer */

  /*
   * case tracks?
   */

  int use_case_tracks;                    /* use case tracks to determine
					   * partial tracks? */
  char *case_tracks_file_path;            /* file path for case tracks */
  
  /*
   * partial tracks
   */

  int partial_track_past_period;          /* past period for computing
					   * partial tracks (secs) */

  int partial_track_future_period;        /* future period for computing
					   * partial tracks (secs) */
  
  /*
   * shared memory
   */
  
  key_t track_shmem_key;                    /* key for track shared mem */
  coord_export_t *coord_export;             /* coordinate exporting */
  time_hist_shmem_t *track_shmem;           /* track shared memory */

  /*
   * track data index - struct which stores pointers to the
   * track data
   */

  tdata_index_t tdata_index;
  
  si32 complex_index;                   /* index for current complex track */
  si32 simple_index;                    /* index for current simple track */

  /*
   * track plot data
   */

  time_t data_start_time;
  time_t data_end_time;
  thist_track_data_t tdata;
  int track_data_avail;                  /* TRUE if track data is available */

  /*
   * flags
   */

  int debug;                              /* debugging */
  int verbose;                            /* verbose debugging */
  int x_sync;                             /* x synchronization flag */
  int draw_copy_title;                    /* draw title on copy flag */

  int help;                               /* TRUE for help mode on */
  int help_code;                          /* code for which help is needed */

  int scan_delta;                         /* +1 if we want to move fwd
					   * -1 if we want to move back
					   * 0 if neither */

  int archive_only;                       /* set to TRUE for archive mode only */
  int realtime_only;                      /* set to TRUE if realtime_only
					   * operations */

  int thist_active;                       /* show time history plot */
  int timeht_active;                      /* show time-ht plot */
  int rdist_active;                       /* show refl dist plot */
  int union_active;                       /* show union data plot */

  int thist_raise_priority;               /* priority with which thist window
					   * is raised to front */
  int timeht_raise_priority;               /* priority with which timeht window
					   * is raised to front */
  int rdist_raise_priority;               /* priority with which rdist window
					   * is raised to front */
  int union_raise_priority;               /* priority with which union window
					   * is raised to front */

  int thist_field_active[THIST_N_FIELDS];/* time history field flags */
  int thist_fit;                         /* plot fit to thist data */
  int timeht_mode;                       /* time height profile mode */
  int rdist_mode;                        /* refl distribution mode */
  double rdist_sign;                     /* plot sense for rdist */
  int rdist_fit;                         /* fit distribution to data
					  * defaults to FALSE */

  int thist_forecast;                    /* plot time history forecast */
  int timeht_centroids;                  /* plot vert centroid on
					  * timeht profile */
  int timeht_htmaxz;                     /* plot ht of max z on
					  * timeht profile */

  int union_mode;                         /* which union mode to plot */
  int union_field_active[UNION_N_FIELDS]; /* union field flags */
  int union_log;                          /* plot union as log */
  int union_fit;                          /* fit normal to data */

  int tscale_status;
  int thist_status;
  int timeht_status;
  int rdist_status;
  int union_status;
  int timeht_scale_status;
  int rdist_scale_status;
  int plot_data_status;
  int help_status;
  int windows_active;

  /*
   * options
   */
  
  double double_click_delay;              /* max double click delay
					   * in millisecs */
  double print_width;                     /* width of copy */
  char *display_name;                     /* X display name */
  char *foregroundstr;
  char *backgroundstr;
  
  /*
   * cursor - activated on button click in main window
   */

  int thist_cursor_active;
  double thist_cursor_x;
  double thist_cursor_y;
  
  int timeht_cursor_active;
  double timeht_cursor_x;
  double timeht_cursor_y;
  
  int rdist_cursor_active;
  double rdist_cursor_x;
  double rdist_cursor_y;
  
  int union_cursor_active;
  double union_cursor_x;
  double union_cursor_y;
  
  /*
   * X declarations
   */
  
  int rscreen;                            /* screen id */
  Display *rdisplay;                      /* display device */
  Window tscale_window;                   /* time scale window */
  Window thist_window;                    /* main thist window */
  Window timeht_window;                   /* time height window */
  Window rdist_window;                    /* refl dist window */
  Window union_window;                    /* union variables window */
  Window help_window;                     /* help main window */

  /*
   * color list index
   */

  x_color_list_index_t color_index;       /* struct to hold the index to the
					   * color list entries */
  
  /*
   * X graphics contexts
   */

  GC foreground_gc;                /* GC for foreground color */
  GC background_gc;                /* GC for background color */
  GC tick_gc;                      /* GC for tick marks */
  GC divider_gc;                   /* GC for line bordering plot */
  GC header_gc;                    /* GC for header in plot frame */
  GC ticklabel_gc;                 /* GC for axis labels */
  GC copyarea_gc;                  /* GC for copying pixmap
				      to screen */
  GC pixmap_gc;                    /* for initializing pixmap */
  GC xor_gc;                       /* for rubberband lines */
  GC text_gc;                      /* GC for text */
  GC crosshair_gc;                 /* GC for crosshair */
  
  GC thist_graph_gc[THIST_N_FIELDS]; /* GCs for time history graphs */
  GC thist_forecast_gc;            /* GC for time history forecast */
  GC ht_maxdbz_gc;                 /* GC for ht of max dbz */
  GC top_base_gc;                  /* GC for base and top */
  GC ht_centroid_gc;               /* GC for ht of centroid */
  GC ht_refl_centroid_gc;          /* GC for ht of refl. weighted centroid */
  GC union_graph_gc[UNION_N_FIELDS]; /* GCs for union graphs */

  GC past_gc;                      /* GC for previous storm scans */
  GC current_gc;                   /* GC for current storm scans */
  GC future_gc;                    /* GC for future storm scans */
  GC forecast_gc;                  /* GC for forecast storm scans */

  /*
   * postscript graphics contexts
   */

  psgc_t thist_graph_psgc[THIST_N_FIELDS];  /* psgcs for time hist graphs */
  psgc_t thist_forecast_psgc;     /* psgc for time hist forecast */
  psgc_t ht_maxdbz_psgc;          /* psgc for ht of max dbz */
  psgc_t top_base_psgc;           /* psgc for ht of centroid */
  psgc_t ht_centroid_psgc;        /* psgc for ht of centroid */
  psgc_t ht_refl_centroid_psgc;   /* psgc for ht of refl. weighted centroid */
  psgc_t union_graph_psgc[UNION_N_FIELDS];  /* psgcs for union graphs */

  /*
   * margins - pixels in X, inches in postscript
   */

  si32 x_xaxis_margin;

  si32 x_tscale_yaxis_margin;
  si32 x_thist_yaxis_margin;
  si32 x_timeht_yaxis_margin;
  si32 x_rdist_yaxis_margin;
  si32 x_union_yaxis_margin;

  si32 x_top_margin;
  si32 x_text_margin;
  si32 x_xaxis_endmargin;
  si32 x_yaxis_endmargin;
  si32 x_tick_length;
  si32 x_header_clearance;

  double ps_xaxis_margin;

  double ps_tscale_yaxis_margin;
  double ps_thist_yaxis_margin;
  double ps_timeht_yaxis_margin;
  double ps_rdist_yaxis_margin;
  double ps_union_yaxis_margin;

  double ps_top_margin;
  double ps_text_margin;
  double ps_xaxis_endmargin;
  double ps_yaxis_endmargin;
  double ps_tick_length;
  double ps_header_clearance;

  /*
   * X window geometry
   */
  
  int x_tscale_x;
  int x_tscale_y;
  ui32 x_tscale_width;
  ui32 x_tscale_height;

  int x_thist_x;
  int x_thist_y;
  ui32 x_thist_width;
  ui32 x_thist_height;

  int x_timeht_x;
  int x_timeht_y;
  ui32 x_timeht_scale_width;
  ui32 x_timeht_width;
  ui32 x_timeht_height;

  int x_rdist_x;
  int x_rdist_y;
  ui32 x_rdist_scale_width;
  ui32 x_rdist_width;
  ui32 x_rdist_height;

  int x_union_x;
  int x_union_y;
  ui32 x_union_width;
  ui32 x_union_height;

  int x_help_x;
  int x_help_y;
  ui32 x_help_width;
  ui32 x_help_height;

  ui32 x_max_button_width;

  /*
   * postscript page geometry
   */
  
  double ps_unitscale;

  double ps_page_width;
  double ps_page_length;
  double ps_width_margin;
  double ps_length_margin;

  double ps_plot_height;
  double ps_plot_width;
  double ps_timeht_scale_width;
  double ps_rdist_scale_width;
  double ps_title_height;
  double ps_title_to_plot_margin;
  double ps_plot_to_scale_margin;

  /*
   * X line widths
   */

  si32 x_main_border_width;
  si32 x_sub_border_width;

  /*
   * postscript line widths
   */

  double ps_tick_line_width;
  double ps_border_line_width;
  double ps_divider_line_width;
  double ps_crosshair_line_width;
  
  /*
   * fonts
   */
  
  XFontStruct *x_title_font;
  XFontStruct *x_button_font;
  XFontStruct *x_scale_font;
  XFontStruct *x_ticklabel_font;
  XFontStruct *x_header_font;
  XFontStruct *x_text_font;
  
  double ps_title_fontsize;
  double ps_scale_fontsize;
  double ps_ticklabel_fontsize;
  double ps_header_fontsize;
  double ps_text_fontsize;
  char *ps_fontname;
  
  /*
   * colors
   */
  
  ui32 foreground;
  ui32 background;
  ui32 hlight_background;
  ui32 border_color;
  g_color_scale_t *x_dbz_cscale, *x_vorticity_cscale, *x_percent_cscale;
  g_color_scale_t *ps_dbz_cscale, *ps_vorticity_cscale, *ps_percent_cscale;
  g_color_scale_t *x_timeht_cscale, *x_rdist_cscale;
  g_color_scale_t *ps_timeht_cscale, *ps_rdist_cscale;
  
  /*
   * frames for graphics
   */
  
  gframe_t *tscale_title_frame;
  gframe_t **tscale_button_frame;
  gframe_t *tscale_plot_frame;

  gframe_t *thist_title_frame;
  gframe_t **thist_button_frame;
  gframe_t *thist_plot_frame;

  gframe_t *timeht_title_frame;
  gframe_t **timeht_button_frame;
  gframe_t *timeht_plot_frame;
  gframe_t *timeht_scale_frame;

  gframe_t *rdist_title_frame;
  gframe_t **rdist_button_frame;
  gframe_t *rdist_plot_frame;
  gframe_t *rdist_scale_frame;

  gframe_t *union_title_frame;
  gframe_t **union_button_frame;
  gframe_t *union_plot_frame;

  gframe_t *vert_page_frame;
  gframe_t *horiz_page_frame;

  gframe_t *help_title_frame;
  gframe_t **help_button_frame;
  gframe_t *help_text_frame;

  titan_grid_t titan_grid;                /* grid for titan data */

  /*
   * miscellaneous
   */

  double thist_x_tick_length;
  double thist_plot_min_y;

  double union_x_tick_length;
  double union_plot_min_y;
  
  int end_c;

  // C++ members - must not be zero'd out

  DsTitan _dsTitan;
  DsLdataInfo _titanLdata;

protected:
private:

};

/*
 * declare the global structure locally in the thist,
 * and as an extern in all other routines
 */

#ifdef MAIN

TimeHistGlobal *Glob = NULL;

#else

extern TimeHistGlobal *Glob;

#endif

/*
 * function prototypes
 */

extern int handle_tserver_sigpipe(int sig);

extern void check_for_new_data();

extern int xerror_handler(Display *display,
			  XErrorEvent *err);

extern void compute_ticks(long start_time,
			  long end_time,
			  long *nticks,
			  long *first_tick_time,
			  long *tick_interval);

extern void compute_track_num();
extern void go_to_next_case();
extern void go_to_prev_case();

extern void copy_rdist(g_color_scale_t *color_scale);

extern void copy_thist();

extern void copy_timeht(g_color_scale_t *color_scale);

extern void copy_union();

extern void create_frames();

extern void draw_help_button(si32 n,
			     ui32 background);

extern void draw_help_text();

extern void draw_help_title();

extern void draw_rdist_button(si32 n,
			      ui32 background);

extern void draw_rdist_plot(int dev,
			    g_color_scale_t *colors);

extern void draw_rdist_title(int dev);

extern void draw_scale(int dev,
		       gframe_t *frame,
		       g_color_scale_t *colors,
		       int format,
		       int plot_legend);

extern void draw_thist_button(si32 n,
			      ui32 background);

extern void draw_thist_plot(int dev);

extern void draw_thist_title(int dev);

extern void draw_timeht_button(si32 n,
			       ui32 background);

extern void draw_timeht_plot(int dev,
			     g_color_scale_t *colors);

extern void draw_timeht_title(int dev);

extern void draw_tscale_button(si32 n,
			       ui32 background);

extern void draw_tscale_plot(int dev);

extern void draw_tscale_title(int dev);

extern void draw_union_button(si32 n,
			      ui32 background);

extern void draw_union_plot(int dev);

extern void draw_union_title(int dev);

extern void event_loop();

extern void free_resources();

extern void get_ps_color_scales();

extern void get_titan_data();

extern void get_track_data();

extern void get_x_color_scales();

extern int load_plot_data();

extern void parse_args(int argc,
		       char **argv);

extern int check_for_print_params(int argc, char **argv);

extern void print_copy(char *ps_file_path);

extern void read_params();

extern void set_help_sens();

extern void set_rdist_sens();

extern void set_thist_sens();

extern void set_timeht_sens();

extern void set_timer();

extern void set_timer_startup();

extern void set_tscale_sens();

extern void set_union_sens();

extern void set_xfonts();

extern void set_xgcs();

extern void setup_help_windows();

extern void setup_rdist_windows();

extern void setup_thist_windows();

extern void setup_timeht_windows();

extern void setup_track_shmem();

extern void setup_tscale_windows();

extern void setup_x();

extern void setup_union_windows();

extern void tidy_and_exit(int sig);

