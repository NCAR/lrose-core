/**
 * @file ShearDetect.cc
 * shear detection at a point
 */

#include <stdio.h>
#include <math.h>

#include "ShearDetect.hh"

/*----------------------------------------------------------------*/
static inline double  _angle_from_radar(int x, int y, int ny,
					const MdvxProj &p)
{
  Mdvx::coord_t coord = p.getCoord();
  

    double betad;
    double xx, yy;

    /*
      * get x,y offsets from current loc to radar, and
      * convert to angle using arctangent function.
      * scale into degrees.
      *
      * xradar/yradar units are pixels from origin (southwest corner)
      */

    double xradar = (-coord.minx/coord.dx);
    double yradar = (-coord.miny/coord.dy);

    yy = y - yradar;
    xx = x - xradar;

    // y is in graphics coordinates, so need to convert to math
    // prior to getting offset to radar. x is ok as is.
//     yy = (double)(ny - y - 1 - _sparm_generic.__yradar);
//     xx = (double)(x - _sparm_generic.__xradar);
    if (xx == 0. && yy == 0.)
      betad = 0.;
    else
      betad = atan2 (yy, xx) * 180. / 3.141592653589;
    while (betad < 0.)
        betad += 360.;
    return betad;
}

/*----------------------------------------------------------------*/
static inline bool
_radar_points_left_to_right(double beta,   //radar pointing dir.
			    double right,  //right pointing template dir
			    double cut_angle) // parameter
{
    double alfa;
  
    // get approximate diff between right and radar pointing dir.
    alfa = right - beta;
    alfa = fabs(alfa);
    
    // if this diff is small or near 360, radar is to the left of the template,
    // i.e. left_to_right is true.
    return (alfa <= 90.0 - cut_angle || alfa >= 270.0 + cut_angle);
}

/*----------------------------------------------------------------*/
static inline bool
_radar_points_right_to_left(double beta,   //radar pointing dir.
			    double right,  //right pointing template dir
			    double cut_angle) // parameter
{
    double alfa;
  
    // get approximate diff between right and radar pointing dir.
    alfa = right - beta;
    alfa = fabs(alfa);
    
    // if this diff is near 180, the radar is to the right of the
    // template, pointing right to left.
    return (alfa >= 90.0 + cut_angle && alfa <= 270.0 - cut_angle);
}

/*----------------------------------------------------------------*/
/*----------------------------------------------------------------*/
/*----------------------------------------------------------------*/
// default constructor
ShearDetect::ShearDetect()
{
  __is_secondary = false;
  __neighbor = NULL;
  _test_circle_diameter = 0;
}

/*----------------------------------------------------------------*/
// default destructor
ShearDetect::~ShearDetect()
{
    if (__neighbor != NULL)
        delete __neighbor;
}

/*----------------------------------------------------------------*/
void ShearDetect::init(const Parms &parm, const MdvxProj &proj,
		       int nx, double missing,
		       double test_circle_diameter)
{
    __p = parm;
    _proj = proj;
    __is_secondary = true;
    if (_test_circle_diameter != test_circle_diameter || __neighbor == NULL)
    {
      _test_circle_diameter = test_circle_diameter;
      // redo neighbor computation
      _compute_neighbor(nx, missing);
    }
}

/********************************************************************

	This function performs a preliminary test for shear line
	detection at pixel (i, j). It checks the maximum intensity on a 
	circle centered at the pixel. If the intensity is not within
	the specified range, the function assumes that there is no
	convergence line and returns FAILURE. Otherwise it returns 
	SUCCESS.
*/
bool ShearDetect::secondary_ok(int x, int y, int nx, int ny, 
			       const Grid2d &secondary,
			       const double *secondaryRange) const
{
    
    /*
     * if not enough room to do complete lookup, give up now
     */
    if (__neighbor->outOfRange(x, y, nx, ny))
        return false;
    
    // abort if more than half the secondary points are bad.
    if (__neighbor->percentIsBad(0.5, secondary, x, y))
        return false;
    
    if (!__is_secondary)
        // no more testing.
        return true;

    // if max secondary value in nbhd is too big or small, abort.
    double m;
    if (__neighbor->maxValue(secondary, x, y, m))
      return (m >= secondaryRange[0] && m <= secondaryRange[1]);
    else
      return false;
}

