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
 * @file HumidityAlg.cc
 */
#include "HumidityAlg.hh"
#include "CloudGaps.hh"
#include "CloudGap.hh"
#include "Kernels.hh"
#include "ClumpAssociate.hh"
#include "Params.hh"
#include <FiltAlg/FiltInfoInput.hh>
#include <FiltAlg/VlevelSlice.hh>
#include <toolsa/LogStream.hh>
#include <cmath>

/*----------------------------------------------------------------*/
static bool _in_mask(const double vlevel, const double r, const double a,
		     const Params::mask_t &mask)
{
  if (vlevel < mask.elevation0 || vlevel > mask.elevation1)
    return false;
  if (r < mask.range0 || r > mask.range1)
    return false;
  return (a >= mask.azimuth0 && a <= mask.azimuth1);
}

/*----------------------------------------------------------------*/
HumidityAlg::HumidityAlg(const FiltInfoInput &inp)
{
  // set the grid dimensions
  _nx = inp.getSlice()->getNx();
  _ny = inp.getSlice()->getNy();
  _vlevel = inp.getVlevel();
  _vlevel_index = inp.getVlevelIndex();
  _gp = inp.getGridProj();
}

/*----------------------------------------------------------------*/
HumidityAlg::~HumidityAlg()
{
}

/*----------------------------------------------------------------*/
Grid2d HumidityAlg::add_bias(const Grid2d &g, const double offset) const
{
  LOG(DEBUG) << "adding bias " << offset;
  GridAlgs ret(g);
  ret.add(offset);
  return ret;
}

/*----------------------------------------------------------------*/
void HumidityAlg::diff_grid(const Grid2d &g1, const Grid2d &g2,
			    Grid2d &d) const
{
  LOG(DEBUG) << " ";
  GridAlgs a = GridAlgs::promote(g2);
  a.multiply(-1.0);
  a.add(g1);
  d = a;
}

/*----------------------------------------------------------------*/
Grid2d HumidityAlg::clumping(const Grid2d &pid, const Grid2d &snoise,
			      const Grid2d &knoise, const Params &params,
			      Grid2d &clump, Grid2d &clump_nosmall)
{
  LOG(DEBUG) << "original";

  // filter to only points with weather:
  Grid2d ret("weather", _nx, _ny, 0);

  // build up weather points 
  for (int y=0; y<_ny; ++y)
    for (int x=0; x<_nx; ++x)
    {
      if (_is_weather(x, y, pid, snoise, knoise, params))
	ret.setValue(x, y, 1.0);
    }
  
  // compute clumps. (assign a color to each weather point)
  _filter_clump(ret);
  clump = ret;

  // filter out all small clumps. 
  _filter_small_clumps(ret, params.clump_min_npt);
  clump_nosmall = ret;

  return ret;
}

/*----------------------------------------------------------------*/
Grid2d HumidityAlg::clumping(const Grid2d &pid, const Params &params,
			      Grid2d &clump, Grid2d &clump_nosmall)
{
  LOG(DEBUG) << "PID";

  Grid2d ret("pid_clump", _nx, _ny, 0);

  int min_gate = 0;

  // build up weather points 
  for (int y=0; y<_ny; ++y)
    for (int x=min_gate; x<_nx; ++x)
    {
      if (_is_pid_clump(x, y, pid, params))
	ret.setValue(x, y, 1.0);
    }
  
  // compute clumps. (assign a color to each weather point)
  _filter_pid_clump(ret);
  clump = ret;

  // filter out all small clumps. 
  _filter_small_pid_clumps(ret, params.clump_min_npt);
  clump_nosmall = ret;

  return ret;
}

/*----------------------------------------------------------------*/
ClumpAssociate HumidityAlg::associate_clumps(const Grid2d &clumps,
					     const Grid2d &pid_clumps)
{
  return ClumpAssociate(_r, clumps, pid_clumps);
}

/*----------------------------------------------------------------*/
CloudGaps HumidityAlg::build_gaps(const Grid2d &clumps,
				  const Grid2d &pid_clumps,
				  ClumpAssociate &ca,
				  const Params &params, Grid2d &edge,
				  Grid2d &outside, Grid2d &fedge,
				  Grid2d &foutside) const
{
  LOG(DEBUG) << " ";

  CloudGaps ret;

  // add in gaps along each beam
  for (int y=0; y<_ny; ++y)
    ret.add_gaps(y, _nx, clumps, params.leading_edge_depth);
  edge = ret.edge(_nx, _ny);
  outside = ret.outside(_nx, _ny);
  
  // filter keeping only gaps that are big enough.
  int min_gridpt = (int)(params.min_km/_gp._dx);
  ret.filter(min_gridpt);

  // filter keeping only clumps not too far inside pid clumps
  int max_gates = (int)(params.max_km_penetration/_gp._dx);
  ret.filter(pid_clumps, ca, max_gates);

  fedge = ret.edge(_nx, _ny);
  foutside = ret.outside(_nx, _ny);
  return ret;
}

