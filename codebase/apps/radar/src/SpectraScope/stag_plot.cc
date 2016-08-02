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
/*************************************************************************
 * SPECTRA_PLOT.CC
 *
 * PLOT radar spectra
 *
 * Based on metar_strip application, which is built on xview toolkit.
 */

#include <cmath>           /* System */

#include <X11/Xlib.h>       /* X11 - XView */
#include <X11/Xresource.h>
#include <xview/xview.h>
#include <xview/canvas.h>

#include <toolsa/pmu.h>    
#include <toolsa/ushmem.h>    
#include <dataport/port_types.h>

#include <Spdb/DsSpdb.hh>
#include <Spdb/Product_defines.hh>
 
#include <toolsa/DateTime.hh>
#include <toolsa/pjg.h>
#include <rapmath/math_macros.h>
#include <radar/RadarFft.hh>
#include <radar/RadarComplex.hh>
#include <radar/RadarMoments.hh>
#include <radar/MomentsFields.hh>
#include <radar/MomentsFields.hh>
#include <radar/RegressionFilter.hh>
#include <radar/ClutFilter.hh>
#include <rapformats/RadarSpectra.hh>
#include <toolsa/TaArray.hh>

#include <algorithm>

#include "Params.hh"
#include "SpectraScope.hh"
#include "FieldInfo.hh"

using namespace std;

typedef struct {
	Xv_opaque	win1;
	Xv_opaque	canvas1;
} stag_chart_win1_objects;

///////////////////////////////
// FILE SCOPE VARIABLES

static Attr_attribute INSTANCE;

static char *_app_name;
  
static Params *_params; // TDRP parameters

static int _channel; // current variable
static string _channelName;

static int _win_width, _win_height;
static int _plot_width, _plot_height;

static int _ascope_start_x, _ascope_end_x;
static int _ascope_start_y, _ascope_end_y;

static double _plot_min_y, _plot_max_y;

static double _pix_x_unit, _pix_x_bias;
static double _pix_y_unit, _pix_y_bias;

static int _text_ht;
static int _line_spacing;

static time_t _start_time, _end_time;
static double _pixels_per_x_val;

static int _fg_cell;      /* foreground color cell */
static int _bg_cell;      /* background color cell */
static int _grid_cell;    /* color for "grid" on the time line */
static int _noise_cell;   /* color for noise level line */
static int _range_cell;   /* color for range line */

static bool _data_avail;

static double _clickElev;
static double _clickAz;
static double _clickRange;

static int _nSamples, _nSamplesHalf;
static int _nGates;
static double _minRange, _maxRange;
static double _deltaAzMedian = 1.0;

static RadarSpectra _spectra;
static double _gateRange;
static int _gateNum;
static MomentsFields _gateMoments;
static MomentsFields _filtMoments;

static FieldInfo *_powerSpec;
static FieldInfo *_adapFiltSpec;
static FieldInfo *_regrFiltSpec;

static FieldInfo *_stagSpecBoth;
static FieldInfo *_stagFiltSpecBoth;

static FieldInfo *_powerTs;
static FieldInfo *_phaseTs;
static FieldInfo *_phaseResidual;
static FieldInfo *_phaseDiff;
static FieldInfo *_cumulativePhaseDiff;

static FieldInfo *_fftPhaseDiff;

static FieldInfo *_powerSpecReal;
static FieldInfo *_powerAscope;

static FieldInfo *_stagSpecShort;
static FieldInfo *_stagTsPowerShort;
static FieldInfo *_stagTsPhaseShort;

static FieldInfo *_stagSpecLong;
static FieldInfo *_stagTsPowerLong;
static FieldInfo *_stagTsPhaseLong;

static FieldInfo *_stagFiltSpecShort;
static FieldInfo *_stagFiltSpecLong;

static FieldInfo *_stagFiltSpecShortInterp;
static FieldInfo *_stagFiltSpecShortRatio;

static FieldInfo *_stagFiltSpecLongInterp;
static FieldInfo *_stagFiltSpecLongRatio;

static FieldInfo *_stagTsPowerPhaseDiff;
static FieldInfo *_stagFiltSpecPhaseDiff;
static FieldInfo *_stagFiltSpecPhaseDiffInterp;

static FieldInfo *_iTs;
static FieldInfo *_iTsPolyFit;
static FieldInfo *_iTsResidual;

static FieldInfo *_qTs;
static FieldInfo *_qTsPolyFit;
static FieldInfo *_qTsResidual;

static FieldInfo *_iqPhasor;
static FieldInfo *_runningCpa;

// X Windows Drawing variables

static Display *_dpy;
static XFontStruct *_fontst;
static Font _font;
static Drawable _canvas_xid;
static Drawable _back_xid;
static GC _def_gc;

static stag_chart_win1_objects *_stag_chart_win1;

// XVIEW actions

static double _lastAscopeSelectTime = 0.0;

// pointer to cidd's shmem control interface  

static coord_export_t *_cidd_mem;

// data retrieval

static DsSpdb _spdb;
static DsFmq *_fmq;

////////////////////////
// file scope prototypes

static void timer_func(Notify_client client, int which);
static void start_timer();
static void set_frame_label();
static void draw_plot();

static void init_xview();
static int  x_error_proc( Display *disp, XErrorEvent *event);
static Notify_value base_win_destroy( Notify_client client,
				      Destroy_status status);
static void init_static_vars();
static void init_data_space();
static void freeField(FieldInfo* &field);
static void modify_xview_objects();

static bool check_click();
static bool check_fmq();
static int retrieve_spdb_data();
static void set_click_point_from_cidd();
static void set_click_point_from_user(double az, double range);
static int find_closest_spdb();

static int compute_number_of_plots();

static void clear_plot_data();
static void load_plot_data();
static void load_plot_data_single_prt();
static void load_plot_data_staggered_prt();

static void print_shmem();

static double compute_tick_interval(double range);

static void plot_string(const char *string_buf, int xx, int yy);
static void plot_string_centered(const char *string_buf, int x_start, int yy);
static void plot_string_justified(const char *string_buf, int x_end, int yy);
		    
static void compute_world_to_pixel(FieldInfo &primaryField,
				   FieldInfo *secondField,
				   FieldInfo *thirdField,
				   int x_start, int x_end,
				   int y_start, int y_end,
				   bool use_field_min_and_max,
                                   bool scale_on_primary_only = true);
		    
static void compute_min_and_max_y(const FieldInfo &field,
                                  double &y_min,
                                  double &y_max);

static double nearest(double target,double delta);

static void  draw_scales( FieldInfo &field,
			  int x_start, int x_end,
			  int y_start, int y_end);
		    
static void plot_field( FieldInfo &field, int x_start, int x_end,
			int y_start, int y_end,
			bool plot_grid,
                        bool plot_label_left,
                        bool plot_label_down = false);

static void plot_field_data(FieldInfo &field,
			    int x_start, int x_end,
			    int y_start, int y_end);
 
static void plot_noise_value(int x_start, int x_end);
static void plot_range_line(int y_start, int y_end);
 
static void add_range_labels(FieldInfo &field,
			     int x_start, int x_end,
			     int y_start, int y_end);
  
static stag_chart_win1_objects
*stag_chart_win1_objects_initialize(stag_chart_win1_objects *, Xv_opaque);

static Xv_opaque stag_chart_win1_win1_create(stag_chart_win1_objects *, Xv_opaque);
static Xv_opaque stag_chart_win1_canvas1_create(stag_chart_win1_objects *, Xv_opaque);

/********************************************************************
 * MAIN: Process arguments, initialize and begin application
 */
 

int stag_chart_main(int argc, char **argv, Params *params,
		     coord_export_t *coord_shmem,
		     DsFmq &input_fmq)

{

  xv_init(XV_INIT_ARGC_PTR_ARGV,
	  &argc,argv,
	  XV_X_ERROR_PROC, x_error_proc, NULL);
  
  // global struct

  init_static_vars();
  _params = params;
  _cidd_mem = coord_shmem;
  _fmq = &input_fmq;
  
  // create Xview objects

  init_xview();

  // initialize globals, get/set defaults, establish data fields etc.
  
  init_data_space();

  // make changes to xview objects not available from DevGuide

  modify_xview_objects();
  
  // start timer

  start_timer();
  
  // Turn control over to XView.

  xv_main_loop(_stag_chart_win1->win1);

  return 0;

}

/***************************************************************************
 * INIT_XVIEW : Initialize the base frame and other global objects
 */ 
 
static void init_xview()
{
 
  _stag_chart_win1 =
    stag_chart_win1_objects_initialize(NULL, (Xv_opaque) NULL);
  
  notify_interpose_destroy_func(_stag_chart_win1->win1,
				(Notify_func) base_win_destroy);

  _dpy = (Display *) xv_get(_stag_chart_win1->win1,XV_DISPLAY);

}

/*****************************************************************
 * X_ERROR_PROC: Handle errors generated by the X server
 */
 
static int x_error_proc(Display *disp, XErrorEvent *event)
{
  char	text[256];
 
  XGetErrorText(disp,event->error_code,text,256);
  fprintf(stderr,"Generated X error : %s, ID:%d\n", text, 
	  (int) event->resourceid);
 
  switch(event->error_code) {
  default :
    return   XV_OK;
    break;
    
  case BadAlloc :
    return   XV_OK;
    break;
  }

}

/*****************************************************************
 * INIT_STATIC_VARS: zero out as applicable
 */

static void init_static_vars()
{

  _app_name = NULL;
  _params = NULL;
  
  _channel = 0;

  _powerSpec = NULL;
  _adapFiltSpec = NULL;
  _regrFiltSpec = NULL;

  _stagSpecBoth = NULL;
  _stagFiltSpecBoth = NULL;

  _stagSpecShort = NULL;
  _stagTsPowerShort = NULL;
  _stagTsPhaseShort = NULL;
  _stagFiltSpecShort = NULL;
  _stagFiltSpecShortInterp = NULL;
  _stagFiltSpecShortRatio = NULL;

  _stagSpecLong = NULL;
  _stagTsPowerLong = NULL;
  _stagTsPhaseLong = NULL;
  _stagFiltSpecLong = NULL;
  _stagFiltSpecLongInterp = NULL;
  _stagFiltSpecLongRatio = NULL;

  _stagTsPowerPhaseDiff = NULL;
  _stagFiltSpecPhaseDiff = NULL;
  _stagFiltSpecPhaseDiffInterp = NULL;

  _powerTs = NULL;
  _phaseTs = NULL;
  _phaseResidual = NULL;
  _phaseDiff = NULL;
  _cumulativePhaseDiff = NULL;
  _powerSpecReal = NULL;
  _powerAscope = NULL;

  _fftPhaseDiff = NULL;

  _iTs = NULL;
  _iTsPolyFit = NULL;
  _iTsResidual = NULL;

  _qTs = NULL;
  _qTsPolyFit = NULL;
  _qTsResidual = NULL;

  _iqPhasor = NULL;
  _runningCpa = NULL;
  
  _win_width = 0;
  _win_height = 0;
  _plot_width = 0;
  _plot_height = 0;

  _start_time = 0;
  _end_time = 0;
  _pixels_per_x_val = 10;
  
  _ascope_start_x = -1;
  _ascope_end_x = -1;
  _ascope_start_y = -1;
  _ascope_end_y = -1;

  _fg_cell = 0;
  _bg_cell = 0;
  _grid_cell = 0;
  _noise_cell = 0;
  _range_cell = 0;
  
  _data_avail = false;

  _clickElev = 0;
  _clickAz = 0;
  _clickRange = 0;

  _dpy = NULL;
  _fontst = NULL;
  _font = 0;
  _canvas_xid = 0;
  _back_xid = 0;
  _def_gc = 0;
  
  _stag_chart_win1 = NULL;
  
  _cidd_mem = NULL;

}

/*****************************************************************
 * INIT_DATA_SPACE : Init all globals and set up defaults
 */

