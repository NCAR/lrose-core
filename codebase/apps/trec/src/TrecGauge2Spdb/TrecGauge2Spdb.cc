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
//
// TrecGauge2Spdb object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// October 1997
//
///////////////////////////////////////////////////////////////

#include <toolsa/pmu.h>
#include <toolsa/str.h>
#include <toolsa/toolsa_macros.h>
#include <rapformats/trec_gauge.h>
#include <rapformats/station_reports.h>
#include <Spdb/DsSpdb.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxProj.hh>
#include "TrecGauge2Spdb.hh"
using namespace std;

// Constructor

TrecGauge2Spdb::TrecGauge2Spdb(int argc, char **argv)

{

  OK = TRUE;
  
  // set programe name

  _progName = "TrecGauge2Spdb";
  ucopyright(_progName.c_str());
  
  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    fprintf(stderr, "ERROR: %s\n", _progName.c_str());
    fprintf(stderr, "Problem with command line args\n");
    OK = FALSE;
    return;
  }

  // get TDRP params
  
  char *_paramsPath = "unknown";
  if (_params.loadFromArgs(argc, argv,
			   _args.override.list,
			   &_paramsPath)) {
    fprintf(stderr, "ERROR: %s\n", _progName.c_str());
    fprintf(stderr, "Problem with TDRP parameters\n");
    OK = FALSE;
    return;
  }

  // check the params
  
  if (_params.mode == Params::ARCHIVE) {
    if (_args.startTime < 0 || _args.endTime < 0) {
      fprintf(stderr, "ERROR - %s\n", _progName.c_str());
      fprintf(stderr, "In ARCHIVE mode, you must set start and end times\n");
      OK = FALSE;
      return;
    }
  }

  // input file object

  if (_params.mode == Params::ARCHIVE) {
    _mdvInput.setArchive(_params.trec_input_url,
			 _args.startTime,
			 _args.endTime);
  } else {
    _mdvInput.setRealtime(_params.trec_input_url,
			  _params.max_realtime_valid_age,
			  PMU_auto_register,
			  1000);
  }

  // initialize process registration
  
  PMU_auto_init(_progName.c_str(),
		_params.instance, PROCMAP_REGISTER_INTERVAL);
  
}

//////////////
// destructor

TrecGauge2Spdb::~TrecGauge2Spdb()

{

  // unregister process

  PMU_auto_unregister();

}

//////
// Run

int TrecGauge2Spdb::Run ()
{
  
  if (_params.debug == Params::DEBUG_LOG) {
    fprintf(stdout, "ZrCalib\n");
    _params.print(stdout, PRINT_SHORT);
  }

  time_t inputTime;
  
  while (_mdvInput.getNext(inputTime) == 0) {

    if (_params.debug) {
      cerr << "Processing TREC data for time: "
	   << DateTime::str(inputTime) << endl;
    }

    // read in MDV

    DsMdvx mdvx;
    mdvx.setReadTime(Mdvx::READ_CLOSEST,
		     _params.trec_input_url,
		     300, inputTime);
    mdvx.addReadField(_params.dbz_label);
    mdvx.addReadField(_params.u_label);
    mdvx.addReadField(_params.v_label);
    mdvx.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
    mdvx.setReadCompressionType(Mdvx::COMPRESSION_NONE);

    if (mdvx.readVolume()) {
      cerr << "ERROR - TrecGauge2Spdb::Run" << endl;
      cerr << mdvx.getErrStr() << endl;
      continue;
    }

    if (mdvx.getNFields() != 3) {
      cerr << "ERROR - TrecGauge2Spdb::Run" << endl;
      cerr << "MDV file at time " << DateTime::str(inputTime)
	   << " does not have dbz, u and v.";
      cerr << "Requested fields are:" << endl;
      cerr << "  dbz name: " << _params.dbz_label << endl;
      cerr << "  u name: " << _params.u_label << endl;
      cerr << "  v name: " << _params.v_label << endl;
      continue;
    }

    for (int i = 0; i < _params.gauges_n; i++) {
      _processGauge(mdvx, _params._gauges[i]);
    }
    
  } // while

  return (0);

}

