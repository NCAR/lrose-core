/**
 * @file EnhancementD.cc
 */

#include "EnhancementD.hh"
#include "ParmEnhance.hh"
#include <cstdio>
#include <euclid/Grid2dOffset.hh>

/*----------------------------------------------------------------*/
EnhancementD::EnhancementD(const ParmEnhance &p)
{
  _f = p._enhanceF;
  if (!_f.maxXAtGivenY(1.0, _min_diff))
  {
    printf("ERROR setting min difference value in enhancedment\n");
    _min_diff = 0.0;
  }
  _allow_missing_side = p._allow_missing_side;
}

/*----------------------------------------------------------------*/
EnhancementD::~EnhancementD()
{
}

/*----------------------------------------------------------------*/
// return best fuzzy fit of data to templates
bool EnhancementD::best_fit(const Grid2d &g, int x, int y,
			    int nx, int ny, const EnhancementOffsetsD &o,
			    double &det, double &ang) const
{
  const EnhancementOffsetD *best, *oi;
  double v, max;
  bool first;
  int k;
    
  // get the best non-fuzzy measure of enhancement.
  max = -1000000.0;
  best = NULL;
  first = true;
  for (k=0; k<o.num(); ++k)
  {
    if ((oi=o.ith_offset(k)) == NULL)
      continue;
    if (oi->out_of_range(x, y, nx, ny))
      continue;
    if (!_enhancement(*oi, g, x, y, v))
      continue;
    if (first)
    {
      max = v;
      first = false;
      best = oi;
    }
    else
    {
      if (v > max)
      {
	max = v;
	best = oi;
      }
    }
  }
  if (first)
    return false;
    
  if (!_fuzzy_enhancement(*best, g, x, y, max, ang, v))
    return false;
  else
  {
    det = v;
    return true;
  }
}

/*----------------------------------------------------------------*/
// non-fuzzy enhancement (difference between center and side) value.
bool EnhancementD::_enhancement(const EnhancementOffsetD &o,
				const Grid2d &input, const int x,
				const int y, double &v) const
{
  const Grid2dOffset *oc, *ol, *ior;
  double mc, ms;
    
  // if too many of the centerpoints are missing, no enhancement.
  oc = o.get_center();
  if (oc->percentBad(input, x, y) > 0.5)
    return false;
  if (!oc->average(input, x, y, mc))
    return false;

  // get side offset tables.
  ol = o.get_left();
  ior = o.get_right();
  if (!_allow_missing_side)
  {
    if (ol->percentBad(input,x,y) > 0.0 ||
	ior->percentBad(input,x,y) > 0.0)
      return false;
  }

  // an average side offset.
  double la, ra;
  la = ol->averageMissingZero(input,x,y);
  ra = ior->averageMissingZero(input, x, y);
  ms = (la + ra)/2;

  // the difference of the averages, center to side.
  v = mc - ms;
  return true;
}

/*----------------------------------------------------------------*/
// fuzzy value for enhancement:
bool EnhancementD::_fuzzy_enhancement(const EnhancementOffsetD &o,
				      const Grid2d &input, const int x,
				      const int y,
				      double non_fuzzy_enhancement,
				      double &angle, double &v) const
{
  double vin;
  if (!input.getValue(x, y, vin))
    return false;

  double maxside;

  // find max value on either side
  maxside = o.max_side(input, x, y);
  angle = o.get_angle();

  if ((vin - maxside)*2 <= _min_diff) 
    // If center value doesn't exceed biggest side value by too much  
    // simply use the non-fuzzy enhancement value. assume not
    // near the end of a line.
    v = non_fuzzy_enhancement;
  else
    // center value is extra big..possibly near the end of a line.
    // get diff from the larger half of centerpoints
    // (to avoid erosion of the line ends.)
    if (!_half_center_enhancement(o, input, x, y, v))
      return false;

  v = _f.apply(v); // result is in [0,1]
  return true;
}

/*----------------------------------------------------------------*/
// Break center into two halves and get average diff between the
// half of centerpoints with bigger value and the average side value.
bool 
EnhancementD::_half_center_enhancement(const EnhancementOffsetD &o,
				       const Grid2d &input, const int x,
				       const int y, double &v) const
{
  double v0, v1, ms;
  const Grid2dOffset *oc, *ol, *ior;
    
  oc = o.get_center();
  ol = o.get_left();
  ior = o.get_right();

  // use half of center with biggest average value
  v0 = oc->halfAverageMissingZero(input, x, y, true);
  v1 = oc->halfAverageMissingZero(input, x, y, false);
  if (v1 > v0)
    v0 = v1;

  // side average:
  double al, ar;
  al = ol->averageMissingZero(input, x, y);
  ar = ior->averageMissingZero(input, x,  y);
  ms = (al + ar)/2.0;

  // enhancement value = differernce.
  v = v0 - ms;
  return true;
}

