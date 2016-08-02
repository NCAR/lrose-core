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
/**
 * @file Kernel.cc
 */
#include "Kernel.hh"
#include "KernelTemplate.hh"
#include "KernelDbzDiffFilter.hh"
#include "CloudGap.hh"
#include "KernelGrids.hh"
#include <FiltAlg/GridProj.hh>
#include <FiltAlg/Statics.hh>
#include <Spdb/DsSpdb.hh>
#include <euclid/Grid2d.hh>
#include <euclid/GridAlgs.hh>
#include <toolsa/LogStream.hh>
#include <Mdv/GenPolyGrid.hh>
#include <dataport/port_types.h>
#include <cmath>

static bool _debug = false;

/*----------------------------------------------------------------*/
// return correlation at a set of points from two grids
static double _correlation(const Grid2d &x, const Grid2d &y, 
			   const vector<pair<int,int> > &pts)
{
  double xbar=0, ybar=0, n=0;
  for (int i=0; i<(int)pts.size(); ++i)
  {
    double xi, yi;
    if (x.getValue(pts[i].first, pts[i].second, xi) &&
	y.getValue(pts[i].first, pts[i].second, yi))
    {
      xbar += xi;
      ybar += yi;
      ++n;
    }
  }
  if (n == 0)
    return 0.0;
  
  xbar /= n;
  ybar /= n;

  double num=0, denomx=0, denomy=0;
  for (int i=0; i<(int)pts.size(); ++i)
  {
    double xi, yi;
    if (x.getValue(pts[i].first, pts[i].second, xi) &&
	y.getValue(pts[i].first, pts[i].second, yi))
    {
      num += (xi-xbar)*(yi-ybar);
      denomx += (xi-xbar)*(xi-xbar);
      denomy += (yi-ybar)*(yi-ybar);
    }
  }
  if (denomx == 0 || denomy == 0)
    return 0.0;
  else
    return (num/sqrt(denomx*denomy));
}

#ifdef NOTNOW
/*----------------------------------------------------------------*/
// mean value at a set of points in a grid
static double _mean(const Grid2d &g, const vector<pair<int,int> > &pts)
{
  double ave = 0.0, n=0.0;

  for (int i=0; i<(int)pts.size(); ++i)
  {
    double v;
    if (g.get_value(pts[i].first, pts[i].second, v))
    {
      ave += v; 
      n++;
    }
  }
  if (n > 0)
    ave /= n;
  return ave;
}
#endif

/*----------------------------------------------------------------*/
// difference between min and max values
static double _diff(const Grid2d &g, const vector<pair<int,int> > &pts)
{
  double min=0, max=0;
  bool first = true;
  for (int i=0; i<(int)pts.size(); ++i)
  {
    double v;
    if (g.getValue(pts[i].first, pts[i].second, v))
    {
      if (first)
      {
	first = false;
	min = max = v;
      }
      else
      {
	if (v < min)
	  min = v;
	if (v > max)
	  max = v;
      }
    }
  }
  return max - min;
}

/*----------------------------------------------------------------*/
// mean linearized value at a set of points in a grid
static double _mean_linear(const Grid2d &g, const vector<pair<int,int> > &pts)
{
  double ave = 0.0, n=0.0;

  for (int i=0; i<(int)pts.size(); ++i)
  {
    double v;
    if (g.getValue(pts[i].first, pts[i].second, v))
    {
      double v2 = v/10;
      v2 = pow(10,v2); 
      ave += v2; 
      n++;
    }
  }
  if (n > 0)
    ave /= n;
  return ave;
}