///////////////////////////////////////
// process the gauge for this MDV file

int TrecGauge2Spdb::_processGauge(const DsMdvx &mdvx,
				  const Params::gauge_t &gauge)

{

  time_t trecTime = mdvx.getMasterHeader().time_centroid;

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    fprintf(stderr, "Time %s, Gauge %s, lat %g, lon %g\n",
	    utimstr(trecTime), gauge.name, gauge.lat, gauge.lon);
  }
  
  // load up gauge info known at this stage

  trec_gauge_handle_t tgauge;
  trec_gauge_init(&tgauge);
  trec_gauge_alloc(&tgauge, _params.n_forecasts);
  
  STRncopy(tgauge.hdr->name, gauge.name, TREC_GAUGE_NAME_LEN);
  tgauge.hdr->lat = gauge.lat;
  tgauge.hdr->lon = gauge.lon;
  tgauge.hdr->forecast_delta_time = _params.forecast_delta_time;
  tgauge.hdr->n_forecasts = _params.n_forecasts;
  tgauge.hdr->trec_time = trecTime;
			  
  // compute the gauge location in (x, y)

  MdvxProj proj(mdvx);
  double gauge_x, gauge_y;
  proj.latlon2xy(gauge.lat, gauge.lon, gauge_x, gauge_y);

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    fprintf(stderr, "Gauge XY: %.3f, %.3f\n", gauge_x, gauge_y);
  }

  // compute the u and v field means over the gauge

  double u_mean, v_mean;
  double u_gauge, v_gauge, wt_gauge;
  _computeMeanVel(trecTime, mdvx, gauge,
		  gauge_x, gauge_y, u_mean, v_mean,
		  u_gauge, v_gauge, wt_gauge);
  
  tgauge.hdr->u = (fl32) u_mean;
  tgauge.hdr->v = (fl32) v_mean;
  tgauge.hdr->u_surface = (fl32) u_gauge;
  tgauge.hdr->v_surface = (fl32) v_gauge;
  tgauge.hdr->wt_surface = (fl32) wt_gauge;

  // compute mean dbz at forecast times, so the (x, y) sampling
  // location moves upstream of the gauge

  for (int itime = 0; itime < tgauge.hdr->n_forecasts; itime++) {
    
    double lead_time = itime * tgauge.hdr->forecast_delta_time;

    // compute (x, y) point upstream of gauge
    
    double xx = gauge_x - (u_mean * lead_time) / 1000.0; // km
    double yy = gauge_y - (v_mean * lead_time) / 1000.0; // km
    
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      fprintf(stderr, "     XX,YY: %.3f, %.3f\n",xx, yy);
    }

    // sample reflectivity at upstream point
    
    double dbz_median;
    _computeKernelDbzMean(mdvx, gauge, xx, yy, dbz_median);

    // add dbz value to handle

    tgauge.dbz[itime] = (fl32) dbz_median;
    if (_params.debug >= Params::DEBUG_VERBOSE ) {
      fprintf(stderr, "itime, dbz_median: %d, %g\n", itime, dbz_median);
    }

  }

  // swap to BE
  
  trec_gauge_to_BE(&tgauge);

  // write SPDB output

  DsSpdb spdb;
  time_t expire_time = trecTime + _params.product_valid_period;
  if (spdb.put(_params.output_url,
	       SPDB_TREC_GAUGE_ID,
	       SPDB_TREC_GAUGE_LABEL,
	       Spdb::hash4CharsToInt32(gauge.name),
	       trecTime, expire_time,
	       MEMbufLen(tgauge.mbuf),
	       MEMbufPtr(tgauge.mbuf))) {
    cerr << "ERROR - TrecGauge2Spdb::_processGauge" << endl;
    cerr << spdb.getErrStr() << endl;
    trec_gauge_free(&tgauge);
    return -1;
  }
  
  return 0;

}

