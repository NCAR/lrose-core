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
///////////////////////////////////////////////////////////////
// VerifyTracks.cc
//
// VerifyTracks object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2005
//
///////////////////////////////////////////////////////////////
//
// VerifyTracks computes verification stats on TITAN forecasts,
// updates TITAN files accordingly and writes results to stdout
//
////////////////////////////////////////////////////////////////

#include "VerifyTracks.hh"
#include "DoVerify.hh"
#include <toolsa/str.h>
#include <toolsa/pmu.h>
#include <rapmath/math_macros.h>
#include <rapmath/umath.h>
using namespace std;

// Constructor

VerifyTracks::VerifyTracks(int argc, char **argv)

{

  isOK = true;
  _input = NULL;

  // set programe name

  _progName = "VerifyTracks";
  ucopyright((char *) _progName.c_str());

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = false;
    return;
  }

  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = false;
  }

  // init process mapper registration

  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);

  // input file object

  if (_args.inputFileList.size() > 0) {
    
    _input = new DsInputPath((char *)_progName.c_str(),
			     _params.debug >= Params::DEBUG_VERBOSE,
			     _args.inputFileList);
      
  } else {

    // check that start and end times are set

    if (_args.startTime == 0 || _args.endTime == 0) {
      fprintf(stderr, "ERROR: %s\n", _progName.c_str());
      fprintf(stderr,
	      "You must either specify a file list using -f,\n"
	      " or you must specify -start and -end.\n");
      isOK = FALSE;
      return;
    }
      
    _input = new DsInputPath((char *) _progName.c_str(),
			     _params.debug >= Params::DEBUG_VERBOSE,
			     _params.input_dir,
			     _args.startTime,
			     _args.endTime);
    _input->setSearchExt(TRACK_HEADER_FILE_EXT);
  }

  return;

}

//////////////
// destructor

VerifyTracks::~VerifyTracks()

{

  // unregister process

  PMU_auto_unregister();

  // free up

  if (_input) {
    delete(_input);
  }

}

//////////////////////////////////////////////////
// Run

int VerifyTracks::Run ()
{
  
  PMU_auto_register("VerifyTracks::Run");

 // loop through the track files

  char *trackFilePath;
  _input->reset();
  vector<string> _pathsUsed;
  DoVerify doVerify(_progName, _params);
  
  while ((trackFilePath = _input->next()) != NULL) {
    if (doVerify.processFile(trackFilePath)) {
      cerr << "ERROR - VerifyTracks::Run" << endl;
      cerr << "  Processing file: " << trackFilePath << endl;
      return -1;
    }
    _pathsUsed.push_back(trackFilePath);
  } // while

  /*
   * print out
   */

  fprintf(stdout, "\nVERIFY_TRACKS\n-------------\n\n");

  fprintf(stdout, "Track files used:\n");

  for (int ifile = 0; ifile < (int) _pathsUsed.size(); ifile++) {
    fprintf(stdout, "  %s\n", _pathsUsed[ifile].c_str());
  }
  fprintf(stdout, "\n");
  
  fprintf(stdout, "Verification grid:\n");
  fprintf(stdout, "  dx, dy                          : %g, %g\n",
	  doVerify.getDx(), doVerify.getDy());
  fprintf(stdout, "  minx, miny                      : %g, %g\n",
	  doVerify.getMinx(), doVerify.getMiny());
  fprintf(stdout, "  nx, ny                          : %d, %d\n",
	  doVerify.getNx(), doVerify.getNy());
  fprintf(stdout, "\n");

  if (_params.forecast_shape == Params::FORECAST_ELLIPSE) {
    fprintf(stdout, "Forecast shape                    : ELLIPSE\n");
  } else {
    fprintf(stdout, "Forecast shape                    : POLYGON\n");
  }

  if (_params.verify_method == Params::VERIFY_ELLIPSE) {
    fprintf(stdout, "Verify method                     : ELLIPSE\n");
  } else if (_params.verify_method == Params::VERIFY_POLYGON) {
    fprintf(stdout, "Verify method                     : POLYGON\n");
  } else if (_params.verify_method == Params::VERIFY_RUNS) {
    fprintf(stdout, "Verify method                     : RUNS\n");
  } else {
    fprintf(stdout, "Verify method                     : MDV files\n");
  }

  fprintf(stdout, "Forecast lead time (mins)         : %g\n", 
	  _params.forecast_lead_time / 60.0);

  fprintf(stdout, "Forecast lead time margin (mins)  : %g\n", 
	  _params.search_time_margin / 60.0);

  fprintf(stdout, "Forecast min history (mins)       : %g\n", 
	  _params.forecast_min_history / 60.0);

  if (_params.growth_function == Params::GROWTH_PARABOLIC) {
    fprintf(stdout, "Growth function                   : PARABOLIC\n");
  } else {
    fprintf(stdout, "Growth function                   : LINEAR\n");
  }

  fprintf(stdout, "Time to zero growth (mins)        : %g\n", 
	  _params.time_to_zero_growth / 60.0);

  fprintf(stdout, "Verify_before_forecast_time       : %s\n", 
	  BOOL_STR(_params.verify_before_forecast_time));

  fprintf(stdout, "Verify_after_track_dies           : %s\n", 
	  BOOL_STR(_params.verify_after_track_dies));

  fprintf(stdout, "Force_xy_fit                      : %s\n", 
	  BOOL_STR(_params.force_xy_fit));

  print_contingency_table(stdout,
			  "Contingency data.\n----------------",
			  doVerify.getTotalCount());

  if (_params.verify_method != Params::VERIFY_MDV) {
    doVerify.computeTotalStats();
    print_stats(stdout,
                "Stats for entire analysis.\n-------------------------",
                doVerify.getTotalStats());
  }

  return 0;

}

