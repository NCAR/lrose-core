
/* System include files / Local include files */
#include "SVD.hh"
#include "RegAtt.hh"
#include <euclid/Line.hh>
#include <cstdio>

/*----------------------------------------------------------------*/
RegAttMeasure::RegAttMeasure()
{
  _mean = _min = _max = 0.0;
}

/*----------------------------------------------------------------*/
RegAttMeasure::RegAttMeasure(double meani, double mini, double maxi)
{
  _mean = meani;
  _min = mini;
  _max = maxi;
}

/*----------------------------------------------------------------*/
RegAttMeasure::RegAttMeasure(const PointList &r, bool is_length)
{
  _mean = _min = _max = 0.0;
  if (is_length)
    _build_length(r);
  else
    _build_width(r);
}

/*----------------------------------------------------------------*/
RegAttMeasure::~RegAttMeasure()
{
}

/*----------------------------------------------------------------*/
void RegAttMeasure::clear(void)
{
  _mean = _min = _max = 0.0;
}

/*----------------------------------------------------------------*/
void RegAttMeasure::print(const std::string label, FILE *fp) const
{
  fprintf(fp, "%s: min:%g  mean:%g  max:%g\n",
	  label.c_str(), _min, _mean, _max);
}

/*----------------------------------------------------------------*/
/*
 * Width means y dimension
 */
void RegAttMeasure::_build_width(const PointList &n)
{
    double x0, x1, x;
    double nv, nmin, nmax;

    /*
     * initialize
     */
    nv = 0.0;

    /*
     * Look at each x
     */
    x0 = n.minX();
    x1 = n.maxX();
    for (x=x0; x<=x1; ++x)
        /*
	 * See if x value is in neighborhood somewhere
	 */
        if (n.yRangeOverX(x-0.5, x+0.5, nmin, nmax))
	    /*
	     * Use this range to update stuff
	     */
	    _update_measure_values(nmin, nmax, nv);

    /*
     * Finish
     */
    if (nv > 0)
	_mean = _mean/nv;
}

/*----------------------------------------------------------------*/
/*
 * Length means x dimension
 */
void RegAttMeasure::_build_length(const PointList &n)
{
    double y0, y1, y;
    double nv, nmin, nmax;

    /*
     * initialize
     */
    nv = 0.0;

    /*
     * Look at each y
     */
    y0 = n.minY();
    y1 = n.maxY();
    for (y=y0; y<=y1; ++y)
        /*
	 * See if x value is in neighborhood somewhere
	 */
        if (n.xRangeOverY(y-0.5, y+0.5, nmin, nmax))
	    /*
	     * Use this range to update stuff
	     */
	    _update_measure_values(nmin, nmax, nv);

    /*
     * Finish
     */
    if (nv > 0)
	_mean = _mean/nv;
}

/*----------------------------------------------------------------*/
/*
 * Update the measure struct using thses inputs
 */
void RegAttMeasure::_update_measure_values(double imin, 
					   double imax, double &nv)
{
    double len;

    /*
     * length is extent from min to max
     */
    len = (double)(imax - imin + 1.0);
    if (nv == 0)
	_min = _max = len;
    else
    {
	if (len < _min)
	    _min = len;
	if (len > _max)
	    _max = len;
    }
    _mean += len;
    nv++;
}

/*----------------------------------------------------------------*/
/*----------------------------------------------------------------*/
/*
 * Set attributes from locations and image.
 */
RegAtt::RegAtt(const PointList &loc, const Grid2d &data,
	       const Grid2d &lineData, const ParmRegion &parm) :
  _minx(loc), _miny(loc), _maxx(loc), _maxy(loc) // temp fill in.
{
  /*
   * Compute area
   */
  _area = loc.size();

  /*
   * Build up x,y minimum, max stuff.
   */
  if (loc.size() > 0)
    _set(loc);
  else
    _set();

  /*
   * Build up length and width
   */
  _neighbors_width_length(loc, data, lineData, parm);
}