/////////////////////////////////////////////////////////////////////
// Compute mean velocity vector. This is an iterative procedure.
// We start with the (u,v) at the gauge, and then iteratively adjust
// this by considering the (u,v values at the mid and start points
// of the vector to get a mean (u,v). 

int TrecGauge2Spdb::_computeMeanVel(time_t trec_time,
				    const DsMdvx &mdvx,
				    const Params::gauge_t &gauge,
				    double xx, double yy,
				    double &u_mean, double &v_mean,
				    double &u_gauge, double &v_gauge,
				    double &wt_gauge)
  
{
  
  if (_params.motion == Params::ZERO_MOTION) {
    u_mean = 0.0;
    v_mean = 0.0;
    u_gauge = 0.0;
    v_gauge = 0.0;
    wt_gauge = 0.0;
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      fprintf(stderr, "  Setting u and v to zero\n");
    }
    return 0;
  }

  wt_gauge = gauge.gauge_weight;
  
  bool use_time_av_wind = true;
  if (!_params.motion == Params::MEAN_WIND_MOTION) {
    use_time_av_wind = false;
  }
  if (_computeTimeAvGaugeWind(trec_time, gauge,
			      u_gauge, v_gauge)) {
    use_time_av_wind = false;
  }
  
  // get the averaged trec motion
  
  double trec_u_mean, trec_v_mean;
  if (_params.trec_motion_method == Params::SPATIAL_AVERAGE) {
    if (_spaceAvTrecMotion(trec_time, mdvx, gauge, xx, yy,
			   trec_u_mean, trec_v_mean)) {
      return -1;
    }
  } else {
    if (_timeAvTrecMotion(trec_time, mdvx, gauge, xx, yy,
			  trec_u_mean, trec_v_mean)) {
      return -1;
    }
  }
  
  // set return values
  
  if (use_time_av_wind) {
    u_mean = (trec_u_mean * gauge.trec_weight +
	      u_gauge * gauge.gauge_weight);
    v_mean = (trec_v_mean * gauge.trec_weight +
	      v_gauge * gauge.gauge_weight);
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      fprintf(stderr, "  gauge u, v = %10.2f, %10.2f\n",
	      u_gauge, v_gauge);
      fprintf(stderr,
	      "  wt-averaged u_mean, v_mean = %10.2f, %10.2f\n",
	      u_mean, v_mean);
    }
  } else {
    u_mean = trec_u_mean;
    v_mean = trec_v_mean;
  }

  return 0;

}

/////////////////////////////////////////////////////////////////////
// Compute mean velocity vector, averaging in space at the current time.
// This is an iterative procedure.
// We start with the (u,v) at the gauge, and then iteratively adjust
// this by considering the (u,v values at the mid and start points
// of the vector to get a mean (u,v). 

#define MAX_ITERATIONS 10

int TrecGauge2Spdb::_spaceAvTrecMotion(time_t trec_time,
				       const DsMdvx &mdvx,
				       const Params::gauge_t &gauge,
				       double xx, double yy,
				       double &trec_u_mean,
				       double &trec_v_mean)
  
