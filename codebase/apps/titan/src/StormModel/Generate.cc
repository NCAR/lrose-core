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
// Generate.cc
//
// Generate object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// February 1998
//
///////////////////////////////////////////////////////////////

#include "Generate.hh"
#include <toolsa/umisc.h>
#include <toolsa/str.h>
#include <rapmath/stats.h>
#include <ctime>
using namespace std;

static double _Dm, _A, _aThresh;

// Constructor

Generate::Generate(const char *prog_name, const Params &params) :
        _params(params)

{

  OK = TRUE;
  _progName = STRdup(prog_name);
  _startTime = NULL;
  _midPoint = NULL;
  _duration = NULL;
  _area = NULL;
  _velocity = NULL;
  _aThresh = _params.area_threshold;
  _sumArea = 0.0;

  // initialize stat distribution generation

  if (_params.random_seed < 0) {
    time_t now = time((time_t *) NULL);
    STATS_uniform_seed(now);
  } else {
    STATS_uniform_seed(_params.random_seed);
  }

  // create StartTime object

  _startTime = new StartTime(_progName, _params);
  if (!_startTime->OK) {
    OK = FALSE;
    return;
  }

  // create MidPoint object

  _midPoint = new MidPoint(_progName, _params);
  if (!_midPoint->OK) {
    OK = FALSE;
    return;
  }

  // create Duration object

  _duration = new Duration(_progName, _params);
  if (!_duration->OK) {
    OK = FALSE;
    return;
  }

  // create Area object

  _area = new Area(_progName, _params);
  if (!_area->OK) {
    OK = FALSE;
    return;
  }

  // create Velocity object

  _velocity = new Velocity(_progName, _params);
  if (!_velocity->OK) {
    OK = FALSE;
    return;
  }

  return;

}

// destructor

Generate::~Generate()

{

  if (_startTime) {
    delete (_startTime);
  }
  if (_midPoint) {
    delete (_midPoint);
  }
  if (_duration) {
    delete (_duration);
  }
  if (_area) {
    delete (_area);
  }
  if (_velocity) {
    delete (_velocity);
  }
  STRfree(_progName);

}

//////////////////
// printHeader()
//

void Generate::printHeader(FILE *out)

{

  fprintf(out, "#Program %s\n", _progName);

  date_time_t file_time;
  ulocaltime(&file_time);
  fprintf(out, "#File create time: %s\n", utimestr(&file_time));
  fprintf(out, "#Min duration (secs): %g\n", _params.min_duration);
  
  fprintf(out,
	  "#labels: %s\n",
	  "Count,"
	  "Unixtime,"
	  "Nscans,"
	  "Duration(hr),"
	  "GaussD(hr),"
	  "Gauss_Amean(km2),"
	  "dbzThresh,"
	  "dbzFitMean,"
	  "dbzFitMin,"
	  "dbzFitMax,"
	  "startX(km),"
	  "startY(km),"
	  "meanU(km/h),"
	  "meanV(km/h),"
	  "Speed(km/hr),"
	  "Dirn(degT),"
	  "ellipseRatio,"
	  "ellipseOrient(degT),"
	  );

  fprintf(out, "#\n");

}

//////////////////////////////////////////////////
// Another() - generate another storm

int Generate::Another(int num, FILE *out)
{

  // generate storm start time, convert to secs

  double start_time = _startTime->Generate();
  double start_secs = start_time * 3600.0;

  // start point

  double midx, midy;
  int mid_point_index;
  _midPoint->Generate(&mid_point_index, &midx, &midy);

  int nscans;
  double duration = -1.0;
  int count = 0;

  while (duration < _params.min_duration ||
	 duration > _params.max_duration) {

    count++;
    if (count > 50) {
      return (-1);
    }

    // generate Duration Measure Dm

    _Dm = _duration->Generate(mid_point_index);
    
    // Generate mean area A
    
    _A = _area->Generate(mid_point_index, _Dm);
    
    // compute duration
    
    duration = rtbis(_durDiff, _params.min_duration, 24.0, 0.001);

  } // while (duration < _params.min_duration) 

  nscans = (int) ((duration * 3600.0) / _params.scan_interval + 0.5);
  _sumArea += _A * nscans; 

  // Generate dBZmax

  double dBZmax_mean, dBZmax_sdev;
  double dBZmax, dBZmin, dBZmean;
  double dBZthresh;

  dBZthresh = _params.dbz_threshold;
  _interp_dBZmax(_Dm, &dBZmax_mean, &dBZmax_sdev);
  dBZmax = STATS_normal_gen(dBZmax_mean, dBZmax_sdev);
  dBZmin = dBZthresh + 10.0;
  dBZmean = (dBZmax + dBZmin) / 2.0;

  // velocity

  double u, v;
  _velocity->Generate(mid_point_index, &u, &v);
  double speed, dirn;
  speed = sqrt(u * u + v * v);
  if (u == 0.0 && v == 0.0) {
    dirn = 0.0;
  } else {
    dirn = atan2(u, v) * RAD_TO_DEG;
  }

  // start point

  double startx = midx - (u * duration) / 2.0;
  double starty = midy - (v * duration) / 2.0;

  // shape

  double ellipse_ratio =
    exp(STATS_normal_gen(_params.ln_ellipse_ratio_norm.mean,
			 _params.ln_ellipse_ratio_norm.sdev));

  double ellipse_orientation =
    STATS_normal_gen(_params.ellipse_orientation_norm.mean,
		     _params.ellipse_orientation_norm.sdev);
  
  // output

  fprintf(out, "%d %.0f %d %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g\n",
	  num, start_secs, nscans, duration, _Dm, _A, dBZthresh,
	  dBZmean, dBZmin, dBZmax,
	  startx, starty, u, v, speed, dirn,
	  ellipse_ratio, ellipse_orientation);

  return (0);

}

