/**
 * @file ShearDetectOffsets.hh
 * @brief  offset indices into images, used as a lookup table.
 * @class ShearDetectOffsets
 * @brief  offset indices into images, used as a lookup table.
 *
 * the entire set of offset tables for a given shear detection parameter:
 * it is a vector of ShearDetectOffset, one for each orientation of the
 * template (rotation by angles).
 */
# ifndef    CLD_SHEAROFFSET_H
# define    CLD_SHEAROFFSET_H

#include <vector>
#include "Parms.hh"
#include <euclid/Grid2dOffset.hh>

class ShearDetectOffset;
class Window;

class ShearDetectOffsets
{
 public:
  ShearDetectOffsets(const Parms &p, const Window &window, int nx, int ny,
		     int width, double missing);
    ~ShearDetectOffsets();
    inline int num(void) const {return __o.size();}
    inline const ShearDetectOffset *ith_offset(int i) const
    {
      if (i < 0 || i >= (int)__o.size())
        return NULL;
      return &(__o[i]);
    }
  const ShearDetectOffset *matchingDirection(double dir) const;

 private:

    vector<ShearDetectOffset> __o;
};

/**
 * @class ShearDetectOffset
 * @brief this is template for building shear detections
 *
 * it has a left and right side offset template.
 */
class ShearDetectOffset
{
 public:
    ShearDetectOffset(const Grid2dOffset &left,
		      const Grid2dOffset &right,
		      double angle);
  ShearDetectOffset(const Parms &p, const Window &window, int nx, int ny,int i,
		    int width, double missing);
    ~ShearDetectOffset();

    inline const Grid2dOffset *get_left(void) const {return &__sidel;}
    inline const Grid2dOffset *get_right(void) const {return &__sider;}
    inline double get_angle(void) const {return __angle;}
    //since the angles are counterclockwise from north orientation of
    //the template, mathematical 'right' is equal to the angle.
    inline double get_right_angle(void) const {return __angle;}
    inline int max_offset(void) const
    {
      return _max_int_2(__sidel.maxOffset(), 
			__sider.maxOffset());
    }
    inline bool out_of_range(int x, int y, int nx, int ny) const
    {
      double m = max_offset();
      return (x < m || y < m || x >= nx - m || y >= ny - m);
    }

 private:

  Grid2dOffset __sider;
  Grid2dOffset __sidel;
  double __angle;
  static int _max_int_2(int v0, int v1);
};


/* External global variables / Non-static global variables / Static globals */


/* External functions / Internal global functions / Internal static functions */
# endif     /* CLDOFFSET_H */