{
  
  // compute the u and v field means over the gauge
  
  double u_start, v_start;
  if (_computeKernelVelMean(mdvx, gauge, _params.u_label,
			    xx, yy, u_start)) {
    return -1;
  }
  if (_computeKernelVelMean(mdvx, gauge, _params.v_label,
			    xx, yy, v_start)) {
    return -1;
  }
  trec_u_mean = u_start;
  trec_v_mean = v_start;
  
  // iterate to compute the mean u & v, using the end and mid points
  // the vector to compute the mean values.

  double lead_time = _params.forecast_delta_time * _params.n_forecasts;


  double prev_trec_u_mean;
  double prev_trec_v_mean;
  int convergence = FALSE;

  for (int i = 0; i < MAX_ITERATIONS; i++) {

    // initialize prev values

    prev_trec_u_mean = trec_u_mean;
    prev_trec_v_mean = trec_v_mean;

    // compute mid and start point locations
    
    double dx = (trec_u_mean * lead_time) / 1000.0; // km
    double dy = (trec_v_mean * lead_time) / 1000.0; // km

    double mid_x = xx - dx;
    double mid_y = yy - dy;

    double start_x = xx - dx / 2.0;
    double start_y = yy - dy / 2.0;

    // compute (u, v) at mid and start points

    double u_mid, v_mid;
    if (_computeKernelVelMean(mdvx, gauge, _params.u_label,
			      mid_x, mid_y, u_mid)) {
      return -1;
    }
    if (_computeKernelVelMean(mdvx, gauge, _params.v_label,
			      mid_x, mid_y, v_mid)) {
      return -1;
    }

    double u_start, v_start;
    if (_computeKernelVelMean(mdvx, gauge, _params.u_label,
			      start_x, start_y, u_start)) {
      return -1;
    }
    if (_computeKernelVelMean(mdvx, gauge, _params.v_label,
			      start_x, start_y, v_start)) {
      return -1;
    }

    // compute mean (u,v)

    trec_u_mean = (u_start + u_mid + u_start) / 3.0;
    trec_v_mean = (v_start + v_mid + v_start) / 3.0;
    
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      fprintf(stderr, "---> Iteration %d\n", i);
      fprintf(stderr, "u_start, u_mid, u_start, trec_u_mean: "
	      "%10.2f, %10.2f, %10.2f, %10.2f\n",
	      u_start, u_mid, u_start, trec_u_mean);
      fprintf(stderr, "v_start, v_mid, v_start, trec_v_mean: "
	      "%10.2f, %10.2f, %10.2f, %10.2f\n",
	      v_start, v_mid, v_start, trec_v_mean);
    }
      
    // check for convergence

    double u_diff = fabs(trec_u_mean - prev_trec_u_mean);
    double v_diff = fabs(trec_v_mean - prev_trec_v_mean);

    if (u_diff < 0.01 && v_diff < 0.1) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	fprintf(stderr, "====> CONVERGENCE: %d iterations\n", i);
      }
      convergence = TRUE;
      break;
    }

  } // i

  // if no convergence, the values are probably oscillating between two
  // states. Compute the mean

  if (!convergence) {
    trec_u_mean = (trec_u_mean + prev_trec_u_mean) / 2.0;
    trec_v_mean = (trec_v_mean + prev_trec_v_mean) / 2.0;
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      fprintf(stderr, "+++++>> NO CONVERGENCE: trec_u_mean, trec_v_mean = "
	      "%10.2f, %10.2f\n", trec_u_mean, trec_v_mean);
    }
  }
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    fprintf(stderr, "  trec trec_u_mean, trec_v_mean = %10.2f, %10.2f\n",
	    trec_u_mean, trec_v_mean);
  }
  
  return 0;

}

/////////////////////////////////////////////////////////////////////
// Compute mean velocity vector, averaging in time.

int TrecGauge2Spdb::_timeAvTrecMotion(time_t trec_time,
				      const DsMdvx &mdvx,
				      const Params::gauge_t &gauge,
				      double xx, double yy,
				      double &trec_u_mean,
				      double &trec_v_mean)
  