static void init_data_space()
{

  INSTANCE = xv_unique_key(); /* get keys for retrieving data */

  _channel = (int) RadarSpectra::CHANNEL_HC;
  _channelName = "CHANNEL_HC";

  _pixels_per_x_val = (double) _params->window_width / 64;

  // set up the fields/plots

  // power spectrum

  _powerSpec = new FieldInfo(_dpy, "Power spectrum (dBm)",
                               _params->power_spec_color,
                               _params->power_spec_min_db,
                               _params->power_spec_max_db);

  // filtered data is plotted on top of power spectrum,
  // so do not add to fields array
  
  _adapFiltSpec = new FieldInfo(_dpy, "Adap filt power spec (dBm)",
                                   _params->adaptive_filtered_spec_color,
                                   _params->power_spec_min_db,
                                   _params->power_spec_max_db);
  
  _regrFiltSpec = new FieldInfo(_dpy, "Regr filt power spec (dBm)",
                                _params->regression_filtered_spec_color,
                                _params->power_spec_min_db,
                                _params->power_spec_max_db);
  
  // staggered spectra
  
  _stagSpecBoth = new FieldInfo(_dpy, "Stag PRT spectrum (dBm)",
                                   _params->power_spec_color,
                                   _params->power_spec_min_db,
                                   _params->power_spec_max_db);
  
  _stagFiltSpecBoth = new FieldInfo(_dpy, "Stag PRT regr filt (dBm)",
                                    _params->regression_filtered_spec_color,
                                    _params->power_spec_min_db,
                                    _params->power_spec_max_db);
  
  _stagSpecShort = new FieldInfo(_dpy, "Stag power spectrum short (dBm)",
                                 _params->short_prt_color,
                                 _params->power_spec_min_db,
                                 _params->power_spec_max_db);
  
  _stagTsPowerShort = new FieldInfo(_dpy, "Stag Ts Power short (dBm)",
                                    _params->short_prt_color,
                                    _params->ts_power_min_db,
                                    _params->ts_power_max_db);
  
  _stagTsPhaseShort = new FieldInfo(_dpy, "Stag short PRT phase (deg)",
                                    _params->short_prt_color, -180, 180);
  
  _stagSpecLong = new FieldInfo(_dpy, "Stag power spectrum long (dBm)",
                                _params->long_prt_color,
                                _params->power_spec_min_db,
                                _params->power_spec_max_db);
  
  _stagTsPowerLong = new FieldInfo(_dpy, "Stag Ts Power long (dBm)",
                                   _params->long_prt_color,
                                   _params->ts_power_min_db,
                                   _params->ts_power_max_db);
  
  _stagTsPhaseLong = new FieldInfo(_dpy, "Stag long PRT phase (deg)",
                                   _params->long_prt_color, -180, 180);
  
  _stagFiltSpecShort =
    new FieldInfo(_dpy, "Stag filt power spectrum short (dBm)",
                  _params->short_prt_color,
                  _params->power_spec_min_db,
                  _params->power_spec_max_db);
  
  _stagFiltSpecLong =
    new FieldInfo(_dpy, "Stag filt power spectrum long (dBm)",
                  _params->long_prt_color,
                  _params->power_spec_min_db,
                  _params->power_spec_max_db);
  
  _stagFiltSpecShortInterp = new FieldInfo(_dpy, "Stag short PRT filt interp (dBm)",
                                           _params->short_prt_color,
                                           _params->power_spec_min_db,
                                           _params->power_spec_max_db);
  
  _stagFiltSpecShortRatio = new FieldInfo(_dpy, "Stag short PRT filt power ratio (dB)",
                                          _params->short_prt_color,
                                          _params->power_spec_min_db,
                                          _params->power_spec_max_db);
  
  _stagFiltSpecLongInterp = new FieldInfo(_dpy, "Stag long PRT filt interp (dBm)",
                                          _params->long_prt_color,
                                          _params->power_spec_min_db,
                                          _params->power_spec_max_db);
  
  _stagFiltSpecLongRatio = new FieldInfo(_dpy, "Stag long PRT filt power ratio (dB)",
                                         _params->long_prt_color,
                                         _params->power_spec_min_db,
                                         _params->power_spec_max_db);
  
  _stagTsPowerPhaseDiff = new FieldInfo(_dpy, "Stag spec phase diff (deg)",
                                     _params->fft_phase_diff_color,
                                     -180, 180);
  
  _stagFiltSpecPhaseDiff = new FieldInfo(_dpy, "Stag filt spec phase diff (deg)",
                                         _params->fft_phase_diff_color,
                                         -180, 180);
  
  _stagFiltSpecPhaseDiffInterp = new FieldInfo(_dpy, "Stag filt interp phase diff (deg)",
                                               _params->fft_phase_diff_color,
                                               -180, 180);
  
  // time domain power

  _powerTs = new FieldInfo(_dpy, "Time series power (dBm)",
                           _params->ts_power_color,
                           _params->ts_power_min_db,
                           _params->ts_power_max_db);
  
  // spectrum of power, ignoring phase
  
  _powerSpecReal = new FieldInfo(_dpy, "Real power spectrum (dBm)",
				 _params->power_real_spec_color,
				 _params->ts_power_min_db,
                                 _params->ts_power_max_db);
     
  // time domain phase

  _phaseTs = new FieldInfo(_dpy, "Time series phase (deg)",
			   _params->ts_phase_color,
			   -180, 180);
  
  _phaseResidual = new FieldInfo(_dpy, "Residual phase (deg)",
                                 _params->ts_residual_color,
                                 -180, 180);

  // pulse-to-pulse phase diff
  
  _phaseDiff = new FieldInfo(_dpy, "Pulse-to-pulse phase diff (deg)",
			     _params->phase_diff_color,
			     -180, 180);
  
  _cumulativePhaseDiff = new FieldInfo(_dpy, "Cumulative phase diff (deg)",
                                       _params->cumulative_phase_diff_color,
                                       -1, 1);

  // fft phase and difference - for clean-ap

  _fftPhaseDiff = new FieldInfo(_dpy, "FFT phase diff (deg)",
                                _params->fft_phase_diff_color,
                                -180, 180);
  
  // ASCOPE

  _powerAscope = new FieldInfo(_dpy, "ASCOPE - power vs. Range (dBm)",
			       _params->ascope_power_color,
			       _params->ascope_power_min_db,
                               _params->ascope_power_max_db);

  // Time series

  _iTs = new FieldInfo(_dpy, "I time series (V * 10**-3)",
                       _params->I_color, -1, 1);
  _iTsPolyFit = new FieldInfo(_dpy, "I polynomial fit",
                              _params->I_polynomial_color, -1, 1);
  _iTsResidual = new FieldInfo(_dpy, "I residual (V * 10**-3)",
                                _params->detrended_ts_color, -1, 1);

  _qTs = new FieldInfo(_dpy, "Q time series (V * 10**-3)",
                       _params->Q_color, -1, 1);
  _qTsPolyFit = new FieldInfo(_dpy, "Q polynomial fit",
                              _params->Q_polynomial_color, -1, 1);
  _qTsResidual = new FieldInfo(_dpy, "Q residual (V * 10**-3)",
                                _params->detrended_ts_color, -1, 1);
  
  _iqPhasor = new FieldInfo(_dpy, "IQ Phasor diagram",
                            _params->iq_phasor_color, -1, 1);
  
  _runningCpa = new FieldInfo(_dpy, "5-pt running CPA",
                              _params->iq_phasor_color, -1, 1);
  
}

/*****************************************************************
 * FREE : free up allocated memory
 */

void stag_chart_free()
{

  freeField(_powerSpec);
  freeField(_adapFiltSpec);
  freeField(_regrFiltSpec);

  freeField(_stagSpecBoth);
  freeField(_stagFiltSpecBoth);

  freeField(_stagSpecShort);
  freeField(_stagTsPowerShort);
  freeField(_stagTsPhaseShort);
  freeField(_stagFiltSpecShort);
  freeField(_stagFiltSpecShortInterp);
  freeField(_stagFiltSpecShortRatio);

  freeField(_stagSpecLong);
  freeField(_stagTsPowerLong);
  freeField(_stagTsPhaseLong);
  freeField(_stagFiltSpecLong);
  freeField(_stagFiltSpecLongInterp);
  freeField(_stagFiltSpecLongRatio);

  freeField(_stagTsPowerPhaseDiff);
  freeField(_stagFiltSpecPhaseDiff);
  freeField(_stagFiltSpecPhaseDiffInterp);

  freeField(_powerTs);
  freeField(_phaseTs);
  freeField(_phaseResidual);
  freeField(_phaseDiff);
  freeField(_cumulativePhaseDiff);

  freeField(_fftPhaseDiff);

  freeField(_powerSpecReal);
  freeField(_powerAscope);

  freeField(_iTs);
  freeField(_iTsPolyFit);
  freeField(_iTsResidual);

  freeField(_qTs);
  freeField(_qTsPolyFit);
  freeField(_qTsResidual);

  freeField(_iqPhasor);
  freeField(_runningCpa);

}

static void freeField(FieldInfo* &field)
{
  if (field) {
    delete field;
    field = NULL;
  }
}

/*****************************************************************
 * BASE_WIN_DESTROY: Interposition for base frame destroys
 */

static Notify_value
base_win_destroy( Notify_client   client, Destroy_status  status)
{

  switch(status) {
  case DESTROY_CLEANUP:
    
  case DESTROY_PROCESS_DEATH:
    PMU_unregister(_app_name, _params->instance);
    if (_cidd_mem != NULL)  {
      ushm_detach(_cidd_mem);
      if (ushm_nattach(_params->cidd_shmem_key) <= 0) {
	ushm_remove(_params->cidd_shmem_key);
      }
    }
    return notify_next_destroy_func(client,status);
    break;
    
  case DESTROY_CHECKING:
    return NOTIFY_DONE;
    break;
    
  case DESTROY_SAVE_YOURSELF:
    return NOTIFY_DONE;
    break;

  }

  return NOTIFY_DONE;

}

/*****************************************************************
 * MODIFY_XVIEW_OBJECTS : Modify any Xview objects that couldn't
 *    be set up in Devguide. This is primarily to avoid manually
 *    changing any *ui.c file
 */

static void modify_xview_objects()
{
  
  Colormap cmap;
  XColor cell_def;
  XColor rgb_def;

  xv_set(_stag_chart_win1->win1,
         WIN_X,_params->window_x,
         WIN_Y,_params->window_y,
         WIN_HEIGHT,_params->window_height,
         WIN_WIDTH,_params->window_width,
	 NULL);
  
  _win_height =  xv_get(_stag_chart_win1->win1,WIN_HEIGHT);
  _win_width =  xv_get(_stag_chart_win1->win1,WIN_WIDTH);
  _plot_height = _win_height - _params->bottom_margin - _params->top_margin;
  _plot_width = _win_width -  _params->right_margin - _params->left_margin;
  
  _canvas_xid =
    xv_get(canvas_paint_window(_stag_chart_win1->canvas1),XV_XID);

  if (_back_xid != 0) {
    XFreePixmap(_dpy, _back_xid);
  }
  _back_xid =  XCreatePixmap(_dpy, _canvas_xid,
			       _win_width, _win_height,
			       DefaultDepth(_dpy,0));

  cmap = DefaultColormap(_dpy,DefaultScreen(_dpy));
  _def_gc = DefaultGC(_dpy, DefaultScreen(_dpy));

  XAllocNamedColor(_dpy,cmap, _params->foreground_color, &cell_def,&rgb_def);
  _fg_cell = cell_def.pixel;

  XAllocNamedColor(_dpy,cmap, _params->background_color, &cell_def,&rgb_def);
  _bg_cell = cell_def.pixel;

  XAllocNamedColor(_dpy,cmap, _params->grid_line_color, &cell_def,&rgb_def);
  _grid_cell = cell_def.pixel;
  
  XAllocNamedColor(_dpy,cmap, _params->noise_level_color, &cell_def,&rgb_def);
  _noise_cell = cell_def.pixel;
  
  XAllocNamedColor(_dpy,cmap, _params->range_line_color, &cell_def,&rgb_def);
  _range_cell = cell_def.pixel;
  
  _fontst = (XFontStruct *) XLoadQueryFont(_dpy, _params->font_name);
  if(_fontst == NULL) {
    fprintf(stderr,"Can't load font %s\n", _params->font_name);
    exit(-1);
  }
  _font  = _fontst->fid;
  XSetFont(_dpy, _def_gc, _font);

  xv_set(_stag_chart_win1->canvas1, XV_HELP_DATA,
	 "stag_chart:canvas",NULL);

  set_frame_label();

  _data_avail = false;

}

/*****************************************************************
 * SET_FRAME_LABEL : 
 */

void set_frame_label()
{

  xv_set(_stag_chart_win1->win1, FRAME_LABEL,
	 _params->window_label, NULL);
  
}

/*************************************************************************
 * Menu handler for `menu1'.
 */

Menu
stag_field_proc( Menu	menu, Menu_generate op)

{
  
  char *string;
  Menu_item item = (Menu_item ) xv_get(menu, MENU_SELECTED_ITEM);
  
  switch (op) {
  case MENU_DISPLAY:
    break;
    
  case MENU_DISPLAY_DONE:
    break;
    
  case MENU_NOTIFY:
    
    _channel = RadarSpectra::CHANNEL_HC;
    
    string = (char *)  xv_get(item,MENU_STRING);

    /* Set a numerical value for which variable we are dealing with */
    if(strncmp(string,"CHANNEL_HC",10) == 0) {
      _channel = RadarSpectra::CHANNEL_HC;
      _channelName = "CHANNEL_HC";
    } else if(strncmp(string,"CHANNEL_VC",10) == 0) {
      _channel = RadarSpectra::CHANNEL_VC;
      _channelName = "CHANNEL_VC";
    } else if(strncmp(string,"CHANNEL_HX",10) == 0) {
      _channel = RadarSpectra::CHANNEL_HC;
      _channelName = "CHANNEL_HX";
    } else if(strncmp(string,"CHANNEL_VX",10) == 0) {
      _channel = RadarSpectra::CHANNEL_VX;
      _channelName = "CHANNEL_VX";
    } else if(strncmp(string,"QUIT",10) == 0) {
      exit(0);
    }

    if (_params->debug >= Params::DEBUG_VERBOSE) {
      cerr << "====>>>> Changing channel: " << string << endl;
      cerr << "         Channel number: " << _channel << endl;
    }
    
    // load up spectra for this channel

    find_closest_spdb();

    break;
    
  case MENU_NOTIFY_DONE:
    break;
  }
  return menu;
}

/*************************************************************************
 * Event callback function for `canvas1'.
 */
Notify_value
stag_chart_win1_canvas1_event_callback(Xv_window win,
					Event  *event,
					Notify_arg arg,
					Notify_event_type type)

