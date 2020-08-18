/**
 * @file EnhancementOffsetD.hh
 * @brief offset indices into images, used as a lookup table.
 * @class EnhancementOffsetD
 * @brief offset indices into images, used as a lookup table.
 */
# ifndef    EnhancementOffsetD_HH
# define    EnhancementOffsetD_HH

#include "Window.hh"
#include <euclid/Grid2dOffset.hh>
#include <vector>

class Grid2d;

/*----------------------------------------------------------------*/
class EnhancementOffsetD
{
public:
  EnhancementOffsetD(int i, const Window &w, int xwidth, int missing);
  virtual ~EnhancementOffsetD();
  inline int max_offset(void) const
  {
    return _max_int_3(_center.maxOffset(),
		      _left.maxOffset(),
		      _right.maxOffset());
  }

  inline bool out_of_range(int x, int y, int nx, int ny) const
  {
    double m = max_offset();
    return (x < m || y < m || x >= nx - m || y >= ny - m);
  }

  inline const Grid2dOffset *get_center(void) const {return &_center;}
  inline const Grid2dOffset *get_left(void) const {return &_left;}
  inline const Grid2dOffset *get_right(void)  const {return &_right;}
  inline double max_side(const Grid2d &g, const int x, const int y) const
  {
    double v0, v1;
    v0 = _left.maxValueOrZero(g, x,  y);
    v1 = _right.maxValueOrZero(g, x, y);
    return _max_double_2(v0, v1);
  }
  inline double get_angle(void) const {return _angle;}

private:
  Grid2dOffset _center;  //!< center has an offset
  Grid2dOffset _left;    //!< left has another
  Grid2dOffset _right;   //!< right has another
  double _angle;       //!< rotaion angle for these offsets.

  inline int _max_int_3(int v0, int v1, int v2) const
  {
    if (v0 < v1)
      return _max_int_2(v1, v2);
    else
      return _max_int_2(v0, v2);
  }
  inline int _max_int_2(int v0, int v1) const
  {
    if (v0 < v1)
      return v1;
    else
      return v0;
  }
  inline double _max_double_2(double v0, double v1) const
  {
    if (v0 < v1)
      return v1;
    else
      return v0;
  }
};

/*----------------------------------------------------------------*/

/** @class EnhancementOffsetsD
 *  @brief the entire set of offset tables for a given enhancement.
 *
 * it is a vector of EnhancementOffsetD, one for each orientation of the
 * template (rotation by angles).
 */
class EnhancementOffsetsD
{
 public:
    EnhancementOffsetsD(const Window &w, int width, int missing);
    virtual ~EnhancementOffsetsD();
    inline int num(void) const {return _o.size();}
    inline const EnhancementOffsetD *ith_offset(int i) const
    {
      if (i < 0 || i >= (int)_o.size())
        return NULL;
      return &(*(_o.begin() + i));
    }
 private:
  std::vector<EnhancementOffsetD> _o;
};

# endif     /* CLDOFFSET_H */