{
  
  // compute the u and v field means over the gauge point
  
  double uu, vv;
  if (_computeKernelVelMean(mdvx, gauge, _params.u_label,
			    xx, yy, uu)) {
    return -1;
  }
  if (_computeKernelVelMean(mdvx, gauge, _params.v_label,
			    xx, yy, vv)) {
    return -1;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "---->> Temporal smoothing" << endl;
    cerr << "  time, uu, vv: "
	 << DateTime::str(trec_time) << ", "
	 << uu << ", " << vv << endl;
  }

  // initialize for averaging

  double sum_uu = uu;
  double sum_vv = vv;
  double nn = 1.0;

  // get time list

  time_t start_time = trec_time - _params.trec_motion_averaging_period;
  time_t end_time = trec_time - 1;
  DsMdvx pastMdvx;
  pastMdvx.setTimeListModeValid(_params.trec_input_url,
				start_time, end_time);
  if (pastMdvx.compileTimeList()) {
    cerr << pastMdvx.getErrStr();
    return -1;
  }
  vector<time_t> tlist = pastMdvx.getTimeList();

  // loop through available input times

  for (size_t ii = 0; ii < tlist.size(); ii++) {

    // read in MDV
    
    pastMdvx.setReadTime(Mdvx::READ_CLOSEST,
			 _params.trec_input_url,
			 300, tlist[ii]);
    pastMdvx.addReadField(_params.u_label);
    pastMdvx.addReadField(_params.v_label);
    pastMdvx.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
    pastMdvx.setReadCompressionType(Mdvx::COMPRESSION_NONE);
    if (pastMdvx.readVolume()) {
      cerr << "ERROR - TrecGauge2Spdb::_timeAvTrecMotion" << endl;
      cerr << pastMdvx.getErrStr() << endl;
      continue;
    }
    
    if (_computeKernelVelMean(pastMdvx, gauge, _params.u_label,
			      xx, yy, uu)) {
      continue;
    }
    if (_computeKernelVelMean(pastMdvx, gauge, _params.v_label,
			      xx, yy, vv)) {
      continue;
    }

    sum_uu += uu;
    sum_vv += vv;
    nn++;

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "  time, uu, vv: "
	   << DateTime::str(tlist[ii]) << ", "
	   << uu << ", " << vv << endl;
    }

  } // ii

  // compute mean

  trec_u_mean = sum_uu / nn;
  trec_v_mean = sum_vv / nn;
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "  time_averaged uu, vv: "
	 << trec_u_mean << ", " << trec_v_mean << endl;
  }

  return 0;

}

/************************************************************************
 * Comparison funct for sorting array of doubles
 */
static int comp_func(const void *e1,const void *e2)
{
	return (int) (*(double*)e1 - *(double*)e2);
}

//////////////////////////////////////////////////////////////
// compute mean reflectivity at a point upstream of the gauge

int TrecGauge2Spdb::_computeKernelDbzMean(const DsMdvx &mdvx,
					  const Params::gauge_t &gauge,
					  double xx, double yy,
					  double &dbz_median)
  