{
  
  xv_get(xv_get(win, CANVAS_PAINT_CANVAS_WINDOW),
	 XV_KEY_DATA, INSTANCE);
  
  /* gxv_start_connections DO NOT EDIT THIS SECTION */

  int xx = event_x(event);
  int yy = event_y(event);

  switch(event_action(event)) {

  case ACTION_MENU:
    if(event_is_down(event)) {
      Menu    menu = (Menu) xv_get(win, WIN_MENU);
      if(menu) menu_show(menu, win, event, 0);
    }
    break;
    
  case WIN_REPAINT:
    draw_plot();
    break;
    
  case ACTION_GO_COLUMN_FORWARD: // up arrow
    if(event_is_up(event)) {
      if (_clickRange > _minRange) {
	_clickRange -= _spectra.getGateSpacing();
        set_click_point_from_user(_clickAz, _clickRange);
	if (_params->debug >= Params::DEBUG_VERBOSE) {
	  cerr << "up arrow, new range: " << _clickRange << endl;
	}
	draw_plot();
      }
    }
    break;
    
  case ACTION_GO_COLUMN_BACKWARD: // down arrow
    if(event_is_up(event)) {
      if (_clickRange < _maxRange) {
	_clickRange += _spectra.getGateSpacing();
        set_click_point_from_user(_clickAz, _clickRange);
	if (_params->debug >= Params::DEBUG_VERBOSE) {
	  cerr << "down arrow, new range: " << _clickRange << endl;
	}
	draw_plot();
      }
    }
    break;
    
  case ACTION_GO_CHAR_BACKWARD: // left arrow
    if(event_is_up(event)) {
      _clickAz -= _deltaAzMedian;
      if (_clickAz < 0) {
        _clickAz += 360.0;
      }
      set_click_point_from_user(_clickAz, _clickRange);
      if (_params->debug >= Params::DEBUG_VERBOSE) {
        cerr << "left arrow, new az: " << _clickAz << endl;
      }
      find_closest_spdb();
      draw_plot();
    }
    break;
    
    case ACTION_GO_CHAR_FORWARD: // right arrow
    if(event_is_up(event)) {
      _clickAz += _deltaAzMedian;
      if (_clickAz >= 360.0) {
        _clickAz -= 360.0;
      }
      set_click_point_from_user(_clickAz, _clickRange);
      if (_params->debug >= Params::DEBUG_VERBOSE) {
        cerr << "right arrow, new az: " << _clickAz << endl;
      }
      find_closest_spdb();
      draw_plot();
    }
    break;
    
  case ACTION_SELECT: // pick point
    if(event_is_up(event)) {

      // check for ASCOPE double click

      if (xx > _ascope_start_x && xx < _ascope_end_x &&
	  yy > _ascope_start_y && yy < _ascope_end_y) {
	// check for double click
	struct timeval tv;
	gettimeofday(&tv, NULL);
	double selectTime = (double) tv.tv_sec + tv.tv_usec / 1000000.0;
	double timeDiff = selectTime - _lastAscopeSelectTime;
	if (timeDiff < 0.25) {
	  double x_fraction =
	    (double) (xx - _ascope_start_x + 1) /
            (double) (_ascope_end_x - _ascope_start_x + 1);
	  _clickRange =
	    _spectra.getStartRange() +
            x_fraction * (_spectra.getGateSpacing() * _nGates);
          set_click_point_from_user(_clickAz, _clickRange);
	  if (_params->debug >= Params::DEBUG_VERBOSE) {
	    cerr << "Double click in ASCOPE" << endl;
	    cerr << "  new _clickRange:" << _clickRange << endl;
	  }
	  draw_plot();
	}
	_lastAscopeSelectTime = selectTime;
      }

    }
    break;
    
  }
  
  /* gxv_end_connections */
  
  return notify_next_event_func(win, (Notify_event) event, arg, type);

}

/*************************************************************************
 * Event callback function for `win1'.
 */

static Notify_value
win1_event_proc(Xv_window  win, Event *event,
                Notify_arg arg, Notify_event_type type)
{

  xv_get(win, XV_KEY_DATA, INSTANCE);
  
  if (event_action(event) != WIN_RESIZE) {
    return notify_next_event_func(win, (Notify_event) event, arg, type);
  }

  // Release backing Pixmap
  
  if(_back_xid) XFreePixmap(_dpy,_back_xid);
  
  // recompute sizes
  
  _win_height =  xv_get(_stag_chart_win1->win1,WIN_HEIGHT);
  _win_width =  xv_get(_stag_chart_win1->win1,WIN_WIDTH);
  _plot_height = _win_height - _params->bottom_margin - _params->top_margin;
  _plot_width = _win_width - _params->right_margin - _params->left_margin;
  _pixels_per_x_val = (double) _plot_width / (_nSamples - 1.0);
  
  if (_params->debug >= Params::DEBUG_VERBOSE) {
    cerr << "Resizing window" << endl;
    cerr << "  nSamples: " << _nSamples << endl;
    cerr << "  pixels_per_x_val: " << _pixels_per_x_val << endl;
    cerr << "  win_height: " << _win_height << endl;
    cerr << "  win_width: " << _win_width << endl;
    cerr << "  plot_height: " << _plot_height << endl;
    cerr << "  plot_width: " << _plot_width << endl;
  }
  
  // create new backing Pixmap
  
  _back_xid =  XCreatePixmap(_dpy, _canvas_xid,
			       _win_width, _win_height,
			       DefaultDepth(_dpy,0));
  
  /* gxv_start_connections DO NOT EDIT THIS SECTION */
  
  /* gxv_end_connections */
  
  return notify_next_event_func(win, (Notify_event) event, arg, type);

}

/************************************************************************
 * TIMER_FUNC: This routine supervises timed operations 
 *   This read data from each source then redraw plot
 *
 */

static void timer_func( Notify_client   client, int which)

{

  PMU_auto_register("In timer_func");

  if (_params->input_mode == Params::INPUT_SPDB) {
    if (check_click()) {
      draw_plot();
    }
  } else {
    if (check_fmq()) {
      draw_plot();
    }
  }
  
}
 
/************************************************************************
 * CHECK_CLICK: check to see whether we have a new click point
 *
 */

static bool check_click()
  
{
  
  static int prev_seq_num = 0;

  PMU_auto_register("Checking for click");

  if(_cidd_mem->pointer_seq_num != prev_seq_num) {

    // new click found
    
    if (_params->debug) {
      cerr << "====>> check_retrieve: new click point <<====" << endl;
    }
    if (_params->debug >= Params::DEBUG_VERBOSE) {
      print_shmem();
    }

    if (_end_time != _cidd_mem->time_current_field) {

      // cidd time has changed, get new data

      _end_time = _cidd_mem->time_current_field;
      _start_time = _end_time - _params->search_interval_secs;

      if (_params->debug) {
	cerr << "Time changed ..." << endl;
	cerr << "  Start time: " << utimstr(_start_time) << endl;
	cerr << "  End   time: " << utimstr(_end_time) << endl;
      }

      if (retrieve_spdb_data()) {
	return false;
      }
      
    }

    // set the click point from CIDD

    set_click_point_from_cidd();
    
    // load spectra data
    
    find_closest_spdb();

    prev_seq_num = _cidd_mem->pointer_seq_num;
    return true;

  }

  return false;

}

/************************************************************************
 * check FMQ for new data
 *
 */

static bool check_fmq()
  
{
  
  bool gotMsg = false;

  if (!_params->seek_to_start_of_fmq) {
    _fmq->seek(DsFmq::FMQ_SEEK_LAST);
  }
  
  if (_fmq->readMsg(&gotMsg, _channel)) {
    cerr << "ERROR - reading FMQ: " << _params->spectra_fmq_url << endl;
    // umsleep(100);
    return false;
  }
  
  // load the spectra from the fmq message

  if (_spectra.disassemble(_fmq->getMsg(), _fmq->getMsgLen())) {
    cerr << "Cannot disassemble message from FMQ" << endl;
  } else {
    if (_params->debug >= Params::DEBUG_VERBOSE) {
      _spectra.printHeader(cerr);
    }
  }
  
  // set data availability flag

  _data_avail = true;
    
  return true;

}

/************************************************************************
 * SET_TIMES: set the times perform the data gathering and drawing
 *
 */

static void print_shmem()
  
{

  fprintf(stderr, "================== CIDD SHMEM ====================\n");

  fprintf(stderr, "  time_current_field: %s\n", utimstr(_cidd_mem->time_current_field));
  fprintf(stderr, "  time_cent: %s\n", utimstr(_cidd_mem->time_cent));
  fprintf(stderr, "  time_min: %s\n", utimstr(_cidd_mem->time_min));
  fprintf(stderr, "  time_max: %s\n", utimstr(_cidd_mem->time_max));

  fprintf(stderr, "  epoch_start: %s\n", utimstr(_cidd_mem->epoch_start));
  fprintf(stderr, "  epoch_end: %s\n", utimstr(_cidd_mem->epoch_end));
  
  fprintf(stderr, "  checkWriteTimeOnRead: %s\n",
	 _cidd_mem->checkWriteTimeOnRead? "true" : "false");
  
  fprintf(stderr, "  latestValidWriteTime: %s\n",
	 utimstr(_cidd_mem->latestValidWriteTime));
  
  fprintf(stderr, "  pointer_seq_num: %d\n", _cidd_mem->pointer_seq_num);
  fprintf(stderr, "  pointer_x: %g\n", _cidd_mem->pointer_x);
  fprintf(stderr, "  pointer_y: %g\n", _cidd_mem->pointer_y);
  fprintf(stderr, "  pointer_lat: %g\n", _cidd_mem->pointer_lat);
  fprintf(stderr, "  pointer_lon: %g\n", _cidd_mem->pointer_lon);
  
  fprintf(stderr, "  datum_lat: %g\n", _cidd_mem->datum_latitude);
  fprintf(stderr, "  datum_lon: %g\n", _cidd_mem->datum_longitude);
  fprintf(stderr, "  data_altitude: %g\n", _cidd_mem->data_altitude);

  fprintf(stderr, "  client_click_time: %s\n",
          DateTime::strm(_cidd_mem->client_click_time).c_str());
  fprintf(stderr, "  client_seq_num: %d\n", _cidd_mem->client_seq_num);
  fprintf(stderr, "  client_x: %g\n", _cidd_mem->client_x);
  fprintf(stderr, "  client_y: %g\n", _cidd_mem->client_y);
  fprintf(stderr, "  client_lat: %g\n", _cidd_mem->client_lat);
  fprintf(stderr, "  client_lon: %g\n", _cidd_mem->client_lon);
  
  fprintf(stderr, "==================================================\n");

}
	  
/**********************************************************************
 * START_TIMER:  Start up the interval timer
 */

void start_timer()
{
  
  struct itimerval timer;
  
  /* set up interval timer interval - 10 ms interval */
  timer.it_value.tv_sec = 0;
  timer.it_value.tv_usec = 10000;
  timer.it_interval.tv_sec = 0;
  timer.it_interval.tv_usec = 10000;
  
  /* Set the interval timer function and start timer */
  notify_set_itimer_func(_stag_chart_win1->win1,
			 (Notify_func)timer_func,
			 ITIMER_REAL, &timer, NULL); /*  */
  timer_func(0,0);
}

/*************************************************************************
 * set click point from CIDD
 */

static void set_click_point_from_cidd()
  
{

  // compute elevation, azimuth and range
  
  _clickElev = _cidd_mem->data_altitude;
  double xx = _cidd_mem->pointer_x;
  double yy = _cidd_mem->pointer_y;
  _clickAz = 0.0;
  if (xx != 0 || yy != 0) {
    _clickAz = atan2(xx, yy) * RAD_TO_DEG;
    if (_clickAz < 0) {
      _clickAz += 360.0;
    }
  }
  _clickRange = sqrt(xx * xx + yy * yy) / cos(_clickElev * DEG_TO_RAD);

  if (_params->debug >= Params::DEBUG_VERBOSE) {
    cerr << "Setting click point from CIDD: " << endl;
    cerr << "  _clickElev: " << _clickElev << endl;
    cerr << "  _clickAz: " << _clickAz << endl;
    cerr << "  _clickRange: " << _clickRange << endl;
  }

}
  
/*************************************************************************
 * set user click point from az and range
 */

static void set_click_point_from_user(double az, double range)
  
{
  
  double xx = range * sin(az * DEG_TO_RAD);
  double yy = range * cos(az * DEG_TO_RAD);

  _cidd_mem->client_x = xx;
  _cidd_mem->client_y = yy;
  
  double lat, lon;

  PJGLatLonPlusDxDy(_cidd_mem->datum_latitude,
                    _cidd_mem->datum_longitude,
                    xx, yy, &lat, &lon);

  _cidd_mem->client_lat = lat;
  _cidd_mem->client_lon = lon;
  _cidd_mem->client_seq_num++;
  _cidd_mem->client_click_time = time(NULL);
  
  // if (_params->debug >= Params::DEBUG_VERBOSE) {
  if (_params->debug) {
    cerr << "Setting click point from USER: " << endl;
    cerr << "  client_click_time: "
         << DateTime::strm(_cidd_mem->client_click_time) << endl;
    cerr << "  client_seq_num: " << _cidd_mem->client_seq_num << endl;
    cerr << "  client_xx: " << _cidd_mem->client_x << endl;
    cerr << "  client_yy: " << _cidd_mem->client_y << endl;
    cerr << "  client_lat: " << _cidd_mem->client_lat << endl;
    cerr << "  client_lon: " << _cidd_mem->client_lon << endl;
  }

}

/*************************************************************************
 * FIND_CLOSEST_SPDB: load data closest to click point
 *
 * Returns 0 on success, -1 on failure
 */

static int find_closest_spdb()

{

  // get reference to chunks

  const vector<Spdb::chunk_t> &chunks = _spdb.getChunks();

  // search through chunks, finding the closest one to the click point
  // also compute median delta azimuth
  
  int dataType = _channel;
  
  double minError = 1.0e99;
  int bestChunk = -1;
  vector<double> deltaAz;
  double prevAz = 0.0;
  for (int ichunk = 0; ichunk < (int) chunks.size(); ichunk++) {

    const Spdb::chunk_t &chunk = chunks[ichunk];
    if (chunk.data_type != dataType) {
      continue;
    }

    RadarSpectra spec;
    if (spec.disassembleHdr(chunk.data, chunk.len)) {
      continue;
    }

    int nGates = spec.getNGates();
    double minRange = spec.getStartRange();
    double maxRange = minRange + spec.getGateSpacing() * (nGates - 1);
    double az = spec.getAzDeg();

    if (ichunk > 0) {
      double dAz = az - prevAz;
      if (dAz > 180) {
        dAz -= 360.0;
      } else if (dAz < -180) {
        dAz += 360.0;
      }
      deltaAz.push_back(dAz);
    }
    prevAz = az;
    
    if (_clickRange < minRange || _clickRange > maxRange) {
      continue;
    }

    double deltaEl = spec.getElevDeg() - _clickElev;
    double deltaAz = spec.getAzDeg() - _clickAz;
    if (deltaAz > 180) {
      deltaAz -= 360.0;
    } else if (deltaAz < -180) {
      deltaAz += 360.0;
    }

    double error = sqrt(deltaAz * deltaAz + deltaEl * deltaEl);
    if (error < minError) {
      bestChunk = ichunk;
      minError = error;
    }

  } // ichunk

  // sort the delta azimuth array
  
  sort(deltaAz.begin(), deltaAz.end());
  if (deltaAz.size() > 2) {
    _deltaAzMedian = deltaAz[deltaAz.size() / 2];
  } else {
    _deltaAzMedian = 1.0;
  }

  // check we have a valid click point
  
  if (minError > _params->max_search_angle_error || bestChunk < 0) {
    _data_avail = false;
    if (_params->debug) {
      cerr << "No suitable chunk found for click point" << endl;
    }
    return 0;
  }
    
  // save the spectra for the click point

  const Spdb::chunk_t &chunk = chunks[bestChunk];
  if (_spectra.disassemble(chunk.data, chunk.len)) {
    cerr << "Cannot disassemble chunk" << endl;
  } else {
    if (_params->debug >= Params::DEBUG_VERBOSE) {
      _spectra.printHeader(cerr);
    }
    // _spectra.print(cerr);
  }
  
  // set data availability flag

  _data_avail = true;
    
  return 0;

}
 
