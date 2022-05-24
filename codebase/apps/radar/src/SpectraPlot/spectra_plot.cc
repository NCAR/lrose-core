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

#include <toolsa/DateTime.hh>
#include <toolsa/pjg.h>
#include <rapmath/math_macros.h>
#include <radar/IwrfCalib.hh>
#include <radar/RadarFft.hh>
#include <radar/RadarComplex.hh>
#include <radar/RadarMoments.hh>
#include <radar/MomentsFields.hh>
#include <radar/MomentsFields.hh>
#include <radar/RegressionFilter.hh>
#include <radar/ClutFilter.hh>
#include <toolsa/TaArray.hh>

#include <algorithm>

#include "Params.hh"
#include "SpectraPlot.hh"
#include "RadarSpectra.hh"
#include "FieldInfo.hh"
#include "TsReader.hh"
#include "BeamMgr.hh"

using namespace std;

typedef struct {
  Xv_opaque win1;
  Xv_opaque canvas1;
} strip_chart_win1_objects;

///////////////////////////////
// FILE SCOPE VARIABLES

static Attr_attribute INSTANCE;

static char *_app_name;
  
static Params *_params; // TDRP parameters

static RadarSpectra::polarization_channel_t _channel; // current variable
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

static time_t _start_time_ppi, _end_time_ppi;
static time_t _start_time_rhi, _end_time_rhi;
static double _pixels_per_x_val;

static int _fg_cell;      /* foreground color cell */
static int _bg_cell;      /* background color cell */
static int _grid_cell;    /* color for "grid" on the time line */
static int _noise_cell;   /* color for noise level line */
static int _range_cell;   /* color for range line */

static bool _data_avail;

static double _clickElevDeg;
static double _clickAzDeg;
static double _clickRangeKm;
static double _clickHtKm;
static bool _clickIsInRhi;

static double _dataElevDeg;
static double _dataAzDeg;

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
static FieldInfo *_nextPowerSpec;
static FieldInfo *_adapFiltSpec;
static FieldInfo *_regrFiltSpec;

static FieldInfo *_stagSpecBoth;
static FieldInfo *_stagFiltSpecBoth;

static FieldInfo *_stagSpecShort;
static FieldInfo *_stagSpecShortPhase;
static FieldInfo *_stagFiltSpecShort;
static FieldInfo *_stagFiltSpecShortInterp;
static FieldInfo *_stagFiltSpecShortRatio;

static FieldInfo *_stagSpecLong;
static FieldInfo *_stagSpecLongPhase;
static FieldInfo *_stagFiltSpecLong;
static FieldInfo *_stagFiltSpecLongInterp;
static FieldInfo *_stagFiltSpecLongRatio;

static FieldInfo *_stagSpecPhaseDiff;
static FieldInfo *_stagFiltSpecPhaseDiff;
static FieldInfo *_stagFiltSpecPhaseDiffInterp;

static FieldInfo *_powerTs;
static FieldInfo *_phaseTs;
static FieldInfo *_phaseResidual;
static FieldInfo *_phaseDiff;
static FieldInfo *_cumulativePhaseDiff;

static FieldInfo *_fftPhaseDiff;

static FieldInfo *_powerSpecReal;
static FieldInfo *_powerAscope;

static FieldInfo *_spfPower;
static FieldInfo *_spfPhaseDiff;

static FieldInfo *_iTs;
static FieldInfo *_iTsPolyFit;
static FieldInfo *_iTsResidual;

static FieldInfo *_qTs;
static FieldInfo *_qTsPolyFit;
static FieldInfo *_qTsResidual;

static FieldInfo *_iqPhasor;
static FieldInfo *_runningCpa;

static double _tssDb;
static double _cpaAlt = -9999;

// X Windows Drawing variables

static Display *_dpy;
static XFontStruct *_fontst;
static Font _font;
static Drawable _canvas_xid;
static Drawable _back_xid;
static GC _def_gc;

static strip_chart_win1_objects *_Strip_chart_win1;

// XVIEW actions

static double _lastAscopeSelectTime = 0.0;

// pointer to cidd's shmem control interface  

static coord_export_t *_cidd_mem;

// data retrieval

static BeamMgr *_beamMgr;
static TsReader *_tsReaderPpi = NULL;
static TsReader *_tsReaderRhi = NULL;
static bool _isRhi = false;
static bool _readPending;
static bool _plotPending;

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
static int find_ts_data(TsReader *reader,
                        time_t startTime,
                        time_t endTime,
                        double az, double el);
static int retrieve_ts_data();
static void set_click_location_from_cidd();
static void set_click_location_from_user(double az, double range);
static int get_beam();

static int compute_number_of_plots();

static void clear_plot_data();
static void load_plot_data();
static void load_plot_data_single_prt();
static void load_plot_data_staggered_prt();

static void compute_tss(const RadarComplex_t *iqWindowed);

static void print_shmem();

static double compute_tick_interval(double range);

static void plot_messages(const vector<string> messages);
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
  
static strip_chart_win1_objects
  *strip_chart_win1_objects_initialize(strip_chart_win1_objects *, Xv_opaque);

static Xv_opaque strip_chart_win1_win1_create(strip_chart_win1_objects *, Xv_opaque);
static Xv_opaque strip_chart_win1_canvas1_create(strip_chart_win1_objects *, Xv_opaque);

/********************************************************************
 * MAIN: Process arguments, initialize and begin application
 */
 

int strip_chart_main(int argc, char **argv, Params *params,
		     coord_export_t *coord_shmem,
                     BeamMgr *beamMgr,
                     TsReader *ts_reader_ppi,
                     TsReader *ts_reader_rhi)
  
{

  xv_init(XV_INIT_ARGC_PTR_ARGV,
	  &argc,argv,
	  XV_X_ERROR_PROC, x_error_proc, NULL);
  
  // global struct

  init_static_vars();
  _params = params;
  _cidd_mem = coord_shmem;
  _beamMgr = beamMgr;
  _tsReaderPpi = ts_reader_ppi;
  _tsReaderRhi = ts_reader_rhi;
  
  // create Xview objects

  init_xview();

  // initialize globals, get/set defaults, establish data fields etc.
  
  init_data_space();

  // make changes to xview objects not available from DevGuide

  modify_xview_objects();
  
  // start timer

  start_timer();
  
  // Turn control over to XView.

  xv_main_loop(_Strip_chart_win1->win1);

  return 0;

}

/***************************************************************************
 * INIT_XVIEW : Initialize the base frame and other global objects
 */ 
 
static void init_xview()
{
 
  _Strip_chart_win1 =
    strip_chart_win1_objects_initialize(NULL, (Xv_opaque) NULL);
  
  notify_interpose_destroy_func(_Strip_chart_win1->win1,
				(Notify_func) base_win_destroy);

  _dpy = (Display *) xv_get(_Strip_chart_win1->win1,XV_DISPLAY);

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
  
  _channel = RadarSpectra::CHANNEL_HC;

  _powerSpec = NULL;
  _nextPowerSpec = NULL;
  _adapFiltSpec = NULL;
  _regrFiltSpec = NULL;

  _stagSpecBoth = NULL;
  _stagFiltSpecBoth = NULL;

  _stagSpecShort = NULL;
  _stagSpecShortPhase = NULL;
  _stagFiltSpecShort = NULL;
  _stagFiltSpecShortInterp = NULL;
  _stagFiltSpecShortRatio = NULL;

  _stagSpecLong = NULL;
  _stagSpecLongPhase = NULL;
  _stagFiltSpecLong = NULL;
  _stagFiltSpecLongInterp = NULL;
  _stagFiltSpecLongRatio = NULL;

  _stagSpecPhaseDiff = NULL;
  _stagFiltSpecPhaseDiff = NULL;
  _stagFiltSpecPhaseDiffInterp = NULL;

  _powerTs = NULL;
  _phaseTs = NULL;
  _phaseResidual = NULL;
  _phaseDiff = NULL;
  _cumulativePhaseDiff = NULL;
  _powerSpecReal = NULL;
  _powerAscope = NULL;

  _spfPower = NULL;
  _spfPhaseDiff = NULL;

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

  _start_time_ppi = 0;
  _end_time_ppi = 0;

  _start_time_rhi = 0;
  _end_time_rhi = 0;

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
  _readPending = false;
  _plotPending = false;

  _clickElevDeg = 0;
  _clickAzDeg = 0;
  _clickRangeKm = 0;
  _clickHtKm = 0;
  _clickIsInRhi = false;

  _dpy = NULL;
  _fontst = NULL;
  _font = 0;
  _canvas_xid = 0;
  _back_xid = 0;
  _def_gc = 0;
  
  _Strip_chart_win1 = NULL;
  
  _cidd_mem = NULL;

}

/*****************************************************************
 * INIT_DATA_SPACE : Init all globals and set up defaults
 */