{

  // get MDV field

  const MdvxField *fld = mdvx.getField(_params.dbz_label);
  if (fld == NULL) {
    cerr << "ERROR - TrecGauge2Spdb::_computeKernelDbzMean" << endl;
    cerr << "  MDV file does not contain field: "
	 << _params.dbz_label << endl;
    return -1;
  }
  const Mdvx::field_header_t &fhdr = fld->getFieldHeader();

  // get kernel limits
  
  int start_ix, start_iy;
  int end_ix, end_iy;
  
  _computeKernelLimits(mdvx, _params.dbz_label,
		       gauge.dbz_kernel_size,
		       xx, yy,
		       start_ix, start_iy, end_ix, end_iy);

  
  if (_params.debug  >= Params::DEBUG_VERBOSE) {
      fprintf(stderr, "Looking at grid X: %d to %d, Y: %d to %d\n",
			  start_ix,end_ix, start_iy, end_iy);
  }

  int npoints = 0;
  fl32 *dbz_array = (fl32 *) fld->getVol();
  fl32 *zp;
  fl32 missing = fhdr.missing_data_value;

  //
  // Find the median of the data in the area.
  //
  //

  int array_size = (end_ix - start_ix +1) *  (end_iy - start_iy +1);

  // Allocate working space for sorting.
  double *vals = (double *) ucalloc(array_size, sizeof(double));
  if(vals == NULL) {
	  perror("TrecGauge2Spdb::_computeKernelDbzMean");
	  cerr << "ERROR: Couldn't calloc " << array_size << " doubles" << endl;
	  exit(-1);
  }


  // Fill the sorting array with the proper data - Count the valid points.
  int num_missing = 0;
  double average = 0.0;
  double max_value = -9999.0;
  for (int iy = start_iy; iy <= end_iy; iy++) {

    zp = dbz_array + iy * fhdr.nx + start_ix;

    for (int ix = start_ix; ix <= end_ix; ix++, zp++) {
      
      if (*zp != missing) {
        vals[npoints] = *zp;
	average += *zp; // Build sum
	npoints++;
	if(*zp > max_value) max_value = *zp;
      } else {
	num_missing++;
	if(_params.treat_missing_as_low) {  // All points are included
            vals[npoints] =  _params.missing_dbz_value;
	    average += _params.missing_dbz_value; // Build sum
	    npoints++;
	}
      }

    } // ix

  } // iy

  average /= npoints;

  if (_params.debug  >= Params::DEBUG_VERBOSE) {
      fprintf(stderr, "Found %d valid, %d missing, average: %.3f, Max: %.3f\n",
		npoints,num_missing,average,max_value);
  }

  if (npoints > 0) {
    // Qsort  the array of valid values.
    qsort(vals,npoints,sizeof(double),comp_func);
	// pick the median -  low side if balanced. 
	int target_index = ((int) ((npoints / 2.0) + 0.5)) -1;
    dbz_median = vals[target_index];
  } else {
    dbz_median = -30.0;
  }

  if(vals != NULL) {
      free(vals);
	  vals = NULL;
  }

  return 0;
  
}

//////////////////////////////////////////////////////////////
// compute mean velocity over a point

int TrecGauge2Spdb::_computeKernelVelMean(const DsMdvx &mdvx,
					  const Params::gauge_t &gauge,
					  const char *vel_label,
					  double xx, double yy,
					  double &vel_mean)
  
{

  // get MDV field

  const MdvxField *fld = mdvx.getField(vel_label);
  if (fld == NULL) {
    cerr << "ERROR - TrecGauge2Spdb::_computeKernelDbzMean" << endl;
    cerr << "  MDV file does not contain field: "
	 << vel_label << endl;
    return -1;
  }
  const Mdvx::field_header_t &fhdr = fld->getFieldHeader();

  // get kernel limits

  int start_ix, start_iy;
  int end_ix, end_iy;
  
  _computeKernelLimits(mdvx, vel_label,
		       gauge.vel_kernel_size,
		       xx, yy,
		       start_ix, start_iy, end_ix, end_iy);

  // sum up velocity data, compute the mean
  
  double sum_vel = 0.0;
  double npoints = 0.0;
  fl32 *vel_array = (fl32 *) fld->getVol();
  fl32 *vp;
  fl32 missing = fhdr.missing_data_value;
  
  for (int iy = start_iy; iy <= end_iy; iy++) {
    
    vp = vel_array + iy * fhdr.nx + start_ix;

    for (int ix = start_ix; ix <= end_ix; ix++, vp++) {

      if (*vp != missing) {
	double vel = *vp;
	sum_vel += vel;
	npoints++;
      }

    } // ix

  } // iy

  if (npoints > 0) {
    vel_mean = sum_vel / npoints;
  } else {
    vel_mean = 0.0;
  }
  
  return 0;

}

//////////////////////////////////////////////////////////////
// compute time-averaged gauge wind

int TrecGauge2Spdb::_computeTimeAvGaugeWind(time_t trec_time,
					    const Params::gauge_t &gauge,
					    double &time_av_gauge_u,
					    double &time_av_gauge_v)