/*****************************************************************
 * compute the number of active plots
 */

static int compute_number_of_plots()
{

  int nPlots = 0;

  if (_params->plot_power_spectrum) {
    nPlots += 3;
  }

  if (_params->plot_power_time_series) {
    nPlots += 2;
  }
  
  if (_params->plot_phase_time_series) {
    nPlots += 2;
    if (_params->plot_residual_phase) {
      nPlots++;
    }
  }

  if (_params->plot_pulse_to_pulse_phase_diff) {
    nPlots++;
  }

  if (_params->plot_cumulative_phase_diff) {
    nPlots++;
  }

  if (_params->plot_fft_phase_diff) {
    nPlots++;
  }

  if (_params->plot_ascope_power) {
    nPlots++;
  }
  
  if (_params->plot_iq_time_series) {
    nPlots += 2;
  }

  if (_params->plot_residual_time_series) {
    nPlots += 2;
  }

  if (_params->plot_iq_phasor) {
    nPlots += 2;
  }

  return nPlots;
     
}

/*************************************************************************
 * clear the plot data
 */

static void clear_plot_data()

{

  _powerSpec->clearData();
  _adapFiltSpec->clearData();
  _regrFiltSpec->clearData();

  _stagSpecBoth->clearData();
  _stagFiltSpecBoth->clearData();

  _stagSpecShort->clearData();
  _stagTsPowerShort->clearData();
  _stagTsPhaseShort->clearData();
  _stagFiltSpecShort->clearData();
  _stagFiltSpecShortInterp->clearData();
  _stagFiltSpecShortRatio->clearData();

  _stagSpecLong->clearData();
  _stagTsPowerLong->clearData();
  _stagTsPhaseLong->clearData();
  _stagFiltSpecLong->clearData();
  _stagFiltSpecLongInterp->clearData();
  _stagFiltSpecLongRatio->clearData();

  _stagTsPowerPhaseDiff->clearData();
  _stagFiltSpecPhaseDiff->clearData();
  _stagFiltSpecPhaseDiffInterp->clearData();

  _powerTs->clearData();
  _phaseTs->clearData();
  _phaseResidual->clearData();
  _phaseDiff->clearData();
  _cumulativePhaseDiff->clearData();

  _fftPhaseDiff->clearData();

  _powerAscope->clearData();
  _powerSpecReal->clearData();

  _iTs->clearData();
  _iTsPolyFit->clearData();
  _iTsResidual->clearData();

  _qTs->clearData();
  _qTsPolyFit->clearData();
  _qTsResidual->clearData();

  _iqPhasor->clearData();
  _runningCpa->clearData();

  _nGates = 0;
  _minRange = 0;
  _maxRange = 0;
  _nSamples = 0;
  _nSamplesHalf = 0;
  _gateNum = 0;
  _gateRange = 0;

}

/*************************************************************************
 * load up plotting data, compute moments etc from spectra
 */

static void load_plot_data()

{

  clear_plot_data();

  if (!_data_avail) {
    return;
  }
  
  _nGates = _spectra.getNGates();
  _minRange = _spectra.getStartRange();
  _maxRange = _minRange + _spectra.getGateSpacing() * (_nGates - 1);
    
  _nSamples = _spectra.getNSamples();
  _nSamplesHalf = _nSamples / 2;
  _pixels_per_x_val = (double) _plot_width / (_nSamples - 1.0);
  
  _gateNum = (int) ((_clickRange - _spectra.getStartRange()) /
		    _spectra.getGateSpacing() + 0.5);

  if (_gateNum < 0 || _gateNum > _nGates - 1) {
    return;
  }

  _gateRange = _spectra.getStartRange() +
    (_gateNum + 0.5) * _spectra.getGateSpacing();

  if (_params->debug >= Params::DEBUG_VERBOSE) {
    cerr << "=====>> _clickRange: " << _clickRange << endl;
    cerr << "=====>> _gateNum: " << _gateNum << endl;
  }

  if (_spectra.isStaggeredPrtMode()) {
    load_plot_data_staggered_prt();
  } else {
    load_plot_data_single_prt();
  }

}

/*************************************************************************
 * load up plotting data for single PRT
 * compute moments etc from spectra
 */

static void load_plot_data_single_prt()

{

  double rcvGainDb = _spectra.getReceiverGainDb();
  double rcvGain = pow(10.0, rcvGainDb / 10.0);
  double sqrtRcvGain = sqrt(rcvGain);

  vector<RadarSpectra::radar_iq_t> gateIq = _spectra.getGateIq(_gateNum);
  TaArray<RadarComplex_t> iq_;
  RadarComplex_t *iq = iq_.alloc(_nSamples);
  for (int ii = 0; ii < _nSamples; ii++) {
    int jj = _nSamples - ii - 1;
    iq[jj].re = gateIq[ii].ival / sqrtRcvGain;
    iq[jj].im = gateIq[ii].qval / sqrtRcvGain;
  }

  // compute moments for this gate
  
  DsRadarCalib calib;
  calib.setWavelengthCm(_spectra.getWavelengthCm());
  calib.setNoiseDbmHc(_spectra.getNoiseDbm() - rcvGainDb);
  calib.setReceiverGainDbHc(0.0);
  calib.setBaseDbz1kmHc(_spectra.getBaseDbz1km());

  RadarMoments moments(_gateNum + 1,
                       _params->debug >= Params::DEBUG_VERBOSE, false);

  moments.setNSamples(_nSamples);
  moments.init(_spectra.getPrt(), _spectra.getWavelengthCm() / 100.0,
	       _spectra.getStartRange(), _spectra.getGateSpacing());
  moments.setCalib(calib);
  
  // compute moments

  moments.singlePol(iq, _gateNum, false, _gateMoments);
  _gateMoments.mvar = moments.computeMvar(iq, _nSamples, _spectra.getPrt());
  _gateMoments.tpt = moments.computeTpt(iq, _nSamples);
  _gateMoments.cpd = moments.computeCpd(iq, _nSamples);

  // compute the power for each gate (AScope mode)
  
  double range = _spectra.getStartRange();
  for (int igate = 0; igate < _nGates; igate++) {

    vector<RadarSpectra::radar_iq_t> gateIq = _spectra.getGateIq(igate);
    double sumPower = 0.0;
    for (int ii = 0; ii < _nSamples; ii++) {
      double ival = gateIq[ii].ival;
      double qval = gateIq[ii].qval;
      sumPower += ival * ival + qval * qval;
    }
    double meanPower = sumPower / _nSamples;
    double powerDbm = 10.0 * log10(meanPower);
    _powerAscope->addData(range, powerDbm);
    range += _spectra.getGateSpacing();

  }
  
}

/*************************************************************************
 * load up plotting data for staggered PRT
 * compute moments etc from spectra
 */

static void load_plot_data_staggered_prt()