/*----------------------------------------------------------------*/
Kernels HumidityAlg::kernel_build(const time_t &time, const Grid2d &clumps,
				  const CloudGaps &gaps,
				  const KernelGrids &grids,
				  const Params &params, Grid2d &kernel_cp) const
{
  LOG(DEBUG) << " ";

  // create kernel center point grid to write to
  kernel_cp = Grid2d("kernel_centerpt", _nx, _ny, -1);

  Kernels vk;

  // build a mask that is non-missing only when not in a clump.
  Grid2d omask = _build_outside_mask(clumps);

  // for each gap between clouds try to build a kernel pair and add it when
  // can do so.
  for (int i=0; i<gaps.num(); ++i)
  {
    const CloudGap &g = gaps.ith_cloudgap(i);
    // build a mask for the 'far' edge points and 'near' edge ponts
    Grid2d mask_far, mask_near;
    if (!_kernel_mask(g, clumps, true, mask_far))
      continue;
    if (!_kernel_mask(g, clumps, false, mask_near))
      continue;

    // build KernelPair for this gap.
    KernelPair kp(_vlevel, mask_far, mask_near, omask, g, clumps, params, _gp);
    if (!kp.is_ok())
      continue;
    // add it to the kernels to be returned
    vk.add(kp, time, _vlevel, grids, params, _gp._dx, kernel_cp);
  }

  return vk;
}

/*----------------------------------------------------------------*/
void HumidityAlg::kernel_output(const Kernels &vk,
				Grid2d &attenuation, Grid2d &humidity,
				std::string &ascii_output_string)
{
  LOG(DEBUG) << " ";

  // build the two grids in which to put attenuation and humidity  values
  attenuation = Grid2d("attenuation", _nx, _ny, -1);
  humidity = Grid2d("Humidity", _nx, _ny, -1);

  // get the humidity estimates in two ways:
  // 1. fill in the att/hum grids
  // 2. create a string with lines where each line is a humidity estimate.
  ascii_output_string = vk.humidity_estimate(_vlevel, _gp, attenuation,
					     humidity);
}

/*----------------------------------------------------------------*/
Kernels HumidityAlg::kernel_filter(const Kernels &vk,
				   Grid2d &fkernel_cp) const
{
  LOG(DEBUG) << " ";

  // create a grid to put kernel centerpoints to
  fkernel_cp = Grid2d("filtered_kernel_centerpt", _nx, _ny, -1);

  // filter down to only the 'good' kernels'
  Kernels knew;
  for (int i=0; i<(int)vk.num(); ++i)
  {
    const KernelPair &ki = vk.ith_kernel_pair(i);
    if (ki.is_good())
    {
      // store centerpoints to grid
      ki.cp_to_grid(fkernel_cp);
      // keep it
      knew.append(ki);
    }
  }

  return knew;
}

/*----------------------------------------------------------------*/
bool HumidityAlg::_is_weather(const int x,  const int y, const Grid2d &pid,
			      const Grid2d &snoise, const Grid2d &knoise,
			      const Params &params) const
{
  // is the point in one of the masks?
  double r = _gp._x0 + _gp._dx*x;
  double a = _gp._y0 + _gp._dy*y;
  while (a < 0)
    a += 360.0;
  while (a >= 360.0)
    a -= 360.0;
  for (int i=0; i<params.mask_n; ++i)
  {
    if (_in_mask(_vlevel, r, a, params._mask[i]))
      return false;
  }

  // the test is:  pid not equal to the 'nonweather' values and
  // sn >= thresh AND kn >= thresh
  double v;
  if (!pid.getValue(x, y, v))
    return false;
  for (int i=0; i<params.pid_non_weather_n; ++i)
  {
    if (fabs(v - params._pid_non_weather[i]) < 0.33)
      return false;
  }

  if (!snoise.getValue(x, y, v))
    return false;
  if ((double)v < params.s_noise_thresh)
    return false;

  if (!knoise.getValue(x, y, v))
    return false;
  if ((double)v < params.k_noise_thresh)
    return false;
  return true;
}

/*----------------------------------------------------------------*/
bool HumidityAlg::_is_pid_clump(const int x,  const int y, const Grid2d &pid,
				const Params &params) const
{
  // is the point in one of the masks?
  double r = _gp._x0 + _gp._dx*x;
  double a = _gp._y0 + _gp._dy*y;
  while (a < 0)
    a += 360.0;
  while (a >= 360.0)
    a -= 360.0;
  for (int i=0; i<params.mask_n; ++i)
  {
    if (_in_mask(_vlevel, r, a, params._mask[i]))
      return false;
  }

  // the test is:  pid equal to a set of values.
  double v;
  if (!pid.getValue(x, y, v))
    return false;
  for (int i=0; i<params.pid_clump_n; ++i)
  {
    if (fabs(v - params._pid_clump[i]) < 0.33)
      return true;
  }
  return false;
}