/*----------------------------------------------------------------*/
RegAtt::~RegAtt()
{
}

/*----------------------------------------------------------------*/
void RegAtt::clear(void)
{
  _clear();
}

/*----------------------------------------------------------------*/
void RegAtt::print(void) const
{
  print(stdout);
}

/*----------------------------------------------------------------*/
void RegAtt::print(FILE *fp) const
{
  fprintf(fp, "\tarea:%g\n", _area);
  _len.print("len:", stdout);
  _width.print("width:", stdout);
  fprintf(fp, "Minx:\n");
  _minx.print(fp);
  fprintf(fp, "Miny:\n");
  _miny.print(fp);
  fprintf(fp, "Maxx:\n");
  _maxx.print(fp);
  fprintf(fp, "Maxy:\n");
  _maxy.print(fp);
  fprintf(fp, "\tlower_left:%lf,%lf\n", _min_x_at_min_y.first,
	  _min_x_at_min_y.second);
  fprintf(fp, "\n");
}

/*----------------------------------------------------------------*/
/*
 * return true if the regions sizes out ok
 */
bool RegAtt::ok(const ParmRegion &p, bool print) const
{
  bool stat;
  stat = _is_ok(p);
  if (!stat && print)
    _ok_print(p);
  return stat;
}

#ifdef NOTDEF
/*----------------------------------------------------------------*/
/*
 * Return the extreme values pulled out of the attributes,
 * adjusted by maxdist (expanded).
 */
cldBox RegAtt::extrema(const int nx, const int ny, double maxdist) const
{
  double x0, y0, x1, y1;
    
  x0 = _minx.min_x() - maxdist;
  y0 = _miny.min_y() - maxdist;
  x1 = _maxx.max_x() + maxdist;
  y1 = _maxy.max_y() + maxdist;
    
  //     /*
  //      * Get image bounds
  //      */
  //     nx = Parm->_proj._nx;
  //     ny = Parm->_proj._ny;

  if (x0 < 0)
    x0 = 0;
  if (y0 < 0)
    y0 = 0;
  if (x1 >= (double)nx)
    x1 = (double)nx - 1.0;
  if (y1 >= (double)ny)
    y1 = (double)ny - 1.0;
    
  return cldBox(x0, y0, x1, y1);
}
#endif

/*----------------------------------------------------------------*/
bool RegAtt::is_empty(void) const
{
  if (_area != 0.0)
    return false;
  if (!_len.is_empty())
    return false;
  if (!_width.is_empty())
    return false;
  if (_minx.size() > 0)
    return false;
  if (_miny.size() > 0)
    return false;
  if (_maxx.size() > 0)
    return false;
  if (_maxy.size() > 0)
    return false;
  return true;
}

/*----------------------------------------------------------------*/
void RegAtt::recompute(const Grid2d &g, const PointList &loc)
{
  /*
   * Compute area
   */
  _area = loc.size();

  /*
   * Build up x,y minimum, max stuff.
   */
  if (_area > 0)
    _set(loc);
  else
    _set();

  /*
   * Build up length and width
   * using fake constant everywhere image.
   */
  Grid2d img(g);
  img.setAllToValue(100);
  _neighbors_width_length(loc, img);
}

/*----------------------------------------------------------------*/
void RegAtt::_clear(void)
{
  _area = 0.0;
  _len.clear();
  _width.clear();
  _minx.clear();
  _miny.clear();
  _maxx.clear();
  _maxy.clear();
  _min_x_at_min_y.first = 0;
  _min_x_at_min_y.second = 0;//= cldXY(0.0, 0.0);
}

/*----------------------------------------------------------------*/
/*
 * compute "width" and "len"
 */
void RegAtt::_neighbors_width_length(const PointList &loc,
				     const Grid2d &image)
{
  Line line;
  double angle;
    
  /*
   * Fit data to line
   */
  SVD s(loc);
  if (s.lsq_min(image, line))
    // Rotate line to horizontal
    line.makeHorizontal(angle, false);
  else
    angle = 0.0;

  /*
   * Rotate neighborhood data to same coordinates
   */
  PointList rlist(loc);
  rlist.rotate(angle);

  /*
   * Compute width and len based on this data
   */
  _len = RegAttMeasure(rlist, true);
  _width = RegAttMeasure(rlist, false);
}