{

  // copy the IQ data
  
  vector<RadarSpectra::radar_iq_t> gateIq = _spectra.getGateIq(_gateNum);
  TaArray<RadarComplex_t> iq_;
  RadarComplex_t *iq = iq_.alloc(_nSamples);
  for (int ii = 0; ii < _nSamples; ii++) {
    int jj = _nSamples - ii - 1;
    iq[jj].set(gateIq[ii].ival, gateIq[ii].qval);
  }

  // separate the time series into short/long PRT series
  
  TaArray<RadarComplex_t> iqShort_, iqLong_;
  RadarComplex_t *iqShort = iqShort_.alloc(_nSamplesHalf);
  RadarComplex_t *iqLong = iqLong_.alloc(_nSamplesHalf);
  RadarMoments::separateStagIq(_nSamples, iq, iqShort, iqLong);

  // compute phase diffs between consecutive short and long samples
  
  TaArray<RadarComplex_t> diffShort_, diffLong_;
  RadarComplex_t *diffShort = diffShort_.alloc(_nSamplesHalf);
  RadarComplex_t *diffLong = diffLong_.alloc(_nSamplesHalf);
  
  for (int ii = 0; ii < _nSamplesHalf - 1; ii++) {
    diffShort[ii] =
      RadarComplex::conjugateProduct(iqLong[ii], iqShort[ii]);
    diffLong[ii] =
      RadarComplex::conjugateProduct(iqShort[ii+1], iqLong[ii]);
  }

  // make short-prt pseudo time series
  
  TaArray<RadarComplex_t> pseudoShort_;
  RadarComplex_t *pseudoShort = pseudoShort_.alloc(_nSamplesHalf);
  RadarComplex_t phaseShort = RadarComplex::norm(iqShort[0]);
  
  for (int ii = 0; ii < _nSamplesHalf; ii++) {
    double mag = RadarComplex::mag(iqShort[ii]);
    pseudoShort[ii].re = mag * phaseShort.re;
    pseudoShort[ii].im = mag * phaseShort.im;
    phaseShort = RadarComplex::normComplexProduct(phaseShort, diffShort[ii]);
  }

  // make long-prt pseudo time series
  
  TaArray<RadarComplex_t> pseudoLong_;
  RadarComplex_t *pseudoLong = pseudoLong_.alloc(_nSamplesHalf);
  RadarComplex_t phaseLong = RadarComplex::norm(iqLong[0]);
  
  for (int ii = 0; ii < _nSamplesHalf; ii++) {
    double mag = RadarComplex::mag(iqLong[ii]);
    pseudoLong[ii].re = mag * phaseLong.re;
    pseudoLong[ii].im = mag * phaseLong.im;
    phaseLong = RadarComplex::normComplexProduct(phaseLong, diffLong[ii]);
  }

  // compute spectra for unfiltered pseudo time series
  
  RadarFft fftHalf(_nSamplesHalf);
  
  TaArray<RadarComplex_t> pseudoShortSpec_;
  RadarComplex_t *pseudoShortSpec = pseudoShortSpec_.alloc(_nSamplesHalf);
  fftHalf.fwd(pseudoShort, pseudoShortSpec);

  TaArray<RadarComplex_t> pseudoLongSpec_;
  RadarComplex_t *pseudoLongSpec = pseudoLongSpec_.alloc(_nSamplesHalf);
  fftHalf.fwd(pseudoLong, pseudoLongSpec);

  // set up cal and moments objects
  
  DsRadarCalib calib;
  calib.setWavelengthCm(_spectra.getWavelengthCm());
  calib.setNoiseDbmHc(_spectra.getNoiseDbm());
  calib.setReceiverGainDbHc(_spectra.getReceiverGainDb());
  // calib.setReceiverGainDbHc(0.0);
  calib.setBaseDbz1kmHc(_spectra.getBaseDbz1km());
  
  RadarMoments moments(_gateNum + 1,
                       _params->debug >= Params::DEBUG_VERBOSE, false);
  moments.setNSamples(_nSamples);
  moments.initStagPrt(_spectra.getPrtShort(),
                      _spectra.getPrtLong(),
                      _spectra.getStaggeredM(),
                      _spectra.getStaggeredN(),
                      _spectra.getNGatesPrtShort(),
                      _spectra.getNGatesPrtLong(),
                      _spectra.getWavelengthCm() / 100.0,
                      _spectra.getStartRange(),
                      _spectra.getGateSpacing());

  moments.setCalib(calib);
  
  // compute moments for this gate
  
  moments.singlePolStagPrt(iq, iqShort, iqLong, _gateNum, false, _gateMoments);
  _gateMoments.mvar = moments.computeMvar(iq, _nSamples, _spectra.getPrt());
  _gateMoments.tpt = moments.computeTpt(iq, _nSamples);
  _gateMoments.cpd = moments.computeCpd(iq, _nSamples);
  
  // compute size of expanded time series for staggered PRT mode

  int nExpanded = RadarMoments::computeNExpandedStagPrt
    (_nSamples, _spectra.getStaggeredM(), _spectra.getStaggeredN());
  
  // create window for the expanded time series
  
  TaArray<double> windowExp_;
  double *windowExp = windowExp_.alloc(nExpanded);
  if (_params->window == Params::WINDOW_RECT) {
    RadarMoments::initWindowRect(nExpanded, windowExp);
  } else if (_params->window == Params::WINDOW_BLACKMAN) {
    RadarMoments::initWindowBlackman(nExpanded, windowExp);
  } else {
    RadarMoments::initWindowVonhann(nExpanded, windowExp);
  }
  RadarFft fftExp(nExpanded);

  // initialize regression filter

  RegressionFilter regrF;
  regrF.setupStaggered(_nSamples,
                       _spectra.getStaggeredM(),
                       _spectra.getStaggeredN(),
                       _params->regression_polynomial_order);

  // apply the regression filter to the non-windowed time series
  
  TaArray<RadarComplex_t> regrFiltered_;
  RadarComplex_t *regrFiltered = regrFiltered_.alloc(_nSamples);
  double regrFilterRatio = 1.0;
  double regrSpectralNoise = 1.0;
  double regrSpectralSnr = 1.0e-13;
  
  moments.applyRegrFilterStagPrt
    (_nSamples, nExpanded, fftExp, regrF,
     iq, _spectra.getNoise(),
     // _params->regression_interp_across_notch,
     false,
     regrFiltered, regrFilterRatio, regrSpectralNoise, regrSpectralSnr);
  
  // separate the filtered time series into short/long PRT series
  // compute filtered moments
  
  TaArray<RadarComplex_t> regrShort_, regrLong_;
  RadarComplex_t *regrShort = regrShort_.alloc(_nSamplesHalf);
  RadarComplex_t *regrLong = regrLong_.alloc(_nSamplesHalf);
  RadarMoments::separateStagIq(_nSamples, regrFiltered, regrShort, regrLong);
  moments.singlePolStagPrt(regrFiltered, regrShort, regrLong,
                           _gateNum, false, _filtMoments);

  // compute expanded time series
  
  TaArray<RadarComplex_t> iqExp_;
  RadarComplex_t *iqExp = iqExp_.alloc(nExpanded);
  RadarMoments::expandStagIq(_nSamples, nExpanded,
                             _spectra.getStaggeredM(),
                             _spectra.getStaggeredN(),
                             iq, iqExp);
  
  TaArray<RadarComplex_t> regrExp_;
  RadarComplex_t *regrExp = regrExp_.alloc(nExpanded);
  RadarMoments::expandStagIq(_nSamples, nExpanded,
                             _spectra.getStaggeredM(),
                             _spectra.getStaggeredN(),
                             regrFiltered, regrExp);

  // apply window to these expanded series
  
  TaArray<RadarComplex_t> iqWindowed_;
  RadarComplex_t *iqWindowed = iqWindowed_.alloc(nExpanded);
  RadarMoments::applyWindow(iqExp, windowExp, iqWindowed, nExpanded);
  
  TaArray<RadarComplex_t> regrWindowed_;
  RadarComplex_t *regrWindowed = regrWindowed_.alloc(nExpanded);
  RadarMoments::applyWindow(regrExp, windowExp, regrWindowed, nExpanded);

  // compute power spectrum - complex
  
  TaArray<RadarComplex_t> powerSpec_;
  RadarComplex_t *powerSpec = powerSpec_.alloc(nExpanded);
  fftExp.fwd(iqWindowed, powerSpec);

  TaArray<RadarComplex_t> regrSpec_;
  RadarComplex_t *regrSpec = regrSpec_.alloc(nExpanded);
  fftExp.fwd(regrWindowed, regrSpec);
  
  // compute power spectrum - real
  
  TaArray<double> powerSpecReal_;
  double *powerSpecReal = powerSpecReal_.alloc(nExpanded);
  RadarComplex::loadPower(powerSpec, powerSpecReal, nExpanded);

  TaArray<double> regrSpecReal_;
  double *regrSpecReal = regrSpecReal_.alloc(nExpanded);
  RadarComplex::loadPower(regrSpec, regrSpecReal, nExpanded);

  // get the polygon fit
  
  const RadarComplex_t *regrPolyFit = regrF.getPolyfitIq();
  const double *regrX = regrF.getX();
  
  /////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////
  // pseudo time series for filtered data

  // compute phase diffs between consecutive short and long samples

  TaArray<RadarComplex_t> filtDiffShort_, filtDiffLong_;
  RadarComplex_t *filtDiffShort = filtDiffShort_.alloc(_nSamplesHalf);
  RadarComplex_t *filtDiffLong = filtDiffLong_.alloc(_nSamplesHalf);
  
  for (int ii = 0; ii < _nSamplesHalf - 1; ii++) {
    filtDiffShort[ii] =
      RadarComplex::conjugateProduct(regrLong[ii], regrShort[ii]);
    filtDiffLong[ii] =
      RadarComplex::conjugateProduct(regrShort[ii+1], regrLong[ii]);
  }

  // filtered short-prt pseudo time series
  
  TaArray<RadarComplex_t> pseudoFiltShort_;
  RadarComplex_t *pseudoFiltShort = pseudoFiltShort_.alloc(_nSamplesHalf);
  RadarComplex_t phaseFiltShort = RadarComplex::norm(regrShort[0]);

  for (int ii = 0; ii < _nSamplesHalf; ii++) {
    double mag = RadarComplex::mag(regrShort[ii]);
    pseudoFiltShort[ii].re = mag * phaseFiltShort.re;
    pseudoFiltShort[ii].im = mag * phaseFiltShort.im;
    phaseFiltShort = 
      RadarComplex::normComplexProduct(phaseFiltShort, filtDiffShort[ii]);
  }

  // filtered long-prt pseudo time series
  
  TaArray<RadarComplex_t> pseudoFiltLong_;
  RadarComplex_t *pseudoFiltLong = pseudoFiltLong_.alloc(_nSamplesHalf);
  RadarComplex_t phaseFiltLong = RadarComplex::norm(regrLong[0]);
  
  for (int ii = 0; ii < _nSamplesHalf; ii++) {
    double mag = RadarComplex::mag(regrLong[ii]);
    pseudoFiltLong[ii].re = mag * phaseFiltLong.re;
    pseudoFiltLong[ii].im = mag * phaseFiltLong.im;
    phaseFiltLong = 
      RadarComplex::normComplexProduct(phaseFiltLong, filtDiffLong[ii]);
  }
  
  // compute spectra for filtered pseudo time series
  
  TaArray<RadarComplex_t> pseudoFiltShortSpec_;
  RadarComplex_t *pseudoFiltShortSpec = 
    pseudoFiltShortSpec_.alloc(_nSamplesHalf);
  fftHalf.fwd(pseudoFiltShort, pseudoFiltShortSpec);
  
  TaArray<RadarComplex_t> pseudoFiltLongSpec_;
  RadarComplex_t *pseudoFiltLongSpec = 
    pseudoFiltLongSpec_.alloc(_nSamplesHalf);
  fftHalf.fwd(pseudoFiltLong, pseudoFiltLongSpec);

  double powerPseudoShort =
    RadarComplex::meanPower(pseudoShortSpec, _nSamplesHalf);
  double powerPseudoLong =
    RadarComplex::meanPower(pseudoLongSpec, _nSamplesHalf);
  double powerPseudoFiltShort =
    RadarComplex::meanPower(pseudoFiltShortSpec, _nSamplesHalf);
  double powerPseudoFiltLong =
    RadarComplex::meanPower(pseudoFiltLongSpec, _nSamplesHalf);

  double clutterShortDbm =
    10.0 * log10(powerPseudoShort / powerPseudoFiltShort);
  double clutterLongDbm =
    10.0 * log10(powerPseudoLong / powerPseudoFiltLong);

  cerr << "11111 pwr short, long, filts, filtl, dbmS, dbzL: "
       << powerPseudoShort << ", "
       << powerPseudoLong << ", "
       << powerPseudoFiltShort << ", "
       << powerPseudoFiltLong << ", "
       << clutterShortDbm << ", "
       << clutterLongDbm << endl;

  // if clutter is present, fill in the notch

  TaArray<RadarComplex_t> pseudoInterpShortSpec_;
  RadarComplex_t *pseudoInterpShortSpec = 
    pseudoInterpShortSpec_.alloc(_nSamplesHalf);
  memcpy(pseudoInterpShortSpec, pseudoFiltShortSpec,
         _nSamplesHalf * sizeof(RadarComplex_t));

  if (clutterShortDbm > 3.0) {
    int notchWidth = 7;
    int notchWidthHalf = notchWidth / 2;
    int startIndex = _nSamplesHalf - notchWidthHalf;
    int endIndex = notchWidthHalf;
    int nCenter = _nSamplesHalf / 2;
    double powerStart = RadarComplex::power(pseudoFiltShortSpec[startIndex]);
    double powerEnd = RadarComplex::power(pseudoFiltShortSpec[endIndex]);
    double deltaPower = powerEnd - powerStart;
    for (int ii = nCenter - notchWidthHalf + 1;
         ii < nCenter + notchWidthHalf; ii++) {
      int jj = (ii + _nSamplesHalf / 2) % _nSamplesHalf;
      int kk = ii - (nCenter - notchWidthHalf);
      double interpFraction = (double) kk / (double) notchWidth;
      double interpPower = powerStart + interpFraction * deltaPower;
      double origPower = RadarComplex::power(pseudoFiltShortSpec[jj]);
      double powerRatio = interpPower / origPower;
      if (powerRatio > 1.0) {
        double magRatio = sqrt(powerRatio);
        pseudoInterpShortSpec[jj].re *= magRatio;
        pseudoInterpShortSpec[jj].im *= magRatio;
      }
    }
  }

  TaArray<RadarComplex_t> pseudoInterpLongSpec_;
  RadarComplex_t *pseudoInterpLongSpec = 
    pseudoInterpLongSpec_.alloc(_nSamplesHalf);
  memcpy(pseudoInterpLongSpec, pseudoFiltLongSpec,
         _nSamplesHalf * sizeof(RadarComplex_t));

  if (clutterLongDbm > 3.0) {
    int notchWidth = 7;
    int notchWidthHalf = notchWidth / 2;
    int startIndex = _nSamplesHalf - notchWidthHalf;
    int endIndex = notchWidthHalf;
    int nCenter = _nSamplesHalf / 2;
    double powerStart = RadarComplex::power(pseudoFiltLongSpec[startIndex]);
    double powerEnd = RadarComplex::power(pseudoFiltLongSpec[endIndex]);
    double deltaPower = powerEnd - powerStart;
    for (int ii = nCenter - notchWidthHalf + 1;
         ii < nCenter + notchWidthHalf; ii++) {
      int jj = (ii + _nSamplesHalf / 2) % _nSamplesHalf;
      int kk = ii - (nCenter - notchWidthHalf);
      double interpFraction = (double) kk / (double) notchWidth;
      double interpPower = powerStart + interpFraction * deltaPower;
      double origPower = RadarComplex::power(pseudoFiltLongSpec[jj]);
      double powerRatio = interpPower / origPower;
      if (powerRatio > 1.0) {
        double magRatio = sqrt(powerRatio);
        pseudoInterpLongSpec[jj].re *= magRatio;
        pseudoInterpLongSpec[jj].im *= magRatio;
      }
    }
  }

  //   double powerPseudoInterpShort =
  //     RadarComplex::meanPower(pseudoInterpShortSpec, _nSamplesHalf);
  //   double powerPseudoInterpLong =
  //     RadarComplex::meanPower(pseudoInterpLongSpec, _nSamplesHalf);
  
  //   double clutterShortInterpDbm =
  //     10.0 * log10(powerPseudoShort / powerPseudoInterpShort);
  //   double clutterLongInterpDbm =
  //     10.0 * log10(powerPseudoLong / powerPseudoInterpLong);
  
  //   cerr << "22222 pwr short, long, filts, filtl, dbmS, dbzL: "
  //        << powerPseudoShort << ", "
  //        << powerPseudoLong << ", "
  //        << powerPseudoFiltShort << ", "
  //        << powerPseudoFiltLong << ", "
  //        << clutterShortInterpDbm << ", "
  //        << clutterLongInterpDbm << endl;

  /////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////

  // populate data fields
  
  double jtime = 0.0;
  double cumulativeDiff = 0.0;
  for (int ii = 0; ii < _nSamples; ii++) {
    
    double power_td = iq[ii].re * iq[ii].re + iq[ii].im * iq[ii].im;
    double phase_td = atan2(iq[ii].im, iq[ii].re) * RAD_TO_DEG;
    double tdDb = 10.0 * log10(power_td) /* - _spectra.getReceiverGainDb() */;
    
    _powerTs->addData(jtime, tdDb);
    _phaseTs->addData(jtime, phase_td);
    _phaseResidual->addData(jtime, phase_td);
    
    // compute pulse-to-pulse phase difference
    if (ii > 0) {
      RadarComplex_t diff = RadarComplex::conjugateProduct(iq[ii], iq[ii - 1]);
      double phaseDiff = RadarComplex::argDeg(diff);
      cumulativeDiff += phaseDiff;
      if (ii == 1) {
	_phaseDiff->addData(jtime, phaseDiff); // first diff equals second
      }
      _phaseDiff->addData(jtime, phaseDiff);
      _cumulativePhaseDiff->addData(jtime, cumulativeDiff);
    }

    // IQ time series
    
    _iTs->addData(regrX[ii], iq[ii].re);
    _qTs->addData(regrX[ii], iq[ii].im);
    
    _iTsPolyFit->addData(regrX[ii], regrPolyFit[ii].re);
    _qTsResidual->addData(regrX[ii], regrPolyFit[ii].im);

    _iTsPolyFit->addData(regrX[ii], regrPolyFit[ii].re);
    _qTsResidual->addData(regrX[ii], regrPolyFit[ii].im);

    if (ii % 2 == 0) {
      jtime += _spectra.getStaggeredM();
    } else {
      jtime += _spectra.getStaggeredN();
    }

  } // ii

  // plot data for power for each gate (AScope mode)
  
  double range = _spectra.getStartRange();
  for (int igate = 0; igate < _nGates; igate++) {

    vector<RadarSpectra::radar_iq_t> gateIq = _spectra.getGateIq(igate);
    double sumPower = 0.0;
    for (int ii = 0; ii < _nSamples; ii++) {
      double ival = gateIq[ii].ival;
      double qval = gateIq[ii].qval;
      sumPower += ival * ival + qval * qval;
    }
    double meanPower = sumPower / _nSamples;
    double powerDbm = 10.0 * log10(meanPower) /* - _spectra.getReceiverGainDb() */;
    _powerAscope->addData(range, powerDbm);
    range += _spectra.getGateSpacing();

  }

  // staggered spectra data
  
  double itime = 0.0;
  for (int ii = 0; ii < nExpanded; ii++) {
    int jj = (ii + nExpanded / 2) % nExpanded;
    double powerDb =
      10.0 * log10(powerSpecReal[jj]) /* - _spectra.getReceiverGainDb() */;
    double regrDb =
      10.0 * log10(regrSpecReal[jj]) /* - _spectra.getReceiverGainDb() */;
    _stagSpecBoth->addData(itime, powerDb);
    _stagFiltSpecBoth->addData(itime, regrDb);
    itime++;
  } // ii

  // plot data for unfiltered pseudo time series
  
  itime = 0.0;
  for (int ii = 0; ii < _nSamplesHalf; ii++) {
    
    double powerShort = RadarComplex::power(pseudoShort[ii]);
    double powerDbShort = 10.0 * log10(powerShort);
    double phaseShort = RadarComplex::argDeg(pseudoShort[ii]);
    _stagTsPowerShort->addData(itime, powerDbShort);
    _stagTsPhaseShort->addData(itime, phaseShort);
    
    double powerLong = RadarComplex::power(pseudoLong[ii]);
    double powerDbLong = 10.0 * log10(powerLong);
    double phaseLong = RadarComplex::argDeg(pseudoLong[ii]);
    _stagTsPowerLong->addData(itime, powerDbLong);
    _stagTsPhaseLong->addData(itime, phaseLong);

    itime++;
    
  } // ii
  
  itime = 0.0;
  for (int ii = 0; ii < _nSamplesHalf; ii++) {
    
    int jj = (ii + _nSamplesHalf / 2) % _nSamplesHalf;
    
    double power = RadarComplex::power(pseudoShortSpec[jj]);
    double powerDb = 10.0 * log10(power);
    _stagSpecShort->addData(itime, powerDb);
    
    power = RadarComplex::power(pseudoLongSpec[jj]);
    powerDb = 10.0 * log10(power);
    _stagSpecLong->addData(itime, powerDb);

    itime++;
    
  } // ii
  
  itime = 0.0;
  for (int ii = 0; ii < _nSamplesHalf; ii++) {
    
    int jj = (ii + _nSamplesHalf / 2) % _nSamplesHalf;
    
    double power = RadarComplex::power(pseudoFiltShortSpec[jj]);
    double powerDb = 10.0 * log10(power);
    _stagFiltSpecShort->addData(itime, powerDb);
    
    power = RadarComplex::power(pseudoFiltLongSpec[jj]);
    powerDb = 10.0 * log10(power);
    _stagFiltSpecLong->addData(itime, powerDb);

    itime++;
    
  } // ii

  //////////////////////////////////////////////////////////////
  // compute spectra for both short and long series
  
  TaArray<RadarComplex_t> shortSpec_;
  RadarComplex_t *shortSpec = shortSpec_.alloc(_nSamplesHalf);
  fftHalf.fwd(iqShort, shortSpec);

  TaArray<RadarComplex_t> longSpec_;
  RadarComplex_t *longSpec = longSpec_.alloc(_nSamplesHalf);
  fftHalf.fwd(iqLong, longSpec);
  
  TaArray<RadarComplex_t> filtShortSpec_;
  RadarComplex_t *filtShortSpec = filtShortSpec_.alloc(_nSamplesHalf);
  fftHalf.fwd(regrShort, filtShortSpec);
  
  TaArray<RadarComplex_t> filtLongSpec_;
  RadarComplex_t *filtLongSpec = filtLongSpec_.alloc(_nSamplesHalf);
  fftHalf.fwd(regrLong, filtLongSpec);

  TaArray<double> powerLongSpec_, powerShortSpec_;
  double *powerLongSpec = powerLongSpec_.alloc(_nSamplesHalf);
  double *powerShortSpec = powerShortSpec_.alloc(_nSamplesHalf);

  // unfiltered

  itime = 0.0;
  for (int ii = 0; ii < _nSamplesHalf; ii++) {
    
    int jj = (ii + _nSamplesHalf / 2) % _nSamplesHalf;
    
    double power = RadarComplex::power(pseudoFiltShortSpec[jj]);
    // double powerDb = 10.0 * log10(power);
    // double phase = RadarComplex::argDeg(pseudoFiltShortSpec[jj]);
    // _stagTsPhaseShort->addData(itime, phase);
    powerShortSpec[jj] = power;
    
    power = RadarComplex::power(pseudoFiltLongSpec[jj]);
    // powerDb = 10.0 * log10(power);
    // phase = RadarComplex::argDeg(pseudoFiltLongSpec[jj]);
    // _stagTsPhaseLong->addData(itime, phase);
    powerLongSpec[jj] = power;

    RadarComplex_t diff =
      RadarComplex::conjugateProduct(pseudoFiltLongSpec[jj], pseudoFiltShortSpec[jj]);
    double diffPhase = RadarComplex::argDeg(diff);
    
    _stagTsPowerPhaseDiff->addData(itime, diffPhase);
    
    itime++;
    
  } // ii
  
  // filtered

  TaArray<double> powerFiltLongSpec_, powerFiltShortSpec_;
  double *powerFiltLongSpec = powerFiltLongSpec_.alloc(_nSamplesHalf);
  double *powerFiltShortSpec = powerFiltShortSpec_.alloc(_nSamplesHalf);

  itime = 0.0;
  for (int ii = 0; ii < _nSamplesHalf; ii++) {
    
    int jj = (ii + _nSamplesHalf / 2) % _nSamplesHalf;
    
    double power = RadarComplex::power(filtShortSpec[jj]);
    // double powerDb = 10.0 * log10(power);
    // double phase = RadarComplex::argDeg(filtShortSpec[jj]);
    // _stagFiltSpecShort->addData(itime, powerDb);
    powerFiltShortSpec[jj] = power;

    power = RadarComplex::power(filtLongSpec[jj]);
    // powerDb = 10.0 * log10(power);
    // phase = RadarComplex::argDeg(filtLongSpec[jj]);
    // _stagFiltSpecLong->addData(itime, powerDb);
    powerFiltLongSpec[jj] = power;
    
    RadarComplex_t diff =
      RadarComplex::conjugateProduct(filtLongSpec[jj], filtShortSpec[jj]);
    double diffPhase = RadarComplex::argDeg(diff);
    
    _stagFiltSpecPhaseDiff->addData(itime, diffPhase);

    itime++;
    
  } // ii

  // compute power ratio for filtered spec

  itime = 0;
  for (int ii = 0; ii < _nSamplesHalf; ii++) {
    
    int jj = (ii + _nSamplesHalf / 2) % _nSamplesHalf;

    double ratioShort = powerFiltShortSpec[jj] / powerShortSpec[jj];
    double ratioShortDb = 10.0 * log10(ratioShort);
    _stagFiltSpecShortRatio->addData(itime, ratioShortDb);
    
    double ratioLong = powerFiltLongSpec[jj] / powerLongSpec[jj];
    double ratioLongDb = 10.0 * log10(ratioLong);
    _stagFiltSpecLongRatio->addData(itime, ratioLongDb);

    //     cerr << "jj, ratioShort, ratioLong, shortDb, longDb: " << ii << ", "
    //          << ratioShort << ", " << ratioLong << ", "
    //          << ratioShortDb << ", " << ratioLongDb << endl;    

    itime++;
    
  }

  // recombine short and long PRT series into full series

#ifdef JUNK
  fftHalf.inv(filtShortSpec, regrShort);
  fftHalf.inv(filtLongSpec, regrLong);
  RadarMoments::combineStagIq(_nSamples, regrShort, regrLong, regrFiltered);
  RadarMoments::expandStagIq(_nSamples, nExpanded,
                             _spectra.getStaggeredM(),
                             _spectra.getStaggeredN(),
                             regrFiltered, regrExp);
  RadarMoments::applyWindow(regrExp, windowExp, regrWindowed, nExpanded);
  fftExp.fwd(regrWindowed, regrSpec);
  RadarComplex::loadPower(regrSpec, regrSpecReal, nExpanded);
#endif

  // load up staggered spectra data

  _stagFiltSpecBoth->clearData();
  itime = 0.0;
  for (int ii = 0; ii < nExpanded; ii++) {
    int jj = (ii + nExpanded / 2) % nExpanded;
    double regrDb =
      10.0 * log10(regrSpecReal[jj]) /* - _spectra.getReceiverGainDb() */;
    _stagFiltSpecBoth->addData(itime, regrDb);
    itime++;
  } // ii
  
}