/*----------------------------------------------------------------*/
// mean of linearized values at a set of points, put back into log space
static double _mean_linear_adjusted(const Grid2d &g,
				    const vector<pair<int,int> > &pts,
				    const bool debug=false)
{
  double ave = 0.0, n=0.0;

  for (int i=0; i<(int)pts.size(); ++i)
  {
    double v;
    if (g.getValue(pts[i].first, pts[i].second, v))
    {
      double v2 = v/10;
      v2 = pow(10,v2); 
      if (debug)
	printf("[%d,%d] v=%10.5lf  linear=%10.5lf\n", pts[i].first,
	       pts[i].second, v, v2);
      ave += v2; 
      n++;
    }
  }
  if (n > 0)
    ave /= n;

  ave = 10.0*log10(ave);
  return ave;
}


/*----------------------------------------------------------------*/
Kernel::Kernel(const double vlevel, const bool is_far, const Grid2d &mask,
	       const Grid2d &omask, const CloudGap &g, const Grid2d &clumps,
	       const Params &params, const GridProj &gp)
{
  // get centerpoint from gaps object
  _center_x = g.get_x(is_far);
  _center_y = g.get_y();

  // debugging
  _debug = false;
  double range = gp._x0 + _center_x*gp._dx;
  double az = gp._y0 + _center_y*gp._dy;
  while (az > 360.0)
    az -= 360.0;
  while (az < 0)
    az += 360.0;
  for (int i=0; i<params.kernel_debug_n; ++i)
  {
    if (_is_debug(params._kernel_debug[i], range, az, vlevel))
    {
      _debug = true;
    }
  }

  _total_pts.push_back(pair<int,int>(_center_x,_center_y));
  _pts.push_back(pair<int,int>(_center_x,_center_y));

  // vlevel passed in
  _vlevel = vlevel;

  // initialize to a dummy color
  _color = 1;

  // so far not good (hasn't been tested)
  _is_good = false;
  _pid_ok = false;
  _sdbz_ok = _sdbz_out_ok = _D0_ok = _corr_ok = false;
  _sdbz_diff_ok = _dbz_diff_ok = false;

  if (g.is_closest() && !is_far)
    // at the origin, must be _ok
    _ok = true;
  else
  {
    // not at origin, add in all the points that define the kernel
    int nx, ny;
    nx = mask.getNx();
    ny = mask.getNy();
    KernelTemplate T(nx, ny, _center_x, _center_y, is_far);
    T.add_kernel_points(mask, params.max_kernel_size, *this);
    T.add_kernel_outside_points(omask, params.max_kernel_size, *this);

    // define _ok as when the resultant set of points is big enough
    _ok = (npt() >= params.min_kernel_size &&
	   npt_outside() >= params.min_kernel_size);
  }
}

/*----------------------------------------------------------------*/
Kernel::~Kernel()
{
}

/*----------------------------------------------------------------*/
void Kernel::cp_to_grid(Grid2d &g) const
{
  double value = _color;
  cp_to_grid(g, value);
}

/*----------------------------------------------------------------*/
void Kernel::cp_to_grid(Grid2d &g, const double value) const
{
  g.setValue(_center_x, _center_y, value);
}

/*----------------------------------------------------------------*/
void Kernel::print_status(void) const
{
  string bad = " ";
  string kstat;

  if (!_is_good)
  {
    kstat = "Bad";
    bad = "(";
    if (!_pid_ok)
      bad += "PID,";
    if (!_sdbz_ok)
      bad += "SDBZ,";
    if (!_sdbz_out_ok)
      bad += "SDBZ_out,";
    if (!_sdbz_diff_ok)
      bad += "SDBZ_diff,";
    if (!_dbz_diff_ok)
      bad += "DBZ_diff,";
    if (!_D0_ok)
      bad += "D0,";
    if (!_corr_ok)
      bad += "corr";
    bad += ")";
  }
  else
    kstat = "OK ";
  printf("Kernel:v:%6.3lf id:%02d Sdbz:%6.2lf Sdbz_in:%6.2lf Sdbz_diff:%6.2lf "
	 " Ddff_nremove:%d D0:%4.2lf (ZDR_ave=%4.2lf z_ave=%8.2lf) "
	 "C:%3.2lf Stat:%s ",
	 _vlevel, _color, _sdbz_ave, _sdbz_ave_out, _sdbz_diff,
	 _dbz_diff_npt_removed, _D0, _ZDR_ave, _z_ave, _corr, kstat.c_str());
  if (!_is_good)
    printf("%s\n", bad.c_str());
  else
    printf("A:%8.2lf H:%8.2lf\n", _Ag, _q);
}