/*****************************************************************************
 * debug printout of contingency table
 */

void VerifyTracks::print_contingency_table(FILE *fout,
                                           const char *label,
                                           const vt_count_t *count)

{
  
  double w, x, y, z;
  double pod, pod_denom;
  double far, far_denom;
  double csi, csi_denom;
  double hss, hss_denom;

  x = count->n_success;
  y = count->n_failure;
  z = count->n_false_alarm;
  w = count->n_non_event;

  pod_denom = x + y;
  far_denom = x + z;
  csi_denom = x + y + z;
  hss_denom =  (y * y) + (z * z) + (2.0 * x * w) + (y + z) * (x + w);

  if (pod_denom > 0)
    pod = x / pod_denom;
  else
    pod = 0.0;

  if (far_denom > 0)
    far = z / far_denom;
  else
    far = 0.0;

  if (csi_denom > 0)
    csi = x / csi_denom;
  else
    csi = 0.0;

  if (hss_denom > 0)
    hss = (2.0 * (x * w - y * z)) / hss_denom;
  else
    hss = 0.0;

  fprintf(fout, "\n%s\n\n", label);
  
  fprintf(fout, "n_success     : %g\n", count->n_success);
  fprintf(fout, "n_failure     : %g\n", count->n_failure);
  fprintf(fout, "n_false_alarm : %g\n", count->n_false_alarm);
  fprintf(fout, "n_non_event   : %g\n", count->n_non_event);
  
  fprintf(fout, "\n");

  fprintf(fout, "POD           : %g\n", pod);
  fprintf(fout, "FAR           : %g\n", far);
  fprintf(fout, "CSI           : %g\n", csi);
  fprintf(fout, "HSS           : %g\n", hss);
  
}

/*****************************************************************************
 * printout of stats
 *
 *****************************************************************************/

void VerifyTracks::print_stats(FILE *fout,
                               const char *heading,
                               const vt_stats_t *stats)

{

  double x_sum, x_sum_sq, x_mean, x_stdev;

  fprintf(fout, "\n%s\n\n", heading);
  fprintf(fout, "%20s: %g\n\n",
	  "nsamples_movement", stats->n_movement);
  fprintf(fout, "%20s: %g\n\n",
	  "nsamples_growth", stats->n_growth);

  fprintf(fout, "%20s  %10s %10s %10s %10s %10s\n",
	  "Variable", "Bias", "Corr", "Rmse", "Norm bias", "Norm rmse");

  fprintf(fout, "%20s  %10s %10s %10s %10s %10s\n",
	  "--------", "----", "----", "----", "---------", "---------");

  print_stat(fout, "Proj_area_centroid_x",
	     &stats->bias.proj_area_centroid_x, FALSE, stats);
  print_stat(fout, "Proj_area_centroid_y",
	     &stats->bias.proj_area_centroid_y, FALSE, stats);
  print_stat(fout, "Vol_centroid_z", &stats->bias.vol_centroid_z,
             FALSE, stats);
  print_stat(fout, "Refl_centroid_z", &stats->bias.refl_centroid_z,
             FALSE, stats);
  print_stat(fout, "Top", &stats->bias.top, FALSE, stats);
  print_stat(fout, "Speed", &stats->bias.smoothed_speed, TRUE, stats);
  print_stat(fout, "Direction", &stats->bias.smoothed_direction, FALSE, stats);
  print_stat(fout, "dBZ_max", &stats->bias.dbz_max, TRUE, stats);
  print_stat(fout, "Volume", &stats->bias.volume, TRUE, stats);
  print_stat(fout, "Precip_flux", &stats->bias.precip_flux, TRUE, stats);
  print_stat(fout, "Mass", &stats->bias.mass, TRUE, stats);
  print_stat(fout, "Proj_area", &stats->bias.proj_area, TRUE, stats);

  /* 
   * calc and print mean distance error between forecast and verify cells 
   */

  fprintf(fout, "\n%20s  %10s %10s \n",
	  "Variable", "Mean", "Stdev" );

  fprintf(fout, "%20s  %10s %10s \n",
	  "--------", "----", "-----" );

  x_sum=stats->sum_dist_error ;
  x_sum_sq=stats->sum_sq_dist_error;
  x_mean = x_sum / stats->n_movement;
  x_stdev=sqrt((x_sum_sq*stats->n_movement - x_sum*x_sum) / (stats->n_movement*(stats->n_movement-1))) ;

  fprintf(fout, "%20s: %10g %10g\n", "Delta_r",x_mean, x_stdev );

  fprintf(fout, "\n");
  fprintf(fout, "Note : bias sense is (forecast - verification).\n");
  
}

