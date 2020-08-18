/**
 * @file LineDetect.cc
 */
#include "LineDetect.hh"
#include "LineDetectOffsets.hh"
#include "Bad.hh"
#include <euclid/Grid2dOffset.hh>
#include <stdio.h>
#include <math.h>

using std::vector;

/*----------------------------------------------------------------*/
LineDetect::LineDetect()
{
  _neighbor = NULL;
  _max_neighbor_offset = 0;
}

/*----------------------------------------------------------------*/
LineDetect::~LineDetect()
{
  if (_neighbor != NULL)
    delete _neighbor;
}

/*----------------------------------------------------------------*/
// init for particular pnew, nx
void LineDetect::init(const Parms &pnew, const int nx, const double missing,
		      bool allow_missing_side)
{
   _p = pnew;
   _allow_missing_side = allow_missing_side;
  if (_neighbor == NULL)
  {
    // make a 3x3 (hardwire) neighborhood.
    _neighbor = new Grid2dOffset(3.0, 0.0, nx, missing);
    _max_neighbor_offset = _neighbor->maxOffset();
  }
}

/*----------------------------------------------------------------*/
/*
  This function performs an preliminary convergence line test
  on pixel (i, j). It checks the average intensity in a 
  L_CENTER_WIN * L_CENTER_WIN neighborhood of the pixel. If
  the intensity is not within the convergence line intensity
  range, the function assume there is no conv. line and returns
  FAILURE. Otherwise it returns SUCCESS.
*/
bool LineDetect::ok(const int x, const int y, const int nx, const int ny,
		    const Grid2d &input) const
{
  double m;
  double v;
    
  if (_neighbor == NULL)
    // definitely not ok.
    return false;
    
  if (_neighbor->outOfRange(x, y, nx, ny))
    return false;

  if (!_allow_missing_side)
  {
    if (_neighbor->numNonFlagged(input, x, y) <= 6)
      return false;
  }
    
  if (_neighbor->average(input, x,  y, m))
  {
    v = _p._lineDetectCenter.apply(m);
    return (v > 0.0);
  }
  else
    return false;
}

/*----------------------------------------------------------------*/
// return pointer to the offset that best fits the data.
bool LineDetect::best_fit(const Grid2d &input, const double missing,
			  const int x, const int y, const int nx,
			  const int ny, const LineDetectOffsets &o,
			  double &det) const //, double &dir) const
{
  int k;
  double max=0.0, v;
  bool first, left_missing, right_missing, lmiss0, rmiss0;
  const LineDetectOffset *best, *oi;
    
  // get non-fuzzy value for each angle and maximize
  first = true;
  best = NULL;
  left_missing = right_missing = false;
  for (k=0; k< o.num(); ++k)
  {
    if ((oi = o.ith_offset(k)) == NULL)
      continue;
    if (oi->out_of_range(x, y, nx, ny))
      continue;
    v = _line_detect(*oi, input, x, y, lmiss0, rmiss0);
    if (v == badvalue::INTEREST)
      continue;
    if (first)
    {
      first = false;
      max = v;
      best = oi;
      left_missing = lmiss0;
      right_missing = rmiss0;
    }
    else
    {
      if (max < v)
      {
	max = v;
	best = oi;
	left_missing = lmiss0;
	right_missing = rmiss0;
      }
    }
  }
  if (first)
    return false;

  /*
   * get fuzzy value at best
   */ 
  v = _fuzzy_line_detect(*best, input, x, y, missing,
			 left_missing, right_missing);
  if (v == badvalue::INTEREST)
    return false;
    
  det = v;
  // dir = best->get_angle();
  return true;
}

