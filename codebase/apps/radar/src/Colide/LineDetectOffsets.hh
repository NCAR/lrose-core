/**
 * @file LineDetectOffsets.hh
 * @brief This is the template for building line detections at one angle.
 * @class LineDetectOffsets
 * @brief This is the template for building line detections at one angle.
 *
 * It has a center offset, left and right side offsets, and a double
 * resolution left and right side offsets.
 */
# ifndef    LineDetectOffsets_H
# define    LineDetectOffsets_H

#include <euclid/Grid2dOffset.hh>
#include <vector>

class Parms;
class Window;

class LineDetectOffset 
{
public:
  LineDetectOffset(const Parms &p,
		   const Window &window,
		   int nx, int ny, 
		   int i,  // angle index
		   int width, // of image
		   double missing
		   );
  inline const Grid2dOffset *get_center(void) const {return &_center;}
  inline const Grid2dOffset *get_left(void) const {return &_left;}
  inline const Grid2dOffset *get_right(void) const {return &_right;}
  inline const Grid2dOffset *get_leftd(void) const {return &_leftd;}
  inline const Grid2dOffset *get_rightd(void) const {return &_rightd;}
  ~LineDetectOffset();
  inline double get_angle(void) const {return _angle;}
  inline int max_offset(void) const
  {
    int m;
    m = _center.maxOffset();
    m = _max_int_2(m, _left.maxOffset());
    m = _max_int_2(m, _right.maxOffset());
    m = _max_int_2(m, _leftd.maxOffset());
    m = _max_int_2(m, _rightd.maxOffset());
    return m;
  }
  inline bool out_of_range(int x, int y, int nx, int ny) const
  {
    double m = max_offset();
    return (x < m || y < m || x >= nx - m || y >= ny - m);
  }

private:
  Grid2dOffset _center;
  Grid2dOffset _left, _right; // left and right side offsets.
  Grid2dOffset _leftd;         // double resolution, left side offset
  Grid2dOffset _rightd;        // double resolution, right side offset.
  double _angle;
  static int _max_int_2(int v0, int v1);
};

/**
 * @class LineDetectOffsets
 * @brief the entire set of offset tables for a given line detection parameter.
 * 
 * It is a vector of LineDetectOffset, one for each orientation of the
 * template (rotation by angles).
 */
class LineDetectOffsets
{
public:
  LineDetectOffsets(const Parms &p, const Window &window, int nx, int ny,
		    int width, double missing);
  ~LineDetectOffsets();
  inline int num(void) const {return (int)_o.size();}
  inline const LineDetectOffset *ith_offset(int i) const
  {
    if (i < 0 || i >= (int)_o.size())
      return NULL;
    return &(*(_o.begin() + i));
  }
  const LineDetectOffset *matchingDirection(double dir) const;

private:
  std::vector<LineDetectOffset> _o;

};

# endif     /* CLDOFFSET_H */