/*----------------------------------------------------------------*/
void Kernel::print(void) const
{
  printf("kernel(%d,%d) pts:\n", _center_x, _center_y);
  for (int i=0; i<(int)_pts.size(); ++i)
    printf("\t(%d,%d)\n", _pts[i].first, _pts[i].second);
  printf("outside pts:\n");
  for (int i=0; i<(int)_out_pts.size(); ++i)
    printf("\t(%d,%d)\n", _out_pts[i].first, _out_pts[i].second);
}

/*----------------------------------------------------------------*/
void Kernel::print_debug(const string dir, const int id, const double vlevel,
			 const KernelGrids &grids, const double km_per_gate)
{
  char buf[1000];
  sprintf(buf, "%s/Kernel_%.2lf_%02d", dir.c_str(), vlevel, id);
  FILE *fp = fopen(buf, "w");
  for (int i=0; i< (int)_pts.size(); ++i)
  {
    double v = (double)_pts[i].first*km_per_gate;
    fprintf(fp, "%10.5f ", v);
  }
  fprintf(fp, "\n");
  _print(fp, *grids._sdbz);
  _print(fp, grids._kdbzAdjusted);
  _print(fp, *grids._szdr);
  _print(fp, *grids._snoise);
  _print(fp, *grids._knoise);
  _print(fp, *grids._srhohv);
  fprintf(fp, "%d %10.5lf %10.5lf %10.5lf %10.5lf %10.5lf %10.5lf %10.5lf "
	  "%10.5lf %10.5lf %d %d\n", id, vlevel, _D0, _corr, _Ag, _q,
	  _x_ave()*km_per_gate, _sdbz_ave, _kdbz_ave, _sdbz_diff, 
	  _dbz_diff_npt_removed, _is_good);
  fclose(fp);
}

/*----------------------------------------------------------------*/
void Kernel::finish_processing(const time_t &time,
			       const int i, const double vlevel,
			       const KernelGrids &grids, const Params &P,
			       const double km_per_gate)
{
  // set color
  _color = i;

  // set 'good' status
  _set_good(grids, P, km_per_gate);

  // print out information to stdout and to an output file in outdir
  if (LOG_STREAM_IS_ENABLED(LogStream::DEBUG_VERBOSE))
    print_status();
}

/*----------------------------------------------------------------*/
bool Kernel::write_genpoly(const time_t &t, const int nx, const int ny,
			   const bool outside, DsSpdb &D) const
{
  if (_center_x == 0)
    return true;

  GenPolyGrid poly;
  GridAlgs gr("tmp", nx, ny, -1);
  if (outside)
    _to_grid1(gr, 1.0);
  else
    _to_grid0(gr, 1.0);
  if (!poly.setBoxes(t, t, _color, gr, Statics::_gproj))
    return false;
  poly.clearVals();
  poly.setNLevels(1);
  poly.clearFieldInfo();

  poly.addFieldInfo("i", "index");
  poly.addVal((double)_color);

  poly.addFieldInfo("D", "mm");
  poly.addVal(_D0);

  poly.addFieldInfo("c", "%");
  poly.addVal(_corr);

  poly.addFieldInfo("q", "gm-3");
  poly.addVal(_q);
  poly.assemble();
  if (D.put(SPDB_GENERIC_POLYLINE_ID,
	    SPDB_GENERIC_POLYLINE_LABEL,
	    poly.getId(), t, t,
	    poly.getBufLen(),
	    (void *)poly.getBufPtr() ))
  {
    LOG(ERROR) << "problems writing to SPDB";
    return false;
  }
  return true;
}