/*************************************************************************
 * Request SPDB data for time as set by CIDD
 *
 * Returns 0 on success, -1 on failure
 */

static int retrieve_spdb_data()

{

  // get the data
  
  if (_params->debug) {
    cerr << "Retrieving spdb data" << endl;
    cerr << "  Start time: " << utimstr(_start_time) << endl;
    cerr << "  End   time: " << utimstr(_end_time) << endl;
  }

  // should we check write time?
  
  if (_cidd_mem &&
      _cidd_mem->checkWriteTimeOnRead) {
    _spdb.setCheckWriteTimeOnGet(_cidd_mem->latestValidWriteTime);
  } else {
    _spdb.clearCheckWriteTimeOnGet();
  }
  
  if (_spdb.getInterval(_params->spectra_spdb_url,
			_start_time, _end_time)) {
    cerr << "ERROR - retrieve_data" << endl;
    cerr << _spdb.getErrStr() << endl;
    return -1;
  }
  
  if (_params->debug) {
    cerr << "Number of chunks found: " << _spdb.getChunks().size() << endl;
  }

  return 0;

}

/*
 * stag_chart_ui.c - User interface object initialization functions.
 */

/*
 * Create object `menu1' in the specified instance.
 */
Xv_opaque
stag_chart_menu1_create(caddr_t ip, Xv_opaque owner)
{
  extern Menu		stag_field_proc(Menu, Menu_generate);
  Xv_opaque	obj;

  obj = xv_create(XV_NULL, MENU_CHOICE_MENU,
		  XV_KEY_DATA, INSTANCE, ip,
		  MENU_GEN_PROC, stag_field_proc,
		  MENU_TITLE_ITEM, owner ? "" : "Show",

		  MENU_ITEM,
		  XV_KEY_DATA, INSTANCE, ip,
		  MENU_STRING, "CHANNEL_HC",
		  NULL,

		  MENU_ITEM, 
		  XV_KEY_DATA, INSTANCE, ip,
		  MENU_STRING, "CHANNEL_VC",
		  NULL,

		  MENU_ITEM,
		  XV_KEY_DATA, INSTANCE, ip,
		  MENU_STRING, "CHANNEL_HX",
		  NULL,

		  MENU_ITEM,
		  XV_KEY_DATA, INSTANCE, ip,
		  MENU_STRING, "CHANNEL_VX",
		  NULL,

		  MENU_ITEM,
		  XV_KEY_DATA, INSTANCE, ip,
		  MENU_STRING, "QUIT",
		  NULL,

		  NULL);

  return obj;

}

/*
 * Initialize an instance of object `win1'.
 */
static stag_chart_win1_objects *
stag_chart_win1_objects_initialize(stag_chart_win1_objects *ip, Xv_opaque owner)
{
  if (!ip && !(ip = (stag_chart_win1_objects *) calloc(1, sizeof (stag_chart_win1_objects))))
    return (stag_chart_win1_objects *) NULL;
  if (!ip->win1)
    ip->win1 = stag_chart_win1_win1_create(ip, owner);
  if (!ip->canvas1)
    ip->canvas1 = stag_chart_win1_canvas1_create(ip, ip->win1);
  return ip;
}

/*
 * Create object `win1' in the specified instance.
 */

static Xv_opaque
stag_chart_win1_win1_create(stag_chart_win1_objects *ip, Xv_opaque owner)
{
	extern Notify_value win1_event_proc(Xv_window, Event *, Notify_arg, Notify_event_type);
	Xv_opaque	obj;
	Xv_opaque		win1_image;
	static unsigned short	win1_bits[] = {
#include "spectra_plot.icon"
	};
	
	win1_image = xv_create(XV_NULL, SERVER_IMAGE,
		SERVER_IMAGE_DEPTH, 1,
		SERVER_IMAGE_BITS, win1_bits,
		XV_WIDTH, 64,
		XV_HEIGHT, 64,
		NULL);
	obj = xv_create(owner, FRAME,
		XV_KEY_DATA, INSTANCE, ip,
		XV_WIDTH, 266,
		XV_HEIGHT, 234,
		XV_LABEL, "Station Trends",
		FRAME_SHOW_FOOTER, FALSE,
		FRAME_SHOW_RESIZE_CORNER, TRUE,
		FRAME_ICON, xv_create(XV_NULL, ICON,
			ICON_IMAGE, win1_image,
			NULL),
		NULL);
	xv_set(obj, WIN_CONSUME_EVENTS,
		WIN_MOUSE_BUTTONS,
		LOC_MOVE,
		LOC_DRAG,
		LOC_WINENTER,
		LOC_WINEXIT,
		WIN_ASCII_EVENTS,
		WIN_LEFT_KEYS,
		WIN_RIGHT_KEYS,
		WIN_TOP_KEYS,
		NULL, NULL);
	notify_interpose_event_func(obj,
		(Notify_func) win1_event_proc, NOTIFY_SAFE);
	return obj;
}

/*
 * Create object `canvas1' in the specified instance.
 */
static Xv_opaque
stag_chart_win1_canvas1_create(stag_chart_win1_objects *ip, Xv_opaque owner)
{

  Xv_opaque obj = xv_create(owner, CANVAS,
			    XV_KEY_DATA, INSTANCE, ip,
			    XV_X, 0,
			    XV_Y, 0,
			    XV_WIDTH, WIN_EXTEND_TO_EDGE,
			    XV_HEIGHT, WIN_EXTEND_TO_EDGE,
			    CANVAS_X_PAINT_WINDOW, TRUE,
			    NULL);

  xv_set(canvas_paint_window(obj),
	 WIN_MENU, stag_chart_menu1_create((caddr_t) ip, ip->win1), NULL);

  xv_set(canvas_paint_window(obj), WIN_CONSUME_EVENTS,
	 WIN_MOUSE_BUTTONS,
	 LOC_MOVE,
	 LOC_DRAG,
	 LOC_WINENTER,
	 LOC_WINEXIT,
	 WIN_ASCII_EVENTS,
	 WIN_LEFT_KEYS,
	 WIN_RIGHT_KEYS,
	 WIN_TOP_KEYS,
	 NULL, NULL);

  notify_interpose_event_func(canvas_paint_window(obj),
			      (Notify_func) stag_chart_win1_canvas1_event_callback,
			      NOTIFY_SAFE);
  return obj;

}

 
/*****************************************************************
 * DRAW_PLOT: plotting functions
 *
 */

#define AXIS_BIT 0x0001  /* Single bit to control printing of an axis */
#define UNITS_BIT 0x0002 /* Single bit to control print units
			  * instead of time label*/
/*****************************************************************
 * DRAW_PLOT: Supervise the plotting of all fields onto the screen
 */