/*----------------------------------------------------------------*/
// return pointer to the offset that best fits the data.
bool LineDetect::best_direction(const Grid2d &input, const double missing,
				const int x, const int y, const int nx,
				const int ny, const LineDetectOffsets &o,
				double &dir) const //, double &dir) const
{
  int k;
  double max=0.0, v;
  bool first;
  bool lmiss0;
  bool rmiss0;
  const LineDetectOffset *best, *oi;
    
  // get non-fuzzy value for each angle and maximize
  first = true;
  best = NULL;
  for (k=0; k< o.num(); ++k)
  {
    if ((oi = o.ith_offset(k)) == NULL)
      continue;
    if (oi->out_of_range(x, y, nx, ny))
      continue;
    v = _line_detect(*oi, input, x, y, lmiss0, rmiss0);
    if (v == badvalue::INTEREST)
      continue;
    if (first)
    {
      first = false;
      max = v;
      best = oi;
    }
    else
    {
      if (max < v)
      {
	max = v;
	best = oi;
      }
    }
  }
  if (first)
    return false;

  // /*
  //  * get fuzzy value at best
  //  */ 
  // v = _fuzzy_line_detect(*best, input, x, y, missing,
  // 			 left_missing, right_missing);
  // if (v == badvalue::INTEREST)
  //   return false;
  // det = v;
  dir = best->get_angle();
  return true;
}

/*----------------------------------------------------------------*/
// return pointer to the offset that best fits the data.
bool LineDetect::valueAtDirection(const Grid2d &input, double dir,
				  const double missing,
				  const int x, const int y, const int nx,
				  const int ny, const LineDetectOffsets &o,
				  double &det) const
{
  double v;
  const LineDetectOffset *best = o.matchingDirection(dir);
  if (best == NULL)
  {
    return false;
  }

  // // get non-fuzzy value for each angle and maximize
  // first = true;
  // best = NULL;
  // left_missing = right_missing = false;
  // for (k=0; k< o.num(); ++k)
  // {
  //   if ((oi = o.ith_offset(k)) == NULL)
  //     continue;
  //   if (oi->out_of_range(x, y, nx, ny))
  //     continue;
  //   v = _line_detect(*oi, input, x, y, lmiss0, rmiss0);
  //   if (v == badvalue::INTEREST)
  //     continue;
  //   if (first)
  //   {
  //     first = false;
  //     max = v;
  //     best = oi;
  //     left_missing = lmiss0;
  //     right_missing = rmiss0;
  //   }
  //   else
  //   {
  //     if (max < v)
  //     {
  // 	max = v;
  // 	best = oi;
  // 	left_missing = lmiss0;
  // 	right_missing = rmiss0;
  //     }
  //   }
  // }
  // if (first)
  //   return false;

  /*
   * get fuzzy value at best
   */ 
  bool lmiss, rmiss;
  _line_detect(*best, input, x, y, lmiss, rmiss);
  v = _fuzzy_line_detect(*best, input, x, y, missing,
			 lmiss, rmiss);
  if (v == badvalue::INTEREST)
    return false;
    
  det = v;
  // dir = best->get_angle();
  return true;
}

/*----------------------------------------------------------------*/
double LineDetect::_line_detect(const LineDetectOffset &l, 
				const Grid2d &input,
				const int x, const int y, 
				bool &left_missing,
				bool &right_missing) const
{
  double cv, lv, rv;
  const Grid2dOffset *o;
    
  // if more than half the center values are flagged, give up and return flag
  o = l.get_center();
  if (o->percentIsBad(0.5, input, x, y))
    return badvalue::INTEREST;

  // get mean of center values.
  if (!o->average(input, x, y, cv))
    return badvalue::INTEREST;

  // get mean of side values offset by 100, including 0 added in for
  // missing values. this allows missing values on one or both sides of
  // the center.
  double offset = 100.0;
  o = l.get_left();
  if ((lv = _mean_side(offset, *o, input, x, y,
		       left_missing)) == badvalue::INTEREST)
    return lv;

  o = l.get_right();
  if ((rv = _mean_side(offset, *o, input, x,  y, right_missing))
      == badvalue::INTEREST)
    return rv;
  return ((cv+offset) - (lv + rv)/2.0);
}