/*----------------------------------------------------------------*/
void Kernel::get_attenuation(int &x_ave, int &y, double &attenuation) const
{
  x_ave = _x_ave();
  y = _center_y;
  attenuation = _sdbz_ave -_kdbz_ave;
}
  
/*----------------------------------------------------------------*/
double Kernel::humidity_from_attenuation(const double a)
{
  return 201.40*a*a*a -209.60*a*a + 120.55*a - 2.25;
}

/*----------------------------------------------------------------*/
void Kernel::_set_good(const KernelGrids &grids, const Params &P,
		       const double km_per_gate)
{
  bool stat = true;

  _pid_ok = true;
  _sdbz_ok = _sdbz_out_ok = _D0_ok = _corr_ok = true;
  _sdbz_diff_ok = true;
  _Ag = 0.0;
  _q =0.0;
  _D0 = 0.0;
  if (_center_x == 0)
  {
    // fake kernel at origin
    _sdbz_diff = _sdbz_ave = _sdbz_ave_out = _kdbz_ave = _ZDR_ave = 0.0;
    _dbz_diff_npt_removed = 0;
    _z_ave = 0.0;
    _corr = 0.0;
    _is_good = true;
    return;
  }

  if (!_filter_dbz_diff(*grids._dbz_diff, P))
  {
    LOG(ERROR) << "computing dbz diff removal filter";
    stat = false;
  }
  if (!_dbz_diff_ok)
  {
    if (_debug) printf("SETTING false due to dbz diff not ok\n");
    stat = false;
  }

  _sdbz_ave = _mean_linear_adjusted(*grids._sdbz, _pts);
  _sdbz_ave_out = _mean_linear_adjusted(*grids._sdbz, _out_pts);
  _sdbz_diff = _diff(*grids._sdbz, _pts);
  _kdbz_ave = _mean_linear_adjusted(grids._kdbzAdjusted, _pts);
  _ZDR_ave = _mean_linear_adjusted(*grids._szdr, _pts);
  if (_ZDR_ave < 0.0)
  {
    if (_ZDR_ave < -0.2) // param
    {
      LOG(DEBUG_VERBOSE) << "ZDR was very negative " << _ZDR_ave;
      stat = false;
    }
    _ZDR_ave = 0.00;
  }
  _z_ave = _mean_linear(*grids._sdbz, _pts);
  _corr = _correlation(*grids._sdbz, grids._kdbzAdjusted, _pts);
  for (int i=0; i<(int)_pts.size(); ++i)
  {
    double v;
    if (grids._pid->getValue(_pts[i].first, _pts[i].second, v))
    {
      bool ok = false;
      for (int j=0; j<P.pid_weather_n; ++j)
	if (fabs(v -P._pid_weather[j]) < 0.33)
	{
	  ok = true;
	  break;
	}
      if (!ok)
      {
	_pid_ok = false;
	stat = false;
      }
    }
  }
  if (_sdbz_ave < P.min_mean_s_dbz)
  {
    stat = false;
    _sdbz_ok = false;
  }
  if (_sdbz_ave - _sdbz_ave_out < P.min_s_dbz_kernel_non_kernel_diff)
  {
    stat = false;
    _sdbz_out_ok = false;
  }
  if (_sdbz_diff > P.max_s_dbz_kernel_diff)
  {
    stat = false;
    _sdbz_diff_ok = false;
  }
  if (_z_ave != 0)
  {
    double zp = pow(_z_ave, 0.37);
    double gamma = 1.81*pow(_ZDR_ave/zp, .486);
    _D0 = gamma*pow(_z_ave, 0.136);
    if (_D0 > P.max_D0)
    {
      _D0_ok = false;
      stat = false;
    }
  }
  else
  {
    //     printf("Rejected no z_ave to compute with\n");
    stat = false;
  }
  
  if (_corr < P.min_S_K_dbz_correlation)
  {
    _corr_ok = false;  
    stat = false;
  }
  _Ag = _gaseous_attenuation(km_per_gate);
  _q = humidity_from_attenuation(_Ag);
  _is_good = stat;
}