void draw_plot()
{

  PMU_auto_register("draw_plot");
  
  // load up the plot data from the _spectra object
  
  load_plot_data();
  
  // initialize x,y locations

  int x_start = _params->left_margin;
  int x_end = x_start + _plot_width;
  
  int y_start = _params->top_margin;
  int y_end = y_start + _plot_height;

  int nPlots = compute_number_of_plots();
  int each_height = (int) ((double) _plot_height / nPlots);
  
  // clear plot
  
  XSetForeground(_dpy, _def_gc, _bg_cell);
  XSetBackground(_dpy, _def_gc, _bg_cell);
  XFillRectangle(_dpy,_back_xid,_def_gc, 0,0, _win_width, _win_height);

  // compute line height

  char string_buf[1024];
  sprintf(string_buf, "TESTING");
  XCharStruct overall;
  int direct,ascent,descent;
  XTextExtents(_fontst,string_buf,
	       strlen(string_buf),&direct,&ascent,&descent,&overall);
  _text_ht = ascent + descent;
  _line_spacing = _text_ht + 4;

  int y_line = 5;

  // no data? Plot messages to indicate this

  if (!_data_avail) {

    sprintf(string_buf, "NO DATA AVAILABLE FOR CLICK POINT AND CIDD TIME");
    
    XSetForeground(_dpy, _def_gc, _fg_cell);
    plot_string_centered(string_buf, x_start, y_line);
    y_line += _line_spacing;

    sprintf(string_buf, "Time: %s  "
	    "El(deg):%.2f  Az(deg):%.2f  Range(km):%.2f",
	    DateTime::strm(_end_time).c_str(),
	    _clickElev, _clickAz, _clickRange);

    plot_string_centered(string_buf, x_start, y_line);
    y_line += _line_spacing;

    XCopyArea(_dpy,_back_xid,_canvas_xid,_def_gc,
	      0,0,_win_width,_win_height,0,0);
    return;

  }
  
  // Title and time

  time_t secs = _spectra.getTimeSecs();
  int msecs = (int) (fmod(_spectra.getDoubleTime(), 1.0) * 1000 + 0.5);
  sprintf(string_buf, "%s %s %s.%.3d %s",
	  _spectra.getRadarName().c_str(),
	  _channelName.c_str(), DateTime::strm(secs).c_str(), msecs,
	  _spectra.getNotes().c_str());

  XSetForeground(_dpy, _def_gc, _fg_cell);
  plot_string_centered(string_buf, x_start, y_line);
  y_line += _line_spacing;

  // pulsing mode

  if (_spectra.isStaggeredPrtMode()) {
    sprintf(string_buf,
            "Staggered PRT mode, M/N: %d/%d, PRT short: %.3g, PRT long: %.3g",
            _spectra.getStaggeredM(), _spectra.getStaggeredN(),
            _spectra.getPrtShort(), _spectra.getPrtLong());
  } else {
    sprintf(string_buf, "PRT: %.3g", _spectra.getPrt());
  }
  XSetForeground(_dpy, _def_gc, _fg_cell);
  plot_string_centered(string_buf, x_start, y_line);
  y_line += _line_spacing;
  
  // Location text

  if (_params->plot_location_text) {
    sprintf(string_buf, "El(deg):%.2f  Az(deg):%.2f  Range(km):%.2f",
            _spectra.getElevDeg(), _spectra.getAzDeg(), _gateRange);
    XSetForeground(_dpy, _def_gc, _fg_cell);
    plot_string_centered(string_buf, x_start, y_line);
    y_line += _line_spacing;
  }

  // Moments text
  
  if (_params->plot_moments_text) {
    sprintf(string_buf,
            "Dbz:%.2f Vel(m/s):%.2f Width(m/s):%.2f Pwr(dBm):%.2f SNR(dB):%.2f",
            _gateMoments.dbz, _gateMoments.vel, _gateMoments.width,
            _gateMoments.dbm, _gateMoments.snr);
    XSetForeground(_dpy, _def_gc, _fg_cell);
    plot_string_centered(string_buf, x_start, y_line);
    y_line += _line_spacing;
  }
  
  // clutter info

  if (_params->plot_clutter_text) {
    sprintf(string_buf,
            "CPA:%.2f  PR:%.2f  CWR(dB):%.2f",
            _gateMoments.cpa, _gateMoments.pratio,
            _gateMoments.clut_2_wx_ratio);
    XSetForeground(_dpy, _def_gc, _fg_cell);
    plot_string_centered(string_buf, x_start, y_line);
    y_line += _line_spacing;
  }
  
  // N SAMPLES text

  sprintf(string_buf, "N SAMPLES: %d", _nSamples);
  XSetForeground(_dpy, _def_gc, _fg_cell);
  plot_string_centered(string_buf, x_start, y_end + _text_ht / 2);

  // plot fields

  int y_start_fld = _params->top_margin;
  int y_end_fld =  y_start_fld + each_height - 1;

  // power in range

  if (_params->plot_ascope_power) {
    compute_world_to_pixel(*_powerAscope, NULL, NULL, x_start, x_end,
                           y_start_fld, y_end_fld, true);
    plot_field(*_powerAscope, x_start, x_end,
               y_start_fld, y_end_fld, true, true);
    plot_range_line(y_start_fld, y_end_fld);
    _ascope_start_x = x_start;
    _ascope_end_x = x_end;
    _ascope_start_y = y_start_fld + _params->subplot_top_margin;
    _ascope_end_y = y_end_fld;
    y_start_fld += each_height;
    y_end_fld += each_height;
    add_range_labels(*_powerAscope, x_start, x_end, y_start, y_end);
  }
  
  if (!_spectra.isStaggeredPrtMode()) {
    cerr << "ERROR - not staggered PRT data" << endl;
    return;
  }

  // power spectrum, with clutter spectrum overlaid
  
  if (_params->plot_power_spectrum) {

    compute_world_to_pixel(*_stagSpecBoth, _stagFiltSpecBoth, NULL,
                           x_start, x_end, y_start_fld, y_end_fld,
                           !_params->autoscale_power_spectrum);
    plot_noise_value(x_start, x_end);
    plot_field(*_stagFiltSpecBoth,
               x_start, x_end, y_start_fld, y_end_fld, true, false);
    plot_field(*_stagSpecBoth,
               x_start, x_end, y_start_fld, y_end_fld, false, true);
    y_start_fld += each_height;
    y_end_fld += each_height;
    
  }

  // power in time domain

  if (_params->plot_power_time_series) {
    if (_params->autoscale_power_time_series &&
        !_params->plot_real_power_spectrum_over_time_series) {
      compute_world_to_pixel(*_powerTs,NULL,NULL,
                             x_start, x_end, y_start_fld, y_end_fld,false);
    } else {
      compute_world_to_pixel(*_powerTs,NULL,NULL,
                             x_start, x_end, y_start_fld, y_end_fld,true);
    }
    if (_params->plot_real_power_spectrum_over_time_series) {
      plot_noise_value(x_start, x_end);
    }
    plot_field(*_powerTs,
               x_start, x_end, y_start_fld, y_end_fld, true, true);
    if (_params->plot_real_power_spectrum_over_time_series) {
      plot_field(*_powerSpecReal,
                 x_start, x_end, y_start_fld, y_end_fld, false, false);
    }
    y_start_fld += each_height;
    y_end_fld += each_height;
  }

  // phase in time domain

  if (_params->plot_phase_time_series) {
    bool autoscale = _params->autoscale_phase_time_series;
    compute_world_to_pixel(*_phaseTs,NULL,NULL,
                           x_start, x_end,
                           y_start_fld, y_end_fld,!autoscale);
    plot_field(*_phaseTs,
               x_start, x_end, y_start_fld, y_end_fld, true, true);
    y_start_fld += each_height;
    y_end_fld += each_height;
    if (_params->plot_residual_phase) {
      compute_world_to_pixel(*_phaseResidual,NULL,NULL,
                             x_start, x_end,
                             y_start_fld, y_end_fld,false);
      plot_field(*_phaseResidual,
                 x_start, x_end, y_start_fld, y_end_fld, true, true);
      y_start_fld += each_height;
      y_end_fld += each_height;
    }
  }

  // pulse-to-pulse phase difference

  if (_params->plot_pulse_to_pulse_phase_diff) {
    compute_world_to_pixel(*_phaseDiff,NULL,NULL,
                           x_start, x_end, y_start_fld, y_end_fld,false);
    plot_field(*_phaseDiff,
               x_start, x_end, y_start_fld, y_end_fld, true, true);
    y_start_fld += each_height;
    y_end_fld += each_height;
  }

  if (_params->plot_cumulative_phase_diff) {
    compute_world_to_pixel(*_cumulativePhaseDiff,NULL,NULL,
                           x_start, x_end, y_start_fld, y_end_fld,true);
    plot_field(*_cumulativePhaseDiff,
               x_start, x_end, y_start_fld, y_end_fld, true, true);
    y_start_fld += each_height;
    y_end_fld += each_height;
  }

  // spectral phase

  if (_params->plot_fft_phase_diff) {
    compute_world_to_pixel(*_fftPhaseDiff,NULL,NULL,
                           x_start, x_end, y_start_fld, y_end_fld,false);
    plot_field(*_fftPhaseDiff,
               x_start, x_end, y_start_fld, y_end_fld, true, true);
    y_start_fld += each_height;
    y_end_fld += each_height;
  }

  // unfiltered pseudo time series
  
  if (_params->plot_power_time_series) {

    compute_world_to_pixel(*_stagTsPowerShort, _stagTsPowerLong, NULL,
                           x_start, x_end, y_start_fld, y_end_fld,
                           false, false);
    
    plot_field(*_stagTsPowerShort,
               x_start, x_end, y_start_fld, y_end_fld, true, false);
    plot_field(*_stagTsPowerLong,
               x_start, x_end, y_start_fld, y_end_fld, false, true);

    y_start_fld += each_height;
    y_end_fld += each_height;

  }

  if (_params->plot_phase_time_series) {

    bool autoscale = _params->autoscale_phase_time_series;

    compute_world_to_pixel(*_stagTsPhaseShort, _stagTsPhaseLong, NULL,
                           x_start, x_end,
                           y_start_fld, y_end_fld, !autoscale);
    plot_field(*_stagTsPhaseShort,
               x_start, x_end, y_start_fld, y_end_fld, true, false);
    plot_field(*_stagTsPhaseLong,
               x_start, x_end, y_start_fld, y_end_fld, false, true);
    y_start_fld += each_height;
    y_end_fld += each_height;

  }

  if (_params->plot_power_spectrum) {
    
    compute_world_to_pixel(*_stagSpecShort, _stagSpecLong, NULL,
                           x_start, x_end, y_start_fld, y_end_fld,
                           false, false);
    
    plot_field(*_stagSpecShort,
               x_start, x_end, y_start_fld, y_end_fld, true, false);
    plot_field(*_stagSpecLong,
               x_start, x_end, y_start_fld, y_end_fld, false, true);
    
    y_start_fld += each_height;
    y_end_fld += each_height;
    
    compute_world_to_pixel(*_stagFiltSpecShort, _stagFiltSpecLong, NULL,
                           x_start, x_end, y_start_fld, y_end_fld,
                           false, false);
    
    plot_field(*_stagFiltSpecShort,
               x_start, x_end, y_start_fld, y_end_fld, true, false);
    plot_field(*_stagFiltSpecLong,
               x_start, x_end, y_start_fld, y_end_fld, false, true);
    
    y_start_fld += each_height;
    y_end_fld += each_height;
    
  }

  // IQ time series
  
  if (_params->plot_iq_time_series) {

    compute_world_to_pixel(*_iTs,_iTsPolyFit,NULL,
                           x_start, x_end, y_start_fld, y_end_fld,false);
    if (_params->plot_polyfit_time_series) {
      plot_field(*_iTsPolyFit,
                 x_start, x_end, y_start_fld, y_end_fld, false, false);
    }
    plot_field(*_iTs,
               x_start, x_end, y_start_fld, y_end_fld, true, true);
    y_start_fld += each_height;
    y_end_fld += each_height;

    compute_world_to_pixel(*_qTs,_qTsPolyFit,NULL,
                           x_start, x_end, y_start_fld, y_end_fld,false);
    if (_params->plot_polyfit_time_series) {
      plot_field(*_qTsPolyFit,
                 x_start, x_end, y_start_fld, y_end_fld, false, false);
    }
    plot_field(*_qTs,
               x_start, x_end, y_start_fld, y_end_fld, true, true);
    y_start_fld += each_height;
    y_end_fld += each_height;

  }

  // residual IQ time series after detrending
  
  if (_params->plot_residual_time_series) {

    compute_world_to_pixel(*_iTsResidual, NULL, NULL,
                           x_start, x_end, y_start_fld, y_end_fld,false);

    plot_field(*_iTsResidual,
               x_start, x_end, y_start_fld, y_end_fld, true, true);

    y_start_fld += each_height;
    y_end_fld += each_height;
    
    compute_world_to_pixel(*_qTsResidual, NULL, NULL,
                           x_start, x_end, y_start_fld, y_end_fld,false);

    plot_field(*_qTsResidual,
               x_start, x_end, y_start_fld, y_end_fld, true, true);

    y_start_fld += each_height;
    y_end_fld += each_height;

  }

  // plot IQ phasor

  if (_params->plot_iq_phasor) {

    compute_world_to_pixel(*_iqPhasor, NULL, NULL,
                           x_start, x_end, y_start_fld, y_end_fld, false);

    plot_field(*_iqPhasor,
               x_start, x_end, y_start_fld, y_end_fld, true, true);

    y_start_fld += each_height;
    y_end_fld += each_height;
    
    compute_world_to_pixel(*_runningCpa, NULL, NULL,
                           x_start, x_end, y_start_fld, y_end_fld, false);
    
    plot_field(*_runningCpa,
               x_start, x_end, y_start_fld, y_end_fld, true, true);
    
    y_start_fld += each_height;
    y_end_fld += each_height;
    
  }

  // copy backing store to main canvas
  
  XCopyArea(_dpy,_back_xid,_canvas_xid,_def_gc,
	    0,0,_win_width,_win_height,0,0);
  
}

/*****************************************************************
 * PLOT_FIELD: Supervise the plotting of one source in the space given
 *
 *
 */

static void plot_field(FieldInfo &field, int x_start, int x_end,
		       int y_start, int y_end,
		       bool plot_grid, bool plot_label_left,
                       bool plot_label_down)