/*----------------------------------------------------------------*/
// compute clumps. (assign a color to each weather point)
void HumidityAlg::_filter_clump(Grid2d &g)
{
  // use Clump object to define the clumps
  Grid2dClump clump(g);
  _r = clump.buildRegions();

  // reuse g (write the clumps into it.)
  g.setAllMissing();
  for (int i=0; i<(int)_r.size(); ++i)
  {
    double color = HumidityAlg::index_to_color(i);
    for (int j=0; j<(int)_r[i].size(); ++j)
      g.setValue(_r[i][j].first, _r[i][j].second, color);
  }
}

/*----------------------------------------------------------------*/
// compute clumps. (assign a color to each weather point)
void HumidityAlg::_filter_pid_clump(Grid2d &g)
{
  // use Clump object to define the clumps
  Grid2dClump clump(g);
  _pid_r = clump.buildRegions();

  // reuse g (write the clumps into it.)
  g.setAllMissing();
  for (int i=0; i<(int)_pid_r.size(); ++i)
  {
    double color = HumidityAlg::index_to_color(i);
    for (int j=0; j<(int)_pid_r[i].size(); ++j)
      g.setValue(_pid_r[i][j].first, _pid_r[i][j].second, color);
  }
}

/*----------------------------------------------------------------*/
// filter out all small clumps. 
void HumidityAlg::_filter_small_clumps(Grid2d &c, const int minpt) const
{
  vector<clump::Region_t>::const_iterator i;
  for (i=_r.begin(); i!=_r.end(); ++i)
  {
    if ((int)i->size() < minpt)
    {
      clump::Region_citer_t j;
      for (j=i->begin(); j!=i->end(); ++j)
	c.setMissing(j->first, j->second);
    }
  }
}

/*----------------------------------------------------------------*/
// filter out all small clumps. 
void HumidityAlg::_filter_small_pid_clumps(Grid2d &c, const int minpt) const
{
  vector<clump::Region_t>::const_iterator i;
  for (i=_pid_r.begin(); i!=_pid_r.end(); ++i)
  {
    bool good = true;
    if ((int)i->size() < minpt)
    {
      good = false;
    }
    else
    {
      // remove those that span only one azimuth
      clump::Region_citer_t j;
      int y0 = -1;
      bool multi = false;
      for (j=i->begin(); j!=i->end(); ++j)
      {
	if (j == i->begin())
	  y0 = j->second;
	else
	{
	  if (y0 != j->second)
	  {
	    multi = true;
	    break;
	  }
	}
      }      
      if (!multi)
	good = false;
    }
    if (!good)
    {
      clump::Region_citer_t j;
      for (j=i->begin(); j!=i->end(); ++j)
	c.setMissing(j->first, j->second);
    }
  }
}

/*----------------------------------------------------------------*/
// build a mask that is non-missing only when not in a clump.
Grid2d HumidityAlg::_build_outside_mask(const Grid2d &clump) const
{
  Grid2d omask(clump);
  omask.setAllToValue(1.0);
  for (int i=0; i<(int)_r.size(); ++i)
  {
    for (int j=0; j<(int)_r[i].size(); ++j)
    {
      int xi, yi;
      xi = _r[i][j].first;
      yi = _r[i][j].second;
      omask.setMissing(xi, yi);
    }
  }
  return omask;
}

/*----------------------------------------------------------------*/
// build a mask for the kernel points associated with a gap
bool HumidityAlg::_kernel_mask(const CloudGap &gap, const Grid2d &clump,
			       const bool is_far, Grid2d &mask) const
{
  // initialize the mask to all missing
  mask = Grid2d(clump);
  mask.setAllMissing();

  if (gap.is_closest() && !is_far)
  {
    // no actual kernel points at origin
    return true;
  }
  
  // pull out the color
  double color = gap.get_color(is_far);

  // and use color to get index into _r vector.
  int index = HumidityAlg::color_to_index(color);
  if (index < 0 || index >= (int)_r.size())
  {
    LOG(ERROR) << "index " << index << " out of range [0," << 
	 (int)_r.size()-1;
    return false;
  }

  const clump::Region_t *ri = &_r[index];

  // make a mask with the appropriate region points
  for (int i=0; i<(int)ri->size(); ++i)
  {
    int xi, yi;
    xi = (*ri)[i].first;
    yi = (*ri)[i].second;
    mask.setValue(xi, yi, 1.0);
  }
  return true;
}