static void init_data_space()
{

  INSTANCE = xv_unique_key(); /* get keys for retrieving data */

  _channel = RadarSpectra::CHANNEL_HC;
  _channelName = "CHANNEL_HC";

  _pixels_per_x_val = (double) _params->window_width / 64;

  // set up the fields/plots

  // power spectrum

  _powerSpec = new FieldInfo(_dpy, "Power spectrum (dBm)",
                             _params->power_spec_color,
                             _params->power_spec_min_db,
                             _params->power_spec_max_db);

  _nextPowerSpec = new FieldInfo(_dpy, "Power spectrum next gate (dBm)",
                                 _params->regression_filtered_spec_color,
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
  
  _stagSpecShort = new FieldInfo(_dpy, "Stag short PRT spec (dBm)",
                                 _params->short_prt_color,
                                 _params->power_spec_min_db,
                                 _params->power_spec_max_db);
  
  _stagSpecShortPhase = new FieldInfo(_dpy, "Stag short PRT phase (deg)",
                                      _params->short_prt_color, -180, 180);
  
  _stagFiltSpecShort = new FieldInfo(_dpy, "Stag short PRT regr filt (dBm)",
                                     _params->short_prt_color,
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
  
  _stagSpecLong = new FieldInfo(_dpy, "Stag long PRT spec (dBm)",
                                _params->long_prt_color,
                                _params->power_spec_min_db,
                                _params->power_spec_max_db);
  
  _stagSpecLongPhase = new FieldInfo(_dpy, "Stag long PRT phase (deg)",
                                     _params->long_prt_color, -180, 180);
  
  _stagFiltSpecLong = new FieldInfo(_dpy, "Stag long PRT regr filt (dBm)",
                                    _params->long_prt_color,
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
  
  _stagSpecPhaseDiff = new FieldInfo(_dpy, "Stag spec phase diff (deg)",
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

  // spectral phase fluctuations

  _spfPower = new FieldInfo(_dpy, "SPF power (dBm)",
                            _params->spf_power_color,
                            _params->ts_power_min_db,
                            _params->ts_power_max_db);
  
  _spfPhaseDiff = new FieldInfo(_dpy, "SPF phase diff (deg)",
                                _params->spf_phase_color,
                                -180, 180);
     
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

void strip_chart_free()
{

  freeField(_powerSpec);
  freeField(_nextPowerSpec);
  freeField(_adapFiltSpec);
  freeField(_regrFiltSpec);

  freeField(_stagSpecBoth);
  freeField(_stagFiltSpecBoth);

  freeField(_stagSpecShort);
  freeField(_stagSpecShortPhase);
  freeField(_stagFiltSpecShort);
  freeField(_stagFiltSpecShortInterp);
  freeField(_stagFiltSpecShortRatio);

  freeField(_stagSpecLong);
  freeField(_stagSpecLongPhase);
  freeField(_stagFiltSpecLong);
  freeField(_stagFiltSpecLongInterp);
  freeField(_stagFiltSpecLongRatio);

  freeField(_stagSpecPhaseDiff);
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

  freeField(_spfPower);
  freeField(_spfPhaseDiff);

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

  xv_set(_Strip_chart_win1->win1,
         WIN_X,_params->window_x,
         WIN_Y,_params->window_y,
         WIN_HEIGHT,_params->window_height,
         WIN_WIDTH,_params->window_width,
	 NULL);
  
  _win_height =  xv_get(_Strip_chart_win1->win1,WIN_HEIGHT);
  _win_width =  xv_get(_Strip_chart_win1->win1,WIN_WIDTH);
  _plot_height = _win_height - _params->bottom_margin - _params->top_margin;
  _plot_width = _win_width -  _params->right_margin - _params->left_margin;
  
  _canvas_xid =
    xv_get(canvas_paint_window(_Strip_chart_win1->canvas1),XV_XID);

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

  xv_set(_Strip_chart_win1->canvas1, XV_HELP_DATA,
	 "strip_chart:canvas",NULL);

  set_frame_label();

  _data_avail = false;

}

/*****************************************************************
 * SET_FRAME_LABEL : 
 */

void set_frame_label()
{

  xv_set(_Strip_chart_win1->win1, FRAME_LABEL,
	 _params->window_label, NULL);
  
}

/*************************************************************************
 * Menu handler for `menu1'.
 */

Menu
  select_field_proc( Menu	menu, Menu_generate op)

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

      get_beam();

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
  strip_chart_win1_canvas1_event_callback(Xv_window win,
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
        if (_clickRangeKm > _minRange) {
          _clickRangeKm -= _spectra.getGateSpacing();
          set_click_location_from_user(_clickAzDeg, _clickRangeKm);
          if (_params->debug >= Params::DEBUG_VERBOSE) {
            cerr << "up arrow, new range: " << _clickRangeKm << endl;
          }
          draw_plot();
        }
      }
      break;
    
    case ACTION_GO_COLUMN_BACKWARD: // down arrow
      if(event_is_up(event)) {
        if (_clickRangeKm < _maxRange) {
          _clickRangeKm += _spectra.getGateSpacing();
          set_click_location_from_user(_clickAzDeg, _clickRangeKm);
          if (_params->debug >= Params::DEBUG_VERBOSE) {
            cerr << "down arrow, new range: " << _clickRangeKm << endl;
          }
          draw_plot();
        }
      }
      break;
    
    case ACTION_GO_CHAR_BACKWARD: // left arrow
      if(event_is_up(event)) {
        _clickAzDeg -= _deltaAzMedian;
        if (_clickAzDeg < 0) {
          _clickAzDeg += 360.0;
        }
        set_click_location_from_user(_clickAzDeg, _clickRangeKm);
        if (_params->debug >= Params::DEBUG_VERBOSE) {
          cerr << "left arrow, new az: " << _clickAzDeg << endl;
        }
        get_beam();
        draw_plot();
      }
      break;
    
    case ACTION_GO_CHAR_FORWARD: // right arrow
      if(event_is_up(event)) {
        _clickAzDeg += _deltaAzMedian;
        if (_clickAzDeg >= 360.0) {
          _clickAzDeg -= 360.0;
        }
        set_click_location_from_user(_clickAzDeg, _clickRangeKm);
        if (_params->debug >= Params::DEBUG_VERBOSE) {
          cerr << "right arrow, new az: " << _clickAzDeg << endl;
        }
        get_beam();
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
            _clickRangeKm =
              _spectra.getStartRange() +
              x_fraction * (_spectra.getGateSpacing() * _nGates);
            set_click_location_from_user(_clickAzDeg, _clickRangeKm);
            if (_params->debug >= Params::DEBUG_VERBOSE) {
              cerr << "Double click in ASCOPE" << endl;
              cerr << "  new _clickRangeKm:" << _clickRangeKm << endl;
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
  
  _win_height =  xv_get(_Strip_chart_win1->win1,WIN_HEIGHT);
  _win_width =  xv_get(_Strip_chart_win1->win1,WIN_WIDTH);
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

static void timer_func(Notify_client client, int which)

{

  PMU_auto_register("In timer_func");

  if (_plotPending) {
    // get the beam
    if (get_beam()) {
    }
    draw_plot();
    _plotPending = false;
    return;
  }

  if (_readPending) {
    if (retrieve_ts_data() == 0) {
      _plotPending = true;
    }
    _readPending = false;
    return;
  }

  if (check_click()) {
    _plotPending = true;
  }
  
}
 
/************************************************************************
 * CHECK_CLICK: check to see whether we have a new click point
 *
 */

static bool check_click()
  
{
  
  static int prev_seq_num = -1;
  static int prev_client_num = -1;
  
  PMU_auto_register("Checking for click");

  // check for a new click from CIDD
  
  if(_cidd_mem->pointer_seq_num != prev_seq_num) {
    
    // new click found
    
    if (_cidd_mem->click_type != CIDD_USER_CLICK) {
      if (_params->debug >= Params::DEBUG_VERBOSE) {
        cerr << "====>> not CIDD_USER_CLICK - ignoring <<====" << endl;
      }
      return false;
    }

    if (_params->debug) {
      cerr << "====>> check_retrieve: new click point <<====" << endl;
    }

    if (_params->debug >= Params::DEBUG_VERBOSE) {
      print_shmem();
    }
    
    if (_cidd_mem->click_is_for_vsection) {

      // RHI

      _isRhi = true;

      bool getNewData = false;

      if (_end_time_rhi != _cidd_mem->time_data_end) {
        // cidd time has changed, get new data
        _start_time_rhi = _cidd_mem->time_data_start;
        _end_time_rhi = _cidd_mem->time_data_end;
        if (_params->debug) {
          cerr << "RHI time changed ..." << endl;
          cerr << "  Start time: " << utimstr(_start_time_rhi) << endl;
          cerr << "  End   time: " << utimstr(_end_time_rhi) << endl;
        }
        getNewData = true;
      } // if (_end_time_rhi != _cidd_mem->time_data_end) {
        
      if (fabs(_dataAzDeg - _clickAzDeg) > 0.05) {
        // cidd azimuth has changed
        if (_params->debug) {
          cerr << "RHI az changed ..." << endl;
          cerr << "  Prev az: " << _dataAzDeg << endl;
          cerr << "  New  az: " << _clickAzDeg << endl;
        }
        _dataAzDeg = _clickAzDeg;
        getNewData = true;
      } // if (_end_time_rhi != _cidd_mem->time_data_end) {

      if (getNewData) {
        if (find_ts_data(_tsReaderRhi,
                         _start_time_rhi, _end_time_rhi,
                         _dataAzDeg, _dataElevDeg)) {
          return false;
        }
      }
      
    } else {

      _isRhi = false;

      // PPI

      bool getNewData = false;

      if (_end_time_ppi != _cidd_mem->time_data_end) {
        // cidd time has changed, get new data
        _start_time_ppi = _cidd_mem->time_data_start;
        _end_time_ppi = _cidd_mem->time_data_end;
        if (_params->debug) {
          cerr << "PPI time changed ..." << endl;
          cerr << "  Start time: " << utimstr(_start_time_ppi) << endl;
          cerr << "  End   time: " << utimstr(_end_time_ppi) << endl;
        }
        getNewData = true;
      } // if (_end_time_ppi != _cidd_mem->time_data_end) {
      
      if (fabs(_dataElevDeg - _clickElevDeg) > 0.05) {
        // cidd elev changed, get new data
        if (_params->debug) {
          cerr << "PPI elevation changed ..." << endl;
          cerr << "  Prev elev: " << _dataElevDeg << endl;
          cerr << "  New  elev: " << _clickElevDeg << endl;
        }
        _dataElevDeg = _clickElevDeg;
        getNewData = true;
      } // if (fabs(_dataElevDeg - _clickElevDeg) > 0.05)
      
      if (getNewData) {
        if (find_ts_data(_tsReaderPpi, 
                         _start_time_ppi, _end_time_ppi,
                         _dataAzDeg, _dataElevDeg)) {
          return false;
        }
      }

    } // if (_cidd_mem->click_is_for_vsection) {
    
    set_click_location_from_cidd();

    prev_seq_num = _cidd_mem->pointer_seq_num;
    return true;

  } // if(_cidd_mem->pointer_seq_num != prev_seq_num) {
  
  // check for a new click from another client - such as another instance
  // of this application

  if(prev_client_num < 0 || _cidd_mem->client_seq_num != prev_client_num) {
    // set click az and range
    double xx = _cidd_mem->client_x;
    double yy = _cidd_mem->client_y;
    _clickRangeKm = sqrt(xx * xx + yy * yy);
    _clickAzDeg = atan2(xx, yy) * RAD_TO_DEG;
    prev_client_num = _cidd_mem->client_seq_num;
    return true;
  }
    
  return false;

}

/************************************************************************
 * print the CIDD chmem segment
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
  
  fprintf(stderr, "  pointer_seq_num: %d\n", (int) _cidd_mem->pointer_seq_num);
  fprintf(stderr, "  pointer_x: %g\n", _cidd_mem->pointer_x);
  fprintf(stderr, "  pointer_y: %g\n", _cidd_mem->pointer_y);
  fprintf(stderr, "  pointer_lat: %g\n", _cidd_mem->pointer_lat);
  fprintf(stderr, "  pointer_lon: %g\n", _cidd_mem->pointer_lon);
  
  fprintf(stderr, "  datum_lat: %g\n", _cidd_mem->datum_latitude);
  fprintf(stderr, "  datum_lon: %g\n", _cidd_mem->datum_longitude);
  fprintf(stderr, "  data_altitude: %g\n", _cidd_mem->data_altitude);

  fprintf(stderr, "  client_click_time: %s\n",
          DateTime::strm(_cidd_mem->client_click_time).c_str());
  fprintf(stderr, "  client_seq_num: %d\n", (int) _cidd_mem->client_seq_num);
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
  
  /* set up interval timer interval - 200 ms interval */
  timer.it_value.tv_sec = 0;
  timer.it_value.tv_usec = 200000;
  timer.it_interval.tv_sec = 0;
  timer.it_interval.tv_usec = 200000;
  
  /* Set the interval timer function and start timer */
  notify_set_itimer_func(_Strip_chart_win1->win1,
			 (Notify_func)timer_func,
			 ITIMER_REAL, &timer, NULL); /*  */
  timer_func(0,0);
}

/*************************************************************************
 * set click point from CIDD
 */

static void set_click_location_from_cidd()
  
{

  // compute elevation, azimuth and range
  
  _clickAzDeg = _cidd_mem->pointer_az_deg;
  _clickElevDeg = _cidd_mem->pointer_el_deg;
  _clickRangeKm = _cidd_mem->pointer_range_km;
  _clickHtKm = _cidd_mem->pointer_ht_km;
  _clickIsInRhi = _cidd_mem->click_is_for_vsection;

  if (_params->debug >= Params::DEBUG_NORM) {
    cerr << "Setting click point from CIDD: " << endl;
    cerr << "  _clickElevDeg: " << _clickElevDeg << endl;
    cerr << "  _clickAzDeg: " << _clickAzDeg << endl;
    cerr << "  _clickRangeKm: " << _clickRangeKm << endl;
    cerr << "  _clickHtKm: " << _clickHtKm << endl;
    cerr << "  _clickIsInRhi?: " << _clickIsInRhi << endl;
  }

}
  
/*************************************************************************
 * set user click point from az and range
 */

static void set_click_location_from_user(double az, double range)
  
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
 * GET_BEAM: load data for click point
 *
 * Returns 0 on success, -1 on failure
 */

static int get_beam()

{

  Beam *beam = NULL;
  _data_avail = false;
  
  if (_isRhi) {
    beam = _tsReaderRhi->getBeam(_clickAzDeg, _clickElevDeg);
    if (beam == NULL) {
      return -1;
    }
  } else {
    beam = _tsReaderPpi->getBeam(_clickAzDeg, _clickElevDeg);
    if (beam == NULL) {
      return -1;
    }
  }

  _beamMgr->loadBeamData(*beam);
  _beamMgr->fillSpectraObj(_spectra, _channel);
  _data_avail = true;

  if (_params->debug >= Params::DEBUG_VERBOSE) {
    _spectra.printHeader(cerr);
  }
    

  return 0;

}

/*****************************************************************
 * compute the number of active plots
 */

static int compute_number_of_plots()
{

  int nPlots = 0;

  if (_spectra.isStaggeredPrtMode()) {
    if (_params->plot_power_spectrum) {
      nPlots++;
    }
  } else {
    if (_params->plot_power_spectrum) {
      nPlots++;
    }
  }

  if (_params->plot_power_time_series) {
    nPlots++;
  }
  
  if (_params->plot_phase_time_series) {
    nPlots++;
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

  if (_params->plot_staggered_half_spectra) {
    nPlots += 6;
#ifdef JUNK
    nPlots += 7;
#endif
  }

  if (_params->plot_iq_phasor) {
    nPlots += 2;
  }

  if (_params->plot_spf) {
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
  _nextPowerSpec->clearData();
  _adapFiltSpec->clearData();
  _regrFiltSpec->clearData();

  _stagSpecBoth->clearData();
  _stagFiltSpecBoth->clearData();

  _stagSpecShort->clearData();
  _stagSpecShortPhase->clearData();
  _stagFiltSpecShort->clearData();
  _stagFiltSpecShortInterp->clearData();
  _stagFiltSpecShortRatio->clearData();

  _stagSpecLong->clearData();
  _stagSpecLongPhase->clearData();
  _stagFiltSpecLong->clearData();
  _stagFiltSpecLongInterp->clearData();
  _stagFiltSpecLongRatio->clearData();

  _stagSpecPhaseDiff->clearData();
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

  _spfPower->clearData();
  _spfPhaseDiff->clearData();

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
  
  _gateNum = (int) ((_clickRangeKm - _spectra.getStartRange()) /
		    _spectra.getGateSpacing() + 0.5);

  if (_gateNum < 0 || _gateNum > _nGates - 1) {
    return;
  }

  _gateRange = _spectra.getStartRange() +
    (_gateNum + 0.5) * _spectra.getGateSpacing();

  if (_params->debug >= Params::DEBUG_EXTRA) {
    cerr << "=====>> _clickRangeKm: " << _clickRangeKm << endl;
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

  // load IQ data for this gate

  vector<RadarSpectra::radar_iq_t> gateIq = _spectra.getGateIq(_gateNum);
  TaArray<RadarComplex_t> iq_;
  RadarComplex_t *iq = iq_.alloc(_nSamples);
  for (int ii = 0; ii < _nSamples; ii++) {
    int jj = _nSamples - ii - 1;
    iq[jj].re = gateIq[ii].ival / sqrtRcvGain;
    iq[jj].im = gateIq[ii].qval / sqrtRcvGain;
  }

  // load IQ data for next gate

  int nextGateNum = _gateNum + 1;
  if (nextGateNum > _nGates -1) nextGateNum = _gateNum;
  vector<RadarSpectra::radar_iq_t> nextGateIq = _spectra.getGateIq(nextGateNum);
  TaArray<RadarComplex_t> nextIq_;
  RadarComplex_t *nextIq = nextIq_.alloc(_nSamples);
  for (int ii = 0; ii < _nSamples; ii++) {
    int jj = _nSamples - ii - 1;
    nextIq[jj].re = nextGateIq[ii].ival / sqrtRcvGain;
    nextIq[jj].im = nextGateIq[ii].qval / sqrtRcvGain;
  }

  // compute moments for this gate
  
  IwrfCalib calib;
  calib.setWavelengthCm(_spectra.getWavelengthCm());
  calib.setNoiseDbmHc(_spectra.getNoiseDbm() - rcvGainDb);
  // calib.setReceiverGainDbHc(_spectra.getReceiverGainDb());
  calib.setReceiverGainDbHc(0.0);
  calib.setBaseDbz1kmHc(_spectra.getBaseDbz1km());

  RadarMoments moments(_gateNum + 1,
                       _params->debug >= Params::DEBUG_EXTRA, false);

  moments.setNSamples(_nSamples);
  moments.init(_spectra.getPrt(), _spectra.getWavelengthCm() / 100.0,
	       _spectra.getStartRange(), _spectra.getGateSpacing());
  moments.setCalib(calib);
  
  // compute moments

  moments.singlePolH(iq, _gateNum, false, _gateMoments);
  _gateMoments.mvar = moments.computeMvar(iq, _nSamples, _spectra.getPrt());
  _gateMoments.tpt = moments.computeTpt(iq, _nSamples);
  _gateMoments.cpd = moments.computeCpd(iq, _nSamples);

  // phasor data

  TaArray<RadarComplex_t> iqPhasor_;
  RadarComplex_t *iqPhasor = iqPhasor_.alloc(_nSamples);
  TaArray<double> mag_;
  double *mag = mag_.alloc(_nSamples);

  double sumRe = 0.0, sumIm = 0.0;
  double minRe = 1.0e99;
  double maxRe = -1.0e99;
  double minIm = 1.0e99;
  double maxIm = -1.0e99;
  for (int ii = 0; ii < _nSamples; ii++) {
    sumRe += iq[ii].re;
    sumIm += iq[ii].im;
    iqPhasor[ii].re = sumRe;
    iqPhasor[ii].im = sumIm;
    mag[ii] = RadarComplex::mag(iq[ii]);
    if (sumRe < minRe) minRe = sumRe;
    if (sumRe > maxRe) maxRe = sumRe;
    if (sumIm < minIm) minIm = sumIm;
    if (sumIm > maxIm) maxIm = sumIm;
  }
  double phasorReRange = maxRe - minRe;
  double phasorImRange = maxIm - minIm;

  // compute 5-point running version of CPA
  
  TaArray<double> runningCpa_;
  double *runningCpa = runningCpa_.alloc(_nSamples);
  memset(runningCpa, 0, _nSamples * sizeof(double));

  int nrun = 5;
  int nhalf = nrun/2;

  if (_nSamples >= nrun) {

    double sumMag = 0.0;
    for (int jj = 0; jj < nrun - 1; jj++) {
      sumMag += mag[jj];
    }
    
    for (int ii = nhalf; ii < _nSamples - nhalf; ii++) {
      sumMag += mag[ii+nhalf];
      double dI = iqPhasor[ii-nhalf].re - iqPhasor[ii+nhalf].re;
      double dQ = iqPhasor[ii-nhalf].im - iqPhasor[ii+nhalf].im;
      double dist = sqrt(dI * dI + dQ * dQ);
      runningCpa[ii] = dist / sumMag;
      sumMag -= mag[ii-nhalf];
    }
    for (int ii = 0; ii < nhalf; ii++) {
      runningCpa[ii] = runningCpa[nhalf];
      runningCpa[_nSamples-ii-1] = runningCpa[_nSamples-nhalf-1];
    }

    // find minimum running cpa

    double minRunningCpa = 99.0;
    int minRunningCpaIndex = 0;
    for (int ii = nhalf; ii < _nSamples - nhalf; ii++) {
      if (runningCpa[ii] < minRunningCpa) {
        minRunningCpa = runningCpa[ii];
        minRunningCpaIndex = ii;
      }
    }

    // compute CPA from either side of minimum

    sumMag = 0;
    for (int ii = 0; ii < _nSamples; ii++) {
      sumMag += mag[ii];
    }
    double dI = iqPhasor[_nSamples-1].re - iqPhasor[0].re;
    double dQ = iqPhasor[_nSamples-1].im - iqPhasor[0].im;
    double dist = sqrt(dI * dI + dQ * dQ);
    double cpa = dist / sumMag;

    _cpaAlt = cpa;

    int limit = _nSamples / 5;
    if (minRunningCpaIndex > limit &&
        minRunningCpaIndex < _nSamples - limit) {

      sumMag = 0;
      int end = minRunningCpaIndex - nhalf;
      for (int ii = 0; ii < end; ii++) {
        sumMag += mag[ii];
      }
      dI = iqPhasor[end-1].re - iqPhasor[0].re;
      dQ = iqPhasor[end-1].im - iqPhasor[0].im;
      dist = sqrt(dI * dI + dQ * dQ);
      double cpa0 = dist / sumMag;
      
      sumMag = 0;
      int start = minRunningCpaIndex + nhalf;
      for (int ii = start; ii < _nSamples; ii++) {
        sumMag += mag[ii];
      }
      dI = iqPhasor[_nSamples-1].re - iqPhasor[start].re;
      dQ = iqPhasor[_nSamples-1].im - iqPhasor[start].im;
      dist = sqrt(dI * dI + dQ * dQ);
      double cpa1 = dist / sumMag;
      
      double cpaMean = (cpa0 + cpa1) / 2.0;
      _cpaAlt = cpaMean;

    }
    
  }

  // rescale for plotting

  for (int ii = 0; ii < _nSamples; ii++) {
    iqPhasor[ii].re = 10.0 +
      ((iqPhasor[ii].re - minRe) / phasorReRange) * (_nSamples - 20);
    iqPhasor[ii].im = -1.0 + ((iqPhasor[ii].im - minIm) / phasorImRange);
  }  

  // compute the detrended time series

  RadarFft fft(_nSamples);

  TaArray<RadarComplex_t> iqDetrended_;
  RadarComplex_t *iqDetrended = iqDetrended_.alloc(_nSamples);
  RadarMoments::detrendTs(iq, _nSamples, iqDetrended);

  RegressionFilter regrOrder1;
  if (_spectra.isStaggeredPrtMode()) {
    regrOrder1.setupStaggered(_nSamples,
                              _spectra.getStaggeredM(),
                              _spectra.getStaggeredN(), 1);
  } else {
    regrOrder1.setup(_nSamples, 1);
  }
  
  TaArray<RadarComplex_t> iqOrder1_;
  RadarComplex_t *iqOrder1 = iqOrder1_.alloc(_nSamples);
  regrOrder1.apply(iq, iqOrder1);
  const RadarComplex_t *order1PolyFit = regrOrder1.getPolyfitIq();

  // create window for the iq data, for FFT operations
  
  TaArray<double> windowCoeff_;
  double *windowCoeff = windowCoeff_.alloc(_nSamples);
  if (_params->window == Params::WINDOW_RECT) {
    RadarMoments::initWindowRect(_nSamples, windowCoeff);
  } else if (_params->window == Params::WINDOW_BLACKMAN) {
    RadarMoments::initWindowBlackman(_nSamples, windowCoeff);
  } else {
    RadarMoments::initWindowVonhann(_nSamples, windowCoeff);
  }

  // compute power spectrum
  
  TaArray<RadarComplex_t> iqWindowed_;
  RadarComplex_t *iqWindowed = iqWindowed_.alloc(_nSamples);
  if (_params->use_order_1_fit_time_series_for_adaptive_filter) {
    RadarMoments::applyWindow(iqOrder1, windowCoeff, iqWindowed, _nSamples);
  } else if (_params->use_detrended_time_series_for_adaptive_filter) {
    RadarMoments::applyWindow(iqDetrended, windowCoeff, iqWindowed, _nSamples);
  } else {
    RadarMoments::applyWindow(iq, windowCoeff, iqWindowed, _nSamples);
  }
  
  TaArray<RadarComplex_t> powerSpec_;
  RadarComplex_t *powerSpec = powerSpec_.alloc(_nSamples);
  fft.fwd(iqWindowed, powerSpec);
  
  // compute power spectrum for next gate
  
  TaArray<RadarComplex_t> nextIqWindowed_;
  RadarComplex_t *nextIqWindowed = nextIqWindowed_.alloc(_nSamples);
  RadarMoments::applyWindow(nextIq, windowCoeff, nextIqWindowed, _nSamples);
  
  TaArray<RadarComplex_t> nextPowerSpec_;
  RadarComplex_t *nextPowerSpec = nextPowerSpec_.alloc(_nSamples);
  fft.fwd(nextIqWindowed, nextPowerSpec);
  
  // apply adaptive clutter filter
  
  double filterRatio = 1.0;
  double spectralNoise = 1.0;
  double spectralSnr = 1.0e-13;
  
  TaArray<RadarComplex_t> filtWindowed_;
  RadarComplex_t *filtWindowed = filtWindowed_.alloc(_nSamples);

  _gateMoments.pratio = moments.computePowerRatio(iqWindowed, _nSamples);
  moments.applyAdaptiveFilter(_nSamples, fft,
                              iqWindowed, NULL,
                              _spectra.getNoise() / rcvGain,
                              filtWindowed, NULL,
                              filterRatio,
                              spectralNoise,
                              spectralSnr);
  
  if (filterRatio > 1.0) {
    _gateMoments.clut_2_wx_ratio = 10.0 * log10(filterRatio - 1.0);
  } else {
    _gateMoments.clut_2_wx_ratio = MomentsFields::missingDouble;
  }
  _gateMoments.spectral_noise =
    10.0 * log10(spectralNoise) /* - _spectra.getReceiverGainDb() */;
  _gateMoments.spectral_snr = 10.0 * log10(spectralSnr);
  
  // compute moments after adaptive filter
  
  moments.singlePolH(filtWindowed, _gateNum, false, _filtMoments);

  // power spectrum of adaptively-filtered data

  TaArray<RadarComplex_t> filtSpec_;
  RadarComplex_t *filtSpec = filtSpec_.alloc(_nSamples);
  fft.fwd(filtWindowed, filtSpec);
  
  // next, filter the iq data using the regression method

  double regrFilterRatio = 1.0;
  double regrSpectralNoise = 1.0;
  double regrSpectralSnr = 1.0e-13;
  
  TaArray<RadarComplex_t> regrFiltered_;
  RadarComplex_t *regrFiltered = regrFiltered_.alloc(_nSamples);
  
  RegressionFilter regrF;
  if (_spectra.isStaggeredPrtMode()) {
    regrF.setupStaggered(_nSamples,
                         _spectra.getStaggeredM(),
                         _spectra.getStaggeredN(),
                         _params->regression_polynomial_order);
  } else {
    regrF.setup(_nSamples, _params->regression_polynomial_order);
  }
  
  moments.applyRegressionFilter(_nSamples, fft, regrF, windowCoeff, iq,
                                _spectra.getNoise() - rcvGainDb,
                                _params->regression_interp_across_notch,
                                regrFiltered, NULL,
                                regrFilterRatio,
                                regrSpectralNoise,
                                regrSpectralSnr);

  // window the regression-filtered time series

  TaArray<RadarComplex_t> regrWindowed_;
  RadarComplex_t *regrWindowed = regrWindowed_.alloc(_nSamples);
  RadarMoments::applyWindow(regrFiltered, windowCoeff, regrWindowed, _nSamples);

  // get the polynomial fit

  const RadarComplex_t *regrPolyFit = regrF.getPolyfitIq();
  const double *regrX = regrF.getX();
  
  // compute the spectrum of the regression-filtered data

  TaArray<RadarComplex_t> regrSpec_;
  RadarComplex_t *regrSpec = regrSpec_.alloc(_nSamples);
  fft.fwd(regrWindowed, regrSpec);

  // compute fft phase diff
  
  TaArray<RadarComplex_t> fftForPhase0_;
  TaArray<RadarComplex_t> fftForPhase1_;
  RadarComplex_t *fftForPhase0 = fftForPhase0_.alloc(_nSamples - 1);
  RadarComplex_t *fftForPhase1 = fftForPhase1_.alloc(_nSamples - 1);

  RadarFft fft1(_nSamples - 1);
  fft1.fwd(iq, fftForPhase0);
  fft1.fwd(iq + 1, fftForPhase1);
  
  TaArray<double> fftPhaseDiff_;
  double *fftPhaseDiff = fftPhaseDiff_.alloc(_nSamples);
  for (int ii = 0; ii < _nSamples - 1; ii++) {
    RadarComplex_t diff =
      RadarComplex::conjugateProduct(fftForPhase1[ii], fftForPhase0[ii]);
    double phaseDiff = atan2(diff.im, diff.re) * RAD_TO_DEG;
    fftPhaseDiff[ii] = phaseDiff;
  }
  fftPhaseDiff[_nSamples-1] = fftPhaseDiff[_nSamples-2];
  
  TaArray<double> accumFftPhaseDiff_;
  double *accumFftPhaseDiff = accumFftPhaseDiff_.alloc(_nSamples);
  accumFftPhaseDiff[0] = fftPhaseDiff[_nSamplesHalf];
  double prevDiff = fftPhaseDiff[_nSamplesHalf];
  for (int ii = 1; ii < _nSamples; ii++) {
    double diff = fftPhaseDiff[(ii + _nSamplesHalf) % _nSamples];
    double deltaDiff = diff - prevDiff;
    if (deltaDiff < -180) {
      deltaDiff += 360;
    } else if (deltaDiff > 180) {
      deltaDiff -= 360;
    }
    accumFftPhaseDiff[ii] = accumFftPhaseDiff[ii-1] + deltaDiff;
    prevDiff = diff;
  }
  
  // populate fields

  double itime = 0.0;
  double cumulativeDiff = 0.0;
  for (int ii = 0; ii < _nSamples; ii++) {
    
    double power_td = iq[ii].re * iq[ii].re + iq[ii].im * iq[ii].im;
    double phase_td = atan2(iq[ii].im, iq[ii].re) * RAD_TO_DEG;
    double phase_residual =
      atan2(regrFiltered[ii].im, regrFiltered[ii].re) * RAD_TO_DEG;

    int jj = (ii + _nSamples / 2) % _nSamples;
    
    double power_spec =
      powerSpec[jj].re * powerSpec[jj].re +
      powerSpec[jj].im * powerSpec[jj].im;
    
    double next_power_spec =
      nextPowerSpec[jj].re * nextPowerSpec[jj].re +
      nextPowerSpec[jj].im * nextPowerSpec[jj].im;
    
    double filt_spec =
      filtSpec[jj].re * filtSpec[jj].re +
      filtSpec[jj].im * filtSpec[jj].im;

    double regr_spec =
      regrSpec[jj].re * regrSpec[jj].re +
      regrSpec[jj].im * regrSpec[jj].im;
    
    double tdDb = 10.0 * log10(power_td) /* - rcvGainDb */;
    double specDb = 10.0 * log10(power_spec) /* - rcvGainDb */;
    double nextSpecDb = 10.0 * log10(next_power_spec) /* - rcvGainDb */;
    double filtDb = 10.0 * log10(filt_spec) /* - rcvGainDb */;
    double regrDb = 10.0 * log10(regr_spec) /* - rcvGainDb */;

    _powerTs->addData(itime, tdDb);
    _phaseTs->addData(itime, phase_td);
    _phaseResidual->addData(itime, phase_residual);
    _powerSpec->addData(ii, specDb);
    _nextPowerSpec->addData(ii, nextSpecDb);
    _adapFiltSpec->addData(ii, filtDb);
    _regrFiltSpec->addData(ii, regrDb);
    
    // compute pulse-to-pulse phase difference
    
    if (ii > 0) {
      RadarComplex_t diff = RadarComplex::conjugateProduct(iq[ii], iq[ii - 1]);
      double phaseDiff = RadarComplex::argDeg(diff);
      cumulativeDiff += phaseDiff;
      if (ii == 1) {
	_phaseDiff->addData(itime, phaseDiff); // first diff equals second
      }
      _phaseDiff->addData(itime, phaseDiff);
      _cumulativePhaseDiff->addData(itime, cumulativeDiff);
    }

    _fftPhaseDiff->addData(ii, accumFftPhaseDiff[ii]);
    // _fftPhaseDiff->addData(ii, fftPhaseDiff[ii]);
    
    // IQ time series
    
    double mult = 1.0e3;
    _iTs->addData(regrX[ii], iq[ii].re * mult);
    _qTs->addData(regrX[ii], iq[ii].im * mult);

    if (_params->use_order_1_fit_time_series_for_adaptive_filter) {
      _iTsPolyFit->addData(regrX[ii], order1PolyFit[ii].re * mult);
      _qTsPolyFit->addData(regrX[ii], order1PolyFit[ii].im * mult);
    } else {
      _iTsPolyFit->addData(regrX[ii], regrPolyFit[ii].re * mult);
      _qTsPolyFit->addData(regrX[ii], regrPolyFit[ii].im * mult);
    }

    if (_params->use_order_1_fit_time_series_for_adaptive_filter) {
      _iTsResidual->addData(regrX[ii], iqOrder1[ii].re * mult);
      _qTsResidual->addData(regrX[ii], iqOrder1[ii].im * mult);
    } else if (_params->use_detrended_time_series_for_adaptive_filter) {
      _iTsResidual->addData(regrX[ii], iqDetrended[ii].re * mult);
      _qTsResidual->addData(regrX[ii], iqDetrended[ii].im * mult);
    } else {
      _iTsResidual->addData(regrX[ii], regrFiltered[ii].re * mult);
      _qTsResidual->addData(regrX[ii], regrFiltered[ii].im * mult);
    }

    _iqPhasor->addData(iqPhasor[ii].re, iqPhasor[ii].im);
    _runningCpa->addData(itime, runningCpa[ii]);

    itime++;
    
  } // ii

  // compute the notch ratio, indicating likelihood of clutter

  compute_tss(iqWindowed);

  // compute the power for each gate (AScope mode)
  
  double range = _spectra.getStartRange();
  for (int igate = 0; igate < _nGates; igate++) {

    vector<RadarSpectra::radar_iq_t> gateIq = _spectra.getGateIq(igate);
    double sumPower = 0.0;
    for (int ii = 0; ii < _nSamples; ii++) {
      double ival = gateIq[ii].ival / sqrtRcvGain;
      double qval = gateIq[ii].qval / sqrtRcvGain;
      sumPower += ival * ival + qval * qval;
    }
    double meanPower = sumPower / _nSamples;
    double powerDbm = 10.0 * log10(meanPower);
    _powerAscope->addData(range, powerDbm);
    range += _spectra.getGateSpacing();

  }

  //////////////////////////////////////////////////
  // spf

  // get windowed IQ

  int nSpf = _nSamples * 5;
  int nSpfHalf = nSpf / 2;
  TaArray<RadarComplex_t> spfIq_;
  RadarComplex_t *spfIq = spfIq_.alloc(nSpf);
  for (int ii = 0; ii < _nSamples; ii++) {
    int jj = nSpf - ii - 1;
    spfIq[jj].re = iqWindowed[ii].re;
    spfIq[jj].im = iqWindowed[ii].im;
  }

  // pad out with zeros

  for (int ii = _nSamples; ii < nSpf; ii++) {
    int jj = nSpf - ii - 1;
    spfIq[jj].re = 0.0;
    spfIq[jj].im = 0.0;
  }

  // compute FFT
  
  RadarFft spfFft(nSpf);
  TaArray<RadarComplex_t> spfSpec_;
  RadarComplex_t *spfSpec = spfSpec_.alloc(nSpf);
  spfFft.fwd(spfIq, spfSpec);

  // load up results
  
  for (int ii = nSpfHalf - _nSamplesHalf; 
       ii <= nSpfHalf + _nSamplesHalf; ii++) {
    int jj = (2 * nSpf - ii - nSpfHalf) % nSpf;
    double pwr = RadarComplex::power(spfSpec[jj]);
    double dbm = 10.0 * log10(pwr);
    double phasePrev = RadarComplex::argDeg(spfSpec[jj-1]);
    double phase = RadarComplex::argDeg(spfSpec[jj]);
    double phaseChange = phase - phasePrev;
    if (phaseChange > 180) {
      phaseChange -= 360.0;
    } else if (phaseChange < -180) {
      phaseChange += 360.0;
    }
    _spfPower->addData(ii, dbm);
    if (ii != nSpfHalf) {
      _spfPhaseDiff->addData(ii, phaseChange);
    }
  } // ii
  
}

/*************************************************************************
 * load up plotting data for single PRT
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
    iq[jj].re = gateIq[ii].ival;
    iq[jj].im = gateIq[ii].qval;
  }
  
  // separate the time series into short/long PRT series
  
  TaArray<RadarComplex_t> iqShort_, iqLong_;
  RadarComplex_t *iqShort = iqShort_.alloc(_nSamplesHalf);
  RadarComplex_t *iqLong = iqLong_.alloc(_nSamplesHalf);
  RadarMoments::separateStagIq(_nSamples, iq, iqShort, iqLong);

  TaArray<RadarComplex_t> diffShort_, diffLong_;
  RadarComplex_t *diffShort = diffShort_.alloc(_nSamplesHalf);
  RadarComplex_t *diffLong = diffLong_.alloc(_nSamplesHalf);
  
  TaArray<double> diffShortDeg_, diffLongDeg_;
  double *diffShortDeg = diffShortDeg_.alloc(_nSamplesHalf);
  double *diffLongDeg = diffLongDeg_.alloc(_nSamplesHalf);
  
  for (int ii = 1; ii < _nSamplesHalf; ii++) {
    
    RadarComplex_t diffShrt =
      RadarComplex::conjugateProduct(iqLong[ii], iqShort[ii]);
    double diffShrtDeg = RadarComplex::argDeg(diffShrt);
    diffShort[ii] = diffShrt;
    diffShortDeg[ii] = diffShrtDeg;

    RadarComplex_t diffLng =
      RadarComplex::conjugateProduct(iqShort[ii], iqLong[ii-1]);
    double diffLngDeg = RadarComplex::argDeg(diffLng);
    diffLong[ii] = diffLng;
    diffLongDeg[ii] = diffLngDeg;

  }

  // make pseudo time series
  
  TaArray<RadarComplex_t> pseudoShort_, pseudoLong_;
  RadarComplex_t *pseudoShort = pseudoShort_.alloc(_nSamplesHalf);
  RadarComplex_t *pseudoLong = pseudoLong_.alloc(_nSamplesHalf);
  // double sumDiff = 0.0;

  //   RadarComplex_t sumDiff;
  //   sumDiff.re = 1.0;
  //   sumDiff.im = 0.0;

  pseudoShort[0] = iqLong[0];
  pseudoLong[0] = iqShort[0];
  
  double argShortDeg = RadarComplex::argDeg(pseudoShort[0]);
  double argLongDeg = RadarComplex::argDeg(pseudoLong[0]);
  
  for (int ii = 1; ii < _nSamplesHalf; ii++) {

    argShortDeg += diffShortDeg[ii];
    argLongDeg += diffLongDeg[ii];
    
    double magShort = RadarComplex::mag(iqLong[ii]);
    pseudoShort[ii].re = magShort * cos(argShortDeg * DEG_TO_RAD);
    pseudoShort[ii].im = magShort * sin(argShortDeg * DEG_TO_RAD);

    double magLong = RadarComplex::mag(iqLong[ii]);
    pseudoLong[ii].re = magLong * cos(argLongDeg * DEG_TO_RAD);
    pseudoLong[ii].im = magLong * sin(argLongDeg * DEG_TO_RAD);

  }

  // memcpy(pseudoShort, iqShort, _nSamplesHalf * sizeof(RadarComplex_t));
  
  // set up cal and moments objects
  
  IwrfCalib calib;
  calib.setWavelengthCm(_spectra.getWavelengthCm());
  calib.setNoiseDbmHc(_spectra.getNoiseDbm());
  calib.setReceiverGainDbHc(_spectra.getReceiverGainDb());
  // calib.setReceiverGainDbHc(0.0);
  calib.setBaseDbz1kmHc(_spectra.getBaseDbz1km());
  
  RadarMoments moments(_gateNum + 1,
                       _params->debug >= Params::DEBUG_EXTRA, false);
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
  
  moments.singlePolHStagPrt(iq, iqShort, iqLong, _gateNum, false, _gateMoments);
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
  moments.singlePolHStagPrt(regrFiltered, regrShort, regrLong,
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

  // load up staggered spectra data
  
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

  // get the polygon fit
  
  const RadarComplex_t *regrPolyFit = regrF.getPolyfitIq();
  const double *regrX = regrF.getX();
  
  // populate other data fields
  
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
    double powerDbm = 10.0 * log10(meanPower) /* - _spectra.getReceiverGainDb() */;
    _powerAscope->addData(range, powerDbm);
    range += _spectra.getGateSpacing();

  }

  //////////////////////////////////////////////////////////////
  // compute spectra for both short and long series
  
  RadarFft fftHalf(_nSamplesHalf);
  
  TaArray<RadarComplex_t> pseudoShortSpec_;
  RadarComplex_t *pseudoShortSpec = pseudoShortSpec_.alloc(_nSamplesHalf);
  fftHalf.fwd(pseudoShort, pseudoShortSpec);

  TaArray<RadarComplex_t> pseudoLongSpec_;
  RadarComplex_t *pseudoLongSpec = pseudoLongSpec_.alloc(_nSamplesHalf);
  fftHalf.fwd(pseudoLong, pseudoLongSpec);

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

    double power = RadarComplex::power(shortSpec[jj]);
    double powerDb = 10.0 * log10(power);
    double phase = RadarComplex::argDeg(shortSpec[jj]);
    _stagSpecShort->addData(itime, powerDb);
    _stagSpecShortPhase->addData(itime, phase);
    powerShortSpec[jj] = power;
    
    power = RadarComplex::power(longSpec[jj]);
    powerDb = 10.0 * log10(power);
    phase = RadarComplex::argDeg(longSpec[jj]);
    _stagSpecLong->addData(itime, powerDb);
    _stagSpecLongPhase->addData(itime, phase);
    powerLongSpec[jj] = power;

    RadarComplex_t diff =
      RadarComplex::conjugateProduct(pseudoLongSpec[jj], pseudoShortSpec[jj]);
    double diffPhase = RadarComplex::argDeg(diff);
    
    _stagSpecPhaseDiff->addData(itime, diffPhase);
    
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
    double powerDb = 10.0 * log10(power);
    // double phase = RadarComplex::argDeg(filtShortSpec[jj]);
    _stagFiltSpecShort->addData(itime, powerDb);
    powerFiltShortSpec[jj] = power;

    power = RadarComplex::power(filtLongSpec[jj]);
    powerDb = 10.0 * log10(power);
    // phase = RadarComplex::argDeg(filtLongSpec[jj]);
    _stagFiltSpecLong->addData(itime, powerDb);
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

    itime++;
    
  }

#ifdef JUNK

  // perform gaussian infill for powers, keeping phases unchanged

  TaArray<RadarComplex_t> filledShortSpec_;
  RadarComplex_t *filledShortSpec = filledShortSpec_.alloc(_nSamplesHalf);
  
  TaArray<RadarComplex_t> filledLongSpec_;
  RadarComplex_t *filledLongSpec = filledLongSpec_.alloc(_nSamplesHalf);
  
  int maxNotchWidth = 7;
  ClutFilter::fillNotchUsingGfit(filtShortSpec, _nSamplesHalf,
                                 maxNotchWidth, filledShortSpec);
  
  ClutFilter::fillNotchUsingGfit(filtLongSpec, _nSamplesHalf,
                                 maxNotchWidth, filledLongSpec);
  
  // for the long-PRT half spectrum, interpolate the phase difference
  // between the short and long
  
  int notchWidthHalf = maxNotchWidth / 2;
  if (notchWidthHalf > _nSamplesHalf - 1) notchWidthHalf = _nSamplesHalf - 1;
  int startIndex =  -notchWidthHalf;
  int endIndex = notchWidthHalf;
  
  RadarComplex_t diffStart =
    RadarComplex::conjugateProduct(filledLongSpec[startIndex],
                                   filledShortSpec[startIndex]);
  double diffPhaseStart = RadarComplex::argRad(diffStart);
  
  RadarComplex_t diffEnd =
    RadarComplex::conjugateProduct(filledLongSpec[endIndex],
                                   filledShortSpec[endIndex]);
  double diffPhaseEnd = RadarComplex::argRad(diffEnd);
  double deltaDiffPhase = RadarComplex::diffRad(diffPhaseEnd, diffPhaseStart);
  
  double count = 0.0;
  for (int ii = startIndex; ii <= endIndex; ii++, count++) {

    double interpFraction = count / (double) maxNotchWidth;
    
    int jj = (ii + _nSamplesHalf) % _nSamplesHalf;
    
    double interpDiffPhase = diffPhaseStart + interpFraction * deltaDiffPhase;
    double phaseShort = RadarComplex::argRad(filledShortSpec[jj]);
    double phaseLong = RadarComplex::sumRad(phaseShort, interpDiffPhase);
    double interpMag = RadarComplex::mag(filledLongSpec[jj]);
    filledLongSpec[jj].re = interpMag * cos(phaseLong);
    filledLongSpec[jj].im = interpMag * sin(phaseLong);
    
  }
  
  itime = 0.0;
  for (int ii = 0; ii < _nSamplesHalf; ii++) {
    
    int jj = (ii + _nSamplesHalf / 2) % _nSamplesHalf;
    
    double power = RadarComplex::power(filledShortSpec[jj]);
    double powerDb = 10.0 * log10(power);
    _stagFiltSpecShortInterp->addData(itime, powerDb);
    
    power = RadarComplex::power(filledLongSpec[jj]);
    powerDb = 10.0 * log10(power);
    _stagFiltSpecLongInterp->addData(itime, powerDb);
    
    RadarComplex_t diff =
      RadarComplex::conjugateProduct(filledLongSpec[jj], filledShortSpec[jj]);
    double diffPhase = RadarComplex::argDeg(diff);
    _stagFiltSpecPhaseDiffInterp->addData(itime, diffPhase);

    itime++;

  } // ii

#endif

  int notchWidth = 5;
  int notchWidthHalf = notchWidth / 2;
  int startIndex = _nSamplesHalf - notchWidthHalf;
  int endIndex = notchWidthHalf;
  int nCenter = _nSamplesHalf / 2;
  
  double powerStart = RadarComplex::power(filtShortSpec[startIndex]);
  double powerEnd = RadarComplex::power(filtShortSpec[endIndex]);
  double deltaPower = powerEnd - powerStart;
  
  for (int ii = nCenter - notchWidthHalf + 1; ii < nCenter + notchWidthHalf; ii++) {
    
    int jj = (ii + _nSamplesHalf / 2) % _nSamplesHalf;
    int kk = ii - (nCenter - notchWidthHalf);
    double interpFraction = (double) kk / (double) notchWidth;
    double interpPower = powerStart + interpFraction * deltaPower;
    double origPower = RadarComplex::power(filtShortSpec[jj]);
    double powerRatio = interpPower / origPower;
    double magRatio = sqrt(powerRatio);
    filtShortSpec[jj].re *= magRatio;
    filtShortSpec[jj].im *= magRatio;

  }
  
  itime = 0.0;
  for (int ii = 0; ii < _nSamplesHalf; ii++) {
    int jj = (ii + _nSamplesHalf / 2) % _nSamplesHalf;
    double power = RadarComplex::power(filtShortSpec[jj]);
    double powerDb = 10.0 * log10(power);
    _stagFiltSpecShortInterp->addData(itime, powerDb);
    itime++;
  } // ii

  // for the long-PRT half spectrum, interpolate the power
  // across the filter notch, and interpolate the phase difference
  // between the short and long

  powerStart = RadarComplex::power(filtLongSpec[startIndex]);
  powerEnd = RadarComplex::power(filtLongSpec[endIndex]);
  deltaPower = powerEnd - powerStart;

  RadarComplex_t diffStart =
    RadarComplex::conjugateProduct(filtLongSpec[startIndex], filtShortSpec[startIndex]);
  double diffPhaseStart = RadarComplex::argRad(diffStart);

  RadarComplex_t diffEnd =
    RadarComplex::conjugateProduct(filtLongSpec[endIndex], filtShortSpec[endIndex]);
  double diffPhaseEnd = RadarComplex::argRad(diffEnd);

  double deltaDiffPhase = RadarComplex::diffRad(diffPhaseEnd, diffPhaseStart);
  
  for (int ii = nCenter - notchWidthHalf + 1; ii < nCenter + notchWidthHalf; ii++) {
    
    int jj = (ii + _nSamplesHalf / 2) % _nSamplesHalf;
    int kk = ii - (nCenter - notchWidthHalf);
    double interpFraction = (double) kk / (double) notchWidth;

    // power

    double interpPower = powerStart + interpFraction * deltaPower;
    // double interpMag = sqrt(interpPower);

    double origPower = RadarComplex::power(filtLongSpec[jj]);
    double powerRatio = interpPower / origPower;
    double magRatio = sqrt(powerRatio);
    filtLongSpec[jj].re *= magRatio;
    filtLongSpec[jj].im *= magRatio;
    
    // phase
    
    double interpDiffPhase = diffPhaseStart + interpFraction * deltaDiffPhase;
    double phaseShort = RadarComplex::argRad(filtShortSpec[jj]);
    double phaseLong = RadarComplex::sumRad(phaseShort, interpDiffPhase);
    double interpMag = RadarComplex::mag(filtLongSpec[jj]);
    filtLongSpec[jj].re = interpMag * cos(phaseLong);
    filtLongSpec[jj].im = interpMag * sin(phaseLong);

  }
  
  itime = 0.0;
  for (int ii = 0; ii < _nSamplesHalf; ii++) {

    int jj = (ii + _nSamplesHalf / 2) % _nSamplesHalf;

    double power = RadarComplex::power(filtLongSpec[jj]);
    double powerDb = 10.0 * log10(power);
    _stagFiltSpecLongInterp->addData(itime, powerDb);

    RadarComplex_t diff =
      RadarComplex::conjugateProduct(filtLongSpec[jj], filtShortSpec[jj]);
    double diffPhase = RadarComplex::argDeg(diff);
    _stagFiltSpecPhaseDiffInterp->addData(itime, diffPhase);

    itime++;

  } // ii

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

///////////////////////////////////////////////////
// compute the periodicity in the time series power

static void compute_tss(const RadarComplex_t *iqWindowed)
  
{

  // load up time series magnitude

  TaArray<double> pwrTs_;
  double *pwrTs = pwrTs_.alloc(_nSamples);
  
  double sumPwrSq = 0.0;
  for (int ii = 0; ii < _nSamples; ii++) {
    double II = iqWindowed[ii].re;
    double QQ = iqWindowed[ii].im;
    double pwr = II * II + QQ * QQ;
    pwrTs[ii] = pwr;
    sumPwrSq += pwr * pwr;
  }
  
  // compute fft of time domain magnitude, using real fft
  
  TaArray<fftw_complex> powerFft_;
  fftw_complex *powerFft = powerFft_.alloc(_nSamples);
  fftw_plan plan =
    fftw_plan_dft_r2c_1d(_nSamples, pwrTs, powerFft, FFTW_MEASURE);
  fftw_execute(plan);
  fftw_destroy_plan(plan);
  
  double sqrtN = sqrt((double) _nSamples);
  for (int ii = 0; ii < _nSamples; ii++) {
    powerFft[ii][0] /= sqrtN;
    powerFft[ii][1] /= sqrtN;
  }

  // load up spectrum for plotting
  
  for (int ii = 0; ii < _nSamples; ii++) {
    int jj = ii;
    if (ii < _nSamples / 2) {
      jj = _nSamples - (ii + _nSamples / 2);
    } else {
      jj = ii - _nSamples / 2;
    }
    double II = powerFft[jj][0];
    double QQ = powerFft[jj][1];
    double specPwr =  sqrt(II * II + QQ * QQ);
    _powerSpecReal->addData(ii, 10.0 * log10(specPwr));
    /* - _spectra.getReceiverGainDb() */
  }

  // compute the power in the notch

  int n_notch = _params->tpss_notch_width;
  double sumSqNotch = 0.0;
  for (int ii = 0; ii < _nSamplesHalf; ii++) {
    double II = powerFft[ii][0];
    double QQ = powerFft[ii][1];
    double pwr =  II * II + QQ * QQ;
    if (ii == 0) {
      sumSqNotch += pwr;
    } else if (ii < n_notch) {
      sumSqNotch += pwr * 2;
    }
  }
  
  if (sumSqNotch > 0) {
    _tssDb = 10.0 * log10(sumSqNotch / (sumPwrSq - sumSqNotch));
  } else {
    _tssDb = -999;
  }

}
/*
 * strip_chart_ui.c - User interface object initialization functions.
 */

/*
 * Create object `menu1' in the specified instance.
 */
Xv_opaque
  strip_chart_menu1_create(caddr_t ip, Xv_opaque owner)
{
  extern Menu		select_field_proc(Menu, Menu_generate);
  Xv_opaque	obj;

  obj = xv_create(XV_NULL, MENU_CHOICE_MENU,
		  XV_KEY_DATA, INSTANCE, ip,
		  MENU_GEN_PROC, select_field_proc,
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
static strip_chart_win1_objects *
  strip_chart_win1_objects_initialize(strip_chart_win1_objects *ip, Xv_opaque owner)
{
  if (!ip && !(ip = (strip_chart_win1_objects *) calloc(1, sizeof (strip_chart_win1_objects))))
    return (strip_chart_win1_objects *) NULL;
  if (!ip->win1)
    ip->win1 = strip_chart_win1_win1_create(ip, owner);
  if (!ip->canvas1)
    ip->canvas1 = strip_chart_win1_canvas1_create(ip, ip->win1);
  return ip;
}

/*
 * Create object `win1' in the specified instance.
 */

static Xv_opaque
  strip_chart_win1_win1_create(strip_chart_win1_objects *ip, Xv_opaque owner)
{
  //	extern Notify_value win1_event_proc(Xv_window, Event *, Notify_arg, Notify_event_type);
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
  strip_chart_win1_canvas1_create(strip_chart_win1_objects *ip, Xv_opaque owner)
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
	 WIN_MENU, strip_chart_menu1_create((caddr_t) ip, ip->win1), NULL);

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
			      (Notify_func) strip_chart_win1_canvas1_event_callback,
			      NOTIFY_SAFE);
  return obj;

}

 
/*****************************************************************
 * CLEAR PLOT AREA
 *
 */

static void clear_plot()
{

  XSetForeground(_dpy, _def_gc, _bg_cell);
  XSetBackground(_dpy, _def_gc, _bg_cell);
  XFillRectangle(_dpy,_back_xid,_def_gc, 0,0, _win_width, _win_height);

}

/*****************************************************************
 * DRAW_PLOT: plotting functions
 *
 */

#define AXIS_BIT 0x0001  /* Single bit to control printing of an axis */
#define UNITS_BIT 0x0002 /* Single bit to control print units
			  * instead of time label*/
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
  
  clear_plot();

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
  string scanModeStr = "PPI";
  if (_isRhi) {
    scanModeStr = "RHI";
  }

  // no data? Plot messages to indicate this

  if (!_data_avail) {

    sprintf(string_buf, "NO DATA AVAILABLE FOR CLICK POINT AND CIDD TIME");
    
    XSetForeground(_dpy, _def_gc, _fg_cell);
    plot_string_centered(string_buf, x_start, y_line);
    y_line += _line_spacing;
    
    if (_isRhi) {
      sprintf(string_buf, "RHI, startTime: %s, endTime: %s",
              DateTime::strm(_start_time_rhi).c_str(),
              DateTime::strm(_end_time_rhi).c_str());
    } else {
      sprintf(string_buf, "PPI, startTime: %s, endTime: %s",
              DateTime::strm(_start_time_ppi).c_str(),
              DateTime::strm(_end_time_ppi).c_str());
    }
    
    plot_string_centered(string_buf, x_start, y_line);
    y_line += _line_spacing;

    sprintf(string_buf,
            "El(deg):%.2f, Az(deg):%.2f, Range(km):%.2f",
            _clickElevDeg, _clickAzDeg, _clickRangeKm);
    
    plot_string_centered(string_buf, x_start, y_line);
    y_line += _line_spacing;

    XCopyArea(_dpy,_back_xid,_canvas_xid,_def_gc,
	      0,0,_win_width,_win_height,0,0);
    return;

  }
  
  // Title and time

  time_t secs = _spectra.getTimeSecs();
  int msecs = (int) (fmod(_spectra.getDoubleTime(), 1.0) * 1000 + 0.5);
  sprintf(string_buf, "%s %s %s.%.3d %s %s",
	  _spectra.getRadarName().c_str(),
	  _channelName.c_str(), DateTime::strm(secs).c_str(), msecs,
	  _spectra.getNotes().c_str(),
          scanModeStr.c_str());

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
    // sprintf(string_buf,
    //         "Dbz:%.2f  dbzF:%.2f Vel(m/s):%.2f  Width(m/s):%.2f  "
    //         "CPA:%.2f  PR:%.2f  MVAR:%.2f TPT: %.1f",
    //         _gateMoments.dbz, _filtMoments.dbz,
    //         _gateMoments.vel, _gateMoments.width,
    //         _gateMoments.cpa, _gateMoments.pratio,
    //         _gateMoments.mvar, _gateMoments.tpt);
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
            "CPA:%.2f  CPA_alt:%.2f  PR:%.2f  CWR(dB):%.2f",
            _gateMoments.cpa, _cpaAlt, _gateMoments.pratio,
            _gateMoments.clut_2_wx_ratio);
    // sprintf(string_buf,
    //         "CWR(dB):%.2f  spectral_noise(dBm): %.2f  "
    //         "spectralSnr(dB):%.2f TSS(dB):%.2f CPD: %.0f",
    //         _gateMoments.clut_2_wx_ratio, _gateMoments.spectral_noise,
    //         _gateMoments.spectralSnr, _tssDb, _gateMoments.cpd);
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
  
  // power spectrum, with clutter spectrum overlaid
  
  if (!_spectra.isStaggeredPrtMode() && _params->plot_power_spectrum) {
    
    FieldInfo *thirdField = _regrFiltSpec;
    if (_params->plot_power_spectrum_for_next_gate) {
      thirdField = _nextPowerSpec;
    }

    if (_params->plot_adaptive_filtered_spectrum &&
        _params->plot_regression_filtered_spectrum) {
      compute_world_to_pixel(*_powerSpec,_adapFiltSpec,thirdField,
                             x_start, x_end, y_start_fld, y_end_fld,
                             !_params->autoscale_power_spectrum);
    } else if (_params->plot_adaptive_filtered_spectrum) {
      compute_world_to_pixel(*_powerSpec,_adapFiltSpec, NULL,
                             x_start, x_end, y_start_fld, y_end_fld,
                             !_params->autoscale_power_spectrum);
    } else if (_params->plot_regression_filtered_spectrum) {
      compute_world_to_pixel(*_powerSpec,thirdField,NULL,
                             x_start, x_end, y_start_fld, y_end_fld,
                             !_params->autoscale_power_spectrum);
    } else {
      compute_world_to_pixel(*_powerSpec,NULL,NULL,
                             x_start, x_end, y_start_fld, y_end_fld,
                             !_params->autoscale_power_spectrum);
    }
    
    plot_noise_value(x_start, x_end);
    
    if (_params->plot_regression_filtered_spectrum) {
      plot_field(*thirdField,x_start, 
                 x_end, y_start_fld, y_end_fld, false, false, true);
    }
    
    if (_params->plot_adaptive_filtered_spectrum) {
      plot_field(*_adapFiltSpec,
                 x_start, x_end, y_start_fld, y_end_fld, false, false);
    }
    
    plot_field(*_powerSpec,
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

  // staggered prt spectrum

  if (_spectra.isStaggeredPrtMode() && _params->plot_power_spectrum) {

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

  // half-spectra for staggered PRT filtering

  if (_params->plot_staggered_half_spectra) {

    // unfiltered spectra

    compute_world_to_pixel(*_stagSpecShort, _stagSpecLong, NULL,
                           x_start, x_end, y_start_fld, y_end_fld, false);
    // !_params->autoscale_power_spectrum);
    
    plot_field(*_stagSpecShort,
               x_start, x_end, y_start_fld, y_end_fld, true, false);
    plot_field(*_stagSpecLong,
               x_start, x_end, y_start_fld, y_end_fld, false, true);

    y_start_fld += each_height;
    y_end_fld += each_height;

    // unfiltered spectrum phase diff

    compute_world_to_pixel(*_stagSpecPhaseDiff, NULL, NULL,
                           x_start, x_end, y_start_fld, y_end_fld, true);
    
    plot_field(*_stagSpecPhaseDiff,
               x_start, x_end, y_start_fld, y_end_fld, true, false);

    //     plot_field(*_stagSpecShortPhase,
    //                x_start, x_end, y_start_fld, y_end_fld, true, false);

    //     plot_field(*_stagSpecLongPhase,
    //                x_start, x_end, y_start_fld, y_end_fld, true, false);

    y_start_fld += each_height;
    y_end_fld += each_height;
    
    // filtered spectra

    compute_world_to_pixel(*_stagFiltSpecShort, _stagFiltSpecLong, NULL,
                           x_start, x_end, y_start_fld, y_end_fld, false);
    // !_params->autoscale_power_spectrum);
    
    plot_field(*_stagFiltSpecShort,
               x_start, x_end, y_start_fld, y_end_fld, true, false);
    plot_field(*_stagFiltSpecLong,
               x_start, x_end, y_start_fld, y_end_fld, false, true);

    y_start_fld += each_height;
    y_end_fld += each_height;
    
    // filtered spectrum phase diff

    compute_world_to_pixel(*_stagFiltSpecPhaseDiff, NULL, NULL,
                           x_start, x_end, y_start_fld, y_end_fld, true);
    
    plot_field(*_stagFiltSpecPhaseDiff,
               x_start, x_end, y_start_fld, y_end_fld, true, false);
    
    y_start_fld += each_height;
    y_end_fld += each_height;

    // interp filtered spectra

    compute_world_to_pixel(*_stagFiltSpecShortInterp, _stagFiltSpecLongInterp, NULL,
                           x_start, x_end, y_start_fld, y_end_fld,
                           !_params->autoscale_power_spectrum);
    
    plot_field(*_stagFiltSpecShortInterp,
               x_start, x_end, y_start_fld, y_end_fld, true, false);
    plot_field(*_stagFiltSpecLongInterp,
               x_start, x_end, y_start_fld, y_end_fld, false, true);

    y_start_fld += each_height;
    y_end_fld += each_height;
    
    // interp filtered spectrum phase diff
    
    compute_world_to_pixel(*_stagFiltSpecPhaseDiffInterp, NULL, NULL,
                           x_start, x_end, y_start_fld, y_end_fld, true);
    
    plot_field(*_stagFiltSpecPhaseDiffInterp,
               x_start, x_end, y_start_fld, y_end_fld, true, false);
    
    y_start_fld += each_height;
    y_end_fld += each_height;

    // power ratio

#ifdef JUNK    
    compute_world_to_pixel(*_stagFiltSpecShortRatio, _stagFiltSpecLongRatio, NULL,
                           x_start, x_end, y_start_fld, y_end_fld, false);
    
    plot_field(*_stagFiltSpecShortRatio,
               x_start, x_end, y_start_fld, y_end_fld, true, false);
    plot_field(*_stagFiltSpecLongRatio,
               x_start, x_end, y_start_fld, y_end_fld, false, true);

    y_start_fld += each_height;
    y_end_fld += each_height;
#endif
    
  } // if (_params->plot_staggered_half_spectra)

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

  // spf power spectrum
  
  if (_params->plot_spf) {
    
    compute_world_to_pixel(*_spfPower, NULL, NULL,
                           x_start, x_end, y_start_fld, y_end_fld, false);
    
    plot_field(*_spfPower,
               x_start, x_end, y_start_fld, y_end_fld, true, true);

    y_start_fld += each_height;
    y_end_fld += each_height;
    
    compute_world_to_pixel(*_spfPhaseDiff, NULL, NULL,
                           x_start, x_end, y_start_fld, y_end_fld, false);
    
    plot_field(*_spfPhaseDiff,
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

/*************************************************************************
 * Find ts data
 * Returns 0 on success, -1 on failure
 */

static int find_ts_data(TsReader *reader,
                        time_t startTime,
                        time_t endTime,
                        double az, double el)

{

  // find the best file for the requested time
  
  if (reader->findBestFile(startTime, endTime, az, el, _isRhi)) {
    cerr << "ERROR - retrieve_ts_data" << endl;
    cerr << "  Cannot find suitable file, dir: "
         << _params->time_series_data_dir << endl;
    cerr << "  startTime: " << DateTime::strm(startTime) << endl;
    cerr << "  endTime: " << DateTime::strm(endTime) << endl;
    return -1;
  }
  
  // put up message indicating we will be retrieving data

  clear_plot();

  char text[1024];
  vector<string> messages;
  sprintf(text, "Retrieving %s data from file",
          reader->getScanModeStr().c_str());
  messages.push_back(text);
  sprintf(text, "%s",
          reader->getFilePath().c_str());
  messages.push_back(text);
  plot_messages(messages);

  _readPending = true;

  return 0;

}

/*************************************************************************
 * Retrieve the time series data from files
 * Returns 0 on success, -1 on failure
 */

static int retrieve_ts_data()

{

  if (_isRhi) {
    if (_tsReaderRhi->readAllPulses()) {
      return -1;
    }
  } else {
    if (_tsReaderPpi->readAllPulses()) {
      return -1;
    }
  }

  return 0;

}

////////////////////////////////////
// plot messages at top of plot area

static void plot_messages(const vector<string> messages)

{

  int x_start = _params->left_margin;

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
  //   int y_line = _line_spacing / 2;

  clear_plot();
  
  for (size_t ii = 0; ii < messages.size(); ii++) {

    XSetForeground(_dpy, _def_gc, _fg_cell);
    plot_string_centered(messages[ii].c_str(), x_start, y_line);
    y_line += _line_spacing;

  }

  // copy backing store to main canvas
  
  XCopyArea(_dpy,_back_xid,_canvas_xid,_def_gc,
            0,0,_win_width,_win_height,0,0);

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
    if (_plot_min_y > y_min) {
      _plot_min_y = y_min;
    }
    _plot_max_y = primaryField.getMaxY();
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