{

  // compute y_top as start plus margin
  
  int y_top = y_start + _params->subplot_top_margin;

  if (plot_grid) {
    draw_scales(field, x_start, x_end, y_top, y_end);
  }
  
  /* plot the data using the mapping between pixel space and time/data units */
  
  if(field.getYData().size() > 0) {
    XSetForeground(_dpy, _def_gc, field.getColorCell());
    plot_field_data(field, x_start, x_end, y_top, y_end);
  }
  
  // Place field name in the Upper left corner

  char string_buf[1024];
  XSetForeground(_dpy, _def_gc, field.getColorCell());
  sprintf(string_buf, "%s", field.getName().c_str());

  XCharStruct overall;
  int direct,ascent,descent;
  XTextExtents(_fontst,string_buf,
	       strlen(string_buf),&direct,&ascent,&descent,&overall);

  int yy = y_start + (_params->subplot_top_margin - _line_spacing) / 2 + 5;
  
  int xx = x_start + 6;
  if (!plot_label_left) {
    xx = x_end - overall.width - 6;
  }
  if (plot_label_down) {
    yy += (_line_spacing * 2) / 3;
  }

  plot_string(string_buf, xx, yy);

}

/*****************************************************************
 * draw string
 */

static void plot_string(const char *string_buf,
			int xx, int yy)
		    
{

  XCharStruct overall;
  int direct,ascent,descent;
  XTextExtents(_fontst,string_buf,
	       strlen(string_buf),&direct,&ascent,&descent,&overall);
    
  XDrawImageString(_dpy,_back_xid,_def_gc,
		   xx, yy + _text_ht - 2,
		   string_buf ,strlen(string_buf));

}
    
static void plot_string_centered(const char *string_buf,
				 int x_start, int yy)
		    
{

  XCharStruct overall;
  int direct,ascent,descent;
  XTextExtents(_fontst,string_buf,
	       strlen(string_buf),&direct,&ascent,&descent,&overall);
  
  XDrawImageString(_dpy,_back_xid,_def_gc,
		   x_start + (_plot_width - overall.width)/2, 
                   yy + _text_ht - 2,
		   string_buf ,strlen(string_buf));

}
    
static void plot_string_justified(const char *string_buf,
				  int x_end, int yy)
		    
{
  
  XCharStruct overall;
  int direct,ascent,descent;
  XTextExtents(_fontst,string_buf,
	       strlen(string_buf),&direct,&ascent,&descent,&overall);
  
  XDrawImageString(_dpy,_back_xid,_def_gc,
		   x_end - overall.width, 
                   yy + _text_ht - 2,
		   string_buf ,strlen(string_buf));

}
    
/*****************************************************************
 * Compute world-to-pixel conversions
 */

static void compute_world_to_pixel(FieldInfo &primaryField,
				   FieldInfo *secondField,
				   FieldInfo *thirdField,
				   int x_start, int x_end,
				   int y_start, int y_end,
				   bool use_field_min_and_max,
                                   bool scale_on_primary_only)
		    
{
  
  // compute data min and max

  double y_min, y_max;
  compute_min_and_max_y(primaryField, y_min, y_max);
  
  // set plot min and max, adjust for data if needed
  
  if (use_field_min_and_max) {

    _plot_min_y = primaryField.getMinY();
    _plot_max_y = primaryField.getMaxY();

    if (_plot_min_y > y_min) {
      _plot_min_y = y_min;
    }
    if (_plot_max_y < y_max) {
      _plot_max_y = y_max;
    }

  } else {

    _plot_min_y = y_min;
    _plot_max_y = y_max;

  }
  
  if (!scale_on_primary_only) {

    if (secondField != NULL) {
      
      compute_min_and_max_y(*secondField, y_min, y_max);
      if (_plot_min_y > y_min) {
        _plot_min_y = y_min;
      }
      if (_plot_max_y < y_max) {
        _plot_max_y = y_max;
      }
    }
    
    if (thirdField != NULL) {
      compute_min_and_max_y(*thirdField, y_min, y_max);
      if (_plot_min_y > y_min) {
        _plot_min_y = y_min;
      }
      if (_plot_max_y < y_max) {
        _plot_max_y = y_max;
      }
    }
    
  }

  double plot_range = _plot_max_y - _plot_min_y;
  
  int nx = (int) primaryField.getXData().size();

  double minx = 1.0e99;
  double maxx = -1.0e99;
  for (int ii = 0; ii < nx; ii++) {
    double xx = primaryField.getXData()[ii];
    if (xx < minx) minx = xx;
    if (xx > maxx) maxx = xx;
  }
  double xrange = maxx - minx;
  
  _pixels_per_x_val = (double) _plot_width / xrange;
  _pix_x_unit = _pixels_per_x_val;
  _pix_x_bias = x_start - minx * _pix_x_unit;

  int y_top = y_start + _params->subplot_top_margin;
  _pix_y_unit = -(y_end - y_top - 1) / plot_range;
  _pix_y_bias = y_end - (_pix_y_unit * _plot_min_y);

}

/*****************************************************************
 * COMPUTE_MIN_AND_MAX: compute min and max values to get the
 * plots all the same if required.
 *
 */

/* Data must be between these vaues to be valid */

static void compute_min_and_max_y(const FieldInfo &field,
                                  double &y_min,
                                  double &y_max)
		    
{

  y_min = 1.0e99;
  y_max = -1.0e99;
  
  const vector<double> &ydata = field.getYData();
  
  for(int i = 0; i < (int) ydata.size(); i++) {
    
    double y_val = ydata[i];
    
    if (y_val > y_max) {
      y_max = y_val;
    }
    if (y_val < y_min) {
      y_min = y_val;
    }
    
  } // i
    
}
    
/*****************************************************************
 * DRAW_SCALES: Set up a mapping between pixel space and data units
 * And Draw vertical scales at the center and right edge.
 *
 */

static void draw_scales(FieldInfo &field,
			int x_start, int x_end,
			int y_start, int y_end)
  
{

  // Draw vertical edge reference lines
  
  XSetForeground(_dpy, _def_gc, _fg_cell);
  XDrawLine(_dpy,_back_xid,_def_gc,x_start,y_start,x_start,y_end);
  XDrawLine(_dpy,_back_xid,_def_gc,x_end,y_start,x_end,y_end);
  
  // Draw vertical grid reference lines to divide the spectrum into 8

  for (int ii = 1; ii < 8; ii++) {
    int xx = x_start + (int) (((double) ii * _plot_width / 8.0) + 0.5);
    if (ii == 4) {
      XSetForeground(_dpy, _def_gc, _fg_cell);
    } else {
      XSetForeground(_dpy, _def_gc, _grid_cell);
    }
    XDrawLine(_dpy,_back_xid,_def_gc,xx,y_start,xx,y_end);
  }

  /* Compute tick locations
   * Pick a  nice even range that span the data */
  
  double plot_range = _plot_max_y - _plot_min_y;
  double tick_delta = compute_tick_interval(plot_range);

  /* compute staring tick value */
     
  double tick_base = nearest(_plot_min_y, tick_delta);
  if (tick_base < _plot_min_y) {
    tick_base += tick_delta;
  }
  
  /* Draw two sets of tick marks; right, and left */

  for(double value = tick_base; value <= _plot_max_y; value += tick_delta) {

    int y_tick = (int) (_pix_y_unit * value + _pix_y_bias);

    if (fabs(value) < 0.00001) {
      XSetForeground(_dpy, _def_gc, _fg_cell);
    } else {
      XSetForeground(_dpy, _def_gc, _grid_cell);
    }
    XDrawLine(_dpy,_back_xid,_def_gc,x_start,y_tick,x_end,y_tick);
    XDrawLine(_dpy,_back_xid,_def_gc,x_start,y_tick,x_start+3,y_tick);
   
    XSetForeground(_dpy, _def_gc, _fg_cell);
    XDrawLine(_dpy,_back_xid,_def_gc,x_end,y_tick,x_end-3,y_tick);
    XDrawLine(_dpy,_back_xid,_def_gc,x_start,y_tick,x_start+3,y_tick);
   
    char string_buf[128];
    if (fabs(value) > 0.001 && fabs(value) < 5) {
      sprintf(string_buf,"%.2g",value);
    } else {
      sprintf(string_buf,"%.0f",value);
    }
    int direct,ascent,descent;
    XCharStruct overall;
    XTextExtents(_fontst,string_buf,strlen(string_buf),
		 &direct,&ascent,&descent,&overall);
    
    int label_x = x_end + 5;
    if (_params->right_margin <= 0) {
      label_x = (x_end - 3 - overall.width);
    }
    
    XSetForeground(_dpy, _def_gc, _fg_cell);
    plot_string(string_buf, label_x, y_tick - _text_ht / 2);

  }

  /* horizontal line at base of plot */
  
  XSetForeground(_dpy, _def_gc, _fg_cell);
  XDrawLine(_dpy,_back_xid,_def_gc,x_start,y_end,x_end,y_end);
  
}

/*************************************************************************
 * NEAREST: Compute the value nearest the target which is divisible by
 *         the absolute value of delta
 */

static double
nearest(double target,double delta)
{
    double answer;
    double rem;

    delta = fabs(delta);
    rem = fmod(target,delta);

    if(target >= 0.0) {
        if(rem > (delta / 2.0)) {
    	   answer = target + (delta - rem);
        } else {
          answer = target -  rem;
        }
    } else {
        if(fabs(rem) > (delta / 2.0)) {
    	   answer = target - (delta + rem);
        } else {
          answer = target -  rem;
        }
    }
      
    return answer;
}
 
/*************************************************************************
 * COMPUTE_TICK_INTERVAL: Return the tick interval given a range
 *        Range Assumed to be > 1.0 && < 10000.0
 */

static double
compute_tick_interval(double range)
{
    double    arange = fabs(range);

    if(arange <= 0.005) return (0.001);
    if(arange <= 0.01) return (0.002);
    if(arange <= 0.02) return (0.0035);
    if(arange <= 0.05) return (0.01);
    if(arange <= 0.1) return (0.02);
    if(arange <= 0.2) return (0.05);
    if(arange <= 0.5) return (0.1);
    if(arange <= 1.0) return (0.2);
    if(arange <= 2.0) return (0.5);
    if(arange <= 5.0) return (1.0);
    if(arange <= 10.0) return (2.0);
    if(arange <= 20.0) return (5.0);
    if(arange <= 50.0) return (10.0);
    if(arange <= 100.0) return (20.0);
    if(arange <= 200.0) return (40.0);
    if(arange <= 300.0) return (90.0);
    if(arange == 360.0) return (100.0);
    if(arange <= 400.0) return (100.0);
    if(arange <= 500.0) return (100.0);
    if(arange <= 1000.0) return (200.0);
    if(arange <= 2000.0) return (400.0);
    if(arange <= 5000.0) return (1000.0);
    if(arange <= 10000) return (2000.0);
    if(arange <= 20000) return (4000.0);
    if(arange <= 50000) return (10000.0);
    if(arange <= 100000) return (20000.0);
    return(50000);
}
 
/*************************************************************************
 * PLOT_FIELD_DATA
 * Plot the data on the pixmap given the data to pixel transform values 
 */

static void plot_field_data(FieldInfo &field,
			    int x_start, int x_end,
			    int y_start, int y_end)
 
{

  int i;
  int n_points;
  const vector<double> &xx = field.getXData();
  const vector<double> &yy = field.getYData();
  TaArray<XPoint> bpt_;
  XPoint *bpt = bpt_.alloc(yy.size());
  
  n_points = 0;

  for(i=0; i < (int) yy.size(); i++) {
    
    double x_val = xx[i];
    double y_val = yy[i];
    
    bpt[n_points].x = (short int) (x_val * _pix_x_unit + _pix_x_bias);
    bpt[n_points].y = (short int) (y_val * _pix_y_unit + _pix_y_bias);

    n_points++;
    
  } // i
  
  if(n_points > 0) {

    // set clipping on

    XRectangle clipRect;
    clipRect.x = x_start;
    clipRect.y = y_start;
    clipRect.width = x_end - x_start + 1;
    clipRect.height = y_end - y_start + 1;
    XSetClipRectangles(_dpy, _def_gc, 0, 0, &clipRect, 1, Unsorted);

    // draw data

    XSetLineAttributes(_dpy, _def_gc, _params->plot_line_width,
		       LineSolid, CapButt, JoinBevel);
    XDrawLines(_dpy,_back_xid,_def_gc,bpt,n_points,CoordModeOrigin);
    XSetLineAttributes(_dpy, _def_gc, 1,
		       LineSolid, CapButt, JoinBevel);

    // unset clipping

    clipRect.x = 0;
    clipRect.y = 0;
    clipRect.width = _win_width;
    clipRect.height = _win_height;
    XSetClipRectangles(_dpy, _def_gc, 0, 0, &clipRect, 1, Unsorted);

  }
  
}

/*************************************************************************
 * Plot noise value as a line
 */

static void plot_noise_value(int x_start, int x_end)
 
{

  double noise = _spectra.getNoiseDbm() - _spectra.getReceiverGainDb();
  int y_noise = (int) (noise * _pix_y_unit + _pix_y_bias);

  XSetForeground(_dpy, _def_gc, _noise_cell);
  XDrawLine(_dpy,_back_xid,_def_gc,x_start,y_noise,x_end,y_noise);
  
}

/*************************************************************************
 * Plot current range as a line
 */

static void plot_range_line(int y_start, int y_end)
{


  double range = _spectra.getStartRange() + _gateNum * _spectra.getGateSpacing();
  int x_gate = (int) (range * _pix_x_unit + _pix_x_bias);
  int y_top = y_start + _params->subplot_top_margin;
  
  XSetForeground(_dpy, _def_gc, _range_cell);
  XDrawLine(_dpy,_back_xid,_def_gc,x_gate,y_top,x_gate,y_end);
  
}

/*****************************************************************
 * add range labels
 *
 */

static void add_range_labels(FieldInfo &field,
			     int x_start, int x_end,
			     int y_start, int y_end)
  
{
  
  int y_top = y_start + _params->subplot_top_margin;
  
  XSetForeground(_dpy, _def_gc, _fg_cell);

  char string_buf[1024];
  sprintf(string_buf, "%.0f km", _minRange);

  int yy = y_top + 5;
  plot_string(string_buf, x_start + 6, yy);

  sprintf(string_buf, "%.0f km", _maxRange);
  plot_string_justified(string_buf, x_end - 6, yy);

}