/*----------------------------------------------------------------*/
// return true if a good fit found and det/dir are set
bool ShearDetect::best_direction(const Grid2d &input, 
				 const int x, const int y, const int nx,
				 const int ny, const ShearDetectOffsets &o,
				 double shearMin, double &dir) const
{
  int k;
  double v, max;
  bool first;
  const ShearDetectOffset *best, *oi;
    
  // get best (biggest) non-fuzzy shear value.
  max = 0;
  best = NULL;
  first = true;
  for (k=0; k<o.num(); ++k)
  {
    if ((oi = o.ith_offset(k)) == NULL)
      continue;
    if (oi->out_of_range(x, y, nx, ny))
      continue;
    if (!_shear_detect(input, x, y, *oi, v))
      continue;
    if (first)
    {
      max = v;
      first = false;
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
  if (max <= shearMin)
    return false;

  dir = best->get_angle();
  return true;
}

/*----------------------------------------------------------------*/
// return true if a good fit found and det/dir are set
bool ShearDetect::valueAtDirection(const Grid2d &input, double direction,
				   int x, int y,
				   int nx, int ny, const ShearDetectOffsets &o,
				   double cutAngle, bool isConvergent,
				   double &det) const
{
    // use direction to get an offset pointer
    const ShearDetectOffset *best = o.matchingDirection(direction);
    if (best == NULL)
    {
      return false;
    }
    
    // compute the fuzzy shear value.
    return _fuzzy_shear_detect(x, y, ny, input, *best, cutAngle, isConvergent,
			       det);
}


/*----------------------------------------------------------------*/
bool ShearDetect::_shear_detect(const Grid2d &input, const int x,
				const int y, const ShearDetectOffset &s,
				double &v) const
{
    double mr, ml;
    const Grid2dOffset *ol, *ior;

    // if more than half the left or right pixels are missing, bad
    ol = s.get_left();
    if (ol->percentBad(input, x, y) > 0.5)
      return false;
    ior = s.get_right();
    if (ior->percentBad(input, x, y) > 0.5)
      return false;
    
    // get absolute value of difference between ave. left and right pixels
    if (ior->average(input, x, y, mr) && ol->average(input, x, y, ml))
    {
      v = mr - ml;
      v = fabs(v);
      return true;
    }
    else
      return false;
}

/*----------------------------------------------------------------*/
bool ShearDetect::_fuzzy_shear_detect(int x, int y, int ny, 
				      const Grid2d &input,
				      const ShearDetectOffset &s,
				      double cutAngle,
				      bool isConvergent,
				      double &v) const
{
    double mean, meanr, meanl;
    const Grid2dOffset *ol, *ior;
    vector<double> pixr, pixl;

    // from left offset table, get mean pixel value.
    ol = s.get_left();
    if (!ol->average(input, x, y, meanl))
      return false;

    // same with right offset table.
    ior = s.get_right();
    if (!ior->average(input, x, y, meanr))
      return false;

    // check the direction for convergent shear.
    if (!_direction_good(x, y, ny, s, meanr, meanl, cutAngle, isConvergent))
      return false;
    
    // fill right/left offset pixel vector
    pixr = ior->fillPixels(input, x, y);
    pixl = ol->fillPixels(input, x, y);

    // get average of all the pixels.
    mean = (meanr+meanl)/2.0;

    // compute the shear across the template compared to average.
    if (meanr >= meanl)
      v = _shear_detection_value(pixr, pixl, input.getMissing(), mean);
    else
      v = _shear_detection_value(pixl, pixr, input.getMissing(), mean);
   return true;
}

/*----------------------------------------------------------------*/
bool ShearDetect::_direction_good(int x, int y, int ny,
				  const ShearDetectOffset &s, 
				  double mr, double ml,
				  double cutAngle,
				  bool isConvergent) const
{  
    double beta, right;
  
    // get the angle pointing from the radar.
    beta = _angle_from_radar(x, y, ny, _proj);

    // right pointing angle out of template.
    right = s.get_right_angle();
    
    // if radar points left to right, want convergence left to right, i.e.
    // ml > mr.  If opposite, want convergence right to left, i.e.
    // mr > ml.
    if (_radar_points_left_to_right(beta, right, cutAngle))
    {
	if (isConvergent)
	    return ml > mr;
	else
	    return mr > ml;
    }
    else if (_radar_points_right_to_left(beta, right, cutAngle))
    {
        if (isConvergent)
	    return mr > ml;
	else
	    return ml > mr;
    }
    else
        return false;  // radar is parallel to template orientation.
}

/*----------------------------------------------------------------*/
double ShearDetect::_shear_detection_value (vector<double> &pixup,
					    vector<double> &pixlow, 
					    double missing, 
					    double mean) const
{
    double v, cnt;
    
    // get the accumulated interest values mapped from up and low
    // templates at the up and low pixels, respectively.
    v = cnt = 0.0;
    _accumulate(pixup, missing, mean, true, v, cnt);
    _accumulate(pixlow, missing, mean, false, v, cnt);
    if (cnt == 0.0)
	return 0;
    else
	return (v/cnt);
}

/*----------------------------------------------------------------*/
// accumulate into v and cnt from input pixel set applying fuzzy tbl
// function.
void ShearDetect::_accumulate(vector<double> &pix, double missing,
			      double mean, bool above_mean, double &v,
			      double &cnt) const
{
    double val;
    
    for (int i=0; i<(int)pix.size(); ++i)
    {
      double vi = pix[i];
      if (vi == missing)
	continue;

      // want difference from mean value.
      if (above_mean)
	val = vi - mean;
      else
	val = mean - vi;
	
      // then apply the fuzzy function to that.
      v += __p._shearDetectSide.apply(val);
      cnt++;
    }
}
    
/*----------------------------------------------------------------*/
/*
 * First time or something changed...compute tables
 */
void ShearDetect::_compute_neighbor(int nx, double missing)
{
    if (__neighbor != NULL)
        delete __neighbor;

    __neighbor = new Grid2dOffset(_test_circle_diameter/2.0, nx, missing);
    __neighbor_max_off = __neighbor->maxOffset();
}