{

  // compute start and end time
  
  time_t end_time = trec_time;
  time_t start_time = end_time - _params.gauge_wind_averaging_period;
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    fprintf(stderr, "===> compute_gauge_wind <====\n");
    fprintf(stderr, "  start_time: %s\n", utimstr(start_time));
    fprintf(stderr, "  end_time: %s\n", utimstr(end_time));
  }
  
  // get gauge data for time interval

  DsSpdb spdb;
  if (spdb.getInterval(_params.physical_gauge_input_url,
		       start_time, end_time,
		       Spdb::hash4CharsToInt32(gauge.name))) {
    cerr << "ERROR - TrecGauge2Spdb::_computeTimeAvGaugeWind" << endl;
    cerr << spdb.getErrStr() << endl;
    return -1;
  }
		       
  // get station report vector
  
  const vector<Spdb::chunk_t> &chunks = spdb.getChunks();
  vector<station_report_t> reports;
  
  for (size_t ichunk = 0; ichunk < chunks.size(); ichunk++) {
    station_report_t report;
    if (chunks[ichunk].len == sizeof(report)) {
      report = *((station_report_t *) chunks[ichunk].data);
      station_report_from_be(&report);
      reports.push_back(report);
    }
  }

  if (reports.size() == 0) {
    return -1;
  }

  // sum the u and v components

  double sum_u = 0.0;
  double sum_v = 0.0;
  double count = 0.0;

  for (size_t i = 0; i < reports.size(); i++) {
    if (reports[i].windspd != STATION_NAN &&
	reports[i].winddir != STATION_NAN) {
      double u, v;
      double dirn = reports[i].winddir + 180.0;
      double speed = reports[i].windspd;
      u = speed * sin(dirn * DEG_TO_RAD);
      v = speed * cos(dirn * DEG_TO_RAD);
      sum_u += u;
      sum_v += v;
      count++;
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	fprintf(stderr,
		"  time, speed, dirn, u, v, sum_u, sum_v, count: "
		"%s, %g, %g, %g, %g %g, %g, %g\n",
		utimstr(reports[i].time), reports[i].windspd,
		reports[i].winddir, u, v, sum_u, sum_v, count);
      }
    } // if (reports[i].windspd != STATION_NAN ...
  } // i

  if (count < 1) {
    return -1;
  }

  time_av_gauge_u = sum_u / count;
  time_av_gauge_v = sum_v / count;

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    fprintf(stderr,
	    "time_av_gauge_u, time_av_gauge_v: %g, %g\n",
	    time_av_gauge_u, time_av_gauge_v);
  }

  return 0;

}
///////////////////////////////////
// compute kernel limits in x and y
//

int TrecGauge2Spdb::_computeKernelLimits(const DsMdvx &mdvx,
					 const char *field_label,
					 double kernel_size,
					 double xx, double yy,
					 int &start_ix,
					 int &start_iy,
					 int &end_ix,
					 int &end_iy)

{

  const MdvxField *fld = mdvx.getField(field_label);
  if (fld == NULL) {
    cerr << "ERROR - TrecGauge2Spdb::_computeKernelLimits" << endl;
    cerr << "  MDV file does not contain field: " << field_label << endl;
    return -1;
  }
  const Mdvx::field_header_t &fhdr = fld->getFieldHeader();
  double half_kernel = kernel_size / 2.0;
  
  // compute kernel limits in x

  start_ix =
    (int) (((xx - half_kernel) - fhdr.grid_minx) / fhdr.grid_dx) + 1;

  if (start_ix < 0) {
    start_ix = 0;
  }

  end_ix =
    (int) (((xx + half_kernel) - fhdr.grid_minx) / fhdr.grid_dx);

  if (end_ix > fhdr.nx - 1) {
    end_ix = fhdr.nx - 1;
  }

  // compute kernel limits in y

  start_iy =
    (int) (((yy - half_kernel) - fhdr.grid_miny) / fhdr.grid_dy) + 1;

  if (start_iy < 0) {
    start_iy = 0;
  }

  end_iy =
    (int) (((yy + half_kernel) - fhdr.grid_miny) / fhdr.grid_dy);

  if (end_iy > fhdr.ny - 1) {
    end_iy = fhdr.ny - 1;
  }

  return 0;

}