/*----------------------------------------------------------------*/
void Kernel::_print(FILE *fp, const Grid2d &g) const
{
  for (int i=0; i< (int)_pts.size(); ++i)
  {
    double v;
    if (g.getValue(_pts[i].first, _pts[i].second, v))
      fprintf(fp, "%10.5f ", v);
    else
      fprintf(fp, "%10.5f ", -99.9999);
  }
  fprintf(fp, "\n");
}

/*----------------------------------------------------------------*/
int Kernel::_x_ave(void) const
{
  if (_pts.size()== 0)
    return 0;

  double xave = 0;
  for (int i=0; i<(int)_pts.size(); ++i)
    xave += _pts[i].first;
  xave /= (double)_pts.size();
  return (int)xave;
}

/*----------------------------------------------------------------*/
void Kernel::_to_grid0(Grid2d &g, const double value) const
{
  for (int i=0; i<(int)_pts.size(); ++i)
    g.setValue(_pts[i].first, _pts[i].second, value);
}

/*----------------------------------------------------------------*/
void Kernel::_to_grid1(Grid2d &g, const double value) const
{
  for (int i=0; i<(int)_out_pts.size(); ++i)
    g.setValue(_out_pts[i].first, _out_pts[i].second, value);
}

/*----------------------------------------------------------------*/
double Kernel::_gaseous_attenuation(const double km_per_gate) const
{
  return (_sdbz_ave -_kdbz_ave)/(2*_x_ave()*km_per_gate);
}

/*----------------------------------------------------------------*/
bool Kernel::_filter_dbz_diff(const Grid2d &dbz_diff, const Params &P)
{
  // keep filtering out more points until either things are good
  // or hopelessly bad
  while (_filter_dbz_diff_1(dbz_diff, P))
    ;

  // figure out whether the kernel should be removed (not enough points left)
  _dbz_diff_ok = (int)_pts.size() >= P.min_kernel_size;
  _dbz_diff_npt_removed = (int)(_total_pts.size() - _pts.size());
  if (_debug)
    printf("ok:%d  nptremove:%d\n", _dbz_diff_ok, _dbz_diff_npt_removed);
  return true;
}

/*----------------------------------------------------------------*/
bool Kernel::_filter_dbz_diff_1(const Grid2d &dbz_diff, const Params &P)
{
  // let this object do all the work:
  KernelDbzDiffFilter d(_debug);

  if (_debug)
    printf("Filtering dbz diff, dbz diff values:\n");
  // accumulate values into the object
  for (int ii=0; ii<(int)_pts.size(); ++ii)
  {
    double v;
    if (dbz_diff.getValue(_pts[ii].first, _pts[ii].second, v))
    {
      if (_debug) printf("%5.3f ", v);
      d.inc(v, ii);
    }
  }
  if (_debug) printf("\n");

  // finish up and if no more fitering, return false
  if (d.finish(P.dbz_diff_threshold))
    return false;

  // get the index to remove and remove it.
  int index = d.choose_remove_index();
  _pts.erase(_pts.begin()+index);

  // return true if the kernel is still big enough to keep.
  return ((int) _pts.size() >= P.min_kernel_size);
}  

/*----------------------------------------------------------------*/
bool Kernel::_is_debug(const Params::mask_t &mask, const double range,
		       const double az, const double vlevel) const
{
  if (range < mask.range0 || range > mask.range1)
    return false;
  if (vlevel < mask.elevation0 || vlevel > mask.elevation1)
    return false;
  return (az >= mask.azimuth0 && az <= mask.azimuth1);
}