/*****************************************************************
 * print_stat()
 */

void VerifyTracks::print_stat(FILE *fout,
                              const char *label,
                              const fl32 *bias_p,
                              int print_norm,
                              const vt_stats_t *stats)

{
  
  long offset;
  fl32 bias, rmse;
  fl32 norm_bias, norm_rmse;
  fl32 corr;

  /*
   * compute the offset into the struct
   */

  offset = bias_p - (fl32 *) &stats->bias;

  bias = *((fl32 *) &stats->bias + offset);
  corr = *((fl32 *) &stats->corr + offset);
  rmse = *((fl32 *) &stats->rmse + offset);
  
  if (print_norm) {

    norm_bias = *((fl32 *) &stats->norm_bias + offset);
    norm_rmse = *((fl32 *) &stats->norm_rmse + offset);

    fprintf(fout, "%20s: %10g %10g %10g %10.2g %10.2g\n",
	    label, bias, corr, rmse, norm_bias, norm_rmse);

  } else {

    fprintf(fout, "%20s: %10g %10g %10g\n", label, bias, corr, rmse);

  }

}

/***************************************************************************
 * print_grid()
 */

void VerifyTracks::print_grid(FILE *fout,
                              const char *label,
                              ui08 **grid,
                              int nx, int ny)
  
{

  int info_found;
  long count;
  long ix, iy;

  /*
   * print header
   */

  fprintf(fout, "\n%s\n", label);

  fprintf(fout, "     ");
  count = 10;
  for (ix = 0; ix < nx; ix++) {
    if (count < 10) {
      fprintf(fout, "   ");
    } else {
      fprintf(fout, " | ");
      count = 0;
    }
    count++;
  } /* ix */

  fprintf(fout, "\n");

  /*
   * print grid
   */

  for (iy = ny - 1; iy >= 0; iy--) {

    info_found = FALSE;

    for (ix = 0; ix < nx; ix++)
      if (grid[iy][ix])
	info_found = TRUE;
    
    if (info_found) {
      
      fprintf(fout, "%4ld ", iy);
      
      for (ix = 0; ix < nx; ix++)
	fprintf(fout, "%3d", grid[iy][ix]);
      
      fprintf(fout, "\n");
      
    } /* if (info_found) */
    
  } /* iy */

}

/***************************************************************************
 * print_grid()
 */

void VerifyTracks::print_grid(FILE *fout,
                              const char *label,
                              ui08 **forecast_grid,
                              ui08 **verify_grid,
                              int nx, int ny)

{

  int info_found;
  long count;
  long ix, iy;
  long min_ix, max_ix, nx_active;

  /*
   * search for max and min x activity
   */

  min_ix = LARGE_LONG;
  max_ix = 0;
  
  for (iy = ny - 1; iy >= 0; iy--) {

    for (ix = 0; ix < nx; ix++) {

      if (verify_grid[iy][ix] ||
	  forecast_grid[iy][ix]) {
	
	min_ix = MIN(min_ix, ix);
	max_ix = MAX(max_ix, ix);
	
      } /* if */
      
    } /* ix */

  } /* iy */

  nx_active = max_ix - min_ix + 1;

  /*
   * print header
   */

  fprintf(fout, "\n%s\n", label);
  fprintf(fout, "start_ix = %ld\n", min_ix);

  fprintf(fout, "     ");
  count = nx_active % 10;
  for (ix = 0; ix < nx_active; ix++) {
    if (count < 10) {
      fprintf(fout, " ");
    } else {
      fprintf(fout, "|");
      count = 0;
    }
    count++;
  } /* ix */

  fprintf(fout, "\n");

  /*
   * print grid
   */

  for (iy = ny - 1; iy >= 0; iy--) {

    info_found = FALSE;

    for (ix = min_ix; ix <= max_ix; ix++)
      if (verify_grid[iy][ix] ||
	  forecast_grid[iy][ix])
	info_found = TRUE;

    if (info_found) {

      fprintf(fout, "%4ld ", iy);
      
      for (ix = min_ix; ix <= max_ix; ix++) {

	if (verify_grid[iy][ix] > 0 &&
	    forecast_grid[iy][ix] > 0) {

	  fprintf(fout, "S");

	} else if (verify_grid[iy][ix] > 0 &&
		   forecast_grid[iy][ix] == 0) {

	  fprintf(fout, "F");
	  
	  
	} else if (verify_grid[iy][ix] == 0 &&
		   forecast_grid[iy][ix] > 0) {

	  fprintf(fout, "A");
	  
	} else {

	  fprintf(fout, "-");
	  
	}
	
      } /* ix */

      fprintf(fout, "\n");

    } /* if (info_found) */

  } /* iy */

}