/*----------------------------------------------------------------*/
double LineDetect::_fuzzy_line_detect(const LineDetectOffset &l, 
				      const Grid2d &input,
				      const int x, const int y, 
				      const double missing,
				      const bool left_missing,
				      const bool right_missing) const
{
  int ccount, lcount, rcount;
  double mean, meansq, fc, fl, fr, f, v, tc, sc, ws;
  vector<double> pixc, pixl, pixr;
  const Grid2dOffset *o;
    
  // for each center value, apply the fuzzy function to it and sum.
  // get mean of center values and meansq of center values.
  o = l.get_center();
  pixc = o->fillPixels(input, x, y);
  _map_center(pixc, missing, ccount, mean, meansq, fc);
    
  // for the side values, 
  o = l.get_leftd();
  pixl = o->fillPixels(input, x, y);
  o = l.get_rightd();
  pixr = o->fillPixels(input, x, y);

  // add in side fuzzy measures of line detection (offsets from mean):
  _map_side(pixl, missing, mean, left_missing, lcount, fl);
  _map_side(pixr, missing, mean, right_missing, rcount, fr);
    
  tc = (double)(ccount + lcount + rcount);
  sc = (double)(lcount + rcount);
  // normalize here to not overweight side values.
  if (fabs(sc) < 1.0e-10 || fabs(tc) < 1.0e-10)
  {
    return badvalue::INTEREST;
  }

  ws = (double)ccount/sc; 
  f = (fc + ws*(fl + fr))/(double)tc;
  if (f > 0.0)
  {
    v = fabs(meansq - mean*mean); 
    v = sqrt(v);
    f *= _p._lineDetectStd.apply(v);
  }
  return f;
}

/*----------------------------------------------------------------*/
// return mean of offset values from side template.
// if too many missing, or allow missing but not quite enough are
// missing yet significanltly many are missing, return badvalue::INTEREST.
double LineDetect::_mean_side(const double offset, 
			      const Grid2dOffset &o,
			      const Grid2d &input,
			      const int x, const int y,
			      bool &is_missing) const
{
  double v;

  is_missing = false;
  if (_allow_missing_side)
  {
    if (o.percentIsBad(0.7, input, x, y))
    {
      // almost all the data is bad..this is our good missing case.
      if (!o.average(input, x, y, v))
	v = 0.0;
      is_missing = true;
    }
    else if (o.percentIsBad(0.5, input, x, y))
      // too much is still bad, but not enough is bad to replace
      // the bad stuff with a low value...this is the case in which
      // we give up.
      v = badvalue::INTEREST;
    else
    {
      // enough is good to use just the good stuff.
      if (!o.average(input, x, y, v))
	v = badvalue::INTEREST;
      else
	v += offset;
    }
  }
  else
  {
    if (o.percentIsBad(0.5, input, x, y))
      // if half or more is missing, we give up.
      v= badvalue::INTEREST;
    else
    {
      if (!o.average(input, x, y, v))
	v = badvalue::INTEREST;
      else
	v += offset;
    }
  }
  return v;
}

/*----------------------------------------------------------------*/
/*
 * Dave:  Build up mean, variance, and mapping value
 * from cpix to ctbl value (which gives high values
 * to cpix data between min_online and max_online and
 * 0 to values outside of abs_min_online or abs_max_online
 */
void LineDetect::_map_center(const vector<double> &cpix, const double missing,
			     int &count, double &mean, double &mean_sq,
			     double &fuzzyv) const
{
  double tmp;
  vector<double>::const_iterator it;
  
  count = 0;
  fuzzyv = mean = mean_sq = 0.0;
  for (it=cpix.begin(); it != cpix.end(); ++it)
  {
    tmp = *it;
    if ((int)tmp != missing)
    {
      (count)++;
      mean += (double)tmp;
      mean_sq += (double)(tmp*tmp);
      fuzzyv += _p._lineDetectCenter.apply((double)tmp);
    }
  }

  mean /= (double)(count);
  mean_sq /= (double)(count);
}

/*----------------------------------------------------------------*/
void LineDetect::_map_side(const vector<double> &pix, const double missing,
			   const double mean_center,
			   const bool is_missing, int &count,
			   double &fuzzyv) const
{
  double tmp;
  double diff;
  vector<double>::const_iterator it;
    
  count = 0;
  fuzzyv = 0.0;
  for (it=pix.begin(); it != pix.end(); ++it)
  {
    tmp = *it;
    if (tmp == missing)
    {
      if (is_missing)
	tmp = 0;
      else
	continue;
    }
	
    diff = mean_center - tmp;
    (count)++;
    fuzzyv += _p._lineDetectSide.apply(diff);
  }
}