//////////////////////////////////////////////////
// _interp_dBZmax()
//
// Interpolate for dBZmax from Dm
//
// Sets mean and sdev of dBZmax for given Dm.

void Generate::_interp_dBZmax(double Dm, double *mean_p, double *sdev_p)

{

  Params::dBZmax_vs_Dm_t *dBZmax = _params._dBZmax_vs_Dm;
  int npts = _params.dBZmax_vs_Dm_n;

  if (Dm < dBZmax[0].Dm) {

    // Dm below range - use lowest value

    *mean_p = dBZmax[0].mean;
    *sdev_p = dBZmax[0].sdev;
    return;
  }

  if (Dm > dBZmax[npts-1].Dm) {

    // Dm above range - use highest value

    *mean_p = dBZmax[npts-1].mean;
    *sdev_p = dBZmax[npts-1].sdev;
    return;
  }

  // Dm in range - interpolate

  for (int i = 1; i < npts; i++) {
    if ((Dm >= dBZmax[i-1].Dm) && (Dm <= dBZmax[i].Dm)) {
      double frac = (Dm - dBZmax[i-1].Dm) / (dBZmax[i].Dm - dBZmax[i-1].Dm);
      *mean_p = dBZmax[i-1].mean + frac * (dBZmax[i].mean - dBZmax[i-1].mean);
      *sdev_p = dBZmax[i-1].sdev + frac * (dBZmax[i].sdev - dBZmax[i-1].sdev);
      return;
    }
  }

  return;

}

/////////////
// _durDiff()
//
// Function for rtbis.
//
// Function which returns diff between guessed duration and dur computed
// with the guess. It returns 0 at root of the duration equation, which
// is the equation to compute duration given A and Dm
//

double Generate::_durDiff(double dur_guess)
 
{

  
  double term1 = -2.0 * _Dm * _Dm;
  double root2pi = sqrt(2.0 * M_PI);
  double term2 = log(root2pi * _Dm * _aThresh / (_A * dur_guess));
  double product = term1 * term2;

  double dur;

  if (product >= 0) {
    dur = 2.0 * sqrt(product);
  } else {
    dur = 0.0;
  }

  return (dur_guess - dur);

}

/////////////////////////////////////////
// rtbis - bisection root-finding method
// Recipes in C, p262
//

#define JMAX 1000

double Generate::rtbis(double (*func)(double),
		       double x1, double x2, double xacc)

{

  int j;
  double dx,f,fmid,xmid,rtb;

  f=(*func)(x1);
  fmid=(*func)(x2);
  if (f*fmid >= 0.0) {
//     fprintf(stderr, "Root must be bracketed for bisection in RTBIS\n");
//     fprintf(stderr, "x1, x2, acc: %g, %g, %g\n", x1, x2, xacc);
//     fprintf(stderr, "f, fmid: %g, %g\n", f, fmid);
    return (-999.0);
  }
  rtb = f < 0.0 ? (dx=x2-x1,x1) : (dx=x1-x2,x2);
  for (j=1;j<=JMAX;j++) {
    fmid=(*func)(xmid=rtb+(dx *= 0.5));
    if (fmid <= 0.0) rtb=xmid;
    if (fabs(dx) < xacc || fmid == 0.0) return (rtb);
  }
//   fprintf(stderr,
// 	  "Too many bisections in RTBIS, Dm, A: %g, %g\n", _Dm, _A);
  return (-999.0);
}

#undef JMAX