/*----------------------------------------------------------------*/
/*
 * compute "width" and "len"
 */
void RegAtt::_neighbors_width_length(const PointList &loc,
				     const Grid2d &data,
				     const Grid2d &lineData,
				     const ParmRegion &parm)
{
  Line line;
  double angle;
    
  /*
   * Fit data to line
   */
  SVD s(loc);
  if (!s.lsq_min(data, lineData, line))
    angle = 0.0;
  else
    // Rotate line to horizontal
    line.makeHorizontal(angle, false);
    
  /*
   * Rotate neighborhood data to same coordinates
   */
  PointList rlist(loc);
  rlist.rotate(angle);

  /*
   * Compute width and len based on this data
   */
  _len = RegAttMeasure(rlist, true);
  _width = RegAttMeasure(rlist, false);
}

/*----------------------------------------------------------------*/
bool RegAtt::_is_ok(const ParmRegion &p) const
{
  if (_area < p._min_area)
    return false;
  if (!_width.min_ok(p._min_min_length) &&
      !_len.min_ok(p._min_min_length))
    return false;
  if (!_width.max_ok(p._min_max_length) &&
      !_len.max_ok(p._min_max_length))
    return false;
  if (!_width.mean_ok(p._min_mean_length) &&
      !_len.mean_ok(p._min_mean_length))
    return false;
  return true;
}


/*----------------------------------------------------------------*/
/*
 * Print out any ok failures..
 */
void RegAtt::_ok_print(const ParmRegion &p) const
{
  printf("Region will not be kept\n");
  if (_area < p._min_area)
    printf("  area = %f  minimum = %f\n", _area, p._min_area);
  if (!_width.min_ok(p._min_min_length))
    printf("  min width = %f  minimum = %f\n", _width.get_min(),
	   p._min_min_length);
  if (!_len.min_ok(p._min_min_length))
    printf("  min len = %f  minimum = %f\n", _len.get_min(),
	   p._min_min_length);
  if (!_width.mean_ok(p._min_mean_length))
    printf("  mean width = %f  minimum = %f\n", _width.get_mean(),
	   p._min_mean_length);
  if (!_len.mean_ok(p._min_mean_length))
    printf("  mean len = %f  minimum = %f\n", _len.get_mean(),
	   p._min_mean_length);

  if (!_width.max_ok(p._min_max_length))
    printf("  max width = %f  minimum = %f\n", _width.get_max(),
	   p._min_max_length);
  if (!_len.max_ok(p._min_max_length))
    printf("  max len = %f  minimum = %f\n", _len.get_max(),
	   p._min_max_length);
}

/*----------------------------------------------------------------*/
void RegAtt::_set(const PointList &iloc)
{
  double x, y, y0, x0;

  _minx = PointList(iloc);
  _maxx = PointList(iloc);
  _miny = PointList(iloc);
  _maxy = PointList(iloc);

  x0 = iloc.minX();
  PointList p = _minx.commonX(x0);
  _minx = p;

  x = iloc.maxX();
  p = _maxx.commonX(x);
  _maxx = p;

  y0 = y = iloc.minY();
  p = _miny.commonY(y0);
  _miny = p;

  y = iloc.maxY();
  p = _maxy.commonY(y);
  _maxy = p;

  iloc.xRangeOverY(y0, y0, x0, x);
  _min_x_at_min_y.first = x0;
  _min_x_at_min_y.second = y0;
}

/*----------------------------------------------------------------*/
void RegAtt::_set(void)
{
  _minx.clear();
  _maxx.clear();
  _miny.clear();
  _maxy.clear();
  _min_x_at_min_y.first = -1;
  _min_x_at_min_y.second = -1;
}
